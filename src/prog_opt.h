
#ifndef PROG_OPT_H
#define PROG_OPT_H

#include "cute_func.h"

#define PROG_OPT_ARG -1
#define PROG_OPT_FLAG -2
#define PROG_OPT_END -3
#define PROG_OPT_UNKNOWN -4

typedef struct ProgOptLong {
    const char* name;
    int*        flag;
    int         value;
} ProgOptLong;

CUTE_FUNC int prog_opt(int argc, char** argv, const ProgOptLong* longopts, int* index);

#endif/*PROG_OPT_H*/
