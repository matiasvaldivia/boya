#include "water_temperature.h"

BqsWaterTemperatureSensor::BqsWaterTemperatureSensor(uint8_t pin)
    : pin(pin), lastTemperature(18.5f) {}

void BqsWaterTemperatureSensor::init() {
    pinMode(pin, INPUT_PULLUP);
}

void BqsWaterTemperatureSensor::oneWireReset() {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    delayMicroseconds(480);
    pinMode(pin, INPUT_PULLUP);
    delayMicroseconds(480);
}

void BqsWaterTemperatureSensor::oneWireWriteByte(uint8_t byte) {
    for (uint8_t i = 0; i < 8; i++) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
        if (byte & 0x01) {
            delayMicroseconds(10);
            pinMode(pin, INPUT_PULLUP);
            delayMicroseconds(55);
        } else {
            delayMicroseconds(65);
            pinMode(pin, INPUT_PULLUP);
            delayMicroseconds(5);
        }
        byte >>= 1;
    }
}

uint8_t BqsWaterTemperatureSensor::oneWireReadByte() {
    uint8_t byte = 0;
    for (uint8_t i = 0; i < 8; i++) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
        delayMicroseconds(3);
        pinMode(pin, INPUT_PULLUP);
        delayMicroseconds(10);
        if (digitalRead(pin) == HIGH) {
            byte |= (1 << i);
        }
        delayMicroseconds(53);
    }
    return byte;
}

bool BqsWaterTemperatureSensor::read(float &temperatureC) {
    // 1. Comando Convert T (0x44)
    oneWireReset();
    oneWireWriteByte(0xCC); // Skip ROM
    oneWireWriteByte(0x44); // Convert T
    delay(200);             // Tiempo de conversión 9-10 bit

    // 2. Comando Read Scratchpad (0xBE)
    oneWireReset();
    oneWireWriteByte(0xCC); // Skip ROM
    oneWireWriteByte(0xBE); // Read Scratchpad

    uint8_t lsb = oneWireReadByte();
    uint8_t msb = oneWireReadByte();

    int16_t raw = (msb << 8) | lsb;
    if (raw == -1 || (lsb == 0xFF && msb == 0xFF)) {
        temperatureC = lastTemperature;
        return false;
    }

    float t = (float)raw / 16.0f;
    if (t >= -10.0f && t <= 50.0f) { // Rango oceánico plausible
        lastTemperature = t;
        temperatureC = t;
        return true;
    }

    temperatureC = lastTemperature;
    return false;
}
