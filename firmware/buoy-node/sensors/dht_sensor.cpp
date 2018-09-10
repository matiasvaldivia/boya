#include "dht_sensor.h"

BqsDhtSensor::BqsDhtSensor(uint8_t pin)
    : pin(pin), lastTemperature(20.0f), lastHumidity(70.0f), lastReadTime(0) {}

void BqsDhtSensor::init() {
    pinMode(pin, INPUT_PULLUP);
}

bool BqsDhtSensor::read(float &temperatureC, float &humidityPct) {
    unsigned long now = millis();
    // DHT22 requiere al menos 2000 ms entre lecturas consecutivas
    if (now - lastReadTime < 2000 && lastReadTime != 0) {
        temperatureC = lastTemperature;
        humidityPct = lastHumidity;
        return true;
    }

    uint8_t data[5] = {0, 0, 0, 0, 0};

    // Señal de arranque del Host (LOW por 18ms)
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    delay(18);
    digitalWrite(pin, HIGH);
    delayMicroseconds(40);
    pinMode(pin, INPUT_PULLUP);

    // Espera respuesta del sensor (80µs LOW, 80µs HIGH)
    unsigned long timeout = micros();
    while (digitalRead(pin) == HIGH) {
        if (micros() - timeout > 100) return false;
    }
    timeout = micros();
    while (digitalRead(pin) == LOW) {
        if (micros() - timeout > 100) return false;
    }
    timeout = micros();
    while (digitalRead(pin) == HIGH) {
        if (micros() - timeout > 100) return false;
    }

    // Lectura de 40 bits
    for (int i = 0; i < 40; i++) {
        timeout = micros();
        while (digitalRead(pin) == LOW) {
            if (micros() - timeout > 100) return false;
        }
        unsigned long tStart = micros();
        while (digitalRead(pin) == HIGH) {
            if (micros() - timeout > 150) return false;
        }
        if ((micros() - tStart) > 40) {
            data[i / 8] |= (1 << (7 - (i % 8)));
        }
    }

    // Validación de Checksum DHT
    if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        float h = ((data[0] << 8) | data[1]) * 0.1f;
        int16_t rawT = ((data[2] & 0x7F) << 8) | data[3];
        float t = rawT * 0.1f;
        if (data[2] & 0x80) t = -t;

        lastHumidity = h;
        lastTemperature = t;
        lastReadTime = now;
        temperatureC = t;
        humidityPct = h;
        return true;
    }

    // En caso de ruido puntual en bus 1-wire, conservar última lectura válida
    temperatureC = lastTemperature;
    humidityPct = lastHumidity;
    return false;
}
