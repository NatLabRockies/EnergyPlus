MODULE InputProcessor
          ! Module containing the input processor routines

          ! MODULE INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   August 1997
          !       MODIFIED       2/25/04 by COP to make it specific for Basement program
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS MODULE:
          ! To provide the capabilities of reading the input data dictionary
          ! and input file and supplying the simulation routines with the data
          ! contained therein.

          ! METHODOLOGY EMPLOYED:
          !

          ! REFERENCES:
          ! The input syntax is designed to allow for future flexibility without
          ! necessitating massive (or any) changes to this code.  Two files are
          ! used as key elements: (1) the input data dictionary will specify the
          ! sections and objects that will be allowed in the actual simulation
          ! input file and (2) the simulation input data file will be processed
          ! with the data therein being supplied to the actual simulation routines.



          ! OTHER NOTES:
          !
          !

          ! USE STATEMENTS:
          ! Use statements for data only modules
 USE DataPrecisionGlobals
 USE DataStringGlobals
 USE DataGlobals
 USE BasementSimData, ONLY: EPObjects
          ! Use statements for access to subroutines in other modules

 IMPLICIT NONE         ! Enforce explicit typing of all variables
 PRIVATE

          !MODULE PARAMETER DEFINITIONS
 INTEGER, PARAMETER         :: ObjectDefAllocInc=100     ! Starting number of Objects allowed in IDD as well as the increment
                                                         ! when max is reached
 INTEGER, PARAMETER         :: SectionDefAllocInc=20     ! Starting number of Sections allowed in IDD as well as the increment
                                                         ! when max is reached
 INTEGER, PARAMETER         :: SectionsIDFAllocInc=20    ! Initial number of Sections allowed in IDF as well as the increment
                                                         ! when max is reached
 INTEGER, PARAMETER         :: ObjectsIDFAllocInc=500    ! Initial number of Objects allowed in IDF as well as the increment
                                                         ! when max is reached
 INTEGER, PARAMETER         :: MaxObjectNameLength=MaxNameLength    ! Maximum number of characters in an Object Name
 INTEGER, PARAMETER         :: MaxSectionNameLength=MaxNameLength   ! Maximum number of characters in a Section Name
 INTEGER, PARAMETER         :: MaxAlphaArgLength=MaxNameLength  ! Maximum number of characters in an Alpha Argument
 INTEGER, PARAMETER         :: MaxInputLineLength=500    ! Maximum number of characters in an input line (in.idf, energy+.idd)
 CHARACTER(len=1), PARAMETER :: Blank=' '
 CHARACTER(len=*), PARAMETER :: AlphaNum='ANan'     ! Valid indicators for Alpha or Numeric fields (A or N)
 CHARACTER(len=*), PARAMETER :: fmta='(A)'
 REAL(r64), PARAMETER :: DefAutoSizeValue=-99999.  !AutoSize
 REAL(r64), PARAMETER :: DefAutoCalculateValue=-99999. !AutoCalculate

          ! DERIVED TYPE DEFINITIONS
 TYPE RangeCheckDef
   LOGICAL :: MinMaxChk                            =.false.   ! true when Min/Max has been added
   INTEGER :: FieldNumber                          =0         ! which field number this is
   CHARACTER(len=MaxNameLength+40) :: FieldName =' '       ! Name of the field
   CHARACTER(len=20), DIMENSION(2) :: MinMaxString =' '       ! appropriate Min/Max Strings
   REAL(r64), DIMENSION(2) :: MinMaxValue          =0.0       ! appropriate Min/Max Values
   INTEGER, DIMENSION(2) :: WhichMinMax            =0         !=0 (none/invalid), =1 \min, =2 \min>, =3 \max, =4 \max<
   LOGICAL :: DefaultChk                           =.false.   ! true when default has been entered
   REAL(r64)  :: Default                           =0.0       ! Default value
   LOGICAL :: DefAutoSize                          =.false.   ! Default value is "autosize"
   LOGICAL :: AutoSizable                          =.false.   ! True if this field can be autosized
   REAL(r64)  :: AutoSizeValue                     =0.0       ! Value to return for autosize field
   LOGICAL :: DefAutoCalculate                     =.false.   ! Default value is "autocalculate"
   LOGICAL :: AutoCalculatable                     =.false.   ! True if this field can be autocalculated
   REAL(r64)  :: AutoCalculateValue                =0.0       ! Value to return for autocalculate field
 END TYPE

 TYPE ObjectsDefinition
   CHARACTER(len=MaxObjectNameLength) :: Name = Blank ! Name of the Object
   INTEGER :: NumParams      =0 ! Number of parameters to be processed for each object
   INTEGER :: NumAlpha       =0 ! Number of Alpha elements in the object
   INTEGER :: NumNumeric     =0 ! Number of Numeric elements in the object
   INTEGER :: MinNumFields                    =0   ! Minimum number of fields to be passed to the Get routines
   LOGICAL :: NameAlpha1                  =.false. ! True if the first alpha appears to "name" the object for error messages
   LOGICAL :: UniqueObject                =.false. ! True if this object has been designated \unique-object
   LOGICAL :: RequiredObject              =.false. ! True if this object has been designated \required-object
   LOGICAL :: ExtensibleObject            =.false. ! True if this object has been designated \extensible
   INTEGER :: ExtensibleNum                   =0   ! how many fields to extend
   INTEGER :: LastExtendAlpha                 =0   ! Count for extended alpha fields
   INTEGER :: LastExtendNum                   =0   ! Count for extended numeric fields
   INTEGER :: ObsPtr                          =0   ! If > 0, object is obsolete and this is the
                                                   ! Pointer to ObsoleteObjectRepNames Array for replacement object
   LOGICAL(1), ALLOCATABLE, DIMENSION(:) :: AlphaorNumeric ! Positionally, whether the argument
                                                           ! is alpha (true) or numeric (false)
   LOGICAL(1), ALLOCATABLE, DIMENSION(:) :: ReqField ! True for required fields
   LOGICAL(1), ALLOCATABLE, DIMENSION(:) :: AlphRetainCase ! true if retaincase is set for this field (alpha fields only)
   CHARACTER(len=MaxNameLength+40),  &
               ALLOCATABLE, DIMENSION(:) :: AlphFieldChks ! Field names for alphas
   CHARACTER(len=MaxNameLength),  &
               ALLOCATABLE, DIMENSION(:) :: AlphFieldDefs ! Defaults for alphas
   TYPE(RangeCheckDef), ALLOCATABLE, DIMENSION(:) :: NumRangeChks  ! Used to range check and default numeric fields
   INTEGER :: NumFound                        =0   ! Number of this object found in IDF
 END TYPE

 TYPE SectionsDefinition
   CHARACTER(len=MaxSectionNameLength) :: Name =Blank ! Name of the Section
   INTEGER :: NumFound       =0 ! Number of this object found in IDF
 END TYPE

 TYPE FileSectionsDefinition
   CHARACTER(len=MaxSectionNameLength) :: Name =Blank ! Name of this section
   INTEGER :: FirstRecord     =0 ! Record number of first object in section
   INTEGER :: FirstLineNo     =0   ! Record number of first object in section
   INTEGER :: LastRecord      =0 ! Record number of last object in section
 END TYPE

 TYPE LineDefinition      ! Will be saved for each "object" input
                          ! The arrays (Alphas, Numbers) will be dimensioned to be
                          ! the size expected from the definition.
   CHARACTER(len=MaxObjectNameLength) :: Name   =Blank ! Object name for this record
   INTEGER :: NumAlphas                        =0   ! Number of alphas on this record
   INTEGER :: NumNumbers                       =0   ! Number of numbers on this record
   INTEGER :: ObjectDefPtr                     =0   ! Which Object Def is this
   CHARACTER(len=MaxAlphaArgLength), ALLOCATABLE, DIMENSION(:) :: Alphas ! Storage for the alphas
   LOGICAL, ALLOCATABLE, DIMENSION(:) :: AlphBlank  ! Set to true if this field was blank on input
   REAL(r64), ALLOCATABLE, DIMENSION(:) :: Numbers       ! Storage for the numbers
   LOGICAL, ALLOCATABLE, DIMENSION(:) :: NumBlank   ! Set to true if this field was blank on input
 END TYPE


          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! MODULE VARIABLE DECLARATIONS:

!Integer Variables for the Module
INTEGER :: NumObjectDefs       =0 ! Count of number of object definitions found in the IDD
INTEGER :: NumSectionDefs      =0 ! Count of number of section defintions found in the IDD
INTEGER :: MaxObjectDefs       =0 ! Current "max" object defs (IDD), when reached will be reallocated and new Max set
INTEGER :: MaxSectionDefs      =0 ! Current "max" section defs (IDD), when reached will be reallocated and new Max set
INTEGER :: IDDFile             =0 ! Unit number for reading IDD (Energy+.idd)
INTEGER :: IDFFile             =0 ! Unit number for reading IDF (in.idf)
INTEGER :: NumLines            =0 ! Count of number of lines in IDF
INTEGER :: MaxIDFRecords       =0 ! Current "max" IDF records (lines), when reached will be reallocated and new Max set
INTEGER :: NumIDFRecords       =0 ! Count of number of IDF records
INTEGER :: MaxIDFSections      =0 ! Current "max" IDF sections (lines), when reached will be reallocated and new Max set
INTEGER :: NumIDFSections      =0 ! Count of number of IDF records
INTEGER, EXTERNAL :: GetNewUnitNumber  ! External  function to "get" a unit number
INTEGER :: EchoInputFile       =0 ! Unit number of the file echoing the IDD and input records (eplusout.audit)
INTEGER :: InputLineLength     =0 ! Actual input line length or position of comment character
INTEGER :: MaxAlphaArgsFound   =0 ! Count of max alpha args found in the IDD
INTEGER :: MaxNumericArgsFound =0 ! Count of max numeric args found in the IDD
INTEGER :: MaxAlphaIDFArgsFound   =0 ! Count of max alpha args found in the IDF
INTEGER :: MaxNumericIDFArgsFound =0 ! Count of max numeric args found in the IDF
INTEGER :: MaxAlphaIDFDefArgsFound   =0 ! Count of max alpha args found in the IDF
INTEGER :: MaxNumericIDFDefArgsFound =0 ! Count of max numeric args found in the IDF
INTEGER :: NumOutOfRangeErrorsFound=0  ! Count of number of "out of range" errors found
INTEGER :: NumBlankReqFieldFound=0 ! Count of number of blank required field errors found
INTEGER :: NumMiscErrorsFound  =0  ! Count of other errors found
INTEGER :: MinimumNumberOfFields=0 ! When ReadLine discovers a "minimum" number of fields for an object, this variable is set
INTEGER :: NumObsoleteObjects=0    ! Number of \obsolete objects
INTEGER :: TotalAuditErrors=0      ! Counting some warnings that go onto only the audit file
INTEGER :: NumSecretObjects=0      ! Number of objects in "Secret Mode"
LOGICAL :: ProcessingIDD=.false.   ! True when processing IDD, false when processing IDF


!Real Variables for Module
!na

!Character Variables for Module
CHARACTER(len=MaxInputLineLength+50) :: InputLine  = Blank ! Each line can be up to MaxInputLineLength characters long
CHARACTER(len=MaxSectionNameLength), ALLOCATABLE:: ListofSections(:)
CHARACTER(len=MaxObjectNameLength),  ALLOCATABLE:: ListofObjects(:)
CHARACTER(len=MaxObjectNameLength) :: CurrentFieldName   = Blank ! Current Field Name (IDD)
CHARACTER(len=MaxObjectNameLength) :: ReplacementName=Blank
INTEGER,  ALLOCATABLE, DIMENSION(:) :: ObjectStartRecord

