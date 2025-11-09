#include "server.hpp"
#include "Request.hpp"
#include "Response.hpp"

/*
    Creacion y conexion del Servidor:
    Primero: tenemos que crear un socket (un punto de conexion) 
        AF_INET → Usar IPv4
        SOCK_STREAM → Conexión TCP (fiable, en orden)
        IPPROTO_TCP → Protocolo TCP
    Segundo: configuraremos nuestro socket
        _server_fd -> sera el servidor que hemos creado antes
        SOL_SOCKET -> tipo de configuracion que queremos recibir. en este caso(TCP/IP)
        SO_REUSEADD -> permite reusar el puerto inmediatamente.
    Tercero vincularemos nuestro socket al puerto 8080
        INADDR_ANY → Escucha en todas las interfaces de red (localhost, IP externa, etc.)
        htons(_puerto) → Convierte el número de puerto al formato de red
        bind() → Asocia el socket con la dirección IP y puerto
    Cuarto: Escucharemos mediante el socket
        listen() → Pone el socket en modo escucha (para esperar las request)
        _backlog = 3 → Cola de espera (máximo 3 conexiones esperando)
        Ahora el servidor está listo para recibir conexion
*/
Server::Server(const ServerConfig& config): _server_fd(-1) // Constructor
{
    _puerto = config.port;
    _backlog = 3; // Valor por defecto
    _document_root = config.root;
    _index_file = config.index;
    _server_name = config.server_name;
    _error_pages = config.error_pages;
    _locations = config.locations;
    
    std::cout << "Configuración cargada:" << std::endl;
    std::cout << "  Puerto: " << _puerto << std::endl;
    std::cout << "  Server name: " << _server_name << std::endl;
    std::cout << "  Document root: " << _document_root << std::endl;
    std::cout << "  Index file: " << _index_file << std::endl;
    std::cout << "  Páginas de error: " << _error_pages.size() << std::endl;
    std::cout << "  Locations: " << _locations.size() << std::endl << std::endl;
}
Server::~Server(){if (_server_fd != -1) {close(_server_fd);}} // Destructor

bool Server::crearSocket()
{
    _server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_server_fd == -1) {
        std::cerr << "Error al crear socket" << std::endl;
        return false;
    }
    return true;
}

bool Server::configurarSocket()
{
    int opt = 1;
    if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error al configurar socket" << std::endl;
        return false;
    }
    return true;
}

bool Server::vincularPuerto()
{
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

bool Server::escucharConexiones() {
    if (listen(_server_fd, _backlog) < 0)
    {
        std::cerr << "Error en listen" << std::endl;
        return false;
    }
    std::cout << "Servidor escuchando en el puerto " << _puerto << std::endl;
    return true;
}

void Server::iniciar()
{
    if (!crearSocket()) return;
    if (!configurarSocket()) return;
    if (!vincularPuerto()) return;
    if (!escucharConexiones()) return;
}

/*
    ManejarCliente: Funcion para procesar una peticion http
    creamos un buufer de 4096 bytes, lo llenamos de 0
    recibe los datos del cliente y los lee.
    convertimos la info del buffer en string
    y la guardamos en un objeto de tipo REQUEST
    Generamos nuestra respuesta de tipo RESPONSE
    y enviamos la respuesta al cliente.
*/
void Server::manejarCliente(int client_fd)
{
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    int bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_read <= 0) {
        close(client_fd);
        return;
    }
    
    std::string rawRequest(buffer);
    Request request(rawRequest);
    Response response(request, _document_root, _error_pages, _locations);
    std::string httpResponse = response.toString();
    send(client_fd, httpResponse.c_str(), httpResponse.length(), 0);
    close(client_fd);
}

/*
    El server se queda esperando hasta que entre un nuevo cliente_fd
    cliente_fd (Línea individual para hablar con un cliente específico añadiendole nuestro server_fd)
    cuando haya un cliente lanzara manejarCliente(client_fd);
*/
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
