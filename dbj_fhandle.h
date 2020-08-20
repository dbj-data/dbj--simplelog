#pragma once
#ifndef DBJ_FHANDLE_INC
#define DBJ_FHANDLE_INC
/*
   log file handling
   
   (c) 2019-2020 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/ 

*/

#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <errno.h>
#include <stdio.h>
#include <stdbool.h>

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

#define DBJ_FHANDLE_SUFFIX "log"

#ifndef BUFSIZ
#define BUFSIZ 512
#endif // BUFSIZ

#define dbj_fhandle_max_name_len BUFSIZ * 2 

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef struct dbj_fhandle {
		char name[dbj_fhandle_max_name_len];
		int file_descriptor;
	} dbj_fhandle;


	/*
	name_ += DBJ_FHANDLE_SUFFIX
	*/
	dbj_fhandle dbj_fhandle_make(const char* name_);
/*
must be called after dbj_fhandle_make
assure file descriptor given file name
on error returns one of errno values
returns 0 on no error

Condition -- Message
EACCES	The given path is a directory, or the file is read-only, but an open-for-writing operation was attempted.
EEXIST	_O_CREAT and _O_EXCL flags were specified, but filename already exists.
EINVAL	Invalid oflag, shflag, or pmode argument, or pfh or filename was a null pointer.
EMFILE	No more file descriptors available.
ENOENT	File or path not found.
ENODEV	No such device
*/
	errno_t  dbj_fhandle_assure(dbj_fhandle* self);

	errno_t  dbj_fhandle_commit(dbj_fhandle* self);
/*

dbj_fhandle_assure must be called before this to assure the file handle from name given

file is opened in "wc" mode , see here
https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/fdopen-wfdopen?view=vs-2019
*/
	FILE* dbj_fhandle_file_ptr(dbj_fhandle* self );

/*
must not call dbj_fhandle_file_ptr if result of dbj_fhandle_log_file_ptr() != NULL

upon app exit use this to close the log file
NOTE: After _fdopen, close by using fclose, not _close.
if (fp_) { ::fclose( fp_) ; fp_ = nullptr; }
*/
	FILE* dbj_fhandle_log_file_ptr(FILE* next_fp_);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // !DBJ_FHANDLE_INC
