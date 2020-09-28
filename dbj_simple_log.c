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
//#include <stdio.h>
//#include <stdlib.h>
//#include <stdarg.h>
//#include <string.h>
//#include <time.h>
//#include <stdbool.h>

#ifndef errno_t
typedef int errno_t;
#endif

/*
currently not public API
making it so is of a questionable value as it inevitably 
will complicate the public API
*/

// someday soon WIN10 will not need this
// as an explicit call
bool enable_vt_mode();

// dbj::simplelog::log_lock_function_ptr 
// user_data is user data pointer
// lock == true  -- lock
// lock == false -- unlock
typedef void (*log_lock_function_ptr)(bool /*lock*/);

// default uses win32 critial section
// implemented in here
// user_data is unused 
void  default_protector_function( bool /*lock*/);

// works but currently unused
// primary use is to keep mutex or whatever else 
// to be used by user locking function
void log_set_user_data(void*);
// user locking function is set here
// has to match log_lock_function_ptr signatured, and logic
void log_set_lock(log_lock_function_ptr);

void log_set_level(int);

/* dbj added the second argument, for full file path */
void log_set_fp(FILE*, const char*);

/* beware: if quiet and no file there is no logging at all */
void log_set_quiet(bool);
/* do not add file + line to log lines stamp, or add */
void log_set_fileline(bool);

static struct {
  void *user_data;
  log_lock_function_ptr lock;
  FILE *fp;
  int level;
  int quiet;
  bool file_line_show;
  char log_f_name[BUFSIZ];
} LOCAL = { 0, 0, 0, DBJ_LOG_TRACE, 0, true, '\0'} ;

const char* const current_log_file_path() {
	return LOCAL.log_f_name;
}

static const char* set_log_file_name(const char new_name[BUFSIZ]) {

	errno_t rez = strncpy_s(LOCAL.log_f_name, BUFSIZ, new_name, BUFSIZ - 1);
	DBJ_ASSERT(rez == 0);
	return LOCAL.log_f_name;
}

static const char *level_names[] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

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

static const char *level_colors[] = {
  /* TRACE */  VT100_LIGHT_BLUE, 
  /* DEBUG */ VT100_LIGHT_CYAN,
  /* INFO */ VT100_LIGHT_GREEN,
  /* WARN */ VT100_LIGHT_YELLOW,
  /* ERROR */ VT100_LIGHT_RED,
  /* FATAL */ VT100_LIGHT_MAGENTA
};
#endif


static void lock(void)   {
  if (LOCAL.lock) {
    LOCAL.lock(true);
  }
}


static void unlock(void) {
  if (LOCAL.lock) {
    LOCAL.lock(false);
  }
}

static void log_set_fileline(bool show) 
{
LOCAL.file_line_show = show;
}

static void log_set_user_data(void *user_data) {
  LOCAL.user_data = user_data;
}

static void * log_get_user_data(void) {
	return LOCAL.user_data;
}


static void log_set_lock(log_lock_function_ptr fn) {
  LOCAL.lock = fn;
}

static void log_set_fp(FILE *fp, const char * file_path_name ) {

	DBJ_ASSERT( fp );
  LOCAL.fp = fp;

  if (!file_path_name) {
	  /* name not given */
	  LOCAL.log_f_name[0] = '\0';
	  return;
  }
  set_log_file_name(file_path_name );
}

// not used currently --> void log_set_level(int level) {  LOCAL.level = level; }

// true for quiet enabled 
// in other words -- silent
static void log_set_quiet(bool enable) {
	LOCAL.quiet = enable;
}

inline void time_stamp_short( char (*buf)[16] )
{
	time_t t = time(NULL);
	struct tm lt;
	errno_t errno_rez = localtime_s(&lt, &t);
	DBJ_ASSERT( errno_rez == 0 );
	(*buf)[strftime((*buf), sizeof(*buf), "%H:%M:%S", &lt)] = '\0';
}

inline void time_stamp_long( char (*buf)[32] )
{
	time_t t = time(NULL);
	struct tm lt;
	errno_t errno_rez = localtime_s(&lt, &t);
	DBJ_ASSERT( errno_rez == 0 );
	(*buf)[strftime((*buf), sizeof(*buf), "%Y-%m-%d %H:%M:%S", &lt)] = '\0';
}

