@echo off
setlocal

set "ROOT=%~dp0"

if not exist "%ROOT%builds" mkdir "%ROOT%builds"

if "%~1"=="" (
    for /d %%D in ("%ROOT%*") do (
        if /I not "%%~nxD"=="builds" (
            call :build "%%~nxD"
        )
    )
    goto :eof
)

:args
if "%~1"=="" goto :eof

if exist "%ROOT%%~1\" (
    call :build "%~1"
) else (
    echo No folder %~1 exists in parent dir
)

shift
goto args

:build
echo Building %~1...

gcc -I"%ROOT%.." "%ROOT%%~1\*.c" -o "%ROOT%builds\%~1.exe" -static

if errorlevel 1 (
    echo Failed to build %~1
) else (
    echo Built %~1 successfully
)

goto :eof