import time
import struct
from .packet_decoder import bqs_crc16, BQS_MAGIC

class BeaconBroadcaster:
    @staticmethod
    def build_beacon_packet(gateway_id_int: int, priority: int = 1) -> bytes:
        magic = BQS_MAGIC
        version = 0x01
        pkt_type = 0x02 # BQS_TYPE_GATEWAY_BEACON
        src_node = gateway_id_int
        dest_node = 0xFF # Broadcast general
        seq = int(time.time()) & 0xFFFFFFFF
        ts = int(time.time())
        lat_scaled = 0
        lon_scaled = 0
        air_t = 0
        hum = 0
        water_t = 0
        batt_mv = 12000 # 12V estación base
        sat_hops = (0 << 4) | (0 & 0x0F)
        ttl_flags = (priority << 4) | (6 & 0x0F)

        # Empaquetar 32 bytes
        header = struct.pack(
            "<HBBB BII ii h H h H BB",
            magic, version, pkt_type, src_node, dest_node, seq, ts,
            lat_scaled, lon_scaled, air_t, hum, water_t, batt_mv,
            sat_hops, ttl_flags
        )
        crc = bqs_crc16(header)
        return header + struct.pack("<H", crc)
