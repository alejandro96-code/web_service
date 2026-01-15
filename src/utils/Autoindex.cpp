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
bool Autoindex::isDirectory(const std::string& path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
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
std::vector<std::string> Autoindex::listDirectory(const std::string& path)
{
    std::vector<std::string> files;
    DIR* dir = opendir(path.c_str());
    
    struct dirent* entry;
    if (dir == NULL)
        return files;
    
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;
        if (name == "." || name == "..") {
            continue;
        }
        
        std::string fullPath = path + "/" + name;
        if (isDirectory(fullPath)) {
            files.push_back(name + "/");
        } else {
            files.push_back(name);
        }
    }
    closedir(dir);
    return files;
}

/*
    Generar HTML con listado de archivos Auto index
    Obtener lista de archivos, Ordenar alfabéticamente y Construir lista de archivos
    Creamos un for para listar los archivos y directorios
    despues construimos URL correcta evitando barras duplicadas y Eliminamos barra final de rutaURL si existe 
    Por último: Leer el template HTML ,Reemplazar placeholders ,Reemplazar {{PATH}} y Reemplazar {{FILE_LIST}}
*/
std::string Autoindex::generateHTML(const std::string& directoryPath, const std::string& urlPath)
{
    
    std::vector<std::string> files = listDirectory(directoryPath);
    std::sort(files.begin(), files.end());
    std::ostringstream fileList;

    if (urlPath != "/") {
        fileList << "        <li><a href=\"..\" class=\"folder\">📁 ../</a></li>\n";
    }
    
    for (size_t i = 0; i < files.size(); i++) {
        std::string name = files[i];
        bool isDir = (name[name.length() - 1] == '/');
        
        fileList << "        <li><a href=\"";
        
        std::string cleanUrl = urlPath;
        if (cleanUrl.length() > 1 && cleanUrl[cleanUrl.length() - 1] == '/') {
            cleanUrl = cleanUrl.substr(0, cleanUrl.length() - 1);
        }
        
        if (cleanUrl == "/") {
            fileList << "/" << name;
        } else {
            fileList << cleanUrl << "/" << name;
        }
        
        fileList << "\" class=\"" << (isDir ? "folder" : "file") << "\">";
        fileList << (isDir ? "📁 " : "📄 ") << name << "</a></li>\n";
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
        htmlTemplate.replace(pos, 8, urlPath);
    }

    pos = htmlTemplate.find("{{FILE_LIST}}");
    if (pos != std::string::npos) {
        htmlTemplate.replace(pos, 13, fileList.str());
    }
    
    return htmlTemplate;
}
