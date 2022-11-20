<h1>dbj--simplelog</h1>

> (c) 2019-2022 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/ 


<h2>simple, resilient, fast, local log</h2>

<h4>Caveat Emptor</h4>

- This is not syslog client implementation. For that please visit [DBJSYSLOGLIB&trade;](https://github.com/dbj-data/dbjsysloglib)
- We develop for single OS 
  - this is Windows only
- We use only clang-cl as packaged with Visual Studio

---

**Table of Contents**

- [1. Why logging?](#1-why-logging)
- [2. How to use](#2-how-to-use)
	- [2.2. Setup](#22-setup)
- [3. BIG FAT WARNINGS](#3-big-fat-warnings)
	- [3.1. Do not enter escape codes `\n \v \f \t \r \b`](#31-do-not-enter-escape-codes-n-v-f-t-r-b)
	- [3.2. dbj simple log is not wchar_t compatible](#32-dbj-simple-log-is-not-wchar_t-compatible)
	- [3.3. Help! I have a name clash?!](#33-help-i-have-a-name-clash)
	- [3.4. Autoflush](#34-autoflush)
- [4. Building the thing](#4-building-the-thing)

## 1. Why logging?

By their makers and by standard practices C/C++ are so called "system languages". No GUI and no UI software. Ditto, C/C++ code is used to build software inside the components making a back tier or more popular "cloud side". Once more: Without UI and without human presence. 

Each data center components or just simply server side app will need to have some logging used. 

> In production there is no console. And administrators need to see the diagnostics (or whichever other output) from your code. 

2022Q4 that means some kind of (again) "cloud side" logging feature.  This is the "last resort" logging. It does not rely on anything but files and files storage being available. And as a such it should work equaly well on your local machine or inside some container. 

## 2. How to use

In your code you wish to use `dbj--simplelog`, include `dbj_simple_log.h`. And use these macros:

```cpp
LOG_TRACE(...) ;
LOG_DEBUG(...) ;
LOG_INFO(...)  ;
LOG_WARN(...)  ;
LOG_ERROR(...) ;
LOG_FATAL(...) ;
```

Usage syntax is exactly the same as for the `printf` family, format string and the rest.
```cpp
DBJ_WARN("Temperature is now %d ", current_temp() );
```

If `DBJ_LOG_USE_COLOR` is defined output is coloured, that is default. 

![coloured view in vs code](doc/in_vs_code.jpg)

NOTE: above is VS Code view which indeed is coloured. But colors are not the same as on Windows 10 console.

Do this exactly once in one compilation unit.

```cpp
#define DBJ_SIMPLELOG_IMPLEMENTATION
#include "dbj_simple_log.h"
#include "dbj_simple_log.c"
```

(Yes, yes, next release will be single header affair.)

### 2.2. Setup

This is double personality logging; to two targets, all setup depending. Console and a log file. The log file is named equal to full application path + ".log".

Console output is used while testing and debugging. Be sure to switch off console logs in release builds. And of course, as another use case, while developing WIN GUI apps.

Also in here there is a "resilience in presence of multiple threads ", built in.

Setup of `dbj--simplelog` is a compile time affair.  It has to be done, it has to be done exactly once and it is simple but flexible. 

Setup is setup (sic!) by combining the symbols defined in the following table:

| Setup tag  | the effect  | default
|---|---|---| 
 DBJ_LOG_MT | set the Multi Threaded protection | off
DBJ_LOG_TO_FILE  | If app full path is given  use it to obtain log file name | off
DBJ_LOG_FILELINE_SHOW | Include file and line | off
DBJ_LOG_NO_CONSOLE | No console output. Beware, if this is set and no file path is given you will have no logging | false

In `dbj_simple_log.h` setup is defined with the `DBJ_LOG_DEFAULT_SETUP` macro, like so:

```cpp
/// predefined setups for both release and debug builds
/// this is used inside the constructor
/// by default we log to console and to file and 
/// we lock each log call
#undef  DBJ_LOG_DEFAULT_SETUP
#define DBJ_LOG_DEFAULT_SETUP ( DBJ_LOG_TO_FILE | DBJ_LOG_MT )
```
For your logging needs (and for production), you might not like the default settings. You combine the above symbols with "oring" them together (vertical pipe '|' is 'OR'). Whatever you mix in will change the deault setting. Example good for a production use:
```cpp
// file logging, no console
// what is not mentioned here has a default value
// see the table above
#undef  DBJ_LOG_DEFAULT_SETUP
#define DBJ_LOG_DEFAULT_SETUP ( DBJ_LOG_TO_FILE | DBJ_LOG_NO_CONSOLE )
// note: log file is full app path + '.log'
```
Example usage is in `dbj_simple_log_main.c`.

> Important: log file is newly created on each application run.

That effectively erases the previous log file, if any.

Log file is full app path + `.log`. For example:

```
c:\users\noobybooby\source\repos\game\game.exe.log
```

## 3. BIG FAT WARNINGS
### 3.1. Do not enter escape codes `\n \v \f \t \r \b` 

Into your strings, you are sending to logging. If you do your output will be strange. And we will not stop you :)

```cpp
// wrong: escape codes
DBJ_INFO("One\nTwo\rThree");
// ok: no escape codes
DBJ_INFO("One Two Three");
```

### 3.2. dbj simple log is not wchar_t compatible

That is not a problem. How? Because Microsoft extension to `printf` family formatting chars has `%S`. That translates strings to/from wide/narrow chars. Example:

```cpp
// some HRESULT indicating error
_com_error  comerr(hr_);
// comerr method ErrorMessage() returns wchar_t *
// It is very simple to log it as a narrow string
// use '%S' not '%s'
dbj_log_fatal("IMMEDIATE EXIT !! '%S'", comerr.ErrorMessage());
```
### 3.3. Help! I have a name clash?!

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

```cpp
// your names example
#define DBJ_USER_DEFINED_MACRO_NAMES
#ifndef DBJ_USER_DEFINED_MACRO_NAMES
	#define MY_TRACE dbj_log_trace
	#define MY_DEBUG dbj_log_debug
	#define MY_INFO dbj_log_info
	#define MY_WARN dbj_log_warn
	#define MY_ERROR dbj_log_error
	#define MY_FATAL dbj_log_fatal
#endif // DBJ_USER_DEFINED_MACRO_NAMES
```

### 3.4. Autoflush

Currently inside `dbj_simple_log.h` there is
```cpp
#define DBJ_SIMPLE_LOG_AUTO_FLUSH
```
Thus we have flush after each write. That is safe and slow(er). That is how we like it. If really keen you can build without the auto flush defined.

## 4. Building the thing

This is to be used with projects built with clang-cl.exe. We use clang-cl as delivered with Visual Studio 2019. We are yet to see the example where cl.exe is unavoidable. Yes `/kernel` builds including.

The rest is history ...

-------

Based on `logc` lib by `rxi`. 

