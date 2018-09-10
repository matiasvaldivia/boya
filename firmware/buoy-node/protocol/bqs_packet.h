#ifndef BQS_PACKET_H
#define BQS_PACKET_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define BQS_MAGIC_HEADER 0xB05A

enum BqsPacketType {
    BQS_TYPE_TELEMETRY       = 0x01,
    BQS_TYPE_GATEWAY_BEACON  = 0x02,
    BQS_TYPE_ROUTE_DISCOVERY = 0x03,
    BQS_TYPE_ROUTE_REPLY     = 0x04,
    BQS_TYPE_ACK             = 0x05,
    BQS_TYPE_HEARTBEAT       = 0x06,
    BQS_TYPE_ERROR           = 0x07
};

#pragma pack(push, 1)
struct BqsTelemetryPacket {
    uint16_t magic;            // 0xB05A
    uint8_t  version;          // 0x01
    uint8_t  type;             // BqsPacketType
    uint8_t  sourceNode;       // ID de la boya de origen
    uint8_t  destination;      // 0x00 = Gateway / Broadcast
    uint32_t sequence;         // Número de secuencia correlativo
    uint32_t timestamp;        // UNIX Epoch (segundos) o tiempo relativo
    int32_t  latitudeScaled;   // Grados * 10^7
    int32_t  longitudeScaled;  // Grados * 10^7
    int16_t  airTempX10;       // °C * 10 (ej. 215 = 21.5 °C)
    uint16_t humidityX10;      // % * 10 (ej. 734 = 73.4 %)
    int16_t  waterTempX10;     // °C * 10 (ej. 189 = 18.9 °C)
    uint16_t batteryMv;        // Tensión en milivoltios (ej. 3860 mV)
    uint8_t  satellitesHops;   // (Satélites & 0x0F) << 4 | (Hops & 0x0F)
    uint8_t  ttlFlags;         // (Flags & 0xF0) | (TTL & 0x0F)
    uint16_t crc;              // CRC-CCITT sobre los 32 bytes anteriores
};
#pragma pack(pop)

#define BQS_PACKET_SIZE sizeof(struct BqsTelemetryPacket) // Exactamente 34 bytes

class BqsProtocolHelper {
public:
    static void buildTelemetryPacket(struct BqsTelemetryPacket &pkt,
                                     uint8_t sourceNode,
                                     uint32_t sequence,
                                     uint32_t timestamp,
                                     double latitude,
                                     double longitude,
                                     float airTempC,
                                     float humidityPct,
                                     float waterTempC,
                                     uint16_t batteryMv,
                                     uint8_t satellites,
                                     uint8_t hops,
                                     uint8_t ttl);

    static bool validatePacket(const struct BqsTelemetryPacket &pkt);
    static uint8_t getSatellites(const struct BqsTelemetryPacket &pkt);
    static uint8_t getHops(const struct BqsTelemetryPacket &pkt);
    static uint8_t getTtl(const struct BqsTelemetryPacket &pkt);
    static void setHops(struct BqsTelemetryPacket &pkt, uint8_t hops);
    static void setTtl(struct BqsTelemetryPacket &pkt, uint8_t ttl);
};

#endif
