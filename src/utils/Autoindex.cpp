#include "../../includes/Autoindex.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>

/*
    Verificar si una ruta es un directorio
    creamos una estructura stat y hacemos una llamada al sistema
    si nos devuelve un valor diferente a 0 significa que ha habido un error
    de lo contrario  usamos S_ISDIR (macro del stat) con la info de la llamada
    que nos devolvera true.
    Asi distinguimos entre archivo y directorios.
*/
bool Autoindex::esDirectorio(const std::string& ruta)
{
    struct stat info;
    if (stat(ruta.c_str(), &info) != 0) {
        return false;
    }
    return S_ISDIR(info.st_mode);
}

/*
    aqui lo que queremos es listar los Directorios.
    Creamos un contenedor vector para guardar los archivos
    tambien uan variable dir que usaremos pra abrir el directorio devolviendo un puntero,
    y una estructura dirent*
    Si no existe el directorio nos devuelve los archivos solo.
    Hacemos un bucle iterado donde si no es NULL leemos el siguiente directorio y archivos
    saltamos el directorio actual y el padre para que no se dupliquen
    y si es un directorio añadimos una "/" y si no lo mostramos tal cual
    cerramos el directorio y lo retornamos
*/
std::vector<std::string> Autoindex::listarDirectorio(const std::string& ruta)
{
    std::vector<std::string> archivos;
    DIR* dir = opendir(ruta.c_str());
    
    struct dirent* entrada;
    if (dir == NULL)
        return archivos;
    
    while ((entrada = readdir(dir)) != NULL)
    {
        std::string nombre = entrada->d_name;
        if (nombre == "." || nombre == "..") {
            continue;
        }
        
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
    
    // Construir lista de archivos
    std::ostringstream listaArchivos;
    
    // Si no estamos en la raíz, agregar enlace al directorio padre
    if (rutaURL != "/") {
        listaArchivos << "        <li><a href=\"..\" class=\"folder\">📁 ../</a></li>\n";
    }
    
    // Listar archivos y directorios
    for (size_t i = 0; i < archivos.size(); i++) {
        std::string nombre = archivos[i];
        bool esDir = (nombre[nombre.length() - 1] == '/');
        
        listaArchivos << "        <li><a href=\"";
        
        // Construir URL correcta evitando barras duplicadas
        std::string urlLimpia = rutaURL;
        // Eliminar barra final de rutaURL si existe
        if (urlLimpia.length() > 1 && urlLimpia[urlLimpia.length() - 1] == '/') {
            urlLimpia = urlLimpia.substr(0, urlLimpia.length() - 1);
        }
        
        if (urlLimpia == "/") {
            listaArchivos << "/" << nombre;
        } else {
            listaArchivos << urlLimpia << "/" << nombre;
        }
        
        listaArchivos << "\" class=\"" << (esDir ? "folder" : "file") << "\">";
        listaArchivos << (esDir ? "📁 " : "📄 ") << nombre << "</a></li>\n";
    }
    
    // Leer el template HTML
    std::ifstream templateFile("templates/autoindex.html");
    std::string htmlTemplate;
    
    if (!templateFile.is_open()) {
        return "<html><body><h1>Error: No se pudo cargar el template de autoindex</h1></body></html>";
    }
    
    std::stringstream buffer;
    buffer << templateFile.rdbuf();
    htmlTemplate = buffer.str();
    templateFile.close();
    
    // Reemplazar placeholders
    size_t pos;
    
    // Reemplazar {{TITLE}}
    pos = htmlTemplate.find("{{TITLE}}");
    if (pos != std::string::npos) {
        htmlTemplate.replace(pos, 9, "Index of " + rutaURL);
    }
    
    // Reemplazar {{PATH}}
    pos = htmlTemplate.find("{{PATH}}");
    if (pos != std::string::npos) {
        htmlTemplate.replace(pos, 8, rutaURL);
    }
    
    // Reemplazar {{FILE_LIST}}
    pos = htmlTemplate.find("{{FILE_LIST}}");
    if (pos != std::string::npos) {
        htmlTemplate.replace(pos, 13, listaArchivos.str());
    }
    
    return htmlTemplate;
}
