#ifndef _DBJ_SIMPLE_LOG_H_INCLUDED_
#define _DBJ_SIMPLE_LOG_H_INCLUDED_

/* (c) 2019-2021 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/ */

///  BIG FAT WARNING
/// 
/// 	do not enter escape codes: \n \v \f \t \r \b
/// 	into your strings
/// 	if you do your file output will be strange
/// 	and we will not stop you :)
/// 
#ifdef __clang__
#pragma GCC system_header
#endif // __clang__

#ifdef __clang__
#define DBJ_SIMPLELOG_CLANG_CONSTRUCTOR
#else 
#undef DBJ_SIMPLELOG_CLANG_CONSTRUCTOR
#pragma message("This is not clang compiler, make sure you have called dbj_simple_log_startup() on app begin, and dbj_simplelog_finalize() on app exit.")
#endif // no clang


#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <crtdbg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// default is coloured output
#ifndef DBJ_LOG_USE_COLOR
#define DBJ_LOG_USE_COLOR
#endif
// default
#ifndef DBJ_SIMPLE_LOG_AUTO_FLUSH
#define DBJ_SIMPLE_LOG_AUTO_FLUSH
#endif

#define DBJ_SIMPLE_LOG_MAJOR 4
#define DBJ_SIMPLE_LOG_MINOR 3
#define DBJ_SIMPLE_LOG_PATCH 0
#define DBJ_SIMPLE_LOG_VERSION "4.3.0"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef enum DBJ_LOG_SETUP_ENUM_ {
		/* set to Multi Threaded */
		DBJ_LOG_MT = 1,
		/* if app full path is given  use it to obtain log file name*/
		DBJ_LOG_TO_FILE = 2,
		/* do not show file line on every log line */
		DBJ_LOG_FILELINE_SHOW = 4,
		/* no console output, beware of no file and seting this in the same time */
		DBJ_LOG_NO_CONSOLE = 8,
		/* default is time  only */
		DBJ_LOG_FULL_TIMESTAMP = 16,
	} DBJ_LOG_SETUP;

	/////////////////////////////////////////////////////////////////////////////////////
	/// predefined setups for both release and debug builds
	/// 
#undef  DBJ_LOG_DEFAULT_SETUP
#define DBJ_LOG_DEFAULT_SETUP ( DBJ_LOG_TO_FILE | DBJ_LOG_MT | DBJ_LOG_NO_CONSOLE )

#undef  DBJ_LOG_DEFAULT_WITH_CONSOLE
#define DBJ_LOG_DEFAULT_WITH_CONSOLE ( DBJ_LOG_MT )

#ifndef __clang__
	// make sure you call this once upon app startup
// make sure DBJ_LOG_DEFAULT_SETUP is set to combinaion 
// you want before calling this function
	int dbj_simple_log_startup(
		/*DBJ_LOG_SETUP*/ int /*dbj_simple_log_setup_*/,
		const char /*app_full_path*/[BUFSIZ]
	);

	// using clang this is called from destructor function
	// conditionaly defined on the bottom of this file
	// 
	// if pure MSVC is used, this function is published 
	// and it is called from a guardian destructor 
	// when cpp app exists
	// 
	// if one used MSVC C the one is responsible to call
	// this function. Somehow.
	// 

	int dbj_simplelog_finalize(void);
#endif
	// can be used from other parts,
	// not just an host app
	void dbj_simple_log_test(const char*);

	/*	for users */
	const char* const current_log_file_path();

	// deprecated
	// 	log file handling is completely hidden from users
	// FILE* dbj_fhandle_log_file_ptr(FILE* next_fp_);

	typedef enum DBJ_LOG_LEVEL_ENUM {
		DBJ_LOG_TRACE,
		DBJ_LOG_DEBUG,
		DBJ_LOG_INFO,
		DBJ_LOG_WARN,
		DBJ_LOG_ERROR,
		DBJ_LOG_FATAL
	} DBJ_LOG_LEVEL;

	// all eventually goes through here
	void dbj_simple_log_log(int /*level*/, const char* /*file*/, int /*line*/, const char* /*fmt*/, ...);

	// bool dbj_log_setup(int, const char*);

	/////////////////////////////////////////////////////////////////////////////////////
	// primary usage is through these macros in the back
	// NOTE: these are active in both debug and release builds

#define dbj_log_trace(...) dbj_simple_log_log(DBJ_LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define dbj_log_debug(...) dbj_simple_log_log(DBJ_LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define dbj_log_info(...)  dbj_simple_log_log(DBJ_LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define dbj_log_warn(...)  dbj_simple_log_log(DBJ_LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define dbj_log_error(...) dbj_simple_log_log(DBJ_LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define dbj_log_fatal(...) dbj_simple_log_log(DBJ_LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

// and these macros are in the front
// which are much more senisitive to name clash
// unles you do not use your own set
#ifndef DBJ_SIMPLELOG_USER_DEFINED_MACRO_NAMES

#define LOG_TRACE(...) dbj_log_trace(__VA_ARGS__)
#define LOG_DEBUG(...) dbj_log_debug(__VA_ARGS__)
#define LOG_INFO(...) dbj_log_info(__VA_ARGS__)
#define LOG_WARN(...) dbj_log_warn(__VA_ARGS__)
#define LOG_ERROR(...) dbj_log_error(__VA_ARGS__)
#define LOG_FATAL(...) dbj_log_fatal(__VA_ARGS__)

#endif // DBJ_USER_DEFINED_MACRO_NAMES

#ifdef __cplusplus
} // extern "C" 
#endif // __cplusplus

#endif // _DBJ_SIMPLE_LOG_H_INCLUDED_
