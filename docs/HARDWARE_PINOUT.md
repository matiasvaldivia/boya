# Conexión Eléctrica y Pinout - Boya BQS (Arduino AVR + RFM95W)

## 1. Módulo de Radio LoRa (HopeRF RFM95W / Semtech SX1276)

| Pin RFM95W | Señal | Pin Arduino (ATmega328P / Nano / Pro Mini) | Función |
| :--- | :--- | :---: | :--- |
| **VCC** | 3.3V Power | **3V3** | Alimentación regulada de radio (máx. 120 mA en TX) |
| **GND** | Ground | **GND** | Masa común |
| **NSS / CS** | SPI Chip Select | **D10** | Salida digital selección de esclavo SPI |
| **MOSI** | SPI Master Out | **D11** | Bus SPI datos hacia transceiver |
| **MISO** | SPI Master In | **D12** | Bus SPI datos desde transceiver |
| **SCK** | SPI Clock | **D13** | Reloj de bus SPI |
| **RESET** | Hardware Reset | **D9** | Pulso LOW para reinicio de módem |
| **DIO0** | Packet Rx/Tx Done | **D2** | Interrupción por hardware INT0 |

---

## 2. Sensores Ambientales y Marinos

| Sensor | Magnitud | Pin Sensor | Pin Arduino | Interfaz / Circuito |
| :--- | :--- | :--- | :---: | :--- |
| **Dallas DS18B20** | Temp. Agua Sumergible | Data (Amarillo/Blanco) | **D6** | Bus OneWire con resistencia Pull-Up de 4.7kΩ a 5V |
| **DHT22 / AM2302** | Temp. y Humedad Aire | Data (Pin 2) | **D5** | Bus Digital bidireccional con Pull-Up 10kΩ |
| **Divisor Batería** | Tensión Acumulador | Vout Divisor | **A0** | Entrada ADC (R1=100kΩ a Vbat, R2=22kΩ a GND) |
| **u-blox NEO-6M** | GPS Posición / Tiempo | TX GPS | **RX1 / D0 / SoftSerial** | Recepción serie de sentencias NMEA (9600 bps) |
