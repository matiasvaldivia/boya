# Arquitectura de Sistema - BQS Buoy Network

## 1. Visión General
**BQS Buoy Network** es un sistema distribuido de boyas marinas y estaciones costeras concebido en 2018 para la monitorización oceanográfica y meteorológica en tiempo real en zonas de mar abierto, costas, lagos y embalses.

A diferencia de las arquitecturas convencionales en estrella (LoRaWAN estándar nodo ↔ gateway), el sistema implementa una **red en malla (*LoRa Mesh / Store-and-Forward*)** sobre el stack **RadioHead RHMesh (1.8x)**, donde cada boya opera simultáneamente como:
1. **Sensor Oceanográfico y Meteorológico**: Mide temperatura superficial del agua, temperatura y humedad del aire, tensión de batería y posición geodésica satelital GPS.
2. **Enrutador / Repetidor de Saltos Múltiples (*Multi-Hop Relay*)**: Reenvía paquetes de otras boyas que se encuentren fuera del alcance de visión directa de las estaciones costeras.
3. **Almacenamiento Local de Contingencia (*Store-and-Forward*)**: Si una boya no detecta rutas viables hacia ningún gateway, encola las muestras en memoria circular y las transmite de forma progresiva al restablecerse la topología.

---

## 2. Topología de Red y Enrutamiento Multi-Hop

```text
       MAR ABIERTO                      ZONA COSTERA                     TIERRA / NUBE

   BOYA 17          BOYA 12             BOYA 04
 ┌─────────┐      ┌─────────┐         ┌─────────┐
 │ Arduino │      │ Arduino │         │ Arduino │
 │ RFM95W  │◄────►│ RFM95W  │◄───────►│ RFM95W  │
 │ GPS/Sen │ LoRa │ GPS/Sen │  LoRa   │ GPS/Sen │
 └─────────┘      └─────────┘         └────┬────┘
                                           │ LoRa
                                           ▼
                                     ┌───────────┐
                                     │ GATEWAY 01│
                                     │ RPi 3     │
                                     └─────┬─────┘
                                           │ Internet / 3G
                                           ▼
                                    ┌─────────────┐
                                    │ BQS SERVER  │
                                    │ MySQL + PHP │
                                    │ Leaflet Map │
                                    └─────────────┘
```
