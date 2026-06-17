@echo off
 if Not "%~1" == "" goto :NoErr
 echo usage: %0 InputFileName (req) WeatherFileName (opt)
 echo Current Parameters:
 echo Program         : %program_path%%program_name%
 echo Input Path      : %input_path%
 echo Output Path     : %output_path%
 echo PostProcess Path: %post_proc%
 echo Weather Path    : %weather_path%
 echo Pausing         : %pausing%
 echo MaxCol          : %maxcol%
 goto :done

:NoErr
:Instructions:
:  Complete the following path and program names.
:  path names must have a following \ or errors will happen
:  does not have the capability to run input macro files (yet)
:   %program_path% contains the path to the executable as well as IDD and is
:                  the root directory
:   %program_name% contains the name of the executable (normally EnergyPlus.exe)
:   %input_path%   contains the path to the input file (passed in as first argument)
:                  if the extension is imf -- will run epmacro to process before
:                  executing energyplus
:   %output_path%  contains the path where the result files should be stored
:   %post_proc%    contains the path to the post processing program (ReadVarsESO)
:   %weather_path% contains the path to the weather files (used with optional argument 2)
:   %pausing%      contains Y if pause should occur between major portions of
:                  batch file (mostly commented out)
:   %maxcol%       contains "250" if limited to 250 columns otherwise contains
:                  "nolimit" if unlimited (used when calling readVarsESO)
 echo ===== %0 (Run EnergyPlus) %~1 %2 ===== Start =====
 set program_path=%~dp0
 set program_name=EnergyPlus.exe
 set input_path=ExampleFiles\
 set output_path=ExampleFiles\Outputs\
 : If called with full path to an existing file, clear these variables since
 : the first parameter is enough to locate the file 
 IF EXIST "%~1.idf" set input_path=
 IF EXIST "%~1.idf" set output_path=
 IF EXIST "%~1.imf" set input_path=
 IF EXIST "%~1.imf" set output_path=

 SET inEPdir=FALSE
 IF "%program_path%"=="%CD%\" SET inEPdir=TRUE
 
 set post_proc=%program_path%PostProcess\
 set weather_path=%program_path%WeatherData\
 set pausing=N
 set maxcol=250
 
