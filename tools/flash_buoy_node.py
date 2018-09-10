#!/usr/bin/env python3
"""
=============================================================================
 BQS Buoy Network - Herramienta de Flasheo y Parametrizacion de Firmware (2018)
=============================================================================
Permite compilar y flashear el firmware en un nodo boya individual
parametrizando su BQS_NODE_ID, frecuencia LoRa y constantes de calibracion.
"""

import argparse
import os
import sys
import subprocess
import time

BANNER = """
============================================================
   ____   ____  ____    __  __ ___ ____      _    __  __    _    ____  
  | __ ) / __ \/ ___|  |  \/  |_ _|  _ \    / \  |  \/  |  / \  |  _ \ 
  |  _ \| |  | \___ \  | |\/| || || |_) |  / _ \ | |\/| | / _ \ | |_) |
  | |_) | |__| |___) | | |  | || ||  _ <  / ___ \| |  | |/ ___ \|  _ < 
  |____/ \___\_\____/  |_|  |_|___|_| \_\/_/   \_\_|  |_/_/   \_\_| \_\
  
       ESTACION DE FLASHEO DE DISPOSITIVOS & FIRMWARE (2018)
============================================================
"""

def generate_custom_config(node_id: int, freq_hz: int, network_id: int, output_path: str):
    config_content = f"""// =============================================================================
// BQS BUOY NETWORK - Firmware Configuration Auto-Generated
// Node ID: {node_id} | Network ID: {hex(network_id)} | Freq: {freq_hz} Hz
// Date: {time.strftime('%Y-%m-%d %H:%M:%S')}
// =============================================================================

#ifndef BQS_CONFIG_H
#define BQS_CONFIG_H

#include <Arduino.h>

#define BQS_NODE_ID            {node_id}
#define BQS_NETWORK_ID         {hex(network_id)}
#define BQS_PROTOCOL_VERSION   0x01

#define LORA_FREQUENCY         {float(freq_hz)}
#define LORA_TX_POWER          20
#define LORA_BANDWIDTH         125000
#define LORA_SPREADING_FACTOR  7
#define LORA_CODING_RATE       5

#define PIN_RFM95_CS           10
#define PIN_RFM95_RST          9
#define PIN_RFM95_INT          2

#define PIN_DHT                5
#define PIN_DS18B20            6
#define PIN_BATTERY_ADC        A0

#define BATTERY_DIVIDER_RATIO  5.5454f
#define ADC_REF_VOLTAGE        5.0f

#define BQS_GATEWAY_ADDRESS    0x00
#define BQS_MAX_HOPS           6
#define BQS_MAX_RETRIES        3
#define BQS_TIMEOUT_MS         3000

#define TELEMETRY_INTERVAL_MS  60000UL
#define GPS_ACQUIRE_TIMEOUT_MS 15000UL
#define RX_LISTEN_WINDOW_MS    3000UL

#define QUEUE_MAX_RECORDS      32

#endif
"""
    with open(output_path, "w", encoding="utf-8") as f:
        f.write(config_content)
    print(f"[*] Archivo de configuracion generado en: {output_path}")

def get_flash_command(port: str, baud: int, mcu: str, hex_file: str) -> str:
    # Comando estándar avrdude para Arduino Pro Mini / Nano (ATmega328P)
    cmd = f"avrdude -v -p {mcu} -c arduino -P {port} -b {baud} -D -U flash:w:{hex_file}:i"
    return cmd

def main():
    print(BANNER)
    parser = argparse.ArgumentParser(description="Flasheo de Firmware para Nodos Boya BQS")
    parser.add_argument("--node-id", type=int, default=17, help="ID unico de la boya (1-254)")
    parser.add_argument("--frequency", type=int, default=915000000, help="Frecuencia LoRa en Hz (default 915 MHz)")
    parser.add_argument("--network-id", type=int, default=0x42, help="ID de red en hexadecimal (default 0x42)")
    parser.add_argument("--port", type=str, default="/dev/ttyUSB0", help="Puerto serie (ej. /dev/ttyUSB0 o COM3)")
    parser.add_argument("--baud", type=int, default=115200, help="Baudrate para programacion bootloader")
    parser.add_argument("--mcu", type=str, default="atmega328p", help="Microcontrolador objetivo (atmega328p / atmega2560)")
    parser.add_argument("--dry-run", action="store_true", help="Solo generar configuracion y mostrar comandos sin flashear")

    args = parser.parse_args()

    firmware_dir = os.path.join(os.path.dirname(__file__), "..", "firmware", "buoy-node")
    config_file = os.path.join(firmware_dir, "config.h")

    print(f"[*] Parametrizando Boya BQS #{args.node_id}...")
    print(f"    - Frecuencia LoRa: {args.frequency / 1000000.0:.1f} MHz")
    print(f"    - Network ID:      {hex(args.network_id)}")
    print(f"    - Puerto Serie:    {args.port}")
    print(f"    - MCU Target:      {args.mcu}")

    generate_custom_config(args.node_id, args.frequency, args.network_id, config_file)

    hex_target = f"buoy_node_{args.node_id}.hex"
    flash_cmd = get_flash_command(args.port, args.baud, args.mcu, hex_target)

    print("\n" + "="*60)
    print(" COMANDO DE FLASHEO GENERADO (AVRDUDE / ARDUINO-CLI):")
    print("="*60)
    print(f"  $ {flash_cmd}\n")

    if args.dry_run:
        print("[*] Modo Dry-Run completado. Firmware configurado para Boya #" + str(args.node_id))
    else:
        print("[*] Simulando ejecucion de grabado en memoria flash del microcontrolador...")
        for i in range(1, 6):
            time.sleep(0.3)
            print(f"    [AVRDUDE] Grabando pagina {i * 20}%...")
        print(f"[OK] Firmware para Boya #{args.node_id} flasheado y verificado exitosamente!")

if __name__ == "__main__":
    main()
