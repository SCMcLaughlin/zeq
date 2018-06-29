
#include <arpa/inet.h>
#include <stdint.h>
#include <string.h>
#include <zlib.h>
#include "ack.h"
#include "ack_mgr.h"
#include "crc.h"
#include "net_recv.h"
#include "net_protocol.h"
#include "packet_structs.h"
#include "zeq_byte.h"
#include "zeq_def.h"
#include "zeq_err.h"

static int net_check_crc(AckMgr* mgr, byte* data, uint32_t len)
{
    uint16_t actual = htons(crc16(data, len - ZEQ_PACKET_CRC_SIZE, mgr->crcKey));
    uint16_t reported;
    memcpy(&reported, &data[len - ZEQ_PACKET_CRC_SIZE], ZEQ_PACKET_CRC_SIZE);
    return !(actual == reported);
}

static int net_recv_decompress(byte* decompressBuf, byte** outData, uint32_t* outLen)
{
    byte* data = *outData;
    uint32_t len = *outLen;
    uint32_t offset = (data[0] == 0x00) ? 2 : 1;
    
    if (data[offset] == 'Z') {
        /* Compressed */
        unsigned long dstlen = ZEQ_PACKET_DECOMPRESS_BUFFER_SIZE;
        int rc = uncompress(&decompressBuf[offset], &dstlen, &data[offset + 1], len - offset - 1);
        if (rc != Z_OK) return ZEQ_ERR_API;
        
        data = decompressBuf;
        len = (uint32_t)dstlen + offset + 1;
    }
    
    /* Remove the compression flag */
    if (offset >= 2)
        data[2] = data[1];
    if (offset >= 1)
        data[1] = data[0];
    
    *outData = data + 1;
    *outLen = len - 1;
    return ZEQ_OK;
}

static void net_recv_combined_long(AckMgr* mgr, byte* data, uint32_t len, uint32_t flags)
{
    uint32_t offset = ZEQ_PACKET_PROTOCOL_OPCODE_SIZE;
    
    while (offset < len) {
        uint16_t size = data[offset];
        offset++;
        
        if (size == 0xff) {
            memcpy(&size, &data[offset], sizeof(size));
            offset += sizeof(size);
        }
        
        net_protocol_recv(mgr, NULL, &data[offset], (uint32_t)size, flags);
        offset += size;
    }
}

static void net_recv_packet(AckMgr* mgr, byte* data, uint32_t len, uint32_t flags)
{
    uint32_t offset = ZEQ_PACKET_PROTOCOL_OPCODE_SIZE + ZEQ_PACKET_SEQUENCE_SIZE;
    uint16_t seq;
    
    memcpy(&seq, &data[ZEQ_PACKET_PROTOCOL_OPCODE_SIZE], ZEQ_PACKET_SEQUENCE_SIZE);
    seq = ntohs(seq);
    
    switch (ack_cmp(ack_mgr_expected_seq(mgr), seq))
    {
    case ZEQ_ACK_PRESENT:
        /* For some reason, what looks like a normal packet may in fact be a wrapper for a long combined packet */
        if (data[offset] == (NET_PROTOCOL_OP_COMBINED_LONG & 0x00ff) && data[offset + 1] == ((NET_PROTOCOL_OP_COMBINED_LONG & 0xff00) >> 8)) {
            net_recv_combined_long(mgr, &data[offset], len - offset, flags);
            break;
        }
        ack_mgr_queue_recv(mgr, &data[offset], len - offset, seq);
        break;
    
    case ZEQ_ACK_FUTURE:
        break;
    
    case ZEQ_ACK_PAST:
        ack_mgr_send_out_of_order_request(mgr, seq, flags);
        break;
    }
}

static void net_recv_combined(AckMgr* mgr, byte* data, uint32_t len, uint32_t flags)
{
    uint32_t offset = ZEQ_PACKET_PROTOCOL_OPCODE_SIZE;
    
    while (offset < len) {
        uint8_t size = data[offset];
        offset++;
        net_protocol_recv(mgr, NULL, &data[offset], (uint32_t)size, flags);
        offset += size;
    }
}

static void net_recv_fragment(AckMgr* mgr, byte* data, uint32_t len, uint32_t flags)
{
    uint32_t offset = ZEQ_PACKET_PROTOCOL_OPCODE_SIZE + ZEQ_PACKET_SEQUENCE_SIZE;
    uint16_t seq;   
    memcpy(&seq, &data[ZEQ_PACKET_PROTOCOL_OPCODE_SIZE], ZEQ_PACKET_SEQUENCE_SIZE);
    seq = ntohs(seq);

    switch (ack_cmp(ack_mgr_expected_seq(mgr), seq))
    {
    case ZEQ_ACK_PRESENT:
    case ZEQ_ACK_FUTURE:
        ack_mgr_queue_recv_fragment(mgr, &data[offset], len - offset, seq);
        break;

    case ZEQ_ACK_PAST:
        ack_mgr_send_out_of_order_request(mgr, seq, flags);
        break;
    }
}

void net_protocol_recv(AckMgr* mgr, byte* decompressBuf, byte* data, uint32_t len, uint32_t flags)
{
    uint16_t protoOp;
    
    if (len < 2) return;
    memcpy(&protoOp, data, sizeof(protoOp));
    
    switch (protoOp) {
    /* Packets that may be CRC'd and compressed */
    case NET_PROTOCOL_OP_COMBINED:
    case NET_PROTOCOL_OP_PACKET:
    case NET_PROTOCOL_OP_FRAGMENT:
    case NET_PROTOCOL_OP_COMBINED_LONG:
        if (decompressBuf) {
            /* Handle CRC */
            if (flags & ZEQ_PACKET_CRC_BIT) {
                if (net_check_crc(mgr, data, len))
                    return;
                len -= ZEQ_PACKET_CRC_SIZE;
            }
            /* Handle compression */
            if (flags & ZEQ_PACKET_COMPRESSION_FLAG_BIT && net_recv_decompress(decompressBuf, &data, &len))
                return;
        }
        
        switch (protoOp) {
        case NET_PROTOCOL_OP_COMBINED:
            net_recv_combined(mgr, data, len, flags);
            break;

        case NET_PROTOCOL_OP_PACKET:
            net_recv_packet(mgr, data, len, flags);
            break;

        case NET_PROTOCOL_OP_FRAGMENT:
            net_recv_fragment(mgr, data, len, flags);
            break;

        case NET_PROTOCOL_OP_COMBINED_LONG:
            net_recv_combined_long(mgr, data, len, flags);
            break;
        }

        break;

    /* Packets that are never CRC'd or compressed */
    case NET_PROTOCOL_OP_SESSION_REQUEST:
        break;

    /* Packets without a protocol opcode */
    default:
        if (protoOp < 0xff) {
            /*fixme: treat as OP_PACKET like eqemu?*/
            break;
        }
        fallthru;
    case 0x0000:
        if (decompressBuf) {
            /* Handle CRC */
            if (flags & ZEQ_PACKET_CRC_BIT) {
                if (net_check_crc(mgr, data, len))
                    return;
                len -= ZEQ_PACKET_CRC_SIZE;
            }
            /* Handle compression */
            if (flags & ZEQ_PACKET_COMPRESSION_FLAG_BIT && net_recv_decompress(decompressBuf, &data, &len))
                return;
        }

        /* Unsequenced packet */
        ack_mgr_recv_unsequenced(mgr, data, len);
        break;
    }
}