:  This batch file will perform the following steps:
:
:   1.  Clean up directory by deleting old working files from prior run
:   2.  Clean up target directory
:   3.  Copy %1.idf (input) into In.idf (or %1.imf to in.imf) and EPMacro/ExpandObjects
:   4.  Copy %2 (weather) into In.epw
:       Run the Basement preprocessor program if necessary
:       Run the Slab preprocessor program if necessary
:   5.  Execute EnergyPlus
:   6.  If available Copy %1.rvi (post processor commands) into Eplusout.inp
:   7.  Execute ReadVarsESO.exe (the Post Processing Program)
:   8.  If available Copy %1.mvi (post processor commands) into test.mvi
:       or create appropriate input to get meter output from eplusout.mtr
:   9.  Execute ReadVarsESO.exe (the Post Processing Program) for meter output
:  10.  Copy Eplusout.* to %1.*
:  11.  Clean up working directory.
:
:  1. Clean up working directory
IF EXIST eplusout.inp   DEL eplusout.inp
IF EXIST eplusout.end   DEL eplusout.end
IF EXIST eplusout.eso   DEL eplusout.eso
IF EXIST eplusout.rdd   DEL eplusout.rdd
IF EXIST eplusout.mdd   DEL eplusout.mdd
IF EXIST eplusout.dbg   DEL eplusout.dbg
IF EXIST eplusout.eio   DEL eplusout.eio
IF EXIST eplusout.err   DEL eplusout.err
IF EXIST eplusout.dxf   DEL eplusout.dxf
IF EXIST eplusout.csv   DEL eplusout.csv
IF EXIST eplusout.tab   DEL eplusout.tab
IF EXIST eplusout.txt   DEL eplusout.txt
IF EXIST eplusmtr.csv   DEL eplusmtr.csv
IF EXIST eplusmtr.tab   DEL eplusmtr.tab
IF EXIST eplusmtr.txt   DEL eplusmtr.txt
IF EXIST eplusout.sln   DEL eplusout.sln
IF EXIST epluszsz.csv   DEL epluszsz.csv
IF EXIST epluszsz.tab   DEL epluszsz.tab
IF EXIST epluszsz.txt   DEL epluszsz.txt
IF EXIST epluspsz.csv   DEL epluspsz.csv
IF EXIST epluspsz.tab   DEL epluspsz.tab
IF EXIST epluspsz.txt   DEL epluspsz.txt
IF EXIST eplusspsz.csv  DEL eplusspsz.csv
IF EXIST eplusspsz.tab  DEL eplusspsz.tab
IF EXIST eplusspsz.txt  DEL eplusspsz.txt
IF EXIST eplusssz.csv   DEL eplusssz.csv
IF EXIST eplusssz.tab   DEL eplusssz.tab
IF EXIST eplusssz.txt   DEL eplusssz.txt
IF EXIST eplusout.mtr   DEL eplusout.mtr
IF EXIST eplusout.mtd   DEL eplusout.mtd
IF EXIST eplusout.bnd   DEL eplusout.bnd
IF EXIST eplusout.dbg   DEL eplusout.dbg
IF EXIST eplusout.sci   DEL eplusout.sci
IF EXIST eplusmap.csv   DEL eplusmap.csv
IF EXIST eplusmap.txt   DEL eplusmap.txt
IF EXIST eplusmap.tab   DEL eplusmap.tab
IF EXIST eplustbl.csv   DEL eplustbl.csv
IF EXIST eplustbl.txt   DEL eplustbl.txt
IF EXIST eplustbl.tab   DEL eplustbl.tab
IF EXIST eplustbl.htm   DEL eplustbl.htm
IF EXIST eplustbl.xml   DEL eplustbl.xml
IF EXIST eplusout.log   DEL eplusout.log
IF EXIST eplusout.svg   DEL eplusout.svg
IF EXIST eplusout.shd   DEL eplusout.shd
IF EXIST eplusout.wrl   DEL eplusout.wrl
IF EXIST eplusout.delightin   DEL eplusout.delightin
IF EXIST eplusout.delightout  DEL eplusout.delightout
IF EXIST eplusout.delighteldmp  DEL eplusout.delighteldmp
IF EXIST eplusout.delightdfdmp  DEL eplusout.delightdfdmp
IF EXIST eplusout.sparklog  DEL eplusout.sparklog
IF EXIST eplusscreen.csv  DEL eplusscreen.csv
IF EXIST in.imf         DEL in.imf
IF EXIST in.idf         DEL in.idf
IF EXIST out.idf        DEL out.idf
IF EXIST audit.out      DEL audit.out
IF EXIST eplusout.inp   DEL eplusout.inp
IF EXIST in.epw         DEL in.epw
IF EXIST in.stat        DEL in.stat
IF EXIST eplusout.audit DEL eplusout.audit
IF EXIST test.mvi       DEL test.mvi
IF EXIST audit.out DEL audit.out
IF EXIST expanded.idf   DEL expanded.idf
IF EXIST expandedidf.err   DEL expandedidf.err
IF EXIST readvars.audit   DEL readvars.audit
IF EXIST eplusout.sql  DEL eplusout.sql
IF EXIST sqlite.err  DEL sqlite.err
IF EXIST eplusout.edd DEL eplusout.edd
IF EXIST eplusout.dfs DEL eplusout.dfs
IF EXIST slab.int DEL slab.int
IF EXIST BasementGHTIn.idf DEL BasementGHTIn.idf
IF EXIST eplusshading.csv DEL eplusshading.csv
:if %pausing%==Y pause

:  2. Clean up target directory
IF NOT EXIST "%output_path%". MKDIR "%output_path%"

