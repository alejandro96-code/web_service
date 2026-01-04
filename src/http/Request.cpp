#include "Request.hpp"

Request::Request() : _method(""), _path(""), _version(""), _body(""), _isChunked(false) {} // Constructor por defecto
Request::Request(const std::string& rawRequest) : _isChunked(false) {parse(rawRequest);} // Constructor con petición

/*
    REQUEST: Convierte texto HTTP en datos estructurados que el servidor puede usar
    Ej: tenemos esta entrada
        GET /index.html HTTP/1.1
        Host: localhost:8080
        User-Agent: Mozilla/5.0
    y buscamos tener esta salida
        _method  = "GET"
        _path    = "/index.html"
        _version = "HTTP/1.1"
        _headers = {"Host": "localhost:8080", "User-Agent": "Mozilla/5.0"}
        _body    = ""
    Para ello realizamos un parseo:
        primero creamos un stream para leer linea por linea el archivo
        Parsear la primera línea: "GET /index.html HTTP/1.1\r"
        Limpiar el \r si existe (Windows line endings) "GET /index.html HTTP/1.1"
        leemos linea por linea hasta encontrar una linea vacia
        buscar ":" y dividimos en clave valor 
        despues quitamos los espacios
            "Host: localhost:8080\r\n"
            "Content-Type: text/html\r\n"
            "\r\n"  ← Línea vacía (fin de headers)

            _headers["Host"] = "localhost:8080"
            _headers["Content-Type"] = "text/html"
        El resto lo guardamos en el body
*/
void Request::parse(const std::string& rawRequest)
{
    _rawRequest = rawRequest;
    std::istringstream stream(rawRequest);
    std::string line;

    if (std::getline(stream, line)) {
        std::istringstream lineStream(line);
        lineStream >> _method >> _path >> _version;
        if (!_version.empty() && _version[_version.length() - 1] == '\r') {
            _version = _version.substr(0, _version.length() - 1);
        }
    }

    while (std::getline(stream, line) && line != "\r" && !line.empty()) {
        if (!line.empty() && line[line.length() - 1] == '\r') {
            line = line.substr(0, line.length() - 1);
        }
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            if (!value.empty() && value[0] == ' ') {
                value = value.substr(1);
            }
            _headers[key] = value;
            
            // Detectar Transfer-Encoding: chunked
            if (key == "Transfer-Encoding" && value.find("chunked") != std::string::npos) {
                _isChunked = true;
            }
        }
    }
    
    // Leer el body (puede estar en formato chunked)
    std::string bodyLine;
    std::string rawBody;
    while (std::getline(stream, bodyLine)) {
        rawBody += bodyLine + "\n";
    }
    
    // Si es chunked, decodificar
    if (_isChunked) {
        _body = decodeChunkedBody(rawBody);
    } else {
        _body = rawBody;
    }
    
    /*
        Como HTML no soporta DELETE en el delete_file.html hemos metido
        "<input type="hidden" name="_method" value="DELETE">"
        para que cuando encuentre _method=DELETE en el body trasforme el metodo POST en DELETE
        Esto se ejecuta solo si el path de la peticion es /delete_file
        buscaremos el donde empieza el filename en el body, saltamos 9 caracteres
        si encontramos un "&" si no hay buscaremos un salto de linea \n
        y si tampoco hay un salto de linea usamos el final de body
        y asi contruimos el path entero.
        EJ: _method=DELETE&filename=upload_1762709182.txt\n
    */
    if (_method == "POST" && _body.find("_method=DELETE") != std::string::npos)
    {
        _method = "DELETE";
        if (_path == "/delete_file")
        {
            size_t pos = _body.find("filename=");
            if (pos != std::string::npos) {
                pos += 9;
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

bool Request::isChunked() const {return _isChunked;}

std::string Request::decodeChunkedBody(const std::string& chunkedData)
{
    std::string decoded;
    size_t pos = 0;
    
    while (pos < chunkedData.length()) {
        // Buscar el fin de la línea del tamaño
        size_t sizeEnd = chunkedData.find("\r\n", pos);
        if (sizeEnd == std::string::npos) {
            break; // No hay más chunks válidos
        }
        
        // Extraer el tamaño del chunk (en hexadecimal)
        std::string sizeStr = chunkedData.substr(pos, sizeEnd - pos);
        
        // Convertir de hexadecimal a decimal
        size_t chunkSize = 0;
        std::istringstream(sizeStr) >> std::hex >> chunkSize;
        
        // Si el tamaño es 0, hemos llegado al último chunk
        if (chunkSize == 0) {
            break;
        }
        
        // Posición del inicio de los datos
        size_t dataStart = sizeEnd + 2; // Saltar \r\n
        
        // Verificar que no nos pasemos del límite
        if (dataStart + chunkSize > chunkedData.length()) {
            break; // Datos incompletos
        }
        
        // Extraer los datos del chunk
        decoded += chunkedData.substr(dataStart, chunkSize);
        
        // Moverse al siguiente chunk (saltar datos + \r\n)
        pos = dataStart + chunkSize + 2;
    }
    
    return decoded;
}