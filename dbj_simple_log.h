#pragma once
#ifndef _DBJ_SIMPLE_LOG_H_INCLUDED_
#define _DBJ_SIMPLE_LOG_H_INCLUDED_

/* (c) 2019-2020 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/ */

///  BIG FAT WARNING
/// 
/// 	do not enter escape codes: \n \v \f \t \r \b
/// 	into your strings
/// 	if you do your file output will be strange
/// 	and we will not stop you :)

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

#define DBJ_SIMPLE_LOG_MAJOR 3
#define DBJ_SIMPLE_LOG_MINOR 5
#define DBJ_SIMPLE_LOG_PATCH 4
#define DBJ_SIMPLE_LOG_VERSION "3.5.4"

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

	returns EXIT_SUCCESS or EXIT_FAILURE
	*/
	int dbj_log_finalize(void);

	/*	for users to see*/
	const char* const current_log_file_path();

	/* for users to be able to flush and close in their finalizers */
	FILE* dbj_fhandle_log_file_ptr(FILE* next_fp_);

	// all eventually goes through here
	void dbj_simple_log_log(int /*level*/, const char* /*file*/, int /*line*/, const char* /*fmt*/, ...);

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
#ifndef DBJ_SIMPLELOG_USER_DEFINED_MACRO_NAMES

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



#ifdef __cplusplus
} // extern "C" 
#endif // __cplusplus

#ifndef __clang__
#pragma message("please make sure dbj_log_finalize() is called before app exit!")
#endif

#endif // _DBJ_SIMPLE_LOG_H_INCLUDED_
