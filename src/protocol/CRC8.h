/*  CRC8.h  --  CRC-8/SMBUS (polynomial 0x07, init 0x00)
 *
 *  Byte-for-byte the algorithm and lookup table from the RS485 protocol
 *  spec so the hub is interoperable with the reference implementation.
 */

#ifndef CONTROLHUB_CRC8_H
#define CONTROLHUB_CRC8_H

#include <Arduino.h>

namespace ControlHubProtocol {

/**
 * @brief Compute CRC-8/SMBUS over @p buf.
 * @param buf pointer to bytes
 * @param len number of bytes
 * @return    CRC value (0x00 initial, polynomial 0x07)
 */
uint8_t crc8_smbus(const uint8_t *buf, uint16_t len);

} // namespace

#endif
