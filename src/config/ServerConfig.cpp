#include "ServerConfig.hpp"
#include <iostream>

ServerConfig::ServerConfig(): _port(80)
{}

ServerConfig::~ServerConfig() {}


int ServerConfig::getPort() const {
    return _port;
}

const std::string& ServerConfig::getServerName() const {
    return _server_name;
}

const std::string& ServerConfig::getRoot() const {
	return _root;
}

const std::string& ServerConfig::getIndex() const {
	return _index;
}

const std::map<int, std::string>& ServerConfig::getErrorPages() const {
    return _error_pages;
}

const std::vector<Location>& ServerConfig::getLocations() const {
    return _locations;
}

void ServerConfig::setPort(int port) {
    _port = port;
}

void ServerConfig::setServerName(const std::string& server_name) {
    _server_name = server_name;
}

void ServerConfig::setRoot(const std::string& root) {
	_root = root;
}

void ServerConfig::setIndex(const std::string& index) {
	_index = index;
}

void ServerConfig::setErrorPages(const std::map<int, std::string>& error_pages) {
    _error_pages = error_pages;
}

void ServerConfig::setLocations(const std::vector<Location>& locations) {
    _locations = locations;
}

/*void ServerConfig::addServerName(const std::string& server_name) {
    _server_names.push_back(server_name);
}*/

void ServerConfig::addErrorPage(int code, const std::string& page) {
    _error_pages[code] = page;
}

void ServerConfig::addLocation(const Location& location) {
    _locations.push_back(location);
}
