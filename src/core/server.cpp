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
    _port = config.port;
    _backlog = 3;
    _document_root = config.root;
    _index_file = config.index;
    _server_name = config.server_name;
    _error_pages = config.error_pages;
    _locations = config.locations;
    _client_max_body_size = config.client_max_body_size;
}
Server::~Server(){if (_server_fd != -1) {close(_server_fd);}}

bool Server::createSocket()
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

bool Server::configureSocket()
{
    int opt = 1;
    if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error al configurar socket" << std::endl;
        return false;
    }
    return true;
}

bool Server::bindPort()
{
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(_port);
    
    if (bind(_server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "No se puede vincular localhost al puerto " << _port << std::endl;
        return false;
    }
    return true;
}

bool Server::listenConnections() {
    if (listen(_server_fd, _backlog) < 0)
    {
        std::cerr << "Error en listen" << std::endl;
        return false;
    }
    return true;
}

void Server::start()
{
    if (!createSocket()) {
        std::cerr << "Error al crear socket para puerto " << _port << std::endl;
        return;
    }
    if (!configureSocket()) {
        std::cerr << "Error al configurar socket para puerto " << _port << std::endl;
        return;
    }
    if (!bindPort()) {
        std::cerr << "Error al vincular puerto " << _port << std::endl;
        return;
    }
    if (!listenConnections()) {
        std::cerr << "Error al escuchar en puerto " << _port << std::endl;
        return;
    }
    std::cout << "✓ Servidor escuchando en puerto " << _port << std::endl;
}

/*
    ManejarCliente: Lee datos del cliente de forma acumulativa.
    1. Lee chunk de datos (máx 4096 bytes)
    2. Acumula en buffer del cliente
    3. Si headers completos, verifica Content-Length
    4. Si Content-Length > límite, rechaza con 413
    5. Si petición completa, procesa y responde
*/
void Server::handleClient(int client_fd)
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
        _is_chunked.erase(client_fd);
        return;
    }
    if (bytes_read == 0) {
        close(client_fd);
        _active_clients.erase(client_fd);
        _client_buffers.erase(client_fd);
        _expected_content_length.erase(client_fd);
        _headers_complete.erase(client_fd);
        _is_chunked.erase(client_fd);
        return;
    }
    
    // Si este cliente tiene respuesta 413 pendiente, descartar datos hasta completar
    if (_pending_responses.find(client_fd) != _pending_responses.end()) {
        // Descartar datos, el navegador necesita completar el envío
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
            
            // Detectar si es chunked
            _is_chunked[client_fd] = isTransferEncodingChunked(headers);
            
            // Extraer Content-Length (solo si NO es chunked)
            if (!_is_chunked[client_fd]) {
                size_t content_length = extractContentLength(headers);
                _expected_content_length[client_fd] = content_length;
                
                // VALIDAR LÍMITE INMEDIATAMENTE
                if (content_length > _client_max_body_size) {
                    // Crear respuesta 413 con template
                    std::string rawRequest = _client_buffers[client_fd];
                    Request request(rawRequest);
                    Response response(request, _document_root, _error_pages, _locations, _client_max_body_size, _index_file);
                    std::string httpResponse = response.toString();
                    
                    // Guardar respuesta para enviar DESPUÉS de recibir todo
                    _pending_responses[client_fd] = httpResponse;
                    
                    // NO limpiamos buffers, seguimos acumulando para completar petición
                    return;
                }
            }
            // Si es chunked, no validamos tamaño hasta decodificar
        }
    }
    
    // Si headers completos, verificar si tenemos toda la petición
    if (_headers_complete[client_fd] && isRequestComplete(client_fd)) {
        // Si es chunked, validar el tamaño decodificado antes de procesar
        if (_is_chunked[client_fd]) {
            size_t header_end = _client_buffers[client_fd].find("\r\n\r\n");
            std::string headers = _client_buffers[client_fd].substr(0, header_end);
            std::string rawBody = _client_buffers[client_fd].substr(header_end + 4);
            
            // Decodificar chunked para obtener tamaño real
            std::string decodedBody = decodeChunkedForValidation(rawBody);
            size_t actualSize = decodedBody.length();
            
            // Validar límite
            if (actualSize > _client_max_body_size) {
                // Crear respuesta 413 con template
                std::string rawRequest = _client_buffers[client_fd];
                Request request(rawRequest);
                Response response(request, _document_root, _error_pages, _locations, _client_max_body_size, _index_file);
                std::string httpResponse = response.toString();
                
                // Guardar respuesta para enviar
                _pending_responses[client_fd] = httpResponse;
                
                // Limpiar buffers
                _client_buffers.erase(client_fd);
                _expected_content_length.erase(client_fd);
                _headers_complete.erase(client_fd);
                _is_chunked.erase(client_fd);
                return;
            }
        }
        
        // Procesar petición completa
        std::string rawRequest = _client_buffers[client_fd];
        Request request(rawRequest);
        Response response(request, _document_root, _error_pages, _locations, _client_max_body_size, _index_file);
        std::string httpResponse = response.toString();
        
        _pending_responses[client_fd] = httpResponse;
        
        // Limpiar buffers
        _client_buffers.erase(client_fd);
        _expected_content_length.erase(client_fd);
        _headers_complete.erase(client_fd);
        _is_chunked.erase(client_fd);
    }
    // Si no está completa, seguir esperando más datos (se leerá en siguiente select)
}

