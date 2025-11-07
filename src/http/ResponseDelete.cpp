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
        
        // Respuesta exitosa
        _statusCode = HttpStatus::OK;
        _statusMessage = HttpStatus::getMessage(HttpStatus::OK);
        _headers["Content-Type"] = "text/html";
        
        std::ostringstream response;
        response << "<!DOCTYPE html>\n"
                 << "<html><head><meta charset=\"UTF-8\"><title>Delete Success</title>\n"
                 << "<style>body{font-family:Arial;max-width:600px;margin:50px auto;padding:20px;text-align:center}"
                 << "h1{color:#4CAF50}.btn{background:#2196F3;color:white;padding:10px 20px;text-decoration:none;"
                 << "border-radius:5px;display:inline-block;margin-top:20px}</style></head>\n"
                 << "<body><h1>🗑️ Archivo eliminado exitosamente</h1>\n"
                 << "<p>Archivo: <strong>" << path << "</strong></p>\n"
                 << "<a href=\"/\" class=\"btn\">Volver al inicio</a>\n"
                 << "<a href=\"/autoindex/\" class=\"btn\">Ver archivos restantes</a>\n"
                 << "</body></html>";
        
        _body = response.str();
    }
    else {
        // Solo se pueden eliminar archivos en /autoindex/
        respuestaError(HttpStatus::FORBIDDEN);
    }
}
