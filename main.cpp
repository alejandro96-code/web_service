#include "webserv.hpp"
#include "parseoConf.hpp"
#include "server.hpp"
#include <fcntl.h>

/*
    Servidor multi-puerto:
    1. Cargar configuración con múltiples servidores
    2. Crear una instancia Server por cada servidor configurado
    3. Inicializar cada servidor (socket, bind, listen)
    4. Bucle principal con select() monitoreando TODOS los server_fd
    5. Cuando hay actividad en un server_fd: accept() y crear cliente
    6. Cuando hay actividad en un cliente: leer/escribir datos
*/
int main(int argc, char *argv[]) {
    
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " [archivo de configuración]" << std::endl;
        return 1;
    }
    
    std::vector<ServerConfig> configs = leerConfig(argv[1]);
    if (configs.empty()) {
        return 1;
    }
    
    std::cout << "Número de servidores: " << configs.size() << std::endl << std::endl;
    
    // Crear e inicializar todos los servidores
    std::vector<Server*> servidores;
    for (size_t i = 0; i < configs.size(); i++) {
        Server* srv = new Server(configs[i]);
        srv->iniciar();
        servidores.push_back(srv);
    }
    
    // Bucle principal con select() sobre TODOS los servidores
    while (true) {
        fd_set read_fds, write_fds;
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        
        int max_fd = -1;
        
        // Añadir todos los server_fd al set de lectura
        for (size_t i = 0; i < servidores.size(); i++) {
            int server_fd = servidores[i]->getServerFd();
            if (server_fd > 0) {
                FD_SET(server_fd, &read_fds);
                if (server_fd > max_fd) max_fd = server_fd;
            }
        }
        
        // Añadir todos los clientes activos de todos los servidores
        for (size_t i = 0; i < servidores.size(); i++) {
            std::set<int>& clients = servidores[i]->getActiveClients();
            std::map<int, std::string>& pending = servidores[i]->getPendingResponses();
            
            for (std::set<int>::iterator it = clients.begin(); it != clients.end(); ++it) {
                // Solo agregar para lectura si NO tiene respuesta pendiente
                if (pending.find(*it) == pending.end()) {
                    FD_SET(*it, &read_fds);
                    if (*it > max_fd) max_fd = *it;
                }
            }
        }
        
        // Añadir clientes con respuestas pendientes para escritura
        for (size_t i = 0; i < servidores.size(); i++) {
            std::map<int, std::string>& pending = servidores[i]->getPendingResponses();
            for (std::map<int, std::string>::iterator it = pending.begin(); 
                 it != pending.end(); ++it) {
                FD_SET(it->first, &write_fds);
                if (it->first > max_fd) max_fd = it->first;
            }
        }
        
        if (max_fd == -1) {
            std::cerr << "No hay file descriptors válidos" << std::endl;
            break;
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
            continue; // Timeout
        }
        
        // Verificar nuevas conexiones en cada servidor
        for (size_t i = 0; i < servidores.size(); i++) {
            int server_fd = servidores[i]->getServerFd();
            if (server_fd > 0 && FD_ISSET(server_fd, &read_fds)) {
                int client_fd = accept(server_fd, NULL, NULL);
                if (client_fd >= 0) {
                    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1) {
                        std::cerr << "Error configurando non-blocking en cliente" << std::endl;
                        close(client_fd);
                    } else {
                        servidores[i]->getActiveClients().insert(client_fd);
                    }
                }
            }
        }
        
        // Leer datos de clientes activos
        for (size_t i = 0; i < servidores.size(); i++) {
            std::set<int> clients_copy = servidores[i]->getActiveClients();
            for (std::set<int>::iterator it = clients_copy.begin(); 
                 it != clients_copy.end(); ++it) {
                if (FD_ISSET(*it, &read_fds)) {
                    servidores[i]->manejarClientePublic(*it);
                }
            }
        }
        
        // Escribir respuestas pendientes
        for (size_t i = 0; i < servidores.size(); i++) {
            std::map<int, std::string>& pending = servidores[i]->getPendingResponses();
            std::vector<int> clients_to_write;
            
            for (std::map<int, std::string>::iterator it = pending.begin(); 
                 it != pending.end(); ++it) {
                if (FD_ISSET(it->first, &write_fds)) {
                    clients_to_write.push_back(it->first);
                }
            }
            
            for (size_t j = 0; j < clients_to_write.size(); j++) {
                int client_fd = clients_to_write[j];
                std::string& response = pending[client_fd];
                
                ssize_t bytes_sent = send(client_fd, response.c_str(), response.length(), 0);
                
                if (bytes_sent == -1) {
                    std::cerr << "Error al enviar respuesta" << std::endl;
                } else if (bytes_sent == 0) {
                    std::cerr << "No se pudo enviar datos" << std::endl;
                }
                
                close(client_fd);
                pending.erase(client_fd);
                servidores[i]->getActiveClients().erase(client_fd);
            }
        }
    }
    
    // Limpieza
    for (size_t i = 0; i < servidores.size(); i++) {
        delete servidores[i];
    }
    
    return 0;
}
