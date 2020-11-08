#ifndef DBJ_SIMPLE_LOG_HOST_INC
#define DBJ_SIMPLE_LOG_HOST_INC
/* (c) 2019-2020 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/

Decision of where the log goes is done at compile time

Only host app includes this, only once, and makes that decision

Host app might include many other callers of the simple log
None of those users must not include this header

DBJ_LOG_DEFAULT_SETUP must be defined if users do not want the default setyp
*/

#ifdef __clang__
#pragma clang system_header
#endif // __clang__

#include "dbj_simple_log.h"

#ifndef WIN32_LEAN_AND_MEAN
// yes this is WIN only
#define NOMINMAX
#define STRICT 1
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif // ! WIN32_LEAN_AND_MEAN

#ifndef DBJ_ASSERT
#ifdef _DEBUG
#define DBJ_ASSERT _ASSERTE
#else
#define	DBJ_ASSERT(X_) ((void)(X_))
#endif
#endif // ! DBJ_ASSERT

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	int dbj_simple_log_startup(const char* /*app_full_path*/);
	/*
	ATTENTION! log file is not explicitly closed by this lib
	please make sure at application end, you call this, (somewhere clever as you do)

	returns EXIT_SUCCESS or EXIT_FAILURE
	*/
	int dbj_log_finalize(void);


	typedef enum DBJ_LOG_SETUP_ENUM {
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
		/* some quick minimal internal tests */
		DBJ_LOG_TESTING = 32
	} DBJ_LOG_SETUP_ENUM;

	/////////////////////////////////////////////////////////////////////////////////////
	/// predefined setups for both release and debug builds
	/// 
#undef  DBJ_LOG_DEFAULT_FILE_SETUP
#define DBJ_LOG_DEFAULT_FILE_SETUP DBJ_LOG_TO_FILE | DBJ_LOG_MT | DBJ_LOG_NO_CONSOLE

#undef  DBJ_LOG_DEFAULT_WITH_CONSOLE
#define DBJ_LOG_DEFAULT_WITH_CONSOLE DBJ_LOG_MT 

	/*
	--------------------------------------------------------------------------------------
	users must give a value to this before dbj simplelog is initialized
	*/
	extern int dbj_simple_log_setup_;

	/*
	--------------------------------------------------------------------------------------
	clang-cl initialization and deinitialization will happen automagically
	C or C++

	for clang on win aka clang-cl.exe
	WARNING 2020 Q4: destructor works only if runtime lib is static lib!
	--------------------------------------------------------------------------------------
	*/
#ifdef __clang__
	__attribute__((constructor))
#endif
		inline void dbj_simplelog_before(void)
	{
		char app_full_path[1024] = { 0 };
		// Q: is __argv available for windows desktop apps?
		// A: no it is not
		// win32 required here
		int rez = GetModuleFileNameA(
			(HINSTANCE)NULL, app_full_path, 1024
		);
		DBJ_ASSERT(rez != 0);

		rez = dbj_simple_log_startup(app_full_path);
		DBJ_ASSERT(EXIT_SUCCESS == rez);
	}

#ifdef __clang__
	__attribute__((destructor))
#endif
		inline void dbj_simple_log_after(void) {
		int rez = dbj_log_finalize();
		DBJ_ASSERT(EXIT_SUCCESS == rez);
	}

	/* --------------------------------------------------------------------------------------*/
#ifdef __cplusplus
} // extern "C" 
#endif // __cplusplus
/* --------------------------------------------------------------------------------------*/

#ifndef __clang__
#ifdef __cplusplus
//
// in no clang c++ situation 
// here is the simple solution
// although, c++ 'static fiasco' might not 
// allow for this to happen early enough
//
struct simple_log_protector final {

	simple_log_protector() noexcept {
		dbj_simplelog_before();
	}

	~simple_log_protector() noexcept {
		dbj_simple_log_after();
	}
};

inline const simple_log_protector simple_log_protector__;

#endif // __cplusplus
#endif // __clang__


#ifndef __clang__
#ifndef __cplusplus

// note for eggheads: yes I know there is a way to code constructor for MSVC
// in C, but I deliberately do not want to use linker hacks
// instead I use clang-cl.exe
// if you feverishly oppose, please insert your MSVC implementation here

#pragma message("please make sure dbj_simple_log_startup() is called before app starts!")
#pragma message("please make sure dbj_log_finalize() is called before app exit!")

#endif // !__cplusplus
#endif // ! __clang__

#endif // DBJ_SIMPLE_LOG_HOST_INC
