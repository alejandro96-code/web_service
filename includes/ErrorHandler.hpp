#ifndef ERROR_HANDLER_HPP
#define ERROR_HANDLER_HPP

#include <string>
#include <map>

/*
    ErrorHandler: Gestión de respuestas de error HTTP
    
    Responsable de generar páginas de error personalizadas
    o páginas de error por defecto según el código de estado HTTP.
    
    Soporta:
    - Carga de páginas de error personalizadas desde templates
    - Generación de páginas de error HTML genéricas
    - Manejo de errores comunes (400, 403, 404, 405, 413, 500, 501)
*/
class ErrorHandler {
public:
    // Estructura para almacenar la respuesta de error generada
    struct ErrorResponse {
        std::string body;           // Contenido HTML de la página de error
        std::string contentType;    // Tipo de contenido (siempre "text/html")
    };
    
    // Generar una respuesta de error completa
    static ErrorResponse generateErrorResponse(
        int code,
        const std::string& statusMessage,
        const std::map<int, std::string>& errorPages
    );
    
private:
    // Cargar página de error personalizada desde archivo
    static std::string loadErrorPage(const std::string& filePath);
    
    // Generar página de error HTML genérica
    static std::string generateGenericError(int code, const std::string& statusMessage);
};

#endif
