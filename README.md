# Webserv

*This project has been created as part of the 42 curriculum by alejanr2.*

## Description

**Webserv** is a non-blocking HTTP/1.1 web server implementation written in C++98. This project demonstrates fundamental concepts of network programming, including socket management, I/O multiplexing with `select()`, HTTP protocol handling, and CGI support.

The server is capable of:
- Handling multiple simultaneous client connections without blocking
- Serving static files (HTML, CSS, JavaScript, images, etc.)
- Processing HTTP methods: GET, POST, DELETE
- Executing CGI scripts (PHP, Python)
- Managing multiple virtual servers with different configurations
- Handling chunked transfer encoding
- Directory listing (autoindex)
- Custom error pages
- File uploads and deletions
- Configurable request body size limits

### Key Features

- **Non-blocking I/O**: Uses `select()` for efficient concurrent connection handling
- **NGINX-style Configuration**: Flexible configuration file format supporting multiple servers and locations
- **CGI Gateway**: Execute dynamic scripts with proper environment variable handling
- **Virtual Servers**: Support for multiple server blocks with different ports and configurations
- **Chunked Encoding**: Proper handling of Transfer-Encoding: chunked requests
- **Custom Error Pages**: Configurable error page templates for different HTTP status codes
- **Request Size Control**: Configurable maximum body size with 413 Payload Too Large response
- **Path Routing**: Location-based routing with method restrictions and redirections

## Instructions

### Prerequisites

- C++ compiler with C++98 support (g++, clang++)
- Make
- PHP-CGI (optional, for CGI support)
- Python3 (optional, for CGI support)

### Compilation

```bash
make
```

This will compile the project and generate the `webserv` executable.

To clean object files:
```bash
make clean
```

To remove all compiled files:
```bash
make fclean
```

To recompile from scratch:
```bash
make re
```

### Configuration

The server uses NGINX-style configuration files. A basic example is provided in `conf/server.conf`:

```nginx
server {
    listen 8080;
    server_name localhost;
    root html;
    index index.html;
    client_max_body_size 10M;
    
    error_page 404 /templates/error404.html;
    error_page 500 /templates/error500.html;
    
    location / {
        allowed_methods GET POST DELETE;
    }
    
    location /autoindex/ {
        autoindex on;
        allowed_methods GET POST DELETE;
    }
}
```

Configuration options:
- `listen`: Port number to listen on
- `server_name`: Server identifier (localhost, domain name, etc.)
- `root`: Document root directory
- `index`: Default index file
- `client_max_body_size`: Maximum request body size
- `error_page`: Custom error page paths
- `location`: Path-specific configurations
- `allowed_methods`: HTTP methods allowed for a location
- `autoindex`: Enable/disable directory listing
- `redirect`: URL redirection with status code

### Execution

Start the server with a configuration file:

```bash
./webserv conf/server.conf
```

The server will start and display:
```
Número de servidores: 1
✓ Servidor escuchando en puerto 8080
```

### Testing

#### Basic Testing with cURL

```bash
# GET request
curl http://localhost:8080/

# GET with headers
curl -I http://localhost:8080/

# POST file upload
curl -X POST -F "file=@test.txt" http://localhost:8080/autoindex/

# DELETE file
curl -X DELETE http://localhost:8080/autoindex/file.txt

# Chunked transfer encoding
curl -X POST -H "Transfer-Encoding: chunked" --data-binary @file.txt http://localhost:8080/autoindex/
```

#### Load Testing with Siege

Install Siege (if not installed):
```bash
# From source (no sudo required)
cd ~
wget http://download.joedog.org/siege/siege-latest.tar.gz
tar -xvzf siege-latest.tar.gz
cd siege-*
./configure --prefix=$HOME/siege
make && make install
echo 'export PATH=$HOME/siege/bin:$PATH' >> ~/.zshrc
source ~/.zshrc
```

Run load tests:
```bash
# Light load: 10 concurrent users, 5 requests each
~/siege/bin/siege -c 10 -r 5 http://localhost:8080/

# Medium load: 50 users for 30 seconds
~/siege/bin/siege -c 50 -t 30s http://localhost:8080/

# Heavy load: 100 concurrent users, 100 requests each (benchmark mode)
~/siege/bin/siege -b -c 100 -r 100 http://localhost:8080/

# Stress test: 200 users for 3 minutes
~/siege/bin/siege -b -c 200 -t 3M http://localhost:8080/
```

