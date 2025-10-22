MODULE SetVersion

USE DataStringGlobals
USE DataVCompareGlobals

PUBLIC

CONTAINS

SUBROUTINE SetThisVersionVariables()
      VerString='Conversion 8.9 => 9.0'
      VersionNum=9.0
      sVersionNum='9.0'
      IDDFileNameWithPath=TRIM(ProgramPath)//'V8-9-0-Energy+.idd'
      NewIDDFileNameWithPath=TRIM(ProgramPath)//'V9-0-0-Energy+.idd'
      RepVarFileNameWithPath=TRIM(ProgramPath)//'Report Variables 8-9-0 to 9-0-0.csv'
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

  ! new variables for WINDOWPROPERTY:SHADINGCONTROL from 8.9 to 9.0
  INTEGER :: TotWinObjs = 0
  INTEGER :: TotObjsWithShadeCtrl = 0
  INTEGER :: TotBaseSurfObjs = 0
  TYPE FenestrationSurf
    CHARACTER(len=MaxObjectNameLength) :: FenSurfName =' ' ! Fenestration surface name
    CHARACTER(len=MaxObjectNameLength) :: FenBaseSurfName =' ' ! Base surface name
    CHARACTER(len=MaxObjectNameLength) :: FenZoneName =' ' ! Zone name
    CHARACTER(len=MaxObjectNameLength) :: FenShadeControlName =' ' ! Original WindowProperty:ShadingControl name
  END TYPE
  TYPE(FenestrationSurf), ALLOCATABLE, DIMENSION(:) :: FenSurf
  INTEGER :: surfNum = 0
  INTEGER :: baseSurfNum = 0
  LOGICAL :: baseSurfFound = .false.

  TYPE ShadeControl
    CHARACTER(len=MaxObjectNameLength) :: OldShadeControlName =' ' ! Original WindowProperty:ShadingControl name
    INTEGER :: NumZones = 0 ! Number of zones for this old shade control name
    CHARACTER(len=MaxObjectNameLength), ALLOCATABLE, DIMENSION(:) :: ShadeControlZoneName ! New WindowShadingControl zone names
    INTEGER , ALLOCATABLE, DIMENSION(:) :: SequenceNum ! New WindowShadingControl sequence numbers
  END TYPE
  TYPE(ShadeControl), ALLOCATABLE, DIMENSION(:) :: ShadeControls
  INTEGER :: TotOldShadeControls = 0
  INTEGER :: shadeCtrlNum = 0
  INTEGER :: NumZones = 0
  INTEGER :: newZoneNum = 0
  INTEGER :: zoneNum = 0
  INTEGER :: seqCount = 0
  CHARACTER(len=MaxObjectNameLength) :: curOldShadeName = ' '
  CHARACTER(len=MaxObjectNameLength) :: prevOldShadeName = ' '
  LOGICAL :: zoneFound = .false.
  INTEGER :: daylightNum = 0
  ! end new variables for WINDOWPROPERTY:SHADINGCONTROL from 8.9 to 9.0


  ! For run period transitions
  TYPE FieldFlagAndValue
    LOGICAL :: wasSet
    CHARACTER(len=MaxNameLength) :: originalValue
  END TYPE FieldFlagAndValue
  TYPE (FieldFlagAndValue) :: RunPeriodStartYear
  TYPE (FieldFlagAndValue) :: RunPeriodRepeated
  INTEGER :: BeginMonthNumber, BeginDayNumber, BeginYearNumber, EndMonthNumber, EndDayNumber, EndYearNumber, RepeatedCount
  INTEGER, EXTERNAL :: GetYearFromStartDayString
  LOGICAL, EXTERNAL :: IsYearNumberALeapYear
  INTEGER, EXTERNAL :: GetLeapYearFromStartDayString
  INTEGER, EXTERNAL :: FindYearForWeekDay

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

     ! Begin Pre-process fenestration surfaces for transition from WindowProperty:ShadingControl to WindowShadingControl
          ! Clean up from any previous passes, then re-allocate
          IF(ALLOCATED(FenSurf)) DEALLOCATE(FenSurf)
          IF(ALLOCATED(ShadeControls)) DEALLOCATE(ShadeControls)
          TotWinObjs = GetNumObjectsFound('FENESTRATIONSURFACE:DETAILED') + GetNumObjectsFound('WINDOW') + GetNumObjectsFound('GLAZEDDOOR')
          TotBaseSurfObjs = GetNumObjectsFound('BUILDINGSURFACE:DETAILED') + GetNumObjectsFound('WALL:DETAILED') + GetNumObjectsFound('ROOFCEILING:DETAILED') + GetNumObjectsFound('FLOOR:DETAILED')
          TotBaseSurfObjs = TotBaseSurfObjs + GetNumObjectsFound('WALL:EXTERIOR') + GetNumObjectsFound('WALL:ADIABATIC') + GetNumObjectsFound('WALL:UNDERGROUND') + GetNumObjectsFound('WALL:INTERZONE')
          TotBaseSurfObjs = TotBaseSurfObjs + GetNumObjectsFound('ROOF') + GetNumObjectsFound('CEILING:ADIABATIC') + GetNumObjectsFound('CEILING:INTERZONE')
          TotBaseSurfObjs = TotBaseSurfObjs + GetNumObjectsFound('FLOOR:GROUNDCONTACT') + GetNumObjectsFound('FLOOR:ADIABATIC') + GetNumObjectsFound('FLOOR:INTERZONE')
          ALLOCATE(FenSurf(TotWinObjs))

          ! Loop through all objects that might have a window shading control assigned and make a list
          TotObjsWithShadeCtrl = 0
          CALL DisplayString('Processing IDF -- WindowShadingControl preprocessing . . .')
          DO surfNum=1,GetNumObjectsFound('FENESTRATIONSURFACE:DETAILED')
            CALL GetObjectItem('FENESTRATIONSURFACE:DETAILED',surfNum,Alphas,NumAlphas,Numbers,NumNumbers,Status)
            IF (Alphas(6) .NE. Blank) THEN
              TotObjsWithShadeCtrl = TotObjsWithShadeCtrl + 1
              FenSurf(TotObjsWithShadeCtrl)%FenSurfName = TRIM(Alphas(1))
              FenSurf(TotObjsWithShadeCtrl)%FenBaseSurfName = TRIM(Alphas(4))
              FenSurf(TotObjsWithShadeCtrl)%FenShadeControlName = TRIM(Alphas(6))
            ENDIF
          ENDDO
          DO surfNum=1,GetNumObjectsFound('WINDOW')
            CALL GetObjectItem('WINDOW',surfNum,Alphas,NumAlphas,Numbers,NumNumbers,Status)
            IF (Alphas(4) .NE. Blank) THEN
              TotObjsWithShadeCtrl = TotObjsWithShadeCtrl + 1
              FenSurf(TotObjsWithShadeCtrl)%FenSurfName = TRIM(Alphas(1))
              FenSurf(TotObjsWithShadeCtrl)%FenBaseSurfName = TRIM(Alphas(3))
              FenSurf(TotObjsWithShadeCtrl)%FenShadeControlName = TRIM(Alphas(4))
            ENDIF
          ENDDO
          DO surfNum=1,GetNumObjectsFound('GLAZEDOOR')
            CALL GetObjectItem('GLAZEDOOR',surfNum,Alphas,NumAlphas,Numbers,NumNumbers,Status)
            IF (Alphas(4) .NE. Blank) THEN
              TotObjsWithShadeCtrl = TotObjsWithShadeCtrl + 1
              FenSurf(TotObjsWithShadeCtrl)%FenSurfName = TRIM(Alphas(1))
              FenSurf(TotObjsWithShadeCtrl)%FenBaseSurfName = TRIM(Alphas(3))
              FenSurf(TotObjsWithShadeCtrl)%FenShadeControlName = TRIM(Alphas(4))
            ENDIF
          ENDDO

          ! Loop through all possible base surfaces then loop through all objects with a window shading control assigned and find out which zone they are in
          DO baseSurfNum=1,GetNumObjectsFound('BUILDINGSURFACE:DETAILED')
            CALL GetObjectItem('BUILDINGSURFACE:DETAILED',baseSurfNum,Alphas,NumAlphas,Numbers,NumNumbers,Status)
            DO surfNum=1,TotObjsWithShadeCtrl
              IF (SameString(FenSurf(surfNum)%FenBaseSurfName,Alphas(1))) FenSurf(surfNum)%FenZoneName = TRIM(Alphas(4))
            ENDDO
          ENDDO
          DO baseSurfNum=1,GetNumObjectsFound('WALL:DETAILED')
            CALL GetObjectItem('WALL:DETAILED',baseSurfNum,Alphas,NumAlphas,Numbers,NumNumbers,Status)
            DO surfNum=1,TotObjsWithShadeCtrl
              IF (SameString(FenSurf(surfNum)%FenBaseSurfName,Alphas(1))) FenSurf(surfNum)%FenZoneName = TRIM(Alphas(3))
            ENDDO
          ENDDO
          DO baseSurfNum=1,GetNumObjectsFound('ROOFCEILING:DETAILED')
            CALL GetObjectItem('ROOFCEILING:DETAILED',baseSurfNum,Alphas,NumAlphas,Numbers,NumNumbers,Status)
            DO surfNum=1,TotObjsWithShadeCtrl
              IF (SameString(FenSurf(surfNum)%FenBaseSurfName,Alphas(1))) FenSurf(surfNum)%FenZoneName = TRIM(Alphas(3))
            ENDDO
          ENDDO
          DO baseSurfNum=1,GetNumObjectsFound('FLOOR:DETAILED')
            CALL GetObjectItem('FLOOR:DETAILED',baseSurfNum,Alphas,NumAlphas,Numbers,NumNumbers,Status)
            DO surfNum=1,TotObjsWithShadeCtrl
              IF (SameString(FenSurf(surfNum)%FenBaseSurfName,Alphas(1))) FenSurf(surfNum)%FenZoneName = TRIM(Alphas(3))
            ENDDO
          ENDDO
          DO baseSurfNum=1,GetNumObjectsFound('WALL:EXTERIOR')
            CALL GetObjectItem('WALL:EXTERIOR',baseSurfNum,Alphas,NumAlphas,Numbers,NumNumbers,Status)
            DO surfNum=1,TotObjsWithShadeCtrl
              IF (SameString(FenSurf(surfNum)%FenBaseSurfName,Alphas(1))) FenSurf(surfNum)%FenZoneName = TRIM(Alphas(3))
            ENDDO
          ENDDO
          DO baseSurfNum=1,GetNumObjectsFound('WALL:ADIABATIC')
            CALL GetObjectItem('WALL:ADIABATIC',baseSurfNum,Alphas,NumAlphas,Numbers,NumNumbers,Status)
            DO surfNum=1,TotObjsWithShadeCtrl
              IF (SameString(FenSurf(surfNum)%FenBaseSurfName,Alphas(1))) FenSurf(surfNum)%FenZoneName = TRIM(Alphas(3))
            ENDDO
          ENDDO
          DO baseSurfNum=1,GetNumObjectsFound('WALL:UNDERGROUND')
            CALL GetObjectItem('WALL:UNDERGROUND',baseSurfNum,Alphas,NumAlphas,Numbers,NumNumbers,Status)
            DO surfNum=1,TotObjsWithShadeCtrl
              IF (SameString(FenSurf(surfNum)%FenBaseSurfName,Alphas(1))) FenSurf(surfNum)%FenZoneName = TRIM(Alphas(3))
            ENDDO
          ENDDO
          DO baseSurfNum=1,GetNumObjectsFound('WALL:INTERZONE')
            CALL GetObjectItem('WALL:INTERZONE',baseSurfNum,Alphas,NumAlphas,Numbers,NumNumbers,Status)
            DO surfNum=1,TotObjsWithShadeCtrl
              IF (SameString(FenSurf(surfNum)%FenBaseSurfName,Alphas(1))) FenSurf(surfNum)%FenZoneName = TRIM(Alphas(3))
            ENDDO
          ENDDO
          DO baseSurfNum=1,GetNumObjectsFound('ROOF')
            CALL GetObjectItem('ROOF',baseSurfNum,Alphas,NumAlphas,Numbers,NumNumbers,Status)
            DO surfNum=1,TotObjsWithShadeCtrl
              IF (SameString(FenSurf(surfNum)%FenBaseSurfName,Alphas(1))) FenSurf(surfNum)%FenZoneName = TRIM(Alphas(3))
            ENDDO
          ENDDO
          DO baseSurfNum=1,GetNumObjectsFound('CEILING:ADIABATIC')
            CALL GetObjectItem('CEILING:ADIABATIC',baseSurfNum,Alphas,NumAlphas,Numbers,NumNumbers,Status)
            DO surfNum=1,TotObjsWithShadeCtrl
              IF (SameString(FenSurf(surfNum)%FenBaseSurfName,Alphas(1))) FenSurf(surfNum)%FenZoneName = TRIM(Alphas(3))
            ENDDO
          ENDDO
          DO baseSurfNum=1,GetNumObjectsFound('CEILING:INTERZONE')
            CALL GetObjectItem('CEILING:INTERZONE',baseSurfNum,Alphas,NumAlphas,Numbers,NumNumbers,Status)
            DO surfNum=1,TotObjsWithShadeCtrl
              IF (SameString(FenSurf(surfNum)%FenBaseSurfName,Alphas(1))) FenSurf(surfNum)%FenZoneName = TRIM(Alphas(3))
            ENDDO
          ENDDO
          DO baseSurfNum=1,GetNumObjectsFound('FLOOR:GROUNDCONTACT')
            CALL GetObjectItem('FLOOR:GROUNDCONTACT',baseSurfNum,Alphas,NumAlphas,Numbers,NumNumbers,Status)
            DO surfNum=1,TotObjsWithShadeCtrl
              IF (SameString(FenSurf(surfNum)%FenBaseSurfName,Alphas(1))) FenSurf(surfNum)%FenZoneName = TRIM(Alphas(3))
            ENDDO
          ENDDO
          DO baseSurfNum=1,GetNumObjectsFound('FLOOR:ADIABATIC')
            CALL GetObjectItem('FLOOR:ADIABATIC',baseSurfNum,Alphas,NumAlphas,Numbers,NumNumbers,Status)
            DO surfNum=1,TotObjsWithShadeCtrl
              IF (SameString(FenSurf(surfNum)%FenBaseSurfName,Alphas(1))) FenSurf(surfNum)%FenZoneName = TRIM(Alphas(3))
            ENDDO
          ENDDO
          DO baseSurfNum=1,GetNumObjectsFound('FLOOR:INTERZONE')
            CALL GetObjectItem('FLOOR:INTERZONE',baseSurfNum,Alphas,NumAlphas,Numbers,NumNumbers,Status)
            DO surfNum=1,TotObjsWithShadeCtrl
              IF (SameString(FenSurf(surfNum)%FenBaseSurfName,Alphas(1))) FenSurf(surfNum)%FenZoneName = TRIM(Alphas(3))
            ENDDO
          ENDDO

          ! Now build list of new WindowShadingControl names for each old WindowProperty:ShadingControl
          TotOldShadeControls = GetNumObjectsFound('WINDOWPROPERTY:SHADINGCONTROL')
          NumZones = GetNumObjectsFound('ZONE')
          ALLOCATE(ShadeControls(TotOldShadeControls))
          DO shadeCtrlNum=1,TotOldShadeControls
            ALLOCATE(ShadeControls(shadeCtrlNum)%ShadeControlZoneName(NumZones))
            ALLOCATE(ShadeControls(shadeCtrlNum)%SequenceNum(NumZones))
            CALL GetObjectItem('WINDOWPROPERTY:SHADINGCONTROL',shadeCtrlNum,Alphas,NumAlphas,Numbers,NumNumbers,Status)
            ShadeControls(shadeCtrlNum)%OldShadeControlName = TRIM(Alphas(1))
            ShadeControls(shadeCtrlNum)%NumZones = 0
            DO surfNum=1,TotObjsWithShadeCtrl
              zoneFound = .false.
              IF (.not. SameString(FenSurf(surfNum)%FenShadeControlName,ShadeControls(shadeCtrlNum)%OldShadeControlName)) CYCLE
              DO newZoneNum=1,ShadeControls(shadeCtrlNum)%NumZones
                IF (.not. SameString(FenSurf(surfNum)%FenZoneName,ShadeControls(shadeCtrlNum)%ShadeControlZoneName(newZoneNum))) CYCLE
                zoneFound = .true.
                EXIT
              ENDDO
              IF (zoneFound) CYCLE
              ShadeControls(shadeCtrlNum)%NumZones = ShadeControls(shadeCtrlNum)%NumZones + 1
              ShadeControls(shadeCtrlNum)%ShadeControlZoneName(ShadeControls(shadeCtrlNum)%NumZones) = FenSurf(surfNum)%FenZoneName
              ShadeControls(shadeCtrlNum)%SequenceNum(newZoneNum) = 0
            ENDDO
          ENDDO
          ! Now set new WindowShadingControl sequence numbers - this isn't right yet - should be driven by surface order, not shading control order, but maybe doesn't matter
          DO zoneNum=1,NumZones
            seqCount = 0
            CALL GetObjectItem('ZONE',zoneNum,Alphas,NumAlphas,Numbers,NumNumbers,Status)
            DO shadeCtrlNum=1,TotOldShadeControls
              DO newZoneNum=1,ShadeControls(shadeCtrlNum)%NumZones
                IF (SameString(Alphas(1),ShadeControls(shadeCtrlNum)%ShadeControlZoneName(newZoneNum))) THEN
                  seqCount = seqCount + 1
                  ShadeControls(shadeCtrlNum)%SequenceNum(newZoneNum) = seqCount
                ENDIF
              ENDDO
            ENDDO
          ENDDO
          CALL DisplayString('Processing IDF -- WindowShadingControl preprocessing complete.')
     ! End Pre-process fenestration surfaces for transition from WindowProperty:ShadingControl to WindowShadingControl

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
                nodiff=.false.

    ! changes for this version, pick one of the spots to add rules, this will reduce the possibility of merge conflicts

