#include "parseoConf.hpp"

// Función para leer el archivo de configuración
std::map<std::string, std::string> leerConfig(const char* archivo) {
    std::map<std::string, std::string> config;
    std::ifstream file(archivo);
    std::string linea;
    
    while (std::getline(file, linea)) {
        // Ignorar líneas vacías y comentarios
        if (linea.empty() || linea[0] == '#' || linea[0] == '[')
            continue;
        
        // Buscar el separador '='
        size_t pos = linea.find('=');
        if (pos == std::string::npos)
            continue;
        
        // Extraer clave y valor
        std::string clave = linea.substr(0, pos);
        std::string valor = linea.substr(pos + 1);
        
        // Limpiar espacios al inicio y final
        clave.erase(0, clave.find_first_not_of(" \t"));
        clave.erase(clave.find_last_not_of(" \t") + 1);
        valor.erase(0, valor.find_first_not_of(" \t"));
        
        // Eliminar comentarios del valor
        size_t comentario = valor.find('#');
        if (comentario != std::string::npos)
            valor = valor.substr(0, comentario);
        
        valor.erase(valor.find_last_not_of(" \t") + 1);
        
        config[clave] = valor;
    }
    
    return config;
}
