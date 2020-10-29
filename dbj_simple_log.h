#pragma once
#ifndef _DBJ_SIMPLE_LOG_H_INCLUDED_
#define _DBJ_SIMPLE_LOG_H_INCLUDED_

/* (c) 2019-2020 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/ */

#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#undef  DBJ_PERROR 
#ifdef _DEBUG
#define DBJ_PERROR (perror(__FILE__ " # " _CRT_STRINGIZE(__LINE__))) 
#else
#define DBJ_PERROR
#endif // _DEBUG

#undef DBJ_FERROR
#ifdef _DEBUG
#define DBJ_FERROR( FP_) \
do { \
if (ferror(FP_) != 0) {\
	DBJ_PERROR ;\
	clearerr_s(FP_);\
} \
} while(0)
#else
#define DBJ_FERROR( FP_ )
#endif // _DEBUG

#include "lib/dbj_assert.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define DBJ_LOG_USE_COLOR

// by default
// undef'd 
// do it before including this header
// 
#define DBJ_SIMPLE_LOG_AUTO_FLUSH


#define DBJ_SIMPLE_LOG_MAJOR 3
#define DBJ_SIMPLE_LOG_MINOR 5
#define DBJ_SIMPLE_LOG_PATCH 0
#define DBJ_SIMPLE_LOG_VERSION "3.5.0"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	// log levels
#ifndef DBJ_LOG_LEVELS_ENUM_DEFINED
#define DBJ_LOG_LEVELS_ENUM_DEFINED
	typedef enum DBJ_LOG_LEVELS_ENUM {
		DBJ_LOG_TRACE, DBJ_LOG_DEBUG, DBJ_LOG_INFO, DBJ_LOG_WARN, DBJ_LOG_ERROR, DBJ_LOG_FATAL
	} DBJ_LOG_LEVELS_ENUM;
#endif // DBJ_LOG_LEVELS_ENUM_DEFINED
	/*
	ATTENTION! log file is not explicitly closed by this lib
	please make sure at application end, you call this, (somewhere clever as you do)
	*/
	void dbj_log_finalize(void);

	/*	for users to see*/
	const char* const current_log_file_path();

	/* for users to be able to flush and close in their finalizers */
	FILE* dbj_fhandle_log_file_ptr(FILE* next_fp_);

	// all eventually goes through here
	void dbj_simple_log_log(int /*level*/, const char* /*file*/, int /*line*/, const char* /*fmt*/, ...);

	typedef enum  DBJ_LOG_SETUP_ENUM {
		/* set to Multi Threaded */
		DBJ_LOG_MT = 1,
		/* if app full path is given  use it to obtain log gile name*/
		DBJ_LOG_TO_APP_PATH = 2,
		/* do not show file line on every log line */
		DBJ_LOG_FILE_LINE_OFF = 4,
		/* no console output, beware of no file and seting this in the same time */
		DBJ_LOG_NO_CONSOLE = 8
	} DBJ_LOG_SETUP_ENUM;

	bool dbj_log_setup
	(int setup /* DBJ_LOG_SETUP_ENUM */, const char* app_full_path);

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
#ifndef DBJ_USER_DEFINED_MACRO_NAMES

#ifdef _DEBUG
	#define LOG_TRACE(...) dbj_log_trace(__VA_ARGS__)
	#define LOG_DEBUG(...) dbj_log_debug(__VA_ARGS__)
	#define LOG_INFO(...) dbj_log_info(__VA_ARGS__)
	#define LOG_WARN(...) dbj_log_warn(__VA_ARGS__)
	#define LOG_ERROR(...) dbj_log_error(__VA_ARGS__)
	#define LOG_FATAL(...) dbj_log_fatal(__VA_ARGS__)
#else // ! _DEBUG
	#define LOG_TRACE(...) 
	#define LOG_DEBUG(...) 
	#define LOG_INFO(...) 
	#define LOG_WARN(...) 
	#define LOG_ERROR(...) 
	#define LOG_FATAL(...) 
#endif // ! _DEBUG

#endif // DBJ_USER_DEFINED_MACRO_NAMES

/////////////////////////////////////////////////////////////////////////////////////
///  BIG FAT WARNING
/// 
/// 	do not enter escape codes: \n \v \f \t \r \b
/// 	into your strings
/// 	if you do your file output will be strange
/// 	asnd we will not stop you :)
/// 
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/// predefined setups for both release and debug builds
/// 
#undef  DBJ_LOG_DEFAULT_FILE_SETUP
#define DBJ_LOG_DEFAULT_FILE_SETUP DBJ_LOG_TO_APP_PATH | DBJ_LOG_FILE_LINE_OFF | DBJ_LOG_MT | DBJ_LOG_NO_CONSOLE

#undef  DBJ_LOG_DEFAULT_WITH_CONSOLE
#define DBJ_LOG_DEFAULT_WITH_CONSOLE DBJ_LOG_TO_APP_PATH | DBJ_LOG_FILE_LINE_OFF | DBJ_LOG_MT 

/// defaul is log to file, MT resilient, no console
/// unless you define your combination that is
#ifndef  DBJ_LOG_DEFAULT_SETUP
#define  DBJ_LOG_DEFAULT_SETUP DBJ_LOG_DEFAULT_FILE_SETUP
#endif // ! DBJ_LOG_DEFAULT_SETUP
/////////////////////////////////////////////////////////////////////////////////////

	// make sure you call this once upon app startup
	// with __argv[0] 
	inline int dbj_simple_log_startup(const char* app_full_path)
	{
		DBJ_ASSERT(app_full_path);

		if (!dbj_log_setup(DBJ_LOG_DEFAULT_SETUP, app_full_path))
			return EXIT_FAILURE;

		// this will thus go into which ever log target you have set
		// log file or console or both indeed.

		dbj_log_trace(" %s", "                                                              ");
		dbj_log_trace(" %s", "--------------------------------------------------------------");
		dbj_log_trace(" %s", "                                                              ");
		dbj_log_trace(" Application: %s", app_full_path);
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

#ifdef __cplusplus
} // extern "C" 
#endif // __cplusplus

#endif // _DBJ_SIMPLE_LOG_H_INCLUDED_
