#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "webserv.hpp"
#include "Request.hpp"
#include "parseoConf.hpp"
#include "Autoindex.hpp"
#include "HttpStatus.hpp"

class Response {
protected:
    int _statusCode;
    std::string _statusMessage;
    std::map<std::string, std::string> _headers;
    std::string _body;
    std::string _documentRoot;
    std::map<int, std::string> _errorPages;
    std::vector<Location> _locations;

    // Métodos auxiliares compartidos
    std::string leerArchivo(const std::string& ruta);
    std::string obtenerContentType(const std::string& extension);
    bool esDirectorio(const std::string& ruta);
    bool tieneAutoindex(const std::string& path);
    bool metodoPermitido(const std::string& path, const std::string& method);
    Location* obtenerLocation(const std::string& path);
    void respuestaError(int codigo);

public:
    Response(const Request& request, const std::string& documentRoot, 
             const std::map<int, std::string>& errorPages,
             const std::vector<Location>& locations);
    virtual ~Response();
    
    void procesar(const Request& request);
    std::string toString() const;
    
    int getStatusCode() const;
    std::string getBody() const;

protected:
    // Métodos virtuales para cada tipo de petición
    virtual void manejarGET(const Request& request);
    virtual void manejarPOST(const Request& request);
    virtual void manejarDELETE(const Request& request);
};

#endif
