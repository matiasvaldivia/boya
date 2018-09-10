# BQS Miramar - Red Distribuida de Boyas Marinas LoRa Mesh

<p align="center">
  <img src="docs/img/bqs_miramar_logo.png" width="130" alt="Logo BQS Miramar">
  <br>
  <b>Sistema Autónomo de Telemetría Oceanográfica y Meteorológica</b>
  <br>
  <i>Miramar &middot; Mar del Plata &middot; Argentina</i>
</p>

<p align="center">
  <img src="docs/img/buoy_prototype_2018.jpg" width="480" alt="Prototipo Físico Boya BQS" style="border-radius: 8px;">
</p>

---

## Descripción General

**BQS Miramar** es una plataforma distribuida de boyas marinas y estaciones costeras concebida para la monitorización oceanográfica y ambiental en tiempo real en zonas de mar abierto, rompiente y zonas costeras.

A diferencia de las redes en estrella tradicionales, el sistema implementa una **topología en malla (*LoRa Mesh / Store-and-Forward*)** sobre el stack **RadioHead RHMesh (1.8x)** en la banda ISM de **915 MHz**. Cada boya opera simultáneamente como estación de sensado y como nodo repetidor (*multi-hop relay*), permitiendo que boyas ubicadas fuera del alcance de visión directa de la costa transmitan sus mediciones a través de boyas intermedias hasta alcanzar las estaciones base (*Gateways*).

---

## Características Principales

1. **Enrutamiento Multi-Hop Dinámico**:
   - Descubrimiento de rutas automático y reenvío de paquetes con decremento de saltos (TTL máx. 6 saltos).
   - Prevención de bucles (*anti-looping*) mediante tabla de tramas vistas por clave `(source_node, sequence)`.
2. **Tolerancia a Desconexión (*Store-and-Forward*)**:
   - Cola circular de contingencia en cada boya para retener hasta 32 paquetes si se pierde la ruta hacia la costa.
   - Base de datos transaccional SQLite (`spool.db`) en el Gateway de tierra para acumular mediciones ante caídas de la conexión celular 3G/Internet.
3. **Sensores Oceanográficos y Físicos**:
   - **Temperatura de Agua Superficial**: Sensor sumergible digital Dallas **DS18B20** montado en la quilla estabilizadora inferior.
   - **Temperatura y Humedad de Aire**: Sensor digital **DHT22 / AM2302** en la torre superior ventilada.
   - **Posicionamiento Geodésico**: Módulo satelital **u-blox NEO-6M / M8N** con antena cerámica activa para geolocalización y sincronización horaria UTC.
   - **Monitoreo de Batería**: Divisor resistivo en pin analógico `A0` con alerta automática a niveles inferiores a 3.5 V.
4. **Protocolo Binario Ultracompacto (34 Bytes)**:
   - Trama serializada con coordenadas y variables físicas escaladas en enteros para reducir el tiempo al aire (*Time on Air*) y maximizar la autonomía energética.
   - Verificación de integridad mediante suma de comprobación **CRC16-CCITT** (polinomio `0x1021`).
5. **Panel de Gestión & ABM Integral**:
   - Visualización en mapa georreferenciado con **Leaflet**.
   - Módulo **ABM (Alta, Baja y Modificación)** de boyas y estaciones.
   - Visor de **Logs de Radio en Vivo** por boya (`RX`, `RELAY`, `ACK`, `SYS`, `FLASH`).
   - **Estación de Flasheo de Firmware** para compilar y grabar microcontroladores ATmega328P vía serie/FTDI (`avrdude` y `arduino-cli`).
   - Control de acceso y autenticación mediante login de operador.

---

## Estructura del Repositorio

