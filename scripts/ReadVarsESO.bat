@echo off
set "energyplus=%~dp0energyplus.exe"
if not exist "%energyplus%" set "energyplus=%~dp0..\energyplus.exe"
"%energyplus%" auxiliary readvars %*
exit /b %errorlevel%
