
#ifndef ZEQ_DEF_H
#define ZEQ_DEF_H

#include "zeq_platform.h"

#if defined(ZEQ_COMPILER_GCC) || defined(ZEQ_COMPILER_CLANG)
# define ZEQ_INLINE inline
# define ZEQ_ALWAYS_INLINE inline __attribute((always_inline))
/* restrict is C99 */
# if __STDC_VERSION__ < 199901L
#  define R 
# else
#  define R restrict
# endif
/* fallthru */
# if defined(ZEQ_COMPILER_GCC)
#  define fallthru __attribute__((fallthrough))
# elif defined(ZEQ_COMPILER_CLANG)
#  define fallthru [[fallthrough]]
# else
#  define fallthru 
# endif
#elif defined(ZEQ_COMPILER_MSVC)
# define ZEQ_INLINE __inline
# define ZEQ_ALWAYS_INLINE __forceinline
# define R __restrict
# define fallthru 
#endif

#ifdef ZEQ_AMALG
# define ZEQ_INTERFACE static
# define ZEQ_INTERFACE_INLINE static ZEQ_ALWAYS_INLINE 
#else
# define ZEQ_INTERFACE 
# define ZEQ_INTERFACE_INLINE 
#endif

#define ZEQ_UNUSED(x) ((void)(x))

#endif/*ZEQ_DEF_H*/
