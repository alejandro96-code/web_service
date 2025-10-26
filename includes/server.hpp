#ifndef SERVER_HPP
#define SERVER_HPP

#include "webserv.hpp"
#include "parseoConf.hpp"

// Clase para manejar el servidor web
class Server {
private:
    int _puerto;
    int _backlog;
    std::string _document_root;
    std::string _index_file;
    std::string _server_name;
    std::map<int, std::string> _error_pages;
    int _server_fd;

public:
    
    Server(const ServerConfig& config); // Constructor
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
