/* (c) 2019-2020 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/ */
/*
 * Copyright (c) 2017 rxi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include "dbj_fhandle.h"
#include "dbj_simple_log.h"

 // pch.h is implicitly included
 // and this is in there
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <fcntl.h>
#include <crtdbg.h>
#include <errno.h>
#include <io.h> // is a tty

static const char* level_names[] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#ifndef errno_t
typedef int errno_t;
#endif

// dbj::simplelog::log_lock_function_ptr 
// user_data is user data pointer
// lock == true  -- lock
// lock == false -- unlock
typedef void (*log_lock_function_ptr)(bool /*lock*/);

// default uses win32 critial section
// implemented in here
// user_data is unused 
void  default_protector_function(bool /*lock*/);

static struct {
	void* user_data;
	log_lock_function_ptr lock;
	FILE* fp;
	int level;
	int no_console;
	bool file_line_show;
	/* default is false, that means: time only */
	bool full_time_stamp;
	char log_f_name[BUFSIZ];
} LOCAL = {
		// defaults
	.user_data = 0,
	.lock = 0,
	.fp = 0,
	.level = DBJ_LOG_TRACE,
	.no_console = 0,
	.file_line_show = false,
	.full_time_stamp = false,
	 .log_f_name = {'\0'} };

const char* const current_log_file_path() {
	return LOCAL.log_f_name;
}

static const char* set_log_file_name(const char new_name[BUFSIZ]) {

	errno_t rez = strncpy_s(LOCAL.log_f_name, BUFSIZ, new_name, BUFSIZ - 1);
	DBJ_ASSERT(rez == 0);
	return LOCAL.log_f_name;
}



enum { dbj_COLOR_RESET = 0, dbj_LIGHT_GRAY = 1 };

#ifdef DBJ_LOG_USE_COLOR

#define VT100_ESC "\x1b["
#define VT100_RESET VT100_ESC "0m"
#define VT100_LIGHT_GRAY VT100_ESC "90m"
#define VT100_LIGHT_BLUE VT100_ESC "94m"
#define VT100_LIGHT_CYAN VT100_ESC "36m"
#define VT100_LIGHT_YELLOW VT100_ESC "33m"
#define VT100_LIGHT_GREEN VT100_ESC "32m"
#define VT100_LIGHT_RED VT100_ESC "31m"
#define VT100_LIGHT_MAGENTA VT100_ESC "35m"

static const char* level_colors[] = {
	/* TRACE */  VT100_LIGHT_BLUE,
	/* DEBUG */ VT100_LIGHT_CYAN,
	/* INFO */ VT100_LIGHT_GREEN,
	/* WARN */ VT100_LIGHT_YELLOW,
	/* ERROR */ VT100_LIGHT_RED,
	/* FATAL */ VT100_LIGHT_MAGENTA
};

static const char* colors_[] = {
	/* RESET */ VT100_RESET, /* dbj_LIGHT_GRAY */ VT100_LIGHT_GRAY
};
#else
static const char* level_colors[] = {
	/* TRACE */  "",
	/* DEBUG */ "",
	/* INFO */ "",
	/* WARN */ "",
	/* ERROR */ "",
	/* FATAL */ ""
};
static const char* colors_[] = {
	/* RESET */ "", /* dbj_LIGHT_GRAY */ ""
};
#endif


static void lock(void) {
	if (LOCAL.lock) {
		LOCAL.lock(true);
	}
}

static void unlock(void) {
	if (LOCAL.lock) {
		LOCAL.lock(false);
	}
}

static void log_set_fp(FILE* fp, const char* file_path_name) {

	DBJ_ASSERT(fp);
	LOCAL.fp = fp;

	if (!file_path_name) {
		/* name not given */
		LOCAL.log_f_name[0] = '\0';
		return;
	}
	set_log_file_name(file_path_name);
}

// not used currently --> void log_set_level(int level) {  LOCAL.level = level; }

static void time_stamp_(char(*buf)[32], bool short_)
{
	time_t t = time(NULL);
	struct tm lt;
	errno_t errno_rez = localtime_s(&lt, &t);
	DBJ_ASSERT(errno_rez == 0);
	if (short_)
		(*buf)[strftime((*buf), sizeof(*buf), "%H:%M:%S", &lt)] = '\0';
	else
		(*buf)[strftime((*buf), sizeof(*buf), "%Y-%m-%d %H:%M:%S", &lt)] = '\0';
}

