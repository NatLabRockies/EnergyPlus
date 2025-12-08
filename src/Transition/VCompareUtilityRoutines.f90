SUBROUTINE CompareOldNew

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   September 2001
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This routine compares old and new IDDs and reports on differences.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
  USE InputProcessor  ! All are public in this version
  USE DataVCompareGlobals

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  INTEGER Count
  INTEGER Found
  INTEGER Fld
  INTEGER NFld
  INTEGER NAlpha
  INTEGER NNum
  INTEGER, EXTERNAL :: GetNewUnitNumber
  LOGICAL FieldNameMatch

  ALLOCATE(ObjStatus(NumObjectDefs))
  NumObjStats=0

  ALLOCATE(NotInNew(NumObjectDefs))
  NotInNew=Blank
  NNew=0

  NumDif=0

  DO Count=1,NumObjectDefs
    Found=FindItemInList(ListOfObjects(Count),NewObjectDef%Name,NewNumObjectDefs)
    IF (Found /= 0) THEN
      NumObjStats=NumObjStats+1
      ObjStatus(NumObjStats)%Name=ListOfObjects(Count)
      ObjStatus(NumObjStats)%OldIndex=Count
      ObjStatus(NumObjStats)%NewIndex=Found
      ObjStatus(NumObjStats)%Same=.true.
      ObjStatus(NumObjStats)%StatusFlag=0
      ALLOCATE(ObjStatus(NumObjStats)%FieldNameMatch(ObjectDef(Count)%NumParams))
      ALLOCATE(ObjStatus(NumObjStats)%UnitsMatch(ObjectDef(Count)%NumParams))
      ObjStatus(NumObjStats)%FieldNameMatch=.true.
      ObjStatus(NumObjStats)%UnitsMatch=Blank
      IF (ObjectDef(Count)%NumParams /= NewObjectDef(Found)%NumParams) THEN
        ObjStatus(NumObjStats)%Same=.false.
        ObjStatus(NumObjStats)%StatusFlag=DiffNumParams
        ! Check Field Status
        FieldnameMatch=.true.
        ObjStatus(NumObjStats)%UnitsMatched=.true.
        NAlpha=0
        NNum=0
        DO Fld=1,ObjectDef(Count)%NumParams
          IF (ObjectDef(Count)%AlphaOrNumeric(Fld)) THEN
            NAlpha=NAlpha+1
            NFld=FindItemInList(ObjectDef(Count)%AlphFieldChks(NAlpha),NewObjectDef(Found)%AlphFieldChks,NewObjectDef(Found)%NumAlpha)
            IF (NFld == 0) THEN
              FieldnameMatch=.false.
            ENDIF
          ELSE
            NNum=NNum+1
            NFld=FindItemInList(ObjectDef(Count)%NumRangeChks(NNum)%FieldName,NewObjectDef(Found)%NumRangeChks%FieldName,NewObjectDef(Found)%NumNumeric)
            IF (NFld == 0) THEN
              FieldnameMatch=.false.
            ELSEIF (.not. SameString(ObjectDef(Count)%NumRangeChks(NNum)%Units,NewObjectDef(Found)%NumRangeChks(NFld)%Units)) THEN
              ObjStatus(NumObjStats)%UnitsMatched=.false.
              ObjStatus(NumObjStats)%UnitsMatch(Fld)=TRIM(ObjectDef(Count)%NumRangeChks(NNum)%Units)//' -> '//TRIM(NewObjectDef(Found)%NumRangeChks(NFld)%Units)
            ENDIF
          ENDIF
        ENDDO
        IF (.not. FieldNameMatch) ObjStatus(NumObjStats)%StatusFlag=ObjStatus(NumObjStats)%StatusFlag+MisMatchFields
        IF (.not. ObjStatus(NumObjStats)%UnitsMatched) ObjStatus(NumObjStats)%StatusFlag=ObjStatus(NumObjStats)%StatusFlag+MisMatchUnits
      ELSE
        DO Fld=1,ObjectDef(Count)%NumParams
          IF (ObjectDef(Count)%AlphaOrNumeric(Fld) .and. .not. NewObjectDef(Found)%AlphaOrNumeric(Fld)) &
               ObjStatus(NumObjStats)%Same=.false.
          IF (.not. ObjectDef(Count)%AlphaOrNumeric(Fld) .and. NewObjectDef(Found)%AlphaOrNumeric(Fld)) &
               ObjStatus(NumObjStats)%Same=.false.
        ENDDO

        IF (ObjStatus(NumObjStats)%Same) THEN
          ! Alpha and Numeric args are same, are fields?
          FieldnameMatch=.true.
          ObjStatus(NumObjStats)%UnitsMatched=.true.
          NAlpha=0
          NNum=0
          DO Fld=1,ObjectDef(Count)%NumParams
            IF (ObjectDef(Count)%AlphaOrNumeric(Fld)) THEN
              NAlpha=NAlpha+1
              IF (.not. SameString(ObjectDef(Count)%AlphFieldChks(NAlpha),NewObjectDef(Found)%AlphFieldChks(NAlpha)))  THEN
                ObjStatus(NumObjStats)%Same=.false.
                ObjStatus(NumObjStats)%FieldNameMatch(Fld)=.false.
                FieldNameMatch=.false.
              ENDIF
            ELSE
              NNum=NNum+1
              IF (.not. SameString(ObjectDef(Count)%NumRangeChks(NNum)%FieldName,NewObjectDef(Found)%NumRangeChks(NNum)%FieldName)) THEN
                ObjStatus(NumObjStats)%Same=.false.
                ObjStatus(NumObjStats)%FieldNameMatch(Fld)=.false.
                FieldNameMatch=.false.
              ELSEIF (.not. SameString(ObjectDef(Count)%NumRangeChks(NNum)%Units,NewObjectDef(Found)%NumRangeChks(NNum)%Units)) THEN
                !ObjStatus(NumObjStats)%UnitsMatch(Fld)=.false.
                ObjStatus(NumObjStats)%Same=.false.
                ObjStatus(NumObjStats)%UnitsMatched=.false.
                ObjStatus(NumObjStats)%UnitsMatch(Fld)=TRIM(ObjectDef(Count)%NumRangeChks(NNum)%Units)//' -> '//TRIM(NewObjectDef(Found)%NumRangeChks(NNum)%Units)
              ENDIF
            ENDIF
          ENDDO
          IF (.not. ObjStatus(NumObjStats)%UnitsMatched) ObjStatus(NumObjStats)%StatusFlag=ObjStatus(NumObjStats)%StatusFlag+MisMatchUnits
          IF (.not. FieldNameMatch) ObjStatus(NumObjStats)%StatusFlag=ObjStatus(NumObjStats)%StatusFlag+MisMatchFields
        ELSE
          ObjStatus(NumObjStats)%StatusFlag=MisMatchArgs
        ENDIF
      ENDIF
    ELSE
      NNew=NNew+1
      NotInNew(NNew)=ListOfObjects(Count)
    ENDIF
    IF (.not. ObjStatus(NumObjStats)%Same) THEN
      NumDif=NumDif+1
    ENDIF

  ENDDO

  NumObsObjs=0
  DO Count=1,NewNumObjectDefs
    IF (NewObjectDef(Count)%ObsPtr > 0) THEN
      NumObsObjs=NumObsObjs+1
    ENDIF
  ENDDO

  IF (NumObsObjs > 0) THEN
    ALLOCATE(ObsObject(NumObsObjs))
    ALLOCATE(ObsObjRepName(NumObsObjs))
    ObsObject=Blank
    ObsObjRepName=Blank
    NumObsObjs=0
    DO Count=1,NewNumObjectDefs
      IF (NewObjectDef(Count)%ObsPtr > 0) THEN
        NumObsObjs=NumObsObjs+1
        ObsObject(NumObsObjs)=NewObjectDef(Count)%Name
        ObsObjRepName(NumObsObjs)=ObsoleteObjectsRepNames(NewObjectDef(Count)%ObsPtr)
      ENDIF
    ENDDO
  ENDIF

  ALLOCATE(NotInOld(NewNumObjectDefs))
  NotInOld=Blank
  NOld=0

  DO Count=1,NewNumObjectDefs
    Found=FindItemInList(NewObjectDef(Count)%Name,ListOfObjects,NumObjectDefs)
    IF (Found /= 0) CYCLE
    NOld=NOld+1
    NotInOld(NOld)=NewObjectDef(Count)%Name
  ENDDO

  RETURN

