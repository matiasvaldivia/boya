#ifndef BQS_CRC16_H
#define BQS_CRC16_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Calcula el CRC-16 CCITT (polinomio 0x1021, valor inicial 0xFFFF).
 */
uint16_t bqs_crc16(const uint8_t *data, size_t length);

#ifdef __cplusplus
}
#endif

#endif
