#ifndef BQS_BATTERY_H
#define BQS_BATTERY_H

#include <Arduino.h>

class BqsBatteryMonitor {
public:
    BqsBatteryMonitor(uint8_t pin, float dividerRatio, float refVoltage);
    void init();
    uint16_t readMillivolts();
    uint8_t estimatePercentage(uint16_t mv);

private:
    uint8_t pin;
    float dividerRatio;
    float refVoltage;
};

#endif
