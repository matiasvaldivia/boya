#ifndef BQS_DHT_SENSOR_H
#define BQS_DHT_SENSOR_H

#include <Arduino.h>

class BqsDhtSensor {
public:
    BqsDhtSensor(uint8_t pin);
    void init();
    bool read(float &temperatureC, float &humidityPct);

private:
    uint8_t pin;
    float lastTemperature;
    float lastHumidity;
    unsigned long lastReadTime;
};

#endif
