#include "Response.hpp"
#include <sys/stat.h>

// Constructor
Response::Response(const Request& request, const std::string& documentRoot,
                   const std::map<int, std::string>& errorPages,
                   const std::vector<Location>& locations,
                   size_t clientMaxBodySize)
    : _statusCode(200), _statusMessage("OK"), _documentRoot(documentRoot), 
      _errorPages(errorPages), _locations(locations), _clientMaxBodySize(clientMaxBodySize)
{
    try {
        procesar(request);
    }
    catch (const std::exception& e) {
        respuestaError(HttpStatus::INTERNAL_SERVER_ERROR);
    }
}

Response::~Response() {}

// Procesar la petición y generar respuesta
void Response::procesar(const Request& request)
{
    std::string method = request.getMethod();
    std::string path = request.getPath();
    
    // Validar que el método no esté vacío
    if (method.empty()) {
        respuestaError(HttpStatus::BAD_REQUEST);
        return;
    }
    
    // Validar tamaño del body solo para POST (único método implementado que acepta body)
    if (method == "POST") {
        std::string contentLengthStr = request.getHeader("Content-Length");
        if (!contentLengthStr.empty()) {
            size_t contentLength = atoi(contentLengthStr.c_str());
            if (contentLength > _clientMaxBodySize) {
                respuestaError(HttpStatus::PAYLOAD_TOO_LARGE);
                return;
            }
        }
    }
    
    // Verificar si hay redirección configurada para esta ruta
    Location* loc = obtenerLocation(path);
    if (loc != NULL && loc->has_redirect) {
        _statusCode = loc->redirect_code;
        _statusMessage = HttpStatus::getMessage(loc->redirect_code);
        _headers["Location"] = loc->redirect_url;
        _body = "";
        return;
    }
    
    // Validar que el método está permitido para esta ruta
    if (!metodoPermitido(path, method)) {
        respuestaError(HttpStatus::METHOD_NOT_ALLOWED);
        return;
    }
    
    if (method == "GET") {
        manejarGET(request);
    }
    else if (method == "POST") {
        manejarPOST(request);
    }
    else if (method == "DELETE") {
        manejarDELETE(request);
    }
    else if (method == "PUT" || method == "PATCH" || method == "HEAD" || 
             method == "OPTIONS" || method == "TRACE" || method == "CONNECT") {
        respuestaError(HttpStatus::NOT_IMPLEMENTED);
    }
    else {
        respuestaError(HttpStatus::BAD_REQUEST);
    }
}

// Convertir a string
std::string Response::toString() const
{
    std::ostringstream response;
    response << "HTTP/1.1 " << _statusCode << " " << _statusMessage << "\r\n";
    
    // Agregar Content-Length si no está presente
    if (_headers.find("Content-Length") == _headers.end() && !_body.empty()) {
        std::ostringstream length_stream;
        length_stream << _body.length();
        response << "Content-Length: " << length_stream.str() << "\r\n";
    }
    
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
         it != _headers.end(); ++it) {
        response << it->first << ": " << it->second << "\r\n";
    }
    
    response << "\r\n" << _body;
    return response.str();
}

