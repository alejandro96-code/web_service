#include "HttpStatus.hpp"

std::map<int, std::string> HttpStatus::_messages = HttpStatus::initMessages();

std::map<int, std::string> HttpStatus::initMessages() {
    std::map<int, std::string> messages;
    
    // 2xx Success
    messages[200] = "OK";
    messages[201] = "Created";
    messages[204] = "No Content";
    
    // 3xx Redirection
    messages[301] = "Moved Permanently";
    messages[302] = "Found";
    messages[304] = "Not Modified";
    
    // 4xx Client Error
    messages[400] = "Bad Request";
    messages[403] = "Forbidden";
    messages[404] = "Not Found";
    messages[405] = "Method Not Allowed";
    messages[413] = "Payload Too Large";
    
    // 5xx Server Error
    messages[500] = "Internal Server Error";
    messages[501] = "Not Implemented";
    messages[505] = "HTTP Version Not Supported";
    
    return messages;
}

std::string HttpStatus::getMessage(int code) {
    std::map<int, std::string>::iterator it = _messages.find(code);
    if (it != _messages.end()) {
        return it->second;
    }
    return "Unknown";
}
