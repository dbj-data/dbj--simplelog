/* (c) 2019-2020 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/ */

#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include "dbj_simple_log.h"

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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#ifndef errno_t
typedef int errno_t;
#endif


static struct {
  void *udata;
  log_lock_function_ptr lock;
  FILE *fp;
  int level;
  int quiet;
  bool file_line_show;
  char log_f_name[BUFSIZ];
} LOCAL = { 0, 0, 0, LOG_TRACE, 0, true, '\0'} ;

const char* const current_log_file_path() {
	return LOCAL.log_f_name;
}

static const char* set_log_file_name(const char new_name[BUFSIZ]) {

	errno_t rez = strncpy_s(LOCAL.log_f_name, BUFSIZ, new_name, BUFSIZ - 1);
	assert(rez == 0);
	return LOCAL.log_f_name;
}

static const char *level_names[] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#ifdef DBJ_LOG_USE_COLOR
static const char *level_colors[] = {
  "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};
#endif


static void lock(void)   {
  if (LOCAL.lock) {
    LOCAL.lock(LOCAL.udata, 1);
  }
}


static void unlock(void) {
  if (LOCAL.lock) {
    LOCAL.lock(LOCAL.udata, 0);
  }
}

void log_set_fileline(bool show) 
{
LOCAL.file_line_show = show;
}

void log_set_udata(void *udata) {
  LOCAL.udata = udata;
}

void log_set_lock(log_lock_function_ptr fn) {
  LOCAL.lock = fn;
}

void log_set_fp(FILE *fp, const char * file_path_name ) {

	assert( fp );
  LOCAL.fp = fp;

  if (!file_path_name) {
	  /* name not given */
	  LOCAL.log_f_name[0] = '\0';
	  return;
  }
  set_log_file_name(file_path_name );
}

void log_set_level(int level) {
  LOCAL.level = level;
}

// true for quiet enabled 
// in other words -- silent
void log_set_quiet(bool enable) {
	LOCAL.quiet = enable;
}


void log_log(int level, const char *file, int line, const char *fmt, ...) 
{
	/* Acquire lock */
	lock();
	
	if (level < LOCAL.level)   goto exit;

  /* Get current time */
  time_t t = time(NULL);
  struct tm lt;
	errno_t errno_rez  = localtime_s(&lt, &t);

  /* Log to console using stderr */
  if (!LOCAL.quiet) {
    va_list args;
    char buf[16];
    buf[strftime(buf, sizeof(buf), "%H:%M:%S", &lt)] = '\0';

#ifdef DBJ_LOG_USE_COLOR

	if (LOCAL.file_line_show) {
		fprintf(
			stderr, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
			buf, level_colors[level], level_names[level], file, line);
	}
	else {
		fprintf(
			stderr, "%s %s%-5s\x1b[0m ",
			buf, level_colors[level], level_names[level] );
	}
#else
	if ( LOCAL.file_line_show)
    fprintf(stderr, "%s %-5s %s:%d: ", buf, level_names[level], file, line);
	else
	fprintf(stderr, "%s %-5s ", buf, level_names[level]);
#endif

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    // fflush(stderr);
  } // log not quiet

  /* Log to file */
  if (LOCAL.fp) {
    va_list args;
    char buf[32];
    buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", & lt)] = '\0';
	if (LOCAL.file_line_show)
    fprintf(LOCAL.fp, "%s %-5s %s:%d: ", buf, level_names[level], file, line);
	else
    fprintf(LOCAL.fp, "%s %-5s: ", buf, level_names[level]);
    va_start(args, fmt);
    vfprintf(LOCAL.fp, fmt, args);
    va_end(args);
	fprintf(LOCAL.fp, "\n");

#ifdef _DEBUG
	int errcode = ferror(LOCAL.fp);
	if (errcode != 0) {
		perror("\n\nferror(LOCAL.fp);\n\n");
		clearerr_s(LOCAL.fp);
	}
#endif
    // fflush(LOCAL.fp);
  }

  exit :
  /* Release lock */
  unlock();
}

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

bool enable_vt_mode()
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
// typedef void (*log_lock_function_ptr)(void* udata, int lock);

void  default_protector_function(void* udata, bool lock)
{
	static CRITICAL_SECTION   CS_ ;

	(void)udata; // ignore for now

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

// #include "dbj_fhandle.h"
#include <string.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <assert.h>

// _fstat
#include <sys/stat.h>
#include <sys/types.h>

#define dbj_fhandle_bad_descriptor -1 

dbj_fhandle dbj_fhandle_make(const char* name_)
{
	dbj_fhandle fh = { '\0', dbj_fhandle_bad_descriptor };
	int rez = _snprintf_s(fh.name, dbj_fhandle_max_name_len, _TRUNCATE, "%s.%s", name_, DBJ_FHANDLE_SUFFIX);
	assert(rez > 0);
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
errno_t  dbj_fhandle_assure(dbj_fhandle* self)
{
	assert(self);
	assert(self->name);

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
		assert(false);
		return ENODEV;
		break;
	}
	return (errno_t)0;
};

// there can be only one
FILE* dbj_fhandle_log_file_ptr(FILE* next_fp_)
{
	static FILE* single_fp_ = NULL;

	if (next_fp_) {
		// must have closed previous explicitly before
		assert(single_fp_ == NULL);
		single_fp_ = next_fp_;
	}

	return single_fp_;
}
/*
ATTENTION! dbj_fhandle_assure must be called before this to assure the file handle from name given
ATTENTION! file_handle.file_ptr() returns FILE * which is not explicitly closed by this lib
because this is a log file ...
it is left to the OS to take care of ... Stupid or clever? I am not sure...

NOTE: After _fdopen, close by using fclose, not _close.
if (fp_) { ::fclose( fp_) ; fp_ = nullptr; }
*/



FILE* dbj_fhandle_file_ptr(dbj_fhandle* self /* const char* options_ */)
{
	// https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/fdopen-wfdopen?view=vs-2019
	// "c" is important
	// c --	Enable the commit flag for the associated filename so that the contents of the file
	//  buffer are written directly to disk if either fflush or _flushall is called.
	static const char* default_open_mode = "wc";

	const char* options_ = default_open_mode;
	assert(options_);
	assert(self->file_descriptor > dbj_fhandle_bad_descriptor);
	// Associates a stream with a file that was previously opened for low-level I/O.
	FILE* fp_ = dbj_fhandle_log_file_ptr(
		_fdopen(self->file_descriptor, options_)
	);
	assert(fp_ != NULL);
	DBJ_FERROR(fp_);
	return fp_;
}

errno_t  dbj_fhandle_commit(dbj_fhandle* self)
{
	assert(self);
	int rez = _commit(self->file_descriptor);
	assert(rez == 0);
	return rez;
}
