#include "server.hpp"
#include "Request.hpp"

// Constructor
Server::Server(const std::map<std::string, std::string>& config): _server_fd(-1)
{
    _puerto = atoi(config.find("port")->second.c_str());
    _backlog = atoi(config.find("backlog")->second.c_str());
    _document_root = config.find("document_root")->second;
    _index_file = config.find("index_file")->second;
    
    std::cout << "Configuración cargada:" << std::endl;
    std::cout << "  Puerto: " << _puerto << std::endl;
    std::cout << "  Backlog: " << _backlog << std::endl;
    std::cout << "  Document root: " << _document_root << std::endl;
    std::cout << "  Index file: " << _index_file << std::endl << std::endl;
}

// Destructor
Server::~Server()
{
    if (_server_fd != -1) {
        close(_server_fd);
    }
}

// Crear socket
bool Server::crearSocket() {
    _server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_server_fd == -1) {
        std::cerr << "Error al crear socket" << std::endl;
        return false;
    }
    return true;
}

// Configurar socket
bool Server::configurarSocket() {
    int opt = 1;
    if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error al configurar socket" << std::endl;
        return false;
    }
    return true;
}

// Vincular puerto
bool Server::vincularPuerto() {
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(_puerto);
    
    if (bind(_server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "No se puede vincular localhost al puerto " << _puerto << std::endl;
        return false;
    }
    return true;
}

// Escuchar conexiones
bool Server::escucharConexiones() {
    if (listen(_server_fd, _backlog) < 0)
    {
        std::cerr << "Error en listen" << std::endl;
        return false;
    }
    std::cout << "Servidor escuchando en el puerto " << _puerto << std::endl;
    return true;
}

// Iniciar el servidor
void Server::iniciar()
{
    if (!crearSocket()) return;
    if (!configurarSocket()) return;
    if (!vincularPuerto()) return;
    if (!escucharConexiones()) return;
}

// Leer archivo HTML
std::string Server::leerArchivoHTML(const std::string& ruta)
{
    std::ifstream file(ruta.c_str());
    if (!file.is_open()) {
        return "<html><body><h1>404 - Archivo no encontrado</h1></body></html>";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Manejar cliente
void Server::manejarCliente(int client_fd)
{
    std::cout << "Nueva conexión" << std::endl;
    
    // 1. Leer la petición del cliente
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    int bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_read <= 0) {
        close(client_fd);
        return;
    }
    
    std::string rawRequest(buffer);
    
    // 2. Parsear la petición con nuestra clase Request
    Request request(rawRequest);
    request.print();  // Para ver qué recibimos (debug)
    
    // 3. Por ahora, siempre servimos index.html
    std::string ruta_html = _document_root + "/" + _index_file;
    std::string html = leerArchivoHTML(ruta_html);
    
    // 4. Construir respuesta HTTP
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/html\r\n\r\n";
    response += html;
    
    send(client_fd, response.c_str(), response.length(), 0);
    close(client_fd);
}

// Ejecutar el servidor (bucle principal)
void Server::ejecutar()
{
    while (true) {
        int client_fd = accept(_server_fd, NULL, NULL);
        if (client_fd < 0) {
            continue;
        }
        manejarCliente(client_fd);
    }
}
