#ifndef BQS_LORA_TRANSCEIVER_H
#define BQS_LORA_TRANSCEIVER_H

#include <Arduino.h>
#include <SPI.h>

class BqsLoRaTransceiver {
public:
    BqsLoRaTransceiver(uint8_t csPin, uint8_t resetPin, uint8_t intPin);
    bool init(float frequencyMhz, int8_t txPowerDbm);
    bool send(const uint8_t *data, uint8_t len);
    bool receive(uint8_t *buf, uint8_t &len, int16_t &rssi, float &snr, uint32_t timeoutMs);
    void sleep();

private:
    uint8_t csPin;
    uint8_t resetPin;
    uint8_t intPin;
    float frequency;
    int8_t txPower;

    uint8_t readRegister(uint8_t reg);
    void writeRegister(uint8_t reg, uint8_t val);
};

#endif
