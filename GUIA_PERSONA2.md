# 🚀 Guía de Inicio - Persona 2 (HTTP + Configuración)

## 📋 Tu Rol
Eres responsable de:
- Entender y parsear HTTP
- Leer y parsear la configuración
- Construir respuestas HTTP correctas
- Implementar la lógica de los métodos GET, POST, DELETE

---

## 🎯 SEMANA 1: Fundamentos

### DÍA 1-2: Estudiar HTTP

#### 1. Lee esto primero (HTTP básico)
Lee "HTTP Made Really Easy": https://www.jmarshall.com/easy/http/

#### 2. Entiende la estructura de HTTP

**Request HTTP básico:**
```http
GET /index.html HTTP/1.1
Host: localhost:8080
User-Agent: Mozilla/5.0
Accept: text/html
Connection: keep-alive

```

**Response HTTP básico:**
```http
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 13
Connection: close

Hello, World!
```

#### 3. Prueba HTTP manualmente con telnet
```bash
# Instalar telnet si no lo tienes
sudo apt install telnet

# Conectar a un servidor (por ejemplo google)
telnet google.com 80

# Luego escribe (presiona Enter 2 veces al final):
GET / HTTP/1.1
Host: google.com

```

#### 4. Usa curl para ver requests/responses
```bash
# Ver headers de respuesta
curl -I https://www.google.com

# Ver request y response completos
curl -v http://www.google.com

# Hacer POST
curl -X POST http://httpbin.org/post -d "key=value"

# Subir archivo
curl -X POST http://httpbin.org/post -F "file=@test.txt"
```

---

### DÍA 3-4: Tu Primer Código - HttpRequest Parser

#### Crea la estructura del proyecto:
```bash
cd /home/alejanr2/proyectos/web_service
mkdir -p includes srcs configs tests
```

#### Archivo: `includes/HttpRequest.hpp`
```cpp
#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>
#include <iostream>

class HttpRequest {
private:
    std::string _method;        // GET, POST, DELETE
    std::string _path;          // /index.html
    std::string _version;       // HTTP/1.1
    std::map<std::string, std::string> _headers;
    std::string _body;
    bool _complete;             // ¿Request completo?

public:
    HttpRequest();
    ~HttpRequest();
    
    // Parsear el request desde un string
    void parse(const std::string& rawRequest);
    
    // Getters
    std::string getMethod() const;
    std::string getPath() const;
    std::string getHeader(const std::string& key) const;
    std::string getBody() const;
    bool isComplete() const;
    
    // Debug
    void print() const;
};

#endif
```

#### Archivo: `srcs/HttpRequest.cpp`
```cpp
#include "../includes/HttpRequest.hpp"
#include <sstream>

HttpRequest::HttpRequest() : _complete(false) {}

HttpRequest::~HttpRequest() {}

void HttpRequest::parse(const std::string& rawRequest) {
    if (rawRequest.empty()) {
        return;
    }
    
    std::istringstream stream(rawRequest);
    std::string line;
    
    // 1. Parsear primera línea: GET /path HTTP/1.1
    if (std::getline(stream, line)) {
        // Remover \r si existe
        if (!line.empty() && line[line.length()-1] == '\r') {
            line.erase(line.length()-1);
        }
        
        std::istringstream lineStream(line);
        lineStream >> _method >> _path >> _version;
    }
    
    // 2. Parsear headers
    while (std::getline(stream, line) && line != "\r") {
        if (!line.empty() && line[line.length()-1] == '\r') {
            line.erase(line.length()-1);
        }
        
        if (line.empty()) {
            break; // Fin de headers
        }
        
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            
            // Eliminar espacios al inicio del value
            size_t start = value.find_first_not_of(" \t");
            if (start != std::string::npos) {
                value = value.substr(start);
            }
            
            _headers[key] = value;
        }
    }
    
    // 3. Parsear body (todo lo que queda)
    std::string bodyPart;
    while (std::getline(stream, bodyPart)) {
        _body += bodyPart + "\n";
    }
    
    _complete = true;
}

std::string HttpRequest::getMethod() const {
    return _method;
}

std::string HttpRequest::getPath() const {
    return _path;
}

std::string HttpRequest::getHeader(const std::string& key) const {
    std::map<std::string, std::string>::const_iterator it = _headers.find(key);
    if (it != _headers.end()) {
        return it->second;
    }
    return "";
}

std::string HttpRequest::getBody() const {
    return _body;
}

bool HttpRequest::isComplete() const {
    return _complete;
}

void HttpRequest::print() const {
    std::cout << "=== HTTP Request ===" << std::endl;
    std::cout << "Method: " << _method << std::endl;
    std::cout << "Path: " << _path << std::endl;
    std::cout << "Version: " << _version << std::endl;
    std::cout << "Headers:" << std::endl;
    
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
         it != _headers.end(); ++it) {
        std::cout << "  " << it->first << ": " << it->second << std::endl;
    }
    
    if (!_body.empty()) {
        std::cout << "Body: " << _body << std::endl;
    }
    std::cout << "===================" << std::endl;
}
```

