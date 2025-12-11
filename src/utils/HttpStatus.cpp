#include "HttpStatus.hpp"

/*
    Codigos de estado:
    Son codigo de 3 digitos que indican el resultado de una solicitud al servidor
    Los codigo 20X indican que la solicitud a sido correcta
        200 solicitud exitosa
        201 solicitud bien creada
    Los codigos 400 indican un error del cliente
        400 solicitud mal creada
        403 solicitud hecha sin tener permisos
        404 archivo, ruta... no encontrada
        405 solicitud de un metodo no permitido
    Los codigos 500 indican error en el servidor
        500 error interno (la solicitud esta bien pero algo falla en el servidor)
        501 no implementado (el servidor no reconoce el metodo solicitado)

    Tambien existen los codigo 30x aunque aqui no los usamos (codigos de redireccion)
        que indican que el recurso requiere una accion adicional o se ha movido
*/

std::map<int, std::string> HttpStatus::_messages = HttpStatus::initMessages();

std::map<int, std::string> HttpStatus::initMessages()
{
    std::map<int, std::string> messages;

    messages[200] = "OK";
    messages[201] = "Created";
    messages[400] = "Bad Request";
    messages[403] = "Forbidden";
    messages[404] = "Not Found";
    messages[405] = "Method Not Allowed";
    messages[500] = "Internal Server Error";
    messages[501] = "Not Implemented";
    
    return messages;
}

std::string HttpStatus::getMessage(int code)
{
    std::map<int, std::string>::iterator it = _messages.find(code);
    if (it != _messages.end())
        return it->second;
    return "Unknown";
}
