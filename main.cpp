#include "webserv.hpp"
#include "parseoConf.hpp"
#include "server.hpp"
#include <fcntl.h>

/*
    Servidor multi-puerto:
    1. Lee el archivo de configuracion, cada ServerConfig representa un servidor individual
    2. Crear una instancia Server por cada servidor configurado y los inicializa
    3. Bucle principal con select() monitoreando TODOS los server_fd
        3.1 Prepara los file descriptors
        3.2 Se realizan las llamadas al select
        3.3 Se aceptan las conexiones
        3.4 Se leen los datos del cliente
        3.5 Se envia la respuesta al cliente
    4. Se libera la memoria de todos los servidores creados dinamicamente
*/

int main(int argc, char *argv[]) {
    
    if (argc != 2) {
        std::cerr << FORMA_DE_USO << std::endl;
        return 1;
    }
    
    /* Paso 1 */
    std::vector<ServerConfig> configs = leerConfig(argv[1]);
    if (configs.empty()) {
        return 1;
    }
    
    std::cout << NUMERO_DE_SERVIDORES << configs.size() << std::endl << std::endl;
    
    /* Paso 2 */
    std::vector<Server*> servidores;
    for (size_t i = 0; i < configs.size(); i++) {
        Server* srv = new Server(configs[i]);
        srv->iniciar();
        servidores.push_back(srv);
    }
    
    /* Paso 3 */
    while (true) {
        fd_set read_fds, write_fds;
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        
        int max_fd = -1;
        
        /* Paso 3.1 */
        for (size_t i = 0; i < servidores.size(); i++) {
            int server_fd = servidores[i]->getServerFd();
            if (server_fd > 0) {
                FD_SET(server_fd, &read_fds);
                if (server_fd > max_fd) max_fd = server_fd;
            }
        }
        
        for (size_t i = 0; i < servidores.size(); i++) {
            std::set<int>& clients = servidores[i]->getActiveClients();
            std::map<int, std::string>& pending = servidores[i]->getPendingResponses();
            
            for (std::set<int>::iterator it = clients.begin(); it != clients.end(); ++it) {
                if (pending.find(*it) == pending.end()) {
                    FD_SET(*it, &read_fds);
                    if (*it > max_fd) max_fd = *it;
                }
            }
        }
        
        for (size_t i = 0; i < servidores.size(); i++) {
            std::map<int, std::string>& pending = servidores[i]->getPendingResponses();
            for (std::map<int, std::string>::iterator it = pending.begin(); 
                 it != pending.end(); ++it) {
                FD_SET(it->first, &write_fds);
                if (it->first > max_fd) max_fd = it->first;
            }
        }
        
        if (max_fd == -1) {
            std::cerr << NOT_FILE_DESCRIPTOR << std::endl;
            break;
        }
        
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        /* Paso 3.2 */
        int activity = select(max_fd + 1, &read_fds, &write_fds, NULL, &timeout);
        
        if (activity < 0) {
            std::cerr << ERROR_SELECT << std::endl;
            continue;
        }
        if (activity == 0) {
            continue;
        }
        
        /* Paso 3.3 */
        for (size_t i = 0; i < servidores.size(); i++) {
            int server_fd = servidores[i]->getServerFd();
            if (server_fd > 0 && FD_ISSET(server_fd, &read_fds)) {
                int client_fd = accept(server_fd, NULL, NULL);
                if (client_fd >= 0) {
                    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1) {
                        std::cerr << ERROR_NON_BLOCKING << std::endl;
                        close(client_fd);
                    } else {
                        servidores[i]->getActiveClients().insert(client_fd);
                    }
                }
            }
        }
        
        /* Paso 3.4 */
        for (size_t i = 0; i < servidores.size(); i++) {
            std::set<int> clients_copy = servidores[i]->getActiveClients();
            for (std::set<int>::iterator it = clients_copy.begin(); 
                 it != clients_copy.end(); ++it) {
                if (FD_ISSET(*it, &read_fds)) {
                    servidores[i]->manejarClientePublic(*it);
                }
            }
        }
        
        /* Paso 3.5 */
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
                    std::cerr << ERROR_RESPONSE << std::endl;
                } else if (bytes_sent == 0) {
                    std::cerr << ERROR_NOT_DATOS << std::endl;
                }
                
                close(client_fd);
                pending.erase(client_fd);
                servidores[i]->getActiveClients().erase(client_fd);
            }
        }
    }
    
    /* Paso 4 */
    for (size_t i = 0; i < servidores.size(); i++) {
        delete servidores[i];
    }
    
    return 0;
}
