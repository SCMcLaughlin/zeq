
#ifndef WLD_H
#define WLD_H

#include <stdint.h>
#include <stdio.h>
#include "zeq_def.h"

typedef struct Wld Wld;

struct WldFrag;

ZEQ_INTERFACE int wld_open(Wld** wld, void* data, uint32_t length);
ZEQ_INTERFACE Wld* wld_close(Wld* wld);

ZEQ_INTERFACE void wld_process_string(void* vstr, uint32_t len);

ZEQ_INTERFACE struct WldFrag** wld_frags(Wld* wld, uint32_t* count);
ZEQ_INTERFACE uint8_t* wld_frag_types(Wld* wld, uint32_t* count);
ZEQ_INTERFACE struct WldFrag* wld_frag_by_ref(Wld* wld, int ref);
ZEQ_INTERFACE struct WldFrag* wld_frag_by_name(Wld* wld, const char* name);
ZEQ_INTERFACE char* wld_string_by_frag(Wld* wld, struct WldFrag* frag);
ZEQ_INTERFACE char* wld_string_by_ref(Wld* wld, int ref);

#endif/*WLD_H*/