////////////////////////////////////////////////////////////////////////////////
/// here the logging is actually done
void dbj_simple_log_log(int level, const char* file, int line, const char* fmt, ...)
{
	/* Acquire lock, if MT was part of the setup */
	lock();

	char timestamp_[32] = { 0 };

	if (LOCAL.full_time_stamp)
		time_stamp_(&timestamp_, false);
	else
		time_stamp_(&timestamp_, true);

	/* Log to console using stderr */
	if (!LOCAL.no_console) {

		if (LOCAL.file_line_show) {
			fprintf(
				stderr, "%s %s%-5s%s%s%s(%d) : %s",
				timestamp_, level_colors[level], level_names[level], colors_[dbj_COLOR_RESET], colors_[dbj_LIGHT_GRAY], file, line, colors_[dbj_COLOR_RESET]
			);
		}
		else {
			fprintf(
				stderr, "%s %s%-5s%s",
				timestamp_, level_colors[level], level_names[level], colors_[dbj_COLOR_RESET]
			);
		}

		va_list args;
		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		va_end(args);
		fprintf(stderr, "\n");

	} // eof log to console using stderr

	/* Log to file */
	if (LOCAL.fp) {
		va_list args;

		if (LOCAL.file_line_show)
			fprintf(LOCAL.fp, "%s %-5s %s:%d: ", timestamp_, level_names[level], file, line);
		else
			fprintf(LOCAL.fp, "%s %-5s: ", timestamp_, level_names[level]);
		/*
		ONE: we do not filter out the escape chars
		*/
		va_start(args, fmt);
		vfprintf(LOCAL.fp, fmt, args);
		va_end(args);

		/*
		TWO: we do add a new line to each line written
		but. we do not really want to do it in here ...?
		*/
		fprintf(LOCAL.fp, "\n");

		DBJ_FERROR(LOCAL.fp);

#ifdef DBJ_SIMPLE_LOG_AUTO_FLUSH
		DBJ_ASSERT(LOCAL.fp);
		DBJ_FERROR(LOCAL.fp);
		(void)_flushall();
#endif // DBJ_SIMPLE_LOG_AUTO_FLUSH

	}

	/* Release lock */
	unlock();
}

////////////////////////////////////////////////////////////////////////////////
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

