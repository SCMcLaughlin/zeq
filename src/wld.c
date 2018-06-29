
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"
#include "wld.h"
#include "wld_types.h"
#include "zeq_alloc.h"
#include "zeq_bit.h"
#include "zeq_byte.h"
#include "zeq_err.h"

#define WLD_SIGNATURE 0x54503d02
#define WLD_VERSION1 0x00015500
#define WLD_VERSION2 0x1000c800
#define WLD_VERSION3 0x1000c801

typedef struct WldHeader {
    uint32_t signature;
    uint32_t version;
    uint32_t fragCount;
    uint32_t unknownA[2];
    uint32_t stringsLength;
    uint32_t unknownB;
} WldHeader;

typedef struct WldNameRef {
    int         ref;
    uint32_t    hash;
} WldNameRef;

struct Wld {
    uint32_t    count;
    uint32_t    version;
    int         negstrlen;
    char*       strings;
    uint8_t*    fragTypes;
    WldFrag**   frags;
    WldNameRef* nameRefs;
    byte*       data;
};

void wld_process_string(void* vstr, uint32_t len)
{
    static const byte hash[] = {0x95, 0x3A, 0xC5, 0x2A, 0x95, 0x7A, 0x95, 0x6A};
    byte* str = (byte*)vstr;
    uint32_t i;
    
    for (i = 0; i < len; i++) {
        byte c = str[i] ^ hash[i & 7];
        if (c >= 'A' && c <= 'Z')
            c -= 'A' - 'a';
        str[i] = c;
    }
}

static int wld_add_frag(Wld* wld, WldFrag* frag)
{
    WldNameRef name;
    
    if (bit_is_pow2_or_zero(wld->count)) {
        uint32_t cap = (wld->count == 0) ? 1 : wld->count * 2;
        uint8_t* fragTypes;
        WldFrag** frags;
        WldNameRef* nameRefs;
        
        fragTypes = (uint8_t*)zeq_realloc(wld->fragTypes, cap * sizeof(uint8_t));
        if (!fragTypes) return ZEQ_ERR_MEMORY;
        wld->fragTypes = fragTypes;
        
        frags = (WldFrag**)zeq_realloc(wld->frags, cap * sizeof(WldFrag*));
        if (!frags) return ZEQ_ERR_MEMORY;
        wld->frags = frags;
        
        nameRefs = (WldNameRef*)zeq_realloc(wld->nameRefs, cap * sizeof(WldNameRef));
        if (!nameRefs) return ZEQ_ERR_MEMORY;
        wld->nameRefs = nameRefs;
    }
    
    name.ref = 0;
    name.hash = 0;
    
    if (frag) {
        if (frag->nameRef < 0 && frag->nameRef > wld->negstrlen) {
            const char* str = wld->strings - frag->nameRef;
            
            name.ref = frag->nameRef;
            name.hash = hash_string(str, strlen(str));
        }
    }
    
    wld->fragTypes[wld->count] = (uint8_t)wld_frag_type(frag);
    wld->frags[wld->count] = frag;
    wld->nameRefs[wld->count] = name;
    wld->count++;
    return ZEQ_OK;
}

static int wld_open_impl(Wld** wld, void* vdata, uint32_t length)
{
    Wld* ptr;
    byte* data = (byte*)vdata;
    WldHeader* h = (WldHeader*)data;
    uint32_t p = sizeof(WldHeader);
    uint32_t i;
    int rc = ZEQ_ERR_FILE_FORMAT;
    
    ptr = (Wld*)zeq_calloc(1, sizeof(Wld));
    if (!ptr) return ZEQ_ERR_MEMORY;
    
    ptr->data = data;
    if (p > length) goto fail;
    
    if (h->signature != WLD_SIGNATURE)
        goto fail;
    
    if (h->version != WLD_VERSION1 && h->version != WLD_VERSION2 && h->version != WLD_VERSION3)
        goto fail;
    
    ptr->version = h->version;
    ptr->negstrlen = -((int)h->stringsLength);
    ptr->strings = (char*)(data + p);
    
    p += h->stringsLength;
    if (p > length) goto fail;
    
    wld_process_string(ptr->strings, h->stringsLength);
    
    if (wld_add_frag(ptr, NULL)) {
        rc = ZEQ_ERR_MEMORY;
        goto fail;
    }
    
    for (i = 0; i < h->fragCount; i++) {
        WldFrag* frag = (WldFrag*)(data + p);
        
        p += sizeof(WldFrag);
        if (p > length) goto fail;
        
        p += frag->length - sizeof(uint32_t);
        if (p > length) goto fail;
        
        if (wld_add_frag(ptr, frag)) {
            rc = ZEQ_ERR_MEMORY;
            goto fail;
        }
    }
    
    *wld = ptr;
    return ZEQ_OK;
    
fail:
    wld_close(ptr);
    return rc;
}

int wld_open(Wld** wld, void* data, uint32_t length)
{
    if (!wld || !data || !length)
        return ZEQ_ERR_INVALID;
    
    return wld_open_impl(wld, data, length);
}

Wld* wld_close(Wld* wld)
{
    if (wld) {
        if (wld->frags) {
            free(wld->frags);
            wld->frags = NULL;
        }
        
        if (wld->nameRefs) {
            free(wld->nameRefs);
            wld->nameRefs = NULL;
        }
        
        if (wld->data) {
            free(wld->data);
            wld->data = NULL;
        }
        
        free(wld);
    }
    
    return NULL;
}

uint8_t* wld_frag_types(Wld* wld, uint32_t* count)
{
    if (!wld || !wld->fragTypes) return NULL;
    if (count) *count = wld->count - 1;
    return wld->fragTypes + 1;
}

WldFrag** wld_frags(Wld* wld, uint32_t* count)
{
    if (!wld || !wld->frags) return NULL;
    if (count) *count = wld->count - 1;
    return wld->frags + 1;
}

static int wld_index_by_nameref(Wld* wld, int ref)
{
    int i;
    
    for (i = 0; i < (int)wld->count; i++) {
        if (wld->nameRefs[i].ref == ref)
            return i;
    }
    
    return -1;
}

WldFrag* wld_frag_by_ref(Wld* wld, int ref)
{
    if (wld) {
        if (ref >= 0) {
            if (ref < (int)wld->count)
                return wld->frags[ref];
        } else {
            int idx = wld_index_by_nameref(wld, ref);
            if (idx >= 0)
                return wld->frags[idx];
        }
    }
    
    return NULL;
}

static int wld_index_by_name(Wld* wld, const char* name)
{
    uint32_t hash = hash_string(name, strlen(name));
    int i;
    
    for (i = 0; i < (int)wld->count; i++)
    {
        WldNameRef* ref = &wld->nameRefs[i];
        
        if (ref->hash == hash) {
            const char* str = wld->strings - ref->ref;
            
            if (strcmp(str, name) == 0)
                return i;
        }
    }
    
    return -1;
}

WldFrag* wld_frag_by_name(Wld* wld, const char* name)
{
    if (wld && name) {
        int idx = wld_index_by_name(wld, name);
        if (idx >= 0)
            return wld->frags[idx];
    }
    
    return NULL;
}

char* wld_string_by_frag(Wld* wld, WldFrag* frag)
{
    return wld_string_by_ref(wld, frag->nameRef);
}

char* wld_string_by_ref(Wld* wld, int ref)
{
    if (wld) {
        if (ref < 0 && ref > wld->negstrlen)
            return wld->strings - ref;
    }
    
    return NULL;
}
