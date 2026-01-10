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
    std::map<int, bool> _is_chunked;                 // ¿Cliente usando Transfer-Encoding: chunked?

public:
    
    Server(const ServerConfig& config); // Constructor
    ~Server(); // Destructor
    
    // Métodos públicos
    void iniciar();
    
    // Getters para acceder a datos privados
    int getServerFd() const { return _server_fd; }
    std::set<int>& getActiveClients() { return _active_clients; }
    std::map<int, std::string>& getPendingResponses() { return _pending_responses; }
    void manejarClientePublic(int client_fd) { manejarCliente(client_fd); }
    
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
    bool isTransferEncodingChunked(const std::string& headers);
    bool chunkedDataCompleta(const std::string& data);
    std::string decodeChunkedForValidation(const std::string& chunkedData);
};

#endif
