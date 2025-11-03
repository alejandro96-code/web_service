#include "parseoConf.hpp"

// Función auxiliar para limpiar espacios
static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// Parsear bloque location
static bool parseLocation(std::ifstream& file, Location& location) {
    std::string linea;
    
    while (std::getline(file, linea)) {
        linea = trim(linea);
        
        if (linea.empty() || linea[0] == '#')
            continue;
        
        if (linea == "}")
            return true;
        
        // Quitar punto y coma
        if (!linea.empty() && linea[linea.length() - 1] == ';')
            linea = linea.substr(0, linea.length() - 1);
        
        std::istringstream iss(linea);
        std::string key;
        iss >> key;
        
        if (key == "allow_methods") {
            std::string method;
            while (iss >> method) {
                location.allow_methods.push_back(method);
            }
        }
        else if (key == "autoindex") {
            std::string value;
            iss >> value;
            location.autoindex = (value == "on");
        }
        else if (key == "index") {
            iss >> location.index;
        }
    }
    
    return false;
}

// Parsear bloque server
static bool parseServer(std::ifstream& file, ServerConfig& server) {
    std::string linea;
    
    while (std::getline(file, linea)) {
        linea = trim(linea);
        
        if (linea.empty() || linea[0] == '#')
            continue;
        
        if (linea == "}")
            return true;
        
        // Detectar location
        if (linea.find("location") == 0) {
            Location loc;
            // Extraer path: "location /" -> "/"
            size_t start = linea.find_first_of(" \t");
            size_t end = linea.find_first_of("{");
            if (start != std::string::npos) {
                std::string path_part = linea.substr(start, end - start);
                loc.path = trim(path_part);
            }
            
            if (parseLocation(file, loc)) {
                server.locations.push_back(loc);
            }
            continue;
        }
        
        // Quitar punto y coma
        if (!linea.empty() && linea[linea.length() - 1] == ';')
            linea = linea.substr(0, linea.length() - 1);
        
        std::istringstream iss(linea);
        std::string key;
        iss >> key;
        
        if (key == "listen") {
            iss >> server.port;
        }
        else if (key == "server_name") {
            iss >> server.server_name;
        }
        else if (key == "root") {
            iss >> server.root;
        }
        else if (key == "index") {
            iss >> server.index;
        }
        else if (key == "error_page") {
            int code;
            std::string page;
            iss >> code >> page;
            server.error_pages[code] = page;
        }
    }
    
    return false;
}

// Función principal para leer el archivo de configuración
std::vector<ServerConfig> leerConfig(const char* archivo) {
    std::vector<ServerConfig> servers;
    std::ifstream file(archivo);
    
    if (!file.is_open()) {
        std::cerr << ERROR_NO_CARGAR << std::endl;
        return servers;
    }
    
    std::string linea;
    bool dentro_http = false;
    bool encontrado_http = false;
    
    while (std::getline(file, linea)) {
        linea = trim(linea);
        
        if (linea.empty() || linea[0] == '#')
            continue;
        
        // Detectar bloque http
        if (linea.find("http") == 0) {
            dentro_http = true;
            encontrado_http = true;
            continue;
        }
        
        // Detectar bloque server dentro de http
        if (dentro_http && linea.find("server") == 0) {
            ServerConfig server;
            if (parseServer(file, server)) {
                servers.push_back(server);
            }
        }
        
        // Fin del bloque http
        if (linea == "}" && dentro_http) {
            dentro_http = false;
        }
    }
    
    file.close();
    
    // Validar que existe bloque http
    if (!encontrado_http) {
        std::cerr << ERROR_NO_CARGAR << std::endl;
        servers.clear();
        return servers;
    }
    
    // Validar que hay al menos un server
    if (servers.empty()) {
        std::cerr << ERROR_NO_CARGAR << std::endl;
        return servers;
    }
    
    // Validar cada servidor
    for (size_t i = 0; i < servers.size(); i++) {
        bool tiene_error = false;
        
        // Validar listen (port)
        if (servers[i].port <= 0 || servers[i].port > 65535) {
            tiene_error = true;
        }
        
        // Validar server_name
        if (servers[i].server_name.empty()) {
            tiene_error = true;
        }
        
        // Validar root
        if (servers[i].root.empty()) {
            tiene_error = true;
        }
        
        // Validar index
        if (servers[i].index.empty()) {
            tiene_error = true;
        }
        
        // Si hay algún error en este servidor, mostrar mensaje y retornar vacío
        if (tiene_error) {
            std::cerr << ERROR_NO_CARGAR << std::endl;
            servers.clear();
            return servers;
        }
    }
    
    return servers;
}
