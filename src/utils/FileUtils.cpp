#include "FileUtils.hpp"
#include <fstream>
#include <sstream>
#include <sys/stat.h>

/*
    Leer archivo del sistema:
    Abre un archivo en modo lectura, lee todo su contenido
    usando un stringstream y lo devuelve como string.
    Si el archivo no existe o no se puede abrir, devuelve string vacío.
*/
std::string FileUtils::readFile(const std::string& path)
{
    std::ifstream file(path.c_str());
    if (!file.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    return buffer.str();
}

/*
    Verificar si una ruta es un directorio:
    Usa la llamada al sistema stat() para obtener información del archivo.
    Luego usa la macro S_ISDIR() para verificar si es un directorio.
    Retorna false si hay error o no es directorio.
*/
bool FileUtils::isDirectory(const std::string& path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false;
    }
    return S_ISDIR(info.st_mode);
}

/*
    Obtener Content-Type según extensión:
    Mapea extensiones de archivo a tipos MIME según el estándar HTTP.
    Soporta los tipos más comunes de archivos web.
    Si la extensión no se reconoce, devuelve "application/octet-stream" (binario genérico).
*/
std::string FileUtils::getContentType(const std::string& extension)
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

/*
    Normalizar path eliminando barras duplicadas:
    Recorre el path caracter por caracter y elimina barras duplicadas.
    Asegura que el path siempre empiece con '/'.
    Ejemplo: "//path///to//file" -> "/path/to/file"
*/
std::string FileUtils::normalizePath(const std::string& path)
{
    if (path.empty()) {
        return "/";
    }
    
    std::string result;
    result.reserve(path.length());
    
    bool lastWasSlash = false;
    for (size_t i = 0; i < path.length(); i++) {
        if (path[i] == '/') {
            if (!lastWasSlash) {
                result += '/';
                lastWasSlash = true;
            }
        } else {
            result += path[i];
            lastWasSlash = false;
        }
    }
    
    // Asegurar que empiece con /
    if (result.empty() || result[0] != '/') {
        result = "/" + result;
    }
    
    return result;
}
