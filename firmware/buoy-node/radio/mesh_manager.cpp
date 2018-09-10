#include "mesh_manager.h"
#include <string.h>

BqsMeshManager::BqsMeshManager(BqsLoRaTransceiver &radio, BqsTelemetryQueue &queue, uint8_t nodeId, uint8_t networkId)
    : radio(radio), queue(queue), nodeId(nodeId), networkId(networkId), seenIndex(0) {
    memset(&activeRoute, 0, sizeof(activeRoute));
    memset(seenPackets, 0, sizeof(seenPackets));
}

void BqsMeshManager::init() {
    activeRoute.gatewayId = 0x01;
    activeRoute.nextHopNode = 0x00; // Directo por defecto
    activeRoute.hopCount = 1;
    activeRoute.rssi = -85;
    activeRoute.lastBeaconAtMs = millis();
    activeRoute.valid = true;
}

bool BqsMeshManager::isDuplicate(uint8_t src, uint32_t seq) {
    for (int i = 0; i < 16; i++) {
        if (seenPackets[i].sourceNode == src && seenPackets[i].sequence == seq) {
            return true;
        }
    }
    return false;
}

void BqsMeshManager::markSeen(uint8_t src, uint32_t seq) {
    seenPackets[seenIndex].sourceNode = src;
    seenPackets[seenIndex].sequence = seq;
    seenPackets[seenIndex].seenAtMs = millis();
    seenIndex = (seenIndex + 1) % 16;
}

void BqsMeshManager::updateGatewayRoute(uint8_t gatewayId, uint8_t nextHop, uint8_t hops, int16_t rssi) {
    activeRoute.gatewayId = gatewayId;
    activeRoute.nextHopNode = nextHop;
    activeRoute.hopCount = hops;
    activeRoute.rssi = rssi;
    activeRoute.lastBeaconAtMs = millis();
    activeRoute.valid = true;
}

bool BqsMeshManager::hasValidGatewayRoute() const {
    if (!activeRoute.valid) return false;
    // Considerar ruta expirada si no hay balizas en 15 minutos
    return (millis() - activeRoute.lastBeaconAtMs < 900000UL);
}

bool BqsMeshManager::routeAndSend(struct BqsTelemetryPacket &pkt) {
    markSeen(pkt.sourceNode, pkt.sequence);

    if (!hasValidGatewayRoute()) {
        // Almacenar en cola offline para retransmisión posterior
        queue.push(pkt);
        return false;
    }

    bool success = radio.send((const uint8_t *)&pkt, sizeof(struct BqsTelemetryPacket));
    if (!success) {
        queue.push(pkt);
    }
    return success;
}

bool BqsMeshManager::forwardPacket(struct BqsTelemetryPacket &pkt) {
    uint8_t ttl = BqsProtocolHelper::getTtl(pkt);
    if (ttl == 0) return false; // Descartar por TTL expirado

    uint8_t hops = BqsProtocolHelper::getHops(pkt);
    BqsProtocolHelper::setHops(pkt, hops + 1);
    BqsProtocolHelper::setTtl(pkt, ttl - 1);

    markSeen(pkt.sourceNode, pkt.sequence);
    return radio.send((const uint8_t *)&pkt, sizeof(struct BqsTelemetryPacket));
}

void BqsMeshManager::processIncomingPackets(uint32_t listenWindowMs) {
    uint8_t rxBuffer[64];
    uint8_t rxLen = 0;
    int16_t rssi = 0;
    float snr = 0.0f;

    unsigned long start = millis();
    while (millis() - start < listenWindowMs) {
        if (radio.receive(rxBuffer, rxLen, rssi, snr, 200)) {
            if (rxLen == sizeof(struct BqsTelemetryPacket)) {
                struct BqsTelemetryPacket rxPkt;
                memcpy(&rxPkt, rxBuffer, sizeof(rxPkt));

                if (BqsProtocolHelper::validatePacket(rxPkt)) {
                    if (isDuplicate(rxPkt.sourceNode, rxPkt.sequence)) {
                        continue; // Evitar bucles de retransmisión
                    }

                    if (rxPkt.type == BQS_TYPE_GATEWAY_BEACON) {
                        updateGatewayRoute(rxPkt.sourceNode, rxPkt.sourceNode, 1, rssi);
                    } else if (rxPkt.type == BQS_TYPE_TELEMETRY) {
                        if (rxPkt.destination == nodeId) {
                            // Paquete dirigido exclusivamente a este nodo
                            markSeen(rxPkt.sourceNode, rxPkt.sequence);
                        } else if (rxPkt.sourceNode != nodeId) {
                            // Reenviar paquete (Multi-Hop Relay)
                            forwardPacket(rxPkt);
                        }
                    }
                }
            }
        }
    }
}

void BqsMeshManager::flushStoredQueue() {
    if (!hasValidGatewayRoute() || queue.isEmpty()) return;

    struct BqsTelemetryPacket storedPkt;
    uint8_t sentCount = 0;
    // Transmitir hasta 3 paquetes atrasados por ciclo para no saturar el canal RF
    while (!queue.isEmpty() && sentCount < 3) {
        if (queue.peek(storedPkt)) {
            if (radio.send((const uint8_t *)&storedPkt, sizeof(struct BqsTelemetryPacket))) {
                queue.pop(storedPkt);
                sentCount++;
                delay(100); // Pequeña guarda entre paquetes
            } else {
                break;
            }
        }
    }
}
