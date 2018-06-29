
#ifndef NET_RECV_H
#define NET_RECV_H

#include <stdint.h>
#include "zeq_byte.h"
#include "zeq_def.h"

struct AckMgr;

ZEQ_INTERFACE void net_protocol_recv(struct AckMgr* mgr, byte* decompressBuf, byte* data, uint32_t len, uint32_t flags);

#endif/*NET_RECV_H*/
