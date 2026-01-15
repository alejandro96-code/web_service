#ifndef FILE_UTILS_HPP
#define FILE_UTILS_HPP

#include <string>

/*
    FileUtils: Utilidades para manejo de archivos del sistema
    
    Proporciona funciones auxiliares para:
    - Leer archivos del sistema de archivos
    - Verificar si una ruta es un directorio
    - Obtener el tipo MIME según la extensión
    - Normalizar rutas eliminando barras duplicadas
*/
class FileUtils {
public:
    // Leer el contenido completo de un archivo
    static std::string readFile(const std::string& path);
    
    // Verificar si una ruta corresponde a un directorio
    static bool isDirectory(const std::string& path);
    
    // Obtener el Content-Type MIME según la extensión del archivo
    static std::string getContentType(const std::string& extension);
    
    // Normalizar un path eliminando barras duplicadas y asegurando formato correcto
    static std::string normalizePath(const std::string& path);
};

#endif
