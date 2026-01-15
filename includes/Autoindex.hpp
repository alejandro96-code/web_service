#ifndef AUTOINDEX_HPP
#define AUTOINDEX_HPP

#include "webserv.hpp"
#include <vector>

class Autoindex {
public:
    // Generar HTML con listado de archivos del directorio
    static std::string generateHTML(const std::string& directoryPath, const std::string& urlPath);
    
private:
    // Verificar si una ruta es un directorio
    static bool isDirectory(const std::string& path);
    
    // Obtener lista de archivos y directorios
    static std::vector<std::string> listDirectory(const std::string& path);
};

#endif
