@echo off
setlocal

set "ROOT=%~dp0" // retrobox

gcc -I"%ROOT%/.." "%ROOT%tetris\*.c" -o "%ROOT%builds\tetris.exe"
gcc -I"%ROOT%/.." "%ROOT%snake\*.c" -o "%ROOT%builds\snake.exe"