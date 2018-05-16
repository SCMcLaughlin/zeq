
#ifndef ZEQ_FALLTHRU_H
#define ZEQ_FALLTHRU_H

#include "zeq_platform.h"

#if defined(ZEQ_COMPILER_GCC)
# define fallthru __attribute__((fallthrough))
#elif defined(ZEQ_COMPILER_CLANG)
# define fallthru [[fallthrough]]
#else
# define fallthru (void)
#endif

#endif/*ZEQ_FALLTHRU_H*/
