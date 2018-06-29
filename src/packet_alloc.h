
#ifndef PACKET_ALLOC_H
#define PACKET_ALLOC_H

#include <stdint.h>
#include <string.h>
#include "zeq_def.h"
#include "zeq_expansion.h"

struct Packet;
struct PacketLegacy;
struct PacketCommon;

typedef struct PacketSet {
    struct PacketLegacy*    legacy[ZEQ_EXPANSION_LEGACY_COUNT];
    struct Packet*          modern[ZEQ_EXPANSION_MODERN_COUNT];
} PacketSet;

ZEQ_INTERFACE int packet_alloc(struct Packet** out, uint32_t flags, uint16_t opcode, const void* vdata, uint32_t dataLength);
ZEQ_INTERFACE int packet_alloc_legacy(struct PacketLegacy** out, uint16_t opcode, const void* vdata, uint32_t dataLength);

ZEQ_INTERFACE void packet_grab(struct PacketCommon* packet);
ZEQ_INTERFACE void packet_drop(struct PacketCommon* packet);
ZEQ_INTERFACE void packet_grab_all(struct PacketCommon* packet);
ZEQ_INTERFACE void packet_drop_all(struct PacketCommon* packet);

#define packet_set_init(set) memset((set), 0, sizeof(PacketSet))
ZEQ_INTERFACE void packet_set_drop_all(PacketSet* set);

#endif/*PACKET_ALLOC_H*/

