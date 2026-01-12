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

/*
    Generar HTML con listado de archivos Auto index
    Obtener lista de archivos, Ordenar alfabéticamente y Construir lista de archivos
    Creamos un for para listar los archivos y directorios
    despues construimos URL correcta evitando barras duplicadas y Eliminamos barra final de rutaURL si existe 
    Por último: Leer el template HTML ,Reemplazar placeholders ,Reemplazar {{PATH}} y Reemplazar {{FILE_LIST}}
*/
std::string Autoindex::generarHTML(const std::string& rutaDirectorio, const std::string& rutaURL)
{
    
    std::vector<std::string> archivos = listarDirectorio(rutaDirectorio);
    std::sort(archivos.begin(), archivos.end());
    std::ostringstream listaArchivos;

    if (rutaURL != "/") {
        listaArchivos << "        <li><a href=\"..\" class=\"folder\">📁 ../</a></li>\n";
    }
    
    for (size_t i = 0; i < archivos.size(); i++) {
        std::string nombre = archivos[i];
        bool esDir = (nombre[nombre.length() - 1] == '/');
        
        listaArchivos << "        <li><a href=\"";
        
        std::string urlLimpia = rutaURL;
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
    
    std::ifstream templateFile("templates/autoindex.html");
    std::string htmlTemplate;
    
    if (!templateFile.is_open()) {
        return "<html><body><h1>Error: No se pudo cargar el template de autoindex</h1></body></html>";
    }
    
    std::stringstream buffer;
    buffer << templateFile.rdbuf();
    htmlTemplate = buffer.str();
    templateFile.close();

    size_t pos;

    pos = htmlTemplate.find("{{PATH}}");
    if (pos != std::string::npos) {
        htmlTemplate.replace(pos, 8, rutaURL);
    }

    pos = htmlTemplate.find("{{FILE_LIST}}");
    if (pos != std::string::npos) {
        htmlTemplate.replace(pos, 13, listaArchivos.str());
    }
    
    return htmlTemplate;
}
