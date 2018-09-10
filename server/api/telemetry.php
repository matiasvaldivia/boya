<?php
require_once __DIR__ . '/config.php';

$pdo = getDbConnection();
$method = $_SERVER['REQUEST_METHOD'];

if ($method === 'GET') {
    // Consulta de última telemetría por nodo
    $nodeId = isset($_GET['node_id']) ? (int)$_GET['node_id'] : null;

    if ($nodeId) {
        $stmt = $pdo->prepare("
            SELECT sequence, timestamp, latitude, longitude, temperature_air, temperature_water,
                   humidity, battery_mv, battery_percent, satellites, hops, rssi, gateway_id, received_at
            FROM telemetry
            WHERE node_id = ?
            ORDER BY id DESC
            LIMIT 50
        ");
        $stmt->execute([$nodeId]);
        echo json_encode($stmt->fetchAll());
    } else {
        $stmt = $pdo->query("
            SELECT t.node_id, n.name AS node_name, t.sequence, t.timestamp, t.latitude, t.longitude,
                   t.temperature_air AS temp_air, t.temperature_water AS temp_water, t.humidity,
                   t.battery_mv, t.battery_percent, t.satellites, t.hops, t.rssi, t.gateway_id, t.received_at
            FROM telemetry t
            JOIN nodes n ON t.node_id = n.id
            WHERE t.id IN (SELECT MAX(id) FROM telemetry GROUP BY node_id)
        ");
        echo json_encode($stmt->fetchAll());
    }
} elseif ($method === 'POST') {
    // Ingesta de paquetes desde Gateways costeros
    $rawInput = file_get_contents("php://input");
    $data = json_decode($rawInput, true);

    if (!$data) {
        http_response_code(400);
        echo json_encode(["status" => "ERROR", "message" => "JSON invalido"]);
        exit;
    }

    $gwId = $data['gateway_id'] ?? 'BQS-GW-01';
    $pkt = $data['packet'] ?? $data;

    $nodeId = (int)($pkt['node_id'] ?? 0);
    $seq = (int)($pkt['sequence'] ?? 0);
    $ts = (int)($pkt['timestamp'] ?? time());
    $lat = (float)($pkt['latitude'] ?? 0.0);
    $lon = (float)($pkt['longitude'] ?? 0.0);
    $tAir = isset($pkt['air_temperature']) ? (float)$pkt['air_temperature'] : null;
    $tWater = isset($pkt['water_temperature']) ? (float)$pkt['water_temperature'] : null;
    $hum = isset($pkt['humidity']) ? (float)$pkt['humidity'] : null;
    $battMv = (int)($pkt['battery_mv'] ?? 0);
    $battPct = (int)($pkt['battery_percent'] ?? 0);
    $sat = (int)($pkt['satellites'] ?? 0);
    $hops = (int)($pkt['hops'] ?? 0);
    $rssi = (int)($pkt['rssi'] ?? -85);
    $now = time();

    try {
        $stmt = $pdo->prepare("
            INSERT OR IGNORE INTO telemetry
            (node_id, sequence, timestamp, latitude, longitude, temperature_air, temperature_water,
             humidity, battery_mv, battery_percent, satellites, hops, rssi, gateway_id, received_at)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        ");
        $stmt->execute([$nodeId, $seq, $ts, $lat, lon, $tAir, $tWater, $hum, $battMv, $battPct, $sat, $hops, $rssi, $gwId, $now]);

        $updateNode = $pdo->prepare("UPDATE nodes SET last_seen_at = ?, status = 'ONLINE' WHERE id = ?");
        $updateNode->execute([$now, $nodeId]);

        // Verificación de Alarma de Batería Baja (< 3500 mV)
        if ($battMv > 0 && $battMv < 3500) {
            $stmtAlert = $pdo->prepare("INSERT INTO alerts (node_id, type, severity, message, created_at) VALUES (?, 'LOW_BATTERY', 'WARNING', ?, ?)");
            $stmtAlert->execute([$nodeId, "Bateria critica en Boya #{$nodeId}: {$battMv} mV ({$battPct}%)", $now]);
        }

        http_response_code(201);
        echo json_encode(["status" => "OK", "message" => "Telemetria registrada correctamente"]);
    } catch (PDOException $e) {
        http_response_code(500);
        echo json_encode(["status" => "ERROR", "message" => $e->getMessage()]);
    }
}
