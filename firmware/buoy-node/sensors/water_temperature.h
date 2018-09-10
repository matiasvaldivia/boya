#ifndef BQS_WATER_TEMPERATURE_H
#define BQS_WATER_TEMPERATURE_H

#include <Arduino.h>

class BqsWaterTemperatureSensor {
public:
    BqsWaterTemperatureSensor(uint8_t pin);
    void init();
    bool read(float &temperatureC);

private:
    uint8_t pin;
    float lastTemperature;

    void oneWireReset();
    void oneWireWriteByte(uint8_t byte);
    uint8_t oneWireReadByte();
};

#endif
