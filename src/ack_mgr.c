
#include <arpa/inet.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include "ack.h"
#include "ack_mgr.h"
#include "crc.h"
#include "net_protocol.h"
#include "packet_alloc.h"
#include "packet_structs.h"
#include "zeq_err.h"

#define ZEQ_ACK_PACKET_COMBINE_SIZE 5 /* size + protocol opcode + seq */
#define ZEQ_ACK_PACKET_ACK_COMBINE_SIZE ZEQ_ACK_PACKET_COMBINE_SIZE
#define ZEQ_ACK_PACKET_COMPRESSION_LEVEL Z_BEST_COMPRESSION

void ack_mgr_init(AckMgr* mgr)
{
    memset(mgr, 0, sizeof(AckMgr));
    mgr->lastAckRecvd = UINT16_MAX;
}

void ack_mgr_deinit(AckMgr* mgr)
{
    if (mgr->sendQueue)
        free(mgr->sendQueue);
}

static uint32_t send_seq_to_index(AckMgr* mgr, uint16_t seq)
{
    uint16_t base = mgr->sendSeqFirst;

    if (seq < base) {
        /* E.g. seq = 10, base = 65530
        ** index = (65536 - 65530) + 10 = 16
        */
        return ((UINT16_MAX + 1) - (uint32_t)base) + seq;
    }

    return seq - base;
}

#include <stdio.h>
static int send_packet_bytes(byte_t* data, uint32_t len)
{
    uint32_t i;
    for (i = 0; i < len; i++) {
        if (i && i%16 == 0)
            fputc('\n', stdout);
        printf("%02x ", data[i]);
    }
    fputc('\n', stdout);
    return ZEQ_OK;
}

static int send_fragment(Packet* packet, uint16_t seq, uint32_t crcKey)
{
    uint32_t offset = packet->offset;
    uint32_t end = offset + packet->len;
    int isCompress = 0;
    byte_t* buffer;
    byte_t compressBuf[ZEQ_PACKET_MTU * 2];
    uint16_t write;

    offset -= ZEQ_PACKET_SEQUENCE_SIZE;
    write = htons(seq++);
    memcpy(&packet->buffer[offset], &write, ZEQ_PACKET_SEQUENCE_SIZE);

    if (packet->flags & ZEQ_PACKET_COMPRESSION_FLAG_BIT && (end - offset) >= ZEQ_PACKET_COMPRESSION_THRESHOLD_SIZE)
        isCompress = 1;

    if (isCompress) {
        unsigned long dstlen = (long)(sizeof(compressBuf) - offset);
        int rc = compress2(&compressBuf[offset], &dstlen, &packet->buffer[offset], (long)(end - offset), ZEQ_ACK_PACKET_COMPRESSION_LEVEL);
        if (rc != Z_OK) return ZEQ_ERR_COMPRESSION;
        end = offset + (uint32_t)dstlen;
        buffer = compressBuf;
    } else {
        buffer = packet->buffer;
    }

    if (packet->flags & ZEQ_PACKET_CRC_BIT) {
        uint16_t crc = htons(crc16(&buffer[offset], end - offset, crcKey));
        memcpy(&buffer[end], &crc, ZEQ_PACKET_CRC_SIZE);
        end += ZEQ_PACKET_CRC_SIZE;
    }

    if (packet->flags & ZEQ_PACKET_COMPRESSION_FLAG_BIT)
        buffer[--offset] = (isCompress) ? 'Z' : 0xa5;

    offset -= ZEQ_PACKET_PROTOCOL_OPCODE_SIZE;
    write = NET_PROTOCOL_OP_FRAGMENT;
    memcpy(&buffer[offset], &write, ZEQ_PACKET_PROTOCOL_OPCODE_SIZE);

    /* Send */
    send_packet_bytes(&buffer[offset], (end - offset));
    return (int)seq;
}

