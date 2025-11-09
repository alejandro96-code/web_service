#include "Response.hpp"

/*
    Peticion Get:
    El metodo que se usa para solicitar un recurso del servidor.
    guardamos en uan variable path la respuesta del servidor
    completamos la ruta con nuestra carpetaRoot
    Ahora tenemos 2 casos:
        Caso Si en directorio:
            Añadimos "/", si ya acaba en / cargarmos index.html (signifca que estamos en localhost:8080/)
            Guardamos en contenido la lectura del archivo
            primera verificacion: (si existe index.html) si lo hay leemos el contenido
            segunda verificacion: (si autoindex es on) si  lo tiene lo cargamos
                si alguna de esta falla sacara un error 403 forbidden
        Caso Si es un archivo -> lo lee
        Si no entra en ningun caso devuelve un 404 not found (porque significa que no encuentra
        ni index.html ni autoindex)
        Despues determinamos el content type que hemos usado para la variable contenido
*/
void Response::manejarGET(const Request& request)
{
    std::string path = normalizarPath(request.getPath());
    std::string rutaCompleta = _documentRoot + path;
    
    // Caso especial: si se solicita /delete_file.html, servir desde templates
    if (path == "/delete_file.html") {
        std::string contenido = leerArchivo("templates/delete_file.html");
        if (!contenido.empty()) {
            _statusCode = HttpStatus::OK;
            _statusMessage = HttpStatus::getMessage(HttpStatus::OK);
            _headers["Content-Type"] = "text/html";
            _body = contenido;
            return;
        }
        respuestaError(HttpStatus::NOT_FOUND);
        return;
    }
    
    if (esDirectorio(rutaCompleta))
    {
        std::string indexPath = rutaCompleta;
        if (indexPath[indexPath.length() - 1] != '/')
            indexPath += "/";
        indexPath += "index.html";
        
        std::string contenido = leerArchivo(indexPath);
        if (!contenido.empty())
        {
            _statusCode = HttpStatus::OK;
            _statusMessage = HttpStatus::getMessage(HttpStatus::OK);
            _headers["Content-Type"] = "text/html";
            _body = contenido;
            return;
        }
        if (tieneAutoindex(path))
        {
            _statusCode = HttpStatus::OK;
            _statusMessage = HttpStatus::getMessage(HttpStatus::OK);
            _headers["Content-Type"] = "text/html";
            _body = Autoindex::generarHTML(rutaCompleta, path);
            return;
        }
        respuestaError(HttpStatus::FORBIDDEN);
        return;
    }
    std::string contenido = leerArchivo(rutaCompleta);
    
    if (contenido.empty()) {
        respuestaError(HttpStatus::NOT_FOUND);
        return;
    }
    
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