IF EXIST "%output_path%%~1.epmidf" DEL "%output_path%%~1.epmidf"
IF EXIST "%output_path%%~1.epmdet" DEL "%output_path%%~1.epmdet"
IF EXIST "%output_path%%~1.eso" DEL "%output_path%%~1.eso"
IF EXIST "%output_path%%~1.rdd" DEL "%output_path%%~1.rdd"
IF EXIST "%output_path%%~1.mdd" DEL "%output_path%%~1.mdd"
IF EXIST "%output_path%%~1.eio" DEL "%output_path%%~1.eio"
IF EXIST "%output_path%%~1.err" DEL "%output_path%%~1.err"
IF EXIST "%output_path%%~1.dxf" DEL "%output_path%%~1.dxf"
IF EXIST "%output_path%%~1.csv" DEL "%output_path%%~1.csv"
IF EXIST "%output_path%%~1.tab" DEL "%output_path%%~1.tab"
IF EXIST "%output_path%%~1.txt" DEL "%output_path%%~1.txt"
IF EXIST "%output_path%%~1Meter.csv" DEL "%output_path%%~1Meter.csv"
IF EXIST "%output_path%%~1Meter.tab" DEL "%output_path%%~1Meter.tab"
IF EXIST "%output_path%%~1Meter.txt" DEL "%output_path%%~1Meter.txt"
IF EXIST "%output_path%%~1.det" DEL "%output_path%%~1.det"
IF EXIST "%output_path%%~1.sln" DEL "%output_path%%~1.sln"
IF EXIST "%output_path%%~1.Zsz" DEL "%output_path%%~1.Zsz"
IF EXIST "%output_path%%~1Zsz.csv" DEL "%output_path%%~1Zsz.csv"
IF EXIST "%output_path%%~1Zsz.tab" DEL "%output_path%%~1Zsz.tab"
IF EXIST "%output_path%%~1Zsz.txt" DEL "%output_path%%~1Zsz.txt"
IF EXIST "%output_path%%~1.Psz" DEL "%output_path%%~1.Psz"
IF EXIST "%output_path%%~1Psz.csv" DEL "%output_path%%~1Psz.csv"
IF EXIST "%output_path%%~1Psz.tab" DEL "%output_path%%~1Psz.tab"
IF EXIST "%output_path%%~1Psz.txt" DEL "%output_path%%~1Psz.txt"
IF EXIST "%output_path%%~1.Spsz" DEL "%output_path%%~1.Spsz"
IF EXIST "%output_path%%~1Spsz.csv" DEL "%output_path%%~1Spsz.csv"
IF EXIST "%output_path%%~1Spsz.tab" DEL "%output_path%%~1Spsz.tab"
IF EXIST "%output_path%%~1Spsz.txt" DEL "%output_path%%~1Spsz.txt"
IF EXIST "%output_path%%~1.ssz" DEL "%output_path%%~1.ssz"
IF EXIST "%output_path%%~1Ssz.csv" DEL "%output_path%%~1Ssz.csv"
IF EXIST "%output_path%%~1Ssz.tab" DEL "%output_path%%~1Ssz.tab"
IF EXIST "%output_path%%~1Ssz.txt" DEL "%output_path%%~1Ssz.txt"
IF EXIST "%output_path%%~1.mtr" DEL "%output_path%%~1.mtr"
IF EXIST "%output_path%%~1.mtd" DEL "%output_path%%~1.mtd"
IF EXIST "%output_path%%~1.bnd" DEL "%output_path%%~1.bnd"
IF EXIST "%output_path%%~1.dbg" DEL "%output_path%%~1.dbg"
IF EXIST "%output_path%%~1.sci" DEL "%output_path%%~1.sci"
IF EXIST "%output_path%%~1.svg" DEL "%output_path%%~1.svg"
IF EXIST "%output_path%%~1.shd" DEL "%output_path%%~1.shd"
IF EXIST "%output_path%%~1.wrl" DEL "%output_path%%~1.wrl"
IF EXIST "%output_path%%~1Screen.csv" DEL "%output_path%%~1Screen.csv"
IF EXIST "%output_path%%~1Map.csv" DEL "%output_path%%~1Map.csv"
IF EXIST "%output_path%%~1Map.tab" DEL "%output_path%%~1Map.tab"
IF EXIST "%output_path%%~1Map.txt" DEL "%output_path%%~1Map.txt"
IF EXIST "%output_path%%~1.audit" DEL "%output_path%%~1.audit"
IF EXIST "%output_path%%~1Table.csv" DEL "%output_path%%~1Table.csv"
IF EXIST "%output_path%%~1Table.tab" DEL "%output_path%%~1Table.tab"
IF EXIST "%output_path%%~1Table.txt" DEL "%output_path%%~1Table.txt"
IF EXIST "%output_path%%~1Table.html" DEL "%output_path%%~1Table.html"
IF EXIST "%output_path%%~1Table.htm" DEL "%output_path%%~1Table.htm"
IF EXIST "%output_path%%~1Table.xml" DEL "%output_path%%~1Table.xml"
IF EXIST "%output_path%%~1DElight.in" DEL "%output_path%%~1DElight.in"
IF EXIST "%output_path%%~1DElight.out" DEL "%output_path%%~1DElight.out"
IF EXIST "%output_path%%~1DElight.dfdmp" DEL "%output_path%%~1DElight.dfdmp"
IF EXIST "%output_path%%~1DElight.eldmp" DEL "%output_path%%~1DElight.eldmp"
IF EXIST "%output_path%%~1Spark.log" DEL "%output_path%%~1Spark.log"
IF EXIST "%output_path%%~1.expidf" DEL "%output_path%%~1.expidf"
IF EXIST "%output_path%%~1.rvaudit" DEL "%output_path%%~1.rvaudit"
IF EXIST "%output_path%%~1.sql" DEL "%output_path%%~1.sql"
IF EXIST "%output_path%%~1.edd" DEL "%output_path%%~1.edd"
IF EXIST "%output_path%%~1DFS.csv" DEL "%output_path%%~1DFS.csv"
IF EXIST "%output_path%%~1*.mat" DEL "%output_path%%~1*.mat"
IF EXIST "%output_path%%~1Shading.csv" DEL "%output_path%%~1Shading.csv"
IF EXIST "%output_path%%~1_perflog.csv" MOVE "%output_path%%~1_perflog.csv" eplusout_perflog.csv

