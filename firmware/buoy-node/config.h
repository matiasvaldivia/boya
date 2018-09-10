#ifndef BQS_CONFIG_H
#define BQS_CONFIG_H

#include <Arduino.h>

// =============================================================================
// BQS BUOY NETWORK - Configuración del Nodo Boya (2018)
// =============================================================================

// Identificación del Nodo y Red
#define BQS_NODE_ID            17          // Identificador único de este nodo boya (1-254)
#define BQS_NETWORK_ID         0x42        // ID de red para aislamiento de tráfico LoRa
#define BQS_PROTOCOL_VERSION   0x01        // Versión del protocolo BQS

// Configuración de Radio LoRa (HopeRF RFM95W / Semtech SX1276)
#define LORA_FREQUENCY         915000000.0 // Frecuencia ISM 915 MHz (Argentina / Región 2)
#define LORA_TX_POWER          20          // Potencia de transmisión en dBm (5 - 20)
#define LORA_BANDWIDTH         125000      // Ancho de banda 125 kHz
#define LORA_SPREADING_FACTOR  7           // Spreading Factor SF7 (optimizado para throughput/alcance)
#define LORA_CODING_RATE       5           // 4/5 Coding Rate

// Pines de Interfaz de Radio RFM95W
#define PIN_RFM95_CS           10          // SPI Chip Select
#define PIN_RFM95_RST          9           // Reset Pin
#define PIN_RFM95_INT          2           // Interrupción DIO0 (Pin 2 = INT0 en ATmega328P)

// Pines de Sensores Ambientales y Marinos
#define PIN_DHT                5           // DHT22 / AM2302 (Temperatura y Humedad de Aire)
#define PIN_DS18B20            6           // Dallas DS18B20 Sumergible (Temperatura de Agua)
#define PIN_BATTERY_ADC        A0          // Entrada analógica con divisor resistivo (100k / 22k)

// Divisor Resistivo de Batería (Relación R1=100k, R2=22k -> Vout = Vin * 22 / 122)
#define BATTERY_DIVIDER_RATIO  5.5454f     // (100 + 22) / 22
#define ADC_REF_VOLTAGE        5.0f        // Tensión de referencia ADC

// Parámetros de Enrutamiento y Malla LoRa (RadioHead RHMesh)
#define BQS_GATEWAY_ADDRESS    0x00        // Dirección lógica de broadcast / gateway primario
#define BQS_MAX_HOPS           6           // Límite de saltos máximos (TTL inicial)
#define BQS_MAX_RETRIES        3           // Reintentos de transmisión antes de encolar
#define BQS_TIMEOUT_MS         3000        // Timeout para recepción de ACK en milisegundos

// Tiempos e Intervalos de Muestreo
#define TELEMETRY_INTERVAL_MS  60000UL     // Período de transmisión de telemetría (1 minuto)
#define GPS_ACQUIRE_TIMEOUT_MS 15000UL     // Tiempo máximo de espera para fix satelital
#define RX_LISTEN_WINDOW_MS    3000UL      // Ventana de escucha para relay de otras boyas

// Capacidad de Almacenamiento Offline (Store-and-Forward)
#define QUEUE_MAX_RECORDS      32          // Capacidad de cola circular local

#endif
