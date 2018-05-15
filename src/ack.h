
#ifndef ACK_H
#define ACK_H

#include <stdint.h>

#define ZEQ_ACK_WINDOW_HALF_SIZE 1024

enum AckRelative {
    ZEQ_ACK_PRESENT,
    ZEQ_ACK_FUTURE,
    ZEQ_ACK_PAST
};

int ack_cmp(uint16_t got, uint16_t expected);

#endif/*ACK_H*/

