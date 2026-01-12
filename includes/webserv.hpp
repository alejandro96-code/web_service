#ifndef WEBSERV_HPP
#define WEBSERV_HPP


// LIBRERÍAS DE C
#include <cstdlib>      // atoi
#include <cstring>      // memset


// LIBRERÍAS ESTÁNDAR DE C++
#include <string>
#include <map>
#include <vector>
#include <set>
#include <iostream>
#include <sstream>
#include <fstream>

// LIBRERÍAS DE SISTEMA (Sockets)
#include <sys/socket.h> // socket, bind, listen, accept, send, recv
#include <netinet/in.h> // struct sockaddr_in, htons
#include <unistd.h>     // close
#include <sys/wait.h>   // waitpid para CGI

// CONSTANTES
#define BUFFER_SIZE 4096

// MENSAJES DE ERROR
#define ERROR_NO_CARGAR "Error: No se puede cargar el sevidor"
#define FORMA_DE_USO "Uso: ./webservice [archivo de configuracion]"
#define NUMERO_DE_SERVIDORES "Número de servidores: "
#define NOT_FILE_DESCRIPTOR "No hay file descriptors válidos"
#define ERROR_SELECT "Error en select"
#define ERROR_NON_BLOCKING "Error configurando non-blocking en cliente"
#define ERROR_RESPONSE "Error al enviar respuesta"
#define ERROR_NOT_DATOS "No se pudo enviar datos"

#endif
