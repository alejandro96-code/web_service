#ifndef PARSEO_CONF_HPP
#define PARSEO_CONF_HPP

#include "webserv.hpp"

// Función para leer el archivo de configuración
std::map<std::string, std::string> leerConfig(const char* archivo);

#endif
