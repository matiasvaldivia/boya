#include "config.h"
#include "protocol/bqs_packet.h"
#include "sensors/dht_sensor.h"
#include "sensors/water_temperature.h"
#include "sensors/battery.h"
#include "gps/gps_tracker.h"
#include "storage/telemetry_queue.h"
#include "radio/lora_transceiver.h"
#include "radio/mesh_manager.h"

// Instanciación de Objetos del Sistema
BqsDhtSensor dht(PIN_DHT);
BqsWaterTemperatureSensor waterTemp(PIN_DS18B20);
BqsBatteryMonitor battery(PIN_BATTERY_ADC, BATTERY_DIVIDER_RATIO, ADC_REF_VOLTAGE);
BqsGpsTracker gps(Serial1, 9600); // UART GPS u-blox

BqsLoRaTransceiver radio(PIN_RFM95_CS, PIN_RFM95_RST, PIN_RFM95_INT);
BqsTelemetryQueue queue(QUEUE_MAX_RECORDS);
BqsMeshManager mesh(radio, queue, BQS_NODE_ID, BQS_NETWORK_ID);

uint32_t packetSequence = 1;
unsigned long lastTelemetryTime = 0;

enum NodeFsmState {
    STATE_BOOT,
    STATE_SENSOR_READ,
    STATE_GPS_ACQUIRE,
    STATE_BUILD_PACKET,
    STATE_RADIO_TX,
    STATE_RADIO_RX_WINDOW,
    STATE_SLEEP
};

NodeFsmState currentState = STATE_BOOT;

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println(F("[BQS-NODE] Iniciando Boya Marina LoRa Mesh (2018)..."));

    dht.init();
    waterTemp.init();
    battery.init();
    gps.init();

    if (radio.init(LORA_FREQUENCY / 1000000.0f, LORA_TX_POWER)) {
        Serial.println(F("[BQS-NODE] Transceiver LoRa RFM95W Inicializado OK (915 MHz)."));
    } else {
        Serial.println(F("[BQS-NODE] ERROR al inicializar radio LoRa!"));
    }

    mesh.init();
    currentState = STATE_SENSOR_READ;
    lastTelemetryTime = millis();
}

void loop() {
    unsigned long now = millis();

    switch (currentState) {
        case STATE_SENSOR_READ: {
            float airTemp = 0.0f, humidity = 0.0f, waterT = 0.0f;
            dht.read(airTemp, humidity);
            waterTemp.read(waterT);
            uint16_t battMv = battery.readMillivolts();

            // Adquirir GPS
            gps.update(2000);
            const BqsGpsData &gpsData = gps.getData();

            // Construir Trama Binaria Compacta
            struct BqsTelemetryPacket pkt;
            BqsProtocolHelper::buildTelemetryPacket(
                pkt,
                BQS_NODE_ID,
                packetSequence++,
                gpsData.unixTimestamp,
                gpsData.latitude,
                gpsData.longitude,
                airTemp,
                humidity,
                waterT,
                battMv,
                gpsData.satellites,
                0, // Hops inicial = 0
                BQS_MAX_HOPS
            );

            // Transmitir vía Malla LoRa
            mesh.routeAndSend(pkt);

            // Intentar descargar muestras acumuladas si hay enlace
            mesh.flushStoredQueue();

            currentState = STATE_RADIO_RX_WINDOW;
            break;
        }

        case STATE_RADIO_RX_WINDOW: {
            // Escuchar el canal de radio durante 3 segundos para retransmitir tramas de otras boyas
            mesh.processIncomingPackets(RX_LISTEN_WINDOW_MS);
            currentState = STATE_SLEEP;
            break;
        }

        case STATE_SLEEP: {
            // Poner el radio en bajo consumo
            radio.sleep();

            if (now - lastTelemetryTime >= TELEMETRY_INTERVAL_MS) {
                lastTelemetryTime = now;
                currentState = STATE_SENSOR_READ;
            }
            break;
        }

        default:
            currentState = STATE_SENSOR_READ;
            break;
    }
}
