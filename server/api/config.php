<?php
// =============================================================================
// BQS BUOY NETWORK - Configuración de Base de Datos y API (2018)
// PHP 7.2 / PDO
// =============================================================================

header("Content-Type: application/json; charset=UTF-8");
header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Methods: GET, POST, OPTIONS");
header("Access-Control-Allow-Headers: Content-Type, Access-Control-Allow-Headers, Authorization, X-Requested-With, X-Gateway-ID, X-API-Key");

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

define('DB_HOST', 'localhost');
define('DB_NAME', 'bqs_buoy_network');
define('DB_USER', 'bqs_user');
define('DB_PASS', 'bqs_secret_2018');
define('BQS_API_KEY', 'bqs_secret_key_2018');

function getDbConnection() {
    $dbPath = __DIR__ . '/../bqs_server.db';
    try {
        // Soporte dual: SQLite local o MySQL según disponibilidad
        if (file_exists($dbPath) || !extension_loaded('pdo_mysql')) {
            $pdo = new PDO("sqlite:" . $dbPath);
            $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
            $pdo->setAttribute(PDO::ATTR_DEFAULT_FETCH_MODE, PDO::FETCH_ASSOC);
            return $pdo;
        } else {
            $dsn = "mysql:host=" . DB_HOST . ";dbname=" . DB_NAME . ";charset=utf8mb4";
            $pdo = new PDO($dsn, DB_USER, DB_PASS);
            $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
            $pdo->setAttribute(PDO::ATTR_DEFAULT_FETCH_MODE, PDO::FETCH_ASSOC);
            return $pdo;
        }
    } catch (PDOException $e) {
        http_response_code(500);
        echo json_encode(["error" => "Error de conexion a base de datos", "details" => $e->getMessage()]);
        exit;
    }
}
