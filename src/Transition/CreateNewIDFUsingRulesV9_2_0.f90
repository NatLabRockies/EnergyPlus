MODULE SetVersion

USE DataStringGlobals
USE DataVCompareGlobals

PUBLIC

CONTAINS

SUBROUTINE SetThisVersionVariables()
      VerString='Conversion 9.1 => 9.2'
      VersionNum=9.2
      sVersionNum='9.2'
      IDDFileNameWithPath=TRIM(ProgramPath)//'V9-1-0-Energy+.idd'
      NewIDDFileNameWithPath=TRIM(ProgramPath)//'V9-2-0-Energy+.idd'
      RepVarFileNameWithPath=TRIM(ProgramPath)//'Report Variables 9-1-0 to 9-2-0.csv'
END SUBROUTINE

END MODULE

SUBROUTINE CreateNewIDFUsingRules(EndOfFile,DiffOnly,InLfn,AskForInput,InputFileName,ArgFile,ArgIDFExtension)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   July 2002
          !       MODIFIED       For each release
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine creates new IDFs based on the rules specified by
          ! developers.  This will result in a more complete transition but
          ! takes more time to create.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
  USE InputProcessor
  USE DataVCompareGlobals
  USE VCompareGlobalRoutines
  USE DataStringGlobals, ONLY: ProgNameConversion
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

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  INTEGER IoS
  INTEGER DotPos
  INTEGER Status
  INTEGER NA
  INTEGER NN
  INTEGER CurArgs
  INTEGER DifLfn
  INTEGER xCount
  INTEGER Num
  INTEGER, EXTERNAL :: GetNewUnitNumber
  INTEGER Arg
  LOGICAL, SAVE :: FirstTime=.true.
  CHARACTER(len=30) UnitsArg
  CHARACTER(len=MaxNameLength) ::  ObjectName
  CHARACTER(len=30), EXTERNAL :: TrimTrailZeros
  CHARACTER(len=MaxNameLength) ::  UCRepVarName=blank
  CHARACTER(len=MaxNameLength) ::  UCCompRepVarName=blank
  LOGICAL DelThis
  INTEGER pos
  INTEGER pos2
  LOGICAL ExitBecauseBadFile
  LOGICAL StillWorking
  LOGICAL NoDiff
  LOGICAL checkrvi
  LOGICAL NoVersion
  LOGICAL DiffMinFields  ! Set to true when diff number of min-fields between the two objects
  LOGICAL Written
  INTEGER :: Var
  INTEGER :: CurVar
  LOGICAL ArgFileBeingDone
  LOGICAL LatestVersion
  CHARACTER(len=10) :: LocalFileExtension=' '
  LOGICAL :: WildMatch

  ! For Table:Lookup objects
  INTEGER NumPerimObjs
  INTEGER PArgs

  INTEGER :: iPt, iPt2, iPt3
  INTEGER, ALLOCATABLE, DIMENSION(:) :: NumIndVarsVals, CurIndices, Increments, StepSize
  INTEGER NumOutputVals, NumIndVars, RefIndex, FlatIndex, NumMiniTables, MiniTableSize
  CHARACTER(len=MaxNameLength), ALLOCATABLE, DIMENSION(:,:) :: IndVars
  INTEGER, ALLOCATABLE, DIMENSION(:,:) :: IndVarOrder
  CHARACTER(len=MaxNameLength), ALLOCATABLE, DIMENSION(:) :: OutputVals

  LOGICAL :: ConnComp
  LOGICAL :: ConnCompCtrl
  LOGICAL :: FileExist
  CHARACTER(len=MaxNameLength) :: CreatedOutputName
  LOGICAL, ALLOCATABLE, DIMENSION(:) :: DeleteThisRecord
  INTEGER :: COutArgs
  CHARACTER(len=16) :: UnitsField
  LOGICAL :: ScheduleTypeLimitsAnyNumber
  LOGICAL :: cycling
  LOGICAL :: continuous
  CHARACTER(len=MaxNameLength) :: OutScheduleName
  LOGICAL :: isDElightOutVar

  REAL :: GLHETempVal = 0.0
  INTEGER :: TempArgsNum = 0

  LOGICAL :: ErrFlag

  INTEGER :: I, CurField, NewField, KAindex=0, SearchNum
  INTEGER :: AlphaNumI
  REAL :: SaveNumber
  CHARACTER(len=MaxNameLength*2), ALLOCATABLE, DIMENSION(:) :: PFldNames
  CHARACTER(len=MaxNameLength), ALLOCATABLE, DIMENSION(:) :: PFldDefaults
  CHARACTER(len=20), ALLOCATABLE, DIMENSION(:) :: PFldUnits
  INTEGER PObjMinFlds
  LOGICAL, ALLOCATABLE, DIMENSION(:) :: PAOrN
  LOGICAL, ALLOCATABLE, DIMENSION(:) :: PReqFld
  INTEGER PNumArgs   ! Number of Arguments in a definition
  CHARACTER(len=MaxNameLength), ALLOCATABLE, DIMENSION(:) :: POutArgs


  ! For Defaulting now-required RunPeriod Name
  INTEGER :: TotRunPeriods = 0
  INTEGER :: runPeriodNum = 0
  INTEGER :: iterateRunPeriod = 0
  CHARACTER(len=MaxNameLength), ALLOCATABLE, DIMENSION(:) :: CurrentRunPeriodNames
  CHARACTER(len=20) :: PotentialRunPeriodName

  ! Only needed for ZoneHVAC:EquipmentList translation
  INTEGER zeqNum
  CHARACTER(len=20) :: zeqNumStr
  CHARACTER(len=7) :: zeqHeatingOrCooling
  LOGICAL :: writeScheduleTypeObj = .true.


  If (FirstTime) THEN  ! do things that might be applicable only to this new version
    FirstTime=.false.
  EndIf

  StillWorking=.true.
  ArgFileBeingDone=.false.
  LatestVersion=.false.
  NoVersion=.true.
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
          WRITE(Auditf,*) 'File not found='//TRIM(FullFileName)
          EndOfFile=.true.
          ExitBecauseBadFile=.true.
          EXIT
        ENDIF
        IF (LocalFileExtension == 'idf' .or. LocalFileExtension == 'imf') THEN
          checkrvi=.false.
          ConnComp=.false.
          ConnCompCtrl=.false.
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

          ! Clean up from any previous passes, then re-allocate
          IF(ALLOCATED(DeleteThisRecord)) DEALLOCATE(DeleteThisRecord)
          IF(ALLOCATED(Alphas)) DEALLOCATE(Alphas)
          IF(ALLOCATED(Numbers)) DEALLOCATE(Numbers)
          IF(ALLOCATED(InArgs)) DEALLOCATE(InArgs)
          IF(ALLOCATED(TempArgs)) DEALLOCATE(TempArgs)
          IF(ALLOCATED(AorN)) DEALLOCATE(AorN)
          IF(ALLOCATED(ReqFld)) DEALLOCATE(ReqFld)
          IF(ALLOCATED(FldNames)) DEALLOCATE(FldNames)
          IF(ALLOCATED(FldDefaults)) DEALLOCATE(FldDefaults)
          IF(ALLOCATED(FldUnits)) DEALLOCATE(FldUnits)
          IF(ALLOCATED(NwAorN)) DEALLOCATE(NwAorN)
          IF(ALLOCATED(NwReqFld)) DEALLOCATE(NwReqFld)
          IF(ALLOCATED(NwFldNames)) DEALLOCATE(NwFldNames)
          IF(ALLOCATED(NwFldDefaults)) DEALLOCATE(NwFldDefaults)
          IF(ALLOCATED(NwFldUnits)) DEALLOCATE(NwFldUnits)
          IF(ALLOCATED(OutArgs)) DEALLOCATE(OutArgs)
          IF(ALLOCATED(PAorN)) DEALLOCATE(PAorN)
          IF(ALLOCATED(PReqFld)) DEALLOCATE(PReqFld)
          IF(ALLOCATED(PFldNames)) DEALLOCATE(PFldNames)
          IF(ALLOCATED(PFldDefaults)) DEALLOCATE(PFldDefaults)
          IF(ALLOCATED(PFldUnits)) DEALLOCATE(PFldUnits)
          IF(ALLOCATED(POutArgs)) DEALLOCATE(POutArgs)
          IF(ALLOCATED(MatchArg)) DEALLOCATE(MatchArg)
          ALLOCATE(Alphas(MaxAlphaArgsFound),Numbers(MaxNumericArgsFound))
          ALLOCATE(InArgs(MaxTotalArgs))
          ALLOCATE(TempArgs(MaxTotalArgs))
          ALLOCATE(AorN(MaxTotalArgs),ReqFld(MaxTotalArgs),FldNames(MaxTotalArgs),FldDefaults(MaxTotalArgs),FldUnits(MaxTotalArgs))
          ALLOCATE(NwAorN(MaxTotalArgs),NwReqFld(MaxTotalArgs),NwFldNames(MaxTotalArgs),NwFldDefaults(MaxTotalArgs),NwFldUnits(MaxTotalArgs))
          ALLOCATE(PAorN(MaxTotalArgs),PReqFld(MaxTotalArgs),PFldNames(MaxTotalArgs),PFldDefaults(MaxTotalArgs),PFldUnits(MaxTotalArgs))
          ALLOCATE(OutArgs(MaxTotalArgs))
          ALLOCATE(POutArgs(MaxTotalArgs))
          ALLOCATE(MatchArg(MaxTotalArgs))
          ALLOCATE(DeleteThisRecord(NumIDFRecords))
          DeleteThisRecord=.false.

          NoVersion=.true.
          DO Num=1,NumIDFRecords
            IF (MakeUPPERCase(IDFRecords(Num)%Name) /= 'VERSION') CYCLE
            NoVersion=.false.
            EXIT
          ENDDO

          ScheduleTypeLimitsAnyNumber=.false.
          DO Num=1,NumIDFRecords
            IF (.not. SameString(IDFRecords(Num)%Name,'ScheduleTypeLimits')) CYCLE
            IF (.not. SameString(IDFRecords(Num)%Alphas(1),'Any Number')) CYCLE
            ScheduleTypeLimitsAnyNumber=.true.
            EXIT
          ENDDO

          DO Num=1,NumIDFRecords
            IF (DeleteThisRecord(Num)) THEN
              Write(DifLfn,fmta) '! Deleting: '//TRIM(IDFRecords(Num)%Name)//'="'//TRIM(IDFRecords(Num)%Alphas(1))//'".'
            ENDIF
          ENDDO

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!                                                    P R E P R O C E S S I N G                                                     !
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

     ! Begin Pre-process RunPeriod to avoid having duplicated names. If the user has one named 'RUNPERIOD 1',
        ! we do not want to default the first blank to 'RUNPERIOD 1', but rather 'RUNPERIOD 2'
          CALL DisplayString('Processing IDF -- RunPeriod preprocessing . . .')

          ! Clean up from any previous passes, then re-allocate
          IF(ALLOCATED(CurrentRunPeriodNames)) DEALLOCATE(CurrentRunPeriodNames)
          iterateRunPeriod = 0
          TotRunPeriods = GetNumObjectsFound('RUNPERIOD')
          ALLOCATE(CurrentRunPeriodNames(TotRunPeriods))

          ! First pass to get all current run period names, ensure we get unique ones in the end
          DO runPeriodNum=1,TotRunPeriods
            CALL GetObjectItem('RUNPERIOD',runPeriodNum,Alphas,NumAlphas,Numbers,NumNumbers,Status)
            CurrentRunPeriodNames(runPeriodNum) = TRIM(Alphas(1));
          ENDDO
          CALL DisplayString('Processing IDF -- RunPeriod preprocessing complete.')
     ! End Pre-process RunPeriod



