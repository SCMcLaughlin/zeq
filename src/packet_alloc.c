
#include <arpa/inet.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "packet_alloc.h"
#include "packet_structs.h"
#include "packet_structs_common.h"
#include "packet_structs_legacy.h"
#include "zeq_byte.h"
#include "zeq_err.h"
#include "zeq_expansion.h"

static void packet_write_header(Packet* packet, uint32_t flags, uint16_t opcode, uint32_t len, uint32_t total)
{
    uint32_t offset = ZEQ_PACKET_MAX_HEADER_SIZE;

    /* Special case: unsequenced packet */
    if ((flags & ZEQ_PACKET_SEQUENCED_BIT) == 0) {
        int padded = ((opcode & 0x00ff) == 0);
        
        /* If our opcode has a leading zero byte (e.g. 0x4200 -> 00 42 in little endian memory)
        ** EQ's network protocol requires that it be prepended with an additional zero byte
        ** (e.g. 00 42 => 00 00 42) in order to make it distinguishable from a protocol opcode
        ** (protocol opcodes always have a leading zero byte).
        **
        ** At the same time, the compression flag, if present, is always at a fixed offset of
        ** 2 bytes into the packet. When these requirements collide, the compression flag
        ** splits the extended 3-byte opcode.
        */

        if (padded)
            packet->buffer[--offset] = (byte_t)((opcode & 0xff00) >> 8);

        if (flags & ZEQ_PACKET_COMPRESSION_FLAG_BIT)
            packet->buffer[--offset] = 0xa5;

        if (padded) {
            packet->buffer[--offset] = 0;
            packet->buffer[--offset] = 0;
        } else {
            offset -= ZEQ_PACKET_OPCODE_SIZE;
            memcpy(&packet->buffer[offset], &opcode, ZEQ_PACKET_OPCODE_SIZE);
        }

        goto done;
    }

    /* General case */
    if ((flags & ZEQ_PACKET_FRAGMENTED_BIT) == 0 || flags & ZEQ_PACKET_FIRST_FRAGMENT_BIT) {
        /* Non-fragmented packets and the first packet in a fragment set need the opcode. 
        ** See above for why we need to pad certain opcodes with an extra zero byte.
        */
        if ((opcode & 0x00ff) == 0) {
            packet->buffer[--offset] = (byte_t)((opcode & 0xff00) >> 8);
            packet->buffer[--offset] = 0;
            packet->buffer[--offset] = 0;
        } else {
            offset -= ZEQ_PACKET_OPCODE_SIZE;
            memcpy(&packet->buffer[offset], &opcode, ZEQ_PACKET_OPCODE_SIZE);
        }
    }

    if (flags & ZEQ_PACKET_FIRST_FRAGMENT_BIT) {
        offset -= ZEQ_PACKET_FIRST_FRAGMENT_TOTAL_LENGTH_SIZE;
        total = htonl(total);
        memcpy(&packet->buffer[offset], &total, ZEQ_PACKET_FIRST_FRAGMENT_TOTAL_LENGTH_SIZE);
    }

    /* Note: we can't write sequence, crc, or do compression until we know what the sequence
    ** value for a specific send will be. Therefore, handling those things is the responsibility
    ** of the network thread.
    */

done:
    packet->common.next = NULL;
    packet->common.refCount = 1;
    packet->offset = (uint8_t)offset;
    packet->flags = (uint8_t)flags;
    packet->len = (uint16_t)(len + (ZEQ_PACKET_MAX_HEADER_SIZE - offset));
}

