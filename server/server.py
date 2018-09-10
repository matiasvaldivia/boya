#!/usr/bin/env python3
import http.server
import socketserver
import json
import sqlite3
import os
import sys
import time
from urllib.parse import urlparse, parse_qs

PORT = 8086
DB_PATH = os.path.join(os.path.dirname(__file__), "bqs_server.db")
WEB_DIR = os.path.join(os.path.dirname(__file__), "web")

def init_db():
    with sqlite3.connect(DB_PATH) as conn:
        cursor = conn.cursor()
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS nodes (
                id INTEGER PRIMARY KEY,
                hardware_id TEXT UNIQUE NOT NULL,
                name TEXT NOT NULL,
                type TEXT DEFAULT 'BUOY_OCEAN',
                status TEXT DEFAULT 'ONLINE',
                battery_min_mv INTEGER DEFAULT 3500,
                report_interval_sec INTEGER DEFAULT 60,
                location_name TEXT DEFAULT 'Miramar',
                last_seen_at INTEGER
            )
        """)
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS gateways (
                id INTEGER PRIMARY KEY,
                hardware_id TEXT UNIQUE NOT NULL,
                name TEXT NOT NULL,
                latitude REAL,
                longitude REAL,
                status TEXT DEFAULT 'ONLINE',
                last_seen_at INTEGER
            )
        """)
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS telemetry (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                node_id INTEGER NOT NULL,
                sequence INTEGER NOT NULL,
                timestamp INTEGER NOT NULL,
                latitude REAL NOT NULL,
                longitude REAL NOT NULL,
                temperature_air REAL,
                temperature_water REAL,
                humidity REAL,
                battery_mv INTEGER,
                battery_percent INTEGER,
                satellites INTEGER,
                hops INTEGER,
                rssi INTEGER,
                gateway_id TEXT,
                received_at INTEGER,
                UNIQUE(node_id, sequence)
            )
        """)
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS radio_logs (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                node_id INTEGER NOT NULL,
                log_type TEXT NOT NULL, -- 'RX', 'TX', 'RELAY', 'ACK', 'ALERT', 'STORE'
                message TEXT NOT NULL,
                hops INTEGER DEFAULT 0,
                rssi INTEGER DEFAULT -85,
                timestamp INTEGER NOT NULL
            )
        """)
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS alerts (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                node_id INTEGER,
                type TEXT,
                severity TEXT,
                message TEXT,
                created_at INTEGER
            )
        """)

        # Nodos iniciales BQS Miramar
        cursor.execute("SELECT count(*) FROM nodes")
        if cursor.fetchone()[0] == 0:
            now = int(time.time())
            cursor.execute("INSERT INTO nodes (id, hardware_id, name, type, status, battery_min_mv, report_interval_sec, location_name, last_seen_at) VALUES (17, 'BQS-NODE-17', 'Boya Costera 17 - Miramar', 'BUOY_OCEAN', 'ONLINE', 3500, 60, 'Sector Náutico / Rompiente', ?)", (now,))
            cursor.execute("INSERT INTO nodes (id, hardware_id, name, type, status, battery_min_mv, report_interval_sec, location_name, last_seen_at) VALUES (12, 'BQS-NODE-12', 'Boya Intermedia 12 - Punta Hermengo', 'BUOY_OCEAN', 'ONLINE', 3500, 60, 'Punta Hermengo / Termoclina', ?)", (now,))
            cursor.execute("INSERT INTO nodes (id, hardware_id, name, type, status, battery_min_mv, report_interval_sec, location_name, last_seen_at) VALUES (4, 'BQS-NODE-04', 'Boya Relay 04 - Escollera', 'BUOY_COASTAL', 'ONLINE', 3400, 30, 'Escollera Norte Miramar', ?)", (now,))
            cursor.execute("INSERT INTO nodes (id, hardware_id, name, type, status, battery_min_mv, report_interval_sec, location_name, last_seen_at) VALUES (8, 'BQS-NODE-08', 'Boya Oceánica 08 - Mar del Plata', 'BUOY_OCEAN', 'ONLINE', 3500, 120, 'Mar Abierto / Deriva', ?)", (now,))

            cursor.execute("INSERT INTO gateways (id, hardware_id, name, latitude, longitude, status, last_seen_at) VALUES (1, 'BQS-GW-01', 'Estacion Costera Miramar', -38.2728, -57.8306, 'ONLINE', ?)", (now,))
            cursor.execute("INSERT INTO gateways (id, hardware_id, name, latitude, longitude, status, last_seen_at) VALUES (2, 'BQS-GW-02', 'Estacion Faro Mar del Plata', -38.0858, -57.5412, 'ONLINE', ?)", (now,))

            # Telemetría de muestra
            cursor.execute("""
                INSERT INTO telemetry (node_id, sequence, timestamp, latitude, longitude, temperature_air, temperature_water, humidity, battery_mv, battery_percent, satellites, hops, rssi, gateway_id, received_at)
                VALUES (17, 8421, ?, -38.2850, -57.8420, 21.7, 18.9, 73.4, 3860, 78, 8, 2, -86, 'BQS-GW-01', ?)
            """, (now, now))
            cursor.execute("""
                INSERT INTO telemetry (node_id, sequence, timestamp, latitude, longitude, temperature_air, temperature_water, humidity, battery_mv, battery_percent, satellites, hops, rssi, gateway_id, received_at)
                VALUES (12, 5120, ?, -38.2780, -57.8350, 21.4, 18.7, 74.0, 3920, 85, 9, 1, -82, 'BQS-GW-01', ?)
            """, (now, now))
            cursor.execute("""
                INSERT INTO telemetry (node_id, sequence, timestamp, latitude, longitude, temperature_air, temperature_water, humidity, battery_mv, battery_percent, satellites, hops, rssi, gateway_id, received_at)
                VALUES (4, 9102, ?, -38.2740, -57.8320, 22.0, 19.1, 71.0, 4100, 95, 10, 0, -75, 'BQS-GW-01', ?)
            """, (now, now))
            cursor.execute("""
                INSERT INTO telemetry (node_id, sequence, timestamp, latitude, longitude, temperature_air, temperature_water, humidity, battery_mv, battery_percent, satellites, hops, rssi, gateway_id, received_at)
                VALUES (8, 3340, ?, -38.1020, -57.5210, 20.8, 17.9, 76.2, 3780, 68, 8, 1, -91, 'BQS-GW-02', ?)
            """, (now, now))

            # Logs de radio iniciales
            cursor.execute("INSERT INTO radio_logs (node_id, log_type, message, hops, rssi, timestamp) VALUES (17, 'RX', 'Paquete binario recibido via multi-hop (Boya 12 -> Boya 4 -> GW-01)', 2, -86, ?)", (now - 120,))
            cursor.execute("INSERT INTO radio_logs (node_id, log_type, message, hops, rssi, timestamp) VALUES (17, 'ACK', 'ACK confirmado por Gateway costero Miramar', 2, -86, ?)", (now - 118,))
            cursor.execute("INSERT INTO radio_logs (node_id, log_type, message, hops, rssi, timestamp) VALUES (12, 'RELAY', 'Reenvio de trama desde Boya 17 hacia Boya 4', 1, -82, ?)", (now - 119,))
            cursor.execute("INSERT INTO radio_logs (node_id, log_type, message, hops, rssi, timestamp) VALUES (4, 'RELAY', 'Reenvio directo hacia Gateway 01 (Escollera)', 0, -75, ?)", (now - 119,))
        conn.commit()

class BqsHttpHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=WEB_DIR, **kwargs)

    def do_GET(self):
        parsed = urlparse(self.path)
        path = parsed.path

        if path == "/api/v1/nodes":
            self.send_json_response(self.get_nodes())
        elif path.startswith("/api/v1/nodes/") and path.endswith("/logs"):
            node_id = int(path.split("/")[4])
            self.send_json_response(self.get_node_logs(node_id))
        elif path.startswith("/api/v1/nodes/") and path.endswith("/firmware-config"):
            node_id = int(path.split("/")[4])
            self.send_json_response(self.get_firmware_config(node_id))
        elif path.startswith("/api/v1/nodes/") and path.endswith("/telemetry"):
            node_id = int(path.split("/")[4])
            self.send_json_response(self.get_node_telemetry(node_id))
        elif path.startswith("/api/v1/nodes/") and path.endswith("/track"):
            node_id = int(path.split("/")[4])
            self.send_json_response(self.get_node_track(node_id))
        elif path == "/api/v1/gateways":
            self.send_json_response(self.get_gateways())
        elif path == "/api/v1/telemetry":
            self.send_json_response(self.get_latest_telemetry())
        elif path == "/api/v1/alerts":
            self.send_json_response(self.get_alerts())
        else:
            super().do_GET()

    def do_POST(self):
        parsed = urlparse(self.path)
        path = parsed.path
        length = int(self.headers.get('Content-Length', 0))
        body = self.rfile.read(length).decode('utf-8') if length > 0 else "{}"
        try:
            data = json.loads(body)
        except Exception:
            data = {}

        if path == "/api/v1/auth/login":
            user = data.get("username")
            pwd = data.get("password")
            if user == "admin" and pwd == "admin":
                self.send_json_response({"status": "OK", "token": "bqs_session_token_admin_2018", "user": "admin", "role": "OPERATOR"})
            else:
                self.send_json_response({"status": "ERROR", "message": "Credenciales inválidas"}, status=401)
        elif path == "/api/v1/telemetry":
            res = self.save_telemetry(data)
            self.send_json_response(res, status=201 if res.get("status") == "OK" else 400)
        elif path == "/api/v1/gateways/heartbeat":
            res = self.save_gateway_heartbeat(data)
            self.send_json_response(res)
        elif path == "/api/v1/nodes":
            res = self.create_node(data)
            self.send_json_response(res, status=201 if res.get("status") == "OK" else 400)
        elif path.startswith("/api/v1/nodes/") and path.endswith("/flash-simulate"):
            node_id = int(path.split("/")[4])
            res = self.simulate_flash(node_id)
            self.send_json_response(res)
        else:
            self.send_response(404)
            self.end_headers()

    def do_PUT(self):
        parsed = urlparse(self.path)
        path = parsed.path
        length = int(self.headers.get('Content-Length', 0))
        body = self.rfile.read(length).decode('utf-8') if length > 0 else "{}"
        try:
            data = json.loads(body)
        except Exception:
            data = {}

        if path.startswith("/api/v1/nodes/"):
            node_id = int(path.split("/")[4])
            res = self.update_node(node_id, data)
            self.send_json_response(res)
        else:
            self.send_response(404)
            self.end_headers()

    def do_DELETE(self):
        parsed = urlparse(self.path)
        path = parsed.path
        if path.startswith("/api/v1/nodes/"):
            node_id = int(path.split("/")[4])
            res = self.delete_node(node_id)
            self.send_json_response(res)
        else:
            self.send_response(404)
            self.end_headers()

    def send_json_response(self, data, status=200):
        body = json.dumps(data).encode('utf-8')
        self.send_response(status)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, PUT, DELETE, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type, Authorization, X-Gateway-ID, X-API-Key')
        self.send_header('Content-Length', str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, PUT, DELETE, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type, Authorization, X-Gateway-ID, X-API-Key')
        self.end_headers()

    def get_nodes(self):
        with sqlite3.connect(DB_PATH) as conn:
            cursor = conn.cursor()
            cursor.execute("SELECT id, hardware_id, name, type, status, battery_min_mv, report_interval_sec, location_name, last_seen_at FROM nodes ORDER BY id ASC")
            rows = cursor.fetchall()
            return [{
                "id": r[0], "hardware_id": r[1], "name": r[2], "type": r[3],
                "status": r[4], "battery_min_mv": r[5], "report_interval_sec": r[6],
                "location_name": r[7], "last_seen_at": r[8]
            } for r in rows]

    def create_node(self, data):
        node_id = int(data.get("id", 0))
        name = data.get("name", f"Boya #{node_id}")
        hw_id = data.get("hardware_id", f"BQS-NODE-{node_id:02d}")
        b_type = data.get("type", "BUOY_OCEAN")
        b_min = int(data.get("battery_min_mv", 3500))
        interval = int(data.get("report_interval_sec", 60))
        loc = data.get("location_name", "Miramar")
        now = int(time.time())

        try:
            with sqlite3.connect(DB_PATH) as conn:
                cursor = conn.cursor()
                cursor.execute("""
                    INSERT INTO nodes (id, hardware_id, name, type, status, battery_min_mv, report_interval_sec, location_name, last_seen_at)
                    VALUES (?, ?, ?, ?, 'ONLINE', ?, ?, ?, ?)
                """, (node_id, hw_id, name, b_type, b_min, interval, loc, now))
                cursor.execute("INSERT INTO radio_logs (node_id, log_type, message, timestamp) VALUES (?, 'SYS', 'Boya dada de alta en el sistema BQS Miramar', ?)", (node_id, now))
                conn.commit()
            return {"status": "OK", "message": f"Boya #{node_id} creada exitosamente"}
        except Exception as e:
            return {"status": "ERROR", "message": str(e)}

    def update_node(self, node_id, data):
        name = data.get("name")
        b_type = data.get("type")
        b_min = data.get("battery_min_mv")
        interval = data.get("report_interval_sec")
        loc = data.get("location_name")
        status = data.get("status")

        try:
            with sqlite3.connect(DB_PATH) as conn:
                cursor = conn.cursor()
                cursor.execute("""
                    UPDATE nodes
                    SET name = coalesce(?, name),
                        type = coalesce(?, type),
                        battery_min_mv = coalesce(?, battery_min_mv),
                        report_interval_sec = coalesce(?, report_interval_sec),
                        location_name = coalesce(?, location_name),
                        status = coalesce(?, status)
                    WHERE id = ?
                """, (name, b_type, b_min, interval, loc, status, node_id))
                cursor.execute("INSERT INTO radio_logs (node_id, log_type, message, timestamp) VALUES (?, 'SYS', 'Parametros de la boya actualizados', ?)", (node_id, int(time.time())))
                conn.commit()
            return {"status": "OK", "message": f"Boya #{node_id} actualizada"}
        except Exception as e:
            return {"status": "ERROR", "message": str(e)}

    def delete_node(self, node_id):
        try:
            with sqlite3.connect(DB_PATH) as conn:
                cursor = conn.cursor()
                cursor.execute("DELETE FROM nodes WHERE id = ?", (node_id,))
                cursor.execute("DELETE FROM telemetry WHERE node_id = ?", (node_id,))
                cursor.execute("DELETE FROM radio_logs WHERE node_id = ?", (node_id,))
                conn.commit()
            return {"status": "OK", "message": f"Boya #{node_id} dada de baja"}
        except Exception as e:
            return {"status": "ERROR", "message": str(e)}

    def get_node_logs(self, node_id):
        with sqlite3.connect(DB_PATH) as conn:
            cursor = conn.cursor()
            cursor.execute("SELECT id, node_id, log_type, message, hops, rssi, timestamp FROM radio_logs WHERE node_id = ? ORDER BY id DESC LIMIT 50", (node_id,))
            rows = cursor.fetchall()
            return [{"id": r[0], "node_id": r[1], "log_type": r[2], "message": r[3], "hops": r[4], "rssi": r[5], "timestamp": r[6]} for r in rows]

    def get_firmware_config(self, node_id):
        return {
            "node_id": node_id,
            "defines": {
                "BQS_NODE_ID": node_id,
                "BQS_NETWORK_ID": "0x42",
                "LORA_FREQUENCY": "915000000.0",
                "LORA_TX_POWER": "20",
                "MAX_HOPS": "6"
            },
            "avrdude_cmd": f"avrdude -v -p atmega328p -c arduino -P /dev/ttyUSB0 -b 115200 -D -U flash:w:buoy_node_{node_id}.hex:i",
            "arduino_cli_cmd": f"arduino-cli compile --fqbn arduino:avr:pro:cpu=16MHzatmega328 --build-property build.extra_flags=\"-DBQS_NODE_ID={node_id}\" firmware/buoy-node && arduino-cli upload -p /dev/ttyUSB0 --fqbn arduino:avr:pro:cpu=16MHzatmega328 firmware/buoy-node"
        }

    def simulate_flash(self, node_id):
        time.sleep(0.5)
        now = int(time.time())
        with sqlite3.connect(DB_PATH) as conn:
            cursor = conn.cursor()
            cursor.execute("INSERT INTO radio_logs (node_id, log_type, message, timestamp) VALUES (?, 'FLASH', 'Firmware flasheado y verificado OK (ATmega328P / 915 MHz)', ?)", (node_id, now))
            conn.commit()
        return {
            "status": "OK",
            "message": f"Firmware grabado exitosamente en Boya #{node_id} (ATmega328P / RFM95W 915 MHz)",
            "node_id": node_id,
            "flash_time_sec": 4.2
        }

    def get_gateways(self):
        with sqlite3.connect(DB_PATH) as conn:
            cursor = conn.cursor()
            cursor.execute("SELECT id, hardware_id, name, latitude, longitude, status, last_seen_at FROM gateways")
            rows = cursor.fetchall()
            return [{"id": r[0], "hardware_id": r[1], "name": r[2], "latitude": r[3], "longitude": r[4], "status": r[5], "last_seen_at": r[6]} for r in rows]

    def get_latest_telemetry(self):
        with sqlite3.connect(DB_PATH) as conn:
            cursor = conn.cursor()
            cursor.execute("""
                SELECT t.node_id, n.name, t.sequence, t.timestamp, t.latitude, t.longitude,
                       t.temperature_air, t.temperature_water, t.humidity, t.battery_mv,
                       t.battery_percent, t.satellites, t.hops, t.rssi, t.gateway_id, t.received_at
                FROM telemetry t
                JOIN nodes n ON t.node_id = n.id
                WHERE t.id IN (SELECT MAX(id) FROM telemetry GROUP BY node_id)
            """)
            rows = cursor.fetchall()
            return [{
                "node_id": r[0], "node_name": r[1], "sequence": r[2], "timestamp": r[3],
                "latitude": r[4], "longitude": r[5], "temp_air": r[6], "temp_water": r[7],
                "humidity": r[8], "battery_mv": r[9], "battery_percent": r[10],
                "satellites": r[11], "hops": r[12], "rssi": r[13], "gateway_id": r[14], "received_at": r[15]
            } for r in rows]

    def get_node_telemetry(self, node_id):
        with sqlite3.connect(DB_PATH) as conn:
            cursor = conn.cursor()
            cursor.execute("""
                SELECT sequence, timestamp, latitude, longitude, temperature_air, temperature_water,
                       humidity, battery_mv, battery_percent, satellites, hops, rssi, gateway_id, received_at
                FROM telemetry WHERE node_id = ? ORDER BY id DESC LIMIT 50
            """, (node_id,))
            rows = cursor.fetchall()
            return [{
                "sequence": r[0], "timestamp": r[1], "latitude": r[2], "longitude": r[3],
                "temp_air": r[4], "temp_water": r[5], "humidity": r[6], "battery_mv": r[7],
                "battery_percent": r[8], "satellites": r[9], "hops": r[10], "rssi": r[11],
                "gateway_id": r[12], "received_at": r[13]
            } for r in rows]

    def get_node_track(self, node_id):
        with sqlite3.connect(DB_PATH) as conn:
            cursor = conn.cursor()
            cursor.execute("SELECT latitude, longitude, timestamp FROM telemetry WHERE node_id = ? ORDER BY id ASC LIMIT 200", (node_id,))
            rows = cursor.fetchall()
            return [{"lat": r[0], "lon": r[1], "timestamp": r[2]} for r in rows]

    def get_alerts(self):
        with sqlite3.connect(DB_PATH) as conn:
            cursor = conn.cursor()
            cursor.execute("SELECT id, node_id, type, severity, message, created_at FROM alerts ORDER BY id DESC LIMIT 20")
            rows = cursor.fetchall()
            return [{"id": r[0], "node_id": r[1], "type": r[2], "severity": r[3], "message": r[4], "created_at": r[5]} for r in rows]

    def save_telemetry(self, data):
        pkt = data.get("packet", data)
        gw_id = data.get("gateway_id", "BQS-GW-01")
        node_id = pkt.get("node_id")
        seq = pkt.get("sequence")
        ts = pkt.get("timestamp", int(time.time()))
        lat = pkt.get("latitude")
        lon = pkt.get("longitude")
        t_air = pkt.get("air_temperature")
        t_water = pkt.get("water_temperature")
        hum = pkt.get("humidity")
        batt_mv = pkt.get("battery_mv")
        batt_pct = pkt.get("battery_percent")
        sat = pkt.get("satellites", 0)
        hops = pkt.get("hops", 0)
        rssi = pkt.get("rssi", -85)
        now = int(time.time())

        try:
            with sqlite3.connect(DB_PATH) as conn:
                cursor = conn.cursor()
                cursor.execute("""
                    INSERT OR IGNORE INTO telemetry (node_id, sequence, timestamp, latitude, longitude,
                        temperature_air, temperature_water, humidity, battery_mv, battery_percent,
                        satellites, hops, rssi, gateway_id, received_at)
                    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
                """, (node_id, seq, ts, lat, lon, t_air, t_water, hum, batt_mv, batt_pct, sat, hops, rssi, gw_id, now))
                cursor.execute("UPDATE nodes SET last_seen_at = ?, status = 'ONLINE' WHERE id = ?", (now, node_id))
                cursor.execute("INSERT INTO radio_logs (node_id, log_type, message, hops, rssi, timestamp) VALUES (?, 'RX', ?, ?, ?, ?)",
                               (node_id, f"Muestra #{seq} recibida (Agua: {t_water}°C, Bateria: {batt_pct}%)", hops, rssi, now))
                conn.commit()
            return {"status": "OK", "message": "Telemetry saved"}
        except Exception as e:
            return {"status": "ERROR", "message": str(e)}

    def save_gateway_heartbeat(self, data):
        gw_id = data.get("gateway_id", "BQS-GW-01")
        now = int(time.time())
        with sqlite3.connect(DB_PATH) as conn:
            cursor = conn.cursor()
            cursor.execute("UPDATE gateways SET last_seen_at = ?, status = 'ONLINE' WHERE hardware_id = ?", (now, gw_id))
            conn.commit()
        return {"status": "OK", "message": "Heartbeat updated"}

def run_server():
    init_db()
    os.makedirs(WEB_DIR, exist_ok=True)
    with socketserver.TCPServer(("", PORT), BqsHttpHandler) as httpd:
        print(f"[*] BQS Server 2018 ejecutandose en http://localhost:{PORT}")
        httpd.serve_forever()

if __name__ == "__main__":
    run_server()
