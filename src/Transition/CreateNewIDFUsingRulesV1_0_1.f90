MODULE SetVersion

USE DataStringGlobals
USE DataVCompareGlobals

PUBLIC

CONTAINS

SUBROUTINE SetThisVersionVariables()
      VerString='Conversion 1.0 => 1.0.1'
      VersionNum=1.0
      IDDFileNameWithPath=TRIM(ProgramPath)//'V1-0-0-Energy+.idd'
      NewIDDFileNameWithPath=TRIM(ProgramPath)//'V1-0-1-Energy+.idd'
      RepVarFileNameWithPath=TRIM(ProgramPath)//'Report Variables 1-0-0-023 to 1-0-1.csv'
END SUBROUTINE

END MODULE

SUBROUTINE CreateNewIDFUsingRules(EndOfFile,DiffOnly,InLfn,AskForInput,InputFileName,ArgFile,ArgIDFExtension)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   April 2002
          !       MODIFIED       For each release
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine creates new IDFs based on the rules specified by
          ! developers.  This will result in a more complete transition but
          ! takes more time to create.  This routine is specifically for rules
          ! 1.0.0 to 1.0.1.

          ! METHODOLOGY EMPLOYED:
          ! Note that some rules may be applied here that would not necessarily run in the
          ! version being transitioned to.  One assumes that the final transition file version will be
          ! the current or current-1 version.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
  USE InputProcessor
  USE DataVCompareGlobals
  USE VCompareGlobalRoutines
  USE General
  USE DataGlobals, ONLY: ShowMessage, ShowContinueError, ShowFatalError, ShowSevereError, ShowWarningError

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  LOGICAL, INTENT(INOUT) :: EndOfFile
  LOGICAL, INTENT(IN)    :: DiffOnly
  INTEGER, INTENT(IN)    :: InLfn
  LOGICAL, INTENT(IN)    :: AskForInput
  CHARACTER(len=*), INTENT(IN) :: InputFileName
  LOGICAL, INTENT(IN)    :: ArgFile
  CHARACTER(len=*), INTENT(IN) :: ArgIDFExtension

          ! SUBROUTINE PARAMETER DEFINITIONS:
  CHARACTER(len=*), PARAMETER :: fmta="(A)"

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  INTEGER IoS
  INTEGER DotPos
  INTEGER Status
  INTEGER NA
  INTEGER NN
  INTEGER CurArgs
  INTEGER DifLfn
  CHARACTER(len=6) LString
  INTEGER xCount
  INTEGER Num
  INTEGER, EXTERNAL :: GetNewUnitNumber
  INTEGER, EXTERNAL :: FindNumber
  INTEGER Arg
  CHARACTER(len=30) UnitsArg
  CHARACTER(len=MaxNameLength) ObjectName
  REAL :: SaveNumber
  LOGICAL ErrFlag
  CHARACTER(len=30), EXTERNAL :: TrimTrailZeros
  REAL :: OSC2,OSC3,OSC4,OSC6,OSC7,SumOSCf
  LOGICAL ErrInOSC
  CHARACTER(len=MaxNameLength) UCRepVarName
  CHARACTER(len=MaxNameLength) UCCompRepVarName
  LOGICAL DelThis
  INTEGER pos
  LOGICAL ExitBecauseBadFile
  LOGICAL StillWorking
  CHARACTER(len=MaxNameLength), DIMENSION(6) :: TAlphas
  INTEGER NTAlphas
  CHARACTER(len=MaxNameLength), DIMENSION(5) :: TNumbers
  INTEGER NTNumbers
  INTEGER NHtCoils
  LOGICAL FField
  LOGICAL MxField
  LOGICAL Minus
  LOGICAL NoDiff
  LOGICAL :: FileExist
  LOGICAL checkrvi
  LOGICAL NoVersion
  LOGICAL DiffMinFields  ! Set to true when diff number of min-fields between the two objects
  LOGICAL Written
  LOGICAL ArgFileBeingDone
  LOGICAL LatestVersion
  CHARACTER(len=10) :: LocalFileExtension=' '
  LOGICAL :: WildMatch
  INTEGER :: Var
  INTEGER :: CurVar
  CHARACTER(len=MaxNameLength) :: CreatedOutputName
  LOGICAL, ALLOCATABLE, DIMENSION(:) :: DeleteThisRecord

  StillWorking=.true.
  ArgFileBeingDone=.false.
  LatestVersion=.false.
  LocalFileExtension=ArgIDFExtension
  EndOfFile=.false.
  IOS=0

  DO WHILE (StillWorking)

    ExitBecauseBadFile=.false.
    DO WHILE (.not. EndOfFile)
      IF (AskForInput) THEN
        WRITE(*,*) 'Enter input file name, with path'
        write(*,fmta,advance='no') '-->'
        READ(*,fmta) FullFileName
      ELSE
        IF (.not. ArgFile) THEN
          READ(InLfn,*,IOSTAT=IoS) FullFileName
        ELSEIF (.not. ArgFileBeingDone) THEN
          FullFileName=InputFileName
          IOS=0
          ArgFileBeingDone=.true.
        ELSE
          FullFileName=Blank
          IOS=1
        ENDIF
        IF (FullFileName(1:1) == '!') THEN
          FullFileName=Blank
          CYCLE
        ENDIF
      ENDIF
      UnitsArg=Blank
      IF (IoS /= 0) FullFileName=Blank
      FullFileName=ADJUSTL(FullFileName)
      IF (FullFileName /= Blank) THEN
        CALL DisplayString('Processing IDF -- '//TRIM(FullFileName))
        WRITE(Auditf,fmta) ' Processing IDF -- '//TRIM(FullFileName)
        DotPos=SCAN(FullFileName,'.',.true.) ! Scan backward looking for extension,
        IF (DotPos /= 0) THEN
          FileNamePath=FullFileName(1:DotPos-1)
          LocalFileExtension=MakeLowerCase(FullFileName(DotPos+1:))
        ELSE
          FileNamePath=FullFileName
          WRITE(*,*) ' assuming file extension of .idf'
          WRITE(Auditf,fmta) ' ..assuming file extension of .idf'
          FullFileName=TRIM(FullFileName)//'.idf'
          LocalFileExtension='idf'
        ENDIF
        ! Process the old input
        DifLfn=GetNewUnitNumber()
        INQUIRE(File=TRIM(FullFileName),EXIST=FileOK)
        IF (.not. FileOK) THEN
          WRITE(*,*) 'File not found='//TRIM(FullFileName)
          WRITE(Auditf,fmta) 'File not found='//TRIM(FullFileName)
          EndOfFile=.true.
          ExitBecauseBadFile=.true.
          EXIT
        ENDIF
        IF (LocalFileExtension == 'idf' .or. LocalFileExtension == 'imf') THEN
          checkrvi=.false.
          IF (DiffOnly) THEN
            OPEN(DifLfn,FILE=TRIM(FileNamePath)//'.'//TRIM(LocalFileExtension)//'dif')
          ELSE
            OPEN(DifLfn,FILE=TRIM(FileNamePath)//'.'//TRIM(LocalFileExtension)//'new')
          ENDIF
          IF (LocalFileExtension == 'imf') THEN
            CALL ShowWarningError('Note: IMF file being processed.  No guarantee of perfection.  Please check new file carefully.',Auditf)
            ProcessingIMFFile=.true.
          ELSE
            ProcessingIMFFile=.false.
          ENDIF
          CALL ProcessInput(IDDFileNameWithPath,NewIDDFileNameWithPath,FullFileName)
          IF (FatalError) THEN
            ExitBecauseBadFile=.true.
            EXIT
          ENDIF

          ALLOCATE(Alphas(MaxAlphaArgsFound),Numbers(MaxNumericArgsFound))
          ALLOCATE(InArgs(MaxTotalArgs))
          ALLOCATE(AorN(MaxTotalArgs),ReqFld(MaxTotalArgs),FldNames(MaxTotalArgs),FldDefaults(MaxTotalArgs),FldUnits(MaxTotalArgs))
          ALLOCATE(NwAorN(MaxTotalArgs),NwReqFld(MaxTotalArgs),NwFldNames(MaxTotalArgs),NwFldDefaults(MaxTotalArgs),NwFldUnits(MaxTotalArgs))
          ALLOCATE(OutArgs(MaxTotalArgs))
          ALLOCATE(MatchArg(MaxTotalArgs))
          ALLOCATE(DeleteThisRecord(NumIDFRecords))
          DeleteThisRecord=.false.

          NoVersion=.true.
          DO Num=1,NumIDFRecords
            IF (MakeUPPERCase(IDFRecords(Num)%Name) /= 'VERSION') CYCLE
            NoVersion=.false.
            EXIT
          ENDDO

          DO Num=1,NumIDFRecords

            DO xcount=IDFRecords(Num)%CommtS+1,IDFRecords(Num)%CommtE
              WRITE(DifLfn,fmta) TRIM(Comments(xcount))
              if (xcount == IDFRecords(Num)%CommtE) WRITE(DifLfn,fmta) ' '
            ENDDO

            IF (NoVersion .and. Num == 1) THEN
              CALL GetNewObjectDefInIDD('VERSION',NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
              OutArgs(1)='1.0.1'
              CurArgs=1
              CALL WriteOutIDFLinesAsComments(DifLfn,'VERSION',CurArgs,OutArgs,NwFldNames,NwFldUnits)
            ENDIF

            ObjectName=IDFRecords(Num)%Name
            IF (FindItemInList(ObjectName,ObjectDef%Name,NumObjectDefs) /= 0) THEN
              CALL GetObjectDefInIDD(ObjectName,NumArgs,AorN,ReqFld,ObjMinFlds,FldNames,FldDefaults,FldUnits)
              NumAlphas=IDFRecords(Num)%NumAlphas
              NumNumbers=IDFRecords(Num)%NumNumbers
              Alphas(1:NumAlphas)=IDFRecords(Num)%Alphas(1:NumAlphas)
              Numbers(1:NumNumbers)=IDFRecords(Num)%Numbers(1:NumNumbers)
              CurArgs=NumAlphas+NumNumbers
              InArgs=Blank
              OutArgs=Blank
              NA=0
              NN=0
              DO Arg=1,CurArgs
                IF (AorN(Arg)) THEN
                  NA=NA+1
                  InArgs(Arg)=Alphas(NA)
                ELSE
                  NN=NN+1
                  InArgs(Arg)=Numbers(NN)
                ENDIF
              ENDDO
            ELSE
              WRITE(Auditf,fmta) 'Object="'//TRIM(ObjectName)//'" does not seem to be on the "old" IDD.'
              WRITE(Auditf,fmta) '... will be listed as comments (no field names) on the new output file.'
              WRITE(Auditf,fmta) '... Alpha fields will be listed first, then numerics.'
              NumAlphas=IDFRecords(Num)%NumAlphas
              NumNumbers=IDFRecords(Num)%NumNumbers
              Alphas(1:NumAlphas)=IDFRecords(Num)%Alphas(1:NumAlphas)
              Numbers(1:NumNumbers)=IDFRecords(Num)%Numbers(1:NumNumbers)
              DO Arg=1,NumAlphas
                OutArgs(Arg)=Alphas(Arg)
              ENDDO
              NN=NumAlphas+1
              DO Arg=1,NumNumbers
                OutArgs(NN)=Numbers(Arg)
                NN=NN+1
              ENDDO
              CurArgs=NumAlphas+NumNumbers
              NwFldNames=Blank
              NwFldUnits=Blank
              CALL WriteOutIDFLinesAsComments(DifLfn,ObjectName,CurArgs,OutArgs,NwFldNames,NwFldUnits)
              CYCLE
            ENDIF

            Nodiff=.true.       ! Nodiff is true by default
            DiffMinFields=.false.
            Written=.false.

            IF (FindItemInList(MakeUPPERCase(ObjectName),NotInNew,SIZE(NotInNew)) == 0) THEN
              CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
              ! Check minfields
              IF (ObjMinFlds /= NwObjMinFlds) THEN
                DiffMinFields=.true.
              ELSE
                DiffMinFields=.false.
              ENDIF
            ENDIF

            IF (.not. MakingPretty) THEN

              SELECT CASE (MakeUPPERCase(TRIM(ObjectName)))

                CASE ('VERSION')
                  IF (InArgs(1)(1:3) == '1.0.1' .and. ArgFile) THEN
                    CALL ShowWarningError('File is already at latest version.  No new diff file made.',Auditf)
                    CLOSE(diflfn,STATUS='DELETE')
                    LatestVersion=.true.
                    EXIT
                  ENDIF
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1)='1.0.1'
                  nodiff=.false.

      !!!    Changes for this version

                CASE ('RUNPERIOD')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                  IF (MakeUPPERCase(OutArgs(5)) == '<BLANK>') OutArgs(5)='UseWeatherFile'
                  Nodiff=.false.

                CASE ('MATERIAL:WINDOWGAS')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  IF (MakeUPPERCase(InArgs(2)) == 'CUSTOM') THEN
                    CALL ShowSevereError('WindowGas object=Custom, arguments beyond thickness must be re-derived, Name='//TRIM(InArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) ' !!! Cannot convert WindowGas, Name='//TRIM(InArgs(1))
                    CYCLE
                  ELSE
                    CurArgs=3
                  ENDIF
                  OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                  Nodiff=.false.

                CASE ('OTHERSIDECOEFFICIENTS')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs=InArgs
                  ErrFlag=.false.
                  OSC7=ProcessNumber(OutArgs(7),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, OtherSideCoefficients field 7, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 7 {'//TRIM(NwFldNames(7))//'} value='//TRIM(OutArgs(7))
                  ELSEIF (OSC7 == 0.0) THEN
                    ErrInOSC=.false.
                    OSC2=ProcessNumber(OutArgs(8),ErrFlag)
                    IF (ErrFlag) THEN
                      CALL ShowSevereError('Invalid Number, OtherSideCoefficients field 8, Name='//TRIM(OutArgs(1)),Auditf)
                      WRITE(DifLfn,fmta) '  ! Invalid Number, field 8 {'//TRIM(NwFldNames(8))//'} value='//TRIM(OutArgs(8))
                      ErrInOSC=.true.
                    ENDIF
                    OSC3=ProcessNumber(OutArgs(5),ErrFlag)
                    IF (ErrFlag) THEN
                      CALL ShowSevereError('Invalid Number, OtherSideCoefficients field 5, Name='//TRIM(OutArgs(1)),Auditf)
                      WRITE(DifLfn,fmta) '  ! Invalid Number, field 5 {'//TRIM(NwFldNames(5))//'} value='//TRIM(OutArgs(5))
                      ErrInOSC=.true.
                    ENDIF
                    OSC4=ProcessNumber(OutArgs(4),ErrFlag)
                    IF (ErrFlag) THEN
                      CALL ShowSevereError('Invalid Number, OtherSideCoefficients field 4, Name='//TRIM(OutArgs(1)),Auditf)
                      WRITE(DifLfn,fmta) '  ! Invalid Number, field 4 {'//TRIM(NwFldNames(4))//'} value='//TRIM(OutArgs(4))
                      ErrInOSC=.true.
                    ENDIF
                    OSC6=ProcessNumber(OutArgs(6),ErrFlag)
                    IF (ErrFlag) THEN
                      CALL ShowSevereError('Invalid Number, OtherSideCoefficients field 6, Name='//TRIM(OutArgs(1)),Auditf)
                      WRITE(DifLfn,fmta) '  ! Invalid Number, field 6 {'//TRIM(NwFldNames(6))//'} value='//TRIM(OutArgs(6))
                      ErrInOSC=.true.
                    ENDIF
                    IF (.not. ErrInOSC) THEN
                      SumOSCf=OSC2+OSC3+OSC4+OSC6
                      IF (SumOSCf /= 0.0) THEN
                        OSC2=OSC2/SumOSCf
                        OutArgs(8)=TrimTrailZeros(OSC2)
                        OSC3=OSC3/SumOSCf
                        OutArgs(5)=TrimTrailZeros(OSC3)
                        OSC4=OSC4/SumOSCf
                        OutArgs(4)=TrimTrailZeros(OSC4)
                        OSC6=OSC6/SumOSCf
                        OutArgs(6)=TrimTrailZeros(OSC6)
                      ELSE
                        CALL ShowSevereError('Cannot convert OtherSideCoefficients, SUM(C2+C3+C4+C6)=0.0, Name='//TRIM(OutArgs(1)),Auditf)
                        ErrInOSC=.true.
                      ENDIF
                    ENDIF
                  ELSE  ! C7 not = 0.0
                    CALL ShowSevereError('Cannot convert OtherSideCoefficients, WindSpeed Modifier <> 0.0, Name='//TRIM(OutArgs(1)),Auditf)
                    ErrInOSC=.true.
                  ENDIF
                  IF (.not. ErrInOSC) THEN
                    WRITE(DifLfn,fmta) '  '//TRIM(ObjectName)//','
                    DO Arg=1,CurArgs
                      IF (Arg /= CurArgs) THEN
                        LString=',  !- '
                      ELSE
                        LString=';  !- '
                      ENDIF
                      IF (withUnits) UnitsArg=NwFldUnits(Arg)
                      IF (withUnits .and. UnitsArg /= Blank) THEN
                        WRITE(DifLfn,fmta) '    '//TRIM(OutArgs(Arg))//LString//TRIM(NwFldNames(Arg))//' {'//TRIM(UnitsArg)//'}'
                      ELSE
                        WRITE(DifLfn,fmta) '    '//TRIM(OutArgs(Arg))//LString//TRIM(NwFldNames(Arg))
                      ENDIF
                    ENDDO
                  ELSE
                    WRITE(DifLfn,fmta) ' ! '//TRIM(ObjectName)//','
                    DO Arg=1,CurArgs
                      IF (Arg /= CurArgs) THEN
                        LString=',  !- '
                      ELSE
                        LString=';  !- '
                      ENDIF
                      IF (withUnits) UnitsArg=NwFldUnits(Arg)
                      IF (withUnits .and. UnitsArg /= Blank) THEN
                        WRITE(DifLfn,fmta) ' !   '//TRIM(OutArgs(Arg))//LString//TRIM(NwFldNames(Arg))//' {'//TRIM(UnitsArg)//'}'
                      ELSE
                        WRITE(DifLfn,fmta) ' !   '//TRIM(OutArgs(Arg))//LString//TRIM(NwFldNames(Arg))
                      ENDIF
                    ENDDO
                  ENDIF
                  Written=.true.
                  !CYCLE

                CASE ('COMIS SIMULATION')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  CurArgs=15
                  InArgs(CurArgs)='CpSet1'
                  OutArgs=InArgs
                  Nodiff=.false.

                CASE ('COMIS CP ARRAY')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1)='CpSet1'
                  OutArgs(2)=InArgs(1)
                  OutArgs(3:CurArgs)=InArgs(3:CurArgs)
                  Nodiff=.false.

                CASE ('COMIS CP VALUES')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1)='CpSet1'
                  OutArgs(2)=InArgs(1)
                  OutArgs(3:CurArgs)=InArgs(3:CurArgs)
                  Nodiff=.false.

                CASE ('ZONE SIZING')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1)=InArgs(1)
                  OutArgs(2:5)=InArgs(4:7)
                  OutArgs(6:9)=NwFldDefaults(6:9)
                  CurArgs=9
                  Nodiff=.false.

                CASE ('SYSTEM SIZING')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1)=InArgs(1)
                  OutArgs(2:7)=InArgs(4:9)
                  OutArgs(8:12)=NwFldDefaults(8:12)
                  CurArgs=12
                  Nodiff=.false.

                CASE ('PLANT LOOP')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:6)=InArgs(1:6)
                  OutArgs(7)=InArgs(8)
                  OutArgs(8)=InArgs(9)
                  OutArgs(9)='autosize'
                  OutArgs(10:CurArgs)=InArgs(10:CurArgs)
                  IF (MakeUPPERCase(OutArgs(18)) == 'OVERLOADING') THEN
                    CALL ShowWarningError('Plant Loop - Overloading no longer allowed, changed to Sequential',Auditf)
                    WRITE(DifLfn,fmta) '  ! Overloading no longer allowed, changed to Sequential'
                    OutArgs(18)='Sequential'
                  ENDIF
                  CurArgs=18
                  Nodiff=.false.

                CASE ('CONDENSER LOOP')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:6)=InArgs(1:6)
                  OutArgs(7)=InArgs(8)
                  OutArgs(8)=InArgs(9)
                  OutArgs(9)='autosize'
                  OutArgs(10:CurArgs)=InArgs(10:CurArgs)
                  ErrFlag=.false.
                  SaveNumber=ProcessNumber(OutArgs(6),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, Condenser Loop field 6, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 6 {'//TRIM(NwFldNames(6))//'} value='//TRIM(OutArgs(6))
                  ELSE
                    IF (SaveNumber < 5.0) THEN
                      OutArgs(6)='10.0'
                    ENDIF
                  ENDIF
                  OutArgs(18)='Sequential'
                  CurArgs=18
                  Nodiff=.false.

                CASE ('FAN COIL UNIT:4 PIPE')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                  ! Multiply fields 14, 15, 18, 19 by .001
                  ErrFlag=.false.
                  SaveNumber=ProcessNumber(OutArgs(14),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, Fan Coil Unit:4 Pipe field 14, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 14 {'//TRIM(NwFldNames(14))//'} value='//TRIM(OutArgs(14))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(14)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  ErrFlag=.false.
                  SaveNumber=ProcessNumber(OutArgs(15),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, Fan Coil Unit:4 Pipe field 15, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 15 {'//TRIM(NwFldNames(15))//'} value='//TRIM(OutArgs(15))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(15)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  ErrFlag=.false.
                  SaveNumber=ProcessNumber(OutArgs(18),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, Fan Coil Unit:4 Pipe field 18, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 18 {'//TRIM(NwFldNames(18))//'} value='//TRIM(OutArgs(18))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(18)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  ErrFlag=.false.
                  SaveNumber=ProcessNumber(OutArgs(19),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, Fan Coil Unit:4 Pipe field 19, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 19 {'//TRIM(NwFldNames(19))//'} value='//TRIM(OutArgs(19))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(19)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  Nodiff=.false.

                CASE ('UNIT VENTILATOR')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:14)=InArgs(1:14)
                  OutArgs(15)=InArgs(7)
                  OutArgs(16:17)=InArgs(15:16)
                  ErrFlag=.false.
                  SaveNumber=ProcessNumber(InArgs(17),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, Unit Ventilator field 18, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 18 {'//TRIM(NwFldNames(18))//'} value='//TRIM(InArgs(17))
                    OutArgs(18)=InArgs(17)
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(18)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  SaveNumber=ProcessNumber(InArgs(18),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, Unit Ventilator field 19, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 19 {'//TRIM(NwFldNames(19))//'} value='//TRIM(InArgs(18))
                    OutArgs(19)=InArgs(18)
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(19)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  OutArgs(20:23)=InArgs(19:22)
                  SaveNumber=ProcessNumber(InArgs(23),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, Unit Ventilator field 24, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 24 {'//TRIM(NwFldNames(24))//'} value='//TRIM(InArgs(23))
                    OutArgs(24)=InArgs(23)
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(24)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  SaveNumber=ProcessNumber(InArgs(24),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, Unit Ventilator field 25, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 25 {'//TRIM(NwFldNames(25))//'} value='//TRIM(InArgs(24))
                    OutArgs(25)=InArgs(24)
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(25)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  OutArgs(26:27)=InArgs(25:26)
                  Nodiff=.false.

                CASE ('UNIT HEATER')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                  ErrFlag=.false.
                  SaveNumber=ProcessNumber(OutArgs(11),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, Unit Heater field 11, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 11 {'//TRIM(NwFldNames(11))//'} value='//TRIM(OutArgs(11))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(11)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  ErrFlag=.false.
                  SaveNumber=ProcessNumber(OutArgs(12),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, Unit Heater field 12, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 12 {'//TRIM(NwFldNames(12))//'} value='//TRIM(OutArgs(12))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(12)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  Nodiff=.false.

                CASE ('DXSYSTEM:AIRLOOP')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                  nodiff=.false.
                  IF (samestring(InArgs(6),'COIL:DX:BF-Empirical')) THEN
                    OutArgs(6)='COIL:DX:CoolingBypassFactorEmpirical'
                  ENDIF

                CASE ('FURNACE:BLOWTHRU:HEATCOOL')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:15)=InArgs(1:15)
                  NHTCoils=GetNumObjectsFound(OutArgs(14))
                  OutArgs(16)=Blank
                  DO Arg=1,NHTCoils
                    CALL GetObjectItem(OutArgs(14),Arg,TAlphas,NTAlphas,TNumbers,NTNumbers,Status)
                    IF (MakeUPPERCase(TAlphas(1)) /= MakeUPPERCase(OutArgs(15))) CYCLE
                    OutArgs(16)=TAlphas(3)
                    EXIT
                  ENDDO
                  IF (OutArgs(16) == Blank) THEN
                    CALL ShowSevereError('User must enter field for Furnace:Blowthru:HeatCool field 16, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) ' ! Need field 16 {'//TRIM(NwFldNames(16))//'} below'
                    OutArgs(16)='Need Entry Here'
                  ENDIF
                  IF (MakeUPPERCase(InArgs(16)) == 'COIL:DX:BF-EMPIRICAL')  InArgs(16)='Coil:DX:CoolingBypassFactorEmpirical'
                  OutArgs(17:19)=InArgs(16:18)
                  OutArgs(20:24)=NwFldDefaults(20:24)
                  CurArgs=24
                  Nodiff=.false.

                CASE ('UNITARYSYSTEM:BLOWTHRU:HEATCOOL')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:15)=InArgs(1:15)
                  NHTCoils=GetNumObjectsFound(OutArgs(14))
                  OutArgs(16)=Blank
                  DO Arg=1,NHTCoils
                    CALL GetObjectItem(OutArgs(14),Arg,TAlphas,NTAlphas,TNumbers,NTNumbers,Status)
                    IF (MakeUPPERCase(TAlphas(1)) /= MakeUPPERCase(OutArgs(15))) CYCLE
                    OutArgs(16)=TAlphas(3)
                    EXIT
                  ENDDO
                  IF (OutArgs(16) == Blank) THEN
                    CALL ShowSevereError('User must enter field for UnitarySystem:Blowthru:HeatCool field 16, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) ' ! Need field 16 {'//TRIM(NwFldNames(16))//'} below'
                    OutArgs(16)='Need Entry Here'
                  ENDIF
                  IF (MakeUPPERCase(InArgs(16)) == 'COIL:DX:BF-EMPIRICAL')  InArgs(16)='Coil:DX:CoolingBypassFactorEmpirical'
                  OutArgs(17:19)=InArgs(16:18)
                  OutArgs(20:24)=NwFldDefaults(20:24)
                  CurArgs=24
                  Nodiff=.false.

                CASE ('SINGLE DUCT:CONST VOLUME:REHEAT')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                  ErrFlag=.false.
                  SaveNumber=ProcessNumber(OutArgs(9),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, SINGLE DUCT:CONST VOLUME:REHEAT field 9, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 9 {'//TRIM(NwFldNames(9))//'} value='//TRIM(OutArgs(9))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(9)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  ErrFlag=.false.
                  SaveNumber=ProcessNumber(OutArgs(10),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, SINGLE DUCT:CONST VOLUME:REHEAT field 10, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 10 {'//TRIM(NwFldNames(10))//'} value='//TRIM(OutArgs(10))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(10)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  SaveNumber=ProcessNumber(OutArgs(11),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, SINGLE DUCT:CONST VOLUME:REHEAT field 11, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 11 {'//TRIM(NwFldNames(11))//'} value='//TRIM(OutArgs(11))
                  ELSE
                    IF (SaveNumber <= 0.0) SaveNumber=.001
                    OutArgs(11)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  Nodiff=.false.

                CASE ('SINGLE DUCT:VAV:REHEAT')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                  ErrFlag=.false.
                  SaveNumber=ProcessNumber(OutArgs(10),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, SINGLE DUCT:VAV:REHEAT field 10, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 10 {'//TRIM(NwFldNames(10))//'} value='//TRIM(OutArgs(10))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(10)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  ErrFlag=.false.
                  SaveNumber=ProcessNumber(OutArgs(11),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, SINGLE DUCT:VAV:REHEAT field 11, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 11 {'//TRIM(NwFldNames(11))//'} value='//TRIM(OutArgs(11))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(11)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  SaveNumber=ProcessNumber(OutArgs(13),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, SINGLE DUCT:VAV:REHEAT field 13, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 13 {'//TRIM(NwFldNames(13))//'} value='//TRIM(OutArgs(13))
                  ELSE
                    IF (SaveNumber <= 0.0) SaveNumber=.001
                    OutArgs(13)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  OutArgs(14)=NwFldDefaults(14)
                  CurArgs=14
                  Nodiff=.false.

                CASE ('SINGLE DUCT:SERIES PIU:REHEAT')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                  ErrFlag=.false.
                  SaveNumber=ProcessNumber(OutArgs(14),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, SINGLE DUCT:SERIES PIU:REHEAT field 14, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 14 {'//TRIM(NwFldNames(14))//'} value='//TRIM(OutArgs(14))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(14)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  ErrFlag=.false.
                  SaveNumber=ProcessNumber(OutArgs(15),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, SINGLE DUCT:SERIES PIU:REHEAT field 15, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 15 {'//TRIM(NwFldNames(15))//'} value='//TRIM(OutArgs(15))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(15)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  Nodiff=.false.

                CASE ('SINGLE DUCT:PARALLEL PIU:REHEAT')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                  ErrFlag=.false.
                  SaveNumber=ProcessNumber(OutArgs(15),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, SINGLE DUCT:PARALLEL PIU:REHEAT field 15, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 15 {'//TRIM(NwFldNames(15))//'} value='//TRIM(OutArgs(15))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(15)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  ErrFlag=.false.
                  SaveNumber=ProcessNumber(OutArgs(16),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, SINGLE DUCT:PARALLEL PIU:REHEAT field 16, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 16 {'//TRIM(NwFldNames(16))//'} value='//TRIM(OutArgs(16))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(16)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  Nodiff=.false.

                CASE ('BASEBOARD HEATER:WATER:CONVECTIVE')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                  ErrFlag=.false.
                  SaveNumber=ProcessNumber(OutArgs(6),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, BASEBOARD HEATER:WATER:CONVECTIVE field 6, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 6 {'//TRIM(NwFldNames(6))//'} value='//TRIM(OutArgs(6))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(6)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  Nodiff=.false.

                CASE ('LOW TEMP RADIANT SYSTEM:HYDRONIC')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                  ErrFlag=.false.
                  SaveNumber=ProcessNumber(OutArgs(8),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, LOW TEMP RADIANT SYSTEM:HYDRONIC field 8, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 8 {'//TRIM(NwFldNames(8))//'} value='//TRIM(OutArgs(8))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(8)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  ErrFlag=.false.
                  SaveNumber=ProcessNumber(OutArgs(13),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, LOW TEMP RADIANT SYSTEM:HYDRONIC field 13, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 13 {'//TRIM(NwFldNames(13))//'} value='//TRIM(OutArgs(13))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(13)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  Nodiff=.false.

                CASE ('BOILER:SIMPLE')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                  ErrFlag=.false.
                  SaveNumber=ProcessNumber(OutArgs(6),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, BOILER:SIMPLE field 6, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 6 {'//TRIM(NwFldNames(6))//'} value='//TRIM(OutArgs(6))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(6)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  Nodiff=.false.

                CASE ('WATERHEATER:SIMPLE')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                  ErrFlag=.false.
                  SaveNumber=ProcessNumber(OutArgs(4),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, WATERHEATER:SIMPLE field 4, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 4 {'//TRIM(NwFldNames(4))//'} value='//TRIM(OutArgs(4))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(4)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  ErrFlag=.false.
                  SaveNumber=ProcessNumber(OutArgs(8),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, WATERHEATER:SIMPLE field 8, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 8 {'//TRIM(NwFldNames(8))//'} below='//TRIM(OutArgs(8))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(8)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  Nodiff=.false.

                CASE ('CHILLER:ELECTRIC')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:14)=InArgs(1:14)
                  OutArgs(15)=InArgs(15)
                  OutArgs(16)=InArgs(15)
                  OutArgs(17:CurArgs+1)=InArgs(16:CurArgs)
                  OutArgs(27)=NwFldDefaults(27)
                  IF (CurArgs < 27) CurArgs=27
                  Nodiff=.false.

                CASE ('CHILLER:BLAST')
                  CALL GetNewObjectDefInIDD('CHILLER:ELECTRIC',NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  ObjectName='CHILLER:ELECTRIC'
                  OutArgs(1)=InArgs(1)
                  OutArgs(2)='Water Cooled'
                  OutArgs(3:CurArgs+1)=InArgs(2:CurArgs)
                  OutArgs(27)=NwFldDefaults(27)
                  CurArgs=27
                  Nodiff=.false.

                CASE ('BRANCH')
                  ! Comp type must be checked for "chiller:blast"
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                  nodiff=.true.
                  DO Arg=3,CurArgs,5
                    IF (MakeUPPERCase(OutArgs(Arg)) /= 'CHILLER:BLAST') CYCLE
                    OutArgs(Arg)='CHILLER:ELECTRIC'
                    nodiff=.false.
                  ENDDO
                  Nodiff=.false.

                CASE ('LOAD RANGE EQUIPMENT LIST')
                  ! Plant eq type must be checked for "chiller:blast"
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                  nodiff=.true.
                  DO Arg=2,CurArgs,2
                    IF (MakeUPPERCase(OutArgs(Arg)) /= 'CHILLER:BLAST') CYCLE
                    OutArgs(Arg)='CHILLER:ELECTRIC'
                    nodiff=.false.
                  ENDDO
                  Nodiff=.false.

                CASE ('CHILLER:GAS TURBINE')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:14)=InArgs(1:14)
                  OutArgs(15)=InArgs(15)
                  OutArgs(16)=InArgs(15)
                  OutArgs(17:47)=InArgs(16:46)
                  ErrFlag=.false.
                  SaveNumber=ProcessNumber(InArgs(47),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, CHILLER:GAS TURBINE field 48, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 48 {'//TRIM(NwFldNames(48))//'} below='//TRIM(InArgs(47))
                    OutArgs(48)=InArgs(47)
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(48)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  OutArgs(49)=InArgs(48)
                  ErrFlag=.false.
                  SaveNumber=ProcessNumber(InArgs(49),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, CHILLER:GAS TURBINE field 50, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 50 {'//TRIM(NwFldNames(50))//'} below='//TRIM(InArgs(49))
                    OutArgs(50)=InArgs(49)
                  ELSE
                    SaveNumber=SaveNumber*3600.
                    OutArgs(50)=TrimTrailZeros(SaveNumber)
                    CALL ShowWarningError('Re-check conversion for CHILLER:GAS TURBINE field 50, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) ' Re-check conversion for field 50 {'//TRIM(NwFldNames(50))//'} below='//TRIM(OutArgs(50))
                  ENDIF
                  OutArgs(51)=NwFldDefaults(51)
                  OutArgs(52:CurArgs+2)=InArgs(50:CurArgs)
                  Nodiff=.false.

                CASE ('CHILLER:ABSORPTION')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:11)=InArgs(1:11)
                  OutArgs(12)=InArgs(13)
                  OutArgs(13)=InArgs(13)
                  OutArgs(14:CurArgs)=InArgs(14:CurArgs)
                  OutArgs(23)=NwFldDefaults(23)
                  IF (CurArgs < 23) CurArgs=23
                  Nodiff=.false.

                CASE ('CHILLER:GAS ABSORPTION')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:18)=InArgs(1:18)
                  CALL ShowWarningError('New field 19 {'//TRIM(NwFldNames(19))//  &
                                        '} (currently unused) for CHILLER:GAS ABSORPTION, Name='//TRIM(OutArgs(1)),Auditf)
                  WRITE(DifLfn,fmta) ' ! New field 19 {'//TRIM(NwFldNames(19))//'} below, leaving blank as is currently unused'
                  CALL ShowWarningError('New field 21 {'//TRIM(NwFldNames(21))//  &
                                        '} (currently unused) for CHILLER:GAS ABSORPTION, Name='//TRIM(OutArgs(1)),Auditf)
                  WRITE(DifLfn,fmta) ' ! New field 21 {'//TRIM(NwFldNames(21))//'} below, leaving blank as is currently unused'
                  OutArgs(19)=Blank
                  OutArgs(20)=InArgs(19)
                  OutArgs(21)=Blank
                  OutArgs(22:CurArgs+2)=InArgs(20:CurArgs)
                  OutArgs(33)=NwFldDefaults(33)
                  IF (CurArgs < 33) CurArgs=33
                  Nodiff=.false.

                CASE ('CHILLER:CONST COP')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:3)=InArgs(1:3)
                  OutArgs(4)=InArgs(4)
                  OutArgs(5)=InArgs(4)
                  OutArgs(6:9)=InArgs(5:8)
                  OutArgs(10)='Water Cooled'
                  OutArgs(11)=NwFldDefaults(11)
                  IF (CurArgs < 11) CurArgs=11
                  Nodiff=.false.

                CASE ('CHILLER:ENGINEDRIVEN')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:14)=InArgs(1:14)
                  OutArgs(15)=InArgs(15)
                  OutArgs(16)=InArgs(15)
                  OutArgs(17:CurArgs+1)=InArgs(16:CurArgs)
                  IF (OutArgs(40) == Blank) THEN
                    OutArgs(40)=NwFldDefaults(40)
                    IF (CurArgs < 40) THEN
                      CurArgs=40
                    ENDIF
                  ENDIF
                  Nodiff=.false.

                CASE ('PUMP:VARIABLE SPEED')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                  OutArgs(13)=NwFldDefaults(13)
                  OutArgs(14)=NwFldDefaults(14)
                  CurArgs=14
                  Nodiff=.false.

                CASE ('COIL:WATER:SIMPLECOOLING')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                  SaveNumber=ProcessNumber(OutArgs(4),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, COIL:WATER:SIMPLECOOLING field 4, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 4 {'//TRIM(NwFldNames(4))//'} below='//TRIM(OutArgs(4))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(4)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  Nodiff=.false.

                CASE ('COIL:WATER:SIMPLEHEATING')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                  SaveNumber=ProcessNumber(OutArgs(4),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, COIL:WATER:SIMPLEHEATING field 4, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 4 {'//TRIM(NwFldNames(4))//'} below='//TRIM(OutArgs(4))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(4)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  Nodiff=.false.

                CASE ('COIL:WATER:DETAILEDFLATCOOLING')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                  SaveNumber=ProcessNumber(OutArgs(3),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, COIL:WATER:DETAILEDFLATCOOLING field 3, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 3 {'//TRIM(NwFldNames(3))//'} below='//TRIM(OutArgs(3))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(3)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  Nodiff=.false.

                CASE ('FAN:SIMPLE:ONOFF')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:9)=InArgs(1:9)
                  CurArgs=9
                  Nodiff=.false.

                CASE ('HUMIDIFIER:STEAM:ELECTRICAL')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                  SaveNumber=ProcessNumber(OutArgs(3),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, HUMIDIFIER:STEAM:ELECTRICAL field 3, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 3 {'//TRIM(NwFldNames(3))//'} below='//TRIM(OutArgs(3))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(3)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  Nodiff=.false.

                CASE ('CONTROLLER:SIMPLE')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:7)=InArgs(1:7)
                  SaveNumber=ProcessNumber(InArgs(9),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, CONTROLLER:SIMPLE field 8, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 8 {'//TRIM(NwFldNames(8))//'} below='//TRIM(InArgs(9))
                    OutArgs(8)=InArgs(9)
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(8)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  SaveNumber=ProcessNumber(InArgs(10),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, CONTROLLER:SIMPLE field 9, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 9 {'//TRIM(NwFldNames(9))//'} below='//TRIM(InArgs(10))
                    OutArgs(9)=InArgs(10)
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(9)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  CurArgs=CurArgs-1
                  Nodiff=.false.

                CASE ('GENERATOR:DIESEL')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                  SaveNumber=ProcessNumber(OutArgs(14),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, GENERATOR:DIESEL field 14, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 14 {'//TRIM(NwFldNames(14))//'} below='//TRIM(OutArgs(14))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(14)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  SaveNumber=ProcessNumber(OutArgs(17),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, GENERATOR:DIESEL field 17, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 17 {'//TRIM(NwFldNames(17))//'} below='//TRIM(OutArgs(17))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(17)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  Nodiff=.false.

                CASE ('GENERATOR:GAS TURBINE')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                  SaveNumber=ProcessNumber(OutArgs(15),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, GENERATOR:DIESEL field 15, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 15 {'//TRIM(NwFldNames(15))//'} below='//TRIM(OutArgs(15))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(15)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  SaveNumber=ProcessNumber(OutArgs(19),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, GENERATOR:DIESEL field 19, Name='//TRIM(OutArgs(1)),Auditf)
                    WRITE(DifLfn,fmta) '  ! Invalid Number, field 19 {'//TRIM(NwFldNames(19))//'} below='//TRIM(OutArgs(19))
                  ELSE
                    SaveNumber=SaveNumber*.001
                    OutArgs(19)=TrimTrailZeros(SaveNumber)
                  ENDIF
                  Nodiff=.false.

                !  Obsolete objects.  Need new names
                CASE ('DAYLIGHTING')
                  IF (CurArgs > 5) THEN  ! Detailed Daylighting
                    CALL GetNewObjectDefInIDD('DAYLIGHTING:DETAILED',NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                    ObjectName='Daylighting:Detailed'
                    OutArgs(1)=InArgs(1)
                    OutArgs(2:CurArgs-3)=InArgs(5:CurArgs)
                    CurArgs=CurArgs-3
                  ELSE
                    CALL GetNewObjectDefInIDD('DAYLIGHTING:SIMPLE',NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                    ObjectName='Daylighting:Simple'
                    OutArgs(1:4)=InArgs(1:4)
                    CurArgs=4
                  ENDIF
                  Nodiff=.false.

                CASE ('LOAD RANGE BASED OPERATION')
                  CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  OutArgs=InArgs
                  FField=.true.
                  MxField=.false.
                  Minus=.false.
                  DO Arg=2,CurArgs,3
                    IF (FField) THEN
                      FField=.false.
                    ELSE
                      Pos=INDEX(OutArgs(Arg),'-')
                      IF (Pos > 0) THEN
                        Minus=.true.
                      ELSEIF (Minus) THEN
                        MxField=.true.
                      ENDIF
                    ENDIF
                    Pos=INDEX(OutArgs(Arg+1),'-')
                    IF (Pos > 0) THEN
                      Minus=.true.
                    ELSEIF (Minus) THEN
                      MxField=.true.
                    ENDIF
                  ENDDO
                  IF (MxField) THEN  ! Mixed is an error, this will be caught with V1.0.1
                    WRITE(DifLfn,fmta) ' ! Next object is obsolete, needs hand transition to new'
                  ELSEIF (.not. Minus) THEN
                    ObjectName='Heating Load Range Based Operation'
                  ELSE
                    ObjectName='Cooling Load Range Based Operation'
                    DO Arg=2,CurArgs,3
                      Pos=INDEX(OutArgs(Arg),'-')
                      IF (Pos > 0) OutArgs(Arg)(Pos:Pos)=' '
                      Pos=INDEX(OutArgs(Arg+1),'-')
                      IF (Pos > 0) OutArgs(Arg+1)(Pos:Pos)=' '
                    ENDDO
                  ENDIF
                  Nodiff=.false.

                CASE ('COIL:DX:BF-EMPIRICAL')
                  CALL GetNewObjectDefInIDD('COIL:DX:COOLINGBYPASSFACTOREMPIRICAL',NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                  ObjectName='COIL:DX:CoolingBypassFactorEmpirical'
                  OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                  Nodiff=.false.

              CASE('WINDOWSHADINGCONTROL')
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                nodiff=.false.
                OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                if (samestring('InteriorNonInsulatingShade',InArgs(2))) then
                  OutArgs(2)='InteriorShade'
                endif
                if (samestring('ExteriorNonInsulatingShade',InArgs(2))) then
                  OutArgs(2)='ExteriorShade'
                endif
                if (samestring('InteriorInsulatingShade',InArgs(2))) then
                  OutArgs(2)='InteriorShade'
                endif
                if (samestring('ExteriorInsulatingShade',InArgs(2))) then
                  OutArgs(2)='ExteriorShade'
                endif
                if (samestring('Schedule',InArgs(4))) then
                  OutArgs(4)='OnIfScheduleAllows'
                endif
                if (samestring('SolarOnWindow',InArgs(4))) then
                  OutArgs(4)='OnIfHighSolarOnWindow'
                endif
                if (samestring('HorizontalSolar',InArgs(4))) then
                  OutArgs(4)='OnIfHighHorizontalSolar'
                endif
                if (samestring('OutsideAirTemp',InArgs(4))) then
                  OutArgs(4)='OnIfHighOutsideAirTemp'
                endif
                if (samestring('ZoneAirTemp',InArgs(4))) then
                  OutArgs(4)='OnIfHighZoneAirTemp'
                endif
                if (samestring('ZoneCooling',InArgs(4))) then
                  OutArgs(4)='OnIfHighZoneCooling'
                endif
                if (samestring('Glare',InArgs(4))) then
                  OutArgs(4)='OnIfHighGlare'
                endif
                if (samestring('DaylightIlluminance',InArgs(4))) then
                  OutArgs(4)='MeetDaylightIlluminanceSetpoint'
                endif

              CASE ('CURVE:BIQUADRATIC')
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                nodiff=.false.
                OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                CurArgs=11
                IF (OutArgs(8) == Blank) OutArgs(8)='-99999'
                IF (OutArgs(9) == Blank) OutArgs(9)='99999'
                IF (OutArgs(10) == Blank) OutArgs(10)='-99999'
                IF (OutArgs(11) == Blank) OutArgs(11)='99999'

              CASE ('CURVE:CUBIC')
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                nodiff=.false.
                OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                CurArgs=7
                IF (OutArgs(6) == Blank) OutArgs(6)='-99999'
                IF (OutArgs(7) == Blank) OutArgs(7)='99999'

              CASE ('CURVE:QUADRATIC')
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                nodiff=.false.
                OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                CurArgs=6
                IF (OutArgs(5) == Blank) OutArgs(5)='-99999'
                IF (OutArgs(6) == Blank) OutArgs(6)='99999'

  !!!   Changes for report variables, meters, tables -- update new names

              CASE ('REPORT VARIABLE')
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                nodiff=.true.
                IF (OutArgs(1) == Blank) THEN
                  OutArgs(1)='*'
                  nodiff=.false.
                ENDIF
                CALL ScanOutputVariablesForReplacement(  &
                   2,  &
                   DelThis,  &
                   checkrvi,  &
                   nodiff,  &
                   ObjectName,  &
                   DifLfn,      &
                   .true.,  & !OutVar
                   .false., & !MtrVar
                   .false., & !TimeBinVar
                   CurArgs, &
                   Written, &
                   .false.)
                IF (DelThis) CYCLE

              CASE ('REPORT METER','REPORT METERFILEONLY','REPORT CUMULATIVE METER','REPORT CUMULATIVE METERFILEONLY')
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                nodiff=.true.
                CALL ScanOutputVariablesForReplacement(  &
                   1,  &
                   DelThis,  &
                   checkrvi,  &
                   nodiff,  &
                   ObjectName,  &
                   DifLfn,      &
                   .false.,  & !OutVar
                   .true., & !MtrVar
                   .false., & !TimeBinVar
                   CurArgs, &
                   Written, &
                   .false.)
                IF (DelThis) CYCLE

              CASE ('REPORT:TABLE:TIMEBINS')
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                nodiff=.true.
                IF (OutArgs(1) == Blank) THEN
                  OutArgs(1)='*'
                  nodiff=.false.
                ENDIF
                CALL ScanOutputVariablesForReplacement(  &
                   2,  &
                   DelThis,  &
                   checkrvi,  &
                   nodiff,  &
                   ObjectName,  &
                   DifLfn,      &
                   .false.,  & !OutVar
                   .false., & !MtrVar
                   .true., & !TimeBinVar
                   CurArgs, &
                   Written, &
                   .false.)
                IF (DelThis) CYCLE

              CASE ('REPORT:TABLE:MONTHLY')
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                nodiff=.true.
                IF (OutArgs(1) == Blank) THEN
                  OutArgs(1)='*'
                  nodiff=.false.
                ENDIF
                CurVar=3
                DO Var=3,CurArgs,2
                  UCRepVarName=MakeUPPERCase(InArgs(Var))
                  OutArgs(CurVar)=InArgs(Var)
                  OutArgs(CurVar+1)=InArgs(Var+1)
                  pos=INDEX(UCRepVarName,'[')
                  IF (pos > 0) THEN
                    UCRepVarName=UCRepVarName(1:pos-1)
                    OutArgs(CurVar)=InArgs(Var)(1:pos-1)
                    OutArgs(CurVar+1)=InArgs(Var+1)
                  ENDIF
                  DelThis=.false.
                  DO Arg=1,NumRepVarNames
                    UCCompRepVarName=MakeUPPERCase(OldRepVarName(Arg))
                    IF (UCCompRepVarName(Len_Trim(UCCompRepVarName):Len_Trim(UCCompRepVarName)) == '*') THEN
                      WildMatch=.true.
                      UCCompRepVarName(Len_Trim(UCCompRepVarName):Len_Trim(UCCompRepVarName))=' '
                    ELSE
                      WildMatch=.false.
                    ENDIF
                    pos=INDEX(TRIM(UCRepVarname),TRIM(UCCompRepVarName))
                    IF (pos > 0 .and. pos /= 1) CYCLE
                    IF (pos > 0) THEN
                      IF (NewRepVarName(Arg) /= '<DELETE>') THEN
                        IF (.not. WildMatch) THEN
                          OutArgs(CurVar)=NewRepVarName(Arg)
                        ELSE
                          OutArgs(CurVar)=TRIM(NewRepVarName(Arg))//OutArgs(CurVar)(Len_Trim(UCCompRepVarName)+1:)
                        ENDIF
                        OutArgs(CurVar+1)=InArgs(Var+1)
                        nodiff=.false.
                      ELSE
                        DelThis=.true.
                      ENDIF
                      IF (OldRepVarName(Arg) == OldRepVarName(Arg+1)) THEN
                        ! Adding a var field.
                        CurVar=CurVar+2
                        IF (.not. WildMatch) THEN
                          OutArgs(CurVar)=NewRepVarName(Arg+1)
                        ELSE
                          OutArgs(CurVar)=TRIM(NewRepVarName(Arg+1))//OutArgs(CurVar)(Len_Trim(UCCompRepVarName)+1:)
                        ENDIF
                        OutArgs(CurVar+1)=InArgs(Var+1)
                        nodiff=.false.
                      ENDIF
                      IF (OldRepVarName(Arg) == OldRepVarName(Arg+2)) THEN
                        ! Adding a var field.
                        CurVar=CurVar+2
                        IF (.not. WildMatch) THEN
                          OutArgs(CurVar)=NewRepVarName(Arg+2)
                        ELSE
                          OutArgs(CurVar)=TRIM(NewRepVarName(Arg+2))//OutArgs(CurVar)(Len_Trim(UCCompRepVarName)+1:)
                        ENDIF
                        OutArgs(CurVar+1)=InArgs(Var+1)
                        nodiff=.false.
                      ENDIF
                      EXIT
                    ENDIF
                  ENDDO
                  IF (.not. DelThis) CurVar=CurVar+2
                ENDDO
                CurArgs=CurVar-1

                CASE DEFAULT
                  IF (FindItemInList(ObjectName,NotInNew,SIZE(NotInNew)) /= 0) THEN
                    WRITE(Auditf,fmta) 'Object="'//TRIM(ObjectName)//'" is not in the "new" IDD.'
                    WRITE(Auditf,fmta) '... will be listed as comments on the new output file.'
                    CALL WriteOutIDFLinesAsComments(DifLfn,ObjectName,CurArgs,InArgs,FldNames,FldUnits)
                    Written=.true.
                    !CYCLE
                  ELSE
                    CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                    OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                    NoDiff=.true.
                  ENDIF
              END SELECT

            ELSE   !!! Making Pretty

              ! Just making pretty -- no changes as above.
              CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
              OutArgs(1:CurArgs)=InArgs(1:CurArgs)
            ENDIF

            IF (DiffMinFields .and. nodiff) THEN
            ! Change in min-fields
              CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
              OutArgs(1:CurArgs)=InArgs(1:CurArgs)
              NoDiff=.false.
              DO Arg=CurArgs+1,NwObjMinFlds
                OutArgs(Arg)=NwFldDefaults(Arg)
              ENDDO
              CurArgs=MAX(NwObjMinFlds,CurArgs)
            ENDIF

            IF (NoDiff .and. DiffOnly) CYCLE

            !! reformat for better readability
            !! BUILDING,SOLUTION ALGORITHM,OUTSIDE CONVECTION ALGORITHM,INSIDE CONVECTION ALGORITHM,REPORT VARIABLE,
            !! SURFACE:HEATTRANSFER,SURFACE:HEATTRANSFER:SUBSURFACE:SHADING:DETACHED,
            !! SURFACE:SHADING:DETACHED:FIXED,SURFACE:SHADING:DETACHED:BUILDING,
            !! SURFACE:SHADING:ATTACHED,
            !! WINDOWGLASSSPECTRALDATA,
            !! FLUIDPROPERTYTEMPERATURES,
            !! FLUIDPROPERTYSATURATED,FLUIDPROPERTYSUPERHEATED,FLUIDPROPERTYCONCENTRATION
            IF (.not. Written) THEN
              CALL CheckSpecialObjects(DifLfn,ObjectName,CurArgs,OutArgs,NwFldNames,NwFldUnits,Written)
            ENDIF

            IF (.not. Written) THEN
              CALL WriteOutIDFLines(DifLfn,ObjectName,CurArgs,OutArgs,NwFldNames,NwFldUnits)
            ENDIF

          ENDDO  ! IDFRecords

          IF (IDFRecords(NumIDFRecords)%CommtE /= CurComment) THEN
            DO xcount=IDFRecords(NumIDFRecords)%CommtE+1,CurComment
              WRITE(DifLfn,fmta) TRIM(Comments(xcount))
              if (xcount == IDFRecords(Num)%CommtE) WRITE(DifLfn,fmta) ' '
            ENDDO
          ENDIF

          CLOSE(DifLfn)
          IF (checkrvi) THEN
            CALL ProcessRviMviFiles(FileNamePath,'rvi')
            CALL ProcessRviMviFiles(FileNamePath,'mvi')
          ENDIF
          CALL CloseOut
        ELSE  ! not a idf or imf
          CALL ProcessRviMviFiles(FileNamePath,'rvi')
          CALL ProcessRviMviFiles(FileNamePath,'mvi')
        ENDIF
      ELSE  ! Full name == Blank
        EndOfFile=.true.
      ENDIF

      CALL CreateNewName('Reallocate',CreatedOutputName,' ')

      IF (Allocated(DeleteThisRecord)) THEN
        DEALLOCATE(DeleteThisRecord)
        DEALLOCATE(Alphas)
        DEALLOCATE(Numbers)
        DEALLOCATE(InArgs)
        DEALLOCATE(AorN)
        DEALLOCATE(ReqFld)
        DEALLOCATE(FldNames)
        DEALLOCATE(FldDefaults)
        DEALLOCATE(FldUnits)
        DEALLOCATE(NwAorN)
        DEALLOCATE(NwReqFld)
        DEALLOCATE(NwFldNames)
        DEALLOCATE(NwFldDefaults)
        DEALLOCATE(NwFldUnits)
        DEALLOCATE(OutArgs)
        DEALLOCATE(MatchArg)
      ENDIF

    ENDDO

    IF (.not. ExitBecauseBadFile) THEN
      StillWorking=.false.
      EXIT
    ELSE
      IF (.not. ArgFileBeingDone) THEN
        EndOfFile=.false.
      ELSE
        EndOfFile=.true.
        StillWorking=.false.
      ENDIF
    ENDIF
  ENDDO

  IF (ArgFileBeingDone .and. .not. LatestVersion .and. .not. ExitBecauseBadFile) THEN
    ! If this is true, then there was a "arg IDF File" on the command line and some files need to be
                         ! renamed.
    ErrFlag=.false.
    CALL copyfile(TRIM(FileNamePath)//'.'//TRIM(ArgIDFExtension),TRIM(FileNamePath)//'.'//TRIM(ArgIDFExtension)//'old',ErrFlag)
!    SysResult=SystemQQ('copy "'//TRIM(FileNamePath)//'.'//TRIM(ArgIDFExtension)//'" "'//  &
!                                    TRIM(FileNamePath)//'.'//TRIM(ArgIDFExtension)//'old"')
    CALL copyfile(TRIM(FileNamePath)//'.'//TRIM(ArgIDFExtension)//'new',TRIM(FileNamePath)//'.'//TRIM(ArgIDFExtension),ErrFlag)
!    SysResult=SystemQQ('copy "'//TRIM(FileNamePath)//'.'//TRIM(ArgIDFExtension)//'new" "'//  &
!                                  TRIM(FileNamePath)//'.'//TRIM(ArgIDFExtension)//'"')
    INQUIRE(File=TRIM(FileNamePath)//'.rvi',EXIST=FileExist)
    IF (FileExist) THEN
      CALL copyfile(TRIM(FileNamePath)//'.rvi',TRIM(FileNamePath)//'.rviold',ErrFlag)
!      SysResult=SystemQQ('copy "'//TRIM(FileNamePath)//'.rvi" "'//TRIM(FileNamePath)//'.rviold"')
    ENDIF
    INQUIRE(File=TRIM(FileNamePath)//'.rvinew',EXIST=FileExist)
    IF (FileExist) THEN
      CALL copyfile(TRIM(FileNamePath)//'.rvinew',TRIM(FileNamePath)//'.rvi',ErrFlag)
!      SysResult=SystemQQ('copy "'//TRIM(FileNamePath)//'.rvinew" "'//TRIM(FileNamePath)//'.rvi"')
    ENDIF
    INQUIRE(File=TRIM(FileNamePath)//'.mvi',EXIST=FileExist)
    IF (FileExist) THEN
      CALL copyfile(TRIM(FileNamePath)//'.mvi',TRIM(FileNamePath)//'.mviold',ErrFlag)
!      SysResult=SystemQQ('copy "'//TRIM(FileNamePath)//'.mvi" "'//TRIM(FileNamePath)//'.mviold"')
    ENDIF
    INQUIRE(File=TRIM(FileNamePath)//'.mvinew',EXIST=FileExist)
    IF (FileExist) THEN
      CALL copyfile(TRIM(FileNamePath)//'.mvinew',TRIM(FileNamePath)//'.mvi',ErrFlag)
!      SysResult=SystemQQ('copy "'//TRIM(FileNamePath)//'.mvinew" "'//TRIM(FileNamePath)//'.mvi"')
    ENDIF
  ENDIF

  RETURN

END SUBROUTINE CreateNewIDFUsingRules
