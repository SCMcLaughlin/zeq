
#ifndef NET_PROTOCOL_H
#define NET_PROTOCOL_H

#include <stdint.h>

#define NET_PROTOCOL_OP_NONE 0x0000
#define NET_PROTOCOL_OP_SESSION_REQUEST 0x0100
#define NET_PROTOCOL_OP_SESSION_RESPONSE 0x0200
#define NET_PROTOCOL_OP_COMBINED 0x0300
#define NET_PROTOCOL_OP_DISCONNECT 0x0500
#define NET_PROTOCOL_OP_KEEP_ALIVE 0x0600
#define NET_PROTOCOL_OP_SESSION_STAT_REQUEST 0x0700
#define NET_PROTOCOL_OP_SESSION_STAT_RESPONSE 0x0800
#define NET_PROTOCOL_OP_PACKET 0x0900
#define NET_PROTOCOL_OP_FRAGMENT 0x0d00
#define NET_PROTOCOL_OP_OUT_OF_ORDER 0x1100
#define NET_PROTOCOL_OP_ACK 0x1500

#pragma pack(1)

typedef struct {
    uint16_t    opcode;
    uint32_t    unknown;
    uint32_t    sessionId;
    uint32_t    maxLength;
} NetProtocol_SessionRequest;

typedef struct {
    uint16_t    opcode;
    uint32_t    sessionId;
    uint32_t    crcKey;
    uint8_t     unknownA;
    uint8_t     format;
    uint8_t     unknownB;
    uint32_t    maxLength;
    uint32_t    unknownC;
} NetProtocol_SessionResponse;

typedef struct {
    uint16_t    opcode;
    uint16_t    requestId;
    uint32_t    lastLocalDelta;
    uint32_t    averageDelta;
    uint32_t    lowDelta;
    uint32_t    highDelta;
    uint32_t    lastRemoteDelta;
    uint64_t    packetsSent;
    uint64_t    packetsReceived;
} NetProtocol_SessionStats;

typedef struct {
    uint16_t    opcode;
    uint32_t    sessionId;
    uint16_t    crc;
} NetProtocol_SessionDisconnect;

#pragma pack()

#endif/*NET_PROTOCOL_H*/