:  3. Copy input data file to working directory and run EPMacro and ExpandObjects
IF NOT EXIST "Energy+.idd" copy "%program_path%Energy+.idd" "Energy+.idd"
IF NOT EXIST "Energy+.ini" copy "%program_path%Energy+.ini" "Energy+.ini"
if exist "%input_path%%~1.imf" copy "%input_path%%~1.imf" in.imf
if exist in.imf "%program_path%EPMacro"
if exist out.idf copy out.idf "%output_path%%~1.epmidf"
if exist audit.out copy audit.out "%output_path%%~1.epmdet"
if exist audit.out erase audit.out
if exist out.idf MOVE out.idf in.idf
if not exist in.idf copy "%input_path%%~1.idf" In.idf
if exist in.idf "%program_path%ExpandObjects"
if exist expandedidf.err COPY expandedidf.err eplusout.end
if exist expanded.idf COPY expanded.idf "%output_path%%~1.expidf"
if exist expanded.idf MOVE expanded.idf in.idf
if not exist in.idf copy "%input_path%%~1.idf" In.idf

:  4. Test for weather file parameter and copy to working directory
 if "%2" == ""  goto exe
 if EXIST "%weather_path%%2.epw" copy "%weather_path%%2.epw" in.epw
 if EXIST "%weather_path%%2.stat"  copy "%weather_path%%2.stat" in.stat

:  Run the Basement preprocessor program if necessary
IF EXIST "%output_path%~1.bsmt" COPY "%output_path%%~1.bsmt" EPObjects.txt
IF EXIST BasementGHTIn.idf DEL EPObjects.txt
IF NOT EXIST BasementGHTIn.idf GOTO :skipBasement
IF EXIST eplusout.end DEL eplusout.end
IF EXIST audit.out DEL audit.out
IF EXIST basementout.audit DEL basementout.audit
IF EXIST "%output_path%%~1_bsmt.csv" ERASE "%output_path%%~1_bsmt.csv"
IF EXIST "%output_path%%~1_bsmt.audit" ERASE "%output_path%%~1_bsmt.audit"
IF EXIST "%output_path%%~1_bsmt.out" ERASE "%output_path%%~1_bsmt.out"
IF EXIST "%output_path%%~1_out_bsmt.idf" ERASE "%output_path%%~1_out_bsmt.idf"
IF EXIST "%program_path%PreProcess\GrndTempCalc\BasementGHT.idd" COPY "%program_path%PreProcess\GrndTempCalc\BasementGHT.idd" BasementGHT.idd
ECHO Begin Basement Temperature Calculation processing . . . 
%program_path%PreProcess\GrndTempCalc\Basement.exe
:if %pausing%==Y pause
IF EXIST "MonthlyResults.csv" MOVE "MonthlyResults.csv" "%output_path%%~1_bsmt.csv"
IF EXIST "RunINPUT.TXT" MOVE "RunINPUT.TXT" "%output_path%%~1_bsmt.out"
IF EXIST "RunDEBUGOUT.txt" MOVE "RunDEBUGOUT.txt" basementout.audit
IF NOT EXIST basementout.audit echo Basement Audit File > basementout.audit
IF EXIST audit.out copy basementout.audit + audit.out basementout.audit
IF EXIST "eplusout.err" copy basementout.audit + eplusout.err basementout.audit
IF EXIST basementout.audit MOVE basementout.audit "%output_path%%~1_bsmt.audit"
IF NOT EXIST eplusout.end GOTO :SkipSim
IF EXIST eplusout.end DEL eplusout.end
IF EXIST audit.out DEL audit.out
IF EXIST basementout.audit DEL basementout.audit
IF EXIST EPObjects.txt COPY EPObjects.txt "%output_path%%~1.bsmt"
IF EXIST BasementGHTIn.idf DEL BasementGHTIn.idf
IF EXIST BasementGHT.idd DEL BasementGHT.idd
:skipBasement
IF EXIST EPObjects.txt COPY in.idf+EPObjects.txt in.idf /B
IF EXIST EPObjects.txt COPY "%output_path%%~1.expidf"+EPObjects.txt "%output_path%%~1.expidf" /B
IF EXIST EPObjects.txt DEL EPObjects.txt

