PROGRAM Transition

!     NOTICE
!
!     Copyright © 1996-2010 The Board of Trustees of the University of Illinois
!     and The Regents of the University of California through Ernest Orlando Lawrence
!     Berkeley National Laboratory, pending any required approval by the
!     US Department of Energy.  All rights reserved.
!
!     Portions of the EnergyPlus software package have been developed and copyrighted
!     by other individuals, companies and institutions.  These portions have been
!     incorporated into the EnergyPlus software package under license.
!
!     NOTICE: The U.S. Government is granted for itself and others acting on its
!     behalf a paid-up, nonexclusive, irrevocable, worldwide license in this data to
!     reproduce, prepare derivative works, and perform publicly and display publicly.
!     Beginning five (5) years after permission to assert copyright is granted,
!     subject to two possible five year renewals, the U.S. Government is granted for
!     itself and others acting on its behalf a paid-up, non-exclusive, irrevocable
!     worldwide license in this data to reproduce, prepare derivative works,
!     distribute copies to the public, perform publicly and display publicly, and to
!     permit others to do so.
!
!     TRADEMARKS: EnergyPlus is a trademark of the US Department of Energy.
!


          ! PROGRAM INFORMATION:
          !       AUTHOR         Linda K. Lawrie, et al
          !       DATE WRITTEN   January 1997.....
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS PROGRAM:
          ! na

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
USE InputProcessor
USE DataGlobals
USE DataStringGlobals
USE DataVCompareGlobals
USE SetVersion

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine


          ! PROGRAM PARAMETER DEFINITIONS:
          ! Note: General Parameters for the entire EnergyPlus program are contained
          ! in "DataGlobals.f90"
    CHARACTER(len=*), PARAMETER :: EPlusiniFormat="(/,'[',A,']',/,'dir=',A)"
    character(len=*), PARAMETER :: fmta="(A)"
    CHARACTER(len=*), PARAMETER :: MultipleTransitions='MULTIPLETRANSITIONS'
    CHARACTER(len=2), PARAMETER :: crlf=CHAR(13)//CHAR(10)

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! PROGRAM LOCAL VARIABLE DECLARATIONS:
    INTEGER LFN  ! Unit Number for reads
    INTEGER, EXTERNAL :: GetNewUnitNumber
    integer ArgNum
    integer cmdargs
    CHARACTER(len=270) :: InputFileName=' '
    CHARACTER(len=270) :: ListProcessingFileName=' '
    Logical AskForInput
    INTEGER Count
    INTEGER InLfn
    LOGICAL DiffOnly
    CHARACTER(len=20) ArgValue
    CHARACTER(len=20) DiffArg
    CHARACTER(len=20) UnitsArg
    CHARACTER(len=20) BlankArg
    LOGICAL InExist
    LOGICAL EndOfFile
    INTEGER IoS
    INTEGER LenPath
    CHARACTER(len=10) :: Yesno
    CHARACTER(len=200) :: RepVarLine
    integer pos
    LOGICAL :: ArgFile=.false.
    LOGICAL :: LstFile=.false.
    CHARACTER(len=3) :: ArgFileExtension='   '
    INTEGER dotpos
    INTEGER Ios1

    CHARACTER(len=10)  :: cEnvValue=' '
    LOGICAL :: AppendAudit


    LOGICAL EPlusINI

