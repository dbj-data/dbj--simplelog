#ifndef DBJ_SIMPLE_LOG_HOST_INC
#define DBJ_SIMPLE_LOG_HOST_INC
/* (c) 2019-2020 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/

Decision of where the log goes is done at compile time

Only host app includes this, only once, and makes that decision

Host app might include many other callers of the simple log
None of those users must not include this header

DBJ_LOG_DEFAULT_SETUP must be defined if users do not want the default setyp
*/

#include "dbj_simple_log.h"

#include <crtdbg.h>
#include <errno.h>
#include <io.h> // is a tty

#ifdef __clang__
#pragma clang system_header
#endif // __clang__
#if 0
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	// __declspec(dllimport)
		//_Success_(return != 0)
		//_Ret_range_(1, nSize)
	unsigned long
		__stdcall
		GetModuleFileNameA(
			void* hModule,
			char* lpFilename,
			unsigned long nSize
		);
#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus



//#include <winsdkver.h>
//
//#ifndef WINVER
//#define WINVER 0x0A00
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT 0x0A00
//#endif
////
//#ifdef __STDC_ALLOC_LIB__
//#define __STDC_WANT_LIB_EXT2__ 1
//#else
//#define _POSIX_C_SOURCE 200809L
//#endif


#ifndef DBJ_ASSERT
#ifdef _DEBUG
#define DBJ_ASSERT _ASSERTE
#else
#define	DBJ_ASSERT(X_) ((void)(X_))
#endif
#endif // ! DBJ_ASSERT

//#ifdef __cplusplus
//extern "C" {
//#endif // __cplusplus
//
//
//
//	/*
//	--------------------------------------------------------------------------------------
//	users must give a value to this before dbj simplelog is initialized
//	*/
//	//extern int dbj_simple_log_setup_;
//
//	// int dbj_simple_log_startup(DBJ_LOG_SETUP_ENUM /*dbj_simple_log_setup_*/, const char* /*app_full_path*/);
//
//	/* --------------------------------------------------------------------------------------*/
//#ifdef __cplusplus
//} // extern "C" 
//#endif // __cplusplus
/* --------------------------------------------------------------------------------------*/

	/*
	--------------------------------------------------------------------------------------
	clang-cl initialization and deinitialization will happen automagically
	C or C++

	for clang on win aka clang-cl.exe
	WARNING 2020 Q4: destructor works only if runtime lib is static lib!
	--------------------------------------------------------------------------------------
	*/



#ifdef __clang__
__attribute__((constructor))
#endif
inline void dbj_simplelog_before(void)
{
	char app_full_path[1024] = { 0 };
	// Q: is __argv available for windows desktop apps?
	// A: no it is not
	// win32 required here
	int rez = GetModuleFileNameA(
		NULL, app_full_path, 1024
	);
	DBJ_ASSERT(rez != 0);

	if (_isatty(0) && _isatty(1)) {
		rez = dbj_simple_log_startup(DBJ_LOG_DEFAULT_WITH_CONSOLE, app_full_path);
	}
	else {
		rez = dbj_simple_log_startup(DBJ_LOG_DEFAULT_SETUP, app_full_path);
	}

	DBJ_ASSERT(EXIT_SUCCESS == rez);
}

#endif // 0

#ifdef __clang__
__attribute__((destructor))
#endif
inline void dbj_simple_log_after(void) {
	int rez = dbj_log_finalize();
	_ASSERTE(EXIT_SUCCESS == rez);
}

#ifndef __clang__
namespace {
	//
	// in no clang c++ situation 
	// here is the simple solution
	// although, c++ 'static fiasco' might not 
	// allow for this to happen early enough
	//
	struct simple_log_protector final {

		//static inline bool worketh = false;

		//simple_log_protector() noexcept {
		//	dbj_simplelog_before();
		//	simple_log_protector::worketh = true;
		//}

		simple_log_protector() = default;

		simple_log_protector(simple_log_protector const&) = delete;
		simple_log_protector & operator = simple_log_protector(simple_log_protector const &) = delete;

		simple_log_protector(simple_log_protector &&) = delete;
		simple_log_protector& operator = simple_log_protector(simple_log_protector &&) = delete;

		~simple_log_protector() noexcept {
			dbj_simple_log_after();
		}
	};

	inline const simple_log_protector simple_log_protector__;

} // ns

#endif // !__clang__

#if 0
// note for eggheads: yes I know there is a way to code constructor for MSVC
// in C, but I deliberately do not want to use linker hacks
// instead I use clang-cl.exe
// if you feverishly oppose, please insert your MSVC implementation here
#ifndef __clang__

#pragma message("MSVC C code: please make sure dbj_simple_log_startup() is called before app starts!")
#pragma message("MSVC C code: please make sure dbj_log_finalize() is called before app exit!")

#endif

#endif // 0

#endif // DBJ_SIMPLE_LOG_HOST_INC
