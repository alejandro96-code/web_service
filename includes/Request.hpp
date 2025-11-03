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

public:
    // Constructor
    Request();
    Request(const std::string& rawRequest);
    
    // Parsear la petición recibida
    void parse(const std::string& rawRequest);
    
    // Getters
    std::string getMethod() const;
    std::string getPath() const;
    std::string getVersion() const;
    std::string getHeader(const std::string& key) const;
    std::string getBody() const;
    
    // Para debug
    void print() const;
};

#endif
