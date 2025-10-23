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
    std::map<std::string, std::string> config = leerConfig(argv[1]);
    
    // Verificar que se haya cargado la configuración correctamente
    if (config.empty()) {
        std::cerr << "Error: No se pudo cargar la configuración o el archivo está vacío" << std::endl;
        return 1;
    }
    
    // Crear e iniciar el servidor
    Server servidor(config);
    servidor.iniciar();
    servidor.ejecutar();
    
    return 0;
}
