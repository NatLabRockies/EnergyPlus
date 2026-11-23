@echo off
setlocal
set "readvars_status=0"
set "energyplus=%~dp0energyplus.exe"
if not exist "%energyplus%" set "energyplus=%~dp0..\energyplus.exe"
if EXIST eplusout.inp goto :inp
rem produces all variables in .eso file to .csv
"%energyplus%" auxiliary readvars
set "readvars_status=%errorlevel%"
goto :done
:inp
rem reads variable specifications from input file
"%energyplus%" auxiliary readvars eplusout.inp
set "readvars_status=%errorlevel%"
:done
endlocal & exit /b %readvars_status%

