#include "../../includes/Autoindex.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include <algorithm>

// Verificar si una ruta es un directorio
bool Autoindex::esDirectorio(const std::string& ruta)
{
    struct stat info;
    if (stat(ruta.c_str(), &info) != 0) {
        return false;
    }
    return S_ISDIR(info.st_mode);
}

// Obtener lista de archivos y directorios
std::vector<std::string> Autoindex::listarDirectorio(const std::string& ruta)
{
    std::vector<std::string> archivos;
    DIR* dir = opendir(ruta.c_str());
    
    if (dir == NULL) {
        return archivos;
    }
    
    struct dirent* entrada;
    while ((entrada = readdir(dir)) != NULL) {
        std::string nombre = entrada->d_name;
        
        // Saltar "." (directorio actual)
        if (nombre == ".") {
            continue;
        }
        
        // Verificar si es un directorio
        std::string rutaCompleta = ruta + "/" + nombre;
        if (esDirectorio(rutaCompleta)) {
            archivos.push_back(nombre + "/");
        } else {
            archivos.push_back(nombre);
        }
    }
    
    closedir(dir);
    return archivos;
}

// Generar HTML con listado de archivos
std::string Autoindex::generarHTML(const std::string& rutaDirectorio, const std::string& rutaURL)
{
    // Obtener lista de archivos
    std::vector<std::string> archivos = listarDirectorio(rutaDirectorio);
    
    // Ordenar alfabéticamente
    std::sort(archivos.begin(), archivos.end());
    
    // Construir solo la lista de archivos
    std::ostringstream listaArchivos;
    
    // Si no estamos en la raíz, agregar enlace al directorio padre
    if (rutaURL != "/") {
        listaArchivos << "<li><a href=\"..\" class=\"folder\">📁 ../</a></li>\n";
    }
    
    // Listar archivos y directorios
    for (size_t i = 0; i < archivos.size(); i++) {
        std::string nombre = archivos[i];
        bool esDir = (nombre[nombre.length() - 1] == '/');
        
        listaArchivos << "<li><a href=\"";
        
        // Construir URL correcta
        if (rutaURL == "/") {
            listaArchivos << "/" << nombre;
        } else {
            listaArchivos << rutaURL << "/" << nombre;
        }
        
        listaArchivos << "\" class=\"" << (esDir ? "folder" : "file") << "\">";
        listaArchivos << (esDir ? "📁 " : "📄 ") << nombre << "</a></li>\n";
    }
    
    // HTML completo más simple
    std::ostringstream html;
    html << "<!DOCTYPE html>\n"
         << "<html><head><meta charset=\"UTF-8\"><title>Index of " << rutaURL << "</title>\n"
         << "<style>body{font-family:Arial;max-width:800px;margin:40px auto;padding:20px}"
         << "h1{color:#333;border-bottom:2px solid #4CAF50}ul{list-style:none;padding:0}"
         << "li{padding:8px;border-bottom:1px solid #eee}li:hover{background:#f5f5f5}"
         << "a{text-decoration:none;color:#2196F3}.folder{color:#FF9800;font-weight:600}</style>\n"
         << "</head><body><h1>Index of " << rutaURL << "</h1>\n"
         << "<ul>\n" << listaArchivos.str() << "</ul>\n"
         << "</body></html>\n";
    
    return html.str();
}
