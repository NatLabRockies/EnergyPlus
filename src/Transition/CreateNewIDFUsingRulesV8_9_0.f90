MODULE SetVersion

USE DataStringGlobals
USE DataVCompareGlobals

PUBLIC

CONTAINS

SUBROUTINE SetThisVersionVariables()
      VerString='Conversion 8.8 => 8.9'
      VersionNum=8.9
      sVersionNum='8.9'
      IDDFileNameWithPath=TRIM(ProgramPath)//'V8-8-0-Energy+.idd'
      NewIDDFileNameWithPath=TRIM(ProgramPath)//'V8-9-0-Energy+.idd'
      RepVarFileNameWithPath=TRIM(ProgramPath)//'Report Variables 8-8-0 to 8-9-0.csv'
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

  ! For exposed foundation perimeter objects
  INTEGER NumPerimObjs
  INTEGER PArgs
  INTEGER PerimNum
  CHARACTER(len=MaxNameLength*2), ALLOCATABLE, DIMENSION(:) :: PFldNames
  CHARACTER(len=MaxNameLength), ALLOCATABLE, DIMENSION(:) :: PFldDefaults
  CHARACTER(len=20), ALLOCATABLE, DIMENSION(:) :: PFldUnits
  INTEGER PObjMinFlds
  LOGICAL, ALLOCATABLE, DIMENSION(:) :: PAOrN
  LOGICAL, ALLOCATABLE, DIMENSION(:) :: PReqFld
  INTEGER PNumArgs   ! Number of Arguments in a definition
  CHARACTER(len=MaxNameLength), ALLOCATABLE, DIMENSION(:) :: POutArgs

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

  ! for Schedule:Compact from 8.8 to 8.9
  CHARACTER(len=MaxNameLength) ::  UpperInArg=blank

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
                nodiff=.false.

    ! changes for this version

             CASE('ZONEHVAC:EQUIPMENTLIST')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs(1)=InArgs(1)
                 OutArgs(2) = 'SequentialLoad'
                 OutArgs(3:CurArgs+1)=InArgs(2:CurArgs)
                 CurArgs = CurArgs + 1

              CASE('GROUNDHEATEXCHANGER:VERTICAL')
                 nodiff=.false.
                 ObjectName='GroundHeatExchanger:System'
                 ! store the date for later
                 TempArgs=InArgs
                 TempArgsNum=CurArgs

                 ! write the GLHE:System object
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 OutArgs(1:4)=TempArgs(1:4) ! no change
                 OutArgs(5)='Site:GroundTemperature:Undisturbed:KusudaAchenbach'
                 OutArgs(6)=TRIM(TempArgs(1)) // TRIM(' Ground Temps')
                 OutArgs(7:8)=TempArgs(8:9)
                 OutArgs(9)=TRIM(TempArgs(1)) // TRIM(' Response Factors')
                 CurArgs=9
                 CALL WriteOutIDFLines(DifLfn,ObjectName,CurArgs,OutArgs,NwFldNames,NwFldUnits)

                 ! write the GLHE:Props object
                 ObjectName='GroundHeatExchanger:Vertical:Properties'
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 OutArgs(1)=TRIM(TempArgs(1)) // TRIM(' Properties')
                 OutArgs(2)='1'
                 OutArgs(3)=TempArgs(6)
                 !OutArgs(4)=TempArgs(7)
                 READ(TempArgs(7), *) GLHETempVal
                 WRITE(OutArgs(4), *) GLHETempVal * 2
                 OutArgs(5)=TempArgs(11)
                 OutArgs(6)='3.90E+06'
                 OutArgs(7)=TempArgs(12)
                 OutArgs(8)='1.77E+06'
                 OutArgs(9)=TempArgs(13)
                 OutArgs(10)=TempArgs(15)
                 OutArgs(11)=TempArgs(14)
                 CurArgs=11
                 CALL WriteOutIDFLines(DifLfn,ObjectName,CurArgs,OutArgs,NwFldNames,NwFldUnits)

                 ! write the ground temps object
                 ObjectName='Site:GroundTemperature:Undisturbed:KusudaAchenbach'
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 OutArgs(1)=TRIM(TempArgs(1)) // TRIM(' Ground Temps')
                 OutArgs(2)=TempArgs(8)
                 OutArgs(3)='920'
                 GLHETempVal = 0.0
                 READ(TempArgs(9), *) GLHETempVal
                 WRITE(OutArgs(4), *) GLHETempVal / 920
                 OutArgs(5)=TempArgs(10)
                 OutArgs(6)='3.2'
                 OutArgs(7)='8'
                 CurArgs=7
                 CALL WriteOutIDFLines(DifLfn,ObjectName,CurArgs,OutArgs,NwFldNames,NwFldUnits)

                 ObjectName='GroundHeatExchanger:ResponseFactors'
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 OutArgs(1)=TRIM(TempArgs(1)) // TRIM(' Response Factors')
                 OutArgs(2)=TRIM(TempArgs(1)) // TRIM(' Properties')
                 OutArgs(3)=TempArgs(5)
                 OutArgs(4)=TempArgs(17)
                 CurArgs=4
                 I = 0
                 DO WHILE (.TRUE.)
                   I = I + 1
                   CurField = 2*(I-1) + 19
                   IF (CurField > TempArgsNum) EXIT
                   OutArgs(CurArgs + 1)=TempArgs(CurField)
                   OutArgs(CurArgs + 2)=TempArgs(CurField + 1)
                   CurArgs=CurArgs + 2
                 END DO

                 CALL WriteOutIDFLines(DifLfn,ObjectName,CurArgs,OutArgs,NwFldNames,NwFldUnits)

                 ! already written
                 Written = .true.

             CASE('AIRCONDITIONER:VARIABLEREFRIGERANTFLOW')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                 IF (CurArgs .GE. 67) THEN
                   CALL FixFuelTypes(OutArgs(67))
                 END IF

             CASE('BOILER:HOTWATER')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                 IF (CurArgs .GE. 2) THEN
                   CALL FixFuelTypes(OutArgs(2))
                 END IF
                 IF ( (CurArgs .GE. 15) .AND. SameString( InArgs(15), 'VariableFlow' ) ) THEN
                   OutArgs(15) = 'LeavingSetpointModulated'
                 END IF

             CASE('BOILER:STEAM')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                 IF (CurArgs .GE. 2) THEN
                   CALL FixFuelTypes(OutArgs(2))
                 END IF

               CASE('BRANCH')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                 nodiff=.true.
                 ! replace GLHE object type name
                 ! object types are on fields: 3, 7, 11, 15, ...
                 I = 0
                 DO WHILE (.TRUE.)
                   I = I + 1
                   CurField = 4*(I-1) + 3
                   IF ( CurField > CurArgs ) EXIT
                   IF ( SameString( InArgs(CurField), "GroundHeatExchanger:Vertical" ) ) THEN
                     OutArgs(CurField) = "GroundHeatExchanger:System"
                   END IF
                 END DO

             CASE('CHILLER:ELECTRIC:EIR')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                 IF ( (CurArgs .GE. 23) .AND. SameString( InArgs(23), 'VariableFlow' ) ) THEN
                   OutArgs(23) = 'LeavingSetpointModulated'
                 END IF

             CASE('CHILLER:ELECTRIC:REFORMULATEDEIR')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                 IF ( (CurArgs .GE. 22) .AND. SameString( InArgs(22), 'VariableFlow' ) ) THEN
                   OutArgs(22) = 'LeavingSetpointModulated'
                 END IF

             CASE('CHILLER:ELECTRIC')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                 IF ( (CurArgs .GE. 27) .AND. SameString( InArgs(27), 'VariableFlow' ) ) THEN
                   OutArgs(27) = 'LeavingSetpointModulated'
                 END IF

             CASE('CHILLER:ABSORPTION:INDIRECT')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                 IF ( (CurArgs .GE. 16) .AND. SameString( InArgs(16), 'VariableFlow' ) ) THEN
                   OutArgs(16) = 'LeavingSetpointModulated'
                 END IF

             CASE('CHILLER:ABSORPTION')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                 IF ( (CurArgs .GE. 23) .AND. SameString( InArgs(23), 'VariableFlow' ) ) THEN
                   OutArgs(23) = 'LeavingSetpointModulated'
                 END IF

             CASE('CHILLER:CONSTANTCOP')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                 IF ( (CurArgs .GE. 11) .AND. SameString( InArgs(11), 'VariableFlow' ) ) THEN
                   OutArgs(11) = 'LeavingSetpointModulated'
                 END IF

             CASE('CHILLER:ENGINEDRIVEN')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                 IF (CurArgs .GE. 36) THEN
                   CALL FixFuelTypes(OutArgs(36))
                 END IF
                 IF ( (CurArgs .GE. 41) .AND. SameString( InArgs(41), 'VariableFlow' ) ) THEN
                   OutArgs(41) = 'LeavingSetpointModulated'
                 END IF

             CASE('CHILLER:COMBUSTIONTURBINE')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                 IF ( (CurArgs .GE. 54) .AND. (SameString( InArgs(54), 'VariableFlow' )) ) THEN
                   OutArgs(54) = 'LeavingSetpointModulated'
                 END IF
                 IF (CurArgs .GE. 55) THEN
                   CALL FixFuelTypes(OutArgs(55))
                 END IF

             CASE('CHILLERHEATER:ABSORPTION:DIRECTFIRED')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                 IF (CurArgs .GE. 33) THEN
                   CALL FixFuelTypes(OutArgs(33))
                 END IF

             CASE('FUELFACTORS')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                 IF (CurArgs .GE. 1) THEN
                   CALL FixFuelTypes(OutArgs(1))
                   ! For fuelfactors, the current IDD choice is Propane, so override that here until #5941 is resolved (standardize fuel types)
                   IF (SameString(OutArgs(1), "PropaneGas")) OutArgs(1) = "Propane"
                 END IF

             CASE('GENERATOR:COMBUSTIONTURBINE')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                 IF (CurArgs .GE. 22) THEN
                   CALL FixFuelTypes(OutArgs(22))
                 END IF

             CASE('GENERATOR:INTERNALCOMBUSTIONENGINE')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                 IF (CurArgs .GE. 20) THEN
                   CALL FixFuelTypes(OutArgs(20))
                 END IF

             CASE('GENERATOR:MICROTURBINE')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                 IF (CurArgs .GE. 12) THEN
                   CALL FixFuelTypes(OutArgs(12))
                 END IF

             CASE('WATERHEATER:MIXED')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                 IF (CurArgs .GE. 11) THEN
                   CALL FixFuelTypes(OutArgs(11))
                 END IF
                 IF (CurArgs .GE. 15) THEN
                   CALL FixFuelTypes(OutArgs(15))
                 END IF
                 IF (CurArgs .GE. 18) THEN
                   CALL FixFuelTypes(OutArgs(18))
                 END IF

             CASE('WATERHEATER:STRATIFIED')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                 IF (CurArgs .GE. 17) THEN
                   CALL FixFuelTypes(OutArgs(17))
                 END IF
                 IF (CurArgs .GE. 20) THEN
                   CALL FixFuelTypes(OutArgs(20))
                 END IF
                 IF (CurArgs .GE. 24) THEN
                   CALL FixFuelTypes(OutArgs(24))
                 END IF

               CASE('CONDENSEREQUIPMENTLIST')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                 nodiff=.true.
                 ! replace GLHE object type name
                 ! object types are on fields: 2, 4, 6, 8, ...
                 I = 0
                 DO WHILE (.TRUE.)
                   I = I + 1
                   CurField = 2*(I-1) + 2
                   IF ( CurField > CurArgs ) EXIT
                   IF ( SameString( InArgs(CurField), "GroundHeatExchanger:Vertical" ) ) THEN
                     OutArgs(CurField) = "GroundHeatExchanger:System"
                   END IF
                 END DO

             CASE('ELECTRICEQUIPMENT:ITE:AIRCOOLED')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs(1:2)=InArgs(1:2)
                 OutArgs(3) = 'FlowFromSystem'
                 OutArgs(4:CurArgs+1)=InArgs(3:CurArgs)
                 CurArgs = CurArgs + 1

             CASE('HEATBALANCEALGORITHM')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                 IF (CurArgs .GE. 1) THEN
                   IF (SameString( InArgs(1), 'Default' )) THEN
                     OutArgs(1) = 'ConductionTransferFunction'
                   ELSE IF (SameString( InArgs(1), 'CTF' )) THEN
                     OutArgs(1) = 'ConductionTransferFunction'
                   ELSE IF (SameString( InArgs(1), 'EMPD' )) THEN
                     OutArgs(1) = 'MoisturePenetrationDepthConductionTransferFunction'
                   ELSE IF (SameString( InArgs(1), 'CondFD' )) THEN
                     OutArgs(1) = 'ConductionFiniteDifference'
                   ELSE IF (SameString( InArgs(1), 'CONDUCTIONFINITEDIFFERENCEDETAILED' )) THEN
                     OutArgs(1) = 'ConductionFiniteDifference'
                   ELSE IF (SameString( InArgs(1), 'HAMT' )) THEN
                     OutArgs(1) = 'CombinedHeatAndMoistureFiniteElement'
                   END IF
                 END IF

             CASE('OUTPUT:CONSTRUCTIONS')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                 IF (CurArgs .GE. 1) THEN
                   IF (SameString( InArgs(1)(1:3), 'Con' )) THEN
                     OutArgs(1) = 'Constructions'
                   ELSE IF (SameString( InArgs(1)(1:3), 'Mat' )) THEN
                     OutArgs(1) = 'Materials'
                   END IF
                 END IF
                 IF (CurArgs .GE. 2) THEN
                   IF (SameString( InArgs(2)(1:3), 'Con' )) THEN
                     OutArgs(2) = 'Constructions'
                   ELSE IF (SameString( InArgs(2)(1:3), 'Mat' )) THEN
                     OutArgs(2) = 'Materials'
                   END IF
                 END IF

             CASE('SCHEDULE:DAY:INTERVAL')
                 ObjectName='Schedule:Day:Interval'
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs=InArgs
                 IF (SameString(InArgs(3), 'YES')) THEN
                   OutArgs(3) = 'Average'
                 ENDIF

             CASE('SCHEDULE:DAY:LIST')
                 ObjectName='Schedule:Day:List'
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs=InArgs
                 IF (SameString(InArgs(3), 'YES')) THEN
                   OutArgs(3) = 'Average'
                 ENDIF

             CASE('SCHEDULE:COMPACT')
                 ObjectName='Schedule:Compact'
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs=InArgs
                 DO Arg=3,CurArgs
                    UpperInArg = MakeUpperCase(InArgs(Arg))
                    IF ( ( INDEX(UpperInArg,"INTERPOLATE") .GT. 0 ).AND. (INDEX(UpperInArg,"YES") .GT. 0 ) ) THEN
                      OutArgs(Arg) = "Interpolate:Average"
                    ENDIF
                 ENDDO

             CASE('SIZINGPERIOD:DESIGNDAY')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                 IF (CurArgs .GE. 18) THEN
                   IF (SameString( InArgs(18), '0' )) THEN
                     OutArgs(18) = 'No'
                   ELSE IF (SameString( InArgs(18), '1' )) THEN
                     OutArgs(18) = 'Yes'
                   END IF
                 END IF
                 IF (CurArgs .GE. 19) THEN
                   IF (SameString( InArgs(19), '0' )) THEN
                     OutArgs(19) = 'No'
                   ELSE IF (SameString( InArgs(19), '1' )) THEN
                     OutArgs(19) = 'Yes'
                   END IF
                 END IF
                 IF (CurArgs .GE. 20) THEN
                   IF (SameString( InArgs(20), '0' )) THEN
                     OutArgs(20) = 'No'
                   ELSE IF (SameString( InArgs(20), '1' )) THEN
                     OutArgs(20) = 'Yes'
                   END IF
                 END IF

             CASE('GENERATOR:WINDTURBINE')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                 IF (CurArgs .GE. 3) THEN
                   IF (SameString( InArgs(3), 'HAWT' )) THEN
                     OutArgs(3) = 'HorizontalAxisWindTurbine'
                   ELSE IF (SameString( InArgs(3), 'None' )) THEN
                     OutArgs(3) = 'HorizontalAxisWindTurbine'
                   ELSE IF (SameString( InArgs(3), 'VAWT' )) THEN
                     OutArgs(3) = 'VerticalAxisWindTurbine'
                   END IF
                 END IF
                 IF (CurArgs .GE. 4) THEN
                   IF (SameString( InArgs(4), 'FSFP' )) THEN
                     OutArgs(4) = 'FixedSpeedFixedPitch'
                   ELSE IF (SameString( InArgs(4), 'FSVP' )) THEN
                     OutArgs(4) = 'FixedSpeedVariablePitch'
                   ELSE IF (SameString( InArgs(4), 'VSFP' )) THEN
                     OutArgs(4) = 'VariableSpeedFixedPitch'
                   ELSE IF (SameString( InArgs(4), 'VSVP' )) THEN
                     OutArgs(4) = 'VariableSpeedVariablePitch'
                   ELSE IF (SameString( InArgs(4), 'None' )) THEN
                     OutArgs(4) = 'VariableSpeedVariablePitch'
                   END IF
                 END IF

             CASE('ZONE')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                 IF (CurArgs .GE. 12) THEN
                   IF (SameString( InArgs(12), 'DOE2' )) THEN
                     OutArgs(12) = 'DOE-2'
                   END IF
                 END IF

             CASE('ZONEAIRHEATBALANCEALGORITHM')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 nodiff=.false.
                 OutArgs(1:CurArgs)=InArgs(1:CurArgs)
                 IF (CurArgs .GE. 1) THEN
                   IF (SameString( InArgs(1), '3RDORDERBACKWARDDIFFERENCE' )) THEN
                     OutArgs(1) = 'ThirdOrderBackwardDifference'
                   END IF
                 END IF

             CASE('OUTPUT:TABLE:SUMMARYREPORTS')
                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                 CurField = 0
                 DO TempArgsNum=1,CurArgs,1
                   ! Skip blank fields
                   IF (.NOT. InArgs(TempArgsNum) == Blank) THEN
                     CurField = CurField + 1
                     OutArgs(CurField) = InArgs(TempArgsNum)
                   END IF
                 END DO
                 IF (CurField < CurArgs) THEN
                   Nodiff=.false.
                   CurArgs = CurField
                 END IF

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