: Run the Slab preprocessor program if necessary
IF EXIST "%output_path%%~1.slab" COPY "%output_path%%~1.slab" SLABSurfaceTemps.TXT
IF EXIST GHTIn.idf DEL SLABSurfaceTemps.TXT
IF NOT EXIST GHTIn.idf GOTO :skipSlab
IF EXIST eplusout.end DEL eplusout.end
IF EXIST SLABINP.TXT DEL SLABINP.TXT
IF EXIST "SLABSplit Surface Temps.TXT" DEL "SLABSplit Surface Temps.TXT"
IF EXIST audit.out DEL audit.out
IF EXIST "%output_path%%~1_slab.ger" ERASE "%output_path%%~1_slab.ger"
IF EXIST "%program_path%PreProcess\GrndTempCalc\SlabGHT.idd" COPY "%program_path%PreProcess\GrndTempCalc\SlabGHT.idd" SlabGHT.idd
ECHO Begin Slab Temperature Calculation processing . . . 
%program_path%PreProcess\GrndTempCalc\Slab.exe
:if %pausing%==Y pause
IF EXIST eplusout.err MOVE eplusout.err "%output_path%%~1_slab.ger"
IF EXIST SLABINP.TXT MOVE SLABINP.TXT "%output_path%%~1_slab.out"
IF NOT EXIST eplusout.end GOTO :SkipSim
IF EXIST eplusout.end DEL eplusout.end
IF EXIST "SLABSplit Surface Temps.TXT" DEL "SLABSplit Surface Temps.TXT"
IF EXIST audit.out DEL audit.out
IF EXIST SLABSurfaceTemps.TXT COPY SLABSurfaceTemps.TXT "%output_path%%~1.slab"
IF EXIST GHTIn.idf DEL GHTIn.idf
IF EXIST SlabGHT.idd DEL SlabGHT.idd
:skipSlab
IF EXIST SLABSurfaceTemps.TXT COPY in.idf+SLABSurfaceTemps.TXT in.idf /B
IF EXIST SLABSurfaceTemps.TXT COPY "%output_path%%~1.expidf"+SLABSurfaceTemps.TXT "%output_path%%~1.expidf" /B
IF EXIST SLABSurfaceTemps.TXT DEL SLABSurfaceTemps.TXT

:  5. Execute the program
:exe
 : Display basic parameters of the run
 echo Running %program_path%%program_name%
 echo Input File  : %input_path%%~1.idf
 echo Output Files: %output_path%
 echo IDD file    : %program_path%Energy+.idd
 if NOT "%2" == "" echo Weather File: %weather_path%%2.epw
 if %pausing%==Y pause
 
 ECHO Begin EnergyPlus processing . . . 
 IF "%numProc%" == "" SET numProc=1
 IF NOT "%numProc%" == "1" SET EP_OMP_NUM_THREADS=1
 IF NOT EXIST expandedidf.err  "%program_path%%program_name%"
 if %pausing%==Y pause
 
:  6.0 If native csv output is present, then skip convertESOMTR and ReadVarsESO post-processing
IF EXIST eplusout.csv GOTO skipReadVars
IF EXIST eplusout.tab GOTO skipReadVars
IF EXIST eplusout.txt GOTO skipReadVars
IF EXIST eplusmtr.csv GOTO skipReadVars
IF EXIST eplusmtr.tab GOTO skipReadVars
IF EXIST eplusmtr.txt GOTO skipReadVars

