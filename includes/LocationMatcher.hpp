#ifndef LOCATION_MATCHER_HPP
#define LOCATION_MATCHER_HPP

#include <string>
#include <vector>
#include "parseoConf.hpp"

/*
    LocationMatcher: Gestión de matching y validación de rutas con configuración
    
    Proporciona funcionalidades para:
    - Encontrar la location que coincide con un path
    - Validar si un método HTTP está permitido en una ruta
    - Verificar si autoindex está activado para una ruta
*/
class LocationMatcher {
public:
    // Obtener la location que mejor coincide con el path dado
    static Location* getLocation(const std::string& path, std::vector<Location>& locations);
    
    // Verificar si un método HTTP está permitido para la ruta
    static bool isMethodAllowed(const std::string& path, const std::string& method, 
                                std::vector<Location>& locations);
    
    // Verificar si autoindex está activado para la ruta
    static bool hasAutoindex(const std::string& path, std::vector<Location>& locations);
};

#endif
