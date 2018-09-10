#include "lora_transceiver.h"

#define REG_FIFO                 0x00
#define REG_OP_MODE              0x01
#define REG_FRF_MSB              0x06
#define REG_FRF_MID              0x07
#define REG_FRF_LSB              0x08
#define REG_PA_CONFIG            0x09
#define REG_FIFO_ADDR_PTR        0x0D
#define REG_FIFO_TX_BASE_ADDR    0x0E
#define REG_FIFO_RX_BASE_ADDR    0x0F
#define REG_FIFO_RX_CURRENT_ADDR 0x10
#define REG_IRQ_FLAGS            0x12
#define REG_RX_NB_BYTES          0x13
#define REG_PKT_SNR_VALUE        0x19
#define REG_PKT_RSSI_VALUE       0x1A
#define REG_MODEM_CONFIG_1       0x1D
#define REG_MODEM_CONFIG_2       0x1E
#define REG_PAYLOAD_LENGTH       0x22
#define REG_DIO_MAPPING_1        0x40
#define REG_VERSION              0x42
#define REG_PA_DAC               0x4D

#define MODE_LONG_RANGE_MODE     0x80
#define MODE_SLEEP               0x00
#define MODE_STDBY               0x01
#define MODE_TX                  0x03
#define MODE_RX_CONTINUOUS       0x05

BqsLoRaTransceiver::BqsLoRaTransceiver(uint8_t csPin, uint8_t resetPin, uint8_t intPin)
    : csPin(csPin), resetPin(resetPin), intPin(intPin), frequency(915.0f), txPower(20) {}

uint8_t BqsLoRaTransceiver::readRegister(uint8_t reg) {
    digitalWrite(csPin, LOW);
    SPI.transfer(reg & 0x7F);
    uint8_t val = SPI.transfer(0x00);
    digitalWrite(csPin, HIGH);
    return val;
}

void BqsLoRaTransceiver::writeRegister(uint8_t reg, uint8_t val) {
    digitalWrite(csPin, LOW);
    SPI.transfer(reg | 0x80);
    SPI.transfer(val);
    digitalWrite(csPin, HIGH);
}

bool BqsLoRaTransceiver::init(float frequencyMhz, int8_t txPowerDbm) {
    this->frequency = frequencyMhz;
    this->txPower = txPowerDbm;

    pinMode(csPin, OUTPUT);
    pinMode(resetPin, OUTPUT);
    pinMode(intPin, INPUT);
    digitalWrite(csPin, HIGH);

    // Reset por hardware
    digitalWrite(resetPin, LOW);
    delay(10);
    digitalWrite(resetPin, HIGH);
    delay(10);

    SPI.begin();

    uint8_t version = readRegister(REG_VERSION);
    if (version != 0x12) { // 0x12 = Semtech SX1276/77/78/79
        return false;
    }

    // Modo Sleep para permitir configuración de LoRa
    writeRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_SLEEP);
    delay(10);

    // Configurar Frecuencia (915 MHz)
    uint64_t frf = ((uint64_t)frequencyMhz * 1000000UL) / 61.03515625;
    writeRegister(REG_FRF_MSB, (uint8_t)(frf >> 16));
    writeRegister(REG_FRF_MID, (uint8_t)(frf >> 8));
    writeRegister(REG_FRF_LSB, (uint8_t)(frf >> 0));

    // Base addresses FIFO
    writeRegister(REG_FIFO_TX_BASE_ADDR, 0);
    writeRegister(REG_FIFO_RX_BASE_ADDR, 0);

    // Configurar Modem (BW 125kHz, CR 4/5, SF7, CRC On)
    writeRegister(REG_MODEM_CONFIG_1, 0x72); // BW=125kHz, CR=4/5, Explicit Header
    writeRegister(REG_MODEM_CONFIG_2, 0x74); // SF=7, CRC Enable

    // Configurar Potencia (PA_BOOST con 20 dBm)
    writeRegister(REG_PA_DAC, 0x87); // High Power PA_DAC
    writeRegister(REG_PA_CONFIG, 0x80 | 0x0F); // PA_BOOST + Max Power

    writeRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
    return true;
}

bool BqsLoRaTransceiver::send(const uint8_t *data, uint8_t len) {
    writeRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
    writeRegister(REG_FIFO_ADDR_PTR, 0);
    writeRegister(REG_PAYLOAD_LENGTH, len);

    for (uint8_t i = 0; i < len; i++) {
        writeRegister(REG_FIFO, data[i]);
    }

    writeRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_TX);

    unsigned long start = millis();
    while ((readRegister(REG_IRQ_FLAGS) & 0x08) == 0) { // Esperar TxDone
        if (millis() - start > 2000) return false;
    }
    writeRegister(REG_IRQ_FLAGS, 0x08); // Limpiar TxDone
    writeRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
    return true;
}

bool BqsLoRaTransceiver::receive(uint8_t *buf, uint8_t &len, int16_t &rssi, float &snr, uint32_t timeoutMs) {
    writeRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_RX_CONTINUOUS);
    unsigned long start = millis();

    while (millis() - start < timeoutMs) {
        uint8_t irq = readRegister(REG_IRQ_FLAGS);
        if (irq & 0x40) { // RxDone
            writeRegister(REG_IRQ_FLAGS, 0x40);

            if (irq & 0x20) { // PayloadCrcError
                writeRegister(REG_IRQ_FLAGS, 0x20);
                return false;
            }

            uint8_t currentAddr = readRegister(REG_FIFO_RX_CURRENT_ADDR);
            uint8_t bytesReceived = readRegister(REG_RX_NB_BYTES);
            writeRegister(REG_FIFO_ADDR_PTR, currentAddr);

            for (uint8_t i = 0; i < bytesReceived; i++) {
                buf[i] = readRegister(REG_FIFO);
            }
            len = bytesReceived;

            // RSSI y SNR
            rssi = readRegister(REG_PKT_RSSI_VALUE) - 157;
            int8_t snrRaw = (int8_t)readRegister(REG_PKT_SNR_VALUE);
            snr = snrRaw * 0.25f;

            writeRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
            return true;
        }
    }
    writeRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
    return false;
}

void BqsLoRaTransceiver::sleep() {
    writeRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_SLEEP);
}
