
#ifndef ACK_MGR_H
#define ACK_MGR_H

#include <stdint.h>
#include "zeq_byte.h"

struct Packet;

typedef struct AckInputPacket AckInputPacket;

typedef struct AckMgr {
    /* Receiving-related fields */
    uint16_t            recvSeqNext;
    uint16_t            recvSeqLast;
    /* Sending-related fields */
    uint16_t            sendSeqFirst;
    uint16_t            sendSeqNext;
    uint16_t            sendSeqLast;
    uint16_t            sendAckPrev;
    uint16_t            sendAckNext;
    /* Other fields */
    uint8_t             recvCapacity;   /* Power of 2, real value is (1 << recvCapacity) */
    uint8_t             sendCapacity;   /* Power of 2, real value is (1 << sendCapacity) */
    uint32_t            crcKey;
    uint32_t            fragLength;
    AckInputPacket*     recvQueue;
    struct Packet**     sendQueue;
} AckMgr;

void ack_mgr_init(AckMgr* mgr);
void ack_mgr_deinit(AckMgr* mgr);
int ack_mgr_queue_send(AckMgr* mgr, struct Packet* packet);
void ack_mgr_queue_recv(AckMgr* mgr, byte_t* data, uint32_t len, uint16_t seq);
void ack_mgr_queue_recv_fragment(AckMgr* mgr, byte_t* data, uint32_t len, uint16_t seq);
void ack_mgr_recv_unsequenced(AckMgr* mgr, byte_t* data, uint32_t len);
void ack_mgr_send_out_of_order_request(AckMgr* mgr, uint16_t seq, uint32_t flags);

#define ack_mgr_expected_seq(mgr) ((mgr)->recvSeqNext)

#endif/*ACK_MGR_H*/

