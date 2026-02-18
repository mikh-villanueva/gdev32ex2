@echo off
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: This simple batch script helps you to quickly compile and run a single-file
:: OpenGL/GLFW program on Windows.
::
:: Happy hacking! - eric
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:: display a message if no filename was given
if [%1] equ [] (
    echo Usage:    .\test {filename of cpp program}
    echo Example:  .\test demo0.cpp
    goto :eof
)

:: append the extension to the filename if it was omitted
set TESTFILE=%1
if exist %1.cpp set TESTFILE=%1.cpp

:: construct the compile command
set TESTCMD=g++ %TESTFILE% src\glad.cpp -std=c++17 -Wall -lglfw3 -lgdi32 -Iinclude -Llib-mingw-w64 -o %~n1.exe

:: show the user what the compile command will look like
echo %TESTCMD%

:: actually compile the program && run it if compilation succeeds
%TESTCMD% && .\%~n1.exe
