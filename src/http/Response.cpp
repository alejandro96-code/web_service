#include "Response.hpp"
#include <sys/stat.h>

// Constructor
Response::Response(const Request& request, const std::string& documentRoot,
                   const std::map<int, std::string>& errorPages,
                   const std::vector<Location>& locations,
                   size_t clientMaxBodySize,
                   const std::string& index)
    : _statusCode(200), _statusMessage("OK"), _documentRoot(documentRoot), 
      _index(index), _errorPages(errorPages), _locations(locations), _clientMaxBodySize(clientMaxBodySize)
{
    try {
        procesar(request);
    }
    catch (const std::exception& e) {
        _statusCode = HttpStatus::INTERNAL_SERVER_ERROR;
        _statusMessage = HttpStatus::getMessage(HttpStatus::INTERNAL_SERVER_ERROR);
        ErrorHandler::ErrorResponse errorResp = ErrorHandler::generarRespuestaError(
            _statusCode, _statusMessage, _errorPages);
        _body = errorResp.body;
        _headers["Content-Type"] = errorResp.contentType;
    }
}

Response::~Response() {}

// Procesar la petición y generar respuesta
void Response::procesar(const Request& request)
{
    std::string method = request.getMethod();
    std::string path = request.getPath();
    
    // Validar que el root no esté vacío y que exista
    if (_documentRoot.empty() || !FileUtils::esDirectorio(_documentRoot)) {
        _statusCode = HttpStatus::NOT_FOUND;
        _statusMessage = HttpStatus::getMessage(HttpStatus::NOT_FOUND);
        ErrorHandler::ErrorResponse errorResp = ErrorHandler::generarRespuestaError(
            _statusCode, _statusMessage, _errorPages);
        _body = errorResp.body;
        _headers["Content-Type"] = errorResp.contentType;
        return;
    }
    
    // Validar que el index no esté vacío y que exista en el root
    if (_index.empty()) {
        _statusCode = HttpStatus::NOT_FOUND;
        _statusMessage = HttpStatus::getMessage(HttpStatus::NOT_FOUND);
        ErrorHandler::ErrorResponse errorResp = ErrorHandler::generarRespuestaError(
            _statusCode, _statusMessage, _errorPages);
        _body = errorResp.body;
        _headers["Content-Type"] = errorResp.contentType;
        return;
    }
    
    // Validar que el archivo index exista en el root
    std::string indexPath = _documentRoot;
    if (indexPath[indexPath.length() - 1] != '/')
        indexPath += "/";
    indexPath += _index;
    
    std::string contenidoIndex = FileUtils::leerArchivo(indexPath);
    if (contenidoIndex.empty()) {
        _statusCode = HttpStatus::NOT_FOUND;
        _statusMessage = HttpStatus::getMessage(HttpStatus::NOT_FOUND);
        ErrorHandler::ErrorResponse errorResp = ErrorHandler::generarRespuestaError(
            _statusCode, _statusMessage, _errorPages);
        _body = errorResp.body;
        _headers["Content-Type"] = errorResp.contentType;
        return;
    }
    
    // Validar que el método no esté vacío
    if (method.empty()) {
        _statusCode = HttpStatus::BAD_REQUEST;
        _statusMessage = HttpStatus::getMessage(HttpStatus::BAD_REQUEST);
        ErrorHandler::ErrorResponse errorResp = ErrorHandler::generarRespuestaError(
            _statusCode, _statusMessage, _errorPages);
        _body = errorResp.body;
        _headers["Content-Type"] = errorResp.contentType;
        return;
    }
    
    // Validar tamaño del body solo para POST (único método implementado que acepta body)
    if (method == "POST") {
        std::string contentLengthStr = request.getHeader("Content-Length");
        if (!contentLengthStr.empty()) {
            size_t contentLength = atoi(contentLengthStr.c_str());
            if (contentLength > _clientMaxBodySize) {
                _statusCode = HttpStatus::PAYLOAD_TOO_LARGE;
                _statusMessage = HttpStatus::getMessage(HttpStatus::PAYLOAD_TOO_LARGE);
                ErrorHandler::ErrorResponse errorResp = ErrorHandler::generarRespuestaError(
                    _statusCode, _statusMessage, _errorPages);
                _body = errorResp.body;
                _headers["Content-Type"] = errorResp.contentType;
                return;
            }
        }
    }
    
    // Verificar si hay redirección configurada para esta ruta
    Location* loc = LocationMatcher::obtenerLocation(path, _locations);
    if (loc != NULL && loc->has_redirect) {
        _statusCode = loc->redirect_code;
        _statusMessage = HttpStatus::getMessage(loc->redirect_code);
        _headers["Location"] = loc->redirect_url;
        _body = "";
        return;
    }
    
    // Validar que el método está permitido para esta ruta
    if (!LocationMatcher::metodoPermitido(path, method, _locations)) {
        _statusCode = HttpStatus::METHOD_NOT_ALLOWED;
        _statusMessage = HttpStatus::getMessage(HttpStatus::METHOD_NOT_ALLOWED);
        ErrorHandler::ErrorResponse errorResp = ErrorHandler::generarRespuestaError(
            _statusCode, _statusMessage, _errorPages);
        _body = errorResp.body;
        _headers["Content-Type"] = errorResp.contentType;
        return;
    }
    
    if (method == "GET") {
        manejarGET(request);
    }
    else if (method == "POST") {
        manejarPOST(request);
    }
    else if (method == "DELETE") {
        manejarDELETE(request);
    }
    else if (method == "PUT" || method == "PATCH" || method == "HEAD" || 
             method == "OPTIONS" || method == "TRACE" || method == "CONNECT") {
        _statusCode = HttpStatus::NOT_IMPLEMENTED;
        _statusMessage = HttpStatus::getMessage(HttpStatus::NOT_IMPLEMENTED);
        ErrorHandler::ErrorResponse errorResp = ErrorHandler::generarRespuestaError(
            _statusCode, _statusMessage, _errorPages);
        _body = errorResp.body;
        _headers["Content-Type"] = errorResp.contentType;
    }
    else {
        _statusCode = HttpStatus::BAD_REQUEST;
        _statusMessage = HttpStatus::getMessage(HttpStatus::BAD_REQUEST);
        ErrorHandler::ErrorResponse errorResp = ErrorHandler::generarRespuestaError(
            _statusCode, _statusMessage, _errorPages);
        _body = errorResp.body;
        _headers["Content-Type"] = errorResp.contentType;
    }
}

// Convertir a string
std::string Response::toString() const
{
    std::ostringstream response;
    response << "HTTP/1.1 " << _statusCode << " " << _statusMessage << "\r\n";
    
    // Agregar Content-Length si no está presente
    if (_headers.find("Content-Length") == _headers.end() && !_body.empty()) {
        std::ostringstream length_stream;
        length_stream << _body.length();
        response << "Content-Length: " << length_stream.str() << "\r\n";
    }
    
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
         it != _headers.end(); ++it) {
        response << it->first << ": " << it->second << "\r\n";
    }
    
    response << "\r\n" << _body;
    return response.str();
}
