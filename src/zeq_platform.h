
#ifndef ZEQ_PLATFORM_H
#define ZEQ_PLATFORM_H

#if defined(_MSC_VER)
# define ZEQ_COMPILER_MSVC
#elif defined(__GNUC__)
# define ZEQ_COMPILER_GCC
#elif defined(__clang__)
# define ZEQ_COMPILER_CLANG
#else
# error "Unsupported compiler"
#endif

#if defined(_WIN32)
# define ZEQ_OS_WINDOWS
#else
# define ZEQ_OS_POSIX
# if defined(__linux__)
#  define ZEQ_OS_LINUX
# elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__)
#  define ZEQ_OS_BSD
# elif defined(__APPLE__) && defined(__MACH__)
#  define ZEQ_OS_MAC
# else
#  error "Could not determine target OS"
# endif
#endif

#ifdef ZEQ_COMPILER_MSVC
# ifdef __cplusplus
#  define ZEQ_EXPORT extern "C" __declspec(dllexport)
# else
#  define ZEQ_EXPORT __declspec(dllexport)
# endif
#else
# ifdef __cplusplus
#  define ZEQ_EXPORT extern "C"
# else
#  define ZEQ_EXPORT extern
# endif
#endif

#endif/*ZEQ_PLATFORM_H*/

