
#ifndef ACK_MGR_H
#define ACK_MGR_H

#include <stdint.h>

struct Packet;

typedef struct AckInputPacket AckInputPacket;

typedef struct AckMgr {
    uint16_t            expectedSeq;
    uint16_t            lastAckRecvd;
    uint16_t            sendSeqFirst;
    uint16_t            sendSeqNext;
    uint16_t            sendSeqLast;
    uint16_t            sendAckPrev;
    uint16_t            sendAckNext;
    uint8_t             sendCapacity;   /* Power of 2, real value is (1 << sendCapacity) */
    struct Packet**     sendQueue;
    uint32_t            crcKey;
} AckMgr;

void ack_mgr_init(AckMgr* mgr);
void ack_mgr_deinit(AckMgr* mgr);
int ack_mgr_queue_send(AckMgr* mgr, struct Packet* packet);

#endif/*ACK_MGR_H*/

