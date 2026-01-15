#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include "webserv.hpp"
#include "Request.hpp"

/*
    CGIHandler: Clase para manejar la ejecución de scripts CGI
    
    CGI (Common Gateway Interface):
    Permite ejecutar scripts externos (.php, .py, etc.) y devolver su salida como respuesta HTTP.
    
    Responsabilidades:
    - Detectar si un archivo es un script CGI basándose en su extensión
    - Obtener la ruta del intérprete apropiado (php-cgi, python3, etc.)
    - Ejecutar el script usando fork + pipe + execve
    - Capturar y devolver la salida del script
*/
class CGIHandler {
public:
    // Verificar si un archivo es CGI basándose en su extensión
    static bool isCGI(const std::string& path);
    
    // Obtener la ruta del intérprete según la extensión
    static std::string getInterpreterPath(const std::string& extension);
    
    // Ejecutar script CGI usando fork + pipe + execve
    static std::string executeCGI(const std::string& scriptPath, const Request& request);
};

#endif
