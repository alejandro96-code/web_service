#include "Response.hpp"
#include <sys/stat.h>

// Constructor
Response::Response(const Request& request, const std::string& documentRoot,
                   const std::map<int, std::string>& errorPages,
                   const std::vector<Location>& locations)
    : _statusCode(200), _statusMessage("OK"), _documentRoot(documentRoot), 
      _errorPages(errorPages), _locations(locations)
{
    try {
        procesar(request);
    }
    catch (const std::exception& e) {
        respuestaError(HttpStatus::INTERNAL_SERVER_ERROR);
    }
}

Response::~Response() {}

// Procesar la petición y generar respuesta
void Response::procesar(const Request& request)
{
    std::string method = request.getMethod();
    std::string path = request.getPath();
    
    // Validar que el método no esté vacío
    if (method.empty()) {
        respuestaError(HttpStatus::BAD_REQUEST);
        return;
    }
    
    // Validar que el método está permitido para esta ruta
    if (!metodoPermitido(path, method)) {
        respuestaError(HttpStatus::METHOD_NOT_ALLOWED);
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
        respuestaError(HttpStatus::NOT_IMPLEMENTED);
    }
    else {
        respuestaError(HttpStatus::BAD_REQUEST);
    }
}

// Convertir a string
std::string Response::toString() const
{
    std::ostringstream response;
    response << "HTTP/1.1 " << _statusCode << " " << _statusMessage << "\r\n";
    
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
         it != _headers.end(); ++it) {
        response << it->first << ": " << it->second << "\r\n";
    }
    
    response << "\r\n" << _body;
    return response.str();
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
    if (extension == ".html" || extension == ".htm") return "text/html";
    if (extension == ".css") return "text/css";
    if (extension == ".js") return "application/javascript";
    if (extension == ".json") return "application/json";
    if (extension == ".png") return "image/png";
    if (extension == ".jpg" || extension == ".jpeg") return "image/jpeg";
    if (extension == ".gif") return "image/gif";
    if (extension == ".txt") return "text/plain";
    return "application/octet-stream";
}

// Verificar si una ruta es un directorio
bool Response::esDirectorio(const std::string& ruta)
{
    struct stat info;
    if (stat(ruta.c_str(), &info) != 0) {
        return false;
    }
    return S_ISDIR(info.st_mode);
}

// Verificar si la ruta tiene autoindex activado
bool Response::tieneAutoindex(const std::string& path)
{
    Location* loc = obtenerLocation(path);
    return (loc != NULL) ? loc->autoindex : false;
}

// Obtener la location que coincide con el path
Location* Response::obtenerLocation(const std::string& path)
{
    Location* mejor = NULL;
    size_t mayorLongitud = 0;
    
    for (size_t i = 0; i < _locations.size(); i++) {
        std::string locPath = _locations[i].path;
        if (path.find(locPath) == 0 && locPath.length() > mayorLongitud) {
            mayorLongitud = locPath.length();
            mejor = &_locations[i];
        }
    }
    return mejor;
}

// Verificar si el método está permitido para esta ruta
bool Response::metodoPermitido(const std::string& path, const std::string& method)
{
    Location* loc = obtenerLocation(path);
    if (loc == NULL || loc->allow_methods.empty()) {
        return true;
    }
    
    for (size_t i = 0; i < loc->allow_methods.size(); i++) {
        if (loc->allow_methods[i] == method) {
            return true;
        }
    }
    return false;
}

// Generar respuesta de error
void Response::respuestaError(int codigo)
{
    _statusCode = codigo;
    _statusMessage = HttpStatus::getMessage(codigo);
    
    std::map<int, std::string>::const_iterator it = _errorPages.find(codigo);
    if (it != _errorPages.end()) {
        // Si la ruta no empieza con '/', es relativa al documentRoot
        std::string rutaError;
        if (it->second[0] == '/') {
            // Ruta absoluta desde la raíz del proyecto
            rutaError = it->second.substr(1); // Quitar el '/' inicial
        } else {
            // Ruta relativa (puede estar en templates/ o html/)
            rutaError = it->second;
        }
        
        std::string contenido = leerArchivo(rutaError);
        
        if (!contenido.empty()) {
            _body = contenido;
            _headers["Content-Type"] = "text/html";
            return;
        }
    }
    
    std::ostringstream html;
    html << "<html><body><h1>" << codigo << " - " << _statusMessage << "</h1></body></html>";
    _body = html.str();
    _headers["Content-Type"] = "text/html";
}

// Normalizar path eliminando barras duplicadas
std::string Response::normalizarPath(const std::string& path)
{
    if (path.empty()) {
        return "/";
    }
    
    std::string resultado;
    resultado.reserve(path.length());
    
    bool ultimaFueBarra = false;
    for (size_t i = 0; i < path.length(); i++) {
        if (path[i] == '/') {
            if (!ultimaFueBarra) {
                resultado += '/';
                ultimaFueBarra = true;
            }
        } else {
            resultado += path[i];
            ultimaFueBarra = false;
        }
    }
    
    // Asegurar que empiece con /
    if (resultado.empty() || resultado[0] != '/') {
        resultado = "/" + resultado;
    }
    
    return resultado;
}
