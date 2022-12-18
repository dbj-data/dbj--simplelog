/*
this is of course just a very quick check
and shows how is dbj_simple_log to be used
*/

#define DBJ_SIMPLELOG_IMPLEMENTATION
#include "dbj_simple_log.h"

// we override the default setup here
#undef  DBJ_LOG_DEFAULT_SETUP
#define DBJ_LOG_DEFAULT_SETUP ( DBJ_LOG_TO_FILE | DBJ_LOG_MT | DBJ_LOG_FILELINE_SHOW | DBJ_LOG_FULL_TIMESTAMP )

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
#include "dbj_simple_log.c"
#ifdef __cplusplus
} // extern "C" {
#endif // __cplusplus

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
				// 
				constexpr auto MSCVER = _MSC_VER; // 1921
				constexpr auto MSCFULLVER = _MSC_FULL_VER; //192127702
				constexpr auto MSCBUILD = _MSC_BUILD; // 2
				/*
				: cl Version 19.21.27702.2 for x86
				: error C2131:  expression did not evaluate to a constant
				: message :  failure was caused by non-constant arguments or reference to a non-constant symbol
				: message :  see usage of '__LINE__Var'
				*/
				constexpr auto LINE = __LINE__;

				LOG_DEBUG("%d", MSCVER);
				LOG_DEBUG("%d", MSCFULLVER);
				LOG_DEBUG("%d", MSCBUILD);
				LOG_DEBUG("%d", LINE);

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