```text
boya/
├── README.md                                  # Documentación general del sistema
├── LICENSE                                    # Licencia MIT (2018)
├── BQS_RECONSTRUCTION_AUDIT.md                # Auditoría técnica de diseño y arquitectura
├── docs/                                      # Especificaciones de ingeniería
│   ├── img/
│   │   ├── bqs_miramar_logo.png               # Logo oficial BQS Miramar
│   │   └── buoy_prototype_2018.jpg            # Fotografía del prototipo físico real
│   ├── ARCHITECTURE.md                        # Diseño de la red en malla y topología
│   ├── BQS_PROTOCOL.md                        # Estructura de la trama binaria de 34 bytes
│   ├── HARDWARE_PINOUT.md                     # Conexiones eléctricas de Arduino y módulos
│   └── FLASHING_GUIDE.md                      # Guía de flasheo y comandos AVRDUDE
│
├── firmware/
│   └── buoy-node/                             # Firmware C++ para Arduino Pro Mini / Nano
│       ├── buoy-node.ino                      # Máquina de estados (Sensado -> GPS -> LoRa -> Sleep)
│       ├── config.h                           # Parámetros de radio (915 MHz), pines y NODE_ID
│       ├── gps/ (gps_tracker.h / .cpp)        # Driver NMEA para receptor u-blox
│       ├── protocol/ (bqs_packet, crc16)      # Serializador binario y comprobación CRC
│       ├── radio/ (lora_transceiver, mesh)    # Capa física SX1276 y enrutador RHMesh
│       ├── sensors/ (dht, water_temp, batt)   # Controladores de sensores ambientales
│       └── storage/ (telemetry_queue)         # Cola circular en memoria RAM local
│
├── gateway/                                   # Software de Estación Costera (Raspberry Pi 3)
│   ├── config.ini                             # Configuración serie y URL de la API
│   ├── requirements.txt                       # Dependencias Python
│   ├── gateway.py                             # Servicio orquestador de recepción
│   └── bqs_gateway/                           # Módulos (decoder, local_spool, uploader, beacon)
│
├── server/                                    # Servidor Central de Telemetría
│   ├── server.py                              # Backend REST API y servidor web standalone
│   ├── api/                                   # Endpoints PHP 7.2 (LAMP Stack alternativo)
│   ├── database/schema.sql                    # Esquema relacional MySQL / SQLite
│   └── web/                                   # Panel interactivo (Bootstrap 4, Leaflet)
│       ├── index.html                         # Mapa, ABM, Logs de Radio y Flasheo
│       └── img/                               # Recursos gráficos y logo
│
└── tools/
    └── flash_buoy_node.py                     # Script de parametrización y flasheo de firmware
```

---

## Puesta en Marcha

### 1. Iniciar el Servidor Central BQS

```bash
# Iniciar el servidor REST y panel web en el puerto 8086
python server/server.py
```

Abrir en el navegador: **`http://localhost:8086`**

**Credenciales de acceso:**
- **Usuario:** `admin`
- **Contraseña:** `admin`

### 2. Iniciar la Estación Costera Gateway (Raspberry Pi)

```bash
# Instalar dependencias
pip install -r gateway/requirements.txt

# Ejecutar el servicio gateway
python gateway/gateway.py
```

### 3. Flasheo de un Nodo Boya

Para parametrizar y flashear una boya con identificador de nodo `#17`:

```bash
# Mediante la herramienta BQS
python tools/flash_buoy_node.py --node-id 17 --port /dev/ttyUSB0 --frequency 915000000

# O directamente con AVRDUDE
avrdude -v -p atmega328p -c arduino -P /dev/ttyUSB0 -b 115200 -D -U flash:w:buoy_node_17.hex:i
```

---

## Especificación de la Trama Binaria (34 Bytes)

| Offset | Tamaño | Campo | Tipo | Descripción |
| :---: | :---: | :--- | :--- | :--- |
| `0x00` | 2 B | **Magic** | `uint16_t` | Identificador de trama (`0xB05A`) |
| `0x02` | 1 B | **Version** | `uint8_t` | Versión de protocolo (`0x01`) |
| `0x03` | 1 B | **Type** | `uint8_t` | Tipo de paquete (`0x01` Telemetría, `0x02` Alerta) |
| `0x04` | 1 B | **Source Node** | `uint8_t` | ID de la boya de origen (`1` a `254`) |
| `0x05` | 1 B | **Dest Node** | `uint8_t` | ID destino (`0x00` para Gateway) |
| `0x06` | 4 B | **Sequence** | `uint32_t` | Número secuencial incremental de paquete |
| `0x0A` | 4 B | **Timestamp** | `uint32_t` | Tiempo Epoch UNIX (segundos) |
| `0x0E` | 4 B | **Latitude** | `int32_t` | Grados de latitud $\times 10^7$ |
| `0x12` | 4 B | **Longitude** | `int32_t` | Grados de longitud $\times 10^7$ |
| `0x16` | 2 B | **Air Temp** | `int16_t` | Temperatura aire en °C $\times 10^1$ |
| `0x18` | 2 B | **Humidity** | `int16_t` | Humedad relativa en % $\times 10^1$ |
| `0x1A` | 2 B | **Water Temp** | `int16_t` | Temperatura agua en °C $\times 10^1$ |
| `0x1C` | 2 B | **Battery mV** | `uint16_t` | Tensión acumulador en milivoltios |
| `0x1E` | 1 B | **Sats / Hops** | `uint8_t` | Satélites (bits 7-4) y Saltos (bits 3-0) |
| `0x1F` | 1 B | **TTL / Flags** | `uint8_t` | TTL restante (bits 7-4) y Flags (bits 3-0) |
| `0x20` | 2 B | **CRC16** | `uint16_t` | CRC-CCITT sobre los 32 bytes anteriores |

---

## Autor

- **Matias Valdivia** (matias.valdivia.9021@gmail.com)
- Miramar / Mar del Plata, Buenos Aires, Argentina

## Licencia

Este proyecto está licenciado bajo los términos de la **Licencia MIT**. Para más detalles, consultar el archivo [LICENSE](LICENSE).