// Leer archivo del sistema
std::string Response::leerArchivo(const std::string& ruta)
{
    std::ifstream file(ruta.c_str());
    if (!file.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    return buffer.str();
}

// Obtener Content-Type según extensión
std::string Response::obtenerContentType(const std::string& extension)
{
    if (extension == ".html" || extension == ".htm") return "text/html";
    if (extension == ".css") return "text/css";
    if (extension == ".js") return "application/javascript";
    if (extension == ".json") return "application/json";
    if (extension == ".png") return "image/png";
    if (extension == ".jpg" || extension == ".jpeg") return "image/jpeg";
    if (extension == ".gif") return "image/gif";
    if (extension == ".txt") return "text/plain";
    return "application/octet-stream";
}

// Verificar si una ruta es un directorio
bool Response::esDirectorio(const std::string& ruta)
{
    struct stat info;
    if (stat(ruta.c_str(), &info) != 0) {
        return false;
    }
    return S_ISDIR(info.st_mode);
}

// Verificar si la ruta tiene autoindex activado
bool Response::tieneAutoindex(const std::string& path)
{
    Location* loc = obtenerLocation(path);
    return (loc != NULL) ? loc->autoindex : false;
}

// Obtener la location que coincide con el path
Location* Response::obtenerLocation(const std::string& path)
{
    Location* mejor = NULL;
    size_t mayorLongitud = 0;
    
    for (size_t i = 0; i < _locations.size(); i++) {
        std::string locPath = _locations[i].path;
        if (path.find(locPath) == 0 && locPath.length() > mayorLongitud) {
            mayorLongitud = locPath.length();
            mejor = &_locations[i];
        }
    }
    return mejor;
}

// Verificar si el método está permitido para esta ruta
bool Response::metodoPermitido(const std::string& path, const std::string& method)
{
    Location* loc = obtenerLocation(path);
    if (loc == NULL || loc->allow_methods.empty()) {
        return true;
    }
    
    for (size_t i = 0; i < loc->allow_methods.size(); i++) {
        if (loc->allow_methods[i] == method) {
            return true;
        }
    }
    return false;
}

// Generar respuesta de error
void Response::respuestaError(int codigo)
{
    _statusCode = codigo;
    _statusMessage = HttpStatus::getMessage(codigo);
    
    std::map<int, std::string>::const_iterator it = _errorPages.find(codigo);
    if (it != _errorPages.end()) {
        // Si la ruta no empieza con '/', es relativa al documentRoot
        std::string rutaError;
        if (it->second[0] == '/') {
            // Ruta absoluta desde la raíz del proyecto
            rutaError = it->second.substr(1); // Quitar el '/' inicial
        } else {
            // Ruta relativa (puede estar en templates/ o html/)
            rutaError = it->second;
        }
        
        std::string contenido = leerArchivo(rutaError);
        
        if (!contenido.empty()) {
            _body = contenido;
            _headers["Content-Type"] = "text/html";
            return;
        }
    }
    
    std::ostringstream html;
    html << "<html><body><h1>" << codigo << " - " << _statusMessage << "</h1></body></html>";
    _body = html.str();
    _headers["Content-Type"] = "text/html";
}

// Normalizar path eliminando barras duplicadas
std::string Response::normalizarPath(const std::string& path)
{
    if (path.empty()) {
        return "/";
    }
    
    std::string resultado;
    resultado.reserve(path.length());
    
    bool ultimaFueBarra = false;
    for (size_t i = 0; i < path.length(); i++) {
        if (path[i] == '/') {
            if (!ultimaFueBarra) {
                resultado += '/';
                ultimaFueBarra = true;
            }
        } else {
            resultado += path[i];
            ultimaFueBarra = false;
        }
    }
    
    // Asegurar que empiece con /
    if (resultado.empty() || resultado[0] != '/') {
        resultado = "/" + resultado;
    }
    
    return resultado;
}

/*
    CGI (Common Gateway Interface):
    Permite ejecutar scripts externos (.php, .py, etc.) y devolver su salida como respuesta HTTP.
*/

// Verificar si un archivo es CGI basándose en su extensión
bool Response::esCGI(const std::string& path)
{
    size_t dotPos = path.find_last_of('.');
    if (dotPos == std::string::npos) {
        return false;
    }
    
    std::string extension = path.substr(dotPos);
    return (extension == ".php" || extension == ".py");
}

// Obtener la ruta del intérprete según la extensión
std::string Response::obtenerInterpreterPath(const std::string& extension)
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
std::string Response::ejecutarCGI(const std::string& scriptPath, const Request& request)
{
    // Obtener extensión y verificar intérprete
    size_t dotPos = scriptPath.find_last_of('.');
    if (dotPos == std::string::npos) {
        return "";
    }
    
    std::string extension = scriptPath.substr(dotPos);
    std::string interpreterPath = obtenerInterpreterPath(extension);
    
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
