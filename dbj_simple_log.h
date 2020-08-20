#pragma once
#ifndef _DBJ_SIMPLE_LOG_H_INCLUDED_
#define _DBJ_SIMPLE_LOG_H_INCLUDED_

/* (c) 2019-2020 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/ */

#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define DBJ_LOG_USE_COLOR
#define DBJ_SIMPLE_LOG_MAJOR 2
#define DBJ_SIMPLE_LOG_MINOR 0
#define DBJ_SIMPLE_LOG_PATCH 0
#define DBJ_SIMPLE_LOG_VERSION "2.0.0"

#include "dbj_fhandle.h"

#ifdef __cplusplus
	extern "C" {
#endif // __cplusplus

		// someday soon WIN10 will not need this
		// as an explicit call
		bool enable_vt_mode();

		// dbj::simplelog::log_lock_function_ptr 
		// udata is user data pointer
		// lock == true  -- lock
		// lock == false -- unlock
		typedef void (*log_lock_function_ptr)(void* /*udata*/, bool /*lock*/);

		// default uses win32 critial section
		// implemented in here
		// udata is unused 
		void  default_protector_function(void* /*udata*/, bool /*lock*/);

		// log levels
		enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

		// works but currently unused
		// primary use is to keep mutex or whatever else 
		// to be used by user locking function
		void log_set_udata(void* );
		// user locking function set here
		// has to match log_lock_function_ptr signatured, and logic
		void log_set_lock(log_lock_function_ptr );

		/* dbj added the second argument, for full file path */
		void log_set_fp(FILE* , const char * );

		void log_set_level(int );
		/* beware: if quiet and no file there is no logging at all */
		void log_set_quiet(bool );
		/* do not add file + line to log lines stamp, or add */
		void log_set_fileline( bool  );
		/*
		for users to see
		*/
		const char * const current_log_file_path();

		// all eventually goes here
		void log_log(int /*level*/, const char* /*file*/, int /*line*/, const char* /*fmt*/, ...);

	 typedef enum  DBJ_LOG_SETUP_ENUM {
		DBJ_LOG_MT = 1 , /* set to Multi Threaded */
		DBJ_LOG_VT100 = 2, /* specificaly switch on the VT100 console mode */
		DBJ_LOG_TO_APP_PATH = 4, /* if app full path is given  use it to obtain log gile name*/
		DBJ_LOG_FILE_LINE_OFF = 8, /* do not show file line on every log line */
		DBJ_LOG_NO_CONSOLE = 16 /* no console output, beware of no file and seting this in the same time */
	} DBJ_LOG_SETUP_ENUM ;

#undef  DBJ_LOG_IS_SETUP
#define DBJ_LOG_IS_SETUP(S_, B_) (((int)S_ & (int)B_) != 0)
	/*
	 ---------------------------------------------------------------------------------------------------------
	usage
	   void main() {
		dbj::simplelog::setup(__argv[0]);
	   }
	*/

	inline bool dbj_log_setup
	(int setup /*= DBJ_LOG_SETUP_ENUM::DBJ_LOG_VT100 */, const char* app_full_path /* = nullptr */
	) 
	{
		if ( DBJ_LOG_IS_SETUP( setup, DBJ_LOG_FILE_LINE_OFF) ) {
			log_set_fileline(false);
		}
		else 
			log_set_fileline(false);

		if (DBJ_LOG_IS_SETUP(setup, DBJ_LOG_VT100)) {
				enable_vt_mode();
		}

		if (DBJ_LOG_IS_SETUP(setup, DBJ_LOG_MT)) {
			log_set_lock( default_protector_function );
		}

		if (DBJ_LOG_IS_SETUP(setup, DBJ_LOG_NO_CONSOLE)) {
				log_set_quiet( true );
#ifdef _DEBUG
			if (app_full_path == NULL ) {
				perror(__FILE__ "\nWARNING: DBJ_LOG_NO_CONSOLE is set, but no log file is requested");
			}
#endif
		} else 
			log_set_quiet(false);

		// caller does not want any kind of local log file
		if (app_full_path == NULL) return true;

	dbj_fhandle log_fh = dbj_fhandle_make(app_full_path);

	// assure file handle is propely opened and set
	errno_t status = dbj_fhandle_assure( &log_fh );

	assert(status == 0);

	log_set_fp(
/*
	ATTENTION! file_handle.file_ptr() returns FILE * which is not explicitly closed by this lib
	please make sure at application end, you place this snippet, (somewhere clever as you do):

	FILE* fp_ = dbj_fhandle_log_file_ptr(NULL);
	assert(fp_);
	(void)_flushall();
		// make sure it is fclose, not close
	if (fp_) { fclose(fp_); fp_ = nullptr; }

	You must flush to see the log file contents eventually
*/
	dbj_fhandle_file_ptr( &log_fh ) , log_fh.name 
	);
				return true;
	} // dbj_log_setup

#undef DBJ_LOG_IS_SETUP

// primary usage is through these

#define log_trace(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  log_log(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_log(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

/////////////////////////////////////////////////////////////////////////////////////

inline int dbj_simple_log_startup(const char* app_full_path)
{
	assert( app_full_path );

	if (! dbj_log_setup ( DBJ_LOG_TO_APP_PATH | DBJ_LOG_VT100 | DBJ_LOG_FILE_LINE_OFF | DBJ_LOG_MT | DBJ_LOG_NO_CONSOLE , app_full_path)
		)
		return EXIT_FAILURE;

	log_trace(" %s", "--------------------------------------------------------------");
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
