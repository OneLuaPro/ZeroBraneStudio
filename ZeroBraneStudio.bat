@echo off
rem https://ss64.com/nt/syntax-args.html
lua.exe "%~dp0\src\main.lua" zbstudio -cwd %cd% %*
