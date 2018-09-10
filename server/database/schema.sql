-- =============================================================================
-- BQS BUOY NETWORK - Esquema Relacional de Base de Datos (2018)
-- MySQL 5.7 / MariaDB 10.3
-- =============================================================================

CREATE TABLE IF NOT EXISTS clients (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(128) NOT NULL,
    api_key VARCHAR(64) UNIQUE NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    client_id INT REFERENCES clients(id) ON DELETE SET NULL,
    username VARCHAR(64) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    role VARCHAR(32) DEFAULT 'OPERATOR',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS nodes (
    id INT AUTO_INCREMENT PRIMARY KEY,
    hardware_id VARCHAR(64) UNIQUE NOT NULL, -- ej. 'BQS-NODE-17'
    name VARCHAR(128) NOT NULL,
    type VARCHAR(32) DEFAULT 'BUOY_OCEAN',   -- 'BUOY_OCEAN', 'BUOY_COASTAL', 'BUOY_LAKE'
    status VARCHAR(32) DEFAULT 'ONLINE',     -- 'ONLINE', 'WARNING', 'OFFLINE'
    battery_min_mv INT DEFAULT 3500,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    last_seen_at TIMESTAMP NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS gateways (
    id INT AUTO_INCREMENT PRIMARY KEY,
    hardware_id VARCHAR(64) UNIQUE NOT NULL, -- ej. 'BQS-GW-01'
    name VARCHAR(128) NOT NULL,
    latitude DOUBLE PRECISION,
    longitude DOUBLE PRECISION,
    altitude DOUBLE PRECISION,
    status VARCHAR(32) DEFAULT 'ONLINE',
    ip_address VARCHAR(45),
    last_seen_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS telemetry (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    node_id INT NOT NULL,
    sequence INT UNSIGNED NOT NULL,
    timestamp BIGINT NOT NULL,
    
    -- Posicionamiento Geodésico
    latitude DOUBLE PRECISION NOT NULL,
    longitude DOUBLE PRECISION NOT NULL,
    altitude DOUBLE PRECISION DEFAULT 0.0,
    
    -- Variables Meteorológicas y Oceanográficas
    temperature_air FLOAT,
    temperature_water FLOAT,
    humidity FLOAT,
    
    -- Energía
    battery_mv INT UNSIGNED,
    battery_percent INT UNSIGNED,
    
    -- Calidad Satelital
    satellites INT UNSIGNED DEFAULT 0,
    hdop FLOAT DEFAULT 1.0,
    
    -- Metadatos de Radio y Malla
    rssi INT DEFAULT -85,
    snr FLOAT DEFAULT 0.0,
    hop_count INT UNSIGNED DEFAULT 0,
    gateway_id VARCHAR(64),
    
    received_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    
    -- Restricción estricta de unicidad para evitar duplicados en multi-gateway
    UNIQUE KEY unq_node_sequence (node_id, sequence),
    INDEX idx_node_timestamp (node_id, timestamp)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS routes (
    id INT AUTO_INCREMENT PRIMARY KEY,
    source_node INT NOT NULL,
    gateway_id VARCHAR(64) NOT NULL,
    hop_count INT UNSIGNED DEFAULT 1,
    rssi INT DEFAULT -85,
    last_seen_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    UNIQUE KEY unq_source_gw (source_node, gateway_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS alerts (
    id INT AUTO_INCREMENT PRIMARY KEY,
    node_id INT REFERENCES nodes(id) ON DELETE CASCADE,
    type VARCHAR(64) NOT NULL, -- 'LOW_BATTERY', 'NO_GPS', 'HIGH_TEMPERATURE', 'SENSOR_ERROR'
    severity VARCHAR(32) DEFAULT 'WARNING', -- 'INFO', 'WARNING', 'CRITICAL'
    message TEXT NOT NULL,
    resolved BOOLEAN DEFAULT FALSE,
    resolved_at TIMESTAMP NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS gateway_events (
    id INT AUTO_INCREMENT PRIMARY KEY,
    gateway_id VARCHAR(64) NOT NULL,
    event_type VARCHAR(64) NOT NULL,
    details TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
