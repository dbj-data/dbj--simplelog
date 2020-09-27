<h1>dbj--simplelog</h1>

**simple, resilient, fast, local log**

> (c) 2019-2020 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/ 

- [Why logging?](#why-logging)
- [1. How to use](#1-how-to-use)
  - [1.1. You need to start it](#11-you-need-to-start-it)
  - [1.2. You need to end it](#12-you-need-to-end-it)
- [2. Setup options](#2-setup-options)
  - [2.1. DBJ_LOG_DEFAULT_SETUP](#21-dbj_log_default_setup)
- [3. Building](#3-building)

## Why logging?

C/C++ code is used to build software in the components making a back tier. Without UI and without human presence.

Each data center app or just simply server side app will need to have some logging used. There is no console. And administrators need to see the diagnostic or whichever output from your code.

This is double personality logging to two targets. Console and a log file. The log file is named equal to full application path + ".log".

Console output is used while testing and debugging. Be sure to switch off console logs in release builds.

Also in here there is a "resilience in presence of multiple threads ", built in.

## 1. How to use

Through these macros

```cpp
log_trace(...) ;

log_debug(...) ;

log_info(...);

log_warn(...);

log_error(...) ;

log_fatal(...) ;
```

Usage syntax is exactly the same as for the `printf` family, format string.

If `DBJ_LOG_USE_COLOR` is defined output is coloured. By default it is.

![coloured view in vs code](doc/in_vs_code.jpg)

NOTE: above is VS Code view which indeed is coloured. But colors are not the same as on Windows 10 console.

### 1.1. You need to start it
On start-up one can use the set-up function `dbj_log_setup`. But we recommend to use the `dbj_simple_log_startup(const char* app_full_path)` 
immediately after main() starts.

```cpp
int main( const int argc, char * argv[] )
{
    if (EXIT_SUCCESS != dbj_simple_log_startup(argv[0]))
        return EXIT_FAILURE;

// log file is named as: argv[0] + ".log".
    const char * lfp = current_log_file_path();

 ... the rest of the main ...

```
> Important: log file is reset on each application run. That is the current policy.

It is on the **roadmap** to offer several setup combinations to use on startup. Including log file opening options

A bit of a advice: be very careful if not using absolute paths. One can very easily loose the log file, if using the relative paths.

### 1.2. You need to end it

This is a Windows lib. And Windows is notorious for being very reluctant to flush. Thus please make sure at application end, you call `dbj_log_finalize()`. Otherwise lof file content might not show.

**Roadmap** is to encapsulate this solution inside the library.


## 2. Setup options

| Setup tag  | the effect  |
|---|---|
 DBJ_LOG_MT | set the Multi Threaded protection
DBJ_LOG_TO_APP_PATH  | If app full path is given  use it to obtain log file name. Make sure you use absolute paths.
DBJ_LOG_FILE_LINE_OFF | Exclude file and line, from time stamp 
DBJ_LOG_NO_CONSOLE | No console output. Beware, if this is set and no file path is given you will have no logging. 

### 2.1. DBJ_LOG_DEFAULT_SETUP

Users can define this macro to change the default setup. 
Default setup is defined as:

```
#define DBJ_LOG_DEFAULT_SETUP DBJ_LOG_TO_APP_PATH | DBJ_LOG_FILE_LINE_OFF | DBJ_LOG_MT | DBJ_LOG_NO_CONSOLE
```

For when you use the debug builds you might need a console output too. For that time we have defined `DBJ_LOG_DEFAULT_WITH_CONSOLE` which we (and you can) use like this (before including `dbj_simple_log.h`)

```cpp
/* we want console while in debug builds */
#ifdef _DEBUG
#define DBJ_LOG_DEFAULT_SETUP DBJ_LOG_DEFAULT_WITH_CONSOLE
#endif

#include "dbj--simplelog/dbj_simple_log.h"
```

## 3. Building

Built with CL.exe, which in reality means C99, but somewhat undocumented. **Roadmap** is to switch to clang 10.x and use C11 goodies.

How to use: best add it as a git submodule. Include `dbj_simple_log.h` 
Include it's only C file  `dbj_simple_log.c` in your project. Use.

-------

Based on `logc` lib by `rxi`. See the sub folder `logc`.

