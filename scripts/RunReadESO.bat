@echo off
setlocal
set "rvlib=%~dp0python_lib"
if not exist "%rvlib%\bin\ReadVarsESO.exe" set "rvlib=%~dp0..\python_lib"
set "PYTHONPATH=%rvlib%;%PYTHONPATH%"
if EXIST eplusout.inp goto :inp
rem produces all variables in .eso file to .csv
"%rvlib%\bin\ReadVarsESO.exe"
goto :done
:inp
rem reads variable specifications from input file
"%rvlib%\bin\ReadVarsESO.exe" eplusout.inp
:done
endlocal