#### Archivo de prueba: `tests/test_request.cpp`
```cpp
#include "../includes/HttpRequest.hpp"
#include <iostream>

int main() {
    // Test 1: GET request simple
    std::string rawRequest1 = 
        "GET /index.html HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "User-Agent: Mozilla/5.0\r\n"
        "\r\n";
    
    HttpRequest req1;
    req1.parse(rawRequest1);
    req1.print();
    
    std::cout << "\n\n";
    
    // Test 2: POST request con body
    std::string rawRequest2 = 
        "POST /upload HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "key=value&id=123";
    
    HttpRequest req2;
    req2.parse(rawRequest2);
    req2.print();
    
    return 0;
}
```

#### Compilar y probar:
```bash
# Desde el directorio web_service
g++ -Wall -Wextra -Werror -std=c++98 srcs/HttpRequest.cpp tests/test_request.cpp -o test_request

# Ejecutar
./test_request
```

**✅ Objetivo del Día 3-4:** Tener un parser de HTTP Request funcional que puedas probar

---

### DÍA 5-6: HttpResponse Builder

#### Archivo: `includes/HttpResponse.hpp`
```cpp
#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <map>
#include <sstream>

class HttpResponse {
private:
    int _statusCode;
    std::string _statusMessage;
    std::map<std::string, std::string> _headers;
    std::string _body;

    std::string getStatusMessage(int code);

public:
    HttpResponse();
    ~HttpResponse();
    
    void setStatus(int code);
    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& body);
    
    std::string build();  // Genera el string HTTP completo
    void print() const;
};

#endif
```

#### Archivo: `srcs/HttpResponse.cpp`
```cpp
#include "../includes/HttpResponse.hpp"

HttpResponse::HttpResponse() : _statusCode(200), _statusMessage("OK") {}

HttpResponse::~HttpResponse() {}

std::string HttpResponse::getStatusMessage(int code) {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 400: return "Bad Request";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 413: return "Payload Too Large";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        default: return "Unknown";
    }
}

void HttpResponse::setStatus(int code) {
    _statusCode = code;
    _statusMessage = getStatusMessage(code);
}

void HttpResponse::setHeader(const std::string& key, const std::string& value) {
    _headers[key] = value;
}

void HttpResponse::setBody(const std::string& body) {
    _body = body;
    
    // Auto-calcular Content-Length
    std::ostringstream ss;
    ss << body.length();
    setHeader("Content-Length", ss.str());
}

std::string HttpResponse::build() {
    std::ostringstream response;
    
    // Status line
    response << "HTTP/1.1 " << _statusCode << " " << _statusMessage << "\r\n";
    
    // Headers
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
         it != _headers.end(); ++it) {
        response << it->first << ": " << it->second << "\r\n";
    }
    
    // Línea vacía entre headers y body
    response << "\r\n";
    
    // Body
    response << _body;
    
    return response.str();
}

void HttpResponse::print() const {
    std::cout << "=== HTTP Response ===" << std::endl;
    std::cout << "Status: " << _statusCode << " " << _statusMessage << std::endl;
    std::cout << "Headers:" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
         it != _headers.end(); ++it) {
        std::cout << "  " << it->first << ": " << it->second << std::endl;
    }
    std::cout << "Body: " << _body << std::endl;
    std::cout << "=====================" << std::endl;
}
```

