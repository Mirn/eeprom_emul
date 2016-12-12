/*
 * crc16_ccitt.h
 *
 *  Created on: 12 дек. 2016 г.
 *      Author: Easy
 */

#ifndef CRC16_CCITT_H_
#define CRC16_CCITT_H_

#include "stdint.h"
#include "stdlib.h"

uint16_t Crc16_CCITT(uint16_t crc, const uint8_t *pcBlock, size_t len);

#endif /* CRC16_CCITT_H_ */
