#ifndef PARSEO_CONF_HPP
#define PARSEO_CONF_HPP

#include "webserv.hpp"
#include <vector>

// Estructura para location
struct Location {
    std::string path;
    std::vector<std::string> allow_methods;
    bool autoindex;
    std::string index;
    
    Location() : autoindex(false) {}
};

// Estructura para server
struct ServerConfig {
    int port;
    std::string server_name;
    std::string root;
    std::string index;
    std::map<int, std::string> error_pages;
    std::vector<Location> locations;
    
    ServerConfig() : port(-1) {}
};

// Función para leer el archivo de configuración estilo NGINX
std::vector<ServerConfig> leerConfig(const char* archivo);

#endif
