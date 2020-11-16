# Logging tool

Logging tool is a method to log the information and/or the running status of a sub-module, and output them in the terminal of host PC through the debug UART port on the board.  It’s for debugging or issue check. Each output information has a level assigned. Only the information whose level value is higher than the system global level value can be output in the debug port. And each module in the system is assigned a bit flag to control enable/disable its output. There're total 32 modules supported by logging tool. 

## Logging level

The logging level is configurable for one information. There are 4 levels for the purposes of debugging message, general information, warning and error. The debugging message has a level value of 0, information's level is 1, warning is 2, and error has highest level value of 3.

`#define LOG_LVL_DEBUG  (0U)`
`#define LOG_LVL_INFO   (1U)`
`#define LOG_LVL_WARN   (2U)`
`#define LOG_LVL_ERROR  (3U)`

## Logging API

There are 4 functions for the ERROR, WARN, INFO, and DEBUG messages respectively. 

`log_e(mod_name, fmt, ...)` 
`log_w(mod_name, fmt, ...)` 
`log_i(mod_name, fmt, ...)` 
`log_d(mod_name, fmt, ...)`

## Using Logging tool in your code

You please use the logging APIs to replace the PRINTF or printf in your code. For example, if you only want to print a message just for general purpose of information, log_i() shall be used. If an error message is needed in case of error condition, log_e() shall be used. Select the correct API according to your expectation in the code. 

An example below shows how to use logging API.

Assuming you are coding for flash and use **HyperFlash** as your mode name. You need the following code to use logging API.

1. Find a big flag (bit 3 in this example) for your module (**HyperFlash** ) by defining a macro in the appconfig.h file.

   `#define LOGGING_HYPERFLASH_BIT	(3U)`

2. Define another macro in your code that should be visible to all of your code. Maybe .h file is good place. *Noted that the macro name is the module name (**HyperFlash**) plus **bit**.*

   `#define HyperFlash_bit	LOGGING_HYPERFLASH_BIT`

3. Use log API in your code now

   `log_e(HyperFlash, "This is an error message example %d", errovalue);`

   `log_w(HyperFlash, "This is an warn message example %d", warnvalue);`

4. Optional method to redefine the log API to simplify your coding

   `#define flash_log_d(...)  log_d(HyperFlash,  ##__VA_ARGS__)`

   `#define flash_log_i(...)  log_i(HyperFlash,  ##__VA_ARGS__)`

   `#define flash_log_w(...)  log_w(HyperFlash,  ##__VA_ARGS__)`

   `#define flash_log_e(...)  log_e(HyperFlash,  ##__VA_ARGS__)`

5. Use new macros in your code as below

   `flash_log_e("This is an error message example %d", errovalue);`

   `flash_log_w("This is an warn message example %d", warnvalue);`

## Control logging tool 

The logging output can be controlled in the shell terminal run on the host PC. There're two commands for controlling the logging tool.

#### 1. loglvl

Under shell prompt, input "**loglvl -h**" to show the brief usage of loglvl command.

**loglvl** shows the global logging level currently set in the system.

**loglvl num** sets the global logging level value as *num*. After command, only the message with level higher than *num* will be printed.

#### 2. logbit

under shell prompt, input "**logbit -h**" to show the brief usage of logbit command.

**logbit** shows the current bits status (1: enabled; 0: disabled), one bit for one module.

**logbit -g num** sets multi-bit flags, one bit for one module. **logbit -g 3** means to set bit 0 and bit 1 with other bits cleared.

**logbit -s n** sets/enables the n-th bit with others unchanged.

**logbit -c n** clears/disables the n-th bit without change for other bits.

If you want to only messages for module A to be printed, assuming the bit 25 is assigned for module A, you can run **logbit -g 0** first, then **logbit -s 25**. or run **logbit -g 0x1000000** directly.

