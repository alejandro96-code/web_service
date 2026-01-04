<?php
header("Content-Type: text/html; charset=UTF-8");
?>
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <title>CGI PHP Test</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 800px;
            margin: 50px auto;
            padding: 20px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
        }
        .container {
            background: white;
            padding: 40px;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
        }
        h1 { color: #667eea; }
        .info { background: #e3f2fd; padding: 15px; border-radius: 10px; margin: 15px 0; }
        .success { color: #4caf50; font-weight: bold; }
        ul { line-height: 1.8; }
    </style>
</head>
<body>
    <div class="container">
        <h1>✅ ¡CGI PHP Funciona!</h1>
        <div class="info">
            <p class="success">Este contenido fue generado por un script PHP ejecutado con CGI.</p>
            <p><strong>Servidor:</strong> Webserv</p>
            <p><strong>Intérprete:</strong> PHP</p>
            <p><strong>Proceso:</strong> fork() + pipe() + execve()</p>
        </div>
        
        <h2>📊 Información de PHP</h2>
        <ul>
            <li><strong>PHP Version:</strong> <?php echo phpversion(); ?></li>
            <li><strong>Fecha y Hora:</strong> <?php echo date('Y-m-d H:i:s'); ?></li>
            <li><strong>SERVER_SOFTWARE:</strong> <?php echo $_SERVER['SERVER_SOFTWARE'] ?? 'Webserv'; ?></li>
        </ul>
        
        <h2>🔧 Variables de Entorno CGI</h2>
        <ul>
            <li><strong>REQUEST_METHOD:</strong> <?php echo $_ENV['REQUEST_METHOD'] ?? 'N/A'; ?></li>
            <li><strong>SCRIPT_FILENAME:</strong> <?php echo $_ENV['SCRIPT_FILENAME'] ?? 'N/A'; ?></li>
            <li><strong>GATEWAY_INTERFACE:</strong> <?php echo $_ENV['GATEWAY_INTERFACE'] ?? 'N/A'; ?></li>
            <li><strong>SERVER_PROTOCOL:</strong> <?php echo $_ENV['SERVER_PROTOCOL'] ?? 'N/A'; ?></li>
        </ul>
        
        <h2>🎯 Estado de CGI</h2>
        <p>Si puedes ver este contenido, tu servidor web soporta CGI con PHP correctamente.</p>
    </div>
</body>
</html>
