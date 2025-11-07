#include "webserv.hpp"
#include "parseoConf.hpp"
#include "server.hpp"

/*
    Verificar que se proporcione el archivo de configuración
    Leer configuración del archivo proporcionado
    Verificar que se haya cargado la configuración correctamente
    Mostrar configuración parseada
    Crear e iniciar el servidor con la nueva configuración
*/
int main(int argc, char *argv[]) {
    
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " [archivo de configuración]" << std::endl;
        return 1;
    }
    
    std::vector<ServerConfig> servers = leerConfig(argv[1]);
    if (servers.empty()) {
        return 1;
    }
    
    std::cout << "Configuración cargada:" << std::endl;
    std::cout << "  Número de servidores: " << servers.size() << std::endl << std::endl;
    
    Server servidor(servers[0]);
    servidor.iniciar();
    servidor.ejecutar();
    
    return 0;
}