END SUBROUTINE CompareOldNew

SUBROUTINE ReportStats

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   September
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine reports the statistics on the differences between two
          ! Energy+.idd files.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
  USE DataGlobals, ONLY: MaxNameLength
  USE DataStringGlobals
  USE InputProcessor
  USE DataVCompareGlobals

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
  CHARACTER(len=*), PARAMETER :: FmtA="(A)"

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  INTEGER, EXTERNAL :: GetNewUnitNumber
  INTEGER LFN
  CHARACTER(len=20) CharNum
  INTEGER Arg
  INTEGER Count
  CHARACTER(len=MaxNameLength) :: DefaultString
  INTEGER DifFound
  INTEGER ObsObjNum
  INTEGER, EXTERNAL :: FindNumber
  CHARACTER(len=MaxNameLength) :: FirstString
  INTEGER Outs
  INTEGER Found
  CHARACTER(len=1) :: FieldDesignator='F'
  CHARACTER(len=25) :: cFieldNumber


      LFN=GetNewUnitNumber()
      OPEN(LFN,File=TRIM(ProgramPath)//'objstats.csv')
      WRITE(LFN,FmtA) 'Old IDD:'//TRIM(IDDFileNameWithPath)
      WRITE(CharNum,*) NNew
      CharNum=ADJUSTL(CharNum)
      WRITE(LFN,FmtA) 'Deleted Objects: '//TRIM(CharNum)
      IF (NNew > 0) THEN
        WRITE(LFN,FmtA) 'Object,Deletion Count'
        DO Count=1,NNew
          WRITE(CharNum,*) Count
          CharNum=ADJUSTL(CharNum)
          WRITE(LFN,FmtA) TRIM(NotInNew(Count))//','//TRIM(CharNum)
        ENDDO
      ENDIF
      WRITE(LFN,FmtA) 'New IDD:'//TRIM(NewIDDFileNameWithPath)
      WRITE(CharNum,*) NOld
      CharNum=ADJUSTL(CharNum)
      WRITE(LFN,FmtA) 'Additional Objects: '//TRIM(CharNum)
      IF (NOld > 0) THEN
        WRITE(LFN,FmtA) 'Object,Addition Count'
        DO Count=1,NOld
          WRITE(CharNum,*) Count
          CharNum=ADJUSTL(CharNum)
          WRITE(LFN,FmtA) TRIM(NotInOld(Count))//','//TRIM(CharNum)
        ENDDO
      ENDIF

      IF (NumObsObjs > 0) THEN
        WRITE(LFN,FmtA) ' '
        WRITE(CharNum,*) NumObsObjs
        CharNum=ADJUSTL(CharNum)
        WRITE(LFN,FmtA) 'Obsolete Objects: '//TRIM(CharNum)
        WRITE(LFN,FmtA) 'Obsolete Object,Replacement Name'
        DO Count=1,NumObsObjs
          WRITE(LFN,FmtA) TRIM(ObsObject(Count))//','//TRIM(ObsObjRepName(Count))
        ENDDO
      ENDIF

      WRITE(LFN,FmtA) ' '

      WRITE(CharNum,*) NumDif
      CharNum=ADJUSTL(CharNum)
      WRITE(LFN,FmtA) 'Different Objects: '//TRIM(CharNum)
      WRITE(LFN,FmtA) 'Object,Diff Status'
      DO Count=1,NumObjectDefs
        IF (ObjStatus(Count)%Same) CYCLE
        DifFound=FindNumber(ObjStatus(Count)%StatusFlag,DiffIndex,8)
        WRITE(charnum,*) ObjStatus(Count)%StatusFlag
        charnum=adjustl(charnum)
        WRITE(LFN,FmtA) TRIM(ObjStatus(Count)%Name)//', '//trim(charnum)//','//DiffDescription(DifFound)
      ENDDO
      WRITE(LFN,FmtA) ' '
      WRITE(LFN,*) 'Diff Description,Value'
      WRITE(LFN,*) 'Diff # Fields,',DiffNumParams
      WRITE(LFN,*) 'Arg Type (A-N) Mismatch,',MisMatchArgs
      WRITE(LFN,*) 'Field Name Change,',MisMatchFields
      WRITE(LFN,*) 'Units Change,',MisMatchUnits
      WRITE(LFN,*) 'Units Chg+Field MisMatch,',MisMatchFields+MisMatchUnits
      WRITE(LFN,*) '#Fields+Field Name Chg,',DiffNumParams+MisMatchFields
      WRITE(LFN,*) '#Fields+Units Change,',DiffNumParams+MisMatchUnits
      WRITE(LFN,*) '#Fields+Field Name Chg+Units Chg,',DiffNumParams+MisMatchUnits+MisMatchFields
      DO Count=1,NumObjectDefs
        IF (ObjStatus(Count)%Same) CYCLE
        IF (NumObsObjs > 0) THEN
          ObsObjNum=FindItemInList(ObjStatus(Count)%Name,ObsObject,NumObsObjs)
        ELSE
          ObsObjNum=0
        ENDIF
        WRITE(LFN,FmtA) ' '
        IF (ObsObjNum == 0) THEN
          WRITE(LFN,FmtA) 'Object,DiffStatus,Min-Fields,,,New Min-Fields'
        ELSE
          WRITE(LFN,FmtA) 'Object,DiffStatus,Min-Fields,,,New Min-Fields,New Object Name'
        ENDIF
        DifFound=FindNumber(ObjStatus(Count)%StatusFlag,DiffIndex,8)
        WRITE(LFN,FmtA,ADVANCE='NO') TRIM(ObjStatus(Count)%Name)//','//TRIM(DiffDescription(DifFound))//','

        CALL GetObjectDefInIDD(ObjStatus(Count)%Name,NumArgs,AorN,ReqFld,ObjMinFlds,FldNames,FldDefaults,FldUnits)
        CALL GetNewObjectDefInIDD(ObjStatus(Count)%Name,NwNumArgs,NwAorN,NwReqFld,NwObjMinFlds,NwFldNames,NwFldDefaults,NwFldUnits)

        WRITE(CharNum,*) ObjMinFlds
        CharNum=ADJUSTL(CharNum)
        WRITE(LFN,FmtA,ADVANCE='NO') TRIM(CharNum)//',,,'
        WRITE(CharNum,*) NwObjMinFlds
        CharNum=ADJUSTL(CharNum)
        IF (ObsObjNum == 0) THEN
          WRITE(LFN,FmtA) TRIM(CharNum)
        ELSE
          WRITE(LFN,FmtA) TRIM(CharNum)//','//TRIM(ObsObjRepName(ObsObjNum))
        ENDIF
        IF (ObjStatus(Count)%Unitsmatched) THEN
          WRITE(LFN,FmtA) ',Req,<Fld Def>,<Field Name>,N Req,<N Fld Def>,<N Field Name>'
        ELSE
          WRITE(LFN,FmtA) 'Units Change (fields shown),Req,<Fld Def>,<Field Name>,N Req,<N Fld Def>,<N Field Name>'
        ENDIF
        MatchArg=0
        DO Outs=1,NumArgs
          Found=FindItemInList(FldNames(Outs),NwFldNames,NwNumArgs)
          IF (Found /= 0) THEN
            MatchArg(Outs)=Found
          ENDIF
        ENDDO
        DO Arg=1,NumArgs
          DefaultString=FldDefaults(Arg)
          IF (DefaultString == Blank) DefaultString='<none>'
          WRITE(cFieldNumber,*) Arg
          cFieldNumber=ADJUSTL(cFieldNumber)
          FirstString=FieldDesignator//TRIM(cFieldNumber)//':'//ObjStatus(Count)%UnitsMatch(Arg)
          IF (FirstString == FieldDesignator//TRIM(cFieldNumber)//':') THEN
            IF (MatchArg(Arg) == 0) THEN
              FirstString=FieldDesignator//TRIM(cFieldNumber)//':'//'<different field name>'
            ELSE
              FirstString=FieldDesignator//TRIM(cFieldNumber)//':'
            ENDIF
          ENDIF
          IF (ReqFld(Arg)) THEN
            WRITE(LFN,FmtA,ADVANCE='NO') TRIM(FirstString)//',*,'//TRIM(DefaultString)//',"'//TRIM(FldNames(Arg))//'"'
          ELSE
            WRITE(LFN,FmtA,ADVANCE='NO') TRIM(FirstString)//',,'//TRIM(DefaultString)//',"'//TRIM(FldNames(Arg))//'"'
          ENDIF
          IF (Arg > NwNumArgs) THEN
            WRITE(LFN,FmtA) ' '
            CYCLE
          ENDIF
          DefaultString=NwFldDefaults(Arg)
          IF (DefaultString == Blank) DefaultString='<none>'
          IF (NwReqFld(Arg)) THEN
            WRITE(LFN,'(A)') ',*,'//TRIM(DefaultString)//',"'//TRIM(NwFldNames(Arg))//'"'
          ELSE
            WRITE(LFN,'(A)') ',,'//TRIM(DefaultString)//',"'//TRIM(NwFldNames(Arg))//'"'
          ENDIF
        ENDDO
        IF (NwNumArgs > NumArgs) THEN
          DO Arg=NumArgs+1,NwNumArgs
            DefaultString=NwFldDefaults(Arg)
            IF (DefaultString == Blank) DefaultString='<none>'
            IF (NwReqFld(Arg)) THEN
              WRITE(LFN,'(A)') ',,,,*,'//TRIM(DefaultString)//',"'//TRIM(NwFldNames(Arg))//'"'
            ELSE
              WRITE(LFN,'(A)') ',,,,,'//TRIM(DefaultString)//',"'//TRIM(NwFldNames(Arg))//'"'
            ENDIF
          ENDDO
        ENDIF
      ENDDO
      CLOSE(LFN)

  RETURN

END SUBROUTINE ReportStats

INTEGER FUNCTION FindNumber(Number,ListofNumbers,NumItems)

          ! FUNCTION INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   October 2001
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS FUNCTION:
          ! This function looks up a string in a similar list of
          ! items and returns the index of the item in the list, if
          ! found.  This routine is not case insensitive and doesn't need
          ! for most inputs -- they are automatically turned to UPPERCASE.
          ! If you need case insensitivity use FindItem.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  INTEGER, INTENT(IN) :: Number
  INTEGER, INTENT(IN), DIMENSION(*) :: ListofNumbers
  INTEGER, INTENT(IN) :: NumItems

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  INTEGER Count

  FindNumber=0

  DO Count=1,NumItems
    IF (Number == ListofNumbers(Count)) THEN
      FindNumber=Count
      EXIT
    ENDIF
  END DO

  RETURN

END FUNCTION FindNumber

FUNCTION TrimTrailZeros(RealValue) RESULT(OutputString)

          ! FUNCTION INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   April 2002
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS FUNCTION:
          ! This function accepts a number as parameter and trims
          ! trailing zeros from it in the resultant string.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! FUNCTION ARGUMENT DEFINITIONS:
  REAL, INTENT(IN) :: RealValue
  CHARACTER(len=30) OutputString

          ! FUNCTION PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! FUNCTION LOCAL VARIABLE DECLARATIONS:
  INTEGER Pos
  INTEGER DPos
  INTEGER IC
  CHARACTER(len=30) String
  CHARACTER(len=10) EString

  WRITE(String,*) RealValue
  Pos=INDEX(String,'E')
  IF (Pos > 0) THEN
    EString=String(Pos:)
    String(Pos:)=' '
  ELSE
    EString=' '
  ENDIF
  DPos=INDEX(String,'.')
  IF (Pos == 0) THEN
    Pos=LEN_TRIM(String)
  ENDIF
  DO IC=Pos,DPos+1,-1
    IF (String(IC:IC) /= '0') EXIT
    String(IC:IC)=' '
  ENDDO
  String=TRIM(String)//EString
  OutputString=ADJUSTL(String)

  RETURN

END FUNCTION TrimTrailZeros

SUBROUTINE CopyComments(IDFFileName,DifLfn)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   Feb 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine copies the initial set of comments from the old IDF to
          ! the newly opened "diflfn" -- if "full" is on.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  CHARACTER(len=*), INTENT(IN) :: IDFFileName  ! Name of IDFFile -- already processed
  INTEGER, INTENT(IN)          :: DifLfn       ! Unit number of open "diff file"

          ! SUBROUTINE PARAMETER DEFINITIONS:
  character(len=*),parameter :: fmta="(A)"
  character(len=*),parameter :: blank=' '

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  INTEGER :: IDFFile
  integer ios1
  character(len=500) line
  character(len=500) aline
  logical endofcommentsfound
  integer, external :: GetNewUnitNumber

  IDFFile=GetNewUnitNumber()
  OPEN(IDFFile,File=TRIM(IDFFileName),ACTION='READ')

    endofcommentsfound=.false.
    ios1=0
    do while (.not. endofcommentsfound .and. ios1 == 0)
      read(IDFFile,fmta,iostat=ios1) line
      aline=adjustl(line)
      if (aline(1:1) == '!' .or. aline(1:1) == '#') then
        write(DifLfn,fmta) trim(line)
        cycle
      endif
      if (aline /= blank) then
        endofcommentsfound=.true.
      else
        write(DifLfn,fmta) trim(line)
      endif
    enddo

  CLOSE(IDFFile)

  RETURN

END SUBROUTINE CopyComments

SUBROUTINE ScanOutputVariablesForReplacement(  &
   WhichArg,  &
   DelThis,  &
   checkrvi,  &
   nodiff,  &
   ObjectName,  &
   DifLfn,      &
   OutVar,      &
   MtrVar,  &
   TimeBinVar,  &
   CurArgs,     &
   Written,     &
   WarnWhenAdding)


          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   January 2013
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! Processes the current variable name for replacement, delete, etc

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
  USE DataGlobals, ONLY: MaxNameLength
  USE DataVCompareGlobals
  USE InputProcessor, ONLY: MakeUPPERCase, SameString
  USE DataStringGlobals, ONLY: ProgNameConversion
  USE VCompareGlobalRoutines

  IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  INTEGER, INTENT(IN)             :: WhichArg    ! Changed variable name, if any
  LOGICAL, INTENT(OUT)            :: DelThis   ! True when this variable is deleted.
  LOGICAL, INTENT(OUT)            :: checkrvi  ! True when need to check rvi (any variable changed)
  LOGICAL, INTENT(OUT)            :: nodiff    ! True when diffs happened
  CHARACTER(len=*), INTENT(IN)    :: ObjectName
  INTEGER, INTENT(IN)             :: DifLfn
  LOGICAL, INTENT(IN)             :: OutVar
  LOGICAL, INTENT(IN)             :: MtrVar
  LOGICAL, INTENT(IN)             :: TimeBinVar
  INTEGER, INTENT(IN)             :: CurArgs
  LOGICAL, INTENT(INOUT)          :: Written
  LOGICAL, INTENT(IN)             :: WarnWhenAdding

          ! SUBROUTINE PARAMETER DEFINITIONS:
  CHARACTER(len=*), PARAMETER :: Blank=' '
  CHARACTER(len=*), PARAMETER :: fmtA='(A)'
! for V8.0 only
  CHARACTER(len=*), PARAMETER, DIMENSION(10) :: Digits=  &
     (/'1 ','2 ','3 ','4 ','5 ','6 ','7 ','8 ','9 ','10'/)
          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  CHARACTER(len=MaxNameLength) :: UCRepVarName
  CHARACTER(len=MaxNameLength) :: UCCompRepVarName
  INTEGER :: Arg
  LOGICAL :: WildMatch
  INTEGER :: pos
  INTEGER :: xCount
  INTEGER :: pos1
  INTEGER :: pos2

  UCRepVarName=MakeUPPERCase(OutArgs(WhichArg))
  ! Remove [ as part of output variable name
  pos=INDEX(UCRepVarName,']',.true.)
  IF (pos > 0 .and. len_trim(UCRepVarName) == pos) THEN
    pos2=INDEX(UCRepVarName,'[',.true.)
    if (pos2 > 0) then  ! could check valid units...
      if (.not. OutArgs(WhichArg)(pos2+1:pos-1) == ' ') then
        if (IsValidOutputUnits(OutArgs(WhichArg)(pos2+1:pos-1))) then
          UCRepVarName=UCRepVarName(1:pos2-1)
          OutArgs(WhichArg)=OutArgs(WhichArg)(1:pos2-1)
        endif
      else  ! blank
        UCRepVarName=UCRepVarName(1:pos2-1)
        OutArgs(WhichArg)=OutArgs(WhichArg)(1:pos2-1)
      endif
    endif
  ENDIF
  DelThis=.false.
  WildMatch=.false.
  Written=.false.
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
          OutArgs(WhichArg)=NewRepVarName(Arg)
          nodiff=.false.
          checkrvi=.true.
        ELSE
          OutArgs(WhichArg)=TRIM(NewRepVarName(Arg))//OutArgs(WhichArg)(Len_Trim(UCCompRepVarName)+1:)
          nodiff=.false.
          checkrvi=.true.
        ENDIF
        IF (VersionNum == 8.0) THEN
          IF (.not. MtrVar) THEN
            IF (UCCompRepVarName == 'CONDFD NODAL TEMPERATURE') THEN
              CondFDVariables=.true.
              IF (OutArgs(WhichArg-1) == '*' .or. OutArgs(WhichArg-1) == ' ') THEN
                pos1=INDEX(NewRepVarName(Arg),'<',.true.)
                DO xCount=1,10
                  OutArgs(WhichArg)=NewRepVarName(Arg)(1:pos1-1)//Digits(xCount)
                  Written=.false.
                  CALL CheckSpecialObjects(DifLfn,ObjectName,CurArgs,OutArgs,NwFldNames,NwFldUnits,Written)
                  IF (.not. Written) THEN
                    CALL WriteOutIDFLines(DifLfn,ObjectName,CurArgs,OutArgs,NwFldNames,NwFldUnits)
                  ENDIF
                ENDDO
                Written=.true.
              ELSE  ! has key value, use it
                pos1=INDEX(OutArgs(WhichArg-1),'#',.true.)
                pos2=INDEX(NewRepVarName(Arg),'<',.true.)
                OutArgs(WhichArg)=NewRepVarName(Arg)(1:pos2-1)//OutArgs(WhichArg-1)(pos1+1:)
                OutArgs(WhichArg-1)=OutArgs(WhichArg-1)(1:pos1-6)
              ENDIF
            ENDIF
          ENDIF
        ENDIF
        IF (NewRepVarCaution(Arg) /= Blank .and. .not. SameString(NewRepVarCaution(Arg)(1:6),'Forkeq') ) THEN
          IF (OutVar) THEN
            IF (.not. OutVarCaution(Arg)) THEN  ! caution message not written yet
              CALL writePreprocessorObject(DifLfn,PrognameConversion,'Warning',  &
                 trim(ObjectName)//' (old)="'//trim(OldRepVarName(Arg))//  &
                 '" conversion to '//trim(ObjectName)//' (new)="'//  &
                 trim(NewRepVarName(Arg))//'" has the following caution "'//trim(NewRepVarCaution(Arg))//'".')
              write(diflfn,fmtA) ' '
              OutVarCaution(Arg)=.true.
            ENDIF
          ELSEIF (MtrVar) THEN
            IF (.not. MtrVarCaution(Arg)) THEN  ! caution message not written yet
              CALL writePreprocessorObject(DifLfn,PrognameConversion,'Warning',  &
                 trim(ObjectName)//' (old)="'//trim(OldRepVarName(Arg))//  &
                 '" conversion to '//trim(ObjectName)//' (new)="'//  &
                 trim(NewRepVarName(Arg))//'" has the following caution "'//trim(NewRepVarCaution(Arg))//'".')
              write(diflfn,fmtA) ' '
              MtrVarCaution(Arg)=.true.
            ENDIF
          ELSEIF (TimeBinVar) THEN
            IF (.not. TimeBinVarCaution(Arg)) THEN  ! caution message not written yet
              CALL writePreprocessorObject(DifLfn,PrognameConversion,'Warning',  &
                 trim(ObjectName)//' (old)="'//trim(OldRepVarName(Arg))//  &
                 '" conversion to '//trim(ObjectName)//' (new)="'//  &
                 trim(NewRepVarName(Arg))//'" has the following caution "'//trim(NewRepVarCaution(Arg))//'".')
              write(diflfn,fmtA) ' '
              TimeBinVarCaution(Arg)=.true.
            ENDIF
          ENDIF
        ENDIF
      ELSE
        DelThis=.true.
        checkrvi=.true.
      ENDIF
      IF (OldRepVarName(Arg) == OldRepVarName(Arg+1)) THEN
        IF (.not. SameString(NewRepVarCaution(Arg)(1:6),'Forkeq')) THEN
          CALL CheckSpecialObjects(DifLfn,ObjectName,CurArgs,OutArgs,NwFldNames,NwFldUnits,Written)
          IF (.not. Written) THEN
            CALL WriteOutIDFLines(DifLfn,ObjectName,CurArgs,OutArgs,NwFldNames,NwFldUnits)
          ENDIF
          IF (WarnWhenAdding) THEN
            CALL writePreprocessorObject(DifLfn,PrognameConversion,'Severe',  &
               trim(ObjectName)//' (old)="'//trim(OldRepVarName(Arg))//  &
               '" conversion to '//trim(ObjectName)//' (new)="'//  &
               trim(NewRepVarName(Arg))//  &
               '"due to variable rename - may not be accurate & cause fatal errors.')
            write(diflfn,fmtA) ' '
          ENDIF
          IF (.not. WildMatch) THEN
            OutArgs(WhichArg)=NewRepVarName(Arg+1)
            nodiff=.false.
            checkrvi=.true.
          ELSE
            OutArgs(WhichArg)=TRIM(NewRepVarName(Arg+1))//OutArgs(WhichArg)(Len_Trim(UCCompRepVarName)+1:)
            nodiff=.false.
            checkrvi=.true.
          ENDIF
          IF (NewRepVarCaution(Arg+1) /= Blank .and. .not. SameString(NewRepVarCaution(Arg+1)(1:6),'Forkeq') ) THEN
            IF (OutVar) THEN
              IF (.not. OutVarCaution(Arg+1)) THEN  ! caution message not written yet
                CALL writePreprocessorObject(DifLfn,PrognameConversion,'Warning',  &
                   trim(ObjectName)//' (old)="'//trim(OldRepVarName(Arg+1))//  &
                   '" conversion to '//trim(ObjectName)//' (new)="'//  &
                   trim(NewRepVarName(Arg+1))//'" has the following caution "'//trim(NewRepVarCaution(Arg+1))//'".')
                write(diflfn,fmtA) ' '
                OutVarCaution(Arg+1)=.true.
              ENDIF
            ELSEIF (MtrVar) THEN
              IF (.not. MtrVarCaution(Arg+1)) THEN  ! caution message not written yet
                CALL writePreprocessorObject(DifLfn,PrognameConversion,'Warning',  &
                   trim(ObjectName)//' (old)="'//trim(OldRepVarName(Arg+1))//  &
                   '" conversion to '//trim(ObjectName)//' (new)="'//  &
                   trim(NewRepVarName(Arg+1))//'" has the following caution "'//trim(NewRepVarCaution(Arg+1))//'".')
                write(diflfn,fmtA) ' '
                MtrVarCaution(Arg+1)=.true.
              ENDIF
            ELSEIF (TimeBinVar) THEN
              IF (.not. TimeBinVarCaution(Arg+1)) THEN  ! caution message not written yet
                CALL writePreprocessorObject(DifLfn,PrognameConversion,'Warning',  &
                   trim(ObjectName)//' (old)="'//trim(OldRepVarName(Arg+1))//  &
                   '" conversion to '//trim(ObjectName)//' (new)="'//  &
                   trim(NewRepVarName(Arg+1))//'" has the following caution "'//trim(NewRepVarCaution(Arg+1))//'".')
                write(diflfn,fmtA) ' '
                TimeBinVarCaution(Arg+1)=.true.
              ENDIF
            ENDIF
          ENDIF
        ELSEIF (VersionNum == 8.0) THEN
          IF (NumChillerHeaters > 0) THEN  ! Forkeq
            IF (NumChillers > 0) THEN
              CALL CheckSpecialObjects(DifLfn,ObjectName,CurArgs,OutArgs,NwFldNames,NwFldUnits,Written)
              IF (.not. Written) THEN
                CALL WriteOutIDFLines(DifLfn,ObjectName,CurArgs,OutArgs,NwFldNames,NwFldUnits)
              ENDIF
            ENDIF
            IF (.not. WildMatch) THEN
              OutArgs(WhichArg)=NewRepVarName(Arg+1)
              nodiff=.false.
              checkrvi=.true.
            ELSE
              OutArgs(WhichArg)=TRIM(NewRepVarName(Arg+1))//OutArgs(WhichArg)(Len_Trim(UCCompRepVarName)+1:)
              nodiff=.false.
              checkrvi=.true.
            ENDIF
            IF (NewRepVarCaution(Arg+1) /= Blank .and. .not. SameString(NewRepVarCaution(Arg+1)(1:6),'Forkeq') ) THEN
              IF (OutVar) THEN
                IF (.not. OutVarCaution(Arg+1)) THEN  ! caution message not written yet
                  CALL writePreprocessorObject(DifLfn,PrognameConversion,'Warning',  &
                     trim(ObjectName)//' (old)="'//trim(OldRepVarName(Arg+1))//  &
                     '" conversion to '//trim(ObjectName)//' (new)="'//  &
                     trim(NewRepVarName(Arg+1))//'" has the following caution "'//trim(NewRepVarCaution(Arg+1))//'".')
                  write(diflfn,fmtA) ' '
                  OutVarCaution(Arg+1)=.true.
                ENDIF
              ELSEIF (MtrVar) THEN
                IF (.not. MtrVarCaution(Arg+1)) THEN  ! caution message not written yet
                  CALL writePreprocessorObject(DifLfn,PrognameConversion,'Warning',  &
                     trim(ObjectName)//' (old)="'//trim(OldRepVarName(Arg+1))//  &
                     '" conversion to '//trim(ObjectName)//' (new)="'//  &
                     trim(NewRepVarName(Arg+1))//'" has the following caution "'//trim(NewRepVarCaution(Arg+1))//'".')
                  write(diflfn,fmtA) ' '
                  MtrVarCaution(Arg+1)=.true.
                ENDIF
              ELSEIF (TimeBinVar) THEN
                IF (.not. TimeBinVarCaution(Arg+1)) THEN  ! caution message not written yet
                  CALL writePreprocessorObject(DifLfn,PrognameConversion,'Warning',  &
                     trim(ObjectName)//' (old)="'//trim(OldRepVarName(Arg+1))//  &
                     '" conversion to '//trim(ObjectName)//' (new)="'//  &
                     trim(NewRepVarName(Arg+1))//'" has the following caution "'//trim(NewRepVarCaution(Arg+1))//'".')
                  write(diflfn,fmtA) ' '
                  TimeBinVarCaution(Arg+1)=.true.
                ENDIF
              ENDIF
            ENDIF
          ENDIF
        ENDIF ! V=8.0
      ENDIF
      IF (OldRepVarName(Arg) == OldRepVarName(Arg+2)) THEN
        CALL CheckSpecialObjects(DifLfn,ObjectName,CurArgs,OutArgs,NwFldNames,NwFldUnits,Written)
        IF (.not. Written) THEN
          CALL WriteOutIDFLines(DifLfn,ObjectName,CurArgs,OutArgs,NwFldNames,NwFldUnits)
        ENDIF
        IF (WarnWhenAdding) THEN
          CALL writePreprocessorObject(DifLfn,PrognameConversion,'Severe',  &
             trim(ObjectName)//' (old)="'//trim(OldRepVarName(Arg))//  &
             '" conversion to '//trim(ObjectName)//' (new)="'//  &
             trim(NewRepVarName(Arg))//  &
             '" due to variable rename - may not be accurate & cause fatal errors.')
          write(diflfn,fmtA) ' '
        ENDIF
        IF (.not. WildMatch) THEN
          OutArgs(WhichArg)=NewRepVarName(Arg+2)
          nodiff=.false.
          checkrvi=.true.
        ELSE
          OutArgs(WhichArg)=TRIM(NewRepVarName(Arg+2))//OutArgs(WhichArg)(Len_Trim(UCCompRepVarName)+1:)
          nodiff=.false.
          checkrvi=.true.
        ENDIF
        IF (NewRepVarCaution(Arg+2) /= Blank) THEN
          IF (OutVar) THEN
            IF (.not. OutVarCaution(Arg+2)) THEN  ! caution message not written yet
              CALL writePreprocessorObject(DifLfn,PrognameConversion,'Warning',  &
                 trim(ObjectName)//' (old)="'//trim(OldRepVarName(Arg+2))//  &
                 '" conversion to '//trim(ObjectName)//' (new)="'//  &
                 trim(NewRepVarName(Arg+2))//'" has the following caution "'//trim(NewRepVarCaution(Arg+2))//'".')
              write(diflfn,fmtA) ' '
              OutVarCaution(Arg+2)=.true.
            ENDIF
          ELSEIF (MtrVar) THEN
            IF (.not. MtrVarCaution(Arg+2)) THEN  ! caution message not written yet
              CALL writePreprocessorObject(DifLfn,PrognameConversion,'Warning',  &
                 trim(ObjectName)//' (old)="'//trim(OldRepVarName(Arg+2))//  &
                 '" conversion to '//trim(ObjectName)//' (new)="'//  &
                 trim(NewRepVarName(Arg+2))//'" has the following caution "'//trim(NewRepVarCaution(Arg+2))//'".')
              write(diflfn,fmtA) ' '
              MtrVarCaution(Arg+2)=.true.
            ENDIF
          ELSEIF (TimeBinVar) THEN
            IF (.not. TimeBinVarCaution(Arg+2)) THEN  ! caution message not written yet
              CALL writePreprocessorObject(DifLfn,PrognameConversion,'Warning',  &
                 trim(ObjectName)//' (old)="'//trim(OldRepVarName(Arg+2))//  &
                 '" conversion to '//trim(ObjectName)//' (new)="'//  &
                 trim(NewRepVarName(Arg+2))//'" has the following caution "'//trim(NewRepVarCaution(Arg+2))//'".')
              write(diflfn,fmtA) ' '
              TimeBinVarCaution(Arg+2)=.true.
            ENDIF
          ENDIF
        ENDIF
      ENDIF
      EXIT
    ENDIF
  ENDDO

  RETURN

END SUBROUTINE ScanOutputVariablesForReplacement

SUBROUTINE ProcessRviMviFiles(FileNameWithoutExtension,FileExtension)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   February 2006
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! Allows for updating of Rvi and/or Mvi files.  RVI and MVI files have report variable names/meter
          ! names one per line.  A colon (:) may separate a key from a variable name/meter name.  Only report
          ! variable or meter names will be in the variable lists.

          ! METHODOLOGY EMPLOYED:
          ! Reads old file, copys to "new<extension>".  Processes Report Variable names, old to new.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
  USE DataVCompareGlobals
  USE InputProcessor, ONLY: MakeUPPERCase

  IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  CHARACTER(len=*), INTENT(IN) :: FileNameWithoutExtension
  CHARACTER(len=*), INTENT(IN) :: FileExtension

          ! SUBROUTINE PARAMETER DEFINITIONS:
  CHARACTER(len=*), PARAMETER :: fmta='(A)'
  CHARACTER(len=*), PARAMETER :: blank=' '
  CHARACTER(len=*), PARAMETER :: tab=char(9)
!   Version 8.0 only
  CHARACTER(len=*), PARAMETER, DIMENSION(10) :: Digits=  &
     (/'1 ','2 ','3 ','4 ','5 ','6 ','7 ','8 ','9 ','10'/)

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  INTEGER :: oldunit
  INTEGER :: newUnit
  CHARACTER(len=MaxNameLength*2+20) unitline
  LOGICAL :: WildMatch  ! User entered * for wild match on output variable names
  LOGICAL :: DelThis
  INTEGER :: IoS
  INTEGER :: Arg
  CHARACTER(len=MaxNameLength+30) :: keyValue
  CHARACTER(len=250) :: comment
  LOGICAL :: keyValueEntered
  LOGICAL :: commentEntered
  CHARACTER(len=MaxNameLength) UCRepVarName
  CHARACTER(len=MaxNameLength) UCCompRepVarName
  CHARACTER(len=MaxNameLength) cNewRepVarName
  INTEGER, EXTERNAL :: GetNewUnitNumber
  INTEGER :: pos
  INTEGER :: pos1
  INTEGER :: pos2
  LOGICAL :: newDup
  INTEGER :: xCount  ! Version 8.0 only

  ! need to treat CondFD special.

  INQUIRE(File=TRIM(FileNameWithoutExtension)//'.'//TRIM(FileExtension),EXIST=FileOK)

  IF (FileOK) THEN
    oldunit=GetNewUnitNumber()
    OPEN(Unit=oldunit,File=TRIM(FileNameWithoutExtension)//'.'//TRIM(FileExtension))
    WRITE(Auditf,fmta) ' Processing '//trim(FileExtension)//' -- '//trim(FileNameWithoutExtension)//'.'//trim(FileExtension)
    newunit=GetNewUnitNumber()
    OPEN(Unit=newunit,File=TRIM(FileNameWithoutExtension)//'.'//TRIM(FileExtension)//'new')
    READ(oldunit,fmta,iostat=IoS) unitline
    WRITE(newunit,fmta) TRIM(unitline)
    READ(oldunit,fmta,iostat=IoS) unitline
    WRITE(newunit,fmta) TRIM(unitline)
    DO
      READ(oldunit,fmta,iostat=IoS) unitline
      if (IoS /= 0) EXIT
      pos=INDEX(unitline,',')  ! check for key value on rvi line.  (none on mvi line)
      IF (pos > 0) THEN
        keyValue=unitline(1:pos-1)
        keyValueEntered=.true.
        unitline=unitline(pos+1:)
      ELSE
        keyValue=blank
        keyValueEntered=.false.
      ENDIF
      pos=INDEX(unitline,'!')  ! check for comment
      if (pos > 0) then
        commentEntered=.true.
        comment=unitline(pos:)
        unitline=unitline(1:pos-1)
        if (pos == 1) then
          write(newunit,fmta) trim(comment)
          cycle
        endif
      else
        commentEntered=.false.
        comment=blank
      endif
      pos=INDEX(unitline,'[')  ! check for units (illegal) entered on rvi/mvi line
      if (pos > 0) then
        unitline=unitline(1:pos-1)
      endif
      pos=INDEX(unitline,tab)  ! check for tabs entered on rvi/mvi line
      do while (pos>0)
        unitline(pos:pos)=blank
        pos=INDEX(unitline,tab)  ! check for tabs entered on rvi/mvi line
      enddo
      unitline=ADJUSTL(unitline)
      UCRepVarName=MakeUPPERCase(unitline)
      DelThis=.false.
      newDup=.false.
      IF (UCRepVarName /= blank) THEN
        DO Arg=1,NumRepVarNames
          ! Wild matches only work on Old Output Variables.
          UCCompRepVarName=MakeUPPERCase(OldRepVarName(Arg))
          IF (UCCompRepVarName(Len_Trim(UCCompRepVarName):Len_Trim(UCCompRepVarName)) == '*') THEN
            WildMatch=.true.
            UCCompRepVarName(Len_Trim(UCCompRepVarName):Len_Trim(UCCompRepVarName))=' '
          ELSE
            WildMatch=.false.
          ENDIF
          IF (UCRepVarName == UCCompRepVarName) THEN
            pos = 1
          ELSE
            CYCLE
          ENDIF
  !        pos=INDEX(TRIM(UCRepVarname),TRIM(UCCompRepVarName))
          ! wildmatches -- must be at "start" of string (i.e., pos=1 or pos-1 = ":"
          newDup=.false.
          IF (pos > 0) THEN
            IF (OldRepVarName(Arg) == OldRepVarName(Arg+1)) newDup=.true.
            IF (NewRepVarName(Arg) /= '<DELETE>') THEN
              IF (VersionNum == 8.0) THEN
                IF (UCCompRepVarName == 'CONDFD NODAL TEMPERATURE') THEN
                  IF (.not. keyvalueEntered) THEN  ! just had Nodal Temperature on line
                    pos1=INDEX(NewRepVarName(Arg),'<',.true.)
                    DO xCount=1,9
                      unitline=NewRepVarName(Arg)(1:pos1-1)//Digits(xCount)
                      if (.not. commentEntered) then
                        WRITE(newunit,fmta) TRIM(unitline)
                      else
                        WRITE(newunit,fmta) TRIM(unitline)//'     '//trim(comment)
                      endif
                    ENDDO
                    xCount=10
                    unitline=NewRepVarName(Arg)(1:pos1-1)//Digits(xCount)
                  ELSE  ! has key value, use it
                    pos1=INDEX(keyValue,'#',.true.)
                    pos2=INDEX(NewRepVarName(Arg),'<',.true.)
                    unitline=NewRepVarName(Arg)(1:pos2-1)//keyValue(pos1+1:)
                    keyvalue=keyvalue(1:pos1-1)
                  ENDIF
                ELSE
                  cNewRepVarName=NewRepVarName(Arg)
                  pos2=INDEX(cNewRepVarName,'<')
                  if (pos2 > 0) then
                    cNewRepVarName=cNewRepVarName(1:pos2-1)  ! this is a wildcard to readvars.
                  endif
                  IF (.not. WildMatch) THEN
                    unitline=cNewRepVarName
                  ELSE  ! WildMatch
                    pos1=INDEX(TRIM(UCRepVarname),TRIM(UCCompRepVarName))
                    IF (pos1 == 1) THEN
                      unitline(pos:)=TRIM(NewRepVarName(Arg))//unitline(pos+Len_Trim(UCCompRepVarName):)
                    ELSE
                      IF (unitline(pos1-1:pos1-1) == ':') THEN  ! okay, can substitute
                        unitline(pos:)=TRIM(NewRepVarName(Arg))//unitline(pos+Len_Trim(UCCompRepVarName):)
                      ENDIF
                    ENDIF
                  ENDIF
                ENDIF
              ELSE ! not Version = 8.0
                cNewRepVarName=NewRepVarName(Arg)
                pos2=INDEX(cNewRepVarName,'<')
                if (pos2 > 0) then
                  cNewRepVarName=cNewRepVarName(1:pos2-1)  ! this is a wildcard to readvars.
                endif
                IF (.not. WildMatch) THEN
                  unitline=cNewRepVarName
                ELSE  ! WildMatch
                  pos1=INDEX(TRIM(UCRepVarname),TRIM(UCCompRepVarName))
                  IF (pos1 == 1) THEN
                    unitline(pos:)=TRIM(NewRepVarName(Arg))//unitline(pos+Len_Trim(UCCompRepVarName):)
                  ELSE
                    IF (unitline(pos1-1:pos1-1) == ':') THEN  ! okay, can substitute
                      unitline(pos:)=TRIM(NewRepVarName(Arg))//unitline(pos+Len_Trim(UCCompRepVarName):)
                    ENDIF
                  ENDIF
                ENDIF
              ENDIF  ! Version 8.0
            ELSE
              DelThis=.true.
            ENDIF
            EXIT
          ENDIF
        ENDDO
      ENDIF
      IF (DelThis) CYCLE
      IF (.not. keyValueEntered) THEN
        if (.not. commentEntered) then
          WRITE(newunit,fmta) TRIM(unitline)
        else
          if (unitline /= blank) then
            WRITE(newunit,fmta) TRIM(unitline)//'     '//trim(comment)
          else
            WRITE(newunit,fmta) trim(comment)
          endif
        endif
      ELSE
        if (.not. commentEntered) then
          WRITE(newunit,fmta) trim(keyValue)//','//TRIM(unitline)
        else
          WRITE(newunit,fmta) trim(keyValue)//','//TRIM(unitline)//'     '//trim(comment)
        endif
      ENDIF
      if (newDup) then
        unitline=NewRepVarName(Arg+1)
        IF (.not. keyValueEntered) THEN
          if (.not. commentEntered) then
            WRITE(newunit,fmta) TRIM(unitline)
          else
            if (unitline /= blank) then
              WRITE(newunit,fmta) TRIM(unitline)//'     '//trim(comment)
            else
              WRITE(newunit,fmta) trim(comment)
            endif
          endif
        ELSE
          if (.not. commentEntered) then
            WRITE(newunit,fmta) trim(keyValue)//','//TRIM(unitline)
          else
            WRITE(newunit,fmta) trim(keyValue)//','//TRIM(unitline)//'     '//trim(comment)
          endif
        ENDIF
      endif
    ENDDO
    CLOSE(newunit)
    CLOSE(oldunit)
  ENDIF

  RETURN

END SUBROUTINE ProcessRviMviFiles

SUBROUTINE CreateNewName(InputName,OutputName,InputType)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   Sept 2005
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This routine creates a new name for a "zone" based object using
          ! the zone name and counter of how many so far.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
  USE InputProcessor, ONLY: FindItemInList

  IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  CHARACTER(len=*), INTENT(IN)  :: InputName
  CHARACTER(len=*), INTENT(OUT) :: OutputName
  CHARACTER(len=*), INTENT(IN)  :: InputType

          ! SUBROUTINE PARAMETER DEFINITIONS:
  CHARACTER(len=*), PARAMETER, DIMENSION(13) :: InputTypes=(/'PEOPLE             ',  &
                                                             'LIGHTS             ',  &
                                                             'ELECTRIC EQUIPMENT ',  &
                                                             'GAS EQUIPMENT      ',  &
                                                             'HOT WATER EQUIPMENT',  &
                                                             'OTHER EQUIPMENT    ',  &
                                                             'STEAM EQUIPMENT    ',  &
                                                             'BASEBOARD HEAT     ',  &
                                                             'INFILTRATION       ',  &
                                                             'VENTILATION        ',  &
                                                             'MIXING             ',  &
                                                             'CROSS MIXING       ',  &
                                                             'WHM:Sch            '/)
  CHARACTER(len=*), PARAMETER, DIMENSION(13) :: cInputTypes=(/'People ',  &
                                                              'Lights ',  &
                                                              'ElecEq ',  &
                                                              'GasEq  ',  &
                                                              'HWEq   ',  &
                                                              'OthEq  ',  &
                                                              'StmEq  ',  &
                                                              'BBHt   ',  &
                                                              'Infil  ',  &
                                                              'Ventl  ',  &
                                                              'Mixng  ',  &
                                                              'XMixng ',  &
                                                              'WHM:Sch'/)


          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
    TYPE AddNames
      CHARACTER(len=60) :: cName=' '
      INTEGER :: Counter=0
    END TYPE


          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    TYPE (AddNames), ALLOCATABLE, SAVE, DIMENSION(:) :: NewAddNames
    TYPE (AddNames), ALLOCATABLE, SAVE, DIMENSION(:) :: TempNewAddNames
    INTEGER,SAVE :: NameRec=0
    INTEGER :: Found
    INTEGER :: IntFnd
    CHARACTER(len=25) :: cNumber
    CHARACTER(len=60) :: LookName

    IF (InputName == 'Reallocate') THEN
      IF (ALLOCATED(NewAddNames)) DEALLOCATE(NewAddNames)
      NameRec=0
      RETURN
    ENDIF

    IntFnd=FindItemInList(InputType,InputTypes,13)
    IF (cInputTypes(IntFnd) /= 'WHM:Sch') THEN
      LookName=TRIM(InputName)//' '//TRIM(cInputTypes(IntFnd))
    ELSE
      LookName=TRIM(cInputTypes(IntFnd))//'_'//TRIM(InputName)
    ENDIF
    IF (NameRec == 0) THEN
      ALLOCATE(NewAddNames(1))
      NewAddNames(1)%cName=LookName
      NewAddNames(1)%Counter=1
      NameRec=1
      Found=1
    ELSE
      Found=FindItemInList(LookName,NewAddNames%cName,NameRec)
      IF (Found == 0) THEN
        !  Add a new name
        ALLOCATE(TempNewAddNames(NameRec))
        TempNewAddNames=NewAddNames
        DEALLOCATE(NewAddNames)
        NameRec=NameRec+1
        ALLOCATE(NewAddNames(NameRec))
        NewAddNames(1:NameRec-1)=TempNewAddNames(1:NameRec-1)
        NewAddNames(NameRec)%cName=LookName
        NewAddNames(NameRec)%Counter=1
        Found=NameRec
        DEALLOCATE(TempNewAddNames)
      ELSE
        NewAddNames(Found)%Counter=NewAddNames(Found)%Counter+1
      ENDIF
    ENDIF

    WRITE(cNumber,*) NewAddNames(Found)%Counter
    cNumber=ADJUSTL(cNumber)
    cNumber=' '//cNumber
    OutputName=TRIM(LookName)//TRIM(cNumber)

  RETURN

END SUBROUTINE CreateNewName

SUBROUTINE writePreProcessorObject(unitNo,programName,severityLevel,inmessage)

          ! SUBROUTINE INFORMATION:
          !    AUTHOR         Jason Glazer of GARD Analytics, Inc.
          !    DATE WRITTEN   October 2005
          !    MODIFIED       LKL; Transition; August 2011
          !    RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !    Writes the errors found during the run to the IDF file
          !  Adapted from epfilter for transition program - LKL August 2011

          ! METHODOLOGY EMPLOYED:

          ! REFERENCES:
          !    na

          ! USE STATEMENTS:
USE DataVCompareGlobals

IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine


          ! SUBROUTINE ARGUMENT DEFINITIONS:
INTEGER, INTENT(IN) :: unitNo  ! file unit for writing
CHARACTER(len=*), INTENT(IN) :: programName ! program name for object
CHARACTER(len=*), INTENT(IN) :: severityLevel ! severity level for message
CHARACTER(len=*), INTENT(IN) :: inmessage

          ! SUBROUTINE PARAMETER DEFINITIONS:
INTEGER, PARAMETER :: charsPerLine=95
CHARACTER(len=*), PARAMETER :: fmta='(A)'

          ! INTERFACE BLOCK SPECIFICATIONS
          !    na

          ! DERIVED TYPE DEFINITIONS
          !    na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
CHARACTER(len=1000) :: message
INTEGER :: lenMsg
INTEGER :: startOfLine
INTEGER :: endOfLine
INTEGER :: iquote1
INTEGER :: iquote2
INTEGER :: commasemipos

 !'SizingPeriod:DesignDay object="LONDON/GATWICK Modified Ann Clg .4% Condns DB=>MWB" Solar Model Indicator="Schedule" does not use Sky Clearness but prior input was not blank. Copied to new field.'

  message=inmessage

  IF (VersionNum >= 3.0) THEN
    write(unitNo,fmta) ' Output:PreprocessorMessage,'//trim(programName)//','//trim(severityLevel)//','
  ELSE
    write(unitNo,fmta) 'Preprocessor Message,'//trim(programName)//','//trim(severityLevel)//','
  ENDIF

  lenMsg = LEN_TRIM(message)
  commasemipos=INDEX(message,',')
  DO WHILE (commasemipos > 0)
    message(commasemipos:commasemipos)='.'
    commasemipos=INDEX(message,',')
  ENDDO
  commasemipos=INDEX(message,';')
  DO WHILE (commasemipos > 0)
    message(commasemipos:commasemipos)='.'
    commasemipos=INDEX(message,';')
  ENDDO
  startOfLine = 1
  endOfLine = startOfLine + charsPerLine
  if (endOfLine > lenMsg) THEN
    endOfLine=lenMsg
    write(unitNo,fmta) message(startOfLine:endOfLine)//';'
  ELSE
    DO
      DO WHILE (message(endOfLine:endOfLine) /= ' ')
        endOfLine = endOfLine - 1
        IF (endOfLine == startOfLine) THEN
          endOfLine = startOfLine + charsPerLine
          IF (endOfLine > lenMsg) endOfLine=lenMsg
          EXIT
        END IF
      END DO
      if (endOfLine < lenMsg) then
        iquote1=0
        iquote2=0
        iquote1=index(message(startOfLine:lenmsg),'"')
        if (iquote1 > 0) then
          if (iquote1 > 0 .and. startOfLine+iquote1 <= endOfLine) then
            iquote2=index(message(startOfLine+iquote1+1:lenmsg),'"')
          endif
          if (iquote1 > 0 .and. startOfLine+iquote1-1 <= endOfLine .and. startOfLine+iquote1+1+iquote2-1 > endOfLine) then
            ! need to break over the "
            endOfLine=startOfLine+iquote1-2
          else
            endOfLine=startOfLine+iquote1+1+iquote2-1
          endif
        endif
      endif
      IF (endOfLine < lenMsg) THEN
          write(unitNo,fmta) message(startOfLine:endOfLine)//','
      ELSE
        write(unitNo,fmta) message(startOfLine:min(endOfLine,lenmsg))//';'
      END IF
      startOfLine = endOfLine + 1
      endOfLine = startOfLine + charsPerLine
      IF (startOfLine > lenMsg) EXIT
      IF (endOfLine > lenMsg) endOfLine=lenMsg
      if (endOfLine-startOfLine+1 <= charsPerLine) then
        write(unitNo,fmta) message(startOfLine:endOfLine)//';'
        EXIT
      endif
    END DO
  ENDIF
END SUBROUTINE writePreProcessorObject
