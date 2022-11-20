/*
this is of course just a very quick check
and shows how is dbj_simple_log to be used
*/

#define DBJ_SIMPLELOG_IMPLEMENTATION
#include "dbj_simple_log.h"

// we override the default setup here
#undef  DBJ_LOG_DEFAULT_SETUP
#define DBJ_LOG_DEFAULT_SETUP ( DBJ_LOG_TO_FILE | DBJ_LOG_MT | DBJ_LOG_FILELINE_SHOW | DBJ_LOG_FULL_TIMESTAMP )

#include "dbj_simple_log.c"

/*
filter whatever SEH is required to be filtered
*/
static inline int seh_filter(unsigned int code, struct _EXCEPTION_POINTERS* ep)
{
	// as an example
	if (code == EXCEPTION_ACCESS_VIOLATION)
	{
		puts("caught AV! ");
		return EXCEPTION_EXECUTE_HANDLER;
	}
	else
	{
		puts("unexpected SE!");
		return EXCEPTION_CONTINUE_SEARCH;
	};
}

int main(const int argc, char* argv[])
{
	__try {
		__try {
			dbj_simple_log_test(0);
		}
		__finally
		{
			puts("\ntermination: ");
			puts(AbnormalTermination() ? "\tabnormal" : "\tnormal");
		}
	}
	__except (
		seh_filter(GetExceptionCode(), GetExceptionInformation())
		)
	{
		puts("SEH exception caught");
		// roadmap: minidump creation.
	}
	return 0;
}