!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!                                                       P R O C E S S I N G                                                        !
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


          CALL DisplayString('Processing IDF -- Processing idf objects . . .')
          DO Num=1,NumIDFRecords

            IF (DeleteThisRecord(Num)) CYCLE
            DO xcount=IDFRecords(Num)%CommtS+1,IDFRecords(Num)%CommtE
              WRITE(DifLfn,fmta) TRIM(Comments(xcount))
              if (xcount == IDFRecords(Num)%CommtE) WRITE(DifLfn,fmta) ''
            ENDDO
            IF (NoVersion .and. Num == 1) THEN
              CALL GetNewObjectDefInIDD('VERSION',NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
              OutArgs(1) = sVersionNum
              CurArgs=1
              CALL WriteOutIDFLinesAsComments(DifLfn,'Version',CurArgs,OutArgs,NwFldNames,NwFldUnits)
            ENDIF

     ! deleted objects.  no transition.
            IF (MakeUPPERCase(TRIM(IDFRecords(Num)%Name)) == 'PROGRAMCONTROL') CYCLE
            IF (MakeUPPERCase(TRIM(IDFRecords(Num)%Name)) == 'SKY RADIANCE DISTRIBUTION') CYCLE
            IF (MakeUPPERCase(TRIM(IDFRecords(Num)%Name)) == 'AIRFLOW MODEL') CYCLE
            IF (MakeUPPERCase(TRIM(IDFRecords(Num)%Name)) == 'GENERATOR:FC:BATTERY DATA') CYCLE
            IF (MakeUPPERCase(TRIM(IDFRecords(Num)%Name)) == 'AIRFLOWNETWORK:MULTIZONE:SITEWINDCONDITIONS') CYCLE
            IF (MakeUPPERCase(TRIM(IDFRecords(Num)%Name)) == 'WATER HEATER:SIMPLE') THEN
              WRITE(DifLfn,fmta) '! ** The WATER HEATER:SIMPLE object has been deleted'
              CALL writePreprocessorObject(DifLfn,PrognameConversion,'Warning','The WATER HEATER:SIMPLE object has been deleted')
              CYCLE
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
              TempArgs=Blank
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

              SELECT CASE (MakeUPPERCase(TRIM(IDFRecords(Num)%Name)))

              CASE ('VERSION')
                IF ((InArgs(1)(1:3)) == sVersionNum .and. ArgFile) THEN
                  CALL ShowWarningError('File is already at latest version.  No new diff file made.',Auditf)
                  CLOSE(diflfn,STATUS='DELETE')
                  LatestVersion=.true.
                  EXIT
                ENDIF
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                OutArgs(1) = sVersionNum
                NoDiff=.false.

    ! changes for this version, pick one of the spots to add rules, this will reduce the possibility of merge conflicts

!             CASE('OBJECTNAMEHERE')
!                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
!                 nodiff=.false.
!                 OutArgs(1)=InArgs(1)
!                 OutArgs(2) = 'SequentialLoad'
!                 OutArgs(3:CurArgs+1)=InArgs(2:CurArgs)
!                 CurArgs = CurArgs + 1
!                 NoDiff = .false.

              ! If your original object starts with A, insert the rules here

              ! If your original object starts with B, insert the rules here

              ! If your original object starts with C, insert the rules here

              ! If your original object starts with D, insert the rules here

              ! If your original object starts with E, insert the rules here

              ! If your original object starts with F, insert the rules here
             CASE('FOUNDATION:KIVA')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff = .false.
                 OutArgs(1) = InArgs(1)
                 OutArgs(2) = Blank
                 OutArgs(3:CurArgs+1) = InArgs(2:CurArgs)
                 CurArgs = CurArgs + 1
                 NoDiff = .false.

              ! If your original object starts with G, insert the rules here

              ! If your original object starts with H, insert the rules here

              ! If your original object starts with I, insert the rules here

              ! If your original object starts with L, insert the rules here

              ! If your original object starts with M, insert the rules here

              ! If your original object starts with N, insert the rules here

              ! If your original object starts with O, insert the rules here

              ! If your original object starts with P, insert the rules here

              ! If your original object starts with R, insert the rules here
              CASE('RUNPERIOD')
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)

                ! Copy all Args here
                OutArgs = InArgs

                ! If the now-required RunPeriod Name is blank, then create it
                IF (SameString(TRIM(InArgs(1)), '')) THEN
                  ! If at least one, then need diff
                  nodiff = .false.
                  iterateRunPeriod = iterateRunPeriod + 1
                  PotentialRunPeriodName='RUNPERIOD '//TrimSigDigits(iterateRunPeriod)
                  ! While we can find another RunPeriod named like this, increment the number
                  DO WHILE (FindItemInList(PotentialRunPeriodName,CurrentRunPeriodNames,TotRunPeriods) /= 0)
                    iterateRunPeriod = iterateRunPeriod + 1
                    PotentialRunPeriodName='RUNPERIOD '//TrimSigDigits(iterateRunPeriod)
                  ENDDO
                  OutArgs(1) = PotentialRunPeriodName
                ENDIF

              ! If your original object starts with S, insert the rules here
              CASE('SCHEDULE:FILE')
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                OutArgs = InArgs
                IF (SameString(TRIM(InArgs(7)), 'FIXED')) THEN
                  nodiff = .false.
                  OutArgs(7) = 'SPACE'
                END IF

              ! If your original object starts with T, insert the rules here

              CASE('TABLE:ONEINDEPENDENTVARIABLE')
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                ! Delete old object
                WRITE(Auditf,fmta) 'Object="'//TRIM(ObjectName)//'" is not in the "new" IDD.'
                WRITE(Auditf,fmta) '... will be listed as comments on the new output file.'
                CALL WriteOutIDFLinesAsComments(DifLfn,ObjectName,CurArgs,InArgs,FldNames,FldUnits)
                Written=.true.

                ! Create new Table:IndependentVariable object
                IF(ALLOCATED(NumIndVarsVals)) DEALLOCATE(NumIndVarsVals)
                IF(ALLOCATED(IndVars)) DEALLOCATE(IndVars)
                IF(ALLOCATED(OutputVals)) DEALLOCATE(OutputVals)
                ALLOCATE(NumIndVarsVals(1))

                NumIndVarsVals(1) = (CurArgs - 10)/2
                NumOutputVals = NumIndVarsVals(1)

                ALLOCATE(IndVars(1,NumIndVarsVals(1)))
                ALLOCATE(OutputVals(NumOutputVals))

                PArgs = 10 + NumIndVarsVals(1)
                CALL GetNewObjectDefInIDD('TABLE:INDEPENDENTVARIABLE',PNumArgs,PAorN,NwReqFld,PObjMinFlds,PFldNames,PFldDefaults,PFldUnits)
                POutArgs(1) = TRIM(InArgs(1))//'_IndependentVariable1'
                IF (SameString(InArgs(3),"LINEARINTERPOLATIONOFTABLE")) THEN
                  POutArgs(2) = "Linear"
                  POutArgs(3) = "Constant"
                ELSEIF (SameString(InArgs(3),"LAGRANGEINTERPOLATIONLINEAREXTRAPOLATION")) THEN
                  POutArgs(2) = "Cubic"
                  POutArgs(3) = "Linear"
                ELSE
                  POutArgs(2) = "Cubic"
                  POutArgs(3) = "Constant"
                END IF
                POutArgs(4:5) = InArgs(4:5)  ! Min/Max

                DO iPt=1,NumIndVarsVals(1)
                  IndVars(1,iPt) = InArgs(11 + 2*(iPt - 1))
                  OutputVals(iPt) = InArgs(12 + 2*(iPt - 1))
                END DO

                POutArgs(6) = Blank
                POutArgs(7) = InArgs(8)
                POutArgs(8:10) = Blank
                POutArgs(11:PArgs) = IndVars(1,1:NumIndVarsVals(1))

                CALL WriteOutIDFLines(DifLfn,'Table:IndependentVariable',PArgs,POutArgs,PFldNames,PFldUnits)

                ! Create new Table:IndependentVariableList object
                PArgs = 2
                CALL GetNewObjectDefInIDD('TABLE:INDEPENDENTVARIABLELIST',PNumArgs,PAorN,NwReqFld,PObjMinFlds,PFldNames,PFldDefaults,PFldUnits)
                POutArgs(1) = TRIM(InArgs(1))//'_IndependentVariableList'
                POutArgs(2) = TRIM(InArgs(1))//'_IndependentVariable1'
                CALL WriteOutIDFLines(DifLfn,'Table:IndependentVariableList',PArgs,POutArgs,PFldNames,PFldUnits)

                ! Create new Table:Lookup object
                PArgs = 10 + NumOutputVals
                CALL GetNewObjectDefInIDD('TABLE:LOOKUP',PNumArgs,PAorN,NwReqFld,PObjMinFlds,PFldNames,PFldDefaults,PFldUnits)
                POutArgs(1) = TRIM(InArgs(1))
                POutArgs(2) = TRIM(InArgs(1))//'_IndependentVariableList'

                ! Normalization Value
                IF (InArgs(10) == Blank) THEN
                  POutArgs(3) = Blank
                  POutArgs(4) = Blank
                ELSE
                  POutArgs(3) = 'DivisorOnly'
                  POutArgs(4) = InArgs(10)
                END IF
                POutArgs(5:6) = InArgs(6:7)
                POutArgs(7) = InArgs(9)
                POutArgs(8:10) = Blank
                POutArgs(11:PArgs) = OutputVals(1:NumOutputVals)

                CALL WriteOutIDFLines(DifLfn,'Table:Lookup',PArgs,POutArgs,PFldNames,PFldUnits)

              CASE('TABLE:TWOINDEPENDENTVARIABLES')
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                ! Delete old object
                WRITE(Auditf,fmta) 'Object="'//TRIM(ObjectName)//'" is not in the "new" IDD.'
                WRITE(Auditf,fmta) '... will be listed as comments on the new output file.'
                CALL WriteOutIDFLinesAsComments(DifLfn,ObjectName,CurArgs,InArgs,FldNames,FldUnits)
                Written=.true.

                IF(ALLOCATED(NumIndVarsVals)) DEALLOCATE(NumIndVarsVals)
                IF(ALLOCATED(IndVars)) DEALLOCATE(IndVars)
                IF(ALLOCATED(IndVarOrder)) DEALLOCATE(IndVarOrder)
                IF(ALLOCATED(OutputVals)) DEALLOCATE(OutputVals)
                ALLOCATE(NumIndVarsVals(2))

                ! Create arrays from data
                NumOutputVals = (CurArgs - 14)/3
                ALLOCATE(IndVars(2,NumOutputVals))
                ALLOCATE(IndVarOrder(2,NumOutputVals))
                ALLOCATE(OutputVals(NumOutputVals))

                IF (InArgs(14) /= Blank) THEN
                  CALL writePreprocessorObject(DifLfn,PrognameConversion,'Warning',  &
                          'Table:TwoIndependentVariables="'//trim(InArgs(1))//  &
                                  '" references an external file="'//  &
                                  trim(InArgs(14))//'". External files must be converted to the new format using'//&
                          'table_convert.py.')
                  write(diflfn,fmtA) ' '
                  CALL ShowWarningError('Table:TwoIndependentVariables="'//trim(InArgs(1))//  &
                          '" references an external file="'//  &
                          trim(InArgs(14))//'". External files must be converted to the new format using'//&
                          'table_convert.py or by appending the contents of the external file to the original IDF object.',Auditf)
                END IF

                DO iPt=1,NumOutputVals
                  IndVars(1,iPt) = InArgs(15 +3*(iPt - 1))
                  IndVars(2,iPt) = InArgs(16 +3*(iPt - 1))
                  OutputVals(iPt) = InArgs(17 +3*(iPt - 1))
                END DO

                DO iPt=1,2
                  NumIndVarsVals(iPt) = NumOutputVals
                  CALL SortUnique(IndVars(iPt,:),NumIndVarsVals(iPt),IndVarOrder(iPt,:))

                  ! Create new Table:IndependentVariable object
                  PArgs = 10 + NumIndVarsVals(iPt)
                  CALL GetNewObjectDefInIDD('TABLE:INDEPENDENTVARIABLE',PNumArgs,PAorN,NwReqFld,PObjMinFlds,PFldNames,PFldDefaults,PFldUnits)
                  POutArgs(1) = TRIM(InArgs(1))//'_IndependentVariable'//RoundSigDigits(iPt)
                  IF (SameString(InArgs(3),"LINEARINTERPOLATIONOFTABLE")) THEN
                    POutArgs(2) = "Linear"
                    POutArgs(3) = "Constant"
                  ELSEIF (SameString(InArgs(3),"LAGRANGEINTERPOLATIONLINEAREXTRAPOLATION")) THEN
                    POutArgs(2) = "Cubic"
                    POutArgs(3) = "Linear"
                  ELSE
                    POutArgs(2) = "Cubic"
                    POutArgs(3) = "Constant"
                  END IF

                  IF (iPt == 1) THEN
                    POutArgs(4:5) = InArgs(4:5)  ! Min/Max
                    POutArgs(7) = InArgs(10)
                  ELSE
                    POutArgs(4:5) = InArgs(6:7)  ! Min/Max
                    POutArgs(7) = InArgs(11)
                  END IF

                  POutArgs(6) = Blank
                  POutArgs(8:10) = Blank

                  POutArgs(11:PArgs) = IndVars(iPt,1:NumIndVarsVals(iPt))

                  CALL WriteOutIDFLines(DifLfn,'Table:IndependentVariable',PArgs,POutArgs,PFldNames,PFldUnits)
                END DO

                IF (NumOutputVals /= NumIndVarsVals(1)*NumIndVarsVals(2)) THEN
                  PRINT *, "Dimensional Mismatch"
                END IF

                ! Create new Table:IndependentVariableList object
                PArgs = 3
                CALL GetNewObjectDefInIDD('TABLE:INDEPENDENTVARIABLELIST',PNumArgs,PAorN,NwReqFld,PObjMinFlds,PFldNames,PFldDefaults,PFldUnits)
                POutArgs(1) = TRIM(InArgs(1))//'_IndependentVariableList'
                POutArgs(2) = TRIM(InArgs(1))//'_IndependentVariable1'
                POutArgs(3) = TRIM(InArgs(1))//'_IndependentVariable2'
                CALL WriteOutIDFLines(DifLfn,'Table:IndependentVariableList',PArgs,POutArgs,PFldNames,PFldUnits)

                ! Create new Table:Lookup object
                PArgs = 10 + NumOutputVals
                CALL GetNewObjectDefInIDD('TABLE:LOOKUP',PNumArgs,PAorN,NwReqFld,PObjMinFlds,PFldNames,PFldDefaults,PFldUnits)
                POutArgs(1) = TRIM(InArgs(1))
                POutArgs(2) = TRIM(InArgs(1))//'_IndependentVariableList'

                ! Normalization Value
                IF (InArgs(13) == Blank) THEN
                  POutArgs(3) = Blank
                  POutArgs(4) = Blank
                ELSE
                  POutArgs(3) = 'DivisorOnly'
                  POutArgs(4) = InArgs(13)
                END IF
                POutArgs(5:6) = InArgs(8:9) ! Min/Max
                POutArgs(7) = InArgs(12)  ! Units
                POutArgs(8:10) = Blank

                DO iPt=1,NumOutputVals
                  POutArgs(10+(IndVarOrder(1,iPt) - 1)*NumIndVarsVals(2) + IndVarOrder(2,iPt)) = OutputVals(iPt)
                END DO

                CALL WriteOutIDFLines(DifLfn,'Table:Lookup',PArgs,POutArgs,PFldNames,PFldUnits)

              CASE('TABLE:MULTIVARIABLELOOKUP')
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                ! Delete old object
                WRITE(Auditf,fmta) 'Object="'//TRIM(ObjectName)//'" is not in the "new" IDD.'
                WRITE(Auditf,fmta) '... will be listed as comments on the new output file.'
                CALL WriteOutIDFLinesAsComments(DifLfn,ObjectName,CurArgs,InArgs,FldNames,FldUnits)
                Written=.true.

                IF(ALLOCATED(NumIndVarsVals)) DEALLOCATE(NumIndVarsVals)
                IF(ALLOCATED(CurIndices)) DEALLOCATE(CurIndices)
                IF(ALLOCATED(Increments)) DEALLOCATE(Increments)
                IF(ALLOCATED(StepSize)) DEALLOCATE(StepSize)
                IF(ALLOCATED(IndVars)) DEALLOCATE(IndVars)
                IF(ALLOCATED(IndVarOrder)) DEALLOCATE(IndVarOrder)
                IF(ALLOCATED(OutputVals)) DEALLOCATE(OutputVals)

                NumIndVars = ProcessNumber(InArgs(31),errFlag)

                ALLOCATE(NumIndVarsVals(NumIndVars))
                ALLOCATE(CurIndices(NumIndVars))
                ALLOCATE(Increments(NumIndVars))
                ALLOCATE(StepSize(NumIndVars))
                DO iPt=1,NumIndVars
                  NumIndVarsVals(iPt) = ProcessNumber(InArgs(31 + iPt),errFlag)
                END DO

                ALLOCATE(IndVars(NumIndVars,MAXVAL(NumIndVarsVals)))

                DO iPt=1,NumIndVars
                  ! Create new Table:IndependentVariable object
                  PArgs = 10 + NumIndVarsVals(iPt)
                  CALL GetNewObjectDefInIDD('TABLE:INDEPENDENTVARIABLE',PNumArgs,PAorN,NwReqFld,PObjMinFlds,PFldNames,PFldDefaults,PFldUnits)
                  POutArgs(1) = TRIM(InArgs(1))//'_IndependentVariable'//RoundSigDigits(iPt)
                  IF (SameString(InArgs(2),"LINEARINTERPOLATIONOFTABLE")) THEN
                    POutArgs(2) = "Linear"
                    POutArgs(3) = "Constant"
                  ELSEIF (SameString(InArgs(2),"LAGRANGEINTERPOLATIONLINEAREXTRAPOLATION")) THEN
                    POutArgs(2) = "Cubic"
                    POutArgs(3) = "Linear"
                  ELSE
                    POutArgs(2) = "Cubic"
                    POutArgs(3) = "Constant"
                  END IF

                  POutArgs(4:5) = InArgs(10 + 2*(iPt - 1):11 + 2*(iPt - 1))  ! Min/Max
                  POutArgs(7) = InArgs(23 + iPt)  ! Unit type

                  POutArgs(6) = Blank
                  POutArgs(8:10) = Blank

                  RefIndex = 31 + NumIndVars
                  DO iPt2=1,iPt-1
                    RefIndex = RefIndex + NumIndVarsVals(iPt2)
                  END DO

                  IndVars(iPt,1:NumIndVarsVals(iPt)) = InArgs(RefIndex + 1:RefIndex + NumIndVarsVals(iPt))

                  POutArgs(11:PArgs) = IndVars(iPt,1:NumIndVarsVals(iPt))

                  CALL WriteOutIDFLines(DifLfn,'Table:IndependentVariable',PArgs,POutArgs,PFldNames,PFldUnits)
                END DO



                ! Create new Table:IndependentVariableList object
                PArgs = 1 + NumIndVars
                CALL GetNewObjectDefInIDD('TABLE:INDEPENDENTVARIABLELIST',PNumArgs,PAorN,NwReqFld,PObjMinFlds,PFldNames,PFldDefaults,PFldUnits)
                POutArgs(1) = TRIM(InArgs(1))//'_IndependentVariableList'

                DO iPt=1,NumIndVars
                  POutArgs(1 + iPt) = TRIM(InArgs(1))//'_IndependentVariable'//RoundSigDigits(iPt)
                END DO
                CALL WriteOutIDFLines(DifLfn,'Table:IndependentVariableList',PArgs,POutArgs,PFldNames,PFldUnits)



                ! Create new Table:Lookup object
                CALL GetNewObjectDefInIDD('TABLE:LOOKUP',PNumArgs,PAorN,NwReqFld,PObjMinFlds,PFldNames,PFldDefaults,PFldUnits)
                POutArgs(1) = TRIM(InArgs(1))
                POutArgs(2) = TRIM(InArgs(1))//'_IndependentVariableList'

                ! Normalization Value
                IF (InArgs(9) == Blank) THEN
                  POutArgs(3) = Blank
                  POutArgs(4) = Blank
                ELSE
                  POutArgs(3) = 'DivisorOnly'
                  POutArgs(4) = InArgs(9)  ! Normalization Value
                END IF
                POutArgs(5:6) = InArgs(22:23) ! Min/Max
                POutArgs(7) = InArgs(30)  ! Units
                POutArgs(8:10) = Blank  ! External file

                IF (InArgs(6) /= Blank) THEN
                  CALL writePreprocessorObject(DifLfn,PrognameConversion,'Warning',  &
                          'Table:MultivariableLookup="'//trim(InArgs(1))//  &
                                  '" references an external file="'//  &
                                  trim(InArgs(6))//'". External files must be converted to the new format using'//&
                                  'table_convert.py.')
                  write(diflfn,fmtA) ' '
                  CALL ShowWarningError('Table:MultivariableLookup="'//trim(InArgs(1))//  &
                          '" references an external file="'//  &
                          trim(InArgs(6))//'". External files must be converted to the new format using'//&
                          'table_convert.py or by appending the contents of the external file to the original IDF object.',Auditf)
                END IF

                ! Create arrays from data
                NumOutputVals = NumIndVarsVals(1)
                RefIndex = 31 + NumIndVars + NumIndVarsVals(1)
                DO iPt=2,NumIndVars
                  NumOutputVals = NumOutputVals*NumIndVarsVals(iPt)
                  RefIndex = RefIndex + NumIndVarsVals(iPt)
                  CurIndices(iPt) = 1
                  Increments(iPt) = 1
                END DO

                StepSize(NumIndVars) = 1
                DO iPt=NumIndVars-1,1,-1
                  StepSize(iPt) = StepSize(iPt+1)*NumIndVarsVals(iPt+1)
                END DO


                MiniTableSize = NumIndVarsVals(1)

                IF (SameString(InArgs(7),"ASCENDING")) THEN
                  CurIndices(1) = 1
                  Increments(1) = 1
                ELSE
                  CurIndices(1) = NumIndVarsVals(1)
                  Increments(1) = -1
                END IF

                IF (NumIndVars > 1) THEN
                  MiniTableSize = MiniTableSize*NumIndVarsVals(2)
                  IF (SameString(InArgs(8),"ASCENDING")) THEN
                    CurIndices(2) = 1
                    Increments(2) = 1
                  ELSE
                    CurIndices(2) = NumIndVarsVals(2)
                    Increments(2) = -1
                  END IF
                END IF

                NumMiniTables = NumOutputVals/MiniTableSize

                PArgs = 10 + NumOutputVals
                ALLOCATE(OutputVals(NumOutputVals))

                ! Loop through inputs and assign to outputs
                DO iPt=1,NumMiniTables
                  RefIndex = RefIndex + MAX(0,NumIndVars - 2)
                  DO iPt2=1,MiniTableSize
                    FlatIndex = 1
                    DO iPt3=1,NumIndVars
                      FlatIndex = FlatIndex + (CurIndices(iPt3) - 1)*(StepSize(iPt3))
                    END DO
                    OutputVals(FlatIndex) = InArgs(RefIndex+iPt2)
                    CurIndices(1) = CurIndices(1) + Increments(1)
                    DO iPt3=1,NumIndVars - 1
                      IF (CurIndices(iPt3) == 0 .OR. CurIndices(iPt3) > NumIndVarsVals(iPt3)) THEN
                        CurIndices(iPt3 + 1) = CurIndices(iPt3 + 1) + Increments(iPt3 + 1)
                        IF (CurIndices(iPt3) == 0) THEN
                          CurIndices(iPt3) = NumIndVarsVals(iPt3)
                        ELSEIF (CurIndices(iPt3) > NumIndVarsVals(iPt3)) THEN
                          CurIndices(iPt3) = 1
                        END IF
                      END IF
                    END DO
                  END DO
                  RefIndex = RefIndex + MiniTableSize
                END DO

                DO iPt=1,NumOutputVals
                  POutArgs(11:10+NumOutputVals) = OutputVals
                END DO

                CALL WriteOutIDFLines(DifLfn,'Table:Lookup',PArgs,POutArgs,PFldNames,PFldUnits)

              CASE('THERMALSTORAGE:ICE:DETAILED')
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                nodiff=.false.
                OutArgs(1:5)=InArgs(1:5)
                IF (SameString(TRIM(InArgs(6)), 'QUADRATICLINEAR')) THEN
                  OutArgs(6) = 'FractionDischargedLMTD'
                ELSEIF (SameString(TRIM(InArgs(6)), 'CUBICLINEAR')) THEN
                  OutArgs(6) = 'LMTDMassFlow'
                ELSE
                  OutArgs(6) = InArgs(6)
                ENDIF
                OutArgs(7)=InArgs(7)
                IF (SameString(TRIM(InArgs(8)), 'QUADRATICLINEAR')) THEN
                  OutArgs(8) = 'FractionChargedLMTD'
                ELSEIF (SameString(TRIM(InArgs(8)), 'CUBICLINEAR')) THEN
                  OutArgs(8) = 'LMTDMassFlow'
                ELSE
                  OutArgs(8) = InArgs(8)
                ENDIF

                 OutArgs(9:CurArgs)=InArgs(9:CurArgs)
                 NoDiff = .false.

              ! If your original object starts with U, insert the rules here

              ! If your original object starts with V, insert the rules here

              ! If your original object starts with W, insert the rules here

              ! If your original object starts with Z, insert the rules here
              CASE('ZONEHVAC:EQUIPMENTLIST')
                nodiff = .false.
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                DO CurField = 1, CurArgs
                  IF (CurField < 3) THEN
                    zeqHeatingOrCooling = 'Neither'
                  ELSE IF (MOD((CurField - 2) - 5, 6) == 0) THEN
                    zeqHeatingOrCooling = 'Cooling'
                  ELSE IF (MOD((CurField - 2) - 6, 6) == 0) THEN
                    zeqHeatingOrCooling = 'Heating'
                  ELSE
                    zeqHeatingOrCooling = 'Neither'
                  END IF
                  IF (InArgs(CurField) /= Blank .AND. (zeqHeatingOrCooling == 'Cooling' .OR. zeqheatingOrCooling == 'Heating')) THEN
                    zeqNum = (CurField - 3) / 6 + 1
                    zeqNumStr = RoundSigDigits(zeqNum,0)
                    ! Write ScheduleTypeLimits objects once
                    IF (writeScheduleTypeObj) THEN
                      CALL GetNewObjectDefInIDD('ScheduleTypeLimits',PNumArgs,PAOrN,PReqFld,PObjMinFlds,PFldNames,PFldDefaults,PFldUnits)
                      POutArgs(1) = 'ZoneEqList ScheduleTypeLimts'
                      POutArgs(2) = '0.0'
                      POutArgs(3) = '1.0'
                      POutArgs(4) = 'Continuous'
                      CALL WriteOutIDFLines(DifLfn,'ScheduleTypeLimits',4,POutArgs,PFldNames,PFldUnits)
                      writeScheduleTypeObj = .false.
                    END IF
                    OutArgs(CurField) = TRIM(InArgs(1)) // ' ' // zeqHeatingOrCooling // 'Frac' // TRIM(ADJUSTL(zeqNumStr))
                    ! Write Schedule:Constant objects as needed
                    CALL GetNewObjectDefInIDD('Schedule:Constant',PNumArgs,PAOrN,PReqFld,PObjMinFlds,PFldNames,PFldDefaults,PFldUnits)
                    POutArgs(1) = OutArgs(CurField)
                    POutArgs(2) = 'ZoneEqList ScheduleTypeLimts'
                    POutArgs(3) = InArgs(CurField)
                    CALL WriteOutIDFLines(DifLfn,'Schedule:Constant',PNumArgs,POutArgs,PFldNames,PFldUnits)
                  ELSE
                    OutArgs(CurField) = InArgs(CurField)
                  END IF
                END DO

    !!!   Changes for report variables, meters, tables -- update names
              CASE('OUTPUT:VARIABLE')
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

              CASE ('OUTPUT:METER','OUTPUT:METER:METERFILEONLY','OUTPUT:METER:CUMULATIVE','OUTPUT:METER:CUMULATIVE:METERFILEONLY')
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

              CASE('OUTPUT:TABLE:TIMEBINS')
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

