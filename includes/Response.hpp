#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "webserv.hpp"
#include "Request.hpp"
#include "parseoConf.hpp"
#include "Autoindex.hpp"
#include "HttpStatus.hpp"
#include "FileUtils.hpp"
#include "LocationMatcher.hpp"
#include "ErrorHandler.hpp"
#include "CGIHandler.hpp"

class Response {
protected:
    int _statusCode;
    std::string _statusMessage;
    std::map<std::string, std::string> _headers;
    std::string _body;
    std::string _documentRoot;
    std::string _index;
    std::map<int, std::string> _errorPages;
    std::vector<Location> _locations;
    size_t _clientMaxBodySize;

public:
    Response(const Request& request, const std::string& documentRoot, 
             const std::map<int, std::string>& errorPages,
             const std::vector<Location>& locations,
             size_t clientMaxBodySize,
             const std::string& index);
    virtual ~Response();
    
    void process(const Request& request);
    std::string toString() const;

protected:
    // Métodos virtuales para cada tipo de petición
    virtual void handleGET(const Request& request);
    virtual void handlePOST(const Request& request);
    virtual void handleDELETE(const Request& request);
};

#endif
