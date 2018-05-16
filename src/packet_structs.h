
#ifndef PACKET_STRUCTS_H
#define PACKET_STRUCTS_H

#include <stdint.h>
#include "packet_structs_common.h"
#include "zeq_atomic.h"
#include "zeq_byte.h"
#include "zeq_platform.h"

#ifdef ZEQ_COMPILER_MSVC
# pragma warning(disable: 4200)
#endif

#define ZEQ_PACKET_MTU 512 /* Does not count inherent UDP overhead */
#define ZEQ_PACKET_CRC_SIZE 2
#define ZEQ_PACKET_PROTOCOL_OPCODE_SIZE 2
#define ZEQ_PACKET_SEQUENCE_SIZE 2
#define ZEQ_PACKET_FIRST_FRAGMENT_TOTAL_LENGTH_SIZE 4
#define ZEQ_PACKET_OPCODE_SIZE 2
#define ZEQ_PACKET_OPCODE_EXTENDED_SIZE 3
#define ZEQ_PACKET_COMPRESSION_THRESHOLD_SIZE 40
#define ZEQ_PACKET_COMPRESSION_FLAG_SIZE 1
#define ZEQ_PACKET_COMBINED_LENGTH_FIELD_SIZE 1
/* Technically, a header could never have *all* of these fields at once, but it's simpler to allow for them all */
#define ZEQ_PACKET_MAX_HEADER_SIZE (ZEQ_PACKET_PROTOCOL_OPCODE_SIZE + ZEQ_PACKET_COMPRESSION_FLAG_SIZE + \
                                    ZEQ_PACKET_COMBINED_LENGTH_FIELD_SIZE + ZEQ_PACKET_SEQUENCE_SIZE + \
                                    ZEQ_PACKET_FIRST_FRAGMENT_TOTAL_LENGTH_SIZE + ZEQ_PACKET_OPCODE_EXTENDED_SIZE)
#define ZEQ_PACKET_DECOMPRESS_BUFFER_SIZE (ZEQ_PACKET_MTU * 2)

#define ZEQ_PACKET_SEQUENCED_BIT (1 << 0)
#define ZEQ_PACKET_COMPRESSION_FLAG_BIT (1 << 1)
#define ZEQ_PACKET_COMPRESSED_BIT (1 << 2)
#define ZEQ_PACKET_CRC_BIT (1 << 3)
#define ZEQ_PACKET_FRAGMENTED_BIT (1 << 4)
#define ZEQ_PACKET_FIRST_FRAGMENT_BIT (1 << 5)

typedef struct Packet Packet;
struct Packet {
    PacketCommon    common;
    uint8_t         offset; /* For sequenced packets, the offset after the sequence field; otherwise, offset of whole packet data */
    uint8_t         flags;
    uint16_t        len; /* Includes opcode and first fragment length, but not footer (CRC) */
    byte_t          buffer[0];
};

#define ZEQ_PACKET_ALLOC_SIZE (sizeof(Packet) + ZEQ_PACKET_MTU + ZEQ_PACKET_MAX_HEADER_SIZE)

#endif/*PACKET_STRUCTS_H*/
