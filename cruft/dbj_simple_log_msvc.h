#ifndef DBJ_SIMPLE_LOG_HOST_INC
#define DBJ_SIMPLE_LOG_HOST_INC
/* 
(c) 2019-2021 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/

this is for users who use cl not clang-cl
*/
#ifndef __clang__

#include "dbj_simple_log.h"

#ifdef __cplusplus
namespace {
	//
	// in no clang yes c++ situation 
	// here is the simple solution
	// although, c++ 'static fiasco' might not 
	// allow for this to happen early enough
	//
	struct simple_log_protector final {

		simple_log_protector() = default;

		simple_log_protector(simple_log_protector const&) = delete;
		simple_log_protector & operator = simple_log_protector(simple_log_protector const &) = delete;

		simple_log_protector(simple_log_protector &&) = delete;
		simple_log_protector& operator = simple_log_protector(simple_log_protector &&) = delete;

		~simple_log_protector() noexcept {
			dbj_simple_log_after();
		}
	};

	inline const simple_log_protector simple_log_protector__;

} // ns
#else  // ! __cplusplus
#pragma message("MSVC C code: please make sure dbj_simple_log_startup() is called before app starts!")
#pragma message("MSVC C code: please make sure dbj_log_finalize() is called before app exit!")
#endif // ! __cplusplus
#endif // !__clang__

// note for eggheads: yes I know there is a way to code constructor for MSVC
// in C, but I deliberately do not want to use linker hacks
// instead I use clang-cl.exe
// if you feverishly oppose, please insert your MSVC implementation here

#endif // DBJ_SIMPLE_LOG_HOST_INC
