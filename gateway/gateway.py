#!/usr/bin/env python3
import configparser
import time
import os
import sys
import threading
from bqs_gateway.packet_decoder import BqsPacketDecoder
from bqs_gateway.local_spool import LocalSpoolQueue
from bqs_gateway.uploader import TelemetryUploader
from bqs_gateway.beacon_broadcaster import BeaconBroadcaster

def main():
    print("==================================================")
    print(" BQS LoRa Gateway - Estacion Receptora (2018)     ")
    print("==================================================")

    config_path = os.path.join(os.path.dirname(__file__), "config.ini")
    config = configparser.ConfigParser()
    if os.path.exists(config_path):
        config.read(config_path)
    else:
        print("[ERROR] Archivo config.ini no encontrado.")
        return

    gw_id = config.get("gateway", "id", fallback="BQS-GW-01")
    api_url = config.get("server", "api_url", fallback="http://localhost:8080/api/v1")
    api_key = config.get("server", "api_key", fallback="bqs_secret_2018")
    db_path = config.get("spool", "db_path", fallback="spool.db")
    hb_interval = config.getint("server", "heartbeat_interval_sec", fallback=60)

    gw_lat = config.getfloat("gateway", "latitude", fallback=-38.2728)
    gw_lon = config.getfloat("gateway", "longitude", fallback=-57.8306)
    gw_alt = config.getfloat("gateway", "altitude", fallback=12.0)

    spool = LocalSpoolQueue(db_path)
    uploader = TelemetryUploader(api_url, api_key, gw_id)

    print(f"[*] Gateway ID: {gw_id}")
    print(f"[*] API Server: {api_url}")
    print(f"[*] Spool DB:   {db_path}")

    # Hilo para vaciar cola SQLite hacia el servidor (Drain Spool)
    def spool_worker():
        while True:
            pending = spool.get_pending(limit=5)
            for item in pending:
                rec_id = item["id"]
                data = item["payload"]
                if uploader.send_telemetry(data):
                    spool.mark_success(rec_id)
                else:
                    spool.increment_retry(rec_id)
            time.sleep(2.0)

    t_spool = threading.Thread(target=spool_worker, daemon=True)
    t_spool.start()

    # Hilo para latidos Heartbeat periódicos
    def heartbeat_worker():
        while True:
            uploader.send_heartbeat(gw_lat, gw_lon, gw_alt)
            time.sleep(hb_interval)

    t_hb = threading.Thread(target=heartbeat_worker, daemon=True)
    t_hb.start()

    print("[*] Gateway en ejecucion. Esperando tramas de radio LoRa...")
    try:
        while True:
            time.sleep(1.0)
    except KeyboardInterrupt:
        print("\n[*] Deteniendo BQS Gateway.")

if __name__ == "__main__":
    main()
