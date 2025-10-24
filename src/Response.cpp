#include "Response.hpp"

// Constructor
Response::Response(const Request& request, const std::string& documentRoot)
    : _statusCode(200), _statusMessage("OK"), _documentRoot(documentRoot)
{
    procesar(request);
}

// Destructor
Response::~Response() {}

// Procesar la petición y generar respuesta
void Response::procesar(const Request& request)
{
    std::string method = request.getMethod();
    
    if (method == "GET") {
        manejarGET(request);
    }
    else if (method == "POST") {
        manejarPOST(request);
    }
    else if (method == "DELETE") {
        manejarDELETE(request);
    }
    else {
        // Método no permitido
        respuestaError(405);
    }
}

// Manejar peticiones GET
void Response::manejarGET(const Request& request)
{
    std::string path = request.getPath();
    
    // Ruta raíz "/"
    if (path == "/") {
        path = "/index.html";
    }
    
    // Construir la ruta completa al archivo
    std::string rutaCompleta = _documentRoot + path;
    
    // Leer el archivo
    std::string contenido = leerArchivo(rutaCompleta);
    
    if (contenido.empty()) {
        respuestaError(404);
        return;
    }
    
    // Determinar Content-Type
    size_t puntoPos = path.find_last_of('.');
    std::string extension = "";
    if (puntoPos != std::string::npos) {
        extension = path.substr(puntoPos);
    }
    
    _statusCode = 200;
    _statusMessage = "OK";
    _headers["Content-Type"] = obtenerContentType(extension);
    _body = contenido;
}

// Manejar peticiones POST
void Response::manejarPOST(const Request& request)
{
    std::string path = request.getPath();
    
    if (path == "/upload") {
        // TODO: Implementar lógica de subida de archivos
        _statusCode = 200;
        _statusMessage = "OK";
        _headers["Content-Type"] = "text/html";
        _body = "<html><body><h1>Archivo subido exitosamente</h1><a href='/'>Volver</a></body></html>";
    }
    else {
        respuestaError(404);
    }
}

// Manejar peticiones DELETE
void Response::manejarDELETE(const Request& request)
{
    std::string path = request.getPath();
    
    // Verificar que la ruta empieza con /files/
    if (path.find("/files/") == 0) {
        // Extraer el nombre del archivo
        std::string nombreArchivo = path.substr(7); // Quitar "/files/"
        
        // TODO: Implementar lógica de eliminación de archivos
        _statusCode = 200;
        _statusMessage = "OK";
        _headers["Content-Type"] = "text/html";
        _body = "<html><body><h1>Archivo eliminado</h1></body></html>";
    }
    else {
        respuestaError(404);
    }
}

// Generar respuesta de error
void Response::respuestaError(int codigo)
{
    _statusCode = codigo;
    
    std::string rutaError;
    
    switch (codigo) {
        case 403:
            _statusMessage = "Forbidden";
            rutaError = _documentRoot + "/error403.html";
            break;
        case 404:
            _statusMessage = "Not Found";
            rutaError = _documentRoot + "/error404.html";
            break;
        case 405:
            _statusMessage = "Method Not Allowed";
            rutaError = _documentRoot + "/error405.html";
            break;
        case 500:
            _statusMessage = "Internal Server Error";
            rutaError = _documentRoot + "/error500.html";
            break;
        default:
            _statusMessage = "Error";
            _body = "<html><body><h1>Error " + std::string(1, '0' + codigo) + "</h1></body></html>";
            _headers["Content-Type"] = "text/html";
            return;
    }
    
    // Intentar leer el archivo de error personalizado
    std::string contenido = leerArchivo(rutaError);
    if (!contenido.empty()) {
        _body = contenido;
    } else {
        _body = "<html><body><h1>Error " + std::string(1, '0' + codigo) + "</h1></body></html>";
    }
    
    _headers["Content-Type"] = "text/html";
}

// Leer archivo del sistema
std::string Response::leerArchivo(const std::string& ruta)
{
    std::ifstream file(ruta.c_str());
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return buffer.str();
}

// Obtener Content-Type según extensión
std::string Response::obtenerContentType(const std::string& extension)
{
    if (extension == ".html" || extension == ".htm") {
        return "text/html";
    }
    else if (extension == ".css") {
        return "text/css";
    }
    else if (extension == ".js") {
        return "application/javascript";
    }
    else if (extension == ".json") {
        return "application/json";
    }
    else if (extension == ".png") {
        return "image/png";
    }
    else if (extension == ".jpg" || extension == ".jpeg") {
        return "image/jpeg";
    }
    else if (extension == ".gif") {
        return "image/gif";
    }
    else if (extension == ".txt") {
        return "text/plain";
    }
    else {
        return "application/octet-stream";
    }
}

// Construir respuesta HTTP completa
std::string Response::construirRespuestaHTTP()
{
    std::ostringstream response;
    
    // Línea de estado
    response << "HTTP/1.1 " << _statusCode << " " << _statusMessage << "\r\n";
    
    // Headers
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
         it != _headers.end(); ++it) {
        response << it->first << ": " << it->second << "\r\n";
    }
    
    // Línea vacía que separa headers del body
    response << "\r\n";
    
    // Body
    response << _body;
    
    return response.str();
}

// Convertir a string (alias de construirRespuestaHTTP)
std::string Response::toString() const
{
    std::ostringstream response;
    
    // Línea de estado
    response << "HTTP/1.1 " << _statusCode << " " << _statusMessage << "\r\n";
    
    // Headers
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
         it != _headers.end(); ++it) {
        response << it->first << ": " << it->second << "\r\n";
    }
    
    // Línea vacía que separa headers del body
    response << "\r\n";
    
    // Body
    response << _body;
    
    return response.str();
}

// Getters
int Response::getStatusCode() const {
    return _statusCode;
}

std::string Response::getBody() const {
    return _body;
}
