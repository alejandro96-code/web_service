#include "Response.hpp"
#include <fstream>
#include <sstream>
#include <cstdio>

// Manejar peticiones DELETE
void Response::manejarDELETE(const Request& request)
{
    std::string path = request.getPath();
    
    // Solo permitir eliminar archivos en /autoindex/
    if (path.find("/autoindex/") == 0) {
        // Construir ruta completa al archivo
        std::string rutaCompleta = _documentRoot + path;
        
        // Verificar que el archivo existe
        std::ifstream checkFile(rutaCompleta.c_str());
        if (!checkFile.is_open()) {
            respuestaError(HttpStatus::NOT_FOUND);
            return;
        }
        checkFile.close();
        
        // Verificar que no es un directorio
        if (esDirectorio(rutaCompleta)) {
            respuestaError(HttpStatus::FORBIDDEN); // No se pueden eliminar directorios
            return;
        }
        
        // Eliminar el archivo
        if (remove(rutaCompleta.c_str()) != 0) {
            respuestaError(HttpStatus::INTERNAL_SERVER_ERROR);
            return;
        }
        
        // Leer el template HTML de éxito
        std::ifstream templateFile("templates/gooddelete.html");
        std::string htmlTemplate;
        
        if (!templateFile.is_open()) {
            respuestaError(HttpStatus::INTERNAL_SERVER_ERROR);
            return;
        }
        
        // Leer el contenido del template
        std::stringstream buffer;
        buffer << templateFile.rdbuf();
        htmlTemplate = buffer.str();
        templateFile.close();
        
        // Extraer el nombre del archivo de la ruta
        size_t lastSlash = path.find_last_of('/');
        std::string filename = (lastSlash != std::string::npos) ? path.substr(lastSlash + 1) : path;
        
        // Reemplazar placeholders
        size_t pos;
        
        // Reemplazar {{FILENAME}}
        while ((pos = htmlTemplate.find("{{FILENAME}}")) != std::string::npos) {
            htmlTemplate.replace(pos, 12, filename);
        }
        
        // Reemplazar {{PATH}}
        while ((pos = htmlTemplate.find("{{PATH}}")) != std::string::npos) {
            htmlTemplate.replace(pos, 8, path);
        }
        
        // Respuesta exitosa
        _statusCode = HttpStatus::OK;
        _statusMessage = HttpStatus::getMessage(HttpStatus::OK);
        _headers["Content-Type"] = "text/html";
        
        _body = htmlTemplate;
    }
    else {
        // Solo se pueden eliminar archivos en /autoindex/
        respuestaError(HttpStatus::FORBIDDEN);
    }
}
