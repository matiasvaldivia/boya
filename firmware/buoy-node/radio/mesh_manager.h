#ifndef BQS_MESH_MANAGER_H
#define BQS_MESH_MANAGER_H

#include "lora_transceiver.h"
#include "../protocol/bqs_packet.h"
#include "../storage/telemetry_queue.h"

struct SeenPacketEntry {
    uint8_t sourceNode;
    uint32_t sequence;
    unsigned long seenAtMs;
};

struct RouteTableEntry {
    uint8_t gatewayId;
    uint8_t nextHopNode;
    uint8_t hopCount;
    int16_t rssi;
    unsigned long lastBeaconAtMs;
    bool valid;
};

class BqsMeshManager {
public:
    BqsMeshManager(BqsLoRaTransceiver &radio, BqsTelemetryQueue &queue, uint8_t nodeId, uint8_t networkId);
    void init();
    bool routeAndSend(struct BqsTelemetryPacket &pkt);
    void processIncomingPackets(uint32_t listenWindowMs);
    void updateGatewayRoute(uint8_t gatewayId, uint8_t nextHop, uint8_t hops, int16_t rssi);
    bool hasValidGatewayRoute() const;
    void flushStoredQueue();

private:
    BqsLoRaTransceiver &radio;
    BqsTelemetryQueue &queue;
    uint8_t nodeId;
    uint8_t networkId;

    RouteTableEntry activeRoute;
    SeenPacketEntry seenPackets[16];
    uint8_t seenIndex;

    bool isDuplicate(uint8_t src, uint32_t seq);
    void markSeen(uint8_t src, uint32_t seq);
    bool forwardPacket(struct BqsTelemetryPacket &pkt);
};

#endif
