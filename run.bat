@echo off
set BUILD_DIR=out\build\x64-Debug
set EXE_NAME=task.exe

cd /d %~dp0
"%cd%\%BUILD_DIR%\%EXE_NAME%" %*