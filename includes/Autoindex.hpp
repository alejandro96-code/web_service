#ifndef AUTOINDEX_HPP
#define AUTOINDEX_HPP

#include "webserv.hpp"
#include <vector>

class Autoindex {
public:
    // Generar HTML con listado de archivos del directorio
    static std::string generarHTML(const std::string& rutaDirectorio, const std::string& rutaURL);
    
private:
    // Verificar si una ruta es un directorio
    static bool esDirectorio(const std::string& ruta);
    
    // Obtener lista de archivos y directorios
    static std::vector<std::string> listarDirectorio(const std::string& ruta);
};

#endif
