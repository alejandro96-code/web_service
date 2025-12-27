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
    std::vector<Location> _locations;
    size_t _client_max_body_size;
    int _server_fd;
    std::set<int> _active_clients;
    std::map<int, std::string> _pending_responses;
    
    // Buffers acumulativos para peticiones grandes
    std::map<int, std::string> _client_buffers;      // Buffer acumulado por cliente
    std::map<int, size_t> _expected_content_length;  // Content-Length esperado
    std::map<int, bool> _headers_complete;           // ¿Headers ya leídos?
    std::map<int, size_t> _bytes_to_discard;         // Bytes que faltan por descartar (clientes rechazados)

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
    
    // Métodos auxiliares
    size_t extraerContentLength(const std::string& headers);
    bool peticionCompleta(int client_fd);
};

#endif
