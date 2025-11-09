#ifndef HTTP_STATUS_HPP
#define HTTP_STATUS_HPP

#include <string>
#include <map>

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
        500 error interno (la solicitud esta bien pero algo falla para realizarla)

    Tambien existen los codigo 30x aunque aqui no los usamos (codigos de redireccion)
        que indican que el recurso requiere una accion adicional o se ha movido
*/

class HttpStatus
{
    public:
        static const int OK = 200;
        static const int CREATED = 201;
        static const int BAD_REQUEST = 400;
        static const int FORBIDDEN = 403;
        static const int NOT_FOUND = 404;
        static const int METHOD_NOT_ALLOWED = 405;
        static const int INTERNAL_SERVER_ERROR = 500;
        static std::string getMessage(int code);
    private:
        static std::map<int, std::string> initMessages();
        static std::map<int, std::string> _messages;
};

#endif