!             CASE('OBJECTNAMEHERE')
!                 CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
!                 nodiff=.false.
!                 OutArgs(1)=InArgs(1)
!                 OutArgs(2) = 'SequentialLoad'
!                 OutArgs(3:CurArgs+1)=InArgs(2:CurArgs)
!                 CurArgs = CurArgs + 1

              ! If your original object starts with A, insert the rules here
              CASE('AIRFLOWNETWORK:DISTRIBUTION:COMPONENT:OUTDOORAIRFLOW')
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                OutArgs(1) = InArgs(1)
                ! Trusting that a previously working file only has one outdoor air mixer
                CALL GetObjectItem('OUTDOORAIR:MIXER',1,Alphas,NumAlphas,Numbers,NumNumbers,Status)
                OutArgs(2) = Alphas(1) ! Outdoor Air Mixer Name
                OutArgs(3:CurArgs+1) = InArgs(2:CurArgs)
                CurArgs = CurArgs + 1
                nodiff = .false.

              CASE('AIRFLOWNETWORK:DISTRIBUTION:COMPONENT:RELIEFAIRFLOW')
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                OutArgs(1) = InArgs(1)
                ! Trusting that a previously working file only has one outdoor air mixer
                CALL GetObjectItem('OUTDOORAIR:MIXER',1,Alphas,NumAlphas,Numbers,NumNumbers,Status)
                OutArgs(2) = Alphas(1) ! Outdoor Air Mixer Name
                OutArgs(3:CurArgs+1) = InArgs(2:CurArgs)
                CurArgs = CurArgs + 1
                nodiff = .false.

              ! If your original object starts with B, insert the rules here
              CASE('BOILER:HOTWATER')
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                OutArgs(1:6) = InArgs(1:6)
                OutArgs(7:CurArgs-1) = InArgs(8:CurArgs)
                CurArgs = CurArgs - 1
                nodiff = .false.

              ! If your original object starts with C, insert the rules here

              ! If your original object starts with D, insert the rules here

              ! If your original object starts with E, insert the rules here

              ! If your original object starts with F, insert the rules here

              CASE('FENESTRATIONSURFACE:DETAILED')
                ! Delete field 7 (A6) Shading Control Name
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                OutArgs(1:6) = InArgs(1:6)
                OutArgs(7:CurArgs-1) = InArgs(8:CurArgs)
                CurArgs = CurArgs - 1
                nodiff = .false.

              ! If your original object starts with G, insert the rules here

              CASE('GLAZEDDOOR')
                ! Delete field 4 (A4) Shading Control Name
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                OutArgs(1:3) = InArgs(1:3)
                OutArgs(4:CurArgs-1) = InArgs(5:CurArgs)
                CurArgs = CurArgs - 1
                nodiff = .false.

              ! If your original object starts with H, insert the rules here

              ! If your original object starts with I, insert the rules here

              ! If your original object starts with L, insert the rules here

              ! If your original object starts with M, insert the rules here

              ! If your original object starts with N, insert the rules here

              ! If your original object starts with O, insert the rules here

              ! If your original object starts with P, insert the rules here

              ! If your original object starts with R, insert the rules here
              CASE('RUNPERIOD:CUSTOMRANGE')
                ! Just change the type to RunPeriod and copy all of the arguments
                ObjectName = 'RunPeriod'
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                OutArgs(1:CurArgs) = InArgs(1:CurArgs)
                IF (SameString(TRIM(OutArgs(8)), "USEWEATHERFILE")) THEN
                    CALL ShowWarningError('Run period start day of week USEWEATHERFILE option has been removed, start week day is set by the input start date.',Auditf)
                    OutArgs(8) = Blank
                END IF
                CALL WriteOutIDFLines(DifLfn,ObjectName,CurArgs,OutArgs,NwFldNames,NwFldUnits)
                Written=.true.
                nodiff = .false.

              CASE('RUNPERIOD')
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                ! spend some time mining out the state of the run period object
                RunPeriodStartYear%wasSet = .FALSE.
                BeginYearNumber = 0
                IF (CurArgs >= 14) THEN
                  IF (TRIM(InArgs(14)) /= Blank) THEN
                    RunPeriodStartYear%wasSet = .TRUE.
                    RunPeriodStartYear%originalValue = InArgs(14)
                    BeginYearNumber = ProcessNumber(InArgs(14),ErrFlag)
                    IF (ErrFlag) THEN
                      CALL ShowSevereError('Invalid Number, RunPeriod=' // TRIM(InArgs(1)) // ', field ' // TRIM(FldNames(14)) // '=' // TRIM(InArgs(14)),Auditf)
                    ENDIF
                  END IF
                END IF
                RunPeriodRepeated%wasSet = .FALSE.
                RepeatedCount = 0
                IF (CurArgs >= 12) THEN
                  IF (TRIM(InArgs(12)) /= Blank) THEN
                    RunPeriodRepeated%originalValue = InArgs(12)
                    RepeatedCount = ProcessNumber(InArgs(12),ErrFlag)
                    IF (ErrFlag) THEN
                      CALL ShowSevereError('Invalid Number, RunPeriod=' // TRIM(InArgs(1)) // ', field ' // TRIM(FldNames(12)) // '=' // TRIM(InArgs(12)),Auditf)
                    ENDIF
                    IF (RepeatedCount > 1) THEN
                      RunPeriodRepeated%wasSet = .TRUE.
                    ENDIF
                  END IF
                END IF
                ! Now start writing some object data out
                ! Name, BeginMonth and BeginDay are the same
                OutArgs(1:3) = InArgs(1:3)
                ! Start year is weird
                IF (RunPeriodStartYear%wasSet) THEN
                  OutArgs(4) = RoundSigDigits(BeginYearNumber,0)
                ELSE IF (RunPeriodRepeated%wasSet) THEN
                  BeginMonthNumber = ProcessNumber(InArgs(2),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, RunPeriod=' // TRIM(InArgs(1)) // ', field ' // TRIM(FldNames(2)) // '=' // TRIM(InArgs(2)),Auditf)
                  ENDIF
                  BeginDayNumber = ProcessNumber(InArgs(3),ErrFlag)
                  IF (ErrFlag) THEN
                    CALL ShowSevereError('Invalid Number, RunPeriod=' // TRIM(InArgs(1)) // ', field ' // TRIM(FldNames(3)) // '=' // TRIM(InArgs(3)),Auditf)
                  ENDIF
                  IF (TRIM(InArgs(6)) /= Blank) THEN
                    BeginYearNumber = FindYearForWeekDay(BeginMonthNumber, BeginDayNumber, InArgs(6))
                  ELSE
                    BeginYearNumber = FindYearForWeekDay(BeginMonthNumber, BeginDayNumber, "SUNDAY")
                  END IF
                  OutArgs(4) = RoundSigDigits(BeginYearNumber, 0)
                ELSE
                  OutArgs(4) = Blank
                END IF
                ! End month and End day or month are the same, just need to shift one field
                OutArgs(5:6) = InArgs(4:5)
                ! End year is weird
                IF (RunPeriodRepeated%wasSet) THEN
                  ! What if start year turns out to be blank above?
                  IF (TRIM(OutArgs(4)) == Blank) THEN
                    OutArgs(7) = Blank
                  ELSE
                    ! RepeatedCount was set earlier
                    EndYearNumber = BeginYearNumber + RepeatedCount
                    OutArgs(7) = RoundSigDigits( EndYearNumber, 0)

                    EndMonthNumber = ProcessNumber(InArgs(4),ErrFlag)
                    IF (ErrFlag) THEN
                      CALL ShowSevereError('Invalid Number, RunPeriod=' // TRIM(InArgs(1)) // ', field ' // TRIM(FldNames(4)) // '=' // TRIM(InArgs(4)),Auditf)
                    ENDIF
                    EndDayNumber = ProcessNumber(InArgs(5),ErrFlag)
                    IF (ErrFlag) THEN
                      CALL ShowSevereError('Invalid Number, RunPeriod=' // TRIM(InArgs(1)) // ', field ' // TRIM(FldNames(5)) // '=' // TRIM(InArgs(5)),Auditf)
                    ENDIF
                    IF (EndMonthNumber==2 .AND. EndDayNumber==29) THEN
                      ! We should have a leap year end year
                      IF (.NOT.IsYearNumberALeapYear(EndYearNumber)) THEN
                        ! Warning about bad end year/end date combination
                        OutArgs(6) = "28"
                      END IF
                    END IF
                  END IF
                ELSE
                  OutArgs(7) = Blank
                END IF
                ! Start day of week is also weird
                IF (RunPeriodStartYear%wasSet) THEN
                  ! Throw warning saying the start of the week has been specified by the year
                  OutArgs(8) = Blank  ! But why is this field even staying?
                ELSE
                  IF (SameString(TRIM(InArgs(6)), "USEWEATHERFILE")) THEN
                    CALL ShowWarningError('Run period start day of week USEWEATHERFILE option has been removed, start week day is set by the input start date.',Auditf)
                    OutArgs(8) = BLANK
                  ELSE
                    ! Copy it over unchanged?
                    OutArgs(8) = InArgs(6)
                  END IF
                END IF
                ! Remaining fields are mostly straightforward
                OutArgs(9)  = InArgs(7)  ! Use Weather File Holidays
                OutArgs(10) = InArgs(8)  ! Use Weather File DST
                OutArgs(11) = InArgs(9)  ! Apply Weekend Holiday Rule
                OutArgs(12) = InArgs(10) ! Use Weather File Rain
                OutArgs(13) = InArgs(11) ! Use Weather File Snow
                ! InArgs(12): Eliminate number of times to repeat runperiod
                IF (TRIM(InArgs(13))=="YES") THEN
                  ! Issue warning about incrementing day of week on repeat...
                END IF
                ! InArgs(14): Start year moved to above
                OutArgs(14) = Blank ! new Treat weather as actual field?
                CurArgs = 14
                nodiff = .false.

              ! If your original object starts with S, insert the rules here

              ! If your original object starts with T, insert the rules here
              CASE('TABLE:ONEINDEPENDENTVARIABLE')
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                OutArgs(1:CurArgs) = InArgs(1:CurArgs)
                IF (SameString(InArgs(2),'EXPONENT')) THEN
                  OutArgs(2) = Blank
                  nodiff=.false.
                ENDIF

              ! If your original object starts with U, insert the rules here

              ! If your original object starts with V, insert the rules here

              ! If your original object starts with W, insert the rules here
              CASE ('WINDOWMATERIAL:COMPLEXSHADE')
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                OutArgs = InArgs
                IF (SameString(InArgs(2), "VENETIAN")) THEN
                  OutArgs(2) = "VenetianHorizontal"
                  nodiff = .FALSE.
                END IF

              CASE('WINDOW')
                ! Delete field 4 (A4) Shading Control Name
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                OutArgs(1:3) = InArgs(1:3)
                OutArgs(4:CurArgs-1) = InArgs(5:CurArgs)
                CurArgs = CurArgs - 1
                nodiff = .false.

              CASE('WINDOWPROPERTY:SHADINGCONTROL')
                ! lots of stuff . . . for transition, make a separate new WindowShadingControl object for each zone it is used in
                ObjectName = 'WindowShadingControl'
                CALL GetNewObjectDefInIDD(ObjectName,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)
                OutArgs(4:CurArgs+2) = InArgs(2:CurArgs)
                IF ( CurArgs .LT. 12) THEN
                  OutArgs(CurArgs+3:14) = Blank
                ENDIF
                CurArgs = 14
                DO shadeCtrlNum=1,TotOldShadeControls
                  IF (.not. SameString(ShadeControls(shadeCtrlNum)%OldShadeControlName,InArgs(1))) CYCLE
                  IF (ShadeControls(shadeCtrlNum)%NumZones .GT. 0) THEN
                    DO newZoneNum=1,ShadeControls(shadeCtrlNum)%NumZones
                      OutArgs(1) = TRIM(InArgs(1)) // '-' // TRIM(ShadeControls(shadeCtrlNum)%ShadeControlZoneName(newZoneNum))
                      OutArgs(2) = TRIM(ShadeControls(shadeCtrlNum)%ShadeControlZoneName(newZoneNum)) ! Zone Name
                      OutArgs(3) = RoundSigDigits(ShadeControls(shadeCtrlNum)%SequenceNum(newZoneNum),0) ! Shading Control Sequence Number
                      OutArgs(15) = Blank ! Daylighting Control Name
                      IF (SameString('SwitchableGlazing',InArgs(2)) .AND. SameString('MeetDaylightIlluminanceSetpoint',InArgs(4))) THEN
                        OutArgs(16) = 'Group' ! Multiple Surface Control Type
                      ELSE
                        OutArgs(16) = 'Sequential' ! Multiple Surface Control Type
                      ENDIF
                      CurArgs = 16
                      ! Find matching Daylighting control object, if any
                      DO daylightNum=1,GetNumObjectsFound('DAYLIGHTING:CONTROLS')
                        CALL GetObjectItem('DAYLIGHTING:CONTROLS',daylightNum,Alphas,NumAlphas,Numbers,NumNumbers,Status)
                        IF (.not. SameString(ShadeControls(shadeCtrlNum)%ShadeControlZoneName(newZoneNum),Alphas(2))) CYCLE
                        OutArgs(15) = TRIM(Alphas(1)) ! Daylighting Control Name
                        EXIT
                      ENDDO
                      ! Add surface fields
                      DO surfNum=1,TotObjsWithShadeCtrl
                        IF (.not. SameString(FenSurf(surfNum)%FenShadeControlName,InArgs(1))) CYCLE
                        IF (.not. SameString(FenSurf(surfNum)%FenZoneName,ShadeControls(shadeCtrlNum)%ShadeControlZoneName(newZoneNum))) CYCLE
                        CurArgs = CurArgs + 1
                        OutArgs(CurArgs) = FenSurf(surfNum)%FenSurfName
                      ENDDO
                      nodiff = .false.
                      ! Write new shading control object for this combination of old shading control plus zone
                      CALL WriteOutIDFLines(DifLfn,ObjectName,CurArgs,OutArgs,NwFldNames,NwFldUnits)
                      Written=.true.
                    ENDDO
                  ELSE
                    ! This control was unused, so don't write it, because a zone name is required, throw a warning
                    CALL ShowWarningError('WindowProperty:ShadingControl=""' // TRIM(InArgs(1)) // '" was not used by any surfaces, so it has not been written to the new idf.',Auditf)
                    !OutArgs(1) = InArgs(1)
                    !OutArgs(2) = Blank ! Zone Name (don't know that here)
                    !OutArgs(15) = Blank ! Daylighting Control Name
                    !OutArgs(16) = 'Sequential' ! Multiple Surface Control Type
                    !CurArgs = 16
                    nodiff = .false.
                    Written=.true.
                  ENDIF
                  EXIT
                ENDDO

              ! If your original object starts with Z, insert the rules here

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
