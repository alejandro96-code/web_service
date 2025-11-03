#include "Response.hpp"
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <ctime>

// Manejar peticiones POST
void Response::manejarPOST(const Request& request)
{
    std::string path = request.getPath();
    
    if (path == "/upload") {
        // Obtener el body de la petición
        std::string body = request.getBody();
        
        if (body.empty()) {
            respuestaError(HttpStatus::BAD_REQUEST);
            return;
        }
        
        // Crear directorio uploads si no existe
        std::string uploadDir = _documentRoot + "/uploads";
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
        
        // Respuesta exitosa
        _statusCode = HttpStatus::CREATED;
        _statusMessage = HttpStatus::getMessage(HttpStatus::CREATED);
        _headers["Content-Type"] = "text/html";
        
        std::ostringstream response;
        response << "<!DOCTYPE html>\n"
                 << "<html><head><meta charset=\"UTF-8\"><title>Upload Success</title>\n"
                 << "<style>body{font-family:Arial;max-width:600px;margin:50px auto;padding:20px;text-align:center}"
                 << "h1{color:#4CAF50}.btn{background:#2196F3;color:white;padding:10px 20px;text-decoration:none;"
                 << "border-radius:5px;display:inline-block;margin-top:20px}</style></head>\n"
                 << "<body><h1>✅ Archivo subido exitosamente</h1>\n"
                 << "<p>Archivo guardado como: <strong>" << filename.str() << "</strong></p>\n"
                 << "<p>Ruta: /uploads/" << filename.str() << "</p>\n"
                 << "<a href=\"/\" class=\"btn\">Volver al inicio</a>\n"
                 << "<a href=\"/uploads/\" class=\"btn\">Ver archivos subidos</a>\n"
                 << "</body></html>";
        
        _body = response.str();
    }
    else {
        respuestaError(HttpStatus::NOT_FOUND);
    }
}
