#ifndef _DBJ_SIMPLE_LOG_H_INCLUDED_
#define _DBJ_SIMPLE_LOG_H_INCLUDED_

/* (c) 2019-2022 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/ */

///  BIG FAT WARNING
/// 
/// 	do not enter escape codes: \n \v \f \t \r \b
/// 	into your strings
/// 	if you do your file output will be strange
/// 	and we will not stop you :)
/// 


#ifndef __clang__
#error use CLANG compiler please
#endif // __clang__

#pragma clang system_header

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

#define DBJ_SIMPLE_LOG_MAJOR 5
#define DBJ_SIMPLE_LOG_MINOR 0
#define DBJ_SIMPLE_LOG_PATCH 0
#define DBJ_SIMPLE_LOG_VERSION "5.0.0"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

////////////////////////////////////////////////////////////////////////////////
#pragma region DBJ_SIMPLELOG_IMPLEMENTATION
#ifdef DBJ_SIMPLELOG_IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>
#include <errno.h>
#include <io.h> // is a tty

#undef DBJ_ASSERT
#define DBJ_ASSERT _ASSERTE
//
// CAUTION! DBJ_VERIFY affects release builds too
//  _ASSERT_AND_INVOKE_WATSON asserts in debug builds
//  in release builds it invokes watson
#undef DBJ_VERIFY
#define DBJ_VERIFY(x) _ASSERT_AND_INVOKE_WATSON(x)

// Here's a better C version (from Google's Chromium project):
#undef DBJ_COUNT_OF
#define DBJ_COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))


#undef  DBJ_PERROR 
#define DBJ_PERROR (perror(__FILE__ " # " _CRT_STRINGIZE(__LINE__))) 

#undef DBJ_FERROR
#define DBJ_FERROR( FP_) \
do { \
if (ferror(FP_) != 0) 	{\
	DBJ_PERROR ;\
	clearerr_s(FP_);\
} \
} while(0)

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
// DBJ FHANDLE IMPLEMENTATION
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/* (c) 2019-2022 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/ */

// user definable
#ifndef DBJ_FHANDLE_SUFFIX
#define DBJ_FHANDLE_SUFFIX "log"
#endif

#define dbj_fhandle_max_name_len BUFSIZ * 2 

	typedef struct dbj_fhandle {
		char name[dbj_fhandle_max_name_len];
		int file_descriptor;
	} dbj_fhandle;


#include <sys/stat.h> // _fstat
#include <sys/types.h>
#include <fcntl.h>
#include <io.h> // is a tty

#define dbj_fhandle_bad_descriptor -1 

	// there can be only one
	__inline FILE* dbj_fhandle_log_file_ptr(FILE* next_fp_)
	{
		static FILE* single_fp_ = NULL;

		if (next_fp_) {
			// must have closed previous explicitly before
			DBJ_ASSERT(single_fp_ == NULL);
			single_fp_ = next_fp_;
		}

		return single_fp_;
	}

	__inline bool dbj_fhandle_is_empty(dbj_fhandle* self)
	{
		DBJ_ASSERT(self);
		return /*(self->name) ||*/ (self->name[0] == '\0');
	}

__inline dbj_fhandle dbj_fhandle_make(const char* name_)
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
__inline errno_t  dbj_fhandle_assure(dbj_fhandle* self)
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

__inline FILE* dbj_fhandle_file_ptr(dbj_fhandle* self /* const char* options_ */)
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
////////////////////////////////////////////////////////////////////////////////
#endif // DBJ_SIMPLELOG_IMPLEMENTATION
#pragma endregion DBJ_SIMPLELOG_IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////


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
	/// this is used inside the constructor
	/// by default we log to console and to file and 
	/// we lock each log call
#undef  DBJ_LOG_DEFAULT_SETUP
#define DBJ_LOG_DEFAULT_SETUP ( DBJ_LOG_TO_FILE | DBJ_LOG_MT )

	// can be used from other parts,
	// not just an host app
	void dbj_simple_log_test(const char*);

	/*	for users */
	const char* const dbj_simplelog_file_path();

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
