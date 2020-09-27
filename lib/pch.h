#ifndef PCH_H
#define PCH_H
// pch.h: This is a precompiled header file.
// NOTE! pch.h is implicitly included!

#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <string.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <crtdbg.h>

#undef DBJ_ASSERT
#define DBJ_ASSERT _ASSERTE

// ALWAYS WORKS!!! in release builds too, be carefull!
#ifndef DBJ_VERIFY
#define DBJ_VERIFY(expr, msg) \
(void)(                                                                                     \
    (!!(expr)) ||                                                                           \
    (1 != _CrtDbgReportW(_CRT_ASSERT, _CRT_WIDE(__FILE__), __LINE__, NULL, L"%ls", msg)) || \
    (_CrtDbgBreak(), 0)                                                                     \
)
#endif

// Here's a better C version (from Google's Chromium project):
#define DBJ_COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#endif //PCH_H
