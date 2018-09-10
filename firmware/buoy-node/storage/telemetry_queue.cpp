#include "telemetry_queue.h"
#include <stdlib.h>
#include <string.h>

BqsTelemetryQueue::BqsTelemetryQueue(uint8_t capacity)
    : capacity(capacity), head(0), tail(0), count(0) {
    buffer = (struct BqsTelemetryPacket *)malloc(sizeof(struct BqsTelemetryPacket) * capacity);
}

BqsTelemetryQueue::~BqsTelemetryQueue() {
    if (buffer != NULL) {
        free(buffer);
    }
}

bool BqsTelemetryQueue::push(const struct BqsTelemetryPacket &pkt) {
    if (buffer == NULL) return false;

    if (isFull()) {
        // En buffer lleno, sobreescribir la muestra más antigua (FIFO circular)
        head = (head + 1) % capacity;
        count--;
    }

    memcpy(&buffer[tail], &pkt, sizeof(struct BqsTelemetryPacket));
    tail = (tail + 1) % capacity;
    count++;
    return true;
}

bool BqsTelemetryQueue::pop(struct BqsTelemetryPacket &pkt) {
    if (isEmpty() || buffer == NULL) return false;

    memcpy(&pkt, &buffer[head], sizeof(struct BqsTelemetryPacket));
    head = (head + 1) % capacity;
    count--;
    return true;
}

bool BqsTelemetryQueue::peek(struct BqsTelemetryPacket &pkt) const {
    if (isEmpty() || buffer == NULL) return false;
    memcpy(&pkt, &buffer[head], sizeof(struct BqsTelemetryPacket));
    return true;
}

bool BqsTelemetryQueue::isEmpty() const {
    return count == 0;
}

bool BqsTelemetryQueue::isFull() const {
    return count >= capacity;
}

uint8_t BqsTelemetryQueue::getCount() const {
    return count;
}

void BqsTelemetryQueue::clear() {
    head = 0;
    tail = 0;
    count = 0;
}
