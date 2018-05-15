
#ifndef CRC_H
#define CRC_H

#include <stdint.h>

uint16_t crc16(const void* data, uint32_t len, uint32_t key);

#endif/*CRC_H*/

