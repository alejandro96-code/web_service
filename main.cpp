#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <cstdlib>
#include "parseoConf.hpp"

int main() {
    // Leer configuración
    std::map<std::string, std::string> config = leerConfig("server.conf");
    
    // Obtener valores de configuración
    int puerto = atoi(config["port"].c_str());
    int backlog = atoi(config["backlog"].c_str());
    std::string document_root = config["document_root"];
    std::string index_file = config["index_file"];
    
    //stdcout para saber si lo leo correctamente
    std::cout << "Configuración cargada:" << std::endl;
    std::cout << "  Puerto: " << puerto << std::endl;
    std::cout << "  Backlog: " << backlog << std::endl;
    std::cout << "  Document root: " << document_root << std::endl;
    std::cout << "  Index file: " << index_file << std::endl << std::endl;
    
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
    
    // 3. Configurar dirección: localhost:puerto
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(puerto);
    
    // 4. Bind al puerto
    if (bind(server, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "No se puede vincular localhost al puerto " << puerto << std::endl;
        close(server);
        return 1;
    }
    
    // 5. Escuchar conexiones entrantes
    if (listen(server, backlog) < 0) {
        std::cerr << "Error en listen" << std::endl;
        close(server);
        return 1;
    }
    
    // 6. Aceptar conexiones
    while (true) {
        int client_fd = accept(server, NULL, NULL);
        if (client_fd < 0) {
            continue;
        }
        
        std::cout << "Nueva conexión" << std::endl;
        
        // Leer el archivo HTML configurado
        std::string ruta_html = document_root + "/" + index_file;
        std::ifstream file(ruta_html.c_str());
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
