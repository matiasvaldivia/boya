#include "bqs_packet.h"
#include "crc16.h"
#include <string.h>

void BqsProtocolHelper::buildTelemetryPacket(struct BqsTelemetryPacket &pkt,
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
                                             uint8_t ttl) {
    memset(&pkt, 0, sizeof(pkt));

    pkt.magic = BQS_MAGIC_HEADER;
    pkt.version = 0x01;
    pkt.type = BQS_TYPE_TELEMETRY;
    pkt.sourceNode = sourceNode;
    pkt.destination = 0x00; // Gateway
    pkt.sequence = sequence;
    pkt.timestamp = timestamp;

    // Escalar coordenadas a enteros con resolución sub-métrica (10^7)
    pkt.latitudeScaled = (int32_t)(latitude * 10000000.0);
    pkt.longitudeScaled = (int32_t)(longitude * 10000000.0);

    // Escalar magnitudes meteorológicas y oceanográficas (10^1)
    pkt.airTempX10 = (int16_t)(airTempC * 10.0f);
    pkt.humidityX10 = (uint16_t)(humidityPct * 10.0f);
    pkt.waterTempX10 = (int16_t)(waterTempC * 10.0f);

    pkt.batteryMv = batteryMv;

    // Empaquetar nibbles
    if (satellites > 15) satellites = 15;
    if (hops > 15) hops = 15;
    pkt.satellitesHops = (satellites << 4) | (hops & 0x0F);

    if (ttl > 15) ttl = 15;
    pkt.ttlFlags = (ttl & 0x0F); // Flags = 0 por defecto

    // Calcular CRC sobre los primeros 32 bytes (excluyendo el campo crc final)
    size_t payloadLen = sizeof(struct BqsTelemetryPacket) - sizeof(uint16_t);
    pkt.crc = bqs_crc16((const uint8_t *)&pkt, payloadLen);
}

bool BqsProtocolHelper::validatePacket(const struct BqsTelemetryPacket &pkt) {
    if (pkt.magic != BQS_MAGIC_HEADER) return false;
    if (pkt.version != 0x01) return false;

    size_t payloadLen = sizeof(struct BqsTelemetryPacket) - sizeof(uint16_t);
    uint16_t calculatedCrc = bqs_crc16((const uint8_t *)&pkt, payloadLen);
    return calculatedCrc == pkt.crc;
}

uint8_t BqsProtocolHelper::getSatellites(const struct BqsTelemetryPacket &pkt) {
    return (pkt.satellitesHops >> 4) & 0x0F;
}

uint8_t BqsProtocolHelper::getHops(const struct BqsTelemetryPacket &pkt) {
    return pkt.satellitesHops & 0x0F;
}

uint8_t BqsProtocolHelper::getTtl(const struct BqsTelemetryPacket &pkt) {
    return pkt.ttlFlags & 0x0F;
}

void BqsProtocolHelper::setHops(struct BqsTelemetryPacket &pkt, uint8_t hops) {
    if (hops > 15) hops = 15;
    pkt.satellitesHops = (pkt.satellitesHops & 0xF0) | (hops & 0x0F);
    // Recalcular CRC
    size_t payloadLen = sizeof(struct BqsTelemetryPacket) - sizeof(uint16_t);
    pkt.crc = bqs_crc16((const uint8_t *)&pkt, payloadLen);
}

void BqsProtocolHelper::setTtl(struct BqsTelemetryPacket &pkt, uint8_t ttl) {
    if (ttl > 15) ttl = 15;
    pkt.ttlFlags = (pkt.ttlFlags & 0xF0) | (ttl & 0x0F);
    // Recalcular CRC
    size_t payloadLen = sizeof(struct BqsTelemetryPacket) - sizeof(uint16_t);
    pkt.crc = bqs_crc16((const uint8_t *)&pkt, payloadLen);
}
