#include "Response.hpp"

// Manejar peticiones GET
void Response::manejarGET(const Request& request)
{
    std::string path = normalizarPath(request.getPath());
    
    // Construir la ruta completa al archivo/directorio
    std::string rutaCompleta = _documentRoot + path;
    
    // Verificar si es un directorio
    if (esDirectorio(rutaCompleta)) {
        // Intentar servir index.html del directorio
        std::string indexPath = rutaCompleta;
        if (indexPath[indexPath.length() - 1] != '/') {
            indexPath += "/";
        }
        indexPath += "index.html";
        
        std::string contenido = leerArchivo(indexPath);
        
        if (!contenido.empty()) {
            _statusCode = HttpStatus::OK;
            _statusMessage = HttpStatus::getMessage(HttpStatus::OK);
            _headers["Content-Type"] = "text/html";
            _body = contenido;
            return;
        }
        
        // No hay index.html, verificar si tiene autoindex activado
        if (tieneAutoindex(path)) {
            _statusCode = HttpStatus::OK;
            _statusMessage = HttpStatus::getMessage(HttpStatus::OK);
            _headers["Content-Type"] = "text/html";
            _body = Autoindex::generarHTML(rutaCompleta, path);
            return;
        }
        
        // No hay index.html ni autoindex
        respuestaError(HttpStatus::FORBIDDEN);
        return;
    }
    
    // Es un archivo, intentar leerlo
    std::string contenido = leerArchivo(rutaCompleta);
    
    if (contenido.empty()) {
        respuestaError(HttpStatus::NOT_FOUND);
        return;
    }
    
    // Determinar Content-Type
    size_t puntoPos = path.find_last_of('.');
    std::string extension = "";
    if (puntoPos != std::string::npos) {
        extension = path.substr(puntoPos);
    }
    
    _statusCode = HttpStatus::OK;
    _statusMessage = HttpStatus::getMessage(HttpStatus::OK);
    _headers["Content-Type"] = obtenerContentType(extension);
    _body = contenido;
}