#### Test: `tests/test_response.cpp`
```cpp
#include "../includes/HttpResponse.hpp"
#include <iostream>

int main() {
    // Test 1: 200 OK con HTML
    HttpResponse resp1;
    resp1.setStatus(200);
    resp1.setHeader("Content-Type", "text/html");
    resp1.setBody("<html><body><h1>Hello World!</h1></body></html>");
    
    std::cout << "Response 1:\n" << resp1.build() << std::endl;
    
    // Test 2: 404 Not Found
    HttpResponse resp2;
    resp2.setStatus(404);
    resp2.setHeader("Content-Type", "text/html");
    resp2.setBody("<html><body><h1>404 Not Found</h1></body></html>");
    
    std::cout << "\nResponse 2:\n" << resp2.build() << std::endl;
    
    return 0;
}
```

```bash
g++ -Wall -Wextra -Werror -std=c++98 srcs/HttpResponse.cpp tests/test_response.cpp -o test_response
./test_response
```

**✅ Objetivo del Día 5-6:** Tener un generador de HTTP Response que crea respuestas válidas

---

### DÍA 7: Servir archivos estáticos (GET básico)

#### Archivo: `includes/FileHandler.hpp`
```cpp
#ifndef FILEHANDLER_HPP
#define FILEHANDLER_HPP

#include <string>
#include "HttpResponse.hpp"

class FileHandler {
public:
    static bool fileExists(const std::string& path);
    static std::string readFile(const std::string& path);
    static std::string getContentType(const std::string& path);
    static void handleGet(const std::string& path, HttpResponse& response);
};

#endif
```

#### Archivo: `srcs/FileHandler.cpp`
```cpp
#include "../includes/FileHandler.hpp"
#include <fstream>
#include <sstream>
#include <sys/stat.h>

bool FileHandler::fileExists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

std::string FileHandler::readFile(const std::string& path) {
    std::ifstream file(path.c_str());
    if (!file.is_open()) {
        return "";
    }
    
    std::ostringstream ss;
    ss << file.rdbuf();
    file.close();
    
    return ss.str();
}

std::string FileHandler::getContentType(const std::string& path) {
    // Buscar extensión
    size_t dotPos = path.find_last_of('.');
    if (dotPos == std::string::npos) {
        return "text/plain";
    }
    
    std::string ext = path.substr(dotPos + 1);
    
    if (ext == "html" || ext == "htm") return "text/html";
    if (ext == "css") return "text/css";
    if (ext == "js") return "application/javascript";
    if (ext == "json") return "application/json";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "png") return "image/png";
    if (ext == "gif") return "image/gif";
    if (ext == "txt") return "text/plain";
    
    return "application/octet-stream";
}

void FileHandler::handleGet(const std::string& path, HttpResponse& response) {
    // Construir path completo (por ahora asumimos root = ./www)
    std::string fullPath = "./www" + path;
    
    if (!fileExists(fullPath)) {
        response.setStatus(404);
        response.setHeader("Content-Type", "text/html");
        response.setBody("<html><body><h1>404 Not Found</h1></body></html>");
        return;
    }
    
    std::string content = readFile(fullPath);
    if (content.empty()) {
        response.setStatus(500);
        response.setHeader("Content-Type", "text/html");
        response.setBody("<html><body><h1>500 Internal Server Error</h1></body></html>");
        return;
    }
    
    response.setStatus(200);
    response.setHeader("Content-Type", getContentType(fullPath));
    response.setBody(content);
}
```

