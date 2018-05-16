
#ifndef NET_RECV_H
#define NET_RECV_H

#include <stdint.h>
#include "zeq_byte.h"

struct AckMgr;

void net_protocol_recv(struct AckMgr* mgr, byte_t* decompressBuf, byte_t* data, uint32_t len, uint32_t flags);

#endif/*NET_RECV_H*/