SUBROUTINE FixFuelTypes(InOutArg)
  USE InputProcessor, ONLY: SameString
  CHARACTER(len=*), INTENT(INOUT) :: InOutArg

  IF (SameString( InOutArg, 'Electric' )) THEN
    InOutArg = 'Electricity'
  ELSE IF (SameString( InOutArg, 'Elec' )) THEN
    InOutArg = 'Electricity'
  ELSE IF (SameString( InOutArg, 'Gas' )) THEN
    InOutArg = 'NaturalGas'
  ELSE IF (SameString( InOutArg, 'Natural Gas' )) THEN
    InOutArg = 'NaturalGas'
  ELSE IF (SameString( InOutArg, 'Propane' )) THEN
    InOutArg = 'PropaneGas'
  ELSE IF (SameString( InOutArg, 'LPG' )) THEN
    InOutArg = 'PropaneGas'
  ELSE IF (SameString( InOutArg, 'Propane Gas' )) THEN
    InOutArg = 'PropaneGas'
  ELSE IF (SameString( InOutArg, 'FUEL OIL #1' )) THEN
    InOutArg = 'FuelOil#1'
  ELSE IF (SameString( InOutArg, 'FUEL OIL' )) THEN
    InOutArg = 'FuelOil#1'
  ELSE IF (SameString( InOutArg, 'DISTILLATE OIL' )) THEN
    InOutArg = 'FuelOil#1'
  ELSE IF (SameString( InOutArg, 'DISTILLATEOIL' )) THEN
    InOutArg = 'FuelOil#1'
  ELSE IF (SameString( InOutArg, 'FUEL OIL #2' )) THEN
    InOutArg = 'FuelOil#2'
  ELSE IF (SameString( InOutArg, 'RESIDUAL OIL' )) THEN
    InOutArg = 'FuelOil#2'
  ELSE IF (SameString( InOutArg, 'RESIDUALOIL' )) THEN
    InOutArg = 'FuelOil#2'
  END IF
END SUBROUTINE
