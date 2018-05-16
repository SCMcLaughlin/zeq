
#include <arpa/inet.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include "packet_alloc.h"
#include "packet_structs.h"
#include "packet_structs_common.h"
#include "packet_structs_legacy.h"
#include "zeq_byte.h"
#include "zeq_err.h"
#include "zeq_expansion.h"

static int packet_write_unsequenced(Packet* packet, uint32_t flags, uint16_t opcode, const byte_t* data, uint32_t len, uint32_t capacity)
{
    uint32_t offset = ZEQ_PACKET_MAX_HEADER_SIZE;
    uint32_t end;
    int isCompress;
    
    /* If our opcode has a leading zero byte (e.g. 0x4200 -> 00 42 in little endian memory)
    ** EQ's network protocol requires that it be prepended with an additional zero byte
    ** (e.g. 00 42 => 00 00 42) in order to make it distinguishable from a protocol opcode
    ** (protocol opcodes always have a leading zero byte).
    **
    ** At the same time, the position of the compression flag, if present, depends on
    ** whether the first byte of the packet is zero. If it is, the compression flag is at
    ** an offset of 2 bytes. Otherwise, it's at an offset of 1 byte. Notice that since
    ** protocol opcodes always begin with a zero byte, only unsequenced packets (which
    ** lack a protocol opcode) will ever have the 1 byte offset.
    **
    ** Everything after the compression flag is subject to compression, including the single 
    ** byte of the opcode that always get split by the compression flag in unsequenced
    ** packets.
    */
    
    if ((flags & ZEQ_PACKET_COMPRESSED_BIT) && len >= ZEQ_PACKET_COMPRESSION_THRESHOLD_SIZE) {
        unsigned long dstlen = (unsigned long)capacity;
        byte_t buf[ZEQ_PACKET_MTU * 2];
        int rc;
        
        /* Unfortunately, we have to do an intermediate copy here.
        ** Only way to avoid this would be to ensure there is one extra byte of padding
        ** at the start of EVERY buffer that would ever be sent as an unsequenced packet.
        */
        /*fixme: might not be unreasonable to do the above*/
        buf[0] = (byte_t)((opcode & 0xff00) >> 8);
        memcpy(&buf[1], data, len);
        rc = compress2(&packet->buffer[offset], &dstlen, buf, len + 1, Z_BEST_COMPRESSION);
        if (rc != Z_OK) return ZEQ_ERR_COMPRESSION;
        end = offset + (uint32_t)dstlen;
        offset--; /*For the first byte of the opcode */
        isCompress = 1;
    } else {
        end = offset + len;
        memcpy(&packet->buffer[offset], data, len);
        packet->buffer[--offset] = (byte_t)((opcode & 0xff00) >> 8);
        isCompress = 0;
    }

    if (flags & ZEQ_PACKET_COMPRESSION_FLAG_BIT)
        packet->buffer[--offset] = (isCompress) ? 'Z' : 0xa5;

    if ((opcode & 0x00ff) == 0) {
        packet->buffer[--offset] = 0;
        packet->buffer[--offset] = 0;
    } else {
        packet->buffer[--offset] = (byte_t)(opcode & 0x00ff);
    }
    
    /* Only thing that remains to be written is the CRC, if applicable. The CRC key is
    ** unique per client, so we can't write it yet.
    */
    
    packet->common.next = NULL;
    packet->common.refCount = 1;
    packet->offset = (uint8_t)offset;
    packet->flags = (uint8_t)flags;
    packet->len = (uint16_t)(end - offset);
    return ZEQ_OK;
}

static void packet_write_header(Packet* packet, uint32_t flags, uint16_t opcode, uint32_t len, uint32_t total)
{
    uint32_t offset = ZEQ_PACKET_MAX_HEADER_SIZE;

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
        /* First packet in a fragment set stores the total size of the fragmented data */
        offset -= ZEQ_PACKET_FIRST_FRAGMENT_TOTAL_LENGTH_SIZE;
        total = htonl(total);
        memcpy(&packet->buffer[offset], &total, ZEQ_PACKET_FIRST_FRAGMENT_TOTAL_LENGTH_SIZE);
    }

    /* Note: we can't write sequence, crc, or do compression until we know what the sequence
    ** value for a specific send will be. Therefore, handling those things is the responsibility
    ** of the network thread.
    */

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
    int rc;
    
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

        if ((flags & ZEQ_PACKET_SEQUENCED_BIT) == 0) {
            rc = packet_write_unsequenced(top, flags, opcode, data, dataLength, fragThreshold);
            if (rc) goto fail;
        } else {
            memcpy(&top->buffer[ZEQ_PACKET_MAX_HEADER_SIZE], data, dataLength);
            packet_write_header(top, flags, opcode, dataLength, 0);
        }
    } else {
        Packet* prev = NULL;
        uint32_t i;

        for (i = 0; i < count; i++) {
            Packet* packet = (Packet*)malloc(ZEQ_PACKET_ALLOC_SIZE);
            uint32_t flag = flags;
            uint32_t len;

            if (!packet) {
                rc = ZEQ_ERR_MEMORY;
                goto fail;
            }
            
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
    
fail:
    while (top) {
        Packet* next = top->common.nextModern;
        free(top);
        top = next;
    }
    return rc;
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

