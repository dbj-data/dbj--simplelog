# dbj--simplelog
### simple resilient fast local log

> (c) 2019-2020 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/ 

Double personality logging. By default all the log goes to 
the console through `stderr`. And to local file named equal to full application path + ".log".
Also there is resilience in presence of threads "built in".

> Important: log file is reset on each application run. That is the current policy.

## How to use

On startup one can use the setup function `dbj_log_setup`. But we recommend to use the `dbj_simple_log_startup(const char* app_full_path)` 
immediately after main() starts.

```cpp
int main( const int argc, char * argv[] )
{
    if (EXIT_SUCCESS != dbj_simple_log_startup(argv[0]))
        return EXIT_FAILURE;

// log file is named equal to argv[0] + ".log".
    const char * lfp = current_log_file_path();

 ... the rest of the main ...

```
### How to end properly

This is a Windows lib. And Windows is notorious for being very reluctant to flush. Thus plese make sure at application end
you place this snippet, somewhere clever as you do:

```cpp
        FILE* fp_ = dbj_fhandle_log_file_ptr(NULL);
        assert(fp_);
        (void)_flushall();
         // make sure it is fclose, not close
        if (fp_) { fclose(fp_); fp_ = nullptr; }
```

## Setup options

| Setup tag  | the effect  |
|---|---|
 MT | set the Multi Threaded protection
VT100_CON  | switch on the VT100 console mode, if no coloured output 
LOG_FROM_APP_PATH  | If app full path is given  use it to obtain log gile name. Make sure you use absoulte paths.
FILE_LINE_OFF | Exclude file and line, from time stamp 
SILENT | No console output. Beware, if this is set and no file path is given you will have a completely quiet logging. 


---

Built with CL.exe, which means C99 but somewhat undocumented. Roadmap is to switch to clang 10.x and use C11 goodies.

How to use: best add it as a git submodule. Include `dbj_simple_log.h` 
Include it's only C file  `dbj_simple_log.c` in your project. Use.

-------

Based on `logc` lib by `rxi`. See the sub folder `logc`.

