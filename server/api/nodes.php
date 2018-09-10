<?php
require_once __DIR__ . '/config.php';

$pdo = getDbConnection();
$stmt = $pdo->query("SELECT id, hardware_id, name, type, status, last_seen_at FROM nodes ORDER BY id ASC");
echo json_encode($stmt->fetchAll());
