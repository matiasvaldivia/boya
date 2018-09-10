#include "battery.h"

BqsBatteryMonitor::BqsBatteryMonitor(uint8_t pin, float dividerRatio, float refVoltage)
    : pin(pin), dividerRatio(dividerRatio), refVoltage(refVoltage) {}

void BqsBatteryMonitor::init() {
    pinMode(pin, INPUT);
}

uint16_t BqsBatteryMonitor::readMillivolts() {
    // Promedio de 8 muestras analógicas para reducir fluctuaciones por oleaje
    long rawSum = 0;
    for (int i = 0; i < 8; i++) {
        rawSum += analogRead(pin);
        delayMicroseconds(50);
    }
    float rawAvg = (float)rawSum / 8.0f;
    float vPin = (rawAvg / 1023.0f) * refVoltage;
    float vBat = vPin * dividerRatio;
    return (uint16_t)(vBat * 1000.0f);
}

uint8_t BqsBatteryMonitor::estimatePercentage(uint16_t mv) {
    // Curva de descarga estimada para Li-ion 1S (4.20V = 100%, 3.30V = 0%)
    if (mv >= 4200) return 100;
    if (mv <= 3300) return 0;
    return (uint8_t)(((mv - 3300) * 100) / (4200 - 3300));
}
