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


	/*	for users to see*/
	const char* const current_log_file_path();

	/* for users to be able to flush and close in their finalizers */
	FILE* dbj_fhandle_log_file_ptr(FILE* next_fp_);

	// all eventually goes through here
	void dbj_simple_log_log(int /*level*/, const char* /*file*/, int /*line*/, const char* /*fmt*/, ...);

	bool dbj_log_setup (int , const char*  );

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

#ifdef __cplusplus
} // extern "C" 
#endif // __cplusplus

#endif // _DBJ_SIMPLE_LOG_H_INCLUDED_
