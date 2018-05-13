
#ifndef PACKET_STRUCTS_LEGACY_H
#define PACKET_STRUCTS_LEGACY_H

#include <stdint.h>
#include "packet_structs_common.h"
#include "zeq_atomic.h"
#include "zeq_byte.h"
#include "zeq_platform.h"

#ifdef ZEQ_COMPILER_MSVC
# pragma warning(disable: 4200)
#endif

#define ZEQ_PACKET_LEGACY_DATA_MTU 512 /* Does not count any overhead except opcode */
#define ZEQ_PACKET_LEGACY_CRC_SIZE 4
#define ZEQ_PACKET_LEGACY_FLAGS_SIZE 2
#define ZEQ_PACKET_LEGACY_SEQUENCE_SIZE 2
#define ZEQ_PACKET_LEGACY_ACK_SIZE 2
#define ZEQ_PACKET_LEGACY_ACK_FIELDS_SIZE (ZEQ_PACKET_LEGACY_ACK_SIZE * 2)
#define ZEQ_PACKET_LEGACY_FRAG_FIELDS_SIZE 6
#define ZEQ_PACKET_LEGACY_COUNTER_FIELDS_SIZE 2
#define ZEQ_PACKET_LEGACY_OPCODE_SIZE 2
/* Technically, a header could never have *all* of these fields at once, but it's simpler to allow for them all */
#define ZEQ_PACKET_LEGACY_MAX_HEADER_SIZE (ZEQ_PACKET_LEGACY_FLAGS_SIZE + ZEQ_PACKET_LEGACY_SEQUENCE_SIZE + \
                                           ZEQ_PACKET_LEGACY_ACK_FIELDS_SIZE + ZEQ_PACKET_LEGACY_FRAG_FIELDS_SIZE + \
                                           ZEQ_PACKET_LEGACY_COUNTER_FIELDS_SIZE + ZEQ_PACKET_LEGACY_OPCODE_SIZE)
#define ZEQ_PACKET_LEGACY_MAX_OVERHEAD_SIZE (ZEQ_PACKET_LEGACY_MAX_HEADER_SIZE + ZEQ_PACKET_LEGACY_CRC_SIZE)

typedef struct PacketLegacy PacketLegacy;
struct PacketLegacy {
    PacketCommon    common;
    uint8_t         offset;
    uint8_t         flags;
    uint16_t        len;
    uint16_t        fragIndex;
    uint16_t        fragCount;
    byte_t          buffer[0];
};

#define ZEQ_PACKET_LEGACY_ALLOC_SIZE (sizeof(PacketLegacy) + ZEQ_PACKET_LEGACY_DATA_MTU + ZEQ_PACKET_LEGACY_MAX_OVERHEAD_SIZE)

#endif/*PACKET_STRUCTS_LEGACY_H*/