static uint16_t send_combined(Packet* first, uint16_t seq, uint16_t ack, uint32_t combineOverhead, int* hasAck, uint32_t crcKey)
{
    uint32_t offset = first->offset;
    uint32_t end = offset + first->len;
    int isCompress = 0;
    int isCombined = 1;
    byte_t* buffer;
    byte_t compressBuf[ZEQ_PACKET_MTU * 2];
    uint16_t write;
    
    /* Write the first packet's sequence */
    offset -= ZEQ_PACKET_SEQUENCE_SIZE;
    write = htons(seq++);
    memcpy(&first->buffer[offset], &write, ZEQ_PACKET_SEQUENCE_SIZE);
    
    if ((end - offset) > UINT8_MAX || (!first->common.nextModern && *hasAck == 0)) {
        isCombined = 0;
    } else {
        Packet* current;
        uint8_t size;
        
        /* Write size for the first packet */
        size = (uint8_t)(end - offset);
        first->buffer[--offset] = size;
        
        /* Combine subsequent packets */
        current = first->common.nextModern;
        while (current) {
            /* Write the next packet's size, sequence and data */
            size = (uint8_t)(current->len + ZEQ_PACKET_SEQUENCE_SIZE);
            first->buffer[end++] = size;
            write = htons(seq++);
            memcpy(&first->buffer[end], &write, ZEQ_PACKET_SEQUENCE_SIZE);
            end += ZEQ_PACKET_SEQUENCE_SIZE;
            memcpy(&first->buffer[end], &current->buffer[current->offset], current->len);
            end += current->len;
            current = current->common.nextModern;
        }
        
        /* Combine ack if we have one and we still have space */
        if (*hasAck && ((end - offset) + combineOverhead + ZEQ_ACK_PACKET_ACK_COMBINE_SIZE) <= ZEQ_PACKET_MTU) {
            *hasAck = 0;
            first->buffer[end++] = ZEQ_PACKET_PROTOCOL_OPCODE_SIZE + ZEQ_PACKET_SEQUENCE_SIZE;
            write = NET_PROTOCOL_OP_ACK;
            memcpy(&first->buffer[end], &write, ZEQ_PACKET_PROTOCOL_OPCODE_SIZE);
            end += ZEQ_PACKET_PROTOCOL_OPCODE_SIZE;
            write = htons(ack);
            memcpy(&first->buffer[end], &write, ZEQ_PACKET_SEQUENCE_SIZE);
            end += ZEQ_PACKET_SEQUENCE_SIZE;
        }
    }

    if (first->flags & ZEQ_PACKET_COMPRESSION_FLAG_BIT && (end - offset) >= ZEQ_PACKET_COMPRESSION_THRESHOLD_SIZE)
        isCompress = 1;
    
    if (isCompress) {
        unsigned long dstlen = (long)(sizeof(compressBuf) - offset);
        int rc = compress2(&compressBuf[offset], &dstlen, &first->buffer[offset], (long)(end - offset), Z_BEST_COMPRESSION);
        if (rc != Z_OK) return ZEQ_ERR_COMPRESSION;
        end = offset + (uint32_t)dstlen;
        buffer = compressBuf;
    } else {
        buffer = first->buffer;
    }

    if (first->flags & ZEQ_PACKET_CRC_BIT) {
        uint16_t crc = htons(crc16(&buffer[offset], end - offset, crcKey));
        memcpy(&buffer[end], &crc, ZEQ_PACKET_CRC_SIZE);
        end += ZEQ_PACKET_CRC_SIZE;
    }

    if (first->flags & ZEQ_PACKET_COMPRESSION_FLAG_BIT)
        buffer[--offset] = (isCompress) ? 'Z' : 0xa5;

    offset -= ZEQ_PACKET_PROTOCOL_OPCODE_SIZE;
    write = (isCombined) ? NET_PROTOCOL_OP_COMBINED : NET_PROTOCOL_OP_PACKET;
    memcpy(&buffer[offset], &write, ZEQ_PACKET_PROTOCOL_OPCODE_SIZE);

    /* Send */
    send_packet_bytes(&buffer[offset], (end - offset));
    return (int)seq;
}

