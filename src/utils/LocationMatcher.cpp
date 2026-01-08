#include "LocationMatcher.hpp"

/*
    Obtener la location que coincide con el path:
    Busca entre todas las locations configuradas y retorna aquella
    cuyo path sea un prefijo del path solicitado.
    Si hay múltiples coincidencias, retorna la más específica (la más larga).
    
    Ejemplo:
    - locations: ["/", "/api", "/api/users"]
    - path: "/api/users/123"
    - retorna: location de "/api/users" (más específica)
*/
Location* LocationMatcher::obtenerLocation(const std::string& path, std::vector<Location>& locations)
{
    Location* mejor = NULL;
    size_t mayorLongitud = 0;
    
    for (size_t i = 0; i < locations.size(); i++) {
        std::string locPath = locations[i].path;
        if (path.find(locPath) == 0 && locPath.length() > mayorLongitud) {
            mayorLongitud = locPath.length();
            mejor = &locations[i];
        }
    }
    return mejor;
}

/*
    Verificar si el método está permitido para esta ruta:
    Busca la location correspondiente y verifica si el método HTTP
    está en la lista de allow_methods.
    Si no hay location o allow_methods está vacío, permite todos los métodos.
    
    Ejemplo:
    - location: { path: "/upload", allow_methods: ["GET", "POST"] }
    - metodoPermitido("/upload", "GET") → true
    - metodoPermitido("/upload", "DELETE") → false
*/
bool LocationMatcher::metodoPermitido(const std::string& path, const std::string& method, 
                                      std::vector<Location>& locations)
{
    Location* loc = obtenerLocation(path, locations);
    if (loc == NULL || loc->allow_methods.empty()) {
        return true;
    }
    
    for (size_t i = 0; i < loc->allow_methods.size(); i++) {
        if (loc->allow_methods[i] == method) {
            return true;
        }
    }
    return false;
}

/*
    Verificar si la ruta tiene autoindex activado:
    Busca la location correspondiente y verifica su configuración de autoindex.
    Si no hay location configurada, retorna false (autoindex desactivado por defecto).
    
    Autoindex permite mostrar el listado de archivos en un directorio
    cuando no existe index.html.
*/
bool LocationMatcher::tieneAutoindex(const std::string& path, std::vector<Location>& locations)
{
    Location* loc = obtenerLocation(path, locations);
    return (loc != NULL) ? loc->autoindex : false;
}
