!****************************************************************************
!  Global data, parameters, and structures
!****************************************************************************

module DataVCompareGlobals

USE DataGlobals, ONLY: MaxNameLength

implicit none

!  Parameters

integer*4, parameter, public :: SIZEOFAPPNAME = 100

integer, parameter :: MisMatchUnits=8
integer, parameter :: DiffNumParams=1
integer, parameter :: MisMatchFields=4
integer, parameter :: MisMatchArgs=2

integer, parameter, dimension(8) :: DiffIndex=(/DiffNumParams,MisMatchArgs,MisMatchFields,  &
                                       MisMatchUnits,MisMatchFields+MisMatchUnits,DiffNumParams+MisMatchFields,  &
                                       DiffNumParams+MisMatchUnits,DiffNumParams+MisMatchUnits+MisMatchFields/)
character(len=*), parameter, dimension(0:8) :: DiffDescription=(/"<unknown>                          ",  &
                                                                 "Diff # Fields                      ",  &
                                                                 "Arg Type (A-N) Mismatch            ",  &
                                                                 "Field Name Change                  ",  &
                                                                 "Units Change                       ",  &
                                                                 "Units Chg+Field Name Chg           ",  &
                                                                 "#Fields+Field Name Chg             ", &
                                                                 "#Fields+Units Change               ", &
                                                                 "#Fields+Field Name Chg+Units Change"/)
  TYPE ObjectStatus
    CHARACTER(len=MaxNameLength) :: Name  = ' '! Object Name
    LOGICAL :: Same                       = .true. ! Definitions are same (true) or different
    INTEGER :: StatusFlag                 = 0 ! What's different
    INTEGER :: OldIndex                   = 0 ! Where this is in ObjectDef
    INTEGER :: NewIndex                   = 0 ! Where this is in NewObjectDef
    LOGICAL :: UnitsMatched               =.false.
    LOGICAL, ALLOCATABLE, DIMENSION(:) :: FieldNameMatch
    CHARACTER(len=MaxNameLength), ALLOCATABLE, DIMENSION(:) :: UnitsMatch
  END TYPE

!  Global data

integer		ghInstance
integer		ghModule
integer		ghwndMain
integer		ghMenu

character(len=270) FullFileName
character(len=255) FileNamePath
integer FullFileNameLength
integer FileNamePathLength
character(len=255) FileErrorMessage
logical FileOK
character(len=255) CurWorkDir
character(len=270) IDDFileNameWithPath
character(len=270) NewIDDFileNameWithPath
CHARACTER(len=255) :: RepVarFileNameWithPath

logical withUnits  ! True if units should be displayed on output lines (by field names)
logical LeaveBlank  ! True if blank fields on input should be left blank (no default fill)
integer auditf  ! auditfile
real  :: VersionNum=0.0
CHARACTER(len=3) :: sVersionNum=''
CHARACTER(len=4) :: sVersionNumFourChars=''

! Added for compare routines

