
#ifndef PACKET_STRUCTS_COMMON_H
#define PACKET_STRUCTS_COMMON_H

#include "zeq_atomic.h"

struct Packet;
struct PacketLegacy;

typedef struct PacketCommon {
    union {
        struct Packet*          nextModern;
        struct PacketLegacy*    nextLegacy;
        struct PacketCommon*    next;
    };
    atomic32_t  refCount;
} PacketCommon;

#endif/*PACKET_STRUCTS_COMMON_H*/