static bool enable_vt_mode()
{
	// Set output mode to handle virtual terminal sequences
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	// this will fail if this app output is 
	// redirected to a file
	DWORD dwMode = 0;
	if (!GetConsoleMode(hOut, &dwMode))
	{
		return false;
	}
	/*
	Virtual terminal mode is available in the console starting with
	Windows 10.0.10586. It's not supported by the OS if setting
	the mode fails with ERROR_INVALID_PARAMETER (87). Also,
	it's only implemented in the new console. With the legacy
	console selected in Windows 10, enabling VT mode may succeed,
	but it won't actually enable VT support.
	*/
#ifndef	ENABLE_VIRTUAL_TERMINAL_PROCESSING
	return false;
#endif

	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	if (!SetConsoleMode(hOut, dwMode))
	{
		return false;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// typedef void (*log_lock_function_ptr)(void* user_data, int lock);

/*
this actually might be NOT slower than using some global CRITICAL_SECTION
and entering/deleting it only once ... why not measuring it?
*/
static void  default_protector_function(bool lock)
{
	// Think: this is one per process
	static CRITICAL_SECTION   CS_;

	if (lock)
	{
		InitializeCriticalSection(&CS_);
		EnterCriticalSection(&CS_);
	}
	else {
		LeaveCriticalSection(&CS_);
		DeleteCriticalSection(&CS_);
	}
}

/*
if in need of some kindergarten enum masks handling refresher please look here
https://stackoverflow.com/a/12255836/10870835
*/
#undef  DBJ_LOG_IS_BIT
#define DBJ_LOG_IS_BIT(S_, B_) ( 0 != ((S_) & (B_)) )

static bool dbj_log_setup
(/*DBJ_LOG_SETUP_ENUM*/ const int setup, const char* app_full_path)
{
	const bool file_log_ = DBJ_LOG_IS_BIT(setup, DBJ_LOG_TO_FILE);

	LOCAL.full_time_stamp = DBJ_LOG_IS_BIT(setup, DBJ_LOG_FULL_TIMESTAMP);
	LOCAL.file_line_show = DBJ_LOG_IS_BIT(setup, DBJ_LOG_FILELINE_SHOW);
	LOCAL.no_console = DBJ_LOG_IS_BIT(setup, DBJ_LOG_NO_CONSOLE);
	LOCAL.lock = DBJ_LOG_IS_BIT(setup, DBJ_LOG_MT) ? default_protector_function : NULL;

	if (LOCAL.no_console)
		if (!file_log_)
		{
			DBJ_VERIFY(LOCAL.no_console || file_log_);
			// run-time feeble attempt
			// WARNING: DBJ_LOG_NO_CONSOLE is set, but no log file is requested?
			DBJ_PERROR;
		}

	// caller does not want any kind of local log file
	if (!file_log_)
		// app_full_path ignored here
	{
#ifdef DBJ_LOG_USE_COLOR
		enable_vt_mode();
#endif
		return true;
	}

	// make it once
	static dbj_fhandle log_file_handle_shared_;

	if (dbj_fhandle_is_empty(&log_file_handle_shared_))
	{
		log_file_handle_shared_ = dbj_fhandle_make(app_full_path);
	}

	// assure file handle is propely open and set
	errno_t status = dbj_fhandle_assure(&log_file_handle_shared_);

	DBJ_ASSERT(status == 0);

	log_set_fp(
		dbj_fhandle_file_ptr(&log_file_handle_shared_), log_file_handle_shared_.name
	);

	// we keep it in user_data void * so we decouple from dbj_fhandle
	LOCAL.user_data = (&log_file_handle_shared_);

	return true;
} // dbj_log_setup

#undef DBJ_LOG_IS_BIT

static bool startup_done = false;

// make sure you call this once upon app startup
// make sure DBJ_LOG_DEFAULT_SETUP is set to combinaion 
// you want before calling this function
int dbj_simple_log_startup(
	/*DBJ_LOG_SETUP*/ int dbj_simple_log_setup_, 
	const char app_full_path[BUFSIZ]
)
{
	if (startup_done) return EXIT_SUCCESS;
	// users must give value to this
	// before this is called
	// ideally before simplelog is ever used
	// (void)/*extern int*/ dbj_simple_log_setup_;

	if (dbj_simple_log_setup_ & DBJ_LOG_TO_FILE) {
		if (!app_full_path) return EXIT_FAILURE;
	}

	if (!dbj_log_setup(dbj_simple_log_setup_, app_full_path))
		return EXIT_FAILURE;

	// this will thus go into which ever log target you have set
	// log file or console or both or none

	char full_tstamp_[32] = { 0 };
	time_stamp_(&full_tstamp_, false);

	dbj_log_info(" %s", "                                                              ");
	dbj_log_info(" %s", "--------------------------------------------------------------");
	dbj_log_info(" Start time: %s", full_tstamp_);
	dbj_log_info(" %s", "                                                              ");
	if (dbj_simple_log_setup_ & DBJ_LOG_TO_FILE)
		dbj_log_info(" Log file: %s", current_log_file_path());
	dbj_log_info(" %s", "                                                              ");

	startup_done = true;
	return EXIT_SUCCESS;
}

/* public API too */
void dbj_simple_log_test(const char* dummy_)
{
	dbj_log_info(" ");
	dbj_log_info("BEGIN Internal Test");
	dbj_log_info(" ");
	dbj_log_info("LOCAL.user_data       :  %4X", LOCAL.user_data);
	dbj_log_info("LOCAL.lock            :  %4X", LOCAL.lock);
	dbj_log_info("LOCAL.fp              :  %4X", LOCAL.fp);
	dbj_log_info("LOCAL.level           :  %d", LOCAL.level);
	dbj_log_info("LOCAL.no_console      :  %d", LOCAL.no_console);
	dbj_log_info("LOCAL.file_line_show  :  %s", LOCAL.file_line_show ? "true" : "false");
	dbj_log_info("LOCAL.full_time_stamp :  %s", LOCAL.full_time_stamp ? "true" : "false");
	dbj_log_info("LOCAL.log_f_name set  :  %s", (LOCAL.log_f_name[0]) ? "true" : "false");
	dbj_log_info(" ");
	dbj_log_trace("Log  TRACE");
	dbj_log_debug("Log  DEBUG");
	dbj_log_info("Log  INFO");
	dbj_log_warn("Log  WARN");
	dbj_log_error("Log  ERROR");
	dbj_log_fatal("Log  FATAL");
	dbj_log_info(" ");
	dbj_log_info("END Internal Test");
	dbj_log_info(" ");
}

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
// this might assert on debug builds
// make sure it does not, on release builds
int dbj_simplelog_finalize(void)
{
	// make sure setup was called 
	dbj_fhandle* fh = LOCAL.user_data;

	// log file was not made
	// the session was in a console mode
	if (fh == NULL) return EXIT_SUCCESS;

	FILE* fp_ = dbj_fhandle_log_file_ptr(NULL);
	DBJ_ASSERT(fp_);
	DBJ_FERROR(fp_);
	(void)_flushall();
	// make sure it is fclose, not close
	if (fp_) {
		fclose(fp_); fp_ = NULL;
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
// DBJ FHANDLE IMPLEMENTATION
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/* (c) 2019-2020 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/ */

#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif
// _fstat
#include <sys/stat.h>
#include <sys/types.h>

#define dbj_fhandle_bad_descriptor -1 

static dbj_fhandle dbj_fhandle_make(const char* name_)
{
	dbj_fhandle fh = { {'\0'}, dbj_fhandle_bad_descriptor };
	int rez = _snprintf_s(fh.name, dbj_fhandle_max_name_len, _TRUNCATE, "%s.%s", name_, DBJ_FHANDLE_SUFFIX);
	DBJ_ASSERT(rez > 0);
	return fh;
}

/*
assure file descriptor given file name
on error returns one of errno values

Condition -- Message
EACCES	The given path is a directory, or the file is read-only, but an open-for-writing operation was attempted.
EEXIST	_O_CREAT and _O_EXCL flags were specified, but filename already exists.
EINVAL	Invalid oflag, shflag, or pmode argument, or pfh or filename was a null pointer.
EMFILE	No more file descriptors available.
ENOENT	File or path not found.
ENODEV	No such device
*/
static errno_t  dbj_fhandle_assure(dbj_fhandle* self)
{
	DBJ_ASSERT(self);
	DBJ_ASSERT(self->name);

	// int fd = self->file_descriptor;

	errno_t rez = _sopen_s(&self->file_descriptor, self->name,
		_O_TRUNC | O_CREAT | _O_WRONLY,
		/* sharing settings    */
		_SH_DENYNO,
		/* permission settings */
		_S_IWRITE);

	if (rez != 0) {
		DBJ_PERROR;
		self->file_descriptor = dbj_fhandle_bad_descriptor;
		return rez;
	}

	struct _stat64i32 sb;

	rez = _fstat(self->file_descriptor, &sb);
	if (rez != 0) {
		DBJ_PERROR;
		self->file_descriptor = dbj_fhandle_bad_descriptor;
		return rez;
	}

	switch (sb.st_mode & S_IFMT) {
	case S_IFCHR:  //character device
#ifndef _MSC_VER
	case S_IFIFO:  //FIFO/pipe
	case S_IFLNK:  //symlink
#endif // !_MSC_VER
	case S_IFREG:  //regular file
		break;
	default:
		DBJ_ASSERT(false);
		return ENODEV;
		break;
	}
	return (errno_t)0;
};

/*
ATTENTION! dbj_fhandle_assure must be called before this to assure the file handle from name given
ATTENTION! file_handle.file_ptr() returns FILE * which is not explicitly closed by this lib
because this is a log file ...
it is left to the OS to take care of ... Stupid or clever? I am not sure...

NOTE: After _fdopen, close by using fclose, not _close.
if (fp_) { ::fclose( fp_) ; fp_ = nullptr; }
*/

static FILE* dbj_fhandle_file_ptr(dbj_fhandle* self /* const char* options_ */)
{
	// https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/fdopen-wfdopen?view=vs-2019
	// "c" is important
	// c --	Enable the commit flag for the associated filename so that the contents of the file
	//  buffer are written directly to disk if either fflush or _flushall is called.
	static const char* default_open_mode = "wc";

	const char* options_ = default_open_mode;
	DBJ_ASSERT(options_);
	DBJ_ASSERT(self->file_descriptor > dbj_fhandle_bad_descriptor);
	// Associates a stream with a file that was previously opened for low-level I/O.
	FILE* fp_ = dbj_fhandle_log_file_ptr(
		_fdopen(self->file_descriptor, options_)
	);
	DBJ_ASSERT(fp_ != NULL);
	DBJ_FERROR(fp_);
	return fp_;
}

////////////////////////////////////////////////////////////////////////////////////
#ifdef __clang__
__attribute__((destructor))
inline void dbj_simple_log_destructor (void) {
	int rez = dbj_simplelog_finalize();
	_ASSERTE(EXIT_SUCCESS == rez);
}


////////////////////////////////////////////////////////////////////////////////////
// WARNING: not used and not tested yet
#if DBJ_SIMPLELOG_CLANG_CONSTRUCTOR 

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


__attribute__((constructor))
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
#endif // DBJ_SIMPLELOG_CLANG_CONSTRUCTOR

#endif // __clang__



