
#include <stdint.h>
#include <string.h>
#include "packet_structs_legacy.h"
#include "zeq_alloc.h"
#include "zeq_atomic.h"
#include "zeq_byte.h"
#include "zeq_err.h"

int packet_alloc_legacy(PacketLegacy** out, uint16_t opcode, const void* vdata, uint32_t dataLength)
{
    const byte* data = (const byte*)vdata;
    PacketLegacy* top = NULL;
    uint32_t count = 1;
    uint32_t total = dataLength + ZEQ_PACKET_LEGACY_OPCODE_SIZE;

    /* Except for the opcode, no overhead is counted toward the MTU for legacy packets.
    ** For fragmented packets, it's important to use as much of the MTU space as possible;
    ** the client will reject non-final fragment packets that don't have a full MTU worth of data.
    */

    if (total > ZEQ_PACKET_LEGACY_DATA_MTU) {
        count = total / ZEQ_PACKET_LEGACY_DATA_MTU;

        if ((total % ZEQ_PACKET_LEGACY_DATA_MTU) != 0)
            count++;
    }

    if (count == 1) {
        top = (PacketLegacy*)zeq_malloc(ZEQ_PACKET_LEGACY_ALLOC_SIZE);
        if (!top) return ZEQ_ERR_MEMORY;

        top->common.next = NULL;
        top->common.refCount = 1;
        top->offset = ZEQ_PACKET_LEGACY_MAX_HEADER_SIZE; /*fixme: if offset and flags are constant, remove*/
        top->flags = 0;
        top->len = (uint16_t)total;
        top->fragIndex = 0;
        top->fragCount = 0;

        memcpy(&top->buffer[ZEQ_PACKET_LEGACY_MAX_HEADER_SIZE], &opcode, ZEQ_PACKET_LEGACY_OPCODE_SIZE);
        memcpy(&top->buffer[ZEQ_PACKET_LEGACY_MAX_HEADER_SIZE + ZEQ_PACKET_LEGACY_OPCODE_SIZE], data, dataLength);
    } else {
        PacketLegacy* prev = NULL;
        uint32_t i;

        for (i = 0; i < count; i++) {
            PacketLegacy* packet = (PacketLegacy*)zeq_malloc(ZEQ_PACKET_LEGACY_ALLOC_SIZE);
            uint32_t offset = ZEQ_PACKET_LEGACY_MAX_HEADER_SIZE;
            uint32_t len;

            if (!packet) goto oom;

            if (i == 0) {
                len = ZEQ_PACKET_LEGACY_DATA_MTU - ZEQ_PACKET_LEGACY_OPCODE_SIZE;
                top = packet;
                offset += ZEQ_PACKET_LEGACY_OPCODE_SIZE;
                memcpy(&packet->buffer[ZEQ_PACKET_LEGACY_MAX_HEADER_SIZE], &opcode, ZEQ_PACKET_LEGACY_OPCODE_SIZE);
            } else {
                prev->common.nextLegacy = packet;

                if (i == (count - 1)) {
                    len = dataLength % ZEQ_PACKET_LEGACY_DATA_MTU;
                    if (len == 0) goto max_mtu;
                } else {
                max_mtu:
                    len = ZEQ_PACKET_LEGACY_DATA_MTU;
                }
            }

            packet->common.next = NULL;
            packet->common.refCount = 1;
            packet->offset = ZEQ_PACKET_LEGACY_MAX_HEADER_SIZE;
            packet->flags = 0;
            packet->len = (uint16_t)len;
            packet->fragIndex = (uint16_t)i;
            packet->fragCount = (uint16_t)count;

            memcpy(&packet->buffer[offset], data, len);
            data += len;
            prev = packet;
        }
    }

    *out = top;
    return ZEQ_OK;
 
oom:
    while (top) {
        PacketLegacy* next = top->common.nextLegacy;
        free(top);
        top = next;
    }
    return ZEQ_ERR_MEMORY;
}

