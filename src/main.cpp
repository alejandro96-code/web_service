#include "webserv.hpp"
#include "parseoConf.hpp"
#include "server.hpp"

int main(int argc, char *argv[]) {
    // Verificar que se proporcione el archivo de configuración
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " [archivo de configuración]" << std::endl;
        return 1;
    }
    
    // Leer configuración del archivo proporcionado
    std::vector<ServerConfig> servers = leerConfig(argv[1]);
    
    // Verificar que se haya cargado la configuración correctamente
    if (servers.empty()) {
        return 1;
    }
    
    // Mostrar configuración parseada
    std::cout << "Configuración cargada:" << std::endl;
    std::cout << "  Número de servidores: " << servers.size() << std::endl << std::endl;
    
    // Crear e iniciar el servidor con la nueva configuración
    Server servidor(servers[0]);
    servidor.iniciar();
    servidor.ejecutar();
    
    return 0;
}
