#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

#include "Location.hpp"
#include <string>
#include <vector>
#include <map>

class ServerConfig {
	private:
		int _port;
		std::string _server_name;
		std::string _root;
		std::string _index;
		std::map<int, std::string> _error_pages;
		std::vector<Location> _locations;
	public:
		ServerConfig();
		~ServerConfig();
		
    	int getPort() const;
    	const std::string& getServerName() const;
		const std::string& getRoot() const;
		const std::string& getIndex() const;
    	const std::map<int, std::string>& getErrorPages() const;
    	const std::vector<Location>& getLocations() const;


    	void setPort(int port);
    	void setServerName(const std::string& server_name);
		void setRoot(const std::string& root);
		void setIndex(const std::string& index);
    	void setErrorPages(const std::map<int, std::string>& error_pages);
    	void setLocations(const std::vector<Location>& locations);


    	//void addServerName(const std::string& server_name);
    	void addErrorPage(int code, const std::string& page);
    	void addLocation(const Location& location);	
};

#endif
