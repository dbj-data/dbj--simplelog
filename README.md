<h1>dbj--simplelog</h1>

**simple, resilient, fast, local log**

> (c) 2019-2020 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/ 

- [1. Why logging?](#1-why-logging)
- [2. How to use](#2-how-to-use)
- [3. Alert! I have a name clash?!](#3-alert-i-have-a-name-clash)
  - [3.1. You need to start it properly](#31-you-need-to-start-it-properly)
  - [3.2. You need to end it properly](#32-you-need-to-end-it-properly)
    - [3.2.1. Autoflush](#321-autoflush)
- [4. Setup options](#4-setup-options)
  - [4.1. DBJ_LOG_DEFAULT_SETUP](#41-dbj_log_default_setup)
- [5. Building the thing](#5-building-the-thing)

## 1. Why logging?

C/C++ code is used to build software in the components making a back tier. Without UI and without human presence.

Each data center app or just simply server side app will need to have some logging used. There is no console. And administrators need to see the diagnostic or whichever output from your code.

This is double personality logging to two targets. Console and a log file. The log file is named equal to full application path + ".log".

Console output is used while testing and debugging. Be sure to switch off console logs in release builds.

Also in here there is a "resilience in presence of multiple threads ", built in.

## 2. How to use

Through these macros

```cpp
LOG_TRACE(...) ;
LOG_DEBUG(...) ;
LOG_INFO(...)  ;
LOG_WARN(...)  ;
LOG_ERROR(...) ;
LOG_FATAL(...) ;
```

Usage syntax is exactly the same as for the `printf` family, format string and the rest.
If `DBJ_LOG_USE_COLOR` is defined output is coloured. And by default it is.

![coloured view in vs code](doc/in_vs_code.jpg)

NOTE: above is VS Code view which indeed is coloured. But colors are not the same as on Windows 10 console.


<h2>BIG FAT WARNINGS</h2>
<h3>&nbsp;</h3>
<h3>1. Do not enter escape codes `\n \v \f \t \r \b` </h3> 
<h3>&nbsp;</h3>
into your strings, you are sending to logging. If you do your output will be strange. And we will not stop you :)

Actually we could but that will slow everybody else down.
<h3>&nbsp;</h3>
<h3>2. dbj simple log is not wchar_t compatible</h3>
<h3>&nbsp;</h3>
But. That is not a problem. How? Because Microsoft extension to `printf` family formatting chars has capital `%S`. That translates strings to/from wide/narrow. Example:

```cpp
// some not S_OK HRESULT
_com_error  comerr(hr_);
// log it but as narrow string
// use '%S' not '%s'
dbj_log_fatal("IMMEDIATE EXIT !! '%S'", comerr.ErrorMessage());
```
And the opposite works too.
```cpp
const char * narrow_ = "Not wchar_t * string" ;

wprintf (L"That narrow string was: %S", narrow_ ) ;
```

<h3>&nbsp;</h3>

## 3. Alert! I have a name clash?!

<h3>&nbsp;</h3>

And we are not surprised. But don't fret. Macros in the "front" are defined like so:

```cpp
// and these macros in the front
// which are much more senisitive to name clash
// unles you do not use your set
#ifndef DBJ_USER_DEFINED_MACRO_NAMES
	#define LOG_TRACE dbj_log_trace
	#define LOG_DEBUG dbj_log_debug
	#define LOG_INFO dbj_log_info
	#define LOG_WARN dbj_log_warn
	#define LOG_ERROR dbj_log_error
	#define LOG_FATAL dbj_log_fatal
#endif // DBJ_USER_DEFINED_MACRO_NAMES
```
Obviously you can define `DBJ_USER_DEFINED_MACRO_NAMES` and provide your own macro names. Hopefully that's all you need to know to solve the name clash if it happens to your project.

<h3>&nbsp;</h3>

### 3.1. You need to start it properly
<h3>&nbsp;</h3>

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

`dbj_simple_log_startup` will start the things for you and will output the proper log starting header.

It is on the **roadmap** to offer several setup combinations to use on startup. Including log file opening options

> Advice: be very careful if not using absolute paths. One can very easily loose the log file, if using the relative paths.

<h3>&nbsp;</h3>

### 3.2. You need to end it properly
<h3>&nbsp;</h3>

This is a Windows lib. And Windows is notorious for being very reluctant to flush. Thus please make sure at application end, you call `dbj_log_finalize()`. Otherwise lof file content might not show.

**Roadmap** is to encapsulate this solution inside the library.
<h3>&nbsp;</h3>

#### 3.2.1. Autoflush

Currently inside `dbj_simple_log.h`.
```cpp
#define DBJ_SIMPLE_LOG_AUTO_FLUSH
```
Thus we have flush after each write. Safe and slow(er). 

## 4. Setup options
<h3>&nbsp;</h3>

| Setup tag  | the effect  |
|---|---|
 DBJ_LOG_MT | set the Multi Threaded protection
DBJ_LOG_TO_APP_PATH  | If app full path is given  use it to obtain log file name. Make sure you use absolute paths.
DBJ_LOG_FILE_LINE_OFF | Exclude file and line, from time stamp 
DBJ_LOG_NO_CONSOLE | No console output. Beware, if this is set and no file path is given you will have no logging. 

### 4.1. DBJ_LOG_DEFAULT_SETUP

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
<h3>&nbsp;</h3>

## 5. Building the thing
<h3>&nbsp;</h3>

This is to be built with CL.exe, which in reality means C99, but somewhat undocumented. **Roadmap** is to switch to clang 10.x and use C11 goodies?

How to use this repo in your projects? 

Provided is Visual Studio lib making project.

In your code include `dbj_simple_log.h` , Link in the lib.

The rest is history ...

-------

Based on `logc` lib by `rxi`. See the sub folder `logc`.