TYPE(ObjectStatus), ALLOCATABLE, DIMENSION(:) :: ObjStatus
INTEGER NumObjStats
CHARACTER(len=MaxNameLength), ALLOCATABLE, DIMENSION(:) :: NotInNew
CHARACTER(len=MaxNameLength), ALLOCATABLE, DIMENSION(:) :: NotInOld
CHARACTER(len=MaxNameLength), ALLOCATABLE, DIMENSION(:) :: ObsObject
CHARACTER(len=MaxNameLength), ALLOCATABLE, DIMENSION(:) :: ObsObjRepName
INTEGER NumObsObjs
INTEGER NNew
INTEGER NOld
INTEGER NumDif
CHARACTER(len=MaxNameLength*2), ALLOCATABLE, DIMENSION(:) :: FldNames
CHARACTER(len=MaxNameLength), ALLOCATABLE, DIMENSION(:) :: FldDefaults
CHARACTER(len=20), ALLOCATABLE, DIMENSION(:) :: FldUnits
INTEGER ObjMinFlds
LOGICAL, ALLOCATABLE, DIMENSION(:) :: AOrN
LOGICAL, ALLOCATABLE, DIMENSION(:) :: ReqFld
INTEGER NumArgs   ! Number of Arguments in a definition
CHARACTER(len=MaxNameLength*2), ALLOCATABLE, DIMENSION(:) :: NwFldNames
CHARACTER(len=MaxNameLength), ALLOCATABLE, DIMENSION(:) :: NwFldDefaults
CHARACTER(len=20), ALLOCATABLE, DIMENSION(:) :: NwFldUnits
INTEGER NwObjMinFlds
LOGICAL, ALLOCATABLE, DIMENSION(:) :: NwAOrN
LOGICAL, ALLOCATABLE, DIMENSION(:) :: NwReqFld
INTEGER NwNumArgs   ! Number of Arguments in a definition
CHARACTER(len=MaxNameLength), ALLOCATABLE, DIMENSION(:) :: Alphas
CHARACTER(len=MaxNameLength), ALLOCATABLE, DIMENSION(:) :: Numbers
INTEGER NumAlphas
INTEGER NumNumbers
CHARACTER(len=MaxNameLength), ALLOCATABLE, DIMENSION(:) :: OutArgs
INTEGER, ALLOCATABLE, DIMENSION(:) :: MatchArg
CHARACTER(len=MaxNameLength), ALLOCATABLE, DIMENSION(:) :: InArgs
CHARACTER(len=MaxNameLength), ALLOCATABLE, DIMENSION(:) :: TempArgs

CHARACTER(len=MaxNameLength), DIMENSION(:), ALLOCATABLE :: OldRepVarName
CHARACTER(len=MaxNameLength), DIMENSION(:), ALLOCATABLE :: NewRepVarName
CHARACTER(len=MaxNameLength), DIMENSION(:), ALLOCATABLE :: NewRepVarCaution
LOGICAL, DIMENSION(:), ALLOCATABLE :: OutVarCaution
LOGICAL, DIMENSION(:), ALLOCATABLE :: MtrVarCaution
LOGICAL, DIMENSION(:), ALLOCATABLE :: TimeBinVarCaution
LOGICAL, DIMENSION(:), ALLOCATABLE :: OTMVarCaution
LOGICAL, DIMENSION(:), ALLOCATABLE :: CMtrVarCaution
LOGICAL, DIMENSION(:), ALLOCATABLE :: CMtrDVarCaution
INTEGER NumRepVarNames

LOGICAL :: MakingPretty=.false.
INTEGER, ALLOCATABLE, DIMENSION(:) :: ObjectFoundCounts
CHARACTER(len=120), ALLOCATABLE, DIMENSION(:) :: ObjectFoundFile
CHARACTER(len=120), DIMENSION(:), ALLOCATABLE :: ReportNames
INTEGER, ALLOCATABLE, DIMENSION(:) :: ReportNamesCounts
CHARACTER(len=120), DIMENSION(:), ALLOCATABLE :: ReportNameFile
CHARACTER(len=120), DIMENSION(:), ALLOCATABLE :: TmpReportNames
INTEGER, ALLOCATABLE, DIMENSION(:) :: TmpReportNamesCounts
INTEGER :: NumReportNames=0
INTEGER :: MaxReportNames=0

CHARACTER(len=255) :: InputFilePath
LOGICAL :: UseInputFilePath=.false.
LOGICAL :: ProcessingIMFFile=.false.

CHARACTER(len=MaxNameLength), DIMENSION(:), ALLOCATABLE :: OldObjectNames
CHARACTER(len=MaxNameLength), DIMENSION(:), ALLOCATABLE :: NewObjectNames
INTEGER :: NumRenamedObjects=0

! Following for V8 only.
INTEGER :: NumChillers=0
INTEGER :: numChillerHeaters=0
LOGICAL :: CondFDVariables=.false.

end module DataVCompareGlobals
