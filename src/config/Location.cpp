#include "Location.hpp"
#include <iostream>

Location::Location(): _autoindex(false) {}

Location::~Location() {}

const std::string& Location::getPath() const {
	return _path;
}

const std::vector<std::string>& Location::getMethods() const{
	return _methods;
}

bool Location::getAutoindex() const {
	return _autoindex;
}
