#ifndef BQS_TELEMETRY_QUEUE_H
#define BQS_TELEMETRY_QUEUE_H

#include "../protocol/bqs_packet.h"
#include <Arduino.h>

class BqsTelemetryQueue {
public:
    BqsTelemetryQueue(uint8_t capacity);
    ~BqsTelemetryQueue();

    bool push(const struct BqsTelemetryPacket &pkt);
    bool pop(struct BqsTelemetryPacket &pkt);
    bool peek(struct BqsTelemetryPacket &pkt) const;
    bool isEmpty() const;
    bool isFull() const;
    uint8_t getCount() const;
    void clear();

private:
    uint8_t capacity;
    struct BqsTelemetryPacket *buffer;
    uint8_t head;
    uint8_t tail;
    uint8_t count;
};

#endif