!Logical Variables for Module
LOGICAL :: OverallErrorFlag =.false.     ! If errors found during parse of IDF, will fatal at end
LOGICAL :: EchoInputLine=.true.          ! Usually True, if the IDD is backspaced, then is set to false, then back to true
LOGICAL :: ReportRangeCheckErrors=.true. ! Module level reporting logical, can be turned off from outside the module (and then
                                         ! must be turned back on.
LOGICAL :: FieldSet=.false.              ! Set to true when ReadInputLine has just scanned a "field"
LOGICAL :: RequiredField=.false.         ! Set to true when ReadInputLine has determined that this field is required
LOGICAL :: RetainCaseFlag=.false.        ! Set to true when ReadInputLine has determined that this field should retain case
LOGICAL :: ObsoleteObject=.false.        ! Set to true when ReadInputLine has an obsolete object
LOGICAL :: RequiredObject=.false.        ! Set to true when ReadInputLine has a required object
LOGICAL :: UniqueObject=.false.          ! Set to true when ReadInputLine has a unique object
LOGICAL :: ExtensibleObject=.false.      ! Set to true when ReadInputLine has an extensible object
INTEGER :: ExtensibleNumFields=0         ! set to number when ReadInputLine has an extensible object
INTEGER, ALLOCATABLE, DIMENSION(:) :: LineBufLen
LOGICAL :: SortedIDD=.false.

!Derived Types Variables

TYPE (ObjectsDefinition), ALLOCATABLE, DIMENSION(:) :: ObjectDef (:)   ! Contains all the Valid Objects on the IDD
TYPE (SectionsDefinition), ALLOCATABLE, DIMENSION(:) :: SectionDef (:) ! Contains all the Valid Sections on the IDD
TYPE (FileSectionsDefinition), ALLOCATABLE, DIMENSION(:) :: SectionsonFile (:)  ! lists the sections on file (IDF)
TYPE (LineDefinition):: LineItem                        ! Description of current record
TYPE (LineDefinition), ALLOCATABLE, DIMENSION(:) :: IDFRecords(:)     ! All the objects read from the IDF

PUBLIC  ProcessInput

PUBLIC  GetNumSectionsFound
PUBLIC  GetNumSectionsinInput
PUBLIC  GetListofSectionsinInput
PUBLIC  FindIteminList
PUBLIC  FindItem
PUBLIC  SameString
PUBLIC  MakeUPPERCase
PUBLIC  ProcessNumber
PUBLIC  RangeCheck
PUBLIC  VerifyName

PUBLIC  GetNumObjectsFound
PUBLIC  GetObjectItem
PUBLIC  GetObjectItemNum
PUBLIC  GetObjectItemfromFile
PUBLIC  GetRecordLocations
PUBLIC  TellMeHowManyObjectItemArgs
PUBLIC  TurnOnReportRangeCheckErrors
PUBLIC  TurnOffReportRangeCheckErrors
PUBLIC  GetNumRangeCheckErrorsFound

PUBLIC  GetNumObjectsInIDD
PUBLIC  GetListOfObjectsInIDD
PUBLIC  GetObjectDefInIDD

CONTAINS

! MODULE SUBROUTINES:
!*************************************************************************

SUBROUTINE ProcessInput(IDDFileName,IDFFileName)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   August 1997
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine processes the input for EnergyPlus.  First, the
          ! input data dictionary is read and interpreted.  Using the structure
          ! from the data dictionary, the actual simulation input file is read.
          ! This file is processed according to the "rules" in the data dictionary
          ! and stored in a local data structure which will be used during the simulation.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  CHARACTER(len=*), INTENT(IN), OPTIONAL :: IDDFileName
  CHARACTER(len=*), INTENT(IN), OPTIONAL :: IDFFileName

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
   LOGICAL FileExists ! Check variable for .idd/.idf files
   LOGICAL :: ErrorsInIDD=.false.   ! to check for any errors flagged during data dictionary processing
   INTEGER :: Loop
   INTEGER :: CountErr
   INTEGER :: Num1
   INTEGER :: Which
   INTEGER :: iNumberOfRecords,iNumberOfDefaultedFields,iTotalFieldsWithDefaults,  &
       iNumberOfAutosizedFields,iTotalAutoSizableFields,iNumberOfAutoCalcedFields,iTotalAutoCalculatableFields
   CHARACTER(len=MaxNameLength) :: SaveIDFFileName

   EchoInputFile=GetNewUnitNumber()
   OPEN(unit=EchoInputFile,file='audit.out')
   !               FullName from StringGlobals is used to build file name with Path

   IF (PRESENT(IDDFileName)) THEN
     FullName=IDDFileName
   ELSE
     FullName=DefaultIDD
   ENDIF

   IDDFile=GetNewUnitNumber()
   Open (unit=IDDFile, file=FullName, action='READ')
   NumLines=0

   WRITE(EchoInputFile,*) ' Processing Data Dictionary ('//trim(FullName)//') File -- Start'

   ProcessingIDD=.true.
   Call ProcessDataDicFile(ErrorsInIDD)


   ALLOCATE (ListofObjects(NumObjectDefs))
   ListofObjects=ObjectDef(1:NumObjectDefs)%Name
   ALLOCATE (ObjectStartRecord(NumObjectDefs))
   ObjectStartRecord=0

   Close (unit=IDDFile)

   IF (NumObjectDefs == 0) THEN
     CALL ShowFatalError('No objects found in IDD.  Program will terminate.',EPObjects)
     ErrorsInIDD=.true.
   ENDIF
   !  If no fatal to here, rewind EchoInputFile -- only keep processing data...
   IF (.not. ErrorsInIDD) THEN
     REWIND(Unit=EchoInputFile)
   ENDIF

   ProcessingIDD=.false.
   WRITE(EchoInputFile,*) ' Processing Data Dictionary ('//trim(FullName)//') File -- Complete'

   WRITE(EchoInputFile,*) ' Maximum number of Alpha Args=',MaxAlphaArgsFound
   WRITE(EchoInputFile,*) ' Maximum number of Numeric Args=',MaxNumericArgsFound
   WRITE(EchoInputFile,*) ' Number of Object Definitions=',NumObjectDefs
   WRITE(EchoInputFile,*) ' Number of Section Definitions=',NumSectionDefs

   IF (PRESENT(IDFFileName)) THEN
     SaveIDFFileName=IDFFileName
   ELSE
     SaveIDFFileName=DefaultIDF
   ENDIF
   WRITE(EchoInputFile,*) ' Processing Input Data File ('//trim(SaveIDFFileName)//') -- Start'

   IDFFile=GetNewUnitNumber()
   Open (unit=IDFFile, file=IDFFIleName, action='READ')
   NumLines=0
   EchoInputLine=.true.

   Call ProcessInputDataFile

   ALLOCATE (ListofSections(NumSectionDefs))
   ListofSections=SectionDef(1:NumSectionDefs)%Name

   Call ValidateSectionsInput

   WRITE(EchoInputFile,*) ' Processing Input Data File ('//trim(SaveIDFFileName)//') -- Complete'
   WRITE(EchoInputFile,*) ' Number of IDF "Lines"=',NumIDFRecords
   IF (NumIDFRecords == 0) THEN
     CALL ShowSevereError('IP: The IDF file has no records.',EPObjects)
     NumMiscErrorsFound=NumMiscErrorsFound+1
   ENDIF

   ! Check for required objects
   DO Loop=1,NumObjectDefs
     IF (.not. ObjectDef(Loop)%RequiredObject) CYCLE
     IF (ObjectDef(Loop)%NumFound > 0) CYCLE
     CALL ShowSevereError('IP: Required Object="'//trim(ObjectDef(Loop)%Name)//'" not found in IDF.',EPObjects)
     NumMiscErrorsFound=NumMiscErrorsFound+1
   ENDDO

   IF (NumOutOfRangeErrorsFound > 0) THEN
     CALL ShowSevereError('IP: Out of "range" values found in input')
   ENDIF

   IF (NumBlankReqFieldFound > 0) THEN
     CALL ShowSevereError('IP: Blank "required" fields found in input')
   ENDIF

   IF (NumMiscErrorsFound > 0) THEN
     CALL ShowSevereError('IP: Other miscellaneous errors found in input')
   ENDIF

   IF (OverallErrorFlag) THEN
     CALL ShowContinueError('Possible Invalid Numerics or other problems',EPObjects)
     CALL ShowFatalError('IP: Errors occurred on processing input file. Preceding condition(s) cause termination.',EPObjects)
   ENDIF

   IF (NumOutOfRangeErrorsFound+NumBlankReqFieldFound+NumMiscErrorsFound > 0) THEN
     CALL ShowSevereError('IP: Out of "range" values and/or blank required fields found in input',EPObjects)
     CALL ShowFatalError('IP: Errors occurred on processing IDF file. Preceding condition(s) cause termination.',EPObjects)
   ENDIF

  RETURN

END SUBROUTINE ProcessInput

SUBROUTINE ProcessDataDicFile(ErrorsFound)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   August 1997
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine processes data dictionary file for EnergyPlus.
          ! The structure of the sections and objects are stored in derived
          ! types (SectionDefs and ObjectDefs)

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  LOGICAL, INTENT(INOUT) :: ErrorsFound ! set to true if any errors flagged during IDD processing

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
   LOGICAL  :: EndofFile = .false.        ! True when End of File has been reached (IDD or IDF)
   INTEGER Pos                            ! Test of scanning position on the current input line
   TYPE (SectionsDefinition), ALLOCATABLE :: TempSectionDef(:)  ! Like SectionDef, used during Re-allocation
   TYPE (ObjectsDefinition), ALLOCATABLE :: TempObjectDef(:)    ! Like ObjectDef, used during Re-allocation
   LOGICAL BlankLine


   MaxSectionDefs=SectionDefAllocInc
   MaxObjectDefs=ObjectDefAllocInc

   ALLOCATE (SectionDef(MaxSectionDefs))

   ALLOCATE(ObjectDef(MaxObjectDefs))

   NumObjectDefs=0
   NumSectionDefs=0
   EndofFile=.false.

   DO WHILE (.not. EndofFile)
     CALL ReadInputLine(IDDFile,Pos,BlankLine,InputLineLength,EndofFile)
     IF (BlankLine .or. EndofFile) CYCLE
     Pos=SCAN(InputLine(1:InputLineLength),',;')
     If (Pos /= 0) then

       If (InputLine(Pos:Pos) == ';') then
         CALL AddSectionDef(InputLine(1:Pos-1),ErrorsFound)
         IF (NumSectionDefs == MaxSectionDefs) THEN
           ALLOCATE (TempSectionDef(MaxSectionDefs+SectionDefAllocInc))
           TempSectionDef(1:MaxSectionDefs)=SectionDef
           DEALLOCATE (SectionDef)
           ALLOCATE (SectionDef(MaxSectionDefs+SectionDefAllocInc))
           SectionDef=TempSectionDef
           DEALLOCATE (TempSectionDef)
           MaxSectionDefs=MaxSectionDefs+SectionDefAllocInc
         ENDIF
       else
         CALL AddObjectDefandParse(InputLine(1:Pos-1),Pos,EndofFile,ErrorsFound)
         IF (NumObjectDefs == MaxObjectDefs) THEN
           ALLOCATE (TempObjectDef(MaxObjectDefs+ObjectDefAllocInc))
           TempObjectDef(1:MaxObjectDefs)=ObjectDef
           DEALLOCATE (ObjectDef)
           ALLOCATE (ObjectDef(MaxObjectDefs+ObjectDefAllocInc))
           ObjectDef=TempObjectDef
           DEALLOCATE (TempObjectDef)
           MaxObjectDefs=MaxObjectDefs+ObjectDefAllocInc
         ENDIF
       endif

     else
       CALL ShowSevereError('IP: IDD line~'//TRIM(IPTrimSigDigits(NumLines))//' , or ; expected on this line',  &
          EchoInputFile,EPObjects)
       ErrorsFound=.true.
     endif

   END DO

   RETURN

END SUBROUTINE ProcessDataDicFile

SUBROUTINE AddSectionDef(ProposedSection,ErrorsFound)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   August 1997
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine adds a new section to SectionDefs.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  CHARACTER(len=*), INTENT(IN) :: ProposedSection  ! Proposed Section to be added
  LOGICAL, INTENT(INOUT) :: ErrorsFound ! set to true if errors found here

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:

  CHARACTER(len=MaxSectionNameLength) SqueezedSection  ! Input Argument, Left-Justified and Uppercase
  LOGICAL ErrFlag  ! Local error flag.  When True, Proposed Section is not added to global list

  SqueezedSection=MakeUPPERCase(ADJUSTL(ProposedSection))
  IF (LEN_TRIM(ADJUSTL(ProposedSection)) > MaxSectionNameLength) THEN
    CALL ShowWarningError('IP: Section length exceeds maximum, will be truncated='//TRIM(ProposedSection),EchoInputFile,EPObjects)
    CALL ShowContinueError('Will be processed as Section='//TRIM(SqueezedSection),EchoInputFile,EPObjects)
    ErrorsFound=.true.
  ENDIF
  ErrFlag=.false.

  IF (SqueezedSection /= Blank) THEN
    IF (FindItemInList(SqueezedSection,SectionDef%Name,NumSectionDefs) > 0) THEN
      CALL ShowSevereError('IP: Already a Section called '//TRIM(SqueezedSection)//'. This definition ignored.',  &
         EchoInputFile,EPObjects)
      ! Error Condition
      ErrFlag=.true.
      ErrorsFound=.true.
    ENDIF
  ELSE
    CALL ShowSevereError('IP: Blank Sections not allowed.  Review audit.out file.',EchoInputFile,EPObjects)
    ErrFlag=.true.
    ErrorsFound=.true.
  ENDIF

  IF (.not. ErrFlag) THEN
    NumSectionDefs=NumSectionDefs+1
    SectionDef(NumSectionDefs)%Name=SqueezedSection
    SectionDef(NumSectionDefs)%NumFound=0
  ENDIF

  RETURN

END SUBROUTINE AddSectionDef

SUBROUTINE AddObjectDefandParse(ProposedObject,CurPos,EndofFile,ErrorsFound)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   August 1997
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine processes data dictionary file for EnergyPlus.
          ! The structure of the sections and objects are stored in derived
          ! types (SectionDefs and ObjectDefs)

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS
  CHARACTER(len=*), INTENT(IN) :: ProposedObject  ! Proposed Object to Add
  INTEGER, INTENT(INOUT) :: CurPos ! Current position (initially at first ',') of InputLine
  LOGICAL, INTENT(INOUT) :: EndofFile ! End of File marker
  LOGICAL, INTENT(INOUT) :: ErrorsFound ! set to true if errors found here

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  CHARACTER(len=MaxObjectNameLength) SqueezedObject  ! Input Object, Left Justified, UpperCase
  INTEGER Count  ! Count on arguments, loop
  INTEGER Pos    ! Position scanning variable
  LOGICAL EndofObjectDef   ! Set to true when ; has been found
  LOGICAL ErrFlag   ! Local Error condition flag, when true, object not added to Global list
  CHARACTER(len=1) TargetChar   ! Single character scanned to test for current field type (A or N)
  LOGICAL BlankLine ! True when this line is "blank" (may have comment characters as first character on line)
  LOGICAL(1), ALLOCATABLE, SAVE, DIMENSION(:) :: AlphaorNumeric    ! Array of argument designations, True is Alpha,
                                                                   ! False is numeric, saved in ObjectDef when done
  LOGICAL(1), ALLOCATABLE, SAVE, DIMENSION(:) :: TempAN            ! Array (ref: AlphaOrNumeric) for re-allocation procedure
  LOGICAL(1), ALLOCATABLE, SAVE, DIMENSION(:) :: RequiredFields    ! Array of argument required fields
  LOGICAL(1), ALLOCATABLE, SAVE, DIMENSION(:) :: TempRqF           ! Array (ref: RequiredFields) for re-allocation procedure
  LOGICAL(1), ALLOCATABLE, SAVE, DIMENSION(:) :: AlphRetainCase    ! Array of argument for retain case
  LOGICAL(1), ALLOCATABLE, SAVE, DIMENSION(:) :: TempRtC           ! Array (ref: AlphRetainCase) for re-allocation procedure
  CHARACTER(len=MaxObjectNameLength+40),   &
              ALLOCATABLE, SAVE, DIMENSION(:) :: AlphFieldChecks   ! Array with alpha field names
  CHARACTER(len=MaxObjectNameLength+40),   &
              ALLOCATABLE, SAVE, DIMENSION(:) :: TempAFC           ! Array (ref: AlphFieldChecks) for re-allocation procedure
  CHARACTER(len=MaxObjectNameLength),   &
              ALLOCATABLE, SAVE, DIMENSION(:) :: AlphFieldDefaults ! Array with alpha field defaults
  CHARACTER(len=MaxObjectNameLength),   &
              ALLOCATABLE, SAVE, DIMENSION(:) :: TempAFD           ! Array (ref: AlphFieldDefaults) for re-allocation procedure
  TYPE(RangeCheckDef), ALLOCATABLE, SAVE, DIMENSION(:) :: NumRangeChecks  ! Structure for Range Check, Defaults of numeric fields
  TYPE(RangeCheckDef), ALLOCATABLE, SAVE, DIMENSION(:) :: TempChecks ! Structure (ref: NumRangeChecks) for re-allocation procedure
  LOGICAL MinMax   ! Set to true when MinMax field has been found by ReadInputLine
  LOGICAL Default  ! Set to true when Default field has been found by ReadInputLine
  LOGICAL AutoSize ! Set to true when Autosizable field has been found by ReadInputLine
  LOGICAL AutoCalculate ! Set to true when Autocalculatable field has been found by ReadInputLine
  CHARACTER(len=20) MinMaxString ! Set from ReadInputLine
  CHARACTER(len=MaxObjectNameLength) AlphDefaultString
  INTEGER WhichMinMax   !=0 (none/invalid), =1 \min, =2 \min>, =3 \max, =4 \max<
  REAL(r64) Value  ! Value returned by ReadInputLine (either min, max, default, autosize or autocalculate)
  LOGICAL MinMaxError  ! Used to see if min, max, defaults have been set appropriately (True if error)
  INTEGER,SAVE   :: MaxANArgs=100  ! Current count of Max args to object
  LOGICAL ErrorsFoundFlag

  IF (.not. ALLOCATED(AlphaorNumeric)) THEN
    ALLOCATE (AlphaorNumeric(0:MaxANArgs))
    ALLOCATE (RequiredFields(0:MaxANArgs))
    ALLOCATE (AlphRetainCase(0:MaxANArgs))
    ALLOCATE (NumRangeChecks(MaxANArgs))
    ALLOCATE (AlphFieldChecks(MaxANArgs))
    ALLOCATE (AlphFieldDefaults(MaxANArgs))
!    ALLOCATE (ObsoleteObjectsRepNames(0))
  ENDIF

  SqueezedObject=MakeUPPERCase(ADJUSTL(ProposedObject))
  IF (LEN_TRIM(ADJUSTL(ProposedObject)) > MaxObjectNameLength) THEN
    CALL ShowWarningError('IP: Object length exceeds maximum, will be truncated='//TRIM(ProposedObject),EchoInputFile,EPObjects)
    CALL ShowContinueError('Will be processed as Object='//TRIM(SqueezedObject),EchoInputFile,EPObjects)
    ErrorsFound=.true.
  ENDIF

  ! Start of Object parse, set object level items
  ErrFlag=.false.
  ErrorsFoundFlag=.false.
  MinimumNumberOfFields=0
  ObsoleteObject=.false.
  UniqueObject=.false.
  RequiredObject=.false.
  ExtensibleObject=.false.
  ExtensibleNumFields=0
  MinMax=.false.
  Default=.false.
  AutoSize=.false.
  AutoCalculate=.false.
  WhichMinMax=0


  IF (SqueezedObject /= Blank) THEN
    IF (FindItemInList(SqueezedObject,ObjectDef%Name,NumObjectDefs) > 0) THEN
      CALL ShowSevereError('IP: Already an Object called '//TRIM(SqueezedObject)//'. This definition ignored.',  &
         EchoInputFile,EPObjects)
      ! Error Condition
      ErrFlag=.true.
      ! Rest of Object has to be processed. Error condition will be caught
      ! at end
      ErrorsFound=.true.
    ENDIF
  ELSE
    ErrFlag=.true.
    ErrorsFound=.true.
  ENDIF

  NumObjectDefs=NumObjectDefs+1
  ObjectDef(NumObjectDefs)%Name=SqueezedObject
  ObjectDef(NumObjectDefs)%NumParams=0
  ObjectDef(NumObjectDefs)%NumAlpha=0
  ObjectDef(NumObjectDefs)%NumNumeric=0
  ObjectDef(NumObjectDefs)%NumFound=0

  AlphaorNumeric=.true.
  RequiredFields=.false.

  NumRangeChecks%MinMaxChk=.false.
  NumRangeChecks%WhichMinMax(1)=0
  NumRangeChecks%WhichMinMax(2)=0
  NumRangeChecks%MinMaxString(1)=Blank
  NumRangeChecks%MinMaxString(2)=Blank
  NumRangeChecks%MinMaxValue(1)=0.0
  NumRangeChecks%MinMaxValue(2)=0.0
  NumRangeChecks%Default=0.0
  NumRangeChecks%DefaultChk=.false.
  NumRangeChecks%DefAutoSize=.false.
  NumRangeChecks%DefAutoCalculate=.false.
  NumRangeChecks%FieldNumber=0
  NumRangeChecks%FieldName=Blank
  NumRangeChecks%AutoSizable=.false.
  NumRangeChecks%AutoSizeValue=DefAutoSizeValue
  NumRangeChecks%AutoCalculatable=.false.
  NumRangeChecks%AutoCalculateValue=DefAutoCalculateValue

  Count=0
  EndofObjectDef=.false.
  ! Parse rest of Object Definition

  DO WHILE (.not. EndofFile .and. .not. EndofObjectDef)

    IF (CurPos <= InputLineLength) THEN
      Pos=SCAN(InputLine(CurPos:InputLineLength),AlphaNum)
      IF (Pos > 0) then

        Count=Count+1
        RequiredField=.false.

        IF (Count > MaxANArgs) THEN   ! Reallocation
          ALLOCATE(TempAN(0:MaxANArgs+ObjectDefAllocInc))
          TempAN=.false.
          TempAN(0:MaxANArgs)=AlphaorNumeric
          DEALLOCATE(AlphaorNumeric)
          ALLOCATE(TempRqF(0:MaxANArgs+ObjectDefAllocInc))
          TempRqF=.false.
          TempRqF(0:MaxANArgs)=RequiredFields
          DEALLOCATE(RequiredFields)
          ALLOCATE(TempRtC(0:MaxANArgs+ObjectDefAllocInc))
          TempRtC=.false.
          TempRtC(0:MaxANArgs)=AlphRetainCase
          DEALLOCATE(AlphRetainCase)
          ALLOCATE(TempChecks(MaxANArgs+ObjectDefAllocInc))
          TempChecks(1:MaxANArgs)=NumRangeChecks(1:MaxANArgs)
          DEALLOCATE(NumRangeChecks)
          ALLOCATE(TempAFC(MaxANArgs+ObjectDefAllocInc))
          TempAFC=Blank
          TempAFC(1:MaxANArgs)=AlphFieldChecks
          DEALLOCATE(AlphFieldChecks)
          ALLOCATE(TempAFD(MaxANArgs+ObjectDefAllocInc))
          TempAFD=Blank
          TempAFD(1:MaxANArgs)=AlphFieldDefaults
          DEALLOCATE(AlphFieldDefaults)
          ALLOCATE(AlphaorNumeric(0:MaxANArgs+ObjectDefAllocInc))
          AlphaorNumeric=TempAN
          DEALLOCATE(TempAN)
          ALLOCATE(RequiredFields(0:MaxANArgs+ObjectDefAllocInc))
          RequiredFields=TempRqF
          DEALLOCATE(TempRqF)
          ALLOCATE(AlphRetainCase(0:MaxANArgs+ObjectDefAllocInc))
          AlphRetainCase=TempRtC
          DEALLOCATE(TempRtC)
          ALLOCATE(NumRangeChecks(MaxANArgs+ObjectDefAllocInc))
          NumRangeChecks=TempChecks
          DEALLOCATE(TempChecks)
          ALLOCATE(AlphFieldChecks(MaxANArgs+ObjectDefAllocInc))
          AlphFieldChecks=TempAFC
          DEALLOCATE(TempAFC)
          ALLOCATE(AlphFieldDefaults(MaxANArgs+ObjectDefAllocInc))
          AlphFieldDefaults=TempAFD
          DEALLOCATE(TempAFD)
          MaxANArgs=MaxANArgs+ObjectDefAllocInc
        ENDIF

        TargetChar=InputLine(CurPos+Pos-1:CurPos+Pos-1)

        IF (TargetChar == 'A' .or. TargetChar == 'a') THEN
          AlphaorNumeric(Count)=.true.
          ObjectDef(NumObjectDefs)%NumAlpha=ObjectDef(NumObjectDefs)%NumAlpha+1
        ELSE
          AlphaorNumeric(Count)=.false.
          ObjectDef(NumObjectDefs)%NumNumeric=ObjectDef(NumObjectDefs)%NumNumeric+1
          IF (FieldSet) NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%FieldName=CurrentFieldName
        ENDIF

      ELSE
        CALL ReadInputLine(IDDFile,CurPos,BlankLine,InputLineLength,EndofFile,  &
                           MinMax=MinMax,WhichMinMax=WhichMinMax,MinMaxString=MinMaxString,  &
                           Value=Value,Default=Default,DefString=AlphDefaultString,AutoSizable=AutoSize, &
                           AutoCalculatable=AutoCalculate,RetainCase=RetainCaseFlag,ErrorsFound=ErrorsFoundFlag)
        IF (.not. AlphaorNumeric(Count)) THEN
          ! only record for numeric fields
          IF (MinMax) THEN
            NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%MinMaxChk=.true.
            NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%FieldNumber=Count
            IF (WhichMinMax <= 2) THEN   !=0 (none/invalid), =1 \min, =2 \min>, =3 \max, =4 \max<
              NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%WhichMinMax(1)=WhichMinMax
              NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%MinMaxString(1)=MinMaxString
              NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%MinMaxValue(1)=Value
            ELSE
              NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%WhichMinMax(2)=WhichMinMax
              NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%MinMaxString(2)=MinMaxString
              NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%MinMaxValue(2)=Value
            ENDIF
          ENDIF   ! End Min/Max
          IF (Default) THEN
            NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%DefaultChk=.true.
            NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%Default=Value
!            IF (AlphDefaultString == 'AUTOSIZE') NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%DefAutoSize=.true.
!            IF (AlphDefaultString == 'AUTOCALCULATE')  NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%DefAutoCalculate=.true.
          ENDIF
!          IF (AutoSize) THEN
!            NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%AutoSizable=.true.
!            NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%AutoSizeValue=Value
!          ENDIF
!          IF (AutoCalculate) THEN
!            NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%AutoCalculatable=.true.
!            NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%AutoCalculateValue=Value
!          ENDIF
        ELSE  ! Alpha Field
          IF (Default) THEN
            AlphFieldDefaults(ObjectDef(NumObjectDefs)%NumAlpha)=AlphDefaultString
          ENDIF
        ENDIF
        IF (ErrorsFoundFlag) THEN
          ErrFlag=.true.
          ErrorsFoundFlag=.false.
        ENDIF
        IF (RequiredField) THEN
          RequiredFields(Count)=.true.
          MinimumNumberOfFields=MAX(Count,MinimumNumberOfFields)
        ENDIF
!        IF (RetainCaseFlag) THEN
!          AlphRetainCase(Count)=.true.
!        ENDIF
        CYCLE
      ENDIF

      !  For the moment dont care about descriptions on each object
      IF (CurPos <= InputLineLength) THEN
        CurPos=CurPos+Pos
        Pos=SCAN(InputLine(CurPos:InputLineLength),',;')
        IF (Pos == 0) THEN
          CALL ShowSevereError('IP: IDD line~'//TRIM(IPTrimSigDigits(NumLines))//' , or ; expected on this line'//  &
               ',position="'//InputLine(CurPos:InputLineLength)//'"',EchoInputFile,EPObjects)
          ErrFlag=.true.
          ErrorsFound=.true.
        ENDIF
        IF (InputLine(InputLineLength:InputLineLength) /= '\') THEN
          CALL ShowWarningError('IP: IDD line~'//TRIM(IPTrimSigDigits(NumLines))//' \ expected on this line',  &
             EchoInputFile,EPObjects)
        ENDIF
      ELSE
        CALL ReadInputLine(IDDFile,CurPos,BlankLine,InputLineLength,EndofFile)
        IF (BlankLine .or. EndofFile) CYCLE
        Pos=SCAN(InputLine(CurPos:InputLineLength),',;')
      ENDIF
    ELSE
      CALL ReadInputLine(IDDFile,CurPos,BlankLine,InputLineLength,EndofFile)
      CYCLE
    ENDIF

    IF (Pos <= 0) THEN
                   ! must be time to read another line
      CALL ReadInputLine(IDDFile,CurPos,BlankLine,InputLineLength,EndofFile)
      IF (BlankLine .or. EndofFile) CYCLE
    ELSE
      IF (InputLine(CurPos+Pos-1:CurPos+Pos-1) == ';') THEN
        EndofObjectDef=.true.
      ENDIF
      CurPos=CurPos+Pos
    ENDIF

  END DO

  ! Reached end of object def but there may still be more \ lines to parse....
  ! Goes until next object is encountered ("not blankline") or end of IDDFile
  ! If last object is not numeric, then exit immediately....
  BlankLine=.true.
  DO WHILE (BlankLine .and. .not.EndofFile)
  ! It's a numeric object as last one...
    CALL ReadInputLine(IDDFile,CurPos,BlankLine,InputLineLength,EndofFile,  &
                       MinMax=MinMax,WhichMinMax=WhichMinMax,MinMaxString=MinMaxString,  &
                       Value=Value,Default=Default,DefString=AlphDefaultString,AutoSizable=AutoSize, &
                       AutoCalculatable=AutoCalculate,RetainCase=RetainCaseFlag,ErrorsFound=ErrorsFoundFlag)
    IF (MinMax) THEN
      NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%MinMaxChk=.true.
      NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%FieldNumber=Count
      IF (WhichMinMax <= 2) THEN   !=0 (none/invalid), =1 \min, =2 \min>, =3 \max, =4 \max<
        NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%WhichMinMax(1)=WhichMinMax
        NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%MinMaxString(1)=MinMaxString
        NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%MinMaxValue(1)=Value
      ELSE
        NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%WhichMinMax(2)=WhichMinMax
        NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%MinMaxString(2)=MinMaxString
        NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%MinMaxValue(2)=Value
      ENDIF
    ENDIF
    IF (Default .and. .not. AlphaorNumeric(Count)) THEN
      NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%DefaultChk=.true.
      NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%Default=Value
!      IF (AlphDefaultString == 'AUTOSIZE') NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%DefAutoSize=.true.
!      IF (AlphDefaultString == 'AUTOCALCULATE') NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%DefAutoCalculate=.true.
    ELSEIF (Default .and. AlphaorNumeric(Count)) THEN
      AlphFieldDefaults(ObjectDef(NumObjectDefs)%NumAlpha)=AlphDefaultString
    ENDIF
!    IF (AutoSize) THEN
!      NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%AutoSizable=.true.
!      NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%AutoSizeValue=Value
!    ENDIF
!    IF (AutoCalculate) THEN
!      NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%AutoCalculatable=.true.
!      NumRangeChecks(ObjectDef(NumObjectDefs)%NumNumeric)%AutoCalculateValue=Value
!    ENDIF
    IF (ErrorsFoundFlag) THEN
      ErrFlag=.true.
      ErrorsFoundFlag=.false.
    ENDIF
  ENDDO

  IF (.not. BlankLine) THEN
    BACKSPACE(Unit=IDDFile)
    EchoInputLine=.false.
  ENDIF

  IF (RequiredField) THEN
    RequiredFields(Count)=.true.
!    MinimumNumberOfFields=MAX(Count,MinimumNumberOfFields)
  ENDIF
!  IF (RetainCaseFlag) THEN
!    AlphRetainCase(Count)=.true.
!  ENDIF


  ObjectDef(NumObjectDefs)%NumParams=Count  ! Also the total of ObjectDef(..)%NumAlpha+ObjectDef(..)%NumNumeric
  ObjectDef(NumObjectDefs)%MinNumFields=MinimumNumberOfFields
!  IF (ObsoleteObject) THEN
!    ALLOCATE(TempAFD(NumObsoleteObjects+1))
!    IF (NumObsoleteObjects > 0) THEN
!      TempAFD(1:NumObsoleteObjects)=ObsoleteObjectsRepNames
!    ENDIF
!    TempAFD(NumObsoleteObjects+1)=ReplacementName
!    DEALLOCATE(ObsoleteObjectsRepNames)
!    NumObsoleteObjects=NumObsoleteObjects+1
!    ALLOCATE(ObsoleteObjectsRepNames(NumObsoleteObjects))
!    ObsoleteObjectsRepNames=TempAFD
!    ObjectDef(NumObjectDefs)%ObsPtr=NumObsoleteObjects
!    DEALLOCATE(TempAFD)
!  ENDIF
  IF (RequiredObject) THEN
    ObjectDef(NumObjectDefs)%RequiredObject=.true.
  ENDIF
  IF (UniqueObject) THEN
    ObjectDef(NumObjectDefs)%UniqueObject=.true.
  ENDIF
!  IF (ExtensibleObject) THEN
!    ObjectDef(NumObjectDefs)%ExtensibleObject=.true.
!    ObjectDef(NumObjectDefs)%ExtensibleNum=ExtensibleNumFields
!  ENDIF

  MaxAlphaArgsFound=MAX(MaxAlphaArgsFound,ObjectDef(NumObjectDefs)%NumAlpha)
  MaxNumericArgsFound=MAX(MaxNumericArgsFound,ObjectDef(NumObjectDefs)%NumNumeric)
  ALLOCATE(ObjectDef(NumObjectDefs)%AlphaorNumeric(Count))
  ObjectDef(NumObjectDefs)%AlphaorNumeric=AlphaorNumeric(1:Count)
  ALLOCATE(ObjectDef(NumObjectDefs)%AlphRetainCase(Count))
  ObjectDef(NumObjectDefs)%AlphRetainCase=AlphRetainCase(1:Count)
  ALLOCATE(ObjectDef(NumObjectDefs)%NumRangeChks(ObjectDef(NumObjectDefs)%NumNumeric))
  IF (ObjectDef(NumObjectDefs)%NumNumeric > 0) THEN
    ObjectDef(NumObjectDefs)%NumRangeChks=NumRangeChecks(1:ObjectDef(NumObjectDefs)%NumNumeric)
  ENDIF
  ALLOCATE(ObjectDef(NumObjectDefs)%AlphFieldChks(ObjectDef(NumObjectDefs)%NumAlpha))
  IF (ObjectDef(NumObjectDefs)%NumAlpha > 0) THEN
    ObjectDef(NumObjectDefs)%AlphFieldChks=AlphFieldChecks(1:ObjectDef(NumObjectDefs)%NumAlpha)
  ENDIF
  ALLOCATE(ObjectDef(NumObjectDefs)%AlphFieldDefs(ObjectDef(NumObjectDefs)%NumAlpha))
  IF (ObjectDef(NumObjectDefs)%NumAlpha > 0) THEN
    ObjectDef(NumObjectDefs)%AlphFieldDefs=AlphFieldDefaults(1:ObjectDef(NumObjectDefs)%NumAlpha)
  ENDIF
  ALLOCATE(ObjectDef(NumObjectDefs)%ReqField(Count))
  ObjectDef(NumObjectDefs)%ReqField=RequiredFields(1:Count)
  DO Count=1,ObjectDef(NumObjectDefs)%NumNumeric
    IF (ObjectDef(NumObjectDefs)%NumRangeChks(Count)%MinMaxChk) THEN
    ! Checking MinMax Range (min vs. max and vice versa)
      MinMaxError=.false.
      ! check min against max
      IF (ObjectDef(NumObjectDefs)%NumRangeChks(Count)%WhichMinMax(1) == 1) THEN
        ! min
        Value=ObjectDef(NumObjectDefs)%NumRangeChks(Count)%MinMaxValue(1)
        IF (ObjectDef(NumObjectDefs)%NumRangeChks(Count)%WhichMinMax(2) == 3) THEN
          IF (Value > ObjectDef(NumObjectDefs)%NumRangeChks(Count)%MinMaxValue(2)) MinMaxError=.true.
        ELSEIF (ObjectDef(NumObjectDefs)%NumRangeChks(Count)%WhichMinMax(2) == 4) THEN
          IF (Value == ObjectDef(NumObjectDefs)%NumRangeChks(Count)%MinMaxValue(2)) MinMaxError=.true.
        ENDIF
      ELSEIF (ObjectDef(NumObjectDefs)%NumRangeChks(Count)%WhichMinMax(1) == 2) THEN
        ! min>
        Value=ObjectDef(NumObjectDefs)%NumRangeChks(Count)%MinMaxValue(1) + rTinyValue  ! infintesimally bigger than min
        IF (ObjectDef(NumObjectDefs)%NumRangeChks(Count)%WhichMinMax(2) == 3) THEN
          IF (Value > ObjectDef(NumObjectDefs)%NumRangeChks(Count)%MinMaxValue(2)) MinMaxError=.true.
        ELSEIF (ObjectDef(NumObjectDefs)%NumRangeChks(Count)%WhichMinMax(2) == 4) THEN
          IF (Value == ObjectDef(NumObjectDefs)%NumRangeChks(Count)%MinMaxValue(2)) MinMaxError=.true.
        ENDIF
      ENDIF
      ! check max against min
      IF (ObjectDef(NumObjectDefs)%NumRangeChks(Count)%WhichMinMax(2) == 3) THEN
        ! max
        Value=ObjectDef(NumObjectDefs)%NumRangeChks(Count)%MinMaxValue(2)
        ! Check max value against min
        IF (ObjectDef(NumObjectDefs)%NumRangeChks(Count)%WhichMinMax(1) == 1) THEN
          IF (Value < ObjectDef(NumObjectDefs)%NumRangeChks(Count)%MinMaxValue(1)) MinMaxError=.true.
        ELSEIF (ObjectDef(NumObjectDefs)%NumRangeChks(Count)%WhichMinMax(1) == 2) THEN
          IF (Value == ObjectDef(NumObjectDefs)%NumRangeChks(Count)%MinMaxValue(1)) MinMaxError=.true.
        ENDIF
      ELSEIF (ObjectDef(NumObjectDefs)%NumRangeChks(Count)%WhichMinMax(2) == 4) THEN
        ! max<
        Value=ObjectDef(NumObjectDefs)%NumRangeChks(Count)%MinMaxValue(2) - rTinyValue  ! infintesimally bigger than min
        IF (ObjectDef(NumObjectDefs)%NumRangeChks(Count)%WhichMinMax(1) == 1) THEN
          IF (Value < ObjectDef(NumObjectDefs)%NumRangeChks(Count)%MinMaxValue(1)) MinMaxError=.true.
        ELSEIF (ObjectDef(NumObjectDefs)%NumRangeChks(Count)%WhichMinMax(1) == 2) THEN
          IF (Value == ObjectDef(NumObjectDefs)%NumRangeChks(Count)%MinMaxValue(1)) MinMaxError=.true.
        ENDIF
      ENDIF
      ! check if error condition
      IF (MinMaxError) THEN
        !  Error stated min is not in range with stated max
        WRITE(MinMaxString,*) ObjectDef(NumObjectDefs)%NumRangeChks(Count)%FieldNumber
        MinMaxString=ADJUSTL(MinMaxString)
        CALL ShowSevereError('IP: IDD: Field #'//TRIM(MinMaxString)//' conflict in Min/Max specifications/values, in class='//  &
                             TRIM(ObjectDef(NumObjectDefs)%Name),EchoInputFile,EPObjects)
        ErrFlag=.true.
      ENDIF
    ENDIF
    IF (ObjectDef(NumObjectDefs)%NumRangeChks(Count)%DefaultChk) THEN
    ! Check Default against MinMaxRange
    !  Don't check when default is autosize...
!      IF (ObjectDef(NumObjectDefs)%NumRangeChks(Count)%Autosizable .and.   &
!          ObjectDef(NumObjectDefs)%NumRangeChks(Count)%DefAutoSize) CYCLE
!      IF (ObjectDef(NumObjectDefs)%NumRangeChks(Count)%Autocalculatable .and.   &
!          ObjectDef(NumObjectDefs)%NumRangeChks(Count)%DefAutoCalculate) CYCLE
      MinMaxError=.false.
      Value=ObjectDef(NumObjectDefs)%NumRangeChks(Count)%Default
      IF (ObjectDef(NumObjectDefs)%NumRangeChks(Count)%WhichMinMax(1) == 1) THEN
        IF (Value < ObjectDef(NumObjectDefs)%NumRangeChks(Count)%MinMaxValue(1)) MinMaxError=.true.
      ELSEIF (ObjectDef(NumObjectDefs)%NumRangeChks(Count)%WhichMinMax(1) == 2) THEN
        IF (Value <= ObjectDef(NumObjectDefs)%NumRangeChks(Count)%MinMaxValue(1)) MinMaxError=.true.
      ENDIF
      IF (ObjectDef(NumObjectDefs)%NumRangeChks(Count)%WhichMinMax(2) == 3) THEN
        IF (Value > ObjectDef(NumObjectDefs)%NumRangeChks(Count)%MinMaxValue(2)) MinMaxError=.true.
      ELSEIF (ObjectDef(NumObjectDefs)%NumRangeChks(Count)%WhichMinMax(2) == 4) THEN
        IF (Value >= ObjectDef(NumObjectDefs)%NumRangeChks(Count)%MinMaxValue(2)) MinMaxError=.true.
      ENDIF
      IF (MinMaxError) THEN
        !  Error stated default is not in min/max range
        WRITE(MinMaxString,*) ObjectDef(NumObjectDefs)%NumRangeChks(Count)%FieldNumber
        MinMaxString=ADJUSTL(MinMaxString)
        CALL ShowSevereError('IP: IDD: Field #'//TRIM(MinMaxString)//' default is invalid for Min/Max values, in class='//  &
                             TRIM(ObjectDef(NumObjectDefs)%Name),EchoInputFile,EPObjects)
        ErrFlag=.true.
      ENDIF
    ENDIF
  ENDDO

  IF (ErrFlag) THEN
    CALL ShowContinueError('IP: Errors occured in ObjectDefinition for Class='//TRIM(ObjectDef(NumObjectDefs)%Name)// &
                           ', Object not available for IDF processing.',EchoInputFile,EPObjects)
    DEALLOCATE(ObjectDef(NumObjectDefs)%AlphaorNumeric)
    DEALLOCATE(ObjectDef(NumObjectDefs)%NumRangeChks)
    DEALLOCATE(ObjectDef(NumObjectDefs)%AlphFieldChks)
    DEALLOCATE(ObjectDef(NumObjectDefs)%AlphFieldDefs)
    DEALLOCATE(ObjectDef(NumObjectDefs)%ReqField)
    DEALLOCATE(ObjectDef(NumObjectDefs)%AlphRetainCase)
    NumObjectDefs=NumObjectDefs-1
    ErrorsFound=.true.
  ENDIF

  RETURN

END SUBROUTINE AddObjectDefandParse

SUBROUTINE ProcessInputDataFile

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   August 1997
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine processes input data file for EnergyPlus.  Each "record" is
          ! parsed into the LineItem data structure and, if okay, put into the
          ! IDFRecords data structure.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
   TYPE (FileSectionsDefinition), ALLOCATABLE :: TempSectionsonFile(:)   ! Used during reallocation procedure
   TYPE (LineDefinition), ALLOCATABLE :: TempIDFRecords(:)   ! Used during reallocation procedure

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:

   LOGICAL :: EndofFile = .false.
   LOGICAL BlankLine
   INTEGER Pos

   MaxIDFRecords=ObjectsIDFAllocInc
   NumIDFRecords=0
   MaxIDFSections=SectionsIDFAllocInc
   NumIDFSections=0

   ALLOCATE (SectionsonFile(MaxIDFSections))
   SectionsonFile%Name=' '        ! Name of this section
   SectionsonFile%FirstRecord=0   ! Record number of first object in section
   SectionsonFile%LastRecord=0    ! Record number of last object in section
   ALLOCATE (IDFRecords(MaxIDFRecords))
   IDFRecords%Name=' '          ! Object name for this record
   IDFRecords%NumAlphas=0       ! Number of alphas on this record
   IDFRecords%NumNumbers=0      ! Number of numbers on this record
   ALLOCATE (LineItem%Numbers(MaxNumericArgsFound))
   ALLOCATE (LineItem%NumBlank(MaxNumericArgsFound))
   ALLOCATE (LineItem%Alphas(MaxAlphaArgsFound))
   ALLOCATE (LineItem%AlphBlank(MaxAlphaArgsFound))
   EndofFile=.false.

   DO WHILE (.not. EndofFile)
     CALL ReadInputLine(IDFFile,Pos,BlankLine,InputLineLength,EndofFile)
     IF (BlankLine .or. EndofFile) CYCLE
     Pos=SCAN(InputLine,',;')
     If (Pos /= 0) then
       If (InputLine(Pos:Pos) == ';') then
         CALL ValidateSection(InputLine(1:Pos-1),NumLines)
         IF (NumIDFSections == MaxIDFSections) THEN
           ALLOCATE (TempSectionsonFile(MaxIDFSections+SectionsIDFAllocInc))
           TempSectionsonFile%Name=' '        ! Name of this section
           TempSectionsonFile%FirstRecord=0   ! Record number of first object in section
           TempSectionsonFile%LastRecord=0    ! Record number of last object in section
           TempSectionsonFile(1:MaxIDFSections)=SectionsonFile
           DEALLOCATE (SectionsonFile)
           ALLOCATE (SectionsonFile(MaxIDFSections+SectionsIDFAllocInc))
           SectionsonFile=TempSectionsonFile
           DEALLOCATE (TempSectionsonFile)
           MaxIDFSections=MaxIDFSections+SectionsIDFAllocInc
         ENDIF
       else
         CALL ValidateObjectandParse(InputLine(1:Pos-1),Pos,EndofFile)
         IF (NumIDFRecords == MaxIDFRecords) THEN
           ALLOCATE(TempIDFRecords(MaxIDFRecords+ObjectsIDFAllocInc))
           TempIDFRecords%Name=' '          ! Object name for this record
           TempIDFRecords%NumAlphas=0       ! Number of alphas on this record
           TempIDFRecords%NumNumbers=0      ! Number of numbers on this record
           TempIDFRecords(1:MaxIDFRecords)=IDFRecords
           DEALLOCATE(IDFRecords)
           ALLOCATE(IDFRecords(MaxIDFRecords+ObjectsIDFAllocInc))
           IDFRecords=TempIDFRecords
           DEALLOCATE(TempIDFRecords)
           MaxIDFRecords=MaxIDFRecords+ObjectsIDFAllocInc
         ENDIF
       endif
     else
       !Error condition, no , or ; on first line
       CALL ShowMessage('IP: IDF Line~'//TRIM(IPTrimSigDigits(NumLines))//' '//TRIM(InputLine),EPObjects)
       CALL ShowSevereError(', or ; expected on this line',EchoInputFile,EPObjects)
     endif

   END DO

!   IF (NumIDFSections > 0) THEN
!     SectionsonFile(NumIDFSections)%LastRecord=NumIDFRecords
!   ENDIF

   IF (NumIDFRecords > 0) THEN
     DO Pos=1,NumObjectDefs
       IF (ObjectDef(Pos)%RequiredObject .and. ObjectDef(Pos)%NumFound == 0) THEN
         CALL ShowSevereError('IP: No items found for Required Object='//TRIM(ObjectDef(Pos)%Name),EPObjects)
         NumMiscErrorsFound=NumMiscErrorsFound+1
       ENDIF
     ENDDO
   ENDIF

   RETURN

END SUBROUTINE ProcessInputDataFile

SUBROUTINE ValidateSection(ProposedSection,LineNo)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   September 1997
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine validates the section from the input data file
          ! with the list of objects from the data dictionary file.

          ! METHODOLOGY EMPLOYED:
          ! A "squeezed" string is formed and checked against the list of
          ! sections.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  CHARACTER(len=*), INTENT(IN) :: ProposedSection
  INTEGER, INTENT(IN)          :: LineNo


          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  CHARACTER(len=MaxSectionNameLength) SqueezedSection
  INTEGER Found
  TYPE (SectionsDefinition), ALLOCATABLE :: TempSectionDef(:)  ! Like SectionDef, used during Re-allocation
  INTEGER OFound

  SqueezedSection=MakeUPPERCase(ADJUSTL(ProposedSection))
  IF (LEN_TRIM(ADJUSTL(ProposedSection)) > MaxSectionNameLength) THEN
    CALL ShowWarningError('IP: Section length exceeds maximum, will be truncated='//TRIM(ProposedSection),  &
       EchoInputFile,EPObjects)
    CALL ShowContinueError('Will be processed as Section='//TRIM(SqueezedSection),EchoInputFile,EPObjects)
  ENDIF
  IF (SqueezedSection(1:3) /= 'END') THEN
    Found=FindIteminList(SqueezedSection,SectionDef%Name,NumSectionDefs)
    IF (Found == 0) THEN
      ! Make sure this Section not an object name
      IF (SortedIDD) THEN
!        OFound=FindItemInSortedList(SqueezedSection,ListOfObjects,NumObjectDefs)
!        IF (OFound /= 0) OFound=iListOfObjects(OFound)
      ELSE
        OFound=FindItemInList(SqueezedSection,ListOfObjects,NumObjectDefs)
      ENDIF
      IF (OFound /= 0) THEN
        CALL AddRecordFromSection(OFound)
      ELSEIF (NumSectionDefs == MaxSectionDefs) THEN
        ALLOCATE (TempSectionDef(MaxSectionDefs+SectionDefAllocInc))
        TempSectionDef%Name=' '
        TempSectionDef%NumFound=0
        TempSectionDef(1:MaxSectionDefs)=SectionDef
        DEALLOCATE (SectionDef)
        ALLOCATE (SectionDef(MaxSectionDefs+SectionDefAllocInc))
        SectionDef=TempSectionDef
        DEALLOCATE (TempSectionDef)
        MaxSectionDefs=MaxSectionDefs+SectionDefAllocInc
      ENDIF
      NumSectionDefs=NumSectionDefs+1
      SectionDef(NumSectionDefs)%Name=SqueezedSection
      SectionDef(NumSectionDefs)%NumFound=1
      ! Add to "Sections on file" if appropriate
      IF (.not. ProcessingIDD) THEN
        NumIDFSections=NumIDFSections+1
        SectionsonFile(NumIDFSections)%Name=SqueezedSection
        SectionsonFile(NumIDFSections)%FirstRecord=NumIDFRecords
        SectionsonFile(NumIDFSections)%FirstLineNo=LineNo
      ENDIF
    ELSE
!      IF (NumIDFSections > 0) THEN
!        SectionsonFile(NumIDFSections)%LastRecord=NumIDFRecords
!      ENDIF
      SectionDef(Found)%NumFound=SectionDef(Found)%NumFound+1
      IF (.not. ProcessingIDD) THEN
        NumIDFSections=NumIDFSections+1
        SectionsonFile(NumIDFSections)%Name=SqueezedSection
        SectionsonFile(NumIDFSections)%FirstRecord=NumIDFRecords
        SectionsonFile(NumIDFSections)%FirstLineNo=LineNo
      ENDIF
    ENDIF
  ELSE  ! End ...
    IF (.not. ProcessingIDD) THEN
      SqueezedSection=SqueezedSection(4:)
      SqueezedSection=ADJUSTL(SqueezedSection)
      DO Found=NumIDFSections,1,-1
        IF (.not. SameString(SectionsonFile(Found)%Name,SqueezedSection)) CYCLE
        SectionsonFile(Found)%LastRecord=NumIDFRecords
      ENDDO
    ENDIF
  ENDIF

  RETURN

END SUBROUTINE ValidateSection

SUBROUTINE ValidateObjectandParse(ProposedObject,CurPos,EndofFile)
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   September 1997
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine validates the proposed object from the IDF and then
          ! parses it, putting it into the internal InputProcessor Data structure.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  CHARACTER(len=*), INTENT(IN) :: ProposedObject
  INTEGER, INTENT(INOUT) :: CurPos
  LOGICAL, INTENT(INOUT) :: EndofFile

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  CHARACTER(len=MaxObjectNameLength) SqueezedObject
  CHARACTER(len=MaxAlphaArgLength) SqueezedArg
  INTEGER Found
  INTEGER NumArg
  INTEGER NumArgExpected
  INTEGER NumAlpha
  INTEGER NumNumeric
  INTEGER Pos
  LOGICAL EndofObject
  LOGICAL BlankLine
  LOGICAL,SAVE  :: ErrFlag=.false.
  INTEGER LenLeft
  INTEGER Count
  CHARACTER(len=20) FieldString
  CHARACTER(len=MaxObjectNameLength) FieldNameString
  CHARACTER(len=300) Message
  CHARACTER(len=500000), SAVE :: LineBuf
  INTEGER, SAVE :: StartLine
  INTEGER, SAVE :: NumConxLines
  INTEGER, SAVE :: CurLines
  INTEGER, SAVE :: CurBufLen
  CHARACTER(len=25) :: String
  LOGICAL IDidntMeanIt
  LOGICAL TestingObject
  LOGICAL TransitionDefer
  INTEGER TFound
!  INTEGER NFound
  INTEGER, EXTERNAL :: FindNonSpace
  INTEGER NextChr
  CHARACTER(len=25) :: String1


  SqueezedObject=MakeUPPERCase(ADJUSTL(ProposedObject))
  IF (LEN_TRIM(ADJUSTL(ProposedObject)) > MaxObjectNameLength) THEN
    CALL ShowWarningError('IP: Object name length exceeds maximum, will be truncated='//TRIM(ProposedObject),  &
       EchoInputFile,EPObjects)
    CALL ShowContinueError('Will be processed as Object='//TRIM(SqueezedObject),EchoInputFile,EPObjects)
  ENDIF

  TestingObject=.true.
  TransitionDefer=.false.
  DO WHILE (TestingObject)
    ErrFlag=.false.
    IDidntMeanIt=.false.
    IF (SortedIDD) THEN
!      Found=FindIteminSortedList(SqueezedObject,ListofObjects,NumObjectDefs)
!      IF (Found /= 0) Found=iListofObjects(Found)
    ELSE
      Found=FindIteminList(SqueezedObject,ListofObjects,NumObjectDefs)
    ENDIF
!    IF (Found /= 0) THEN
!      IF (ObjectDef(Found)%ObsPtr > 0) THEN
!        TFound=FindItemInList(SqueezedObject,RepObjects%OldName,NumSecretObjects)
!        IF (TFound /= 0) THEN
!          IF (RepObjects(TFound)%Transitioned) THEN
!            IF (.not. RepObjects(TFound)%Used)  &
!               CALL ShowWarningError('IP: Objects="'//TRIM(ADJUSTL(ProposedObject))//  &
!                  '" are being transitioned to this object="'//  &
!                  TRIM(RepObjects(TFound)%NewName)//'"')
!            RepObjects(TFound)%Used=.true.
!            IF (SortedIDD) THEN
!              Found=FindIteminSortedList(SqueezedObject,ListofObjects,NumObjectDefs)
!              IF (Found /= 0) Found=iListofObjects(Found)
!            ELSE
!              Found=FindIteminList(SqueezedObject,ListofObjects,NumObjectDefs)
!            ENDIF
!          ELSEIF (RepObjects(TFound)%TransitionDefer) THEN
!            IF (.not. RepObjects(TFound)%Used)  &
!               CALL ShowWarningError('IP: Objects="'//TRIM(ADJUSTL(ProposedObject))//  &
!                  '" are being transitioned to this object="'//  &
!                  TRIM(RepObjects(TFound)%NewName)//'"')
!            RepObjects(TFound)%Used=.true.
!            IF (SortedIDD) THEN
!              Found=FindIteminSortedList(SqueezedObject,ListofObjects,NumObjectDefs)
!              IF (Found /= 0) Found=iListofObjects(Found)
!            ELSE
!              Found=FindIteminList(SqueezedObject,ListofObjects,NumObjectDefs)
!            ENDIF
!            TransitionDefer=.true.
!          ELSE
!            Found=0    ! being handled differently for this obsolete object
!          ENDIF
!        ENDIF
!      ENDIF
!    ENDIF

    TestingObject=.false.
    IF (Found == 0) THEN
!      ! Check to see if it's a "secret" object
!      Found=FindItemInList(SqueezedObject,RepObjects%OldName,NumSecretObjects)
!      IF (Found == 0) THEN
        CALL ShowSevereError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
           ' Did not find "'//TRIM(ADJUSTL(ProposedObject))//'" in list of Objects',EchoInputFile,EPObjects)
!        ! Will need to parse to next ;
        ErrFlag=.true.
!      ELSEIF (RepObjects(Found)%Deleted) THEN
!        IF (.not. RepObjects(Found)%Used) THEN
!          CALL ShowWarningError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
!               ' Objects="'//TRIM(ADJUSTL(ProposedObject))//'" have been deleted from the IDD.  Will be ignored.')
!          RepObjects(Found)%Used=.true.
!        ENDIF
!        IDidntMeanIt=.true.
!        ErrFlag=.true.
!        Found=0
!      ELSEIF (RepObjects(Found)%TransitionDefer) THEN
!
!      ELSE ! This name is replaced with something else
!        IF (.not. RepObjects(Found)%Used) THEN
!          IF (.not. RepObjects(Found)%Transitioned) THEN
!            CALL ShowWarningError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
!                 ' Objects="'//TRIM(ADJUSTL(ProposedObject))//'" are being replaced with this object="'//  &
!                 TRIM(RepObjects(Found)%NewName)//'"')
!            RepObjects(Found)%Used=.true.
!            SqueezedObject=RepObjects(Found)%NewName
!            TestingObject=.true.
!          ELSE
!            CALL ShowWarningError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
!                 ' Objects="'//TRIM(ADJUSTL(ProposedObject))//'" are being transitioned to this object="'//  &
!                 TRIM(RepObjects(Found)%NewName)//'"')
!            RepObjects(Found)%Used=.true.
!            IF (SortedIDD) THEN
!              Found=FindIteminSortedList(SqueezedObject,ListofObjects,NumObjectDefs)
!              IF (Found /= 0) Found=iListofObjects(Found)
!            ELSE
!              Found=FindIteminList(SqueezedObject,ListofObjects,NumObjectDefs)
!            ENDIF
!          ENDIF
!        ELSEIF (.not. RepObjects(Found)%Transitioned) THEN
!          SqueezedObject=RepObjects(Found)%NewName
!          TestingObject=.true.
!        ELSE
!          IF (SortedIDD) THEN
!            Found=FindIteminSortedList(SqueezedObject,ListofObjects,NumObjectDefs)
!            IF (Found /= 0) Found=iListofObjects(Found)
!          ELSE
!            Found=FindIteminList(SqueezedObject,ListofObjects,NumObjectDefs)
!          ENDIF
!        ENDIF
!      ENDIF
    ELSE

    ! Start Parsing the Object according to definition

      ErrFlag=.false.
      LineItem%Name=SqueezedObject
      LineItem%Alphas=' '
      LineItem%AlphBlank=.false.
      LineItem%NumAlphas=0
      LineItem%Numbers=0.0
      LineItem%NumNumbers=0
      LineItem%NumBlank=.false.
      LineItem%ObjectDefPtr=Found
      NumArgExpected=ObjectDef(Found)%NumParams
      ObjectDef(Found)%NumFound=ObjectDef(Found)%NumFound+1
      IF (ObjectDef(Found)%UniqueObject .and. ObjectDef(Found)%NumFound > 1) THEN
        CALL ShowSevereError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
             ' Multiple occurrences of Unique Object='//TRIM(ADJUSTL(ProposedObject)),EPObjects)
        NumMiscErrorsFound=NumMiscErrorsFound+1
      ENDIF
!      IF (ObjectDef(Found)%ObsPtr > 0) THEN
!        TFound=FindItemInList(SqueezedObject,RepObjects%OldName,NumSecretObjects)
!        IF (TFound == 0) THEN
!          CALL ShowWarningError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
!                      ' Obsolete object='//TRIM(ADJUSTL(ProposedObject))//  &
!                      ', encountered.  Should be replaced with new object='//  &
!                      TRIM(ObsoleteObjectsRepNames(ObjectDef(Found)%ObsPtr)))
!        ELSEIF (.not. RepObjects(TFound)%Used .and. RepObjects(TFound)%Transitioned) THEN
!          CALL ShowWarningError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
!                      ' Objects="'//TRIM(ADJUSTL(ProposedObject))//'" are being transitioned to this object="'//  &
!                      TRIM(RepObjects(TFound)%NewName)//'"')
!          RepObjects(TFound)%Used=.true.
!        ENDIF
!      ENDIF
    ENDIF
  ENDDO

  NumArg=0
  NumAlpha=0
  NumNumeric=0
  EndofObject=.false.
  CurPos=CurPos+1

  !  Keep context buffer in case of errors
  LineBuf=Blank
  IF (.not. ALLOCATED(LineBufLen)) THEN
    ALLOCATE(LineBufLen(MaxNumericArgsFound+MaxAlphaArgsFound))
  ENDIF
  LineBufLen=0
  NumConxLines=0
  StartLine=NumLines
  LineBuf(1:InputLineLength)=InputLine
  NumConxLines=1
  LineBufLen(NumConxLines)=InputLineLength
  CurLines=NumLines
  CurBufLen=InputLineLength

  DO WHILE (.not. EndofFile .and. .not. EndofObject)
    IF (CurLines /= NumLines) THEN
      NumConxLines=NumConxLines+1
      LineBuf(CurBufLen+1:CurBufLen+1+InputLineLength)=InputLine
      LineBufLen(NumConxLines)=InputLineLength
      CurBufLen=CurBufLen+InputLineLength
      CurLines=NumLines
    ENDIF
    IF (CurPos <= InputLineLength) THEN
      Pos=SCAN(InputLine(CurPos:InputLineLength),',;')
!      If (Pos == 0) CurPos=InputLineLength+1
      IF (Pos == 0) THEN
!        IF (InputLine(InputLineLength:InputLineLength) == '!' .or.  &
!            InputLine(InputLineLength:InputLineLength) == '\') THEN
        IF (InputLine(InputLineLength:InputLineLength) == '!') THEN
          LenLeft=LEN_TRIM(InputLine(CurPos:InputLineLength-1))
        ELSE
          LenLeft=LEN_TRIM(InputLine(CurPos:InputLineLength))
        ENDIF
        IF (LenLeft == 0) THEN
          CurPos=InputLineLength+1
          CYCLE
        ELSE
!          IF (InputLine(InputLineLength:InputLineLength) == '!' .or.  &
!              InputLine(InputLineLength:InputLineLength) == '\') THEN
          IF (InputLine(InputLineLength:InputLineLength) == '!') THEN
            Pos=InputLineLength-CurPos+1
            CALL DumpCurrentLineBuffer(StartLine,NumConxLines,LineBuf,LineBufLen)
            CALL ShowWarningError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
                         ' Comma being inserted after:"'//InputLine(CurPos:InputLineLength-1)//   &
                         '" in Object='//TRIM(SqueezedObject),EchoInputFile,EPObjects)
          ELSE
            Pos=InputLineLength-CurPos+2
            CALL DumpCurrentLineBuffer(StartLine,NumConxLines,LineBuf,LineBufLen)
            CALL ShowWarningError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
                         ' Comma being inserted after:"'//InputLine(CurPos:InputLineLength)// &
                         '" in Object='//TRIM(SqueezedObject),EchoInputFile,EPObjects)
          ENDIF
        ENDIF
      ENDIF
    ELSE
     CALL ReadInputLine(IDFFile,CurPos,BlankLine,InputLineLength,EndofFile)
     CYCLE
    ENDIF
    IF (Pos > 0) THEN
      IF (.not. ErrFlag) THEN
        IF (CurPos <= CurPos+Pos-2) THEN
          SqueezedArg=MakeUPPERCase(ADJUSTL(InputLine(CurPos:CurPos+Pos-2)))
          IF (LEN_TRIM(ADJUSTL(InputLine(CurPos:CurPos+Pos-2))) > MaxAlphaArgLength) THEN
            CALL DumpCurrentLineBuffer(StartLine,NumConxLines,LineBuf,LineBufLen)
            CALL ShowWarningError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
                         ' Alpha Argument length exceeds maximum, will be truncated='// &
                         TRIM(InputLine(CurPos:CurPos+Pos-2)), EchoInputFile,EPObjects)
            CALL ShowContinueError('Will be processed as Alpha='//TRIM(SqueezedArg),EchoInputFile,EPObjects)
          ENDIF
        ELSE
          SqueezedArg=' '
        ENDIF
        IF (NumArg == NumArgExpected .and. .not. ObjectDef(Found)%ExtensibleObject) THEN
          CALL DumpCurrentLineBuffer(StartLine,NumConxLines,LineBuf,LineBufLen)
          CALL ShowSevereError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
                      ' Error detected for Object='//TRIM(ObjectDef(Found)%Name),EchoInputFile,EPObjects)
          CALL ShowContinueError(' Maximum arguments reached for this object, trying to process ->'//TRIM(SqueezedArg)//'<-',  &
                           EchoInputFile,EPObjects)
!          IF (ObjectDef(Found)%ExtensibleObject) THEN
!            CALL ShowContinueError(' '//TRIM(ObjectDef(Found)%Name)//' is extensible.  Edit "Energy+.idd" to expand the'//  &
!                                   ' number of fields.')
!            CALL ShowContinueError(' See notes in the Input Output Reference and/or at the beginning of this object'//  &
!                                   ' in the IDD file.')
!          ENDIF
          ErrFlag=.true.
        ELSE
!          IF (NumArg == NumArgExpected .and. ObjectDef(Found)%ExtensibleObject) THEN
!            CALL ExtendObjectDefinition(Found,NumArgExpected)
!          ENDIF
          NumArg=NumArg+1
          IF (ObjectDef(Found)%AlphaorNumeric(NumArg)) THEN
            IF (NumAlpha == ObjectDef(Found)%NumAlpha) THEN
              CALL DumpCurrentLineBuffer(StartLine,NumConxLines,LineBuf,LineBufLen)
              CALL ShowSevereError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
                        ' Error detected for Object='//TRIM(ObjectDef(Found)%Name),EchoInputFile,EPObjects)
              CALL ShowContinueError(' Too many Alphas for this object, trying to process ->'//TRIM(SqueezedArg)//'<-',  &
                               EchoInputFile,EPObjects)
              ErrFlag=.true.
            ELSE
              NumAlpha=NumAlpha+1
              LineItem%NumAlphas=NumAlpha
              IF (ObjectDef(Found)%AlphRetainCase(NumArg)) THEN
                SqueezedArg=InputLine(CurPos:CurPos+Pos-2)
                SqueezedArg=ADJUSTL(SqueezedArg)
              ENDIF
              IF (SqueezedArg /= Blank) THEN
                LineItem%Alphas(NumAlpha)=SqueezedArg
              ELSEIF (ObjectDef(Found)%ReqField(NumArg)) THEN  ! Blank Argument
                IF (ObjectDef(Found)%AlphFieldDefs(NumAlpha) /= Blank) THEN
                  LineItem%Alphas(NumAlpha)=ObjectDef(Found)%AlphFieldDefs(NumAlpha)
                ELSE
                  IF (ObjectDef(Found)%NameAlpha1 .and. NumAlpha /= 1) THEN
                    CALL DumpCurrentLineBuffer(StartLine,NumConxLines,LineBuf,LineBufLen)
                    CALL ShowSevereError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
                                ' Error detected in Object='//TRIM(ObjectDef(Found)%Name)//', name='//  &
                                TRIM(LineItem%Alphas(1)),EchoInputFile,EPObjects)
                  ELSE
                    CALL DumpCurrentLineBuffer(StartLine,NumConxLines,LineBuf,LineBufLen)
                    CALL ShowSevereError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
                                ' Error detected in Object='//TRIM(ObjectDef(Found)%Name),EchoInputFile,EPObjects)
                  ENDIF
                  CALL ShowContinueError('Field ['//TRIM(ObjectDef(Found)%AlphFieldChks(NumAlpha))//  &
                                         '] is required but was blank',EchoInputFile,EPObjects)
                  NumBlankReqFieldFound=NumBlankReqFieldFound+1
                ENDIF
              ELSE
                LineItem%AlphBlank(NumAlpha)=.true.
                IF (ObjectDef(Found)%AlphFieldDefs(NumAlpha) /= Blank) THEN
                  LineItem%Alphas(NumAlpha)=ObjectDef(Found)%AlphFieldDefs(NumAlpha)
                ENDIF
              ENDIF
            ENDIF
          ELSE
            IF (NumNumeric == ObjectDef(Found)%NumNumeric) THEN
              CALL DumpCurrentLineBuffer(StartLine,NumConxLines,LineBuf,LineBufLen)
              CALL ShowSevereError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
                           ' Error detected for Object='//TRIM(ObjectDef(Found)%Name),EchoInputFile,EPObjects)
              CALL ShowContinueError(' Too many Numbers for this object, trying to process ->'//TRIM(SqueezedArg)//'<-',  &
                                     EchoInputFile,EPObjects)
              ErrFlag=.true.
            ELSE
              NumNumeric=NumNumeric+1
              LineItem%NumNumbers=NumNumeric
              IF (SqueezedArg /= Blank) THEN
                IF (.not. ObjectDef(Found)%NumRangeChks(NumNumeric)%AutoSizable .and.   &
                    .not. ObjectDef(Found)%NumRangeChks(NumNumeric)%AutoCalculatable) THEN
                  LineItem%Numbers(NumNumeric)=ProcessNumber(SqueezedArg,Errflag)
                ELSEIF (SqueezedArg == 'AUTOSIZE') THEN
                  LineItem%Numbers(NumNumeric)=ObjectDef(Found)%NumRangeChks(NumNumeric)%AutoSizeValue
                ELSEIF (SqueezedArg == 'AUTOCALCULATE') THEN
                  LineItem%Numbers(NumNumeric)=ObjectDef(Found)%NumRangeChks(NumNumeric)%AutoCalculateValue
                ELSE
                  LineItem%Numbers(NumNumeric)=ProcessNumber(SqueezedArg,Errflag)
                ENDIF
              ELSE
                IF (ObjectDef(Found)%NumRangeChks(NumNumeric)%DefaultChk) THEN
                  IF (.not. ObjectDef(Found)%NumRangeChks(NumNumeric)%DefAutoSize .and.   &
                      .not. ObjectDef(Found)%NumRangeChks(NumNumeric)%AutoCalculatable) THEN
                    LineItem%Numbers(NumNumeric)=ObjectDef(Found)%NumRangeChks(NumNumeric)%Default
                    LineItem%NumBlank(NumNumeric)=.true.
                  ELSEIF (ObjectDef(Found)%NumRangeChks(NumNumeric)%DefAutoSize) THEN
                    LineItem%Numbers(NumNumeric)=ObjectDef(Found)%NumRangeChks(NumNumeric)%AutoSizeValue
                    LineItem%NumBlank(NumNumeric)=.true.
                  ELSEIF (ObjectDef(Found)%NumRangeChks(NumNumeric)%DefAutoCalculate) THEN
                    LineItem%Numbers(NumNumeric)=ObjectDef(Found)%NumRangeChks(NumNumeric)%AutoCalculateValue
                    LineItem%NumBlank(NumNumeric)=.true.
                  ENDIF
                ELSE
                  IF (ObjectDef(Found)%ReqField(NumArg)) THEN
                    IF (ObjectDef(Found)%NameAlpha1) THEN
                      CALL DumpCurrentLineBuffer(StartLine,NumConxLines,LineBuf,LineBufLen)
                      CALL ShowSevereError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
                                  ' Error detected in Object='//TRIM(ObjectDef(Found)%Name)// &
                                  ', name='//TRIM(LineItem%Alphas(1)),EchoInputFile,EPObjects)
                    ELSE
                      CALL DumpCurrentLineBuffer(StartLine,NumConxLines,LineBuf,LineBufLen)
                      CALL ShowSevereError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
                                  ' Error detected in Object='//TRIM(ObjectDef(Found)%Name),EchoInputFile,EPObjects)
                    ENDIF
                    CALL ShowContinueError('Field ['//TRIM(ObjectDef(Found)%NumRangeChks(NumNumeric)%FieldName)//  &
                                     '] is required but was blank',EchoInputFile,EPObjects)
                    NumBlankReqFieldFound=NumBlankReqFieldFound+1
                  ENDIF
                  LineItem%Numbers(NumNumeric)=0.0
                  LineItem%NumBlank(NumNumeric)=.true.
                  !LineItem%Numbers(NumNumeric)=-999999.  !0.0
                  !CALL ShowWarningError('Default number in Input, in object='//TRIM(ObjectDef(Found)%Name))
                ENDIF
                ErrFlag=.false.
              ENDIF
              IF (ErrFlag) THEN
                IF (SqueezedArg(1:1) /= '=') THEN
                  WRITE(FieldString,*) NumNumeric
                  FieldString=ADJUSTL(FieldString)
                  FieldNameString=ObjectDef(Found)%NumRangeChks(NumNumeric)%FieldName
                  IF (FieldNameString /= Blank) THEN
                    Message='Invalid Number in Numeric Field#'//TRIM(FieldString)//' ('//TRIM(FieldNameString)//  &
                                  '), value='//TRIM(SqueezedArg)
                  ELSE ! Field Name not recorded
                    Message='Invalid Number in Numeric Field#'//TRIM(FieldString)//', value='//TRIM(SqueezedArg)
                  ENDIF
                  Message=TRIM(Message)//', in '//TRIM(ObjectDef(Found)%Name)
                  IF (ObjectDef(Found)%NameAlpha1) THEN
                    Message=TRIM(Message)//'='//TRIM(LineItem%Alphas(1))
                  ENDIF
                  CALL DumpCurrentLineBuffer(StartLine,NumConxLines,LineBuf,LineBufLen)
                  CALL ShowSevereError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
                                  ' '//TRIM(Message),EchoInputFile,EPObjects)
                ENDIF
                ErrFlag=.false.
              ENDIF
            ENDIF
          ENDIF
        ENDIF
      ENDIF

      IF (InputLine(CurPos+Pos-1:CurPos+Pos-1) == ';') THEN
        EndofObject=.true.
        ! Check if more characters on line -- and if first is a comment character
        IF (InputLine(CurPos+Pos:) /= Blank) THEN
          NextChr=FindNonSpace(InputLine(CurPos+Pos:))
          IF (InputLine(CurPos+Pos+NextChr-1:CurPos+Pos+NextChr-1) /= '!') THEN
            CALL DumpCurrentLineBuffer(StartLine,NumConxLines,LineBuf,LineBufLen)
            CALL ShowWarningError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
                       ' End of Object="'//TRIM(ObjectDef(Found)%Name)//  &
                                  '" reached, but next part of line not comment.',EchoInputFile,EPObjects)
            CALL ShowContinueError('Final line above shows line that contains error.',EPObjects)
          ENDIF
        ENDIF
      ENDIF
      CurPos=CurPos+Pos
    ENDIF

  END DO

    ! Store to IDFRecord Data Structure, ErrFlag is true if there was an error
  ! Check out MinimumNumberOfFields
  IF (.not. ErrFlag .and. .not. IDidntMeanIt) THEN
    IF (NumArg < ObjectDef(Found)%MinNumFields) THEN
      IF (ObjectDef(Found)%NameAlpha1) THEN
        CALL ShowAuditErrorMessage(' ** Warning ** ','IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
                            ' Object='//TRIM(ObjectDef(Found)%Name)//  &
                            ', name='//TRIM(LineItem%Alphas(1))//       &
                                     ', entered with less than minimum number of fields.')
      ELSE
        CALL ShowAuditErrorMessage(' ** Warning ** ','IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
                            ' Object='//TRIM(ObjectDef(Found)%Name)//  &
                            ', entered with less than minimum number of fields.')
      ENDIF
      CALL ShowAuditErrorMessage(' **   ~~~   ** ','Attempting fill to minimum.')
      NumAlpha=0
      NumNumeric=0
      IF (ObjectDef(Found)%MinNumFields > ObjectDef(Found)%NumParams) THEN
        WRITE(String,*) ObjectDef(Found)%MinNumFields
        WRITE(String1,*) ObjectDef(Found)%NumParams
        String=ADJUSTL(String)
        String1=ADJUSTL(String1)
        CALL ShowSevereError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
                      ' Object \min-fields > number of fields specified, Object='//TRIM(ObjectDef(Found)%Name),EPObjects)
        CALL ShowContinueError('..\min-fields='//TRIM(String)//  &
                               ', total number of fields in object definition='//TRIM(String1),EPObjects)
        ErrFlag=.true.
      ELSE
        DO Count=1,ObjectDef(Found)%MinNumFields
          IF (ObjectDef(Found)%AlphaOrNumeric(Count)) THEN
            NumAlpha=NumAlpha+1
            IF (NumAlpha <= LineItem%NumAlphas) CYCLE
            LineItem%NumAlphas=LineItem%NumAlphas+1
            IF (ObjectDef(Found)%AlphFieldDefs(LineItem%NumAlphas) /= Blank) THEN
              LineItem%Alphas(LineItem%NumAlphas)=ObjectDef(Found)%AlphFieldDefs(LineItem%NumAlphas)
              CALL ShowAuditErrorMessage(' **   Add   ** ',TRIM(ObjectDef(Found)%AlphFieldDefs(LineItem%NumAlphas))//   &
                                  '   ! field=>'//TRIM(ObjectDef(Found)%AlphFieldChks(NumAlpha)))
            ELSEIF (ObjectDef(Found)%ReqField(Count)) THEN
              IF (ObjectDef(Found)%NameAlpha1) THEN
                CALL ShowSevereError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
                                    ' Object='//TRIM(ObjectDef(Found)%Name)//  &
                                    ', name='//TRIM(LineItem%Alphas(1))// &
                                    ', Required Field=['//  &
                                    TRIM(ObjectDef(Found)%AlphFieldChks(NumAlpha))//   &
                                    '] was blank.',EchoInputFile,EPObjects)
              ELSE
                CALL ShowSevereError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
                                    ' Object='//TRIM(ObjectDef(Found)%Name)//  &
                                    ', Required Field=['//  &
                                    TRIM(ObjectDef(Found)%AlphFieldChks(NumAlpha))//   &
                                    '] was blank.',EchoInputFile,EPObjects)
              ENDIF
              ErrFlag=.true.
            ELSE
              LineItem%Alphas(LineItem%NumAlphas)=Blank
              LineItem%AlphBlank(LineItem%NumAlphas)=.true.
              CALL ShowAuditErrorMessage(' **   Add   ** ','<blank field>   ! field=>'//  &
                                   TRIM(ObjectDef(Found)%AlphFieldChks(NumAlpha)))
            ENDIF
          ELSE
            NumNumeric=NumNumeric+1
            IF (NumNumeric <= LineItem%NumNumbers) CYCLE
            LineItem%NumNumbers=LineItem%NumNumbers+1
            LineItem%NumBlank(NumNumeric)=.true.
            IF (ObjectDef(Found)%NumRangeChks(NumNumeric)%Defaultchk) THEN
              IF (.not. ObjectDef(Found)%NumRangeChks(NumNumeric)%DefAutoSize .and.   &
                  .not. ObjectDef(Found)%NumRangeChks(NumNumeric)%DefAutoCalculate) THEN
                LineItem%Numbers(NumNumeric)=ObjectDef(Found)%NumRangeChks(NumNumeric)%Default
                WRITE(String,*) ObjectDef(Found)%NumRangeChks(NumNumeric)%Default
                String=ADJUSTL(String)
                CALL ShowAuditErrorMessage(' **   Add   ** ',TRIM(String)//  &
                                    '   ! field=>'//TRIM(ObjectDef(Found)%NumRangeChks(NumNumeric)%FieldName))
              ELSEIF (ObjectDef(Found)%NumRangeChks(NumNumeric)%DefAutoSize) THEN
                LineItem%Numbers(NumNumeric)=ObjectDef(Found)%NumRangeChks(NumNumeric)%AutoSizeValue
                CALL ShowAuditErrorMessage(' **   Add   ** ','autosize    ! field=>'//  &
                                    TRIM(ObjectDef(Found)%NumRangeChks(NumNumeric)%FieldName))
              ELSEIF (ObjectDef(Found)%NumRangeChks(NumNumeric)%DefAutoCalculate) THEN
                LineItem%Numbers(NumNumeric)=ObjectDef(Found)%NumRangeChks(NumNumeric)%AutoCalculateValue
                CALL ShowAuditErrorMessage(' **   Add   ** ','autocalculate    ! field=>'//  &
                                    TRIM(ObjectDef(Found)%NumRangeChks(NumNumeric)%FieldName))
              ENDIF
            ELSEIF (ObjectDef(Found)%ReqField(Count)) THEN
              IF (ObjectDef(Found)%NameAlpha1) THEN
                CALL ShowSevereError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
                                    ' Object='//TRIM(ObjectDef(Found)%Name)//  &
                                    ', name='//TRIM(LineItem%Alphas(1))// &
                                    ', Required Field=['//  &
                                    TRIM(ObjectDef(Found)%NumRangeChks(NumNumeric)%FieldName)//   &
                                    '] was blank.',EchoInputFile,EPObjects)
              ELSE
                CALL ShowSevereError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
                                    ' Object='//TRIM(ObjectDef(Found)%Name)//  &
                                    ', Required Field=['//  &
                                    TRIM(ObjectDef(Found)%NumRangeChks(NumNumeric)%FieldName)//   &
                                    '] was blank.',EchoInputFile,EPObjects)
              ENDIF
              ErrFlag=.true.
            ELSE
              LineItem%Numbers(NumNumeric)=0.0
              LineItem%NumBlank(NumNumeric)=.true.
              CALL ShowAuditErrorMessage(' **   Add   ** ','<blank field>   ! field=>'//  &
                                  TRIM(ObjectDef(Found)%NumRangeChks(NumNumeric)%FieldName))
            ENDIF
          ENDIF
        ENDDO
      ENDIF
    ENDIF
  ENDIF

  IF (.not. ErrFlag .and. .not. IDidntMeanIt) THEN
!    IF (TransitionDefer) THEN
!      CALL MakeTransition(Found)
!    ENDIF
    NumIDFRecords=NumIDFRecords+1
    IF (ObjectStartRecord(Found) == 0) ObjectStartRecord(Found)=NumIDFRecords
    MaxAlphaIDFArgsFound=MAX(MaxAlphaIDFArgsFound,LineItem%NumAlphas)
    MaxNumericIDFArgsFound=MAX(MaxNumericIDFArgsFound,LineItem%NumNumbers)
    MaxAlphaIDFDefArgsFound=MAX(MaxAlphaIDFDefArgsFound,ObjectDef(Found)%NumAlpha)
    MaxNumericIDFDefArgsFound=MAX(MaxNumericIDFDefArgsFound,ObjectDef(Found)%NumNumeric)
    IDFRecords(NumIDFRecords)%Name=LineItem%Name
    IDFRecords(NumIDFRecords)%NumNumbers=LineItem%NumNumbers
    IDFRecords(NumIDFRecords)%NumAlphas=LineItem%NumAlphas
    IDFRecords(NumIDFRecords)%ObjectDefPtr=LineItem%ObjectDefPtr
    ALLOCATE(IDFRecords(NumIDFRecords)%Alphas(LineItem%NumAlphas))
    ALLOCATE(IDFRecords(NumIDFRecords)%AlphBlank(LineItem%NumAlphas))
    ALLOCATE(IDFRecords(NumIDFRecords)%Numbers(LineItem%NumNumbers))
    ALLOCATE(IDFRecords(NumIDFRecords)%NumBlank(LineItem%NumNumbers))
    IDFRecords(NumIDFRecords)%Alphas(1:LineItem%NumAlphas)=LineItem%Alphas(1:LineItem%NumAlphas)
    IDFRecords(NumIDFRecords)%AlphBlank(1:LineItem%NumAlphas)=LineItem%AlphBlank(1:LineItem%NumAlphas)
    IDFRecords(NumIDFRecords)%Numbers(1:LineItem%NumNumbers)=LineItem%Numbers(1:LineItem%NumNumbers)
    IDFRecords(NumIDFRecords)%NumBlank(1:LineItem%NumNumbers)=LineItem%NumBlank(1:LineItem%NumNumbers)
    IF (LineItem%NumNumbers > 0) THEN
      DO Count=1,LineItem%NumNumbers
        IF (ObjectDef(Found)%NumRangeChks(Count)%MinMaxChk .and. .not. LineItem%NumBlank(Count)) THEN
          CALL InternalRangeCheck(LineItem%Numbers(Count),Count,Found,LineItem%Alphas(1),  &
                                  ObjectDef(Found)%NumRangeChks(Count)%AutoSizable,        &
                                  ObjectDef(Found)%NumRangeChks(Count)%AutoCalculatable)
        ENDIF
      ENDDO
    ENDIF
  ELSEIF (.not. IDidntMeanIt) THEN
    OverallErrorFlag=.true.
  ENDIF

  RETURN

END SUBROUTINE ValidateObjectandParse

SUBROUTINE ValidateSectionsInput

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   September 1997
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine uses the data structure that is set up during
          ! IDF processing and makes sure that record pointers are accurate.
          ! They could be inaccurate if a 'section' is input without any
          ! 'objects' following.  The invalidity will show itself in the
          ! values of the FirstRecord and Last Record pointer.
          ! If FirstRecord>LastRecord, then no records (Objects) have been
          ! written to the SIDF file for that Section.

          ! METHODOLOGY EMPLOYED:
          ! Scan the SectionsonFile data structure and look for invalid
          ! FirstRecord,LastRecord items.  Reset those items to -1.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

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

  DO Count=1,NumIDFSections
    IF (SectionsonFile(Count)%FirstRecord > SectionsonFile(Count)%LastRecord) THEN
      WRITE(EchoInputFile,*) ' Section ',Count,' ',TRIM(SectionsonFile(Count)%Name),' had no object records'
      SectionsonFile(Count)%FirstRecord=-1
      SectionsonFile(Count)%LastRecord=-1
    ENDIF
  END DO

  RETURN

END SUBROUTINE ValidateSectionsInput

INTEGER FUNCTION GetNumSectionsFound(SectionWord)

          ! FUNCTION INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   September 1997
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This function returns the number of a particular section (in input data file)
          ! found in the current run.  If it can't find the section in list
          ! of sections, a -1 will be returned.

          ! METHODOLOGY EMPLOYED:
          ! Look up section in list of sections.  If there, return the
          ! number of sections of that kind found in the current input.  If not, return
          ! -1.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  CHARACTER(len=*), INTENT(IN) :: SectionWord

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  INTEGER Found

  Found=FindIteminList(MakeUPPERCase(SectionWord),ListofSections,NumSectionDefs)
  IF (Found == 0) THEN
!    CALL ShowFatalError('Requested Section not found in Definitions: '//TRIM(SectionWord))
    GetNumSectionsFound=0
  ELSE
    GetNumSectionsFound=SectionDef(Found)%NumFound
  ENDIF

  RETURN

END FUNCTION GetNumSectionsFound

INTEGER FUNCTION GetNumSectionsinInput()

          ! FUNCTION INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   September 1997
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This function returns the number of sections in the entire input data file
          ! of the current run.

          ! METHODOLOGY EMPLOYED:
          ! Return value of NumIDFSections.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

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
          ! na

  GetNumSectionsinInput=NumIDFSections

  RETURN

END FUNCTION GetNumSectionsinInput

SUBROUTINE GetListofSectionsinInput(SectionList,NuminList)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   September 1997
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine returns the list of sections as they occurred
          ! in the Input Data File (IDF).

          ! METHODOLOGY EMPLOYED:
          ! Look up object in list of objects.  If there, return the
          ! number of objects found in the current input.  If not, return
          ! -1.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  CHARACTER(len=*), DIMENSION(:), INTENT(OUT) :: SectionList
  INTEGER, INTENT(OUT) :: NuminList

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  INTEGER MaxAllowedOut

  MaxAllowedOut=MIN(NumIDFSections,SIZE(SectionList))
  IF (MaxAllowedOut /= NumIDFSections) THEN
    CALL ShowWarningError('More in list than allowed in passed array - (GetListofSectionsinInput)')
  ENDIF
  NuminList=MaxAllowedOut
  SectionList(1:MaxAllowedOut)=SectionsonFile(1:MaxAllowedOut)%Name

  RETURN

END SUBROUTINE GetListofSectionsinInput

INTEGER FUNCTION GetNumObjectsFound(ObjectWord)

          ! FUNCTION INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   September 1997
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This function returns the number of objects (in input data file)
          ! found in the current run.  If it can't find the object in list
          ! of objects, a 0 will be returned.

          ! METHODOLOGY EMPLOYED:
          ! Look up object in list of objects.  If there, return the
          ! number of objects found in the current input.  If not, return 0.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  CHARACTER(len=*), INTENT(IN) :: ObjectWord

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  INTEGER Found

  IF (SortedIDD) THEN
!    Found=FindIteminSortedList(MakeUPPERCase(ObjectWord),ListofObjects,NumObjectDefs)
!    IF (Found /= 0) Found=iListofObjects(Found)
  ELSE
    Found=FindIteminList(MakeUPPERCase(ObjectWord),ListofObjects,NumObjectDefs)
  ENDIF

  IF (Found /= 0) THEN
    GetNumObjectsFound=ObjectDef(Found)%NumFound
  ELSE
    GetNumObjectsFound=0
    CALL ShowWarningError('Requested Object not found in Definitions: '//TRIM(ObjectWord),EPObjects)
  ENDIF

  RETURN

END FUNCTION GetNumObjectsFound

SUBROUTINE GetRecordLocations(Which,FirstRecord,LastRecord)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   September 1997
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine returns the record location values (which will be
          ! passed to 'GetObjectItem') for a section from the list of inputted
          ! sections (sequential).

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  INTEGER, INTENT(IN) :: Which
  INTEGER, INTENT(OUT) :: FirstRecord
  INTEGER, INTENT(OUT) :: LastRecord

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
          ! na

  FirstRecord=SectionsonFile(Which)%FirstRecord
  LastRecord=SectionsonFile(Which)%LastRecord

  RETURN

END SUBROUTINE GetRecordLocations

SUBROUTINE GetObjectItem(Object,Number,Alphas,NumAlphas,Numbers,NumNumbers,Status,NumBlank,AlphaBlank,   &
                         AlphaFieldNames,NumericFieldNames)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   September 1997
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine gets the 'number' 'object' from the IDFRecord data structure.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  CHARACTER(len=*), INTENT(IN) :: Object
  INTEGER, INTENT(IN) :: Number
  CHARACTER(len=*), INTENT(OUT), DIMENSION(:) :: Alphas
  INTEGER, INTENT(OUT) :: NumAlphas
  REAL(r64), INTENT(OUT), DIMENSION(:) :: Numbers
  INTEGER, INTENT(OUT) :: NumNumbers
  INTEGER, INTENT(OUT) :: Status
  LOGICAL, INTENT(OUT), DIMENSION(:), OPTIONAL :: AlphaBlank
  LOGICAL, INTENT(OUT), DIMENSION(:), OPTIONAL :: NumBlank
  CHARACTER(len=*), DIMENSION(:), OPTIONAL :: AlphaFieldNames
  CHARACTER(len=*), DIMENSION(:), OPTIONAL :: NumericFieldNames

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  INTEGER Count
  INTEGER LoopIndex
  CHARACTER(len=MaxObjectNameLength) ObjectWord
  CHARACTER(len=MaxObjectNameLength) UCObject
  CHARACTER(len=MaxObjectNameLength), SAVE, ALLOCATABLE, DIMENSION(:) :: AlphaArgs
  REAL(r64), SAVE, ALLOCATABLE, DIMENSION(:) :: NumberArgs
  LOGICAL, SAVE, ALLOCATABLE, DIMENSION(:) :: AlphaArgsBlank
  LOGICAL, SAVE, ALLOCATABLE, DIMENSION(:) :: NumberArgsBlank
  INTEGER MaxAlphas,MaxNumbers
  INTEGER Found
  INTEGER StartRecord
  INTEGER Loop
  character(len=25) :: cfld1
  character(len=25) :: cfld2

  MaxAlphas=SIZE(Alphas,1)
  MaxNumbers=SIZE(Numbers,1)

  IF (.not. ALLOCATED(AlphaArgs)) THEN
    IF (NumObjectDefs == 0) THEN
      CALL ProcessInput
    ENDIF
    ALLOCATE(AlphaArgs(MaxAlphaArgsFound))
    ALLOCATE(NumberArgs(MaxNumericArgsFound))
    ALLOCATE(NumberArgsBlank(MaxNumericArgsFound))
    ALLOCATE(AlphaArgsBlank(MaxAlphaArgsFound))
  ENDIF

  Count=0
  Status=-1
  UCOBject=MakeUPPERCase(Object)
  IF (SortedIDD) THEN
!    Found=FindIteminSortedList(UCOBject,ListofObjects,NumObjectDefs)
!    IF (Found /= 0) Found=iListofObjects(Found)
  ELSE
    Found=FindIteminList(UCOBject,ListofObjects,NumObjectDefs)
  ENDIF
  IF (Found == 0) THEN   !  This is more of a developer problem
    CALL ShowFatalError('Requested object='//TRIM(UCObject)//  &
       ', not found in Object Definitions -- incorrect IDD attached.',EPObjects)
  ENDIF

  IF (ObjectDef(Found)%NumAlpha > 0) THEN
    IF (ObjectDef(Found)%NumAlpha > MaxAlphas) THEN
      WRITE(cfld1,*) ObjectDef(Found)%NumAlpha
      cfld1=adjustl(cfld1)
      WRITE(cfld2,*) MaxAlphas
      cfld2=adjustl(cfld2)
      CALL ShowFatalError('GetObjectItem: '//TRIM(Object)//', Number of ObjectDef Alpha Args ['//TRIM(cfld1)//  &
          '] > Size of AlphaArg array ['//TRIM(cfld2)//'].',EPObjects)
    ENDIF
    Alphas(1:ObjectDef(Found)%NumAlpha)=' '
  ENDIF
  IF (ObjectDef(Found)%NumNumeric > 0) THEN
    IF (ObjectDef(Found)%NumNumeric > MaxNumbers) THEN
      WRITE(cfld1,*) ObjectDef(Found)%NumNumeric
      cfld1=adjustl(cfld1)
      WRITE(cfld2,*) MaxNumbers
      cfld2=adjustl(cfld2)
      CALL ShowFatalError('GetObjectItem: '//TRIM(Object)//', Number of ObjectDef Numeric Args ['//TRIM(cfld1)//  &
          '] > Size of NumericArg array ['//TRIM(cfld2)//'].',EPObjects)
    ENDIF
    Numbers(1:ObjectDef(Found)%NumNumeric)=0.0
  ENDIF

  StartRecord=ObjectStartRecord(Found)
  IF (StartRecord == 0) THEN
    CALL ShowWarningError('Requested object='//TRIM(UCObject)//', not found in IDF.',EPObjects)
    Status=-1
    StartRecord=NumIDFRecords+1
  ENDIF

!  IF (ObjectGotCount(Found) == 0) THEN
!    WRITE(EchoInputFile,*) 'Getting object=',TRIM(UCObject)
!  ENDIF
!  ObjectGotCount(Found)=ObjectGotCount(Found)+1

  DO LoopIndex=StartRecord,NumIDFRecords
    IF (IDFRecords(LoopIndex)%Name == UCObject) THEN
      Count=Count+1
      IF (Count == Number) THEN
!        IDFRecordsGotten(LoopIndex)=.true.  ! only object level "gets" recorded
        ! Read this one
        CALL GetObjectItemfromFile(LoopIndex,ObjectWord,AlphaArgs,NumAlphas,NumberArgs,NumNumbers,AlphaArgsBlank,NumberArgsBlank)
        IF (NumAlphas > MaxAlphas .or. NumNumbers > MaxNumbers) THEN
          CALL ShowFatalError('Too many actual arguments for those expected on Object: '//TRIM(ObjectWord)//     &
                                 ' (GetObjectItem)',EchoInputFile,EPObjects)
        ENDIF
        NumAlphas=MIN(MaxAlphas,NumAlphas)
        NumNumbers=MIN(MaxNumbers,NumNumbers)
        IF (NumAlphas > 0) THEN
          Alphas(1:NumAlphas)=AlphaArgs(1:NumAlphas)
        ENDIF
        IF (NumNumbers > 0) THEN
          Numbers(1:NumNumbers)=NumberArgs(1:NumNumbers)
        ENDIF
        IF (PRESENT(NumBlank)) THEN
          NumBlank=.true.
          IF (NumNumbers > 0) &
            NumBlank(1:NumNumbers)=NumberArgsBlank(1:NumNumbers)
        ENDIF
        IF (PRESENT(AlphaBlank)) THEN
          AlphaBlank=.true.
          IF (NumAlphas > 0) &
            AlphaBlank(1:NumAlphas)=AlphaArgsBlank(1:NumAlphas)
        ENDIF
        IF (PRESENT(AlphaFieldNames)) THEN
          AlphaFieldNames(1:ObjectDef(Found)%NumAlpha)=ObjectDef(Found)%AlphFieldChks(1:ObjectDef(Found)%NumAlpha)
        ENDIF
        IF (PRESENT(NumericFieldNames)) THEN
          NumericFieldNames(1:ObjectDef(Found)%NumNumeric)=ObjectDef(Found)%NumRangeChks(1:ObjectDef(Found)%NumNumeric)%FieldName
        ENDIF
        Status=1
        EXIT
      ENDIF
    ENDIF
  END DO

  RETURN

END SUBROUTINE GetObjectItem

INTEGER FUNCTION GetObjectItemNum(ObjType,ObjName)

          ! SUBROUTINE INFORMATION
          !             AUTHOR:  Fred Buhl
          !       DATE WRITTEN:  Jan 1998
          !           MODIFIED:  Lawrie, September 1999. Take advantage of internal
          !                      InputProcessor structures to speed search.
          !      RE-ENGINEERED:  This is new code, not reengineered

          ! PURPOSE OF THIS SUBROUTINE:
          ! Get the occurrence number of an object of type ObjType and name ObjName

          ! METHODOLOGY EMPLOYED:
          ! Use internal IDF record structure for each object occurrence
          ! and compare the name with ObjName.

          ! REFERENCES:
          ! na

IMPLICIT NONE

          ! SUBROUTINE ARGUMENTS:
CHARACTER(len=*), INTENT(IN) :: ObjType   ! Object Type (ref: IDD Objects)
CHARACTER(len=*), INTENT(IN) :: ObjName   ! Name of the object type

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK DEFINITIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DEFINITIONS
INTEGER                                 :: NumObjOfType ! Total number of Object Type in IDF
INTEGER                                 :: ObjNum       ! Loop index variable
INTEGER                                 :: ItemNum      ! Item number for Object Name
INTEGER                                 :: Found        ! Indicator for Object Type in list of Valid Objects
CHARACTER(len=MaxObjectNameLength)      :: UCObjType    ! Upper Case for ObjType
LOGICAL                                 :: ItemFound    ! Set to true if item found
LOGICAL                                 :: ObjectFound  ! Set to true if object found
INTEGER                                 :: StartRecord  ! Start record for objects

ItemNum = 0
ItemFound=.false.
ObjectFound=.false.
UCObjType=MakeUPPERCase(ObjType)
IF (SortedIDD) THEN
!  Found=FindIteminSortedList(UCObjType,ListofObjects,NumObjectDefs)
!  IF (Found /= 0) Found=iListofObjects(Found)
ELSE
  Found=FindIteminList(UCObjType,ListofObjects,NumObjectDefs)
ENDIF

IF (Found /= 0) THEN

  ObjectFound=.true.
  NumObjOfType=ObjectDef(Found)%NumFound
  ItemNum=0
  StartRecord=ObjectStartRecord(Found)

  IF (StartRecord > 0) THEN
    DO ObjNum=StartRecord,NumIDFRecords
      IF (IDFRecords(ObjNum)%Name /= UCObjType) CYCLE
      ItemNum=ItemNum+1
      IF (ItemNum > NumObjOfType) EXIT
      IF (IDFRecords(ObjNum)%Alphas(1) == ObjName) THEN
        ItemFound=.true.
        EXIT
      ENDIF
    END DO
  ENDIF
ENDIF

IF (ObjectFound) THEN
  IF (.not. ItemFound) ItemNum=0
ELSE
  ItemNum=-1  ! if object not found, then flag it
ENDIF

GetObjectItemNum = ItemNum

RETURN

END FUNCTION GetObjectItemNum


SUBROUTINE TellMeHowManyObjectItemArgs(Object,Number,NumAlpha,NumNumbers,Status)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   September 1997
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine returns the number of arguments (alpha and numeric) for
          ! the referenced 'number' Object.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  CHARACTER(len=*), INTENT(IN) :: Object
  INTEGER, INTENT(IN) :: Number
  INTEGER, INTENT(OUT) :: NumAlpha
  INTEGER, INTENT(OUT) :: NumNumbers
  INTEGER, INTENT(OUT) :: Status


          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  INTEGER Count
  INTEGER LoopIndex
  CHARACTER(len=MaxObjectNameLength) ObjectWord

  Count=0
  Status=-1
  DO LoopIndex=1,NumIDFRecords
    IF (IDFRecords(LoopIndex)%Name == MakeUPPERCase(Object)) THEN
      Count=Count+1
      IF (Count == Number) THEN
        ! Read this one
        CALL GetObjectItemfromFile(LoopIndex,ObjectWord,NumAlpha=NumAlpha,NumNumeric=NumNumbers)
        Status=1
        EXIT
      ENDIF
    ENDIF
  END DO


  RETURN

END SUBROUTINE TellMeHowManyObjectItemArgs

SUBROUTINE GetObjectItemfromFile(Which,ObjectWord,AlphaArgs,NumAlpha,NumericArgs,NumNumeric,AlphaBlanks,NumericBlanks)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   September 1997
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine "gets" the object instance from the data structure.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  INTEGER, INTENT(IN) :: Which
  CHARACTER(len=*), INTENT(OUT) :: ObjectWord
  CHARACTER(len=*), INTENT(OUT), DIMENSION(:), OPTIONAL :: AlphaArgs
  INTEGER, INTENT(OUT) :: NumAlpha
  REAL(r64), INTENT(OUT), DIMENSION(:), OPTIONAL :: NumericArgs
  INTEGER, INTENT(OUT) :: NumNumeric
  LOGICAL, INTENT(OUT), DIMENSION(:), OPTIONAL :: AlphaBlanks
  LOGICAL, INTENT(OUT), DIMENSION(:), OPTIONAL :: NumericBlanks

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  TYPE (LineDefinition):: xLineItem                        ! Description of current record

  IF (Which > 0 .and. Which <= NumIDFRecords) THEN
    xLineItem=IDFRecords(Which)
    ObjectWord=xLineItem%Name
    NumAlpha=xLineItem%NumAlphas
    NumNumeric=xLineItem%NumNumbers
    IF (PRESENT(AlphaArgs)) THEN
      IF (NumAlpha >=1) THEN
        AlphaArgs(1:NumAlpha)=xLineItem%Alphas(1:NumAlpha)
      ENDIF
    ENDIF
    IF (PRESENT(AlphaBlanks)) THEN
      IF (NumAlpha >=1) THEN
        AlphaBlanks(1:NumAlpha)=xLineItem%AlphBlank(1:NumAlpha)
      ENDIF
    ENDIF
    IF (PRESENT(NumericArgs)) THEN
      IF (NumNumeric >= 1) THEN
        NumericArgs(1:NumNumeric)=xLineItem%Numbers(1:NumNumeric)
      ENDIF
    ENDIF
    IF (PRESENT(NumericBlanks)) THEN
      IF (NumNumeric >= 1) THEN
        NumericBlanks(1:NumNumeric)=xLineItem%NumBlank(1:NumNumeric)
      ENDIF
    ENDIF
  ELSE
    WRITE(EchoInputFile,*) ' Requested Record',Which,' not in range, 1 -- ',NumIDFRecords
  ENDIF

  RETURN

END SUBROUTINE GetObjectItemfromFile

! Utility Functions/Routines for Module

SUBROUTINE ReadInputLine(UnitNumber,CurPos,BlankLine,InputLineLength,EndofFile,  &
           MinMax,WhichMinMax,MinMaxString,Value,Default,DefString,AutoSizable,  &
           AutoCalculatable,RetainCase,ErrorsFound)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   September 1997
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine reads a line in the specified file and checks for end of file

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  INTEGER, INTENT(IN) :: UnitNumber
  INTEGER, INTENT(INOUT) :: CurPos
  LOGICAL, INTENT(INOUT) :: EndofFile
  LOGICAL, INTENT(INOUT) :: BlankLine
  INTEGER, INTENT(INOUT) :: InputLineLength
  LOGICAL, INTENT(INOUT), OPTIONAL :: MinMax
  INTEGER, INTENT(INOUT), OPTIONAL :: WhichMinMax   !=0 (none/invalid), =1 \min, =2 \min>, =3 \max, =4 \max<
  CHARACTER(len=*), INTENT(INOUT), OPTIONAL :: MinMaxString
  REAL(r64), INTENT(INOUT), OPTIONAL :: Value
  LOGICAL, INTENT(INOUT), OPTIONAL :: Default
  CHARACTER(len=*), INTENT(INOUT), OPTIONAL :: DefString
  LOGICAL, INTENT(INOUT), OPTIONAL :: AutoSizable
  LOGICAL, INTENT(INOUT), OPTIONAL :: AutoCalculatable
  LOGICAL, INTENT(INOUT), OPTIONAL :: RetainCase
  LOGICAL, INTENT(INOUT), OPTIONAL :: ErrorsFound

          ! SUBROUTINE PARAMETER DEFINITIONS:
  CHARACTER(len=1), PARAMETER :: TabChar=CHAR(9)

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
          INTEGER ReadStat
          INTEGER Pos
          INTEGER Slash
          INTEGER P1
          CHARACTER(len=MaxInputLineLength) UCInputLine        ! Each line can be up to MaxInputLineLength characters long
          LOGICAL TabsInLine
          INTEGER NSpace
          LOGICAL ErrFlag
          INTEGER, EXTERNAL :: FindNonSpace
          INTEGER ErrLevel
          INTEGER endcol
          CHARACTER(len=25) cNumLines
          LOGICAL LineTooLong

      ErrFlag=.false.
      LineTooLong=.false.
      READ(UnitNumber,fmta,IOSTAT=ReadStat) InputLine

      IF (ReadStat /= 0) InputLine=' '

      ! Following section of code allows same software to read Win or Unix files without translating
      endcol=LEN_TRIM(InputLine)
      IF (endcol > 0) THEN
        IF (ICHAR(InputLine(endcol:endcol)) == 13) THEN
          InputLine(endcol:endcol)=' '
        ENDIF
      ENDIF

      IF (InputLine(MaxInputLineLength+1:) /= Blank) THEN
        LineTooLong=.true.
        InputLine=InputLine(1:MaxInputLineLength)
      ENDIF

      P1=SCAN(InputLine,TabChar)
      TabsInLine=.false.
      DO WHILE (P1>0)
        TabsInLine=.true.
        InputLine(P1:P1)=' '
        P1=SCAN(InputLine,TabChar)
      ENDDO
      BlankLine=.false.
      CurPos=1
      IF (ReadStat == -1) THEN
        EndofFile=.true.
      ELSE
        IF (EchoInputLine) THEN
          NumLines=NumLines+1
          IF (NumLines < 100000) THEN
            WRITE(EchoInputFile,'(2X,I5,1X,A)') NumLines,TRIM(InputLine)
          ELSE
            WRITE(cNumLines,*) NumLines
            cNumLines=ADJUSTL(cNumLines)
            WRITE(EchoInputFile,'(1X,A,1X,A)') TRIM(cNumLines),TRIM(InputLine)
          ENDIF
          IF (TabsInLine) WRITE(EchoInputFile,"(6X,'***** Tabs eliminated from above line')")
          IF (LineTooLong) THEN
            CALL ShowSevereError('Input line longer than maximum length allowed='//TRIM(IPTrimSigDigits(MaxInputLineLength))//  &
                           ' characters. Other errors may follow.')
            CALL ShowContinueError('.. at line='//TRIM(IPTrimSigDigits(NumLines))//', first 50 characters='//  &
                           TRIM(InputLine(1:50)))
            WRITE(EchoInputFile,"(6X,'***** Previous line is longer than allowed length for input line')")
          ENDIF
        ENDIF
        EchoInputLine=.true.
        InputLineLength=LEN_TRIM(InputLine)
        IF (InputLineLength == 0) THEN
          BlankLine=.true.
        ENDIF
        IF (ProcessingIDD) THEN
          Pos=SCAN(InputLine,'!\')  ! 4/30/09 remove ~
          Slash=INDEX(InputLine,'\')
        ELSE
          Pos=SCAN(InputLine,'!')  ! 4/30/09 remove ~
          Slash=0
        ENDIF
        IF (Pos /= 0) THEN
          InputLineLength=Pos
          IF (Pos-1 > 0) THEN
            IF (LEN_TRIM(InputLine(1:Pos-1)) == 0) THEN
              BlankLine=.true.
            ENDIF
          ELSE
            BlankLine=.true.
          ENDIF
          IF (Slash /= 0 .and. Pos == Slash) THEN
            UCInputLine=MakeUPPERCase(InputLine)
            IF (UCInputLine(Slash:Slash+5) == '\FIELD') THEN
              ! Capture Field Name
              CurrentFieldName=InputLine(Slash+6:)
              CurrentFieldName=ADJUSTL(CurrentFieldName)
              P1=SCAN(CurrentFieldName,'!')
              IF (P1 /= 0) CurrentFieldName(P1:)=Blank
              FieldSet=.true.
            ELSE
              FieldSet=.false.
            ENDIF
            IF (UCInputLine(Slash:Slash+14) == '\REQUIRED-FIELD') THEN
              RequiredField=.true.
            ENDIF  ! Required-field arg
            IF (UCInputLine(Slash:Slash+15) == '\REQUIRED-OBJECT') THEN
              RequiredObject=.true.
            ENDIF  ! Required-object arg
            IF (UCInputLine(Slash:Slash+13) == '\UNIQUE-OBJECT') THEN
              UniqueObject=.true.
            ENDIF  ! Unique-object arg
            IF (UCInputLine(Slash:Slash+10) == '\EXTENSIBLE') THEN
              ExtensibleObject=.true.
              IF (UCInputLine(Slash+11:Slash+11) /= ':') THEN
                CALL ShowFatalError('IP: IDD Line='//TRIM(IPTrimSigDigits(NumLines))//  &
                    ' Illegal definition for extensible object, should be "\extensible:<num>"',EchoInputFile,EPObjects)
              ELSE
                ! process number
                NSpace=SCAN(UCInputLine(Slash+12:),' !')
                ExtensibleNumFields=INT(ProcessNumber(UCInputLine(Slash+12:Slash+12+NSpace-1),ErrFlag))
                IF (ErrFlag) THEN
                  CALL ShowSevereError('IP: IDD Line='//TRIM(IPTrimSigDigits(NumLines))//  &
                               ' Illegal Number for \extensible:<num>',EchoInputFile,EPObjects)
                ENDIF
              ENDIF
            ENDIF  ! Extensible arg
            IF (UCInputLine(Slash:Slash+10) == '\RETAINCASE') THEN
              RetainCase=.true.
            ENDIF  ! Unique-object arg
            IF (UCInputLine(Slash:Slash+10) == '\MIN-FIELDS') THEN
!              RequiredField=.true.
              NSpace=FindNonSpace(UCInputLine(Slash+11:))
              IF (NSpace == 0) THEN
                CALL ShowSevereError('IP: IDD Line='//TRIM(IPTrimSigDigits(NumLines))//  &
                            ' Need number for \Min-Fields',EchoInputFile,EPObjects)
                ErrFlag=.true.
                MinimumNumberOfFields=0
              ELSE
                Slash=Slash+11+NSpace-1
                NSpace=SCAN(UCInputLine(Slash:),' !')
                MinimumNumberOfFields=INT(ProcessNumber(UCInputLine(Slash:Slash+NSpace-1),ErrFlag))
                IF (ErrFlag) THEN
                  CALL ShowSevereError('IP: IDD Line='//TRIM(IPTrimSigDigits(NumLines))//  &
                               ' Illegal Number for \Min-Fields',EchoInputFile,EPObjects)
                ENDIF
              ENDIF
            ENDIF  ! Min-Fields Arg
            IF (UCInputLine(Slash:Slash+9) == '\OBSOLETE') THEN
              NSpace=INDEX(UCInputLine(Slash+9:),'=>')
              IF (NSpace == 0) THEN
                CALL ShowSevereError('IP: IDD Line='//TRIM(IPTrimSigDigits(NumLines))//  &
                              ' Need replacement object for \Obsolete objects',EchoInputFile,EPObjects)
                ErrFlag=.true.
              ELSE
                Slash=Slash+9+NSpace+1
                NSpace=SCAN(UCInputLine(Slash:),'!')
                IF (NSpace == 0) THEN
                  ReplacementName=InputLine(Slash:)
                ELSE
                  ReplacementName=InputLine(Slash:Slash+NSpace-2)
                ENDIF
                ObsoleteObject=.true.
              ENDIF
            ENDIF  ! Obsolete Arg
            IF (PRESENT(MinMax)) THEN
              IF (UCInputLine(Pos:Pos+7)=='\MINIMUM' .or.  &
                  UCInputLine(Pos:Pos+7)=='\MAXIMUM') THEN
                MinMax=.true.
                CALL ProcessMinMaxDefLine(UCInputLine(Pos:),WhichMinMax,MinMaxString,Value,DefString,ErrLevel)
                IF (ErrLevel > 0) THEN
                  CALL ShowSevereError('IP: IDD Line='//TRIM(IPTrimSigDigits(NumLines))//  &
                                ' Error in Minimum/Maximum designation -- invalid number='//TRIM(UCInputLine(Pos:)),  &
                                EchoInputFile,EPObjects)
                  ErrFlag=.true.
                ENDIF
              ELSE
                MinMax=.false.
              ENDIF
            ENDIF  ! Min/Max Args
            IF (PRESENT(Default)) THEN
              IF (UCInputLine(Pos:Pos+7)=='\DEFAULT') THEN
                 ! WhichMinMax, MinMaxString not filled here
                Default=.true.
                CALL ProcessMinMaxDefLine(InputLine(Pos:),WhichMinMax,MinMaxString,Value,DefString,ErrLevel)
                IF (.not. RetainCase .and. DefString /= ' ') DefString=MakeUPPERCase(DefString)
                IF (ErrLevel > 1) THEN
                  CALL ShowContinueError('Blank Default Field Encountered',EchoInputFile,EPObjects)
                  ErrFlag=.true.
                ENDIF
              ELSE
                Default=.false.
              ENDIF
            ENDIF  ! Default Arg
            IF (PRESENT(AutoSizable)) THEN
              IF (UCInputLine(Pos:Pos+5)=='\AUTOS') THEN
                AutoSizable=.true.
                CALL ProcessMinMaxDefLine(UCInputLine(Pos:),WhichMinMax,MinMaxString,Value,DefString,ErrLevel)
                IF (ErrLevel > 0) THEN
                  CALL ShowSevereError('IP: IDD Line='//TRIM(IPTrimSigDigits(NumLines))//  &
                               ' Error in Autosize designation -- invalid number='//TRIM(UCInputLine(Pos:)),  &
                                  EchoInputFile,EPObjects)
                  ErrFlag=.true.
                ENDIF
              ELSE
                AutoSizable=.false.
              ENDIF
            ENDIF  ! AutoSizable Arg
            IF (PRESENT(AutoCalculatable)) THEN
              IF (UCInputLine(Pos:Pos+5)=='\AUTOC') THEN
                AutoCalculatable=.true.
                CALL ProcessMinMaxDefLine(UCInputLine(Pos:),WhichMinMax,MinMaxString,Value,DefString,ErrLevel)
                IF (ErrLevel > 0) THEN
                  CALL ShowSevereError('IP: IDD Line='//TRIM(IPTrimSigDigits(NumLines))//  &
                               ' Error in Autocalculate designation -- invalid number='//  &
                               TRIM(UCInputLine(Pos:)),EchoInputFile,EPObjects)
                  ErrFlag=.true.
                ENDIF
              ELSE
                AutoCalculatable=.false.
              ENDIF
            ENDIF  ! AutoCalculatable Arg
          ENDIF
        ENDIF
      ENDIF
      IF (ErrFlag) THEN
        IF (PRESENT(ErrorsFound)) THEN
          ErrorsFound=.true.
        ENDIF
      ENDIF

  RETURN

END SUBROUTINE ReadInputLine

FUNCTION ProcessNumber(String,ErrorFlag) RESULT(rProcessNumber)

          ! FUNCTION INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   September 1997
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS FUNCTION:
          ! This function processes a string that should be numeric and
          ! returns the real value of the string.

          ! METHODOLOGY EMPLOYED:
          ! FUNCTION ProcessNumber translates the argument (a string)
          ! into a real number.  The string should consist of all
          ! numeric characters (except a decimal point).  Numerics
          ! with exponentiation (i.e. 1.2345E+03) are allowed but if
          ! it is not a valid number an error message along with the
          ! string causing the error is printed out and 0.0 is returned
          ! as the value.

          ! The Fortran input processor is used to make the conversion.

          ! REFERENCES:
          ! List directed Fortran input/output.

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  CHARACTER(len=*), INTENT(IN) :: String
  LOGICAL, INTENT(OUT)         :: ErrorFlag
  REAL(r64) :: rProcessNumber

          ! SUBROUTINE PARAMETER DEFINITIONS:
  CHARACTER(len=*), PARAMETER  :: ValidNumerics='0123456789.+-EeDd'//CHAR(9)

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:

  REAL(r64) Temp
  INTEGER IoStatus
  INTEGER VerNumber
  INTEGER StringLen
  CHARACTER(len=MaxNameLength) :: PString


  rProcessNumber=0.0
  !  Make sure the string has all what we think numerics should have
  PString=ADJUSTL(String)
  StringLen=LEN_TRIM(PString)
  ErrorFlag=.false.
  IoStatus=0
  IF (StringLen == 0) RETURN
  VerNumber=VERIFY(PString(1:StringLen),ValidNumerics)
  IF (VerNumber == 0) THEN
    Read(PString,*,IOSTAT=IoStatus) Temp
    rProcessNumber=Temp
    ErrorFlag=.false.
  ELSE
    rProcessNumber=0.0
    ErrorFlag=.true.
  ENDIF
  IF (IoStatus /= 0) THEN
    rProcessNumber=0.0
    ErrorFlag=.true.
  ENDIF

RETURN

END FUNCTION ProcessNumber

SUBROUTINE ProcessMinMaxDefLine(UCInputLine,WhichMinMax,MinMaxString,Value,DefaultString,ErrLevel)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   July 2000
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine processes the IDD lines that start with
          ! \minimum or \maximum and set up the parameters so that it can
          ! be automatically checked.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! IDD Statements.
          !  \minimum         Minimum that includes the following value
          !  i.e. min >=
          !  \minimum>        Minimum that must be > than the following value
          !
          !  \maximum         Maximum that includes the following value
          !  i.e. max <=
          !  \maximum<        Maximum that must be < than the following value
          !
          !  \default         Default for field (when field is blank)

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  CHARACTER(len=*), INTENT(IN)  :: UCInputLine ! part of input line starting \min or \max
  INTEGER, INTENT(OUT)          :: WhichMinMax  !=0 (none/invalid), =1 \min, =2 \min>, =3 \max, =4 \max<
  CHARACTER(len=*), INTENT(OUT) :: MinMaxString
  REAL(r64), INTENT(OUT)             :: Value
  CHARACTER(len=*), INTENT(OUT) :: DefaultString
  INTEGER, INTENT(OUT)          :: ErrLevel

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  INTEGER Pos
  INTEGER NSpace
  INTEGER, EXTERNAL :: FindNonSpace
  LOGICAL ErrFlag

  ErrLevel=0
  Pos=SCAN(UCInputLine,' ')

  SELECT CASE (MakeUPPERCase(UCInputLine(1:4)))

  CASE('\MIN')
    WhichMinMax=1
    IF (SCAN(UCInputLine,'>') /= 0) THEN
      Pos=SCAN(UCInputLine,'>')+1
      WhichMinMax=2
    ENDIF
    IF (WhichMinMax == 1) THEN
      MinMaxString='>='
    ELSE
      MinMaxString='>'
    ENDIF

  CASE('\MAX')
    WhichMinMax=3
    IF (SCAN(UCInputLine,'<') /= 0) THEN
      POS=SCAN(UCInputLine,'<')+1
      WhichMinMax=4
    ENDIF
    IF (WhichMinMax == 3) THEN
      MinMaxString='<='
    ELSE
      MinMaxString='<'
    ENDIF

  CASE('\DEF')
    WhichMinMax=5
    MinMaxString=Blank

  CASE('\AUT')
    WhichMinMax=6
    MinMaxString=Blank

  CASE DEFAULT
    WhichMinMax=0  ! invalid field
    MinMaxString=Blank
    Value=-999999.d0

  END SELECT

  IF (WhichMinMax /= 0) THEN
    NSpace=FindNonSpace(UCInputLine(Pos:))
    IF (NSpace == 0) THEN
      IF (WhichMinMax /= 6) THEN  ! Only autosize/autocalculate can't have argument
        CALL ShowSevereError('IP: IDD Line='//TRIM(IPTrimSigDigits(NumLines))//  &
                     'Min/Max/Default field cannot be blank -- must have value',EchoInputFile,EPObjects)
        ErrLevel=2
      ELSEIF (UCINPUTLINE(1:6) == '\AUTOS') THEN
        Value=DefAutosizeValue
      ELSEIF (UCINPUTLINE(1:6) == '\AUTOC') THEN
        Value=DefAutocalculateValue
      ENDIF
    ELSE
      Pos=Pos+NSpace-1
      NSpace=SCAN(UCInputLine(Pos:),' !')
      MinMaxString=TRIM(MinMaxString)//TRIM(UCInputLine(Pos:Pos+NSpace-1))
      Value=ProcessNumber(UCInputLine(Pos:Pos+NSpace-1),ErrFlag)
      IF (ErrFlag) ErrLevel=1
      NSpace=Scan(UCInputLine(Pos:),'!')
      IF (NSpace > 0) THEN
        DefaultString=UCInputLine(Pos:Pos+NSpace-2)
      ELSE
        DefaultString=UCInputLine(Pos:)
      ENDIF
      DefaultString=ADJUSTL(DefaultString)
      IF (DefaultString == Blank) THEN
        IF (WhichMinMax == 6) THEN
          IF (UCINPUTLINE(1:6) == '\AUTOS') THEN
            Value=DefAutosizeValue
          ELSE
            Value=DefAutoCalculateValue
          ENDIF
        ELSE
          CALL ShowSevereError('IP: IDD Line='//TRIM(IPTrimSigDigits(NumLines))//  &
                       'Min/Max/Default field cannot be blank -- must have value',EchoInputFile,EPObjects)
          ErrLevel=2
        ENDIF
      ENDIF
    ENDIF
  ENDIF

  RETURN

END SUBROUTINE ProcessMinMaxDefLine

INTEGER FUNCTION FindIteminList(String,ListofItems,NumItems)

          ! FUNCTION INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   September 1997
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
  CHARACTER(len=*), INTENT(IN) :: String
  CHARACTER(len=*), INTENT(IN), DIMENSION(:) :: ListofItems
  INTEGER, INTENT(IN) :: NumItems

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  INTEGER Count

  FindIteminList=0

  DO Count=1,NumItems
    IF (String == ListofItems(Count)) THEN
      FindIteminList=Count
      EXIT
    ENDIF
  END DO

  RETURN

END FUNCTION FindIteminList

INTEGER FUNCTION FindItem(String,ListofItems,NumItems)

          ! FUNCTION INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   April 1999
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS FUNCTION:
          ! This function looks up a string in a similar list of
          ! items and returns the index of the item in the list, if
          ! found.  This routine is case insensitive -- it uses the
          ! SameString function to assure that both strings are in
          ! all upper case.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  CHARACTER(len=*), INTENT(IN) :: String
  CHARACTER(len=*), INTENT(IN), DIMENSION(:) :: ListofItems
  INTEGER, INTENT(IN) :: NumItems

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  INTEGER Count
  CHARACTER(len=MaxInputLineLength) StringUC

  FindItem=0
  FindItem=FindItemInList(String,ListofItems,NumItems)
  IF (FindItem /= 0) RETURN
!
  StringUC=MakeUPPERCase(String)

  DO Count=1,NumItems
    IF (StringUC == MakeUPPERCase(ListofItems(Count))) THEN
      FindItem=Count
      EXIT
    ENDIF
  END DO

  RETURN

END FUNCTION FindItem

FUNCTION MakeUPPERCase(InputString) RESULT (ResultString)

          ! FUNCTION INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   September 1997
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This function returns the Upper Case representation of the InputString.

          ! METHODOLOGY EMPLOYED:
          ! Uses the Intrinsic SCAN function to scan the lowercase representation of
          ! characters (DataStringGlobals) for each character in the given string.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine


          ! FUNCTION ARGUMENT DEFINITIONS:
   CHARACTER(len=*), INTENT(IN) :: InputString    ! Input String
   CHARACTER(len=len(InputString)) ResultString ! Result String, string is limited to
                                                  ! MaxInputLineLength because of PowerStation Compiler
                                                  ! otherwise could say (CHARACTER(len=LEN(InputString))


          ! FUNCTION PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! FUNCTION LOCAL VARIABLE DECLARATIONS:
  INTEGER i                  ! Loop Counter
  INTEGER :: CurCharVal

    ResultString=InputString

    do i = 1, LEN_TRIM(InputString)
      curCharVal = ICHAR(InputString(i:i))
      SELECT CASE (curCharVal)
        CASE (97:122,224:255) !lowercase ASCII and accented characters
          ResultString(i:i) = CHAR(curCharVal-32)
        CASE DEFAULT
      END SELECT
    end do

  RETURN

END FUNCTION MakeUPPERCase

LOGICAL FUNCTION SameString(TestString1,TestString2)

          ! FUNCTION INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   November 1997
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This function returns true if the two strings are equal (case insensitively)

          ! METHODOLOGY EMPLOYED:
          ! Make both strings uppercase.  Do internal compare.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine


          ! FUNCTION ARGUMENT DEFINITIONS:
   CHARACTER(len=*), INTENT(IN) :: TestString1  ! First String to Test
   CHARACTER(len=*), INTENT(IN) :: TestString2  ! Second String to Test


          ! FUNCTION PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! FUNCTION LOCAL VARIABLE DECLARATIONS:
          ! na

  IF (LEN_TRIM(TestString1) /= LEN_TRIM(TestString2)) THEN
    SameString=.false.
  ELSEIF (TestString1 == TestString2) THEN
    SameString=.true.
  ELSE
    SameString=MakeUPPERCase(TestString1) == MakeUPPERCase(TestString2)
  ENDIF

  RETURN

END FUNCTION SameString

SUBROUTINE VerifyName(NameToVerify,NamesList,NumOfNames,ErrorFound,IsBlank,StringToDisplay)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   February 2000
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine verifys that a new name can be added to the
          ! list of names for this item (i.e., that there isn't one of that
          ! name already and that this name is not blank).

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  CHARACTER(len=*), INTENT(IN)               :: NameToVerify
  CHARACTER(len=*), DIMENSION(:), INTENT(IN) :: NamesList
  INTEGER, INTENT(IN)                        :: NumOfNames
  LOGICAL, INTENT(OUT)                       :: ErrorFound
  LOGICAL, INTENT(OUT)                       :: IsBlank
  CHARACTER(len=*), INTENT(IN)               :: StringToDisplay

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  INTEGER Found

  ErrorFound=.false.
  IF (NumOfNames > 0) THEN
    Found=FindItemInList(NameToVerify,NamesList,NumOfNames)
    IF (Found /= 0) THEN
      CALL ShowSevereError(TRIM(StringToDisplay)//', duplicate name='//TRIM(NameToVerify),EPObjects)
      ErrorFound=.true.
    ENDIF
  ENDIF

  IF (NameToVerify == '     ') THEN
    CALL ShowSevereError(TRIM(StringToDisplay)//', cannot be blank',EPObjects)
    ErrorFound=.true.
    IsBlank=.true.
  ELSE
    IsBlank=.false.
  ENDIF

  RETURN

END SUBROUTINE VerifyName

SUBROUTINE RangeCheck(ErrorsFound,WhatFieldString,WhatObjectString,ErrorLevel,  &
                      LowerBoundString,LowerBoundCondition,UpperBoundString,UpperBoundCondition,  &
                      ValueString)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   July 2000
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine is a general purpose "range check" routine for GetInput routines.
          ! Using the standard "ErrorsFound" logical, this routine can produce a reasonable
          ! error message to describe the situation in addition to setting the ErrorsFound variable
          ! to true.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  LOGICAL, INTENT(INOUT)                 :: ErrorsFound          ! Set to true if error detected
  CHARACTER(len=*), INTENT(IN)           :: WhatFieldString      ! Descriptive field for string
  CHARACTER(len=*), INTENT(IN)           :: WhatObjectString     ! Descriptive field for object, Zone Name, etc.
  CHARACTER(len=*), INTENT(IN)           :: ErrorLevel           ! 'Warning','Severe','Fatal')
  CHARACTER(len=*), INTENT(IN), OPTIONAL :: LowerBoundString     ! String for error message, if applicable
  LOGICAL, INTENT(IN), OPTIONAL          :: LowerBoundCondition  ! Condition for error condition, if applicable
  CHARACTER(len=*), INTENT(IN), OPTIONAL :: UpperBoundString     ! String for error message, if applicable
  LOGICAL, INTENT(IN), OPTIONAL          :: UpperBoundCondition  ! Condition for error condition, if applicable
  CHARACTER(len=*), INTENT(IN), OPTIONAL :: ValueString          ! Value with digits if to be displayed with error

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  CHARACTER(len=7) ErrorString  ! Uppercase representation of ErrorLevel
  LOGICAL Error
  CHARACTER(len=120) Message

  Error=.false.
  IF (PRESENT(UpperBoundCondition)) THEN
    IF (.not. UpperBoundCondition) Error=.true.
  ENDIF
  IF (PRESENT(LowerBoundCondition)) THEN
    IF (.not. LowerBoundCondition) Error=.true.
  ENDIF

  IF (Error) THEN
    CALL ConvertCasetoUPPER(ErrorLevel,ErrorString)
    Message='Out of range value field='//TRIM(WhatFieldString)//','
    IF (PRESENT(ValueString)) Message=trim(Message)//', Value=['//trim(ValueString)//']'
    Message=trim(Message)//', range={'
    IF (PRESENT(LowerBoundString)) Message=TRIM(Message)//TRIM(LowerBoundString)
    IF (PRESENT(LowerBoundString) .and. PRESENT(UpperBoundString)) THEN
      Message=TRIM(Message)//' and '//TRIM(UpperBoundString)
    ELSEIF (PRESENT(UpperBoundString)) THEN
      Message=TRIM(Message)//TRIM(UpperBoundString)
    ENDIF
    Message=TRIM(Message)//'}, for item='//TRIM(WhatObjectString)

    SELECT CASE(ErrorString(1:1))

    CASE('W','w')
      CALL ShowWarningError(TRIM(Message))

    CASE('S','s')
      CALL ShowSevereError(TRIM(Message))
      ErrorsFound=.true.

    CASE('F','f')
      CALL ShowFatalError(TRIM(Message))

    CASE DEFAULT
      CALL ShowSevereError(TRIM(Message))
      ErrorsFound=.true.

    END SELECT

  ENDIF

  RETURN

END SUBROUTINE RangeCheck

SUBROUTINE InternalRangeCheck(Value,FieldNumber,WhichObject,PossibleAlpha,AutoSizable,AutoCalculatable)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   July 2000
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine is an internal range check that checks fields which have
          ! the \min and/or \max values set for appropriate values.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  REAL(r64), INTENT(IN)             :: Value
  INTEGER, INTENT(IN)          :: FieldNumber
  INTEGER, INTENT(IN)          :: WhichObject
  CHARACTER(len=*), INTENT(IN) :: PossibleAlpha
  LOGICAL, INTENT(IN)          :: AutoSizable
  LOGICAL, INTENT(IN)          :: AutoCalculatable

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  LOGICAL Error
  CHARACTER(len=20) FieldString
  CHARACTER(len=MaxObjectNameLength) FieldNameString
  CHARACTER(len=25) ValueString
  CHARACTER(len=300) Message

  Error=.false.
  IF (ObjectDef(WhichObject)%NumRangeChks(FieldNumber)%WhichMinMax(1) == 1) THEN
    IF (Value < ObjectDef(WhichObject)%NumRangeChks(FieldNumber)%MinMaxValue(1)) Error=.true.
  ELSEIF (ObjectDef(WhichObject)%NumRangeChks(FieldNumber)%WhichMinMax(1) == 2) THEN
    IF (Value <= ObjectDef(WhichObject)%NumRangeChks(FieldNumber)%MinMaxValue(1)) Error=.true.
  ENDIF
  IF (ObjectDef(WhichObject)%NumRangeChks(FieldNumber)%WhichMinMax(2) == 3) THEN
    IF (Value > ObjectDef(WhichObject)%NumRangeChks(FieldNumber)%MinMaxValue(2)) Error=.true.
  ELSEIF (ObjectDef(WhichObject)%NumRangeChks(FieldNumber)%WhichMinMax(2) == 4) THEN
    IF (Value >= ObjectDef(WhichObject)%NumRangeChks(FieldNumber)%MinMaxValue(2)) Error=.true.
  ENDIF

  IF (Error) THEN
    IF (.not. (AutoSizable .and. Value == ObjectDef(WhichObject)%NumRangeChks(FieldNumber)%AutoSizeValue) .and.   &
        .not. (AutoCalculatable .and. Value == ObjectDef(WhichObject)%NumRangeChks(FieldNumber)%AutoCalculateValue)) THEN
      NumOutOfRangeErrorsFound=NumOutOfRangeErrorsFound+1
      IF (ReportRangeCheckErrors) THEN
        WRITE(FieldString,*) FieldNumber
        FieldString=ADJUSTL(FieldString)
        FieldNameString=ObjectDef(WhichObject)%NumRangeChks(FieldNumber)%FieldName
        WRITE(ValueString,'(F20.5)') Value
        ValueString=ADJUSTL(ValueString)
        IF (FieldNameString /= Blank) THEN
          Message='Out of range value Numeric Field#'//TRIM(FieldString)//' ('//TRIM(FieldNameString)//  &
                     '), value='//TRIM(ValueString)//', range={'
        ELSE ! Field Name not recorded
          Message='Out of range value Numeric Field#'//TRIM(FieldString)//', value='//TRIM(ValueString)//', range={'
        ENDIF
        IF (ObjectDef(WhichObject)%NumRangeChks(FieldNumber)%WhichMinMax(1) /= 0) &
                   Message=TRIM(Message)//ObjectDef(WhichObject)%NumRangeChks(FieldNumber)%MinMaxString(1)
        IF (ObjectDef(WhichObject)%NumRangeChks(FieldNumber)%WhichMinMax(1) /= 0 .and. &
            ObjectDef(WhichObject)%NumRangeChks(FieldNumber)%WhichMinMax(2) /= 0) THEN
          Message=TRIM(Message)//' and '//ObjectDef(WhichObject)%NumRangeChks(FieldNumber)%MinMaxString(2)
        ELSEIF (ObjectDef(WhichObject)%NumRangeChks(FieldNumber)%WhichMinMax(2) /= 0) THEN
          Message=TRIM(Message)//ObjectDef(WhichObject)%NumRangeChks(FieldNumber)%MinMaxString(2)
        ENDIF
        Message=TRIM(Message)//'}, in '//TRIM(ObjectDef(WhichObject)%Name)
        IF (ObjectDef(WhichObject)%NameAlpha1) THEN
          Message=TRIM(Message)//'='//PossibleAlpha
        ENDIF
        CALL ShowSevereError(TRIM(Message),EchoInputFile,EPObjects)
      ENDIF
    ENDIF
  ENDIF

  RETURN

END SUBROUTINE InternalRangeCheck

SUBROUTINE TurnOnReportRangeCheckErrors

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   July 2000
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine turns on the logical to report range check errors
          ! directly out of the InputProcessor.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

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
          ! na

  ReportRangeCheckErrors=.true.

  RETURN

END SUBROUTINE TurnOnReportRangeCheckErrors

SUBROUTINE TurnOffReportRangeCheckErrors

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   July 20000
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine turns off the logical to report range check errors
          ! directly out of the InputProcessor.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

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
          ! na

  ReportRangeCheckErrors=.false.

  RETURN

END SUBROUTINE TurnOffReportRangeCheckErrors

INTEGER FUNCTION GetNumRangeCheckErrorsFound()

          ! FUNCTION INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   July 2000
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS FUNCTION:
          ! This function returns the number of OutOfRange errors found during
          ! input processing.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! FUNCTION ARGUMENT DEFINITIONS:
          ! na

          ! FUNCTION PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! FUNCTION LOCAL VARIABLE DECLARATIONS:
          ! na

  GetNumRangeCheckErrorsFound=NumOutOfRangeErrorsFound

  RETURN

END FUNCTION GetNumRangeCheckErrorsFound

!==============================================================================
! The following routines allow access to the definition lines of the IDD and
! thus can be used to "report" on expected arguments for the Input Processor.

INTEGER FUNCTION GetNumObjectsInIDD()

          ! FUNCTION INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   May 1998
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine returns the number of objects found in the IDD and
          ! can be used to allocate the array for determining the definitions.

          ! METHODOLOGY EMPLOYED:
          ! Essentially allows outside access to an internal variable of the InputProcessor.
          ! Used primarily by utility programs that use the InputProcessor outside of the
          ! "true" EnergyPlus code.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

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
          ! na


  GetNumObjectsInIDD=NumObjectDefs

  RETURN

END FUNCTION GetNumObjectsInIDD

SUBROUTINE GetListOfObjectsInIDD(ObjectNames,Number)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   May 1998
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine returns the list of Object names that occur in the IDD.

          ! METHODOLOGY EMPLOYED:
          ! Essentially allows outside access to an internal variable of the InputProcessor.
          ! Used primarily by utility programs that use the InputProcessor outside of the
          ! "true" EnergyPlus code.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  CHARACTER(len=*),  &
              DIMENSION(:), INTENT(OUT) :: ObjectNames  ! List of Object Names (from IDD)
  INTEGER, INTENT(OUT)                  :: Number       ! Number in List

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
          ! na

  ObjectNames(1:NumObjectDefs)=ObjectDef(1:NumObjectDefs)%Name
  Number=NumObjectDefs
  RETURN

END SUBROUTINE GetListOfObjectsInIDD


SUBROUTINE GetObjectDefInIDD(ObjectWord,NumArgs,AlphaorNumeric,RequiredFields,MinNumFields)
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   May 1998
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine returns the "definition" of an Object from the IDD.  This is
          ! the "maximum" definition with total number of arguments, and whether each argument
          ! is "alpha" or "numeric".

          ! METHODOLOGY EMPLOYED:
          ! Essentially allows outside access to an internal variable of the InputProcessor.
          ! Used primarily by utility programs that use the InputProcessor outside of the
          ! "true" EnergyPlus code.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  CHARACTER(len=*), INTENT(IN) :: ObjectWord ! Object for definition
  INTEGER, INTENT(OUT) :: NumArgs                              ! How many arguments (max) this Object can have
  LOGICAL, INTENT(OUT), DIMENSION(:) :: AlphaorNumeric         ! Array designating Alpha (true) or Numeric (false) for each
                                                               ! argument
  LOGICAL, INTENT(OUT), DIMENSION(:) :: RequiredFields         ! Array designating RequiredFields (true) for each argument
  INTEGER, INTENT(OUT) :: MinNumFields                         ! Minimum Number of Fields to be returned to Get routines

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  INTEGER Which  ! to determine which object definition to use

  IF (SortedIDD) THEN
!    Which=FindItemInSortedList(ObjectWord,ListOfObjects,NumObjectDefs)
!    IF (Which /= 0) Which=iListofObjects(Which)
  ELSE
    Which=FindItemInList(ObjectWord,ListOfObjects,NumObjectDefs)
  ENDIF
  NumArgs=ObjectDef(Which)%NumParams
  AlphaorNumeric(1:NumArgs)=ObjectDef(Which)%AlphaorNumeric(1:NumArgs)
  RequiredFields(1:NumArgs)=ObjectDef(Which)%ReqField(1:NumArgs)
  MinNumFields=ObjectDef(Which)%MinNumFields

  RETURN

END SUBROUTINE GetObjectDefInIDD


SUBROUTINE AddRecordFromSection(Which)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   March 2009
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! When an object is entered like a section (i.e., <objectname>;), try to add a record
          ! of the object using minfields, etc.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  INTEGER, INTENT(IN) :: Which ! Which object was matched

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  INTEGER :: NumArg
  INTEGER :: NumAlpha
  INTEGER :: NumNumeric
!  LOGICAL :: ErrFlag
  INTEGER :: Count
  CHARACTER(len=30) :: String

  NumArg=0
  LineItem%Name=ObjectDef(Which)%Name
  LineItem%Alphas=' '
  LineItem%AlphBlank=.false.
  LineItem%NumAlphas=0
  LineItem%Numbers=0.0
  LineItem%NumNumbers=0
  LineItem%NumBlank=.false.
  LineItem%ObjectDefPtr=Which

  ObjectDef(Which)%NumFound=ObjectDef(Which)%NumFound+1

  ! Check out MinimumNumberOfFields
  IF (NumArg < ObjectDef(Which)%MinNumFields) THEN
    IF (ObjectDef(Which)%NameAlpha1) THEN
      CALL ShowAuditErrorMessage(' ** Warning ** ','IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
                          ' Object='//TRIM(ObjectDef(Which)%Name)//  &
                          ', name='//TRIM(LineItem%Alphas(1))//       &
                          ', entered with less than minimum number of fields.')
    ELSE
      CALL ShowAuditErrorMessage(' ** Warning ** ','IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
                          ' Object='//TRIM(ObjectDef(Which)%Name)//  &
                          ', entered with less than minimum number of fields.')
    ENDIF
    CALL ShowAuditErrorMessage(' **   ~~~   ** ','Attempting fill to minimum.')
    NumAlpha=0
    NumNumeric=0
    IF (ObjectDef(Which)%MinNumFields > ObjectDef(Which)%NumParams) THEN
      CALL ShowSevereError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
                    ' Object \min-fields > number of fields specified, Object='//TRIM(ObjectDef(Which)%Name),EPObjects)
      CALL ShowContinueError('..\min-fields='//TRIM(IPTrimSigDigits(ObjectDef(Which)%MinNumFields))//  &
                   ', total number of fields in object definition='//TRIM(IPTrimSigDigits(ObjectDef(Which)%NumParams)),EPObjects)
!      ErrFlag=.true.
    ELSE
      DO Count=1,ObjectDef(Which)%MinNumFields
        IF (ObjectDef(Which)%AlphaOrNumeric(Count)) THEN
          NumAlpha=NumAlpha+1
          IF (NumAlpha <= LineItem%NumAlphas) CYCLE
          LineItem%NumAlphas=LineItem%NumAlphas+1
          IF (ObjectDef(Which)%AlphFieldDefs(LineItem%NumAlphas) /= Blank) THEN
            LineItem%Alphas(LineItem%NumAlphas)=ObjectDef(Which)%AlphFieldDefs(LineItem%NumAlphas)
            CALL ShowAuditErrorMessage(' **   Add   ** ',TRIM(ObjectDef(Which)%AlphFieldDefs(LineItem%NumAlphas))//   &
                                '   ! field=>'//TRIM(ObjectDef(Which)%AlphFieldChks(NumAlpha)))
          ELSEIF (ObjectDef(Which)%ReqField(Count)) THEN
            IF (ObjectDef(Which)%NameAlpha1) THEN
              CALL ShowSevereError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
                                  ' Object='//TRIM(ObjectDef(Which)%Name)//  &
                                  ', name='//TRIM(LineItem%Alphas(1))// &
                                  ', Required Field=['//  &
                                  TRIM(ObjectDef(Which)%AlphFieldChks(NumAlpha))//   &
                                  '] was blank.',EchoInputFile,EPObjects)
            ELSE
              CALL ShowSevereError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
                                  ' Object='//TRIM(ObjectDef(Which)%Name)//  &
                                  ', Required Field=['//  &
                                  TRIM(ObjectDef(Which)%AlphFieldChks(NumAlpha))//   &
                                  '] was blank.',EchoInputFile,EPObjects)
            ENDIF
!            ErrFlag=.true.
          ELSE
            LineItem%Alphas(LineItem%NumAlphas)=Blank
            LineItem%AlphBlank(LineItem%NumAlphas)=.true.
            CALL ShowAuditErrorMessage(' **   Add   ** ','<blank field>   ! field=>'//  &
                                 TRIM(ObjectDef(Which)%AlphFieldChks(NumAlpha)))
          ENDIF
        ELSE
          NumNumeric=NumNumeric+1
          IF (NumNumeric <= LineItem%NumNumbers) CYCLE
          LineItem%NumNumbers=LineItem%NumNumbers+1
          LineItem%NumBlank(NumNumeric)=.true.
          IF (ObjectDef(Which)%NumRangeChks(NumNumeric)%Defaultchk) THEN
            IF (.not. ObjectDef(Which)%NumRangeChks(NumNumeric)%DefAutoSize .and.   &
                .not. ObjectDef(Which)%NumRangeChks(NumNumeric)%DefAutoCalculate) THEN
              LineItem%Numbers(NumNumeric)=ObjectDef(Which)%NumRangeChks(NumNumeric)%Default
              WRITE(String,*) ObjectDef(Which)%NumRangeChks(NumNumeric)%Default
              String=ADJUSTL(String)
              CALL ShowAuditErrorMessage(' **   Add   ** ',TRIM(String)//  &
                                  '   ! field=>'//TRIM(ObjectDef(Which)%NumRangeChks(NumNumeric)%FieldName))
            ELSEIF (ObjectDef(Which)%NumRangeChks(NumNumeric)%DefAutoSize) THEN
              LineItem%Numbers(NumNumeric)=ObjectDef(Which)%NumRangeChks(NumNumeric)%AutoSizeValue
              CALL ShowAuditErrorMessage(' **   Add   ** ','autosize    ! field=>'//  &
                                  TRIM(ObjectDef(Which)%NumRangeChks(NumNumeric)%FieldName))
            ELSEIF (ObjectDef(Which)%NumRangeChks(NumNumeric)%DefAutoCalculate) THEN
              LineItem%Numbers(NumNumeric)=ObjectDef(Which)%NumRangeChks(NumNumeric)%AutoCalculateValue
              CALL ShowAuditErrorMessage(' **   Add   ** ','autocalculate    ! field=>'//  &
                                  TRIM(ObjectDef(Which)%NumRangeChks(NumNumeric)%FieldName))
            ENDIF
          ELSEIF (ObjectDef(Which)%ReqField(Count)) THEN
            IF (ObjectDef(Which)%NameAlpha1) THEN
              CALL ShowSevereError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
                                  ' Object='//TRIM(ObjectDef(Which)%Name)//  &
                                  ', name='//TRIM(LineItem%Alphas(1))// &
                                  ', Required Field=['//  &
                                  TRIM(ObjectDef(Which)%NumRangeChks(NumNumeric)%FieldName)//   &
                                  '] was blank.',EchoInputFile,EPObjects)
            ELSE
              CALL ShowSevereError('IP: IDF line~'//TRIM(IPTrimSigDigits(NumLines))//  &
                                  ' Object='//TRIM(ObjectDef(Which)%Name)//  &
                                  ', Required Field=['//  &
                                  TRIM(ObjectDef(Which)%NumRangeChks(NumNumeric)%FieldName)//   &
                                  '] was blank.',EchoInputFile,EPObjects)
            ENDIF
!            ErrFlag=.true.
          ELSE
            LineItem%Numbers(NumNumeric)=0.0
            LineItem%NumBlank(NumNumeric)=.true.
            CALL ShowAuditErrorMessage(' **   Add   ** ','<blank field>   ! field=>'//  &
                                TRIM(ObjectDef(Which)%NumRangeChks(NumNumeric)%FieldName))
          ENDIF
        ENDIF
      ENDDO
    ENDIF
  ENDIF

!  IF (TransitionDefer) THEN
!    CALL MakeTransition(Which)
!  ENDIF
  NumIDFRecords=NumIDFRecords+1
  IF (ObjectStartRecord(Which) == 0) ObjectStartRecord(Which)=NumIDFRecords
  MaxAlphaIDFArgsFound=MAX(MaxAlphaIDFArgsFound,LineItem%NumAlphas)
  MaxNumericIDFArgsFound=MAX(MaxNumericIDFArgsFound,LineItem%NumNumbers)
  MaxAlphaIDFDefArgsFound=MAX(MaxAlphaIDFDefArgsFound,ObjectDef(Which)%NumAlpha)
  MaxNumericIDFDefArgsFound=MAX(MaxNumericIDFDefArgsFound,ObjectDef(Which)%NumNumeric)
  IDFRecords(NumIDFRecords)%Name=LineItem%Name
  IDFRecords(NumIDFRecords)%NumNumbers=LineItem%NumNumbers
  IDFRecords(NumIDFRecords)%NumAlphas=LineItem%NumAlphas
  IDFRecords(NumIDFRecords)%ObjectDefPtr=LineItem%ObjectDefPtr
  ALLOCATE(IDFRecords(NumIDFRecords)%Alphas(LineItem%NumAlphas))
  ALLOCATE(IDFRecords(NumIDFRecords)%AlphBlank(LineItem%NumAlphas))
  ALLOCATE(IDFRecords(NumIDFRecords)%Numbers(LineItem%NumNumbers))
  ALLOCATE(IDFRecords(NumIDFRecords)%NumBlank(LineItem%NumNumbers))
  IDFRecords(NumIDFRecords)%Alphas(1:LineItem%NumAlphas)=LineItem%Alphas(1:LineItem%NumAlphas)
  IDFRecords(NumIDFRecords)%AlphBlank(1:LineItem%NumAlphas)=LineItem%AlphBlank(1:LineItem%NumAlphas)
  IDFRecords(NumIDFRecords)%Numbers(1:LineItem%NumNumbers)=LineItem%Numbers(1:LineItem%NumNumbers)
  IDFRecords(NumIDFRecords)%NumBlank(1:LineItem%NumNumbers)=LineItem%NumBlank(1:LineItem%NumNumbers)
  IF (LineItem%NumNumbers > 0) THEN
    DO Count=1,LineItem%NumNumbers
      IF (ObjectDef(Which)%NumRangeChks(Count)%MinMaxChk .and. .not. LineItem%NumBlank(Count)) THEN
        CALL InternalRangeCheck(LineItem%Numbers(Count),Count,Which,LineItem%Alphas(1),  &
                                ObjectDef(Which)%NumRangeChks(Count)%AutoSizable,        &
                                ObjectDef(Which)%NumRangeChks(Count)%AutoCalculatable)
      ENDIF
    ENDDO
  ENDIF

  RETURN

END SUBROUTINE AddRecordFromSection

SUBROUTINE DumpCurrentLineBuffer(StartLine,NumConxLines,LineBuf,LineBufLen)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   February 2003
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine dumps the "context" lines for error messages detected by
          ! the input processor.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  INTEGER, INTENT(IN)               :: StartLine
  INTEGER, INTENT(IN)               :: NumConxLines
  CHARACTER(len=*), INTENT(IN)      :: LineBuf
  INTEGER, INTENT(IN), DIMENSION(:) :: LineBufLen

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  INTEGER Line
  INTEGER PLine
  INTEGER SLine
  INTEGER CurPos
  CHARACTER(len=300) TextLine

  CALL ShowMessage('IDF Context for following error/warning message:')
  CALL ShowMessage('Note -- lines truncated at 280 characters, if necessary...')
  WRITE(TextLine,'(1X,I5,1X,A)') StartLine,LineBuf(1:MIN(280,LineBufLen(1)))
  CALL ShowMessage(TRIM(TextLine))
  IF (NumConxLines > 10) THEN
    CALL ShowMessage('Only last 10 lines before error line shown.....')
    PLine=NumConxLines-10
  ELSE
    PLine=2
  ENDIF
  CurPos=0
  SLine=StartLine
  DO Line=1,PLine-1
    CurPos=CurPos+LineBufLen(Line)
    SLine=SLine+1
  ENDDO
  CurPos=CurPos+1
  DO Line=PLine,NumConxLines
    WRITE(TextLine,'(1X,I5,1X,A)') SLine,TRIM(LineBuf(CurPos:MIN(CurPos+279,CurPos+LineBufLen(Line))))
    CALL ShowMessage(TRIM(TextLine))
    CurPos=CurPos+LineBufLen(Line)
    SLine=SLine+1
  ENDDO

  RETURN

END SUBROUTINE DumpCurrentLineBuffer

SUBROUTINE ShowAuditErrorMessage(Severity,ErrorMessage)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   March 2003
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine is just for messages that will be displayed on the audit trail
          ! (echo of the input file).  Errors are counted and a summary is displayed after
          ! finishing the scan of the input file.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  CHARACTER(len=*) Severity     ! if blank, does not add to sum
  CHARACTER(len=*) ErrorMessage

          ! SUBROUTINE PARAMETER DEFINITIONS:
  CHARACTER(len=*), PARAMETER :: ErrorFormat='(2X,A)'

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
          ! na

  IF (Severity /= Blank) THEN
    TotalAuditErrors=TotalAuditErrors+1
    WRITE(EchoInputFile,ErrorFormat) Severity//TRIM(ErrorMessage)
  ELSE
    WRITE(EchoInputFile,ErrorFormat) ' ************* '//TRIM(ErrorMessage)
  ENDIF


  RETURN

END SUBROUTINE ShowAuditErrorMessage

FUNCTION IPTrimSigDigits(IntegerValue) RESULT(OutputString)

          ! FUNCTION INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   March 2002
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS FUNCTION:
          ! This function accepts a number as parameter as well as the number of
          ! significant digits after the decimal point to report and returns a string
          ! that is appropriate.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! FUNCTION ARGUMENT DEFINITIONS:
  INTEGER, INTENT(IN) :: IntegerValue
  CHARACTER(len=30) OutputString

          ! FUNCTION PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! FUNCTION LOCAL VARIABLE DECLARATIONS:
  CHARACTER(len=30) String     ! Working string

  WRITE(String,*) IntegerValue
  OutputString=ADJUSTL(String)

  RETURN

END FUNCTION IPTrimSigDigits

!     NOTICE
!
!     Copyright © 1996-2010 The Board of Trustees of the University of Illinois
!     and The Regents of the University of California through Ernest Orlando Lawrence
!     Berkeley National Laboratory.  All rights reserved.
!
!     Portions of the EnergyPlus software package have been developed and copyrighted
!     by other individuals, companies and institutions.  These portions have been
!     incorporated into the EnergyPlus software package under license.   For a complete
!     list of contributors, see "Notice" located in EnergyPlus.f90.
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

END MODULE InputProcessor
