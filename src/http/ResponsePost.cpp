#include "Response.hpp"
#include <fstream>
#include <sstream>
#include <ctime>

// 
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
void Response::manejarPOST(const Request& request)
{
    std::string path = request.getPath();
    
    if (path == "/autoindex")
    {
        std::string body = request.getBody();
        
        // Verificar si el body está vacío o si es un multipart sin archivo
        // Cuando el formulario se envía sin seleccionar archivo, el body contiene
        // solo los boundaries del multipart pero sin contenido real de archivo
        if (body.empty())
        {
            respuestaError(HttpStatus::BAD_REQUEST);
            return;
        }
        
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
                    respuestaError(HttpStatus::BAD_REQUEST);
                    return;
                }
            }
        }
        else
        {
            // Si no hay filename en el multipart, es un POST inválido
            respuestaError(HttpStatus::BAD_REQUEST);
            return;
        }
        
        std::time_t now = std::time(NULL);
        std::ostringstream filenameStream;
        filenameStream << "archivo_" << now << ".txt";
        
        std::string rutaArchivo = _documentRoot + "/autoindex/archivosSubidos/" + filenameStream.str();
        std::ofstream file(rutaArchivo.c_str(), std::ios::binary);
        if (!file.is_open()) {
            respuestaError(HttpStatus::INTERNAL_SERVER_ERROR);
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
        respuestaError(HttpStatus::NOT_FOUND);
    }
}