#### Test: Crear archivos de prueba
```bash
mkdir -p www
echo "<html><body><h1>Hello from index!</h1></body></html>" > www/index.html
echo "body { background-color: lightblue; }" > www/style.css
```

#### Test: `tests/test_filehandler.cpp`
```cpp
#include "../includes/FileHandler.hpp"
#include "../includes/HttpResponse.hpp"
#include <iostream>

int main() {
    HttpResponse response;
    
    // Test GET /index.html
    FileHandler::handleGet("/index.html", response);
    std::cout << response.build() << std::endl;
    
    // Test GET /notfound.html
    HttpResponse response2;
    FileHandler::handleGet("/notfound.html", response2);
    std::cout << "\n" << response2.build() << std::endl;
    
    return 0;
}
```

```bash
g++ -Wall -Wextra -Werror -std=c++98 srcs/HttpResponse.cpp srcs/FileHandler.cpp tests/test_filehandler.cpp -o test_filehandler
./test_filehandler
```

**✅ Objetivo del Día 7:** Poder servir archivos HTML, CSS, imágenes desde disco

---

## 🎯 SEMANA 2: Parser de Configuración

### DÍA 8-10: Entender formato de configuración

#### Ejemplo de config (similar a NGINX):
```nginx
# configs/default.conf
server {
    listen 8080;
    server_name localhost;
    root ./www;
    index index.html;
    client_max_body_size 10M;
    
    error_page 404 /errors/404.html;
    error_page 500 /errors/500.html;
    
    location / {
        methods GET POST;
        autoindex on;
    }
    
    location /upload {
        methods POST DELETE;
        upload_path ./uploads;
    }
    
    location /cgi-bin {
        methods GET POST;
        cgi_extension .php;
        cgi_path /usr/bin/php-cgi;
    }
}

server {
    listen 8081;
    server_name example.com;
    root ./www2;
}
```

#### Estructura de clases para Config:
```cpp
// includes/Config.hpp
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include <map>

struct LocationConfig {
    std::string path;                          // /upload
    std::vector<std::string> methods;          // GET, POST
    std::string root;                          // ./www
    std::string uploadPath;                    // ./uploads
    bool autoindex;                            // true/false
    std::string index;                         // index.html
    std::string cgiExtension;                  // .php
    std::string cgiPath;                       // /usr/bin/php-cgi
    std::string redirect;                      // /new-url
};

struct ServerConfig {
    int port;                                  // 8080
    std::string serverName;                    // localhost
    std::string root;                          // ./www
    std::string index;                         // index.html
    size_t clientMaxBodySize;                  // 10485760 (bytes)
    std::map<int, std::string> errorPages;     // 404 -> /errors/404.html
    std::map<std::string, LocationConfig> locations;  // "/" -> LocationConfig
};

class Config {
private:
    std::vector<ServerConfig> _servers;
    std::string _configFile;
    
    void parseFile();
    ServerConfig parseServer(std::ifstream& file);
    LocationConfig parseLocation(std::ifstream& file);

public:
    Config(const std::string& filename);
    ~Config();
    
    const std::vector<ServerConfig>& getServers() const;
    void print() const;
};

#endif
```

