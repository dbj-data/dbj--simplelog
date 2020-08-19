#pragma once
#ifndef _DBJ_SIMPLE_LOG_H_INCLUDED_
#define _DBJ_SIMPLE_LOG_H_INCLUDED_

/* (c) 2019/2020 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */

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

		// uses win32 critial section
		void  default_protector_function(void* udata, bool lock);

		enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

		void log_set_udata(void* );
		void log_set_lock(log_lock_function_ptr );
		/* dbj added the second argument, for file name/path */
		void log_set_fp(FILE* , const char * );
		void log_set_level(int );
		/* beware: if quiet and no file there is no logging at all */
		void log_set_quiet(bool );
		/* 0 do not add file line to log lines stamp, 1 add */
		void log_set_fileline( bool  );
		/**/
		const char * const current_log_file_path();

		void log_log(int /*level*/, const char* /*file*/, int /*line*/, const char* /*fmt*/, ...);

	 typedef enum  SETUP {
		MT = 1 , /* set to Multi Threaded */
		VT100_CON = 2, /* specificaly switch on the VT100 console mode */
		LOG_FROM_APP_PATH = 4, /* if app full path is given  use it to obtain log gile name*/
		FILE_LINE_OFF = 8, /* do not show file line on every log line */
		SILENT = 16 /* no console output, beware of no file and quiet */
	} SETUP ;

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
	(int setup /*= SETUP::VT100_CON */, const char* app_full_path /* = nullptr */
	) 
	{
		if ( DBJ_LOG_IS_SETUP( setup, FILE_LINE_OFF) ) {
			log_set_fileline(false);
		}
		else 
			log_set_fileline(false);

		if (DBJ_LOG_IS_SETUP(setup, VT100_CON)) {
				enable_vt_mode();
		}

		if (DBJ_LOG_IS_SETUP(setup, MT)) {
			log_set_lock( default_protector_function );
		}

		if (DBJ_LOG_IS_SETUP(setup, SILENT)) {
				log_set_quiet( true );
#ifdef _DEBUG
			if (app_full_path == NULL ) {
				perror(__FILE__ "\nWARNING: SILENT is set, but no log file is requested");
			}
#endif
		} else 
			log_set_quiet(false);

		// caller does not want any kind of local log file
		if (app_full_path == NULL) return true;

	dbj_fhandle log_fh = dbj_fhandle_make(app_full_path);

	errno_t status = dbj_fhandle_assure( &log_fh );

	assert(status == 0);

	log_set_fp(
		/*
	ATTENTION! file_handle.file_ptr() returns FILE * which is not explicitly closed by this lib
	it is left to the OS to take care of it?.
	fdopen mode is "wc"
	*/
	dbj_fhandle_file_ptr( &log_fh ) , log_fh.name 
	);
				return true;
	} // dbj_log_setup

#undef DBJ_LOG_IS_SETUP

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

	if (! dbj_log_setup ( LOG_FROM_APP_PATH | VT100_CON | FILE_LINE_OFF | MT | SILENT , app_full_path)
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
