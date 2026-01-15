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
    std::string line;
    
    while (std::getline(file, line)) {
        line = trim(line);
        
        if (line.empty() || line[0] == '#')
            continue;
        
        if (line == "}")
            return true;
        
        // Quitar punto y coma
        if (!line.empty() && line[line.length() - 1] == ';')
            line = line.substr(0, line.length() - 1);
        
        std::istringstream iss(line);
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
        else if (key == "return") {
            int code;
            std::string url;
            iss >> code >> url;
            if (code == 301 || code == 302) {
                location.has_redirect = true;
                location.redirect_code = code;
                location.redirect_url = url;
            }
        }
    }
    
    return false;
}

// Parsear bloque server
static bool parseServer(std::ifstream& file, ServerConfig& server) {
    std::string line;
    
    while (std::getline(file, line)) {
        line = trim(line);
        
        if (line.empty() || line[0] == '#')
            continue;
        
        if (line == "}")
            return true;
        
        // Detectar location
        if (line.find("location") == 0) {
            Location loc;
            // Extraer path: "location /" -> "/"
            size_t start = line.find_first_of(" \t");
            size_t end = line.find_first_of("{");
            if (start != std::string::npos) {
                std::string path_part = line.substr(start, end - start);
                loc.path = trim(path_part);
            }
            
            if (parseLocation(file, loc)) {
                server.locations.push_back(loc);
            }
            continue;
        }
        
        // Quitar punto y coma
        if (!line.empty() && line[line.length() - 1] == ';')
            line = line.substr(0, line.length() - 1);
        
        std::istringstream iss(line);
        std::string key;
        iss >> key;
        
        if (key == "listen") {
            std::string port_str;
            if (iss >> port_str) {
                // Validar que sea un número
                bool is_number = true;
                for (size_t i = 0; i < port_str.length(); i++) {
                    if (!isdigit(port_str[i])) {
                        is_number = false;
                        break;
                    }
                }
                
                if (!is_number) {
                    server.port = -1; // Marcar como inválido
                } else {
                    server.port = atoi(port_str.c_str());
                    // Validar rango de puerto
                    if (server.port == 0 || server.port > 65535) {
                        server.port = -1; // Marcar como inválido
                    }
                }
            } else {
                // Sin valor: puerto predefinido 8080
                server.port = 8080;
            }
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
        else if (key == "client_max_body_size") {
            std::string value;
            iss >> value;
            
            // Validar que no esté vacío
            if (value.empty()) {
                std::cerr << "Error en la configuración del client_max_body_size: valor vacío" << std::endl;
                return false;
            }
            
            // Extraer la unidad (último carácter)
            char unit = value[value.length() - 1];
            bool has_unit = (unit == 'M' || unit == 'm' || unit == 'G' || unit == 'g' || unit == 'K' || unit == 'k');
            
            // Extraer la parte numérica
            std::string num_part = has_unit ? value.substr(0, value.length() - 1) : value;
            
            // Validar que la parte numérica sea válida (solo dígitos)
            if (num_part.empty()) {
                std::cerr << "Error en la configuración del client_max_body_size: '" << value << "' no es válido" << std::endl;
                return false;
            }
            
            for (size_t i = 0; i < num_part.length(); i++) {
                if (!isdigit(num_part[i])) {
                    std::cerr << "Error en la configuración del client_max_body_size: '" << value << "' no es válido" << std::endl;
                    return false;
                }
            }
            
            // Convertir a número
            int num = atoi(num_part.c_str());
            
            // Validar que no sea 0
            if (num == 0) {
                std::cerr << "Error en la configuración del client_max_body_size: no puede ser 0" << std::endl;
                return false;
            }
            
            // Calcular tamaño en bytes según la unidad
            size_t size = 0;
            if (unit == 'M' || unit == 'm') {
                // Validar límite de 10M
                if (num > 10) {
                    std::cerr << "Error en la configuración del client_max_body_size: máximo permitido es 10M" << std::endl;
                    return false;
                }
                size = num * 1024 * 1024;
            } else if (unit == 'K' || unit == 'k') {
                size = num * 1024;
            } else if (unit == 'G' || unit == 'g') {
                std::cerr << "Error en la configuración del client_max_body_size: máximo permitido es 10M" << std::endl;
                return false;
            } else {
                // Sin unidad, son bytes directos
                size = num;
            }
            
            server.client_max_body_size = size;
        }
    }
    
    return false;
}

// Función principal para leer el archivo de configuración
std::vector<ServerConfig> readConfig(const char* file) {
    std::vector<ServerConfig> servers;
    std::ifstream infile(file);
    
    if (!infile.is_open()) {
        std::cerr << ERROR_NO_CARGAR << std::endl;
        return servers;
    }
    
    std::string line;
    bool inside_http = false;
    bool found_http = false;
    size_t http_client_max_body_size = 1048576; // Default 1MB
    
    while (std::getline(infile, line)) {
        line = trim(line);
        
        if (line.empty() || line[0] == '#')
            continue;
        
        // Detectar bloque http
        if (line.find("http") == 0) {
            inside_http = true;
            found_http = true;
            continue;
        }
        
        // Parsear directivas a nivel http
        if (inside_http && line.find("server") != 0 && line.find("}") != 0) {
            // Quitar punto y coma
            if (!line.empty() && line[line.length() - 1] == ';')
                line = line.substr(0, line.length() - 1);
            
            std::istringstream iss(line);
            std::string key;
            iss >> key;
            
            if (key == "client_max_body_size") {
                std::string value;
                iss >> value;
                
                // Validar que no esté vacío
                if (value.empty()) {
                    std::cerr << "Error en la configuración del client_max_body_size: valor vacío" << std::endl;
                    servers.clear();
                    return servers;
                }
                
                // Extraer la unidad (último carácter)
                char unit = value[value.length() - 1];
                bool has_unit = (unit == 'M' || unit == 'm' || unit == 'G' || unit == 'g' || unit == 'K' || unit == 'k');
                
                // Extraer la parte numérica
                std::string num_part = has_unit ? value.substr(0, value.length() - 1) : value;
                
                // Validar que la parte numérica sea válida (solo dígitos)
                if (num_part.empty()) {
                    std::cerr << "Error en la configuración del client_max_body_size: '" << value << "' no es válido" << std::endl;
                    servers.clear();
                    return servers;
                }
                
                for (size_t i = 0; i < num_part.length(); i++) {
                    if (!isdigit(num_part[i])) {
                        std::cerr << "Error en la configuración del client_max_body_size: '" << value << "' no es válido" << std::endl;
                        servers.clear();
                        return servers;
                    }
                }
                
                // Convertir a número
                int num = atoi(num_part.c_str());
                
                // Validar que no sea 0
                if (num == 0) {
                    std::cerr << "Error en la configuración del client_max_body_size: no puede ser 0" << std::endl;
                    servers.clear();
                    return servers;
                }
                
                // Calcular tamaño en bytes según la unidad
                size_t size = 0;
                if (unit == 'M' || unit == 'm') {
                    // Validar límite de 10M
                    if (num > 10) {
                        std::cerr << "Error en la configuración del client_max_body_size: máximo permitido es 10M" << std::endl;
                        servers.clear();
                        return servers;
                    }
                    size = num * 1024 * 1024;
                } else if (unit == 'K' || unit == 'k') {
                    size = num * 1024;
                } else if (unit == 'G' || unit == 'g') {
                    std::cerr << "Error en la configuración del client_max_body_size: máximo permitido es 10M" << std::endl;
                    servers.clear();
                    return servers;
                } else {
                    // Sin unidad, son bytes directos
                    size = num;
                }
                
                http_client_max_body_size = size;
            }
        }
        
        // Detectar bloque server dentro de http
        if (inside_http && line.find("server") == 0) {
            ServerConfig server;
            if (parseServer(infile, server)) {
                // Aplicar client_max_body_size del nivel http si no está configurado en server
                if (server.client_max_body_size == 1048576) {
                    server.client_max_body_size = http_client_max_body_size;
                }
                servers.push_back(server);
            }
        }
        
        // Fin del bloque http
        if (line == "}" && inside_http) {
            inside_http = false;
        }
    }
    
    infile.close();
    
    // Validar que existe bloque http
    if (!found_http) {
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
        bool has_error = false;
        
        // Validar listen (port)
        if (servers[i].port <= 0 || servers[i].port > 65535) {
            has_error = true;
        }
        
        // Si hay algún error en este servidor, mostrar mensaje y retornar vacío
        if (has_error) {
            std::cerr << ERROR_NO_CARGAR << std::endl;
            servers.clear();
            return servers;
        }
    }
    
    return servers;
}
