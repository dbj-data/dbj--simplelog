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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define DBJ_LOG_USE_COLOR
#define DBJ_SIMPLE_LOG_MAJOR 3
#define DBJ_SIMPLE_LOG_MINOR 0
#define DBJ_SIMPLE_LOG_PATCH 0
#define DBJ_SIMPLE_LOG_VERSION "3.0.0"

/*


	FILE* fp_ = dbj_fhandle_log_file_ptr(NULL);
	assert(fp_);
	DBJ_FERROR(fp_);
	(void)_flushall();
		// make sure it is fclose, not close
	if (fp_) { fclose(fp_); fp_ = nullptr; }

	You must flush to see the log file contents eventually
*/

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	// log levels
#ifndef DBJ_LOG_LEVELS_ENUM_DEFINED
#define DBJ_LOG_LEVELS_ENUM_DEFINED
	typedef enum DBJ_LOG_LEVELS_ENUM { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL } DBJ_LOG_LEVELS_ENUM;
#endif // DBJ_LOG_LEVELS_ENUM_DEFINED
	/*
	ATTENTION! log file is not explicitly closed by this lib
	please make sure at application end, you call this, (somewhere clever as you do)
	*/
	void dbj_log_finalize(void);

	/*	for users to see*/
	const char* const current_log_file_path();

	/* for users ot be able to flush and close in their finalizers */
	FILE* dbj_fhandle_log_file_ptr(FILE* next_fp_);

	// all eventually goes in here
	void log_log(int /*level*/, const char* /*file*/, int /*line*/, const char* /*fmt*/, ...);

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

	// primary usage is through these

#define log_trace(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  log_log(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_log(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

/////////////////////////////////////////////////////////////////////////////////////
#ifndef  DBJ_LOG_DEFAULT_SETUP
#define DBJ_LOG_DEFAULT_SETUP DBJ_LOG_TO_APP_PATH | DBJ_LOG_FILE_LINE_OFF | DBJ_LOG_MT | DBJ_LOG_NO_CONSOLE
#endif

#ifndef  DBJ_LOG_DEFAULT_WITH_CONSOLE
#define DBJ_LOG_DEFAULT_WITH_CONSOLE DBJ_LOG_TO_APP_PATH | DBJ_LOG_FILE_LINE_OFF | DBJ_LOG_MT 
#endif
/////////////////////////////////////////////////////////////////////////////////////

	inline int dbj_simple_log_startup(const char* app_full_path)
	{
		assert(app_full_path);

		if (!dbj_log_setup(DBJ_LOG_DEFAULT_SETUP, app_full_path))
			return EXIT_FAILURE;

		log_trace(" %s", "                                                              ");
		log_trace(" %s", "--------------------------------------------------------------");
		log_trace(" %s", "                                                              ");
		log_trace(" Application: %s", app_full_path);
		log_trace(" Log file: %s", current_log_file_path());
		log_trace(" %s", "                                                              ");

#ifdef DBJ_LOG_TESTING
		log_trace("Log  TRACE");
		log_debug("Log  DEBUG");
		log_info("Log  INFO");
		log_warn("Log  WARN");
		log_error("Log  ERROR");
		log_fatal("Log  FATAL");
#endif
		return EXIT_SUCCESS;
	}

#ifdef __cplusplus
} // extern "C" 
#endif // __cplusplus

#endif // _DBJ_SIMPLE_LOG_H_INCLUDED_