static int send_queued_advance(AckMgr* mgr, uint32_t index, uint16_t seq)
{
    Packet* first = mgr->sendQueue[index];
    Packet* prev = NULL;
    uint16_t nextAck = mgr->sendAckNext;
    int hasAck = (nextAck != mgr->sendAckPrev);
    uint32_t combinedOverhead = ZEQ_PACKET_PROTOCOL_OPCODE_SIZE;
    uint32_t end = send_seq_to_index(mgr, mgr->sendSeqLast);
    uint32_t len = 0;
    uint32_t i;

    if (first->flags & ZEQ_PACKET_COMPRESSION_FLAG_BIT)
        combinedOverhead += ZEQ_PACKET_COMPRESSION_FLAG_SIZE;

    if (first->flags & ZEQ_PACKET_CRC_BIT)
        combinedOverhead += ZEQ_PACKET_CRC_SIZE;
    
    first = NULL;

    /* We want to opportunistically combine as many packets together as possible.
    ** At the same time, if we have an ack to send, we want to combine that in as well.
    **
    ** This makes things a bit complicated:
    **   1) If the current packet has a length greater than 256 after writing the seq,
    **      it can't be combined with anything.
    **   2) If the current packet is small enough to be combined, we won't know if it
    **      actually can be combined until we examine the next packet. So we can't write
    **      the first packet's size, or the entire packet's compression flag and protocol
    **      opcode, until we look at the second packet.
    **   3) Fragment packets can never be combined, as they must have the fragment opcode,
    **      not the combined packet opcode.
    */
    for (i = index; i < end; i++) {
        Packet* packet = mgr->sendQueue[i];
        uint32_t curlen;
        int hadAck;
        int rc;
        
        if (packet->flags & ZEQ_PACKET_FRAGMENTED_BIT) {
            /* If we have a previous or combined packet queued up, send it first */
            if (first) {
                hadAck = hasAck;
                rc = send_combined(first, seq, nextAck, combinedOverhead, &hasAck, mgr->crcKey);
                if (rc < 0) return rc;
                seq = (uint16_t)rc;
                if (hasAck != hadAck)
                    mgr->sendAckPrev = nextAck;
                len = 0;
                first = NULL;
                prev = NULL;
            }
            
            /* Send the current fragment packet */
            rc = send_fragment(packet, seq, mgr->crcKey);
            if (rc < 0) return rc;
            seq = (uint16_t)rc;
            continue;
        }
        
        packet->common.nextModern = NULL; /* Remove any previous link */
        curlen = packet->len + ZEQ_PACKET_SEQUENCE_SIZE;

        if (curlen > UINT8_MAX)
            goto too_big;
        
        if (!first) {
            first = packet;
            prev = packet;
            len = curlen + combinedOverhead;
        } else if ((len + curlen) > ZEQ_PACKET_MTU) {
        too_big:
            /* If we have a previous or combined packet queued up, send it first */
            if (first) {
                hadAck = hasAck;
                rc = send_combined(first, seq, nextAck, combinedOverhead, &hasAck, mgr->crcKey);
                if (rc < 0) return rc;
                seq = (uint16_t)rc;
                if (hasAck != hadAck)
                    mgr->sendAckPrev = nextAck;
                len = 0;
                first = NULL;
                prev = NULL;
            }
            
            /* Send the current packet by itself */
            hadAck = hasAck;
            rc = send_combined(packet, seq, nextAck, combinedOverhead, &hasAck, mgr->crcKey);
            if (rc < 0) return rc;
            seq = (uint16_t)rc;
            if (hasAck != hadAck)
                mgr->sendAckPrev = nextAck;
        } else {
            /* Link to the previous packet for combination */
            prev->common.nextModern = packet;
            prev = packet;
            len += curlen;
        }
    }
    
    if (first) {
        int hadAck = hasAck;
        int rc = send_combined(first, seq, nextAck, combinedOverhead, &hasAck, mgr->crcKey);
        if (rc < 0) return rc;
        seq = (uint16_t)rc;
        if (hasAck != hadAck)
            mgr->sendAckPrev = nextAck;
    }

    mgr->sendSeqNext = seq;
    return ZEQ_OK;
}

int ack_mgr_queue_send(AckMgr* mgr, Packet* packet)
{
    Packet* firstPacket = packet;
    uint16_t firstSeq = mgr->sendSeqLast;
    uint32_t firstIndex = send_seq_to_index(mgr, firstSeq);
    uint32_t index = firstIndex;

    do {
        if (index >= (1UL << mgr->sendCapacity) || !mgr->sendCapacity) {
            uint32_t cap = (1UL << (++mgr->sendCapacity));
            Packet** sendQueue = (Packet**)realloc(mgr->sendQueue, sizeof(Packet*) * cap);
            if (!sendQueue) goto oom;
            mgr->sendQueue = sendQueue;
        }
    
        mgr->sendQueue[index++] = packet;
        packet = packet->common.nextModern;
    } while (packet);

    mgr->sendSeqLast += (uint16_t)(index - firstIndex);

    if (firstSeq == mgr->sendSeqNext) {
        /* This is the next packet to send */
        /*fixme: would it ever not be?*/
        int rc = send_queued_advance(mgr, firstIndex, firstSeq);
        if (rc) return rc;
    }

    return ZEQ_OK;

oom:
    packet_drop_all(&firstPacket->common);
    return ZEQ_ERR_MEMORY;
}

