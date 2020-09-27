#pragma once
#include <crtdbg.h>

#undef DBJ_ASSERT
#define DBJ_ASSERT _ASSERTE

// In release builds too, be carefull!
#ifndef DBJ_VERIFY
#define DBJ_VERIFY(expr, msg) \
(void)(                                                                                     \
    (!!(expr)) ||                                                                           \
    (1 != _CrtDbgReportW(_CRT_ASSERT, _CRT_WIDE(__FILE__), __LINE__, NULL, L"%ls", msg)) || \
    (_CrtDbgBreak(), 0)                                                                     \
)
#endif // ! DBJ_VERIFY

// Here's a better C version (from Google's Chromium project):
#undef DBJ_COUNT_OF
#define DBJ_COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

