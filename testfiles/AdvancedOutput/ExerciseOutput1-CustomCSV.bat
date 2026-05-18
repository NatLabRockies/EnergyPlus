@echo off
set post_proc=C:\EnergyPlusV7-1-0\PostProcess\

:  Batch File for writing custom RVI results out to CSVs
:
:  The ReadVarsESO post processing program requires the following input files:
:       Eplusout.inp  -  command file (specifies input and output files)
:         This file name is user specified as standard input on the command line
:         but we will standardize on Eplusout.inp (optional)
:
:       Eplusout.eso  -  Input data (the standard output file from EnergyPlus)
:         This file name is user specified in Eplusout.inp but we will
:         standardize on Eplusout.eso (can also accept the eplusout.mtr file)
:
:  ReadVarsESO produces the following output files:
:       Eplusout.csv  -  post processing output file in CSV format
:
:  This batch requires two inputs--name of .eso file, e.g.,  exercise1.eso  and name of the RVI
:  that you wish to extract. 
:
:  The csv file is automatically created with appropriate timeperiod added to the name.
:  
:  To invoke the batch file, open a DOS (Command) window (and change directory to the directory
:  where your input file (IDF) is location) and type:
:     CustomCSV  filename rvifile
:  
:  This will create a CSV for each variable defined in the RVI with data for the various
:  time steps in the output file--timestep, hourly, daily, monthly, or runperiod: 
:      filename_rvifile_timestep.csv
:

IF EXIST eplusout.eso   DEL eplusout.eso
IF EXIST eplusout.mtr   DEL eplusout.mtr
IF EXIST eplusout.csv   DEL eplusout.csv

:  1. Copy existing EnergyPlus output file to temporary file

copy "%1.eso" eplusout.eso
copy "%1.mtr" eplusout.mtr


: duplicate these lines below for multiple RVIs

IF EXIST eplusout.inp DEL eplusout.inp
IF EXIST eplusout.csv DEL eplusout.csv

@echo .
@echo =====  Extracting Results
@echo .
IF EXIST "%2.rvi" %post_proc%ReadVarsESO.bat "%2.rvi" timestep
IF EXIST eplusout.csv MOVE eplusout.csv "%1_%2_timestep.csv"
IF EXIST "%2.rvi" %post_proc%ReadVarsESO.bat "%2.rvi" hourly
IF EXIST eplusout.csv MOVE eplusout.csv "%1_%2_hourly.csv"
IF EXIST "%2.rvi" %post_proc%ReadVarsESO.bat "%2.rvi" daily
IF EXIST eplusout.csv MOVE eplusout.csv "%1_%2_daily.csv"
IF EXIST "%2.rvi" %post_proc%ReadVarsESO.bat "%2.rvi" monthly
IF EXIST eplusout.csv MOVE eplusout.csv "%1_%2_monthly.csv"
IF EXIST "%2.rvi" %post_proc%ReadVarsESO.bat "%2.rvi" annual
IF EXIST eplusout.csv MOVE eplusout.csv "%1_%2_annual.csv"
@echo .

IF EXIST "%2.mvi" %post_proc%ReadVarsESO.bat "%2.mvi" timestep
IF EXIST eplusout.csv MOVE eplusout.csv "%1_%2_Meter_timestep.csv"
IF EXIST "%2.mvi" %post_proc%ReadVarsESO.bat "%2.mvi" hourly
IF EXIST eplusout.csv MOVE eplusout.csv "%1_%2_Meter_hourly.csv"
IF EXIST "%2.mvi" %post_proc%ReadVarsESO.bat "%2.mvi" daily
IF EXIST eplusout.csv MOVE eplusout.csv "%1_%2_Meter_daily.csv"
IF EXIST "%2.mvi" %post_proc%ReadVarsESO.bat "%2.mvi" monthly
IF EXIST eplusout.csv MOVE eplusout.csv "%1_%2_Meter_monthly.csv"
IF EXIST "%2.mvi" %post_proc%ReadVarsESO.bat "%2.mvi" annual
IF EXIST eplusout.csv MOVE eplusout.csv "%1_%2_Meter_annual.csv"
@echo .

IF EXIST eplusout.eso   DEL eplusout.eso
IF EXIST eplusout.mtr   DEL eplusout.mtr
IF EXIST eplusout.csv   DEL eplusout.csv

