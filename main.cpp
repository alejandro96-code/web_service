#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fstream>
#include <sstream>

int main() {
    // 1. Crear socket
    int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == -1)
    {
        std::cerr << "Error al crear socket" << std::endl;
        return 1;
    }
    
    // 2. Reusar puerto
    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // 3. Configurar dirección: localhost:8080
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);
    
    // 4. Bind al puerto 8080
    if (bind(server, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "No se puede vincular localhost al puerto 8080" << std::endl;
        close(server);
        return 1;
    }

    std::cout << "Conexion con el servidor" << std::endl;
    
    // 6. Aceptar conexiones
    while (true) {
        int client_fd = accept(server, NULL, NULL);
        if (client_fd < 0) {
            continue;
        }
        
        std::cout << "Nueva conexión" << std::endl;
        
        // Leer el index.html
        std::ifstream file("www/index.html");
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string html = buffer.str();
        
        // Construir respuesta HTTP
        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: text/html\r\n\r\n";
        response += html;
        
        send(client_fd, response.c_str(), response.length(), 0);
        
        close(client_fd);
    }
    
    close(server);
    return 0;
}
