
# ansicolor

> VT100 terminal color codes. 

## Features

|Escape sequence|Text attributes|
|---------------|----|
|\x1b[0m|All attributes off(color at startup)|
|\x1b[1m|Bold on(enable foreground intensity)|
|\x1b[4m|Underline on|
|\x1b[5m|Blink on(enable background intensity)|
|\x1b[21m|Bold off(disable foreground intensity)|
|\x1b[24m|Underline off|
|\x1b[25m|Blink off(disable background intensity)|

|Escape sequence|Foreground colors|
|---------------|----|
|\x1b[30m|Black|
|\x1b[31m|Red|
|\x1b[32m|Green|
|\x1b[33m|Yellow|
|\x1b[34m|Blue|
|\x1b[35m|Magenta|
|\x1b[36m|Cyan|
|\x1b[37m|White|
|\x1b[39m|Default(foreground color at startup)|
|\x1b[90m|Light Gray|
|\x1b[91m|Light Red|
|\x1b[92m|Light Green|
|\x1b[93m|Light Yellow|
|\x1b[94m|Light Blue|
|\x1b[95m|Light Magenta|
|\x1b[96m|Light Cyan|
|\x1b[97m|Light White|

|Escape sequence|Background colors|
|---------------|----|
|\x1b[40m|Black|
|\x1b[41m|Red|
|\x1b[42m|Green|
|\x1b[43m|Yellow|
|\x1b[44m|Blue|
|\x1b[45m|Magenta|
|\x1b[46m|Cyan|
|\x1b[47m|White|
|\x1b[49m|Default(background color at startup)|
|\x1b[100m|Light Gray|
|\x1b[101m|Light Red|
|\x1b[102m|Light Green|
|\x1b[103m|Light Yellow|
|\x1b[104m|Light Blue|
|\x1b[105m|Light Magenta|
|\x1b[106m|Light Cyan|
|\x1b[107m|Light White|

---
Example javascript
```javascript
	console.log('\x1b[91m', 'RED');
```


Source: https://godoc.org/github.com/shiena/ansicolor