:  6&8. Copy Post Processing Program command file(s) to working directory
 IF EXIST "%input_path%%~1.rvi" copy "%input_path%%~1.rvi" eplusout.inp
 IF EXIST "%input_path%%~1.mvi" copy "%input_path%%~1.mvi" eplusmtr.inp

:  7&9. Run Post Processing Program(s)
if %maxcol%==250     SET rvset=
if %maxcol%==nolimit SET rvset=unlimited
: readvars creates audit in append mode.  start it off
echo %date% %time% ReadVars >readvars.audit

IF EXIST eplusout.inp %post_proc%ReadVarsESO.exe eplusout.inp %rvset%
IF NOT EXIST eplusout.inp %post_proc%ReadVarsESO.exe " " %rvset%
IF EXIST eplusmtr.inp %post_proc%ReadVarsESO.exe eplusmtr.inp %rvset%
IF NOT EXIST eplusmtr.inp echo eplusout.mtr >test.mvi
IF NOT EXIST eplusmtr.inp echo eplusmtr.csv >>test.mvi
IF NOT EXIST eplusmtr.inp %post_proc%ReadVarsESO.exe test.mvi %rvset%
:skipReadVars

IF EXIST eplusout.bnd %post_proc%HVAC-Diagram.exe
:if %pausing%==Y pause

:  10. Move output files to output path
:SkipSim
 IF EXIST eplusout.eso MOVE eplusout.eso "%output_path%%~1.eso"
 IF EXIST eplusout.rdd MOVE eplusout.rdd "%output_path%%~1.rdd"
 IF EXIST eplusout.mdd MOVE eplusout.mdd "%output_path%%~1.mdd"
 IF EXIST eplusout.eio MOVE eplusout.eio "%output_path%%~1.eio"
 IF EXIST eplusout.err MOVE eplusout.err "%output_path%%~1.err"
 IF EXIST eplusout.dxf MOVE eplusout.dxf "%output_path%%~1.dxf"
 IF EXIST eplusout.csv MOVE eplusout.csv "%output_path%%~1.csv"
 IF EXIST eplusout.tab MOVE eplusout.tab "%output_path%%~1.tab"
 IF EXIST eplusout.txt MOVE eplusout.txt "%output_path%%~1.txt"
 IF EXIST eplusmtr.csv MOVE eplusmtr.csv "%output_path%%~1Meter.csv"
 IF EXIST eplusmtr.tab MOVE eplusmtr.tab "%output_path%%~1Meter.tab"
 IF EXIST eplusmtr.txt MOVE eplusmtr.txt "%output_path%%~1Meter.txt"
 IF EXIST eplusout.sln MOVE eplusout.sln "%output_path%%~1.sln"
 IF EXIST epluszsz.csv MOVE epluszsz.csv "%output_path%%~1Zsz.csv"
 IF EXIST epluszsz.tab MOVE epluszsz.tab "%output_path%%~1Zsz.tab"
 IF EXIST epluszsz.txt MOVE epluszsz.txt "%output_path%%~1Zsz.txt"
 IF EXIST epluspsz.csv MOVE epluspsz.csv "%output_path%%~1Psz.csv"
 IF EXIST epluspsz.tab MOVE epluspsz.tab "%output_path%%~1Psz.tab"
 IF EXIST epluspsz.txt MOVE epluspsz.txt "%output_path%%~1Psz.txt"
 IF EXIST eplusspsz.csv MOVE eplusspsz.csv "%output_path%%~1Spsz.csv"
 IF EXIST eplusspsz.tab MOVE eplusspsz.tab "%output_path%%~1Spsz.tab"
 IF EXIST eplusspsz.txt MOVE eplusspsz.txt "%output_path%%~1Spsz.txt"
 IF EXIST eplusssz.csv MOVE eplusssz.csv "%output_path%%~1Ssz.csv"
 IF EXIST eplusssz.tab MOVE eplusssz.tab "%output_path%%~1Ssz.tab"
 IF EXIST eplusssz.txt MOVE eplusssz.txt "%output_path%%~1Ssz.txt"
 IF EXIST eplusout.mtr MOVE eplusout.mtr "%output_path%%~1.mtr"
 IF EXIST eplusout.mtd MOVE eplusout.mtd "%output_path%%~1.mtd"
 IF EXIST eplusout.bnd MOVE eplusout.bnd "%output_path%%~1.bnd"
 IF EXIST eplusout.dbg MOVE eplusout.dbg "%output_path%%~1.dbg"
 IF EXIST eplusout.sci MOVE eplusout.sci "%output_path%%~1.sci"
 IF EXIST eplusmap.csv MOVE eplusmap.csv "%output_path%%~1Map.csv"
 IF EXIST eplusmap.tab MOVE eplusmap.tab "%output_path%%~1Map.tab"
 IF EXIST eplusmap.txt MOVE eplusmap.txt "%output_path%%~1Map.txt"
 IF EXIST eplusout.audit MOVE eplusout.audit "%output_path%%~1.audit"
 IF EXIST eplustbl.csv MOVE eplustbl.csv "%output_path%%~1Table.csv"
 IF EXIST eplustbl.tab MOVE eplustbl.tab "%output_path%%~1Table.tab"
 IF EXIST eplustbl.txt MOVE eplustbl.txt "%output_path%%~1Table.txt"
 IF EXIST eplustbl.htm MOVE eplustbl.htm "%output_path%%~1Table.html"
 IF EXIST eplustbl.xml MOVE eplustbl.xml "%output_path%%~1Table.xml"
 IF EXIST eplusout.delightin MOVE eplusout.delightin "%output_path%%~1DElight.in"
 IF EXIST eplusout.delightout  MOVE eplusout.delightout "%output_path%%~1DElight.out"
 IF EXIST eplusout.delighteldmp  MOVE eplusout.delighteldmp "%output_path%%~1DElight.eldmp"
 IF EXIST eplusout.delightdfdmp  MOVE eplusout.delightdfdmp "%output_path%%~1DElight.dfdmp"
 IF EXIST eplusout.svg MOVE eplusout.svg "%output_path%%~1.svg"
 IF EXIST eplusout.shd MOVE eplusout.shd "%output_path%%~1.shd"
 IF EXIST eplusout.wrl MOVE eplusout.wrl "%output_path%%~1.wrl"
 IF EXIST eplusscreen.csv MOVE eplusscreen.csv "%output_path%%~1Screen.csv"
 IF EXIST expandedidf.err copy expandedidf.err+eplusout.err "%output_path%%~1.err"
 IF EXIST readvars.audit MOVE readvars.audit "%output_path%%~1.rvaudit"
 IF EXIST eplusout.sql MOVE eplusout.sql "%output_path%%~1.sql"
 IF EXIST eplusout.edd MOVE eplusout.edd "%output_path%%~1.edd"
 IF EXIST eplusout.dfs MOVE eplusout.dfs "%output_path%%~1DFS.csv"
 IF EXIST eplusshading.csv MOVE eplusshading.csv "%output_path%%~1Shading.csv"
 IF EXIST eplusout_perflog.csv MOVE eplusout_perflog.csv "%output_path%%~1_perflog.csv" 

 :  if exist *.mat (
 :    set matp=%~n1
 :    for /f %%x IN ('dir /b *.mat') DO call :s_sub %%x
 :  )
 :  goto :cleanup

