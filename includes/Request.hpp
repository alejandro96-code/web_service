#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "webserv.hpp"

// Clase para manejar peticiones HTTP
class Request {
private:
    std::string _rawRequest;                        // Petición completa recibida
    std::string _method;                            // GET, POST, DELETE
    std::string _path;                              // /index.html, /upload, etc.
    std::string _version;                           // HTTP/1.1
    std::map<std::string, std::string> _headers;    // Headers de la petición
    std::string _body;                              // Cuerpo de la petición (para POST)
    bool _isChunked;                                // Transfer-Encoding: chunked

public:
    
    Request(); // Constructor
    Request(const std::string& rawRequest); // Constructor con parametros
    
    
    void parse(const std::string& rawRequest); // Parsear la petición recibida
    
    // Getters
    std::string getMethod() const;
    std::string getPath() const;
    std::string getVersion() const;
    std::string getHeader(const std::string& key) const;
    std::string getBody() const;
    bool isChunked() const;
    
    // Método estático para decodificar chunks
    static std::string decodeChunkedBody(const std::string& chunkedData);
};

#endif