// Extraer Content-Length de los headers
size_t Server::extractContentLength(const std::string& headers)
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
bool Server::isRequestComplete(int client_fd)
{
    size_t header_end = _client_buffers[client_fd].find("\r\n\r\n");
    if (header_end == std::string::npos)
        return false;
    
    // Si es chunked, verificar que tenemos todos los chunks
    if (_is_chunked[client_fd]) {
        size_t header_size = header_end + 4;
        std::string body = _client_buffers[client_fd].substr(header_size);
        return isChunkedDataComplete(body);
    }
    
    // Si no es chunked, verificar Content-Length normal
    size_t header_size = header_end + 4;
    size_t body_received = _client_buffers[client_fd].length() - header_size;
    size_t expected = _expected_content_length[client_fd];
    
    return body_received >= expected;
}

// Detectar si Transfer-Encoding: chunked está presente en los headers
bool Server::isTransferEncodingChunked(const std::string& headers)
{
    size_t pos = headers.find("Transfer-Encoding:");
    if (pos == std::string::npos)
        return false;
    
    size_t start = pos + 18; // Longitud de "Transfer-Encoding:"
    size_t end = headers.find("\r\n", start);
    
    std::string value = headers.substr(start, end - start);
    
    // Buscar "chunked" en el valor
    return value.find("chunked") != std::string::npos;
}

// Verificar si tenemos todos los chunks (último chunk es "0\r\n\r\n")
bool Server::isChunkedDataComplete(const std::string& data)
{
    // Buscar el patrón de finalización: "0\r\n\r\n"
    // El último chunk tiene tamaño 0
    size_t pos = data.find("0\r\n\r\n");
    if (pos != std::string::npos) {
        return true;
    }
    
    // También puede ser "0\r\n" seguido de trailers y luego "\r\n"
    // pero por simplicidad, buscamos el patrón básico
    return false;
}

// Decodificar chunked para validación de tamaño
std::string Server::decodeChunkedForValidation(const std::string& chunkedData)
{
    std::string decoded;
    size_t pos = 0;
    
    while (pos < chunkedData.length()) {
        // Buscar el fin de la línea del tamaño
        size_t sizeEnd = chunkedData.find("\r\n", pos);
        if (sizeEnd == std::string::npos) {
            break; // No hay más chunks válidos
        }
        
        // Extraer el tamaño del chunk (en hexadecimal)
        std::string sizeStr = chunkedData.substr(pos, sizeEnd - pos);
        
        // Convertir de hexadecimal a decimal
        size_t chunkSize = 0;
        std::istringstream(sizeStr) >> std::hex >> chunkSize;
        
        // Si el tamaño es 0, hemos llegado al último chunk
        if (chunkSize == 0) {
            break;
        }
        
        // Posición del inicio de los datos
        size_t dataStart = sizeEnd + 2; // Saltar \r\n
        
        // Verificar que no nos pasemos del límite
        if (dataStart + chunkSize > chunkedData.length()) {
            break; // Datos incompletos
        }
        
        // Extraer los datos del chunk
        decoded += chunkedData.substr(dataStart, chunkSize);
        
        // Moverse al siguiente chunk (saltar datos + \r\n)
        pos = dataStart + chunkSize + 2;
    }
    
    return decoded;
}

// Nota: ejecutar() ahora está en main.cpp para manejar múltiples servidores