!                            INITIALIZE VARIABLES
!
      BigNumber=HUGE(BigNumber)
      DBigNumber=HUGE(DBigNumber)

      INQUIRE(File='Energy+.ini',EXIST=EPlusINI)
      IF (EPlusINI) THEN
        LFN=GetNewUnitNumber()
        OPEN(UNIT=LFN,FILE='Energy+.ini')
                              !       Get directories from ini file
        CALL ReadINIFile(LFN,'program','dir',ProgramPath)

        CLOSE(LFN)
        LenPath=Len_TRIM(ProgramPath)
        IF (LenPath > 0) THEN
          IF (ProgramPath(LenPath:LenPath) /= PathChar) THEN
            ProgramPath=TRIM(ProgramPath)//TRIM(PathChar)
          ENDIF
        ENDIF
      ELSE
        ProgramPath='  '
        LFN=GetNewUnitNumber()
        OPEN(UNIT=LFN,File='Energy+.ini')
        WRITE(LFN,EPlusiniFormat) 'program',ProgramPath
        CLOSE(LFN)
      ENDIF

      cEnvValue=' '
      CALL Get_Environment_Variable(MultipleTransitions,cEnvValue)
      cEnvValue=MakeUPPERCase(cEnvValue)
      AppendAudit=(cEnvValue(1:1)=='Y')

      AuditF=GetNewUnitNumber()

      CALL DisplayString('Transition Starting')

      CALL SetThisVersionVariables()

      Progname='Conversion'
      PrognameConversion=VerString
      CALL DisplayString(VerString)
      IF (AppendAudit) THEN
        OPEN(Unit=Auditf,File='Transition.audit',ACCESS='APPEND')
      ELSE
        OPEN(Unit=Auditf,File='Transition.audit')
      ENDIF
      WRITE(Auditf,fmta) TRIM(VerString)
      IF (AppendAudit) THEN
        WRITE(Auditf,fmta) ' Appending to previous Transition.audit'
      ELSE
        WRITE(Auditf,fmta) ' Starting new Transition.audit'
      ENDIF

      ! Default Diff, Units, Blanks
      DiffArg='FULL'
      UnitsArg='YES'
      BlankArg='YES'


        !Call ProcessInput to produce the IDF file which is read by all of the
        ! Get input routines in the rest of the simulation
      cmdargs=Command_Argument_Count()
      if (cmdargs == 0) then
        AskForInput=.true.
        ArgFile=.false.
      else
        ArgNum=1
        Call Get_Command_Argument(ArgNum,InputFileName)
        InputFileName=ADJUSTL(InputFileName)
        AskForInput=.false.
        dotpos=SCAN(InputFileName,'.',.true.)
        IF (dotpos > 0) THEN
          IF (MakeUPPERCase(InputFileName(dotpos:)) == '.IDF' .or. MakeUPPERCase(InputFileName(dotpos:)) == '.IMF' .or.  &
              MakeUPPERCase(InputFileName(dotpos:)) == '.RVI' .or. MakeUPPERCase(InputFileName(dotpos:)) == '.MVI') THEN
            ArgFile=.true.
            ArgFileExtension=MakeLowerCase(InputFileName(dotpos+1:))
            LstFile=.false.
          ELSEIF (MakeUPPERCase(InputFileName(dotpos:)) == '.LST') THEN
            ListProcessingFileName=InputFileName
            ArgFile=.true.
            LstFile=.true.
          ELSE  ! Assume everything else is list processing
            ListProcessingFileName=InputFileName
            ArgFile=.true.
            LstFile=.true.
          ENDIF
        ELSE  ! if file name but no extension, use list processing.
          ListProcessingFileName=InputFileName
          ArgFile=.true.
          LstFile=.true.
        ENDIF
        IF (cmdargs > 1) THEN
          ArgNum=2
          CALL Get_Command_Argument(ArgNum,DiffArg)
          DiffArg=MakeUPPERCase(DiffArg)
          IF (cmdargs > 2) THEN
            ArgNum=3
            CALL Get_Command_Argument(ArgNum,UnitsArg)
            UnitsArg=MakeUPPERCase(UnitsArg)
            IF (cmdargs > 3) THEN
              ArgNum=4
              CALL Get_Command_Argument(ArgNum,BlankArg)
              BlankArg=MakeUPPERCase(BlankArg)
            ENDIF
          ENDIF
        ENDIF
      ENDIF
      CALL ProcessInput(IDDFileNameWithPath,NewIDDFileNameWithPath)

      CALL CompareOldNew

