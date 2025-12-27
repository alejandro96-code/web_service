#include "server.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include <fcntl.h>

/*
    Creacion y conexion del Servidor:
    1º Tenemos que crear un socket (un punto de conexion) 
        AF_INET → Usar IPv4
        SOCK_STREAM → Conexión TCP (fiable, en orden)
        IPPROTO_TCP → Protocolo TCP
    2º Configuramos el non-blocking:
        Basicamente es una funcion para que el cliente no se quede congelado esperando
        una respuestay los demas clientes puedan seguir utilizando el servidor_web,
        si no hay datos devuelve un -1 con el mensaje de error y si hay datos, los devuelve.
        Debemos configurar esto tanto al crear el socket como al ejeutar las llamadas de los clientes
    3º Configuraremos nuestro socket
        _server_fd -> sera el servidor que hemos creado antes
        SOL_SOCKET -> tipo de configuracion que queremos recibir. en este caso(TCP/IP)
        SO_REUSEADD -> permite reusar el puerto inmediatamente.
    4º Vincularemos nuestro socket al puerto 8080
        INADDR_ANY → Escucha en todas las interfaces de red (localhost, IP externa, etc.)
        htons(_puerto) → Convierte el número de puerto al formato de red
        bind() → Asocia el socket con la dirección IP y puerto
    5º Escucharemos mediante el socket
        listen() → Pone el socket en modo escucha (para esperar las request)
        _backlog = 3 → Cola de espera (máximo 3 conexiones esperando)
        Ahora el servidor está listo para recibir conexion
*/
Server::Server(const ServerConfig& config): _server_fd(-1)
{
    _puerto = config.port;
    _backlog = 3;
    _document_root = config.root;
    _index_file = config.index;
    _server_name = config.server_name;
    _error_pages = config.error_pages;
    _locations = config.locations;
    _client_max_body_size = config.client_max_body_size;
}
Server::~Server(){if (_server_fd != -1) {close(_server_fd);}}

bool Server::crearSocket()
{
    _server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_server_fd == -1) {
        std::cerr << "Error al crear socket" << std::endl;
        return false;
    }
    if (fcntl(_server_fd, F_SETFL, O_NONBLOCK) == -1) {
        std::cerr << "Error al configurar non-blocking" << std::endl;
        close(_server_fd);
        _server_fd = -1;
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
    ManejarCliente: Lee datos del cliente de forma acumulativa.
    1. Lee chunk de datos (máx 4096 bytes)
    2. Acumula en buffer del cliente
    3. Si headers completos, verifica Content-Length
    4. Si Content-Length > límite, rechaza con 413
    5. Si petición completa, procesa y responde
*/
void Server::manejarCliente(int client_fd)
{
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    int bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_read == -1) {
        std::cerr << "Error en recv del cliente" << std::endl;
        close(client_fd);
        _active_clients.erase(client_fd);
        _client_buffers.erase(client_fd);
        _expected_content_length.erase(client_fd);
        _headers_complete.erase(client_fd);
        _bytes_to_discard.erase(client_fd);
        return;
    }
    if (bytes_read == 0) {
        close(client_fd);
        _active_clients.erase(client_fd);
        _client_buffers.erase(client_fd);
        _expected_content_length.erase(client_fd);
        _headers_complete.erase(client_fd);
        _bytes_to_discard.erase(client_fd);
        return;
    }
    
    // Si este cliente está siendo rechazado, descartar datos sin procesar
    if (_bytes_to_discard.find(client_fd) != _bytes_to_discard.end()) {
        size_t& remaining = _bytes_to_discard[client_fd];
        if ((size_t)bytes_read >= remaining) {
            _bytes_to_discard.erase(client_fd);
        } else {
            remaining -= bytes_read;
        }
        return;
    }
    
    // Acumular datos en el buffer del cliente
    _client_buffers[client_fd].append(buffer, bytes_read);
    
    // Si aún no hemos parseado los headers, intentar hacerlo
    if (!_headers_complete[client_fd]) {
        size_t header_end = _client_buffers[client_fd].find("\r\n\r\n");
        
        if (header_end != std::string::npos) {
            // Headers completos
            _headers_complete[client_fd] = true;
            std::string headers = _client_buffers[client_fd].substr(0, header_end);
            
            // Extraer Content-Length
            size_t content_length = extraerContentLength(headers);
            _expected_content_length[client_fd] = content_length;
            
            // VALIDAR LÍMITE INMEDIATAMENTE
            if (content_length > _client_max_body_size) {
                // Crear Request dummy para generar error 413 con template
                std::string rawRequest = _client_buffers[client_fd];
                Request request(rawRequest);
                Response response(request, _document_root, _error_pages, _locations, _client_max_body_size);
                std::string httpResponse = response.toString();
                
                // La validación en Response.cpp generará el 413 con template
                _pending_responses[client_fd] = httpResponse;
                
                // Calcular cuántos bytes del body faltan por recibir y descartar
                size_t header_end = _client_buffers[client_fd].find("\r\n\r\n");
                size_t bytes_already_received = _client_buffers[client_fd].length() - (header_end + 4);
                size_t bytes_remaining = content_length - bytes_already_received;
                
                // Marcar este cliente para descartar el resto de datos
                _bytes_to_discard[client_fd] = bytes_remaining;
                
                // NO borrar _active_clients aquí - se borrará después de enviar
                _client_buffers.erase(client_fd);
                _expected_content_length.erase(client_fd);
                _headers_complete.erase(client_fd);
                return;
            }
        }
    }
    
    // Si headers completos, verificar si tenemos toda la petición
    if (_headers_complete[client_fd] && peticionCompleta(client_fd)) {
        // Procesar petición completa
        std::string rawRequest = _client_buffers[client_fd];
        Request request(rawRequest);
        Response response(request, _document_root, _error_pages, _locations, _client_max_body_size);
        std::string httpResponse = response.toString();
        
        _pending_responses[client_fd] = httpResponse;
        _active_clients.erase(client_fd);
        
        // Limpiar buffers
        _client_buffers.erase(client_fd);
        _expected_content_length.erase(client_fd);
        _headers_complete.erase(client_fd);
    }
    // Si no está completa, seguir esperando más datos (se leerá en siguiente select)
}

