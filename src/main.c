
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "packet_structs.h"
#include "packet_alloc.h"
#include "ack_mgr.h"


int main(int argc, const char** argv)
{
    AckMgr mgr;
    Packet *p1, *p2, *p3;
    uint8_t a[10];
    uint8_t b[6];
    uint8_t c[250];
    uint32_t flags = ZEQ_PACKET_SEQUENCED_BIT | ZEQ_PACKET_CRC_BIT | ZEQ_PACKET_COMPRESSION_FLAG_BIT;

    memset(a, 'a', sizeof(a));
    memset(b, 'b', sizeof(b));
    memset(c, 'c', sizeof(c));

    packet_alloc(&p1, flags, 0x1111, a, sizeof(a));
    packet_alloc(&p2, flags, 0x2222, b, sizeof(b));
    packet_alloc(&p3, flags, 0x3333, c, sizeof(c));
    p1->common.nextModern = p2;
    p2->common.nextModern = p3;

    ack_mgr_init(&mgr);
    mgr.crcKey = 0x11223344;
    ack_mgr_queue_send(&mgr, p1);
    packet_drop_all(&p1->common);
    ack_mgr_deinit(&mgr);
    return EXIT_SUCCESS;
}