void dbj_simple_log_log(int level, const char *file, int line, const char *fmt, ...) 
{
	/* Acquire lock */
	lock();
	
	// not used currently --> 	if (level < LOCAL.level)   goto exit;

	// errno_t errno_rez = 0; 

  /* Log to console using stderr */
  if (!LOCAL.quiet) {

	 va_list args;
	 char buf[16] = {0};
	time_stamp_short( & buf);

#ifdef DBJ_LOG_USE_COLOR

	if (LOCAL.file_line_show) {
		fprintf(
			stderr, "%s %s%-5s" VT100_RESET VT100_LIGHT_GRAY "%s : %d : "VT100_RESET ,
			buf, level_colors[level], level_names[level], file, line);
	}
	else {
		fprintf(
			stderr, "%s %s%-5s "VT100_RESET,
			buf, level_colors[level], level_names[level] );
	}
#else
	if ( LOCAL.file_line_show)
    fprintf(stderr, "%s %-5s %s : %d : ", buf, level_names[level], file, line);
	else
	fprintf(stderr, "%s %-5s ", buf, level_names[level]);
#endif

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");

  } // log not quiet

  /* Log to file */
  if (LOCAL.fp) {
    va_list args;

	char buf[32] = {0};
	time_stamp_long(& buf);

	if (LOCAL.file_line_show)
    fprintf(LOCAL.fp, "%s %-5s %s:%d: ", buf, level_names[level], file, line);
	else
    fprintf(LOCAL.fp, "%s %-5s: ", buf, level_names[level]);
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

	DBJ_FERROR( LOCAL.fp );
	
#ifdef DBJ_SIMPLE_LOG_AUTO_FLUSH
	DBJ_ASSERT( LOCAL.fp);
	DBJ_FERROR( LOCAL.fp);
	(void)_flushall();
#endif // DBJ_SIMPLE_LOG_AUTO_FLUSH

  }

  /* Release lock */
  unlock();
}

////////////////////////////////////////////////////////////////////////////////

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

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
	static CRITICAL_SECTION   CS_ ;

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

#undef  DBJ_LOG_IS_BIT
#define DBJ_LOG_IS_BIT(S_, B_) (((int)S_ & (int)B_) != 0)

bool dbj_log_setup
(int setup /* DBJ_LOG_SETUP_ENUM */, const char* app_full_path)
{
	if (DBJ_LOG_IS_BIT(setup, DBJ_LOG_FILE_LINE_OFF)) {
		log_set_fileline(false);
	}
	else
		log_set_fileline(true);

	// someday soon, windows console will not need this
	enable_vt_mode();

	if (DBJ_LOG_IS_BIT(setup, DBJ_LOG_MT)) {
		log_set_lock(default_protector_function);
	}

	if (DBJ_LOG_IS_BIT(setup, DBJ_LOG_NO_CONSOLE)) {
		log_set_quiet(true);
#ifdef _DEBUG
		if (app_full_path == NULL) {
			perror("\nWARNING: DBJ_LOG_NO_CONSOLE is set, but no log file is requested");
		}
#endif
	}
	else
		log_set_quiet(false);

	// caller does not want any kind of local log file
	if (app_full_path == NULL) return true;

	// make it once
	static dbj_fhandle log_file_handle_shared_ ;

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
	log_set_user_data( &log_file_handle_shared_);

	return true;
} // dbj_log_setup

#undef DBJ_LOG_IS_BIT

void dbj_log_finalize(void)
{
	// make sure setup was called 
	dbj_fhandle * fh = log_get_user_data();
	DBJ_ASSERT(fh);

	FILE* fp_ = dbj_fhandle_log_file_ptr(NULL);
	DBJ_ASSERT(fp_);
	DBJ_FERROR(fp_);
	(void)_flushall();
	// make sure it is fclose, not close
	if (fp_) { fclose(fp_); fp_ = NULL ; }
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
	dbj_fhandle fh = { '\0', dbj_fhandle_bad_descriptor };
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