#### Implementación básica (simplificada):
```cpp
// srcs/Config.cpp
#include "../includes/Config.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

Config::Config(const std::string& filename) : _configFile(filename) {
    parseFile();
}

Config::~Config() {}

void Config::parseFile() {
    std::ifstream file(_configFile.c_str());
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open config file" << std::endl;
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Saltar líneas vacías y comentarios
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Buscar inicio de server block
        if (line.find("server") != std::string::npos) {
            ServerConfig server = parseServer(file);
            _servers.push_back(server);
        }
    }
    
    file.close();
}

ServerConfig Config::parseServer(std::ifstream& file) {
    ServerConfig server;
    server.port = 8080;  // default
    server.clientMaxBodySize = 1048576;  // 1MB default
    server.index = "index.html";
    
    std::string line;
    while (std::getline(file, line)) {
        // Remover espacios al inicio
        size_t start = line.find_first_not_of(" \t");
        if (start != std::string::npos) {
            line = line.substr(start);
        }
        
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Fin del server block
        if (line.find("}") != std::string::npos) {
            break;
        }
        
        // Parsear directivas
        if (line.find("listen") != std::string::npos) {
            std::istringstream iss(line);
            std::string key;
            iss >> key >> server.port;
        }
        else if (line.find("server_name") != std::string::npos) {
            std::istringstream iss(line);
            std::string key;
            iss >> key >> server.serverName;
        }
        else if (line.find("root") != std::string::npos) {
            std::istringstream iss(line);
            std::string key;
            iss >> key >> server.root;
        }
        // ... más directivas
    }
    
    return server;
}

const std::vector<ServerConfig>& Config::getServers() const {
    return _servers;
}

void Config::print() const {
    for (size_t i = 0; i < _servers.size(); i++) {
        std::cout << "Server " << i << ":" << std::endl;
        std::cout << "  Port: " << _servers[i].port << std::endl;
        std::cout << "  Name: " << _servers[i].serverName << std::endl;
        std::cout << "  Root: " << _servers[i].root << std::endl;
    }
}
```

**✅ Objetivo Día 8-10:** Parsear lo básico del config (listen, server_name, root)

---

## 📚 Recursos para Estudiar

### HTTP
- HTTP Made Really Easy: https://www.jmarshall.com/easy/http/
- RFC 2616 (HTTP/1.1): Secciones 4, 5, 6, 9
- MDN Web Docs - HTTP: https://developer.mozilla.org/es/docs/Web/HTTP

### C++ File I/O
- cplusplus.com - File I/O: https://cplusplus.com/doc/tutorial/files/
- Reading files line by line
- Using `stat()` para info de archivos

### Testing
- Usa `curl` constantemente
- Usa `telnet` para enviar requests manuales
- Compara con NGINX

---

## ✅ Checklist Primera Semana

- [ ] Entiendo la estructura de HTTP request
- [ ] Entiendo la estructura de HTTP response
- [ ] Puedo parsear un HTTP request básico
- [ ] Puedo construir un HTTP response válido
- [ ] Puedo leer archivos del disco
- [ ] Puedo determinar Content-Type por extensión
- [ ] Tengo tests que funcionan

---

## 🤝 Coordinación con Persona 1

Al final de la semana 1, deberías poder mostrarle:
1. Tu clase `HttpRequest` parseando requests
2. Tu clase `HttpResponse` generando responses válidas
3. Tu `FileHandler` leyendo archivos

Pregúntale:
- ¿Ya tiene el socket básico funcionando?
- ¿Cómo va a pasar los datos del socket a tu HttpRequest?
- ¿Cómo va a enviar tu HttpResponse de vuelta al cliente?

---

## 💡 Tips

1. **Compila frecuentemente** con `-Wall -Wextra -Werror`
2. **Testea cada función** antes de seguir
3. **Usa `std::string`** en lugar de char* para evitar problemas
4. **Lee el RFC HTTP** cuando tengas dudas
5. **Compara con NGINX** para ver comportamiento esperado
6. **No te compliques** - empieza simple y añade features gradualmente

---

## 🎯 Siguiente Paso

Una vez termines esta primera semana, tendrás:
- ✅ Parser de HTTP Request
- ✅ Builder de HTTP Response
- ✅ Handler para servir archivos (GET básico)

Y estarás listo para:
- 📝 Parser de configuración completo
- 📤 Método POST con upload
- 🗑️ Método DELETE
- 🔀 Redirects

**¡Empieza por el Día 1 y avanza paso a paso!** 🚀