// Extraer Content-Length de los headers
size_t Server::extraerContentLength(const std::string& headers)
{
    size_t pos = headers.find("Content-Length:");
    if (pos == std::string::npos)
        return 0;
    
    size_t start = pos + 15; // Longitud de "Content-Length:"
    size_t end = headers.find("\r\n", start);
    
    std::string value = headers.substr(start, end - start);
    
    // Limpiar espacios
    while (!value.empty() && value[0] == ' ')
        value = value.substr(1);
    
    return atoi(value.c_str());
}

// Verificar si tenemos la petición completa
bool Server::peticionCompleta(int client_fd)
{
    size_t header_end = _client_buffers[client_fd].find("\r\n\r\n");
    if (header_end == std::string::npos)
        return false;
    
    size_t header_size = header_end + 4;
    size_t body_received = _client_buffers[client_fd].length() - header_size;
    size_t expected = _expected_content_length[client_fd];
    
    return body_received >= expected;
}

/*
    Ejecuccion del servidor:
    1º  Preparamos 2 listas de vigilancia (FD_ZERO) debido a que usamos select()
        una ista sera para ver quien tiene algo para leer y otra quien tiene listo algo para escribir.
    2º  Añadimos el socket al servidor
    3º  Añadimos una lista de cliente (active_clients) y la recorremos para saber si los cliente si esperan ser leidos
    4º  Hacemos lo mismo con la lista de clientes con respuesta pendientes de escritura
    5º  LLamamos al select con un timeout de 1 segundo 
        (Es decir nuestro servidor "duerme") hasta 1 de lo siguiente:
            - un cliente nuevo se conecte
            - Un cliente envie datos (request)
            - Un cliente este listo para recibir datos (response)
            - Pase un segundo
    6º  Verificacion de las conexiones y hacemos el non-blocking
    7º  Leemios los datos de un cliente activo
    8º  Escribimos la respuesta de correspondiente y la enviamos
*/
void Server::ejecutar()
{
    while (true) {
        fd_set read_fds, write_fds;
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
    
        FD_SET(_server_fd, &read_fds);
        int max_fd = _server_fd;
        
        for (std::set<int>::iterator it = _active_clients.begin(); it != _active_clients.end(); ++it) {
            FD_SET(*it, &read_fds);
            if (*it > max_fd) max_fd = *it;
        }
        
        for (std::map<int, std::string>::iterator it = _pending_responses.begin(); 
             it != _pending_responses.end(); ++it) {
            FD_SET(it->first, &write_fds);
            if (it->first > max_fd) max_fd = it->first;
        }
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int activity = select(max_fd + 1, &read_fds, &write_fds, NULL, &timeout);
        
        if (activity < 0) {
            std::cerr << "Error en select" << std::endl;
            continue;
        }
        if (activity == 0) {
            continue;
        }
    
        if (FD_ISSET(_server_fd, &read_fds)) {
            int client_fd = accept(_server_fd, NULL, NULL);
            if (client_fd >= 0) {
                if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1) {
                    std::cerr << "Error configurando non-blocking en cliente" << std::endl;
                    close(client_fd);
                } else {
                    _active_clients.insert(client_fd);
                }
            }
        }
        
        std::set<int> clients_to_check = _active_clients;
        for (std::set<int>::iterator it = clients_to_check.begin(); 
             it != clients_to_check.end(); ++it) {
            if (FD_ISSET(*it, &read_fds)) {
                manejarCliente(*it);
            }
        }
        
        std::vector<int> clients_to_write;
        for (std::map<int, std::string>::iterator it = _pending_responses.begin(); 
             it != _pending_responses.end(); ++it) {
            if (FD_ISSET(it->first, &write_fds)) {
                clients_to_write.push_back(it->first);
            }
        }
        for (size_t i = 0; i < clients_to_write.size(); ++i) {
            int client_fd = clients_to_write[i];
            
            // Solo enviar si ya no hay bytes por descartar
            if (_bytes_to_discard.find(client_fd) != _bytes_to_discard.end()) {
                continue; // Esperar a que termine de descartar
            }
            
            std::string& response = _pending_responses[client_fd];
            
            ssize_t bytes_sent = send(client_fd, response.c_str(), response.length(), 0);
            
            if (bytes_sent == -1) {
                std::cerr << "Error al enviar respuesta" << std::endl;
            } else if (bytes_sent == 0) {
                std::cerr << "No se pudo enviar datos" << std::endl;
            }
            
            close(client_fd);
            _pending_responses.erase(client_fd);
            _active_clients.erase(client_fd);
            _bytes_to_discard.erase(client_fd);
        }
    }
}

