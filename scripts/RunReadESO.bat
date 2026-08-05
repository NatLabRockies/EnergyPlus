set rvpath=
if EXIST eplusout.inp goto :inp
rem produces all variables in .eso file to .csv
%rvpath%ReadVarsESO.bat
goto :done
:inp
rem reads variable specifications from input file
%rvpath%ReadVarsESO.bat eplusout.inp
:done
set rvpath=

