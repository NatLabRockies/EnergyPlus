@echo off
setlocal EnableExtensions DisableDelayedExpansion

rem Windows batch file version of apply_formatting.sh. Set CLANG_FORMAT to an
rem explicit executable path to override the clang-format found on PATH.

if defined CLANG_FORMAT (
  set "formatter=%CLANG_FORMAT%"
) else (
  set "formatter=clang-format"
)

"%formatter%" --version >nul 2>&1
if errorlevel 1 (
  echo ERROR: Could not run "%formatter%". 1>&2
  echo Install clang-format or set CLANG_FORMAT to its full path. 1>&2
  exit /b 1
)

pushd "%~dp0\..\.." || exit /b 1

for %%d in (src tst) do (
  call :format_directory "%%d"
  if errorlevel 1 goto :format_failed
)

popd
exit /b 0

:format_failed
popd
exit /b 1

:format_directory
echo Formatting files under "%~1"
for /r "%~1" %%f in (*.hpp *.h *.hh *.cc *.cpp *.c) do (
  call :format "%%f"
  if errorlevel 1 exit /b 1
)
exit /b 0

:format
echo Formatting "%~1"
"%formatter%" --style=file --fallback-style=none -i "%~1"
if errorlevel 1 (
  echo ERROR: clang-format failed for "%~1". 1>&2
  exit /b 1
)
exit /b 0
