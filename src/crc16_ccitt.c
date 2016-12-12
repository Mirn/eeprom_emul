/*
 * crc16_ccitt.c
 *
 *  Created on: 12 дек. 2016 г.
 *      Author: Easy
 */

#include "crc16_ccitt.h"

/*Name  : CRC-16 CCITT
  Poly  : 0x1021    x^16 + x^12 + x^5 + 1
  Init  : 0xFFFF
  Revert: false
  XorOut: 0x0000
  Check : 0x29B1 ("123456789")
  MaxLen: 4095 байт (32767 бит) - обнаружение одинарных, двойных, тройных и всех нечетных ошибок
*/
uint16_t Crc16_CCITT(uint16_t crc, const uint8_t *pcBlock, size_t len)
{
    while (len--)
    {
        crc ^= *(pcBlock++) << 8;

        uint32_t i;
        for (i = 0; i < 8; i++)
            crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
    }
    return crc;
}

