#ifndef BQS_GPS_TRACKER_H
#define BQS_GPS_TRACKER_H

#include <Arduino.h>

struct BqsGpsData {
    double latitude;
    double longitude;
    float altitudeM;
    uint8_t satellites;
    float hdop;
    uint32_t unixTimestamp;
    bool fixValid;
};

class BqsGpsTracker {
public:
    BqsGpsTracker(HardwareSerial &serialPort, uint32_t baudRate);
    void init();
    bool update(uint32_t timeoutMs);
    const BqsGpsData& getData() const { return currentData; }

private:
    HardwareSerial &serial;
    uint32_t baud;
    BqsGpsData currentData;

    void parseNmeaSentence(const char *sentence);
};

#endif
