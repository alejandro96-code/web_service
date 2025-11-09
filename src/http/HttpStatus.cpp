#include "HttpStatus.hpp"

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
    
    return messages;
}

std::string HttpStatus::getMessage(int code)
{
    std::map<int, std::string>::iterator it = _messages.find(code);
    if (it != _messages.end())
        return it->second;
    return "Unknown";
}