!ExternalInterface:FunctionalMockupUnitImport:From:Variable, field 2
!ExternalInterface:FunctionalMockupUnitExport:From:Variable, field 2
              CASE('EXTERNALINTERFACE:FUNCTIONALMOCKUPUNITIMPORT:FROM:VARIABLE',  &
                   'EXTERNALINTERFACE:FUNCTIONALMOCKUPUNITEXPORT:FROM:VARIABLE')
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
                   .false., & !TimeBinVar
                   CurArgs, &
                   Written, &
                   .false.)
                IF (DelThis) CYCLE

!EnergyManagementSystem:Sensor, field 3
              CASE('ENERGYMANAGEMENTSYSTEM:SENSOR')
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                nodiff=.true.
                CALL ScanOutputVariablesForReplacement(  &
                   3,  &
                   DelThis,  &
                   checkrvi,  &
                   nodiff,  &
                   ObjectName,  &
                   DifLfn,      &
                   .false.,  & !OutVar
                   .false., & !MtrVar
                   .false., & !TimeBinVar
                   CurArgs, &
                   Written, &
                   .true.)
                IF (DelThis) CYCLE

              CASE('OUTPUT:TABLE:MONTHLY')
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                nodiff=.true.
                OutArgs(1:CurArgs)=InArgs(1:CurArgs)
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
                      pos=INDEX(TRIM(UCRepVarname),TRIM(UCCompRepVarName))
                    ELSE
                      WildMatch=.false.
                      pos=0
                      if (UCRepVarName == UCCompRepVarName) pos=1
                    ENDIF
                    IF (pos > 0 .and. pos /= 1) CYCLE
                    IF (pos > 0) THEN
                      IF (NewRepVarName(Arg) /= '<DELETE>') THEN
                        IF (.not. WildMatch) THEN
                          OutArgs(CurVar)=NewRepVarName(Arg)
                        ELSE
                          OutArgs(CurVar)=TRIM(NewRepVarName(Arg))//OutArgs(CurVar)(Len_Trim(UCCompRepVarName)+1:)
                        ENDIF
                        IF (NewRepVarCaution(Arg) /= Blank .and. .not. SameString(NewRepVarCaution(Arg)(1:6),'Forkeq') ) THEN
                          IF (.not. OTMVarCaution(Arg)) THEN  ! caution message not written yet
                            CALL writePreprocessorObject(DifLfn,PrognameConversion,'Warning',  &
                               'Output Table Monthly (old)="'//trim(OldRepVarName(Arg))//  &
                               '" conversion to Output Table Monthly (new)="'//  &
                               trim(NewRepVarName(Arg))//'" has the following caution "'//trim(NewRepVarCaution(Arg))//'".')
                            write(diflfn,fmtA) ' '
                            OTMVarCaution(Arg)=.true.
                          ENDIF
                        ENDIF
                        OutArgs(CurVar+1)=InArgs(Var+1)
                        nodiff=.false.
                      ELSE
                        DelThis=.true.
                      ENDIF
                      IF (OldRepVarName(Arg) == OldRepVarName(Arg+1)) THEN
                        IF (.not. SameString(NewRepVarCaution(Arg)(1:6),'Forkeq')) THEN
                          ! Adding a var field.
                          CurVar=CurVar+2
                          IF (.not. WildMatch) THEN
                            OutArgs(CurVar)=NewRepVarName(Arg+1)
                          ELSE
                            OutArgs(CurVar)=TRIM(NewRepVarName(Arg+1))//OutArgs(CurVar)(Len_Trim(UCCompRepVarName)+1:)
                          ENDIF
                          IF (NewRepVarCaution(Arg+1) /= Blank) THEN
                            IF (.not. OTMVarCaution(Arg+1)) THEN  ! caution message not written yet
                              CALL writePreprocessorObject(DifLfn,PrognameConversion,'Warning',  &
                                 'Output Table Monthly (old)="'//trim(OldRepVarName(Arg))//  &
                                 '" conversion to Output Table Monthly (new)="'//  &
                                 trim(NewRepVarName(Arg+1))//'" has the following caution "'//trim(NewRepVarCaution(Arg+1))//'".')
                              write(diflfn,fmtA) ' '
                              OTMVarCaution(Arg+1)=.true.
                            ENDIF
                          ENDIF
                          OutArgs(CurVar+1)=InArgs(Var+1)
                          nodiff=.false.
                        ENDIF
                      ENDIF
                      IF (OldRepVarName(Arg) == OldRepVarName(Arg+2)) THEN  ! only 1 more... for ForkEq
                        ! Adding a var field.
                        CurVar=CurVar+2
                        IF (.not. WildMatch) THEN
                          OutArgs(CurVar)=NewRepVarName(Arg+2)
                        ELSE
                          OutArgs(CurVar)=TRIM(NewRepVarName(Arg+2))//OutArgs(CurVar)(Len_Trim(UCCompRepVarName)+1:)
                        ENDIF
                        IF (NewRepVarCaution(Arg+2) /= Blank) THEN
                          IF (.not. OTMVarCaution(Arg+2)) THEN  ! caution message not written yet
                            CALL writePreprocessorObject(DifLfn,PrognameConversion,'Warning',  &
                               'Output Table Monthly (old)="'//trim(OldRepVarName(Arg))//  &
                               '" conversion to Output Table Monthly (new)="'//  &
                               trim(NewRepVarName(Arg+2))//'" has the following caution "'//trim(NewRepVarCaution(Arg+2))//'".')
                            write(diflfn,fmtA) ' '
                            OTMVarCaution(Arg+2)=.true.
                          ENDIF
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

              CASE('METER:CUSTOM')
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                nodiff=.true.
                CurVar=4
                DO Var=4,CurArgs,2
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
                      pos=INDEX(TRIM(UCRepVarname),TRIM(UCCompRepVarName))
                    ELSE
                      WildMatch=.false.
                      pos=0
                      if (UCRepVarName == UCCompRepVarName) pos=1
                    ENDIF
                    IF (pos > 0 .and. pos /= 1) CYCLE
                    IF (pos > 0) THEN
                      IF (NewRepVarName(Arg) /= '<DELETE>') THEN
                        IF (.not. WildMatch) THEN
                          OutArgs(CurVar)=NewRepVarName(Arg)
                        ELSE
                          OutArgs(CurVar)=TRIM(NewRepVarName(Arg))//OutArgs(CurVar)(Len_Trim(UCCompRepVarName)+1:)
                        ENDIF
                        IF (NewRepVarCaution(Arg) /= Blank .and. .not. SameString(NewRepVarCaution(Arg)(1:6),'Forkeq') ) THEN
                          IF (.not. CMtrVarCaution(Arg)) THEN  ! caution message not written yet
                            CALL writePreprocessorObject(DifLfn,PrognameConversion,'Warning',  &
                               'Custom Meter (old)="'//trim(OldRepVarName(Arg))//  &
                               '" conversion to Custom Meter (new)="'//  &
                               trim(NewRepVarName(Arg))//'" has the following caution "'//trim(NewRepVarCaution(Arg))//'".')
                            write(diflfn,fmtA) ' '
                            CMtrVarCaution(Arg)=.true.
                          ENDIF
                        ENDIF
                        OutArgs(CurVar+1)=InArgs(Var+1)
                        nodiff=.false.
                      ELSE
                        DelThis=.true.
                      ENDIF
                      IF (OldRepVarName(Arg) == OldRepVarName(Arg+1)) THEN
                        IF (.not. SameString(NewRepVarCaution(Arg)(1:6),'Forkeq')) THEN
                          ! Adding a var field.
                          CurVar=CurVar+2
                          IF (.not. WildMatch) THEN
                            OutArgs(CurVar)=NewRepVarName(Arg+1)
                          ELSE
                            OutArgs(CurVar)=TRIM(NewRepVarName(Arg+1))//OutArgs(CurVar)(Len_Trim(UCCompRepVarName)+1:)
                          ENDIF
                          IF (NewRepVarCaution(Arg+1) /= Blank .and. .not. SameString(NewRepVarCaution(Arg+1)(1:6),'Forkeq') ) THEN
                            IF (.not. CMtrVarCaution(Arg+1)) THEN  ! caution message not written yet
                              CALL writePreprocessorObject(DifLfn,PrognameConversion,'Warning',  &
                                 'Custom Meter (old)="'//trim(OldRepVarName(Arg))//  &
                                 '" conversion to Custom Meter (new)="'//  &
                                 trim(NewRepVarName(Arg+1))//'" has the following caution "'//trim(NewRepVarCaution(Arg+1))//'".')
                              write(diflfn,fmtA) ' '
                              CMtrVarCaution(Arg+1)=.true.
                            ENDIF
                          ENDIF
                          OutArgs(CurVar+1)=InArgs(Var+1)
                          nodiff=.false.
                        ENDIF
                      ENDIF
                      IF (OldRepVarName(Arg) == OldRepVarName(Arg+2)) THEN
                        ! Adding a var field.
                        CurVar=CurVar+2
                        IF (.not. WildMatch) THEN
                          OutArgs(CurVar)=NewRepVarName(Arg+2)
                        ELSE
                          OutArgs(CurVar)=TRIM(NewRepVarName(Arg+2))//OutArgs(CurVar)(Len_Trim(UCCompRepVarName)+1:)
                        ENDIF
                        IF (NewRepVarCaution(Arg+2) /= Blank) THEN
                          IF (.not. CMtrVarCaution(Arg+2)) THEN  ! caution message not written yet
                            CALL writePreprocessorObject(DifLfn,PrognameConversion,'Warning',  &
                               'Custom Meter (old)="'//trim(OldRepVarName(Arg))//  &
                               '" conversion to Custom Meter (new)="'//  &
                               trim(NewRepVarName(Arg+2))//'" has the following caution "'//trim(NewRepVarCaution(Arg+2))//'".')
                            write(diflfn,fmtA) ' '
                            CMtrVarCaution(Arg+2)=.true.
                          ENDIF
                        ENDIF
                        OutArgs(CurVar+1)=InArgs(Var+1)
                        nodiff=.false.
                      ENDIF
                      EXIT
                    ENDIF
                  ENDDO
                  IF (.not. DelThis) CurVar=CurVar+2
                ENDDO
                CurArgs=CurVar
                DO Arg=CurVar,1,-1
                  IF (OutArgs(Arg) == Blank) THEN
                    CurArgs=CurArgs-1
                  ELSE
                    EXIT
                  ENDIF
                ENDDO

              CASE('METER:CUSTOMDECREMENT')
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                nodiff=.true.
                CurVar=4   ! In case Source Meter would change
                DO Var=4,CurArgs,2
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
                      pos=INDEX(TRIM(UCRepVarname),TRIM(UCCompRepVarName))
                    ELSE
                      WildMatch=.false.
                      pos=0
                      if (UCRepVarName == UCCompRepVarName) pos=1
                    ENDIF
                    IF (pos > 0 .and. pos /= 1) CYCLE
                    IF (pos > 0) THEN
                      IF (NewRepVarName(Arg) /= '<DELETE>') THEN
                        IF (.not. WildMatch) THEN
                          OutArgs(CurVar)=NewRepVarName(Arg)
                        ELSE
                          OutArgs(CurVar)=TRIM(NewRepVarName(Arg))//OutArgs(CurVar)(Len_Trim(UCCompRepVarName)+1:)
                        ENDIF
                        IF (NewRepVarCaution(Arg) /= Blank .and. .not. SameString(NewRepVarCaution(Arg)(1:6),'Forkeq') ) THEN
                          IF (.not. CMtrDVarCaution(Arg)) THEN  ! caution message not written yet
                            CALL writePreprocessorObject(DifLfn,PrognameConversion,'Warning',  &
                               'Custom Decrement Meter (old)="'//trim(OldRepVarName(Arg))//  &
                               '" conversion to Custom Meter (new)="'//  &
                               trim(NewRepVarName(Arg))//'" has the following caution "'//trim(NewRepVarCaution(Arg))//'".')
                            write(diflfn,fmtA) ' '
                            CMtrDVarCaution(Arg)=.true.
                          ENDIF
                        ENDIF
                        OutArgs(CurVar+1)=InArgs(Var+1)
                        nodiff=.false.
                      ELSE
                        DelThis=.true.
                      ENDIF
                      IF (OldRepVarName(Arg) == OldRepVarName(Arg+1)) THEN
                        IF (.not. SameString(NewRepVarCaution(Arg)(1:6),'Forkeq')) THEN
                          ! Adding a var field.
                          CurVar=CurVar+2
                          IF (.not. WildMatch) THEN
                            OutArgs(CurVar)=NewRepVarName(Arg+1)
                          ELSE
                            OutArgs(CurVar)=TRIM(NewRepVarName(Arg+1))//OutArgs(CurVar)(Len_Trim(UCCompRepVarName)+1:)
                          ENDIF
                          IF (NewRepVarCaution(Arg+1) /= Blank .and. .not. SameString(NewRepVarCaution(Arg+1)(1:6),'Forkeq') ) THEN
                            IF (.not. CMtrDVarCaution(Arg+1)) THEN  ! caution message not written yet
                              CALL writePreprocessorObject(DifLfn,PrognameConversion,'Warning',  &
                                 'Custom Decrement Meter (old)="'//trim(OldRepVarName(Arg))//  &
                                 '" conversion to Custom Decrement Meter (new)="'//  &
                                 trim(NewRepVarName(Arg+1))//'" has the following caution "'//trim(NewRepVarCaution(Arg+1))//'".')
                              write(diflfn,fmtA) ' '
                              CMtrDVarCaution(Arg+1)=.true.
                            ENDIF
                          ENDIF
                          OutArgs(CurVar+1)=InArgs(Var+1)
                          nodiff=.false.
                        ENDIF
                      ENDIF
                      IF (OldRepVarName(Arg) == OldRepVarName(Arg+2)) THEN
                        ! Adding a var field.
                        CurVar=CurVar+2
                        IF (.not. WildMatch) THEN
                          OutArgs(CurVar)=NewRepVarName(Arg+2)
                        ELSE
                          OutArgs(CurVar)=TRIM(NewRepVarName(Arg+2))//OutArgs(CurVar)(Len_Trim(UCCompRepVarName)+1:)
                        ENDIF
                        IF (NewRepVarCaution(Arg+2) /= Blank) THEN
                          IF (.not. CMtrDVarCaution(Arg+2)) THEN  ! caution message not written yet
                            CALL writePreprocessorObject(DifLfn,PrognameConversion,'Warning',  &
                               'Custom Decrement Meter (old)="'//trim(OldRepVarName(Arg))//  &
                               '" conversion to Custom Meter (new)="'//  &
                               trim(NewRepVarName(Arg+2))//'" has the following caution "'//trim(NewRepVarCaution(Arg+2))//'".')
                            write(diflfn,fmtA) ' '
                            CMtrDVarCaution(Arg+2)=.true.
                          ENDIF
                        ENDIF
                        OutArgs(CurVar+1)=InArgs(Var+1)
                        nodiff=.false.
                      ENDIF
                      EXIT
                    ENDIF
                  ENDDO
                  IF (.not. DelThis) CurVar=CurVar+2
                ENDDO
                CurArgs=CurVar
                DO Arg=CurVar,1,-1
                  IF (OutArgs(Arg) == Blank) THEN
                    CurArgs=CurArgs-1
                  ELSE
                    EXIT
                  ENDIF
                ENDDO

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
              CALL GetNewObjectDefInIDD(IDFRecords(Num)%Name,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
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

          CALL DisplayString('Processing IDF -- Processing idf objects complete.')
          IF (IDFRecords(NumIDFRecords)%CommtE /= CurComment) THEN
            DO xcount=IDFRecords(NumIDFRecords)%CommtE+1,CurComment
              WRITE(DifLfn,fmta) TRIM(Comments(xcount))
              if (xcount == IDFRecords(Num)%CommtE) WRITE(DifLfn,fmta) ''
            ENDDO
          ENDIF

          IF (GetNumSectionsFound('Report Variable Dictionary') > 0) THEN
            ObjectName='Output:VariableDictionary'
            CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
            nodiff=.false.
            OutArgs(1)='Regular'
            CurArgs=1
            CALL WriteOutIDFLines(DifLfn,ObjectName,CurArgs,OutArgs,NwFldNames,NwFldUnits)
          ENDIF

          INQUIRE(FILE=trim(FileNamePath)//'.rvi',EXIST=FileExist)
!          IF (FileExist) THEN
!            CALL writePreprocessorObject(DifLfn,PrognameConversion,'Warning',  &
!               'rvi file associated with this input is being processed. Review for accuracy.')
!            write(diflfn,fmtA) ' '
!          ENDIF
          CLOSE(DifLfn)
          CALL ProcessRviMviFiles(FileNamePath,'rvi')
          CALL ProcessRviMviFiles(FileNamePath,'mvi')
          CALL CloseOut
        ELSE  ! not a idf or imf
          CALL ProcessRviMviFiles(FileNamePath,'rvi')
          CALL ProcessRviMviFiles(FileNamePath,'mvi')
        ENDIF
      ELSE  ! Full name == Blank
        EndOfFile=.true.
      ENDIF

      CALL CreateNewName('Reallocate',CreatedOutputName,' ')

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
    ! If this is true, then there was a "arg IDF File" on the command line and some files need to be renamed
    ErrFlag=.false.
    CALL copyfile(TRIM(FileNamePath)//'.'//TRIM(ArgIDFExtension),TRIM(FileNamePath)//'.'//TRIM(ArgIDFExtension)//'old',ErrFlag)
    CALL copyfile(TRIM(FileNamePath)//'.'//TRIM(ArgIDFExtension)//'new',TRIM(FileNamePath)//'.'//TRIM(ArgIDFExtension),ErrFlag)
    INQUIRE(File=TRIM(FileNamePath)//'.rvi',EXIST=FileExist)
    IF (FileExist) THEN
      CALL copyfile(TRIM(FileNamePath)//'.rvi',TRIM(FileNamePath)//'.rviold',ErrFlag)
    ENDIF
    INQUIRE(File=TRIM(FileNamePath)//'.rvinew',EXIST=FileExist)
    IF (FileExist) THEN
      CALL copyfile(TRIM(FileNamePath)//'.rvinew',TRIM(FileNamePath)//'.rvi',ErrFlag)
    ENDIF
    INQUIRE(File=TRIM(FileNamePath)//'.mvi',EXIST=FileExist)
    IF (FileExist) THEN
      CALL copyfile(TRIM(FileNamePath)//'.mvi',TRIM(FileNamePath)//'.mviold',ErrFlag)
    ENDIF
    INQUIRE(File=TRIM(FileNamePath)//'.mvinew',EXIST=FileExist)
    IF (FileExist) THEN
      CALL copyfile(TRIM(FileNamePath)//'.mvinew',TRIM(FileNamePath)//'.mvi',ErrFlag)
    ENDIF
  ENDIF

  RETURN

END SUBROUTINE CreateNewIDFUsingRules

SUBROUTINE SortUnique(StrArray, Size, Order)
  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine
  CHARACTER(len=*), DIMENSION(Size) :: StrArray
  Integer, DIMENSION(Size) :: Order
  INTEGER :: Size, InitSize

  REAL, ALLOCATABLE, DIMENSION(:)  :: InNumbers
  REAL, ALLOCATABLE, DIMENSION(:)  :: OutNumbers
  INTEGER :: I, I2
  REAL :: min_val, max_val

  ALLOCATE(InNumbers(Size))
  ALLOCATE(OutNumbers(Size))

  InitSize = Size

  DO I=1,Size
    READ(StrArray(I),*) InNumbers(I)
  END DO

  min_val = minval(InNumbers)-1
  max_val = maxval(InNumbers)

  Size = 0
  DO WHILE (min_val<max_val)
    Size = Size+1
    min_val = minval(InNumbers, MASK=InNumbers>min_val)
    OutNumbers(Size) = min_val
  END DO

  DO I=1,Size
    WRITE(StrArray(I),'(F0.5)') OutNumbers(I)
    StrArray(I) =  TRIM(StrArray(I))
  END DO

  DO I=1,InitSize
    DO I2=1,Size
      IF (OutNumbers(I2) == InNumbers(I)) THEN
        Order(I) = I2
        EXIT
      END IF
    END DO
  END DO

END SUBROUTINE SortUnique
