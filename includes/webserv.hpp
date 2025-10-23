#ifndef WEBSERV_HPP
#define WEBSERV_HPP


// LIBRERÍAS DE C
#include <cstdlib>      // atoi
#include <cstring>      // memset


// LIBRERÍAS ESTÁNDAR DE C++
#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include <fstream>

// LIBRERÍAS DE SISTEMA (Sockets)
#include <sys/socket.h> // socket, bind, listen, accept, send, recv
#include <netinet/in.h> // struct sockaddr_in, htons
#include <unistd.h>     // close

// CONSTANTES
#define BUFFER_SIZE 4096

#endif
