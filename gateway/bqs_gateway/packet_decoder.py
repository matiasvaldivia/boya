import struct
import time

BQS_MAGIC = 0xB05A
BQS_PKT_FORMAT = "<HBBB BII ii H h H H BB H" # 34 bytes packed

def bqs_crc16(data: bytes) -> int:
    crc = 0xFFFF
    for byte in data:
        crc ^= (byte << 8)
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc

class BqsPacketDecoder:
    @staticmethod
    def decode(raw_bytes: bytes, rssi: int = -85, snr: float = 8.5) -> dict:
        if len(raw_bytes) < 34:
            return None

        # Desempaquetar
        try:
            magic, version, pkt_type, src_node, dest_node, seq, ts, lat_scaled, lon_scaled, \
            air_t_x10, hum_x10, water_t_x10, batt_mv, sat_hops, ttl_flags, crc = struct.unpack(
                "<HBBB BII ii h H h H BB H", raw_bytes[:34]
            )
        except Exception:
            return None

        if magic != BQS_MAGIC or version != 0x01:
            return None

        # Validar CRC-CCITT sobre los primeros 32 bytes
        calculated_crc = bqs_crc16(raw_bytes[:32])
        if calculated_crc != crc:
            return None

        satellites = (sat_hops >> 4) & 0x0F
        hops = sat_hops & 0x0F
        ttl = ttl_flags & 0x0F
        flags = (ttl_flags >> 4) & 0x0F

        # Conversión de valores escalados a magnitudes físicas
        lat = lat_scaled / 10000000.0
        lon = lon_scaled / 10000000.0
        air_temp = air_t_x10 / 10.0
        humidity = hum_x10 / 10.0
        water_temp = water_t_x10 / 10.0

        # Estimar porcentaje de batería
        batt_pct = max(0, min(100, int(((batt_mv - 3300) * 100) / (4200 - 3300))))

        return {
            "node_id": src_node,
            "destination": dest_node,
            "packet_type": pkt_type,
            "sequence": seq,
            "timestamp": ts if ts > 1000000000 else int(time.time()),
            "latitude": round(lat, 6),
            "longitude": round(lon, 6),
            "air_temperature": round(air_temp, 1),
            "humidity": round(humidity, 1),
            "water_temperature": round(water_temp, 1),
            "battery_mv": batt_mv,
            "battery_percent": batt_pct,
            "satellites": satellites,
            "hops": hops,
            "ttl": ttl,
            "flags": flags,
            "rssi": rssi,
            "snr": snr
        }
