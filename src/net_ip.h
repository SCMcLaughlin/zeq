
#ifndef NET_IP_H
#define NET_IP_H

#include <netinet/in.h>
#include <stdint.h>

typedef struct IpAddr {
    uint32_t    ip;
    uint16_t    port;
} IpAddr;

typedef struct sockaddr_in IpAddrRaw;

#endif/*NET_IP_H*/