:cleanup
:   11.  Clean up directory.
 ECHO Removing extra files . . .
 IF EXIST eplusout.inp DEL eplusout.inp
 IF EXIST eplusmtr.inp DEL eplusmtr.inp
 IF EXIST in.idf       DEL in.idf
 IF EXIST in.imf       DEL in.imf
 IF EXIST in.epw       DEL in.epw
 IF EXIST in.stat      DEL in.stat
 IF EXIST eplusout.dbg DEL eplusout.dbg
 IF EXIST test.mvi DEL test.mvi
 IF EXIST expandedidf.err DEL expandedidf.err
 IF EXIST readvars.audit DEL readvars.audit
 IF EXIST sqlite.err  DEL sqlite.err
 IF EXIST utilSocket.log   MOVE utilSocket.log testsocket.log
 :  Delete FMUs files and folder --Added for FMI
 :  IF EXIST *.mat DEL *.mat 
 :  IF EXIST *.fmu DEL *.fmu 
 IF EXIST tmp-fmus rmdir tmp-fmus /Q/S

 IF "%inEPdir%"=="FALSE" DEL Energy+.idd
 IF "%inEPdir%"=="FALSE" DEL Energy+.ini
 goto :done

:s_sub
  move %1 %output_path%%~1%
goto :eof

 
 :done
 echo ===== %0 %~1 ===== Complete =====
