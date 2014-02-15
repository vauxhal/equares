@echo off

if "%NODEJS_ROOT%"=="" (
	echo ERROR: NODEJS_ROOT environment variable is not set, don't know where node.js is.
	goto :end
)

if "%QT5_BIN%"=="" (
	echo ERROR: QT5_BIN environment variable is not set, don't know where Qt5's binaries are.
	goto :end
)

if "%MONGODB_BIN%"=="" (
	echo ERROR: MONGODB_BIN environment variable is not set, don't know where mongodb binaries are.
	goto :end
)

call "%NODEJS_ROOT%\nodevars.bat"
set PATH=%QT5_BIN%;%PATH%
set PATH=%MONGODB_BIN%;%PATH%

:end
