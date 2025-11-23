#ifndef LOCATION_HPP
# define LOCATION_HPP

#include <string>
#include <vector>
#include <map>

class Location{
	private:
		std::string _path;
		std::vector<std::string> _allow_methods;
		bool _autoindex;
		//std::vector<std::string> _index_files;
		std::string _index;
	public:
		Location();
		~Location();

		const std::string& getPath() const;
		const std::vector<std::string>& getMethods() const;
		bool getAutoindex() const;
		const std::vector<std::string>& getIndexFiles() const;
		
};

#endif
