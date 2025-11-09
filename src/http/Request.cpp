#include "Request.hpp"

Request::Request() : _method(""), _path(""), _version(""), _body("") {} // Constructor por defecto
Request::Request(const std::string& rawRequest) {parse(rawRequest);} // Constructor con petición

// Parsear la petición HTTP
void Request::parse(const std::string& rawRequest)
{
    _rawRequest = rawRequest;
    
    // Dividir la petición en líneas
    std::istringstream stream(rawRequest);
    std::string line;
    
    // 1. Parsear la primera línea: "GET /index.html HTTP/1.1"
    if (std::getline(stream, line)) {
        std::istringstream lineStream(line);
        lineStream >> _method >> _path >> _version;
        
        // Limpiar el \r si existe (Windows line endings)
        if (!_version.empty() && _version[_version.length() - 1] == '\r') {
            _version = _version.substr(0, _version.length() - 1);
        }
    }
    
    // 2. Parsear los headers (líneas que contienen ":")
    while (std::getline(stream, line) && line != "\r" && !line.empty()) {
        // Limpiar \r al final
        if (!line.empty() && line[line.length() - 1] == '\r') {
            line = line.substr(0, line.length() - 1);
        }
        
        // Buscar el separador ":"
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            // Limpiar espacios al inicio del valor
            if (!value.empty() && value[0] == ' ') {
                value = value.substr(1);
            }
            
            _headers[key] = value;
        }
    }
    
    // 3. El resto es el body (si existe)
    std::string bodyLine;
    while (std::getline(stream, bodyLine)) {
        _body += bodyLine + "\n";
    }
    
    // 4. Detectar _method=DELETE en el body (method override)
    if (_method == "POST" && _body.find("_method=DELETE") != std::string::npos)
    {
        _method = "DELETE";
        
        // Si el POST viene de /delete_file, extraer filename y modificar el path
        if (_path == "/delete_file") {
            size_t pos = _body.find("filename=");
            if (pos != std::string::npos) {
                pos += 9; // Longitud de "filename="
                size_t end = _body.find("&", pos);
                if (end == std::string::npos) {
                    end = _body.find("\n", pos);
                }
                if (end == std::string::npos) {
                    end = _body.length();
                }
                std::string filename = _body.substr(pos, end - pos);
                _path = "/autoindex/archivosSubidos/" + filename;
            }
        }
    }
}

// Getters
std::string Request::getMethod() const {return _method;}
std::string Request::getPath() const {return _path;}
std::string Request::getVersion() const {return _version;}

std::string Request::getHeader(const std::string& key) const {
    std::map<std::string, std::string>::const_iterator it = _headers.find(key);
    if (it != _headers.end()) {
        return it->second;
    }
    return "";
}

std::string Request::getBody() const {return _body;}