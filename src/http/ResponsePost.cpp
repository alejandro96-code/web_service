#include "Response.hpp"
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <ctime>

// 
/*
    Peticion POST:
    Se usa principalmente para enviar datos a un servidor para crear o actualizar un recurso.

*/
void Response::manejarPOST(const Request& request)
{
    std::string path = request.getPath();
    
    if (path == "/autoindex") {
        // Obtener el body de la petición
        std::string body = request.getBody();
        
        if (body.empty()) {
            respuestaError(HttpStatus::BAD_REQUEST);
            return;
        }
        
        // Verificar si se seleccionó un archivo (filename no está vacío)
        std::string filename_marker = "filename=\"";
        size_t filename_pos = body.find(filename_marker);
        
        if (filename_pos != std::string::npos) {
            size_t start = filename_pos + filename_marker.length();
            size_t end = body.find("\"", start);
            
            // Si el filename está vacío (filename=""), devolver 400
            if (end != std::string::npos && start == end) {
                respuestaError(HttpStatus::BAD_REQUEST);
                return;
            }
        }
        
        // Crear directorio autoindex/archivosSubidos si no existe
        std::string autoindexDir = _documentRoot + "/autoindex";
        std::string uploadDir = autoindexDir + "/archivosSubidos";
        mkdir(autoindexDir.c_str(), 0755);
        mkdir(uploadDir.c_str(), 0755);
        
        // Generar nombre de archivo único con timestamp
        std::time_t now = std::time(NULL);
        std::ostringstream filename;
        filename << "upload_" << now << ".txt";
        
        std::string rutaArchivo = uploadDir + "/" + filename.str();
        
        // Guardar el archivo
        std::ofstream file(rutaArchivo.c_str(), std::ios::binary);
        if (!file.is_open()) {
            respuestaError(HttpStatus::INTERNAL_SERVER_ERROR);
            return;
        }
        
        file.write(body.c_str(), body.length());
        file.close();
        
        // Leer el template HTML de éxito
        std::ifstream templateFile("templates/goodpost.html");
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
        
        // Reemplazar placeholders
        size_t pos;
        
        // Reemplazar {{FILENAME}}
        while ((pos = htmlTemplate.find("{{FILENAME}}")) != std::string::npos) {
            htmlTemplate.replace(pos, 12, filename.str());
        }
        
        // Reemplazar {{PATH}}
        std::string filePath = "/autoindex/archivosSubidos/" + filename.str();
        while ((pos = htmlTemplate.find("{{PATH}}")) != std::string::npos) {
            htmlTemplate.replace(pos, 8, filePath);
        }
        
        // Respuesta exitosa
        _statusCode = HttpStatus::CREATED;
        _statusMessage = HttpStatus::getMessage(HttpStatus::CREATED);
        _headers["Content-Type"] = "text/html";
        
        _body = htmlTemplate;
    }
    else {
        respuestaError(HttpStatus::NOT_FOUND);
    }
}