!      CALL ReportStats


      ! Check out inputs now
      IF (AskForInput) THEN
        WRITE(*,*) 'Enter "diff" for differences only, "full" for full idf outputs'
        write(*,fmta,advance='no') '-->'
        READ(*,*) InputFileName
        IF (InputFileName(1:1) == '@') THEN
          AskForInput=.false.
          InputFileName=InputFileName(2:)
          INQUIRE(File=TRIM(InputFileName),EXIST=InExist)
          IF (.not. InExist) THEN
            WRITE(*,*) 'No file=', TRIM(InputFileName)
            WRITE(Auditf,fmta) ' No file='//TRIM(InputFileName)
            STOP "Errors"
          ENDIF
          InLfn=GetNewUnitNumber()
          OPEN(InLfn,File=TRIM(InputFileName))
          READ(InLfn,*,IOSTAT=IoS) ArgValue
          IF (IoS /= 0) THEN
            EndOfFile=.true.
            ArgValue=' '
          ENDIF
        ELSE
          ArgValue=InputFileName
        ENDIF
      ELSE
        IF (.not. ArgFile) THEN
          INQUIRE(File=TRIM(InputFileName),EXIST=InExist)
          IF (.not. InExist) THEN
            WRITE(*,*) 'No file=', TRIM(InputFileName)
            WRITE(Auditf,fmta) ' No file='//TRIM(InputFileName)
            STOP "Errors"
          ENDIF
          InLfn=GetNewUnitNumber()
          OPEN(InLfn,File=TRIM(InputFileName))
          READ(InLfn,*,IOSTAT=IoS) ArgValue
          IF (IoS /= 0) THEN
            EndOfFile=.true.
            ArgValue=' '
          ENDIF
        ELSE
          ArgValue=DiffArg
        ENDIF
      ENDIF
      ArgValue=ADJUSTL(ArgValue)
      ArgValue=MakeUPPERCase(ArgValue)
      IF (ArgValue(1:1) == 'D') THEN
        DiffOnly=.true.
        CALL DisplayString('Will create new IDFs with Diff only')
        WRITE(Auditf,fmta) ' Will create new IDFs with Diff only'
      ELSE
        DiffOnly=.false.
        CALL DisplayString('Will create new full IDFs')
        WRITE(Auditf,fmta) ' Will create new full IDFs'
      ENDIF
      IF (ArgValue == Blank) EndOfFile=.true.

      IF (AskForInput) THEN
        WRITE(*,*) 'Enter "yes" for including units on output lines, "no" for no units inclusion'
        write(*,fmta,advance='no') '-->'
        READ(*,*) ArgValue
      ELSE
        IF (.not. ArgFile) THEN
          READ(InLfn,*,IOSTAT=IoS) ArgValue
          IF (IoS /= 0) THEN
            EndOfFile=.true.
            ArgValue=' '
          ENDIF
        ELSE
          ArgValue=UnitsArg
        ENDIF
      ENDIF
      ArgValue=ADJUSTL(ArgValue)
      ArgValue=MakeUPPERCase(ArgValue)
      IF (ArgValue(1:1) == 'Y') THEN
        withUnits=.true.
        CALL DisplayString('Will create new IDF lines with units where applicable')
        WRITE(Auditf,fmta) ' Will create new IDF lines with units where applicable'
      ELSE
        withUnits=.false.
        CALL DisplayString('New IDF lines will not include units')
        WRITE(Auditf,fmta) ' New IDF lines will not include units'
      ENDIF

      IF (AskForInput) THEN
        WRITE(*,*) 'Enter "yes" for preserving blanks in default fields, "no" to fill those fields with defaults'
        write(*,fmta,advance='no') '-->'
        READ(*,*) ArgValue
      ELSE
        IF (.not. ArgFile) THEN
          READ(InLfn,*,IOSTAT=IoS) ArgValue
          IF (IoS /= 0) THEN
            EndOfFile=.true.
            ArgValue=' '
          ENDIF
        ELSE
          ArgValue=BlankArg
        ENDIF
      ENDIF
      ArgValue=ADJUSTL(ArgValue)
      ArgValue=MakeUPPERCase(ArgValue)
      IF (ArgValue(1:1) == 'Y') THEN
        LeaveBlank=.true.
        CALL DisplayString('Will create new IDF lines leaving blank incoming fields as blank (no default fill)')
        WRITE(Auditf,fmta) ' Will create new IDF lines leaving blank incoming fields as blank (no default fill)'
      ELSE
        LeaveBlank=.false.
        CALL DisplayString('New IDF lines will have blank fields filled with defaults as applicable')
        WRITE(Auditf,fmta) ' New IDF lines will have blank fields filled with defaults as applicable'
      ENDIF

