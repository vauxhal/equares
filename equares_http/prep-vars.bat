@echo off

if "%NODEJS_ROOT%"=="" (
	echo ERROR: NODEJS_ROOT environment variable is not set, don't know where node.js is.
	goto :end
)

if "%QT5_BIN%"=="" (
	echo ERROR: QT5_BIN environment variable is not set, don't know where Qt5's binaries are.
	goto :end
)

call "%NODEJS_ROOT%\nodevars.bat"
set PATH=%QT5_BIN%;%PATH%

rem TODO: Remove
rem call "C:\Program Files\nodejs\nodevars.bat"
rem set PATH=c:\oss\Qt\Qt5.2.0\5.2.0\msvc2012_64_opengl\bin;%PATH%

:end