#### Browser Testing

Open your browser and navigate to:
- `http://localhost:8080/` - Main page
- `http://localhost:8080/autoindex/` - Directory listing
- `http://localhost:8080/cgi.html` - CGI test page
- `http://localhost:8080/teapot` - Easter egg (418 I'm a teapot)

### CGI Support

The server supports CGI scripts for PHP and Python. Make sure the interpreters are installed:

```bash
# Check PHP-CGI
which php-cgi

# Check Python3
which python3
```

Example CGI scripts are provided in the `html/` directory:
- `test.php` - PHP CGI example
- `test.py` - Python CGI example
- `simple.py` - Simple Python script

### Project Structure

```
web_service/
├── main.cpp                      # Entry point, select() loop
├── Makefile                      # Build configuration
├── webserv                       # Compiled executable
├── README.md                     # Project documentation
├── urls.txt                      # URL list for siege testing
├── .gitignore                    # Git ignore file
│
├── conf/                         # Configuration files
│   ├── server.conf               # Single server config
│   ├── multi_server.conf         # Multiple servers config
│   └── empty.conf                # Minimal config
│
├── html/                         # Web content directory
│   ├── index.html                # Main page
│   ├── cgi.html                  # CGI test page
│   ├── chunked.html              # Chunked encoding test
│   ├── delete_file.html          # File deletion interface
│   ├── formupload.html           # File upload form
│   ├── test.php                  # PHP CGI script
│   ├── test.py                   # Python CGI script
│   ├── simple.py                 # Simple Python script
│   └── autoindex/                # Directory with autoindex enabled
│       ├── archivosSubidos/      # Upload destination folder
│       ├── carpeta1/             # Test directory 1
│       └── carpeta2/             # Test directory 2
│
├── templates/                    # Error page templates
│   ├── autoindex.html            # Directory listing template
│   ├── error400.html             # Bad Request
│   ├── error403.html             # Forbidden
│   ├── error404.html             # Not Found
│   ├── error405.html             # Method Not Allowed
│   ├── error413.html             # Payload Too Large
│   ├── error418.html             # I'm a teapot (Easter egg)
│   ├── error500.html             # Internal Server Error
│   ├── error501.html             # Not Implemented
│   ├── gooddelete.html           # Successful deletion page
│   └── goodpost.html             # Successful upload page
│
├── includes/                     # Header files
│   ├── webserv.hpp               # Common includes and definitions
│   ├── server.hpp                # Server class declaration
│   ├── Request.hpp               # HTTP request parser
│   ├── Response.hpp              # HTTP response handler (base class)
│   ├── parseoConf.hpp            # Configuration parser
│   ├── CGIHandler.hpp            # CGI execution handler
│   ├── FileUtils.hpp             # File operations utilities
│   ├── ErrorHandler.hpp          # Error page generator
│   ├── LocationMatcher.hpp       # Route matching utilities
│   ├── HttpStatus.hpp            # HTTP status codes
│   └── Autoindex.hpp             # Directory listing generator
│
├── src/                          # Source files
│   ├── core/                     # Core server logic
│   │   └── server.cpp            # Server implementation
│   │
│   ├── http/                     # HTTP request/response handling
│   │   ├── Request.cpp           # Request parsing
│   │   ├── Response.cpp          # Base response class
│   │   ├── ResponseGet.cpp       # GET method handler
│   │   ├── ResponsePost.cpp      # POST method handler
│   │   └── ResponseDelete.cpp    # DELETE method handler
│   │
│   ├── config/                   # Configuration parsing
│   │   └── parseoConf.cpp        # Config file parser
│   │
│   ├── cgi/                      # CGI handling
│   │   └── CGIHandler.cpp        # CGI execution and environment setup
│   │
│   └── utils/                    # Utility functions
│       ├── FileUtils.cpp         # File operations
│       ├── ErrorHandler.cpp      # Error page handling
│       ├── LocationMatcher.cpp   # Location matching logic
│       ├── HttpStatus.cpp        # HTTP status utilities
│       └── Autoindex.cpp         # Directory listing generation
│
├── obj/                          # Compiled object files (generated)
│   └── src/
│       ├── core/
│       ├── http/
│       ├── config/
│       ├── cgi/
│       └── utils/
│
└── infoProyecto/                 # Project information
    ├── curl.text                 # cURL examples
    └── infoconf.conf             # Configuration notes
```

## Resources

### HTTP Protocol & Web Servers
- [RFC 2616 - HTTP/1.1](https://www.rfc-editor.org/rfc/rfc2616) - Official HTTP/1.1 specification
- [RFC 3875 - CGI](https://www.rfc-editor.org/rfc/rfc3875) - Common Gateway Interface specification
- [MDN HTTP Documentation](https://developer.mozilla.org/en-US/docs/Web/HTTP) - Comprehensive HTTP reference
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) - Socket programming tutorial
- [NGINX Documentation](https://nginx.org/en/docs/) - Configuration reference and concepts

### Technical Documentation
- Linux `man` pages: `socket(2)`, `select(2)`, `recv(2)`, `send(2)`, `bind(2)`, `listen(2)`, `accept(2)`
- [fcntl() - Non-blocking I/O](https://man7.org/linux/man-pages/man2/fcntl.2.html)
- [TCP/IP Illustrated](https://www.pearson.com/en-us/subject-catalog/p/tcp-ip-illustrated-volume-1-the-protocols/P200000009556) - In-depth networking concepts

### Tools & Testing
- [cURL Documentation](https://curl.se/docs/) - Command-line HTTP client
- [Siege Manual](https://www.joedog.org/siege-manual/) - Load testing tool
- [Postman](https://www.postman.com/) - API testing platform
- [Wireshark](https://www.wireshark.org/) - Network protocol analyzer

### AI Usage

AI (GitHub Copilot) was used in this project for:

1. **Code Translation**: Translating Spanish variable and function names to English while preserving Spanish comments for documentation purposes.

2. **Documentation**: Generating comprehensive inline comments explaining complex logic, particularly in:
   - HTTP request parsing and chunked encoding handling
   - CGI environment variable setup
   - Configuration file parsing
   - Error handling strategies

3. **Debugging Assistance**: Identifying and resolving issues related to:
   - Socket buffer management
   - Memory leaks in dynamic allocations
   - Edge cases in HTTP parsing
   - Select() timeout handling

4. **Code Review**: Suggesting improvements for:
   - Error handling consistency
   - Resource cleanup in exception paths
   - Configuration validation
   - Response header formatting

5. **Testing Strategy**: Designing test scenarios for:
   - Concurrent connection handling
   - Large file uploads
   - CGI script execution
   - Edge cases (empty requests, malformed headers, etc.)

**Note**: All core logic, architecture decisions, and implementation were human-designed. AI was used as a productivity tool for code quality improvements, documentation, and identifying potential issues.

## Technical Choices

### Non-blocking I/O with select()
- Chosen over threading for better resource efficiency
- Allows handling hundreds of concurrent connections with a single process
- Avoids complexity of thread synchronization

### C++98 Standard
- Project requirement for compatibility
- Demonstrates understanding of pre-C++11 features
- No use of STL algorithms, smart pointers, or modern features

### NGINX-style Configuration
- Industry-standard format that's familiar to web developers
- Flexible and readable
- Supports complex routing scenarios

### CGI over FastCGI
- Simpler implementation suitable for educational purposes
- Demonstrates process creation and inter-process communication
- Easier to debug and understand

### Custom HTTP Parser
- Full control over parsing logic
- Better error handling and edge case management
- Educational value in understanding the HTTP protocol

## Known Limitations

- HTTP/1.1 only (no HTTP/2 or HTTP/3 support)
- No HTTPS/SSL support
- No keep-alive connection support
- Basic CGI only (no FastCGI or application server integration)
- Limited to `select()` for I/O multiplexing (no epoll/kqueue)
- Maximum file descriptors limited by `select()` (typically 1024)

## Authors

- **alejanr2** - [GitHub Profile](https://github.com/alejandro96-code)

## License

This project is part of the 42 School curriculum and follows the school's academic policies.
