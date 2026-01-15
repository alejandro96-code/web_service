#include "Response.hpp"
#include "FileUtils.hpp"
#include "LocationMatcher.hpp"
#include "ErrorHandler.hpp"

/*
    Peticion Get:
    El metodo que se usa para solicitar un recurso del servidor.
    guardamos en uan variable path la respuesta del servidor
    completamos la ruta con nuestra carpetaRoot
    Ahora tenemos 2 casos:
        Caso Si en directorio:
            Añadimos "/", si ya acaba en / cargarmos index.html (signifca que estamos en localhost:8080/)
            Guardamos en contenido la lectura del archivo
            primera verificacion: (si existe index.html) si lo hay leemos el contenido
            segunda verificacion: (si autoindex es on) si  lo tiene lo cargamos
                si alguna de esta falla sacara un error 403 forbidden
        Caso Si es un archivo -> lo lee
        Si no entra en ningun caso devuelve un 404 not found (porque significa que no encuentra
        ni index.html ni autoindex)
        Despues determinamos el content type que hemos usado para la variable contenido
*/
void Response::handleGET(const Request& request)
{
    std::string path = FileUtils::normalizePath(request.getPath());
    std::string fullPath = _documentRoot + path;
    
    if (FileUtils::isDirectory(fullPath))
    {
        std::string indexPath = fullPath;
        if (indexPath[indexPath.length() - 1] != '/')
            indexPath += "/";
        indexPath += _index;
        
        std::string content = FileUtils::readFile(indexPath);
        if (!content.empty())
        {
            _statusCode = HttpStatus::OK;
            _statusMessage = HttpStatus::getMessage(HttpStatus::OK);
            _headers["Content-Type"] = "text/html";
            _body = content;
            return;
        }
        if (LocationMatcher::hasAutoindex(path, _locations))
        {
            _statusCode = HttpStatus::OK;
            _statusMessage = HttpStatus::getMessage(HttpStatus::OK);
            _headers["Content-Type"] = "text/html";
            _body = Autoindex::generateHTML(fullPath, path);
            return;
        }
        _statusCode = HttpStatus::FORBIDDEN;
        _statusMessage = HttpStatus::getMessage(HttpStatus::FORBIDDEN);
        ErrorHandler::ErrorResponse errorResp = ErrorHandler::generateErrorResponse(
            _statusCode, _statusMessage, _errorPages);
        _body = errorResp.body;
        _headers["Content-Type"] = errorResp.contentType;
        return;
    }
    
    // Detectar si es un archivo CGI (.php, .py)
    if (CGIHandler::isCGI(fullPath))
    {
        std::string cgiOutput = CGIHandler::executeCGI(fullPath, request);
        
        if (cgiOutput.empty()) {
            _statusCode = HttpStatus::INTERNAL_SERVER_ERROR;
            _statusMessage = HttpStatus::getMessage(HttpStatus::INTERNAL_SERVER_ERROR);
            ErrorHandler::ErrorResponse errorResp = ErrorHandler::generateErrorResponse(
                _statusCode, _statusMessage, _errorPages);
            _body = errorResp.body;
            _headers["Content-Type"] = errorResp.contentType;
            return;
        }
        
        // La salida CGI puede incluir headers HTTP
        // Formato: "Content-Type: text/html\r\n\r\n<html>..."
        size_t headerEnd = cgiOutput.find("\r\n\r\n");
        
        if (headerEnd != std::string::npos) {
            // Parsear headers de la salida CGI
            std::string headers = cgiOutput.substr(0, headerEnd);
            std::string body = cgiOutput.substr(headerEnd + 4);
            
            // Extraer Content-Type si existe
            size_t ctPos = headers.find("Content-Type:");
            if (ctPos != std::string::npos) {
                size_t ctEnd = headers.find("\r\n", ctPos);
                std::string contentType = headers.substr(ctPos + 13, ctEnd - (ctPos + 13));
                // Limpiar espacios
                while (!contentType.empty() && contentType[0] == ' ')
                    contentType = contentType.substr(1);
                _headers["Content-Type"] = contentType;
            } else {
                _headers["Content-Type"] = "text/html";
            }
            
            _statusCode = HttpStatus::OK;
            _statusMessage = HttpStatus::getMessage(HttpStatus::OK);
            _body = body;
        } else {
            // No hay headers, toda la salida es el body
            _statusCode = HttpStatus::OK;
            _statusMessage = HttpStatus::getMessage(HttpStatus::OK);
            _headers["Content-Type"] = "text/html";
            _body = cgiOutput;
        }
        return;
    }
    
    // Archivo estático normal
    std::string content = FileUtils::readFile(fullPath);
    
    if (content.empty()) {
        _statusCode = HttpStatus::NOT_FOUND;
        _statusMessage = HttpStatus::getMessage(HttpStatus::NOT_FOUND);
        ErrorHandler::ErrorResponse errorResp = ErrorHandler::generateErrorResponse(
            _statusCode, _statusMessage, _errorPages);
        _body = errorResp.body;
        _headers["Content-Type"] = errorResp.contentType;
        return;
    }
    
    size_t dotPos = path.find_last_of('.');
    std::string extension = "";
    if (dotPos != std::string::npos) {
        extension = path.substr(dotPos);
    }
    _statusCode = HttpStatus::OK;
    _statusMessage = HttpStatus::getMessage(HttpStatus::OK);
    _headers["Content-Type"] = FileUtils::getContentType(extension);
    _body = content;
}
