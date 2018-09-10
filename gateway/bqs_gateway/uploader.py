import requests
import json
import logging

class TelemetryUploader:
    def __init__(self, api_url, api_key, gateway_id):
        self.api_url = api_url.rstrip("/")
        self.api_key = api_key
        self.gateway_id = gateway_id
        self.session = requests.Session()
        self.session.headers.update({
            "Content-Type": "application/json",
            "X-Gateway-ID": self.gateway_id,
            "X-API-Key": self.api_key,
            "User-Agent": "BQS-Gateway-Client/2018.1"
        })

    def send_telemetry(self, telemetry_data: dict) -> bool:
        endpoint = f"{self.api_url}/telemetry"
        payload = {
            "gateway_id": self.gateway_id,
            "packet": telemetry_data
        }

        try:
            resp = self.session.post(endpoint, json=payload, timeout=5.0)
            return resp.status_code in [200, 201, 204]
        except Exception as e:
            return False

    def send_heartbeat(self, lat, lon, alt) -> bool:
        endpoint = f"{self.api_url}/gateways/heartbeat"
        payload = {
            "gateway_id": self.gateway_id,
            "latitude": lat,
            "longitude": lon,
            "altitude": alt,
            "status": "ONLINE"
        }

        try:
            resp = self.session.post(endpoint, json=payload, timeout=5.0)
            return resp.status_code in [200, 201, 204]
        except Exception:
            return False
