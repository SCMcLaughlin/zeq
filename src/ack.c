
#include <stdint.h>
#include "ack.h"

int ack_cmp(uint16_t got, uint16_t expected)
{
    if (got == expected)
        return ZEQ_ACK_PRESENT;

    if ((got > expected && got < (expected + ZEQ_ACK_WINDOW_HALF_SIZE)) || got < (expected - ZEQ_ACK_WINDOW_HALF_SIZE))
        return ZEQ_ACK_FUTURE;

    return ZEQ_ACK_PAST;
}

