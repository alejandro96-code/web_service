#!/usr/bin/env python3
# -*- coding: utf-8 -*-

print("<!DOCTYPE html>")
print("<html lang='es'>")
print("<head>")
print("    <meta charset='UTF-8'>")
print("    <title>CGI Python Test</title>")
print("    <style>")
print("        body {")
print("            font-family: Arial, sans-serif;")
print("            max-width: 800px;")
print("            margin: 50px auto;")
print("            padding: 20px;")
print("            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);")
print("        }")
print("        .container {")
print("            background: white;")
print("            padding: 40px;")
print("            border-radius: 20px;")
print("            box-shadow: 0 20px 60px rgba(0,0,0,0.3);")
print("        }")
print("        h1 { color: #667eea; }")
print("        .info { background: #e3f2fd; padding: 15px; border-radius: 10px; margin: 15px 0; }")
print("        .success { color: #4caf50; font-weight: bold; }")
print("    </style>")
print("</head>")
print("<body>")
print("    <div class='container'>")
print("        <h1>✅ ¡CGI Python Funciona!</h1>")
print("        <div class='info'>")
print("            <p class='success'>Este contenido fue generado por un script Python ejecutado con CGI.</p>")
print("            <p><strong>Servidor:</strong> Webserv</p>")
print("            <p><strong>Intérprete:</strong> Python 3</p>")
print("            <p><strong>Proceso:</strong> fork() + pipe() + execve()</p>")
print("        </div>")
print("        <h2>📊 Información del Sistema</h2>")
print("        <ul>")

import sys
import os
from datetime import datetime

print(f"            <li><strong>Python Version:</strong> {sys.version}</li>")
print(f"            <li><strong>Fecha y Hora:</strong> {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</li>")
print(f"            <li><strong>Path del Script:</strong> {os.environ.get('SCRIPT_FILENAME', 'N/A')}</li>")
print(f"            <li><strong>Método HTTP:</strong> {os.environ.get('REQUEST_METHOD', 'N/A')}</li>")

print("        </ul>")
print("        <h2>🔧 Variables de Entorno CGI</h2>")
print("        <ul>")

cgi_vars = ['REQUEST_METHOD', 'SCRIPT_FILENAME', 'QUERY_STRING', 'CONTENT_TYPE', 
            'CONTENT_LENGTH', 'SERVER_PROTOCOL', 'GATEWAY_INTERFACE']

for var in cgi_vars:
    value = os.environ.get(var, 'No definida')
    print(f"            <li><strong>{var}:</strong> {value}</li>")

print("        </ul>")
print("        <h2>🎯 Prueba de Funcionalidad</h2>")
print("        <p>Si puedes ver este contenido, tu servidor web:</p>")
print("        <ul>")
print("            <li>✅ Detecta archivos .py como CGI</li>")
print("            <li>✅ Ejecuta fork() correctamente</li>")
print("            <li>✅ Crea pipes para comunicación</li>")
print("            <li>✅ Usa execve() para ejecutar Python</li>")
print("            <li>✅ Captura y devuelve la salida</li>")
print("        </ul>")
print("    </div>")
print("</body>")
print("</html>")
