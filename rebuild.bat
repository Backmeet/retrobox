@echo off
setlocal

set "ROOT=%~dp0"

if not exist "%ROOT%builds" mkdir "%ROOT%builds"

if "%~1"=="" (
    call :build tetris
    call :build snake
    goto :eof
)

:args
if "%~1"=="" goto :eof

if /I "%~1"=="tetris" (
    call :build tetris
) else if /I "%~1"=="snake" (
    call :build snake
) else (
    echo Unknown project: %~1
)

shift
goto args

:build
echo Building %~1...

gcc -I"%ROOT%.." "%ROOT%%~1\*.c" -o "%ROOT%builds\%~1.exe"

if errorlevel 1 (
    echo Failed to build %~1
) else (
    echo Built %~1 successfully
)

goto :eof