@echo off


if "%1"=="debug" goto :debug
if "%1"=="release" goto :release
if "%1"=="build" goto :build
if "%1"=="run" goto :run
if "%1"=="brun" goto :brun

:inputType
set /p type=Input type(debug, release, build, run, brun, done): 

if "%type%"=="debug" goto :debug
if "%type%"=="release" goto :release
if "%type%"=="build" goto :build
if "%type%"=="run" goto :run
if "%type%"=="brun" goto :brun
if "%type%"=="done" goto :done

echo unavailable type
goto inputType

:debug
rmdir /s /q build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -G "MinGW Makefiles"
goto done

:release
rmdir /s /q build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles"
goto done

:build
cmake --build build
goto done

:run
build\nopf-core.exe %*
goto done

:brun
cmake --build build && build\nopf-core.exe %*
goto done

:done