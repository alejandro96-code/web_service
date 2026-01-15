#include "CGIHandler.hpp"
#include <sys/stat.h>

/*
    CGIHandler: Implementación de la gestión de scripts CGI
    
    Esta clase maneja la ejecución de scripts CGI (.php, .py, etc.)
    mediante fork + pipe + execve, configurando las variables de entorno
    necesarias según el estándar CGI/1.1
*/

// Verificar si un archivo es CGI basándose en su extensión
bool CGIHandler::isCGI(const std::string& path)
{
    size_t dotPos = path.find_last_of('.');
    if (dotPos == std::string::npos) {
        return false;
    }
    
    std::string extension = path.substr(dotPos);
    return (extension == ".php" || extension == ".py");
}

// Obtener la ruta del intérprete según la extensión
std::string CGIHandler::getInterpreterPath(const std::string& extension)
{
    if (extension == ".php") {
        // Intentar múltiples rutas posibles para PHP
        const char* phpPaths[] = {
            "/usr/bin/php-cgi",
            "/usr/bin/php",
            "/usr/local/bin/php-cgi",
            "/usr/local/bin/php",
            NULL
        };
        
        for (int i = 0; phpPaths[i] != NULL; i++) {
            struct stat buffer;
            if (stat(phpPaths[i], &buffer) == 0) {
                return phpPaths[i];
            }
        }
        
        // Si no encuentra PHP, devolver vacío
        return "";
    }
    if (extension == ".py") {
        return "/usr/bin/python3";
    }
    return "";
}

// Ejecutar script CGI usando fork + pipe + execve
std::string CGIHandler::executeCGI(const std::string& scriptPath, const Request& request)
{
    // Obtener extensión y verificar intérprete
    size_t dotPos = scriptPath.find_last_of('.');
    if (dotPos == std::string::npos) {
        return "";
    }
    
    std::string extension = scriptPath.substr(dotPos);
    std::string interpreterPath = getInterpreterPath(extension);
    
    if (interpreterPath.empty()) {
        std::cerr << "Intérprete no encontrado para " << extension << std::endl;
        return "";
    }
    
    // Verificar que el intérprete existe
    struct stat statBuffer;
    if (stat(interpreterPath.c_str(), &statBuffer) != 0) {
        std::cerr << "No se puede acceder al intérprete: " << interpreterPath << std::endl;
        return "";
    }
    
    // Crear pipes para comunicación
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        std::cerr << "Error al crear pipe" << std::endl;
        return "";
    }
    
    pid_t pid = fork();
    
    if (pid == -1) {
        // Error al hacer fork
        std::cerr << "Error al hacer fork" << std::endl;
        close(pipefd[0]);
        close(pipefd[1]);
        return "";
    }
    
    if (pid == 0) {
        // PROCESO HIJO: ejecutar el script CGI
        
        // Redirigir stdout al pipe
        close(pipefd[0]); // Cerrar lectura
        dup2(pipefd[1], STDOUT_FILENO); // stdout -> pipe escritura
        close(pipefd[1]);
        
        // Preparar variables de entorno CGI
        setenv("REQUEST_METHOD", request.getMethod().c_str(), 1);
        setenv("SCRIPT_FILENAME", scriptPath.c_str(), 1);
        setenv("QUERY_STRING", "", 1);
        setenv("CONTENT_TYPE", request.getHeader("Content-Type").c_str(), 1);
        setenv("CONTENT_LENGTH", request.getHeader("Content-Length").c_str(), 1);
        setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
        setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
        setenv("REDIRECT_STATUS", "200", 1);  // Requerido por php-cgi
        
        // Preparar argumentos para execve
        char* argv[] = {
            const_cast<char*>(interpreterPath.c_str()),
            const_cast<char*>(scriptPath.c_str()),
            NULL
        };
        
        // Ejecutar el intérprete
        execve(interpreterPath.c_str(), argv, environ);
        
        // Si llegamos aquí, execve falló
        std::cerr << "Error al ejecutar " << interpreterPath << std::endl;
        exit(1);
    }
    
    // PROCESO PADRE: leer la salida del script
    close(pipefd[1]); // Cerrar escritura
    
    std::string output;
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    
    // Leer toda la salida del pipe
    while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';
        output += buffer;
    }
    
    close(pipefd[0]);
    
    // Esperar a que termine el proceso hijo
    int status;
    waitpid(pid, &status, 0);
    
    // Verificar si el script terminó correctamente
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return output;
    }
    
    std::cerr << "El script CGI terminó con error" << std::endl;
    return "";
}
