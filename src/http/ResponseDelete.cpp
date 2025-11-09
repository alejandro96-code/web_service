#include "Response.hpp"
#include <fstream>
#include <sstream>
#include <cstdio>

/*
    Peticion Delete:
    Se utiliza para eliminar un recurso especifico del servidor
    Recibe la ruta del archivo a eliminar
    Solamente permitimos borrar archivos dentro del autoindex si no devolvemos un 403
    Verificamos de que el archivo exite (si se puede abrir), si no devolvemos un 404
    Verificamos de que no es un directorio, si lo es devolvemos un 403
    En el caso de que se pueda borrar devolvera 0 y borramos si no devolvera -1
    (quitariamos permisos a archivosSubidos)
    Si todo funcion a bien, leeremos el template y devolveremos un 200
*/
void Response::manejarDELETE(const Request& request)
{
    std::string path = request.getPath();
    
    if (path.find("/autoindex/") == 0)
    {
        std::string rutaCompleta = _documentRoot + path;

        std::ifstream checkFile(rutaCompleta.c_str());
        if (!checkFile.is_open())
        {
            respuestaError(HttpStatus::NOT_FOUND);
            return;
        }
        checkFile.close();
        
        if (esDirectorio(rutaCompleta)) {
            respuestaError(HttpStatus::FORBIDDEN);
            return;
        }
        
        if (remove(rutaCompleta.c_str()) != 0) {
            respuestaError(HttpStatus::INTERNAL_SERVER_ERROR);
            return;
        }
        
        std::ifstream templateFile("templates/gooddelete.html");
        std::string htmlTemplate;
        std::stringstream buffer;
        buffer << templateFile.rdbuf();
        htmlTemplate = buffer.str();
        templateFile.close();
        _statusCode = HttpStatus::OK;
        _statusMessage = HttpStatus::getMessage(HttpStatus::OK);
        _headers["Content-Type"] = "text/html";
        _body = htmlTemplate;
    }
    else
        respuestaError(HttpStatus::FORBIDDEN);
}
