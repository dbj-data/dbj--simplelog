#pragma once
#ifndef DBJ_FHANDLE_INC
#define DBJ_FHANDLE_INC
/*
   log file handling
   
   (c) 2019-2020 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/ 

*/

#include <errno.h>
#include <stdio.h>

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
/*

dbj_fhandle_assure must be called before this to assure the file handle from name given

*/
	FILE* dbj_fhandle_file_ptr(dbj_fhandle* self, const char* options_ /*= "w"*/);

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
