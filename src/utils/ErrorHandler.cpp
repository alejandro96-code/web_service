#include "ErrorHandler.hpp"
#include "FileUtils.hpp"
#include <sstream>

/*
    Generar respuesta de error HTTP:
    1. Intenta cargar una página de error personalizada desde errorPages
    2. Si existe el archivo, lo carga y retorna
    3. Si no existe o falla, genera una página de error HTML genérica
    
    La página personalizada se busca en la ruta especificada:
    - Si la ruta empieza con '/', se considera absoluta desde la raíz del proyecto
    - Si no, es relativa (templates/ o html/)
*/
ErrorHandler::ErrorResponse ErrorHandler::generateErrorResponse(
    int code,
    const std::string& statusMessage,
    const std::map<int, std::string>& errorPages)
{
    ErrorResponse response;
    response.contentType = "text/html";
    
    // Buscar página de error personalizada
    std::map<int, std::string>::const_iterator it = errorPages.find(code);
    if (it != errorPages.end()) {
        // Si la ruta no empieza con '/', es relativa al documentRoot
        std::string errorPath;
        if (it->second[0] == '/') {
            // Ruta absoluta desde la raíz del proyecto
            errorPath = it->second.substr(1); // Quitar el '/' inicial
        } else {
            // Ruta relativa (puede estar en templates/ o html/)
            errorPath = it->second;
        }
        
        std::string content = loadErrorPage(errorPath);
        
        if (!content.empty()) {
            response.body = content;
            return response;
        }
    }
    
    // Si no hay página personalizada o falló la carga, generar error genérico
    response.body = generateGenericError(code, statusMessage);
    return response;
}

/*
    Cargar página de error personalizada:
    Lee el contenido de un archivo de template de error.
    Usa FileUtils para la lectura del archivo.
    Retorna string vacío si el archivo no existe o no se puede leer.
*/
std::string ErrorHandler::loadErrorPage(const std::string& filePath)
{
    return FileUtils::readFile(filePath);
}

/*
    Generar página de error HTML genérica:
    Crea un HTML simple mostrando el código de error y mensaje.
    Se usa como fallback cuando no hay página personalizada.
    
    Formato:
    <html><body><h1>404 - Not Found</h1></body></html>
*/
std::string ErrorHandler::generateGenericError(int code, const std::string& statusMessage)
{
    std::ostringstream html;
    html << "<html><body><h1>" << code << " - " << statusMessage << "</h1></body></html>";
    return html.str();
}