100   INQUIRE(File=TRIM(RepVarFileNameWithPath),EXIST=InExist)
      IF (InExist) THEN
        LFN=GetNewUnitNumber()
        OPEN(LFN,FILE=TRIM(RepVarFileNameWithPath),IOSTAT=Ios,action='READ')
        IF (Ios == 0) THEN
          READ(LFN,'(1X)')
          READ(LFN,*) NumRepVarNames
          ALLOCATE(OldRepVarName(NumRepVarNames+2))  ! To accomodate being able to spawn 3 vars from one old one.
          ALLOCATE(NewRepVarName(NumRepVarNames+2))
          ALLOCATE(NewRepVarCaution(NumRepVarNames+2))
          ALLOCATE(OutVarCaution(NumRepVarNames+2))
          ALLOCATE(MtrVarCaution(NumRepVarNames+2))
          ALLOCATE(TimeBinVarCaution(NumRepVarNames+2))
          ALLOCATE(OTMVarCaution(NumRepVarNames+2))
          ALLOCATE(CMtrVarCaution(NumRepVarNames+2))
          ALLOCATE(CMtrDVarCaution(NumRepVarNames+2))

          OldRepVarName=Blank
          NewRepVarName=Blank
          NewRepVarCaution=Blank
          OutVarCaution=.false.
          MtrVarCaution=.false.
          TimeBinVarCaution=.false.
          OTMVarCaution=.false.
          CMtrVarCaution=.false.
          CMtrDVarCaution=.false.

          DO Count=1,NumRepVarNames
            READ(LFN,'(A)') RepVarLine
            pos=INDEX(RepVarLine,',')
            OldRepVarName(Count)=RepVarLine(1:pos-1)
            NewRepVarName(Count)=RepVarLine(pos+1:)
            pos=INDEX(NewRepVarName(Count),',')
            IF (pos > 0) THEN
              NewRepVarName(Count)=NewRepVarName(Count)(1:pos-1)
              RepVarLine=RepVarLine(pos+1:)
            ENDIF
            pos=INDEX(OldRepVarName(Count),'"')
            DO WHILE (pos>0)
              OldRepVarName(Count)(pos:pos)=' '
              pos=INDEX(OldRepVarName(Count),'"')
            ENDDO
            pos=INDEX(OldRepVarName(Count),'''')
            DO WHILE (pos>0)
              OldRepVarName(Count)(pos:pos)=' '
              pos=INDEX(OldRepVarName(Count),'''')
            ENDDO
            pos=INDEX(NewRepVarName(Count),'"')
            DO WHILE (pos>0)
              NewRepVarName(Count)(pos:pos)=' '
              pos=INDEX(NewRepVarName(Count),'"')
            ENDDO
            pos=INDEX(NewRepVarName(Count),'''')
            DO WHILE (pos>0)
              NewRepVarName(Count)(pos:pos)=' '
              pos=INDEX(NewRepVarName(Count),'''')
            ENDDO
            pos=INDEX(RepVarLine,',')
            DO WHILE (pos>0)
              RepVarLine=RepVarLine(pos+1:)
              pos=INDEX(RepVarLine,',')
            ENDDO
            pos=INDEX(RepVarLine,'"')
            DO WHILE (pos>0)
              RepVarLine(pos:pos)=' '
              pos=INDEX(RepVarLine,'"')
            ENDDO
            pos=INDEX(RepVarLine,'''')
            DO WHILE (pos>0)
              RepVarLine(pos:pos)=' '
              pos=INDEX(RepVarLine,'''')
            ENDDO
            NewRepVarCaution(Count)=ADJUSTL(RepVarLine)
            OldRepVarName(Count)=ADJUSTL(OldRepVarName(Count))
            NewRepVarName(Count)=ADJUSTL(NewRepVarName(Count))
!            READ(LFN,*) OldRepVarName(Count),NewRepVarName(Count)
          ENDDO
          CLOSE(LFN)
        ELSE  ! file in use
          WRITE(*,*) ' Report Variable Name file='//TRIM(RepVarFileNameWithPath)
          WRITE(*,*) ' is not accessible.  Might be in use by another program.'
          WRITE(Auditf,fmta) ' Report Variable Name file='//TRIM(RepVarFileNameWithPath)
          WRITE(Auditf,fmta) ' is not accessible.  Might be in use by another program.'
          IF (AskForInput) THEN
            WRITE(*,*) ' Enter Y to proceed anyway, N to try again'
            READ (*,*) YesNo
            IF (YesNo == 'N' .or. YesNo == 'n') GOTO 100
          ELSE
            NumRepVarNames=0
            ALLOCATE(OldRepVarName(NumRepVarNames+2))  ! To accomodate being able to spawn 3 vars from one old one.
            ALLOCATE(NewRepVarName(NumRepVarNames+2))
            ALLOCATE(NewRepVarCaution(NumRepVarNames+2))
            ALLOCATE(OutVarCaution(NumRepVarNames+2))
            ALLOCATE(MtrVarCaution(NumRepVarNames+2))
            ALLOCATE(TimeBinVarCaution(NumRepVarNames+2))
            ALLOCATE(OTMVarCaution(NumRepVarNames+2))
            ALLOCATE(CMtrVarCaution(NumRepVarNames+2))
            ALLOCATE(CMtrDVarCaution(NumRepVarNames+2))

            OldRepVarName=Blank
            NewRepVarName=Blank
            NewRepVarCaution=Blank
            OutVarCaution=.false.
            MtrVarCaution=.false.
            TimeBinVarCaution=.false.
            OTMVarCaution=.false.
            CMtrVarCaution=.false.
            CMtrDVarCaution=.false.
          ENDIF
        ENDIF
      ELSE
        WRITE(*,*) ' Report Variable Name file='//TRIM(RepVarFileNameWithPath)
        WRITE(*,*) ' not found.'
        WRITE(Auditf,fmta) ' Report Variable Name file='//TRIM(RepVarFileNameWithPath)
        WRITE(Auditf,fmta) ' not found.'
        NumRepVarNames=0
        ALLOCATE(OldRepVarName(NumRepVarNames+2))  ! To accomodate being able to spawn 3 vars from one old one.
        ALLOCATE(NewRepVarName(NumRepVarNames+2))
        ALLOCATE(NewRepVarCaution(NumRepVarNames+2))
        ALLOCATE(OutVarCaution(NumRepVarNames+2))
        ALLOCATE(MtrVarCaution(NumRepVarNames+2))
        ALLOCATE(TimeBinVarCaution(NumRepVarNames+2))
        ALLOCATE(OTMVarCaution(NumRepVarNames+2))
        ALLOCATE(CMtrVarCaution(NumRepVarNames+2))
        ALLOCATE(CMtrDVarCaution(NumRepVarNames+2))

        OldRepVarName=Blank
        NewRepVarName=Blank
        NewRepVarCaution=Blank
        OutVarCaution=.false.
        MtrVarCaution=.false.
        TimeBinVarCaution=.false.
        OTMVarCaution=.false.
        CMtrVarCaution=.false.
        CMtrDVarCaution=.false.
      ENDIF

      WRITE(*,*) LstFile

      IF (.not. LstFile) THEN
        CALL CreateNewIDFUsingRules(EndOfFile,DiffOnly,InLfn,AskForInput,InputFileName,ArgFile,ArgFileExtension)
      ELSE
        LFN=GetNewUnitNumber()
        INQUIRE(File=TRIM(ListProcessingFileName),Exist=InExist)
        IF (InExist) THEN
          OPEN(LFN,File=TRIM(ListProcessingFileName),IOSTAT=Ios,action='READ')
          IF (Ios == 0) THEN
            Ios1=0
            WRITE(*,*) ' ListProcessing with file='//TRIM(ListProcessingFileName)
            WRITE(auditf,fmta) ' ListProcessing with file='//TRIM(ListProcessingFileName)
            DO WHILE (Ios1==0)
              READ(LFN,'(A)',IOSTAT=Ios1) InputFileName
              dotpos=SCAN(InputFileName,'.',.true.)
              ArgFileExtension=MakeLowerCase(InputFileName(dotpos+1:))
              IF (ArgFileExtension == ' ') ArgFileExtension='idf'
              IF (Ios1 == 0) THEN
                CALL CreateNewIDFUsingRules(EndOfFile,DiffOnly,InLfn,AskForInput,InputFileName,ArgFile,ArgFileExtension)
              ENDIF
            ENDDO
          ELSE  ! file in use
            WRITE(*,*) ' ListProcessing file='//TRIM(ListProcessingFileName)
            WRITE(*,*) ' is not accessible.  Might be in use by another program.'
            WRITE(Auditf,fmta) ' ListProcessing file='//TRIM(ListProcessingFileName)
            WRITE(Auditf,fmta) ' is not accessible.  Might be in use by another program.'
          ENDIF
        ELSE
          WRITE(*,*) 'No file=', TRIM(ListProcessingFileName)
          WRITE(Auditf,fmta) ' No file='//TRIM(ListProcessingFileName)
        ENDIF
      ENDIF

      CLOSE(Auditf)

      CALL EndEnergyPlus
!      STOP
!


CONTAINS

SUBROUTINE ReadINIFile(UnitNumber,Heading,KindofParameter,DataOut)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   September 1997
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This routine reads the .ini file and retrieves
          ! the path names for the files from it.

          ! METHODOLOGY EMPLOYED:
          ! Duplicate the kind of reading the Windows "GetINISetting" would
          ! do.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
USE DataStringGlobals


  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine


  ! SUBROUTINE ARGUMENT DEFINITIONS:

  INTEGER UnitNumber                 ! Unit number of the opened INI file
  CHARACTER(len=*) KindofParameter   ! Kind of parameter to be found (String)
  CHARACTER(len=*) Heading           ! Heading for the parameters ('[heading]')
  CHARACTER(len=*) DataOut           ! Output from the retrieval

  ! SUBROUTINE PARAMETER DEFINITIONS:
  INTEGER, PARAMETER :: LineLength=PathLimit+10

  ! INTERFACE BLOCK SPECIFICATIONS
  ! na

  ! DERIVED TYPE DEFINITIONS
  ! na

  ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
      CHARACTER(len=LineLength) LINE
      CHARACTER(len=20) Param
      integer IHEAD,ILB,IRB,IEQ,IPAR,IPOS,ILEN
      INTEGER ReadStat
      LOGICAL EndofFile
      LOGICAL Found
      LOGICAL NewHeading

      DataOut='           '


            ! I tried ADJUSTL(TRIM(KindofParameter)) and got an internal compiler error

      Param=TRIM(KindofParameter)
      Param=ADJUSTL(Param)
      ILEN=LEN_TRIM(Param)
      REWIND(UnitNumber)
      EndofFile=.false.
      Found=.false.
      NewHeading=.false.

 700  FORMAT(A)

      DO WHILE (.not. EndofFile .and. .not. Found)
        READ(UnitNumber,700,IOSTAT=ReadStat) LINE
        IF (ReadStat == -1) THEN
          EndofFile=.true.
          EXIT
        ENDIF

        IF (LEN_TRIM(LINE) == 0) CYCLE      ! Ignore Blank Lines

        CALL ConvertCasetoLower(LINE,LINE)    ! Turn line into lower case

        IHEAD=INDEX(LINE,Heading)
        IF (IHEAD .EQ. 0) CYCLE

!                                  See if [ and ] are on line
        ILB=INDEX(LINE,'[')
        IRB=INDEX(LINE,']')
        IF (ILB == 0 .AND. IRB == 0) CYCLE
        IF (INDEX(LINE,'['//TRIM(Heading)//']') == 0) CYCLE    ! Must be really correct heading line
        ILB=0
        IRB=0

!                                  Heading line found, now looking for Kind
        DO WHILE (.not. EndofFile .and. .not. NewHeading)
          READ(UnitNumber,700,IOSTAT=ReadStat) LINE
          IF (ReadStat == -1) THEN
            EndofFile=.true.
            EXIT
          ENDIF

          IF (LEN_TRIM(LINE) == 0) CYCLE      ! Ignore Blank Lines

          CALL ConvertCasetoLower(LINE,LINE)    ! Turn line into lower case

          ILB=INDEX(LINE,'[')
          IRB=INDEX(LINE,']')
          NewHeading=(ILB /= 0 .and. IRB /= 0)

!                                  Should be a parameter line
!                                  KindofParameter = string
          IEQ=INDEX(LINE,'=')
          IPAR=INDEX(LINE,TRIM(Param))
          IF (IEQ == 0) CYCLE
          IF (IPAR == 0) CYCLE
          IF (INDEX(LINE,TRIM(Param)//'=') == 0) CYCLE      ! needs to be param=

!                                  = found and parameter found.
          IF (IPAR > IEQ) CYCLE

!                                  parameter = found
!                                  Set output string to start with non-blank character

          DataOut=ADJUSTL(LINE(IEQ+1:))
          Found=.true.
          EXIT

        END DO

      END DO


      SELECT CASE (Param)

        CASE('dir')
          IPOS=LEN_TRIM(DataOut)
          IF (IPOS /= 0) THEN
                             ! Non-blank make sure last position is valid path character
                             !  (Set in DataStringGlobals)

            IF (DataOut(IPOS:IPOS) /= PathChar) THEN
              DataOut(IPOS+1:IPOS+1)=PathChar
            ENDIF

          ENDIF


        CASE DEFAULT
      END SELECT


RETURN


END SUBROUTINE ReadINIFile

END PROGRAM Transition
