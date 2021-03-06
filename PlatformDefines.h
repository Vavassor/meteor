#ifndef PLATFORM_DEFINES_H
#define PLATFORM_DEFINES_H

// OPERATING SYSTEMS
//----------------------------------------------------------------------------------------------------
#if defined(__linux__)
#define OS_LINUX
#elif defined(_WIN32)
#define OS_WINDOWS
#elif defined(__APPLE__) && defined(__MACH__)
#define OS_OSX
#endif

// UNIX SYSTEM STANDARDS
//----------------------------------------------------------------------------------------------------
#if defined(__unix__) || defined(OS_OSX)
#include <unistd.h>

#if defined(_POSIX_VERSION)
#define STANDARDS_POSIX
#endif

#if defined(_XOPEN_VERSION)
#define STANDARDS_XOPEN
#endif

#if defined(__LSB_VERSION__)
#define STANDARDS_LSB
#endif

#endif

// WINDOWING SYSTEMS
//----------------------------------------------------------------------------------------------------
#if defined(__linux__)

#if defined(X11)
#define	WINDOW_SYSTEM_X
#elif (0)
#define WINDOW_SYSTEM_WAYLAND
#define WINDOW_SYSTEM_MIR
#endif

#elif defined(__APPLE__) && defined(__MACH__)
#define	WINDOW_SYSTEM_QUARTZ
#endif

#endif
