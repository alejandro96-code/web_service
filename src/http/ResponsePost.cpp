#include "Response.hpp"
#include "ErrorHandler.hpp"
#include <fstream>
#include <sstream>
#include <ctime>

/*
    Peticion POST:
    Se usa principalmente para enviar datos a un servidor para crear o actualizar un recurso.
    primero obtenemos si no es autoindex devolvemos in 404
        comprobamos que el body (que se haya seleccionado un archivo)
        si el body esta vacio devolvemos un 400
        generamos un time para darle nombre al archivo (archivo_fecha)
        añadimos la ruta del archivo y lo guardamos.
        si el archivo no tiene lectura devolvemos un 500
    La peticion ya se ha completado asi que leemos el contenido de goodPost
    damos el codigo 201 a la peticion e imprimos el template de goodPost.
*/
void Response::handlePOST(const Request& request)
{
    std::string path = request.getPath();
    
    if (path == "/autoindex")
    {
        std::string body = request.getBody();
        std::string contentType = request.getHeader("Content-Type");
        
        // Verificar si el body está vacío o si es un multipart sin archivo
        // Cuando el formulario se envía sin seleccionar archivo, el body contiene
        // solo los boundaries del multipart pero sin contenido real de archivo
        if (body.empty())
        {
            _statusCode = HttpStatus::BAD_REQUEST;
            _statusMessage = HttpStatus::getMessage(HttpStatus::BAD_REQUEST);
            ErrorHandler::ErrorResponse errorResp = ErrorHandler::generateErrorResponse(
                _statusCode, _statusMessage, _errorPages);
            _body = errorResp.body;
            _headers["Content-Type"] = errorResp.contentType;
            return;
        }
        
        // SOLO validar filename si es multipart/form-data
        // Si es POST normal (Content-Length) sin multipart, aceptar el body directamente
        if (contentType.find("multipart/form-data") != std::string::npos)
        {
            // Buscar el marcador de filename="" (sin archivo seleccionado)
            // o verificar si después del boundary no hay datos reales
            size_t filenamePos = body.find("filename=\"");
            if (filenamePos != std::string::npos)
            {
                size_t filenameEnd = body.find("\"", filenamePos + 10);
                if (filenameEnd != std::string::npos)
                {
                    std::string filename = body.substr(filenamePos + 10, filenameEnd - (filenamePos + 10));
                    // Si el filename está vacío, no hay archivo
                    if (filename.empty())
                    {
                        _statusCode = HttpStatus::BAD_REQUEST;
                        _statusMessage = HttpStatus::getMessage(HttpStatus::BAD_REQUEST);
                        ErrorHandler::ErrorResponse errorResp = ErrorHandler::generateErrorResponse(
                            _statusCode, _statusMessage, _errorPages);
                        _body = errorResp.body;
                        _headers["Content-Type"] = errorResp.contentType;
                        return;
                    }
                }
            }
            else
            {
                // Si es multipart pero no hay filename, es un POST inválido
                _statusCode = HttpStatus::BAD_REQUEST;
                _statusMessage = HttpStatus::getMessage(HttpStatus::BAD_REQUEST);
                ErrorHandler::ErrorResponse errorResp = ErrorHandler::generateErrorResponse(
                    _statusCode, _statusMessage, _errorPages);
                _body = errorResp.body;
                _headers["Content-Type"] = errorResp.contentType;
                return;
            }
        }
        
        std::time_t now = std::time(NULL);
        std::ostringstream filenameStream;
        filenameStream << "archivo_" << now << ".txt";
        
        std::string filePath = _documentRoot + "/autoindex/archivosSubidos/" + filenameStream.str();
        std::ofstream file(filePath.c_str(), std::ios::binary);
        if (!file.is_open()) {
            _statusCode = HttpStatus::INTERNAL_SERVER_ERROR;
            _statusMessage = HttpStatus::getMessage(HttpStatus::INTERNAL_SERVER_ERROR);
            ErrorHandler::ErrorResponse errorResp = ErrorHandler::generateErrorResponse(
                _statusCode, _statusMessage, _errorPages);
            _body = errorResp.body;
            _headers["Content-Type"] = errorResp.contentType;
            return;
        }
        file.write(body.c_str(), body.length());
        file.close();
        
        std::ifstream templateFile("templates/goodpost.html");
        std::stringstream buffer;
        buffer << templateFile.rdbuf();
        std::string htmlTemplate = buffer.str();
        templateFile.close();
        _statusCode = HttpStatus::CREATED;
        _statusMessage = HttpStatus::getMessage(HttpStatus::CREATED);
        _headers["Content-Type"] = "text/html";
        
        _body = htmlTemplate;
    }
    else {
        _statusCode = HttpStatus::NOT_FOUND;
        _statusMessage = HttpStatus::getMessage(HttpStatus::NOT_FOUND);
        ErrorHandler::ErrorResponse errorResp = ErrorHandler::generateErrorResponse(
            _statusCode, _statusMessage, _errorPages);
        _body = errorResp.body;
        _headers["Content-Type"] = errorResp.contentType;
    }
}
