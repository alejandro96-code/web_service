#ifndef HTTP_STATUS_HPP
#define HTTP_STATUS_HPP

#include <string>
#include <map>

class HttpStatus {
public:
    // Códigos de estado HTTP comunes
    static const int OK = 200;
    static const int CREATED = 201;
    static const int NO_CONTENT = 204;
    
    static const int MOVED_PERMANENTLY = 301;
    static const int FOUND = 302;
    static const int NOT_MODIFIED = 304;
    
    static const int BAD_REQUEST = 400;
    static const int FORBIDDEN = 403;
    static const int NOT_FOUND = 404;
    static const int METHOD_NOT_ALLOWED = 405;
    static const int PAYLOAD_TOO_LARGE = 413;
    
    static const int INTERNAL_SERVER_ERROR = 500;
    static const int NOT_IMPLEMENTED = 501;
    static const int HTTP_VERSION_NOT_SUPPORTED = 505;
    
    // Obtener mensaje de estado para un código
    static std::string getMessage(int code);
    
private:
    static std::map<int, std::string> initMessages();
    static std::map<int, std::string> _messages;
};

#endif
