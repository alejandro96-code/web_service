#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "webserv.hpp"
#include "Request.hpp"

class Response {
private:
    int _statusCode;
    std::string _statusMessage;
    std::map<std::string, std::string> _headers;
    std::string _body;
    std::string _documentRoot;

    // Métodos privados auxiliares
    std::string leerArchivo(const std::string& ruta);
    std::string obtenerContentType(const std::string& extension);
    std::string construirRespuestaHTTP();
    
    // Métodos para manejar cada tipo de petición
    void manejarGET(const Request& request);
    void manejarPOST(const Request& request);
    void manejarDELETE(const Request& request);
    
    // Métodos para páginas de error
    void respuestaError(int codigo);

public:
    
    Response(const Request& request, const std::string& documentRoot); // Constructor
    ~Response(); // Destructor
    
    // Método principal para procesar la petición y generar respuesta
    void procesar(const Request& request);
    
    // Obtener la respuesta HTTP completa como string
    std::string toString() const;
    
    // Getters
    int getStatusCode() const;
    std::string getBody() const;
};

#endif
