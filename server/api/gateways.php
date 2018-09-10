<?php
require_once __DIR__ . '/config.php';

$pdo = getDbConnection();
$method = $_SERVER['REQUEST_METHOD'];

if ($method === 'GET') {
    $stmt = $pdo->query("SELECT id, hardware_id, name, latitude, longitude, status, last_seen_at FROM gateways ORDER BY id ASC");
    echo json_encode($stmt->fetchAll());
} elseif ($method === 'POST') {
    $rawInput = file_get_contents("php://input");
    $data = json_decode($rawInput, true);

    $gwId = $data['gateway_id'] ?? 'BQS-GW-01';
    $now = time();

    $stmt = $pdo->prepare("UPDATE gateways SET last_seen_at = ?, status = 'ONLINE' WHERE hardware_id = ?");
    $stmt->execute([$now, $gwId]);

    echo json_encode(["status" => "OK", "message" => "Heartbeat actualizado"]);
}
