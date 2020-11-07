#pragma once

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

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	/*
	ATTENTION! log file is not explicitly closed by this lib
	please make sure at application end, you call this, (somewhere clever as you do)

	returns EXIT_SUCCESS or EXIT_FAILURE
	*/
	int dbj_log_finalize(void);

typedef enum {
	/* set to Multi Threaded */
	DBJ_LOG_MT = 1,
	/* if app full path is given  use it to obtain log file name*/
	DBJ_LOG_TO_APP_PATH = 2,
	/* do not show file line on every log line */
	DBJ_LOG_FILE_LINE_OFF = 4,
	/* no console output, beware of no file and seting this in the same time */
	DBJ_LOG_NO_CONSOLE = 8
} DBJ_LOG_SETUP_ENUM;

/////////////////////////////////////////////////////////////////////////////////////
/// predefined setups for both release and debug builds
/// 
#undef  DBJ_LOG_DEFAULT_FILE_SETUP
#define DBJ_LOG_DEFAULT_FILE_SETUP DBJ_LOG_TO_APP_PATH | DBJ_LOG_FILE_LINE_OFF | DBJ_LOG_MT | DBJ_LOG_NO_CONSOLE

#undef  DBJ_LOG_DEFAULT_WITH_CONSOLE
#define DBJ_LOG_DEFAULT_WITH_CONSOLE DBJ_LOG_FILE_LINE_OFF | DBJ_LOG_MT 

/// default is log to file, MT resilient, no console
/// unless you define your combination differently that is
#ifndef  DBJ_LOG_DEFAULT_SETUP
#define  DBJ_LOG_DEFAULT_SETUP DBJ_LOG_DEFAULT_FILE_SETUP
#endif // ! DBJ_LOG_DEFAULT_SETUP

	// make sure you call this once upon app startup
	// with __argv[0] 
	// make sure DBJ_LOG_DEFAULT_SETUP is set to combinaion 
	// you want before calling this function
inline int dbj_simple_log_startup(const char* app_full_path)
{
	_ASSERTE(app_full_path);

	if (!dbj_log_setup(DBJ_LOG_DEFAULT_SETUP, app_full_path))
		return EXIT_FAILURE;

	// this will thus go into which ever log target you have set
	// log file or console or both.

	dbj_log_trace(" %s", "                                                              ");
	dbj_log_trace(" %s", "--------------------------------------------------------------");
	dbj_log_trace(" %s", "                                                              ");
	if ((DBJ_LOG_DEFAULT_SETUP)&DBJ_LOG_TO_APP_PATH)
		dbj_log_trace(" Log file: %s", current_log_file_path());
	dbj_log_trace(" %s", "                                                              ");

#ifdef DBJ_LOG_TESTING
	dbj_log_trace("Log  TRACE");
	dbj_log_debug("Log  DEBUG");
	dbj_log_info("Log  INFO");
	dbj_log_warn("Log  WARN");
	dbj_log_error("Log  ERROR");
	dbj_log_fatal("Log  FATAL");
#endif
	return EXIT_SUCCESS;
}

/* --------------------------------------------------------------------------------------*/
#ifdef __cplusplus
} // extern "C" 
#endif // __cplusplus

#if defined(_WIN32)

/*
 --------------------------------------------------------------------------------------
initialization and deinitialization encapsulated here

NOTE 4 Student: this must be outside of extern "C" block, why ;)
*/

#if defined(_WIN64)
#define DBJ_LOG_SYMBOL_PREFIX
#else
#define DBJ_LOG_SYMBOL_PREFIX "_"
#endif

#undef DBJ_LOG_INITIALIZER
#pragma section(".CRT$XCU", read) 
#define DBJ_LOG_INITIALIZER(f)                                                  \
inline void __cdecl f(void);                                                 \
  __pragma(comment(linker, "/include:" DBJ_LOG_SYMBOL_PREFIX #f "_"));          \
  __declspec(allocate(".CRT$XCU")) void(__cdecl * f##_)(void) =  f; \
   inline void __cdecl f(void)

// for clang on win aka clang-cl.exe do another version
#ifdef __clang__
#undef DBJ_LOG_INITIALIZER
#define DBJ_LOG_INITIALIZER(f)        \
  inline void f(void) __attribute__((constructor));  \
  inline void f(void)
#endif // __clang__

DBJ_LOG_INITIALIZER(dbj_simple_log_before)
{
	static bool been_here_ = false;

	if (been_here_) {
		// is __argv available for windows desktop apps?
		_ASSERTE(__argv);
		_ASSERTE(EXIT_SUCCESS == dbj_simple_log_startup(__argv[0]));
		been_here_ = true;
	}
}

// for clang-cl we also do enjoy this
// BUT! Only if runtime lib is static lib!
#ifdef __clang__

extern "C"
__attribute__((destructor))
inline void dbj_simple_log_after(void) {

	_ASSERTE(dbj_log_finalize() == EXIT_SUCCESS);

}
#else // ! __clang__
#pragma message("please make sure dbj_log_finalize() is called before app exit!")
#endif // ! __clang__


#undef DBJ_C_FUNC
#undef DBJ_LOG_SYMBOL_PREFIX

#endif // defined(_WIN32)


#endif // DBJ_SIMPLE_LOG_HOST_INC
