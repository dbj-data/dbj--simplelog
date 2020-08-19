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

dbj_fhandle dbj_fhandle_make ( const char * name_ ) 
{
	dbj_fhandle fh = { '\0', dbj_fhandle_bad_descriptor } ;
	int rez = _snprintf_s( fh.name, dbj_fhandle_max_name_len, _TRUNCATE, "%s.%s", name_ , DBJ_FHANDLE_SUFFIX );
	assert( rez > 0 );
    return fh ;
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
errno_t  dbj_fhandle_assure( dbj_fhandle * self )
			{
				assert(self);
				const char* fn = self->name ;
				assert(fn);

				int fd = self->file_descriptor;

				errno_t rez = _sopen_s( &fd, fn, O_WRONLY | O_APPEND | O_CREAT, _SH_DENYNO, _S_IREAD | _S_IWRITE);

				if (rez != 0) {
					self->file_descriptor =  dbj_fhandle_bad_descriptor;
					return rez;
				}

				struct stat sb ;
				fstat(fd, &sb);

				switch (sb.st_mode & S_IFMT) {
				case S_IFCHR:  //printf("character device\n");        break;
#ifndef _MSC_VER
				case S_IFIFO:  //printf("FIFO/pipe\n");               break;
				case S_IFLNK:  //printf("symlink\n");                 break;
#endif // !_MSC_VER
				case S_IFREG:  //printf("regular file\n");            break;
					break;
				default:
					return ENODEV;
					break;
				}
					return (errno_t)0;
			};

	// there can be only one
	FILE * dbj_fhandle_log_file_ptr( FILE * next_fp_ )
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
FILE * dbj_fhandle_file_ptr( dbj_fhandle * self , const char* options_ /*= "w"*/ ) 
{
	assert( options_ );
	assert(self->file_descriptor > dbj_fhandle_bad_descriptor );
	// Associates a stream with a file that was previously opened for low-level I/O.
	FILE * fp_ = dbj_fhandle_log_file_ptr(
		_fdopen( self->file_descriptor, options_)
		) ;
	assert(fp_ != NULL);
	assert(0 == ferror(fp_));
	return fp_ ;
}
