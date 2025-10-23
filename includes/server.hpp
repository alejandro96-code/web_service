#ifndef SERVER_HPP
#define SERVER_HPP

#include "webserv.hpp"

// Clase para manejar el servidor web
class Server {
private:
    int _puerto;
    int _backlog;
    std::string _document_root;
    std::string _index_file;
    int _server_fd;

public:
    
    Server(const std::map<std::string, std::string>& config); // Constructor
    ~Server(); // Destructor
    
    // Métodos públicos
    void iniciar();
    void ejecutar();
    
private:
    // Métodos privados
    bool crearSocket();
    bool configurarSocket();
    bool vincularPuerto();
    bool escucharConexiones();
    void manejarCliente(int client_fd);
    std::string leerArchivoHTML(const std::string& ruta);
};

#endif