int packet_alloc(Packet** out, uint32_t flags, uint16_t opcode, const void* vdata, uint32_t dataLength)
{
    const byte_t* data = (const byte_t*)vdata;
    Packet* top = NULL;
    uint32_t overhead;
    uint32_t fragThreshold;
    uint32_t crcSize;
    uint32_t opcodeSize;
    uint32_t count = 1;
    uint32_t first;
    
    opcodeSize = ((opcode & 0x00ff) == 0) ? ZEQ_PACKET_OPCODE_EXTENDED_SIZE : ZEQ_PACKET_OPCODE_SIZE;
    crcSize = (flags & ZEQ_PACKET_CRC_BIT) ? ZEQ_PACKET_CRC_SIZE : 0;
    overhead = opcodeSize + crcSize;

    if (flags & ZEQ_PACKET_COMPRESSION_FLAG_BIT)
        overhead += ZEQ_PACKET_COMPRESSION_FLAG_SIZE;
    
    if (flags & ZEQ_PACKET_SEQUENCED_BIT || dataLength > (ZEQ_PACKET_MTU - overhead)) {
        flags |= ZEQ_PACKET_SEQUENCED_BIT;
        overhead += ZEQ_PACKET_PROTOCOL_OPCODE_SIZE + ZEQ_PACKET_SEQUENCE_SIZE;
        
        if (flags & ZEQ_PACKET_COMPRESSION_FLAG_BIT)
            flags |= ZEQ_PACKET_COMPRESSED_BIT;
    }
    
    fragThreshold = ZEQ_PACKET_MTU - overhead;
    if (dataLength > fragThreshold) {
        flags |= ZEQ_PACKET_FRAGMENTED_BIT;
        first = fragThreshold - ZEQ_PACKET_FIRST_FRAGMENT_TOTAL_LENGTH_SIZE;
        fragThreshold += opcodeSize;
        count += (dataLength - first) / fragThreshold;
        
        if ((dataLength - first) % fragThreshold != 0)
            count++;
    }
    
    if (count == 1) {
        top = (Packet*)malloc(ZEQ_PACKET_ALLOC_SIZE);
        if (!top) return ZEQ_ERR_MEMORY;

        memcpy(&top->buffer[ZEQ_PACKET_MAX_HEADER_SIZE], data, dataLength);
        packet_write_header(top, flags, opcode, dataLength, dataLength);       
    } else {
        Packet* prev = NULL;
        uint32_t i;

        for (i = 0; i < count; i++) {
            Packet* packet = (Packet*)malloc(ZEQ_PACKET_ALLOC_SIZE);
            uint32_t flag = flags;
            uint32_t len;

            if (!packet) goto oom;
            
            if (i == 0) {
                flag |= ZEQ_PACKET_FIRST_FRAGMENT_BIT;
                len = first;
                top = packet;
            } else {
                prev->common.nextModern = packet;

                if (i == (count - 1)) {
                    len = (dataLength - first) % fragThreshold;
                    if (len == 0) goto max_mtu;
                } else {
                max_mtu:
                    len = fragThreshold;
                }
            }

            memcpy(&packet->buffer[ZEQ_PACKET_MAX_HEADER_SIZE], data, len);
            data += len;
            packet_write_header(packet, flag, opcode, len, dataLength);
            prev = packet;
        }
    }
    
    *out = top;
    return ZEQ_OK;
    
oom:
    while (top) {
        Packet* next = top->common.nextModern;
        free(top);
        top = next;
    }
    return ZEQ_ERR_MEMORY;
}

void packet_grab(PacketCommon* packet)
{
    atomic32_add(&packet->refCount, 1);
}

void packet_drop(PacketCommon* packet)
{
    if (atomic32_sub(&packet->refCount, 1) <= 0)
        free(packet);
}

void packet_grab_all(PacketCommon* packet)
{
    while (packet) {
        packet_grab(packet);
        packet = packet->next;
    }
}

void packet_drop_all(PacketCommon* packet)
{
    while (packet) {
        PacketCommon* next = packet->next;
        packet_drop(packet);
        packet = next;
    }
}

void packet_set_drop_all(PacketSet* set)
{
    int i;

    for (i = 0; i < ZEQ_EXPANSION_LEGACY_COUNT; i++) {
        PacketLegacy* packet = set->legacy[i];
        if (packet) packet_drop_all(&packet->common);       
    }

    for (i = 0; i < ZEQ_EXPANSION_MODERN_COUNT; i++) {
        Packet* packet = set->modern[i];
        if (packet) packet_drop_all(&packet->common);
    }
}

