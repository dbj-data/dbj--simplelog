/* (c) 2019-2020 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/ */

#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include "dbj_fhandle.h"
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

	errno_t rez = _sopen_s(&self->file_descriptor, self->name, O_WRONLY | O_APPEND | O_CREAT, _SH_DENYNO, _S_IREAD | _S_IWRITE);

	if (rez != 0) {
		DBJ_PERROR;
		self->file_descriptor = dbj_fhandle_bad_descriptor;
		return rez;
	}

	struct stat sb;
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
		assert( false );
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

// https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/fdopen-wfdopen?view=vs-2019
// c is important
// c --	Enable the commit flag for the associated filename so that the contents of the file
//  buffer are written directly to disk if either fflush or _flushall is called.
static const char* default_open_mode = "wc";

FILE* dbj_fhandle_file_ptr(dbj_fhandle* self /* const char* options_ */ )
{
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
