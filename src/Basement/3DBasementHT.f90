MODULE General    !  UTILITY OUTPUT PROCESSING ROUTINES

IMPLICIT NONE

INTERFACE RoundSigDigits
  MODULE PROCEDURE r64RoundSigDigits, rRoundSigDigits, iRoundSigDigits
END INTERFACE RoundSigDigits
INTERFACE TrimSigDigits
  MODULE PROCEDURE r64TrimSigDigits, rTrimSigDigits, iTrimSigDigits
END INTERFACE TrimSigDigits
INTERFACE SafeDivide
  MODULE PROCEDURE dSafeDivide, rSafeDivide
END INTERFACE SafeDivide

CONTAINS

FUNCTION r64TrimSigDigits(RealValue,SigDigits) RESULT(OutputString)

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
  USE DataPrecisionGlobals

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! FUNCTION ARGUMENT DEFINITIONS:
  REAL(r64), INTENT(IN) :: RealValue
  INTEGER, INTENT(IN) :: SigDigits
  CHARACTER(len=30) OutputString

          ! FUNCTION PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! FUNCTION LOCAL VARIABLE DECLARATIONS:
  INTEGER DotPos               ! Position of decimal point in original string
  INTEGER EPos                 ! Position of E in original string format xxEyy
  INTEGER SLen                 ! Length of String (w/o E part)
  CHARACTER(len=30) String     ! Working string
  CHARACTER(len=10) EString    ! E string retained from original string
  LOGICAL IncludeDot           ! True when decimal point output

  IF (RealValue /= 0.0d0) THEN
    WRITE(String,*) RealValue
  ELSE
    String='0.000000000000000000000000000'
  ENDIF
  EPos=INDEX(String,'E')
  IF (EPos > 0) THEN
    EString=String(EPos:)
    String(EPos:)=' '
  ELSE
    EString=' '
  ENDIF
  DotPos=INDEX(String,'.')
  SLen=LEN_TRIM(String)
  IF (SigDigits > 0 .or. EString /= ' ') THEN
    IncludeDot=.true.
  ELSE
    IncludeDot=.false.
  ENDIF
  IF (IncludeDot) THEN
    String=String(1:MIN(DotPos+SigDigits,SLen))//EString
  ELSE
    String=String(1:DotPos-1)
  ENDIF
  IF (IsNAN(RealValue)) THEN
    String='NAN'
  ENDIF
  OutputString=ADJUSTL(String)

  RETURN

END FUNCTION r64TrimSigDigits

FUNCTION rTrimSigDigits(RealValue,SigDigits) RESULT(OutputString)

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
  REAL, INTENT(IN) :: RealValue
  INTEGER, INTENT(IN) :: SigDigits
  CHARACTER(len=30) OutputString

          ! FUNCTION PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! FUNCTION LOCAL VARIABLE DECLARATIONS:
  INTEGER DotPos               ! Position of decimal point in original string
  INTEGER EPos                 ! Position of E in original string format xxEyy
  INTEGER SLen                 ! Length of String (w/o E part)
  CHARACTER(len=30) String     ! Working string
  CHARACTER(len=10) EString    ! E string retained from original string
  LOGICAL IncludeDot           ! True when decimal point output

  IF (RealValue /= 0.0) THEN
    WRITE(String,*) RealValue
  ELSE
    String='0.000000000000000000000000000'
  ENDIF
  EPos=INDEX(String,'E')
  IF (EPos > 0) THEN
    EString=String(EPos:)
    String(EPos:)=' '
  ELSE
    EString=' '
  ENDIF
  DotPos=INDEX(String,'.')
  SLen=LEN_TRIM(String)
  IF (SigDigits > 0 .or. EString /= ' ') THEN
    IncludeDot=.true.
  ELSE
    IncludeDot=.false.
  ENDIF
  IF (IncludeDot) THEN
    String=String(1:MIN(DotPos+SigDigits,SLen))//EString
  ELSE
    String=String(1:DotPos-1)
  ENDIF
  IF (IsNAN(RealValue)) THEN
    String='NAN'
  ENDIF
  OutputString=ADJUSTL(String)

  RETURN

END FUNCTION rTrimSigDigits

FUNCTION iTrimSigDigits(IntegerValue,SigDigits) RESULT(OutputString)

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
  INTEGER, INTENT(IN), OPTIONAL :: SigDigits  ! ignored
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

END FUNCTION iTrimSigDigits

FUNCTION r64RoundSigDigits(RealValue,SigDigits) RESULT(OutputString)

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
  USE DataPrecisionGlobals

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! FUNCTION ARGUMENT DEFINITIONS:
  REAL(r64), INTENT(IN) :: RealValue
  INTEGER, INTENT(IN) :: SigDigits
  CHARACTER(len=30) OutputString

          ! FUNCTION PARAMETER DEFINITIONS:
  CHARACTER(len=11) :: DigitChar='01234567890'

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! FUNCTION LOCAL VARIABLE DECLARATIONS:
  INTEGER DotPos               ! Position of decimal point in original string
  INTEGER EPos                 ! Position of E in original string format xxEyy
  INTEGER TPos                 ! Position of Testchar in Digit string
  INTEGER NPos                 ! Position of "next" char in Digit String
  INTEGER TPos1                ! Position of "next" char rounded in Digit string
  INTEGER SPos                 ! Actual string position being replaced
  INTEGER SLen                 ! Length of String (w/o E part)
  CHARACTER(len=30) String     ! Working string
  CHARACTER(len=10) EString    ! E string retained from original string
  CHARACTER(len=1) TestChar    ! Test character (digit) for rounding, if position in digit string > 5 (digit is 5 or greater)
                               ! then will round
  CHARACTER(len=1) Char2Rep    ! Character (digit) to be replaced
  LOGICAL IncludeDot           ! True when decimal point output

  IF (RealValue /= 0.0d0) THEN
    WRITE(String,*) RealValue
  ELSE
    String='0.000000000000000000000000000'
  ENDIF
  EPos=INDEX(String,'E')
  IF (EPos > 0) THEN
    EString=String(EPos:)
    String(Epos:)=' '
  ELSE
    EString=' '
  ENDIF

  DotPos=INDEX(String,'.')
  TestChar=String(DotPos+SigDigits+1:DotPos+SigDigits+1)
  TPos=INDEX(DigitChar,TestChar)

  IF (SigDigits == 0) THEN
    SPos=DotPos-1
  ELSE
    SPos=DotPos+SigDigits
  ENDIF

  IF (TPos > 5) THEN  ! Must round to next Digit
    Char2Rep=String(SPos:SPos)
    NPos=INDEX(DigitChar,Char2Rep)
    String(SPos:SPos)=DigitChar(NPos+1:NPos+1)
    DO WHILE (NPos == 10)
        ! Must change other char too
      IF (SigDigits == 1) THEN
        TestChar=String(SPos-2:SPos-2)
        IF (TestChar == '.') THEN
          TestChar=String(SPos-3:SPos-3)
          SPos=SPos-2
        ENDIF
        IF (TestChar == ' ') TestChar='0'  ! all 999s
        TPos1=INDEX(DigitChar,TestChar)
        String(SPos-2:SPos-2)=DigitChar(TPos1+1:TPos1+1)
      ELSE
        TestChar=String(SPos-1:SPos-1)
        IF (TestChar == '.') THEN
          TestChar=String(SPos-2:SPos-2)
          SPos=SPos-1
        ENDIF
        IF (TestChar == ' ') TestChar='0'  ! all 999s
        TPos1=INDEX(DigitChar,TestChar)
        String(SPos-1:SPos-1)=DigitChar(TPos1+1:TPos1+1)
      ENDIF
      SPos=SPos-1
      NPos=TPos1
    ENDDO
  ENDIF

  SLen=LEN_TRIM(String)
  IF (SigDigits > 0 .or. EString /= ' ') THEN
    IncludeDot=.true.
  ELSE
    IncludeDot=.false.
  ENDIF
  IF (IncludeDot) THEN
    String=String(1:MIN(DotPos+SigDigits,SLen))//EString
  ELSE
    String=String(1:DotPos-1)
  ENDIF
  IF (IsNAN(RealValue)) THEN
    String='NAN'
  ENDIF
  OutputString=ADJUSTL(String)

  RETURN

END FUNCTION r64RoundSigDigits

FUNCTION rRoundSigDigits(RealValue,SigDigits) RESULT(OutputString)

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
  REAL, INTENT(IN) :: RealValue
  INTEGER, INTENT(IN) :: SigDigits
  CHARACTER(len=30) OutputString

          ! FUNCTION PARAMETER DEFINITIONS:
  CHARACTER(len=11) :: DigitChar='01234567890'

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! FUNCTION LOCAL VARIABLE DECLARATIONS:
  INTEGER DotPos               ! Position of decimal point in original string
  INTEGER EPos                 ! Position of E in original string format xxEyy
  INTEGER TPos                 ! Position of Testchar in Digit string
  INTEGER NPos                 ! Position of "next" char in Digit String
  INTEGER TPos1                ! Position of "next" char rounded in Digit string
  INTEGER SPos                 ! Actual string position being replaced
  INTEGER SLen                 ! Length of String (w/o E part)
  CHARACTER(len=30) String     ! Working string
  CHARACTER(len=10) EString    ! E string retained from original string
  CHARACTER(len=1) TestChar    ! Test character (digit) for rounding, if position in digit string > 5 (digit is 5 or greater)
                               ! then will round
  CHARACTER(len=1) Char2Rep    ! Character (digit) to be replaced
  LOGICAL IncludeDot           ! True when decimal point output

  IF (RealValue /= 0.0) THEN
    WRITE(String,*) RealValue
  ELSE
    String='0.000000000000000000000000000'
  ENDIF
  EPos=INDEX(String,'E')
  IF (EPos > 0) THEN
    EString=String(EPos:)
    String(Epos:)=' '
  ELSE
    EString=' '
  ENDIF

  DotPos=INDEX(String,'.')
  TestChar=String(DotPos+SigDigits+1:DotPos+SigDigits+1)
  TPos=INDEX(DigitChar,TestChar)

  IF (SigDigits == 0) THEN
    SPos=DotPos-1
  ELSE
    SPos=DotPos+SigDigits
  ENDIF

  IF (TPos > 5) THEN  ! Must round to next Digit
    Char2Rep=String(SPos:SPos)
    NPos=INDEX(DigitChar,Char2Rep)
    String(SPos:SPos)=DigitChar(NPos+1:NPos+1)
    DO WHILE (NPos == 10)
        ! Must change other char too
      IF (SigDigits == 1) THEN
        TestChar=String(SPos-2:SPos-2)
        IF (TestChar == '.') THEN
          TestChar=String(SPos-3:SPos-3)
          SPos=SPos-2
        ENDIF
        IF (TestChar == ' ') TestChar='0'  ! all 999s
        TPos1=INDEX(DigitChar,TestChar)
        String(SPos-2:SPos-2)=DigitChar(TPos1+1:TPos1+1)
      ELSE
        TestChar=String(SPos-1:SPos-1)
        IF (TestChar == '.') THEN
          TestChar=String(SPos-2:SPos-2)
          SPos=SPos-1
        ENDIF
        IF (TestChar == ' ') TestChar='0'  ! all 999s
        TPos1=INDEX(DigitChar,TestChar)
        String(SPos-1:SPos-1)=DigitChar(TPos1+1:TPos1+1)
      ENDIF
      SPos=SPos-1
      NPos=TPos1
    ENDDO
  ENDIF

  SLen=LEN_TRIM(String)
  IF (SigDigits > 0 .or. EString /= ' ') THEN
    IncludeDot=.true.
  ELSE
    IncludeDot=.false.
  ENDIF
  IF (IncludeDot) THEN
    String=String(1:MIN(DotPos+SigDigits,SLen))//EString
  ELSE
    String=String(1:DotPos-1)
  ENDIF
  IF (IsNAN(RealValue)) THEN
    String='NAN'
  ENDIF
  OutputString=ADJUSTL(String)

  RETURN

END FUNCTION rRoundSigDigits

FUNCTION iRoundSigDigits(IntegerValue,SigDigits) RESULT(OutputString)

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
  INTEGER, INTENT(IN), OPTIONAL :: SigDigits  ! ignored
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

END FUNCTION iRoundSigDigits

FUNCTION dSafeDivide(a, b) RESULT (c)
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!! returns a / b while preventing division by zero
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  REAL(8), INTENT(IN) :: a, b
  REAL(8) :: c

  REAL(8), PARAMETER :: SMALL=1.D-10

  IF (ABS(b) >= SMALL) THEN
    c = a / b
  ELSE
    c = a / SIGN(SMALL, b)
  END IF
END FUNCTION

FUNCTION rSafeDivide(a, b) RESULT (c)
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!! returns a / b while preventing division by zero
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  REAL(4), INTENT(IN) :: a, b
  REAL(4) :: c

  REAL(4), PARAMETER :: SMALL=1.E-10

  IF (ABS(b) >= SMALL) THEN
    c = a / b
  ELSE
    c = a / SIGN(SMALL, b)
  END IF
END FUNCTION

RECURSIVE SUBROUTINE QsortR(Reals)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   March 2009
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! Make sort order for an real Array

          ! METHODOLOGY EMPLOYED:
          ! recursion and quick-sort methodology

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
  USE DataPrecisionGlobals

  IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  REAL(r64), INTENT(INOUT),DIMENSION(:) :: Reals  ! Reals to be sorted

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  integer :: iq

  if(size(Reals) > 1) then
     call QsortPartition(Reals,iq)
     call QsortR(Reals(:iq-1))
     call QsortR(Reals(iq:))
  endif

  RETURN

END SUBROUTINE QsortR

SUBROUTINE QsortPartition(Reals,marker)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   March 2009
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! <description>

          ! METHODOLOGY EMPLOYED:
          ! <description>

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
  USE DataPrecisionGlobals

  IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  REAL(r64), INTENT(INOUT),DIMENSION(:) :: Reals  ! Alphas to be sorted
  INTEGER, INTENT(INOUT)       :: marker

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  integer :: i, j
  real(r64) :: rtemp
  real(r64) :: rpivot      ! pivot point

  rpivot = Reals(1)
  i= 0
  j= size(Reals) + 1

  do
     j = j-1
     do
        if (Reals(j) <= rpivot) exit
        j = j-1
     end do
     i = i+1
     do
        if (Reals(i) >= rpivot) exit
        i = i+1
     end do
     if (i < j) then
        ! exchange iReals(i) and iReals(j)
        rtemp=Reals(i)
        Reals(i)=Reals(j)
        Reals(j)=rtemp
     elseif (i == j) then
        marker = i+1
        return
     else
        marker = i
        return
     endif
  end do

END SUBROUTINE QsortPartition

END MODULE General

MODULE BASE3D
! MODULE INFORMATION:
     !       AUTHOR         Edward Clements
     !       DATE WRITTEN   December 1999
     !       MODIFIED       na
     !       RE-ENGINEERED  na
     !       VERSION NUMBER 2.0
     !
     ! PURPOSE OF THIS MODULE:
     ! This Program calculates the hourly, three dimensional temperature distribution
     ! through a basement foundation and its surrounding earth by an alternating
     ! direction implicit numerical solution method.

     ! METHODOLOGY EMPLOYED:
     ! Standard EnergyPlus module formatting.
     ! Modified f-factor Alternating Direction Implicit Solution method outlined
     ! by Chang, Chow, and Chang, 1991

     !REFERENCES: BASE 3D Version 1.0, Cynthia A. Cogil. 23 July, 1996.
     !
     ! Bahnfleth, W.  1989.  Three-dimensional modelling of heat transfer from slab
     ! floors. Ph.D. diss., University of Illinois at Urbana-Champaign.  Also
     ! published as US Army CERL Technical Manuscript E-89/11.


     ! OTHER NOTES: none
     !USE MSFLIB
     USE DataGlobals, ONLY: MaxNameLength
     USE DataPrecisionGlobals
     USE BasementSimData
     USE InputProcessor
     IMPLICIT NONE


     CONTAINS
!**********************************  SUBROUTINES  ***************************************
!************************************  Driver  ******************************************
SUBROUTINE Base3Ddriver
USE InputProcessor
! SUBROUTINE INFORMATION:
     !       AUTHOR         Edward Clements
     !       DATE WRITTEN   December 6, 1999
     !       MODIFIED       na
     !       RE-ENGINEERED  na
     ! VERSION NUMBER 2.0
     !
     ! PURPOSE OF THIS MODULE:
     ! This subroutine acts as the driver for the basement heat transfer module.

     ! METHODOLOGY EMPLOYED:
     ! Standard EnergyPlus subroutine formatting for drivers

     ! REFERENCES: NONE

     ! OTHER NOTES: none
     IMPLICIT NONE
     CALL SimController
     RETURN
END SUBROUTINE Base3Ddriver
!******************************   Simulation Controller  ********************************
SUBROUTINE SimController
! SUBROUTINE INFORMATION:
     !       AUTHOR         Edward Clements
     !       DATE WRITTEN   December 6, 1999
     !       MODIFIED       August 2000
     !       RE-ENGINEERED  na
     !       VERSION NUMBER 3.0
     !
     ! PURPOSE OF THIS MODULE:
     ! This subroutine contains all looping activity in the basement heat transfer model
     ! and any other code which does not lend itself to containment within a subroutine

     ! METHODOLOGY EMPLOYED:
     ! Standard EnergyPlus subroutine formatting for subroutines

     ! REFERENCES: BASE 3D Version 1.0, Cynthia A. Cogil. 23 July, 1996.

     ! OTHER NOTES: none

     !USE MSFLIB
     USE BasementSimData
     USE InputProcessor
     IMPLICIT NONE

!***  DECLARATIONS:
     INTEGER NUMRUNS         ! Number of different runs to perform in this batch       []
     INTEGER NMAT            ! Number of materials in the simulation domain            []

     LOGICAL CVG             ! Convergence test                                        []
     LOGICAL QUIT            ! End of simulation flag                                  []
     CHARACTER *5 OLDTG           ! Is there an old temperature file?                       []

     CHARACTER *3 RUNID      ! Run identifier for this run                             []
     CHARACTER *6 TGNAM      ! Name of the 1D ground temperature profile file          []

!*** Variables added with the inclusion of the Autogridding function
     INTEGER XDIM            ! Array dimensioning constant for use with surface        []
                             ! temperature and flux calculation variables
     INTEGER YDIM            ! Array dimensioning constant for use with surface        []
                             ! temperature and flux calculation variables
     INTEGER ZDIM            ! Array dimensioning constant for use with surface        []
                             ! temperature and flux calculation variables
     REAL(r64) TG(0:100)          ! 1D Ground temperature profile                           [C]
     OLDTG =BCS%OLDTG
     TGNAM =BCS%TGNAM

     EPObjects = GetNewUnitNumber()
     OPEN (Unit = EPObjects,File = 'EPObjects.TXT')
!*** GET THE NUMBER OF RUNS FOR BATCHING
     CALL ProcessInput('BasementGHT.idd','BasementGHTIn.idf')
!    NUMRUNS= GetNumObjectsFound('Location')
     NUMRUNS = 1
!     PRINT *,'There is/are: ',NUMRUNS,' run/s in this batch'
     DO NUM=1,NUMRUNS
       CVG=.FALSE.
       QUIT=.FALSE.

!*** Retrieve preliminary input
       CALL PrelimInput(RUNID)
!       print *, 'passed prelim input'

!*** Parse EnergyPlus Weather files to create TMY databases for annual simulations
       CALL WeatherServer

!*** Retrieve the rest of the input required by the program
       CALL GetInput(RUNID,TG)

!*** Open IO Files
!      print *, 'going into connectIO'
       CALL ConnectIO(RUNID)

!*** Set up dimensioning constant for use in the calculation of surface temperatures
       XDIM=IBASE+3
       YDIM=JBASE+3
       ZDIM=KBASE+2

!*** Perform the calculations for this simulation
       CALL BasementSimulator(RUNID,NMAT,CVG,XDIM,YDIM,ZDIM,TG)
       CALL CloseIO
     END DO
     RETURN
END SUBROUTINE SimController

!******************************  GET ALL INPUT  *****************************************
SUBROUTINE PrelimInput(RUNID)
USE BasementSimData
IMPLICIT NONE
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   April 28, 2001
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

     !*** PURPOSE OF THIS SUBROUTINE:
     !*** This subroutine will retrieve all input necessary for the weather parsing
     !*** routine to function

     !*** METHODOLOGY EMPLOYED:
     !*** Standard EnergyPlus "manager" methodology.

     !*** REFERENCES:
     !*** na
     CHARACTER *3 RUNID       ! Run Identifier

!*** GET SIMULATION PARAMETERS
     CALL GetSimParams(RUNID)
!     print *, 'passed getsimparams'
     RETURN

END SUBROUTINE PrelimInput

SUBROUTINE GetInput(RUNID,TG)
USE BasementSimData
IMPLICIT NONE
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   May 8-10, 2000
     !***       MODIFIED       June 14, 2000
     !***       RE-ENGINEERED  August 2000

     !*** PURPOSE OF THIS SUBROUTINE:
     !*** This subroutine is the input retrieval driver.
     !*** All retrieval of input information is launched from this subroutine.

     !*** METHODOLOGY EMPLOYED:
     !*** Standard EnergyPlus "manager" methodology.

     !*** REFERENCES:
     !*** na

!*** VARIABLES DECLARATIONS
     CHARACTER *3 RUNID       ! Run Identifier
     CHARACTER *5 OLDTG       ! Is there an old ground temperature file?
  !   CHARACTER *1 EPlusGeom   ! Will you be using the EnergyPlus input file for the
 !                             ! slab geometry? Y/N
     REAL(r64) TG(0:100)           ! 1D Ground temperature profile               [C]


!*** GET LOCATION INFORMATION
!    CALL GetLocation  No longer needed.  Location info taken from EPWeath
!    print *,'passed location'

!*** GET BOUNDARY CONDITIONS
     CALL GetBoundConds
!    print *,'passed getboundconds'

!*** GET MATERIALS PROPERTIES
     CALL GetMatlsProps
!    print *,'passed matls props'

!*** GET INSULATION INFORMATION
     CALL GetInsulationProps
!    print *,'passed insulation'

!*** GET SURFACE PROPERTIES
     CALL GetSurfaceProps
!    print *,'passed surface'

!*** GET BUILDING INFO
     CALL GetBuildingInfo
     !print *,'passed bldg info'

!*** GET INTERIOR BOUNDARY INFORMATION (EXCEPT TEMPERATURE)
     CALL GetInteriorInfo
!    print *,'passed interiors'

!*** GET INDOOR TEMPERATURE
!***  Commercial building
!***  In commercial buildings, the basement temperature will be set to a constant
     IF(.not. SameString(ComBldg,'FALSE'))THEN
       CALL GetComBldgIndoorTemp
     ELSE
!***  Residential building, the basement temperature is calculated using a heat balance
!***  based on the building indoor set point temperatures (heating and cooling season)
!       CALL GetResBldgIndoorTemp
     END IF


!*** IF THIS RUN IS TO BE USED WITH ENERGYPLUS, RETRIEVE THE
!*** GEOMETRY FROM THE MAIN INPUT FILE
 !    IF  (EPlusGeom.EQ.'Y') THEN
 !      CALL GetEPlusGeom
 !    ELSE
!*** GET EQUIVALENT SIZING INFORMATION
       CALL GetEquivSlabInfo
!     END IF

!*** GET AUTOGRIDDING INFORMATION
     IF (.not. SameString(AUTOGRID,'FALSE'))THEN
       IF (.not. SameString(EquivSizing,'FALSE'))THEN

!*** EQUIVALENT SQUARE CALCULATION
         CALL GetEquivAutoGridInfo
!        print *,'passed equiv auto grid'
       ELSE

!*** ACTUAL GEOMETRY CALCULATION
         CALL GetAutoGridInfo
!        print *,'passed auto grid'
       END IF

!*** GET MANUAL GRIDDING INFORMATION
     ELSE
       CALL GetManualGridInfo
!      print *,'passed manual grid'
     END IF

!*** INITIALIZE THE GROUND TEMPERATURE
     OLDTG=BCS%OLDTG
     IF(SameString(OLDTG,'FALSE'))THEN
       CALL InitializeTG(TG)
     END IF
     RETURN
END SUBROUTINE GetInput

!*****************************  SIMULATION PARAMETERS  **********************************
SUBROUTINE GetSimParams(RUNID)
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   August 17, 2000
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor
USE BasementSimData
USE DataGlobals, ONLY: ShowSevereError,ShowContinueError,ShowFatalError
USE General

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(8) :: AlphArray !character string data
     REAL(r64),DIMENSION(3)              :: NumArray  !numeric data
     CHARACTER *3 RUNID

     INTEGER :: EnvVarNumYearsStringLength
     CHARACTER * 4 :: EnvVarNumYearsString
     INTEGER :: EnvVarNumYears
     INTEGER :: EnvVarNumYearsStatus

     ! Set defaults
     SimParams%F=.1d0
     SimParams%IYRS=15

!*** RETRIEVING THE DATA
     CALL GetObjectItem('SimParameters',NUM,AlphArray,NumAlphas,     &
     &    NumArray,NumNums,IOSTAT)

!*** ASSIGNING THE DATA TO SPECIFIC SEGMENTS OF THE
!*** DERIVED TYPES IN BasementSimData WHERE APPLICABLE
 !    AUTOGRID   =TRIM(AlphArray(1))
 !    EPlus      =TRIM(AlphArray(2))
 !    WeatherFile=TRIM(AlphArray(3))
 !    SNOW       =TRIM(AlphArray(4))
 !    RUNID      =TRIM(AlphArray(5))
 !    ComBldg    =TRIM(AlphArray(6))
 !    EPWFile    =TRIM(AlphArray(7))
 !    EPlusGeom  =TRIM(AlphArray(8))
 !    SimParams%TSTEP=NumArray(3)

     AUTOGRID = 'TRUE'
     EPlus = 'TRUE'
     WeatherFile = 'TMYWeath'
     RUNID = 'Run'
     ComBldg = 'TRUE'
     EPWFile = 'in'
     SimParams%F    =NumArray(1)
     IF (SimParams%F <= 0.0 .or. SimParams%F > .3d0) THEN
       CALL ShowSevereError('GetSimParams: "F: Multiplier for the ADI solution" > .3, Set to .1 for this run.')
       SimParams%F=.1d0
     ENDIF
     SimParams%IYRS =NumArray(2)

     ! Override with environment variable for quicker testing
     CALL GET_ENVIRONMENT_VARIABLE("CI_BASEMENT_NUMYEARS", EnvVarNumYearsString, EnvVarNumYearsStringLength, EnvVarNumYearsStatus)
     SELECT CASE (EnvVarNumYearsStatus)
     CASE (-1)
       ! environment variable exists, but too big to fit in the string; ignoring
     CASE (1)
       ! environment variable does not exist, move along
     CASE (2)
       ! no environment variables, what?
     CASE (0)
       ! good, got a nice value, try to read it into the integer
       READ(EnvVarNumYearsString, '(I4)', IOSTAT=EnvVarNumYearsStatus) EnvVarNumYears
       ! if it worked, assign the value, if not just ignore and move on
       IF (EnvVarNumYearsStatus == 0) THEN
         SimParams%IYRS = EnvVarNumYears
       END IF
     END SELECT

     IF (SimParams%IYRS <= 0.d0) THEN
       CALL ShowSevereError('GetSimParams: Entered "IYRS: Maximum number of yearly iterations:" '//  &
          'choice is not valid.'//  &
          ' Entered value=['//trim(RoundSigDigits(SimParams%IYRS,4))//'], 15 will be used.')
       SimParams%IYRS=15
     ENDIF
     SimParams%TSTEP=1.0
     RETURN
END SUBROUTINE GetSimParams

!**********************************  LOCATION INFORMATION  ******************************
SUBROUTINE GetLocation
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   August 17, 2000
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor
USE BasementSimData

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(0) :: AlphArray !character string data
     REAL(r64),DIMENSION(4)              :: NumArray  !numeric data

!*** RETRIEVING THE DATA
     CALL GetObjectItem('Location',NUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)

!*** ASSIGNING THE DATA TO SPECIFIC SEGMENTS OF THE
!*** DERIVED TYPES IN BasementSimData WHERE APPLICABLE
 !    SiteInfo%LONG =NumArray(1)
     SiteInfo%LONG = -110   ! Longitude doesn'tmatter for monthly averages
     SiteInfo%LAT  =NumArray(1)
!     SiteInfo%MSTD =NumArray(3)
     SiteInfo%MSTD = 75  !  Time zone doesn't matter for monthly averages
     SiteInfo%ELEV =NumArray(2)
     RETURN
END SUBROUTINE GetLocation

!********************************  BOUNDARY CONDITIONS  *********************************
SUBROUTINE GetBoundConds
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   August 17, 2000
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor
USE BasementSimData

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(6) :: AlphArray !character string data
     REAL(r64),DIMENSION(0)              :: NumArray  !numeric data


!*** RETRIEVING THE DATA
!    CALL GetObjectItem('BoundConds',NUM,AlphArray,NumAlphas, NumArray,NumNums,IOSTAT)

!*** ASSIGNING THE DATA TO SPECIFIC SEGMENTS OF THE
!*** DERIVED TYPES IN BasementSimData WHERE APPLICABLE
     BCS%OLDTG ='FALSE'
     BCS%TGNAM ='GrTemp'
     BCS%TWRITE='FALSE'
     BCS%TREAD ='FALSE'
     BCS%TINIT ='Tinit'
     BCS%FIXBC ='TRUE'
     RETURN
END SUBROUTINE GetBoundConds

!******************************  MATERIALS PROPERTIES  **********************************
SUBROUTINE GetMatlsProps
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   August 17, 2000
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor
USE BasementSimData

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE

!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(0) :: AlphArray !character string data
     REAL(r64),DIMENSION(19)             :: NumArray  !numeric data

! Set Defaults
     BCS%NMAT=6
     RHO(1)=2243.d0         ! Foundation Wall
     RHO(2)=2243.d0         ! Floor Slab
     RHO(3)=311.d0          ! Ceiling
     RHO(4)=1500.d0         ! Soil
     RHO(5)=2000.d0         ! Gravel
     RHO(6)=449.d0          ! Wood
     RHO(7)=1.25d0          ! Air

     CP(1) =880.d0          ! Foundation Wall
     CP(2) =880.d0          ! Floor Slab
     CP(3) =1530.d0         ! Ceiling
     CP(4) =840.d0          ! Soil
     CP(5) =720.d0          ! Gravel
     CP(6) =1530.d0         ! Wood
     CP(7) =1012.d0         ! Air

     TCON(1) =1.4d0         ! Foundation Wall
     TCON(2) =1.4d0         ! Floor Slab
     TCON(3) =.09d0         ! Ceiling
     TCON(4) =1.1d0         ! Soil
     TCON(5) =1.9d0         ! Gravel
     TCON(6) =.12d0         ! Wood
     TCON(7) =.025d0        ! Air

!*** RETRIEVING THE DATA
     CALL GetObjectItem('MatlProps',NUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
     BCS%NMAT=NumArray(1)
     RHO(1)  =NumArray(2)     ! Foundation Wall
     RHO(2)  =NumArray(3)     ! Floor Slab
     RHO(3)  =NumArray(4)     ! Ceiling
     RHO(4)  =NumArray(5)     ! Soil
     RHO(5)  =NumArray(6)     ! Gravel
     RHO(6)  =NumArray(7)     ! Wood
     RHO(7)  =1.25d0
     CP(1)   =NumArray(8)     ! Foundation Wall
     CP(2)   =NumArray(9)     ! Floor Slab
     CP(3)   =NumArray(10)    ! Ceiling
     CP(4)   =NumArray(11)    ! Soil
     CP(5)   =NumArray(12)    ! Gravel
     CP(6)   =NumArray(13)    ! Wood
     CP(7)   =1012.d0
     TCON(1) =NumArray(14)    ! Foundation Wall
     TCON(2) =NumArray(15)    ! Floor Slab
     TCON(3) =NumArray(16)    ! Ceiling
     TCON(4) =NumArray(17)    ! Soil
     TCON(5) =NumArray(18)    ! Gravel
     TCON(6) =NumArray(19)    ! Wood
     TCON(7) =.025d0          ! Air
     RETURN
END SUBROUTINE GetMatlsProps

!****************************  INSULATION PROPERTIES  ***********************************
SUBROUTINE GetInsulationProps
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   August 17, 2000
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor
USE BasementSimData
USE General
USE DataGlobals, ONLY: ShowSevereError,ShowContinueError,ShowFatalError,ShowWarningError

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(2) :: AlphArray !character string data
     REAL(r64),DIMENSION(5)              :: NumArray  !numeric data


!*** RETRIEVING THE DATA
     CALL GetObjectItem('Insulation',NUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
     Insul%REXT   =NumArray(1)
     IF (Insul%REXT <= 0.0d0) THEN
       CALL ShowSevereError('GetInsulationProps: Entered "REXT: R Value of any exterior insulation" choice is not valid.'//  &
          ' Entered value=['//trim(RoundSigDigits(Insul%REXT,4))//'], .001 will be used.')
       Insul%REXT=.001d0
     ENDIF
     Insul%RINT   = 0.    ! NumArray(2)
     Insul%INSFULL=MakeUPPERCase(AlphArray(1))
     IF (.not. (SameString(AlphArray(1),'TRUE') .or. SameString(AlphArray(1),'FALSE'))) THEN
       CALL ShowWarningError('GetInsulationProps: Entered "INSFULL: Flag: Is the wall fully insulated?" choice is not valid.'//  &
          ' Entered value="'//trim(AlphArray(1))//'", FALSE will be used.')
       Insul%INSFULL='FALSE'
     ENDIF
     Insul%RSID   =   0.   !NumArray(3)
     Insul%RSILL  =   0.   !NumArray(4)
     Insul%RCEIL  =  0.  !NumArray(5)
 !    Insul%RSNOW  =TRIM(AlphArray(2))
      Insul%RSNOW = 'TRUE'
     RETURN
END SUBROUTINE GetInsulationProps

!***********************************  BUILDING INFORMATION  *****************************
SUBROUTINE GetSurfaceProps
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   August 17, 2000
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor
USE BasementSimData
USE DataGlobals, ONLY: ShowWarningError,ShowContinueError

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(1) :: AlphArray !character string data
     REAL(r64),DIMENSION(6)              :: NumArray  !numeric data


! Set defaults
     SP%ALBEDO(1)=.16d0
     SP%ALBEDO(2)=.40d0
     SP%EPSLN(1) =.94d0
     SP%EPSLN(2) =.86d0
     SP%VEGHT(1) =6.0d0
     SP%VEGHT(2) =.25d0
     SP%PET      ='FALSE'

!*** RETRIEVING THE DATA
     CALL GetObjectItem('SurfaceProps',NUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
     SP%ALBEDO(1)=NumArray(1)
     SP%ALBEDO(2)=NumArray(2)
     SP%EPSLN(1) =NumArray(3)
     SP%EPSLN(2) =NumArray(4)
     SP%VEGHT(1) =NumArray(5)
     SP%VEGHT(2) =NumArray(6)
     SP%PET      =MakeUPPERCase(AlphArray(1))
     IF (.not. (SameString(SP%PET,'TRUE') .or. SameString(SP%PET,'FALSE'))) THEN
       CALL ShowWarningError('GetSurfaceProps: "PET: Flag, Potential evapotranspiration on?" choice is not valid'//  &
          ' Entered value="'//trim(AlphArray(1))//'", FALSE will be used.')
       SP%PET='FALSE'
     ENDIF
     RETURN
END SUBROUTINE GetSurfaceProps

!*******************************  INSULATION   ******************************************
SUBROUTINE GetBuildingInfo
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   May 10, 2000
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor
USE BasementSimData
USE General
USE DataGlobals, ONLY: ShowSevereError,ShowContinueError,ShowFatalError

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(0) :: AlphArray !character string data
     REAL(r64),DIMENSION(5)              :: NumArray  !numeric data

     ! Setup defaults
     BuildingData%DWALL=.2d0
     BuildingData%DSLAB=.1d0
     BuildingData%DGRAVXY=.3d0
     BuildingData%DGRAVZN=.2d0
     BuildingData%DGRAVZP=.1d0

!*** RETRIEVING THE DATA
     CALL GetObjectItem('BldgData',NUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
     BuildingData%DWALL  =NumArray(1)
     IF (BuildingData%DWALL < 0.2d0) THEN
       CALL ShowSevereError('GetInsulationProps: Entered "DWALL: Wall thickness" choice is not valid.'//  &
          ' Entered value=['//trim(RoundSigDigits(BuildingData%DWALL,4))//'], .2 will be used.')
       BuildingData%DWALL=.2d0
     ENDIF
     BuildingData%DSLAB  =NumArray(2)
     IF (BuildingData%DSLAB <= 0.d0 .or. BuildingData%DSLAB > .25d0) THEN
       CALL ShowSevereError('GetInsulationProps: Entered "DSLAB: Floor slab thickness" choice is not valid.'//  &
          ' Entered value=['//trim(RoundSigDigits(BuildingData%DSLAB,4))//'], .1 will be used.')
       BuildingData%DSLAB=.1d0
     ENDIF
     BuildingData%DGRAVXY=NumArray(3)
     IF (BuildingData%DGRAVXY <= 0.d0) THEN
       CALL ShowSevereError('GetInsulationProps: Entered "DGRAVXY: Width of gravel pit beside basement wall" '//  &
          'choice is not valid.'//  &
          ' Entered value=['//trim(RoundSigDigits(BuildingData%DGRAVXY,4))//'], .3 will be used.')
       BuildingData%DGRAVXY=.3d0
     ENDIF
     BuildingData%DGRAVZN=NumArray(4)
     IF (BuildingData%DGRAVZN <= 0.d0) THEN
       CALL ShowSevereError('GetInsulationProps: Entered "DGRAVZN: Gravel depth extending above the floor slab" '//  &
          'choice is not valid.'//  &
          ' Entered value=['//trim(RoundSigDigits(BuildingData%DGRAVZN,4))//'], .2 will be used.')
       BuildingData%DGRAVZN=.2d0
     ENDIF
     BuildingData%DGRAVZP=NumArray(5)
     IF (BuildingData%DGRAVZP <= 0.d0) THEN
       CALL ShowSevereError('GetInsulationProps: Entered "DGRAVZP: Gravel depth below the floor slab" '//  &
          'choice is not valid.'//  &
          ' Entered value=['//trim(RoundSigDigits(BuildingData%DGRAVZP,4))//'], .1 will be used.')
       BuildingData%DGRAVZP=.1d0
     ENDIF

     RETURN
END SUBROUTINE GetBuildingInfo

!*******************************  INTERIOR INFO *****************************************
SUBROUTINE GetInteriorInfo
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   August 18, 2000
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor
USE BasementSimData
USE General
USE DataGlobals, ONLY: ShowSevereError,ShowContinueError,ShowFatalError,ShowWarningError

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(1) :: AlphArray !character string data
     REAL(r64),DIMENSION(6)              :: NumArray  !numeric data

!    Set defaults
     Interior%HIN(1)=.92d0
     Interior%HIN(2)=4.04d0
     Interior%HIN(3)=3.08d0
     Interior%HIN(4)=6.13d0
     Interior%HIN(5)=9.26d0
     Interior%HIN(6)=8.29d0

!*** RETRIEVING THE DATA
     CALL GetObjectItem('Interior',NUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
     Interior%COND=AlphArray(1)
     IF (.not. (SameString(Interior%COND,'TRUE') .or. SameString(Interior%COND,'FALSE'))) THEN
       CALL ShowWarningError('GetInteriorInfo: "COND: Flag: Is the basement conditioned?" choice is not valid'//  &
          ' Entered value="'//trim(AlphArray(1))//'", TRUE will be used.')
       Interior%COND='TRUE'
     ENDIF
     Interior%HIN(1)=NumArray(1)
     IF (Interior%HIN(1) <= 0.d0) THEN
       CALL ShowSevereError('GetInteriorInfo: Entered "HIN: Downward convection only heat transfer coefficient" '//  &
          'choice is not valid.'//  &
          ' Entered value=['//trim(RoundSigDigits(Interior%HIN(1),4))//'], .92 will be used.')
       Interior%HIN(1)=.92d0
     ENDIF
     Interior%HIN(2)=NumArray(2)
     IF (Interior%HIN(2) <= 0.d0) THEN
       CALL ShowSevereError('GetInteriorInfo: Entered "HIN: Upward convection only heat transfer coefficient" '//  &
          'choice is not valid.'//  &
          ' Entered value=['//trim(RoundSigDigits(Interior%HIN(2),4))//'], 4.04 will be used.')
       Interior%HIN(1)=4.04d0
     ENDIF
     Interior%HIN(3)=NumArray(3)
     IF (Interior%HIN(3) <= 0.d0) THEN
       CALL ShowSevereError('GetInteriorInfo: Entered "HIN: Horizontal convection only heat transfer coefficient" '//  &
          'choice is not valid.'//  &
          ' Entered value=['//trim(RoundSigDigits(Interior%HIN(3),4))//'], 3.08 will be used.')
       Interior%HIN(3)=3.08d0
     ENDIF
     Interior%HIN(4)=NumArray(4)
     IF (Interior%HIN(4) <= 0.d0) THEN
       CALL ShowSevereError('GetInteriorInfo: Entered '//  &
          '"HIN: Downward combined (convection and radiation) heat transfer coefficient" '//  &
          'choice is not valid.'//  &
          ' Entered value=['//trim(RoundSigDigits(Interior%HIN(4),4))//'], 6.13 will be used.')
       Interior%HIN(4)=6.13d0
     ENDIF
     Interior%HIN(5)=NumArray(5)
     IF (Interior%HIN(5) <= 0.d0) THEN
       CALL ShowSevereError('GetInteriorInfo: Entered '//  &
          '"HIN: Upward combined (convection and radiation) heat transfer coefficient" '//  &
          'choice is not valid.'//  &
          ' Entered value=['//trim(RoundSigDigits(Interior%HIN(5),4))//'], 9.26 will be used.')
       Interior%HIN(5)=9.26d0
     ENDIF
     Interior%HIN(6)=NumArray(6)
     IF (Interior%HIN(6) <= 0.d0) THEN
       CALL ShowSevereError('GetInteriorInfo: Entered '//  &
          '"HIN: Horizontal combined (convection and radiation) heat transfer coefficient" '//  &
          'choice is not valid.'//  &
          ' Entered value=['//trim(RoundSigDigits(Interior%HIN(6),4))//'], 8.29 will be used.')
       Interior%HIN(6)=8.29d0
     ENDIF
     RETURN
END SUBROUTINE GetInteriorInfo

!*******************  Commercial Building Indoor Temp  **********************************
SUBROUTINE GetComBldgIndoorTemp
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   August 23, 2000
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor
USE BasementSimData
USE General
USE DataGlobals, ONLY: ShowSevereError,ShowContinueError,ShowFatalError,ShowWarningError

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(0) :: AlphArray !character string data
     REAL(r64),DIMENSION(13)              :: NumArray  !numeric data

!*** RETRIEVING THE DATA
     CALL GetObjectItem('ComBldg',NUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
     IF(NumNums <12) Then
          TBasementAve=NumArray(1)  ! Set all values to single input
          TBasementDailyAmp =0.
          CALL ShowWarningError('GetComBldgIndoorTemp: Not all monthly average temperature entered. '//  &
               'Average temperature is set to ['//trim(RoundSigDigits(Interior%HIN(6),4))//'].')
     ELSE
          TBasementAve=NumArray(1:12) ! Monthly basement temp
          TBasementDailyAmp = NumArray(13)
     END IF
     RETURN
END SUBROUTINE GetComBldgIndoorTemp

!*******************  Residential Building Indoor Temp  *********************************
SUBROUTINE GetResBldgIndoorTemp
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   August 23, 2000
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor
USE BasementSimData

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(0) :: AlphArray !character string data
     REAL(r64),DIMENSION(4)              :: NumArray  !numeric data

!*** RETRIEVING THE DATA
     CALL GetObjectItem('ResBldg',NUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
     Interior%TIN(1)=NumArray(1)
     Interior%TIN(2)=NumArray(2)
     TDeadBandUp    =NumArray(3)
     TDeadBandLow   =NumArray(4)
     RETURN
END SUBROUTINE GetResBldgIndoorTemp

!*************  CALCULATION OF EQUIVALENT RECTANGLE OR SQUARE  **************************
SUBROUTINE GetEquivSlabInfo
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   August 16,2000
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor
USE BasementSimData
USE General
USE DataGlobals, ONLY: ShowSevereError,ShowContinueError,ShowFatalError,ShowWarningError

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(1) :: AlphArray !character string data
     REAL(r64),DIMENSION(1)              :: NumArray  !numeric data
     LOGICAL :: errFound

!*** LOCAL VARIABLE ASSIGNMENTS
     REAL(r64) L                ! Square slab dimension from A/P ratio

!*** RETRIEVING THE DATA
     errFound=.false.
     NumNums=GetNumObjectsFound('EquivSlab')
     IF (NumNums <= 0) RETURN
     CALL GetObjectItem('EquivSlab',NUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
     APRatio    =NumArray(1)
     IF (APRatio < 0.0d0) Then
        CALL ShowSevereError('GetEquivSlabInfo: APRatio =['//TRIM(RoundSigDigits(APRatio,3))//  &
           '] less than zero.')
        errFound=.true.
     END IF

     EquivSizing=MakeUPPERCase(AlphArray(1))
     IF (.not. (SameString(AlphArray(1),'TRUE') .or. SameString(AlphArray(1),'FALSE'))) THEN
       CALL ShowWarningError('GetEquivSlabInfo: Entered "EquivSizing: Flag" choice is not valid.'//  &
          ' Entered value="'//trim(AlphArray(1))//'", TRUE will be used.')
       EquivSizing='TRUE'
     ENDIF

     L=4.d0*APRatio
     IF(L.LT.6.0d0)THEN
       SLABX=6.0d0
       SLABY=(2.d0*APRatio*SLABX)/(1.d0-(2.d0*APRatio))
     ELSE
       SLABX=L
       SLABY=L
     END IF

     IF (errFound) THEN
       CALL ShowFatalError('GetEquivSlabInfo: program terminates due to previous condition.')
     ENDIF

     RETURN
END SUBROUTINE GetEquivSlabInfo

!************************** EQUIVALENT AUTOMATED GRID INFORMATION  **********************
SUBROUTINE GetEquivAutoGridInfo
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   August 16,2000
     !***       MODIFIED
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor !, ONLY: GetObjectItem
USE BasementSimData

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas    ! Number of elements in the alpha array
     INTEGER  :: NumNums      ! Number of elements in the numeric array
     INTEGER  :: IOStat       ! IO Status when calling get input subroutine

     CHARACTER(len=MaxNameLength),DIMENSION(0) :: AlphArray !character string data
     REAL(r64),DIMENSION(4)              :: NumArray  !numeric data

!*** RETRIEVING THE DATA
     NumNums=GetNumObjectsFound('EquivAutoGrid')

700  Format(1X,/,' You have selected to use an equivalent foundation based on the',/,  &
      ' area to perimeter ratio supplied in the IDF file.'/)

!     PRINT 700
!     PRINT *,''
!     PRINT *,'YOU HAVE SELECTED TO USE AN EQUIVALENT FOUNDATION BASED ON THE '
!     PRINT *,'AREA TO PERIMETER RATIO SUPPLIED IN THE IDF FILE'
!     PRINT *,''
     IF (NumNums > 0) THEN
       WRITE(DebugOutFile,700)

       CALL GetObjectItem('EquivAutoGrid',NUM,AlphArray,  &
       &    NumAlphas,NumArray,NumNums,IOSTAT)
       CLEARANCE=NumArray(1)
       ConcAGHeight=0.0   !This is hard wired to zero for EneergyPlus runs
       SlabDepth=NumArray(2)
       BaseDepth=NumArray(3)
       CALL AutoGridding
     ENDIF
     RETURN
END SUBROUTINE GetEquivAutoGridInfo

!**************************  AUTOMATED GRID INFORMATION  ********************************
SUBROUTINE GetAutoGridInfo
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   August 17, 2000
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor
USE BasementSimData

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas    ! Number of elements in the alpha array
     INTEGER  :: NumNums      ! Number of elements in the numeric array
     INTEGER  :: IOStat       ! IO Status when calling get input subroutine

     CHARACTER(len=MaxNameLength),DIMENSION(0) :: AlphArray !character string data
     REAL(r64),DIMENSION(6)              :: NumArray  !numeric data

!*** RETRIEVING THE DATA
700  Format(1X,/,' You have selected to use the actual building geometry.',/,  &
      ' If your building is not a square or rectangle, please reselect',/,     &
      ' to use an equivalent square foundation based on the area to',/,        &
      ' perimeter ratio of the actual geometry.',/)
     NumNums=GetNumObjectsFound('AutoGrid')
     IF (NumNums > 0) THEN
       PRINT 700
!     PRINT *,''
!     PRINT *,'YOU HAVE SELECTED TO USE THE ACTUAL BUILDING GEOMETRY.'
!     PRINT *,'IF YOUR BUILDING IS NOT A SQUARE OR RECTANGLE, PLEASE RESELECT '
!     PRINT *,'TO USE AN EQUIVALENT SQUARE FOUNDATION BASED ON THE AREA TO'
!     PRINT *,'PERIMETER RATIO OF THE ACTUAL GEOMETRY.'
!     PRINT *,''
       WRITE(DebugOutFile,700)

       CALL GetObjectItem('AutoGrid',NUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
       CLEARANCE   =NumArray(1)
       SLABX       =NumArray(2)
       SLABY       =NumArray(3)
       ConcAGHeight=NumArray(4)
       SlabDepth   =NumArray(5)
       BaseDepth   =NumArray(6)

       CALL AutoGridding
     ENDIF
     RETURN
END SUBROUTINE GetAutoGridInfo

!*************************  MANUAL GRID INFORMATION  ************************************
SUBROUTINE GetManualGridInfo
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   May 10, 2000
     !***       MODIFIED       June 14, 2000
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor !, ONLY: GetObjectItem
USE BasementSimData

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(0) :: AlphArray !character string data
     REAL(r64),DIMENSION(7)              :: NumArray  !numeric data

!*** RETRIEVING THE DATA
     NumNums=GetNumObjectsFound('ManualGrid')
     IF (NumNums > 0) THEN
       CALL GetObjectItem('ManualGrid',NUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
       NX   =NumArray(1)
       NY   =NumArray(2)
       NZAG =NumArray(3)
       NZBG =NumArray(4)
       IBASE=NumArray(5)
       JBASE=NumArray(6)
       KBASE=NumArray(7)
       CALL GetXFACEData
       CALL GetYFACEData
       CALL GetZFACEData
     ENDIF
     RETURN
END SUBROUTINE GetManualGridInfo

!***********************************  XFACE VALUES  *************************************
SUBROUTINE GetXFACEData
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   May 10, 2000
     !***       MODIFIED       June 14, 2000
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor
USE BasementSimData

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
!*** LOCAL VARIABLE DECLARATIONS
!     INTEGER  :: NX
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(1) :: AlphArray !character string data
     REAL(r64),DIMENSION(0:NX)           :: NumArray  !numeric data

!*** RETRIEVING THE DATA
     CALL GetObjectItem('XFACE',NUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
     XFACE=NumArray
     RETURN
END SUBROUTINE GetXFACEData

!***********************************  YFACE VALUES  *************************************
SUBROUTINE GetYFACEData
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   May 10, 2000
     !***       MODIFIED       June 14, 2000
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor
USE BasementSimData

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(1) :: AlphArray !character string data
     REAL(r64),DIMENSION(0:NY)           :: NumArray  !numeric data


!*** RETRIEVING THE DATA
     CALL GetObjectItem('YFACE',NUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
     YFACE=NumArray
     RETURN
END SUBROUTINE GetYFACEData

!***********************************  ZFACE VALUES  *************************************
SUBROUTINE GetZFACEData
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   May 10, 2000
     !***       MODIFIED       June 14, 2000
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor
USE BasementSimData

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(1) :: AlphArray !character string data
     REAL(r64),DIMENSION(-NZAG:NZBG)     :: NumArray  !numeric data

!*** RETRIEVING THE DATA
     CALL GetObjectItem('ZFACE',NUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
     ZFACE=NumArray
     RETURN
END SUBROUTINE GetZFACEData

!************************  ENERGY PLUS GEOMETRY RETRIEVAL  ******************************
SUBROUTINE GetEPlusGeom !(EPlus)
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   March 16, 2001
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor
USE BasementSimData

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(16) :: AlphArray !character string data
     REAL(r64),DIMENSION(31)              :: NumArray  !numeric data
     CHARACTER *7 SurfType ! Surface type of a given surface
     REAL(r64) FloorArea        ! Floor area
     REAL(r64) Perimeter        ! Floor perimeter
     REAL(r64) X1,X2,X3,X4,Y1,Y2,Y3,Y4,DIMX1,DIMX2,DIMY1,DIMY2

!*** LOCAL VARIABLE
     REAL(r64) L
!     CHARACTER *5 EPlus
!*** RETRIEVING THE DATA
     CALL GetObjectItem('SURFACE',NUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)

!*** ASSIGNING THE DATA TO SPECIFIC SEGMENTS OF THE
!*** DERIVED TYPES IN SimData WHERE APPLICABLE
     SurfType  =AlphArray(2)
     FloorArea =NumArray(1)
     IF (SurfType.EQ.'FLOOR')THEN
       X1        =NumArray(7)
       X2        =NumArray(10)
       X3        =NumArray(13)
       X4        =NumArray(16)
       Y1        =NumArray(8)
       Y2        =NumArray(11)
       Y3        =NumArray(14)
       Y4        =NumArray(17)
       DIMX1     =MAX(X2,X1)-MIN(X2,X1)
       DIMX2     =MAX(X3,X4)-MIN(X3,X4)
       DIMY1     =MAX(Y2,Y1)-MIN(Y2,Y1)
       DIMY2     =MAX(Y4,Y3)-MIN(Y4,Y3)
       Perimeter =DIMX1+DIMX2+DIMY1+DIMY2
       APRatio   =FloorArea/Perimeter
     END IF

!*** RETRIEVING THE DATA
     IF (.not. SameString(EPlus,'FALSE').AND.APRatio.NE.0.)THEN
       L=4.d0*APRatio
       IF(L.LT.6.0d0)THEN
         SLABX=6.0d0
         SLABY=(2.d0*APRatio*SLABX)/(1.d0-(2.d0*APRatio))
       ELSE
         SLABX=L
         SLABY=L
       END IF
     ELSE
       NumNums=GetNumObjectsFound('EquivSlab')
       IF (NumNums > 0) THEN
         CALL GetObjectItem('EquivSlab',NUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
         APRatio    =NumArray(1)
         EquivSizing=AlphArray(1)
         L=4.d0*APRatio
         IF(L.LT.6.0d0)THEN
           SLABX=6.0d0
           SLABY=(2.d0*APRatio*SLABX)/(1.d0-(2.d0*APRatio))
         ELSE
           SLABX=L
           SLABY=L
         END IF
       ELSE
         APRatio=0.0
         EquivSizing='FALSE'
         L=0.0d0
         SLABX=0.0d0
         SLABY=0.0d0
       ENDIF
     END IF
     RETURN
END SUBROUTINE GetEPlusGeom
!****************************************************************************************
!*******************************  END ENERGY PLUS INPUT STAGE  **************************
!****************************************************************************************

!******************************  BASEMENT SIMULATOR  ************************************
SUBROUTINE BasementSimulator(RUNID,NMAT,CVG,XDIM,YDIM,ZDIM,TG)
! SUBROUTINE INFORMATION:
     !       AUTHOR         Edward Clements
     !       DATE WRITTEN   December 8, 1999
     !       MODIFIED       na
     !       RE-ENGINEERED  na
     !      VERSION NUMBER 2.0
     !
     ! PURPOSE OF THIS MODULE:
     ! This subroutine will perform the calculations required for the simulation of
     ! three dimensional basement heat transfer.

     ! METHODOLOGY EMPLOYED:
     ! Standard EnergyPlus subroutine formatting for subroutines

     ! REFERENCES: BASE 3D Version 1.0, Cynthia A. Cogil. 23 July, 1996.

     ! OTHER NOTES: none

USE BasementSimData
USE InputProcessor
USE General
USE DataGlobals, ONLY: ShowSevereError,ShowContinueError,ShowFatalError,ShowWarningError
IMPLICIT NONE

!***  Variable Declarations
!***  Character variables
  CHARACTER *3 RUNID      ! Run Identifier for this set                            []
  CHARACTER *6 TGNAM      ! Name of the ground temperature file                    []

!***  Integer variables
  INTEGER CONDITION       ! Describes whether the basement is being heated (1),    []
                          ! cooled (-1), is unconditioned or in the dead band (0)
                          ! During house cooling, basement may be cooled, too, if
                          ! TBAV>TIN(2) (-1) or left to float if TBAV<TIN(2) (-2)

  INTEGER DSNOW(24)       ! Snow depth                                           [cm]
  INTEGER EFFECT          ! Describes whether the heat loss/gain between the       []
                          ! house and the basement is beneficial (+1),adverse(-1),
                          ! or neutral (0)
                          ! basement interior dimensions of quadrant modeled
  !INTEGER IDAY            ! Day counter variable                                   []
  INTEGER IHR             ! Hour counter variable                                  []
  INTEGER IMON            ! Month counter variable                                 []
  INTEGER ISNW            ! Snow indicator 2/1 for snow/no snow                    []
  INTEGER IYR             ! Year counter variable                                  []
  INTEGER IYRS            ! Maximum number of years to iterate solution            []
  INTEGER L               ! Counter variable                                       []
  INTEGER MTYPE(0:100,0:100,-35:100)     ! Array of material types                 []
                          ! 1) Foundation wall,  2) Floor Slab
                          ! 3) Ceiling, 4) Soil, 5) Gravel,
                          ! 6) Wood, 7) Air
  INTEGER N               ! Number of cell faces in the positive Z direction       []
  INTEGER NMAT            ! Number of material types                               []
  INTEGER NXM1            ! NX-1
  INTEGER NYM1            ! NY-1
  INTEGER NZBGM1          ! NZBG-1
  INTEGER XDIM            ! Array dimensioning constant for use with surface       []
                          ! temperature and flux calculation variables
  INTEGER YDIM            ! Array dimensioning constant for use with surface       []
                          ! temperature and flux calculation variables
  INTEGER ZDIM            ! Array dimensioning constant for use with surface       []
                          ! temperature and flux calculation variables
!*Real variables
  REAL(r64) A(0:100,0:100,-35:100) ! Coefficients of tridiagonal matrix           [      ]
  REAL(r64) AA(100)            ! Tridiagonal matrix coefficient set                     []
  REAL(r64) ACEIL              ! Total surface area of ceiling                       [m^2]
  REAL(r64) AFLOOR             ! Total surface area of the floor slab                [m^2]
  REAL(r64) ALB                ! Snow presence modified surface albedo value            []
  REAL(r64) ALBEDO(2)          ! Surface albedo array                                   []
  REAL(r64) ARIM               ! Total surface area of rim joist                     [m^2]
  REAL(r64) ASILL              ! Total surface area of sill box                      [m^2]
  REAL(r64) ATOT               ! Total surface area of the domain                    [m^2]
  REAL(r64) AWALL              ! Total surface area of foundation wall               [m^2]
  REAL(r64) B(0:100,0:100,-35:100) ! Coefficients of tridiagonal matrix             [    ]
  REAL(r64) BB(100)            ! Tridiagonal matrix coefficient set                     []
  REAL(r64) C(0:100,0:100,-35:100) ! Coefficients of tridiagonal matrix             [    ]
  REAL(r64) CC(100)            ! Tridiagonal matrix coefficient set                     []
  REAL(r64) CLOAD              ! Cooling Load                                          [W]
  REAL(r64) CONST(0:100,0:100,-35:100)  ! Array of constants in tridiagonal array   [    ]
  REAL(r64) COUNTER            ! Dummy counter for time step do loop                    []
  REAL(r64) CXM(0:100,0:100,-35:100)    ! Coefficient arrays referring to cell faces in []
                                        ! the positive coordinate direction indicated
                                        ! from the center cell node
  REAL(r64) CXP(0:100,0:100,-35:100)    ! Coefficient arrays referring to cell faces in []
                                        ! the positive coordinate direction indicated
                                        ! from the center cell node
  REAL(r64) CYM(0:100,0:100,-35:100)    ! Coefficient arrays referring to cell faces in []
                                        ! the positive coordinate direction indicated
                                        ! from the center cell node
  REAL(r64) CYP(0:100,0:100,-35:100)    ! Coefficient arrays referring to cell faces in []
                                        ! the positive coordinate direction indicated
                                        ! from the center cell node
  REAL(r64) CZM(0:100,0:100,-35:100)    ! Coefficient arrays referring to cell faces in []
                                        ! the positive coordinate direction indicated
                                        ! from thecenter cell node
  REAL(r64) CZP(0:100,0:100,-35:100)    ! Coefficient arrays referring to cell faces in []
                                        ! the positive coordinate direction indicated
                                        ! from the center cell node
  REAL(r64) DGRAVZN    ! Thickness of gravel in negative Z direction from the top of the floor slab          [m]
  REAL(r64) DGRAVXY    ! Thickness of gravel in positive X asnd Y directions from the wall                   [m]
  REAL(r64) DGRAVZP    ! Thickness of gravel in positive Z direction from beneath the slab                   [m]
  REAL(r64) DODPG      ! Delta over Delta plus Gamma                                                         []
  REAL(r64) DSLAB      ! Thickness of floor slab                                                             [m]
  REAL(r64) DQCA       ! Daily time/space averaged surface heat flux (Ceil)                                  [W/m^2]
  REAL(r64) DQCSUM     ! Daily net surface heat losses (Ceiling)                                             [W-h]
  REAL(r64) DQFA       ! Daily time/space averaged surface heat flux (Floor)                                 [W/m^2]
  REAL(r64) DQFSUM     ! Daily net surface heat losses (Floor)                                               [W-h]
  REAL(r64) DQRA       ! Daily time/space averaged surface heat flux (Rim)                                   [W/m^2]
  REAL(r64) DQRSUM     ! Daily net surface heat losses (Rim)                                                 [W-h]

  REAL(r64) DQSA       ! Daily time/space averaged surface heat flux (Sill)                                  [W/m^2]
  REAL(r64) DQSSUM     ! Daily net surface heat losses (Sill)                                                [W-h]
  REAL(r64) DQWA       ! Daily time/space averaged surface heat flux (Wall)                                  [W/m^2]
  REAL(r64) DQWSUM     ! Daily net surface heat losses (Wall)                                                [W-h]
  REAL(r64) DTBA       ! Daily average basement temperature                                                  [C]
  REAL(r64) DTCA       ! Daily time/space averaged surface temperature (Ceil)                                [C]
  REAL(r64) DTDBA      ! Daily average outdoor dry bulb temperature                                          [C]
  REAL(r64) DTFA       ! Daily time/space averaged surface temperature (Floor)                               [C]
  REAL(r64) DTRA       ! Daily time/space averaged surface temperature (Rim)                                 [C]
  REAL(r64) DTSA       ! Daily time/space averaged surface temperature (Sill)                                [C]
  REAL(r64) DTWA       ! Daily time/space averaged surface temperature (Wall)                                [C]
  REAL(r64) DWALL      ! Foundation wall thickness                                                           [m]
  REAL(r64) DX(0:100)  ! Array of cell dimensions                                                            [m]
  REAL(r64) DXP(0:100) ! Array of disatnces between cells in the X direction                                 [m]
  REAL(r64) DY(0:100)  ! Array of cell dimensions                                                            [m]
  REAL(r64) DYP(0:100) ! Array of distances between cells in the Y direction                                 [m]
  REAL(r64) DZ(-35:100) ! Array of cell dimensions                                                           [m]
  REAL(r64) DZP(-35:100) ! Array of distances between cells in the Z direction                               [m]
  REAL(r64) EPS        ! Snow presence modified surface emissivity value                                     []
  REAL(r64) EPSLN(2)   ! Surface emissivity                                                                  []
  REAL(r64) F          ! F-factor for modified ADI method                                                    []
  REAL(r64) GINIT      ! Initial surface heat flux                                                           [W/m^2]
  REAL(r64) GOFT(0:100,0:100,2) ! Surface heat fluxes                                                        [W/m^2]
                           ! GOFT(COUNT1,COUNT2,1)=Current hour value
                           ! GOFT(COUNT1,COUNT2,2)=Previous hour value
  REAL(r64) GOFTAV(0:100,0: 100) ! Average surface heat flux for one time step                               [W/m^2]
  REAL(r64) GOFTSUM(0:100,0:100) ! Sum of surface heat fluxes for one time step                              [W/m^2]
  REAL(r64) HHEAT              ! Convective heat transfer coefficient                                        [J/m^2-K-s]
  REAL(r64) HIN(6)             ! Inside heat transfer coefficients                                           [W/m^2/K]
                          ! Convection Only:
                          ! 1)Q Downward 2)Q Upward 3)Q Horizontal
                          ! Convection and Radiation
                          ! 4)Q Downward 5)Q Upward 6)Q Horizontal
  REAL(r64) HINZ      ! Inside heat transfer coefficient for heat transfer in the vertical direction         [W/m^2/K]
  REAL(r64) HINZH     ! Inside heat transfer coefficient for heat transfer in the horizontal direction       [W/m^2/K]
  REAL(r64) HLOAD     ! Heating Load                                                                         [W]
  REAL(r64) HMASS     ! Convective mass transfer coefficient                                                 [J/m^2-K-s]
  REAL(r64) HO        ! Outside convective heat transfer coefficient                                         [K/(W/m^2)]
  REAL(r64) HOAV      ! Average outside heat transfer coefficient                                            [K/(W/m^2)]
  REAL(r64) HOSUM     ! Total outside convective heat transfer coefficient for one time step                 [K/(W/m^2)]
  REAL(r64) HRAT(24)  ! Hourly humidity ratio data                                                           [kg/kg]
  REAL(r64) IEXT      ! Coordinates of basement ext. measured from origin                                    [m]
  REAL(r64) IINT      ! Coordinates of basement interior measured from origin                                [m]
  REAL(r64) JEXT      ! Coordinates of basement ext. measured from origin                                    [m]
  REAL(r64) JINT      ! Coordinates of basement interior measured from origin                                [m]
  REAL(r64) KEXT      ! Coordinates of basement ext. measured from origin                                    [m]
  REAL(r64) KINT      ! Coordinates of basement interior measured from origin                                [m]
  REAL(r64) PBAR(24)
  REAL(r64) PERIM     ! Perimeter of floor slab                                                              [m]
  REAL(r64) QCMN      ! Local minimum cell surface heat flux for one time step                               [W/m^2]
  REAL(r64) QCMX      ! Local maximum cell surface heat flux for one time step. (Ceiling)                    [W/m^2]
  REAL(r64) QCSUM     ! Sum of cell surface heat losses for one time step (ceil)                             [W]
  REAL(r64) QFMN      ! Local minimum cell surface heat flux for one time step. (Floor)                      [W/m^2]
  REAL(r64) QFMX      ! Local maximum cell surface heat flux for one time step. (Floor)                      [W/m^2]
  REAL(r64) QFSUM     ! Sum of cell surface heat losses for one time step (floor)                            [W]
  REAL(r64) QHOUSE    ! House conditioning flux for one time step                                            [W]
  REAL(r64) QPET      ! Latent heat transfer by potential evapotranspiration                                 [W/m^2]
  REAL(r64) QRMN      ! Local minimum cell surface heat flux for one time step. (Rim Joist)                  [W/m^2]
  REAL(r64) QRMX      ! Local maximum cell surface heat flux for one time step. (Rim Joist)                  [W/m^2]
  REAL(r64) QRSUM     ! Sum of cell surface heat losses for one time step (rim)                              [W]
  REAL(r64) QSMN      ! Local minimum cell surface heat flux for one time step. (Sill Plate)                 [W/m^2]
  REAL(r64) QSSUM     ! Sum of cell surface heat losses for one time step (sill)                             [W]
  REAL(r64) QSMX      ! Local maximum cell surface heat flux for one time step. (Sill plate)                 [W/m^2]
  REAL(r64) QWMN      ! Local minimum cell surface heat flux for one time step. (Wall)                       [W/m^2]
  REAL(r64) QWSUM     ! Sum of cell surface heat losses for one time step (wall)                             [W]
  REAL(r64) QWMX      ! Local maximum cell surface heat flux for one time step. (Wall)                       [W/m^2]
  REAL(r64) R(0:100,0:100,-35:100)    ! Coefficients of tridiagonal matrix           [   ]
  REAL(r64) RCEIL     ! Thermal resistance of the ceiling                                                    [K/(W/m^2)]
  REAL(r64) RDIFH(24) ! Diffuse solar radiation on a horizontal surface                                      [W/m^2]
  REAL(r64) RDIFHO    ! Previous hour's diffuse solar radiation on a horizontal surface                      [W/m^2]
  REAL(r64) RDIRH(24) ! Direct (global) solar radiation on a horizontal surface                              [W/m^2]
  REAL(r64) RBEAM(24) ! Direct beam solar radiation on a horizontal(?) surface                               [W/m^2]
  REAL(r64) RDIRHO    ! Direct (global) solar radiation on a horizontal surface during the preceeding hour   [W/m^2]
  REAL(r64) REXT      ! R value of the exterior foundation wall insulation                                   [K/(W-m^2)]
  REAL(r64) RGRND     ! Ground surface infrared radiation (grey body radiation)                              [W/m^2]
  REAL(r64) RINT      ! Thermal resistance of interior foundation wall insulation                            [K/(W/m^2)]
  REAL(r64) RR(100)   ! Tridiagonal matrix solution set                                                      []
  REAL(r64) RSID      ! Thermal resistance of the siding material                                            [K/(W/m^2)]
  REAL(r64) RSILL     ! Thermal resistance of sill box insulation                                            [K/(W/m^2)]
  REAL(r64) RSKY      ! Incoming infrared sky radiation                                                      [W/m^2]
  REAL(r64) RSNW      ! Thermal resistance of ground snow cover                                              [K/(W/m^2)]
  REAL(r64) RSNWSUM   ! Sum of of thermal resistances of snow cover for one time step                        [K/(W/m^2)]
  REAL(r64) RSNWAV    ! Average thermal resistance of snow cover                                             [K/(W/m^2)]
  REAL(r64) RSOLH     ! Total absorbed solar radiation on a horizontal surface                               [W/m^2]
  REAL(r64) RSOLV(24) ! Total absorbed solar radiation on a vertical surface                                 [W/m^2]
  REAL(r64) RSOLVAV   ! Average absorbed solar radiation on a vertical surface for one time step             [W/m^2]
  REAL(r64) RSOLVSUM  ! Sum of absorbed solar radiation on a vertical surface for one time step              [W/m^2]
  REAL(r64) RTOT      ! Effective incoming radiation at earth's surface                                      [W/m^2]
  REAL(r64) T(-1:100,-1:100,-35:100)! 3D Temperature field                                                   [C]
  REAL(r64) TB(24)    ! Basement temperature                                                                 [C]
  REAL(r64) TBAV      ! Average basement temperature for one time step                                       [C]
  REAL(r64) TBSUM     ! Sum of basement temperatures for one time step                                       [C]
  REAL(r64) TCMX      ! Local maximum cell surface temperature for one time step. (Ceiling)                  [C]
  REAL(r64) TCMN      ! Local minimum cell surface temperature for one time step. (Ceiling)                  [C]
  REAL(r64) TCSUM     ! Sum of cell surface temperatures for one time step (ceil)                            [C-m^2]
  REAL(r64) TCVG(-1:100,-1:100,-35:100)   ! 3D Temperature field for convergence                             [C]
  REAL(r64) TDB(24)   ! Ambient dry bulb temperature                                                         [C]
  REAL(r64) TDBC      ! Ambient temperature above which the conditioning system is in cooling mode           [C]
  REAL(r64) TDBAV     ! Average outdoor dry bulb temperature                                                 [C]
  REAL(r64) TDBH      ! Ambient temperature below which the conditioning system is in heating mode           [C]
  REAL(r64) TDBSUM    ! Sum of outdoor dry bulb temperatures for one time step                               [C]
  REAL(r64) TDEEP     ! Deep ground temperature if fixed temp lower b.c.                                     [C]
  REAL(r64) TFMN      ! Local minimum cell surface temperature for one time step. (Floor)                    [C]
  REAL(r64) TFMX      ! Local maximum cell surface temperature for one time step. (Floor)                    [C]
  REAL(r64) TFSUM     ! Sum of cell surface temperatures for one time step (flr)                             [C-m^2]
  REAL(r64) TG(0:100)    ! One Dimensional Ground temperature profile                                        [C]
  REAL(r64) TGAV(0:100)  ! Average ground temperature for one time step                                      [C]
  REAL(r64) TGSUM(0:100) ! Sum of ground temperatures for one time step                                      [C]
  REAL(r64) TI        ! Inside (house) dry bulb temperature for one time step                                [C]
  REAL(r64) TIAV      ! Average indoor temperature for one time step                                         [C]
  REAL(r64) TISUM     ! Sum of inside dry bulb temperatures for one time step                                [C]
  REAL(r64) TIN(2)    ! Indoor thermostat set point temperature                                              [C]
  REAL(r64) TRMN      ! Local minimum cell surface temperature for one time step. (Rim Joist)                [C]
  REAL(r64) TRMX      ! Local maximum cell surface temperature for one time step. (Rim Joist)                [C]
  REAL(r64) TRSUM     ! Sum of cell surface temperatures for one time step (rim)                             [C-m^2]
  REAL(r64) TSMN      ! Local minimum cell surface temperature for one time step. (Sill Plate)               [C]
  REAL(r64) TSMX      ! Local maximum cell surface temperature for one time step. (Sill plate)               [C]
  REAL(r64) TSSUM     ! Sum of cell surface temperatures for one time step (sill)                            [C-m^2]
  REAL(r64) TSTEP     ! Simulation time step                                                                 [hrs]
     ! Following are Daily average cell temperatures:
  REAL(r64) TV1(-1:100,-35:100) ! in the X-Z plane on the 21st of each month at COUNT2=0                     [C]
  REAL(r64) TV2(-1:100,-35:100) ! in the X-Z plane on the 21st of each month at COUNT2=JBASE-1               [C]
  REAL(r64) TV3(-1:100,-35:100) ! in the X-Z plane on the 21st of each month at COUNT2=JBASE                 [C]
  REAL(r64) TWB(24)   ! Ambient wet bulb temperature                                                         [C]
  REAL(r64) TWMN      ! Local minimum cell surface temperature for one time step. (Wall)                     [C]
  REAL(r64) TWMX      ! Local maximum cell surface temperature for one time step. (Wall)                     [C]
  REAL(r64) TWSUM     ! Sum of cell surface temperatures for one time step (wall)                            [C-m^2]
  REAL(r64) U(-1:100,-1:100,-35:100)    ! Intermediate temperature array                                     [C]
  REAL(r64) V(-1:100,-1:100,-35:100)    ! Intermediate temperature array                                     [C]
  REAL(r64) VHT       ! Snow presence modified vegetation height                                             [cm]
  REAL(r64) VEGHT(2)  ! Vegetation height (surface roughness parameter)                                      [cm]
  REAL(r64) WND(24)   ! Wind speed at 10m                                                                    [m/s]
  REAL(r64) WNDL      ! Wind speed close to a leeward wall                                                   [m/s]
  REAL(r64) WNDW      ! Wind speed close to a windward wall                                                  [m/s]
  REAL(r64) X(110)     ! Coefficients of tridiagonal matrix                                                  [   ]
  REAL(r64) XC(-1:100) ! Array of cell center coordinates                                                    [m]
  REAL(r64) YC(-1:100) ! Array of cell center coordinates                                                    [m]
  REAL(r64) YCLOAD     ! Yearly Cooling Load from the basement                                               [W]
  REAL(r64) YHLOAD     ! Yearly Heating Load from the basement                                               [W]
  REAL(r64) YQBSUM     ! Yearly total basement heat flux                                                     [W-h]
  REAL(r64) YQCSUM     ! Yearly total ceiling heat flux                                                      [W-h]
  REAL(r64) YQFSUM     ! Yearly total basement floor heat flux                                               [W-h]
  REAL(r64) YQRSUM     ! Yearly total rim joist heat flux                                                    [W-h]
  REAL(r64) YQSSUM     ! Yearly total sill heat flux                                                         [W-h]
  REAL(r64) YQWSUM     ! Yearly total basement wall heat flux                                                [W-h]
  REAL(r64) YTDB(8760)  ! One year of hourly dry bulb temperature data                                       [C]
  REAL(r64) ZC(-35:100) ! Array of cell center coordinates                                                   [m]

    ! Following are Daily average cell surface heat flux for the 21st day of each month
  REAL(r64), ALLOCATABLE, DIMENSION(:,:) :: DQC21  ! (Ceiling)                                                [W/m^2]
  REAL(r64), ALLOCATABLE, DIMENSION(:,:) :: DQF21  ! (Floor)                                                  [W/m^2]
  REAL(r64), ALLOCATABLE, DIMENSION(:)   :: DQRS21 ! (South rim joist)                                        [W/m^2]
  REAL(r64), ALLOCATABLE, DIMENSION(:)   :: DQRW21 ! (West rim joist)                                         [W/m^2]
  REAL(r64), ALLOCATABLE, DIMENSION(:,:) :: DQSS21 ! (South sill plate)                                       [W/m^2]
  REAL(r64), ALLOCATABLE, DIMENSION(:,:) :: DQSW21 ! (West sill plate)                                        [W/m^2]
  REAL(r64), ALLOCATABLE, DIMENSION(:,:) :: DQWS21 ! (South wall)                                             [W/m^2]
  REAL(r64), ALLOCATABLE, DIMENSION(:,:) :: DQWW21 ! (West wall)                                              [W/m^2]
    ! Following are Daily average cell surface temperature for the 21st day of each month
  REAL(r64), ALLOCATABLE, DIMENSION(:,:) :: DTC21  ! (Ceiling)                                                [C]
  REAL(r64), ALLOCATABLE, DIMENSION(:,:) :: DTF21  ! (floor)                                                  [C]
  REAL(r64), ALLOCATABLE, DIMENSION(:)   :: DTRS21 ! (south rim joist)                                        [C]
  REAL(r64), ALLOCATABLE, DIMENSION(:)   :: DTRW21 ! (west rim joist)                                         [C]
  REAL(r64), ALLOCATABLE, DIMENSION(:,:) :: DTSS21 ! (South Sill)                                             [C]
  REAL(r64), ALLOCATABLE, DIMENSION(:,:) :: DTSW21 ! (West Sill)                                              [C]
  REAL(r64), ALLOCATABLE, DIMENSION(:,:) :: DTWW21 ! (West Wall)                                              [C]
  REAL(r64), ALLOCATABLE, DIMENSION(:,:) :: DTWS21 ! (South Wall)                                             [C]
    ! Following are Cell surface heat flux for one time step
  REAL(r64), ALLOCATABLE, DIMENSION(:,:) :: QC     ! Ceiling                                                  [W/m^2]
  REAL(r64), ALLOCATABLE, DIMENSION(:,:) :: QF     ! Floor                                                    [W/m^2]
  REAL(r64), ALLOCATABLE, DIMENSION(:)   :: QRS    ! South Rim Joist                                          [W/m^2]
  REAL(r64), ALLOCATABLE, DIMENSION(:)   :: QRW    ! West Rim Joist                                           [W/m^2]
  REAL(r64), ALLOCATABLE, DIMENSION(:,:) :: QSS    ! South Sill                                               [W/m^2]
  REAL(r64), ALLOCATABLE, DIMENSION(:,:) :: QSW    ! West Sill                                                [W/m^2]
  REAL(r64), ALLOCATABLE, DIMENSION(:,:) :: QWS    ! South Wall                                               [W/m^2]
  REAL(r64), ALLOCATABLE, DIMENSION(:,:) :: QWW    ! West Wall                                                [W/m^2]
                              !
  REAL(r64), ALLOCATABLE, DIMENSION(:,:,:) :: QEXT ! Above grade exterior surf heat flux                      [W/m^2]
  REAL(r64), ALLOCATABLE, DIMENSION(:,:,:) :: TEXT ! Above grade exterior surface temperature                 [C]

    ! Following are Cell surface temperature for one time step
  REAL(r64), ALLOCATABLE, DIMENSION(:,:) :: TC     ! (ceiling)                                                [C]
  REAL(r64), ALLOCATABLE, DIMENSION(:,:) :: TF     ! (floor)                                                  [C]
  REAL(r64), ALLOCATABLE, DIMENSION(:)   :: TRS    ! (rim south)                                              [C]
  REAL(r64), ALLOCATABLE, DIMENSION(:)   :: TRW    ! (rim west)                                               [C]
  REAL(r64), ALLOCATABLE, DIMENSION(:,:) :: TSS    ! (sill south)                                             [C]
  REAL(r64), ALLOCATABLE, DIMENSION(:,:) :: TSW    ! (sill west)                                              [C]
  REAL(r64), ALLOCATABLE, DIMENSION(:,:) :: TWS    ! (wall south)                                             [C]
  REAL(r64), ALLOCATABLE, DIMENSION(:,:) :: TWW    ! (wall west)                                              [C]

  REAL(r64), ALLOCATABLE, DIMENSION(:,:,:) :: UEXT ! Intermediate exterior surface cell temp                  [C]
  REAL(r64), ALLOCATABLE, DIMENSION(:,:,:) :: VEXT ! Intermediate temperature array                           [C]

!*Logical variables

  LOGICAL CVG             ! True indicates 3D convergence - flags termination      []
  LOGICAL CVG1D           ! Convergence test for 1D ground temperature model       []
  LOGICAL QUIT            ! True indicates termination of execution                []


!*Modified logical variables
  CHARACTER *5 COND       ! True if basement is mechanically conditioned           []
  CHARACTER *5 FIXBC      ! True if a fixed temperature lower b.c. is applied      []
  CHARACTER *5 INSFULL    ! True if the foundation wall insulated full height      []
  CHARACTER *5 OLDTG      ! True if there is an old ground temperature file        []
                          ! If not, one will be created using a 1D model
  CHARACTER *5 PET        ! True if potential evapotranspiration model is on       []
  CHARACTER *5 TREAD      ! True indicates that intialized temperatures            []
                          ! will be read from a file
  CHARACTER *5 RSNOW      ! True if snow cover model is on                         []
  CHARACTER *5 TWRITE     ! True indicates that initialization temperatures        []
                          ! will be written to a file

  REAL(r64) LONG          ! Site longitude                                      [deg]
  REAL(r64) LAT           ! Site latitude                                       [deg]
  REAL(r64) MSTD          ! Site standard meridian                              [deg]
  REAL(r64) ELEV          ! Site elevation                                        [m]
  INTEGER AHH             ! Annual heating hours (based on user defined set point) []
  INTEGER ACH             ! Annual cooling hours (based on user defined set point) []
  INTEGER Twriting        ! Unit number for Tinit file

!*Variables added with autogridding
  REAL(r64) ZCINIT(-35:100)     ! Array of cell center coordinates for initialization  [m]
  REAL(r64) DZINIT(-35:100)     ! Array of cell dimensions for initialization          [m]
  REAL(r64) DZPINIT(-35:100)    ! Array of distances between cell centers              [m]

!*Variables added for EnergyPlus surface temperature calculation
  INTEGER INS(-1:100,0:100,-35:100)     ! Integer indication of insulation          []
                                       ! 1= cell insulated 0= not insulated
  REAL(r64) TSurfWallYZ        ! Area weighted avg wall surface temperature array      [C]
  REAL(r64) TSurfWallXZ        ! Area weighted avg wall surface temperature array      [C]
  REAL(r64) TSurfFloor         ! Area weighted avg floor surface temperature           [C]
  REAL(r64) TSFloorIn          ! Area weighted avg slab temperature (for comparison)   [C]
  REAL(r64) TSWallXZIn         ! Area weighted avg in wall temperature (comparison)    [C]
  REAL(r64) TSWallYZIn         ! Area weighted avg in wall temperature (comparison)    [C]
  REAL(r64) TSXZCL             ! XZ plane centerline wall temperature                  [C]
  REAL(r64) TSYZCL             ! YZ plane centerline wall temperature                  [C]
  REAL(r64) TSFXCL             ! X direction floor centerline temperature              [C]
  REAL(r64) TSFYCL             ! Y direction floor centerline temperature              [C]
  REAL(r64) DAYZUpperSum       ! Upper band YZ wall area                            [m**2]
  REAL(r64) DAXZUpperSum       ! Upper band XZ wall area                            [m**2]
  REAL(r64) DAYZLowerSum       ! Lower band YZ wall area                            [m**2]
  REAL(r64) DAXZLowerSum       ! Lower band XZ wall area                            [m**2]
  REAL(r64) DAPerim            ! Perimeter zone floor area                          [m**2]
  REAL(r64) DACore             ! Core zone floor area                               [m**2]
  REAL(r64) DAXYSum            ! Total floor area                                   [m**2]
  REAL(r64) DAYZSum            ! Below grade YZ wall area                           [m**2]
  REAL(r64) DAXZSum            ! Below grade XZ wall area                           [m**2]
  REAL(r64) TSurfWallYZUpper   ! Upper band YZ wall temperature                        [C]
  REAL(r64) TSurfWallYZUpperIn ! Upper band YZ inside wall temperature                 [C]
  REAL(r64) TSurfWallYZLower   ! Lower band YZ wall tepmerature                        [C]
  REAL(r64) TSurfWallYZLowerIn ! Lower band YZ inside wall temperature                 [C]
  REAL(r64) TSurfWallXZLower   ! Lower band XZ wall temperature                        [C]
  REAL(r64) TSurfWallXZLowerIn ! Lower band XZ inside wall temperature                 [C]
  REAL(r64) TSurfWallXZUpper   ! Upper band XZ wall temperature                        [C]
  REAL(r64) TSurfWallXZUpperIn ! Upper band XZ wall inside temperature                 [C]
  REAL(r64) TSurfFloorPerim    ! Perimeter area floor surface temperature              [C]
  REAL(r64) TSurfFloorPerimIn  ! Perimeter area floor inside surface temperature       [C]
  REAL(r64) TSurfFloorCore     ! Core area floor surface temperature                   [C]
  REAL(r64) TSurfFloorCoreIn   ! Core area floor inside surface temperature            [C]
  REAL(r64) FloorHeatFlux      ! Area weighted average floor heat flux            [W/m**2]
  REAL(r64) CoreHeatFlux       ! Area weighted average core zone floor q"         [W/m**2]
  REAL(r64) PerimHeatFlux      ! Area weighted average floor perimeter q"         [W/m**2]
  REAL(r64) XZWallHeatFlux     ! Area weighted average XZ wall q"                 [W/m**2]
  REAL(r64) YZWallHeatFlux     ! Area weighted average YZ wall q"                 [W/m**2]
  REAL(r64) UpperXZWallFlux    ! Area weighted avg upper band XZ wall q"          [W/m**2]
  REAL(r64) UpperYZWallFlux    ! Area weighted avg upper band YZ wall q"          [W/m**2]
  REAL(r64) LowerXZWallFlux    ! Area weighted avg lower band XZ wall q"          [W/m**2]
  REAL(r64) LowerYZWallFlux    ! Area weighted avg lower band YZ wall q"          [W/m**2]

!*Variables added with the addition of a local sorting function
  INTEGER INDEX           ! Placeholder variable for the sorting function          []
  REAL(r64) FIRST              ! First value in the sorted dry bulb temp array         [C]
  REAL(r64) TEMP               ! Temporary placeholder for the sorting function         []

  REAL(r64) CVGDeltaT
  REAL(r64) Time_Start
  REAL(r64) Time_Finish
  REAL(r64) Elapsed_Time
  INTEGER IHrStart
  INTEGER IHrEnd
  INTEGER CI_BAIL_EARLY_STATUS


      CALL CPU_TIME(Time_Start)
      QUIT=.false.
!***  Setting Derived Type variable values
      LONG   =SiteInfo%LONG
      LAT    =SiteInfo%LAT
      MSTD   =SiteInfo%MSTD
      ELEV   =SiteInfo%ELEV
      AHH    =SiteInfo%AHH
      ACH    =SiteInfo%ACH

      IYRS   =SimParams%IYRS
      TSTEP  =SimParams%TSTEP
      F      =SimParams%F

      TIN    =Interior%TIN
      HIN    =Interior%HIN
      COND   =Interior%COND

      PET    =SP%PET
      VEGHT  =SP%VEGHT
      EPSLN  =SP%EPSLN
      ALBEDO =SP%ALBEDO

      TREAD  =BCS%TREAD
      TWRITE =BCS%TWRITE
      NMAT   =BCS%NMAT
      OLDTG  =BCS%OLDTG
      TGNAM  =BCS%TGNAM
      FIXBC  =BCS%FIXBC

      DWALL  =BuildingData%DWALL
      DSLAB  =BuildingData%DSLAB
      DGRAVZN=BuildingData%DGRAVZN
      DGRAVXY=BuildingData%DGRAVXY
      DGRAVZP=BuildingData%DGRAVZP

      INSFULL=Insul%INSFULL
      REXT   =Insul%REXT
      RSNOW  =Insul%RSNOW
      RCEIL  =Insul%RCEIL
      RSID   =Insul%RSID
      RSILL  =Insul%RSILL
      RINT   =Insul%RINT

!***  COMMONLY USED VARIABLES
      NXM1=NX-1
      NYM1=NY-1
      NZBGM1=NZBG-1

! **** Allocate arrays;  Using this to avoid stack overflows for certain X,Y,Z dimensions
      ALLOCATE(DQC21(0:XDIM,0:YDIM))
      ALLOCATE(DQF21(0:XDIM,0:YDIM))
      ALLOCATE(DQRS21(0:YDIM))
      ALLOCATE(DQRW21(0:XDIM))
      ALLOCATE(DQSS21(0:XDIM,0:YDIM))
      ALLOCATE(DQSW21(0:XDIM,0:YDIM))
      ALLOCATE(DQWS21(0:YDIM,-35:ZDIM))
      ALLOCATE(DQWW21(0:XDIM,-35:ZDIM))
      ALLOCATE(DTC21(0:XDIM,0:YDIM))
      ALLOCATE(DTF21(0:XDIM,0:YDIM))
      ALLOCATE(DTRS21(0:YDIM))
      ALLOCATE(DTRW21(0:XDIM))
      ALLOCATE(DTSS21(0:XDIM,0:YDIM))
      ALLOCATE(DTSW21(0:XDIM,0:YDIM))
      ALLOCATE(DTWW21(0:XDIM,-35:ZDIM))
      ALLOCATE(DTWS21(0:YDIM,-35:ZDIM))
      ALLOCATE(QC(0:XDIM,0:YDIM))
      ALLOCATE(QF(0:XDIM,0:YDIM))
      ALLOCATE(QEXT(0:XDIM,0:YDIM,-35:-1))
      ALLOCATE(QRS(0:YDIM))
      ALLOCATE(QRW(0:XDIM))
      ALLOCATE(QSS(0:XDIM,0:YDIM))
      ALLOCATE(QSW(0:XDIM,0:YDIM))
      ALLOCATE(QWS(0:YDIM,-35:ZDIM))
      ALLOCATE(QWW(0:XDIM,-35:ZDIM))
      ALLOCATE(TC(0:XDIM,0:YDIM))
      ALLOCATE(TEXT(0:XDIM,0:YDIM,-35:-1))
      ALLOCATE(TF(0:XDIM,0:YDIM))
      ALLOCATE(TRS(0:YDIM))
      ALLOCATE(TRW(0:XDIM))
      ALLOCATE(TSS(0:XDIM,0:YDIM))
      ALLOCATE(TSW(0:XDIM,0:YDIM))
      ALLOCATE(TWS(0:YDIM,-35:ZDIM))
      ALLOCATE(TWW(0:XDIM,-35:ZDIM))
      ALLOCATE(UEXT(-1:XDIM,-1:YDIM,-35:-1))
      ALLOCATE(VEXT(-1:XDIM,-1:YDIM,-35:-1))

!***  Compute some cell geometry factors
!***  X-Direction
      DO COUNT1=0,NXM1
        XC(COUNT1)=(XFACE(COUNT1)+XFACE(COUNT1+1))/2.d0
        DX(COUNT1)=XFACE(COUNT1+1)-XFACE(COUNT1)
      END DO
      XC(IBASE)=XFACE(IBASE)
      XC(IBASE+2)=XFACE(IBASE+3)
      DO COUNT1=0,NX-2
        DXP(COUNT1)=XC(COUNT1+1)-XC(COUNT1)
      END DO
!***  Y-Direction
      DO COUNT1=0,NYM1
        YC(COUNT1)=(YFACE(COUNT1)+YFACE(COUNT1+1))/2.d0
        DY(COUNT1)=YFACE(COUNT1+1)-YFACE(COUNT1)
      END DO
      YC(JBASE)=YFACE(JBASE)
      YC(JBASE+2)=YFACE(JBASE+3)
      DO COUNT1=0,NY-2
        DYP(COUNT1)=YC(COUNT1+1)-YC(COUNT1)
      END DO
!***  Z-Direction
      IF (.not. SameString(AUTOGRID,'FALSE')) THEN
        DO COUNT1=-NZAG,NZBGM1
          ZCINIT(COUNT1)=(ZFACEINIT(COUNT1)+ZFACEINIT(COUNT1+1))/2.d0
          DZINIT(COUNT1)=ZFACEINIT(COUNT1+1)-ZFACEINIT(COUNT1)
        END DO
        ZCINIT(0)=0.
        ZCINIT(KBASE)=ZFACEINIT(KBASE)
        DO COUNT1=-NZAG,NZBG-2
          DZPINIT(COUNT1)=ZCINIT(COUNT1+1)-ZCINIT(COUNT1)
        END DO

!*** Calculation of the minimum cell sizes for stability
        CALL CalcDZmin(DX,DY,DZINIT)
        DO COUNT1=-NZAG,NZBGM1
          ZC(COUNT1)=(ZFACE(COUNT1)+ZFACE(COUNT1+1))/2.d0
          DZ(COUNT1)=ZFACE(COUNT1+1)-ZFACE(COUNT1)
        END DO
        ZC(0)=0.
        ZC(KBASE)=ZFACE(KBASE)
        DO COUNT1=-NZAG,NZBG-2
          DZP(COUNT1)=ZC(COUNT1+1)-ZC(COUNT1)
        END DO
      ELSE
        DO COUNT1=-NZAG,NZBGM1
          ZC(COUNT1)=(ZFACE(COUNT1)+ZFACE(COUNT1+1))/2.d0
          DZ(COUNT1)=ZFACE(COUNT1+1)-ZFACE(COUNT1)
        END DO
        ZC(0)=0.
        ZC(KBASE)=ZFACE(KBASE)
        DO COUNT1=-NZAG,NZBG-2
          DZP(COUNT1)=ZC(COUNT1+1)-ZC(COUNT1)
        END DO
      END IF

      VEXT=0.0
!      write(*,*) '2115=',VEXT(12,15,-1)
!***  Assign material properties according to material type
      AFLOOR=0.
      ATOT=(XFACE(NX)*YFACE(NY))
      IEXT=XFACE(IBASE)+DWALL
      JEXT=YFACE(JBASE)+DWALL
      KEXT=ZFACE(KBASE)+DSLAB
      DO COUNT1=0,NXM1
        DO COUNT2=0,NYM1
          DO COUNT3=-NZAG,NZBGM1
!***  MTYPE(4) (Soil)
            IF ((XC(COUNT1).GT.IEXT.OR.YC(COUNT2).GT.JEXT).AND.(ZC(COUNT3).GE.0.AND. &
              &  ZC(COUNT3).LT.(ZFACE(KBASE)-DGRAVZN))) THEN
              MTYPE(COUNT1,COUNT2,COUNT3)=4
!***  MTYPE(4) (Soil)
            ELSE IF ((XC(COUNT1).GT.(IEXT+DGRAVXY).OR.YC(COUNT2).GT. &
              & (JEXT+DGRAVXY)).AND.(ZC(COUNT3).GE.(ZFACE(KBASE)-DGRAVZN).AND. &
              & ZC(COUNT3).LT.(KEXT+DGRAVZP))) THEN
              MTYPE(COUNT1,COUNT2,COUNT3)=4
!***  MTYPE(4) (Soil)
            ELSE IF (ZC(COUNT3).GE.(KEXT+DGRAVZP)) THEN
              MTYPE(COUNT1,COUNT2,COUNT3)=4
!***  MTYPE(1) (Foundation Wall)
            ELSE IF (((XC(COUNT1).GE.XFACE(IBASE).AND.XC(COUNT1).LE.IEXT.AND. &
              & YC(COUNT2).LE.YFACE(JBASE)).OR.(YC(COUNT2).GE.YFACE(JBASE).AND. &
              & YC(COUNT2).LE.JEXT.AND.XC(COUNT1).LE.IEXT)).AND.ZC(COUNT3).LE.KEXT.AND. &
              & ZC(COUNT3).GE.ZC(-NZAG+3)) THEN
              MTYPE(COUNT1,COUNT2,COUNT3)=1
!***  MTYPE(2) (Floor Slab)
            ELSE IF (XC(COUNT1).LT.XFACE(IBASE).AND.YC(COUNT2).LT.YFACE(JBASE).AND. &
              & ZC(COUNT3).EQ.ZFACE(KBASE)) THEN
              MTYPE(COUNT1,COUNT2,COUNT3)=2
!***  MTYPE(3) (Ceiling)
            ELSE IF (XC(COUNT1).LE.IEXT.AND.YC(COUNT2).LE.JEXT.AND.ZC(COUNT3).EQ.    &
              & ZC(-NZAG)) THEN
              MTYPE(COUNT1,COUNT2,COUNT3)=3
!***  MTYPE(5) (Gravel)
            ELSE IF (((XC(COUNT1).LT.(IEXT+DGRAVXY).AND.XC(COUNT1).GT.IEXT.AND. &
              & YC(COUNT2).LT.JEXT+DGRAVXY).OR.(YC(COUNT2).LT.(JEXT+DGRAVXY).AND. &
              & YC(COUNT2).GT.JEXT.AND.XC(COUNT1).LE.IEXT)).AND.(ZC(COUNT3).LT.KEXT.AND. &
              & ZC(COUNT3).GT.(ZFACE(KBASE)-DGRAVZN))) THEN
              MTYPE(COUNT1,COUNT2,COUNT3)=5
!***  MTYPE(5) (Gravel)
            ELSE IF ((XC(COUNT1).LT.(IEXT+DGRAVXY).AND.YC(COUNT2).LT. &
              & (JEXT+DGRAVXY)).AND.(ZC(COUNT3).EQ.ZC(KBASE+1))) THEN
              MTYPE(COUNT1,COUNT2,COUNT3)=5
!***  MTYPE(6) (Wood)
            ELSE IF (((XC(COUNT1).EQ.IEXT.AND.YC(COUNT2).LE.JEXT).OR. &
              & (YC(COUNT2).EQ.JEXT.AND.XC(COUNT1).LE.IEXT)).AND. &
              & ZC(COUNT3).EQ.ZC(-NZAG+1)) THEN
              MTYPE(COUNT1,COUNT2,COUNT3)=6
!***  MTYPE(6) (Wood)
            ELSE IF (((XC(COUNT1).GE.XFACE(IBASE).AND.XC(COUNT1).LE.IEXT.AND. &
              & YC(COUNT2).LE.JEXT).OR.(YC(COUNT2).GE.YFACE(JBASE).AND. &
              & YC(COUNT2).LE.JEXT.AND.XC(COUNT1).LE.XFACE(IBASE))).AND. &
              & ZC(COUNT3).EQ.ZC(-NZAG+2)) THEN
              MTYPE(COUNT1,COUNT2,COUNT3)=6
!***  MTYPE(7) (Air)
            ELSE
              MTYPE(COUNT1,COUNT2,COUNT3)=7
            END IF
          END DO
        END DO
      END DO
!PRINT *,WeatherFile
!*** Set Density and Specific heat values
      DO COUNT1=0,NXM1
        DO COUNT2=0,NYM1
          DO COUNT3=-NZAG+1,NZBGM1
            IF (MTYPE(COUNT1,COUNT2,COUNT3).EQ.7) THEN
              RHO(MTYPE(COUNT1,COUNT2,COUNT3))=880.0
              CP(MTYPE(COUNT1,COUNT2,COUNT3))=1.402
            END IF
          END DO
        END DO
      END DO
!PRINT *,WeatherFile
!!      REWIND (Weather)
!***  Calculate the area of the floor slab
      AFLOOR=4.d0*XFACE(IBASE)*YFACE(JBASE)
!***  Calculate the area of the ceiling
      ACEIL=4.d0*XFACE(IBASE+2)*YFACE(JBASE+2)
!***  Calculate the area of the rim joist
      ARIM=4.d0*DZ(-NZAG+1)*(XFACE(IBASE+2)+YFACE(JBASE+2))
!***  Calculate the surface areas of the foundation wall and sill box
      AWALL=4.d0*(ZFACE(KBASE)-ZFACE(-NZAG+2))*(XFACE(IBASE)+YFACE(JBASE))
      ASILL=4.d0*((XFACE(IBASE+2)-XFACE(IBASE))*YFACE(JBASE)+ &
      & (YFACE(JBASE+2)-YFACE(JBASE))*XFACE(IBASE+2))
!***  Calculate the building perimeter
      PERIM=4.d0*(IEXT+JEXT)

      IF (AFloor < 1.d-8) THEN
        CALL ShowFatalError('BasementSimulator: Improper slab entered.  Describe slab using EquivalentSlab object or similar.')
      ENDIF

      HOSUM=0.0
!      write(*,*) '2207=',VEXT(12,15,-1)

!***  Pre-calculate some finite difference constants
      CALL FDMCoefficients(NXM1,NYM1,NZBGM1,INSFULL,REXT,DX,DY,DZ,DXP,DYP,  &
      &    DZP,MTYPE,CXM,CYM,CZM,CXP,CYP,CZP,ZC,INS)
!***  Write the headers for the EnergyPlus Output file
      CALL EPlusHeader

!***  Open the blast weather file and skip the header
!!      REWIND (Weather)
!      Write (99,*) Weather
!      READ (Weather,2300)
! 2300 FORMAT (/)
!      OPEN (UNIT=68,FILE='weather.txt')

!***  Find the conditioning system dead band boundary temperatures
      IF(SameString(ComBldg,'FALSE')) THEN
        IHrStart=1
        IHrEnd=24
        DO IDAY=1,365
!!          READ (Weather,*) TodaysWeather
          TodaysWeather%TDB=FullYearWeather(IDAY)%TDB
          TodaysWeather%TWB=FullYearWeather(IDAY)%TWB
          TodaysWeather%PBAR=FullYearWeather(IDAY)%PBAR
          TodaysWeather%HRAT=FullYearWeather(IDAY)%HRAT
          TodaysWeather%WND=FullYearWeather(IDAY)%WND
          TodaysWeather%RBEAM=FullYearWeather(IDAY)%RBEAM
          TodaysWeather%RDIFH=FullYearWeather(IDAY)%RDIFH
          TodaysWeather%ISNW=FullYearWeather(IDAY)%ISNW
          TodaysWeather%DSNOW=FullYearWeather(IDAY)%DSNOW
          TDB=TodaysWeather%TDB
!200       FORMAT(/,3(8F10.6,/),19/)
!          WRITE (68,*) IDAY
!          WRITE (YTDBFile,*) (TDB(IHR), IHR=1,24)
          YTDB(IHrStart:IHrEnd)=TDB
          IHrStart=IHrStart+24
          IHrEnd=IHrEnd+24
        END DO
!        REWIND(Weather)
!        WRITE(YTDBFile,*) YTDB
!        REWIND(YTDBFile)
!        READ (YTDBFile,*) YTDB
        !CALL SORTQQ (LOC(YTDB), 8760, SRT$REAL4)
! Sort YTDB values.  Ascending.
        CALL QSortR(YTDB)
        TDBH=YTDB(AHH)
        TDBC=YTDB(8760-ACH)
!        DO COUNT1=1,8759
!          FIRST=YTDB(COUNT1)
!          INDEX=COUNT1
!          DO COUNT2=COUNT1+1,8760
!            IF(YTDB(COUNT2).LT.FIRST)THEN
!              FIRST=YTDB(COUNT2)
!              INDEX=COUNT2
!            END IF
!          END DO
!          IF(INDEX.NE.COUNT1)THEN
!            TEMP=YTDB(COUNT1)
!            YTDB(COUNT1)=YTDB(INDEX)
!            YTDB(INDEX)=TEMP
!          END IF
!        END DO
!        WRITE (YTDBFile,*) (YTDB(COUNT1), COUNT1=1,8760)
!        TDBH=YTDB(AHH)
!        TDBC=YTDB(8760-ACH)
!        PRINT *,'PASSED SETTING DEAD BAND'
      ELSE
!        PRINT *,'THIS IS A COMMERCIAL BULDING. THERE IS NO BASEMENT TEMP DEAD BAND'
         Write(DebugOutFile,*) 'This is a commercial bulding. There is no basement temp dead band.'
      END IF

!***  If there is an old boundary condition file for this case, connect it and close
!***  the input file. Otherwise, read the initial ground temperatures and create a new
!***  file with the 1-D model (SUBROUTINE CalcTearth)
      TGNAM=BCS%TGNAM
      CALL CalcTearth(IEXT,JEXT,DZ,DZP,TG,CVG1D)
      IF (.NOT.CVG1D) STOP

!***  Echo input data
      CALL PrelimOutput(ACEIL,AFLOOR,ARIM,ASILL,AWALL,PERIM,RUNID,TDBH,TDBC)

      CALL GET_ENVIRONMENT_VARIABLE("CI_BAIL_EARLY", status=CI_BAIL_EARLY_STATUS)
      IF (CI_BAIL_EARLY_STATUS == 0) THEN
        print *, 'Exiting early because envionment variable CI_BAIL_EARLY was found'
        CALL EXIT(0)
      END IF

!***  Initialize temperatures in 3-D domain
!***  T(X,Y,Z)=TG(Z)
      READ (GroundTemp,*) RSKY,HHEAT,HMASS,DODPG,(TG(COUNT1), COUNT1=0,NZBGM1)
      IF (.not. SameString(TREAD,'FALSE')) THEN
        REWIND(75)
        DO COUNT1=0,NXM1
          DO COUNT2=0,NYM1
            DO COUNT3=-NZAG,NZBGM1
              READ (75,*) T(COUNT1,COUNT2,COUNT3)
              TCVG(COUNT1,COUNT2,COUNT3)=T(COUNT1,COUNT2,COUNT3)
            END DO
          END DO
        END DO
      ELSE
        DO COUNT1=0,NXM1
          DO COUNT2=0,NYM1
            DO COUNT3=0,NZBGM1
              T(COUNT1,COUNT2,COUNT3)=TG(COUNT3)
              TCVG(COUNT1,COUNT2,COUNT3)=TG(COUNT3)
            END DO
          END DO
        END DO
        DO COUNT1=0,NXM1
          DO COUNT2=0,NYM1
            DO COUNT3=-NZAG,-1
               T(COUNT1,COUNT2,COUNT3)=TIN(1)
               TCVG(COUNT1,COUNT2,COUNT3)=TIN(1)
            END DO
          END DO
        END DO
      END IF

!***  INITIALIZE GROUND SURFACE HEAT FLUX VALUES
      GINIT=TCON(4)*(TG(0)-TG(1))/DZP(0)
      DO COUNT1=0,NXM1
        DO COUNT2=0,NYM1
          GOFT(COUNT1,COUNT2,2)=GINIT
        END DO
      END DO

!      write(*,*) '2324=',VEXT(12,15,-1)
!***  COMPUTE A FEW MORE CONSTANTS
      IINT=XFACE(IBASE)
      JINT=YFACE(JBASE)
      KINT=ZFACE(KBASE)-ZFACE(-NZAG+1)
!      PRINT *, 'Entering main simulation block'
      WRITE(DebugOutFile,*) 'Entering main simulation block'

!*** TIME LOOP FOR 3-D CALCULATION
!*** YEARS:
      YEARS: DO IYR=1,IYRS
!***  If solution has converged, set switch to terminate
        IF (CVG) QUIT=.TRUE.
!***  Position BLAST weather file at the beginning of the year
!        REWIND (Weather)
!       READ (Weather,2300)
! 2301   FORMAT (/)

!***  Position TMY2 weather file at the beginning of the year
!        REWIND (Weather2)
!***  Rewind boundary condition file (Ground Temperature)
        REWIND (GroundTemp)
!***  Rewind boundary condition file (Above-grade surfaces)
        REWIND (SolarFile)
!** DAYS:
        IMON=1
        DAYS: DO IDAY=1,365
          IF (IDAY.EQ.NFDM(IMON)+NDIM(IMON)) IMON=IMON+1
!***  If this is the final year, initialize variables for output statistics
          IF (CVG.OR.IYR.EQ.IYRS) THEN
            DQCSUM=0.d0
            DQFSUM=0.d0
            DQRSUM=0.d0
            DQSSUM=0.d0
            DQWSUM=0.d0
            YHLOAD=0.d0
            YCLOAD=0.d0
            DTDBA=0.d0
            DTBA=0.d0
            DQCA=0.d0
            DQFA=0.d0
            DQRA=0.d0
            DQSA=0.d0
            DQWA=0.d0
            DTCA=0.d0
            DTFA=0.d0
            DTRA=0.d0
            DTSA=0.d0
            DTWA=0.d0
            QCMN=999999.d0
            QCMX=-999999.d0
            QFMN=999999.d0
            QFMX=-999999.d0
            QRMN=999999.d0
            QRMX=-999999.d0
            QSMN=999999.d0
            QSMX=-999999.d0
            QWMN=999999.d0
            QWMX=-999999.d0
            TCMN=999.d0
            TCMX=-999.d0
            TFMN=999.d0
            TFMX=-999.d0
            TRMN=999.d0
            TRMX=-999.d0
            TSMN=999.d0
            TSMX=-999.d0
            TWMN=999.d0
            TWMX=-999.d0
            DO COUNT1=0,IBASE+1
              DO COUNT2=0,JBASE+1
                DQC21(COUNT1,COUNT2)=0.d0
                DTC21(COUNT1,COUNT2)=0.d0
              END DO
            END DO
            DO COUNT1=0,IBASE-1
              DO COUNT2=0,JBASE-1
                DQF21(COUNT1,COUNT2)=0.d0
                DTF21(COUNT1,COUNT2)=0.d0
              END DO
            END DO
            DO COUNT2=0,JBASE+1
              DQRS21(COUNT2)=0.d0
              DTRS21(COUNT2)=0.d0
            END DO
            DO COUNT1=0,IBASE+1
              DQRW21(COUNT1)=0.d0
              DTRW21(COUNT1)=0.d0
            END DO
            DO COUNT1=IBASE,IBASE+1
              DO COUNT2=0,JBASE+1
                DQSS21(COUNT1,COUNT2)=0.d0
                DTSS21(COUNT1,COUNT2)=0.d0
              END DO
            END DO
            DO COUNT2=JBASE,JBASE+1
              DO COUNT1=0,IBASE-1
                DQSW21(COUNT1,COUNT2)=0.d0
                DTSW21(COUNT1,COUNT2)=0.d0
              END DO
            END DO
            DO COUNT2=0,JBASE-1
              DO COUNT3=-NZAG+2,KBASE-1
                DQWS21(COUNT2,COUNT3)=0.d0
                DTWS21(COUNT2,COUNT3)=0.d0
              END DO
            END DO
            DO COUNT1=0,IBASE-1
              DO COUNT3=-NZAG+2,KBASE-1
                DQWW21(COUNT1,COUNT3)=0.d0
                DTWW21(COUNT1,COUNT3)=0.d0
              END DO
            END DO
            DO COUNT1=0,NXM1
              DO COUNT3=-NZAG,NZBGM1
                TV1(COUNT1,COUNT3)=0.d0
                TV2(COUNT1,COUNT3)=0.d0
                TV3(COUNT1,COUNT3)=0.d0
              END DO
            END DO
          END IF
!      write(*,*) '2445=',VEXT(12,15,-1)

!***  Read one day of weather from files
          CALL GetWeatherData(IDAY) !(DSNOW)
          WND    =TodaysWeather%WND
          TWB    =TodaysWeather%TWB
          TDB    =TodaysWeather%TDB
          RDIFH  =TodaysWeather%RDIFH
          PBAR   =TodaysWeather%PBAR
          RBEAM  =TodaysWeather%RBEAM
          IF (MOD(IDAY,30) == 0 .or. IDAY == 365) THEN
            PRINT *, 'Year:',IYR,'Day:',IDAY,'Converge Status:',CVG
            Write(DebugOutFile,*) 'Year:',IYR,'Day:',IDAY,'Converge Status:',CVG
          ENDIF
!** HOURS:
          HOSUM=0.0
          TDBSUM=0.0
          TBSUM=0.0
          TISUM=0.0
          RSNWSUM=0.0
          RSOLVSUM=0.0
          COUNTER=0.0
          HOURS: DO IHR=1,24

!***  If this is the final year, initialize variables for output statistics
            IF (CVG.OR.IYR.EQ.IYRS) THEN
              QCSUM=0.d0
              QFSUM=0.d0
              QRSUM=0.d0
              QSSUM=0.d0
              QWSUM=0.d0
              TCSUM=0.d0
              TFSUM=0.d0
              TRSUM=0.d0
              TSSUM=0.d0
              TWSUM=0.d0
            END IF
!***  Read one hour of ground temperatures, etc.
            READ (GroundTemp,*) RSKY,HHEAT,HMASS,DODPG,(TG(COUNT1),COUNT1=0,NZBG)
!***  Read one hour of the area-weighted total absorbed solar radiation on a vertical
!***  surface (RSOLV) and the direct solar radiation incident on a horizontal surface
!***  (RDIRH)
            READ (SolarFile,*) RSOLV(IHR),RDIRH(IHR)

!***  For fixed temperature lower boundary condition runs, set value
            IF (.not. SameString(FIXBC,'FALSE')) TDEEP=TG(NZBG)
!***  For ground surface cells, compute surface boundary condition G(T) for this hour
!***  Save old values of direct and diffuse solar radiation incident on a
!***  horizontal surface
            IF (IHR.EQ.1) THEN
              RDIRHO=RDIRH(IHR)
              RDIFHO=RDIFH(IHR)
            ELSE
              RDIRHO=RDIRH(IHR-1)
              RDIFHO=RDIFH(IHR-1)
            END IF
!***  Set surface properties for this hour
            IF (DSNOW(IHR).EQ.0) THEN
              ISNW=1
            ELSE
              ISNW=2
            END IF
            IF (SameString(RSNOW,'FALSE')) ISNW=1
            ALB=ALBEDO(ISNW)
            EPS=EPSLN(ISNW)
            VHT=VEGHT(ISNW)
            RSNW=DSNOW(IHR)/100.d0/1.55d0
!***  Loop through ground surface cells
            DO COUNT1=0,NXM1
              DO COUNT2=0,NYM1
                IF (MTYPE(COUNT1,COUNT2,0).EQ.4) THEN
!***  Determine radiative flux to ground surface cells
                  RGRND=EPS*SIGMA*(T(COUNT1,COUNT2,0)+273.15d0)**4.
                  RSOLH=(1.-ALB)*(RDIRHO+RDIFHO+RDIRH(IHR)+RDIFH(IHR))/2.
                  RTOT=RSOLH+RSKY-RGRND
!***  Calculate G(T)
                  IF (.not. SameString(PET,'FALSE')) THEN
                    QPET=DODPG*(RTOT-GOFT(COUNT1,COUNT2,2))+HMASS*(TDB(IHR)-TWB(IHR))
                  ELSE
                    QPET=0.
                  END IF
                  GOFT(COUNT1,COUNT2,1)=RTOT-HHEAT*(T(COUNT1,COUNT2,0)-TDB(IHR))-QPET
                END IF
              END DO
            END DO
!***  Reset GOFT(COUNT1,COUNT2,2)
            DO COUNT1=0,NXM1
              DO COUNT2=0,NYM1
                GOFT(COUNT1,COUNT2,2)=GOFT(COUNT1,COUNT2,1)
              END DO
            END DO
!      write(*,*) '2536=',VEXT(12,15,-1)
!***  Calculate the convection component of the
!***  outside surface heat transfer coefficient
            IF (WND(IHR).GT.2.) THEN
              WNDW=0.25d0*WND(IHR)
            ELSE
              WNDW=0.5d0
            END IF
            WNDL=0.3d0+0.05d0*WND(IHR)
            HO=3.28d0*(((WNDW+WNDL)/2.d0)**0.605d0)
!***  Set surface temperature if the first time step,
!***  otherwise calculate them at the end of the time step
            IF (IYR.EQ.1.AND.IDAY.EQ.1.AND.IHR.LE.TSTEP) THEN
              DO COUNT1=0,IBASE+1
                DO COUNT2=0,JBASE+1
                  TC(COUNT1,COUNT2)=T(COUNT1,COUNT2,-NZAG)
                END DO
              END DO
              DO COUNT1=0,IBASE-1
                DO COUNT2=0,JBASE-1
                  TF(COUNT1,COUNT2)=T(COUNT1,COUNT2,KBASE)
                END DO
              END DO
              DO COUNT2=0,JBASE+1
                TRS(COUNT2)=T(IBASE+2,COUNT2,-NZAG+1)
              END DO
              DO COUNT1=0,IBASE+1
                TRW(COUNT1)=T(COUNT1,JBASE+2,-NZAG+1)
              END DO
              DO COUNT1=IBASE,IBASE+1
                DO COUNT2=0,JBASE+1
                  TSS(COUNT1,COUNT2)=T(COUNT1,COUNT2,-NZAG+2)
                END DO
              END DO
              DO COUNT2=JBASE,JBASE+1
                DO COUNT1=0,IBASE-1
                  TSW(COUNT1,COUNT2)=T(COUNT1,COUNT2,-NZAG+2)
                END DO
              END DO
              DO COUNT2=0,JBASE-1
                DO COUNT3=-NZAG+2,KBASE-1
                  TWS(COUNT2,COUNT3)=T(IBASE,COUNT2,COUNT3)
                END DO
              END DO
              DO COUNT1=0,IBASE-1
                DO COUNT3=-NZAG+2,KBASE-1
                  TWW(COUNT1,COUNT3)=T(COUNT1,JBASE,COUNT3)
                END DO
              END DO
            END IF

!***  Perform heat balance to determine the basement temperature.
!***  The basement air temperature will not be equal to the set point
!***  temperature during the cooling season because TB was found to be
!***  lower that TIN for the unconditioned case.
!
!           Commercial building case, monthly average temps
!
!      write(*,*) '2594=',VEXT(12,15,-1)
            IF(.not. SameString(ComBldg,'FALSE'))THEN
              TB(IHR)=TBasementAve(IMON) + TbasementDailyAmp*sin(6.28138d0*IHR/24.d0)
              TI=TIN(1)
            ELSE   ! Residential Case
              IF (.not. SameString(COND,'FALSE').AND.TDB(IHR).LE.(TDBH+13.67d0)/2.d0) THEN
                TB(IHR)=TIN(1)
                TI=TIN(1)
              ELSE
                CALL BasementHeatBalance(TB(IHR),TC,TF,TRS,TRW,TSS,TSW,TWS, &
                &    TWW,HIN,DX,DY,DZ,XDIM,YDIM,ZDIM)
                IF (TDB(IHR).GE.TDBC) THEN
                  TI=TIN(2)
                ELSE IF (TDB(IHR).LE.(TDBH+13.67d0)/2.d0) THEN
                  TI=TIN(1)
                ELSE
                   TI=((TIN(2)-TIN(1))/(TDBC-(TDBH+13.67d0)/2.d0))*             &
                   (TDB(IHR)-(TDBH+13.67d0)/2.d0)+TIN(1)
                END IF
                IF (TB(IHR).GT.TIN(2)) THEN
                  TB(IHR)=TIN(2)
                END IF
              END IF
            END IF
!***  Calculate variable sums for each hour
            HOSUM=HOSUM+HO
            TDBSUM=TDBSUM+TDB(IHR)
            TBSUM=TBSUM+TB(IHR)
            TISUM=TISUM+TI
            RSNWSUM=RSNWSUM+RSNW
            RSOLVSUM=RSOLVSUM+RSOLV(IHR)
            DO COUNT1=0,NXM1
              DO COUNT2=0,NYM1
                GOFTSUM(COUNT1,COUNT2)=GOFTSUM(COUNT1,COUNT2)+GOFT(COUNT1,COUNT2,1)
              END DO
            END DO
            DO COUNT3=0,NZBGM1
              TGSUM(COUNT3)=TGSUM(COUNT3)+TG(COUNT3)
            END DO
!***  TIME STEP
            COUNTER=COUNTER+1.d0/TSTEP
            COUNT: IF (INT(COUNTER).EQ.1.) THEN
!***  Calculate variable averages for each time step
              HOAV=HOSUM/TSTEP
              TDBAV=TDBSUM/TSTEP
              TBAV=TBSUM/TSTEP
              TIAV=TISUM/TSTEP
              RSNWAV=RSNWSUM/TSTEP
              RSOLVAV=RSOLVSUM/TSTEP
              DO COUNT1=0,NXM1
                DO COUNT2=0,NYM1
                  GOFTAV(COUNT1,COUNT2)=GOFTSUM(COUNT1,COUNT2)/TSTEP
                END DO
              END DO
              DO COUNT3=0,NZBGM1
                TGAV(COUNT3)=TGSUM(COUNT3)/TSTEP
              END DO
!*** If first time step, initialize exterior surface temperatures.
              IF(IYR.EQ.1.AND.IDAY.EQ.1.AND.NINT(TSTEP).EQ.IHR) THEN
                DO COUNT3=-NZAG,-1
                  DO COUNT2=0,JBASE+2
                    TEXT(IBASE+2,COUNT2,COUNT3)=T(IBASE+2,COUNT2,COUNT3)
                  END DO
                  DO COUNT1=0,IBASE+1
                    TEXT(COUNT1,JBASE+2,COUNT3)=T(COUNT1,JBASE+2,COUNT3)
                  END DO
                END DO
              END IF

!*** SOLUTION BY THE ALTERNATING DIRECTION IMPLICIT F-FACTOR METHOD
!*********************************************************************!
!***  FIRST FRACTION OF TIME INCREMENT (X-DIRECTION IMPLICIT,      ***!
!***  Y AND Z-DIRECTIONS EXPLICIT)                                 ***!
!*********************************************************************!

!***  SET CONSTANTS
              DO COUNT1=0,NXM1
                DO COUNT2=0,NYM1
                  DO COUNT3=-NZAG,NZBGM1
                    CONST(COUNT1,COUNT2,COUNT3)=(TSTEP*3600.d0)/3.d0/ &
                    & RHO(MTYPE(COUNT1,COUNT2,COUNT3))/   &
                    & CP(MTYPE(COUNT1,COUNT2,COUNT3))
                    IF (COUNT2.EQ.0) THEN
                      T(COUNT1,COUNT2-1,COUNT3)=T(COUNT1,COUNT2,COUNT3)
                    END IF
                    IF (COUNT2.EQ.NYM1.AND.COUNT3.GE.0) THEN
                      T(COUNT1,COUNT2+1,COUNT3)=TGAV(COUNT3)
                    END IF
                    IF (COUNT3.EQ.NZBGM1) THEN
                      IF (.not. SameString(FIXBC,'FALSE')) THEN
                        T(COUNT1,COUNT2,COUNT3+1)=TDEEP
                      ELSE
                        T(COUNT1,COUNT2,COUNT3+1)=T(COUNT1,COUNT2,COUNT3)
                      END IF
                    END IF
                  END DO
                END DO
              END DO

!      write(*,*) '2694=',VEXT(12,15,-1)
!***  SET UP COEFFICIENT MATRIX IN X-DIRECTION:
!***  SECTION 1:  CEILING CELLS
              DO COUNT2=0,JBASE+1
                IF (TIAV.GT.T(0,COUNT2,-NZAG)) THEN
                  HINZH=HIN(4)
                ELSE
                  HINZH=HIN(5)
                END IF
                IF (TBAV.GE.T(0,COUNT2,-NZAG)) THEN
                  HINZ=HIN(5)
                ELSE
                  HINZ=HIN(4)
                END IF
                A(0,COUNT2,-NZAG)=0.
                B(0,COUNT2,-NZAG)=1.d0+(3.d0-2.d0*F)*CONST(0,COUNT2,-NZAG)*           &
                & CXP(0,COUNT2,-NZAG)
                C(0,COUNT2,-NZAG)=(3.d0-2.d0*F)*CONST(0,COUNT2,-NZAG)*              &
                & (-CXP(0,COUNT2,-NZAG))
                R(0,COUNT2,-NZAG)=F*CONST(0,COUNT2,-NZAG)*(CYM(0,COUNT2,-NZAG)* &
                & T(0,COUNT2-1,-NZAG)-(CYM(0,COUNT2,-NZAG)+CYP(0,COUNT2,-NZAG))*&
                & T(0,COUNT2,-NZAG)+CYP(0,COUNT2,-NZAG)*T(0,COUNT2+1,-NZAG)-    &
                & (1.d0/DZ(-NZAG)/(1.d0/HINZH+DZ(-NZAG)/2.d0/                         &
                & TCON(MTYPE(0,COUNT2,-NZAG)))+1.d0/DZ(-NZAG)/(RCEIL+             &
                & 1.d0/HINZ+DZ(-NZAG)/2.d0/TCON(MTYPE(0,COUNT2,-NZAG))))*           &
                & T(0,COUNT2,-NZAG))+T(0,COUNT2,-NZAG)+F*CONST(0,COUNT2,-NZAG)* &
                & (TIAV/DZ(-NZAG)/(1.d0/HINZH+DZ(-NZAG)/2.d0/                       &
                & TCON(MTYPE(0,COUNT2,-NZAG)))+TBAV/DZ(-NZAG)/(RCEIL+     &
                & 1.d0/HINZ+DZ(-NZAG)/2.d0/TCON(MTYPE(0,COUNT2,-NZAG))))
                DO COUNT1=1,IBASE+1
                  IF (TIAV.GT.T(COUNT1,COUNT2,-NZAG)) THEN
                    HINZH=HIN(4)
                  ELSE
                    HINZH=HIN(5)
                  END IF
                  IF (TBAV.GE.T(COUNT1,COUNT2,-NZAG)) THEN
                    HINZ=HIN(5)
                  ELSE
                    HINZ=HIN(4)
                  END IF
                  A(COUNT1,COUNT2,-NZAG)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,-NZAG)*      &
                  & (-CXM(COUNT1,COUNT2,-NZAG))
                  B(COUNT1,COUNT2,-NZAG)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,-NZAG)*   &
                  &  (CXM(COUNT1,COUNT2,-NZAG)+CXP(COUNT1,COUNT2,-NZAG))
                  C(COUNT1,COUNT2,-NZAG)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,-NZAG)*      &
                  & (-CXP(COUNT1,COUNT2,-NZAG))
                  R(COUNT1,COUNT2,-NZAG)=F*CONST(COUNT1,COUNT2,-NZAG)*              &
                  & (CYM(COUNT1,COUNT2,-NZAG)*T(COUNT1,COUNT2-1,-NZAG)-             &
                  & (CYM(COUNT1,COUNT2,-NZAG)+CYP(COUNT1,COUNT2,-NZAG))*            &
                  & T(COUNT1,COUNT2,-NZAG)+CYP(COUNT1,COUNT2,-NZAG)*                &
                  & T(COUNT1,COUNT2+1,-NZAG)-(1.d0/DZ(-NZAG)/(1.d0/HINZH+DZ(-NZAG)/2.d0/  &
                  & TCON(MTYPE(COUNT1,COUNT2,-NZAG)))+1.d0/DZ(-NZAG)/(RCEIL+          &
                  & 1.d0/HINZ+DZ(-NZAG)/2.d0/TCON(MTYPE(COUNT1,COUNT2,-NZAG))))*        &
                  & T(COUNT1,COUNT2,-NZAG))+T(COUNT1,COUNT2,-NZAG)+F*               &
                  & CONST(COUNT1,COUNT2,-NZAG)*(TIAV/DZ(-NZAG)/(1.d0/HINZH+           &
                  & DZ(-NZAG)/2.d0/TCON(MTYPE(COUNT1,COUNT2,-NZAG)))+TBAV/DZ(-NZAG)/  &
                  & (RCEIL+1.d0/HINZ+DZ(-NZAG)/2.d0/TCON(MTYPE(COUNT1,COUNT2,-NZAG))))
                END DO

!***  PERIMETER CEILING CELL, PARALLEL TO Y-AXIS
                A(IBASE+2,COUNT2,-NZAG)=(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,-NZAG)*        &
                & (-CXM(IBASE+2,COUNT2,-NZAG))
                B(IBASE+2,COUNT2,-NZAG)=1.d0+(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,-NZAG)*     &
                & (CXM(IBASE+2,COUNT2,-NZAG)+1.d0/DX(IBASE+2)/(REXT+RSID+1.d0/HOAV))
                C(IBASE+2,COUNT2,-NZAG)=0.
                R(IBASE+2,COUNT2,-NZAG)=F*CONST(IBASE+2,COUNT2,-NZAG)*                &
                & (CYM(IBASE+2,COUNT2,-NZAG)*                                         &
                & T(IBASE+2,COUNT2-1,-NZAG)-(CYM(IBASE+2,COUNT2,-NZAG)+               &
                & CYP(IBASE+2,COUNT2,-NZAG))*T(IBASE+2,COUNT2,-NZAG)+                 &
                & CYP(IBASE+2,COUNT2,-NZAG)*T(IBASE+2,COUNT2+1,-NZAG)-                &
                & CZP(IBASE+2,COUNT2,-NZAG)*T(IBASE+2,COUNT2,-NZAG)+                  &
                & CZP(IBASE+2,COUNT2,-NZAG)*T(IBASE+2,COUNT2,-NZAG+1))+               &
                & T(IBASE+2,COUNT2,-NZAG)+(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,-NZAG)*      &
                & (TDBAV/DX(IBASE+2)/(REXT+RSID+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-         &
                & (0.88d0*SIGMA*(TEXT(IBASE+2,COUNT2,-NZAG)+273.15d0)**4)/DX(IBASE+2))
                N=IBASE+2-0+1
                L=1
                DO COUNT1=0,IBASE+2
                  AA(L)=A(COUNT1,COUNT2,-NZAG)
                  BB(L)=B(COUNT1,COUNT2,-NZAG)
                  CC(L)=C(COUNT1,COUNT2,-NZAG)
                  RR(L)=R(COUNT1,COUNT2,-NZAG)
                  L=L+1
                END DO
                CALL TRIDI3D (AA,BB,CC,RR,N,X)
                L=0
                DO COUNT1=0,IBASE+2
                  U(COUNT1,COUNT2,-NZAG)=X(L+1)
                  L=L+1
                END DO
                QEXT(IBASE+2,COUNT2,-NZAG)=(TDBAV-U(IBASE+2,COUNT2,-NZAG))/            &
                & (REXT+RSID+1.d0/HOAV)
                UEXT(IBASE+2,COUNT2,-NZAG)=QEXT(IBASE+2,COUNT2,-NZAG)*(REXT+RSID)+     &
                & U(IBASE+2,COUNT2,-NZAG)
              END DO
!***  PERIMETER CEILING CELLS, PARALLEL TO X-AXIS
              A(0,JBASE+2,-NZAG)=0.
              B(0,JBASE+2,-NZAG)=1.d0+(3.d0-2.d0*F)*CONST(0,JBASE+2,-NZAG)*           &
              & CXP(0,JBASE+2,-NZAG)
              C(0,JBASE+2,-NZAG)=(3.d0-2.d0*F)*CONST(0,JBASE+2,-NZAG)*              &
              & (-CXP(0,JBASE+2,-NZAG))
              R(0,JBASE+2,-NZAG)=F*CONST(0,JBASE+2,-NZAG)*                      &
              & (CYM(0,JBASE+2,-NZAG)*T(0,JBASE+2-1,-NZAG)-                     &
              & (CYM(0,JBASE+2,-NZAG)+1.d0/DY(JBASE+2)/(REXT+RSID+1.d0/HOAV))*      &
              & T(0,JBASE+2,-NZAG)-CZP(0,JBASE+2,-NZAG)*T(0,JBASE+2,-NZAG)+     &
              & CZP(0,JBASE+2,-NZAG)*T(0,JBASE+2,-NZAG+1))+                     &
              & T(0,JBASE+2,-NZAG)+F*CONST(0,JBASE+2,-NZAG)*                    &
              & (TDBAV/DY(JBASE+2)/(REXT+RSID+1.d0/HOAV)+RSOLVAV/DY(JBASE+2)-     &
              & (0.88d0*SIGMA*(TEXT(0,JBASE+2,-NZAG)+273.15d0)**4)/DY(JBASE+2))

              DO COUNT1=1,IBASE+1
                A(COUNT1,JBASE+2,-NZAG)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,-NZAG)*       &
                & (-CXM(COUNT1,JBASE+2,-NZAG))
                B(COUNT1,JBASE+2,-NZAG)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,-NZAG)*    &
                & (CXM(COUNT1,JBASE+2,-NZAG)+CXP(COUNT1,JBASE+2,-NZAG))
                C(COUNT1,JBASE+2,-NZAG)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,-NZAG)*       &
                & (-CXP(COUNT1,JBASE+2,-NZAG))
                R(COUNT1,JBASE+2,-NZAG)=F*CONST(COUNT1,JBASE+2,-NZAG)*               &
                & (CYM(COUNT1,JBASE+2,-NZAG)*T(COUNT1,JBASE+2-1,-NZAG)-              &
                & (CYM(COUNT1,JBASE+2,-NZAG)+1.d0/DY(JBASE+2)/(REXT+RSID+1.d0/HOAV))*    &
                & T(COUNT1,JBASE+2,-NZAG)-CZP(COUNT1,JBASE+2,-NZAG)*                 &
                & T(COUNT1,JBASE+2,-NZAG)+CZP(COUNT1,JBASE+2,-NZAG)*                 &
                & T(COUNT1,JBASE+2,-NZAG+1))+T(COUNT1,JBASE+2,-NZAG)+F*              &
                & CONST(COUNT1,JBASE+2,-NZAG)*(TDBAV/DY(JBASE+2)/(REXT+RSID+1.d0/HOAV)+ &
                & RSOLVAV/DY(JBASE+2)-                                               &
                & (0.88d0*SIGMA*(TEXT(COUNT1,JBASE+2,-NZAG)+273.15d0)**4)/DY(JBASE+2))
              END DO

!***  TOP OUTER CORNER CELL
              A(IBASE+2,JBASE+2,-NZAG)=(3.d0-2.d0*F)*CONST(IBASE+2,JBASE+2,-NZAG)*  &
                    & (-CXM(IBASE+2,JBASE+2,-NZAG))
              B(IBASE+2,JBASE+2,-NZAG)=1.d0+(3.d0-2.d0*F)*                            &
                    & CONST(IBASE+2,JBASE+2,-NZAG)*(CXM(IBASE+2,JBASE+2,-NZAG)+       &
                & 1.d0/DX(IBASE+2)/(REXT+RSID+1.d0/HOAV))
              C(IBASE+2,JBASE+2,-NZAG)=0.
              R(IBASE+2,JBASE+2,-NZAG)=F*CONST(IBASE+2,JBASE+2,-NZAG)*          &
              & (CYM(IBASE+2,JBASE+2,-NZAG)*T(IBASE+2,JBASE+2-1,-NZAG)-         &
              & (CYM(IBASE+2,JBASE+2,-NZAG)+1.d0/DY(JBASE+2)/(REXT+RSID+          &
              & 1.d0/HOAV))*T(IBASE+2,JBASE+2,-NZAG)-                             &
              & CZP(IBASE+2,JBASE+2,-NZAG)*T(IBASE+2,JBASE+2,-NZAG)+            &
              & CZP(IBASE+2,JBASE+2,-NZAG)*T(IBASE+2,JBASE+2,-NZAG+1))+         &
              & T(IBASE+2,JBASE+2,-NZAG)+(3.d0-2.d0*F)*                             &
              & CONST(IBASE+2,JBASE+2,-NZAG)*(TDBAV/DX(IBASE+2)/                &
              & (REXT+RSID+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-(0.88d0*SIGMA*            &
              & (TEXT(IBASE+2,JBASE+2,-NZAG)+273.15d0)**4)/DX(IBASE+2))+F*       &
              & CONST(IBASE+2,JBASE+2,-NZAG)*(TDBAV/DY(JBASE+2)/                &
              & (REXT+RSID+1.d0/HOAV)+RSOLVAV/DY(JBASE+2)-(0.88d0*SIGMA*            &
              & (TEXT(IBASE+2,JBASE+2,-NZAG)+273.15d0)**4)/DY(JBASE+2))

              N=IBASE+2-0+1
              L=1
              DO COUNT1=0,IBASE+2
                AA(L)=A(COUNT1,JBASE+2,-NZAG)
                BB(L)=B(COUNT1,JBASE+2,-NZAG)
                CC(L)=C(COUNT1,JBASE+2,-NZAG)
                RR(L)=R(COUNT1,JBASE+2,-NZAG)
                L=L+1
              END DO
              CALL TRIDI3D (AA,BB,CC,RR,N,X)
              L=0
              DO COUNT1=0,IBASE+2
                U(COUNT1,JBASE+2,-NZAG)=X(L+1)
                L=L+1
                QEXT(COUNT1,JBASE+2,-NZAG)=(TDBAV-U(COUNT1,JBASE+2,-NZAG))/           &
                &   (REXT+RSID+1.d0/HOAV)
                UEXT(COUNT1,JBASE+2,-NZAG)=QEXT(COUNT1,JBASE+2,-NZAG)*(REXT+RSID)+    &
                &     U(COUNT1,JBASE+2,-NZAG)
              END DO

!***  SECTION 2:  ABOVE-GRADE FOUNDATION WALL, PARALLEL TO X-AXIS
!***  TOP INSIDE WALL CELLS
              IF (TBAV.GT.T(0,JBASE,-NZAG+2)) THEN
                HINZ=HIN(4)
              ELSE
                HINZ=HIN(5)
              END IF
              A(0,JBASE,-NZAG+2)=0.
              B(0,JBASE,-NZAG+2)=1.d0+(3.d0-2.d0*F)*CONST(0,JBASE,-NZAG+2)*           &
                   & CXP(0,JBASE,-NZAG+2)
              C(0,JBASE,-NZAG+2)=(3.d0-2.d0*F)*CONST(0,JBASE,-NZAG+2)*              &
                   & (-CXP(0,JBASE,-NZAG+2))
              R(0,JBASE,-NZAG+2)=F*CONST(0,JBASE,-NZAG+2)*(-(1.d0/DY(JBASE)/      &
              & (RINT+1.d0/HIN(6))+CYP(0,JBASE,-NZAG+2))*T(0,JBASE,-NZAG+2)+      &
              & CYP(0,JBASE,-NZAG+2)*T(0,JBASE+1,-NZAG+2)-(1.d0/DZ(-NZAG+2)/      &
              & (1.d0/HINZ+RSILL+DZ(-NZAG+2)/2.d0/TCON(MTYPE(0,JBASE,-NZAG+2)))+    &
              & CZP(0,JBASE,-NZAG+2))*T(0,JBASE,-NZAG+2)+                       &
              & CZP(0,JBASE,-NZAG+2)*T(0,JBASE,-NZAG+2+1))+                     &
              & T(0,JBASE,-NZAG+2)+F*CONST(0,JBASE,-NZAG+2)*(TBAV/DY(JBASE)/    &
              & (RINT+1.d0/HIN(6))+TBAV/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+               &
              & DZ(-NZAG+2)/2.d0/TCON(MTYPE(0,JBASE,-NZAG+2))))
              DO COUNT1=1,IBASE-1
                IF (TBAV.GT.T(COUNT1,JBASE,-NZAG+2)) THEN
                  HINZ=HIN(4)
                ELSE
                  HINZ=HIN(5)
                END IF
                A(COUNT1,JBASE,-NZAG+2)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,-NZAG+2)*         &
                & (-CXM(COUNT1,JBASE,-NZAG+2))
                B(COUNT1,JBASE,-NZAG+2)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,-NZAG+2)*      &
                & (CXM(COUNT1,JBASE,-NZAG+2)+CXP(COUNT1,JBASE,-NZAG+2))
                C(COUNT1,JBASE,-NZAG+2)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,-NZAG+2)*         &
                & (-CXP(COUNT1,JBASE,-NZAG+2))
                R(COUNT1,JBASE,-NZAG+2)=F*CONST(COUNT1,JBASE,-NZAG+2)*(-(1.d0/DY(JBASE)/ &
                & (RINT+1.d0/HIN(6))+CYP(COUNT1,JBASE,-NZAG+2))*T(COUNT1,JBASE,-NZAG+2)+ &
                & CYP(COUNT1,JBASE,-NZAG+2)*T(COUNT1,JBASE+1,-NZAG+2)-(1.d0/DZ(-NZAG+2)/ &
                & (1.d0/HINZ+RSILL+DZ(-NZAG+2)/2.d0/TCON(MTYPE(COUNT1,JBASE,-NZAG+2)))+    &
                & CZP(COUNT1,JBASE,-NZAG+2))*T(COUNT1,JBASE,-NZAG+2)+                  &
                & CZP(COUNT1,JBASE,-NZAG+2)*T(COUNT1,JBASE,-NZAG+2+1))+                &
                & T(COUNT1,JBASE,-NZAG+2)+F*CONST(COUNT1,JBASE,-NZAG+2)*(TBAV/DY(JBASE)/ &
                & (RINT+1.d0/HIN(6))+TBAV/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+                    &
                & DZ(-NZAG+2)/2.d0/TCON(MTYPE(COUNT1,JBASE,-NZAG+2))))
              END DO

!***  INSIDE CORNER CELL AND TOP CENTER WALL CELL
              DO COUNT1=IBASE,IBASE+1
                IF (TBAV.GT.T(COUNT1,JBASE,-NZAG+2)) THEN
                  HINZ=HIN(4)
                ELSE
                  HINZ=HIN(5)
                END IF
                A(COUNT1,JBASE,-NZAG+2)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,-NZAG+2)*      &
                & (-CXM(COUNT1,JBASE,-NZAG+2))
                B(COUNT1,JBASE,-NZAG+2)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,-NZAG+2)*   &
                & (CXM(COUNT1,JBASE,-NZAG+2)+CXP(COUNT1,JBASE,-NZAG+2))
                C(COUNT1,JBASE,-NZAG+2)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,-NZAG+2)*      &
                & (-CXP(COUNT1,JBASE,-NZAG+2))
                R(COUNT1,JBASE,-NZAG+2)=F*CONST(COUNT1,JBASE,-NZAG+2)*              &
                & (CYM(COUNT1,JBASE,-NZAG+2)*T(COUNT1,JBASE-1,-NZAG+2)-             &
                & (CYM(COUNT1,JBASE,-NZAG+2)+CYP(COUNT1,JBASE,-NZAG+2))*            &
                & T(COUNT1,JBASE,-NZAG+2)+CYP(COUNT1,JBASE,-NZAG+2)*                &
                & T(COUNT1,JBASE+1,-NZAG+2)-(1.d0/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+    &
                & DZ(-NZAG+2)/2.d0/TCON(MTYPE(COUNT1,JBASE,-NZAG+2)))+           &
                & CZP(COUNT1,JBASE,-NZAG+2))*T(COUNT1,JBASE,-NZAG+2)+               &
                & CZP(COUNT1,JBASE,-NZAG+2)*T(COUNT1,JBASE,-NZAG+2+1))+             &
                & T(COUNT1,JBASE,-NZAG+2)+F*CONST(COUNT1,JBASE,-NZAG+2)*            &
                & (TBAV/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+DZ(-NZAG+2)/2.d0/        &
                & TCON(MTYPE(COUNT1,JBASE,-NZAG+2))))
              END DO

!***  TOP OUTSIDE WALL CELL
              A(IBASE+2,JBASE,-NZAG+2)=(3.d0-2.d0*F)*CONST(IBASE+2,JBASE,-NZAG+2)*  &
              & (-CXM(IBASE+2,JBASE,-NZAG+2))
              B(IBASE+2,JBASE,-NZAG+2)=1.d0+(3.d0-2.d0*F)*                            &
              & CONST(IBASE+2,JBASE,-NZAG+2)*(CXM(IBASE+2,JBASE,-NZAG+2)+       &
              & 1.d0/DX(IBASE+2)/(REXT+RSID+1.d0/HOAV))
              C(IBASE+2,JBASE,-NZAG+2)=0.
              R(IBASE+2,JBASE,-NZAG+2)=F*CONST(IBASE+2,JBASE,-NZAG+2)*          &
              & (CYM(IBASE+2,JBASE,-NZAG+2)*T(IBASE+2,JBASE-1,-NZAG+2)-         &
              & (CYM(IBASE+2,JBASE,-NZAG+2)+CYP(IBASE+2,JBASE,-NZAG+2))*        &
              & T(IBASE+2,JBASE,-NZAG+2)+CYP(IBASE+2,JBASE,-NZAG+2)*            &
              & T(IBASE+2,JBASE+1,-NZAG+2)+CZM(IBASE+2,JBASE,-NZAG+2)*          &
              & T(IBASE+2,JBASE,-NZAG+2-1)-(CZM(IBASE+2,JBASE,-NZAG+2)+         &
              & CZP(IBASE+2,JBASE,-NZAG+2))*T(IBASE+2,JBASE,-NZAG+2)+           &
              & CZP(IBASE+2,JBASE,-NZAG+2)*T(IBASE+2,JBASE,-NZAG+2+1))+         &
              & T(IBASE+2,JBASE,-NZAG+2)+(3.d0-2.d0*F)*                             &
              & CONST(IBASE+2,JBASE,-NZAG+2)*(TDBAV/DX(IBASE+2)/                &
              & (REXT+RSID+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-(0.88d0*SIGMA*            &
              & (TEXT(IBASE+2,JBASE,-NZAG+2)+273.15d0)**4)/DX(IBASE+2))

              N=IBASE+2-0+1
              L=1
              DO COUNT1=0,IBASE+2
                AA(L)=A(COUNT1,JBASE,-NZAG+2)
                BB(L)=B(COUNT1,JBASE,-NZAG+2)
                CC(L)=C(COUNT1,JBASE,-NZAG+2)
                RR(L)=R(COUNT1,JBASE,-NZAG+2)
                L=L+1
              END DO
              CALL TRIDI3D (AA,BB,CC,RR,N,X)
              L=0
              DO COUNT1=0,IBASE+2
                U(COUNT1,JBASE,-NZAG+2)=X(L+1)
                L=L+1
              END DO

              QEXT(IBASE+2,JBASE,-NZAG+2)=(TDBAV-U(IBASE+2,JBASE,-NZAG+2))/     &
              & (REXT+RSID+1.d0/HOAV)
              UEXT(IBASE+2,JBASE,-NZAG+2)=QEXT(IBASE+2,JBASE,-NZAG+2)*          &
              & (REXT+RSID)+U(IBASE+2,JBASE,-NZAG+2)

!***  TOP CENTER WALL CELLS
              IF (TBAV.GT.T(0,JBASE+1,-NZAG+2)) THEN
                HINZ=HIN(4)
              ELSE
                HINZ=HIN(5)
              END IF
              A(0,JBASE+1,-NZAG+2)=0.
              B(0,JBASE+1,-NZAG+2)=1.d0+(3.d0-2.d0*F)*CONST(0,JBASE+1,-NZAG+2)*       &
              & CXP(0,JBASE+1,-NZAG+2)
              C(0,JBASE+1,-NZAG+2)=(3.d0-2.d0*F)*CONST(0,JBASE+1,-NZAG+2)*          &
              & (-CXP(0,JBASE+1,-NZAG+2))
              R(0,JBASE+1,-NZAG+2)=F*CONST(0,JBASE+1,-NZAG+2)*                  &
              & (CYM(0,JBASE+1,-NZAG+2)*T(0,JBASE+1-1,-NZAG+2)-                 &
              & (CYM(0,JBASE+1,-NZAG+2)+CYP(0,JBASE+1,-NZAG+2))*                &
              & T(0,JBASE+1,-NZAG+2)+CYP(0,JBASE+1,-NZAG+2)*                    &
              & T(0,JBASE+1+1,-NZAG+2)-(1.d0/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+          &
              & DZ(-NZAG+2)/2.d0/TCON(MTYPE(0,JBASE+1,-NZAG+2)))+                 &
              & CZP(0,JBASE+1,-NZAG+2))*T(0,JBASE+1,-NZAG+2)+                   &
              & CZP(0,JBASE+1,-NZAG+2)*T(0,JBASE+1,-NZAG+2+1))+                 &
              & T(0,JBASE+1,-NZAG+2)+F*CONST(0,JBASE+1,-NZAG+2)*                &
              & (TBAV/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+DZ(-NZAG+2)/2.d0/                &
              & TCON(MTYPE(0,JBASE+1,-NZAG+2))))

              DO COUNT1=1,IBASE+1
                IF (TBAV.GT.T(COUNT1,JBASE+1,-NZAG+2)) THEN
                  HINZ=HIN(4)
                ELSE
                  HINZ=HIN(5)
                END IF
                A(COUNT1,JBASE+1,-NZAG+2)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+1,-NZAG+2)*    &
                & (-CXM(COUNT1,JBASE+1,-NZAG+2))
                B(COUNT1,JBASE+1,-NZAG+2)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE+1,-NZAG+2)* &
                & (CXM(COUNT1,JBASE+1,-NZAG+2)+CXP(COUNT1,JBASE+1,-NZAG+2))
                C(COUNT1,JBASE+1,-NZAG+2)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+1,-NZAG+2)*    &
                & (-CXP(COUNT1,JBASE+1,-NZAG+2))
                R(COUNT1,JBASE+1,-NZAG+2)=F*CONST(COUNT1,JBASE+1,-NZAG+2)*            &
                & (CYM(COUNT1,JBASE+1,-NZAG+2)*T(COUNT1,JBASE+1-1,-NZAG+2)-           &
                & (CYM(COUNT1,JBASE+1,-NZAG+2)+CYP(COUNT1,JBASE+1,-NZAG+2))*          &
                & T(COUNT1,JBASE+1,-NZAG+2)+CYP(COUNT1,JBASE+1,-NZAG+2)*              &
                & T(COUNT1,JBASE+1+1,-NZAG+2)-(1.d0/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+         &
                & DZ(-NZAG+2)/2.d0/TCON(MTYPE(COUNT1,JBASE+1,-NZAG+2)))+                &
                & CZP(COUNT1,JBASE+1,-NZAG+2))*T(COUNT1,JBASE+1,-NZAG+2)+             &
                & CZP(COUNT1,JBASE+1,-NZAG+2)*T(COUNT1,JBASE+1,-NZAG+2+1))+           &
                & T(COUNT1,JBASE+1,-NZAG+2)+F*CONST(COUNT1,JBASE+1,-NZAG+2)*          &
                & (TBAV/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+DZ(-NZAG+2)/2.d0/                    &
                & TCON(MTYPE(COUNT1,JBASE+1,-NZAG+2))))
              END DO

!***  TOP OUTSIDE WALL CELL
              A(IBASE+2,JBASE+1,-NZAG+2)=(3.d0-2.d0*F)*                           &
              & CONST(IBASE+2,JBASE+1,-NZAG+2)*                               &
              & (-CXM(IBASE+2,JBASE+1,-NZAG+2))
              B(IBASE+2,JBASE+1,-NZAG+2)=1.d0+(3.d0-2.d0*F)*                        &
              & CONST(IBASE+2,JBASE+1,-NZAG+2)*                               &
              & (CXM(IBASE+2,JBASE+1,-NZAG+2)+1.d0/DX(IBASE+2)/                 &
              & (REXT+RSID+1.d0/HOAV))
              C(IBASE+2,JBASE+1,-NZAG+2)=0.
              R(IBASE+2,JBASE+1,-NZAG+2)=F*CONST(IBASE+2,JBASE+1,-NZAG+2)*    &
              & (CYM(IBASE+2,JBASE+1,-NZAG+2)*T(IBASE+2,JBASE+1-1,-NZAG+2)-   &
              & (CYM(IBASE+2,JBASE+1,-NZAG+2)+CYP(IBASE+2,JBASE+1,-NZAG+2))*  &
              & T(IBASE+2,JBASE+1,-NZAG+2)+CYP(IBASE+2,JBASE+1,-NZAG+2)*      &
              & T(IBASE+2,JBASE+1+1,-NZAG+2)+CZM(IBASE+2,JBASE+1,-NZAG+2)*    &
              & T(IBASE+2,JBASE+1,-NZAG+2-1)-(CZM(IBASE+2,JBASE+1,-NZAG+2)+   &
              & CZP(IBASE+2,JBASE+1,-NZAG+2))*T(IBASE+2,JBASE+1,-NZAG+2)+     &
              & CZP(IBASE+2,JBASE+1,-NZAG+2)*T(IBASE+2,JBASE+1,-NZAG+2+1))+   &
              & T(IBASE+2,JBASE+1,-NZAG+2)+(3.d0-2.d0*F)*                         &
              & CONST(IBASE+2,JBASE+1,-NZAG+2)*(TDBAV/DX(IBASE+2)/            &
              & (REXT+RSID+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-(0.88d0*SIGMA*          &
              & (TEXT(IBASE+2,JBASE+1,-NZAG+2)+273.15d0)**4)/DX(IBASE+2))

              N=IBASE+2-0+1
              L=1
              DO COUNT1=0,IBASE+2
                AA(L)=A(COUNT1,JBASE+1,-NZAG+2)
                BB(L)=B(COUNT1,JBASE+1,-NZAG+2)
                CC(L)=C(COUNT1,JBASE+1,-NZAG+2)
                RR(L)=R(COUNT1,JBASE+1,-NZAG+2)
                L=L+1
              END DO
              CALL TRIDI3D (AA,BB,CC,RR,N,X)
              L=0
              DO COUNT1=0,IBASE+2
                U(COUNT1,JBASE+1,-NZAG+2)=X(L+1)
                L=L+1
              END DO
              QEXT(IBASE+2,JBASE+1,-NZAG+2)=(TDBAV-U(IBASE+2,JBASE+1,-NZAG+2))/ &
              & (REXT+RSID+1.d0/HOAV)
              UEXT(IBASE+2,JBASE+1,-NZAG+2)=QEXT(IBASE+2,JBASE+1,-NZAG+2)*      &
              & (REXT+RSID)+U(IBASE+2,JBASE+1,-NZAG+2)

!***  TOP OUTSIDE WALL CELLS
              A(0,JBASE+2,-NZAG+2)=0.
              B(0,JBASE+2,-NZAG+2)=1.d0+(3.d0-2.d0*F)*CONST(0,JBASE+2,-NZAG+2)*       &
              & CXP(0,JBASE+2,-NZAG+2)
              C(0,JBASE+2,-NZAG+2)=(3.d0-2.d0*F)*CONST(0,JBASE+2,-NZAG+2)*          &
              & (-CXP(0,JBASE+2,-NZAG+2))
              R(0,JBASE+2,-NZAG+2)=F*CONST(0,JBASE+2,-NZAG+2)*                  &
              & (CYM(0,JBASE+2,-NZAG+2)*T(0,JBASE+2-1,-NZAG+2)-                 &
              & (CYM(0,JBASE+2,-NZAG+2)+1.d0/DY(JBASE+2)/(REXT+RSID+1.d0/HOAV))*    &
              & T(0,JBASE+2,-NZAG+2)+CZM(0,JBASE+2,-NZAG+2)*                    &
              & T(0,JBASE+2,-NZAG+2-1)-(CZM(0,JBASE+2,-NZAG+2)+                 &
              & CZP(0,JBASE+2,-NZAG+2))*T(0,JBASE+2,-NZAG+2)+                   &
              & CZP(0,JBASE+2,-NZAG+2)*T(0,JBASE+2,-NZAG+2+1))+                 &
              & T(0,JBASE+2,-NZAG+2)+F*CONST(0,JBASE+2,-NZAG+2)*                &
              & (TDBAV/DY(JBASE+2)/(REXT+RSID+1.d0/HOAV)+RSOLVAV/DY(JBASE+2)-     &
              & (0.88d0*SIGMA*(TEXT(0,JBASE+2,-NZAG+2)+273.15d0)**4)/              &
              & DY(JBASE+2))

              DO COUNT1=1,IBASE+1
                A(COUNT1,JBASE+2,-NZAG+2)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,-NZAG+2)*    &
                & (-CXM(COUNT1,JBASE+2,-NZAG+2))
                B(COUNT1,JBASE+2,-NZAG+2)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,-NZAG+2)* &
                & (CXM(COUNT1,JBASE+2,-NZAG+2)+CXP(COUNT1,JBASE+2,-NZAG+2))
                C(COUNT1,JBASE+2,-NZAG+2)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,-NZAG+2)*    &
                & (-CXP(COUNT1,JBASE+2,-NZAG+2))
                R(COUNT1,JBASE+2,-NZAG+2)=F*CONST(COUNT1,JBASE+2,-NZAG+2)*            &
                & (CYM(COUNT1,JBASE+2,-NZAG+2)*T(COUNT1,JBASE+2-1,-NZAG+2)-           &
                & (CYM(COUNT1,JBASE+2,-NZAG+2)+1.d0/DY(JBASE+2)/(REXT+RSID+1.d0/HOAV))*   &
                & T(COUNT1,JBASE+2,-NZAG+2)+CZM(COUNT1,JBASE+2,-NZAG+2)*              &
                & T(COUNT1,JBASE+2,-NZAG+2-1)-(CZM(COUNT1,JBASE+2,-NZAG+2)+           &
                & CZP(COUNT1,JBASE+2,-NZAG+2))*T(COUNT1,JBASE+2,-NZAG+2)+             &
                & CZP(COUNT1,JBASE+2,-NZAG+2)*T(COUNT1,JBASE+2,-NZAG+2+1))+           &
                & T(COUNT1,JBASE+2,-NZAG+2)+F*CONST(COUNT1,JBASE+2,-NZAG+2)*          &
                & (TDBAV/DY(JBASE+2)/(REXT+RSID+1.d0/HOAV)+RSOLVAV/DY(JBASE+2)-         &
                & (0.88d0*SIGMA*(TEXT(COUNT1,JBASE+2,-NZAG+2)+273.15d0)**4)/            &
                & DY(JBASE+2))
              END DO

!***  TOP OUTER CORNER CELL
              A(IBASE+2,JBASE+2,-NZAG+2)=(3.d0-2.d0*F)*                             &
              & CONST(IBASE+2,JBASE+2,-NZAG+2)*                                 &
              & (-CXM(IBASE+2,JBASE+2,-NZAG+2))
              B(IBASE+2,JBASE+2,-NZAG+2)=1.d0+(3.d0-2.d0*F)*                          &
              & CONST(IBASE+2,JBASE+2,-NZAG+2)*                                 &
              & (CXM(IBASE+2,JBASE+2,-NZAG+2)+1.d0/DX(IBASE+2)/                   &
              & (REXT+RSID+1.d0/HOAV))
              C(IBASE+2,JBASE+2,-NZAG+2)=0.
              R(IBASE+2,JBASE+2,-NZAG+2)=F*CONST(IBASE+2,JBASE+2,-NZAG+2)*      &
              & (CYM(IBASE+2,JBASE+2,-NZAG+2)*T(IBASE+2,JBASE+2-1,-NZAG+2)-     &
              & (CYM(IBASE+2,JBASE+2,-NZAG+2)+1.d0/DY(JBASE+2)/(REXT+RSID+        &
              & 1.d0/HOAV))*T(IBASE+2,JBASE+2,-NZAG+2)+                           &
              & CZM(IBASE+2,JBASE+2,-NZAG+2)*T(IBASE+2,JBASE+2,-NZAG+2-1)-      &
              & (CZM(IBASE+2,JBASE+2,-NZAG+2)+CZP(IBASE+2,JBASE+2,-NZAG+2))*    &
              & T(IBASE+2,JBASE+2,-NZAG+2)+CZP(IBASE+2,JBASE+2,-NZAG+2)*        &
              & T(IBASE+2,JBASE+2,-NZAG+2+1))+T(IBASE+2,JBASE+2,-NZAG+2)+       &
              & (3.d0-2.d0*F)*CONST(IBASE+2,JBASE+2,-NZAG+2)*(TDBAV/DX(IBASE+2)/    &
              & (REXT+RSID+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-(0.88d0*SIGMA*            &
              & (TEXT(IBASE+2,JBASE+2,-NZAG+2)+273.15d0)**4)/DX(IBASE+2))+       &
              & F*CONST(IBASE+2,JBASE+2,-NZAG+2)*(TDBAV/DY(JBASE+2)/            &
              & (REXT+RSID+1.d0/HOAV)+RSOLVAV/DY(JBASE+2)-(0.88d0*SIGMA*            &
              & (TEXT(IBASE+2,JBASE+2,-NZAG+2)+273.15d0)**4)/DY(JBASE+2))

              N=IBASE+2-0+1
              L=1
              DO COUNT1=0,IBASE+2
                AA(L)=A(COUNT1,JBASE+2,-NZAG+2)
                BB(L)=B(COUNT1,JBASE+2,-NZAG+2)
                CC(L)=C(COUNT1,JBASE+2,-NZAG+2)
                RR(L)=R(COUNT1,JBASE+2,-NZAG+2)
                L=L+1
              END DO
              CALL TRIDI3D (AA,BB,CC,RR,N,X)
              L=0
              DO COUNT1=0,IBASE+2
                U(COUNT1,JBASE+2,-NZAG+2)=X(L+1)
                L=L+1
                QEXT(COUNT1,JBASE+2,-NZAG+2)=(TDBAV-U(COUNT1,JBASE+2,-NZAG+2))/       &
                & (REXT+RSID+1.d0/HOAV)
                UEXT(COUNT1,JBASE+2,-NZAG+2)=QEXT(COUNT1,JBASE+2,-NZAG+2)*            &
                & (REXT+RSID)+U(COUNT1,JBASE+2,-NZAG+2)
              END DO

!***  INSIDE WALL CELLS
              IF (NZAG.GT.3) THEN
                DO COUNT3=-NZAG+3,-1
                  A(0,JBASE,COUNT3)=0.
                  B(0,JBASE,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(0,JBASE,COUNT3)*            &
                  & CXP(0,JBASE,COUNT3)
                  C(0,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(0,JBASE,COUNT3)*               &
                  & (-CXP(0,JBASE,COUNT3))
                  R(0,JBASE,COUNT3)=F*CONST(0,JBASE,COUNT3)*(-(1.d0/DY(JBASE)/       &
                  & (RINT+1.d0/HIN(6))+CYP(0,JBASE,COUNT3))*T(0,JBASE,COUNT3)+       &
                  & CYP(0,JBASE,COUNT3)*T(0,JBASE+1,COUNT3)+CZM(0,JBASE,COUNT3)*   &
                  & T(0,JBASE,COUNT3-1)-(CZM(0,JBASE,COUNT3)+CZP(0,JBASE,COUNT3))* &
                  & T(0,JBASE,COUNT3)+CZP(0,JBASE,COUNT3)*T(0,JBASE,COUNT3+1))+    &
                  & T(0,JBASE,COUNT3)+F*CONST(0,JBASE,COUNT3)*          &
                  & (TBAV/DY(JBASE)/(RINT+1.d0/HIN(6)))
                  DO COUNT1=1,IBASE-1
                    A(COUNT1,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)*   &
                    & (-CXM(COUNT1,JBASE,COUNT3))
                    B(COUNT1,JBASE,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)* &
                    & (CXM(COUNT1,JBASE,COUNT3)+CXP(COUNT1,JBASE,COUNT3))
                    C(COUNT1,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)*   &
                    & (-CXP(COUNT1,JBASE,COUNT3))
                    R(COUNT1,JBASE,COUNT3)=F*CONST(COUNT1,JBASE,COUNT3)*(-(1.d0/DY   &
                    & (JBASE)/(RINT+1.d0/HIN(6))+CYP(COUNT1,JBASE,COUNT3))*          &
                    & T(COUNT1,JBASE,COUNT3)+CYP(COUNT1,JBASE,COUNT3)*             &
                    & T(COUNT1,JBASE+1,COUNT3)+CZM(COUNT1,JBASE,COUNT3)*           &
                    & T(COUNT1,JBASE,COUNT3-1)-(CZM(COUNT1,JBASE,COUNT3)+          &
                    & CZP(COUNT1,JBASE,COUNT3))*T(COUNT1,JBASE,COUNT3)+            &
                    & CZP(COUNT1,JBASE,COUNT3)*T(COUNT1,JBASE,COUNT3+1))+          &
                    & T(COUNT1,JBASE,COUNT3)+F*CONST(COUNT1,JBASE,COUNT3)*         &
                    & (TBAV/DY(JBASE)/(RINT+1.d0/HIN(6)))
                  END DO

!***  INSIDE CORNER WALL CELL AND CENTER WALL CELL
                  DO COUNT1=IBASE,IBASE+1
                    A(COUNT1,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)*     &
                    & (-CXM(COUNT1,JBASE,COUNT3))
                    B(COUNT1,JBASE,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)*  &
                    &(CXM(COUNT1,JBASE,COUNT3)+CXP(COUNT1,JBASE,COUNT3))
                    C(COUNT1,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)*     &
                    & (-CXP(COUNT1,JBASE,COUNT3))
                    R(COUNT1,JBASE,COUNT3)=F*CONST(COUNT1,JBASE,COUNT3)*             &
                    & (CYM(COUNT1,JBASE,COUNT3)*T(COUNT1,JBASE-1,COUNT3)-            &
                    & (CYM(COUNT1,JBASE,COUNT3)+CYP(COUNT1,JBASE,COUNT3))*           &
                    & T(COUNT1,JBASE,COUNT3)+CYP(COUNT1,JBASE,COUNT3)*               &
                    & T(COUNT1,JBASE+1,COUNT3)+CZM(COUNT1,JBASE,COUNT3)*             &
                    & T(COUNT1,JBASE,COUNT3-1)-(CZM(COUNT1,JBASE,COUNT3)+            &
                    & CZP(COUNT1,JBASE,COUNT3))*T(COUNT1,JBASE,COUNT3)+              &
                    & CZP(COUNT1,JBASE,COUNT3)*T(COUNT1,JBASE,COUNT3+1))+            &
                    & T(COUNT1,JBASE,COUNT3)
                  END DO

!***  OUTSIDE WALL CELL
                  A(IBASE+2,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE+2,JBASE,COUNT3)*     &
                  & (-CXM(IBASE+2,JBASE,COUNT3))
                  B(IBASE+2,JBASE,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(IBASE+2,JBASE,COUNT3)*  &
                  & (CXM(IBASE+2,JBASE,COUNT3)+1.d0/DX(IBASE+2)/(REXT+1.d0/HOAV))
                  C(IBASE+2,JBASE,COUNT3)=0.
                  R(IBASE+2,JBASE,COUNT3)=F*CONST(IBASE+2,JBASE,COUNT3)*             &
                  & (CYM(IBASE+2,JBASE,COUNT3)*T(IBASE+2,JBASE-1,COUNT3)-            &
                  & (CYM(IBASE+2,JBASE,COUNT3)+CYP(IBASE+2,JBASE,COUNT3))*           &
                  & T(IBASE+2,JBASE,COUNT3)+CYP(IBASE+2,JBASE,COUNT3)*               &
                  & T(IBASE+2,JBASE+1,COUNT3)+CZM(IBASE+2,JBASE,COUNT3)*             &
                  & T(IBASE+2,JBASE,COUNT3-1)-(CZM(IBASE+2,JBASE,COUNT3)+            &
                  & CZP(IBASE+2,JBASE,COUNT3))*T(IBASE+2,JBASE,COUNT3)+              &
                  & CZP(IBASE+2,JBASE,COUNT3)*T(IBASE+2,JBASE,COUNT3+1))+            &
                  & T(IBASE+2,JBASE,COUNT3)+(3.d0-2.d0*F)*CONST(IBASE+2,JBASE,COUNT3)*   &
                  & (TDBAV/DX(IBASE+2)/(REXT+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-           &
                  & (0.88d0*SIGMA*(TEXT(IBASE+2,JBASE,COUNT3)+273.15d0)**4)/DX(IBASE+2))
                  N=IBASE+2-0+1
                  L=1
                  DO COUNT1=0,IBASE+2
                    AA(L)=A(COUNT1,JBASE,COUNT3)
                    BB(L)=B(COUNT1,JBASE,COUNT3)
                    CC(L)=C(COUNT1,JBASE,COUNT3)
                    RR(L)=R(COUNT1,JBASE,COUNT3)
                    L=L+1
                  END DO
                  CALL TRIDI3D (AA,BB,CC,RR,N,X)
                  L=0
                  DO COUNT1=0,IBASE+2
                    U(COUNT1,JBASE,COUNT3)=X(L+1)
                    L=L+1
                  END DO
                  QEXT(IBASE+2,JBASE,COUNT3)=(TDBAV-U(IBASE+2,JBASE,COUNT3))/        &
                  & (REXT+RSID+1.d0/HOAV)
                  UEXT(IBASE+2,JBASE,COUNT3)=QEXT(IBASE+2,JBASE,COUNT3)*             &
                  & (REXT+RSID)+U(IBASE+2,JBASE,COUNT3)

!***  CENTER WALL CELLS
                  A(0,JBASE+1,COUNT3)=0.
                  B(0,JBASE+1,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(0,JBASE+1,COUNT3)*          &
                  & CXP(0,JBASE+1,COUNT3)
                  C(0,JBASE+1,COUNT3)=(3.d0-2.d0*F)*CONST(0,JBASE+1,COUNT3)*             &
                  & (-CXP(0,JBASE+1,COUNT3))
                  R(0,JBASE+1,COUNT3)=F*CONST(0,JBASE+1,COUNT3)*                     &
                  & (CYM(0,JBASE+1,COUNT3)*T(0,JBASE+1-1,COUNT3)-                    &
                  & (CYM(0,JBASE+1,COUNT3)+CYP(0,JBASE+1,COUNT3))*                   &
                  & T(0,JBASE+1,COUNT3)+CYP(0,JBASE+1,COUNT3)*T(0,JBASE+1+1,COUNT3)+ &
                  & CZM(0,JBASE+1,COUNT3)*T(0,JBASE+1,COUNT3-1)-                     &
                  & (CZM(0,JBASE+1,COUNT3)+CZP(0,JBASE+1,COUNT3))*                   &
                  & T(0,JBASE+1,COUNT3)+CZP(0,JBASE+1,COUNT3)*                       &
                  & T(0,JBASE+1,COUNT3+1))+T(0,JBASE+1,COUNT3)
                  DO COUNT1=1,IBASE+1
                    A(COUNT1,JBASE+1,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+1,COUNT3)* &
                    & (-CXM(COUNT1,JBASE+1,COUNT3))
                    B(COUNT1,JBASE+1,COUNT3)=1.d0+(3.d0-2.d0*F)*                           &
                    & CONST(COUNT1,JBASE+1,COUNT3)*(CXM(COUNT1,JBASE+1,COUNT3)+      &
                    & CXP(COUNT1,JBASE+1,COUNT3))
                    C(COUNT1,JBASE+1,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+1,COUNT3)* &
                    & (-CXP(COUNT1,JBASE+1,COUNT3))
                    R(COUNT1,JBASE+1,COUNT3)=F*CONST(COUNT1,JBASE+1,COUNT3)*         &
                    & (CYM(COUNT1,JBASE+1,COUNT3)*T(COUNT1,JBASE+1-1,COUNT3)-        &
                    & (CYM(COUNT1,JBASE+1,COUNT3)+CYP(COUNT1,JBASE+1,COUNT3))*       &
                    & T(COUNT1,JBASE+1,COUNT3)+CYP(COUNT1,JBASE+1,COUNT3)*           &
                    & T(COUNT1,JBASE+1+1,COUNT3)+CZM(COUNT1,JBASE+1,COUNT3)*         &
                    & T(COUNT1,JBASE+1,COUNT3-1)-(CZM(COUNT1,JBASE+1,COUNT3)+        &
                    & CZP(COUNT1,JBASE+1,COUNT3))*T(COUNT1,JBASE+1,COUNT3)+          &
                    & CZP(COUNT1,JBASE+1,COUNT3)*T(COUNT1,JBASE+1,COUNT3+1))+        &
                    & T(COUNT1,JBASE+1,COUNT3)
                  END DO

!***  OUTSIDE WALL CELL
                  A(IBASE+2,JBASE+1,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE+2,JBASE+1,COUNT3)* &
                  & (-CXM(IBASE+2,JBASE+1,COUNT3))
                  B(IBASE+2,JBASE+1,COUNT3)=1.d0+(3.d0-2.d0*F)*                            &
                  & CONST(IBASE+2,JBASE+1,COUNT3)*(CXM(IBASE+2,JBASE+1,COUNT3)+1.d0/   &
                  & DX(IBASE+2)/(REXT+1.d0/HOAV))
                  C(IBASE+2,JBASE+1,COUNT3)=0.
                  R(IBASE+2,JBASE+1,COUNT3)=F*CONST(IBASE+2,JBASE+1,COUNT3)*         &
                  & (CYM(IBASE+2,JBASE+1,COUNT3)*T(IBASE+2,JBASE+1-1,COUNT3)-        &
                  & (CYM(IBASE+2,JBASE+1,COUNT3)+CYP(IBASE+2,JBASE+1,COUNT3))*       &
                  & T(IBASE+2,JBASE+1,COUNT3)+CYP(IBASE+2,JBASE+1,COUNT3)*           &
                  & T(IBASE+2,JBASE+1+1,COUNT3)+CZM(IBASE+2,JBASE+1,COUNT3)*         &
                  & T(IBASE+2,JBASE+1,COUNT3-1)-(CZM(IBASE+2,JBASE+1,COUNT3)+        &
                  & CZP(IBASE+2,JBASE+1,COUNT3))*T(IBASE+2,JBASE+1,COUNT3)+          &
                  & CZP(IBASE+2,JBASE+1,COUNT3)*T(IBASE+2,JBASE+1,COUNT3+1))+        &
                  & T(IBASE+2,JBASE+1,COUNT3)+(3.d0-2.d0*F)*                             &
                  & CONST(IBASE+2,JBASE+1,COUNT3)*                                   &
                  & (TDBAV/DX(IBASE+2)/(REXT+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-           &
                  & (0.88d0*SIGMA*(TEXT(IBASE+2,JBASE+1,COUNT3)+273.15d0)**4)/          &
                  & DX(IBASE+2))

                  N=IBASE+2-0+1
                  L=1
                  DO COUNT1=0,IBASE+2
                    AA(L)=A(COUNT1,JBASE+1,COUNT3)
                    BB(L)=B(COUNT1,JBASE+1,COUNT3)
                    CC(L)=C(COUNT1,JBASE+1,COUNT3)
                    RR(L)=R(COUNT1,JBASE+1,COUNT3)
                    L=L+1
                  END DO
                  CALL TRIDI3D (AA,BB,CC,RR,N,X)
                  L=0
                  DO COUNT1=0,IBASE+2
                    U(COUNT1,JBASE+1,COUNT3)=X(L+1)
                    L=L+1
                  END DO
                  QEXT(IBASE+2,JBASE+1,COUNT3)=(TDBAV-U(IBASE+2,JBASE+1,COUNT3))/    &
                  & (REXT+RSID+1.d0/HOAV)
                  UEXT(IBASE+2,JBASE+1,COUNT3)=QEXT(IBASE+2,JBASE+1,COUNT3)*         &
                  & (REXT+RSID)+U(IBASE+2,JBASE+1,COUNT3)

!***  OUTSIDE WALL CELLS
                  A(0,JBASE+2,COUNT3)=0.
                  B(0,JBASE+2,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(0,JBASE+2,COUNT3)*          &
                  & CXP(0,JBASE+2,COUNT3)
                  C(0,JBASE+2,COUNT3)=(3.d0-2.d0*F)*CONST(0,JBASE+2,COUNT3)*             &
                  & (-CXP(0,JBASE+2,COUNT3))
                  R(0,JBASE+2,COUNT3)=F*CONST(0,JBASE+2,COUNT3)*                     &
                  & (CYM(0,JBASE+2,COUNT3)*T(0,JBASE+2-1,COUNT3)-                    &
                  & (CYM(0,JBASE+2,COUNT3)+1.d0/DY(JBASE+2)/(REXT+1.d0/HOAV))*           &
                  & T(0,JBASE+2,COUNT3)+CZM(0,JBASE+2,COUNT3)*                       &
                  & T(0,JBASE+2,COUNT3-1)-(CZM(0,JBASE+2,COUNT3)+                    &
                  & CZP(0,JBASE+2,COUNT3))*T(0,JBASE+2,COUNT3)+                      &
                  & CZP(0,JBASE+2,COUNT3)*T(0,JBASE+2,COUNT3+1))+                    &
                  & T(0,JBASE+2,COUNT3)+F*CONST(0,JBASE+2,COUNT3)*                   &
                  & (TDBAV/DY(JBASE+2)/(REXT+1.d0/HOAV)+RSOLVAV/DY(JBASE+2)-           &
                  & (0.88d0*SIGMA*(TEXT(0,JBASE+2,COUNT3)+273.15d0)**4)/DY(JBASE+2))

                  DO COUNT1=1,IBASE+1
                    A(COUNT1,JBASE+2,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,COUNT3)* &
                    & (-CXM(COUNT1,JBASE+2,COUNT3))
                    B(COUNT1,JBASE+2,COUNT3)=1.d0+(3.d0-2.d0*F)*                           &
                    & CONST(COUNT1,JBASE+2,COUNT3)*(CXM(COUNT1,JBASE+2,COUNT3)+      &
                    & CXP(COUNT1,JBASE+2,COUNT3))
                    C(COUNT1,JBASE+2,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,COUNT3)* &
                    & (-CXP(COUNT1,JBASE+2,COUNT3))
                    R(COUNT1,JBASE+2,COUNT3)=F*CONST(COUNT1,JBASE+2,COUNT3)*         &
                    & (CYM(COUNT1,JBASE+2,COUNT3)*T(COUNT1,JBASE+2-1,COUNT3)-        &
                    & (CYM(COUNT1,JBASE+2,COUNT3)+1.d0/DY(JBASE+2)/                    &
                    & (REXT+1.d0/HOAV))*T(COUNT1,JBASE+2,COUNT3)+                      &
                    & CZM(COUNT1,JBASE+2,COUNT3)*T(COUNT1,JBASE+2,COUNT3-1)-         &
                    & (CZM(COUNT1,JBASE+2,COUNT3)+CZP(COUNT1,JBASE+2,COUNT3))*       &
                    & T(COUNT1,JBASE+2,COUNT3)+CZP(COUNT1,JBASE+2,COUNT3)*           &
                    & T(COUNT1,JBASE+2,COUNT3+1))+T(COUNT1,JBASE+2,COUNT3)+F*        &
                    & CONST(COUNT1,JBASE+2,COUNT3)*(TDBAV/DY(JBASE+2)/               &
                    & (REXT+1.d0/HOAV)+RSOLVAV/DY(JBASE+2)-(0.88d0*SIGMA*                &
                    & (TEXT(COUNT1,JBASE+2,COUNT3)+273.15d0)**4)/DY(JBASE+2))
                  END DO

!***  OUTER CORNER CELL
                  A(IBASE+2,JBASE+2,COUNT3)=(3.d0-2.d0*F)*                               &
                  & CONST(IBASE+2,JBASE+2,COUNT3)*(-CXM(IBASE+2,JBASE+2,COUNT3))
                  B(IBASE+2,JBASE+2,COUNT3)=1.d0+(3.d0-2.d0*F)*                            &
                  & CONST(IBASE+2,JBASE+2,COUNT3)*(CXM(IBASE+2,JBASE+2,COUNT3)+      &
                  & 1.d0/DX(IBASE+2)/(REXT+1.d0/HOAV))
                  C(IBASE+2,JBASE+2,COUNT3)=0.
                  R(IBASE+2,JBASE+2,COUNT3)=F*CONST(IBASE+2,JBASE+2,COUNT3)*         &
                  & (CYM(IBASE+2,JBASE+2,COUNT3)*T(IBASE+2,JBASE+2-1,COUNT3)-        &
                  & (CYM(IBASE+2,JBASE+2,COUNT3)+1.d0/DY(JBASE+2)/(REXT+1.d0/HOAV))*     &
                  & T(IBASE+2,JBASE+2,COUNT3)+CZM(IBASE+2,JBASE+2,COUNT3)*           &
                  & T(IBASE+2,JBASE+2,COUNT3-1)-(CZM(IBASE+2,JBASE+2,COUNT3)+        &
                  & CZP(IBASE+2,JBASE+2,COUNT3))*T(IBASE+2,JBASE+2,COUNT3)+          &
                  & CZP(IBASE+2,JBASE+2,COUNT3)*T(IBASE+2,JBASE+2,COUNT3+1))+        &
                  & T(IBASE+2,JBASE+2,COUNT3)+(3.d0-2.d0*F)*CONST(IBASE+2,JBASE+2,COUNT3)* &
                  & (TDBAV/DX(IBASE+2)/(REXT+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-           &
                  & (0.88d0*SIGMA*(TEXT(IBASE+2,JBASE+2,COUNT3)+273.15d0)**4)/          &
                  & DX(IBASE+2))+F*CONST(IBASE+2,JBASE+2,COUNT3)*(TDBAV/DY(JBASE+2)/ &
                  & (REXT+1.d0/HOAV)+RSOLVAV/DY(JBASE+2)-(0.88d0*SIGMA*                  &
                  & (TEXT(IBASE+2,JBASE+2,COUNT3)+273.15d0)**4)/DY(JBASE+2))

                  N=IBASE+2-0+1
                  L=1
                  DO COUNT1=0,IBASE+2
                    AA(L)=A(COUNT1,JBASE+2,COUNT3)
                    BB(L)=B(COUNT1,JBASE+2,COUNT3)
                    CC(L)=C(COUNT1,JBASE+2,COUNT3)
                    RR(L)=R(COUNT1,JBASE+2,COUNT3)
                    L=L+1
                  END DO
                  CALL TRIDI3D (AA,BB,CC,RR,N,X)
                  L=0
                  DO COUNT1=0,IBASE+2
                    U(COUNT1,JBASE+2,COUNT3)=X(L+1)
                    L=L+1
                    QEXT(COUNT1,JBASE+2,COUNT3)=(TDBAV-U(COUNT1,JBASE+2,COUNT3))/                         &
                    & (REXT+RSID+1.d0/HOAV)
                    UEXT(COUNT1,JBASE+2,COUNT3)=QEXT(COUNT1,JBASE+2,COUNT3)*                              &
                    & (REXT+RSID)+U(COUNT1,JBASE+2,COUNT3)
                  END DO
                END DO
              END IF

!***  SECTION 3:  RIM JOIST, PARALLEL TO X-AXIS
              A(0,JBASE+2,-NZAG+1)=0.
              B(0,JBASE+2,-NZAG+1)=1.d0+(3.d0-2.d0*F)*CONST(0,JBASE+2,-NZAG+1)* &
              & CXP(0,JBASE+2,-NZAG+1)
              C(0,JBASE+2,-NZAG+1)=(3.d0-2.d0*F)*CONST(0,JBASE+2,-NZAG+1)*          &
              & (-CXP(0,JBASE+2,-NZAG+1))
              R(0,JBASE+2,-NZAG+1)=F*CONST(0,JBASE+2,-NZAG+1)*                  &
              & (-(1.d0/DY(JBASE+2)/(RSILL+1.d0/HIN(6)+DY(JBASE+2)/                 &
              & TCON(MTYPE(0,JBASE+2,-NZAG+1)))+1.d0/DY(JBASE+2)/                 &
              & (REXT+RSID+1.d0/HOAV))*T(0,JBASE+2,-NZAG+1)+                      &
              & CZM(0,JBASE+2,-NZAG+1)*T(0,JBASE+2,-NZAG+1-1)-                  &
              & (CZM(0,JBASE+2,-NZAG+1)+CZP(0,JBASE+2,-NZAG+1))*                &
              & T(0,JBASE+2,-NZAG+1)+CZP(0,JBASE+2,-NZAG+1)*                    &
              & T(0,JBASE+2,-NZAG+1+1))+T(0,JBASE+2,-NZAG+1)+                   &
              & F*CONST(0,JBASE+2,-NZAG+1)*(TBAV/DY(JBASE+2)/                   &
              & (RSILL+1.d0/HIN(6)+DY(JBASE+2)/                                   &
              & TCON(MTYPE(0,JBASE+2,-NZAG+1)))+TDBAV/DY(JBASE+2)/              &
              & (REXT+RSID+1.d0/HOAV)+RSOLVAV/DY(JBASE+2)-(0.88d0*SIGMA*            &
              & (TEXT(0,JBASE+2,-NZAG+1)+273.15d0)**4)/DY(JBASE+2))
              DO COUNT1=1,IBASE+1
                A(COUNT1,JBASE+2,-NZAG+1)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,-NZAG+1)*   &
                    & (-CXM(COUNT1,JBASE+2,-NZAG+1))
                B(COUNT1,JBASE+2,-NZAG+1)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,-NZAG+1)* &
                    & (CXM(COUNT1,JBASE+2,-NZAG+1)+CXP(COUNT1,JBASE+2,-NZAG+1))
                C(COUNT1,JBASE+2,-NZAG+1)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,-NZAG+1)*   &
                    & (-CXP(COUNT1,JBASE+2,-NZAG+1))
                R(COUNT1,JBASE+2,-NZAG+1)=F*CONST(COUNT1,JBASE+2,-NZAG+1)*           &
                & (-(1.d0/DY(JBASE+2)/(RSILL+1.d0/HIN(6)+DY(JBASE+2)/                    &
                & TCON(MTYPE(COUNT1,JBASE+2,-NZAG+1)))+1.d0/DY(JBASE+2)/               &
                & (REXT+RSID+1.d0/HOAV))*T(COUNT1,JBASE+2,-NZAG+1)+                    &
                & CZM(COUNT1,JBASE+2,-NZAG+1)*T(COUNT1,JBASE+2,-NZAG+1-1)-           &
                & (CZM(COUNT1,JBASE+2,-NZAG+1)+CZP(COUNT1,JBASE+2,-NZAG+1))*         &
                & T(COUNT1,JBASE+2,-NZAG+1)+CZP(COUNT1,JBASE+2,-NZAG+1)*             &
                & T(COUNT1,JBASE+2,-NZAG+1+1))+T(COUNT1,JBASE+2,-NZAG+1)+            &
                & F*CONST(COUNT1,JBASE+2,-NZAG+1)*(TBAV/DY(JBASE+2)/                 &
                & (RSILL+1.d0/HIN(6)+DY(JBASE+2)/                                      &
                & TCON(MTYPE(COUNT1,JBASE+2,-NZAG+1)))+TDBAV/DY(JBASE+2)/            &
                & (REXT+RSID+1.d0/HOAV)+RSOLVAV/DY(JBASE+2)-(0.88d0*SIGMA*               &
                & (TEXT(COUNT1,JBASE+2,-NZAG+1)+273.15d0)**4)/DY(JBASE+2))
              END DO

!***  CORNER RIM JOIST CELL
              A(IBASE+2,JBASE+2,-NZAG+1)=(3.d0-2.d0*F)*                 &
              & CONST(IBASE+2,JBASE+2,-NZAG+1)*                     &
              & (-CXM(IBASE+2,JBASE+2,-NZAG+1))
              B(IBASE+2,JBASE+2,-NZAG+1)=1.d0+(3.d0-2.d0*F)*              &
              & CONST(IBASE+2,JBASE+2,-NZAG+1)*                     &
              & (CXM(IBASE+2,JBASE+2,-NZAG+1)+1.d0/DX(IBASE+2)/       &
              & (REXT+RSID+1.d0/HOAV))
              C(IBASE+2,JBASE+2,-NZAG+1)=0.
              R(IBASE+2,JBASE+2,-NZAG+1)=F*CONST(IBASE+2,JBASE+2,-NZAG+1)*      &
              & (CYM(IBASE+2,JBASE+2,-NZAG+1)*T(IBASE+2,JBASE+2-1,-NZAG+1)-     &
              & (CYM(IBASE+2,JBASE+2,-NZAG+1)+1.d0/DY(JBASE+2)/                   &
              & (REXT+RSID+1.d0/HOAV))*T(IBASE+2,JBASE+2,-NZAG+1)+                &
              & CZM(IBASE+2,JBASE+2,-NZAG+1)*T(IBASE+2,JBASE+2,-NZAG+1-1)-      &
              & (CZM(IBASE+2,JBASE+2,-NZAG+1)+CZP(IBASE+2,JBASE+2,-NZAG+1))*    &
              & T(IBASE+2,JBASE+2,-NZAG+1)+CZP(IBASE+2,JBASE+2,-NZAG+1)*        &
              & T(IBASE+2,JBASE+2,-NZAG+1+1))+T(IBASE+2,JBASE+2,-NZAG+1)+       &
              & (3.d0-2.d0*F)*CONST(IBASE+2,JBASE+2,-NZAG+1)*(TDBAV/DX(IBASE+2)/    &
              & (REXT+RSID+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-(0.88d0*SIGMA*            &
              & (TEXT(IBASE+2,JBASE+2,-NZAG+1)+273.15d0)**4)/DX(IBASE+2))+       &
              & F*CONST(IBASE+2,JBASE+2,-NZAG+1)*(TDBAV/DY(JBASE+2)/            &
              & (REXT+RSID+1.d0/HOAV)+RSOLVAV/DY(JBASE+2)-(0.88d0*SIGMA*            &
              & (TEXT(IBASE+2,JBASE+2,-NZAG+1)+273.15d0)**4)/DY(JBASE+2))
              N=IBASE+2-0+1
              L=1
              DO COUNT1=0,IBASE+2
                AA(L)=A(COUNT1,JBASE+2,-NZAG+1)
                BB(L)=B(COUNT1,JBASE+2,-NZAG+1)
                CC(L)=C(COUNT1,JBASE+2,-NZAG+1)
                RR(L)=R(COUNT1,JBASE+2,-NZAG+1)
                L=L+1
              END DO
              CALL TRIDI3D (AA,BB,CC,RR,N,X)
              L=0
              DO COUNT1=0,IBASE+2
                U(COUNT1,JBASE+2,-NZAG+1)=X(L+1)
                L=L+1
                QEXT(COUNT1,JBASE+2,-NZAG+1)=(TDBAV-U(COUNT1,JBASE+2,-NZAG+1))/ &
                      & (REXT+RSID+1.d0/HOAV)
                UEXT(COUNT1,JBASE+2,-NZAG+1)=QEXT(COUNT1,JBASE+2,-NZAG+1)*      &
                      & (REXT+RSID)+U(COUNT1,JBASE+2,-NZAG+1)
              END DO

!***  SECTION 4:  ABOVE-GRADE FOUNDATION WALL, PARALLEL TO Y-AXIS
!***  TOP INSIDE WALL CELL
              DO COUNT2=0,JBASE-1
                IF (TBAV.GT.T(IBASE,COUNT2,-NZAG+2)) THEN
                  HINZ=HIN(4)
                ELSE
                  HINZ=HIN(5)
                END IF
                A(IBASE,COUNT2,-NZAG+2)=0.
                B(IBASE,COUNT2,-NZAG+2)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,COUNT2,-NZAG+2)* &
                      & (1.d0/DX(IBASE)/(RINT+1.d0/HIN(6))+CXP(IBASE,COUNT2,-NZAG+2))
                C(IBASE,COUNT2,-NZAG+2)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,-NZAG+2)*    &
                      & (-CXP(IBASE,COUNT2,-NZAG+2))
                R(IBASE,COUNT2,-NZAG+2)=F*CONST(IBASE,COUNT2,-NZAG+2)*               &
                & (CYM(IBASE,COUNT2,-NZAG+2)*T(IBASE,COUNT2-1,-NZAG+2)-              &
                & (CYM(IBASE,COUNT2,-NZAG+2)+CYP(IBASE,COUNT2,-NZAG+2))*             &
                & T(IBASE,COUNT2,-NZAG+2)+CYP(IBASE,COUNT2,-NZAG+2)*                 &
                & T(IBASE,COUNT2+1,-NZAG+2)-(1.d0/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+          &
                & DZ(-NZAG+2)/2.d0/TCON(MTYPE(IBASE,COUNT2,-NZAG+2)))+                 &
                & CZP(IBASE,COUNT2,-NZAG+2))*T(IBASE,COUNT2,-NZAG+2)+                &
                & CZP(IBASE,COUNT2,-NZAG+2)*T(IBASE,COUNT2,-NZAG+2+1))+              &
                & T(IBASE,COUNT2,-NZAG+2)+(3.d0-2.d0*F)*CONST(IBASE,COUNT2,-NZAG+2)*     &
                & (TBAV/DX(IBASE)/(RINT+1.d0/HIN(6)))+F*CONST(IBASE,COUNT2,-NZAG+2)*   &
                & (TBAV/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+DZ(-NZAG+2)/2.d0/                   &
                & TCON(MTYPE(IBASE,COUNT2,-NZAG+2))))

!***  TOP CENTER WALL CELL
                IF (TBAV.GT.T(IBASE+1,COUNT2,-NZAG+2)) THEN
                  HINZ=HIN(4)
                ELSE
                  HINZ=HIN(5)
                END IF
                A(IBASE+1,COUNT2,-NZAG+2)=(3.d0-2.d0*F)*CONST(IBASE+1,COUNT2,-NZAG+2)*   &
                      & (-CXM(IBASE+1,COUNT2,-NZAG+2))
                B(IBASE+1,COUNT2,-NZAG+2)=1.d0+(3.d0-2.d0*F)*CONST(IBASE+1,COUNT2,-NZAG+2)* &
                      & (CXM(IBASE+1,COUNT2,-NZAG+2)+CXP(IBASE+1,COUNT2,-NZAG+2))
                C(IBASE+1,COUNT2,-NZAG+2)=(3.d0-2.d0*F)*CONST(IBASE+1,COUNT2,-NZAG+2)*   &
                      & (-CXP(IBASE+1,COUNT2,-NZAG+2))
                R(IBASE+1,COUNT2,-NZAG+2)=F*CONST(IBASE+1,COUNT2,-NZAG+2)*           &
                & (CYM(IBASE+1,COUNT2,-NZAG+2)*T(IBASE+1,COUNT2-1,-NZAG+2)-          &
                & (CYM(IBASE+1,COUNT2,-NZAG+2)+CYP(IBASE+1,COUNT2,-NZAG+2))*         &
                & T(IBASE+1,COUNT2,-NZAG+2)+CYP(IBASE+1,COUNT2,-NZAG+2)*             &
                & T(IBASE+1,COUNT2+1,-NZAG+2)-(1.d0/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+        &
                & DZ(-NZAG+2)/2.d0/TCON(MTYPE(IBASE+1,COUNT2,-NZAG+2)))+               &
                & CZP(IBASE+1,COUNT2,-NZAG+2))*T(IBASE+1,COUNT2,-NZAG+2)+            &
                & CZP(IBASE+1,COUNT2,-NZAG+2)*T(IBASE+1,COUNT2,-NZAG+2+1))+          &
                & T(IBASE+1,COUNT2,-NZAG+2)+F*CONST(IBASE+1,COUNT2,-NZAG+2)*         &
                & (TBAV/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+DZ(-NZAG+2)/2.d0/                   &
                & TCON(MTYPE(IBASE+1,COUNT2,-NZAG+2))))

!***  TOP OUTSIDE WALL CELL
                A(IBASE+2,COUNT2,-NZAG+2)=(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,-NZAG+2)*   &
                      & (-CXM(IBASE+2,COUNT2,-NZAG+2))
                B(IBASE+2,COUNT2,-NZAG+2)=1.d0+(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,-NZAG+2)* &
                    & (CXM(IBASE+2,COUNT2,-NZAG+2)+1.d0/DX(IBASE+2)/(REXT+RSID+1.d0/HOAV))
                C(IBASE+2,COUNT2,-NZAG+2)=0.
                R(IBASE+2,COUNT2,-NZAG+2)=F*CONST(IBASE+2,COUNT2,-NZAG+2)*           &
                & (CYM(IBASE+2,COUNT2,-NZAG+2)*T(IBASE+2,COUNT2-1,-NZAG+2)-          &
                & (CYM(IBASE+2,COUNT2,-NZAG+2)+CYP(IBASE+2,COUNT2,-NZAG+2))*         &
                & T(IBASE+2,COUNT2,-NZAG+2)+CYP(IBASE+2,COUNT2,-NZAG+2)*             &
                & T(IBASE+2,COUNT2+1,-NZAG+2)+CZM(IBASE+2,COUNT2,-NZAG+2)*           &
                & T(IBASE+2,COUNT2,-NZAG+2-1)-(CZM(IBASE+2,COUNT2,-NZAG+2)+          &
                & CZP(IBASE+2,COUNT2,-NZAG+2))*T(IBASE+2,COUNT2,-NZAG+2)+            &
                & CZP(IBASE+2,COUNT2,-NZAG+2)*T(IBASE+2,COUNT2,-NZAG+2+1))+          &
                & T(IBASE+2,COUNT2,-NZAG+2)+(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,-NZAG+2)* &
                & (TDBAV/DX(IBASE+2)/(REXT+RSID+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-        &
                & (0.88d0*SIGMA*(TEXT(IBASE+2,COUNT2,-NZAG+2)+273.15d0)**4)/            &
                & DX(IBASE+2))

                N=IBASE+2-IBASE+1
                L=1
                DO COUNT1=IBASE,IBASE+2
                  AA(L)=A(COUNT1,COUNT2,-NZAG+2)
                  BB(L)=B(COUNT1,COUNT2,-NZAG+2)
                  CC(L)=C(COUNT1,COUNT2,-NZAG+2)
                  RR(L)=R(COUNT1,COUNT2,-NZAG+2)
                  L=L+1
                END DO
                CALL TRIDI3D (AA,BB,CC,RR,N,X)
                L=0
                DO COUNT1=IBASE,IBASE+2
                  U(COUNT1,COUNT2,-NZAG+2)=X(L+1)
                  L=L+1
                END DO
                QEXT(IBASE+2,COUNT2,-NZAG+2)=(TDBAV-U(IBASE+2,COUNT2,-NZAG+2))/      &
                & (REXT+RSID+1.d0/HOAV)
                UEXT(IBASE+2,COUNT2,-NZAG+2)=QEXT(IBASE+2,COUNT2,-NZAG+2)*           &
                & (REXT+RSID)+U(IBASE+2,COUNT2,-NZAG+2)
              END DO

!***  INSIDE WALL CELLS
              IF (NZAG.GT.3) THEN
                DO COUNT3=-NZAG+3,-1
                  DO COUNT2=0,JBASE-1
                    A(IBASE,COUNT2,COUNT3)=0.
                    B(IBASE,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)*  &
                          & (1.d0/DX(IBASE)/(RINT+1.d0/HIN(6))+CXP(IBASE,COUNT2,COUNT3))
                    C(IBASE,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)*     &
                          & (-CXP(IBASE,COUNT2,COUNT3))
                    R(IBASE,COUNT2,COUNT3)=F*CONST(IBASE,COUNT2,COUNT3)*             &
                    & (CYM(IBASE,COUNT2,COUNT3)*T(IBASE,COUNT2-1,COUNT3)-            &
                    & (CYM(IBASE,COUNT2,COUNT3)+CYP(IBASE,COUNT2,COUNT3))*           &
                    & T(IBASE,COUNT2,COUNT3)+CYP(IBASE,COUNT2,COUNT3)*               &
                    & T(IBASE,COUNT2+1,COUNT3)+CZM(IBASE,COUNT2,COUNT3)*             &
                    & T(IBASE,COUNT2,COUNT3-1)-(CZM(IBASE,COUNT2,COUNT3)+            &
                    & CZP(IBASE,COUNT2,COUNT3))*T(IBASE,COUNT2,COUNT3)+              &
                    & CZP(IBASE,COUNT2,COUNT3)*T(IBASE,COUNT2,COUNT3+1))+            &
                    & T(IBASE,COUNT2,COUNT3)+(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)*   &
                    & (TBAV/DX(IBASE)/(RINT+1.d0/HIN(6)))

!***  CENTER WALL CELL
                    A(IBASE+1,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE+1,COUNT2,COUNT3)*   &
                         & (-CXM(IBASE+1,COUNT2,COUNT3))
                    B(IBASE+1,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(IBASE+1,COUNT2,COUNT3)* &
                         & (CXM(IBASE+1,COUNT2,COUNT3)+CXP(IBASE+1,COUNT2,COUNT3))
                    C(IBASE+1,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE+1,COUNT2,COUNT3)*   &
                         & (-CXP(IBASE+1,COUNT2,COUNT3))
                    R(IBASE+1,COUNT2,COUNT3)=F*CONST(IBASE+1,COUNT2,COUNT3)*           &
                    & (CYM(IBASE+1,COUNT2,COUNT3)*T(IBASE+1,COUNT2-1,COUNT3)-          &
                    & (CYM(IBASE+1,COUNT2,COUNT3)+CYP(IBASE+1,COUNT2,COUNT3))*         &
                    & T(IBASE+1,COUNT2,COUNT3)+CYP(IBASE+1,COUNT2,COUNT3)*             &
                    & T(IBASE+1,COUNT2+1,COUNT3)+CZM(IBASE+1,COUNT2,COUNT3)*           &
                    & T(IBASE+1,COUNT2,COUNT3-1)-(CZM(IBASE+1,COUNT2,COUNT3)+          &
                    & CZP(IBASE+1,COUNT2,COUNT3))*T(IBASE+1,COUNT2,COUNT3)+            &
                    & CZP(IBASE+1,COUNT2,COUNT3)*T(IBASE+1,COUNT2,COUNT3+1))+          &
                    & T(IBASE+1,COUNT2,COUNT3)

!***  OUTSIDE WALL CELL
                    A(IBASE+2,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,COUNT3)*   &
                            & (-CXM(IBASE+2,COUNT2,COUNT3))
                    B(IBASE+2,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,COUNT3)* &
                            & (CXM(IBASE+2,COUNT2,COUNT3)+1.d0/DX(IBASE+2)/(REXT+1.d0/HOAV))
                    C(IBASE+2,COUNT2,COUNT3)=0.
                    R(IBASE+2,COUNT2,COUNT3)=F*CONST(IBASE+2,COUNT2,COUNT3)*           &
                    & (CYM(IBASE+2,COUNT2,COUNT3)*T(IBASE+2,COUNT2-1,COUNT3)-          &
                    & (CYM(IBASE+2,COUNT2,COUNT3)+CYP(IBASE+2,COUNT2,COUNT3))*         &
                    & T(IBASE+2,COUNT2,COUNT3)+CYP(IBASE+2,COUNT2,COUNT3)*             &
                    & T(IBASE+2,COUNT2+1,COUNT3)+CZM(IBASE+2,COUNT2,COUNT3)*           &
                    & T(IBASE+2,COUNT2,COUNT3-1)-(CZM(IBASE+2,COUNT2,COUNT3)+          &
                    & CZP(IBASE+2,COUNT2,COUNT3))*T(IBASE+2,COUNT2,COUNT3)+            &
                    & CZP(IBASE+2,COUNT2,COUNT3)*T(IBASE+2,COUNT2,COUNT3+1))+          &
                    & T(IBASE+2,COUNT2,COUNT3)+(3.d0-2.d0*F)*                              &
                    & CONST(IBASE+2,COUNT2,COUNT3)*(TDBAV/DX(IBASE+2)/(REXT+1.d0/HOAV)+  &
                    & RSOLVAV/DX(IBASE+2)-(0.88d0*SIGMA*                                 &
                    & (TEXT(IBASE+2,COUNT2,COUNT3)+273.15d0)**4)/DX(IBASE+2))

                    N=IBASE+2-IBASE+1
                    L=1
                    DO COUNT1=IBASE,IBASE+2
                      AA(L)=A(COUNT1,COUNT2,COUNT3)
                      BB(L)=B(COUNT1,COUNT2,COUNT3)
                      CC(L)=C(COUNT1,COUNT2,COUNT3)
                      RR(L)=R(COUNT1,COUNT2,COUNT3)
                      L=L+1
                    END DO
                    CALL TRIDI3D (AA,BB,CC,RR,N,X)
                    L=0
                    DO COUNT1=IBASE,IBASE+2
                      U(COUNT1,COUNT2,COUNT3)=X(L+1)
                      L=L+1
                    END DO
                    QEXT(IBASE+2,COUNT2,COUNT3)=(TDBAV-U(IBASE+2,COUNT2,COUNT3))/      &
                         & (REXT+RSID+1.d0/HOAV)
                    UEXT(IBASE+2,COUNT2,COUNT3)=QEXT(IBASE+2,COUNT2,COUNT3)*           &
                         & (REXT+RSID)+U(IBASE+2,COUNT2,COUNT3)
                  END DO
                END DO
              END IF

!***  SECTION 5:  RIM JOIST, PARALLEL TO Y-AXIS
              DO COUNT2=0,JBASE+1
                A(IBASE+2,COUNT2,-NZAG+1)=0.
                B(IBASE+2,COUNT2,-NZAG+1)=1.d0+(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,-NZAG+1)* &
                & (1.d0/DX(IBASE+2)/(RSILL+1.d0/HIN(6)+DX(IBASE+2)/                     &
                & TCON(MTYPE(IBASE+2,COUNT2,-NZAG+1)))+1.d0/DX(IBASE+2)/              &
                & (REXT+RSID+1.d0/HOAV))
                C(IBASE+2,COUNT2,-NZAG+1)=0.
                R(IBASE+2,COUNT2,-NZAG+1)=F*CONST(IBASE+2,COUNT2,-NZAG+1)*          &
                & (CYM(IBASE+2,COUNT2,-NZAG+1)*T(IBASE+2,COUNT2-1,-NZAG+1)-         &
                & (CYM(IBASE+2,COUNT2,-NZAG+1)+CYP(IBASE+2,COUNT2,-NZAG+1))*        &
                & T(IBASE+2,COUNT2,-NZAG+1)+CYP(IBASE+2,COUNT2,-NZAG+1)*            &
                & T(IBASE+2,COUNT2+1,-NZAG+1)+CZM(IBASE+2,COUNT2,-NZAG+1)*          &
                & T(IBASE+2,COUNT2,-NZAG+1-1)-(CZM(IBASE+2,COUNT2,-NZAG+1)+         &
                & CZP(IBASE+2,COUNT2,-NZAG+1))*T(IBASE+2,COUNT2,-NZAG+1)+           &
                & CZP(IBASE+2,COUNT2,-NZAG+1)*T(IBASE+2,COUNT2,-NZAG+1+1))+         &
                & T(IBASE+2,COUNT2,-NZAG+1)+(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,-NZAG+1)* &
                & (TBAV/DX(IBASE+2)/(RSILL+1.d0/HIN(6)+DX(IBASE+2)/                   &
                & TCON(MTYPE(IBASE+2,COUNT2,-NZAG+1)))+TDBAV/DX(IBASE+2)/           &
                & (REXT+RSID+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-(0.88d0*SIGMA*              &
                & (TEXT(IBASE+2,COUNT2,-NZAG+1)+273.15d0)**4)/DX(IBASE+2))
                N=1
                L=1
                AA(L)=A(IBASE+2,COUNT2,-NZAG+1)
                BB(L)=B(IBASE+2,COUNT2,-NZAG+1)
                CC(L)=C(IBASE+2,COUNT2,-NZAG+1)
                RR(L)=R(IBASE+2,COUNT2,-NZAG+1)
                CALL TRIDI3D (AA,BB,CC,RR,N,X)
                L=0
                U(IBASE+2,COUNT2,-NZAG+1)=X(L+1)
                QEXT(IBASE+2,COUNT2,-NZAG+1)=(TDBAV-U(IBASE+2,COUNT2,-NZAG+1))/       &
                      & (REXT+RSID+1.d0/HOAV)
                UEXT(IBASE+2,COUNT2,-NZAG+1)=QEXT(IBASE+2,COUNT2,-NZAG+1)*            &
                      & (REXT+RSID)+U(IBASE+2,COUNT2,-NZAG+1)
              END DO

!***  SECTION 6:  BELOW-GRADE (1)
!***  INSIDE WALL CELLS, PARALLEL TO X-AXIS (UPPER BAND) COUNT3=0
              A(0,JBASE,0)=0.
              B(0,JBASE,0)=1.d0+(3.d0-2.d0*F)*CONST(0,JBASE,0)*CXP(0,JBASE,0)
              C(0,JBASE,0)=(3.d0-2.d0*F)*CONST(0,JBASE,0)*(-CXP(0,JBASE,0))
              R(0,JBASE,0)=F*CONST(0,JBASE,0)*(-(1.d0/DY(JBASE)/(RINT+            &
              & 1.d0/HIN(6))+CYP(0,JBASE,0))*T(0,JBASE,0)+CYP(0,JBASE,0)*         &
              & T(0,JBASE+1,0)+CZM(0,JBASE,0)*T(0,JBASE,0-1)-                   &
              & (CZM(0,JBASE,0)+CZP(0,JBASE,0))*T(0,JBASE,0)+CZP(0,JBASE,0)*    &
              & T(0,JBASE,0+1))+T(0,JBASE,0)+F*CONST(0,JBASE,0)*                &
              & (TBAV/DY(JBASE)/(RINT+1.d0/HIN(6)))

              DO COUNT1=1,IBASE-1
                A(COUNT1,JBASE,0)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,0)*              &
                    & (-CXM(COUNT1,JBASE,0))
                B(COUNT1,JBASE,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,0)*           &
                    & (CXM(COUNT1,JBASE,0)+CXP(COUNT1,JBASE,0))
                C(COUNT1,JBASE,0)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,0)*              &
                    & (-CXP(COUNT1,JBASE,0))
                R(COUNT1,JBASE,0)=F*CONST(COUNT1,JBASE,0)*(-(1.d0/DY(JBASE)/      &
                & (RINT+1.d0/HIN(6))+CYP(COUNT1,JBASE,0))*T(COUNT1,JBASE,0)+      &
                & CYP(COUNT1,JBASE,0)*T(COUNT1,JBASE+1,0)+CZM(COUNT1,JBASE,0)*  &
                & T(COUNT1,JBASE,0-1)-(CZM(COUNT1,JBASE,0)+CZP(COUNT1,JBASE,0))* &
                & T(COUNT1,JBASE,0)+CZP(COUNT1,JBASE,0)*T(COUNT1,JBASE,0+1))+   &
                & T(COUNT1,JBASE,0)+F*CONST(COUNT1,JBASE,0)*                    &
                & (TBAV/DY(JBASE)/(RINT+1.d0/HIN(6)))
              END DO

!***  FOUNDATION WALL CELLS COUNT3=0
              DO COUNT1=IBASE,IBASE+2
                A(COUNT1,JBASE,0)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,0)*              &
                      & (-CXM(COUNT1,JBASE,0))
                B(COUNT1,JBASE,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,0)*           &
                      & (CXM(COUNT1,JBASE,0)+CXP(COUNT1,JBASE,0))
                C(COUNT1,JBASE,0)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,0)*              &
                      & (-CXP(COUNT1,JBASE,0))
                R(COUNT1,JBASE,0)=F*CONST(COUNT1,JBASE,0)*(CYM(COUNT1,JBASE,0)* &
                & T(COUNT1,JBASE-1,0)-(CYM(COUNT1,JBASE,0)+CYP(COUNT1,JBASE,0))* &
                & T(COUNT1,JBASE,0)+CYP(COUNT1,JBASE,0)*                        &
                & T(COUNT1,JBASE+1,0)+CZM(COUNT1,JBASE,0)*T(COUNT1,JBASE,0-1)-  &
                & (CZM(COUNT1,JBASE,0)+CZP(COUNT1,JBASE,0))*T(COUNT1,JBASE,0)+  &
                & CZP(COUNT1,JBASE,0)*T(COUNT1,JBASE,0+1))+T(COUNT1,JBASE,0)
              END DO

!***  GROUND SURFACE CELLS COUNT3=0
              DO COUNT1=IBASE+3,NXM1-1
                IF (ISNW.EQ.1) THEN
                  A(COUNT1,JBASE,0)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,0)*              &
                      & (-CXM(COUNT1,JBASE,0))
                  B(COUNT1,JBASE,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,0)*           &
                      &   (CXM(COUNT1,JBASE,0)+CXP(COUNT1,JBASE,0))
                  C(COUNT1,JBASE,0)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,0)*              &
                      & (-CXP(COUNT1,JBASE,0))
                  R(COUNT1,JBASE,0)=F*CONST(COUNT1,JBASE,0)*(CYM(COUNT1,JBASE,0)* &
                      & T(COUNT1,JBASE-1,0)-(CYM(COUNT1,JBASE,0)+                 &
                      & CYP(COUNT1,JBASE,0))*T(COUNT1,JBASE,0)+                   &
                      & CYP(COUNT1,JBASE,0)*T(COUNT1,JBASE+1,0)-                  &
                      & CZP(COUNT1,JBASE,0)*T(COUNT1,JBASE,0)+CZP(COUNT1,JBASE,0)* &
                      & T(COUNT1,JBASE,0+1))+T(COUNT1,JBASE,0)+F*                 &
                      & CONST(COUNT1,JBASE,0)*(GOFTAV(COUNT1,JBASE)/DZ(0))
                ELSE
                  A(COUNT1,JBASE,0)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,0)*              &
                      & (-CXM(COUNT1,JBASE,0))
                  B(COUNT1,JBASE,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,0)*           &
                      & (CXM(COUNT1,JBASE,0)+CXP(COUNT1,JBASE,0))
                  C(COUNT1,JBASE,0)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,0)*              &
                      & (-CXP(COUNT1,JBASE,0))
                  R(COUNT1,JBASE,0)=F*CONST(COUNT1,JBASE,0)*(CYM(COUNT1,JBASE,0)* &
                  & T(COUNT1,JBASE-1,0)-(CYM(COUNT1,JBASE,0)+CYP(COUNT1,JBASE,0))* &
                  & T(COUNT1,JBASE,0)+CYP(COUNT1,JBASE,0)*T(COUNT1,JBASE+1,0)-    &
                  & CZP(COUNT1,JBASE,0)*T(COUNT1,JBASE,0)+CZP(COUNT1,JBASE,0)*    &
                  & T(COUNT1,JBASE,0+1))+T(COUNT1,JBASE,0)+F*CONST(COUNT1,JBASE,0)* &
                  & (GOFTAV(COUNT1,JBASE)/DZ(0)+TDBAV/DZ(0)/RSNWAV)
                END IF
              END DO
              IF (ISNW.EQ.1) THEN
                A(NXM1,JBASE,0)=(3.d0-2.d0*F)*CONST(NXM1,JBASE,0)*              &
                & (-CXM(NXM1,JBASE,0))
                B(NXM1,JBASE,0)=1.d0+(3.d0-2.d0*F)*CONST(NXM1,JBASE,0)*           &
                & (CXM(NXM1,JBASE,0)+CXP(NXM1,JBASE,0))
                C(NXM1,JBASE,0)=0.
                R(NXM1,JBASE,0)=F*CONST(NXM1,JBASE,0)*(CYM(NXM1,JBASE,0)*   &
                & T(NXM1,JBASE-1,0)-(CYM(NXM1,JBASE,0)+CYP(NXM1,JBASE,0))*  &
                & T(NXM1,JBASE,0)+CYP(NXM1,JBASE,0)*T(NXM1,JBASE+1,0)-      &
                & CZP(NXM1,JBASE,0)*T(NXM1,JBASE,0)+CZP(NXM1,JBASE,0)*      &
                & T(NXM1,JBASE,0+1))+T(NXM1,JBASE,0)+F*CONST(NXM1,JBASE,0)* &
                & (GOFTAV(NXM1,JBASE)/DZ(0))+(3.d0-2.d0*F)*CONST(NXM1,JBASE,0)* &
                & CXP(NXM1,JBASE,0)*TGAV(0)
              ELSE
                A(NXM1,JBASE,0)=(3.d0-2.d0*F)*CONST(NXM1,JBASE,0)*              &
                & (-CXM(NXM1,JBASE,0))
                B(NXM1,JBASE,0)=1.d0+(3.d0-2.d0*F)*CONST(NXM1,JBASE,0)*           &
                & (CXM(NXM1,JBASE,0)+CXP(NXM1,JBASE,0))
                C(NXM1,JBASE,0)=0.
                R(NXM1,JBASE,0)=F*CONST(NXM1,JBASE,0)*(CYM(NXM1,JBASE,0)*   &
                & T(NXM1,JBASE-1,0)-(CYM(NXM1,JBASE,0)+CYP(NXM1,JBASE,0))*  &
                & T(NXM1,JBASE,0)+CYP(NXM1,JBASE,0)*T(NXM1,JBASE+1,0)-      &
                & CZP(NXM1,JBASE,0)*T(NXM1,JBASE,0)+CZP(NXM1,JBASE,0)*      &
                & T(NXM1,JBASE,0+1))+T(NXM1,JBASE,0)+F*CONST(NXM1,JBASE,0)* &
                & (GOFTAV(NXM1,JBASE)/DZ(0)+TDBAV/DZ(0)/RSNWAV)+(3.d0-2.d0*F)*  &
                & CONST(NXM1,JBASE,0)*CXP(NXM1,JBASE,0)*TGAV(0)
              END IF
              N=NXM1-0+1
              L=1
              DO COUNT1=0,NXM1
                AA(L)=A(COUNT1,JBASE,0)
                BB(L)=B(COUNT1,JBASE,0)
                CC(L)=C(COUNT1,JBASE,0)
                RR(L)=R(COUNT1,JBASE,0)
                L=L+1
              END DO
              CALL TRIDI3D (AA,BB,CC,RR,N,X)
              L=0
              DO COUNT1=0,NXM1
                U(COUNT1,JBASE,0)=X(L+1)
                L=L+1
              END DO

!***  FOUNDATION WALL CELLS COUNT3=0
              DO COUNT2=JBASE+1,JBASE+2
                A(0,COUNT2,0)=0.
                B(0,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(0,COUNT2,0)*CXP(0,COUNT2,0)
                C(0,COUNT2,0)=(3.d0-2.d0*F)*CONST(0,COUNT2,0)*(-CXP(0,COUNT2,0))
                R(0,COUNT2,0)=F*CONST(0,COUNT2,0)*(CYM(0,COUNT2,0)*         &
                & T(0,COUNT2-1,0)-(CYM(0,COUNT2,0)+CYP(0,COUNT2,0))*        &
                & T(0,COUNT2,0)+CYP(0,COUNT2,0)*T(0,COUNT2+1,0)+            &
                & CZM(0,COUNT2,0)*T(0,COUNT2,0-1)-(CZM(0,COUNT2,0)+         &
                & CZP(0,COUNT2,0))*T(0,COUNT2,0)+CZP(0,COUNT2,0)*           &
                & T(0,COUNT2,0+1))+T(0,COUNT2,0)
                DO COUNT1=1,IBASE+2
                  A(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*      &
                      & (-CXM(COUNT1,COUNT2,0))
                  B(COUNT1,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*   &
                      & (CXM(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0))
                  C(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*      &
                      & (-CXP(COUNT1,COUNT2,0))
                  R(COUNT1,COUNT2,0)=F*CONST(COUNT1,COUNT2,0)*              &
                  & (CYM(COUNT1,COUNT2,0)*T(COUNT1,COUNT2-1,0)-             &
                  & (CYM(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0))*            &
                  & T(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0)*                &
                  & T(COUNT1,COUNT2+1,0)+CZM(COUNT1,COUNT2,0)*              &
                  & T(COUNT1,COUNT2,0-1)-(CZM(COUNT1,COUNT2,0)+             &
                  & CZP(COUNT1,COUNT2,0))*T(COUNT1,COUNT2,0)+               &
                  & CZP(COUNT1,COUNT2,0)*T(COUNT1,COUNT2,0+1))+             &
                  & T(COUNT1,COUNT2,0)
                END DO

!***  GROUND SURFACE CELLS COUNT3=0
                DO COUNT1=IBASE+3,NXM1-1
                  IF (ISNW.EQ.1) THEN
                    A(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*    &
                      & (-CXM(COUNT1,COUNT2,0))
                    B(COUNT1,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)* &
                      & (CXM(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0))
                    C(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*    &
                      & (-CXP(COUNT1,COUNT2,0))
                    R(COUNT1,COUNT2,0)=F*CONST(COUNT1,COUNT2,0)*            &
                    & (CYM(COUNT1,COUNT2,0)*T(COUNT1,COUNT2-1,0)-           &
                    & (CYM(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0))*          &
                    & T(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0)*              &
                    & T(COUNT1,COUNT2+1,0)-CZP(COUNT1,COUNT2,0)*            &
                    & T(COUNT1,COUNT2,0)+CZP(COUNT1,COUNT2,0)*              &
                    & T(COUNT1,COUNT2,0+1))+T(COUNT1,COUNT2,0)+             &
                    & F*CONST(COUNT1,COUNT2,0)*(GOFTAV(COUNT1,COUNT2)/DZ(0))
                  ELSE
                    A(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*    &
                      & (-CXM(COUNT1,COUNT2,0))
                    B(COUNT1,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)* &
                      & (CXM(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0))
                    C(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*    &
                      & (-CXP(COUNT1,COUNT2,0))
                    R(COUNT1,COUNT2,0)=F*CONST(COUNT1,COUNT2,0)*            &
                    & (CYM(COUNT1,COUNT2,0)*T(COUNT1,COUNT2-1,0)-           &
                    & (CYM(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0))*          &
                    & T(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0)*              &
                    & T(COUNT1,COUNT2+1,0)-CZP(COUNT1,COUNT2,0)*            &
                    & T(COUNT1,COUNT2,0)+CZP(COUNT1,COUNT2,0)*              &
                    & T(COUNT1,COUNT2,0+1))+T(COUNT1,COUNT2,0)+             &
                    & F*CONST(COUNT1,COUNT2,0)*(GOFTAV(COUNT1,COUNT2)/      &
                    & DZ(0)+TDBAV/DZ(0)/RSNWAV)
                  END IF
                END DO
                IF (ISNW.EQ.1) THEN
                  A(NXM1,COUNT2,0)=(3.d0-2.d0*F)*CONST(NXM1,COUNT2,0)*          &
                      & (-CXM(NXM1,COUNT2,0))
                  B(NXM1,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(NXM1,COUNT2,0)*       &
                      & (CXM(NXM1,COUNT2,0)+CXP(NXM1,COUNT2,0))
                  C(NXM1,COUNT2,0)=0.
                  R(NXM1,COUNT2,0)=F*CONST(NXM1,COUNT2,0)*                  &
                  & (CYM(NXM1,COUNT2,0)*T(NXM1,COUNT2-1,0)-                 &
                  & (CYM(NXM1,COUNT2,0)+CYP(NXM1,COUNT2,0))*T(NXM1,COUNT2,0)+ &
                  & CYP(NXM1,COUNT2,0)*T(NXM1,COUNT2+1,0)-CZP(NXM1,COUNT2,0)* &
                  & T(NXM1,COUNT2,0)+CZP(NXM1,COUNT2,0)*T(NXM1,COUNT2,0+1))+ &
                  & T(NXM1,COUNT2,0)+F*CONST(NXM1,COUNT2,0)*                &
                  & (GOFTAV(NXM1,COUNT2)/DZ(0))+(3.d0-2.d0*F)*                  &
                  & CONST(NXM1,COUNT2,0)*CXP(NXM1,COUNT2,0)*TGAV(0)
                ELSE
                  A(NXM1,COUNT2,0)=(3.d0-2.d0*F)*CONST(NXM1,COUNT2,0)*          &
                      & (-CXM(NXM1,COUNT2,0))
                  B(NXM1,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(NXM1,COUNT2,0)*       &
                      & (CXM(NXM1,COUNT2,0)+CXP(NXM1,COUNT2,0))
                  C(NXM1,COUNT2,0)=0.
                  R(NXM1,COUNT2,0)=F*CONST(NXM1,COUNT2,0)*                     &
                  & (CYM(NXM1,COUNT2,0)*T(NXM1,COUNT2-1,0)-(CYM(NXM1,COUNT2,0) &
                  & +CYP(NXM1,COUNT2,0))*T(NXM1,COUNT2,0)+CYP(NXM1,COUNT2,0)*  &
                  & T(NXM1,COUNT2+1,0)-CZP(NXM1,COUNT2,0)*T(NXM1,COUNT2,0)+    &
                  & CZP(NXM1,COUNT2,0)*T(NXM1,COUNT2,0+1))+T(NXM1,COUNT2,0)+   &
                  & F*CONST(NXM1,COUNT2,0)*(GOFTAV(NXM1,COUNT2)/DZ(0)+         &
                  & TDBAV/DZ(0)/RSNWAV)+(3.d0-2.d0*F)*CONST(NXM1,COUNT2,0)*        &
                  & CXP(NXM1,COUNT2,0)*TGAV(0)
                END IF
                N=NXM1-0+1
                L=1
                DO COUNT1=0,NXM1
                  AA(L)=A(COUNT1,COUNT2,0)
                  BB(L)=B(COUNT1,COUNT2,0)
                  CC(L)=C(COUNT1,COUNT2,0)
                  RR(L)=R(COUNT1,COUNT2,0)
                  L=L+1
                END DO
                CALL TRIDI3D (AA,BB,CC,RR,N,X)
                L=0
                DO COUNT1=0,NXM1
                  U(COUNT1,COUNT2,0)=X(L+1)
                  L=L+1
                END DO
              END DO

!***  GROUND SURFACE CELLS COUNT3=0
              DO COUNT2=JBASE+3,NYM1
                IF (ISNW.EQ.1) THEN
                  A(0,COUNT2,0)=0.
                  B(0,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(0,COUNT2,0)*CXP(0,COUNT2,0)
                  C(0,COUNT2,0)=(3.d0-2.d0*F)*CONST(0,COUNT2,0)*(-CXP(0,COUNT2,0))
                  R(0,COUNT2,0)=F*CONST(0,COUNT2,0)*(CYM(0,COUNT2,0)*             &
                  & T(0,COUNT2-1,0)-(CYM(0,COUNT2,0)+CYP(0,COUNT2,0))*            &
                  & T(0,COUNT2,0)+CYP(0,COUNT2,0)*T(0,COUNT2+1,0)-CZP(0,COUNT2,0)* &
                  & T(0,COUNT2,0)+CZP(0,COUNT2,0)*T(0,COUNT2,0+1))+T(0,COUNT2,0)+ &
                  & F*CONST(0,COUNT2,0)*(GOFTAV(0,COUNT2)/DZ(0))
                ELSE
                  A(0,COUNT2,0)=0.
                  B(0,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(0,COUNT2,0)*CXP(0,COUNT2,0)
                  C(0,COUNT2,0)=(3.d0-2.d0*F)*CONST(0,COUNT2,0)*(-CXP(0,COUNT2,0))
                  R(0,COUNT2,0)=F*CONST(0,COUNT2,0)*(CYM(0,COUNT2,0)*             &
                  & T(0,COUNT2-1,0)-(CYM(0,COUNT2,0)+CYP(0,COUNT2,0))*            &
                  & T(0,COUNT2,0)+CYP(0,COUNT2,0)*T(0,COUNT2+1,0)-                &
                  & CZP(0,COUNT2,0)*T(0,COUNT2,0)+CZP(0,COUNT2,0)*                &
                  & T(0,COUNT2,0+1))+T(0,COUNT2,0)+F*CONST(0,COUNT2,0)*           &
                  & (GOFTAV(0,COUNT2)/DZ(0)+TDBAV/DZ(0)/RSNWAV)
                END IF
                DO COUNT1=1,NXM1-1
                  IF (ISNW.EQ.1) THEN
                    A(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*          &
                      & (-CXM(COUNT1,COUNT2,0))
                    B(COUNT1,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*       &
                      & (CXM(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0))
                    C(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*          &
                      & (-CXP(COUNT1,COUNT2,0))
                    R(COUNT1,COUNT2,0)=F*CONST(COUNT1,COUNT2,0)*                  &
                    & (CYM(COUNT1,COUNT2,0)*T(COUNT1,COUNT2-1,0)-                 &
                    & (CYM(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0))*                &
                    & T(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0)*                    &
                    & T(COUNT1,COUNT2+1,0)-CZP(COUNT1,COUNT2,0)*                  &
                    & T(COUNT1,COUNT2,0)+CZP(COUNT1,COUNT2,0)*                    &
                    & T(COUNT1,COUNT2,0+1))+T(COUNT1,COUNT2,0)+                   &
                    & F*CONST(COUNT1,COUNT2,0)*(GOFTAV(COUNT1,COUNT2)/DZ(0))
                  ELSE
                    A(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*          &
                      & (-CXM(COUNT1,COUNT2,0))
                    B(COUNT1,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*       &
                      & (CXM(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0))
                    C(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*          &
                      & (-CXP(COUNT1,COUNT2,0))
                    R(COUNT1,COUNT2,0)=F*CONST(COUNT1,COUNT2,0)*                  &
                    & (CYM(COUNT1,COUNT2,0)*T(COUNT1,COUNT2-1,0)-                 &
                    & (CYM(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0))*                &
                    & T(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0)*T(COUNT1,COUNT2+1,0)- &
                    & CZP(COUNT1,COUNT2,0)*T(COUNT1,COUNT2,0)+CZP(COUNT1,COUNT2,0)* &
                    & T(COUNT1,COUNT2,0+1))+T(COUNT1,COUNT2,0)+F*                 &
                    & CONST(COUNT1,COUNT2,0)*(GOFTAV(COUNT1,COUNT2)/DZ(0)+        &
                    & TDBAV/DZ(0)/RSNWAV)
                  END IF
                END DO
                IF (ISNW.EQ.1) THEN
                  A(NXM1,COUNT2,0)=(3.d0-2.d0*F)*CONST(NXM1,COUNT2,0)*                &
                      & (-CXM(NXM1,COUNT2,0))
                  B(NXM1,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(NXM1,COUNT2,0)*             &
                      & (CXM(NXM1,COUNT2,0)+CXP(NXM1,COUNT2,0))
                  C(NXM1,COUNT2,0)=0.
                  R(NXM1,COUNT2,0)=F*CONST(NXM1,COUNT2,0)*(CYM(NXM1,COUNT2,0)*    &
                  & T(NXM1,COUNT2-1,0)-(CYM(NXM1,COUNT2,0)+CYP(NXM1,COUNT2,0))*   &
                  & T(NXM1,COUNT2,0)+CYP(NXM1,COUNT2,0)*T(NXM1,COUNT2+1,0)-       &
                  & CZP(NXM1,COUNT2,0)*T(NXM1,COUNT2,0)+CZP(NXM1,COUNT2,0)*       &
                  & T(NXM1,COUNT2,0+1))+T(NXM1,COUNT2,0)+F*CONST(NXM1,COUNT2,0)*  &
                  & (GOFTAV(NXM1,COUNT2)/DZ(0))+(3.d0-2.d0*F)*CONST(NXM1,COUNT2,0)*   &
                  & CXP(NXM1,COUNT2,0)*TGAV(0)
                ELSE
                  A(NXM1,COUNT2,0)=(3.d0-2.d0*F)*CONST(NXM1,COUNT2,0)*                &
                      & (-CXM(NXM1,COUNT2,0))
                  B(NXM1,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(NXM1,COUNT2,0)*             &
                      & (CXM(NXM1,COUNT2,0)+CXP(NXM1,COUNT2,0))
                  C(NXM1,COUNT2,0)=0.
                  R(NXM1,COUNT2,0)=F*CONST(NXM1,COUNT2,0)*(CYM(NXM1,COUNT2,0)*    &
                  & T(NXM1,COUNT2-1,0)-(CYM(NXM1,COUNT2,0)+CYP(NXM1,COUNT2,0))*   &
                  & T(NXM1,COUNT2,0)+CYP(NXM1,COUNT2,0)*T(NXM1,COUNT2+1,0)-       &
                  & CZP(NXM1,COUNT2,0)*T(NXM1,COUNT2,0)+CZP(NXM1,COUNT2,0)*       &
                  & T(NXM1,COUNT2,0+1))+T(NXM1,COUNT2,0)+F*CONST(NXM1,COUNT2,0)*  &
                  & (GOFTAV(NXM1,COUNT2)/DZ(0)+TDBAV/DZ(0)/RSNWAV)+               &
                  & (3.d0-2.d0*F)*CONST(NXM1,COUNT2,0)*CXP(NXM1,COUNT2,0)*TGAV(0)
                END IF
                N=NXM1-0+1
                L=1
                DO COUNT1=0,NXM1
                  AA(L)=A(COUNT1,COUNT2,0)
                  BB(L)=B(COUNT1,COUNT2,0)
                  CC(L)=C(COUNT1,COUNT2,0)
                  RR(L)=R(COUNT1,COUNT2,0)
                  L=L+1
                END DO
                CALL TRIDI3D (AA,BB,CC,RR,N,X)
                L=0
                DO COUNT1=0,NXM1
                  U(COUNT1,COUNT2,0)=X(L+1)
                  L=L+1
                END DO
              END DO

!***  INSIDE WALL CELLS, PARALLEL TO X-AXIS (UPPER BAND)
              DO COUNT3=1,INT(1+(KBASE-NZAG)/2)
                A(0,JBASE,COUNT3)=0.
                B(0,JBASE,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(0,JBASE,COUNT3)*           &
                    & CXP(0,JBASE,COUNT3)
                C(0,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(0,JBASE,COUNT3)*              &
                    & (-CXP(0,JBASE,COUNT3))
                R(0,JBASE,COUNT3)=F*CONST(0,JBASE,COUNT3)*(-(1.d0/DY(JBASE)/(RINT+ &
                & 1.d0/HIN(6))+CYP(0,JBASE,COUNT3))*T(0,JBASE,COUNT3)+            &
                & CYP(0,JBASE,COUNT3)*T(0,JBASE+1,COUNT3)+CZM(0,JBASE,COUNT3)*  &
                & T(0,JBASE,COUNT3-1)-(CZM(0,JBASE,COUNT3)+CZP(0,JBASE,COUNT3))* &
                & T(0,JBASE,COUNT3)+CZP(0,JBASE,COUNT3)*T(0,JBASE,COUNT3+1))+   &
                & T(0,JBASE,COUNT3)+F*CONST(0,JBASE,COUNT3)*(TBAV/DY(JBASE)/    &
                & (RINT+1.d0/HIN(6)))
                DO COUNT1=1,IBASE-1
                  A(COUNT1,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)*  &
                    & (-CXM(COUNT1,JBASE,COUNT3))
                  B(COUNT1,JBASE,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3) &
                    & *(CXM(COUNT1,JBASE,COUNT3)+CXP(COUNT1,JBASE,COUNT3))
                  C(COUNT1,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)*  &
                    & (-CXP(COUNT1,JBASE,COUNT3))
                  R(COUNT1,JBASE,COUNT3)=F*CONST(COUNT1,JBASE,COUNT3)*          &
                  & (-(1.d0/DY(JBASE)/(RINT+1.d0/HIN(6))+CYP(COUNT1,JBASE,COUNT3))* &
                  & T(COUNT1,JBASE,COUNT3)+CYP(COUNT1,JBASE,COUNT3)*            &
                  & T(COUNT1,JBASE+1,COUNT3)+CZM(COUNT1,JBASE,COUNT3)*          &
                  & T(COUNT1,JBASE,COUNT3-1)-(CZM(COUNT1,JBASE,COUNT3)+         &
                  & CZP(COUNT1,JBASE,COUNT3))*T(COUNT1,JBASE,COUNT3)+           &
                  & CZP(COUNT1,JBASE,COUNT3)* T(COUNT1,JBASE,COUNT3+1))+        &
                  & T(COUNT1,JBASE,COUNT3)+F*CONST(COUNT1,JBASE,COUNT3)*        &
                  & (TBAV/DY(JBASE)/(RINT+1.d0/HIN(6)))
                END DO

!***  FOUNDATION WALL/GROUND
                DO COUNT1=IBASE,NXM1-1
                  A(COUNT1,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)*    &
                    & (-CXM(COUNT1,JBASE,COUNT3))
                  B(COUNT1,JBASE,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)* &
                    & (CXM(COUNT1,JBASE,COUNT3)+CXP(COUNT1,JBASE,COUNT3))
                  C(COUNT1,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)*    &
                    & (-CXP(COUNT1,JBASE,COUNT3))
                  R(COUNT1,JBASE,COUNT3)=F*CONST(COUNT1,JBASE,COUNT3)*            &
                  & (CYM(COUNT1,JBASE,COUNT3)*T(COUNT1,JBASE-1,COUNT3)-           &
                  & (CYM(COUNT1,JBASE,COUNT3)+CYP(COUNT1,JBASE,COUNT3))*          &
                  & T(COUNT1,JBASE,COUNT3)+CYP(COUNT1,JBASE,COUNT3)*              &
                  & T(COUNT1,JBASE+1,COUNT3)+CZM(COUNT1,JBASE,COUNT3)*            &
                  & T(COUNT1,JBASE,COUNT3-1)-(CZM(COUNT1,JBASE,COUNT3)+           &
                  & CZP(COUNT1,JBASE,COUNT3))*T(COUNT1,JBASE,COUNT3)+             &
                  & CZP(COUNT1,JBASE,COUNT3)*T(COUNT1,JBASE,COUNT3+1))+           &
                  & T(COUNT1,JBASE,COUNT3)
                END DO
                A(NXM1,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(NXM1,JBASE,COUNT3)*          &
                & (-CXM(NXM1,JBASE,COUNT3))
                B(NXM1,JBASE,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(NXM1,JBASE,COUNT3)*       &
                & (CXM(NXM1,JBASE,COUNT3)+CXP(NXM1,JBASE,COUNT3))
                C(NXM1,JBASE,COUNT3)=0.
                R(NXM1,JBASE,COUNT3)=F*CONST(NXM1,JBASE,COUNT3)*                  &
                & (CYM(NXM1,JBASE,COUNT3)*T(NXM1,JBASE-1,COUNT3)-                 &
                & (CYM(NXM1,JBASE,COUNT3)+CYP(NXM1,JBASE,COUNT3))*                &
                & T(NXM1,JBASE,COUNT3)+CYP(NXM1,JBASE,COUNT3)*                    &
                & T(NXM1,JBASE+1,COUNT3)+CZM(NXM1,JBASE,COUNT3)*                  &
                & T(NXM1,JBASE,COUNT3-1)-(CZM(NXM1,JBASE,COUNT3)+                 &
                & CZP(NXM1,JBASE,COUNT3))*T(NXM1,JBASE,COUNT3)+                   &
                & CZP(NXM1,JBASE,COUNT3)*T(NXM1,JBASE,COUNT3+1))+                 &
                & T(NXM1,JBASE,COUNT3)+(3.d0-2.d0*F)*CONST(NXM1,JBASE,COUNT3)*        &
                & CXP(NXM1,JBASE,COUNT3)*TGAV(COUNT3)
                N=NXM1-0+1
                L=1
                DO COUNT1=0,NXM1
                  AA(L)=A(COUNT1,JBASE,COUNT3)
                  BB(L)=B(COUNT1,JBASE,COUNT3)
                  CC(L)=C(COUNT1,JBASE,COUNT3)
                  RR(L)=R(COUNT1,JBASE,COUNT3)
                  L=L+1
                END DO
                CALL TRIDI3D (AA,BB,CC,RR,N,X)
                L=0
                DO COUNT1=0,NXM1
                  U(COUNT1,JBASE,COUNT3)=X(L+1)
                  L=L+1
                END DO
              END DO

!***  INSIDE WALL CELLS, PARALLEL TO X-AXIS (LOWER BAND)
              DO COUNT3=INT(2+(KBASE-NZAG)/2),KBASE-1
                A(0,JBASE,COUNT3)=0.
                B(0,JBASE,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(0,JBASE,COUNT3)*           &
                   & CXP(0,JBASE,COUNT3)
                C(0,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(0,JBASE,COUNT3)*              &
                   & (-CXP(0,JBASE,COUNT3))
                R(0,JBASE,COUNT3)=F*CONST(0,JBASE,COUNT3)*(-(1.d0/DY(JBASE)/      &
                & (RINT+1.d0/HIN(6))+CYP(0,JBASE,COUNT3))*T(0,JBASE,COUNT3)+      &
                & CYP(0,JBASE,COUNT3)*T(0,JBASE+1,COUNT3)+CZM(0,JBASE,COUNT3)*  &
                & T(0,JBASE,COUNT3-1)-(CZM(0,JBASE,COUNT3)+CZP(0,JBASE,COUNT3))* &
                & T(0,JBASE,COUNT3)+CZP(0,JBASE,COUNT3)*T(0,JBASE,COUNT3+1))+   &
                & T(0,JBASE,COUNT3)+F*CONST(0,JBASE,COUNT3)*                    &
                & (TBAV/DY(JBASE)/(RINT+1.d0/HIN(6)))
                DO COUNT1=1,IBASE-1
                  A(COUNT1,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)*  &
                     & (-CXM(COUNT1,JBASE,COUNT3))
                  B(COUNT1,JBASE,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)* &
                     & (CXM(COUNT1,JBASE,COUNT3)+CXP(COUNT1,JBASE,COUNT3))
                  C(COUNT1,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)*  &
                     & (-CXP(COUNT1,JBASE,COUNT3))
                  R(COUNT1,JBASE,COUNT3)=F*CONST(COUNT1,JBASE,COUNT3)*          &
                  & (-(1.d0/DY(JBASE)/(RINT+1.d0/HIN(6))+CYP(COUNT1,JBASE,COUNT3))* &
                  & T(COUNT1,JBASE,COUNT3)+CYP(COUNT1,JBASE,COUNT3)*            &
                  & T(COUNT1,JBASE+1,COUNT3)+CZM(COUNT1,JBASE,COUNT3)*          &
                  & T(COUNT1,JBASE,COUNT3-1)-(CZM(COUNT1,JBASE,COUNT3)+         &
                  & CZP(COUNT1,JBASE,COUNT3))*T(COUNT1,JBASE,COUNT3)+           &
                  & CZP(COUNT1,JBASE,COUNT3)*T(COUNT1,JBASE,COUNT3+1))+         &
                  & T(COUNT1,JBASE,COUNT3)+F*CONST(COUNT1,JBASE,COUNT3)*        &
                  & (TBAV/DY(JBASE)/(RINT+1.d0/HIN(6)))
                END DO

!***  FOUNDATION WALL/GRAVEL/GROUND
                DO COUNT1=IBASE,NXM1-1
                  A(COUNT1,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)*    &
                      & (-CXM(COUNT1,JBASE,COUNT3))
                  B(COUNT1,JBASE,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)* &
                      & (CXM(COUNT1,JBASE,COUNT3)+CXP(COUNT1,JBASE,COUNT3))
                  C(COUNT1,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)*    &
                      & (-CXP(COUNT1,JBASE,COUNT3))
                  R(COUNT1,JBASE,COUNT3)=F*CONST(COUNT1,JBASE,COUNT3)*            &
                  & (CYM(COUNT1,JBASE,COUNT3)*T(COUNT1,JBASE-1,COUNT3)-           &
                  & (CYM(COUNT1,JBASE,COUNT3)+CYP(COUNT1,JBASE,COUNT3))*          &
                  & T(COUNT1,JBASE,COUNT3)+CYP(COUNT1,JBASE,COUNT3)*              &
                  & T(COUNT1,JBASE+1,COUNT3)+CZM(COUNT1,JBASE,COUNT3)*            &
                  & T(COUNT1,JBASE,COUNT3-1)-(CZM(COUNT1,JBASE,COUNT3)+           &
                  & CZP(COUNT1,JBASE,COUNT3))*T(COUNT1,JBASE,COUNT3)+             &
                  & CZP(COUNT1,JBASE,COUNT3)*T(COUNT1,JBASE,COUNT3+1))+           &
                  & T(COUNT1,JBASE,COUNT3)
                END DO
                A(NXM1,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(NXM1,JBASE,COUNT3)*          &
                     & (-CXM(NXM1,JBASE,COUNT3))
                B(NXM1,JBASE,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(NXM1,JBASE,COUNT3)*       &
                     & (CXM(NXM1,JBASE,COUNT3)+CXP(NXM1,JBASE,COUNT3))
                C(NXM1,JBASE,COUNT3)=0.
                R(NXM1,JBASE,COUNT3)=F*CONST(NXM1,JBASE,COUNT3)*                  &
                & (CYM(NXM1,JBASE,COUNT3)*T(NXM1,JBASE-1,COUNT3)-                 &
                & (CYM(NXM1,JBASE,COUNT3)+CYP(NXM1,JBASE,COUNT3))*                &
                & T(NXM1,JBASE,COUNT3)+CYP(NXM1,JBASE,COUNT3)*                    &
                & T(NXM1,JBASE+1,COUNT3)+ CZM(NXM1,JBASE,COUNT3)*                 &
                & T(NXM1,JBASE,COUNT3-1)-(CZM(NXM1,JBASE,COUNT3)+                 &
                & CZP(NXM1,JBASE,COUNT3))*T(NXM1,JBASE,COUNT3)+                   &
                & CZP(NXM1,JBASE,COUNT3)*T(NXM1,JBASE,COUNT3+1))+                 &
                & T(NXM1,JBASE,COUNT3)+(3.d0-2.d0*F)*CONST(NXM1,JBASE,COUNT3)*        &
                & CXP(NXM1,JBASE,COUNT3)*TGAV(COUNT3)
                N=NXM1-0+1
                L=1
                DO COUNT1=0,NXM1
                  AA(L)=A(COUNT1,JBASE,COUNT3)
                  BB(L)=B(COUNT1,JBASE,COUNT3)
                  CC(L)=C(COUNT1,JBASE,COUNT3)
                  RR(L)=R(COUNT1,JBASE,COUNT3)
                  L=L+1
                END DO
                CALL TRIDI3D (AA,BB,CC,RR,N,X)
                L=0
                DO COUNT1=0,NXM1
                  U(COUNT1,JBASE,COUNT3)=X(L+1)
                  L=L+1
                END DO
              END DO

!***  FLOOR SLAB/GRAVEL/GROUND
              DO COUNT3=KBASE,NZBGM1
                A(0,JBASE,COUNT3)=0.
                B(0,JBASE,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(0,JBASE,COUNT3)*             &
                      & CXP(0,JBASE,COUNT3)
                C(0,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(0,JBASE,COUNT3)*                &
                      & (-CXP(0,JBASE,COUNT3))
                R(0,JBASE,COUNT3)=F*CONST(0,JBASE,COUNT3)*(CYM(0,JBASE,COUNT3)*   &
                & T(0,JBASE-1,COUNT3)-(CYM(0,JBASE,COUNT3)+CYP(0,JBASE,COUNT3))*  &
                & T(0,JBASE,COUNT3)+CYP(0,JBASE,COUNT3)*                          &
                & T(0,JBASE+1,COUNT3)+CZM(0,JBASE,COUNT3)*T(0,JBASE,COUNT3-1)-    &
                & (CZM(0,JBASE,COUNT3)+CZP(0,JBASE,COUNT3))*T(0,JBASE,COUNT3)+    &
                & CZP(0,JBASE,COUNT3)*T(0,JBASE,COUNT3+1))+T(0,JBASE,COUNT3)
                DO COUNT1=1,NXM1-1
                  A(COUNT1,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)*    &
                      &  (-CXM(COUNT1,JBASE,COUNT3))
                  B(COUNT1,JBASE,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)* &
                      & (CXM(COUNT1,JBASE,COUNT3)+CXP(COUNT1,JBASE,COUNT3))
                  C(COUNT1,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)*    &
                      & (-CXP(COUNT1,JBASE,COUNT3))
                  R(COUNT1,JBASE,COUNT3)=F*CONST(COUNT1,JBASE,COUNT3)*            &
                  & (CYM(COUNT1,JBASE,COUNT3)*T(COUNT1,JBASE-1,COUNT3)-           &
                  & (CYM(COUNT1,JBASE,COUNT3)+CYP(COUNT1,JBASE,COUNT3))*          &
                  & T(COUNT1,JBASE,COUNT3)+CYP(COUNT1,JBASE,COUNT3)*              &
                  & T(COUNT1,JBASE+1,COUNT3)+CZM(COUNT1,JBASE,COUNT3)*            &
                  & T(COUNT1,JBASE,COUNT3-1)-(CZM(COUNT1,JBASE,COUNT3)+           &
                  & CZP(COUNT1,JBASE,COUNT3))*T(COUNT1,JBASE,COUNT3)+             &
                  & CZP(COUNT1,JBASE,COUNT3)*T(COUNT1,JBASE,COUNT3+1))+           &
                  & T(COUNT1,JBASE,COUNT3)
                END DO
                A(NXM1,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(NXM1,JBASE,COUNT3)*          &
                      & (-CXM(NXM1,JBASE,COUNT3))
                B(NXM1,JBASE,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(NXM1,JBASE,COUNT3)*       &
                      & (CXM(NXM1,JBASE,COUNT3)+CXP(NXM1,JBASE,COUNT3))
                C(NXM1,JBASE,COUNT3)=0.
                R(NXM1,JBASE,COUNT3)=F*CONST(NXM1,JBASE,COUNT3)*                  &
                & (CYM(NXM1,JBASE,COUNT3)*T(NXM1,JBASE-1,COUNT3)-                 &
                & (CYM(NXM1,JBASE,COUNT3)+CYP(NXM1,JBASE,COUNT3))*                &
                & T(NXM1,JBASE,COUNT3)+CYP(NXM1,JBASE,COUNT3)*                    &
                & T(NXM1,JBASE+1,COUNT3)+CZM(NXM1,JBASE,COUNT3)*                  &
                & T(NXM1,JBASE,COUNT3-1)-(CZM(NXM1,JBASE,COUNT3)+                 &
                & CZP(NXM1,JBASE,COUNT3))*T(NXM1,JBASE,COUNT3)+                   &
                & CZP(NXM1,JBASE,COUNT3)* T(NXM1,JBASE,COUNT3+1))+                &
                & T(NXM1,JBASE,COUNT3)+(3.d0-2.d0*F)*CONST(NXM1,JBASE,COUNT3)*        &
                & CXP(NXM1,JBASE,COUNT3)*TGAV(COUNT3)
                N=NXM1-0+1
                L=1
                DO COUNT1=0,NXM1
                  AA(L)=A(COUNT1,JBASE,COUNT3)
                  BB(L)=B(COUNT1,JBASE,COUNT3)
                  CC(L)=C(COUNT1,JBASE,COUNT3)
                  RR(L)=R(COUNT1,JBASE,COUNT3)
                  L=L+1
                END DO
                CALL TRIDI3D (AA,BB,CC,RR,N,X)
                L=0
                DO COUNT1=0,NXM1
                  U(COUNT1,JBASE,COUNT3)=X(L+1)
                  L=L+1
                END DO
              END DO

!***  FOUNDATION WALL/GRAVEL/GROUND
              DO COUNT3=1,NZBGM1
                DO COUNT2=JBASE+1,NYM1
                  A(0,COUNT2,COUNT3)=0.
                  B(0,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(0,COUNT2,COUNT3)*         &
                      & CXP(0,COUNT2,COUNT3)
                  C(0,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(0,COUNT2,COUNT3)*            &
                      & (-CXP(0,COUNT2,COUNT3))
                  R(0,COUNT2,COUNT3)=F*CONST(0,COUNT2,COUNT3)*(CYM(0,COUNT2,COUNT3)* &
                  & T(0,COUNT2-1,COUNT3)-(CYM(0,COUNT2,COUNT3)+ &
                  & CYP(0,COUNT2,COUNT3))*T(0,COUNT2,COUNT3)+CYP(0,COUNT2,COUNT3)* &
                  & T(0,COUNT2+1,COUNT3)+CZM(0,COUNT2,COUNT3)*T(0,COUNT2,COUNT3-1) &
                  & -(CZM(0,COUNT2,COUNT3)+CZP(0,COUNT2,COUNT3))*                 &
                  & T(0,COUNT2,COUNT3)+CZP(0,COUNT2,COUNT3)*T(0,COUNT2,COUNT3+1))+ &
                  & T(0,COUNT2,COUNT3)
                  DO COUNT1=1,NXM1-1
                    A(COUNT1,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,COUNT3)* &
                       & (-CXM(COUNT1,COUNT2,COUNT3))
                    B(COUNT1,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,COUNT3)* &
                       & (CXM(COUNT1,COUNT2,COUNT3)+CXP(COUNT1,COUNT2,COUNT3))
                    C(COUNT1,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,COUNT3)* &
                       &     (-CXP(COUNT1,COUNT2,COUNT3))
                    R(COUNT1,COUNT2,COUNT3)=F*CONST(COUNT1,COUNT2,COUNT3)*        &
                    & (CYM(COUNT1,COUNT2,COUNT3)*T(COUNT1,COUNT2-1,COUNT3)-       &
                    & (CYM(COUNT1,COUNT2,COUNT3)+CYP(COUNT1,COUNT2,COUNT3))*      &
                    & T(COUNT1,COUNT2,COUNT3)+CYP(COUNT1,COUNT2,COUNT3)*          &
                    & T(COUNT1,COUNT2+1,COUNT3)+CZM(COUNT1,COUNT2,COUNT3)*        &
                    & T(COUNT1,COUNT2,COUNT3-1)-(CZM(COUNT1,COUNT2,COUNT3)+       &
                    & CZP(COUNT1,COUNT2,COUNT3))*T(COUNT1,COUNT2,COUNT3)+         &
                    & CZP(COUNT1,COUNT2,COUNT3)*T(COUNT1,COUNT2,COUNT3+1))+       &
                    & T(COUNT1,COUNT2,COUNT3)
                  END DO
                  A(NXM1,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(NXM1,COUNT2,COUNT3)*      &
                      & (-CXM(NXM1,COUNT2,COUNT3))
                  B(NXM1,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(NXM1,COUNT2,COUNT3)*   &
                      & (CXM(NXM1,COUNT2,COUNT3)+CXP(NXM1,COUNT2,COUNT3))
                  C(NXM1,COUNT2,COUNT3)=0.
                  R(NXM1,COUNT2,COUNT3)=F*CONST(NXM1,COUNT2,COUNT3)*              &
                  & (CYM(NXM1,COUNT2,COUNT3)*T(NXM1,COUNT2-1,COUNT3)-             &
                  & (CYM(NXM1,COUNT2,COUNT3)+CYP(NXM1,COUNT2,COUNT3))*            &
                  & T(NXM1,COUNT2,COUNT3)+CYP(NXM1,COUNT2,COUNT3)*                &
                  & T(NXM1,COUNT2+1,COUNT3)+CZM(NXM1,COUNT2,COUNT3)*              &
                  & T(NXM1,COUNT2,COUNT3-1)-(CZM(NXM1,COUNT2,COUNT3)+             &
                  & CZP(NXM1,COUNT2,COUNT3))*T(NXM1,COUNT2,COUNT3)+               &
                  & CZP(NXM1,COUNT2,COUNT3)*T(NXM1,COUNT2,COUNT3+1))+             &
                  & T(NXM1,COUNT2,COUNT3)+(3.d0-2.d0*F)*CONST(NXM1,COUNT2,COUNT3)*    &
                  &   CXP(NXM1,COUNT2,COUNT3)*TGAV(COUNT3)
                  N=NXM1-0+1
                  L=1
                  DO COUNT1=0,NXM1
                    AA(L)=A(COUNT1,COUNT2,COUNT3)
                    BB(L)=B(COUNT1,COUNT2,COUNT3)
                    CC(L)=C(COUNT1,COUNT2,COUNT3)
                    RR(L)=R(COUNT1,COUNT2,COUNT3)
                    L=L+1
                  END DO
                  CALL TRIDI3D (AA,BB,CC,RR,N,X)
                  L=0
                  DO COUNT1=0,NXM1
                    U(COUNT1,COUNT2,COUNT3)=X(L+1)
                    L=L+1
                  END DO
                END DO
              END DO

!***  SECTION 7:  BELOW-GRADE (2)
!***  INSIDE WALL CELLS, PARALLEL TO Y-AXIS (UPPER BAND) COUNT3=0
              DO COUNT2=0,JBASE-1
                A(IBASE,COUNT2,0)=0.
                B(IBASE,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,COUNT2,0)*             &
                    & (1.d0/DX(IBASE)/(RINT+1.d0/HIN(6))+CXP(IBASE,COUNT2,0))
                C(IBASE,COUNT2,0)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,0)*               &
                    & (-CXP(IBASE,COUNT2,0))
                R(IBASE,COUNT2,0)=F*CONST(IBASE,COUNT2,0)*(CYM(IBASE,COUNT2,0)*  &
                & T(IBASE,COUNT2-1,0)-(CYM(IBASE,COUNT2,0)+CYP(IBASE,COUNT2,0))* &
                & T(IBASE,COUNT2,0)+CYP(IBASE,COUNT2,0)*T(IBASE,COUNT2+1,0)+     &
                & CZM(IBASE,COUNT2,0)*T(IBASE,COUNT2,0-1)-(CZM(IBASE,COUNT2,0)+  &
                & CZP(IBASE,COUNT2,0))*T(IBASE,COUNT2,0)+CZP(IBASE,COUNT2,0)*    &
                & T(IBASE,COUNT2,0+1))+T(IBASE,COUNT2,0)+(3.d0-2.d0*F)*              &
                & CONST(IBASE,COUNT2,0)*(TBAV/DX(IBASE)/(RINT+1.d0/HIN(6)))

!***  FOUNDATION WALL CELLS COUNT3=0
                DO COUNT1=IBASE+1,IBASE+2
                  A(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*            &
                      & (-CXM(COUNT1,COUNT2,0))
                  B(COUNT1,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*         &
                      & (CXM(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0))
                  C(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*            &
                      & (-CXP(COUNT1,COUNT2,0))
                  R(COUNT1,COUNT2,0)=F*CONST(COUNT1,COUNT2,0)*                    &
                  & (CYM(COUNT1,COUNT2,0)*T(COUNT1,COUNT2-1,0)-                   &
                  & (CYM(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0))*                  &
                  & T(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0)*                      &
                  & T(COUNT1,COUNT2+1,0)+CZM(COUNT1,COUNT2,0)*                    &
                  & T(COUNT1,COUNT2,0-1)-(CZM(COUNT1,COUNT2,0)                    &
                  & +CZP(COUNT1,COUNT2,0))*T(COUNT1,COUNT2,0)+                    &
                  & CZP(COUNT1,COUNT2,0)*T(COUNT1,COUNT2,0+1))+T(COUNT1,COUNT2,0)
                END DO

!***  GROUND SURFACE CELLS COUNT3=0
                DO COUNT1=IBASE+3,NXM1-1
                  IF (ISNW.EQ.1) THEN
                    A(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*          &
                      & (-CXM(COUNT1,COUNT2,0))
                    B(COUNT1,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*       &
                      &  (CXM(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0))
                    C(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*          &
                      & (-CXP(COUNT1,COUNT2,0))
                    R(COUNT1,COUNT2,0)=F*CONST(COUNT1,COUNT2,0)*                  &
                    & (CYM(COUNT1,COUNT2,0)*T(COUNT1,COUNT2-1,0)-                 &
                    & (CYM(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0))*                &
                    & T(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0)*T(COUNT1,COUNT2+1,0)- &
                    & CZP(COUNT1,COUNT2,0)*T(COUNT1,COUNT2,0)+CZP(COUNT1,COUNT2,0)* &
                    & T(COUNT1,COUNT2,0+1))+T(COUNT1,COUNT2,0)+F*                 &
                    & CONST(COUNT1,COUNT2,0)*(GOFTAV(COUNT1,COUNT2)/DZ(0))
                  ELSE
                    A(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*          &
                     & (-CXM(COUNT1,COUNT2,0))
                    B(COUNT1,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*       &
                      & (CXM(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0))
                    C(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*          &
                      & (-CXP(COUNT1,COUNT2,0))
                    R(COUNT1,COUNT2,0)=F*CONST(COUNT1,COUNT2,0)*                  &
                    & (CYM(COUNT1,COUNT2,0)*T(COUNT1,COUNT2-1,0)-                 &
                    & (CYM(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0))*                &
                    & T(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0)*T(COUNT1,COUNT2+1,0)- &
                    & CZP(COUNT1,COUNT2,0)*T(COUNT1,COUNT2,0)+CZP(COUNT1,COUNT2,0)* &
                    & T(COUNT1,COUNT2,0+1))+T(COUNT1,COUNT2,0)+                   &
                    & F*CONST(COUNT1,COUNT2,0)*(GOFTAV(COUNT1,COUNT2)/DZ(0)+      &
                    & TDBAV/DZ(0)/RSNWAV)
                  END IF
                END DO
                IF (ISNW.EQ.1) THEN
                  A(NXM1,COUNT2,0)=(3.d0-2.d0*F)*CONST(NXM1,COUNT2,0)*(-CXM(NXM1,COUNT2,0))
                  B(NXM1,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(NXM1,COUNT2,0)*(CXM(NXM1,COUNT2,0) &
                  & + CXP(NXM1,COUNT2,0))
                  C(NXM1,COUNT2,0)=0.
                  R(NXM1,COUNT2,0)=F*CONST(NXM1,COUNT2,0)*(CYM(NXM1,COUNT2,0)*    &
                  & T(NXM1,COUNT2-1,0)-(CYM(NXM1,COUNT2,0)+CYP(NXM1,COUNT2,0))*   &
                  & T(NXM1,COUNT2,0)+CYP(NXM1,COUNT2,0)*T(NXM1,COUNT2+1,0)-       &
                  & CZP(NXM1,COUNT2,0)*T(NXM1,COUNT2,0)+CZP(NXM1,COUNT2,0)*       &
                  & T(NXM1,COUNT2,0+1))+T(NXM1,COUNT2,0)+F*CONST(NXM1,COUNT2,0)*  &
                  & (GOFTAV(NXM1,COUNT2)/DZ(0))+(3.d0-2.d0*F)*CONST(NXM1,COUNT2,0)*   &
                  & CXP(NXM1,COUNT2,0)*TGAV(0)
                ELSE
                  A(NXM1,COUNT2,0)=(3.d0-2.d0*F)*CONST(NXM1,COUNT2,0)*                &
                      & (-CXM(NXM1,COUNT2,0))
                  B(NXM1,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(NXM1,COUNT2,0)*             &
                      & (CXM(NXM1,COUNT2,0)+CXP(NXM1,COUNT2,0))
                  C(NXM1,COUNT2,0)=0.
                  R(NXM1,COUNT2,0)=F*CONST(NXM1,COUNT2,0)*(CYM(NXM1,COUNT2,0)*    &
                  & T(NXM1,COUNT2-1,0)-(CYM(NXM1,COUNT2,0)+CYP(NXM1,COUNT2,0))*   &
                  & T(NXM1,COUNT2,0)+CYP(NXM1,COUNT2,0)*T(NXM1,COUNT2+1,0)-       &
                  & CZP(NXM1,COUNT2,0)*T(NXM1,COUNT2,0)+CZP(NXM1,COUNT2,0)*       &
                  & T(NXM1,COUNT2,0+1))+T(NXM1,COUNT2,0)+F*CONST(NXM1,COUNT2,0)*  &
                  & (GOFTAV(NXM1,COUNT2)/DZ(0)+TDBAV/DZ(0)/RSNWAV)+(3.d0-2.d0*F)*     &
                  & CONST(NXM1,COUNT2,0)*CXP(NXM1,COUNT2,0)*TGAV(0)
                END IF
                N=NXM1-IBASE+1
                L=1
                DO COUNT1=IBASE,NXM1
                  AA(L)=A(COUNT1,COUNT2,0)
                  BB(L)=B(COUNT1,COUNT2,0)
                  CC(L)=C(COUNT1,COUNT2,0)
                  RR(L)=R(COUNT1,COUNT2,0)
                  L=L+1
                END DO
                CALL TRIDI3D (AA,BB,CC,RR,N,X)
                L=0
                DO COUNT1=IBASE,NXM1
                  U(COUNT1,COUNT2,0)=X(L+1)
                  L=L+1
                END DO
              END DO

!***  INSIDE WALL CELLS, PARALLEL TO Y-AXIS (UPPER BAND)
              DO COUNT3=1,INT(1+(KBASE-NZAG)/2)
                DO COUNT2=0,JBASE-1
                  A(IBASE,COUNT2,COUNT3)=0.
                  B(IBASE,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)* &
                    & (1.d0/DX(IBASE)/(RINT+1.d0/HIN(6))+CXP(IBASE,COUNT2,COUNT3))
                  C(IBASE,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)*  &
                    & (-CXP(IBASE,COUNT2,COUNT3))
                  R(IBASE,COUNT2,COUNT3)=F*CONST(IBASE,COUNT2,COUNT3)*          &
                  & (CYM(IBASE,COUNT2,COUNT3)*T(IBASE,COUNT2-1,COUNT3)-         &
                  & (CYM(IBASE,COUNT2,COUNT3)+CYP(IBASE,COUNT2,COUNT3))*        &
                  & T(IBASE,COUNT2,COUNT3)+CYP(IBASE,COUNT2,COUNT3)*            &
                  & T(IBASE,COUNT2+1,COUNT3)+CZM(IBASE,COUNT2,COUNT3)*          &
                  & T(IBASE,COUNT2,COUNT3-1)-(CZM(IBASE,COUNT2,COUNT3)+         &
                  & CZP(IBASE,COUNT2,COUNT3))*T(IBASE,COUNT2,COUNT3)+           &
                  & CZP(IBASE,COUNT2,COUNT3)*T(IBASE,COUNT2,COUNT3+1))+         &
                  & T(IBASE,COUNT2,COUNT3)+(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)* &
                  & (TBAV/DX(IBASE)/(RINT+1.d0/HIN(6)))

!***  FOUNDATION WALL/GROUND
                  DO COUNT1=IBASE+1,NXM1-1
                    A(COUNT1,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,COUNT3)* &
                          & (-CXM(COUNT1,COUNT2,COUNT3))
                    B(COUNT1,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*                         &
                      & CONST(COUNT1,COUNT2,COUNT3)*(CXM(COUNT1,COUNT2,COUNT3)+   &
                      & CXP(COUNT1,COUNT2,COUNT3))
                    C(COUNT1,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,COUNT3)* &
                      & (-CXP(COUNT1,COUNT2,COUNT3))
                    R(COUNT1,COUNT2,COUNT3)=F*CONST(COUNT1,COUNT2,COUNT3)*        &
                    & (CYM(COUNT1,COUNT2,COUNT3)*T(COUNT1,COUNT2-1,COUNT3)-       &
                    & (CYM(COUNT1,COUNT2,COUNT3)+CYP(COUNT1,COUNT2,COUNT3))*      &
                    & T(COUNT1,COUNT2,COUNT3)+CYP(COUNT1,COUNT2,COUNT3)*          &
                    & T(COUNT1,COUNT2+1,COUNT3)+CZM(COUNT1,COUNT2,COUNT3)*        &
                    & T(COUNT1,COUNT2,COUNT3-1)-(CZM(COUNT1,COUNT2,COUNT3)+       &
                    & CZP(COUNT1,COUNT2,COUNT3))*T(COUNT1,COUNT2,COUNT3)+         &
                    & CZP(COUNT1,COUNT2,COUNT3)*T(COUNT1,COUNT2,COUNT3+1))+       &
                    & T(COUNT1,COUNT2,COUNT3)
                  END DO
                  A(NXM1,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(NXM1,COUNT2,COUNT3)*      &
                      & (-CXM(NXM1,COUNT2,COUNT3))
                  B(NXM1,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(NXM1,COUNT2,COUNT3)*   &
                      & (CXM(NXM1,COUNT2,COUNT3)+CXP(NXM1,COUNT2,COUNT3))
                  C(NXM1,COUNT2,COUNT3)=0.
                  R(NXM1,COUNT2,COUNT3)=F*CONST(NXM1,COUNT2,COUNT3)*              &
                  & (CYM(NXM1,COUNT2,COUNT3)*T(NXM1,COUNT2-1,COUNT3)-             &
                  & (CYM(NXM1,COUNT2,COUNT3)+CYP(NXM1,COUNT2,COUNT3))*            &
                  & T(NXM1,COUNT2,COUNT3)+CYP(NXM1,COUNT2,COUNT3)*                &
                  & T(NXM1,COUNT2+1,COUNT3)+CZM(NXM1,COUNT2,COUNT3)*              &
                  & T(NXM1,COUNT2,COUNT3-1)-(CZM(NXM1,COUNT2,COUNT3)+             &
                  & CZP(NXM1,COUNT2,COUNT3))*T(NXM1,COUNT2,COUNT3)+               &
                  & CZP(NXM1,COUNT2,COUNT3)*T(NXM1,COUNT2,COUNT3+1))+             &
                  & T(NXM1,COUNT2,COUNT3)+(3.d0-2.d0*F)*CONST(NXM1,COUNT2,COUNT3)*    &
                  & CXP(NXM1,COUNT2,COUNT3)*TGAV(COUNT3)
                  N=NXM1-IBASE+1
                  L=1
                  DO COUNT1=IBASE,NXM1
                    AA(L)=A(COUNT1,COUNT2,COUNT3)
                    BB(L)=B(COUNT1,COUNT2,COUNT3)
                    CC(L)=C(COUNT1,COUNT2,COUNT3)
                    RR(L)=R(COUNT1,COUNT2,COUNT3)
                    L=L+1
                  END DO
                  CALL TRIDI3D (AA,BB,CC,RR,N,X)
                  L=0
                  DO COUNT1=IBASE,NXM1
                    U(COUNT1,COUNT2,COUNT3)=X(L+1)
                    L=L+1
                  END DO
                END DO
              END DO

!***  INSIDE WALL CELLS, PARALLEL TO Y-AXIS (LOWER BAND)
              DO COUNT3=INT(2+(KBASE-NZAG)/2),KBASE-1
                DO COUNT2=0,JBASE-1
                  A(IBASE,COUNT2,COUNT3)=0.
                  B(IBASE,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)* &
                    & (1.d0/DX(IBASE)/(RINT+1.d0/HIN(6))+CXP(IBASE,COUNT2,COUNT3))
                  C(IBASE,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)*  &
                    & (-CXP(IBASE,COUNT2,COUNT3))
                  R(IBASE,COUNT2,COUNT3)=F*CONST(IBASE,COUNT2,COUNT3)*          &
                  & (CYM(IBASE,COUNT2,COUNT3)*T(IBASE,COUNT2-1,COUNT3)-         &
                  & (CYM(IBASE,COUNT2,COUNT3)+CYP(IBASE,COUNT2,COUNT3))*        &
                  & T(IBASE,COUNT2,COUNT3)+CYP(IBASE,COUNT2,COUNT3)*            &
                  & T(IBASE,COUNT2+1,COUNT3)+CZM(IBASE,COUNT2,COUNT3)*          &
                  & T(IBASE,COUNT2,COUNT3-1)-(CZM(IBASE,COUNT2,COUNT3)+         &
                  & CZP(IBASE,COUNT2,COUNT3))*T(IBASE,COUNT2,COUNT3)+           &
                  & CZP(IBASE,COUNT2,COUNT3)*T(IBASE,COUNT2,COUNT3+1))+         &
                  & T(IBASE,COUNT2,COUNT3)+(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)* &
                  & (TBAV/DX(IBASE)/(RINT+1.d0/HIN(6)))

!***  FOUNDATION WALL/GRAVEL/GROUND
                  DO COUNT1=IBASE+1,NXM1-1
                    A(COUNT1,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,COUNT3)* &
                      & (-CXM(COUNT1,COUNT2,COUNT3))
                    B(COUNT1,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*                         &
                    & CONST(COUNT1,COUNT2,COUNT3)*(CXM(COUNT1,COUNT2,COUNT3)+     &
                    & CXP(COUNT1,COUNT2,COUNT3))
                    C(COUNT1,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,COUNT3)* &
                    & (-CXP(COUNT1,COUNT2,COUNT3))
                    R(COUNT1,COUNT2,COUNT3)=F*CONST(COUNT1,COUNT2,COUNT3)*        &
                    & (CYM(COUNT1,COUNT2,COUNT3)*T(COUNT1,COUNT2-1,COUNT3)-       &
                    & (CYM(COUNT1,COUNT2,COUNT3)+CYP(COUNT1,COUNT2,COUNT3))*      &
                    & T(COUNT1,COUNT2,COUNT3)+CYP(COUNT1,COUNT2,COUNT3)*          &
                    & T(COUNT1,COUNT2+1,COUNT3)+CZM(COUNT1,COUNT2,COUNT3)*        &
                    & T(COUNT1,COUNT2,COUNT3-1)-(CZM(COUNT1,COUNT2,COUNT3)+       &
                    & CZP(COUNT1,COUNT2,COUNT3))*T(COUNT1,COUNT2,COUNT3)+         &
                    & CZP(COUNT1,COUNT2,COUNT3)*T(COUNT1,COUNT2,COUNT3+1))+       &
                    & T(COUNT1,COUNT2,COUNT3)
                  END DO
                  A(NXM1,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(NXM1,COUNT2,COUNT3)*      &
                      & (-CXM(NXM1,COUNT2,COUNT3))
                  B(NXM1,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(NXM1,COUNT2,COUNT3)*   &
                  & (CXM(NXM1,COUNT2,COUNT3)+CXP(NXM1,COUNT2,COUNT3))
                  C(NXM1,COUNT2,COUNT3)=0.
                  R(NXM1,COUNT2,COUNT3)=F*CONST(NXM1,COUNT2,COUNT3)*              &
                  & (CYM(NXM1,COUNT2,COUNT3)*T(NXM1,COUNT2-1,COUNT3)-             &
                  & (CYM(NXM1,COUNT2,COUNT3)+CYP(NXM1,COUNT2,COUNT3))*            &
                  & T(NXM1,COUNT2,COUNT3)+CYP(NXM1,COUNT2,COUNT3)*                &
                  & T(NXM1,COUNT2+1,COUNT3)+CZM(NXM1,COUNT2,COUNT3)*              &
                  & T(NXM1,COUNT2,COUNT3-1)-(CZM(NXM1,COUNT2,COUNT3)+             &
                  & CZP(NXM1,COUNT2,COUNT3))*T(NXM1,COUNT2,COUNT3)+               &
                  & CZP(NXM1,COUNT2,COUNT3)*T(NXM1,COUNT2,COUNT3+1))+             &
                  & T(NXM1,COUNT2,COUNT3)+(3.d0-2.d0*F)*CONST(NXM1,COUNT2,COUNT3)*    &
                  & CXP(NXM1,COUNT2,COUNT3)*TGAV(COUNT3)
                  N=NXM1-IBASE+1
                  L=1
                  DO COUNT1=IBASE,NXM1
                    AA(L)=A(COUNT1,COUNT2,COUNT3)
                    BB(L)=B(COUNT1,COUNT2,COUNT3)
                    CC(L)=C(COUNT1,COUNT2,COUNT3)
                    RR(L)=R(COUNT1,COUNT2,COUNT3)
                    L=L+1
                  END DO
                  CALL TRIDI3D (AA,BB,CC,RR,N,X)
                  L=0
                  DO COUNT1=IBASE,NXM1
                    U(COUNT1,COUNT2,COUNT3)=X(L+1)
                    L=L+1
                  END DO
                END DO
              END DO

!***  SECTION 8:  BELOW-GRADE (3)
!***  FLOOR SLAB
              DO COUNT2=0,JBASE-1
                IF (TBAV.GT.T(0,COUNT2,KBASE)) THEN
                  HINZ=HIN(4)
                ELSE
                  HINZ=HIN(5)
                END IF
                A(0,COUNT2,KBASE)=0.
                B(0,COUNT2,KBASE)=1.d0+(3.d0-2.d0*F)*CONST(0,COUNT2,KBASE)*           &
                    & CXP(0,COUNT2,KBASE)
                C(0,COUNT2,KBASE)=(3.d0-2.d0*F)*CONST(0,COUNT2,KBASE)*              &
                    & (-CXP(0,COUNT2,KBASE))
                R(0,COUNT2,KBASE)=F*CONST(0,COUNT2,KBASE)*(CYM(0,COUNT2,KBASE)* &
                & T(0,COUNT2-1,KBASE)-(CYM(0,COUNT2,KBASE)+CYP(0,COUNT2,KBASE))* &
                & T(0,COUNT2,KBASE)+CYP(0,COUNT2,KBASE)*T(0,COUNT2+1,KBASE)-    &
                & (HINZ/DZ(KBASE)+CZP(0,COUNT2,KBASE))*T(0,COUNT2,KBASE)+       &
                & CZP(0,COUNT2,KBASE)*T(0,COUNT2,KBASE+1))+T(0,COUNT2,KBASE)+   &
                & F*CONST(0,COUNT2,KBASE)*HINZ*TBAV/DZ(KBASE)

                DO COUNT1=1,IBASE-1
                  IF (TBAV.GT.T(COUNT1,COUNT2,KBASE)) THEN
                    HINZ=HIN(4)
                  ELSE
                    HINZ=HIN(5)
                  END IF
                  A(COUNT1,COUNT2,KBASE)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,KBASE)*  &
                    & (-CXM(COUNT1,COUNT2,KBASE))
                  B(COUNT1,COUNT2,KBASE)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,KBASE)* &
                    & (CXM(COUNT1,COUNT2,KBASE)+CXP(COUNT1,COUNT2,KBASE))
                  C(COUNT1,COUNT2,KBASE)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,KBASE)*  &
                    & (-CXP(COUNT1,COUNT2,KBASE))
                  R(COUNT1,COUNT2,KBASE)=F*CONST(COUNT1,COUNT2,KBASE)*          &
                  & (CYM(COUNT1,COUNT2,KBASE)*T(COUNT1,COUNT2-1,KBASE)-         &
                  & (CYM(COUNT1,COUNT2,KBASE)+CYP(COUNT1,COUNT2,KBASE))*        &
                  & T(COUNT1,COUNT2,KBASE)+CYP(COUNT1,COUNT2,KBASE)*            &
                  & T(COUNT1,COUNT2+1,KBASE)-(HINZ/DZ(KBASE)+                   &
                  & CZP(COUNT1,COUNT2,KBASE))*T(COUNT1,COUNT2,KBASE)+           &
                  & CZP(COUNT1,COUNT2,KBASE)*T(COUNT1,COUNT2,KBASE+1))+         &
                  & T(COUNT1,COUNT2,KBASE)+F*CONST(COUNT1,COUNT2,KBASE)*HINZ*   &
                  & TBAV/DZ(KBASE)
                END DO

!***  FOUNDATION WALL/GRAVEL/GROUND
                DO COUNT1=IBASE,NXM1-1
                  A(COUNT1,COUNT2,KBASE)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,KBASE)*    &
                     & (-CXM(COUNT1,COUNT2,KBASE))
                  B(COUNT1,COUNT2,KBASE)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,KBASE)* &
                      & (CXM(COUNT1,COUNT2,KBASE)+CXP(COUNT1,COUNT2,KBASE))
                  C(COUNT1,COUNT2,KBASE)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,KBASE)*    &
                      & (-CXP(COUNT1,COUNT2,KBASE))
                  R(COUNT1,COUNT2,KBASE)=F*CONST(COUNT1,COUNT2,KBASE)*            &
                      & (CYM(COUNT1,COUNT2,KBASE)*T(COUNT1,COUNT2-1,KBASE)-       &
                      & (CYM(COUNT1,COUNT2,KBASE)+CYP(COUNT1,COUNT2,KBASE))*      &
                      & T(COUNT1,COUNT2,KBASE)+CYP(COUNT1,COUNT2,KBASE)*          &
                      & T(COUNT1,COUNT2+1,KBASE)+CZM(COUNT1,COUNT2,KBASE)*        &
                      & T(COUNT1,COUNT2,KBASE-1)-(CZM(COUNT1,COUNT2,KBASE)+       &
                      & CZP(COUNT1,COUNT2,KBASE))*T(COUNT1,COUNT2,KBASE)+         &
                      & CZP(COUNT1,COUNT2,KBASE)*T(COUNT1,COUNT2,KBASE+1))+       &
                      & T(COUNT1,COUNT2,KBASE)
                END DO
                A(NXM1,COUNT2,KBASE)=(3.d0-2.d0*F)*CONST(NXM1,COUNT2,KBASE)*          &
                      & (-CXM(NXM1,COUNT2,KBASE))
                B(NXM1,COUNT2,KBASE)=1.d0+(3.d0-2.d0*F)*CONST(NXM1,COUNT2,KBASE)*       &
                      & (CXM(NXM1,COUNT2,KBASE)+CXP(NXM1,COUNT2,KBASE))
                C(NXM1,COUNT2,KBASE)=0.
                R(NXM1,COUNT2,KBASE)=F*CONST(NXM1,COUNT2,KBASE)*                  &
                & (CYM(NXM1,COUNT2,KBASE)*T(NXM1,COUNT2-1,KBASE)-                 &
                & (CYM(NXM1,COUNT2,KBASE)+CYP(NXM1,COUNT2,KBASE))*                &
                & T(NXM1,COUNT2,KBASE)+CYP(NXM1,COUNT2,KBASE)*                    &
                & T(NXM1,COUNT2+1,KBASE)+CZM(NXM1,COUNT2,KBASE)*                  &
                & T(NXM1,COUNT2,KBASE-1)-(CZM(NXM1,COUNT2,KBASE)+                 &
                & CZP(NXM1,COUNT2,KBASE))*T(NXM1,COUNT2,KBASE)+                   &
                & CZP(NXM1,COUNT2,KBASE)*T(NXM1,COUNT2,KBASE+1))+                 &
                & T(NXM1,COUNT2,KBASE)+(3.d0-2.d0*F)*CONST(NXM1,COUNT2,KBASE)*        &
                & CXP(NXM1,COUNT2,KBASE)*TGAV(KBASE)
                N=NXM1-0+1
                L=1
                DO COUNT1=0,NXM1
                  AA(L)=A(COUNT1,COUNT2,KBASE)
                  BB(L)=B(COUNT1,COUNT2,KBASE)
                  CC(L)=C(COUNT1,COUNT2,KBASE)
                  RR(L)=R(COUNT1,COUNT2,KBASE)
                  L=L+1
                END DO
                CALL TRIDI3D (AA,BB,CC,RR,N,X)
                L=0
                DO COUNT1=0,NXM1
                  U(COUNT1,COUNT2,KBASE)=X(L+1)
                  L=L+1
                END DO
              END DO

!***  BELOW FLOOR SLAB
              DO COUNT3=KBASE+1,NZBGM1
                DO COUNT2=0,JBASE-1
                  A(0,COUNT2,COUNT3)=0.
                  B(0,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(0,COUNT2,COUNT3)*         &
                      & CXP(0,COUNT2,COUNT3)
                  C(0,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(0,COUNT2,COUNT3)*            &
                      & (-CXP(0,COUNT2,COUNT3))
                  R(0,COUNT2,COUNT3)=F*CONST(0,COUNT2,COUNT3)*(CYM(0,COUNT2,COUNT3)* &
                  & T(0,COUNT2-1,COUNT3)-(CYM(0,COUNT2,COUNT3)+                   &
                  & CYP(0,COUNT2,COUNT3))*T(0,COUNT2,COUNT3)+CYP(0,COUNT2,COUNT3)* &
                  & T(0,COUNT2+1,COUNT3)+CZM(0,COUNT2,COUNT3)*T(0,COUNT2,COUNT3-1) &
                  & -(CZM(0,COUNT2,COUNT3)+CZP(0,COUNT2,COUNT3))*T(0,COUNT2,COUNT3)+ &
                  & CZP(0,COUNT2,COUNT3)*T(0,COUNT2,COUNT3+1))+T(0,COUNT2,COUNT3)
                  DO COUNT1=1,NXM1-1
                    A(COUNT1,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,COUNT3)* &
                          & (-CXM(COUNT1,COUNT2,COUNT3))
                    B(COUNT1,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*                         &
                    & CONST(COUNT1,COUNT2,COUNT3)*(CXM(COUNT1,COUNT2,COUNT3)+     &
                    & CXP(COUNT1,COUNT2,COUNT3))
                    C(COUNT1,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,COUNT3)* &
                    & (-CXP(COUNT1,COUNT2,COUNT3))
                    R(COUNT1,COUNT2,COUNT3)=F*CONST(COUNT1,COUNT2,COUNT3)*        &
                    & (CYM(COUNT1,COUNT2,COUNT3)*T(COUNT1,COUNT2-1,COUNT3)-       &
                    & (CYM(COUNT1,COUNT2,COUNT3)+CYP(COUNT1,COUNT2,COUNT3))*      &
                    & T(COUNT1,COUNT2,COUNT3)+CYP(COUNT1,COUNT2,COUNT3)*          &
                    & T(COUNT1,COUNT2+1,COUNT3)+CZM(COUNT1,COUNT2,COUNT3)*        &
                    & T(COUNT1,COUNT2,COUNT3-1)-(CZM(COUNT1,COUNT2,COUNT3)+       &
                    & CZP(COUNT1,COUNT2,COUNT3))*T(COUNT1,COUNT2,COUNT3)+         &
                    & CZP(COUNT1,COUNT2,COUNT3)*T(COUNT1,COUNT2,COUNT3+1))+       &
                    & T(COUNT1,COUNT2,COUNT3)
                  END DO
                  A(NXM1,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(NXM1,COUNT2,COUNT3)*      &
                      & (-CXM(NXM1,COUNT2,COUNT3))
                  B(NXM1,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(NXM1,COUNT2,COUNT3)*   &
                      & (CXM(NXM1,COUNT2,COUNT3)+CXP(NXM1,COUNT2,COUNT3))
                  C(NXM1,COUNT2,COUNT3)=0.
                  R(NXM1,COUNT2,COUNT3)=F*CONST(NXM1,COUNT2,COUNT3)*              &
                  & (CYM(NXM1,COUNT2,COUNT3)*T(NXM1,COUNT2-1,COUNT3)-             &
                  & (CYM(NXM1,COUNT2,COUNT3)+CYP(NXM1,COUNT2,COUNT3))*            &
                  & T(NXM1,COUNT2,COUNT3)+CYP(NXM1,COUNT2,COUNT3)*                &
                  & T(NXM1,COUNT2+1,COUNT3)+CZM(NXM1,COUNT2,COUNT3)*              &
                  & T(NXM1,COUNT2,COUNT3-1)-(CZM(NXM1,COUNT2,COUNT3)+             &
                  & CZP(NXM1,COUNT2,COUNT3))*T(NXM1,COUNT2,COUNT3)+               &
                  & CZP(NXM1,COUNT2,COUNT3)*T(NXM1,COUNT2,COUNT3+1))+             &
                  & T(NXM1,COUNT2,COUNT3)+(3.d0-2.d0*F)*CONST(NXM1,COUNT2,COUNT3)*    &
                  & CXP(NXM1,COUNT2,COUNT3)*TGAV(COUNT3)
                  N=NXM1-0+1
                  L=1
                  DO COUNT1=0,NXM1
                    AA(L)=A(COUNT1,COUNT2,COUNT3)
                    BB(L)=B(COUNT1,COUNT2,COUNT3)
                    CC(L)=C(COUNT1,COUNT2,COUNT3)
                    RR(L)=R(COUNT1,COUNT2,COUNT3)
                    L=L+1
                  END DO
                  CALL TRIDI3D (AA,BB,CC,RR,N,X)
                  L=0
                  DO COUNT1=0,NXM1
                    U(COUNT1,COUNT2,COUNT3)=X(L+1)
                    L=L+1
                  END DO
                END DO
              END DO

!*********************************************************************!
!***  SECOND FRACTION OF TIME INCREMENT (Y-DIRECTION IMPLICIT,     ***!
!***  X AND Z-DIRECTIONS EXPLICIT)                                 ***!
!*********************************************************************!

!***  SET CONSTANTS
              DO COUNT1=0,NXM1
                DO COUNT2=0,NYM1
                  DO COUNT3=-NZAG,NZBGM1
                    IF (COUNT1.EQ.0) U(COUNT1-1,COUNT2,COUNT3)=U(COUNT1,COUNT2,COUNT3)
                    IF (COUNT1.EQ.NXM1.AND.COUNT3.GE.0) U(COUNT1+1,COUNT2,COUNT3)=TGAV(COUNT3)
                    IF (COUNT3.EQ.NZBGM1) THEN
                      IF (.not. SameString(FIXBC,'FALSE')) THEN
                        U(COUNT1,COUNT2,COUNT3+1)=TDEEP
                      ELSE
                        U(COUNT1,COUNT2,COUNT3+1)=U(COUNT1,COUNT2,COUNT3)
                      END IF
                    END IF
                  END DO
                END DO
              END DO

!***  SET UP COEFFICIENT MATRIX IN Y-DIRECTION:
!***  SECTION 1:  CEILING CELLS
              DO COUNT1=0,IBASE+1
                IF (TIAV.GT.U(COUNT1,0,-NZAG)) THEN
                   HINZH=HIN(4)
                ELSE
                   HINZH=HIN(5)
                END IF
                IF (TBAV.GE.U(COUNT1,0,-NZAG)) THEN
                  HINZ=HIN(5)
                ELSE
                  HINZ=HIN(4)
                END IF
                A(COUNT1,0,-NZAG)=0.
                B(COUNT1,0,-NZAG)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,0,-NZAG)*           &
                    &  CYP(COUNT1,0,-NZAG)
                C(COUNT1,0,-NZAG)=(3.d0-2.d0*F)*CONST(COUNT1,0,-NZAG)*              &
                    & (-CYP(COUNT1,0,-NZAG))
                R(COUNT1,0,-NZAG)=F*CONST(COUNT1,0,-NZAG)*(CXM(COUNT1,0,-NZAG)* &
                & U(COUNT1-1,0,-NZAG)-(CXM(COUNT1,0,-NZAG)+CXP(COUNT1,0,-NZAG))* &
                & U(COUNT1,0,-NZAG)+CXP(COUNT1,0,-NZAG)*U(COUNT1+1,0,-NZAG)-    &
                & (1.d0/DZ(-NZAG)/(1.d0/HINZH+DZ(-NZAG)/2.d0/TCON                     &
                & (MTYPE(COUNT1,0,-NZAG)))+1.d0/DZ(-NZAG)/(RCEIL+1.d0/HINZ+         &
                & DZ(-NZAG)/2.d0/TCON(MTYPE(COUNT1,0,-NZAG))))*U(COUNT1,0,-NZAG))+ &
                & U(COUNT1,0,-NZAG)+F*CONST(COUNT1,0,-NZAG)*(TIAV/DZ(-NZAG)/    &
                & (1.d0/HINZH+DZ(-NZAG)/2.d0/TCON(MTYPE(COUNT1,0,-NZAG)))+TBAV/     &
                & DZ(-NZAG)/(RCEIL+1.d0/HINZ+DZ(-NZAG)/2.d0/                        &
                & TCON(MTYPE(COUNT1,0,-NZAG))))
                DO COUNT2=1,JBASE+1
                IF (TIAV.GT.U(COUNT1,COUNT2,-NZAG)) THEN
                  HINZH=HIN(4)
                ELSE
                  HINZH=HIN(5)
                END IF
                IF (TBAV.GE.U(COUNT1,COUNT2,-NZAG)) THEN
                  HINZ=HIN(5)
                ELSE
                  HINZ=HIN(4)
                END IF
                A(COUNT1,COUNT2,-NZAG)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,-NZAG)*  &
                  & (-CYM(COUNT1,COUNT2,-NZAG))
                B(COUNT1,COUNT2,-NZAG)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,-NZAG)* &
                  & (CYM(COUNT1,COUNT2,-NZAG)+CYP(COUNT1,COUNT2,-NZAG))
                C(COUNT1,COUNT2,-NZAG)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,-NZAG)*  &
                  & (-CYP(COUNT1,COUNT2,-NZAG))
                R(COUNT1,COUNT2,-NZAG)=F*CONST(COUNT1,COUNT2,-NZAG)*          &
                & (CXM(COUNT1,COUNT2,-NZAG)*U(COUNT1-1,COUNT2,-NZAG)-         &
                & (CXM(COUNT1,COUNT2,-NZAG)+CXP(COUNT1,COUNT2,-NZAG))*        &
                & U(COUNT1,COUNT2,-NZAG)+CXP(COUNT1,COUNT2,-NZAG)*            &
                & U(COUNT1+1,COUNT2,-NZAG)-(1.d0/DZ(-NZAG)/(1.d0/HINZH+DZ(-NZAG)/ &
                & 2./TCON(MTYPE(COUNT1,COUNT2,-NZAG)))+1.d0/DZ(-NZAG)/(RCEIL+1.d0/ &
                & HINZ+DZ(-NZAG)/2.d0/TCON(MTYPE(COUNT1,COUNT2,-NZAG))))*       &
                & U(COUNT1,COUNT2,-NZAG))+U(COUNT1,COUNT2,-NZAG)+F*           &
                & CONST(COUNT1,COUNT2,-NZAG)*(TIAV/DZ(-NZAG)/(1.d0/HINZH+DZ     &
                & (-NZAG)/2.d0/TCON(MTYPE(COUNT1,COUNT2,-NZAG)))+TBAV/DZ(-NZAG)/ &
                & (RCEIL+1.d0/HINZ+DZ(-NZAG)/2.d0/TCON(MTYPE(COUNT1,COUNT2,-NZAG))))
              END DO

!***  PERIMETER CEILING CELL, PARALLEL TO X-AXIS
              A(COUNT1,JBASE+2,-NZAG)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,-NZAG)*    &
                    & (-CYM(COUNT1,JBASE+2,-NZAG))
              B(COUNT1,JBASE+2,-NZAG)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,-NZAG)* &
                    & (CYM(COUNT1,JBASE+2,-NZAG)+1.d0/DY(JBASE+2)/(REXT+RSID+1.d0/HOAV))
              C(COUNT1,JBASE+2,-NZAG)=0.
              R(COUNT1,JBASE+2,-NZAG)=F*CONST(COUNT1,JBASE+2,-NZAG)*            &
              & (CXM(COUNT1,JBASE+2,-NZAG)*U(COUNT1-1,JBASE+2,-NZAG)-           &
              & (CXM(COUNT1,JBASE+2,-NZAG)+CXP(COUNT1,JBASE+2,-NZAG))*          &
              & U(COUNT1,JBASE+2,-NZAG)+CXP(COUNT1,JBASE+2,-NZAG)*              &
              & U(COUNT1+1,JBASE+2,-NZAG)-CZP(COUNT1,JBASE+2,-NZAG)*            &
              & U(COUNT1,JBASE+2,-NZAG)+CZP(COUNT1,JBASE+2,-NZAG)*              &
              & U(COUNT1,JBASE+2,-NZAG+1))+U(COUNT1,JBASE+2,-NZAG)+(3.d0-2.d0*F)*   &
              & CONST(COUNT1,JBASE+2,-NZAG)*(TDBAV/DY(JBASE+2)/                 &
              & (REXT+RSID+1.d0/HOAV)+RSOLVAV/DY(JBASE+2)-(0.88d0*SIGMA*            &
              & (UEXT(COUNT1,JBASE+2,-NZAG)+273.15d0)**4)/DY(JBASE+2))
              N=JBASE+2-0+1
              L=1
              DO COUNT2=0,JBASE+2
                AA(L)=A(COUNT1,COUNT2,-NZAG)
                BB(L)=B(COUNT1,COUNT2,-NZAG)
                CC(L)=C(COUNT1,COUNT2,-NZAG)
                RR(L)=R(COUNT1,COUNT2,-NZAG)
                L=L+1
              END DO
              CALL TRIDI3D (AA,BB,CC,RR,N,X)
              L=0
              DO COUNT2=0,JBASE+2
                V(COUNT1,COUNT2,-NZAG)=X(L+1)
                L=L+1
              END DO
                QEXT(COUNT1,JBASE+2,-NZAG)=(TDBAV-V(COUNT1,JBASE+2,-NZAG))/     &
                   & (REXT+RSID+1.d0/HOAV)
                VEXT(COUNT1,JBASE+2,-NZAG)=QEXT(COUNT1,JBASE+2,-NZAG)*          &
                   & (REXT+RSID)+V(COUNT1,JBASE+2,-NZAG)
              END DO

!***  PERIMETER CEILING CELLS, PARALLEL TO Y-AXIS
              A(IBASE+2,0,-NZAG)=0.
              B(IBASE+2,0,-NZAG)=1.d0+(3.d0-2.d0*F)*CONST(IBASE+2,0,-NZAG)*             &
                & CYP(IBASE+2,0,-NZAG)
              C(IBASE+2,0,-NZAG)=(3.d0-2.d0*F)*CONST(IBASE+2,0,-NZAG)*                &
                & (-CYP(IBASE+2,0,-NZAG))
              R(IBASE+2,0,-NZAG)=F*CONST(IBASE+2,0,-NZAG)*                        &
              & (CXM(IBASE+2,0,-NZAG)*U(IBASE+2-1,0,-NZAG)-                       &
              & (CXM(IBASE+2,0,-NZAG)+1.d0/DX(IBASE+2)/(REXT+RSID+1.d0/HOAV))*        &
              & U(IBASE+2,0,-NZAG)-CZP(IBASE+2,0,-NZAG)*U(IBASE+2,0,-NZAG)+       &
              & CZP(IBASE+2,0,-NZAG)*U(IBASE+2,0,-NZAG+1))+                       &
              & U(IBASE+2,0,-NZAG)+F*CONST(IBASE+2,0,-NZAG)*                      &
              & (TDBAV/DX(IBASE+2)/(REXT+RSID+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-       &
              & (0.88d0*SIGMA*(UEXT(IBASE+2,0,-NZAG)+273.15d0)**4)/DX(IBASE+2))
              DO COUNT2=1,JBASE+1
                A(IBASE+2,COUNT2,-NZAG)=(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,-NZAG)*    &
                      & (-CYM(IBASE+2,COUNT2,-NZAG))
                B(IBASE+2,COUNT2,-NZAG)=1.d0+(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,-NZAG)* &
                      & (CYM(IBASE+2,COUNT2,-NZAG)+CYP(IBASE+2,COUNT2,-NZAG))
                C(IBASE+2,COUNT2,-NZAG)=(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,-NZAG)*    &
                      & (-CYP(IBASE+2,COUNT2,-NZAG))
                R(IBASE+2,COUNT2,-NZAG)=F*CONST(IBASE+2,COUNT2,-NZAG)*            &
                & (CXM(IBASE+2,COUNT2,-NZAG)*U(IBASE+2-1,COUNT2,-NZAG)-           &
                & (CXM(IBASE+2,COUNT2,-NZAG)+1.d0/DX(IBASE+2)/(REXT+RSID+1.d0/HOAV))* &
                & U(IBASE+2,COUNT2,-NZAG)-CZP(IBASE+2,COUNT2,-NZAG)*              &
                & U(IBASE+2,COUNT2,-NZAG)+CZP(IBASE+2,COUNT2,-NZAG)*              &
                & U(IBASE+2,COUNT2,-NZAG+1))+U(IBASE+2,COUNT2,-NZAG)+F*           &
                & CONST(IBASE+2,COUNT2,-NZAG)*(TDBAV/DX(IBASE+2)/(REXT+RSID+1.d0/   &
                & HOAV)+RSOLVAV/DX(IBASE+2)-(0.88d0*SIGMA*(UEXT(IBASE+2,COUNT2,-NZAG)+ &
                & 273.15d0)**4)/DX(IBASE+2))
              END DO

!***  TOP OUTER CORNER CELL
              A(IBASE+2,JBASE+2,-NZAG)=(3.d0-2.d0*F)*CONST(IBASE+2,JBASE+2,-NZAG)*    &
                & (-CYM(IBASE+2,JBASE+2,-NZAG))
              B(IBASE+2,JBASE+2,-NZAG)=1.d0+(3.d0-2.d0*F)*CONST(IBASE+2,JBASE+2,-NZAG)* &
                & (CYM(IBASE+2,JBASE+2,-NZAG)+1.d0/DY(JBASE+2)/(REXT+RSID+1.d0/HOAV))
              C(IBASE+2,JBASE+2,-NZAG)=0.
              R(IBASE+2,JBASE+2,-NZAG)=F*CONST(IBASE+2,JBASE+2,-NZAG)*            &
              & (CXM(IBASE+2,JBASE+2,-NZAG)*U(IBASE+2-1,JBASE+2,-NZAG)-           &
              & (CXM(IBASE+2,JBASE+2,-NZAG)+1.d0/DX(IBASE+2)/(REXT+RSID+1.d0/HOAV))*  &
              & U(IBASE+2,JBASE+2,-NZAG)-CZP(IBASE+2,JBASE+2,-NZAG)*              &
              & U(IBASE+2,JBASE+2,-NZAG)+CZP(IBASE+2,JBASE+2,-NZAG)*              &
              & U(IBASE+2,JBASE+2,-NZAG+1))+U(IBASE+2,JBASE+2,-NZAG)+F*           &
              & CONST(IBASE+2,JBASE+2,-NZAG)*(TDBAV/DX(IBASE+2)/(REXT+RSID+1.d0/    &
              & HOAV)+RSOLVAV/DX(IBASE+2)-(0.88d0*SIGMA*(UEXT(IBASE+2,JBASE+2,-NZAG)+ &
              & 273.15d0)**4)/DX(IBASE+2))+(3.d0-2.d0*F)*CONST(IBASE+2,JBASE+2,-NZAG)* &
              & (TDBAV/DY(JBASE+2)/(REXT+RSID+1.d0/HOAV)+RSOLVAV/DY(JBASE+2)-       &
              & (0.88d0*SIGMA*(UEXT(IBASE+2,JBASE+2,-NZAG)+273.15d0)**4)/            &
              & DY(JBASE+2))

              N=JBASE+2-0+1
              L=1
              DO COUNT2=0,JBASE+2
                AA(L)=A(IBASE+2,COUNT2,-NZAG)
                BB(L)=B(IBASE+2,COUNT2,-NZAG)
                CC(L)=C(IBASE+2,COUNT2,-NZAG)
                RR(L)=R(IBASE+2,COUNT2,-NZAG)
                L=L+1
              END DO
              CALL TRIDI3D (AA,BB,CC,RR,N,X)
              L=0
              DO COUNT2=0,JBASE+2
                V(IBASE+2,COUNT2,-NZAG)=X(L+1)
                L=L+1
                QEXT(IBASE+2,COUNT2,-NZAG)=(TDBAV-V(IBASE+2,COUNT2,-NZAG))/       &
                      & (REXT+RSID+1.d0/HOAV)
                VEXT(IBASE+2,COUNT2,-NZAG)=QEXT(IBASE+2,COUNT2,-NZAG)*            &
                      & (REXT+RSID)+V(IBASE+2,COUNT2,-NZAG)
              END DO

!***  SECTION 2:  ABOVE-GRADE FOUNDATION WALL, PARALLEL TO Y-AXIS
!***  TOP INSIDE WALL CELLS
              IF (TBAV.GT.U(IBASE,0,-NZAG+2)) THEN
                HINZ=HIN(4)
              ELSE
                HINZ=HIN(5)
              END IF
              A(IBASE,0,-NZAG+2)=0.
              B(IBASE,0,-NZAG+2)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,0,-NZAG+2)*             &
                & CYP(IBASE,0,-NZAG+2)
              C(IBASE,0,-NZAG+2)=(3.d0-2.d0*F)*CONST(IBASE,0,-NZAG+2)*                &
                & (-CYP(IBASE,0,-NZAG+2))
              R(IBASE,0,-NZAG+2)=F*CONST(IBASE,0,-NZAG+2)*(-(1.d0/DX(IBASE)/        &
              & (RINT+1.d0/HIN(6))+CXP(IBASE,0,-NZAG+2))*U(IBASE,0,-NZAG+2)+        &
              & CXP(IBASE,0,-NZAG+2)*U(IBASE+1,0,-NZAG+2)-(1.d0/DZ(-NZAG+2)/(1.d0/    &
              & HINZ+RSILL+DZ(-NZAG+2)/2.d0/TCON(MTYPE(IBASE,0,-NZAG+2)))+          &
              & CZP(IBASE,0,-NZAG+2))*U(IBASE,0,-NZAG+2)+CZP(IBASE,0,-NZAG+2)*    &
              & U(IBASE,0,-NZAG+2+1))+U(IBASE,0,-NZAG+2)+F*CONST(IBASE,0,-NZAG+2)* &
              & (TBAV/DX(IBASE)/(RINT+1.d0/HIN(6))+TBAV/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+ &
              & DZ(-NZAG+2)/2.d0/TCON(MTYPE(IBASE,0,-NZAG+2))))
              DO COUNT2=1,JBASE-1
                IF (TBAV.GT.U(IBASE,COUNT2,-NZAG+2)) THEN
                  HINZ=HIN(4)
                ELSE
                  HINZ=HIN(5)
                END IF
                A(IBASE,COUNT2,-NZAG+2)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,-NZAG+2)*    &
                      & (-CYM(IBASE,COUNT2,-NZAG+2))
                B(IBASE,COUNT2,-NZAG+2)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,COUNT2,-NZAG+2)* &
                      & (CYM(IBASE,COUNT2,-NZAG+2)+CYP(IBASE,COUNT2,-NZAG+2))
                C(IBASE,COUNT2,-NZAG+2)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,-NZAG+2)*    &
                      & (-CYP(IBASE,COUNT2,-NZAG+2))
                R(IBASE,COUNT2,-NZAG+2)=F*CONST(IBASE,COUNT2,-NZAG+2)*(-(1.d0/      &
                & DX(IBASE)/(RINT+1.d0/HIN(6))+CXP(IBASE,COUNT2,-NZAG+2))*          &
                & U(IBASE,COUNT2,-NZAG+2)+CXP(IBASE,COUNT2,-NZAG+2)*              &
                & U(IBASE+1,COUNT2,-NZAG+2)-(1.d0/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+       &
                & DZ(-NZAG+2)/2.d0/TCON(MTYPE(IBASE,COUNT2,-NZAG+2)))+              &
                & CZP(IBASE,COUNT2,-NZAG+2))*U(IBASE,COUNT2,-NZAG+2)+             &
                & CZP(IBASE,COUNT2,-NZAG+2)*U(IBASE,COUNT2,-NZAG+2+1))+           &
                & U(IBASE,COUNT2,-NZAG+2)+F*CONST(IBASE,COUNT2,-NZAG+2)*          &
                & (TBAV/DX(IBASE)/(RINT+1.d0/HIN(6))+TBAV/DZ(-NZAG+2)/(1.d0/HINZ+     &
                & RSILL+DZ(-NZAG+2)/2.d0/TCON(MTYPE(IBASE,COUNT2,-NZAG+2))))
              END DO

!***  INSIDE CORNER CELL AND TOP CENTER WALL CELL
              DO COUNT2=JBASE,JBASE+1
                IF (TBAV.GT.U(IBASE,COUNT2,-NZAG+2)) THEN
                  HINZ=HIN(4)
                ELSE
                  HINZ=HIN(5)
                END IF
                A(IBASE,COUNT2,-NZAG+2)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,-NZAG+2)*    &
                    & (-CYM(IBASE,COUNT2,-NZAG+2))
                B(IBASE,COUNT2,-NZAG+2)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,COUNT2,-NZAG+2)* &
                    & (CYM(IBASE,COUNT2,-NZAG+2)+CYP(IBASE,COUNT2,-NZAG+2))
                C(IBASE,COUNT2,-NZAG+2)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,-NZAG+2)*    &
                    & (-CYP(IBASE,COUNT2,-NZAG+2))
                R(IBASE,COUNT2,-NZAG+2)=F*CONST(IBASE,COUNT2,-NZAG+2)*            &
                & (CXM(IBASE,COUNT2,-NZAG+2)*U(IBASE-1,COUNT2,-NZAG+2)-           &
                & (CXM(IBASE,COUNT2,-NZAG+2)+CXP(IBASE,COUNT2,-NZAG+2))*          &
                & U(IBASE,COUNT2,-NZAG+2)+CXP(IBASE,COUNT2,-NZAG+2)*              &
                & U(IBASE+1,COUNT2,-NZAG+2)-(1.d0/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+       &
                & DZ(-NZAG+2)/2.d0/TCON(MTYPE(IBASE,COUNT2,-NZAG+2)))+              &
                & CZP(IBASE,COUNT2,-NZAG+2))*U(IBASE,COUNT2,-NZAG+2)+             &
                & CZP(IBASE,COUNT2,-NZAG+2)*U(IBASE,COUNT2,-NZAG+2+1))+           &
                & U(IBASE,COUNT2,-NZAG+2)+F*CONST(IBASE,COUNT2,-NZAG+2)*          &
                & (TBAV/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+DZ(-NZAG+2)/2.d0/                &
                & TCON(MTYPE(IBASE,COUNT2,-NZAG+2))))
              END DO

!***  TOP OUTSIDE WALL CELL
              A(IBASE,JBASE+2,-NZAG+2)=(3.d0-2.d0*F)*CONST(IBASE,JBASE+2,-NZAG+2)*    &
                & (-CYM(IBASE,JBASE+2,-NZAG+2))
              B(IBASE,JBASE+2,-NZAG+2)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,JBASE+2,-NZAG+2)* &
                & (CYM(IBASE,JBASE+2,-NZAG+2)+1.d0/DY(JBASE+2)/(REXT+RSID+1.d0/HOAV))
              C(IBASE,JBASE+2,-NZAG+2)=0.
              R(IBASE,JBASE+2,-NZAG+2)=F*CONST(IBASE,JBASE+2,-NZAG+2)*            &
              & (CXM(IBASE,JBASE+2,-NZAG+2)*U(IBASE-1,JBASE+2,-NZAG+2)-           &
              & (CXM(IBASE,JBASE+2,-NZAG+2)+CXP(IBASE,JBASE+2,-NZAG+2))*          &
              & U(IBASE,JBASE+2,-NZAG+2)+CXP(IBASE,JBASE+2,-NZAG+2)*              &
              & U(IBASE+1,JBASE+2,-NZAG+2)+CZM(IBASE,JBASE+2,-NZAG+2)*            &
              & U(IBASE,JBASE+2,-NZAG+2-1)-(CZM(IBASE,JBASE+2,-NZAG+2)+           &
              & CZP(IBASE,JBASE+2,-NZAG+2))*U(IBASE,JBASE+2,-NZAG+2)+             &
              & CZP(IBASE,JBASE+2,-NZAG+2)*U(IBASE,JBASE+2,-NZAG+2+1))+           &
              & U(IBASE,JBASE+2,-NZAG+2)+(3.d0-2.d0*F)*CONST(IBASE,JBASE+2,-NZAG+2)*  &
              & (TDBAV/DY(JBASE+2)/(REXT+RSID+1.d0/HOAV)+RSOLVAV/DY(JBASE+2)-       &
              & (0.88d0*SIGMA*(UEXT(IBASE,JBASE+2,-NZAG+2)+273.15d0)**4)/DY(JBASE+2))
              N=JBASE+2-0+1
              L=1
              DO COUNT2=0,JBASE+2
                AA(L)=A(IBASE,COUNT2,-NZAG+2)
                BB(L)=B(IBASE,COUNT2,-NZAG+2)
                CC(L)=C(IBASE,COUNT2,-NZAG+2)
                RR(L)=R(IBASE,COUNT2,-NZAG+2)
                L=L+1
              END DO
              CALL TRIDI3D (AA,BB,CC,RR,N,X)
              L=0
              DO COUNT2=0,JBASE+2
                V(IBASE,COUNT2,-NZAG+2)=X(L+1)
                L=L+1
              END DO
              QEXT(IBASE,JBASE+2,-NZAG+2)=(TDBAV-V(IBASE,JBASE+2,-NZAG+2))/       &
                    & (REXT+RSID+1.d0/HOAV)
              VEXT(IBASE,JBASE+2,-NZAG+2)=QEXT(IBASE,JBASE+2,-NZAG+2)*            &
                    & (REXT+RSID)+V(IBASE,JBASE+2,-NZAG+2)

!***  TOP CENTER WALL CELLS
              IF (TBAV.GT.U(IBASE+1,0,-NZAG+2)) THEN
                HINZ=HIN(4)
              ELSE
                HINZ=HIN(5)
              END IF
              A(IBASE+1,0,-NZAG+2)=0.
              B(IBASE+1,0,-NZAG+2)=1.d0+(3.d0-2.d0*F)*CONST(IBASE+1,0,-NZAG+2)*       &
                 & CYP(IBASE+1,0,-NZAG+2)
              C(IBASE+1,0,-NZAG+2)=(3.d0-2.d0*F)*CONST(IBASE+1,0,-NZAG+2)*          &
                 & (-CYP(IBASE+1,0,-NZAG+2))
              R(IBASE+1,0,-NZAG+2)=F*CONST(IBASE+1,0,-NZAG+2)*                  &
              & (CXM(IBASE+1,0,-NZAG+2)*U(IBASE+1-1,0,-NZAG+2)-                 &
              & (CXM(IBASE+1,0,-NZAG+2)+CXP(IBASE+1,0,-NZAG+2))*                &
              & U(IBASE+1,0,-NZAG+2)+CXP(IBASE+1,0,-NZAG+2)*                    &
              & U(IBASE+1+1,0,-NZAG+2)-(1.d0/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+          &
              & DZ(-NZAG+2)/2.d0/TCON(MTYPE(IBASE+1,0,-NZAG+2)))+                 &
              & CZP(IBASE+1,0,-NZAG+2))*U(IBASE+1,0,-NZAG+2)+                   &
              & CZP(IBASE+1,0,-NZAG+2)*U(IBASE+1,0,-NZAG+2+1))+                 &
              & U(IBASE+1,0,-NZAG+2)+F*CONST(IBASE+1,0,-NZAG+2)*(TBAV/          &
              & DZ(-NZAG+2)/(1.d0/HINZ+RSILL+DZ(-NZAG+2)/2.d0/                      &
              & TCON(MTYPE(IBASE+1,0,-NZAG+2))))
              DO COUNT2=1,JBASE+1
                IF (TBAV.GT.U(IBASE+1,COUNT2,-NZAG+2)) THEN
                  HINZ=HIN(4)
                ELSE
                  HINZ=HIN(5)
                END IF
                A(IBASE+1,COUNT2,-NZAG+2)=(3.d0-2.d0*F)*CONST(IBASE+1,COUNT2,-NZAG+2)* &
                    & (-CYM(IBASE+1,COUNT2,-NZAG+2))
                B(IBASE+1,COUNT2,-NZAG+2)=1.d0+(3.d0-2.d0*F)*CONST(IBASE+1,COUNT2,-NZAG+2)* &
                    & (CYM(IBASE+1,COUNT2,-NZAG+2)+CYP(IBASE+1,COUNT2,-NZAG+2))
                C(IBASE+1,COUNT2,-NZAG+2)=(3.d0-2.d0*F)*CONST(IBASE+1,COUNT2,-NZAG+2)* &
                    & (-CYP(IBASE+1,COUNT2,-NZAG+2))
                R(IBASE+1,COUNT2,-NZAG+2)=F*CONST(IBASE+1,COUNT2,-NZAG+2)*      &
                & (CXM(IBASE+1,COUNT2,-NZAG+2)*U(IBASE+1-1,COUNT2,-NZAG+2)-     &
                & (CXM(IBASE+1,COUNT2,-NZAG+2)+CXP(IBASE+1,COUNT2,-NZAG+2))*    &
                & U(IBASE+1,COUNT2,-NZAG+2)+CXP(IBASE+1,COUNT2,-NZAG+2)*        &
                & U(IBASE+1+1,COUNT2,-NZAG+2)-(1.d0/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+   &
                & DZ(-NZAG+2)/2.d0/TCON(MTYPE(IBASE+1,COUNT2,-NZAG+2)))+          &
                & CZP(IBASE+1,COUNT2,-NZAG+2))*U(IBASE+1,COUNT2,-NZAG+2)+       &
                & CZP(IBASE+1,COUNT2,-NZAG+2)*U(IBASE+1,COUNT2,-NZAG+2+1))+     &
                & U(IBASE+1,COUNT2,-NZAG+2)+F*CONST(IBASE+1,COUNT2,-NZAG+2)*    &
                & (TBAV/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+DZ(-NZAG+2)/2.d0/              &
                & TCON(MTYPE(IBASE+1,COUNT2,-NZAG+2))))
              END DO

!***  TOP OUTSIDE WALL CELL
              A(IBASE+1,JBASE+2,-NZAG+2)=(3.d0-2.d0*F)*CONST(IBASE+1,JBASE+2,-NZAG+2)* &
                & (-CYM(IBASE+1,JBASE+2,-NZAG+2))
              B(IBASE+1,JBASE+2,-NZAG+2)=1.d0+(3.d0-2.d0*F)*CONST(IBASE+1,JBASE+2,-NZAG+2)* &
                & (CYM(IBASE+1,JBASE+2,-NZAG+2)+1.d0/DY(JBASE+2)/(REXT+RSID+1.d0/HOAV))
              C(IBASE+1,JBASE+2,-NZAG+2)=0.
              R(IBASE+1,JBASE+2,-NZAG+2)=F*CONST(IBASE+1,JBASE+2,-NZAG+2)*        &
              & (CXM(IBASE+1,JBASE+2,-NZAG+2)*U(IBASE+1-1,JBASE+2,-NZAG+2)-       &
              & (CXM(IBASE+1,JBASE+2,-NZAG+2)+CXP(IBASE+1,JBASE+2,-NZAG+2))*      &
              & U(IBASE+1,JBASE+2,-NZAG+2)+CXP(IBASE+1,JBASE+2,-NZAG+2)*          &
              & U(IBASE+1+1,JBASE+2,-NZAG+2)+CZM(IBASE+1,JBASE+2,-NZAG+2)*        &
              & U(IBASE+1,JBASE+2,-NZAG+2-1)-(CZM(IBASE+1,JBASE+2,-NZAG+2)+       &
              & CZP(IBASE+1,JBASE+2,-NZAG+2))*U(IBASE+1,JBASE+2,-NZAG+2)+         &
              & CZP(IBASE+1,JBASE+2,-NZAG+2)*U(IBASE+1,JBASE+2,-NZAG+2+1))+       &
              & U(IBASE+1,JBASE+2,-NZAG+2)+(3.d0-2.d0*F)*CONST(IBASE+1,JBASE+2,-NZAG+2)* &
              & (TDBAV/DY(JBASE+2)/(REXT+RSID+1.d0/HOAV)+RSOLVAV/DY(JBASE+2)-       &
              & (0.88d0*SIGMA*(UEXT(IBASE+1,JBASE+2,-NZAG+2)+273.15d0)**4)/DY(JBASE+2))
              N=JBASE+2-0+1
              L=1
              DO COUNT2=0,JBASE+2
                AA(L)=A(IBASE+1,COUNT2,-NZAG+2)
                BB(L)=B(IBASE+1,COUNT2,-NZAG+2)
                CC(L)=C(IBASE+1,COUNT2,-NZAG+2)
                RR(L)=R(IBASE+1,COUNT2,-NZAG+2)
                L=L+1
              END DO
              CALL TRIDI3D (AA,BB,CC,RR,N,X)
              L=0
              DO COUNT2=0,JBASE+2
                V(IBASE+1,COUNT2,-NZAG+2)=X(L+1)
                L=L+1
              END DO
              QEXT(IBASE+1,JBASE+2,-NZAG+2)=(TDBAV-V(IBASE+1,JBASE+2,-NZAG+2))/   &
                & (REXT+RSID+1.d0/HOAV)
              VEXT(IBASE+1,JBASE+2,-NZAG+2)=QEXT(IBASE+1,JBASE+2,-NZAG+2)*        &
                & (REXT+RSID)+V(IBASE+1,JBASE+2,-NZAG+2)

!***  TOP OUTSIDE WALL CELLS
              A(IBASE+2,0,-NZAG+2)=0.
              B(IBASE+2,0,-NZAG+2)=1.d0+(3.d0-2.d0*F)*CONST(IBASE+2,0,-NZAG+2)*         &
                & CYP(IBASE+2,0,-NZAG+2)
              C(IBASE+2,0,-NZAG+2)=(3.d0-2.d0*F)*CONST(IBASE+2,0,-NZAG+2)*            &
                & (-CYP(IBASE+2,0,-NZAG+2))
              R(IBASE+2,0,-NZAG+2)=F*CONST(IBASE+2,0,-NZAG+2)*                    &
              & (CXM(IBASE+2,0,-NZAG+2)*U(IBASE+2-1,0,-NZAG+2)-                   &
              & (CXM(IBASE+2,0,-NZAG+2)+1.d0/DX(IBASE+2)/(REXT+RSID+1.d0/HOAV))*      &
              & U(IBASE+2,0,-NZAG+2)+CZM(IBASE+2,0,-NZAG+2)*                      &
              & U(IBASE+2,0,-NZAG+2-1)-(CZM(IBASE+2,0,-NZAG+2)+                   &
              & CZP(IBASE+2,0,-NZAG+2))*U(IBASE+2,0,-NZAG+2)+                     &
              & CZP(IBASE+2,0,-NZAG+2)*U(IBASE+2,0,-NZAG+2+1))+                   &
              & U(IBASE+2,0,-NZAG+2)+F*CONST(IBASE+2,0,-NZAG+2)*(TDBAV/           &
              & DX(IBASE+2)/(REXT+RSID+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-(0.88d0*SIGMA*  &
              & (U(IBASE+2,0,-NZAG+2)+273.15d0)**4)/DX(IBASE+2))
              DO COUNT2=1,JBASE+1
                A(IBASE+2,COUNT2,-NZAG+2)=(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,-NZAG+2)* &
                      & (-CYM(IBASE+2,COUNT2,-NZAG+2))
                B(IBASE+2,COUNT2,-NZAG+2)=1.d0+(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,-NZAG+2)* &
                      & (CYM(IBASE+2,COUNT2,-NZAG+2)+CYP(IBASE+2,COUNT2,-NZAG+2))
                C(IBASE+2,COUNT2,-NZAG+2)=(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,-NZAG+2)* &
                      & (-CYP(IBASE+2,COUNT2,-NZAG+2))
                R(IBASE+2,COUNT2,-NZAG+2)=F*CONST(IBASE+2,COUNT2,-NZAG+2)*        &
                & (CXM(IBASE+2,COUNT2,-NZAG+2)*U(IBASE+2-1,COUNT2,-NZAG+2)-       &
                & (CXM(IBASE+2,COUNT2,-NZAG+2)+1.d0/DX(IBASE+2)/(REXT+RSID+1.d0/HOAV))* &
                & U(IBASE+2,COUNT2,-NZAG+2)+CZM(IBASE+2,COUNT2,-NZAG+2)*          &
                & U(IBASE+2,COUNT2,-NZAG+2-1)-(CZM(IBASE+2,COUNT2,-NZAG+2)+       &
                & CZP(IBASE+2,COUNT2,-NZAG+2))*U(IBASE+2,COUNT2,-NZAG+2)+         &
                & CZP(IBASE+2,COUNT2,-NZAG+2)*U(IBASE+2,COUNT2,-NZAG+2+1))+       &
                & U(IBASE+2,COUNT2,-NZAG+2)+F*CONST(IBASE+2,COUNT2,-NZAG+2)*      &
                & (TDBAV/DX(IBASE+2)/(REXT+RSID+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-     &
                & (0.88d0*SIGMA*(U(IBASE+2,COUNT2,-NZAG+2)+273.15d0)**4)/DX(IBASE+2))
              END DO

!***  TOP OUTER CORNER CELL
              A(IBASE+2,JBASE+2,-NZAG+2)=(3.d0-2.d0*F)*CONST(IBASE+2,JBASE+2,-NZAG+2)* &
                & (-CYM(IBASE+2,JBASE+2,-NZAG+2))
              B(IBASE+2,JBASE+2,-NZAG+2)=1.d0+(3.d0-2.d0*F)*                            &
                & CONST(IBASE+2,JBASE+2,-NZAG+2)*(CYM(IBASE+2,JBASE+2,-NZAG+2)+   &
                & 1.d0/DY(JBASE+2)/(REXT+RSID+1.d0/HOAV))
              C(IBASE+2,JBASE+2,-NZAG+2)=0.
              R(IBASE+2,JBASE+2,-NZAG+2)=F*CONST(IBASE+2,JBASE+2,-NZAG+2)*        &
              & (CXM(IBASE+2,JBASE+2,-NZAG+2)*U(IBASE+2-1,JBASE+2,-NZAG+2)-       &
              & (CXM(IBASE+2,JBASE+2,-NZAG+2)+1.d0/DX(IBASE+2)/(REXT+RSID+1.d0/HOAV))* &
              & U(IBASE+2,JBASE+2,-NZAG+2)+CZM(IBASE+2,JBASE+2,-NZAG+2)*          &
              & U(IBASE+2,JBASE+2,-NZAG+2-1)-(CZM(IBASE+2,JBASE+2,-NZAG+2)+       &
              & CZP(IBASE+2,JBASE+2,-NZAG+2))*U(IBASE+2,JBASE+2,-NZAG+2)+         &
              & CZP(IBASE+2,JBASE+2,-NZAG+2)*U(IBASE+2,JBASE+2,-NZAG+2+1))+       &
              & U(IBASE+2,JBASE+2,-NZAG+2)+F*CONST(IBASE+2,JBASE+2,-NZAG+2)*      &
              & (TDBAV/DX(IBASE+2)/(REXT+RSID+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-       &
              & (0.88d0*SIGMA*(U(IBASE+2,JBASE+2,-NZAG+2)+273.15d0)**4)/DX(IBASE+2))+ &
              & (3.d0-2.d0*F)*CONST(IBASE+2,JBASE+2,-NZAG+2)*(TDBAV/DY(JBASE+2)/      &
              & (REXT+RSID+1.d0/HOAV)+RSOLVAV/DY(JBASE+2)-(0.88d0*SIGMA*              &
              & (U(IBASE+2,JBASE+2,-NZAG+2)+273.15d0)**4)/DY(JBASE+2))
              N=JBASE+2-0+1
              L=1
              DO COUNT2=0,JBASE+2
                AA(L)=A(IBASE+2,COUNT2,-NZAG+2)
                BB(L)=B(IBASE+2,COUNT2,-NZAG+2)
                CC(L)=C(IBASE+2,COUNT2,-NZAG+2)
                RR(L)=R(IBASE+2,COUNT2,-NZAG+2)
                L=L+1
              END DO
              CALL TRIDI3D (AA,BB,CC,RR,N,X)
              L=0
              DO COUNT2=0,JBASE+2
                V(IBASE+2,COUNT2,-NZAG+2)=X(L+1)
                L=L+1
                QEXT(IBASE+2,COUNT2,-NZAG+2)=(TDBAV-V(IBASE+2,COUNT2,-NZAG+2))/   &
                      & (REXT+RSID+1.d0/HOAV)
                VEXT(IBASE+2,COUNT2,-NZAG+2)=QEXT(IBASE+2,COUNT2,-NZAG+2)*        &
                      & (REXT+RSID)+V(IBASE+2,COUNT2,-NZAG+2)
              END DO

!***  INSIDE WALL CELLS
              IF (NZAG.GT.3) THEN
                DO COUNT3=-NZAG+3,-1
                  A(IBASE,0,COUNT3)=0.
                  B(IBASE,0,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,0,COUNT3)*         &
                      & CYP(IBASE,0,COUNT3)
                  C(IBASE,0,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,0,COUNT3)*            &
                      & (-CYP(IBASE,0,COUNT3))
                  R(IBASE,0,COUNT3)=F*CONST(IBASE,0,COUNT3)*(-(1.d0/DX(IBASE)/(RINT+ &
                  & 1.d0/HIN(6))+CXP(IBASE,0,COUNT3))*U(IBASE,0,COUNT3)+          &
                  & CXP(IBASE,0,COUNT3)*U(IBASE+1,0,COUNT3)+CZM(IBASE,0,COUNT3)* &
                  & U(IBASE,0,COUNT3-1)-(CZM(IBASE,0,COUNT3)+CZP(IBASE,0,COUNT3))* &
                  & U(IBASE,0,COUNT3)+CZP(IBASE,0,COUNT3)*U(IBASE,0,COUNT3+1))+ &
                  & U(IBASE,0,COUNT3)+F*CONST(IBASE,0,COUNT3)*(TBAV/DX(IBASE)/  &
                  & (RINT+1.d0/HIN(6)))
                  DO COUNT2=1,JBASE-1
                    A(IBASE,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)* &
                        & (-CYM(IBASE,COUNT2,COUNT3))
                    B(IBASE,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)* &
                        & (CYM(IBASE,COUNT2,COUNT3)+CYP(IBASE,COUNT2,COUNT3))
                    C(IBASE,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)* &
                        & (-CYP(IBASE,COUNT2,COUNT3))
                    R(IBASE,COUNT2,COUNT3)=F*CONST(IBASE,COUNT2,COUNT3)*(-(1.d0/  &
                    & DX(IBASE)/(RINT+1.d0/HIN(6))+CXP(IBASE,COUNT2,COUNT3))*     &
                    & U(IBASE,COUNT2,COUNT3)+CXP(IBASE,COUNT2,COUNT3)*          &
                    & U(IBASE+1,COUNT2,COUNT3)+CZM(IBASE,COUNT2,COUNT3)*        &
                    & U(IBASE,COUNT2,COUNT3-1)-(CZM(IBASE,COUNT2,COUNT3)+       &
                    & CZP(IBASE,COUNT2,COUNT3))*U(IBASE,COUNT2,COUNT3)+         &
                    & CZP(IBASE,COUNT2,COUNT3)*U(IBASE,COUNT2,COUNT3+1))+       &
                    & U(IBASE,COUNT2,COUNT3)+F*CONST(IBASE,COUNT2,COUNT3)*      &
                    & (TBAV/DX(IBASE)/(RINT+1.d0/HIN(6)))
                  END DO

!***  INSIDE CORNER WALL CELL AND CENTER WALL CELL
                  DO COUNT2=JBASE,JBASE+1
                    A(IBASE,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)*  &
                      & (-CYM(IBASE,COUNT2,COUNT3))
                    B(IBASE,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)* &
                      & (CYM(IBASE,COUNT2,COUNT3)+CYP(IBASE,COUNT2,COUNT3))
                    C(IBASE,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)*  &
                      & (-CYP(IBASE,COUNT2,COUNT3))
                    R(IBASE,COUNT2,COUNT3)=F*CONST(IBASE,COUNT2,COUNT3)*          &
                    & (CXM(IBASE,COUNT2,COUNT3)*U(IBASE-1,COUNT2,COUNT3)-         &
                    & (CXM(IBASE,COUNT2,COUNT3)+CXP(IBASE,COUNT2,COUNT3))*        &
                    & U(IBASE,COUNT2,COUNT3)+CXP(IBASE,COUNT2,COUNT3)*            &
                    & U(IBASE+1,COUNT2,COUNT3)+CZM(IBASE,COUNT2,COUNT3)*          &
                    & U(IBASE,COUNT2,COUNT3-1)-(CZM(IBASE,COUNT2,COUNT3)+         &
                    & CZP(IBASE,COUNT2,COUNT3))*U(IBASE,COUNT2,COUNT3)+           &
                    & CZP(IBASE,COUNT2,COUNT3)*U(IBASE,COUNT2,COUNT3+1))+         &
                    & U(IBASE,COUNT2,COUNT3)
                  END DO

!***  OUTSIDE WALL CELL
                  A(IBASE,JBASE+2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,JBASE+2,COUNT3)*  &
                      & (-CYM(IBASE,JBASE+2,COUNT3))
                  B(IBASE,JBASE+2,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,JBASE+2,COUNT3)* &
                      & (CYM(IBASE,JBASE+2,COUNT3)+1.d0/DY(JBASE+2)/(REXT+1.d0/HOAV))
                  C(IBASE,JBASE+2,COUNT3)=0.
                  R(IBASE,JBASE+2,COUNT3)=F*CONST(IBASE,JBASE+2,COUNT3)*          &
                  & (CXM(IBASE,JBASE+2,COUNT3)*U(IBASE-1,JBASE+2,COUNT3)-         &
                  & (CXM(IBASE,JBASE+2,COUNT3)+CXP(IBASE,JBASE+2,COUNT3))*        &
                  & U(IBASE,JBASE+2,COUNT3)+CXP(IBASE,JBASE+2,COUNT3)*            &
                  & U(IBASE+1,JBASE+2,COUNT3)+CZM(IBASE,JBASE+2,COUNT3)*          &
                  & U(IBASE,JBASE+2,COUNT3-1)-(CZM(IBASE,JBASE+2,COUNT3)+         &
                  & CZP(IBASE,JBASE+2,COUNT3))*U(IBASE,JBASE+2,COUNT3)+           &
                  & CZP(IBASE,JBASE+2,COUNT3)*U(IBASE,JBASE+2,COUNT3+1))+         &
                  & U(IBASE,JBASE+2,COUNT3)+(3.d0-2.d0*F)*CONST(IBASE,JBASE+2,COUNT3)*  &
                  & (TDBAV/DY(JBASE+2)/(REXT+1.d0/HOAV)+RSOLVAV/DY(JBASE+2)-        &
                  & (0.88d0*SIGMA*(UEXT(IBASE,JBASE+2,COUNT3)+273.15d0)**4)/DY(JBASE+2))
                  N=JBASE+2-0+1
                  L=1
                  DO COUNT2=0,JBASE+2
                    AA(L)=A(IBASE,COUNT2,COUNT3)
                    BB(L)=B(IBASE,COUNT2,COUNT3)
                    CC(L)=C(IBASE,COUNT2,COUNT3)
                    RR(L)=R(IBASE,COUNT2,COUNT3)
                    L=L+1
                  END DO
                  CALL TRIDI3D (AA,BB,CC,RR,N,X)
                  L=0
                  DO COUNT2=0,JBASE+2
                    V(IBASE,COUNT2,COUNT3)=X(L+1)
                    L=L+1
                  END DO
                  QEXT(IBASE,JBASE+2,COUNT3)=(TDBAV-V(IBASE,JBASE+2,COUNT3))/     &
                      & (REXT+RSID+1.d0/HOAV)
                  VEXT(IBASE,JBASE+2,COUNT3)=QEXT(IBASE,JBASE+2,COUNT3)*          &
                      & (REXT+RSID)+V(IBASE,JBASE+2,COUNT3)

!***  CENTER WALL CELLS
                  A(IBASE+1,0,COUNT3)=0.
                  B(IBASE+1,0,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(IBASE+1,0,COUNT3)*       &
                      & CYP(IBASE+1,0,COUNT3)
                  C(IBASE+1,0,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE+1,0,COUNT3)*          &
                      & (-CYP(IBASE+1,0,COUNT3))
                  R(IBASE+1,0,COUNT3)=F*CONST(IBASE+1,0,COUNT3)*                  &
                  & (CXM(IBASE+1,0,COUNT3)*U(IBASE+1-1,0,COUNT3)-                 &
                  & (CXM(IBASE+1,0,COUNT3)+CXP(IBASE+1,0,COUNT3))*                &
                  & U(IBASE+1,0,COUNT3)+CXP(IBASE+1,0,COUNT3)*U(IBASE+1+1,0,COUNT3)+ &
                  & CZM(IBASE+1,0,COUNT3)*U(IBASE+1,0,COUNT3-1)-                  &
                  & (CZM(IBASE+1,0,COUNT3)+CZP(IBASE+1,0,COUNT3))*                &
                  & U(IBASE+1,0,COUNT3)+CZP(IBASE+1,0,COUNT3)*                    &
                  & U(IBASE+1,0,COUNT3+1))+U(IBASE+1,0,COUNT3)
                  DO COUNT2=1,JBASE+1
                    A(IBASE+1,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE+1,COUNT2,COUNT3)* &
                      & (-CYM(IBASE+1,COUNT2,COUNT3))
                    B(IBASE+1,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*                        &
                      & CONST(IBASE+1,COUNT2,COUNT3)*(CYM(IBASE+1,COUNT2,COUNT3)+ &
                      & CYP(IBASE+1,COUNT2,COUNT3))
                    C(IBASE+1,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE+1,COUNT2,COUNT3)* &
                      & (-CYP(IBASE+1,COUNT2,COUNT3))
                    R(IBASE+1,COUNT2,COUNT3)=F*CONST(IBASE+1,COUNT2,COUNT3)*      &
                    & (CXM(IBASE+1,COUNT2,COUNT3)*U(IBASE+1-1,COUNT2,COUNT3)-     &
                    & (CXM(IBASE+1,COUNT2,COUNT3)+CXP(IBASE+1,COUNT2,COUNT3))*    &
                    & U(IBASE+1,COUNT2,COUNT3)+CXP(IBASE+1,COUNT2,COUNT3)*        &
                    & U(IBASE+1+1,COUNT2,COUNT3)+CZM(IBASE+1,COUNT2,COUNT3)*      &
                    & U(IBASE+1,COUNT2,COUNT3-1)-(CZM(IBASE+1,COUNT2,COUNT3)+     &
                    & CZP(IBASE+1,COUNT2,COUNT3))*U(IBASE+1,COUNT2,COUNT3)+       &
                    & CZP(IBASE+1,COUNT2,COUNT3)*U(IBASE+1,COUNT2,COUNT3+1))+     &
                    & U(IBASE+1,COUNT2,COUNT3)
                  END DO

!***  OUTSIDE WALL CELL
                  A(IBASE+1,JBASE+2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE+1,JBASE+2,COUNT3)* &
                      & (-CYM(IBASE+1,JBASE+2,COUNT3))
                  B(IBASE+1,JBASE+2,COUNT3)=1.d0+(3.d0-2.d0*F)*                         &
                      & CONST(IBASE+1,JBASE+2,COUNT3)*(CYM(IBASE+1,JBASE+2,COUNT3)+ &
                      & 1.d0/DY(JBASE+2)/(REXT+1.d0/HOAV))
                  C(IBASE+1,JBASE+2,COUNT3)=0.
                  R(IBASE+1,JBASE+2,COUNT3)=F*CONST(IBASE+1,JBASE+2,COUNT3)*      &
                  & (CXM(IBASE+1,JBASE+2,COUNT3)*U(IBASE+1-1,JBASE+2,COUNT3)-     &
                  & (CXM(IBASE+1,JBASE+2,COUNT3)+CXP(IBASE+1,JBASE+2,COUNT3))*    &
                  & U(IBASE+1,JBASE+2,COUNT3)+CXP(IBASE+1,JBASE+2,COUNT3)*        &
                  & U(IBASE+1+1,JBASE+2,COUNT3)+CZM(IBASE+1,JBASE+2,COUNT3)*      &
                  & U(IBASE+1,JBASE+2,COUNT3-1)-(CZM(IBASE+1,JBASE+2,COUNT3)+     &
                  & CZP(IBASE+1,JBASE+2,COUNT3))*U(IBASE+1,JBASE+2,COUNT3)+       &
                  & CZP(IBASE+1,JBASE+2,COUNT3)*U(IBASE+1,JBASE+2,COUNT3+1))+     &
                  & U(IBASE+1,JBASE+2,COUNT3)+(3.d0-2.d0*F)*                          &
                  & CONST(IBASE+1,JBASE+2,COUNT3)*(TDBAV/DY(JBASE+2)/(REXT+1.d0/HOAV)+ &
                  & RSOLVAV/DY(JBASE+2)-(0.88d0*SIGMA*(UEXT(IBASE+1,JBASE+2,COUNT3)+ &
                  & 273.15d0)**4)/DY(JBASE+2))
                  N=JBASE+2-0+1
                  L=1
                  DO COUNT2=0,JBASE+2
                    AA(L)=A(IBASE+1,COUNT2,COUNT3)
                    BB(L)=B(IBASE+1,COUNT2,COUNT3)
                    CC(L)=C(IBASE+1,COUNT2,COUNT3)
                    RR(L)=R(IBASE+1,COUNT2,COUNT3)
                    L=L+1
                  END DO
                  CALL TRIDI3D (AA,BB,CC,RR,N,X)
                  L=0
                  DO COUNT2=0,JBASE+2
                    V(IBASE+1,COUNT2,COUNT3)=X(L+1)
                    L=L+1
                  END DO
                  QEXT(IBASE+1,JBASE+2,COUNT3)=(TDBAV-V(IBASE+1,JBASE+2,COUNT3))/ &
                    & (REXT+RSID+1.d0/HOAV)
                  VEXT(IBASE+1,JBASE+2,COUNT3)=QEXT(IBASE+1,JBASE+2,COUNT3)*      &
                    & (REXT+RSID)+V(IBASE+1,JBASE+2,COUNT3)

!***  OUTSIDE WALL CELLS
                  A(IBASE+2,0,COUNT3)=0.
                  B(IBASE+2,0,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(IBASE+2,0,COUNT3)*       &
                     & CYP(IBASE+2,0,COUNT3)
                  C(IBASE+2,0,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE+2,0,COUNT3)*          &
                     & (-CYP(IBASE+2,0,COUNT3))
                  R(IBASE+2,0,COUNT3)=F*CONST(IBASE+2,0,COUNT3)*                  &
                  & (CXM(IBASE+2,0,COUNT3)*U(IBASE+2-1,0,COUNT3)-                 &
                  & (CXM(IBASE+2,0,COUNT3)+1.d0/DX(IBASE+2)/                        &
                  & (REXT+1.d0/HOAV))*U(IBASE+2,0,COUNT3)+CZM(IBASE+2,0,COUNT3)*    &
                  & U(IBASE+2,0,COUNT3-1)-(CZM(IBASE+2,0,COUNT3)+                 &
                  & CZP(IBASE+2,0,COUNT3))*U(IBASE+2,0,COUNT3)+                   &
                  & CZP(IBASE+2,0,COUNT3)*U(IBASE+2,0,COUNT3+1))+                 &
                  & U(IBASE+2,0,COUNT3)+F*CONST(IBASE+2,0,COUNT3)*                &
                  & (TDBAV/DX(IBASE+2)/(REXT+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-        &
                  & (0.88d0*SIGMA*(U(IBASE+2,0,COUNT3)+273.15d0)**4)/DX(IBASE+2))
                  DO COUNT2=1,JBASE+1
                    A(IBASE+2,COUNT2,COUNT3)=(3.d0-2.d0*F)*                           &
                      & CONST(IBASE+2,COUNT2,COUNT3)*(-CYM(IBASE+2,COUNT2,COUNT3))
                    B(IBASE+2,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*                        &
                      & CONST(IBASE+2,COUNT2,COUNT3)*(CYM(IBASE+2,COUNT2,COUNT3)+ &
                      & CYP(IBASE+2,COUNT2,COUNT3))
                    C(IBASE+2,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,COUNT3)* &
                        & (-CYP(IBASE+2,COUNT2,COUNT3))
                    R(IBASE+2,COUNT2,COUNT3)=F*CONST(IBASE+2,COUNT2,COUNT3)*      &
                    & (CXM(IBASE+2,COUNT2,COUNT3)*U(IBASE+2-1,COUNT2,COUNT3)-     &
                    & (CXM(IBASE+2,COUNT2,COUNT3)+1.d0/DX(IBASE+2)/(REXT+1.d0/HOAV))* &
                    & U(IBASE+2,COUNT2,COUNT3)+CZM(IBASE+2,COUNT2,COUNT3)*        &
                    & U(IBASE+2,COUNT2,COUNT3-1)-(CZM(IBASE+2,COUNT2,COUNT3)+     &
                    & CZP(IBASE+2,COUNT2,COUNT3))*U(IBASE+2,COUNT2,COUNT3)+       &
                    & CZP(IBASE+2,COUNT2,COUNT3)*U(IBASE+2,COUNT2,COUNT3+1))+     &
                    & U(IBASE+2,COUNT2,COUNT3)+F*CONST(IBASE+2,COUNT2,COUNT3)*    &
                    & (TDBAV/DX(IBASE+2)/(REXT+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-      &
                    & (0.88d0*SIGMA*(U(IBASE+2,COUNT2,COUNT3)+273.15d0)**4)/DX(IBASE+2))
                  END DO

!***  OUTER CORNER CELL
                  A(IBASE+2,JBASE+2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE+2,JBASE+2,COUNT3)* &
                        & (-CYM(IBASE+2,JBASE+2,COUNT3))
                  B(IBASE+2,JBASE+2,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(IBASE+2,JBASE+2,COUNT3)* &
                        & (CYM(IBASE+2,JBASE+2,COUNT3)+1.d0/DY(JBASE+2)/(REXT+1.d0/HOAV))
                  C(IBASE+2,JBASE+2,COUNT3)=0.
                  R(IBASE+2,JBASE+2,COUNT3)=F*CONST(IBASE+2,JBASE+2,COUNT3)*      &
                  & (CXM(IBASE+2,JBASE+2,COUNT3)*U(IBASE+2-1,JBASE+2,COUNT3)-     &
                  & (CXM(IBASE+2,JBASE+2,COUNT3)+1.d0/DX(IBASE+2)/(REXT+1.d0/HOAV))*  &
                  & U(IBASE+2,JBASE+2,COUNT3)+CZM(IBASE+2,JBASE+2,COUNT3)*        &
                  & U(IBASE+2,JBASE+2,COUNT3-1)-(CZM(IBASE+2,JBASE+2,COUNT3)+     &
                  & CZP(IBASE+2,JBASE+2,COUNT3))*U(IBASE+2,JBASE+2,COUNT3)+       &
                  & CZP(IBASE+2,JBASE+2,COUNT3)*U(IBASE+2,JBASE+2,COUNT3+1))+     &
                  & U(IBASE+2,JBASE+2,COUNT3)+F*CONST(IBASE+2,JBASE+2,COUNT3)*    &
                  & (TDBAV/DX(IBASE+2)/(REXT+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-        &
                  & (0.88d0*SIGMA*(U(IBASE+2,JBASE+2,COUNT3)+273.15d0)**4)/DX(IBASE+2))+ &
                  & (3.d0-2.d0*F)*CONST(IBASE+2,JBASE+2,COUNT3)*(TDBAV/DY(JBASE+2)/   &
                  & (REXT+1.d0/HOAV)+RSOLVAV/DY(JBASE+2)-(0.88d0*SIGMA*               &
                  & (U(IBASE+2,JBASE+2,COUNT3)+273.15d0)**4)/DY(JBASE+2))
                  N=JBASE+2-0+1
                  L=1
                  DO COUNT2=0,JBASE+2
                    AA(L)=A(IBASE+2,COUNT2,COUNT3)
                    BB(L)=B(IBASE+2,COUNT2,COUNT3)
                    CC(L)=C(IBASE+2,COUNT2,COUNT3)
                    RR(L)=R(IBASE+2,COUNT2,COUNT3)
                    L=L+1
                  END DO
                  CALL TRIDI3D (AA,BB,CC,RR,N,X)
                  L=0
                  DO COUNT2=0,JBASE+2
                    V(IBASE+2,COUNT2,COUNT3)=X(L+1)
                    L=L+1
                    QEXT(IBASE+2,COUNT2,COUNT3)=(TDBAV-V(IBASE+2,COUNT2,COUNT3))/ &
                        & (REXT+RSID+1.d0/HOAV)
                    VEXT(IBASE+2,COUNT2,COUNT3)=QEXT(IBASE+2,COUNT2,COUNT3)*      &
                        & (REXT+RSID)+V(IBASE+2,COUNT2,COUNT3)
                  END DO
                END DO
              END IF

!***  SECTION 3:  RIM JOIST, PARALLEL TO Y-AXIS
              A(IBASE+2,0,-NZAG+1)=0.
              B(IBASE+2,0,-NZAG+1)=1.d0+(3.d0-2.d0*F)*CONST(IBASE+2,0,-NZAG+1)*       &
                 & CYP(IBASE+2,0,-NZAG+1)
              C(IBASE+2,0,-NZAG+1)=(3.d0-2.d0*F)*CONST(IBASE+2,0,-NZAG+1)*          &
                 & (-CYP(IBASE+2,0,-NZAG+1))
              R(IBASE+2,0,-NZAG+1)=F*CONST(IBASE+2,0,-NZAG+1)*                  &
              & (-(1.d0/DX(IBASE+2)/(RSILL+1.d0/HIN(6)+DX(IBASE+2)/TCON             &
              & (MTYPE(IBASE+2,0,-NZAG+1)))+1.d0/DX(IBASE+2)/(REXT+RSID+1.d0/HOAV))* &
              & U(IBASE+2,0,-NZAG+1)+CZM(IBASE+2,0,-NZAG+1)*                    &
              & U(IBASE+2,0,-NZAG+1-1)-(CZM(IBASE+2,0,-NZAG+1)+                 &
              & CZP(IBASE+2,0,-NZAG+1))*U(IBASE+2,0,-NZAG+1)+                   &
              & CZP(IBASE+2,0,-NZAG+1)*U(IBASE+2,0,-NZAG+1+1))+                 &
              & U(IBASE+2,0,-NZAG+1)+F*CONST(IBASE+2,0,-NZAG+1)*                &
              & (TBAV/DX(IBASE+2)/(RSILL+1.d0/HIN(6)+DX(IBASE+2)/                 &
              & TCON(MTYPE(IBASE+2,0,-NZAG+1)))+TDBAV/DX(IBASE+2)/              &
              & (REXT+RSID+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-(0.88d0*SIGMA*            &
              & (UEXT(IBASE+2,0,-NZAG+1)+273.15d0)**4)/DX(IBASE+2))
              DO COUNT2=1,JBASE+1
                A(IBASE+2,COUNT2,-NZAG+1)=(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,-NZAG+1)* &
                   & (-CYM(IBASE+2,COUNT2,-NZAG+1))
                B(IBASE+2,COUNT2,-NZAG+1)=1.d0+(3.d0-2.d0*F)*                         &
                   &  CONST(IBASE+2,COUNT2,-NZAG+1)*(CYM(IBASE+2,COUNT2,-NZAG+1)+ &
                   & CYP(IBASE+2,COUNT2,-NZAG+1))
                C(IBASE+2,COUNT2,-NZAG+1)=(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,-NZAG+1)* &
                   & (-CYP(IBASE+2,COUNT2,-NZAG+1))
                R(IBASE+2,COUNT2,-NZAG+1)=F*CONST(IBASE+2,COUNT2,-NZAG+1)*      &
                & (-(1.d0/DX(IBASE+2)/(RSILL+1.d0/HIN(6)+DX(IBASE+2)/TCON           &
                & (MTYPE(IBASE+2,COUNT2,-NZAG+1)))+1.d0/DX(IBASE+2)/(REXT+RSID+1.d0/ &
                & HOAV))*U(IBASE+2,COUNT2,-NZAG+1)+CZM(IBASE+2,COUNT2,-NZAG+1)* &
                & U(IBASE+2,COUNT2,-NZAG+1-1)-(CZM(IBASE+2,COUNT2,-NZAG+1)+     &
                & CZP(IBASE+2,COUNT2,-NZAG+1))*U(IBASE+2,COUNT2,-NZAG+1)+       &
                & CZP(IBASE+2,COUNT2,-NZAG+1)*U(IBASE+2,COUNT2,-NZAG+1+1))+     &
                & U(IBASE+2,COUNT2,-NZAG+1)+F*CONST(IBASE+2,COUNT2,-NZAG+1)*    &
                & (TBAV/DX(IBASE+2)/(RSILL+1.d0/HIN(6)+DX(IBASE+2)/               &
                & TCON(MTYPE(IBASE+2,COUNT2,-NZAG+1)))+TDBAV/DX(IBASE+2)/       &
                & (REXT+RSID+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-(0.88d0*SIGMA*          &
                & (UEXT(IBASE+2,COUNT2,-NZAG+1)+273.15d0)**4)/DX(IBASE+2))
              END DO

!***  CORNER RIM JOIST CELL
              A(IBASE+2,JBASE+2,-NZAG+1)=(3.d0-2.d0*F)*CONST(IBASE+2,JBASE+2,-NZAG+1)* &
                &(-CYM(IBASE+2,JBASE+2,-NZAG+1))
              B(IBASE+2,JBASE+2,-NZAG+1)=1.d0+(3.d0-2.d0*F)*                            &
                & CONST(IBASE+2,JBASE+2,-NZAG+1)*(CYM(IBASE+2,JBASE+2,-NZAG+1)+1. &
                & /DY(JBASE+2)/(REXT+RSID+1.d0/HOAV))
              C(IBASE+2,JBASE+2,-NZAG+1)=0.
              R(IBASE+2,JBASE+2,-NZAG+1)=F*CONST(IBASE+2,JBASE+2,-NZAG+1)*        &
              & (CXM(IBASE+2,JBASE+2,-NZAG+1)*U(IBASE+2-1,JBASE+2,-NZAG+1)-       &
              & (CXM(IBASE+2,JBASE+2,-NZAG+1)+1.d0/DX(IBASE+2)/(REXT+RSID+1.d0/HOAV))* &
              & U(IBASE+2,JBASE+2,-NZAG+1)+CZM(IBASE+2,JBASE+2,-NZAG+1)*          &
              & U(IBASE+2,JBASE+2,-NZAG+1-1)-(CZM(IBASE+2,JBASE+2,-NZAG+1)+       &
              & CZP(IBASE+2,JBASE+2,-NZAG+1))*U(IBASE+2,JBASE+2,-NZAG+1)+         &
              & CZP(IBASE+2,JBASE+2,-NZAG+1)*U(IBASE+2,JBASE+2,-NZAG+1+1))+       &
              & U(IBASE+2,JBASE+2,-NZAG+1)+F*CONST(IBASE+2,JBASE+2,-NZAG+1)*      &
              & (TDBAV/DX(IBASE+2)/(REXT+RSID+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-       &
              & (0.88d0*SIGMA*(UEXT(IBASE+2,JBASE+2,-NZAG+1)+273.15d0)**4)/          &
              & DX(IBASE+2))+(3.d0-2.d0*F)*CONST(IBASE+2,JBASE+2,-NZAG+1)*            &
              & (TDBAV/DY(JBASE+2)/(REXT+RSID+1.d0/HOAV)+RSOLVAV/DY(JBASE+2)-       &
              & (0.88d0*SIGMA*(UEXT(IBASE+2,JBASE+2,-NZAG+1)+273.15d0)**4)/          &
              & DY(JBASE+2))
              N=JBASE+2-0+1
              L=1
              DO COUNT2=0,JBASE+2
                AA(L)=A(IBASE+2,COUNT2,-NZAG+1)
                BB(L)=B(IBASE+2,COUNT2,-NZAG+1)
                CC(L)=C(IBASE+2,COUNT2,-NZAG+1)
                RR(L)=R(IBASE+2,COUNT2,-NZAG+1)
                L=L+1
              END DO
              CALL TRIDI3D (AA,BB,CC,RR,N,X)
              L=0
              DO COUNT2=0,JBASE+2
                V(IBASE+2,COUNT2,-NZAG+1)=X(L+1)
                L=L+1
                QEXT(IBASE+2,COUNT2,-NZAG+1)=(TDBAV-V(IBASE+2,COUNT2,-NZAG+1))/   &
                      & (REXT+RSID+1.d0/HOAV)
                VEXT(IBASE+2,COUNT2,-NZAG+1)=QEXT(IBASE+2,COUNT2,-NZAG+1)*        &
                      & (REXT+RSID)+V(IBASE+2,COUNT2,-NZAG+1)
              END DO

!***  SECTION 4:  ABOVE-GRADE FOUNDATION WALL, PARALLEL TO X-AXIS
!***  TOP INSIDE WALL CELL
              DO COUNT1=0,IBASE-1
                IF (TBAV.GT.U(COUNT1,JBASE,-NZAG+2)) THEN
                  HINZ=HIN(4)
                ELSE
                  HINZ=HIN(5)
                END IF
                A(COUNT1,JBASE,-NZAG+2)=0.
                B(COUNT1,JBASE,-NZAG+2)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,-NZAG+2)* &
                      & (1.d0/DY(JBASE)/(RINT+1.d0/HIN(6))+CYP(COUNT1,JBASE,-NZAG+2))
                C(COUNT1,JBASE,-NZAG+2)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,-NZAG+2)*    &
                      & (-CYP(COUNT1,JBASE,-NZAG+2))
                R(COUNT1,JBASE,-NZAG+2)=F*CONST(COUNT1,JBASE,-NZAG+2)*            &
                & (CXM(COUNT1,JBASE,-NZAG+2)*U(COUNT1-1,JBASE,-NZAG+2)-           &
                & (CXM(COUNT1,JBASE,-NZAG+2)+CXP(COUNT1,JBASE,-NZAG+2))*          &
                & U(COUNT1,JBASE,-NZAG+2)+CXP(COUNT1,JBASE,-NZAG+2)*              &
                & U(COUNT1+1,JBASE,-NZAG+2)-(1.d0/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+       &
                & DZ(-NZAG+2)/2.d0/TCON(MTYPE(COUNT1,JBASE,-NZAG+2)))+              &
                & CZP(COUNT1,JBASE,-NZAG+2))*U(COUNT1,JBASE,-NZAG+2)+             &
                & CZP(COUNT1,JBASE,-NZAG+2)*U(COUNT1,JBASE,-NZAG+2+1))+           &
                & U(COUNT1,JBASE,-NZAG+2)+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,-NZAG+2)*  &
                & (TBAV/DY(JBASE)/(RINT+1.d0/HIN(6)))+F*CONST(COUNT1,JBASE,-NZAG+2)* &
                & (TBAV/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+DZ(-NZAG+2)/2.d0/                &
                & TCON(MTYPE(COUNT1,JBASE,-NZAG+2))))

!***  TOP CENTER WALL CELL
                IF (TBAV.GT.U(COUNT1,JBASE+1,-NZAG+2)) THEN
                  HINZ=HIN(4)
                ELSE
                  HINZ=HIN(5)
                END IF
                A(COUNT1,JBASE+1,-NZAG+2)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+1,-NZAG+2)* &
                      & (-CYM(COUNT1,JBASE+1,-NZAG+2))
                B(COUNT1,JBASE+1,-NZAG+2)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE+1,-NZAG+2)* &
                      & (CYM(COUNT1,JBASE+1,-NZAG+2)+CYP(COUNT1,JBASE+1,-NZAG+2))
                C(COUNT1,JBASE+1,-NZAG+2)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+1,-NZAG+2)* &
                      & (-CYP(COUNT1,JBASE+1,-NZAG+2))
                R(COUNT1,JBASE+1,-NZAG+2)=F*CONST(COUNT1,JBASE+1,-NZAG+2)*        &
                & (CXM(COUNT1,JBASE+1,-NZAG+2)*U(COUNT1-1,JBASE+1,-NZAG+2)-       &
                & (CXM(COUNT1,JBASE+1,-NZAG+2)+CXP(COUNT1,JBASE+1,-NZAG+2))*      &
                & U(COUNT1,JBASE+1,-NZAG+2)+CXP(COUNT1,JBASE+1,-NZAG+2)*          &
                & U(COUNT1+1,JBASE+1,-NZAG+2)-(1.d0/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+     &
                & DZ(-NZAG+2)/2.d0/TCON(MTYPE(COUNT1,JBASE+1,-NZAG+2)))+            &
                & CZP(COUNT1,JBASE+1,-NZAG+2))*U(COUNT1,JBASE+1,-NZAG+2)+         &
                & CZP(COUNT1,JBASE+1,-NZAG+2)*U(COUNT1,JBASE+1,-NZAG+2+1))+       &
                & U(COUNT1,JBASE+1,-NZAG+2)+F*CONST(COUNT1,JBASE+1,-NZAG+2)*      &
                & (TBAV/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+DZ(-NZAG+2)/2.d0/                &
                & TCON(MTYPE(COUNT1,JBASE+1,-NZAG+2))))

!***  TOP OUTSIDE WALL CELL
                A(COUNT1,JBASE+2,-NZAG+2)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,-NZAG+2)*  &
                      & (-CYM(COUNT1,JBASE+2,-NZAG+2))
                B(COUNT1,JBASE+2,-NZAG+2)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,-NZAG+2)* &
                      & (CYM(COUNT1,JBASE+2,-NZAG+2)+1.d0/DY(JBASE+2)/(REXT+RSID+1.d0/HOAV))
                C(COUNT1,JBASE+2,-NZAG+2)=0.
                R(COUNT1,JBASE+2,-NZAG+2)=F*CONST(COUNT1,JBASE+2,-NZAG+2)*        &
                & (CXM(COUNT1,JBASE+2,-NZAG+2)*U(COUNT1-1,JBASE+2,-NZAG+2)-       &
                & (CXM(COUNT1,JBASE+2,-NZAG+2)+CXP(COUNT1,JBASE+2,-NZAG+2))*      &
                U(COUNT1,JBASE+2,-NZAG+2)+CXP(COUNT1,JBASE+2,-NZAG+2)*            &
                & U(COUNT1+1,JBASE+2,-NZAG+2)+CZM(COUNT1,JBASE+2,-NZAG+2)*        &
                & U(COUNT1,JBASE+2,-NZAG+2-1)-(CZM(COUNT1,JBASE+2,-NZAG+2)+       &
                & CZP(COUNT1,JBASE+2,-NZAG+2))*U(COUNT1,JBASE+2,-NZAG+2)+         &
                & CZP(COUNT1,JBASE+2,-NZAG+2)*U(COUNT1,JBASE+2,-NZAG+2+1))+       &
                & U(COUNT1,JBASE+2,-NZAG+2)+(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,-NZAG+2)* &
                & (TDBAV/DY(JBASE+2)/(REXT+RSID+1.d0/HOAV)+RSOLVAV/DY(JBASE+2)-     &
                & (0.88d0*SIGMA*(UEXT(COUNT1,JBASE+2,-NZAG+2)+273.15d0)**4)/DY(JBASE+2))
                N=JBASE+2-JBASE+1
                L=1
                DO COUNT2=JBASE,JBASE+2
                  AA(L)=A(COUNT1,COUNT2,-NZAG+2)
                  BB(L)=B(COUNT1,COUNT2,-NZAG+2)
                  CC(L)=C(COUNT1,COUNT2,-NZAG+2)
                  RR(L)=R(COUNT1,COUNT2,-NZAG+2)
                  L=L+1
                END DO
                CALL TRIDI3D (AA,BB,CC,RR,N,X)
                L=0
                DO COUNT2=JBASE,JBASE+2
                  V(COUNT1,COUNT2,-NZAG+2)=X(L+1)
                  L=L+1
                END DO
                QEXT(COUNT1,JBASE+2,-NZAG+2)=(TDBAV-V(COUNT1,JBASE+2,-NZAG+2))/   &
                      & (REXT+RSID+1.d0/HOAV)
                VEXT(COUNT1,JBASE+2,-NZAG+2)=QEXT(COUNT1,JBASE+2,-NZAG+2)*        &
                      & (REXT+RSID)+V(COUNT1,JBASE+2,-NZAG+2)
              END DO

!***  INSIDE WALL CELLS
              IF (NZAG.GT.3) THEN
                DO COUNT3=-NZAG+3,-1
                  DO COUNT1=0,IBASE-1
                    A(COUNT1,JBASE,COUNT3)=0.
                    B(COUNT1,JBASE,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)* &
                        & (1.d0/DY(JBASE)/(RINT+1.d0/HIN(6))+CYP(COUNT1,JBASE,COUNT3))
                    C(COUNT1,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)*  &
                        & (-CYP(COUNT1,JBASE,COUNT3))
                    R(COUNT1,JBASE,COUNT3)=F*CONST(COUNT1,JBASE,COUNT3)*          &
                    & (CXM(COUNT1,JBASE,COUNT3)*U(COUNT1-1,JBASE,COUNT3)-         &
                    & (CXM(COUNT1,JBASE,COUNT3)+CXP(COUNT1,JBASE,COUNT3))*        &
                    & U(COUNT1,JBASE,COUNT3)+CXP(COUNT1,JBASE,COUNT3)*            &
                    & U(COUNT1+1,JBASE,COUNT3)+CZM(COUNT1,JBASE,COUNT3)*          &
                    & U(COUNT1,JBASE,COUNT3-1)-(CZM(COUNT1,JBASE,COUNT3)+         &
                    & CZP(COUNT1,JBASE,COUNT3))*U(COUNT1,JBASE,COUNT3)+           &
                    & CZP(COUNT1,JBASE,COUNT3)*U(COUNT1,JBASE,COUNT3+1))+         &
                    & U(COUNT1,JBASE,COUNT3)+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)* &
                    & (TBAV/DY(JBASE)/(RINT+1.d0/HIN(6)))

!***  CENTER WALL CELL
                    A(COUNT1,JBASE+1,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+1,COUNT3)* &
                       & (-CYM(COUNT1,JBASE+1,COUNT3))
                    B(COUNT1,JBASE+1,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE+1,COUNT3)* &
                       & (CYM(COUNT1,JBASE+1,COUNT3)+CYP(COUNT1,JBASE+1,COUNT3))
                    C(COUNT1,JBASE+1,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+1,COUNT3)* &
                       & (-CYP(COUNT1,JBASE+1,COUNT3))
                    R(COUNT1,JBASE+1,COUNT3)=F*CONST(COUNT1,JBASE+1,COUNT3)*        &
                    & (CXM(COUNT1,JBASE+1,COUNT3)*U(COUNT1-1,JBASE+1,COUNT3)-       &
                    & (CXM(COUNT1,JBASE+1,COUNT3)+CXP(COUNT1,JBASE+1,COUNT3))*      &
                    & U(COUNT1,JBASE+1,COUNT3)+CXP(COUNT1,JBASE+1,COUNT3)*          &
                    & U(COUNT1+1,JBASE+1,COUNT3)+CZM(COUNT1,JBASE+1,COUNT3)*        &
                    & U(COUNT1,JBASE+1,COUNT3-1)-(CZM(COUNT1,JBASE+1,COUNT3)+       &
                    & CZP(COUNT1,JBASE+1,COUNT3))*U(COUNT1,JBASE+1,COUNT3)+         &
                    & CZP(COUNT1,JBASE+1,COUNT3)*U(COUNT1,JBASE+1,COUNT3+1))+       &
                    & U(COUNT1,JBASE+1,COUNT3)

!***  OUTSIDE WALL CELL
                    A(COUNT1,JBASE+2,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,COUNT3)* &
                        & (-CYM(COUNT1,JBASE+2,COUNT3))
                    B(COUNT1,JBASE+2,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,COUNT3)* &
                        & (CYM(COUNT1,JBASE+2,COUNT3)+1.d0/DY(JBASE+2)/(REXT+1.d0/HOAV))
                    C(COUNT1,JBASE+2,COUNT3)=0.
                    R(COUNT1,JBASE+2,COUNT3)=F*CONST(COUNT1,JBASE+2,COUNT3)*        &
                    & (CXM(COUNT1,JBASE+2,COUNT3)*U(COUNT1-1,JBASE+2,COUNT3)-       &
                    & (CXM(COUNT1,JBASE+2,COUNT3)+CXP(COUNT1,JBASE+2,COUNT3))*      &
                    & U(COUNT1,JBASE+2,COUNT3)+CXP(COUNT1,JBASE+2,COUNT3)*          &
                    & U(COUNT1+1,JBASE+2,COUNT3)+CZM(COUNT1,JBASE+2,COUNT3)*        &
                    & U(COUNT1,JBASE+2,COUNT3-1)-(CZM(COUNT1,JBASE+2,COUNT3)+       &
                    & CZP(COUNT1,JBASE+2,COUNT3))*U(COUNT1,JBASE+2,COUNT3)+         &
                    & CZP(COUNT1,JBASE+2,COUNT3)*U(COUNT1,JBASE+2,COUNT3+1))+       &
                    & U(COUNT1,JBASE+2,COUNT3)+(3.d0-2.d0*F)*                           &
                    & CONST(COUNT1,JBASE+2,COUNT3)*(TDBAV/DY(JBASE+2)/(REXT+1.d0/HOAV)+ &
                    & RSOLVAV/DY(JBASE+2)-(0.88d0*SIGMA*(UEXT(COUNT1,JBASE+2,COUNT3)+ &
                    & 273.15d0)**4)/DY(JBASE+2))
                    N=JBASE+2-JBASE+1
                    L=1
                    DO COUNT2=JBASE,JBASE+2
                      AA(L)=A(COUNT1,COUNT2,COUNT3)
                      BB(L)=B(COUNT1,COUNT2,COUNT3)
                      CC(L)=C(COUNT1,COUNT2,COUNT3)
                      RR(L)=R(COUNT1,COUNT2,COUNT3)
                      L=L+1
                    END DO
                    CALL TRIDI3D (AA,BB,CC,RR,N,X)
                    L=0
                    DO COUNT2=JBASE,JBASE+2
                      V(COUNT1,COUNT2,COUNT3)=X(L+1)
                      L=L+1
                    END DO
                    QEXT(COUNT1,JBASE+2,COUNT3)=(TDBAV-V(COUNT1,JBASE+2,COUNT3))/   &
                        & (REXT+RSID+1.d0/HOAV)
                    VEXT(COUNT1,JBASE+2,COUNT3)=QEXT(COUNT1,JBASE+2,COUNT3)*        &
                        & (REXT+RSID)+V(COUNT1,JBASE+2,COUNT3)
                  END DO
                END DO
              END IF

!***  SECTION 5:  RIM JOIST, PARALLEL TO X-AXIS
              DO COUNT1=0,IBASE+1
                A(COUNT1,JBASE+2,-NZAG+1)=0.
                B(COUNT1,JBASE+2,-NZAG+1)=1.d0+(3.d0-2.d0*F)*                         &
                    & CONST(COUNT1,JBASE+2,-NZAG+1)*(1.d0/DY(JBASE+2)/(RSILL+1.d0/  &
                    & HIN(6)+DY(JBASE+2)/TCON(MTYPE(COUNT1,JBASE+2,-NZAG+1)))+  &
                    & 1.d0/DY(JBASE+2)/(REXT+RSID+1.d0/HOAV))
                C(COUNT1,JBASE+2,-NZAG+1)=0.
                R(COUNT1,JBASE+2,-NZAG+1)=F*CONST(COUNT1,JBASE+2,-NZAG+1)*      &
                & (CXM(COUNT1,JBASE+2,-NZAG+1)*U(COUNT1-1,JBASE+2,-NZAG+1)-     &
                & (CXM(COUNT1,JBASE+2,-NZAG+1)+CXP(COUNT1,JBASE+2,-NZAG+1))*    &
                & U(COUNT1,JBASE+2,-NZAG+1)+CXP(COUNT1,JBASE+2,-NZAG+1)*        &
                & U(COUNT1+1,JBASE+2,-NZAG+1)+CZM(COUNT1,JBASE+2,-NZAG+1)*      &
                & U(COUNT1,JBASE+2,-NZAG+1-1)-(CZM(COUNT1,JBASE+2,-NZAG+1)+     &
                & CZP(COUNT1,JBASE+2,-NZAG+1))*U(COUNT1,JBASE+2,-NZAG+1)+       &
                & CZP(COUNT1,JBASE+2,-NZAG+1)*U(COUNT1,JBASE+2,-NZAG+1+1))+     &
                & U(COUNT1,JBASE+2,-NZAG+1)+(3.d0-2.d0*F)*                          &
                & CONST(COUNT1,JBASE+2,-NZAG+1)*(TBAV/DY(JBASE+2)/              &
                & (RSILL+1.d0/HIN(6)+DY(JBASE+2)/TCON                             &
                & (MTYPE(COUNT1,JBASE+2,-NZAG+1)))+TDBAV/DY(JBASE+2)/           &
                & (REXT+RSID+1.d0/HOAV)+RSOLVAV/DY(JBASE+2)-(0.88d0*SIGMA*          &
                & (UEXT(COUNT1,JBASE+2,-NZAG+1)+273.15d0)**4)/DY(JBASE+2))
                N=1
                L=1
                AA(L)=A(COUNT1,JBASE+2,-NZAG+1)
                BB(L)=B(COUNT1,JBASE+2,-NZAG+1)
                CC(L)=C(COUNT1,JBASE+2,-NZAG+1)
                RR(L)=R(COUNT1,JBASE+2,-NZAG+1)
                CALL TRIDI3D (AA,BB,CC,RR,N,X)
                L=0
                V(COUNT1,JBASE+2,-NZAG+1)=X(L+1)
                QEXT(COUNT1,JBASE+2,-NZAG+1)=(TDBAV-V(COUNT1,JBASE+2,-NZAG+1))/   &
                     & (REXT+RSID+1.d0/HOAV)
                VEXT(COUNT1,JBASE+2,-NZAG+1)=QEXT(COUNT1,JBASE+2,-NZAG+1)*        &
                     & (REXT+RSID)+V(COUNT1,JBASE+2,-NZAG+1)
              END DO

!***  SECTION 6:  BELOW-GRADE (1)
!***  INSIDE WALL CELLS, PARALLEL TO Y-AXIS (UPPER BAND) COUNT3=0
                A(IBASE,0,0)=0.
                B(IBASE,0,0)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,0,0)*CYP(IBASE,0,0)
                C(IBASE,0,0)=(3.d0-2.d0*F)*CONST(IBASE,0,0)*(-CYP(IBASE,0,0))
                R(IBASE,0,0)=F*CONST(IBASE,0,0)*(-(1.d0/DX(IBASE)/(RINT+            &
                & 1.d0/HIN(6))+CXP(IBASE,0,0))*U(IBASE,0,0)+CXP(IBASE,0,0)*         &
                & U(IBASE+1,0,0)+CZM(IBASE,0,0)*U(IBASE,0,0-1)-                   &
                & (CZM(IBASE,0,0)+CZP(IBASE,0,0))*U(IBASE,0,0)+CZP(IBASE,0,0)*    &
                & U(IBASE,0,0+1))+U(IBASE,0,0)+F*CONST(IBASE,0,0)*                &
                & (TBAV/DX(IBASE)/(RINT+1.d0/HIN(6)))
                DO COUNT2=1,JBASE-1
                  A(IBASE,COUNT2,0)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,0)*              &
                     & (-CYM(IBASE,COUNT2,0))
                  B(IBASE,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,COUNT2,0)*           &
                     & (CYM(IBASE,COUNT2,0)+CYP(IBASE,COUNT2,0))
                  C(IBASE,COUNT2,0)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,0)*              &
                     & (-CYP(IBASE,COUNT2,0))
                  R(IBASE,COUNT2,0)=F*CONST(IBASE,COUNT2,0)*(-(1.d0/DX(IBASE)/(RINT+ &
                  & 1.d0/HIN(6))+CXP(IBASE,COUNT2,0))*U(IBASE,COUNT2,0)+            &
                  & CXP(IBASE,COUNT2,0)*U(IBASE+1,COUNT2,0)+CZM(IBASE,COUNT2,0)*  &
                  & U(IBASE,COUNT2,0-1)-(CZM(IBASE,COUNT2,0)+CZP(IBASE,COUNT2,0))* &
                  & U(IBASE,COUNT2,0)+CZP(IBASE,COUNT2,0)*U(IBASE,COUNT2,0+1))+   &
                  & U(IBASE,COUNT2,0)+F*CONST(IBASE,COUNT2,0)*(TBAV/DX(IBASE)/    &
                  & (RINT+1.d0/HIN(6)))
                END DO

!***  FOUNDATION WALL CELLS COUNT3=0
                DO COUNT2=JBASE,JBASE+2
                  A(IBASE,COUNT2,0)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,0)*                &
                        & (-CYM(IBASE,COUNT2,0))
                  B(IBASE,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,COUNT2,0)*             &
                        & (CYM(IBASE,COUNT2,0)+CYP(IBASE,COUNT2,0))
                  C(IBASE,COUNT2,0)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,0)*                &
                        & (-CYP(IBASE,COUNT2,0))
                  R(IBASE,COUNT2,0)=F*CONST(IBASE,COUNT2,0)*(CXM(IBASE,COUNT2,0)*   &
                  & U(IBASE-1,COUNT2,0)-(CXM(IBASE,COUNT2,0)+CXP(IBASE,COUNT2,0))*  &
                  & U(IBASE,COUNT2,0)+CXP(IBASE,COUNT2,0)*U(IBASE+1,COUNT2,0)+      &
                  & CZM(IBASE,COUNT2,0)*U(IBASE,COUNT2,0-1)-(CZM(IBASE,COUNT2,0)+   &
                  & CZP(IBASE,COUNT2,0))*U(IBASE,COUNT2,0)+CZP(IBASE,COUNT2,0)*     &
                  & U(IBASE,COUNT2,0+1))+U(IBASE,COUNT2,0)
                END DO

!***  GROUND SURFACE CELLS COUNT3=0
                DO COUNT2=JBASE+3,NYM1-1
                  IF (ISNW.EQ.1) THEN
                    A(IBASE,COUNT2,0)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,0)*              &
                        & (-CYM(IBASE,COUNT2,0))
                    B(IBASE,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,COUNT2,0)*           &
                        & (CYM(IBASE,COUNT2,0)+CYP(IBASE,COUNT2,0))
                    C(IBASE,COUNT2,0)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,0)*              &
                        & (-CYP(IBASE,COUNT2,0))
                    R(IBASE,COUNT2,0)=F*CONST(IBASE,COUNT2,0)*(CXM(IBASE,COUNT2,0)* &
                    & U(IBASE-1,COUNT2,0)-(CXM(IBASE,COUNT2,0)+CXP(IBASE,COUNT2,0))* &
                    & U(IBASE,COUNT2,0)+CXP(IBASE,COUNT2,0)*U(IBASE+1,COUNT2,0)-    &
                    & CZP(IBASE,COUNT2,0)*U(IBASE,COUNT2,0)+CZP(IBASE,COUNT2,0)*    &
                    & U(IBASE,COUNT2,0+1))+U(IBASE,COUNT2,0)+F*CONST(IBASE,COUNT2,0)* &
                    & (GOFTAV(IBASE,COUNT2)/DZ(0))
                  ELSE
                    A(IBASE,COUNT2,0)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,0)*              &
                        & (-CYM(IBASE,COUNT2,0))
                    B(IBASE,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,COUNT2,0)*           &
                        & (CYM(IBASE,COUNT2,0)+CYP(IBASE,COUNT2,0))
                    C(IBASE,COUNT2,0)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,0)*              &
                        & (-CYP(IBASE,COUNT2,0))
                    R(IBASE,COUNT2,0)=F*CONST(IBASE,COUNT2,0)*(CXM(IBASE,COUNT2,0)* &
                    & U(IBASE-1,COUNT2,0)-(CXM(IBASE,COUNT2,0)+CXP(IBASE,COUNT2,0))* &
                    & U(IBASE,COUNT2,0)+CXP(IBASE,COUNT2,0)*U(IBASE+1,COUNT2,0)-    &
                    & CZP(IBASE,COUNT2,0)*U(IBASE,COUNT2,0)+CZP(IBASE,COUNT2,0)*    &
                    & U(IBASE,COUNT2,0+1))+U(IBASE,COUNT2,0)+F*CONST(IBASE,COUNT2,0)* &
                    & (GOFTAV(IBASE,COUNT2)/DZ(0)+TDBAV/DZ(0)/RSNWAV)
                  END IF
                END DO

                IF (ISNW.EQ.1) THEN
                  A(IBASE,NYM1,0)=(3.d0-2.d0*F)*CONST(IBASE,NYM1,0)*                    &
                        & (-CYM(IBASE,NYM1,0))
                  B(IBASE,NYM1,0)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,NYM1,0)*                 &
                        & (CYM(IBASE,NYM1,0)+CYP(IBASE,NYM1,0))
                  C(IBASE,NYM1,0)=0.
                  R(IBASE,NYM1,0)=F*CONST(IBASE,NYM1,0)*(CXM(IBASE,NYM1,0)*         &
                  & U(IBASE-1,NYM1,0)-(CXM(IBASE,NYM1,0)+CXP(IBASE,NYM1,0))*        &
                  & U(IBASE,NYM1,0)+CXP(IBASE,NYM1,0)*U(IBASE+1,NYM1,0)-            &
                  & CZP(IBASE,NYM1,0)*U(IBASE,NYM1,0)+CZP(IBASE,NYM1,0)*            &
                  & U(IBASE,NYM1,0+1))+U(IBASE,NYM1,0)+F*CONST(IBASE,NYM1,0)*       &
                  & (GOFTAV(IBASE,NYM1)/DZ(0))+(3.d0-2.d0*F)*CONST(IBASE,NYM1,0)*       &
                  & CYP(IBASE,NYM1,0)*TGAV(0)
                ELSE
                  A(IBASE,NYM1,0)=(3.d0-2.d0*F)*CONST(IBASE,NYM1,0)*(-CYM(IBASE,NYM1,0))
                  B(IBASE,NYM1,0)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,NYM1,0)*                 &
                        &     (CYM(IBASE,NYM1,0)+CYP(IBASE,NYM1,0))
                  C(IBASE,NYM1,0)=0.
                  R(IBASE,NYM1,0)=F*CONST(IBASE,NYM1,0)*(CXM(IBASE,NYM1,0)*         &
                  & U(IBASE-1,NYM1,0)-(CXM(IBASE,NYM1,0)+CXP(IBASE,NYM1,0))*        &
                  & U(IBASE,NYM1,0)+CXP(IBASE,NYM1,0)*U(IBASE+1,NYM1,0)-            &
                  & CZP(IBASE,NYM1,0)*U(IBASE,NYM1,0)+CZP(IBASE,NYM1,0)*            &
                  & U(IBASE,NYM1,0+1))+U(IBASE,NYM1,0)+F*CONST(IBASE,NYM1,0)*       &
                  & (GOFTAV(IBASE,NYM1)/DZ(0)+TDBAV/DZ(0)/RSNWAV)+(3.d0-2.d0*F)*        &
                  & CONST(IBASE,NYM1,0)*CYP(IBASE,NYM1,0)*TGAV(0)
                END IF
                N=NYM1-0+1
                L=1
                DO COUNT2=0,NYM1
                  AA(L)=A(IBASE,COUNT2,0)
                  BB(L)=B(IBASE,COUNT2,0)
                  CC(L)=C(IBASE,COUNT2,0)
                  RR(L)=R(IBASE,COUNT2,0)
                  L=L+1
                END DO
                CALL TRIDI3D (AA,BB,CC,RR,N,X)
                L=0
                DO COUNT2=0,NYM1
                  V(IBASE,COUNT2,0)=X(L+1)
                  L=L+1
                END DO

!***  FOUNDATION WALL CELLS COUNT3=0
                DO COUNT1=IBASE+1,IBASE+2
                  A(COUNT1,0,0)=0.
                  B(COUNT1,0,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,0,0)*CYP(COUNT1,0,0)
                  C(COUNT1,0,0)=(3.d0-2.d0*F)*CONST(COUNT1,0,0)*(-CYP(COUNT1,0,0))
                  R(COUNT1,0,0)=F*CONST(COUNT1,0,0)*(CXM(COUNT1,0,0)*U(COUNT1-1,0,0)- &
                  & (CXM(COUNT1,0,0)+CXP(COUNT1,0,0))*U(COUNT1,0,0)+                &
                  & CXP(COUNT1,0,0)*U(COUNT1+1,0,0)+CZM(COUNT1,0,0)*                &
                  & U(COUNT1,0,0-1)-(CZM(COUNT1,0,0)+CZP(COUNT1,0,0))*              &
                  & U(COUNT1,0,0)+CZP(COUNT1,0,0)*U(COUNT1,0,0+1))+U(COUNT1,0,0)
                  DO COUNT2=1,JBASE+2
                    A(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*            &
                        & (-CYM(COUNT1,COUNT2,0))
                    B(COUNT1,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*         &
                        & (CYM(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0))
                    C(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*            &
                        & (-CYP(COUNT1,COUNT2,0))
                    R(COUNT1,COUNT2,0)=F*CONST(COUNT1,COUNT2,0)*                    &
                    & (CXM(COUNT1,COUNT2,0)*U(COUNT1-1,COUNT2,0)-                   &
                    & (CXM(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0))*                  &
                    & U(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0)*U(COUNT1+1,COUNT2,0)+ &
                    & CZM(COUNT1,COUNT2,0)*U(COUNT1,COUNT2,0-1)-                    &
                    & (CZM(COUNT1,COUNT2,0)+CZP(COUNT1,COUNT2,0))*U(COUNT1,COUNT2,0)+ &
                    & CZP(COUNT1,COUNT2,0)*U(COUNT1,COUNT2,0+1))+U(COUNT1,COUNT2,0)
                  END DO

!***  GROUND SURFACE CELLS COUNT3=0
                  DO COUNT2=JBASE+3,NYM1-1
                    IF (ISNW.EQ.1) THEN
                      A(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*          &
                        & (-CYM(COUNT1,COUNT2,0))
                      B(COUNT1,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*       &
                        & (CYM(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0))
                      C(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*          &
                        & (-CYP(COUNT1,COUNT2,0))
                      R(COUNT1,COUNT2,0)=F*CONST(COUNT1,COUNT2,0)*                  &
                      & (CXM(COUNT1,COUNT2,0)*U(COUNT1-1,COUNT2,0)-                 &
                      & (CXM(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0))*                &
                      & U(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0)*                    &
                      & U(COUNT1+1,COUNT2,0)-CZP(COUNT1,COUNT2,0)*                  &
                      & U(COUNT1,COUNT2,0)+CZP(COUNT1,COUNT2,0)*                    &
                      & U(COUNT1,COUNT2,0+1))+U(COUNT1,COUNT2,0)+                   &
                      & F*CONST(COUNT1,COUNT2,0)*(GOFTAV(COUNT1,COUNT2)/DZ(0))
                    ELSE
                      A(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*          &
                        & (-CYM(COUNT1,COUNT2,0))
                      B(COUNT1,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*       &
                        & (CYM(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0))
                      C(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*          &
                        & (-CYP(COUNT1,COUNT2,0))
                      R(COUNT1,COUNT2,0)=F*CONST(COUNT1,COUNT2,0)*                  &
                      & (CXM(COUNT1,COUNT2,0)*U(COUNT1-1,COUNT2,0)-                 &
                      & (CXM(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0))*                &
                      & U(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0)*                    &
                      & U(COUNT1+1,COUNT2,0)-CZP(COUNT1,COUNT2,0)*U(COUNT1,COUNT2,0)+ &
                      & CZP(COUNT1,COUNT2,0)*U(COUNT1,COUNT2,0+1))+                 &
                      & U(COUNT1,COUNT2,0)+F*CONST(COUNT1,COUNT2,0)*                &
                      & (GOFTAV(COUNT1,COUNT2)/DZ(0)+TDBAV/DZ(0)/RSNWAV)
                    END IF
                  END DO
                  IF (ISNW.EQ.1) THEN
                    A(COUNT1,NYM1,0)=(3.d0-2.d0*F)*CONST(COUNT1,NYM1,0)*(-CYM(COUNT1,NYM1,0))
                    B(COUNT1,NYM1,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,NYM1,0)*             &
                    &   (CYM(COUNT1,NYM1,0)+CYP(COUNT1,NYM1,0))
                    C(COUNT1,NYM1,0)=0.
                    R(COUNT1,NYM1,0)=F*CONST(COUNT1,NYM1,0)*(CXM(COUNT1,NYM1,0)*    &
                    & U(COUNT1-1,NYM1,0)-(CXM(COUNT1,NYM1,0)+CXP(COUNT1,NYM1,0))*   &
                    & U(COUNT1,NYM1,0)+CXP(COUNT1,NYM1,0)*U(COUNT1+1,NYM1,0)-       &
                    & CZP(COUNT1,NYM1,0)*U(COUNT1,NYM1,0)+CZP(COUNT1,NYM1,0)*       &
                    & U(COUNT1,NYM1,0+1))+U(COUNT1,NYM1,0)+F*CONST(COUNT1,NYM1,0)*  &
                    & (GOFTAV(COUNT1,NYM1)/DZ(0))+(3.d0-2.d0*F)*CONST(COUNT1,NYM1,0)*   &
                    & CYP(COUNT1,NYM1,0)*TGAV(0)
                  ELSE
                    A(COUNT1,NYM1,0)=(3.d0-2.d0*F)*CONST(COUNT1,NYM1,0)*(-CYM(COUNT1,NYM1,0))
                    B(COUNT1,NYM1,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,NYM1,0)*             &
                        & (CYM(COUNT1,NYM1,0)+CYP(COUNT1,NYM1,0))
                    C(COUNT1,NYM1,0)=0.
                    R(COUNT1,NYM1,0)=F*CONST(COUNT1,NYM1,0)*(CXM(COUNT1,NYM1,0)*    &
                    & U(COUNT1-1,NYM1,0)-(CXM(COUNT1,NYM1,0)+CXP(COUNT1,NYM1,0))*   &
                    & U(COUNT1,NYM1,0)+CXP(COUNT1,NYM1,0)*U(COUNT1+1,NYM1,0)-       &
                    & CZP(COUNT1,NYM1,0)*U(COUNT1,NYM1,0)+CZP(COUNT1,NYM1,0)*       &
                    & U(COUNT1,NYM1,0+1))+U(COUNT1,NYM1,0)+F*CONST(COUNT1,NYM1,0)*  &
                    & (GOFTAV(COUNT1,NYM1)/DZ(0)+TDBAV/DZ(0)/RSNWAV)+(3.d0-2.d0*F)*     &
                    & CONST(COUNT1,NYM1,0)*CYP(COUNT1,NYM1,0)*TGAV(0)
                  END IF
                  N=NYM1-0+1
                  L=1
                  DO COUNT2=0,NYM1
                    AA(L)=A(COUNT1,COUNT2,0)
                    BB(L)=B(COUNT1,COUNT2,0)
                    CC(L)=C(COUNT1,COUNT2,0)
                    RR(L)=R(COUNT1,COUNT2,0)
                    L=L+1
                  END DO
                  CALL TRIDI3D (AA,BB,CC,RR,N,X)
                  L=0
                  DO COUNT2=0,NYM1
                    V(COUNT1,COUNT2,0)=X(L+1)
                    L=L+1
                  END DO
                END DO

!***  GROUND SURFACE CELLS COUNT3=0
                DO COUNT1=IBASE+3,NXM1
                  IF (ISNW.EQ.1) THEN
                    A(COUNT1,0,0)=0.
                    B(COUNT1,0,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,0,0)*CYP(COUNT1,0,0)
                    C(COUNT1,0,0)=(3.d0-2.d0*F)*CONST(COUNT1,0,0)*(-CYP(COUNT1,0,0))
                    R(COUNT1,0,0)=F*CONST(COUNT1,0,0)*(CXM(COUNT1,0,0)*             &
                    & U(COUNT1-1,0,0)-(CXM(COUNT1,0,0)+CXP(COUNT1,0,0))*            &
                    & U(COUNT1,0,0)+CXP(COUNT1,0,0)*U(COUNT1+1,0,0)-CZP(COUNT1,0,0)* &
                    & U(COUNT1,0,0)+CZP(COUNT1,0,0)*U(COUNT1,0,0+1))+U(COUNT1,0,0)+ &
                    & F*CONST(COUNT1,0,0)*(GOFTAV(COUNT1,0)/DZ(0))
                  ELSE
                    A(COUNT1,0,0)=0.
                    B(COUNT1,0,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,0,0)*CYP(COUNT1,0,0)
                    C(COUNT1,0,0)=(3.d0-2.d0*F)*CONST(COUNT1,0,0)*(-CYP(COUNT1,0,0))
                    R(COUNT1,0,0)=F*CONST(COUNT1,0,0)*(CXM(COUNT1,0,0)*             &
                    & U(COUNT1-1,0,0)-(CXM(COUNT1,0,0)+CXP(COUNT1,0,0))*            &
                    & U(COUNT1,0,0)+CXP(COUNT1,0,0)*U(COUNT1+1,0,0)-CZP(COUNT1,0,0)* &
                    & U(COUNT1,0,0)+CZP(COUNT1,0,0)*U(COUNT1,0,0+1))+U(COUNT1,0,0)+ &
                    & F*CONST(COUNT1,0,0)*(GOFTAV(COUNT1,0)/DZ(0)+TDBAV/DZ(0)/RSNWAV)
                  END IF
                  DO COUNT2=1,NYM1-1
                    IF (ISNW.EQ.1) THEN
                      A(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*          &
                        &(-CYM(COUNT1,COUNT2,0))
                      B(COUNT1,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*       &
                        & (CYM(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0))
                      C(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*          &
                        & (-CYP(COUNT1,COUNT2,0))
                      R(COUNT1,COUNT2,0)=F*CONST(COUNT1,COUNT2,0)*                  &
                      & (CXM(COUNT1,COUNT2,0)*U(COUNT1-1,COUNT2,0)-                 &
                      & (CXM(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0))*                &
                      & U(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0)*U(COUNT1+1,COUNT2,0)- &
                      & CZP(COUNT1,COUNT2,0)*U(COUNT1,COUNT2,0)+                    &
                      & CZP(COUNT1,COUNT2,0)*U(COUNT1,COUNT2,0+1))+                 &
                      & U(COUNT1,COUNT2,0)+F*CONST(COUNT1,COUNT2,0)*                &
                      & (GOFTAV(COUNT1,COUNT2)/DZ(0))
                    ELSE
                      A(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*          &
                        & (-CYM(COUNT1,COUNT2,0))
                      B(COUNT1,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*       &
                        & (CYM(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0))
                      C(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*          &
                        & (-CYP(COUNT1,COUNT2,0))
                      R(COUNT1,COUNT2,0)=F*CONST(COUNT1,COUNT2,0)*                  &
                      & (CXM(COUNT1,COUNT2,0)*U(COUNT1-1,COUNT2,0)-                 &
                      & (CXM(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0))*                &
                      & U(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0)*                    &
                      & U(COUNT1+1,COUNT2,0)-CZP(COUNT1,COUNT2,0)*                  &
                      & U(COUNT1,COUNT2,0)+CZP(COUNT1,COUNT2,0)*                    &
                      & U(COUNT1,COUNT2,0+1))+U(COUNT1,COUNT2,0)+                   &
                      & F*CONST(COUNT1,COUNT2,0)*(GOFTAV(COUNT1,COUNT2)/DZ(0)+      &
                      & TDBAV/DZ(0)/RSNWAV)
                    END IF
                  END DO
                  IF (ISNW.EQ.1) THEN
                    A(COUNT1,NYM1,0)=(3.d0-2.d0*F)*CONST(COUNT1,NYM1,0)*(-CYM(COUNT1,NYM1,0))
                    B(COUNT1,NYM1,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,NYM1,0)*             &
                        & (CYM(COUNT1,NYM1,0)+CYP(COUNT1,NYM1,0))
                    C(COUNT1,NYM1,0)=0.
                    R(COUNT1,NYM1,0)=F*CONST(COUNT1,NYM1,0)*(CXM(COUNT1,NYM1,0)*    &
                    & U(COUNT1-1,NYM1,0)-(CXM(COUNT1,NYM1,0)+CXP(COUNT1,NYM1,0))*   &
                    & U(COUNT1,NYM1,0)+CXP(COUNT1,NYM1,0)*U(COUNT1+1,NYM1,0)-       &
                    & CZP(COUNT1,NYM1,0)*U(COUNT1,NYM1,0)+CZP(COUNT1,NYM1,0)*       &
                    & U(COUNT1,NYM1,0+1))+U(COUNT1,NYM1,0)+F*CONST(COUNT1,NYM1,0)*  &
                    & (GOFTAV(COUNT1,NYM1)/DZ(0))+(3.d0-2.d0*F)*CONST(COUNT1,NYM1,0)*   &
                    & CYP(COUNT1,NYM1,0)*TGAV(0)
                  ELSE
                    A(COUNT1,NYM1,0)=(3.d0-2.d0*F)*CONST(COUNT1,NYM1,0)*(-CYM(COUNT1,NYM1,0))
                    B(COUNT1,NYM1,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,NYM1,0)*             &
                        & (CYM(COUNT1,NYM1,0)+CYP(COUNT1,NYM1,0))
                    C(COUNT1,NYM1,0)=0.
                    R(COUNT1,NYM1,0)=F*CONST(COUNT1,NYM1,0)*(CXM(COUNT1,NYM1,0)*    &
                    & U(COUNT1-1,NYM1,0)-(CXM(COUNT1,NYM1,0)+CXP(COUNT1,NYM1,0))*   &
                    & U(COUNT1,NYM1,0)+CXP(COUNT1,NYM1,0)*U(COUNT1+1,NYM1,0)-       &
                    & CZP(COUNT1,NYM1,0)*U(COUNT1,NYM1,0)+CZP(COUNT1,NYM1,0)*       &
                    & U(COUNT1,NYM1,0+1))+U(COUNT1,NYM1,0)+F*CONST(COUNT1,NYM1,0)*  &
                    & (GOFTAV(COUNT1,NYM1)/DZ(0)+TDBAV/DZ(0)/RSNWAV)+(3.d0-2.d0*F)*     &
                    & CONST(COUNT1,NYM1,0)*CYP(COUNT1,NYM1,0)*TGAV(0)
                  END IF
                  N=NYM1-0+1
                  L=1
                  DO COUNT2=0,NYM1
                    AA(L)=A(COUNT1,COUNT2,0)
                    BB(L)=B(COUNT1,COUNT2,0)
                    CC(L)=C(COUNT1,COUNT2,0)
                    RR(L)=R(COUNT1,COUNT2,0)
                    L=L+1
                  END DO
                  CALL TRIDI3D (AA,BB,CC,RR,N,X)
                  L=0
                  DO COUNT2=0,NYM1
                    V(COUNT1,COUNT2,0)=X(L+1)
                    L=L+1
                  END DO
                END DO

!***  INSIDE WALL CELLS, PARALLEL TO Y-AXIS (UPPER BAND)
                DO COUNT3=1,INT(1+(KBASE-NZAG)/2)
                    A(IBASE,0,COUNT3)=0.
                    B(IBASE,0,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,0,COUNT3)*           &
                        & CYP(IBASE,0,COUNT3)
                    C(IBASE,0,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,0,COUNT3)*              &
                        & (-CYP(IBASE,0,COUNT3))
                    R(IBASE,0,COUNT3)=F*CONST(IBASE,0,COUNT3)*(-(1.d0/DX(IBASE)/(RINT+ &
                    & 1.d0/HIN(6))+CXP(IBASE,0,COUNT3))*U(IBASE,0,COUNT3)+            &
                    & CXP(IBASE,0,COUNT3)*U(IBASE+1,0,COUNT3)+CZM(IBASE,0,COUNT3)*  &
                    & U(IBASE,0,COUNT3-1)-(CZM(IBASE,0,COUNT3)+CZP(IBASE,0,COUNT3))* &
                    & U(IBASE,0,COUNT3)+CZP(IBASE,0,COUNT3)*U(IBASE,0,COUNT3+1))+   &
                    & U(IBASE,0,COUNT3)+F*CONST(IBASE,0,COUNT3)*(TBAV/DX(IBASE)/    &
                    & (RINT+1.d0/HIN(6)))
                  DO COUNT2=1,JBASE-1
                      A(IBASE,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)*  &
                        & (-CYM(IBASE,COUNT2,COUNT3))
                      B(IBASE,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)* &
                        & (CYM(IBASE,COUNT2,COUNT3)+CYP(IBASE,COUNT2,COUNT3))
                      C(IBASE,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)*  &
                        & (-CYP(IBASE,COUNT2,COUNT3))
                      R(IBASE,COUNT2,COUNT3)=F*CONST(IBASE,COUNT2,COUNT3)*(-(1.d0/    &
                      & DX(IBASE)/(RINT+1.d0/HIN(6))+CXP(IBASE,COUNT2,COUNT3))*       &
                      & U(IBASE,COUNT2,COUNT3)+CXP(IBASE,COUNT2,COUNT3)*            &
                      & U(IBASE+1,COUNT2,COUNT3)+CZM(IBASE,COUNT2,COUNT3)*          &
                      & U(IBASE,COUNT2,COUNT3-1)-(CZM(IBASE,COUNT2,COUNT3)+         &
                      & CZP(IBASE,COUNT2,COUNT3))*U(IBASE,COUNT2,COUNT3)+           &
                      & CZP(IBASE,COUNT2,COUNT3)*U(IBASE,COUNT2,COUNT3+1))+         &
                      & U(IBASE,COUNT2,COUNT3)+F*CONST(IBASE,COUNT2,COUNT3)*        &
                      & (TBAV/DX(IBASE)/(RINT+1.d0/HIN(6)))
                  END DO

!***  FOUNDATION WALL/GROUND
                  DO COUNT2=JBASE,NYM1-1
                    A(IBASE,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)*    &
                        & (-CYM(IBASE,COUNT2,COUNT3))
                    B(IBASE,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)* &
                        & (CYM(IBASE,COUNT2,COUNT3)+CYP(IBASE,COUNT2,COUNT3))
                    C(IBASE,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)*    &
                        & (-CYP(IBASE,COUNT2,COUNT3))
                    R(IBASE,COUNT2,COUNT3)=F*CONST(IBASE,COUNT2,COUNT3)*            &
                    & (CXM(IBASE,COUNT2,COUNT3)*U(IBASE-1,COUNT2,COUNT3)-           &
                    & (CXM(IBASE,COUNT2,COUNT3)+CXP(IBASE,COUNT2,COUNT3))*          &
                    & U(IBASE,COUNT2,COUNT3)+CXP(IBASE,COUNT2,COUNT3)*              &
                    & U(IBASE+1,COUNT2,COUNT3)+CZM(IBASE,COUNT2,COUNT3)*            &
                    & U(IBASE,COUNT2,COUNT3-1)-(CZM(IBASE,COUNT2,COUNT3)+           &
                    & CZP(IBASE,COUNT2,COUNT3))*U(IBASE,COUNT2,COUNT3)+             &
                    & CZP(IBASE,COUNT2,COUNT3)*U(IBASE,COUNT2,COUNT3+1))+           &
                    & U(IBASE,COUNT2,COUNT3)
                  END DO
                  A(IBASE,NYM1,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,NYM1,COUNT3)*          &
                        & (-CYM(IBASE,NYM1,COUNT3))
                  B(IBASE,NYM1,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,NYM1,COUNT3)*       &
                        & (CYM(IBASE,NYM1,COUNT3)+CYP(IBASE,NYM1,COUNT3))
                  C(IBASE,NYM1,COUNT3)=0.
                  R(IBASE,NYM1,COUNT3)=F*CONST(IBASE,NYM1,COUNT3)*                  &
                  & (CXM(IBASE,NYM1,COUNT3)*U(IBASE-1,NYM1,COUNT3)-                 &
                  & (CXM(IBASE,NYM1,COUNT3)+CXP(IBASE,NYM1,COUNT3))*                &
                  & U(IBASE,NYM1,COUNT3)+CXP(IBASE,NYM1,COUNT3)*                    &
                  & U(IBASE+1,NYM1,COUNT3)+CZM(IBASE,NYM1,COUNT3)*                  &
                  & U(IBASE,NYM1,COUNT3-1)-(CZM(IBASE,NYM1,COUNT3)+                 &
                  & CZP(IBASE,NYM1,COUNT3))*U(IBASE,NYM1,COUNT3)+                   &
                  & CZP(IBASE,NYM1,COUNT3)*U(IBASE,NYM1,COUNT3+1))+                 &
                  & U(IBASE,NYM1,COUNT3)+(3.d0-2.d0*F)*CONST(IBASE,NYM1,COUNT3)*        &
                  & CYP(IBASE,NYM1,COUNT3)*TGAV(COUNT3)
                  N=NYM1-0+1
                  L=1
                  DO COUNT2=0,NYM1
                    AA(L)=A(IBASE,COUNT2,COUNT3)
                    BB(L)=B(IBASE,COUNT2,COUNT3)
                    CC(L)=C(IBASE,COUNT2,COUNT3)
                    RR(L)=R(IBASE,COUNT2,COUNT3)
                    L=L+1
                  END DO
                  CALL TRIDI3D (AA,BB,CC,RR,N,X)
                  L=0
                  DO COUNT2=0,NYM1
                    V(IBASE,COUNT2,COUNT3)=X(L+1)
                    L=L+1
                  END DO
                END DO

!***  INSIDE WALL CELLS, PARALLEL TO Y-AXIS (LOWER BAND)
                DO COUNT3=INT(2+(KBASE-NZAG)/2),KBASE-1
                    A(IBASE,0,COUNT3)=0.
                    B(IBASE,0,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,0,COUNT3)*       &
                      & CYP(IBASE,0,COUNT3)
                    C(IBASE,0,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,0,COUNT3)*          &
                      & (-CYP(IBASE,0,COUNT3))
                    R(IBASE,0,COUNT3)=F*CONST(IBASE,0,COUNT3)*(-(1.d0/DX(IBASE)/  &
                    & (RINT+1.d0/HIN(6))+CXP(IBASE,0,COUNT3))*U(IBASE,0,COUNT3)+  &
                    & CXP(IBASE,0,COUNT3)*U(IBASE+1,0,COUNT3)+CZM(IBASE,0,COUNT3)* &
                    & U(IBASE,0,COUNT3-1)-(CZM(IBASE,0,COUNT3)+                 &
                    & CZP(IBASE,0,COUNT3))*U(IBASE,0,COUNT3)+CZP(IBASE,0,COUNT3)* &
                    & U(IBASE,0,COUNT3+1))+U(IBASE,0,COUNT3)+F*                 &
                    & CONST(IBASE,0,COUNT3)*(TBAV/DX(IBASE)/(RINT+1.d0/HIN(6)))
                 DO COUNT2=1,JBASE-1
                      A(IBASE,COUNT2,COUNT3)=(3.d0-2.d0*F)*                         &
                         & CONST(IBASE,COUNT2,COUNT3)*(-CYM(IBASE,COUNT2,COUNT3))
                      B(IBASE,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*                      &
                         & CONST(IBASE,COUNT2,COUNT3)*(CYM(IBASE,COUNT2,COUNT3)+ &
                         & CYP(IBASE,COUNT2,COUNT3))
                      C(IBASE,COUNT2,COUNT3)=(3.d0-2.d0*F)*                         &
                         & CONST(IBASE,COUNT2,COUNT3)*(-CYP(IBASE,COUNT2,COUNT3))
                      R(IBASE,COUNT2,COUNT3)=F*CONST(IBASE,COUNT2,COUNT3)*      &
                      & (-(1.d0/DX(IBASE)/(RINT+1.d0/HIN(6))+                       &
                      & CXP(IBASE,COUNT2,COUNT3))*U(IBASE,COUNT2,COUNT3)+       &
                      & CXP(IBASE,COUNT2,COUNT3)*U(IBASE+1,COUNT2,COUNT3)+      &
                      & CZM(IBASE,COUNT2,COUNT3)*U(IBASE,COUNT2,COUNT3-1)-      &
                      & (CZM(IBASE,COUNT2,COUNT3)+CZP(IBASE,COUNT2,COUNT3))*    &
                      & U(IBASE,COUNT2,COUNT3)+CZP(IBASE,COUNT2,COUNT3)*        &
                      & U(IBASE,COUNT2,COUNT3+1))+U(IBASE,COUNT2,COUNT3)+F*     &
                      & CONST(IBASE,COUNT2,COUNT3)*(TBAV/DX(IBASE)/(RINT+1.d0/HIN(6)))
                  END DO

!***  FOUNDATION WALL/GRAVEL/GROUND
                  DO COUNT2=JBASE,NYM1-1
                    A(IBASE,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)* &
                         & (-CYM(IBASE,COUNT2,COUNT3))
                    B(IBASE,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*                        &
                         & CONST(IBASE,COUNT2,COUNT3)*                          &
                         & (CYM(IBASE,COUNT2,COUNT3)+CYP(IBASE,COUNT2,COUNT3))
                    C(IBASE,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)* &
                         & (-CYP(IBASE,COUNT2,COUNT3))
                    R(IBASE,COUNT2,COUNT3)=F*CONST(IBASE,COUNT2,COUNT3)*        &
                    & (CXM(IBASE,COUNT2,COUNT3)*U(IBASE-1,COUNT2,COUNT3)-       &
                    & (CXM(IBASE,COUNT2,COUNT3)+CXP(IBASE,COUNT2,COUNT3))*      &
                    & U(IBASE,COUNT2,COUNT3)+CXP(IBASE,COUNT2,COUNT3)*          &
                    & U(IBASE+1,COUNT2,COUNT3)+CZM(IBASE,COUNT2,COUNT3)*        &
                    & U(IBASE,COUNT2,COUNT3-1)-(CZM(IBASE,COUNT2,COUNT3)+       &
                    & CZP(IBASE,COUNT2,COUNT3))*U(IBASE,COUNT2,COUNT3)+         &
                    & CZP(IBASE,COUNT2,COUNT3)*U(IBASE,COUNT2,COUNT3+1))+       &
                    & U(IBASE,COUNT2,COUNT3)
                  END DO
                  A(IBASE,NYM1,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,NYM1,COUNT3)*      &
                    & (-CYM(IBASE,NYM1,COUNT3))
                  B(IBASE,NYM1,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,NYM1,COUNT3)*   &
                    & (CYM(IBASE,NYM1,COUNT3)+CYP(IBASE,NYM1,COUNT3))
                  C(IBASE,NYM1,COUNT3)=0.
                  R(IBASE,NYM1,COUNT3)=F*CONST(IBASE,NYM1,COUNT3)*              &
                  & (CXM(IBASE,NYM1,COUNT3)*U(IBASE-1,NYM1,COUNT3)-             &
                  & (CXM(IBASE,NYM1,COUNT3)+CXP(IBASE,NYM1,COUNT3))*            &
                  & U(IBASE,NYM1,COUNT3)+CXP(IBASE,NYM1,COUNT3)*                &
                  & U(IBASE+1,NYM1,COUNT3)+CZM(IBASE,NYM1,COUNT3)*              &
                  & U(IBASE,NYM1,COUNT3-1)-(CZM(IBASE,NYM1,COUNT3)+             &
                  & CZP(IBASE,NYM1,COUNT3))*U(IBASE,NYM1,COUNT3)+               &
                  & CZP(IBASE,NYM1,COUNT3)*U(IBASE,NYM1,COUNT3+1))+             &
                  & U(IBASE,NYM1,COUNT3)+(3.d0-2.d0*F)*CONST(IBASE,NYM1,COUNT3)*    &
                  & CYP(IBASE,NYM1,COUNT3)*TGAV(COUNT3)
                  N=NYM1-0+1
                  L=1
                  DO COUNT2=0,NYM1
                    AA(L)=A(IBASE,COUNT2,COUNT3)
                    BB(L)=B(IBASE,COUNT2,COUNT3)
                    CC(L)=C(IBASE,COUNT2,COUNT3)
                    RR(L)=R(IBASE,COUNT2,COUNT3)
                    L=L+1
                  END DO
                  CALL TRIDI3D (AA,BB,CC,RR,N,X)
                  L=0
                  DO COUNT2=0,NYM1
                    V(IBASE,COUNT2,COUNT3)=X(L+1)
                    L=L+1
                  END DO
                END DO

!***  FLOOR SLAB/GRAVEL/GROUND
                DO COUNT3=KBASE,NZBGM1
                  A(IBASE,0,COUNT3)=0.
                  B(IBASE,0,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,0,COUNT3)*         &
                    & CYP(IBASE,0,COUNT3)
                  C(IBASE,0,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,0,COUNT3)*            &
                    & (-CYP(IBASE,0,COUNT3))
                  R(IBASE,0,COUNT3)=F*CONST(IBASE,0,COUNT3)*                    &
                  & (CXM(IBASE,0,COUNT3)*U(IBASE-1,0,COUNT3)-                   &
                  & (CXM(IBASE,0,COUNT3)+CXP(IBASE,0,COUNT3))*                  &
                  & U(IBASE,0,COUNT3)+CXP(IBASE,0,COUNT3)*                      &
                  & U(IBASE+1,0,COUNT3)+CZM(IBASE,0,COUNT3)*                    &
                  & U(IBASE,0,COUNT3-1)-(CZM(IBASE,0,COUNT3)+                   &
                  & CZP(IBASE,0,COUNT3))*U(IBASE,0,COUNT3)+                     &
                  & CZP(IBASE,0,COUNT3)*U(IBASE,0,COUNT3+1))+U(IBASE,0,COUNT3)
                  DO COUNT2=1,NYM1-1
                    A(IBASE,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)* &
                         & (-CYM(IBASE,COUNT2,COUNT3))
                    B(IBASE,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*                        &
                         & CONST(IBASE,COUNT2,COUNT3)*(CYM(IBASE,COUNT2,COUNT3)+ &
                         & CYP(IBASE,COUNT2,COUNT3))
                    C(IBASE,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)* &
                         & (-CYP(IBASE,COUNT2,COUNT3))
                    R(IBASE,COUNT2,COUNT3)=F*CONST(IBASE,COUNT2,COUNT3)*        &
                    & (CXM(IBASE,COUNT2,COUNT3)*U(IBASE-1,COUNT2,COUNT3)-       &
                    & (CXM(IBASE,COUNT2,COUNT3)+CXP(IBASE,COUNT2,COUNT3))*      &
                    & U(IBASE,COUNT2,COUNT3)+CXP(IBASE,COUNT2,COUNT3)*          &
                    & U(IBASE+1,COUNT2,COUNT3)+CZM(IBASE,COUNT2,COUNT3)*        &
                    & U(IBASE,COUNT2,COUNT3-1)-(CZM(IBASE,COUNT2,COUNT3)+       &
                    & CZP(IBASE,COUNT2,COUNT3))*U(IBASE,COUNT2,COUNT3)+         &
                    & CZP(IBASE,COUNT2,COUNT3)*U(IBASE,COUNT2,COUNT3+1))+       &
                    & U(IBASE,COUNT2,COUNT3)
                  END DO
                  A(IBASE,NYM1,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,NYM1,COUNT3)*      &
                    & (-CYM(IBASE,NYM1,COUNT3))
                  B(IBASE,NYM1,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,NYM1,COUNT3)*   &
                    & (CYM(IBASE,NYM1,COUNT3)+CYP(IBASE,NYM1,COUNT3))
                  C(IBASE,NYM1,COUNT3)=0.
                  R(IBASE,NYM1,COUNT3)=F*CONST(IBASE,NYM1,COUNT3)*              &
                  & (CXM(IBASE,NYM1,COUNT3)*U(IBASE-1,NYM1,COUNT3)-             &
                  & (CXM(IBASE,NYM1,COUNT3)+CXP(IBASE,NYM1,COUNT3))*            &
                  & U(IBASE,NYM1,COUNT3)+CXP(IBASE,NYM1,COUNT3)*                &
                  & U(IBASE+1,NYM1,COUNT3)+CZM(IBASE,NYM1,COUNT3)*              &
                  & U(IBASE,NYM1,COUNT3-1)-(CZM(IBASE,NYM1,COUNT3)+             &
                  & CZP(IBASE,NYM1,COUNT3))*U(IBASE,NYM1,COUNT3)+               &
                  & CZP(IBASE,NYM1,COUNT3)*U(IBASE,NYM1,COUNT3+1))+             &
                  & U(IBASE,NYM1,COUNT3)+(3.d0-2.d0*F)*CONST(IBASE,NYM1,COUNT3)*    &
                  & CYP(IBASE,NYM1,COUNT3)*TGAV(COUNT3)
                  N=NYM1-0+1
                  L=1
                  DO COUNT2=0,NYM1
                    AA(L)=A(IBASE,COUNT2,COUNT3)
                    BB(L)=B(IBASE,COUNT2,COUNT3)
                    CC(L)=C(IBASE,COUNT2,COUNT3)
                    RR(L)=R(IBASE,COUNT2,COUNT3)
                    L=L+1
                  END DO
                  CALL TRIDI3D (AA,BB,CC,RR,N,X)
                  L=0
                  DO COUNT2=0,NYM1
                    V(IBASE,COUNT2,COUNT3)=X(L+1)
                    L=L+1
                  END DO
                END DO

!***  FOUNDATION WALL/GRAVEL/GROUND
                DO COUNT3=1,NZBGM1
                  DO COUNT1=IBASE+1,NXM1
                    A(COUNT1,0,COUNT3)=0.
                    B(COUNT1,0,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,0,COUNT3)*     &
                         & CYP(COUNT1,0,COUNT3)
                    C(COUNT1,0,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,0,COUNT3)*        &
                         & (-CYP(COUNT1,0,COUNT3))
                    R(COUNT1,0,COUNT3)=F*CONST(COUNT1,0,COUNT3)*                &
                    & (CXM(COUNT1,0,COUNT3)*U(COUNT1-1,0,COUNT3)-               &
                    & (CXM(COUNT1,0,COUNT3)+CXP(COUNT1,0,COUNT3))*              &
                    & U(COUNT1,0,COUNT3)+CXP(COUNT1,0,COUNT3)*                  &
                    & U(COUNT1+1,0,COUNT3)+CZM(COUNT1,0,COUNT3)*                &
                    & U(COUNT1,0,COUNT3-1)-(CZM(COUNT1,0,COUNT3)+               &
                    & CZP(COUNT1,0,COUNT3))*U(COUNT1,0,COUNT3)+                 &
                    & CZP(COUNT1,0,COUNT3)*U(COUNT1,0,COUNT3+1))+U(COUNT1,0,COUNT3)
                    DO COUNT2=1,NYM1-1
                      A(COUNT1,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,COUNT3)* &
                         & (-CYM(COUNT1,COUNT2,COUNT3))
                      B(COUNT1,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*                     &
                         & CONST(COUNT1,COUNT2,COUNT3)*(CYM(COUNT1,COUNT2,COUNT3)+ &
                         & CYP(COUNT1,COUNT2,COUNT3))
                      C(COUNT1,COUNT2,COUNT3)=(3.d0-2.d0*F)*                        &
                         & CONST(COUNT1,COUNT2,COUNT3)*(-CYP(COUNT1,COUNT2,COUNT3))
                      R(COUNT1,COUNT2,COUNT3)=F*CONST(COUNT1,COUNT2,COUNT3)*    &
                      & (CXM(COUNT1,COUNT2,COUNT3)*U(COUNT1-1,COUNT2,COUNT3)-   &
                      & (CXM(COUNT1,COUNT2,COUNT3)+CXP(COUNT1,COUNT2,COUNT3))*  &
                      & U(COUNT1,COUNT2,COUNT3)+CXP(COUNT1,COUNT2,COUNT3)*      &
                      & U(COUNT1+1,COUNT2,COUNT3)+CZM(COUNT1,COUNT2,COUNT3)*    &
                      & U(COUNT1,COUNT2,COUNT3-1)-(CZM(COUNT1,COUNT2,COUNT3)+   &
                      & CZP(COUNT1,COUNT2,COUNT3))*U(COUNT1,COUNT2,COUNT3)+     &
                      & CZP(COUNT1,COUNT2,COUNT3)*U(COUNT1,COUNT2,COUNT3+1))+   &
                      & U(COUNT1,COUNT2,COUNT3)
                    END DO
                    A(COUNT1,NYM1,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,NYM1,COUNT3)*  &
                         & (-CYM(COUNT1,NYM1,COUNT3))
                    B(COUNT1,NYM1,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,NYM1,COUNT3)* &
                         & (CYM(COUNT1,NYM1,COUNT3)+CYP(COUNT1,NYM1,COUNT3))
                    C(COUNT1,NYM1,COUNT3)=0.
                    R(COUNT1,NYM1,COUNT3)=F*CONST(COUNT1,NYM1,COUNT3)*          &
                    & (CXM(COUNT1,NYM1,COUNT3)*U(COUNT1-1,NYM1,COUNT3)-         &
                    & (CXM(COUNT1,NYM1,COUNT3)+CXP(COUNT1,NYM1,COUNT3))*        &
                    & U(COUNT1,NYM1,COUNT3)+CXP(COUNT1,NYM1,COUNT3)*            &
                    & U(COUNT1+1,NYM1,COUNT3)+CZM(COUNT1,NYM1,COUNT3)*          &
                    & U(COUNT1,NYM1,COUNT3-1)-(CZM(COUNT1,NYM1,COUNT3)+         &
                    & CZP(COUNT1,NYM1,COUNT3))*U(COUNT1,NYM1,COUNT3)+           &
                    & CZP(COUNT1,NYM1,COUNT3)*U(COUNT1,NYM1,COUNT3+1))+         &
                    & U(COUNT1,NYM1,COUNT3)+(3.d0-2.d0*F)*CONST(COUNT1,NYM1,COUNT3)* &
                    & CYP(COUNT1,NYM1,COUNT3)*TGAV(COUNT3)
                    N=NYM1-0+1
                    L=1
                    DO COUNT2=0,NYM1
                      AA(L)=A(COUNT1,COUNT2,COUNT3)
                      BB(L)=B(COUNT1,COUNT2,COUNT3)
                      CC(L)=C(COUNT1,COUNT2,COUNT3)
                      RR(L)=R(COUNT1,COUNT2,COUNT3)
                      L=L+1
                    END DO
                    CALL TRIDI3D (AA,BB,CC,RR,N,X)
                    L=0
                    DO COUNT2=0,NYM1
                      V(COUNT1,COUNT2,COUNT3)=X(L+1)
                      L=L+1
                    END DO
                  END DO
                END DO

!***  SECTION 7:  BELOW-GRADE (2)
!***  INSIDE WALL CELL, PARALLEL TO X-AXIS (UPPER BAND) COUNT3=0
                DO COUNT1=0,IBASE-1
                    A(COUNT1,JBASE,0)=0.
                    B(COUNT1,JBASE,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,0)*       &
                         & (1.d0/DY(JBASE)/(RINT+1.d0/HIN(6))+CYP(COUNT1,JBASE,0))
                    C(COUNT1,JBASE,0)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,0)*          &
                         & (-CYP(COUNT1,JBASE,0))
                    R(COUNT1,JBASE,0)=F*CONST(COUNT1,JBASE,0)*                  &
                    & (CXM(COUNT1,JBASE,0)*U(COUNT1-1,JBASE,0)-                 &
                    & (CXM(COUNT1,JBASE,0)+CXP(COUNT1,JBASE,0))*                &
                    & U(COUNT1,JBASE,0)+CXP(COUNT1,JBASE,0)*U(COUNT1+1,JBASE,0)+ &
                    & CZM(COUNT1,JBASE,0)*U(COUNT1,JBASE,0-1)-                  &
                    & (CZM(COUNT1,JBASE,0)+CZP(COUNT1,JBASE,0))*U(COUNT1,JBASE,0)+ &
                    & CZP(COUNT1,JBASE,0)*U(COUNT1,JBASE,0+1))+U(COUNT1,JBASE,0)+ &
                    & (3.d0-2.d0*F)*CONST(COUNT1,JBASE,0)*(TBAV/DY(JBASE)/          &
                    & (RINT+1.d0/HIN(6)))
!***  FOUNDATION WALL CELLS COUNT3=0
                  DO COUNT2=JBASE+1,JBASE+2
                    A(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*        &
                         & (-CYM(COUNT1,COUNT2,0))
                    B(COUNT1,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*     &
                         & (CYM(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0))
                    C(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*        &
                         & (-CYP(COUNT1,COUNT2,0))
                    R(COUNT1,COUNT2,0)=F*CONST(COUNT1,COUNT2,0)*                &
                    & (CXM(COUNT1,COUNT2,0)*U(COUNT1-1,COUNT2,0)-               &
                    & (CXM(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0))*              &
                    & U(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0)*                  &
                    & U(COUNT1+1,COUNT2,0)+CZM(COUNT1,COUNT2,0)*                &
                    & U(COUNT1,COUNT2,0-1)-(CZM(COUNT1,COUNT2,0)+               &
                    & CZP(COUNT1,COUNT2,0))*U(COUNT1,COUNT2,0)+                 &
                    & CZP(COUNT1,COUNT2,0)*U(COUNT1,COUNT2,0+1))+               &
                    & U(COUNT1,COUNT2,0)
                  END DO

!***  GROUND SURFACE CELLS
                  DO COUNT2=JBASE+3,NYM1-1
                    IF (ISNW.EQ.1) THEN
                      A(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*      &
                         & (-CYM(COUNT1,COUNT2,0))
                      B(COUNT1,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*   &
                         & (CYM(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0))
                      C(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*      &
                         & (-CYP(COUNT1,COUNT2,0))
                      R(COUNT1,COUNT2,0)=F*CONST(COUNT1,COUNT2,0)*              &
                      & (CXM(COUNT1,COUNT2,0)*U(COUNT1-1,COUNT2,0)-             &
                      & (CXM(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0))*            &
                      & U(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0)*                &
                      & U(COUNT1+1,COUNT2,0)-CZP(COUNT1,COUNT2,0)*              &
                      & U(COUNT1,COUNT2,0)+CZP(COUNT1,COUNT2,0)*                &
                      & U(COUNT1,COUNT2,0+1))+U(COUNT1,COUNT2,0)+               &
                      & F*CONST(COUNT1,COUNT2,0)*(GOFTAV(COUNT1,COUNT2)/DZ(0))
                    ELSE
                      A(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*      &
                         & (-CYM(COUNT1,COUNT2,0))
                      B(COUNT1,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*   &
                         & (CYM(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0))
                      C(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*      &
                         & (-CYP(COUNT1,COUNT2,0))
                      R(COUNT1,COUNT2,0)=F*CONST(COUNT1,COUNT2,0)*              &
                      & (CXM(COUNT1,COUNT2,0)*U(COUNT1-1,COUNT2,0)-             &
                      & (CXM(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0))*            &
                      & U(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0)*                &
                      & U(COUNT1+1,COUNT2,0)-CZP(COUNT1,COUNT2,0)*              &
                      & U(COUNT1,COUNT2,0)+CZP(COUNT1,COUNT2,0)*                &
                      & U(COUNT1,COUNT2,0+1))+U(COUNT1,COUNT2,0)+               &
                      & F*CONST(COUNT1,COUNT2,0)*(GOFTAV(COUNT1,COUNT2)/DZ(0)+  &
                      & TDBAV/DZ(0)/RSNWAV)
                    END IF
                  END DO
                  IF (ISNW.EQ.1) THEN
                    A(COUNT1,NYM1,0)=(3.d0-2.d0*F)*CONST(COUNT1,NYM1,0)*(-CYM(COUNT1,NYM1,0))
                    B(COUNT1,NYM1,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,NYM1,0)*         &
                         & (CYM(COUNT1,NYM1,0)+CYP(COUNT1,NYM1,0))
                    C(COUNT1,NYM1,0)=0.
                    R(COUNT1,NYM1,0)=F*CONST(COUNT1,NYM1,0)*(CXM(COUNT1,NYM1,0)* &
                    & U(COUNT1-1,NYM1,0)-(CXM(COUNT1,NYM1,0)+CXP(COUNT1,NYM1,0))* &
                    & U(COUNT1,NYM1,0)+CXP(COUNT1,NYM1,0)*U(COUNT1+1,NYM1,0)-   &
                    & CZP(COUNT1,NYM1,0)*U(COUNT1,NYM1,0)+CZP(COUNT1,NYM1,0)*   &
                    & U(COUNT1,NYM1,0+1))+U(COUNT1,NYM1,0)+F*CONST(COUNT1,NYM1,0)* &
                    & (GOFTAV(COUNT1,NYM1)/DZ(0))+(3.d0-2.d0*F)*CONST(COUNT1,NYM1,0)* &
                    & CYP(COUNT1,NYM1,0)*TGAV(0)
                  ELSE
                    A(COUNT1,NYM1,0)=(3.d0-2.d0*F)*CONST(COUNT1,NYM1,0)*(-CYM(COUNT1,NYM1,0))
                    B(COUNT1,NYM1,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,NYM1,0)*         &
                         & (CYM(COUNT1,NYM1,0)+CYP(COUNT1,NYM1,0))
                    C(COUNT1,NYM1,0)=0.
                    R(COUNT1,NYM1,0)=F*CONST(COUNT1,NYM1,0)*(CXM(COUNT1,NYM1,0)* &
                    & U(COUNT1-1,NYM1,0)-(CXM(COUNT1,NYM1,0)+CXP(COUNT1,NYM1,0))* &
                    & U(COUNT1,NYM1,0)+CXP(COUNT1,NYM1,0)*U(COUNT1+1,NYM1,0)-   &
                    & CZP(COUNT1,NYM1,0)*U(COUNT1,NYM1,0)+CZP(COUNT1,NYM1,0)*   &
                    & U(COUNT1,NYM1,0+1))+U(COUNT1,NYM1,0)+F*                   &
                    & CONST(COUNT1,NYM1,0)*(GOFTAV(COUNT1,NYM1)/DZ(0)+TDBAV/    &
                    & DZ(0)/RSNWAV)+(3.d0-2.d0*F)*CONST(COUNT1,NYM1,0)*             &
                    & CYP(COUNT1,NYM1,0)*TGAV(0)
                  END IF
                  N=NYM1-JBASE+1
                  L=1
                  DO COUNT2=JBASE,NYM1
                    AA(L)=A(COUNT1,COUNT2,0)
                    BB(L)=B(COUNT1,COUNT2,0)
                    CC(L)=C(COUNT1,COUNT2,0)
                    RR(L)=R(COUNT1,COUNT2,0)
                    L=L+1
                  END DO
                  CALL TRIDI3D (AA,BB,CC,RR,N,X)
                  L=0
                  DO COUNT2=JBASE,NYM1
                    V(COUNT1,COUNT2,0)=X(L+1)
                    L=L+1
                  END DO
                END DO

!***  INSIDE WALL CELLS, PARALLEL TO X-AXIS (UPPER BAND)
                DO COUNT3=1,INT(1+(KBASE-NZAG)/2)
                  DO COUNT1=0,IBASE-1
                      A(COUNT1,JBASE,COUNT3)=0.
                      B(COUNT1,JBASE,COUNT3)=1.d0+(3.d0-2.d0*F)*                      &
                         & CONST(COUNT1,JBASE,COUNT3)*(1.d0/DY(JBASE)/(RINT+1.d0/   &
                         & HIN(6))+CYP(COUNT1,JBASE,COUNT3))
                      C(COUNT1,JBASE,COUNT3)=(3.d0-2.d0*F)*                         &
                         & CONST(COUNT1,JBASE,COUNT3)*                          &
                         & (-CYP(COUNT1,JBASE,COUNT3))
                      R(COUNT1,JBASE,COUNT3)=F*CONST(COUNT1,JBASE,COUNT3)*      &
                      & (CXM(COUNT1,JBASE,COUNT3)*U(COUNT1-1,JBASE,COUNT3)-     &
                      & (CXM(COUNT1,JBASE,COUNT3)+CXP(COUNT1,JBASE,COUNT3))*    &
                      & U(COUNT1,JBASE,COUNT3)+CXP(COUNT1,JBASE,COUNT3)*        &
                      & U(COUNT1+1,JBASE,COUNT3)+CZM(COUNT1,JBASE,COUNT3)*      &
                      & U(COUNT1,JBASE,COUNT3-1)-(CZM(COUNT1,JBASE,COUNT3)+     &
                      & CZP(COUNT1,JBASE,COUNT3))*U(COUNT1,JBASE,COUNT3)+       &
                      & CZP(COUNT1,JBASE,COUNT3)*U(COUNT1,JBASE,COUNT3+1))+     &
                      & U(COUNT1,JBASE,COUNT3)+(3.d0-2.d0*F)*                       &
                      & CONST(COUNT1,JBASE,COUNT3)*(TBAV/DY(JBASE)/             &
                      & (RINT+1.d0/HIN(6)))
!***  FOUNDATION WALL/GROUND
                    DO COUNT2=JBASE+1,NYM1-1
                      A(COUNT1,COUNT2,COUNT3)=(3.d0-2.d0*F)*                        &
                         & CONST(COUNT1,COUNT2,COUNT3)*(-CYM(COUNT1,COUNT2,COUNT3))
                      B(COUNT1,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*                     &
                         & CONST(COUNT1,COUNT2,COUNT3)*                         &
                         & (CYM(COUNT1,COUNT2,COUNT3)+CYP(COUNT1,COUNT2,COUNT3))
                      C(COUNT1,COUNT2,COUNT3)=(3.d0-2.d0*F)*                        &
                         & CONST(COUNT1,COUNT2,COUNT3)*(-CYP(COUNT1,COUNT2,COUNT3))
                      R(COUNT1,COUNT2,COUNT3)=F*CONST(COUNT1,COUNT2,COUNT3)*    &
                      & (CXM(COUNT1,COUNT2,COUNT3)*U(COUNT1-1,COUNT2,COUNT3)-   &
                      & (CXM(COUNT1,COUNT2,COUNT3)+CXP(COUNT1,COUNT2,COUNT3))*  &
                      & U(COUNT1,COUNT2,COUNT3)+CXP(COUNT1,COUNT2,COUNT3)*      &
                      & U(COUNT1+1,COUNT2,COUNT3)+CZM(COUNT1,COUNT2,COUNT3)*    &
                      & U(COUNT1,COUNT2,COUNT3-1)-(CZM(COUNT1,COUNT2,COUNT3)+   &
                      & CZP(COUNT1,COUNT2,COUNT3))*U(COUNT1,COUNT2,COUNT3)+     &
                      & CZP(COUNT1,COUNT2,COUNT3)*U(COUNT1,COUNT2,COUNT3+1))+   &
                      & U(COUNT1,COUNT2,COUNT3)
                    END DO
                    A(COUNT1,NYM1,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,NYM1,COUNT3)*  &
                       & (-CYM(COUNT1,NYM1,COUNT3))
                    B(COUNT1,NYM1,COUNT3)=1.d0+(3.d0-2.d0*F)*                         &
                       & CONST(COUNT1,NYM1,COUNT3)*(CYM(COUNT1,NYM1,COUNT3)+    &
                       & CYP(COUNT1,NYM1,COUNT3))
                    C(COUNT1,NYM1,COUNT3)=0.
                    R(COUNT1,NYM1,COUNT3)=F*CONST(COUNT1,NYM1,COUNT3)*          &
                    & (CXM(COUNT1,NYM1,COUNT3)*U(COUNT1-1,NYM1,COUNT3)-         &
                    & (CXM(COUNT1,NYM1,COUNT3)+CXP(COUNT1,NYM1,COUNT3))*        &
                    & U(COUNT1,NYM1,COUNT3)+CXP(COUNT1,NYM1,COUNT3)*            &
                    & U(COUNT1+1,NYM1,COUNT3)+CZM(COUNT1,NYM1,COUNT3)*          &
                    & U(COUNT1,NYM1,COUNT3-1)-(CZM(COUNT1,NYM1,COUNT3)+         &
                    & CZP(COUNT1,NYM1,COUNT3))*U(COUNT1,NYM1,COUNT3)+           &
                    & CZP(COUNT1,NYM1,COUNT3)*U(COUNT1,NYM1,COUNT3+1))+         &
                    & U(COUNT1,NYM1,COUNT3)+(3.d0-2.d0*F)*                          &
                    & CONST(COUNT1,NYM1,COUNT3)*CYP(COUNT1,NYM1,COUNT3)*TGAV(COUNT3)
                    N=NYM1-JBASE+1
                    L=1
                    DO COUNT2=JBASE,NYM1
                      AA(L)=A(COUNT1,COUNT2,COUNT3)
                      BB(L)=B(COUNT1,COUNT2,COUNT3)
                      CC(L)=C(COUNT1,COUNT2,COUNT3)
                      RR(L)=R(COUNT1,COUNT2,COUNT3)
                      L=L+1
                    END DO
                    CALL TRIDI3D (AA,BB,CC,RR,N,X)
                    L=0
                    DO COUNT2=JBASE,NYM1
                      V(COUNT1,COUNT2,COUNT3)=X(L+1)
                      L=L+1
                    END DO
                  END DO
                END DO

!***  INSIDE WALL CELLS, PARALLEL TO X-AXIS (LOWER BAND)
                DO COUNT3=INT(2+(KBASE-NZAG)/2),KBASE-1
                  DO COUNT1=0,IBASE-1
                      A(COUNT1,JBASE,COUNT3)=0.
                      B(COUNT1,JBASE,COUNT3)=1.d0+(3.d0-2.d0*F)*                      &
                         & CONST(COUNT1,JBASE,COUNT3)*(1.d0/DY(JBASE)/(RINT+1.d0/   &
                         & HIN(6))+CYP(COUNT1,JBASE,COUNT3))
                      C(COUNT1,JBASE,COUNT3)=(3.d0-2.d0*F)*                         &
                         & CONST(COUNT1,JBASE,COUNT3)*(-CYP(COUNT1,JBASE,COUNT3))
                      R(COUNT1,JBASE,COUNT3)=F*CONST(COUNT1,JBASE,COUNT3)*      &
                      & (CXM(COUNT1,JBASE,COUNT3)*U(COUNT1-1,JBASE,COUNT3)-     &
                      & (CXM(COUNT1,JBASE,COUNT3)+CXP(COUNT1,JBASE,COUNT3))*    &
                      & U(COUNT1,JBASE,COUNT3)+CXP(COUNT1,JBASE,COUNT3)*        &
                      & U(COUNT1+1,JBASE,COUNT3)+CZM(COUNT1,JBASE,COUNT3)*      &
                      & U(COUNT1,JBASE,COUNT3-1)-(CZM(COUNT1,JBASE,COUNT3)+     &
                      & CZP(COUNT1,JBASE,COUNT3))*U(COUNT1,JBASE,COUNT3)+       &
                      & CZP(COUNT1,JBASE,COUNT3)*U(COUNT1,JBASE,COUNT3+1))+     &
                      & U(COUNT1,JBASE,COUNT3)+(3.d0-2.d0*F)*                       &
                      & CONST(COUNT1,JBASE,COUNT3)*(TBAV/DY(JBASE)/(RINT+1.d0/HIN(6)))
!***  FOUNDATION WALL/GRAVEL/GROUND
                    DO COUNT2=JBASE+1,NYM1-1
                      A(COUNT1,COUNT2,COUNT3)=(3.d0-2.d0*F)*                        &
                         & CONST(COUNT1,COUNT2,COUNT3)*(-CYM(COUNT1,COUNT2,COUNT3))
                      B(COUNT1,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*                     &
                         & CONST(COUNT1,COUNT2,COUNT3)*(CYM(COUNT1,COUNT2,COUNT3)+ &
                         & CYP(COUNT1,COUNT2,COUNT3))
                      C(COUNT1,COUNT2,COUNT3)=(3.d0-2.d0*F)*                        &
                         & CONST(COUNT1,COUNT2,COUNT3)*(-CYP(COUNT1,COUNT2,COUNT3))
                      R(COUNT1,COUNT2,COUNT3)=F*CONST(COUNT1,COUNT2,COUNT3)*    &
                      & (CXM(COUNT1,COUNT2,COUNT3)*U(COUNT1-1,COUNT2,COUNT3)-   &
                      & (CXM(COUNT1,COUNT2,COUNT3)+CXP(COUNT1,COUNT2,COUNT3))*  &
                      & U(COUNT1,COUNT2,COUNT3)+CXP(COUNT1,COUNT2,COUNT3)*      &
                      & U(COUNT1+1,COUNT2,COUNT3)+CZM(COUNT1,COUNT2,COUNT3)*    &
                      & U(COUNT1,COUNT2,COUNT3-1)-(CZM(COUNT1,COUNT2,COUNT3)+   &
                      & CZP(COUNT1,COUNT2,COUNT3))*U(COUNT1,COUNT2,COUNT3)+     &
                      & CZP(COUNT1,COUNT2,COUNT3)*U(COUNT1,COUNT2,COUNT3+1))+   &
                      & U(COUNT1,COUNT2,COUNT3)
                    END DO
                    A(COUNT1,NYM1,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,NYM1,COUNT3)*  &
                       & (-CYM(COUNT1,NYM1,COUNT3))
                    B(COUNT1,NYM1,COUNT3)=1.d0+(3.d0-2.d0*F)*                         &
                       & CONST(COUNT1,NYM1,COUNT3)*(CYM(COUNT1,NYM1,COUNT3)+    &
                       & CYP(COUNT1,NYM1,COUNT3))
                    C(COUNT1,NYM1,COUNT3)=0.
                    R(COUNT1,NYM1,COUNT3)=F*CONST(COUNT1,NYM1,COUNT3)*          &
                    & (CXM(COUNT1,NYM1,COUNT3)*U(COUNT1-1,NYM1,COUNT3)-         &
                    & (CXM(COUNT1,NYM1,COUNT3)+CXP(COUNT1,NYM1,COUNT3))*        &
                    & U(COUNT1,NYM1,COUNT3)+CXP(COUNT1,NYM1,COUNT3)*            &
                    & U(COUNT1+1,NYM1,COUNT3)+CZM(COUNT1,NYM1,COUNT3)*          &
                    & U(COUNT1,NYM1,COUNT3-1)-(CZM(COUNT1,NYM1,COUNT3)+         &
                    & CZP(COUNT1,NYM1,COUNT3))*U(COUNT1,NYM1,COUNT3)+           &
                    & CZP(COUNT1,NYM1,COUNT3)*U(COUNT1,NYM1,COUNT3+1))+         &
                    & U(COUNT1,NYM1,COUNT3)+(3.d0-2.d0*F)*                          &
                    & CONST(COUNT1,NYM1,COUNT3)*CYP(COUNT1,NYM1,COUNT3)*        &
                    & TGAV(COUNT3)
                    N=NYM1-JBASE+1
                    L=1
                    DO COUNT2=JBASE,NYM1
                      AA(L)=A(COUNT1,COUNT2,COUNT3)
                      BB(L)=B(COUNT1,COUNT2,COUNT3)
                      CC(L)=C(COUNT1,COUNT2,COUNT3)
                      RR(L)=R(COUNT1,COUNT2,COUNT3)
                      L=L+1
                    END DO
                    CALL TRIDI3D (AA,BB,CC,RR,N,X)
                    L=0
                    DO COUNT2=JBASE,NYM1
                      V(COUNT1,COUNT2,COUNT3)=X(L+1)
                      L=L+1
                    END DO
                  END DO
                END DO

!***  SECTION 8:  BELOW-GRADE (3)
                DO COUNT1=0,IBASE-1

!***  FLOOR SLAB
                    IF (TBAV.GT.U(COUNT1,0,KBASE)) THEN
                      HINZ=HIN(4)
                    ELSE
                      HINZ=HIN(5)
                    END IF
                    A(COUNT1,0,KBASE)=0.
                    B(COUNT1,0,KBASE)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,0,KBASE)*     &
                      &      CYP(COUNT1,0,KBASE)
                    C(COUNT1,0,KBASE)=(3.d0-2.d0*F)*CONST(COUNT1,0,KBASE)*        &
                      & (-CYP(COUNT1,0,KBASE))
                    R(COUNT1,0,KBASE)=F*CONST(COUNT1,0,KBASE)*                &
                    & (CXM(COUNT1,0,KBASE)*U(COUNT1-1,0,KBASE)-               &
                    & (CXM(COUNT1,0,KBASE)+CXP(COUNT1,0,KBASE))*              &
                    & U(COUNT1,0,KBASE)+CXP(COUNT1,0,KBASE)*                  &
                    & U(COUNT1+1,0,KBASE)-(HINZ/DZ(KBASE)+CZP(COUNT1,0,KBASE))* &
                    & U(COUNT1,0,KBASE)+CZP(COUNT1,0,KBASE)*                  &
                    & U(COUNT1,0,KBASE+1))+U(COUNT1,0,KBASE)+                 &
                    & F*CONST(COUNT1,0,KBASE)*HINZ*TBAV/DZ(KBASE)

                  DO COUNT2=1,JBASE-1
                      IF (TBAV.GT.U(COUNT1,COUNT2,KBASE)) THEN
                        HINZ=HIN(4)
                      ELSE
                        HINZ=HIN(5)
                      END IF
                      A(COUNT1,COUNT2,KBASE)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,KBASE)* &
                         & (-CYM(COUNT1,COUNT2,KBASE))
                      B(COUNT1,COUNT2,KBASE)=1.d0+(3.d0-2.d0*F)*                      &
                         & CONST(COUNT1,COUNT2,KBASE)*(CYM(COUNT1,COUNT2,KBASE)+ &
                         & CYP(COUNT1,COUNT2,KBASE))
                      C(COUNT1,COUNT2,KBASE)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,KBASE)* &
                         & (-CYP(COUNT1,COUNT2,KBASE))
                      R(COUNT1,COUNT2,KBASE)=F*CONST(COUNT1,COUNT2,KBASE)*      &
                      & (CXM(COUNT1,COUNT2,KBASE)*U(COUNT1-1,COUNT2,KBASE)-     &
                      & (CXM(COUNT1,COUNT2,KBASE)+CXP(COUNT1,COUNT2,KBASE))*    &
                      & U(COUNT1,COUNT2,KBASE)+CXP(COUNT1,COUNT2,KBASE)*        &
                      & U(COUNT1+1,COUNT2,KBASE)-(HINZ/DZ(KBASE)+               &
                      & CZP(COUNT1,COUNT2,KBASE))*U(COUNT1,COUNT2,KBASE)+       &
                      & CZP(COUNT1,COUNT2,KBASE)*U(COUNT1,COUNT2,KBASE+1))+     &
                      & U(COUNT1,COUNT2,KBASE)+F*CONST(COUNT1,COUNT2,KBASE)*    &
                      & HINZ*TBAV/DZ(KBASE)
                  END DO
!***  FOUNDATION WALL/GRAVEL/GROUND
                  DO COUNT2=JBASE+1,NYM1-1
                    A(COUNT1,COUNT2,KBASE)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,KBASE)* &
                         & (-CYM(COUNT1,COUNT2,KBASE))
                    B(COUNT1,COUNT2,KBASE)=1.d0+(3.d0-2.d0*F)*                        &
                         & CONST(COUNT1,COUNT2,KBASE)*(CYM(COUNT1,COUNT2,KBASE)+ &
                         & CYP(COUNT1,COUNT2,KBASE))
                    C(COUNT1,COUNT2,KBASE)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,KBASE)* &
                         & (-CYP(COUNT1,COUNT2,KBASE))
                    R(COUNT1,COUNT2,KBASE)=F*CONST(COUNT1,COUNT2,KBASE)*        &
                    & (CXM(COUNT1,COUNT2,KBASE)*U(COUNT1-1,COUNT2,KBASE)-       &
                    & (CXM(COUNT1,COUNT2,KBASE)+CXP(COUNT1,COUNT2,KBASE))*      &
                    & U(COUNT1,COUNT2,KBASE)+CXP(COUNT1,COUNT2,KBASE)*          &
                    & U(COUNT1+1,COUNT2,KBASE)+CZM(COUNT1,COUNT2,KBASE)*        &
                    & U(COUNT1,COUNT2,KBASE-1)-(CZM(COUNT1,COUNT2,KBASE)+       &
                    & CZP(COUNT1,COUNT2,KBASE))*U(COUNT1,COUNT2,KBASE)+         &
                    & CZP(COUNT1,COUNT2,KBASE)*U(COUNT1,COUNT2,KBASE+1))+       &
                    & U(COUNT1,COUNT2,KBASE)
                  END DO
                  A(COUNT1,NYM1,KBASE)=(3.d0-2.d0*F)*CONST(COUNT1,NYM1,KBASE)*      &
                    & (-CYM(COUNT1,NYM1,KBASE))
                  B(COUNT1,NYM1,KBASE)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,NYM1,KBASE)*   &
                    & (CYM(COUNT1,NYM1,KBASE)+CYP(COUNT1,NYM1,KBASE))
                  C(COUNT1,NYM1,KBASE)=0.
                  R(COUNT1,NYM1,KBASE)=F*CONST(COUNT1,NYM1,KBASE)*              &
                  & (CXM(COUNT1,NYM1,KBASE)*U(COUNT1-1,NYM1,KBASE)-             &
                  & (CXM(COUNT1,NYM1,KBASE)+CXP(COUNT1,NYM1,KBASE))*            &
                  & U(COUNT1,NYM1,KBASE)+CXP(COUNT1,NYM1,KBASE)*                &
                  & U(COUNT1+1,NYM1,KBASE)+CZM(COUNT1,NYM1,KBASE)*              &
                  & U(COUNT1,NYM1,KBASE-1)-(CZM(COUNT1,NYM1,KBASE)+             &
                  & CZP(COUNT1,NYM1,KBASE))*U(COUNT1,NYM1,KBASE)+               &
                  & CZP(COUNT1,NYM1,KBASE)*U(COUNT1,NYM1,KBASE+1))+             &
                  & U(COUNT1,NYM1,KBASE)+(3.d0-2.d0*F)*CONST(COUNT1,NYM1,KBASE)*    &
                  & CYP(COUNT1,NYM1,KBASE)*TGAV(KBASE)
                  N=NYM1-0+1
                  L=1
                  DO COUNT2=0,NYM1
                    AA(L)=A(COUNT1,COUNT2,KBASE)
                    BB(L)=B(COUNT1,COUNT2,KBASE)
                    CC(L)=C(COUNT1,COUNT2,KBASE)
                    RR(L)=R(COUNT1,COUNT2,KBASE)
                    L=L+1
                  END DO
                  CALL TRIDI3D (AA,BB,CC,RR,N,X)
                  L=0
                  DO COUNT2=0,NYM1
                    V(COUNT1,COUNT2,KBASE)=X(L+1)
                    L=L+1
                  END DO
                END DO
!***  BELOW FLOOR SLAB
                DO COUNT3=KBASE+1,NZBGM1
                  DO COUNT1=0,IBASE-1
                    A(COUNT1,0,COUNT3)=0.
                    B(COUNT1,0,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,0,COUNT3)*     &
                         & CYP(COUNT1,0,COUNT3)
                    C(COUNT1,0,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,0,COUNT3)*        &
                         & (-CYP(COUNT1,0,COUNT3))
                    R(COUNT1,0,COUNT3)=F*CONST(COUNT1,0,COUNT3)*                &
                    & (CXM(COUNT1,0,COUNT3)*U(COUNT1-1,0,COUNT3)-               &
                    & (CXM(COUNT1,0,COUNT3)+CXP(COUNT1,0,COUNT3))*              &
                    & U(COUNT1,0,COUNT3)+CXP(COUNT1,0,COUNT3)*                  &
                    & U(COUNT1+1,0,COUNT3)+CZM(COUNT1,0,COUNT3)*                &
                    & U(COUNT1,0,COUNT3-1)-(CZM(COUNT1,0,COUNT3)+               &
                    & CZP(COUNT1,0,COUNT3))*U(COUNT1,0,COUNT3)+                 &
                    & CZP(COUNT1,0,COUNT3)*U(COUNT1,0,COUNT3+1))+U(COUNT1,0,COUNT3)
                    DO COUNT2=1,NYM1-1
                      A(COUNT1,COUNT2,COUNT3)=(3.d0-2.d0*F)*                        &
                         & CONST(COUNT1,COUNT2,COUNT3)*(-CYM(COUNT1,COUNT2,COUNT3))
                      B(COUNT1,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*                     &
                         & CONST(COUNT1,COUNT2,COUNT3)*                         &
                         & (CYM(COUNT1,COUNT2,COUNT3)+CYP(COUNT1,COUNT2,COUNT3))
                      C(COUNT1,COUNT2,COUNT3)=(3.d0-2.d0*F)*                        &
                      & CONST(COUNT1,COUNT2,COUNT3)*(-CYP(COUNT1,COUNT2,COUNT3))
                      R(COUNT1,COUNT2,COUNT3)=F*CONST(COUNT1,COUNT2,COUNT3)*    &
                      & (CXM(COUNT1,COUNT2,COUNT3)*U(COUNT1-1,COUNT2,COUNT3)-   &
                      & (CXM(COUNT1,COUNT2,COUNT3)+CXP(COUNT1,COUNT2,COUNT3))*  &
                      & U(COUNT1,COUNT2,COUNT3)+CXP(COUNT1,COUNT2,COUNT3)*      &
                      & U(COUNT1+1,COUNT2,COUNT3)+CZM(COUNT1,COUNT2,COUNT3)*    &
                      & U(COUNT1,COUNT2,COUNT3-1)-(CZM(COUNT1,COUNT2,COUNT3)+   &
                      & CZP(COUNT1,COUNT2,COUNT3))*U(COUNT1,COUNT2,COUNT3)+     &
                      & CZP(COUNT1,COUNT2,COUNT3)*U(COUNT1,COUNT2,COUNT3+1))+   &
                      & U(COUNT1,COUNT2,COUNT3)
                    END DO
                    A(COUNT1,NYM1,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,NYM1,COUNT3)*  &
                         & (-CYM(COUNT1,NYM1,COUNT3))
                    B(COUNT1,NYM1,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,NYM1,COUNT3)* &
                         & (CYM(COUNT1,NYM1,COUNT3)+CYP(COUNT1,NYM1,COUNT3))
                    C(COUNT1,NYM1,COUNT3)=0.
                    R(COUNT1,NYM1,COUNT3)=F*CONST(COUNT1,NYM1,COUNT3)*          &
                    & (CXM(COUNT1,NYM1,COUNT3)*U(COUNT1-1,NYM1,COUNT3)-         &
                    & (CXM(COUNT1,NYM1,COUNT3)+CXP(COUNT1,NYM1,COUNT3))*        &
                    & U(COUNT1,NYM1,COUNT3)+CXP(COUNT1,NYM1,COUNT3)*            &
                    & U(COUNT1+1,NYM1,COUNT3)+CZM(COUNT1,NYM1,COUNT3)*          &
                    & U(COUNT1,NYM1,COUNT3-1)-(CZM(COUNT1,NYM1,COUNT3)+         &
                    & CZP(COUNT1,NYM1,COUNT3))*U(COUNT1,NYM1,COUNT3)+           &
                    & CZP(COUNT1,NYM1,COUNT3)*U(COUNT1,NYM1,COUNT3+1))+         &
                    & U(COUNT1,NYM1,COUNT3)+(3.d0-2.d0*F)*CONST(COUNT1,NYM1,COUNT3)* &
                    & CYP(COUNT1,NYM1,COUNT3)*TGAV(COUNT3)
                    N=NYM1-0+1
                    L=1
                    DO COUNT2=0,NYM1
                      AA(L)=A(COUNT1,COUNT2,COUNT3)
                      BB(L)=B(COUNT1,COUNT2,COUNT3)
                      CC(L)=C(COUNT1,COUNT2,COUNT3)
                      RR(L)=R(COUNT1,COUNT2,COUNT3)
                      L=L+1
                    END DO
                    CALL TRIDI3D (AA,BB,CC,RR,N,X)
                    L=0
                    DO COUNT2=0,NYM1
                      V(COUNT1,COUNT2,COUNT3)=X(L+1)
                      L=L+1
                    END DO
                  END DO
                END DO

!*********************************************************************!
!***  THIRD FRACTION OF TIME INCREMENT (Z-DIRECTION IMPLICIT,      ***!
!***  X AND Y-DIRECTIONS EXPLICIT)                                 ***!
!*********************************************************************!

!***  SET CONSTANTS
                DO COUNT1=0,NXM1
                  DO COUNT2=0,NYM1
                    DO COUNT3=-NZAG,NZBGM1
                      IF (COUNT1.EQ.0) V(COUNT1-1,COUNT2,COUNT3)=V(COUNT1,COUNT2,COUNT3)
                      IF (COUNT1.EQ.NXM1.AND.COUNT3.GE.0) V(COUNT1+1,COUNT2,COUNT3)=TGAV(COUNT3)
                      IF (COUNT2.EQ.0) V(COUNT1,COUNT2-1,COUNT3)=V(COUNT1,COUNT2,COUNT3)
                      IF (COUNT2.EQ.NYM1.AND.COUNT3.GE.0) V(COUNT1,COUNT2+1,COUNT3)=TGAV(COUNT3)
                    END DO
                  END DO
                END DO

!***  SET UP COEFFICIENT MATRIX IN Z-DIRECTION:
!***  SECTION 1:  CEILING CELLS
                DO COUNT1=0,IBASE+1
                  DO COUNT2=0,JBASE+1
                    IF (TIAV.GT.V(COUNT1,COUNT2,-NZAG)) THEN
                      HINZH=HIN(4)
                    ELSE
                      HINZH=HIN(5)
                    END IF
                    IF (TBAV.GE.V(COUNT1,COUNT2,-NZAG)) THEN
                      HINZ=HIN(5)
                    ELSE
                      HINZ=HIN(4)
                    END IF
                    A(COUNT1,COUNT2,-NZAG)=0.
                    B(COUNT1,COUNT2,-NZAG)=1.d0+(3.d0-2.d0*F)*                      &
                       & CONST(COUNT1,COUNT2,-NZAG)*(1.d0/DZ(-NZAG)/            &
                       & (1.d0/HINZH+DZ(-NZAG)/2.d0/TCON                          &
                       & (MTYPE(COUNT1,COUNT2,-NZAG)))+1.d0/DZ(-NZAG)/          &
                       & (RCEIL+1.d0/HINZ+DZ(-NZAG)/2.d0/TCON(MTYPE(COUNT1,COUNT2,-NZAG))))
                    C(COUNT1,COUNT2,-NZAG)=0.
                    R(COUNT1,COUNT2,-NZAG)=F*CONST(COUNT1,COUNT2,-NZAG)*      &
                    & (CXM(COUNT1,COUNT2,-NZAG)*V(COUNT1-1,COUNT2,-NZAG)-     &
                    & (CXM(COUNT1,COUNT2,-NZAG)+CXP(COUNT1,COUNT2,-NZAG))*    &
                    & V(COUNT1,COUNT2,-NZAG)+CXP(COUNT1,COUNT2,-NZAG)*        &
                    & V(COUNT1+1,COUNT2,-NZAG)+CYM(COUNT1,COUNT2,-NZAG)*      &
                    & V(COUNT1,COUNT2-1,-NZAG)-(CYM(COUNT1,COUNT2,-NZAG)+     &
                    & CYP(COUNT1,COUNT2,-NZAG))*V(COUNT1,COUNT2,-NZAG)+       &
                    & CYP(COUNT1,COUNT2,-NZAG)*V(COUNT1,COUNT2+1,-NZAG))+     &
                    & V(COUNT1,COUNT2,-NZAG)+(3.d0-2.d0*F)*                       &
                    & CONST(COUNT1,COUNT2,-NZAG)*(TIAV/DZ(-NZAG)/(1.d0/HINZH+   &
                    & DZ(-NZAG)/2.d0/TCON(MTYPE(COUNT1,COUNT2,-NZAG)))+TBAV/    &
                    & DZ(-NZAG)/(RCEIL+1.d0/HINZ+DZ(-NZAG)/2.d0/                  &
                    & TCON(MTYPE(COUNT1,COUNT2,-NZAG))))
                    N=1
                    L=1
                    AA(L)=A(COUNT1,COUNT2,-NZAG)
                    BB(L)=B(COUNT1,COUNT2,-NZAG)
                    CC(L)=C(COUNT1,COUNT2,-NZAG)
                    RR(L)=R(COUNT1,COUNT2,-NZAG)
                    CALL TRIDI3D (AA,BB,CC,RR,N,X)
                    L=0
                    T(COUNT1,COUNT2,-NZAG)=X(L+1)
                  END DO
                END DO

!***  SECTION 2:  BELOW SLAB
                DO COUNT1=0,IBASE-1
                  DO COUNT2=0,JBASE-1
                    IF (TBAV.GT.V(COUNT1,COUNT2,KBASE)) THEN
                      HINZ=HIN(4)
                    ELSE
                      HINZ=HIN(5)
                    END IF
                    A(COUNT1,COUNT2,KBASE)=0.
                    B(COUNT1,COUNT2,KBASE)=1.d0+(3.d0-2.d0*F)*                     &
                     & CONST(COUNT1,COUNT2,KBASE)*(HINZ/DZ(KBASE)+           &
                     & CZP(COUNT1,COUNT2,KBASE))
                    C(COUNT1,COUNT2,KBASE)=(3.d0-2.d0*F)*                        &
                     & CONST(COUNT1,COUNT2,KBASE)*(-CZP(COUNT1,COUNT2,KBASE))
                    R(COUNT1,COUNT2,KBASE)=F*CONST(COUNT1,COUNT2,KBASE)*     &
                    & (CXM(COUNT1,COUNT2,KBASE)*V(COUNT1-1,COUNT2,KBASE)-    &
                    & (CXM(COUNT1,COUNT2,KBASE)+CXP(COUNT1,COUNT2,KBASE))*   &
                    & V(COUNT1,COUNT2,KBASE)+CXP(COUNT1,COUNT2,KBASE)*       &
                    & V(COUNT1+1,COUNT2,KBASE)+CYM(COUNT1,COUNT2,KBASE)*     &
                    & V(COUNT1,COUNT2-1,KBASE)-(CYM(COUNT1,COUNT2,KBASE)+    &
                    & CYP(COUNT1,COUNT2,KBASE))*V(COUNT1,COUNT2,KBASE)+      &
                    & CYP(COUNT1,COUNT2,KBASE)*V(COUNT1,COUNT2+1,KBASE))+    &
                    & V(COUNT1,COUNT2,KBASE)+(3.d0-2.d0*F)*                      &
                    & CONST(COUNT1,COUNT2,KBASE)*HINZ*TBAV/DZ(KBASE)
                    DO COUNT3=KBASE+1,NZBGM1-1
                      A(COUNT1,COUNT2,COUNT3)=(3.d0-2.d0*F)*                       &
                        & CONST(COUNT1,COUNT2,COUNT3)*(-CZM(COUNT1,COUNT2,COUNT3))
                      B(COUNT1,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*                    &
                        & CONST(COUNT1,COUNT2,COUNT3)*(CZM(COUNT1,COUNT2,COUNT3)+ &
                        & CZP(COUNT1,COUNT2,COUNT3))
                      C(COUNT1,COUNT2,COUNT3)=(3.d0-2.d0*F)*                       &
                        & CONST(COUNT1,COUNT2,COUNT3)*(-CZP(COUNT1,COUNT2,COUNT3))
                      R(COUNT1,COUNT2,COUNT3)=F*CONST(COUNT1,COUNT2,COUNT3)*   &
                      & (CXM(COUNT1,COUNT2,COUNT3)*V(COUNT1-1,COUNT2,COUNT3)-  &
                      & (CXM(COUNT1,COUNT2,COUNT3)+CXP(COUNT1,COUNT2,COUNT3))* &
                      & V(COUNT1,COUNT2,COUNT3)+CXP(COUNT1,COUNT2,COUNT3)*     &
                      & V(COUNT1+1,COUNT2,COUNT3)+CYM(COUNT1,COUNT2,COUNT3)*   &
                      & V(COUNT1,COUNT2-1,COUNT3)-(CYM(COUNT1,COUNT2,COUNT3)+  &
                      & CYP(COUNT1,COUNT2,COUNT3))*V(COUNT1,COUNT2,COUNT3)+    &
                      & CYP(COUNT1,COUNT2,COUNT3)*V(COUNT1,COUNT2+1,COUNT3))+  &
                      & V(COUNT1,COUNT2,COUNT3)
                    END DO
                    IF (.not. SameString(FIXBC,'FALSE')) THEN
                      A(COUNT1,COUNT2,NZBGM1)=(3.d0-2.d0*F)*                       &
                        & CONST(COUNT1,COUNT2,NZBGM1)*(-CZM(COUNT1,COUNT2,NZBGM1))
                      B(COUNT1,COUNT2,NZBGM1)=1.d0+(3.d0-2.d0*F)*                    &
                        & CONST(COUNT1,COUNT2,NZBGM1)*(CZM(COUNT1,COUNT2,NZBGM1)+ &
                        & CZP(COUNT1,COUNT2,NZBGM1))
                      C(COUNT1,COUNT2,NZBGM1)=0.
                      R(COUNT1,COUNT2,NZBGM1)=F*CONST(COUNT1,COUNT2,NZBGM1)*   &
                      & (CXM(COUNT1,COUNT2,NZBGM1)*V(COUNT1-1,COUNT2,NZBGM1)-  &
                      & (CXM(COUNT1,COUNT2,NZBGM1)+CXP(COUNT1,COUNT2,NZBGM1))* &
                      & V(COUNT1,COUNT2,NZBGM1)+CXP(COUNT1,COUNT2,NZBGM1)*     &
                      & V(COUNT1+1,COUNT2,NZBGM1)+CYM(COUNT1,COUNT2,NZBGM1)*   &
                      & V(COUNT1,COUNT2-1,NZBGM1)-(CYM(COUNT1,COUNT2,NZBGM1)+  &
                      & CYP(COUNT1,COUNT2,NZBGM1))*V(COUNT1,COUNT2,NZBGM1)+    &
                      & CYP(COUNT1,COUNT2,NZBGM1)*V(COUNT1,COUNT2+1,NZBGM1))+  &
                      & V(COUNT1,COUNT2,NZBGM1)+(3.d0-2.d0*F)*                     &
                      & CONST(COUNT1,COUNT2,NZBGM1)*CZP(COUNT1,COUNT2,NZBGM1)*TDEEP
                    ELSE
                      A(COUNT1,COUNT2,NZBGM1)=(3.d0-2.d0*F)*                       &
                        & CONST(COUNT1,COUNT2,NZBGM1)*(-CZM(COUNT1,COUNT2,NZBGM1))
                      B(COUNT1,COUNT2,NZBGM1)=1.d0+(3.d0-2.d0*F)*                    &
                        & CONST(COUNT1,COUNT2,NZBGM1)*CZM(COUNT1,COUNT2,NZBGM1)
                      C(COUNT1,COUNT2,NZBGM1)=0.
                      R(COUNT1,COUNT2,NZBGM1)=F*CONST(COUNT1,COUNT2,NZBGM1)*   &
                      & (CXM(COUNT1,COUNT2,NZBGM1)*V(COUNT1-1,COUNT2,NZBGM1)-  &
                      & (CXM(COUNT1,COUNT2,NZBGM1)+CXP(COUNT1,COUNT2,NZBGM1))*  &
                      & V(COUNT1,COUNT2,NZBGM1)+CXP(COUNT1,COUNT2,NZBGM1)*     &
                      & V(COUNT1+1,COUNT2,NZBGM1)+CYM(COUNT1,COUNT2,NZBGM1)*   &
                      & V(COUNT1,COUNT2-1,NZBGM1)-(CYM(COUNT1,COUNT2,NZBGM1)+  &
                      & CYP(COUNT1,COUNT2,NZBGM1))*V(COUNT1,COUNT2,NZBGM1)+    &
                      & CYP(COUNT1,COUNT2,NZBGM1)*V(COUNT1,COUNT2+1,NZBGM1))+  &
                      & V(COUNT1,COUNT2,NZBGM1)
                    END IF
                    N=NZBGM1-KBASE+1
                    L=1
                    DO COUNT3=KBASE,NZBGM1
                      AA(L)=A(COUNT1,COUNT2,COUNT3)
                      BB(L)=B(COUNT1,COUNT2,COUNT3)
                      CC(L)=C(COUNT1,COUNT2,COUNT3)
                      RR(L)=R(COUNT1,COUNT2,COUNT3)
                      L=L+1
                    END DO
                    CALL TRIDI3D (AA,BB,CC,RR,N,X)
                    L=0
                    DO COUNT3=KBASE,NZBGM1
                      T(COUNT1,COUNT2,COUNT3)=X(L+1)
                      L=L+1
                    END DO
                  END DO
                END DO

!***  SECTION 3:  FOUNDATION WALL, PARALLEL TO X-AXIS
!***  TOP INSIDE WALL CELLS
                DO COUNT1=0,IBASE-1
                  IF (TBAV.GT.V(COUNT1,JBASE,-NZAG+2)) THEN
                    HINZ=HIN(4)
                  ELSE
                    HINZ=HIN(5)
                  END IF
                  A(COUNT1,JBASE,-NZAG+2)=0.
                  B(COUNT1,JBASE,-NZAG+2)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,-NZAG+2)* &
                       & (1.d0/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+DZ(-NZAG+2)/2.d0/            &
                       & TCON(MTYPE(COUNT1,JBASE,-NZAG+2)))+CZP(COUNT1,JBASE,-NZAG+2))
                  C(COUNT1,JBASE,-NZAG+2)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,-NZAG+2)*   &
                       & (-CZP(COUNT1,JBASE,-NZAG+2))
                  R(COUNT1,JBASE,-NZAG+2)=F*CONST(COUNT1,JBASE,-NZAG+2)*           &
                  & (CXM(COUNT1,JBASE,-NZAG+2)*V(COUNT1-1,JBASE,-NZAG+2)-          &
                  & (CXM(COUNT1,JBASE,-NZAG+2)+CXP(COUNT1,JBASE,-NZAG+2))*         &
                  & V(COUNT1,JBASE,-NZAG+2)+CXP(COUNT1,JBASE,-NZAG+2)*             &
                  & V(COUNT1+1,JBASE,-NZAG+2)-(1.d0/DY(JBASE)/(RINT+1.d0/HIN(6))+      &
                  & CYP(COUNT1,JBASE,-NZAG+2))*V(COUNT1,JBASE,-NZAG+2)+            &
                  & CYP(COUNT1,JBASE,-NZAG+2)*V(COUNT1,JBASE+1,-NZAG+2))+          &
                  & V(COUNT1,JBASE,-NZAG+2)+F*CONST(COUNT1,JBASE,-NZAG+2)*         &
                  & (TBAV/DY(JBASE)/(RINT+1.d0/HIN(6)))+(3.d0-2.d0*F)*                   &
                  & CONST(COUNT1,JBASE,-NZAG+2)*(TBAV/DZ(-NZAG+2)/                 &
                  & (1.d0/HINZ+RSILL+DZ(-NZAG+2)/2.d0/TCON(MTYPE(COUNT1,JBASE,-NZAG+2))))

!***  INSIDE WALL CELLS (UPPER BAND)
                  DO COUNT3=-NZAG+3,INT(1+(KBASE-NZAG)/2)
                    A(COUNT1,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)* &
                      & (-CZM(COUNT1,JBASE,COUNT3))
                    B(COUNT1,JBASE,COUNT3)=1.d0+(3.d0-2.d0*F)*                         &
                      & CONST(COUNT1,JBASE,COUNT3)*(CZM(COUNT1,JBASE,COUNT3)+     &
                      & CZP(COUNT1,JBASE,COUNT3))
                    C(COUNT1,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)* &
                      & (-CZP(COUNT1,JBASE,COUNT3))
                    R(COUNT1,JBASE,COUNT3)=F*CONST(COUNT1,JBASE,COUNT3)*         &
                    & (CXM(COUNT1,JBASE,COUNT3)*V(COUNT1-1,JBASE,COUNT3)-        &
                    & (CXM(COUNT1,JBASE,COUNT3)+CXP(COUNT1,JBASE,COUNT3))*       &
                    & V(COUNT1,JBASE,COUNT3)+CXP(COUNT1,JBASE,COUNT3)*           &
                    & V(COUNT1+1,JBASE,COUNT3)-(1.d0/DY(JBASE)/(RINT+1.d0/HIN(6))+   &
                    & CYP(COUNT1,JBASE,COUNT3))*V(COUNT1,JBASE,COUNT3)+          &
                    & CYP(COUNT1,JBASE,COUNT3)*V(COUNT1,JBASE+1,COUNT3))+        &
                    & V(COUNT1,JBASE,COUNT3)+F*CONST(COUNT1,JBASE,COUNT3)*       &
                    & (TBAV/DY(JBASE)/(RINT+1.d0/HIN(6)))
                  END DO

!***  INSIDE WALL CELLS (LOWER BAND)
                  DO COUNT3=INT(2+(KBASE-NZAG)/2),KBASE-1
                    A(COUNT1,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)* &
                       & (-CZM(COUNT1,JBASE,COUNT3))
                    B(COUNT1,JBASE,COUNT3)=1.d0+(3.d0-2.d0*F)*                         &
                       & CONST(COUNT1,JBASE,COUNT3)*(CZM(COUNT1,JBASE,COUNT3)+ &
                       & CZP(COUNT1,JBASE,COUNT3))
                    C(COUNT1,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)* &
                       & (-CZP(COUNT1,JBASE,COUNT3))
                    R(COUNT1,JBASE,COUNT3)=F*CONST(COUNT1,JBASE,COUNT3)*         &
                    & (CXM(COUNT1,JBASE,COUNT3)*V(COUNT1-1,JBASE,COUNT3)-        &
                    & (CXM(COUNT1,JBASE,COUNT3)+CXP(COUNT1,JBASE,COUNT3))*       &
                    & V(COUNT1,JBASE,COUNT3)+CXP(COUNT1,JBASE,COUNT3)*           &
                    & V(COUNT1+1,JBASE,COUNT3)-(1.d0/DY(JBASE)/(RINT+1.d0/HIN(6))+   &
                    & CYP(COUNT1,JBASE,COUNT3))*V(COUNT1,JBASE,COUNT3)+          &
                    & CYP(COUNT1,JBASE,COUNT3)*V(COUNT1,JBASE+1,COUNT3))+        &
                    & V(COUNT1,JBASE,COUNT3)+F*CONST(COUNT1,JBASE,COUNT3)*       &
                    & (TBAV/DY(JBASE)/(RINT+1.d0/HIN(6)))
                  END DO

!***  FOUNDATION WALL/GRAVEL/GROUND
                  DO COUNT3=KBASE,NZBGM1-1
                    A(COUNT1,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)*   &
                       & (-CZM(COUNT1,JBASE,COUNT3))
                    B(COUNT1,JBASE,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)* &
                       & (CZM(COUNT1,JBASE,COUNT3)+CZP(COUNT1,JBASE,COUNT3))
                    C(COUNT1,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)*   &
                       & (-CZP(COUNT1,JBASE,COUNT3))
                    R(COUNT1,JBASE,COUNT3)=F*CONST(COUNT1,JBASE,COUNT3)*           &
                    & (CXM(COUNT1,JBASE,COUNT3)*V(COUNT1-1,JBASE,COUNT3)-          &
                    & (CXM(COUNT1,JBASE,COUNT3)+CXP(COUNT1,JBASE,COUNT3))*         &
                    & V(COUNT1,JBASE,COUNT3)+CXP(COUNT1,JBASE,COUNT3)*             &
                    & V(COUNT1+1,JBASE,COUNT3)+CYM(COUNT1,JBASE,COUNT3)*           &
                    & V(COUNT1,JBASE-1,COUNT3)-(CYM(COUNT1,JBASE,COUNT3)+          &
                    & CYP(COUNT1,JBASE,COUNT3))*V(COUNT1,JBASE,COUNT3)+            &
                    & CYP(COUNT1,JBASE,COUNT3)*V(COUNT1,JBASE+1,COUNT3))+          &
                    & V(COUNT1,JBASE,COUNT3)
                  END DO
                  IF (.not. SameString(FIXBC,'FALSE')) THEN
                    A(COUNT1,JBASE,NZBGM1)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,NZBGM1)*   &
                       & (-CZM(COUNT1,JBASE,NZBGM1))
                    B(COUNT1,JBASE,NZBGM1)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,NZBGM1)* &
                       & (CZM(COUNT1,JBASE,NZBGM1)+CZP(COUNT1,JBASE,NZBGM1))
                    C(COUNT1,JBASE,NZBGM1)=0.
                    R(COUNT1,JBASE,NZBGM1)=F*CONST(COUNT1,JBASE,NZBGM1)*           &
                    & (CXM(COUNT1,JBASE,NZBGM1)*V(COUNT1-1,JBASE,NZBGM1)-          &
                    & (CXM(COUNT1,JBASE,NZBGM1)+CXP(COUNT1,JBASE,NZBGM1))*         &
                    & V(COUNT1,JBASE,NZBGM1)+CXP(COUNT1,JBASE,NZBGM1)*             &
                    & V(COUNT1+1,JBASE,NZBGM1)+CYM(COUNT1,JBASE,NZBGM1)*           &
                    & V(COUNT1,JBASE-1,NZBGM1)-(CYM(COUNT1,JBASE,NZBGM1)+          &
                    & CYP(COUNT1,JBASE,NZBGM1))*V(COUNT1,JBASE,NZBGM1)+            &
                    & CYP(COUNT1,JBASE,NZBGM1)*V(COUNT1,JBASE+1,NZBGM1))+          &
                    & V(COUNT1,JBASE,NZBGM1)+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,NZBGM1)* &
                    & CZP(COUNT1,JBASE,NZBGM1)*TDEEP
                  ELSE
                    A(COUNT1,JBASE,NZBGM1)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,NZBGM1)*   &
                       & (-CZM(COUNT1,JBASE,NZBGM1))
                    B(COUNT1,JBASE,NZBGM1)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,NZBGM1)* &
                       & CZM(COUNT1,JBASE,NZBGM1)
                    C(COUNT1,JBASE,NZBGM1)=0.
                    R(COUNT1,JBASE,NZBGM1)=F*CONST(COUNT1,JBASE,NZBGM1)*           &
                    & (CXM(COUNT1,JBASE,NZBGM1)*V(COUNT1-1,JBASE,NZBGM1)-          &
                    & (CXM(COUNT1,JBASE,NZBGM1)+CXP(COUNT1,JBASE,NZBGM1))*         &
                    & V(COUNT1,JBASE,NZBGM1)+CXP(COUNT1,JBASE,NZBGM1)*             &
                    & V(COUNT1+1,JBASE,NZBGM1)+CYM(COUNT1,JBASE,NZBGM1)*           &
                    & V(COUNT1,JBASE-1,NZBGM1)-(CYM(COUNT1,JBASE,NZBGM1)+          &
                    & CYP(COUNT1,JBASE,NZBGM1))*V(COUNT1,JBASE,NZBGM1)+            &
                    & CYP(COUNT1,JBASE,NZBGM1)*V(COUNT1,JBASE+1,NZBGM1))+          &
                    & V(COUNT1,JBASE,NZBGM1)
                  END IF
                  N=NZBGM1-(-NZAG+2)+1
                  L=1
                  DO COUNT3=-NZAG+2,NZBGM1
                    AA(L)=A(COUNT1,JBASE,COUNT3)
                    BB(L)=B(COUNT1,JBASE,COUNT3)
                    CC(L)=C(COUNT1,JBASE,COUNT3)
                    RR(L)=R(COUNT1,JBASE,COUNT3)
                    L=L+1
                  END DO
                  CALL TRIDI3D (AA,BB,CC,RR,N,X)
                  L=0
                  DO COUNT3=-NZAG+2,NZBGM1
                    T(COUNT1,JBASE,COUNT3)=X(L+1)
                    L=L+1
                  END DO
                END DO

!***  INSIDE CORNER WALL CELL AND CENTER WALL CELL
                DO COUNT1=IBASE,IBASE+1
                  IF (TBAV.GT.V(COUNT1,JBASE,-NZAG+2)) THEN
                    HINZ=HIN(4)
                  ELSE
                    HINZ=HIN(5)
                  END IF
                  A(COUNT1,JBASE,-NZAG+2)=0.
                  B(COUNT1,JBASE,-NZAG+2)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,-NZAG+2)* &
                       & (1.d0/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+DZ(-NZAG+2)/2.d0/            &
                       & TCON(MTYPE(COUNT1,JBASE,-NZAG+2)))+CZP(COUNT1,JBASE,-NZAG+2))
                  C(COUNT1,JBASE,-NZAG+2)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,-NZAG+2)*   &
                       & (-CZP(COUNT1,JBASE,-NZAG+2))
                  R(COUNT1,JBASE,-NZAG+2)=F*CONST(COUNT1,JBASE,-NZAG+2)*           &
                  & (CXM(COUNT1,JBASE,-NZAG+2)*V(COUNT1-1,JBASE,-NZAG+2)-          &
                  & (CXM(COUNT1,JBASE,-NZAG+2)+CXP(COUNT1,JBASE,-NZAG+2))*         &
                  & V(COUNT1,JBASE,-NZAG+2)+CXP(COUNT1,JBASE,-NZAG+2)*             &
                  & V(COUNT1+1,JBASE,-NZAG+2)+CYM(COUNT1,JBASE,-NZAG+2)*           &
                  & V(COUNT1,JBASE-1,-NZAG+2)-(CYM(COUNT1,JBASE,-NZAG+2)+          &
                  & CYP(COUNT1,JBASE,-NZAG+2))*V(COUNT1,JBASE,-NZAG+2)+            &
                  & CYP(COUNT1,JBASE,-NZAG+2)*V(COUNT1,JBASE+1,-NZAG+2))+          &
                  & V(COUNT1,JBASE,-NZAG+2)+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,-NZAG+2)* &
                  & (TBAV/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+DZ(-NZAG+2)/2.d0/               &
                  & TCON(MTYPE(COUNT1,JBASE,-NZAG+2))))

!***  FOUNDATION WALL/GRAVEL/GROUND
                  DO COUNT3=-NZAG+3,NZBGM1-1
                    A(COUNT1,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)*   &
                       & (-CZM(COUNT1,JBASE,COUNT3))
                    B(COUNT1,JBASE,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)* &
                       & (CZM(COUNT1,JBASE,COUNT3)+CZP(COUNT1,JBASE,COUNT3))
                    C(COUNT1,JBASE,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,COUNT3)*   &
                       & (-CZP(COUNT1,JBASE,COUNT3))
                    R(COUNT1,JBASE,COUNT3)=F*CONST(COUNT1,JBASE,COUNT3)*           &
                    & (CXM(COUNT1,JBASE,COUNT3)*V(COUNT1-1,JBASE,COUNT3)-          &
                    & (CXM(COUNT1,JBASE,COUNT3)+CXP(COUNT1,JBASE,COUNT3))*         &
                    & V(COUNT1,JBASE,COUNT3)+CXP(COUNT1,JBASE,COUNT3)*             &
                    & V(COUNT1+1,JBASE,COUNT3)+CYM(COUNT1,JBASE,COUNT3)*           &
                    & V(COUNT1,JBASE-1,COUNT3)-(CYM(COUNT1,JBASE,COUNT3)+          &
                    & CYP(COUNT1,JBASE,COUNT3))*V(COUNT1,JBASE,COUNT3)+            &
                    & CYP(COUNT1,JBASE,COUNT3)*V(COUNT1,JBASE+1,COUNT3))+          &
                    & V(COUNT1,JBASE,COUNT3)
                  END DO
                  IF (.not. SameString(FIXBC,'FALSE')) THEN
                    A(COUNT1,JBASE,NZBGM1)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,NZBGM1)*   &
                       & (-CZM(COUNT1,JBASE,NZBGM1))
                    B(COUNT1,JBASE,NZBGM1)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,NZBGM1)* &
                       & (CZM(COUNT1,JBASE,NZBGM1)+CZP(COUNT1,JBASE,NZBGM1))
                    C(COUNT1,JBASE,NZBGM1)=0.
                    R(COUNT1,JBASE,NZBGM1)=F*CONST(COUNT1,JBASE,NZBGM1)*           &
                    & (CXM(COUNT1,JBASE,NZBGM1)*V(COUNT1-1,JBASE,NZBGM1)-          &
                    & (CXM(COUNT1,JBASE,NZBGM1)+CXP(COUNT1,JBASE,NZBGM1))*         &
                    & V(COUNT1,JBASE,NZBGM1)+CXP(COUNT1,JBASE,NZBGM1)*             &
                    & V(COUNT1+1,JBASE,NZBGM1)+CYM(COUNT1,JBASE,NZBGM1)*           &
                    & V(COUNT1,JBASE-1,NZBGM1)-(CYM(COUNT1,JBASE,NZBGM1)+          &
                    & CYP(COUNT1,JBASE,NZBGM1))*V(COUNT1,JBASE,NZBGM1)+            &
                    & CYP(COUNT1,JBASE,NZBGM1)*V(COUNT1,JBASE+1,NZBGM1))+          &
                    & V(COUNT1,JBASE,NZBGM1)+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,NZBGM1)* &
                    & CZP(COUNT1,JBASE,NZBGM1)*TDEEP
                  ELSE
                    A(COUNT1,JBASE,NZBGM1)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE,NZBGM1)*   &
                       & (-CZM(COUNT1,JBASE,NZBGM1))
                    B(COUNT1,JBASE,NZBGM1)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE,NZBGM1)* &
                       & CZM(COUNT1,JBASE,NZBGM1)
                    C(COUNT1,JBASE,NZBGM1)=0.
                    R(COUNT1,JBASE,NZBGM1)=F*CONST(COUNT1,JBASE,NZBGM1)*           &
                    & (CXM(COUNT1,JBASE,NZBGM1)*V(COUNT1-1,JBASE,NZBGM1)-          &
                    & (CXM(COUNT1,JBASE,NZBGM1)+CXP(COUNT1,JBASE,NZBGM1))*         &
                    & V(COUNT1,JBASE,NZBGM1)+CXP(COUNT1,JBASE,NZBGM1)*             &
                    & V(COUNT1+1,JBASE,NZBGM1)+CYM(COUNT1,JBASE,NZBGM1)*           &
                    & V(COUNT1,JBASE-1,NZBGM1)-(CYM(COUNT1,JBASE,NZBGM1)+          &
                    & CYP(COUNT1,JBASE,NZBGM1))*V(COUNT1,JBASE,NZBGM1)+            &
                    & CYP(COUNT1,JBASE,NZBGM1)*V(COUNT1,JBASE+1,NZBGM1))+          &
                    & V(COUNT1,JBASE,NZBGM1)
                  END IF
                  N=NZBGM1-(-NZAG+2)+1
                  L=1
                  DO COUNT3=-NZAG+2,NZBGM1
                    AA(L)=A(COUNT1,JBASE,COUNT3)
                    BB(L)=B(COUNT1,JBASE,COUNT3)
                    CC(L)=C(COUNT1,JBASE,COUNT3)
                    RR(L)=R(COUNT1,JBASE,COUNT3)
                    L=L+1
                  END DO
                  CALL TRIDI3D (AA,BB,CC,RR,N,X)
                  L=0
                  DO COUNT3=-NZAG+2,NZBGM1
                    T(COUNT1,JBASE,COUNT3)=X(L+1)
                    L=L+1
                  END DO
                END DO

!***  TOP CENTER WALL CELLS
                DO COUNT1=0,IBASE+1
                  IF(TBAV.GT.V(COUNT1,JBASE+1,-NZAG+2)) THEN
                    HINZ=HIN(4)
                  ELSE
                    HINZ=HIN(5)
                  END IF
                  A(COUNT1,JBASE+1,-NZAG+2)=0.
                  B(COUNT1,JBASE+1,-NZAG+2)=1.d0+(3.d0-2.d0*F)*                          &
                       & CONST(COUNT1,JBASE+1,-NZAG+2)*(1.d0/DZ(-NZAG+2)/(1.d0/        &
                       & HINZ+RSILL+DZ(-NZAG+2)/2.d0/                                &
                       & TCON(MTYPE(COUNT1,JBASE+1,-NZAG+2)))+                     &
                       & CZP(COUNT1,JBASE+1,-NZAG+2))
                  C(COUNT1,JBASE+1,-NZAG+2)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+1,-NZAG+2)* &
                       & (-CZP(COUNT1,JBASE+1,-NZAG+2))
                  R(COUNT1,JBASE+1,-NZAG+2)=F*CONST(COUNT1,JBASE+1,-NZAG+2)*       &
                  & (CXM(COUNT1,JBASE+1,-NZAG+2)*V(COUNT1-1,JBASE+1,-NZAG+2)-      &
                  & (CXM(COUNT1,JBASE+1,-NZAG+2)+CXP(COUNT1,JBASE+1,-NZAG+2))*     &
                  & V(COUNT1,JBASE+1,-NZAG+2)+CXP(COUNT1,JBASE+1,-NZAG+2)*         &
                  & V(COUNT1+1,JBASE+1,-NZAG+2)+CYM(COUNT1,JBASE+1,-NZAG+2)*       &
                  & V(COUNT1,JBASE+1-1,-NZAG+2)-(CYM(COUNT1,JBASE+1,-NZAG+2)+      &
                  & CYP(COUNT1,JBASE+1,-NZAG+2))*V(COUNT1,JBASE+1,-NZAG+2)+        &
                  & CYP(COUNT1,JBASE+1,-NZAG+2)*V(COUNT1,JBASE+1+1,-NZAG+2))+      &
                  & V(COUNT1,JBASE+1,-NZAG+2)+(3.d0-2.d0*F)*CONST(COUNT1,JBASE+1,-NZAG+2)* &
                  & (TBAV/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+DZ(-NZAG+2)/2.d0/               &
                  & TCON(MTYPE(COUNT1,JBASE+1,-NZAG+2))))

!***  FOUNDATION WALL/GRAVEL/GROUND
                  DO COUNT3=-NZAG+3,NZBGM1-1
                    A(COUNT1,JBASE+1,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+1,COUNT3)* &
                       & (-CZM(COUNT1,JBASE+1,COUNT3))
                    B(COUNT1,JBASE+1,COUNT3)=1.d0+(3.d0-2.d0*F)*                         &
                       & CONST(COUNT1,JBASE+1,COUNT3)*(CZM(COUNT1,JBASE+1,COUNT3)+ &
                       & CZP(COUNT1,JBASE+1,COUNT3))
                    C(COUNT1,JBASE+1,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+1,COUNT3)* &
                       & (-CZP(COUNT1,JBASE+1,COUNT3))
                    R(COUNT1,JBASE+1,COUNT3)=F*CONST(COUNT1,JBASE+1,COUNT3)*       &
                    & (CXM(COUNT1,JBASE+1,COUNT3)*V(COUNT1-1,JBASE+1,COUNT3)-      &
                    & (CXM(COUNT1,JBASE+1,COUNT3)+CXP(COUNT1,JBASE+1,COUNT3))*     &
                    & V(COUNT1,JBASE+1,COUNT3)+CXP(COUNT1,JBASE+1,COUNT3)*         &
                    & V(COUNT1+1,JBASE+1,COUNT3)+CYM(COUNT1,JBASE+1,COUNT3)*       &
                    & V(COUNT1,JBASE+1-1,COUNT3)-(CYM(COUNT1,JBASE+1,COUNT3)+      &
                    & CYP(COUNT1,JBASE+1,COUNT3))*V(COUNT1,JBASE+1,COUNT3)+        &
                    & CYP(COUNT1,JBASE+1,COUNT3)*V(COUNT1,JBASE+1+1,COUNT3))+      &
                    & V(COUNT1,JBASE+1,COUNT3)
                  END DO
                  IF (.not. SameString(FIXBC,'FALSE')) THEN
                    A(COUNT1,JBASE+1,NZBGM1)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+1,NZBGM1)* &
                       & (-CZM(COUNT1,JBASE+1,NZBGM1))
                    B(COUNT1,JBASE+1,NZBGM1)=1.d0+(3.d0-2.d0*F)*                         &
                       & CONST(COUNT1,JBASE+1,NZBGM1)*(CZM(COUNT1,JBASE+1,NZBGM1)+ &
                       & CZP(COUNT1,JBASE+1,NZBGM1))
                    C(COUNT1,JBASE+1,NZBGM1)=0.
                    R(COUNT1,JBASE+1,NZBGM1)=F*CONST(COUNT1,JBASE+1,NZBGM1)*       &
                    & (CXM(COUNT1,JBASE+1,NZBGM1)*V(COUNT1-1,JBASE+1,NZBGM1)-      &
                    & (CXM(COUNT1,JBASE+1,NZBGM1)+CXP(COUNT1,JBASE+1,NZBGM1))*     &
                    & V(COUNT1,JBASE+1,NZBGM1)+CXP(COUNT1,JBASE+1,NZBGM1)*         &
                    & V(COUNT1+1,JBASE+1,NZBGM1)+CYM(COUNT1,JBASE+1,NZBGM1)*       &
                    & V(COUNT1,JBASE+1-1,NZBGM1)-(CYM(COUNT1,JBASE+1,NZBGM1)+      &
                    & CYP(COUNT1,JBASE+1,NZBGM1))*V(COUNT1,JBASE+1,NZBGM1)+        &
                    & CYP(COUNT1,JBASE+1,NZBGM1)*V(COUNT1,JBASE+1+1,NZBGM1))+      &
                    & V(COUNT1,JBASE+1,NZBGM1)+(3.d0-2.d0*F)*                          &
                    & CONST(COUNT1,JBASE+1,NZBGM1)*CZP(COUNT1,JBASE+1,NZBGM1)*TDEEP
                  ELSE
                    A(COUNT1,JBASE+1,NZBGM1)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+1,NZBGM1)* &
                       & (-CZM(COUNT1,JBASE+1,NZBGM1))
                    B(COUNT1,JBASE+1,NZBGM1)=1.d0+(3.d0-2.d0*F)*                         &
                       & CONST(COUNT1,JBASE+1,NZBGM1)*CZM(COUNT1,JBASE+1,NZBGM1)
                    C(COUNT1,JBASE+1,NZBGM1)=0.
                    R(COUNT1,JBASE+1,NZBGM1)=F*CONST(COUNT1,JBASE+1,NZBGM1)*       &
                    & (CXM(COUNT1,JBASE+1,NZBGM1)*V(COUNT1-1,JBASE+1,NZBGM1)-      &
                    & (CXM(COUNT1,JBASE+1,NZBGM1)+CXP(COUNT1,JBASE+1,NZBGM1))*     &
                    & V(COUNT1,JBASE+1,NZBGM1)+CXP(COUNT1,JBASE+1,NZBGM1)*         &
                    & V(COUNT1+1,JBASE+1,NZBGM1)+CYM(COUNT1,JBASE+1,NZBGM1)*       &
                    & V(COUNT1,JBASE+1-1,NZBGM1)-(CYM(COUNT1,JBASE+1,NZBGM1)+      &
                    & CYP(COUNT1,JBASE+1,NZBGM1))*V(COUNT1,JBASE+1,NZBGM1)+        &
                    & CYP(COUNT1,JBASE+1,NZBGM1)*V(COUNT1,JBASE+1+1,NZBGM1))+      &
                    & V(COUNT1,JBASE+1,NZBGM1)
                  END IF
                  N=NZBGM1-(-NZAG+2)+1
                  L=1
                  DO COUNT3=-NZAG+2,NZBGM1
                    AA(L)=A(COUNT1,JBASE+1,COUNT3)
                    BB(L)=B(COUNT1,JBASE+1,COUNT3)
                    CC(L)=C(COUNT1,JBASE+1,COUNT3)
                    RR(L)=R(COUNT1,JBASE+1,COUNT3)
                    L=L+1
                  END DO
                  CALL TRIDI3D (AA,BB,CC,RR,N,X)
                  L=0
                  DO COUNT3=-NZAG+2,NZBGM1
                    T(COUNT1,JBASE+1,COUNT3)=X(L+1)
                    L=L+1
                  END DO
                END DO

!***  SECTION 4:  FOUNDATION WALL, PARALLEL TO Y-AXIS
!***  TOP INSIDE WALL CELLS
                DO COUNT2=0,JBASE-1
                  IF (TBAV.GT.V(IBASE,COUNT2,-NZAG+2)) THEN
                    HINZ=HIN(4)
                  ELSE
                    HINZ=HIN(5)
                  END IF
                  A(IBASE,COUNT2,-NZAG+2)=0.
                  B(IBASE,COUNT2,-NZAG+2)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,COUNT2,-NZAG+2)* &
                       & (1.d0/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+DZ(-NZAG+2)/2.d0/            &
                       & TCON(MTYPE(IBASE,COUNT2,-NZAG+2)))+CZP(IBASE,COUNT2,-NZAG+2))
                  C(IBASE,COUNT2,-NZAG+2)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,-NZAG+2)*   &
                       & (-CZP(IBASE,COUNT2,-NZAG+2))
                  R(IBASE,COUNT2,-NZAG+2)=F*CONST(IBASE,COUNT2,-NZAG+2)*(-(1.d0/     &
                  & DX(IBASE)/(RINT+1.d0/HIN(6))+CXP(IBASE,COUNT2,-NZAG+2))*         &
                  & V(IBASE,COUNT2,-NZAG+2)+CXP(IBASE,COUNT2,-NZAG+2)*             &
                  & V(IBASE+1,COUNT2,-NZAG+2)+CYM(IBASE,COUNT2,-NZAG+2)*           &
                  & V(IBASE,COUNT2-1,-NZAG+2)-(CYM(IBASE,COUNT2,-NZAG+2)+          &
                  & CYP(IBASE,COUNT2,-NZAG+2))*V(IBASE,COUNT2,-NZAG+2)+            &
                  & CYP(IBASE,COUNT2,-NZAG+2)*V(IBASE,COUNT2+1,-NZAG+2))+          &
                  & V(IBASE,COUNT2,-NZAG+2)+F*CONST(IBASE,COUNT2,-NZAG+2)*         &
                  & (TBAV/DX(IBASE)/(RINT+1.d0/HIN(6)))+(3.d0-2.d0*F)*                   &
                  & CONST(IBASE,COUNT2,-NZAG+2)*(TBAV/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+  &
                  & DZ(-NZAG+2)/2.d0/TCON(MTYPE(IBASE,COUNT2,-NZAG+2))))

!***  INSIDE WALL CELLS (UPPER BAND)
                  DO COUNT3=-NZAG+3,INT(1+(KBASE-NZAG)/2)
                    A(IBASE,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)* &
                         & (-CZM(IBASE,COUNT2,COUNT3))
                    B(IBASE,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*                         &
                         & CONST(IBASE,COUNT2,COUNT3)*(CZM(IBASE,COUNT2,COUNT3)+ &
                         & CZP(IBASE,COUNT2,COUNT3))
                    C(IBASE,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)* &
                         & (-CZP(IBASE,COUNT2,COUNT3))
                    R(IBASE,COUNT2,COUNT3)=F*CONST(IBASE,COUNT2,COUNT3)*         &
                    & (-(1.d0/DX(IBASE)/(RINT+1.d0/HIN(6))+CXP(IBASE,COUNT2,COUNT3))* &
                    & V(IBASE,COUNT2,COUNT3)+CXP(IBASE,COUNT2,COUNT3)*           &
                    & V(IBASE+1,COUNT2,COUNT3)+CYM(IBASE,COUNT2,COUNT3)*         &
                    & V(IBASE,COUNT2-1,COUNT3)-(CYM(IBASE,COUNT2,COUNT3)+        &
                    & CYP(IBASE,COUNT2,COUNT3))*V(IBASE,COUNT2,COUNT3)+          &
                    & CYP(IBASE,COUNT2,COUNT3)*V(IBASE,COUNT2+1,COUNT3))+        &
                    & V(IBASE,COUNT2,COUNT3)+F*CONST(IBASE,COUNT2,COUNT3)*       &
                    & (TBAV/DX(IBASE)/(RINT+1.d0/HIN(6)))
                  END DO

!***  INSIDE WALL CELLS (LOWER BAND)
                  DO COUNT3=INT(2+(KBASE-NZAG)/2),KBASE-1
                    A(IBASE,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)* &
                       & (-CZM(IBASE,COUNT2,COUNT3))
                    B(IBASE,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)* &
                       & (CZM(IBASE,COUNT2,COUNT3)+CZP(IBASE,COUNT2,COUNT3))
                    C(IBASE,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)* &
                       & (-CZP(IBASE,COUNT2,COUNT3))
                    R(IBASE,COUNT2,COUNT3)=F*CONST(IBASE,COUNT2,COUNT3)*(-(1.d0/   &
                    & DX(IBASE)/(RINT+1.d0/HIN(6))+CXP(IBASE,COUNT2,COUNT3))*      &
                    & V(IBASE,COUNT2,COUNT3)+CXP(IBASE,COUNT2,COUNT3)*           &
                    & V(IBASE+1,COUNT2,COUNT3)+CYM(IBASE,COUNT2,COUNT3)*         &
                    & V(IBASE,COUNT2-1,COUNT3)-(CYM(IBASE,COUNT2,COUNT3)+        &
                    & CYP(IBASE,COUNT2,COUNT3))*V(IBASE,COUNT2,COUNT3)+          &
                    & CYP(IBASE,COUNT2,COUNT3)*V(IBASE,COUNT2+1,COUNT3))+        &
                    & V(IBASE,COUNT2,COUNT3)+F*CONST(IBASE,COUNT2,COUNT3)*       &
                    & (TBAV/DX(IBASE)/(RINT+1.d0/HIN(6)))
                  END DO

!***  FOUNDATION WALL/GRAVEL/GROUND
                  DO COUNT3=KBASE,NZBGM1-1
                    A(IBASE,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)*   &
                       & (-CZM(IBASE,COUNT2,COUNT3))
                    B(IBASE,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)* &
                       & (CZM(IBASE,COUNT2,COUNT3)+CZP(IBASE,COUNT2,COUNT3))
                    C(IBASE,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,COUNT3)*   &
                       & (-CZP(IBASE,COUNT2,COUNT3))
                    R(IBASE,COUNT2,COUNT3)=F*CONST(IBASE,COUNT2,COUNT3)*           &
                    & (CXM(IBASE,COUNT2,COUNT3)*V(IBASE-1,COUNT2,COUNT3)-          &
                    & (CXM(IBASE,COUNT2,COUNT3)+CXP(IBASE,COUNT2,COUNT3))*         &
                    & V(IBASE,COUNT2,COUNT3)+CXP(IBASE,COUNT2,COUNT3)*             &
                    & V(IBASE+1,COUNT2,COUNT3)+CYM(IBASE,COUNT2,COUNT3)*           &
                    & V(IBASE,COUNT2-1,COUNT3)-(CYM(IBASE,COUNT2,COUNT3)+          &
                    & CYP(IBASE,COUNT2,COUNT3))*V(IBASE,COUNT2,COUNT3)+            &
                    & CYP(IBASE,COUNT2,COUNT3)*V(IBASE,COUNT2+1,COUNT3))+          &
                    & V(IBASE,COUNT2,COUNT3)
                  END DO
                  IF (.not. SameString(FIXBC,'FALSE')) THEN
                    A(IBASE,COUNT2,NZBGM1)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,NZBGM1)*   &
                       & (-CZM(IBASE,COUNT2,NZBGM1))
                    B(IBASE,COUNT2,NZBGM1)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,COUNT2,NZBGM1)* &
                       & (CZM(IBASE,COUNT2,NZBGM1)+CZP(IBASE,COUNT2,NZBGM1))
                    C(IBASE,COUNT2,NZBGM1)=0.
                    R(IBASE,COUNT2,NZBGM1)=F*CONST(IBASE,COUNT2,NZBGM1)*           &
                    & (CXM(IBASE,COUNT2,NZBGM1)*V(IBASE-1,COUNT2,NZBGM1)-          &
                    & (CXM(IBASE,COUNT2,NZBGM1)+CXP(IBASE,COUNT2,NZBGM1))*         &
                    & V(IBASE,COUNT2,NZBGM1)+CXP(IBASE,COUNT2,NZBGM1)*             &
                    & V(IBASE+1,COUNT2,NZBGM1)+CYM(IBASE,COUNT2,NZBGM1)*           &
                    & V(IBASE,COUNT2-1,NZBGM1)-(CYM(IBASE,COUNT2,NZBGM1)+          &
                    & CYP(IBASE,COUNT2,NZBGM1))*V(IBASE,COUNT2,NZBGM1)+            &
                    & CYP(IBASE,COUNT2,NZBGM1)*V(IBASE,COUNT2+1,NZBGM1))+          &
                    & V(IBASE,COUNT2,NZBGM1)+(3.d0-2.d0*F)*CONST(IBASE,COUNT2,NZBGM1)* &
                    & CZP(IBASE,COUNT2,NZBGM1)*TDEEP
                  ELSE
                    A(IBASE,COUNT2,NZBGM1)=(3.d0-2.d0*F)*CONST(IBASE,COUNT2,NZBGM1)*   &
                       & (-CZM(IBASE,COUNT2,NZBGM1))
                    B(IBASE,COUNT2,NZBGM1)=1.d0+(3.d0-2.d0*F)*CONST(IBASE,COUNT2,NZBGM1)* &
                       & CZM(IBASE,COUNT2,NZBGM1)
                    C(IBASE,COUNT2,NZBGM1)=0.
                    R(IBASE,COUNT2,NZBGM1)=F*CONST(IBASE,COUNT2,NZBGM1)*           &
                    & (CXM(IBASE,COUNT2,NZBGM1)*V(IBASE-1,COUNT2,NZBGM1)-          &
                    & (CXM(IBASE,COUNT2,NZBGM1)+CXP(IBASE,COUNT2,NZBGM1))*         &
                    & V(IBASE,COUNT2,NZBGM1)+CXP(IBASE,COUNT2,NZBGM1)*             &
                    & V(IBASE+1,COUNT2,NZBGM1)+CYM(IBASE,COUNT2,NZBGM1)*           &
                    & V(IBASE,COUNT2-1,NZBGM1)-(CYM(IBASE,COUNT2,NZBGM1)+          &
                    & CYP(IBASE,COUNT2,NZBGM1))*V(IBASE,COUNT2,NZBGM1)+            &
                    & CYP(IBASE,COUNT2,NZBGM1)*V(IBASE,COUNT2+1,NZBGM1))+          &
                    & V(IBASE,COUNT2,NZBGM1)
                  END IF
                  N=NZBGM1-(-NZAG+2)+1
                  L=1
                  DO COUNT3=-NZAG+2,NZBGM1
                    AA(L)=A(IBASE,COUNT2,COUNT3)
                    BB(L)=B(IBASE,COUNT2,COUNT3)
                    CC(L)=C(IBASE,COUNT2,COUNT3)
                    RR(L)=R(IBASE,COUNT2,COUNT3)
                    L=L+1
                  END DO
                  CALL TRIDI3D (AA,BB,CC,RR,N,X)
                  L=0
                  DO COUNT3=-NZAG+2,NZBGM1
                    T(IBASE,COUNT2,COUNT3)=X(L+1)
                  L=L+1
                  END DO
                END DO

!***  TOP CENTER WALL CELLS
                DO COUNT2=0,JBASE-1
                  IF (TBAV.GT.V(IBASE+1,COUNT2,-NZAG+2)) THEN
                    HINZ=HIN(4)
                  ELSE
                    HINZ=HIN(5)
                  END IF
                  A(IBASE+1,COUNT2,-NZAG+2)=0.
                  B(IBASE+1,COUNT2,-NZAG+2)=1.d0+(3.d0-2.d0*F)*                          &
                    & CONST(IBASE+1,COUNT2,-NZAG+2)*(1.d0/DZ(-NZAG+2)/(1.d0/HINZ+   &
                    & RSILL+DZ(-NZAG+2)/2.d0/TCON(MTYPE(IBASE+1,COUNT2,-NZAG+2)))+ &
                    & CZP(IBASE+1,COUNT2,-NZAG+2))
                  C(IBASE+1,COUNT2,-NZAG+2)=(3.d0-2.d0*F)*CONST(IBASE+1,COUNT2,-NZAG+2)* &
                    & (-CZP(IBASE+1,COUNT2,-NZAG+2))
                  R(IBASE+1,COUNT2,-NZAG+2)=F*CONST(IBASE+1,COUNT2,-NZAG+2)*       &
                  & (CXM(IBASE+1,COUNT2,-NZAG+2)*V(IBASE+1-1,COUNT2,-NZAG+2)-      &
                  & (CXM(IBASE+1,COUNT2,-NZAG+2)+CXP(IBASE+1,COUNT2,-NZAG+2))*     &
                  & V(IBASE+1,COUNT2,-NZAG+2)+CXP(IBASE+1,COUNT2,-NZAG+2)*         &
                  & V(IBASE+1+1,COUNT2,-NZAG+2)+CYM(IBASE+1,COUNT2,-NZAG+2)*       &
                  & V(IBASE+1,COUNT2-1,-NZAG+2)-(CYM(IBASE+1,COUNT2,-NZAG+2)+      &
                  & CYP(IBASE+1,COUNT2,-NZAG+2))*V(IBASE+1,COUNT2,-NZAG+2)+        &
                  & CYP(IBASE+1,COUNT2,-NZAG+2)*V(IBASE+1,COUNT2+1,-NZAG+2))+      &
                  & V(IBASE+1,COUNT2,-NZAG+2)+(3.d0-2.d0*F)*                           &
                  & CONST(IBASE+1,COUNT2,-NZAG+2)*(TBAV/DZ(-NZAG+2)/(1.d0/HINZ+RSILL+ &
                  & DZ(-NZAG+2)/2.d0/TCON(MTYPE(IBASE+1,COUNT2,-NZAG+2))))

!***  FOUNDATION WALL/GRAVEL/GROUND
                  DO COUNT3=-NZAG+3,NZBGM1-1
                    A(IBASE+1,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE+1,COUNT2,COUNT3)* &
                       & (-CZM(IBASE+1,COUNT2,COUNT3))
                    B(IBASE+1,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*                         &
                       & CONST(IBASE+1,COUNT2,COUNT3)*(CZM(IBASE+1,COUNT2,COUNT3)+ &
                       & CZP(IBASE+1,COUNT2,COUNT3))
                    C(IBASE+1,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE+1,COUNT2,COUNT3)* &
                       & (-CZP(IBASE+1,COUNT2,COUNT3))
                    R(IBASE+1,COUNT2,COUNT3)=F*CONST(IBASE+1,COUNT2,COUNT3)*       &
                    & (CXM(IBASE+1,COUNT2,COUNT3)*V(IBASE+1-1,COUNT2,COUNT3)-      &
                    & (CXM(IBASE+1,COUNT2,COUNT3)+CXP(IBASE+1,COUNT2,COUNT3))*     &
                    & V(IBASE+1,COUNT2,COUNT3)+CXP(IBASE+1,COUNT2,COUNT3)*         &
                    & V(IBASE+1+1,COUNT2,COUNT3)+CYM(IBASE+1,COUNT2,COUNT3)*       &
                    & V(IBASE+1,COUNT2-1,COUNT3)-(CYM(IBASE+1,COUNT2,COUNT3)+      &
                    & CYP(IBASE+1,COUNT2,COUNT3))*V(IBASE+1,COUNT2,COUNT3)+        &
                    & CYP(IBASE+1,COUNT2,COUNT3)*V(IBASE+1,COUNT2+1,COUNT3))+      &
                    & V(IBASE+1,COUNT2,COUNT3)
                  END DO
                  IF (.not. SameString(FIXBC,'FALSE')) THEN
                    A(IBASE+1,COUNT2,NZBGM1)=(3.d0-2.d0*F)*                            &
                       & CONST(IBASE+1,COUNT2,NZBGM1)*(-CZM(IBASE+1,COUNT2,NZBGM1))
                    B(IBASE+1,COUNT2,NZBGM1)=1.d0+(3.d0-2.d0*F)*                         &
                       & CONST(IBASE+1,COUNT2,NZBGM1)*(CZM(IBASE+1,COUNT2,NZBGM1)+ &
                       & CZP(IBASE+1,COUNT2,NZBGM1))
                    C(IBASE+1,COUNT2,NZBGM1)=0.
                    R(IBASE+1,COUNT2,NZBGM1)=F*CONST(IBASE+1,COUNT2,NZBGM1)*       &
                    & (CXM(IBASE+1,COUNT2,NZBGM1)*V(IBASE+1-1,COUNT2,NZBGM1)-      &
                    & (CXM(IBASE+1,COUNT2,NZBGM1)+CXP(IBASE+1,COUNT2,NZBGM1))*     &
                    & V(IBASE+1,COUNT2,NZBGM1)+CXP(IBASE+1,COUNT2,NZBGM1)*         &
                    & V(IBASE+1+1,COUNT2,NZBGM1)+CYM(IBASE+1,COUNT2,NZBGM1)*       &
                    & V(IBASE+1,COUNT2-1,NZBGM1)-(CYM(IBASE+1,COUNT2,NZBGM1)+      &
                    & CYP(IBASE+1,COUNT2,NZBGM1))*V(IBASE+1,COUNT2,NZBGM1)+        &
                    & CYP(IBASE+1,COUNT2,NZBGM1)*V(IBASE+1,COUNT2+1,NZBGM1))+      &
                    & V(IBASE+1,COUNT2,NZBGM1)+(3.d0-2.d0*F)*                          &
                    & CONST(IBASE+1,COUNT2,NZBGM1)*CZP(IBASE+1,COUNT2,NZBGM1)*TDEEP
                  ELSE
                    A(IBASE+1,COUNT2,NZBGM1)=(3.d0-2.d0*F)*CONST(IBASE+1,COUNT2,NZBGM1)* &
                       & (-CZM(IBASE+1,COUNT2,NZBGM1))
                    B(IBASE+1,COUNT2,NZBGM1)=1.d0+(3.d0-2.d0*F)*                         &
                       & CONST(IBASE+1,COUNT2,NZBGM1)*CZM(IBASE+1,COUNT2,NZBGM1)
                    C(IBASE+1,COUNT2,NZBGM1)=0.
                    R(IBASE+1,COUNT2,NZBGM1)=F*CONST(IBASE+1,COUNT2,NZBGM1)*       &
                    & (CXM(IBASE+1,COUNT2,NZBGM1)*V(IBASE+1-1,COUNT2,NZBGM1)-      &
                    & (CXM(IBASE+1,COUNT2,NZBGM1)+CXP(IBASE+1,COUNT2,NZBGM1))*     &
                    & V(IBASE+1,COUNT2,NZBGM1)+CXP(IBASE+1,COUNT2,NZBGM1)*         &
                    & V(IBASE+1+1,COUNT2,NZBGM1)+CYM(IBASE+1,COUNT2,NZBGM1)*       &
                    & V(IBASE+1,COUNT2-1,NZBGM1)-(CYM(IBASE+1,COUNT2,NZBGM1)+      &
                    & CYP(IBASE+1,COUNT2,NZBGM1))*V(IBASE+1,COUNT2,NZBGM1)+        &
                    & CYP(IBASE+1,COUNT2,NZBGM1)*V(IBASE+1,COUNT2+1,NZBGM1))+      &
                    & V(IBASE+1,COUNT2,NZBGM1)
                  END IF
                  N=NZBGM1-(-NZAG+2)+1
                  L=1
                  DO COUNT3=-NZAG+2,NZBGM1
                    AA(L)=A(IBASE+1,COUNT2,COUNT3)
                    BB(L)=B(IBASE+1,COUNT2,COUNT3)
                    CC(L)=C(IBASE+1,COUNT2,COUNT3)
                    RR(L)=R(IBASE+1,COUNT2,COUNT3)
                    L=L+1
                  END DO
                  CALL TRIDI3D (AA,BB,CC,RR,N,X)
                  L=0
                  DO COUNT3=-NZAG+2,NZBGM1
                    T(IBASE+1,COUNT2,COUNT3)=X(L+1)
                    L=L+1
                  END DO
                END DO

!***  SECTION 5:  RIM JOIST, PARALLEL TO X-AXIS
!***  PERIMETER CEILING CELL
                DO COUNT1=0,IBASE+1
                  A(COUNT1,JBASE+2,-NZAG)=0.
                  B(COUNT1,JBASE+2,-NZAG)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,-NZAG)* &
                      & (CZP(COUNT1,JBASE+2,-NZAG))
                  C(COUNT1,JBASE+2,-NZAG)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,-NZAG)*   &
                      & (-CZP(COUNT1,JBASE+2,-NZAG))
                  R(COUNT1,JBASE+2,-NZAG)=F*CONST(COUNT1,JBASE+2,-NZAG)*           &
                  & (CXM(COUNT1,JBASE+2,-NZAG)*V(COUNT1-1,JBASE+2,-NZAG)-          &
                  & (CXM(COUNT1,JBASE+2,-NZAG)+CXP(COUNT1,JBASE+2,-NZAG))*         &
                  & V(COUNT1,JBASE+2,-NZAG)+CXP(COUNT1,JBASE+2,-NZAG)*             &
                  & V(COUNT1+1,JBASE+2,-NZAG)+CYM(COUNT1,JBASE+2,-NZAG)*           &
                  & V(COUNT1,JBASE+2-1,-NZAG)-(CYM(COUNT1,JBASE+2,-NZAG)+1.d0/       &
                  & DY(JBASE+2)/(REXT+RSID+1.d0/HOAV))*V(COUNT1,JBASE+2,-NZAG))+     &
                  & V(COUNT1,JBASE+2,-NZAG)+F*CONST(COUNT1,JBASE+2,-NZAG)*(TDBAV/  &
                  & DY(JBASE+2)/(REXT+RSID+1.d0/HOAV)+RSOLVAV/DY(JBASE+2)-           &
                  & (0.88d0*SIGMA*(VEXT(COUNT1,JBASE+2,-NZAG)+273.15d0)**4)/DY(JBASE+2))

!***  RIM JOIST CELL
                  A(COUNT1,JBASE+2,-NZAG+1)=(3.d0-2.d0*F)*                           &
                     & CONST(COUNT1,JBASE+2,-NZAG+1)*(-CZM(COUNT1,JBASE+2,-NZAG+1))
                  B(COUNT1,JBASE+2,-NZAG+1)=1.d0+(3.d0-2.d0*F)*                        &
                     & CONST(COUNT1,JBASE+2,-NZAG+1)*(CZM(COUNT1,JBASE+2,-NZAG+1)+ &
                     & CZP(COUNT1,JBASE+2,-NZAG+1))
                  C(COUNT1,JBASE+2,-NZAG+1)=(3.d0-2.d0*F)*                           &
                     & CONST(COUNT1,JBASE+2,-NZAG+1)*(-CZP(COUNT1,JBASE+2,-NZAG+1))
                  R(COUNT1,JBASE+2,-NZAG+1)=F*CONST(COUNT1,JBASE+2,-NZAG+1)*     &
                  & (CXM(COUNT1,JBASE+2,-NZAG+1)*V(COUNT1-1,JBASE+2,-NZAG+1)-    &
                  & (CXM(COUNT1,JBASE+2,-NZAG+1)+CXP(COUNT1,JBASE+2,-NZAG+1))*   &
                  & V(COUNT1,JBASE+2,-NZAG+1)+CXP(COUNT1,JBASE+2,-NZAG+1)*       &
                  & V(COUNT1+1,JBASE+2,-NZAG+1)-(1.d0/DY(JBASE+2)/(RSILL+1.d0/HIN(6)+ &
                  & DY(JBASE+2)/TCON(MTYPE(COUNT1,JBASE+2,-NZAG+1)))+1.d0/DY(JBASE+2)/ &
                  & (REXT+RSID+1.d0/HOAV))*V(COUNT1,JBASE+2,-NZAG+1))+             &
                  & V(COUNT1,JBASE+2,-NZAG+1)+F*CONST(COUNT1,JBASE+2,-NZAG+1)*   &
                  &  (TBAV/DY(JBASE+2)/(RSILL+1.d0/HIN(6)+DY(JBASE+2)/             &
                  & TCON(MTYPE(COUNT1,JBASE+2,-NZAG+1)))+TDBAV/DY(JBASE+2)/      &
                  & (REXT+RSID+1.d0/HOAV)+RSOLVAV/DY(JBASE+2)-(0.88d0*SIGMA*         &
                  & (VEXT(COUNT1,JBASE+2,-NZAG+1)+273.15d0)**4)/DY(JBASE+2))

!***  TOP OUTSIDE WALL CELLS
                  A(COUNT1,JBASE+2,-NZAG+2)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,-NZAG+2)* &
                       & (-CZM(COUNT1,JBASE+2,-NZAG+2))
                  B(COUNT1,JBASE+2,-NZAG+2)=1.d0+(3.d0-2.d0*F)*                            &
                       & CONST(COUNT1,JBASE+2,-NZAG+2)*(CZM(COUNT1,JBASE+2,-NZAG+2)+ &
                       & CZP(COUNT1,JBASE+2,-NZAG+2))
                  C(COUNT1,JBASE+2,-NZAG+2)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,-NZAG+2)* &
                       & (-CZP(COUNT1,JBASE+2,-NZAG+2))
                  R(COUNT1,JBASE+2,-NZAG+2)=F*CONST(COUNT1,JBASE+2,-NZAG+2)*       &
                  & (CXM(COUNT1,JBASE+2,-NZAG+2)*V(COUNT1-1,JBASE+2,-NZAG+2)-      &
                  & (CXM(COUNT1,JBASE+2,-NZAG+2)+CXP(COUNT1,JBASE+2,-NZAG+2))*     &
                  & V(COUNT1,JBASE+2,-NZAG+2)+CXP(COUNT1,JBASE+2,-NZAG+2)*         &
                  & V(COUNT1+1,JBASE+2,-NZAG+2)+CYM(COUNT1,JBASE+2,-NZAG+2)*       &
                  & V(COUNT1,JBASE+2-1,-NZAG+2)-(CYM(COUNT1,JBASE+2,-NZAG+2)+      &
                  & 1.d0/DY(JBASE+2)/(REXT+RSID+1.d0/HOAV))*V(COUNT1,JBASE+2,-NZAG+2))+ &
                  & V(COUNT1,JBASE+2,-NZAG+2)+F*CONST(COUNT1,JBASE+2,-NZAG+2)*     &
                  & (TDBAV/DY(JBASE+2)/(REXT+RSID+1.d0/HOAV)+RSOLVAV/DY(JBASE+2)-    &
                  & (0.88d0*SIGMA*(VEXT(COUNT1,JBASE+2,-NZAG+2)+273.15d0)**4)/        &
                  & DY(JBASE+2))

!***  ABOVE-GRADE OUTSIDE WALL CELLS
                  IF (NZAG.GT.3) THEN
                    DO COUNT3=-NZAG+3,-1
                      A(COUNT1,JBASE+2,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,COUNT3)* &
                         & (-CZM(COUNT1,JBASE+2,COUNT3))
                      B(COUNT1,JBASE+2,COUNT3)=1.d0+(3.d0-2.d0*F)*                       &
                         & CONST(COUNT1,JBASE+2,COUNT3)*(CZM(COUNT1,JBASE+2,COUNT3)+ &
                          & CZP(COUNT1,JBASE+2,COUNT3))
                      C(COUNT1,JBASE+2,COUNT3)=(3.d0-2.d0*F)*                          &
                         & CONST(COUNT1,JBASE+2,COUNT3)*(-CZP(COUNT1,JBASE+2,COUNT3))
                      R(COUNT1,JBASE+2,COUNT3)=F*CONST(COUNT1,JBASE+2,COUNT3)*     &
                      & (CXM(COUNT1,JBASE+2,COUNT3)*V(COUNT1-1,JBASE+2,COUNT3)-    &
                      & (CXM(COUNT1,JBASE+2,COUNT3)+CXP(COUNT1,JBASE+2,COUNT3))*   &
                      & V(COUNT1,JBASE+2,COUNT3)+CXP(COUNT1,JBASE+2,COUNT3)*       &
                      & V(COUNT1+1,JBASE+2,COUNT3)+CYM(COUNT1,JBASE+2,COUNT3)*     &
                      & V(COUNT1,JBASE+2-1,COUNT3)-(CYM(COUNT1,JBASE+2,COUNT3)+    &
                      & 1.d0/DY(JBASE+2)/(REXT+1.d0/HOAV))*V(COUNT1,JBASE+2,COUNT3))+  &
                      & V(COUNT1,JBASE+2,COUNT3)+F*CONST(COUNT1,JBASE+2,COUNT3)*   &
                      & (TDBAV/DY(JBASE+2)/(REXT+1.d0/HOAV)+RSOLVAV/DY(JBASE+2)-     &
                      & (0.88d0*SIGMA*(VEXT(COUNT1,JBASE+2,COUNT3)+273.15d0)**4)/     &
                      & DY(JBASE+2))
                    END DO
                  END IF

!***  BELOW-GRADE OUTSIDE WALL CELLS/GRAVEL/GROUND
                  DO COUNT3=0,NZBGM1-1
                    A(COUNT1,JBASE+2,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,COUNT3)* &
                       & (-CZM(COUNT1,JBASE+2,COUNT3))
                    B(COUNT1,JBASE+2,COUNT3)=1.d0+(3.d0-2.d0*F)*                         &
                       & CONST(COUNT1,JBASE+2,COUNT3)*(CZM(COUNT1,JBASE+2,COUNT3)+ &
                       & CZP(COUNT1,JBASE+2,COUNT3))
                    C(COUNT1,JBASE+2,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,COUNT3)* &
                       & (-CZP(COUNT1,JBASE+2,COUNT3))
                    R(COUNT1,JBASE+2,COUNT3)=F*CONST(COUNT1,JBASE+2,COUNT3)*       &
                    & (CXM(COUNT1,JBASE+2,COUNT3)*V(COUNT1-1,JBASE+2,COUNT3)-      &
                    & (CXM(COUNT1,JBASE+2,COUNT3)+CXP(COUNT1,JBASE+2,COUNT3))*     &
                    & V(COUNT1,JBASE+2,COUNT3)+CXP(COUNT1,JBASE+2,COUNT3)*         &
                    & V(COUNT1+1,JBASE+2,COUNT3)+CYM(COUNT1,JBASE+2,COUNT3)*       &
                    & V(COUNT1,JBASE+2-1,COUNT3)-(CYM(COUNT1,JBASE+2,COUNT3)+      &
                    & CYP(COUNT1,JBASE+2,COUNT3))*V(COUNT1,JBASE+2,COUNT3)+        &
                    & CYP(COUNT1,JBASE+2,COUNT3)*V(COUNT1,JBASE+2+1,COUNT3))+      &
                    & V(COUNT1,JBASE+2,COUNT3)
                  END DO
                  IF (.not. SameString(FIXBC,'FALSE')) THEN
                    A(COUNT1,JBASE+2,NZBGM1)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,NZBGM1)* &
                       & (-CZM(COUNT1,JBASE+2,NZBGM1))
                    B(COUNT1,JBASE+2,NZBGM1)=1.d0+(3.d0-2.d0*F)*                         &
                       & CONST(COUNT1,JBASE+2,NZBGM1)*(CZM(COUNT1,JBASE+2,NZBGM1)+ &
                       & CZP(COUNT1,JBASE+2,NZBGM1))
                    C(COUNT1,JBASE+2,NZBGM1)=0.
                    R(COUNT1,JBASE+2,NZBGM1)=F*CONST(COUNT1,JBASE+2,NZBGM1)*       &
                    & (CXM(COUNT1,JBASE+2,NZBGM1)*V(COUNT1-1,JBASE+2,NZBGM1)-      &
                    & (CXM(COUNT1,JBASE+2,NZBGM1)+CXP(COUNT1,JBASE+2,NZBGM1))*     &
                    & V(COUNT1,JBASE+2,NZBGM1)+CXP(COUNT1,JBASE+2,NZBGM1)*         &
                    & V(COUNT1+1,JBASE+2,NZBGM1)+CYM(COUNT1,JBASE+2,NZBGM1)*       &
                    & V(COUNT1,JBASE+2-1,NZBGM1)-(CYM(COUNT1,JBASE+2,NZBGM1)+      &
                    & CYP(COUNT1,JBASE+2,NZBGM1))*V(COUNT1,JBASE+2,NZBGM1)+        &
                    & CYP(COUNT1,JBASE+2,NZBGM1)*V(COUNT1,JBASE+2+1,NZBGM1))+      &
                    & V(COUNT1,JBASE+2,NZBGM1)+(3.d0-2.d0*F)*                          &
                    & CONST(COUNT1,JBASE+2,NZBGM1)*CZP(COUNT1,JBASE+2,NZBGM1)*TDEEP
                  ELSE
                    A(COUNT1,JBASE+2,NZBGM1)=(3.d0-2.d0*F)*CONST(COUNT1,JBASE+2,NZBGM1)* &
                       & (-CZM(COUNT1,JBASE+2,NZBGM1))
                    B(COUNT1,JBASE+2,NZBGM1)=1.d0+(3.d0-2.d0*F)*                         &
                       & CONST(COUNT1,JBASE+2,NZBGM1)*CZM(COUNT1,JBASE+2,NZBGM1)
                    C(COUNT1,JBASE+2,NZBGM1)=0.
                    R(COUNT1,JBASE+2,NZBGM1)=F*CONST(COUNT1,JBASE+2,NZBGM1)*       &
                    & (CXM(COUNT1,JBASE+2,NZBGM1)*V(COUNT1-1,JBASE+2,NZBGM1)-      &
                    & (CXM(COUNT1,JBASE+2,NZBGM1)+CXP(COUNT1,JBASE+2,NZBGM1))*     &
                    & V(COUNT1,JBASE+2,NZBGM1)+CXP(COUNT1,JBASE+2,NZBGM1)*         &
                    & V(COUNT1+1,JBASE+2,NZBGM1)+CYM(COUNT1,JBASE+2,NZBGM1)*       &
                    & V(COUNT1,JBASE+2-1,NZBGM1)-(CYM(COUNT1,JBASE+2,NZBGM1)+      &
                    & CYP(COUNT1,JBASE+2,NZBGM1))*V(COUNT1,JBASE+2,NZBGM1)+        &
                    & CYP(COUNT1,JBASE+2,NZBGM1)*V(COUNT1,JBASE+2+1,NZBGM1))+      &
                    & V(COUNT1,JBASE+2,NZBGM1)
                  END IF
                  N=NZBGM1-(-NZAG)+1
                  L=1
                  DO COUNT3=-NZAG,NZBGM1
                    AA(L)=A(COUNT1,JBASE+2,COUNT3)
                    BB(L)=B(COUNT1,JBASE+2,COUNT3)
                    CC(L)=C(COUNT1,JBASE+2,COUNT3)
                    RR(L)=R(COUNT1,JBASE+2,COUNT3)
                    L=L+1
                  END DO
                  CALL TRIDI3D (AA,BB,CC,RR,N,X)
                  L=0
                  DO COUNT3=-NZAG,NZBGM1
                    T(COUNT1,JBASE+2,COUNT3)=X(L+1)
                    L=L+1
                  END DO
                  DO COUNT3=-NZAG,-1
                    QEXT(COUNT1,JBASE+2,COUNT3)=(TDBAV-T(COUNT1,JBASE+2,COUNT3))/  &
                         & (REXT+RSID+1.d0/HOAV)
                    TEXT(COUNT1,JBASE+2,COUNT3)=QEXT(COUNT1,JBASE+2,COUNT3)*       &
                         & (REXT+RSID)+T(COUNT1,JBASE+2,COUNT3)
                  END DO
                END DO

!***  TOP OUTER CORNER CELL
                A(IBASE+2,JBASE+2,-NZAG)=0.
                B(IBASE+2,JBASE+2,-NZAG)=1.d0+(3.d0-2.d0*F)*CONST(IBASE+2,JBASE+2,-NZAG)* &
                  & (CZP(IBASE+2,JBASE+2,-NZAG))
                C(IBASE+2,JBASE+2,-NZAG)=(3.d0-2.d0*F)*CONST(IBASE+2,JBASE+2,-NZAG)*   &
                  & (-CZP(IBASE+2,JBASE+2,-NZAG))
                R(IBASE+2,JBASE+2,-NZAG)=F*CONST(IBASE+2,JBASE+2,-NZAG)*           &
                & (CXM(IBASE+2,JBASE+2,-NZAG)*                                     &
                & V(IBASE+2-1,JBASE+2,-NZAG)-(CXM(IBASE+2,JBASE+2,-NZAG)+          &
                & 1.d0/DX(IBASE+2)/(REXT+RSID+1.d0/HOAV))*                             &
                & V(IBASE+2,JBASE+2,-NZAG)+CYM(IBASE+2,JBASE+2,-NZAG)*             &
                & V(IBASE+2,JBASE+2-1,-NZAG)-(CYM(IBASE+2,JBASE+2,-NZAG)+          &
                & 1.d0/DY(JBASE+2)/(REXT+RSID+1.d0/HOAV))*                             &
                & V(IBASE+2,JBASE+2,-NZAG))+V(IBASE+2,JBASE+2,-NZAG)+              &
                & F*CONST(IBASE+2,JBASE+2,-NZAG)*(TDBAV/DX(IBASE+2)/               &
                & (REXT+RSID+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-(0.88d0*SIGMA*             &
                & (VEXT(IBASE+2,JBASE+2,-NZAG)+273.15d0)**4)/DX(IBASE+2)+           &
                & TDBAV/DY(JBASE+2)/(REXT+RSID+1.d0/HOAV)+RSOLVAV/DY(JBASE+2)-       &
                & (0.88d0*SIGMA*(VEXT(IBASE+2,JBASE+2,-NZAG)+273.15d0)**4)/           &
                & DY(JBASE+2))

!***  OUTER CORNER CELL (RIM JOIST)
                A(IBASE+2,JBASE+2,-NZAG+1)=(3.d0-2.d0*F)*CONST(IBASE+2,JBASE+2,-NZAG+1)* &
                  & (-CZM(IBASE+2,JBASE+2,-NZAG+1))
                B(IBASE+2,JBASE+2,-NZAG+1)=1.d0+(3.d0-2.d0*F)*                           &
                  & CONST(IBASE+2,JBASE+2,-NZAG+1)*                                &
                  & (CZM(IBASE+2,JBASE+2,-NZAG+1)+CZP(IBASE+2,JBASE+2,-NZAG+1))
                C(IBASE+2,JBASE+2,-NZAG+1)=(3.d0-2.d0*F)*CONST(IBASE+2,JBASE+2,-NZAG+1)* &
                  & (-CZP(IBASE+2,JBASE+2,-NZAG+1))
                R(IBASE+2,JBASE+2,-NZAG+1)=F*CONST(IBASE+2,JBASE+2,-NZAG+1)*       &
                & (CXM(IBASE+2,JBASE+2,-NZAG+1)*V(IBASE+2-1,JBASE+2,-NZAG+1)-      &
                & (CXM(IBASE+2,JBASE+2,-NZAG+1)+1.d0/DX(IBASE+2)/(REXT+RSID+1.d0/HOAV))* &
                & V(IBASE+2,JBASE+2,-NZAG+1)+CYM(IBASE+2,JBASE+2,-NZAG+1)*         &
                & V(IBASE+2,JBASE+2-1,-NZAG+1)-(CYM(IBASE+2,JBASE+2,-NZAG+1)+1.d0/   &
                & DY(JBASE+2)/(REXT+RSID+1.d0/HOAV))*V(IBASE+2,JBASE+2,-NZAG+1))+    &
                & V(IBASE+2,JBASE+2,-NZAG+1)+F*CONST(IBASE+2,JBASE+2,-NZAG+1)*     &
                & (TDBAV/DX(IBASE+2)/(REXT+RSID+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-      &
                & (0.88d0*SIGMA*(VEXT(IBASE+2,JBASE+2,-NZAG+1)+273.15d0)**4)/         &
                & DX(IBASE+2)+TDBAV/DY(JBASE+2)/(REXT+RSID+1.d0/HOAV)+RSOLVAV/       &
                & DY(JBASE+2)-(0.88d0*SIGMA*(VEXT(IBASE+2,JBASE+2,-NZAG+1)+273.15d0)**4) &
                & /DY(JBASE+2))

!***  TOP OUTER CORNER CELL (WALL)
                A(IBASE+2,JBASE+2,-NZAG+2)=(3.d0-2.d0*F)*CONST(IBASE+2,JBASE+2,-NZAG+2)* &
                  & (-CZM(IBASE+2,JBASE+2,-NZAG+2))
                B(IBASE+2,JBASE+2,-NZAG+2)=1.d0+(3.d0-2.d0*F)*                           &
                  & CONST(IBASE+2,JBASE+2,-NZAG+2)*(CZM(IBASE+2,JBASE+2,-NZAG+2)+  &
                  & CZP(IBASE+2,JBASE+2,-NZAG+2))
                C(IBASE+2,JBASE+2,-NZAG+2)=(3.d0-2.d0*F)*CONST(IBASE+2,JBASE+2,-NZAG+2)* &
                  & (-CZP(IBASE+2,JBASE+2,-NZAG+2))
                R(IBASE+2,JBASE+2,-NZAG+2)=F*CONST(IBASE+2,JBASE+2,-NZAG+2)*       &
                & (CXM(IBASE+2,JBASE+2,-NZAG+2)*V(IBASE+2-1,JBASE+2,-NZAG+2)-      &
                & (CXM(IBASE+2,JBASE+2,-NZAG+2)+1.d0/DX(IBASE+2)/(REXT+RSID+1.d0/HOAV))* &
                & V(IBASE+2,JBASE+2,-NZAG+2)+CYM(IBASE+2,JBASE+2,-NZAG+2)*         &
                & V(IBASE+2,JBASE+2-1,-NZAG+2)-(CYM(IBASE+2,JBASE+2,-NZAG+2)+1.d0/   &
                & DY(JBASE+2)/(REXT+RSID+1.d0/HOAV))*V(IBASE+2,JBASE+2,-NZAG+2))+    &
                & V(IBASE+2,JBASE+2,-NZAG+2)+F*CONST(IBASE+2,JBASE+2,-NZAG+2)*     &
                & (TDBAV/DX(IBASE+2)/(REXT+RSID+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-      &
                & (0.88d0*SIGMA*(VEXT(IBASE+2,JBASE+2,-NZAG+2)+273.15d0)**4)/         &
                & DX(IBASE+2)+TDBAV/DY(JBASE+2)/(REXT+RSID+1.d0/HOAV)+RSOLVAV/       &
                & DY(JBASE+2)-(0.88d0*SIGMA*(VEXT(IBASE+2,JBASE+2,-NZAG+2)+273.15d0)**4) &
                & /DY(JBASE+2))

!***  ABOVE-GRADE OUTER CORNER CELLS
                IF (NZAG.GT.3) THEN
                  DO COUNT3=-NZAG+3,-1
                    A(IBASE+2,JBASE+2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE+2,JBASE+2,COUNT3)* &
                       & (-CZM(IBASE+2,JBASE+2,COUNT3))
                    B(IBASE+2,JBASE+2,COUNT3)=1.d0+(3.d0-2.d0*F)*                        &
                       & CONST(IBASE+2,JBASE+2,COUNT3)*(CZM(IBASE+2,JBASE+2,COUNT3)+ &
                       & CZP(IBASE+2,JBASE+2,COUNT3))
                    C(IBASE+2,JBASE+2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE+2,JBASE+2,COUNT3)* &
                       & (-CZP(IBASE+2,JBASE+2,COUNT3))
                    R(IBASE+2,JBASE+2,COUNT3)=F*CONST(IBASE+2,JBASE+2,COUNT3)*     &
                    & (CXM(IBASE+2,JBASE+2,COUNT3)*V(IBASE+2-1,JBASE+2,COUNT3)-    &
                    & (CXM(IBASE+2,JBASE+2,COUNT3)+1.d0/DX(IBASE+2)/(REXT+1.d0/HOAV))* &
                    & V(IBASE+2,JBASE+2,COUNT3)+CYM(IBASE+2,JBASE+2,COUNT3)*       &
                    & V(IBASE+2,JBASE+2-1,COUNT3)-(CYM(IBASE+2,JBASE+2,COUNT3)+1.d0/ &
                    & DY(JBASE+2)/(REXT+1.d0/HOAV))*V(IBASE+2,JBASE+2,COUNT3))+      &
                    & V(IBASE+2,JBASE+2,COUNT3)+F*CONST(IBASE+2,JBASE+2,COUNT3)*   &
                    & (TDBAV/DX(IBASE+2)/(REXT+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-       &
                    & (0.88d0*SIGMA*(VEXT(IBASE+2,JBASE+2,COUNT3)+273.15d0)**4)/      &
                    & DX(IBASE+2)+TDBAV/DY(JBASE+2)/(REXT+1.d0/HOAV)+RSOLVAV/        &
                    & DY(JBASE+2)-(0.88d0*SIGMA*(VEXT(IBASE+2,JBASE+2,COUNT3)+273.15d0)**4) &
                    & /DY(JBASE+2))
                  END DO
                END IF

!***  BELOW-GRADE OUTER CORNER CELLS/GRAVEL/GROUND
                DO COUNT3=0,NZBGM1-1
                  A(IBASE+2,JBASE+2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE+2,JBASE+2,COUNT3)* &
                       & (-CZM(IBASE+2,JBASE+2,COUNT3))
                  B(IBASE+2,JBASE+2,COUNT3)=1.d0+(3.d0-2.d0*F)*                          &
                       & CONST(IBASE+2,JBASE+2,COUNT3)*(CZM(IBASE+2,JBASE+2,COUNT3)+ &
                       & CZP(IBASE+2,JBASE+2,COUNT3))
                  C(IBASE+2,JBASE+2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE+2,JBASE+2,COUNT3)* &
                       & (-CZP(IBASE+2,JBASE+2,COUNT3))
                  R(IBASE+2,JBASE+2,COUNT3)=F*CONST(IBASE+2,JBASE+2,COUNT3)*       &
                  & (CXM(IBASE+2,JBASE+2,COUNT3)*V(IBASE+2-1,JBASE+2,COUNT3)-      &
                  & (CXM(IBASE+2,JBASE+2,COUNT3)+CXP(IBASE+2,JBASE+2,COUNT3))*     &
                  & V(IBASE+2,JBASE+2,COUNT3)+CXP(IBASE+2,JBASE+2,COUNT3)*         &
                  & V(IBASE+2+1,JBASE+2,COUNT3)+CYM(IBASE+2,JBASE+2,COUNT3)*       &
                  & V(IBASE+2,JBASE+2-1,COUNT3)-(CYM(IBASE+2,JBASE+2,COUNT3)+      &
                  & CYP(IBASE+2,JBASE+2,COUNT3))*V(IBASE+2,JBASE+2,COUNT3)+        &
                  & CYP(IBASE+2,JBASE+2,COUNT3)*V(IBASE+2,JBASE+2+1,COUNT3))+      &
                  & V(IBASE+2,JBASE+2,COUNT3)
                END DO
                IF (.not. SameString(FIXBC,'FALSE')) THEN
                  A(IBASE+2,JBASE+2,NZBGM1)=(3.d0-2.d0*F)*                             &
                       & CONST(IBASE+2,JBASE+2,NZBGM1)*(-CZM(IBASE+2,JBASE+2,NZBGM1))
                  B(IBASE+2,JBASE+2,NZBGM1)=1.d0+(3.d0-2.d0*F)*                          &
                       & CONST(IBASE+2,JBASE+2,NZBGM1)*(CZM(IBASE+2,JBASE+2,NZBGM1)+ &
                       & CZP(IBASE+2,JBASE+2,NZBGM1))
                  C(IBASE+2,JBASE+2,NZBGM1)=0.
                  R(IBASE+2,JBASE+2,NZBGM1)=F*CONST(IBASE+2,JBASE+2,NZBGM1)*       &
                  & (CXM(IBASE+2,JBASE+2,NZBGM1)*V(IBASE+2-1,JBASE+2,NZBGM1)-      &
                  & (CXM(IBASE+2,JBASE+2,NZBGM1)+CXP(IBASE+2,JBASE+2,NZBGM1))*     &
                  & V(IBASE+2,JBASE+2,NZBGM1)+CXP(IBASE+2,JBASE+2,NZBGM1)*         &
                  & V(IBASE+2+1,JBASE+2,NZBGM1)+CYM(IBASE+2,JBASE+2,NZBGM1)*       &
                  & V(IBASE+2,JBASE+2-1,NZBGM1)-(CYM(IBASE+2,JBASE+2,NZBGM1)+      &
                  & CYP(IBASE+2,JBASE+2,NZBGM1))*V(IBASE+2,JBASE+2,NZBGM1)+        &
                  & CYP(IBASE+2,JBASE+2,NZBGM1)*V(IBASE+2,JBASE+2+1,NZBGM1))+      &
                  & V(IBASE+2,JBASE+2,NZBGM1)+(3.d0-2.d0*F)*                           &
                  & CONST(IBASE+2,JBASE+2,NZBGM1)*CZP(IBASE+2,JBASE+2,NZBGM1)*TDEEP
                ELSE
                  A(IBASE+2,JBASE+2,NZBGM1)=(3.d0-2.d0*F)*                             &
                       & CONST(IBASE+2,JBASE+2,NZBGM1)*(-CZM(IBASE+2,JBASE+2,NZBGM1))
                  B(IBASE+2,JBASE+2,NZBGM1)=1.d0+(3.d0-2.d0*F)*                          &
                       & CONST(IBASE+2,JBASE+2,NZBGM1)*CZM(IBASE+2,JBASE+2,NZBGM1)
                  C(IBASE+2,JBASE+2,NZBGM1)=0.
                  R(IBASE+2,JBASE+2,NZBGM1)=F*CONST(IBASE+2,JBASE+2,NZBGM1)*       &
                  & (CXM(IBASE+2,JBASE+2,NZBGM1)*V(IBASE+2-1,JBASE+2,NZBGM1)-      &
                  & (CXM(IBASE+2,JBASE+2,NZBGM1)+CXP(IBASE+2,JBASE+2,NZBGM1))*     &
                  & V(IBASE+2,JBASE+2,NZBGM1)+CXP(IBASE+2,JBASE+2,NZBGM1)*         &
                  & V(IBASE+2+1,JBASE+2,NZBGM1)+CYM(IBASE+2,JBASE+2,NZBGM1)*       &
                  & V(IBASE+2,JBASE+2-1,NZBGM1)-(CYM(IBASE+2,JBASE+2,NZBGM1)+      &
                  & CYP(IBASE+2,JBASE+2,NZBGM1))*V(IBASE+2,JBASE+2,NZBGM1)+        &
                  & CYP(IBASE+2,JBASE+2,NZBGM1)*V(IBASE+2,JBASE+2+1,NZBGM1))+      &
                  & V(IBASE+2,JBASE+2,NZBGM1)
                END IF
                N=NZBGM1-(-NZAG)+1
                L=1
                DO COUNT3=-NZAG,NZBGM1
                  AA(L)=A(IBASE+2,JBASE+2,COUNT3)
                  BB(L)=B(IBASE+2,JBASE+2,COUNT3)
                  CC(L)=C(IBASE+2,JBASE+2,COUNT3)
                  RR(L)=R(IBASE+2,JBASE+2,COUNT3)
                  L=L+1
                END DO
                CALL TRIDI3D (AA,BB,CC,RR,N,X)
                L=0
                DO COUNT3=-NZAG,NZBGM1
                  T(IBASE+2,JBASE+2,COUNT3)=X(L+1)
                  L=L+1
                END DO
                DO COUNT3=-NZAG,-1
                  QEXT(IBASE+2,JBASE+2,COUNT3)=(TDBAV-T(IBASE+2,JBASE+2,COUNT3))/  &
                     & (REXT+RSID+1.d0/HOAV)
                  TEXT(IBASE+2,JBASE+2,COUNT3)=QEXT(IBASE+2,JBASE+2,COUNT3)*       &
                     & (REXT+RSID)+T(IBASE+2,JBASE+2,COUNT3)
                END DO

!***  SECTION 6:  RIM JOIST, PARALLEL TO Y-AXIS
!***  PERIMETER CEILING CELL
                DO COUNT2=0,JBASE+1
                  A(IBASE+2,COUNT2,-NZAG)=0.
                  B(IBASE+2,COUNT2,-NZAG)=1.d0+(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,-NZAG)* &
                       & (CZP(IBASE+2,COUNT2,-NZAG))
                  C(IBASE+2,COUNT2,-NZAG)=(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,-NZAG)*   &
                       & (-CZP(IBASE+2,COUNT2,-NZAG))
                  R(IBASE+2,COUNT2,-NZAG)=F*CONST(IBASE+2,COUNT2,-NZAG)*           &
                  & (CXM(IBASE+2,COUNT2,-NZAG)*V(IBASE+2-1,COUNT2,-NZAG)-          &
                  & (CXM(IBASE+2,COUNT2,-NZAG)+1.d0/DX(IBASE+2)/(REXT+RSID+1.d0/HOAV))* &
                  & V(IBASE+2,COUNT2,-NZAG)+CYM(IBASE+2,COUNT2,-NZAG)*             &
                  & V(IBASE+2,COUNT2-1,-NZAG)-(CYM(IBASE+2,COUNT2,-NZAG)+          &
                  & CYP(IBASE+2,COUNT2,-NZAG))*V(IBASE+2,COUNT2,-NZAG)+            &
                  & CYP(IBASE+2,COUNT2,-NZAG)*V(IBASE+2,COUNT2+1,-NZAG))+          &
                  & V(IBASE+2,COUNT2,-NZAG)+F*CONST(IBASE+2,COUNT2,-NZAG)*         &
                  & (TDBAV/DX(IBASE+2)/(REXT+RSID+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-    &
                  & (0.88d0*SIGMA*(VEXT(IBASE+2,COUNT2,-NZAG)+273.15d0)**4)/DX(IBASE+2))

!***  RIM JOIST CELL
                  A(IBASE+2,COUNT2,-NZAG+1)=(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,-NZAG+1)* &
                     & (-CZM(IBASE+2,COUNT2,-NZAG+1))
                  B(IBASE+2,COUNT2,-NZAG+1)=1.d0+(3.d0-2.d0*F)*                        &
                     & CONST(IBASE+2,COUNT2,-NZAG+1)*(CZM(IBASE+2,COUNT2,-NZAG+1)+ &
                     & CZP(IBASE+2,COUNT2,-NZAG+1))
                  C(IBASE+2,COUNT2,-NZAG+1)=(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,-NZAG+1)* &
                     & (-CZP(IBASE+2,COUNT2,-NZAG+1))
                  R(IBASE+2,COUNT2,-NZAG+1)=F*CONST(IBASE+2,COUNT2,-NZAG+1)*     &
                  & (-(1.d0/DX(IBASE+2)/(RSILL+1.d0/HIN(6)+DX(IBASE+2)/              &
                  & TCON(MTYPE(IBASE+2,COUNT2,-NZAG+1)))+1.d0/DX(IBASE+2)/         &
                  & (REXT+RSID+1.d0/HOAV))*V(IBASE+2,COUNT2,-NZAG+1)+              &
                  & CYM(IBASE+2,COUNT2,-NZAG+1)*V(IBASE+2,COUNT2-1,-NZAG+1)-     &
                  & (CYM(IBASE+2,COUNT2,-NZAG+1)+CYP(IBASE+2,COUNT2,-NZAG+1))*   &
                  & V(IBASE+2,COUNT2,-NZAG+1)+CYP(IBASE+2,COUNT2,-NZAG+1)*       &
                  & V(IBASE+2,COUNT2+1,-NZAG+1))+V(IBASE+2,COUNT2,-NZAG+1)+      &
                  & F*CONST(IBASE+2,COUNT2,-NZAG+1)*(TBAV/DX(IBASE+2)/           &
                  & (RSILL+1.d0/HIN(6)+DX(IBASE+2)/                                &
                  & TCON(MTYPE(IBASE+2,COUNT2,-NZAG+1)))+TDBAV/DX(IBASE+2)/      &
                  & (REXT+RSID+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-(0.88d0*SIGMA*         &
                  & (VEXT(IBASE+2,COUNT2,-NZAG+1)+273.15d0)**4)/DX(IBASE+2))

!***  TOP OUTSIDE WALL CELLS
                  A(IBASE+2,COUNT2,-NZAG+2)=(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,-NZAG+2)* &
                      & (-CZM(IBASE+2,COUNT2,-NZAG+2))
                  B(IBASE+2,COUNT2,-NZAG+2)=1.d0+(3.d0-2.d0*F)*                        &
                      & CONST(IBASE+2,COUNT2,-NZAG+2)*(CZM(IBASE+2,COUNT2,-NZAG+2)+ &
                      & CZP(IBASE+2,COUNT2,-NZAG+2))
                  C(IBASE+2,COUNT2,-NZAG+2)=(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,-NZAG+2)* &
                      & (-CZP(IBASE+2,COUNT2,-NZAG+2))
                  R(IBASE+2,COUNT2,-NZAG+2)=F*CONST(IBASE+2,COUNT2,-NZAG+2)*     &
                  & (CXM(IBASE+2,COUNT2,-NZAG+2)*V(IBASE+2-1,COUNT2,-NZAG+2)-    &
                  & (CXM(IBASE+2,COUNT2,-NZAG+2)+1.d0/DX(IBASE+2)/(REXT+RSID+1.d0/   &
                  &  HOAV))* &
                  & V(IBASE+2,COUNT2,-NZAG+2)+CYM(IBASE+2,COUNT2,-NZAG+2)*       &
                  & V(IBASE+2,COUNT2-1,-NZAG+2)-(CYM(IBASE+2,COUNT2,-NZAG+2)+    &
                  & CYP(IBASE+2,COUNT2,-NZAG+2))*V(IBASE+2,COUNT2,-NZAG+2)+      &
                  & CYP(IBASE+2,COUNT2,-NZAG+2)*V(IBASE+2,COUNT2+1,-NZAG+2))+    &
                  & V(IBASE+2,COUNT2,-NZAG+2)+F*CONST(IBASE+2,COUNT2,-NZAG+2)*   &
                  & (TDBAV/DX(IBASE+2)/(REXT+RSID+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-  &
                  & (0.88d0*SIGMA*(VEXT(IBASE+2,COUNT2,-NZAG+2)+273.15d0)**4)/      &
                  & DX(IBASE+2))

!***  ABOVE-GRADE OUTSIDE WALL CELLS
                  IF (NZAG.GT.3) THEN
                    DO COUNT3=-NZAG+3,-1
                      A(IBASE+2,COUNT2,COUNT3)=(3.d0-2.d0*F)*                          &
                            & CONST(IBASE+2,COUNT2,COUNT3)*(-CZM(IBASE+2,COUNT2,COUNT3))
                      B(IBASE+2,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*                       &
                            & CONST(IBASE+2,COUNT2,COUNT3)*                        &
                            & (CZM(IBASE+2,COUNT2,COUNT3)+CZP(IBASE+2,COUNT2,COUNT3))
                      C(IBASE+2,COUNT2,COUNT3)=(3.d0-2.d0*F)*                          &
                            & CONST(IBASE+2,COUNT2,COUNT3)*(-CZP(IBASE+2,COUNT2,COUNT3))
                      R(IBASE+2,COUNT2,COUNT3)=F*CONST(IBASE+2,COUNT2,COUNT3)*     &
                      & (CXM(IBASE+2,COUNT2,COUNT3)*V(IBASE+2-1,COUNT2,COUNT3)-    &
                      & (CXM(IBASE+2,COUNT2,COUNT3)+1.d0/DX(IBASE+2)/                &
                      & (REXT+1.d0/HOAV))*V(IBASE+2,COUNT2,COUNT3)+                  &
                      & CYM(IBASE+2,COUNT2,COUNT3)*V(IBASE+2,COUNT2-1,COUNT3)-     &
                      & (CYM(IBASE+2,COUNT2,COUNT3)+CYP(IBASE+2,COUNT2,COUNT3))*   &
                      & V(IBASE+2,COUNT2,COUNT3)+CYP(IBASE+2,COUNT2,COUNT3)*       &
                      & V(IBASE+2,COUNT2+1,COUNT3))+V(IBASE+2,COUNT2,COUNT3)+F*    &
                      & CONST(IBASE+2,COUNT2,COUNT3)*(TDBAV/DX(IBASE+2)/           &
                      & (REXT+1.d0/HOAV)+RSOLVAV/DX(IBASE+2)-(0.88d0*SIGMA*            &
                      & (VEXT(IBASE+2,COUNT2,COUNT3)+273.15d0)**4)/DX(IBASE+2))
                    END DO
                  END IF

!***  BELOW-GRADE OUTSIDE WALL CELLS/GRAVEL/GROUND
                  DO COUNT3=0,NZBGM1-1
                    A(IBASE+2,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,COUNT3)* &
                       & (-CZM(IBASE+2,COUNT2,COUNT3))
                    B(IBASE+2,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*                         &
                       & CONST(IBASE+2,COUNT2,COUNT3)*(CZM(IBASE+2,COUNT2,COUNT3)+ &
                       & CZP(IBASE+2,COUNT2,COUNT3))
                    C(IBASE+2,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,COUNT3)* &
                       & (-CZP(IBASE+2,COUNT2,COUNT3))
                    R(IBASE+2,COUNT2,COUNT3)=F*CONST(IBASE+2,COUNT2,COUNT3)*       &
                    & (CXM(IBASE+2,COUNT2,COUNT3)*V(IBASE+2-1,COUNT2,COUNT3)-      &
                    & (CXM(IBASE+2,COUNT2,COUNT3)+CXP(IBASE+2,COUNT2,COUNT3))*     &
                    & V(IBASE+2,COUNT2,COUNT3)+CXP(IBASE+2,COUNT2,COUNT3)*         &
                    & V(IBASE+2+1,COUNT2,COUNT3)+CYM(IBASE+2,COUNT2,COUNT3)*       &
                    & V(IBASE+2,COUNT2-1,COUNT3)-(CYM(IBASE+2,COUNT2,COUNT3)+      &
                    & CYP(IBASE+2,COUNT2,COUNT3))*V(IBASE+2,COUNT2,COUNT3)+        &
                    & CYP(IBASE+2,COUNT2,COUNT3)*V(IBASE+2,COUNT2+1,COUNT3))+      &
                    & V(IBASE+2,COUNT2,COUNT3)
                  END DO
                  IF (.not. SameString(FIXBC,'FALSE')) THEN
                    A(IBASE+2,COUNT2,NZBGM1)=(3.d0-2.d0*F)*                            &
                       & CONST(IBASE+2,COUNT2,NZBGM1)*(-CZM(IBASE+2,COUNT2,NZBGM1))
                    B(IBASE+2,COUNT2,NZBGM1)=1.d0+(3.d0-2.d0*F)*                         &
                       & CONST(IBASE+2,COUNT2,NZBGM1)*(CZM(IBASE+2,COUNT2,NZBGM1)+ &
                       & CZP(IBASE+2,COUNT2,NZBGM1))
                    C(IBASE+2,COUNT2,NZBGM1)=0.
                    R(IBASE+2,COUNT2,NZBGM1)=F*CONST(IBASE+2,COUNT2,NZBGM1)*       &
                    & (CXM(IBASE+2,COUNT2,NZBGM1)*V(IBASE+2-1,COUNT2,NZBGM1)-      &
                    & (CXM(IBASE+2,COUNT2,NZBGM1)+CXP(IBASE+2,COUNT2,NZBGM1))*     &
                    & V(IBASE+2,COUNT2,NZBGM1)+CXP(IBASE+2,COUNT2,NZBGM1)*         &
                    & V(IBASE+2+1,COUNT2,NZBGM1)+CYM(IBASE+2,COUNT2,NZBGM1)*       &
                    & V(IBASE+2,COUNT2-1,NZBGM1)-(CYM(IBASE+2,COUNT2,NZBGM1)+      &
                    & CYP(IBASE+2,COUNT2,NZBGM1))*V(IBASE+2,COUNT2,NZBGM1)+        &
                    & CYP(IBASE+2,COUNT2,NZBGM1)*V(IBASE+2,COUNT2+1,NZBGM1))+      &
                    & V(IBASE+2,COUNT2,NZBGM1)+(3.d0-2.d0*F)*                          &
                    & CONST(IBASE+2,COUNT2,NZBGM1)*CZP(IBASE+2,COUNT2,NZBGM1)*TDEEP
                  ELSE
                    A(IBASE+2,COUNT2,NZBGM1)=(3.d0-2.d0*F)*CONST(IBASE+2,COUNT2,NZBGM1)* &
                       & (-CZM(IBASE+2,COUNT2,NZBGM1))
                    B(IBASE+2,COUNT2,NZBGM1)=1.d0+(3.d0-2.d0*F)*                         &
                         & CONST(IBASE+2,COUNT2,NZBGM1)*CZM(IBASE+2,COUNT2,NZBGM1)
                    C(IBASE+2,COUNT2,NZBGM1)=0.
                    R(IBASE+2,COUNT2,NZBGM1)=F*CONST(IBASE+2,COUNT2,NZBGM1)*       &
                    & (CXM(IBASE+2,COUNT2,NZBGM1)*V(IBASE+2-1,COUNT2,NZBGM1)-      &
                    & (CXM(IBASE+2,COUNT2,NZBGM1)+CXP(IBASE+2,COUNT2,NZBGM1))*     &
                    & V(IBASE+2,COUNT2,NZBGM1)+CXP(IBASE+2,COUNT2,NZBGM1)*         &
                    & V(IBASE+2+1,COUNT2,NZBGM1)+CYM(IBASE+2,COUNT2,NZBGM1)*       &
                    & V(IBASE+2,COUNT2-1,NZBGM1)-(CYM(IBASE+2,COUNT2,NZBGM1)+      &
                    & CYP(IBASE+2,COUNT2,NZBGM1))*V(IBASE+2,COUNT2,NZBGM1)+        &
                    & CYP(IBASE+2,COUNT2,NZBGM1)*V(IBASE+2,COUNT2+1,NZBGM1))+      &
                    & V(IBASE+2,COUNT2,NZBGM1)
                  END IF
                  N=NZBGM1-(-NZAG)+1
                  L=1
                  DO COUNT3=-NZAG,NZBGM1
                    AA(L)=A(IBASE+2,COUNT2,COUNT3)
                    BB(L)=B(IBASE+2,COUNT2,COUNT3)
                    CC(L)=C(IBASE+2,COUNT2,COUNT3)
                    RR(L)=R(IBASE+2,COUNT2,COUNT3)
                    L=L+1
                  END DO
                  CALL TRIDI3D (AA,BB,CC,RR,N,X)
                  L=0
                  DO COUNT3=-NZAG,NZBGM1
                    T(IBASE+2,COUNT2,COUNT3)=X(L+1)
                    L=L+1
                  END DO
                  DO COUNT3=-NZAG,-1
                    QEXT(IBASE+2,COUNT2,COUNT3)=(TDBAV-T(IBASE+2,COUNT2,COUNT3))/  &
                       & (REXT+RSID+1.d0/HOAV)
                    TEXT(IBASE+2,COUNT2,COUNT3)=QEXT(IBASE+2,COUNT2,COUNT3)*       &
                       & (REXT+RSID)+T(IBASE+2,COUNT2,COUNT3)
                  END DO
                END DO

 !***  SECTION 7:  BELOW-GRADE (1)
                DO COUNT1=0,NXM1
                  DO COUNT2=JBASE+3,NYM1

!***  GROUND SURFACE CELLS
                    IF (ISNW.EQ.1) THEN
                      A(COUNT1,COUNT2,0)=0.
                      B(COUNT1,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*      &
                            & (CZP(COUNT1,COUNT2,0))
                      C(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*         &
                            & (-CZP(COUNT1,COUNT2,0))
                      R(COUNT1,COUNT2,0)=F*CONST(COUNT1,COUNT2,0)*                 &
                      & (CXM(COUNT1,COUNT2,0)*V(COUNT1-1,COUNT2,0)-                &
                      & (CXM(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0))*               &
                      & V(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0)*                   &
                      & V(COUNT1+1,COUNT2,0)+CYM(COUNT1,COUNT2,0)*                 &
                      & V(COUNT1,COUNT2-1,0)-(CYM(COUNT1,COUNT2,0)+                &
                      & CYP(COUNT1,COUNT2,0))*V(COUNT1,COUNT2,0)+                  &
                      & CYP(COUNT1,COUNT2,0)*V(COUNT1,COUNT2+1,0))+                &
                      & V(COUNT1,COUNT2,0)+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*       &
                      & (GOFTAV(COUNT1,COUNT2)/DZ(0))
                    ELSE
                      A(COUNT1,COUNT2,0)=0.
                      B(COUNT1,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*      &
                            & (CZP(COUNT1,COUNT2,0))
                      C(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*         &
                            & (-CZP(COUNT1,COUNT2,0))
                      R(COUNT1,COUNT2,0)=F*CONST(COUNT1,COUNT2,0)*                 &
                      & (CXM(COUNT1,COUNT2,0)*V(COUNT1-1,COUNT2,0)-                &
                      & (CXM(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0))*               &
                      & V(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0)*V(COUNT1+1,COUNT2,0)+ &
                      & CYM(COUNT1,COUNT2,0)*V(COUNT1,COUNT2-1,0)-                 &
                      & (CYM(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0))*               &
                      & V(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0)*                   &
                      & V(COUNT1,COUNT2+1,0))+V(COUNT1,COUNT2,0)+(3.d0-2.d0*F)*        &
                      & CONST(COUNT1,COUNT2,0)*(GOFTAV(COUNT1,COUNT2)/DZ(0)+TDBAV/ &
                      & DZ(0)/RSNWAV)
                    END IF

!***  BELOW-GRADE
                    DO COUNT3=1,NZBGM1-1
                      A(COUNT1,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,COUNT3)* &
                             & (-CZM(COUNT1,COUNT2,COUNT3))
                      B(COUNT1,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*                        &
                             & CONST(COUNT1,COUNT2,COUNT3)*                         &
                            & (CZM(COUNT1,COUNT2,COUNT3)+CZP(COUNT1,COUNT2,COUNT3))
                      C(COUNT1,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,COUNT3)* &
                            & (-CZP(COUNT1,COUNT2,COUNT3))
                      R(COUNT1,COUNT2,COUNT3)=F*CONST(COUNT1,COUNT2,COUNT3)*       &
                      & (CXM(COUNT1,COUNT2,COUNT3)*V(COUNT1-1,COUNT2,COUNT3)-      &
                      & (CXM(COUNT1,COUNT2,COUNT3)+CXP(COUNT1,COUNT2,COUNT3))*     &
                      & V(COUNT1,COUNT2,COUNT3)+CXP(COUNT1,COUNT2,COUNT3)*         &
                      & V(COUNT1+1,COUNT2,COUNT3)+CYM(COUNT1,COUNT2,COUNT3)*       &
                      & V(COUNT1,COUNT2-1,COUNT3)-(CYM(COUNT1,COUNT2,COUNT3)+      &
                      & CYP(COUNT1,COUNT2,COUNT3))*V(COUNT1,COUNT2,COUNT3)+        &
                      & CYP(COUNT1,COUNT2,COUNT3)*V(COUNT1,COUNT2+1,COUNT3))+      &
                      & V(COUNT1,COUNT2,COUNT3)
                    END DO
                    IF (.not. SameString(FIXBC,'FALSE')) THEN
                      A(COUNT1,COUNT2,NZBGM1)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,NZBGM1)* &
                            & (-CZM(COUNT1,COUNT2,NZBGM1))
                      B(COUNT1,COUNT2,NZBGM1)=1.d0+(3.d0-2.d0*F)*                        &
                            & CONST(COUNT1,COUNT2,NZBGM1)*(CZM(COUNT1,COUNT2,NZBGM1)+ &
                            & CZP(COUNT1,COUNT2,NZBGM1))
                      C(COUNT1,COUNT2,NZBGM1)=0.
                      R(COUNT1,COUNT2,NZBGM1)=F*CONST(COUNT1,COUNT2,NZBGM1)*       &
                      & (CXM(COUNT1,COUNT2,NZBGM1)*V(COUNT1-1,COUNT2,NZBGM1)-      &
                      & (CXM(COUNT1,COUNT2,NZBGM1)+CXP(COUNT1,COUNT2,NZBGM1))*     &
                      & V(COUNT1,COUNT2,NZBGM1)+CXP(COUNT1,COUNT2,NZBGM1)*         &
                      & V(COUNT1+1,COUNT2,NZBGM1)+CYM(COUNT1,COUNT2,NZBGM1)*       &
                      & V(COUNT1,COUNT2-1,NZBGM1)-(CYM(COUNT1,COUNT2,NZBGM1)+      &
                      & CYP(COUNT1,COUNT2,NZBGM1))*V(COUNT1,COUNT2,NZBGM1)+        &
                      & CYP(COUNT1,COUNT2,NZBGM1)*V(COUNT1,COUNT2+1,NZBGM1))+      &
                      & V(COUNT1,COUNT2,NZBGM1)+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,NZBGM1)* &
                      & CZP(COUNT1,COUNT2,NZBGM1)*TDEEP
                    ELSE
                      A(COUNT1,COUNT2,NZBGM1)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,NZBGM1)* &
                            & (-CZM(COUNT1,COUNT2,NZBGM1))
                      B(COUNT1,COUNT2,NZBGM1)=1.d0+(3.d0-2.d0*F)*                        &
                            & CONST(COUNT1,COUNT2,NZBGM1)*CZM(COUNT1,COUNT2,NZBGM1)
                      C(COUNT1,COUNT2,NZBGM1)=0.
                      R(COUNT1,COUNT2,NZBGM1)=F*CONST(COUNT1,COUNT2,NZBGM1)*       &
                      & (CXM(COUNT1,COUNT2,NZBGM1)*V(COUNT1-1,COUNT2,NZBGM1)-      &
                      & (CXM(COUNT1,COUNT2,NZBGM1)+CXP(COUNT1,COUNT2,NZBGM1))*     &
                      & V(COUNT1,COUNT2,NZBGM1)+CXP(COUNT1,COUNT2,NZBGM1)*         &
                      & V(COUNT1+1,COUNT2,NZBGM1)+CYM(COUNT1,COUNT2,NZBGM1)*       &
                      & V(COUNT1,COUNT2-1,NZBGM1)-(CYM(COUNT1,COUNT2,NZBGM1)+      &
                      & CYP(COUNT1,COUNT2,NZBGM1))*V(COUNT1,COUNT2,NZBGM1)+        &
                      & CYP(COUNT1,COUNT2,NZBGM1)*V(COUNT1,COUNT2+1,NZBGM1))+      &
                      & V(COUNT1,COUNT2,NZBGM1)
                    END IF
                    N=NZBGM1-(0)+1
                    L=1
                    DO COUNT3=0,NZBGM1
                      AA(L)=A(COUNT1,COUNT2,COUNT3)
                      BB(L)=B(COUNT1,COUNT2,COUNT3)
                      CC(L)=C(COUNT1,COUNT2,COUNT3)
                      RR(L)=R(COUNT1,COUNT2,COUNT3)
                      L=L+1
                    END DO
                    CALL TRIDI3D (AA,BB,CC,RR,N,X)
                    L=0
                    DO COUNT3=0,NZBGM1
                      T(COUNT1,COUNT2,COUNT3)=X(L+1)
                      L=L+1
                    END DO
                  END DO
                END DO

!***  SECTION 8:  BELOW-GRADE (2)
                DO COUNT1=IBASE+3,NXM1
                  DO COUNT2=0,JBASE+2
!***  GROUND SURFACE CELLS
                    IF (ISNW.EQ.1) THEN
                      A(COUNT1,COUNT2,0)=0.
                      B(COUNT1,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*      &
                            & (CZP(COUNT1,COUNT2,0))
                      C(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*         &
                            & (-CZP(COUNT1,COUNT2,0))
                      R(COUNT1,COUNT2,0)=F*CONST(COUNT1,COUNT2,0)*                 &
                      & (CXM(COUNT1,COUNT2,0)*V(COUNT1-1,COUNT2,0)-                &
                      & (CXM(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0))*               &
                      & V(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0)*V(COUNT1+1,COUNT2,0)+ &
                      & CYM(COUNT1,COUNT2,0)*V(COUNT1,COUNT2-1,0)-                 &
                      & (CYM(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0))*               &
                      & V(COUNT1,COUNT2,0)+CYP(COUNT1,COUNT2,0)*                   &
                      & V(COUNT1,COUNT2+1,0))+V(COUNT1,COUNT2,0)+                  &
                      & (3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*(GOFTAV(COUNT1,COUNT2)/DZ(0))
                    ELSE
                      A(COUNT1,COUNT2,0)=0.
                      B(COUNT1,COUNT2,0)=1.d0+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*      &
                            & (CZP(COUNT1,COUNT2,0))
                      C(COUNT1,COUNT2,0)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*         &
                            & (-CZP(COUNT1,COUNT2,0))
                      R(COUNT1,COUNT2,0)=F*CONST(COUNT1,COUNT2,0)*                 &
                      & (CXM(COUNT1,COUNT2,0)*V(COUNT1-1,COUNT2,0)-                &
                      & (CXM(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0))*               &
                      & V(COUNT1,COUNT2,0)+CXP(COUNT1,COUNT2,0)*                   &
                      & V(COUNT1+1,COUNT2,0)+CYM(COUNT1,COUNT2,0)*                 &
                      & V(COUNT1,COUNT2-1,0)-(CYM(COUNT1,COUNT2,0)+                &
                      & CYP(COUNT1,COUNT2,0))*V(COUNT1,COUNT2,0)+                  &
                      & CYP(COUNT1,COUNT2,0)*V(COUNT1,COUNT2+1,0))+                &
                      & V(COUNT1,COUNT2,0)+(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,0)*       &
                      & (GOFTAV(COUNT1,COUNT2)/DZ(0)+TDBAV/DZ(0)/RSNWAV)
                    END IF

!***  BELOW-GRADE
                    DO COUNT3=1,NZBGM1-1
                      A(COUNT1,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,COUNT3)* &
                            & (-CZM(COUNT1,COUNT2,COUNT3))
                      B(COUNT1,COUNT2,COUNT3)=1.d0+(3.d0-2.d0*F)*                        &
                            & CONST(COUNT1,COUNT2,COUNT3)*(CZM(COUNT1,COUNT2,COUNT3)+ &
                            & CZP(COUNT1,COUNT2,COUNT3))
                      C(COUNT1,COUNT2,COUNT3)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,COUNT3)* &
                            & (-CZP(COUNT1,COUNT2,COUNT3))
                      R(COUNT1,COUNT2,COUNT3)=F*CONST(COUNT1,COUNT2,COUNT3)*       &
                      & (CXM(COUNT1,COUNT2,COUNT3)*V(COUNT1-1,COUNT2,COUNT3)-      &
                      & (CXM(COUNT1,COUNT2,COUNT3)+CXP(COUNT1,COUNT2,COUNT3))*     &
                      & V(COUNT1,COUNT2,COUNT3)+CXP(COUNT1,COUNT2,COUNT3)*         &
                      & V(COUNT1+1,COUNT2,COUNT3)+CYM(COUNT1,COUNT2,COUNT3)*       &
                      & V(COUNT1,COUNT2-1,COUNT3)-(CYM(COUNT1,COUNT2,COUNT3)+      &
                      & CYP(COUNT1,COUNT2,COUNT3))*V(COUNT1,COUNT2,COUNT3)+        &
                      & CYP(COUNT1,COUNT2,COUNT3)*V(COUNT1,COUNT2+1,COUNT3))+      &
                      & V(COUNT1,COUNT2,COUNT3)
                    END DO
                    IF (.not. SameString(FIXBC,'FALSE')) THEN
                      A(COUNT1,COUNT2,NZBGM1)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,NZBGM1)* &
                            & (-CZM(COUNT1,COUNT2,NZBGM1))
                      B(COUNT1,COUNT2,NZBGM1)=1.d0+(3.d0-2.d0*F)*                        &
                            & CONST(COUNT1,COUNT2,NZBGM1)*(CZM(COUNT1,COUNT2,NZBGM1)+ &
                            & CZP(COUNT1,COUNT2,NZBGM1))
                      C(COUNT1,COUNT2,NZBGM1)=0.
                      R(COUNT1,COUNT2,NZBGM1)=F*CONST(COUNT1,COUNT2,NZBGM1)*       &
                      & (CXM(COUNT1,COUNT2,NZBGM1)*V(COUNT1-1,COUNT2,NZBGM1)-      &
                      & (CXM(COUNT1,COUNT2,NZBGM1)+CXP(COUNT1,COUNT2,NZBGM1))*     &
                      & V(COUNT1,COUNT2,NZBGM1)+CXP(COUNT1,COUNT2,NZBGM1)*         &
                      & V(COUNT1+1,COUNT2,NZBGM1)+CYM(COUNT1,COUNT2,NZBGM1)*       &
                      & V(COUNT1,COUNT2-1,NZBGM1)-(CYM(COUNT1,COUNT2,NZBGM1)+      &
                      & CYP(COUNT1,COUNT2,NZBGM1))*V(COUNT1,COUNT2,NZBGM1)+        &
                      & CYP(COUNT1,COUNT2,NZBGM1)*V(COUNT1,COUNT2+1,NZBGM1))+      &
                      & V(COUNT1,COUNT2,NZBGM1)+(3.d0-2.d0*F)*                         &
                      & CONST(COUNT1,COUNT2,NZBGM1)*CZP(COUNT1,COUNT2,NZBGM1)*TDEEP
                    ELSE
                      A(COUNT1,COUNT2,NZBGM1)=(3.d0-2.d0*F)*CONST(COUNT1,COUNT2,NZBGM1)* &
                            & (-CZM(COUNT1,COUNT2,NZBGM1))
                      B(COUNT1,COUNT2,NZBGM1)=1.d0+(3.d0-2.d0*F)*                        &
                            & CONST(COUNT1,COUNT2,NZBGM1)*CZM(COUNT1,COUNT2,NZBGM1)
                      C(COUNT1,COUNT2,NZBGM1)=0.
                      R(COUNT1,COUNT2,NZBGM1)=F*CONST(COUNT1,COUNT2,NZBGM1)*       &
                      & (CXM(COUNT1,COUNT2,NZBGM1)*V(COUNT1-1,COUNT2,NZBGM1)-      &
                      & (CXM(COUNT1,COUNT2,NZBGM1)+CXP(COUNT1,COUNT2,NZBGM1))*     &
                      & V(COUNT1,COUNT2,NZBGM1)+CXP(COUNT1,COUNT2,NZBGM1)*         &
                      & V(COUNT1+1,COUNT2,NZBGM1)+CYM(COUNT1,COUNT2,NZBGM1)*       &
                      & V(COUNT1,COUNT2-1,NZBGM1)-(CYM(COUNT1,COUNT2,NZBGM1)+      &
                      & CYP(COUNT1,COUNT2,NZBGM1))*V(COUNT1,COUNT2,NZBGM1)+        &
                      & CYP(COUNT1,COUNT2,NZBGM1)*V(COUNT1,COUNT2+1,NZBGM1))+      &
                      & V(COUNT1,COUNT2,NZBGM1)
                    END IF
                    N=NZBGM1-(0)+1
                    L=1
                    DO COUNT3=0,NZBGM1
                      AA(L)=A(COUNT1,COUNT2,COUNT3)
                      BB(L)=B(COUNT1,COUNT2,COUNT3)
                      CC(L)=C(COUNT1,COUNT2,COUNT3)
                      RR(L)=R(COUNT1,COUNT2,COUNT3)
                      L=L+1
                    END DO
                    CALL TRIDI3D (AA,BB,CC,RR,N,X)
                    L=0
                    DO COUNT3=0,NZBGM1
                      T(COUNT1,COUNT2,COUNT3)=X(L+1)
                      L=L+1
                    END DO
                  END DO
                END DO

                IF (SameString(FIXBC,'FALSE')) THEN
                  DO COUNT1=0,NXM1
                    DO COUNT2=0,NYM1
                      T(COUNT1,COUNT2,NZBG)=T(COUNT1,COUNT2,NZBGM1)
                    END DO
                  END DO
                END IF

!***  RESET COUNTER AND VARIABLE SUMS
                COUNTER=0.
                HOSUM=0.
                TDBSUM=0.
                TBSUM=0.
                TISUM=0.
                RSNWSUM=0.
                RSOLVSUM=0.
                DO COUNT1=0,NXM1
                  DO COUNT2=0,NYM1
                    GOFTSUM(COUNT1,COUNT2)=0.
                  END DO
                END DO
                DO COUNT3=0,NZBGM1
                  TGSUM(COUNT3)=0.
                END DO
!***  COMPUTE SURFACE TEMPERATURES FOR EACH TIME STEP REGARDLESS OF
!***  CONVERGENCE
                IF (.NOT.CVG) THEN
                  DO COUNT1=0,IBASE+1
                    DO COUNT2=0,JBASE+1
                      IF (TBAV.GE.T(COUNT1,COUNT2,-NZAG)) THEN
                        HINZ=HIN(5)
                      ELSE
                        HINZ=HIN(4)
                      END IF
                      QC(COUNT1,COUNT2)=(TBAV-T(COUNT1,COUNT2,-NZAG))/         &
                      & (1.d0/HINZ+RCEIL+DZ(-NZAG)/2.d0/TCON(MTYPE(COUNT1,COUNT2,-NZAG)))
                      TC(COUNT1,COUNT2)=TBAV-QC(COUNT1,COUNT2)/HINZ
                    END DO
                  END DO
                  DO COUNT1=0,IBASE-1
                    DO COUNT2=0,JBASE-1
                      TF(COUNT1,COUNT2)=T(COUNT1,COUNT2,KBASE)
                    END DO
                  END DO
                DO COUNT2=0,JBASE+1
                    QRS(COUNT2)=(TBAV-T(IBASE+2,COUNT2,-NZAG+1))/(1.d0/HIN(6)+     &
                    & RSILL+DX(IBASE+2)/TCON(MTYPE(IBASE+2,COUNT2,-NZAG+1)))
                    TRS(COUNT2)=TBAV-QRS(COUNT2)/HIN(6)
                  END DO

                  DO COUNT1=0,IBASE+1
                    QRW(COUNT1)=(TBAV-T(COUNT1,JBASE+2,-NZAG+1))/(1.d0/HIN(6)+     &
                    & RSILL+DY(JBASE+2)/TCON(MTYPE(COUNT1,JBASE+2,-NZAG+1)))
                    TRW(COUNT1)=TBAV-QRW(COUNT1)/HIN(6)
                  END DO

                  DO COUNT1=IBASE,IBASE+1
                    DO COUNT2=0,JBASE+1
                      IF(TBAV.GT.T(COUNT1,COUNT2,-NZAG+2)) THEN
                        HINZ=HIN(4)
                      ELSE
                        HINZ=HIN(5)
                      END IF
                      QSS(COUNT1,COUNT2)=(TBAV-T(COUNT1,COUNT2,-NZAG+2))/(1.d0/HINZ+ &
                      & RSILL+DZ(-NZAG+2)/2.d0/TCON(MTYPE(COUNT1,COUNT2,-NZAG+2)))
                      TSS(COUNT1,COUNT2)=TBAV-QSS(COUNT1,COUNT2)/HINZ
                    END DO
                  END DO

                  DO COUNT2=JBASE,JBASE+1
                    DO COUNT1=0,IBASE-1
                      IF(TBAV.GT.T(COUNT1,COUNT2,-NZAG+2)) THEN
                        HINZ=HIN(4)
                      ELSE
                        HINZ=HIN(5)
                      END IF
                      QSW(COUNT1,COUNT2)=(TBAV-T(COUNT1,COUNT2,-NZAG+2))/(1.d0/HINZ+ &
                      & RSILL+DZ(-NZAG+2)/2.d0/TCON(MTYPE(COUNT1,COUNT2,-NZAG+2)))
                      TSW(COUNT1,COUNT2)=TBAV-QSW(COUNT1,COUNT2)/HINZ
                    END DO
                  END DO

                  DO COUNT2=0,JBASE-1
                    DO COUNT3=-NZAG+2,INT(1+(KBASE-NZAG)/2)
                      QWS(COUNT2,COUNT3)=(TBAV-T(IBASE,COUNT2,COUNT3))/(RINT+1.d0/ &
                          & HIN(6))
                      TWS(COUNT2,COUNT3)=TBAV-QWS(COUNT2,COUNT3)/HIN(6)
                    END DO
                  END DO

                  DO COUNT2=0,JBASE-1
                    DO COUNT3=INT(2+(KBASE-NZAG)/2),KBASE-1
                        QWS(COUNT2,COUNT3)=(TBAV-T(IBASE,COUNT2,COUNT3))/          &
                            & (RINT+1.d0/HIN(6))
                        TWS(COUNT2,COUNT3)=TBAV-QWS(COUNT2,COUNT3)/HIN(6)
                    END DO
                  END DO

                  DO COUNT1=0,IBASE-1
                    DO COUNT3=-NZAG+2,INT(1+(KBASE-NZAG)/2)
                      QWW(COUNT1,COUNT3)=(TBAV-T(COUNT1,JBASE,COUNT3))/          &
                          & (RINT+1.d0/HIN(6))
                      TWW(COUNT1,COUNT3)=TBAV-QWW(COUNT1,COUNT3)/HIN(6)
                    END DO
                  END DO
                  DO COUNT1=0,IBASE-1
                    DO COUNT3=INT(2+(KBASE-NZAG)/2),KBASE-1
                      QWW(COUNT1,COUNT3)=(TBAV-T(COUNT1,JBASE,COUNT3))/          &
                        & (RINT+1.d0/HIN(6))
                      TWW(COUNT1,COUNT3)=TBAV-QWW(COUNT1,COUNT3)/HIN(6)
                    END DO
                  END DO
                END IF

!***  CALCULATE THE FOLLOWING IF PROGRAM HAS CONVERGED OR HAS
!***  REACHED THE MAX. NO. OF YEARS FOR WHICH TO ITERATE
                CONVERGE: IF (CVG.OR.IYR.EQ.IYRS) THEN

!***  SET CELL TEMPERATURES EQUAL TO AMBIENT AIR OR BASEMENT AIR
                  DO COUNT1=0,NXM1
                    DO COUNT2=0,NYM1
                      DO COUNT3=-NZAG,NZBGM1
                        IF (MTYPE(COUNT1,COUNT2,COUNT3).EQ.7.AND.COUNT1.LT.IBASE+2   &
                              & .AND.COUNT2.LT.JBASE+2) THEN
                          T(COUNT1,COUNT2,COUNT3)=TBAV
                        ELSE IF (MTYPE(COUNT1,COUNT2,COUNT3).EQ.7) THEN
                          T(COUNT1,COUNT2,COUNT3)=TDBAV
                        END IF
                      END DO
                    END DO
                  END DO


!***  COMPUTE LOCAL MIN/MAX SURFACE TEMPERATURES AND HEAT FLUXES AND
!***  SUM OF SURFACE TEMPERATURES AND HEAT FLUXES FOR EACH TIME STEP
                  DO COUNT1=0,IBASE+1
                    DO COUNT2=0,JBASE+1
                      IF (TBAV.GE.T(COUNT1,COUNT2,-NZAG)) THEN
                        HINZ=HIN(5)
                      ELSE
                        HINZ=HIN(4)
                      END IF
                      QC(COUNT1,COUNT2)=(TBAV-T(COUNT1,COUNT2,-NZAG))/               &
                       & (1.d0/HINZ+RCEIL+DZ(-NZAG)/2.d0/TCON(MTYPE(COUNT1,COUNT2,-NZAG)))
                      TC(COUNT1,COUNT2)=TBAV-(QC(COUNT1,COUNT2)/HINZ)
                      TCSUM=TCSUM+TC(COUNT1,COUNT2)*DX(COUNT1)*DY(COUNT2)
                      TCMN=MIN(TCMN,TC(COUNT1,COUNT2))
                      TCMX=MAX(TCMX,TC(COUNT1,COUNT2))
                      QCSUM=QCSUM+QC(COUNT1,COUNT2)*DX(COUNT1)*DY(COUNT2)
                      QCMN=MIN(QCMN,QC(COUNT1,COUNT2))
                      QCMX=MAX(QCMX,QC(COUNT1,COUNT2))
                    END DO
                  END DO
                  DO COUNT1=0,IBASE-1
                    DO COUNT2=0,JBASE-1
                      TF(COUNT1,COUNT2)=T(COUNT1,COUNT2,KBASE)
                      TFSUM=TFSUM+TF(COUNT1,COUNT2)*DX(COUNT1)*DY(COUNT2)
                      TFMN=MIN(TFMN,TF(COUNT1,COUNT2))
                      TFMX=MAX(TFMX,TF(COUNT1,COUNT2))
                      HINZ=HIN(2)
                      IF (TBAV.GT.T(COUNT1,COUNT2,KBASE)) THEN
                        HINZ=HIN(4)
                      ELSE
                        HINZ=HIN(5)
                      END IF
                      QF(COUNT1,COUNT2)=HINZ*(TBAV-TF(COUNT1,COUNT2))
                      QFSUM=QFSUM+QF(COUNT1,COUNT2)*DX(COUNT1)*DY(COUNT2)
                      QFMN=MIN(QFMN,QF(COUNT1,COUNT2))
                      QFMX=MAX(QFMX,QF(COUNT1,COUNT2))
                    END DO
                  END DO
                  DO COUNT2=0,JBASE+1
                    QRS(COUNT2)=(TBAV-T(IBASE+2,COUNT2,-NZAG+1))/(1.d0/HIN(6)+RSILL+ &
                    & DX(IBASE+2)/TCON(MTYPE(IBASE+2,COUNT2,-NZAG+1)))
                    TRS(COUNT2)=TBAV-QRS(COUNT2)/HIN(6)
                    TRSUM=TRSUM+TRS(COUNT2)*DY(COUNT2)*DZ(-NZAG+1)
                    TRMN=MIN(TRMN,TRS(COUNT2))
                    TRMX=MAX(TRMX,TRS(COUNT2))
                    QRSUM=QRSUM+QRS(COUNT2)*DY(COUNT2)*DZ(-NZAG+1)
                    QRMN=MIN(QRMN,QRS(COUNT2))
                    QRMX=MAX(QRMX,QRS(COUNT2))
                  END DO
                  DO COUNT1=0,IBASE+1
                    QRW(COUNT1)=(TBAV-T(COUNT1,JBASE+2,-NZAG+1))/(1.d0/HIN(6)+RSILL+ &
                      & DY(JBASE+2)/TCON(MTYPE(COUNT1,JBASE+2,-NZAG+1)))
                    TRW(COUNT1)=TBAV-QRW(COUNT1)/HIN(6)
                    TRSUM=TRSUM+TRW(COUNT1)*DX(COUNT1)*DZ(-NZAG+1)
                    TRMN=MIN(TRMN,TRW(COUNT1))
                    TRMX=MAX(TRMX,TRW(COUNT1))
                    QRSUM=QRSUM+QRW(COUNT1)*DX(COUNT1)*DZ(-NZAG+1)
                    QRMN=MIN(QRMN,QRW(COUNT1))
                    QRMX=MAX(QRMX,QRW(COUNT1))
                  END DO
                  DO COUNT1=IBASE,IBASE+1
                    DO COUNT2=0,JBASE+1
                      IF(TBAV.GT.T(COUNT1,COUNT2,-NZAG+2)) THEN
                        HINZ=HIN(4)
                      ELSE
                        HINZ=HIN(5)
                      END IF
                      QSS(COUNT1,COUNT2)=(TBAV-T(COUNT1,COUNT2,-NZAG+2))/(1.d0/HINZ+   &
                         & RSILL+DZ(-NZAG+2)/2.d0/TCON(MTYPE(COUNT1,COUNT2,-NZAG+2)))
                      TSS(COUNT1,COUNT2)=TBAV-QSS(COUNT1,COUNT2)/HINZ
                      TSSUM=TSSUM+TSS(COUNT1,COUNT2)*DX(COUNT1)*DY(COUNT2)
                      TSMN=MIN(TSMN,TSS(COUNT1,COUNT2))
                      TSMX=MAX(TSMX,TSS(COUNT1,COUNT2))
                      QSSUM=QSSUM+QSS(COUNT1,COUNT2)*DX(COUNT1)*DY(COUNT2)
                      QSMN=MIN(QSMN,QSS(COUNT1,COUNT2))
                      QSMX=MAX(QSMX,QSS(COUNT1,COUNT2))
                    END DO
                  END DO
                  DO COUNT2=JBASE,JBASE+1
                    DO COUNT1=0,IBASE-1
                      IF(TBAV.GT.T(COUNT1,COUNT2,-NZAG+2)) THEN
                        HINZ=HIN(4)
                      ELSE
                        HINZ=HIN(5)
                      END IF
                      QSW(COUNT1,COUNT2)=(TBAV-T(COUNT1,COUNT2,-NZAG+2))/(1.d0/HINZ+ &
                         & RSILL+DZ(-NZAG+2)/2.d0/TCON(MTYPE(COUNT1,COUNT2,-NZAG+2)))
                      TSW(COUNT1,COUNT2)=TBAV-QSW(COUNT1,COUNT2)/HINZ
                      TSSUM=TSSUM+TSW(COUNT1,COUNT2)*DX(COUNT1)*DY(COUNT2)
                      TSMN=MIN(TSMN,TSW(COUNT1,COUNT2))
                      TSMX=MAX(TSMX,TSW(COUNT1,COUNT2))
                      QSSUM=QSSUM+QSW(COUNT1,COUNT2)*DX(COUNT1)*DY(COUNT2)
                      QSMN=MIN(QSMN,QSW(COUNT1,COUNT2))
                      QSMX=MAX(QSMX,QSW(COUNT1,COUNT2))
                    END DO
                  END DO
                  DO COUNT2=0,JBASE-1
                    DO COUNT3=-NZAG+2,INT(1+(KBASE-NZAG)/2)
                      QWS(COUNT2,COUNT3)=(TBAV-T(IBASE,COUNT2,COUNT3))/       &
                          & (RINT+1.d0/HIN(6))
                      TWS(COUNT2,COUNT3)=TBAV-QWS(COUNT2,COUNT3)/HIN(6)
                      TWSUM=TWSUM+TWS(COUNT2,COUNT3)*DY(COUNT2)*DZ(COUNT3)
                      TWMN=MIN(TWMN,TWS(COUNT2,COUNT3))
                      TWMX=MAX(TWMX,TWS(COUNT2,COUNT3))
                      QWSUM=QWSUM+QWS(COUNT2,COUNT3)*DY(COUNT2)*DZ(COUNT3)
                      QWMN=MIN(QWMN,QWS(COUNT2,COUNT3))
                      QWMX=MAX(QWMX,QWS(COUNT2,COUNT3))
                    END DO
                  END DO
                  DO COUNT2=0,JBASE-1
                    DO COUNT3=INT(2+(KBASE-NZAG)/2),KBASE-1
                      QWS(COUNT2,COUNT3)=(TBAV-T(IBASE,COUNT2,COUNT3))/       &
                         & (RINT+1.d0/HIN(6))
                      TWS(COUNT2,COUNT3)=TBAV-QWS(COUNT2,COUNT3)/HIN(6)
                      TWSUM=TWSUM+TWS(COUNT2,COUNT3)*DY(COUNT2)*DZ(COUNT3)
                      TWMN=MIN(TWMN,TWS(COUNT2,COUNT3))
                      TWMX=MAX(TWMX,TWS(COUNT2,COUNT3))
                      QWSUM=QWSUM+QWS(COUNT2,COUNT3)*DY(COUNT2)*DZ(COUNT3)
                      QWMN=MIN(QWMN,QWS(COUNT2,COUNT3))
                      QWMX=MAX(QWMX,QWS(COUNT2,COUNT3))
                    END DO
                  END DO
                  DO COUNT1=0,IBASE-1
                    DO COUNT3=-NZAG+2,INT(1+(KBASE-NZAG)/2)
                      QWW(COUNT1,COUNT3)=(TBAV-T(COUNT1,JBASE,COUNT3))/       &
                         & (RINT+1.d0/HIN(6))
                      TWW(COUNT1,COUNT3)=TBAV-QWW(COUNT1,COUNT3)/HIN(6)
                      TWSUM=TWSUM+TWW(COUNT1,COUNT3)*DX(COUNT1)*DZ(COUNT3)
                      TWMN=MIN(TWMN,TWW(COUNT1,COUNT3))
                      TWMX=MAX(TWMX,TWW(COUNT1,COUNT3))
                      QWSUM=QWSUM+QWW(COUNT1,COUNT3)*DX(COUNT1)*DZ(COUNT3)
                      QWMN=MIN(QWMN,QWW(COUNT1,COUNT3))
                      QWMX=MAX(QWMX,QWW(COUNT1,COUNT3))
                    END DO
                  END DO
                  DO COUNT1=0,IBASE-1
                    DO COUNT3=INT(2+(KBASE-NZAG)/2),KBASE-1
                      QWW(COUNT1,COUNT3)=(TBAV-T(COUNT1,JBASE,COUNT3))/       &
                         & (RINT+1.d0/HIN(6))
                      TWW(COUNT1,COUNT3)=TBAV-QWW(COUNT1,COUNT3)/HIN(6)
                      TWSUM=TWSUM+TWW(COUNT1,COUNT3)*DX(COUNT1)*DZ(COUNT3)
                      TWMN=MIN(TWMN,TWW(COUNT1,COUNT3))
                      TWMX=MAX(TWMX,TWW(COUNT1,COUNT3))
                      QWSUM=QWSUM+QWW(COUNT1,COUNT3)*DX(COUNT1)*DZ(COUNT3)
                      QWMN=MIN(QWMN,QWW(COUNT1,COUNT3))
                      QWMX=MAX(QWMX,QWW(COUNT1,COUNT3))
                    END DO
                  END DO

     !***  COMPUTE OUTSIDE SURFACE TEMPERATURES FOR EnergyPlus

                  IF (.not. SameString(EPlus,'FALSE')) THEN
                    CALL SurfaceTemps(T,DX,DY,DZ,MTYPE,INS,TSurfWallXZ,         &
                    &    TSurfWallYZ,TSurfFloor,TSWallYZIn,TSWallXZIn,          &
                    &    TSFloorIn,TSYZCL,TSXZCL,TSFXCL,TSFYCL,XC,YC,ZC,        &
                    &    TSurfWallYZUpper,TSurfWallYZUpperIn,TSurfWallXZUpper,  &
                    &    TSurfWallXZUpperIn,TSurfWallYZLower,                   &
                    &    TSurfWallYZLowerIn,TSurfWallXZLower,                   &
                    &    TSurfWallXZLowerIn,DAPerim,DACore,DAYZUpperSum,        &
                    &    DAYZLowerSum,DAXZUpperSum,DAXZLowerSum,                &
                    &    TSurfFloorPerim,TSurfFloorPerimIn,TSurfFloorCore,      &
                    &    TSurfFloorCoreIn,TWW,TWS,TF,XDIM,YDIM,ZDIM,DAXZSum,    &
                    &    DAYZSum,DAXYSum)

                    CALL AvgHeatFlux(DACore,DAPerim,XC,YC,ZC,DX,DY,DZ,QWS,      &
                    &    QWW,QF,XDIM,YDIM,ZDIM,FloorHeatFlux,CoreHeatFlux,      &
                    &    PerimHeatFlux,XZWallHeatFlux,YZWallHeatFlux,           &
                    &    UpperXZWallFlux,UpperYZWallFlux,LowerXZWallFlux,       &
                    & LowerYZWallFlux,DAYZUpperSum,DAYZLowerSum,DAXZUpperSum,   &
                    & DAXZLowerSum,DAXZSum,DAYZSum,DAXYSum)

!***  EnergyPlus OUTPUT STAGE
                    CALL EPlusOutput(IHR,IDAY,TSurfWallXZ,TSurfWallYZ,TSurfFloor,        &
                    & TSWallYZIn,TSWallXZIn,TSFloorIn,TSYZCL,TSXZCL,TSFXCL,     &
                    & TSFYCL,TSurfWallYZUpper,TSurfWallYZUpperIn,               &
                    & TSurfWallXZUpper,TSurfWallXZUpperIn,TSurfWallYZLower,     &
                    & TSurfWallYZLowerIn,TSurfWallXZLower,TSurfWallXZLowerIn,   &
                    & TSurfFloorPerim,TSurfFloorPerimIn,TSurfFloorCore,         &
                    & TSurfFloorCoreIn,FloorHeatFlux,CoreHeatFlux,              &
                    & PerimHeatFlux,XZWallHeatFlux,YZWallHeatFlux,              &
                    & UpperXZWallFlux,UpperYZWallFlux,LowerXZWallFlux,          &
                    & LowerYZWallFlux,TB(IHR),TCON)
             END IF

!***  CALCULATE AND OUTPUT TOTAL HEAT LOSS/GAIN FROM/TO HOUSE AND
!***  ITS EFFECT ON HOUSE FOR EACH TIME STEP
!***  QHOUSE>0: HEAT GAIN TO HOUSE, QHOUSE<0: HEAT LOSS FROM HOUSE
                  IF(SameString(EPlus,'FALSE')) THEN
                    QHOUSE=0.
                    DO COUNT1=0,IBASE+1
                      DO COUNT2=0,JBASE+1
                        IF (TIAV.GT.T(COUNT1,COUNT2,-NZAG)) THEN
                          HINZH=HIN(4)
                        ELSE
                          HINZH=HIN(5)
                        END IF
                        QHOUSE=QHOUSE+TSTEP*((T(COUNT1,COUNT2,-NZAG)-TIAV)/     &
                        & (1.d0/HINZH+DZ(-NZAG)/2.d0/                              &
                        & TCON(MTYPE(COUNT1,COUNT2,-NZAG))))*DX(COUNT1)*DY(COUNT2)
                      END DO
                    END DO
                    IF (SameString(COND,'FALSE').AND.TDBAV.LE.((TDBH+13.67d0)/2.d0).AND.           &
                    &    QHOUSE.LT.0.)THEN
                      EFFECT=-1
                    ELSE IF (SameString(COND,'FALSE').AND.TDBAV.LE.((TDBH+13.67d0)/2.d0).AND.      &
                    & QHOUSE.GT.0.) THEN
                      EFFECT=1
                    ELSE IF (SameString(COND,'FALSE').AND.TDBAV.GE.TDBC.AND.QHOUSE.LT.0.) THEN
                      EFFECT=1
                    ELSE IF (SameString(COND,'FALSE').AND.TDBAV.GE.TDBC.AND.QHOUSE.GT.0.) THEN
                      EFFECT=-1
                    ELSE IF (.not. SameString(COND,'FALSE').AND.TDBAV.GE.TDBC.AND.TBAV.LT.TIN(2).AND. &
                    & QHOUSE.LT.0.) THEN
                      EFFECT=1
                    ELSE
                      EFFECT=0
                    END IF
                    CALL HouseLoadOutput(QHOUSE,EFFECT)

!***  COMPUTE DAILY AVERAGE OUTDOOR DRY-BULB TEMPERATURE AND DAILY
!***  AVERAGE BASEMENT TEMPERATURE
                    DTDBA=DTDBA+TDBAV*TSTEP/24.
                    DTBA=DTBA+TBAV*TSTEP/24.

!***  COMPUTE DAILY AVERAGE, SPATIALLY-AVERAGED SURFACE
!***  TEMPERATURES AND HEAT FLUXES
                    DTCA=DTCA+(TCSUM*TSTEP)/(ACEIL/4.)/24.
                    DQCA=DQCA+(QCSUM*TSTEP)/(ACEIL/4.)/24.
                    DTFA=DTFA+(TFSUM*TSTEP)/(AFLOOR/4.)/24.
                    DQFA=DQFA+(QFSUM*TSTEP)/(AFLOOR/4.)/24.
                    DTRA=DTRA+(TRSUM*TSTEP)/(ARIM/4.)/24.
                    DQRA=DQRA+(QRSUM*TSTEP)/(ARIM/4.)/24.
                    DTSA=DTSA+(TSSUM*TSTEP)/(ASILL/4.)/24.
                    DQSA=DQSA+(QSSUM*TSTEP)/(ASILL/4.)/24.
                    DTWA=DTWA+(TWSUM*TSTEP)/(AWALL/4.)/24.
                    DQWA=DQWA+(QWSUM*TSTEP)/(AWALL/4.)/24.

!***  COMPUTE DAILY AVERAGED CELL SURFACE TEMPERATURES AND HEAT FLUXES
!***  FOR 21ST DAY OF EACH MONTH
                    DAY21: IF (IDAY.EQ.NFDM(IMON)+20) THEN
                      DO COUNT1=0,IBASE+1
                        DO COUNT2=0,JBASE+1
                          DTC21(COUNT1,COUNT2)=DTC21(COUNT1,COUNT2)+            &
                              & TC(COUNT1,COUNT2)*TSTEP/24.
                          DQC21(COUNT1,COUNT2)=DQC21(COUNT1,COUNT2)+            &
                              & QC(COUNT1,COUNT2)*TSTEP/24.
                        END DO
                      END DO
                      DO COUNT1=0,IBASE-1
                        DO COUNT2=0,JBASE-1
                          DTF21(COUNT1,COUNT2)=DTF21(COUNT1,COUNT2)+            &
                              & TF(COUNT1,COUNT2)*TSTEP/24.
                          DQF21(COUNT1,COUNT2)=DQF21(COUNT1,COUNT2)+            &
                              & QF(COUNT1,COUNT2)*TSTEP/24.
                        END DO
                      END DO
                      DO COUNT2=0,JBASE+1
                        DTRS21(COUNT2)=DTRS21(COUNT2)+TRS(COUNT2)*TSTEP/24.
                        DQRS21(COUNT2)=DQRS21(COUNT2)+QRS(COUNT2)*TSTEP/24.
                      END DO
                      DO COUNT1=0,IBASE+1
                        DTRW21(COUNT1)=DTRW21(COUNT1)+TRW(COUNT1)*TSTEP/24.
                        DQRW21(COUNT1)=DQRW21(COUNT1)+QRW(COUNT1)*TSTEP/24.
                      END DO
                      DO COUNT1=IBASE,IBASE+1
                        DO COUNT2=0,JBASE+1
                          DTSS21(COUNT1,COUNT2)=DTSS21(COUNT1,COUNT2)+          &
                              & TSS(COUNT1,COUNT2)*TSTEP/24.
                          DQSS21(COUNT1,COUNT2)=DQSS21(COUNT1,COUNT2)+          &
                              & QSS(COUNT1,COUNT2)*TSTEP/24.
                        END DO
                      END DO
                      DO COUNT2=JBASE,JBASE+1
                        DO COUNT1=0,IBASE-1
                          DTSW21(COUNT1,COUNT2)=DTSW21(COUNT1,COUNT2)+          &
                              & TSW(COUNT1,COUNT2)*TSTEP/24.
                          DQSW21(COUNT1,COUNT2)=DQSW21(COUNT1,COUNT2)+          &
                              & QSW(COUNT1,COUNT2)*TSTEP/24.
                        END DO
                      END DO
                      DO COUNT2=0,JBASE-1
                        DO COUNT3=-NZAG+2,KBASE-1
                          DTWS21(COUNT2,COUNT3)=DTWS21(COUNT2,COUNT3)+         &
                            & TWS(COUNT2,COUNT3)*TSTEP/24.
                          DQWS21(COUNT2,COUNT3)=DQWS21(COUNT2,COUNT3)+         &
                            & QWS(COUNT2,COUNT3)*TSTEP/24.
                        END DO
                      END DO
                      DO COUNT1=0,IBASE-1
                        DO COUNT3=-NZAG+2,KBASE-1
                          DTWW21(COUNT1,COUNT3)=DTWW21(COUNT1,COUNT3)+         &
                             & TWW(COUNT1,COUNT3)*TSTEP/24.
                          DQWW21(COUNT1,COUNT3)=DQWW21(COUNT1,COUNT3)+         &
                             & QWW(COUNT1,COUNT3)*TSTEP/24.
                        END DO
                      END DO
                      DO COUNT1=0,NXM1
                        DO COUNT3=-NZAG,NZBGM1
                          TV1(COUNT1,COUNT3)=TV1(COUNT1,COUNT3)+               &
                             & T(COUNT1,0,COUNT3)*TSTEP/24.
                          TV2(COUNT1,COUNT3)=TV2(COUNT1,COUNT3)+               &
                             & T(COUNT1,INT(JBASE/2),COUNT3)*TSTEP/24.
                          TV3(COUNT1,COUNT3)=TV3(COUNT1,COUNT3)+               &
                             & T(COUNT1,JBASE,COUNT3)*TSTEP/24.
                        END DO
                      END DO
                    END IF DAY21

!***  OUTPUT DAILY AVERAGE SURFACE TEMPERATURES ON THE 21ST DAY OF
!***  JANUARY AT EVERY TIME STEP
                    JAN21: IF (IDAY.EQ.21) THEN
                      CALL Jan21Output(IHR,TC,TF,TRS,TRW,TSS,TSW,TWS,TWW,       &
                      &    XDIM,YDIM,ZDIM,XC,YC,ZC)
                    END IF JAN21

!***  COMPUTE DAILY NET SURFACE HEAT LOSSES (W-H)
                    DQCSUM=DQCSUM+QCSUM*TSTEP
                    DQFSUM=DQFSUM+QFSUM*TSTEP
                    DQRSUM=DQRSUM+QRSUM*TSTEP
                    DQSSUM=DQSSUM+QSSUM*TSTEP
                    DQWSUM=DQWSUM+QWSUM*TSTEP

!***  COMPUTE AND OUTPUT HEATING/COOLING LOAD (+) FOR EACH TIME STEP
!***  (W-H) AND SYSTEM OPERATING CONDITION
                    IF (.not. SameString(COND,'FALSE').AND.TDBAV.LE.((TDBH+13.67)/2.)) THEN
                      HLOAD=(QFSUM+QRSUM+QSSUM+QWSUM)*TSTEP
                      CONDITION=1
                    ELSE IF (SameString(COND,'FALSE').AND.TDBAV.LE.((TDBH+13.67)/2.)) THEN
                      HLOAD=-QCSUM*TSTEP
                      CONDITION=0
                    ELSE IF (.not. SameString(COND,'FALSE').AND.TDBAV.GE.TDBC) THEN
                      IF (TBAV.GE.TIN(2)) THEN
                        CLOAD=(-QFSUM-QRSUM-QSSUM-QWSUM)*TSTEP
                        CONDITION=-1
                      ELSE
                        CLOAD=QCSUM*TSTEP
                        CONDITION=0
                      END IF
                    ELSE IF (SameString(COND,'FALSE').AND.TDBAV.GE.TDBC) THEN
                      CLOAD=QCSUM*TSTEP
                      CONDITION=0
                    ELSE
                      HLOAD=0.
                      CLOAD=0.
                      CONDITION=0
                    END IF
                    YHLOAD=YHLOAD+HLOAD
                    YCLOAD=YCLOAD+CLOAD
                    CALL OutputLoadS(HLOAD,CLOAD,CONDITION)
                  END IF !*** END THE EPlus CONDITIONAL BLOCK.
                END IF CONVERGE
!***  COUNTER
              END IF COUNT

!***  HOURS
           END DO HOURS

!***  OUTPUT DAILY LOCAL MIN/MAX TEMPERATURES AND HEAT FLUXES,
!***  DAILY AVERAGE, SPATIALLY-AVERAGE TEMPERATURES AND HEAT FLUXES,
!***  AND DAILY AVERAGE OUTDOOR AND BASEMENT AIR TEMPERATURES.
            CVGD: IF (CVG.OR.IYR.EQ.IYRS) THEN
              IF (SameString(EPlus,'FALSE')) THEN
                CALL MainOutput(TCMN,TCMX,TFMN,TFMX,TRMN,TRMX,TSMN,TSMX,     &
                & TWMN,TWMX,QCMN,QCMX,QFMN,QFMX,QRMN,QRMX,QSMN,QSMX,QWMN,QWMX,    &
                & DTCA,DTFA,DTRA,DTSA,DTWA,DQCA,DQFA,DQRA,DQSA,DQWA,DTDBA,DTBA)

!***  OUTPUT DAILY AVERAGE SURFACE TEMPERATURES AND HEAT FLUXES
!***  ON THE 21ST DAY OF EACH MONTH
                IF (IDAY.EQ.NFDM(IMON)+20.) THEN
                  CALL Day21Output(IMON,IBASE,JBASE,KBASE,NZAG,DTRW21,DTRS21,DTC21, &
                  & DTWW21,DTWS21,DTSW21,DTSS21,DQWW21,DQWS21,DQSW21,DQSS21,DQRW21, &
                  & DQRS21,DQF21,DQC21,DTF21,TV1,TV2,TV3,NXM1,NZBGM1,XDIM,YDIM,     &
                  & ZDIM,XC,YC,ZC)
                END IF
!***  OUTPUT DAILY NET SURFACE HEAT LOSSES
                  CALL DailyOutput(DQCSUM,DQFSUM,DQRSUM,DQSSUM,DQWSUM)

!***  SUM THE DAILY NET SURFACE HEAT LOSSES (W-H)
                YQCSUM=YQCSUM+DQCSUM
                YQFSUM=YQFSUM+DQFSUM
                YQRSUM=YQRSUM+DQRSUM
                YQSSUM=YQSSUM+DQSSUM
                YQWSUM=YQWSUM+DQWSUM
                YQBSUM=YQCSUM+YQFSUM+YQRSUM+YQSSUM+YQWSUM
              END IF
            END IF CVGD
!***  DAYS
          END DO DAYS  !  end of main day loop

!***  OUTPUT THE YEARLY TOTAL HEATING AND COOLING LOAD (W-H)
          IF (CVG.OR.IYR.EQ.IYRS) THEN
            IF (SameString(EPlus,'FALSE')) THEN
              CALL YearlyOutput(YHLOAD,YCLOAD,YQCSUM,YQFSUM,YQRSUM,YQSSUM,   &
              & YQWSUM,YQBSUM)
            END IF
          END IF

!***  IF 3-D SOLUTION HAS CONVERGED, WRITE TEMPERATURES TO FILE TO
!***  SAVE TO INITIALIZE DOMAIN
          IF (CVG.AND..not. SameString(TWRITE,'FALSE')) THEN
            Twriting=GetNewUnitNumber()
            OPEN (Twriting,FILE='TINIT.TXT')
            CALL InitializeTemps(NXM1,NZBGM1,NYM1,T)
            REWIND(Twriting)
          END IF

!***  TEST FOR CONVERGENCE AT END OF YEAR
          IF (.NOT.CVG) THEN
            CVG=.TRUE.
            CVGDeltaT=-99999.
            DO COUNT1=0,IBASE+2
              DO COUNT2=0,JBASE+2
                IF (ABS(T(COUNT1,COUNT2,-NZAG)-TCVG(COUNT1,COUNT2,-NZAG)).GE.0.1) THEN
                  CVG=.FALSE.
                END IF
                CVGDeltaT=MAX(CVGDeltaT,ABS(T(COUNT1,COUNT2,-NZAG)-TCVG(COUNT1,COUNT2,-NZAG)))
                TCVG(COUNT1,COUNT2,-NZAG)=T(COUNT1,COUNT2,-NZAG)
               END DO
            END DO
            DO COUNT2=0,JBASE+2
              IF (ABS(T(IBASE+2,COUNT2,-NZAG+1)-TCVG(IBASE+2,COUNT2,-NZAG+1)).GE.0.1) THEN
                CVG=.FALSE.
              END IF
              CVGDeltaT=MAX(CVGDeltaT,ABS(T(IBASE+2,COUNT2,-NZAG+1)-TCVG(IBASE+2,COUNT2,-NZAG+1)))
              TCVG(IBASE+2,COUNT2,-NZAG+1)=T(IBASE+2,COUNT2,-NZAG+1)
            END DO
            DO COUNT1=0,IBASE+1
              IF (ABS(T(COUNT1,JBASE+2,-NZAG+1)-TCVG(COUNT1,JBASE+2,-NZAG+1)).GE.0.1) THEN
                CVG=.FALSE.
              END IF
              CVGDeltaT=MAX(CVGDeltaT,ABS(T(COUNT1,JBASE+2,-NZAG+1)-TCVG(COUNT1,JBASE+2,-NZAG+1)))
              TCVG(COUNT1,JBASE+2,-NZAG+1)=T(COUNT1,JBASE+2,-NZAG+1)
            END DO
            DO COUNT3=-NZAG+2,-1
              DO COUNT1=IBASE,IBASE+2
                DO COUNT2=0,JBASE+2
                  IF (ABS(T(COUNT1,COUNT2,COUNT3)-TCVG(COUNT1,COUNT2,COUNT3)).GE.0.1) THEN
                    CVG=.FALSE.
                  END IF
                  CVGDeltaT=MAX(CVGDeltaT,ABS(T(COUNT1,COUNT2,COUNT3)-TCVG(COUNT1,COUNT2,COUNT3)))
                  TCVG(COUNT1,COUNT2,COUNT3)=T(COUNT1,COUNT2,COUNT3)
                END DO
              END DO

              DO COUNT1=0,IBASE-1
                DO COUNT2=JBASE,JBASE+2
                  IF (ABS(T(COUNT1,COUNT2,COUNT3)-TCVG(COUNT1,COUNT2,COUNT3)).GE.0.1) THEN
                    CVG=.FALSE.
                  END IF
                  TCVG(COUNT1,COUNT2,COUNT3)=T(COUNT1,COUNT2,COUNT3)
                  CVGDeltaT=MAX(CVGDeltaT,ABS(T(COUNT1,COUNT2,COUNT3)-TCVG(COUNT1,COUNT2,COUNT3)))
                END DO
              END DO
            END DO

            DO COUNT1=0,NXM1
              DO COUNT2=JBASE,NYM1
                DO COUNT3=0,NZBGM1
                  IF (ABS(T(COUNT1,COUNT2,COUNT3)-TCVG(COUNT1,COUNT2,COUNT3)).GE.0.1) THEN
                    CVG=.FALSE.
                  ENDIF
                  TCVG(COUNT1,COUNT2,COUNT3)=T(COUNT1,COUNT2,COUNT3)
                  CVGDeltaT=MAX(CVGDeltaT,ABS(T(COUNT1,COUNT2,COUNT3)-TCVG(COUNT1,COUNT2,COUNT3)))
                END DO
              END DO
            END DO

            DO COUNT1=IBASE,NXM1
              DO COUNT2=0,JBASE-1
                DO COUNT3=0,NZBGM1
                  IF (ABS(T(COUNT1,COUNT2,COUNT3)-TCVG(COUNT1,COUNT2,COUNT3)).GE.0.1) THEN
                    CVG=.FALSE.
                  ENDIF
                  TCVG(COUNT1,COUNT2,COUNT3)=T(COUNT1,COUNT2,COUNT3)
                  CVGDeltaT=MAX(CVGDeltaT,ABS(T(COUNT1,COUNT2,COUNT3)-TCVG(COUNT1,COUNT2,COUNT3)))
                END DO
              END DO
            END DO

            DO COUNT1=0,IBASE-1
              DO COUNT2=0,JBASE-1
                DO COUNT3=KBASE,NZBGM1
                  IF (ABS(T(COUNT1,COUNT2,COUNT3)-TCVG(COUNT1,COUNT2,COUNT3)).GE.0.1) THEN
                    CVG=.FALSE.
                  ENDIF
                  CVGDeltaT=MAX(CVGDeltaT,ABS(T(COUNT1,COUNT2,COUNT3)-TCVG(COUNT1,COUNT2,COUNT3)))
                  TCVG(COUNT1,COUNT2,COUNT3)=T(COUNT1,COUNT2,COUNT3)
                END DO
              END DO
            END DO
            IF (.NOT.CVG) QUIT=.FALSE.
            PRINT*,'3-D Solution Has Not Converged, Delta T>0.1'
            PRINT *,'Max DeltaT=',trim(roundsigdigits(CVGDeltaT,3))
            WRITE (Debugoutfile,*) '3-D Solution Has Not Converged, Delta T>0.1'
            WRITE (DebugOutFile,*) 'Max DeltaT=',trim(roundsigdigits(CVGDeltaT,3))
            WRITE (Debugoutfile,*) 'CVG = ',CVG,' QUIT = ',QUIT
            CALL CPU_TIME(Time_Finish)
            Elapsed_Time=Time_Finish-Time_Start
            PRINT *,'Elapsed time: ',trim(roundsigdigits(Elapsed_Time,2)),' seconds'
            WRITE(DebugOutFile,*) 'Elapsed time: ',trim(roundsigdigits(Elapsed_Time,2)),' seconds'
          ELSE
            PRINT*,'3-D Solution Has Converged'
            CALL CPU_TIME(Time_Finish)
            Elapsed_Time=Time_Finish-Time_Start
            PRINT *,'Elapsed time: ',trim(roundsigdigits(Elapsed_Time,2)),' seconds'
            WRITE (Debugoutfile,*) '3-D Solution Has Converged'
            WRITE(DebugOutFile,*) 'Elapsed time: ',trim(roundsigdigits(Elapsed_Time,2)),' seconds'
          END IF

!***  DETERMINE WHETHER EXECUTION SHOULD CONTINUE.  IF NOT,
!***  CLOSE ALL OPEN FILES AND STOP.
          IF (QUIT.OR.IYR.EQ.IYRS) THEN
            PRINT*, 'IYR=',IYR, '  Program Terminated'
            WRITE (Debugoutfile,*) 'IYR=',IYR, '  Program Terminated'
!           WRITE (Debugoutfile,*) 'CVG = ',CVG,'QUIT = ',QUIT
!            WRITE (YZWallSplit,*) 'Upper band YZ wall area: ',DAYZUpperSum*4.
!            WRITE (YZWallSplit,*) 'Lower band YZ wall area: ',DAYZLowerSum*4.
!            WRITE (XZWallSplit,*) 'Upper band XZ wall area: ',DAXZUpperSum*4.
!            WRITE (XZWallSplit,*) 'Lower band XZ wall area: ',DAXZLowerSum*4.
 !           WRITE (FloorSplit,*) 'Perimeter zone floor area: ',DAPerim*4.
 !           WRITE (FloorSplit,*) 'Core zone floor area: ',DACore*4.

!***  REWIND DATA FILES
!!            REWIND(Weather)
!            REWIND(Weather2)
            REWIND(GroundTemp)
            REWIND(SolarFile)
          END IF

!***  YEARS
          IF (QUIT) EXIT YEARS
        END DO YEARS
      DEALLOCATE(DQC21)
      DEALLOCATE(DQF21)
      DEALLOCATE(DQRS21)
      DEALLOCATE(DQRW21)
      DEALLOCATE(DQSS21)
      DEALLOCATE(DQSW21)
      DEALLOCATE(DQWS21)
      DEALLOCATE(DQWW21)
      DEALLOCATE(DTC21)
      DEALLOCATE(DTF21)
      DEALLOCATE(DTRS21)
      DEALLOCATE(DTRW21)
      DEALLOCATE(DTSS21)
      DEALLOCATE(DTSW21)
      DEALLOCATE(DTWW21)
      DEALLOCATE(DTWS21)
      DEALLOCATE(QC)
      DEALLOCATE(QF)
      DEALLOCATE(QEXT)
      DEALLOCATE(QRS)
      DEALLOCATE(QRW)
      DEALLOCATE(QSS)
      DEALLOCATE(QSW)
      DEALLOCATE(QWS)
      DEALLOCATE(QWW)
      DEALLOCATE(TC)
      DEALLOCATE(TEXT)
      DEALLOCATE(TF)
      DEALLOCATE(TRS)
      DEALLOCATE(TRW)
      DEALLOCATE(TSS)
      DEALLOCATE(TSW)
      DEALLOCATE(TWS)
      DEALLOCATE(TWW)
      DEALLOCATE(UEXT)
      DEALLOCATE(VEXT)

      RETURN
END SUBROUTINE BasementSimulator !end subroutine basementsimulator

!**************************   CONNECT I/O FILES  ****************************************
SUBROUTINE ConnectIO(RUNID)
! SUBROUTINE INFORMATION:
     !       AUTHOR         Edward Clements
     !       DATE WRITTEN   December 8, 1999
     !       MODIFIED       na
     !       RE-ENGINEERED  na
     !      VERSION NUMBER 1.0
     !
     ! PURPOSE OF THIS MODULE:
     ! This subroutine will connect all input and output files (other than the file that
     ! contains the RUNID data

     ! METHODOLOGY EMPLOYED:
     ! Standard EnergyPlus subroutine formatting for subroutines

     ! REFERENCES: BASE 3D Version 1.0, Cynthia A. Cogil. 23 July, 1996.

     ! OTHER NOTES: none

USE BasementSimData
IMPLICIT NONE
     CHARACTER *3 RUNID            ! Run identifier for this run                   []
     CHARACTER *5 TINIT            ! Name of the teperature initialization file    []
     CHARACTER *5 OLDTG            ! True if there is an old ground temp file      []
     CHARACTER *5 TWRITE           ! True indicates that cell temperatures will    []
                                   ! be written to a file to be saved for domain                                      ! initialization
     CHARACTER *5 TREAD            ! True indicates that ground temperatures will  []
                                   ! be read from a file
     CHARACTER *6 TGNAM
     OLDTG =BCS%OLDTG
     TWRITE=BCS%TWRITE
     TINIT =BCS%TINIT
     TREAD =BCS%TREAD
     TGNAM =BCS%TGNAM
!open (unit=99,file='testing.txt')
!*** Connect Debugging file
    Debugoutfile=GetNewUnitNumber()
    OPEN (UNIT=Debugoutfile,FILE=RUNID//'DEBUGOUT.TXT')
!!*** Connect the BLAST ASCII weather file
!!     Weather=GetNewUnitNumber()
!!     OPEN (UNIT=Weather,FILE=WeatherFile//'.txt',STATUS='OLD')
!write (99,*) 'passed weather'
!*** Connect the TMY2 weather file for snow data
!     Weather2=GetNewUnitNumber()
!     OPEN (UNIT=Weather2, FILE=SNOW//'.TM2',STATUS='OLD')

!**** Dry bulb air temperature echo
!       YTDBFile=GetNewUnitNumber()
!       OPEN (UNIT=YTDBFile, FILE=RUNID//'YTDB.TXT')!,STATUS='NEW')
!*** Connect I/O files
     InputEcho=GetNewUnitNumber()
     OPEN (UNIT=InputEcho,FILE=RUNID//'INPUT.TXT')!,STATUS='NEW')
     IF (SameString(OLDTG,'FALSE')) THEN
       GroundTemp=GetNewUnitNumber()
       OPEN (UNIT=GroundTemp, FILE=TGNAM//'.TXT')!,STATUS='REPLACE')
       SolarFile=GetNewUnitNumber()
       OPEN (UNIT=SolarFile,FILE=RUNID//'Solar.TXT')!,STATUS='NEW')
       AvgTG=GetNewUnitNumber()
       OPEN (UNIT=AvgTG, FILE=RUNID//'TGMAVG.TXT')!,STATUS='NEW')
      ELSE
       GroundTemp=GetNewUnitNumber()
       OPEN (UNIT=GroundTemp, FILE=TGNAM//'.TXT',STATUS='REPLACE')
 !      SolarFile=GetNewUnitNumber()
 !      OPEN (UNIT=SolarFile,FILE=RUNID//'Solar.TXT',STATUS='OLD')
     END IF
!     InitT=GetNewUnitNumber()
!     IF (TWRITE.EQ.'FALSE')   OPEN (UNIT=InitT, FILE=TINIT//'.TXT')!,STATUS='NEW')
   !  IF (TREAD.NE.'FALSE') THEN
   !    !InitT=GetNewUnitNumber()
   !    OPEN (UNIT=75, FILE=TINIT//'.TXT',STATUS='OLD')
   !  END IF

!*** Output files for EnergyPlus
!
!*** EPMonthly  outputs all monthly data
     EPMonthly = GetNewUnitNumber()
     OPEN (Unit = EPMonthly,File = 'MonthlyResults.csv')
!     EPObjects = GetNewUnitNumber()
!     OPEN (Unit = EPObjects,File = 'EPObjects.TXT')
!*** XZWall Single surface temperature
 !    XZWallTs   =GetNewUnitNumber()
 !    OPEN (UNIT=XZWallTs,FILE=RUNID//'XZWallSurfTemp.TXT')!,STATUS='NEW')

!*** YZ Wall Single surface temperature
 !    YZWallTs   =GetNewUnitNumber()
 !    OPEN (UNIT=YZWallTs,FILE=RUNID//'YZWALLSurfTemp.TXT')!,STATUS='NEW')

!*** Floor single surface temperature
 !    FloorTs    =GetNewUnitNumber()
 !    OPEN (UNIT=FloorTs,FILE=RUNID//'FloorSurfTemp.TXT')!,STATUS='NEW')

!*** Floor and wall centerline temperatures
 !    Centerline=GetNewUnitNumber()
 !    OPEN (UNIT=Centerline,FILE=RUNID//'CenterLineTemp.TXT')!,STATUS='NEW')

!*** YZ Wall split surface temperature
!     YZWallSplit=GetNewUnitNumber()
!     OPEN (UNIT=YZWallSplit,FILE=RUNID//'YZWall Split Ts.TXT')!,STATUS='NEW')

!*** XZ Wall split surface temperature
 !    XZWallSplit =GetNewUnitNumber()
 !    OPEN (UNIT=XZWallSplit,FILE=RUNID//'XZWall Split Ts.TXT')!,STATUS='NEW')

!*** Floor split surface temperature
 !    FloorSplit =GetNewUnitNumber()
 !    OPEN (UNIT=FloorSplit,FILE=RUNID//'Floor Split Ts.TXT')!,STATUS='NEW')

!**************** Non EnergyPlus output files
     IF(SameString(EPlus,'FALSE')) THEN
!**** Whole building q" output
       QHouseFile=GetNewUnitNumber()
       OPEN (UNIT=QHouseFile, FILE=RUNID//'QHOUSE.TXT')!,STATUS='NEW')
!**** Daily min/max average data echo
       DOUT=GetNewUnitNumber()
       OPEN (UNIT=DOUT, FILE=RUNID//'DOUT.TXT')!,STATUS='NEW')
!**** Daily fluxes
       DYFLX=GetNewUnitNumber()
       OPEN (UNIT=DYFLX, FILE=RUNID//'DYFLX.TXT')!,STATUS='NEW')
!**** Conditioning system load calculation
       LoadFile=GetNewUnitNumber()
       OPEN (UNIT=LoadFile, FILE=RUNID//'LOAD.TXT')!,STATUS='NEW')
!**** January 21st hourly data files
       Ceil121=GetNewUnitNumber()
       OPEN (UNIT=Ceil121,FILE=RUNID//'JAN21CEIL.TXT')!,STATUS='NEW')
       Flor121=GetNewUnitNumber()
       OPEN (UNIT=Flor121,FILE=RUNID//'JAN21FLOR.TXT')!,STATUS='NEW')
       RMJS121=GetNewUnitNumber()
       OPEN (UNIT=RMJS121,FILE=RUNID//'JAN21RMJS.TXT')!,STATUS='NEW')
       RMJW121=GetNewUnitNumber()
       OPEN (UNIT=RMJW121,FILE=RUNID//'JAN21RMJW.TXT')!,STATUS='NEW')
       SILS121=GetNewUnitNumber()
       OPEN (UNIT=SILS121,FILE=RUNID//'JAN21SILS.TXT')!,STATUS='NEW')
       SILW121=GetNewUnitNumber()
       OPEN (UNIT=SILW121,FILE=RUNID//'JAN21SILW.TXT')!,STATUS='NEW')
       WALS121=GetNewUnitNumber()
       OPEN (UNIT=WALS121,FILE=RUNID//'JAN21WALS.TXT')!,STATUS='NEW')
       WALW121=GetNewUnitNumber()
       OPEN (UNIT=WALW121,FILE=RUNID//'JAN21WALW.TXT')!,STATUS='NEW')

!**** Averaged data on the 21st of each month
       CeilD21=GetNewUnitNumber()
       OPEN (UNIT=CeilD21,FILE=RUNID//'DAY21CEIL.TXT')!,STATUS='NEW')
       FlorD21=GetNewUnitNumber()
       OPEN (UNIT=FlorD21,FILE=RUNID//'DAY21FLOR.TXT')!,STATUS='NEW')
       RMJSD21=GetNewUnitNumber()
       OPEN (UNIT=RMJSD21,FILE=RUNID//'DAY21RMJS.TXT')!,STATUS='NEW')
       RMJWD21=GetNewUnitNumber()
       OPEN (UNIT=RMJWD21,FILE=RUNID//'DAY21RMJW.TXT')!,STATUS='NEW')
       SILSD21=GetNewUnitNumber()
       OPEN (UNIT=SILSD21,FILE=RUNID//'DAY21SILS.TXT')!,STATUS='NEW')
       SILWD21=GetNewUnitNumber()
       OPEN (UNIT=SILWD21,FILE=RUNID//'DAY21SILW.TXT')!,STATUS='NEW')
       WALSD21=GetNewUnitNumber()
       OPEN (UNIT=WALSD21,FILE=RUNID//'DAY21WALS.TXT')!,STATUS='NEW')
       WALWD21=GetNewUnitNumber()
       OPEN (UNIT=WALWD21,FILE=RUNID//'DAY21WALW.TXT')!,STATUS='NEW')

!**** XZ plane temperatures
       XZYZero=GetNewUnitNumber()
       OPEN (UNIT=XZYZero,FILE=RUNID//'XZYZERO.TXT')!,STATUS='NEW')
       XZYHalf=GetNewUnitNumber()
       OPEN (UNIT=XZYHalf,FILE=RUNID//'XZYHALF.TXT')!,STATUS='NEW')
       XZYFull=GetNewUnitNumber()
       OPEN (UNIT=XZYFull,FILE=RUNID//'XZYFULL.TXT')!,STATUS='NEW')
     END IF
     RETURN
END SUBROUTINE ConnectIO

!*********************************  Skip Header  ****************************************
!SUBROUTINE SkipHeader
!IMPLICIT NONE
!!*** SUBROUTINE INFORMATION:
!     !***       AUTHOR         Edward D. Clements
!     !***       DATE WRITTEN   October 12, 1999
!     !***       MODIFIED       na
!     !***       RE-ENGINEERED  na
!
!     !*** PURPOSE OF THIS SUBROUTINE:
!     !*** This subroutine will skip the header written into BLAST weather Files.
!
!     !*** METHODOLOGY EMPLOYED:
!     !*** Standard EnergyPlus "manager" methodology.
!!
!     !*** REFERENCES:
!     !*** na
!
!!*** Skip the header row at the beginning of the weather file
!     READ (Weather,100)
!100  FORMAT (/)
!     RETURN
!END SUBROUTINE SkipHeader

!***********************  Calculate Finite Difference Matrix Coefficients  **************
SUBROUTINE FDMCoefficients(NXM1,NYM1,NZBGM1,INSFULL,REXT,DX,DY,DZ,DXP,DYP,DZP,MTYPE,    &
&          CXM,CYM,CZM,CXP,CYP,CZP,ZC,INS)
! SUBROUTINE INFORMATION:
     !       AUTHOR         Cynthia Cogil
     !       DATE WRITTEN   July 29, 1996
     !       MODIFIED       December 6, 1999, April 2, 2000, August 17, 2000
     !       MODIFIED BY    Edward Clements
     !       RE-ENGINEERED  na
     !       VERSION NUMBER 2.0
     !
     ! PURPOSE OF THIS MODULE:
     ! This subroutine computes constants used in teh finite-difference equations
     ! so that they do not need to be regenerated at every time step. They are functions
     ! of cell dimensions and thermal conductivity. They must be recomputed every time
     ! material properties are varied, but need to be calculated only once for a
     ! constant property run.

     ! METHODOLOGY EMPLOYED:
     ! Standard EnergyPlus subroutine formatting for subroutines

     ! REFERENCES: BASE 3D Version 1.0, Cynthia A. Cogil. 23 July, 1996.

     ! OTHER NOTES: none

     !USE MSFLIB
     USE BasementSimData
     IMPLICIT NONE

!*** DECLARATIONS:
     REAL(r64) CXM(0:100,0:100,-35:100), CXP(0:100,0:100,-35:100),              &
     & CYM(0:100,0:100,-35:100), CYP(0:100,0:100,-35:100),                 &
     & CZM(0:100,0:100,-35:100), CZP(0:100,0:100,-35:100),                 &
     & DX(0:100), DY(0:100),DZ(-35:100), DXP(0:100), DYP(0:100),           &
     & DZP(-35:100), REXT, ZC(-35:100), XK, YK, ZK

     INTEGER MTYPE(0:100,0:100,-35:100),NXM1,NYM1,NZBGM1
     CHARACTER *5 INSFULL

!*** Variables added 3/30/00 by EDC for EnergyPlus Surface temperature calculation
     INTEGER INS(0:100,0:100,-35:100)

!*** VARIABLE DESCRIPTIONS:

!*** CXM,CYM,CZM        COEFFICIENT ARRAYS REFERRING TO CELL FACES IN
!***                       THE NEGATIVE COORDINATE DIRECTION INDICATED
!***                       FROM THE CENTER CELL NODE
!*** CXP,CYP,CZP        COEFFICIENT ARRAYS REFERRING TO CELL FACES IN
!***                       THE POSITIVE COORDINATE DIRECTION INDICATED
!***                    FROM THE CENTER CELL NODE
!*** DX,DY,DZ           ARRAYS OF CELL DIMENSIONS [M]
!*** DXP,DYP,DZP        ARRAYS OF DISTANCES BETWEEN CENTERS (I) AND
!***                    (I+1) [M]
!*** I,J,K              INTEGER COUNTERS FOR DO LOOPS
!*** IBASE,JBASE        NUMBER OF CELLS IN X, Y,AND Z DIRECTIONS
!***                    THAT DESCRIBE BASEMENT INTERIOR DIMENSIONS OF
!***                    QUADRANT MODELED
!*** INSFULL            T/F:  IF INSULATED ON EXTERIOR, DOES INSULATION
!***                    EXTEND THE FULL LENGTH OF THE FOUNDATION WALL?
!***                    (ELSE FALSE)
!*** MTYPE              ARRAY OF MATERIAL PROPERTY INDICES
!***                    1)FOUNDATION WALL
!***                    2)FLOOR SLAB
!***                    3)CEILING
!***                    4)SOIL
!***                    5)GRAVEL
!***                    6)WOOD
!***                    7)AIR
!*** NZAG               NUMBER OF CELL FACES IN NEGATIVE Z DIRECTION
!***                    ABOVE GRADE (1) THAT DEFINES ENTIRE DOMAIN OF
!***                    QUADRANT
!*** NXM1,NYM1,NZBGM1   NX-1, NY-1, NZBG-1
!*** REXT               THERMAL RESISTANCE OF EXTERIOR FOUNDATION WALL
!***                    INSULATION [K/(W/M**2)]
!*** TCON               THERMAL CONDUCTIVITY OF SOLID [W/M/K]
!*** XC,YC,ZC           ARRAYS OF CELL CENTER COORDINATES [M]
!*** ZFACE              ARRAY OF CELL FACE COORDINATES [M]
!*** XK,YK,ZK           EFFECTIVE CONDUCTIVITY AT CELL INTERFACE (I-1/I) [W/M/K]

!*** DETERMINE WHICH CELLS ARE INSULATED AND CALCULATE EFFECTIVE
!*** CONDUCTIVITY (XK, YK, ZK) AT THE CELL INTERFACE.  UNLESS NEIGHBOR
!*** CELLS ARE OF DIFFERENT MATERIALS OR THERE IS A SURFACE
!*** RESISTANCE, THE EFFECTIVE CONDUCTIVITY AT THE INTERFACE IS THE
!*** SAME AS THE ACTUAL CELL CONDUCTIVITY.

!*** EFFECTIVE CONDUCTIVITY AT X-INTERFACE (COUNT1-1/COUNT1)
!*** INITIALIZING THE VALUES

     DO COUNT2=0,NYM1
       DO COUNT3=-NZAG,NZBGM1
         DO COUNT1=1,NXM1
           CXM(COUNT1,COUNT2,COUNT3)=0.0
           CXP(COUNT1,COUNT2,COUNT3)=0.0
         END DO
       END DO
     END DO
     DO COUNT2=0,NYM1
       DO COUNT3=-NZAG,NZBGM1
         DO COUNT1=1,NXM1
!***  EFFECTIVE CONDUCTIVITY FOR CELLS OF MATERIAL TYPE EQUAL TO AIR
           IF (MTYPE(COUNT1,COUNT2,COUNT3).EQ.7) THEN
             XK=0.
!***  EFFECTIVE CONDUCTIVITY FOR NEIGHGOR CELLS OF SAME MATERIAL TYPE
           ELSE IF (MTYPE(COUNT1,COUNT2,COUNT3).EQ.MTYPE(COUNT1-1,COUNT2,COUNT3)) THEN
             XK=TCON(MTYPE(COUNT1,COUNT2,COUNT3))
!***  EFFECTIVE CONDUCTIVITY FOR MTYPE(COUNT1-1,COUNT2,COUNT3) EQUAL TO AIR
           ELSE IF (MTYPE(COUNT1,COUNT2,COUNT3).NE.7.AND.                 &
             & MTYPE(COUNT1-1,COUNT2,COUNT3).EQ.7) THEN
             XK=TCON(MTYPE(COUNT1,COUNT2,COUNT3))
!***  EFFECTIVE CONDUCTIVITY FOR BELOW-GRADE FOUNDATION WALL WITH
!***  FULL LENGTH EXTERIOR INSULATION
           ELSE IF ((MTYPE(COUNT1,COUNT2,COUNT3).EQ.4.OR.                 &
             & MTYPE(COUNT1,COUNT2,COUNT3).EQ.5).AND. &
             & MTYPE(COUNT1-1,COUNT2,COUNT3).EQ.1.AND..not. SameString(INSFULL,'FALSE').AND.   &
             & COUNT3.LE.KBASE) THEN
             XK=(DX(COUNT1)+DX(COUNT1-1))/(DX(COUNT1)/                    &
             & TCON(MTYPE(COUNT1,COUNT2,COUNT3))+                         &
             & DX(COUNT1-1)/TCON(MTYPE(COUNT1-1,COUNT2,COUNT3))+REXT)
!***  EFFECTIVE CONDUCTIVITY FOR BELOW-GRADE FOUNDATION WALL WITH
!***  PARTIAL LENGTH EXTERIOR INSULATION
           ELSE IF ((MTYPE(COUNT1,COUNT2,COUNT3).EQ.4.OR.                 &
             & MTYPE(COUNT1,COUNT2,COUNT3).EQ.5).AND.                       &
             & MTYPE(COUNT1-1,COUNT2,COUNT3).EQ.1.AND.SameString(INSFULL,'FALSE').AND.   &
             & ZC(COUNT3).LE.ZC(1+(KBASE-NZAG)/2)) THEN
             XK=(DX(COUNT1)+DX(COUNT1-1))/(DX(COUNT1)/                    &
             & TCON(MTYPE(COUNT1,COUNT2,COUNT3))+                         &
             & DX(COUNT1-1)/TCON(MTYPE(COUNT1-1,COUNT2,COUNT3))+REXT)
!***  EFFECTIVE CONDUCTIVITY FOR NEIGHBOR CELLS OF DIFFERENT MATERIAL
!***  TYPES
           ELSE
             XK=(DX(COUNT1)+DX(COUNT1-1))/(DX(COUNT1)/                    &
             & TCON(MTYPE(COUNT1,COUNT2,COUNT3))+                         &
             & DX(COUNT1-1)/TCON(MTYPE(COUNT1-1,COUNT2,COUNT3)))
           END IF

!***  CALCULATE COEFFICIENTS CXM, CXP
           CXM(COUNT1,COUNT2,COUNT3)=XK/DX(COUNT1)/DXP(COUNT1-1)
           CXP(COUNT1-1,COUNT2,COUNT3)=XK/DX(COUNT1-1)/DXP(COUNT1-1)
         END DO
         CXM(0,COUNT2,COUNT3)=TCON(MTYPE(0,COUNT2,COUNT3))/DX(0)/DX(0)
         CXP(NXM1,COUNT2,COUNT3)=TCON(MTYPE(NXM1,COUNT2,COUNT3))/DX(NXM1)/DX(NXM1)
       END DO
     END DO

!***  EFFECTIVE CONDUCTIVITY AT Y-INTERFACE (COUNT2-1/COUNT2)
!***  INITIALIZING CYM AND CYP
     DO COUNT1=0,NXM1
       DO COUNT3=-NZAG,NZBGM1
         DO COUNT2=1,NYM1
           CYM(COUNT1,COUNT2,COUNT3)=0.0
           CYP(COUNT1,COUNT2,COUNT3)=0.0
        END DO
       END DO
     END DO

     DO COUNT1=0,NXM1
       DO COUNT3=-NZAG,NZBGM1
         DO COUNT2=1,NYM1
!***  EFFECTIVE CONDUCTIVITY FOR CELLS OF MATERIAL TYPE EQUAL TO AIR
           IF (MTYPE(COUNT1,COUNT2,COUNT3).EQ.7) THEN
             YK=0.
!***  EFFECTIVE CONDUCTIVITY FOR NEIGHGOR CELLS OF SAME MATERIAL TYPE
           ELSE IF (MTYPE(COUNT1,COUNT2,COUNT3).EQ.MTYPE(COUNT1,COUNT2-1,COUNT3)) THEN
             YK=TCON(MTYPE(COUNT1,COUNT2,COUNT3))
!***  EFFECTIVE CONDUCTIVITY FOR MTYPE(COUNT1,COUNT2-1,COUNT3) EQUAL TO AIR
           ELSE IF (MTYPE(COUNT1,COUNT2,COUNT3).NE.7.AND.                 &
           & MTYPE(COUNT1,COUNT2-1,COUNT3).EQ.7) THEN
             YK=TCON(MTYPE(COUNT1,COUNT2,COUNT3))
!***  EFFECTIVE CONDUCTIVITY FOR BELOW-GRADE FOUNDATION WALL WITH
!***  FULL LENGTH EXTERIOR INSULATION
           ELSE IF ((MTYPE(COUNT1,COUNT2,COUNT3).EQ.4.OR.                 &
           & MTYPE(COUNT1,COUNT2,COUNT3).EQ.5).AND.                       &
           & MTYPE(COUNT1,COUNT2-1,COUNT3).EQ.1.AND..not. SameString(INSFULL,'FALSE').AND. &
           & COUNT3.LE.KBASE) THEN
             YK=(DY(COUNT2)+DY(COUNT2-1))/(DY(COUNT2)/                    &
             & TCON(MTYPE(COUNT1,COUNT2,COUNT3))+                         &
             & DY(COUNT2-1)/TCON(MTYPE(COUNT1,COUNT2-1,COUNT3))+REXT)
!***  EFFECTIVE CONDUCTIVITY FOR BELOW-GRADE FOUNDATION WALL WITH
!***  PARTIAL LENGTH EXTERIOR INSULATION
           ELSE IF ((MTYPE(COUNT1,COUNT2,COUNT3).EQ.4.OR.                 &
           & MTYPE(COUNT1,COUNT2,COUNT3).EQ.5).AND.                       &
           & MTYPE(COUNT1,COUNT2-1,COUNT3).EQ.1.AND.SameString(INSFULL,'FALSE').AND. &
           & ZC(COUNT3).LE.ZC(1+(KBASE-NZAG)/2)) THEN
             YK=(DY(COUNT2)+DY(COUNT2-1))/(DY(COUNT2)/                    &
             & TCON(MTYPE(COUNT1,COUNT2,COUNT3))+                         &
             & DY(COUNT2-1)/TCON(MTYPE(COUNT1,COUNT2-1,COUNT3))+REXT)
!***  EFFECTIVE CONDUCTIVITY FOR NEIGHBOR CELLS OF DIFFERENT MATERIAL
!***  TYPES
           ELSE
             YK=(DY(COUNT2)+DY(COUNT2-1))/(DY(COUNT2)/                    &
             & TCON(MTYPE(COUNT1,COUNT2,COUNT3))+                         &
             & DY(COUNT2-1)/TCON(MTYPE(COUNT1,COUNT2-1,COUNT3)))
           END IF
!***  CALCULATE COEFFICIENTS CYM, CYP
           CYM(COUNT1,COUNT2,COUNT3)=YK/DY(COUNT2)/DYP(COUNT2-1)
           CYP(COUNT1,COUNT2-1,COUNT3)=YK/DY(COUNT2-1)/DYP(COUNT2-1)
         END DO
         CYM(COUNT1,0,COUNT3)=TCON(MTYPE(COUNT1,0,COUNT3))/DY(0)/DY(0)
         CYP(COUNT1,NYM1,COUNT3)=TCON(MTYPE(COUNT1,NYM1,COUNT3))/DY(NYM1)/DY(NYM1)
       END DO
     END DO

!***  EFFECTIVE CONDUCTIVITY AT Z-INTERFACE (COUNT3-1/COUNT3)
!***  INITIALIZING CZM AND CZP
     DO COUNT1=0,NXM1
       DO COUNT2=0,NYM1
         DO COUNT3=-NZAG+1,NZBGM1
            CZM(COUNT1,COUNT2,COUNT3)=0.0
            CZP(COUNT1,COUNT2,COUNT3)=0.0
         END DO
       END DO
     END DO

     DO COUNT1=0,NXM1
       DO COUNT2=0,NYM1
         DO COUNT3=-NZAG+1,NZBGM1
!*** EFFECTIVE CONDUCTIVITY FOR CELLS OF MATERIAL TYPE EQUAL TO AIR
           IF (MTYPE(COUNT1,COUNT2,COUNT3).EQ.7) THEN
             ZK=0.
!*** EFFECTIVE CONDUCTIVITY FOR NEIGHGOR CELLS OF SAME MATERIAL TYPE
           ELSE IF (MTYPE(COUNT1,COUNT2,COUNT3).EQ.MTYPE(COUNT1,COUNT2,COUNT3-1)) THEN
             ZK=TCON(MTYPE(COUNT1,COUNT2,COUNT3))
!*** EFFECTIVE CONDUCTIVITY FOR MTYPE(COUNT1,COUNT2,COUNT3-1) EQUAL TO AIR
           ELSE IF (MTYPE(COUNT1,COUNT2,COUNT3).NE.7.AND.                 &
           & MTYPE(COUNT1,COUNT2,COUNT3-1).EQ.7) THEN
             ZK=TCON(MTYPE(COUNT1,COUNT2,COUNT3))
!*** EFFECTIVE CONDUCTIVITY FOR NEIGHBOR CELLS OF DIFFERENT MATERIAL
!*** TYPES
           ELSE
             ZK=(DZ(COUNT3)+DZ(COUNT3-1))/(DZ(COUNT3)/                    &
             & TCON(MTYPE(COUNT1,COUNT2,COUNT3))+                         &
             & DZ(COUNT3-1)/TCON(MTYPE(COUNT1,COUNT2,COUNT3-1)))
           END IF

!*** CALCULATE COEFFICIENTS CZM, CZP
           CZM(COUNT1,COUNT2,COUNT3)=ZK/DZ(COUNT3)/DZP(COUNT3-1)
           CZP(COUNT1,COUNT2,COUNT3-1)=ZK/DZ(COUNT3-1)/DZP(COUNT3-1)
         END DO
         CZP(COUNT1,COUNT2,NZBGM1)=2.*TCON(MTYPE(COUNT1,COUNT2,NZBGM1))/  &
         & DZ(NZBGM1)/DZ(NZBGM1)
       END DO
     END DO

!*** CREATE AN ARRAY OF MULTIPLIERS THAT INDICATE WHETER A CELL IS INSULATED OR NOT
!*** THIS WILL BE USED FOR THE SURFACE TEMPERATURE CALCULATION FOR ENERGYPLUS
!*** THIS SECTION WAS ADDED BY EDC
     DO COUNT1=1,NXM1
       DO COUNT2=1,NYM1
         DO COUNT3=-NZAG,NZBGM1
!*** FULL HEIGHT EXTERIOR INSULATION
            IF(((MTYPE(COUNT1,COUNT2,COUNT3).EQ.4.OR.                        &
              & MTYPE(COUNT1,COUNT2,COUNT3).EQ.5).AND.                       &
              & MTYPE(COUNT1-1,COUNT2,COUNT3).EQ.1.AND.                      &
              & .not. SameString(INSFULL,'FALSE').AND.COUNT3.LE.KBASE).OR.                  &
              & ((MTYPE(COUNT1,COUNT2,COUNT3).EQ.4.OR.                       &
              & MTYPE(COUNT1,COUNT2,COUNT3).EQ.5).AND.                       &
              & MTYPE(COUNT1,COUNT2-1,COUNT3).EQ.1.AND..not. SameString(INSFULL,'FALSE').AND. &
              & COUNT3.LE.KBASE)) THEN
              INS(COUNT1,COUNT2,COUNT3)=1
!*** PARTIAL HEIGHT INSULATION
            ELSE IF (((MTYPE(COUNT1,COUNT2,COUNT3).EQ.4.OR.                  &
              & MTYPE(COUNT1,COUNT2,COUNT3).EQ.5).AND.                       &
              & MTYPE(COUNT1-1,COUNT2,COUNT3).EQ.1.AND.SameString(INSFULL,'FALSE').AND. &
              & ZC(COUNT3).LE.ZC(1+(KBASE-NZAG)/2)).OR.                      &
              & ((MTYPE(COUNT1,COUNT2,COUNT3).EQ.4.OR.                       &
              & MTYPE(COUNT1,COUNT2,COUNT3).EQ.5).AND.                       &
              & MTYPE(COUNT1,COUNT2-1,COUNT3).EQ.1.AND.SameString(INSFULL,'FALSE').AND. &
              & ZC(COUNT3).LE.ZC(1+(KBASE-NZAG)/2))) THEN
              INS(COUNT1,COUNT2,COUNT3)=1
            ELSE
              INS(COUNT1,COUNT2,COUNT3)=0
            END IF
         END DO
       END DO
     END DO
     RETURN
END SUBROUTINE FDMCoefficients

!*************************  Calculate 1D Ground Temperature Profile  ********************
SUBROUTINE CalcTearth(IEXT,JEXT,DZ,DZP,TG,CVG)
     USE BasementSimData
     IMPLICIT NONE
! SUBROUTINE INFORMATION:
     !       AUTHOR         Edward Clements
     !       DATE WRITTEN   December 7, 1999
     !       MODIFIED       na
     !       MODIFIED BY    na
     !       RE-ENGINEERED  na
     !       VERSION NUMBER 2.0
     !
     ! PURPOSE OF THIS MODULE:
     ! This subroutine calculates the 1-D hourly temperature distribution in the
     ! ground. It writes one year of hourly ground temperatures, sky radiation,
     ! surface convection coefficients, and solar radiative heat fluxes to the
     ! boundary condition file.

     ! METHODOLOGY EMPLOYED:
     ! Complex ground surface boundary heat balance including solar and infrared
     ! radiation, conduction, convection, and evapotranspiration

     ! REFERENCES: TEARTH Version 1.0, Cynthia A. Cogil. 30 July, 1996.
     !
     ! Bahnfleth, W.  1989.  Three-dimensional modelling of heat transfer from slab
     ! floors. Ph.D. diss., University of Illinois at Urbana-Champaign.  Also
     ! published as US Army CERL Technical Manuscript E-89/11.

     ! OTHER NOTES: none


!*** DECLARATIONS:
     REAL(r64) A(100), B(100), C(100), R(100), X(100), ALB, ALBEDO(2),             &
     & AVGWND, CG, CONST(0:100,2), CPA, DH, DODPG, DW,                   &
     & DZ(-35:100), DZP(-35:100), ELEV, EPS, EPSLN(2),                   &
     & GOFT, GOLD, HRAT(24), IEXT, JEXT, LAT, LONG, MSTD, PBAR(24),      &
     & PVAP, QCL, QEV, QPET, QCS, RBEAM(24), RDIFH(24), RDIFHO,          &
     & RDIRH(24), RDIRHO, RGRND,RHOA, RSKY, RSNW, RSOLH,                 &
     & RSOLV(24), RTOT,SoilDens, TCOND, TDB(24), TDEEP, TG(0:100),       &
     & TGCVG(0:100), TGDAVG (0:100), TGDAVGSUM(0:100),                   &
     & TGMAVG(0:100), TGSUM(0:100), TWB(24), VEGHT(2), VHT, WND(24)

     INTEGER DSNOW(24), II, IDAYSUM, IHR, IMON,                   &
     & IMONSUM, ISNW, IYR, MAXYR

     LOGICAL CVG
     CHARACTER *5 FIXBC,PET,RSNOW
     REAL(r64) :: MaxDeltaT
     DATA MAXYR/20/

     SoilDens=RHO(4)
     CG=CP(4)
     TCOND=TCON(4)
     PET     =SP%PET
     VEGHT   =SP%VEGHT
     EPSLN   =SP%EPSLN
     ALBEDO  =SP%ALBEDO
     FIXBC   =BCS%FIXBC
     RSNOW   =Insul%RSNOW
     LAT     =SiteInfo%LAT
     LONG    =SiteInfo%LONG
     MSTD    =SiteInfo%MSTD
     ELEV    =SiteInfo%ELEV

!*** VARIABLE DESCRIPTIONS:

!*** A,B,C,R,X         COEFFICIENTS AND VARIABLES OF TRIDIAGONAL MATRIX
!*** ALB               CURRENT HOUR'S SURFACE SOLAR ALBEDO
!*** ALBEDO            SURFACE SOLAR ALBEDO ARRAY  1)NO SNOW  2)SNOW
!*** AVGWND            AVERAGE DAILY WIND SPEED [M/S]
!*** CG                SPECIFIC HEAT OF GROUND (SOIL) [J/KG/K]
!*** CONST             ARRAY OF COEFFICIENTS FOR CELLS IN THE NEGATIVE
!***                      Z DIRECTION (1) AND IN THE POSITIVE Z
!***                   DIRECTION (2)
!*** CPA               CONSTANT PRESSURE SPECIFIC HEAT OF AIR [J/KG/K]
!*** CVG               TRUE IF THE ABSOLUTE VALUE OF THE DIFFERENCE
!***                   BETWEEN THE CURRENT AND PREVIOUS CELL
!***                      TEMPERATURES ARE LESS THAN 0.5 DEGREES CELSIUS
!*** DH                TURBULENT HEAT TRANSFER COEFFICIENT [M/S]
!*** DODPG             DELTA/(DELTA + GAMMA), DIMENSIONLESS
!*** DSNOW             THICKNESS OF GROUND SNOW COVER [CM]
!*** DW                TURBULENT MASS TRANSFER COEFFICIENT [M/S]
!*** DZ                ARRAY OF CELL DIMENSIONS [M]
!*** DZP               ARRAY OF DISTANCES BETWEEN CENTERS (COUNT1) AND
!***                      (COUNT1+1) [M]
!*** ELEV              ELEVATION ABOVE SEA LEVEL [M]
!*** EPS               CURRENT HOUR'S SURFACE INFRARED EMISSIVITY
!*** EPSLN             SURFACE INFRARED EMISSIVITY ARRAY
!*** FIXBC             TRUE INDICATES A FIXED TEMPERATURE DEEP GROUND
!***                      CONDITION
!*** GOFT              SURFACE HEAT FLUX INTO GROUND [W/M**2]
!*** GOLD              PREVIOUS HOUR'S G(T) [W/M**2]
!*** HRAT              HUMIDITY RATION, DIMENSIONLESS
!*** COUNT1                 INTEGER COUNTERS FOR DO LOOPS
!*** IDAY              INTEGER COUNTER FOR DO LOOP (DAY)
!*** IDAYSUM,IMONSUM   DAY OF THE YEAR FROM 1 TO 365 (DAY)
!*** IEXT,JEXT         COORDINATES OF BASEMENT EXTERIOR AS MEASURED
!***                      FROM ORIGIN [M]
!*** IHR               INTEGER COUNTER FOR DO LOOP   (HOUR)
!*** II                COUNT1+1
!*** IMON              INTEGER COUNTER FOR DO LOOP (MONTH)
!*** ISNW              2/1 FOR SNOW/NO SNOW ON GROUND FOR THIS HOUR
!*** IYR               INTEGER COUNTER FOR DO LOOP (YEAR)
!*** MAXYR             MAXIMUM NUMBER OF YEARS TO RUN PROGRAM
!*** MSTD              STANDARD TIME MERIDIAN [DEGREES]
!*** NDIM              NUMBER OF DAYS IN MONTH
!*** NZBG              NUMBER OF CELL FACES IN POSITIVE Z DIRECTION
!***                      BELOW GRADE
!*** PBAR              BAROMETRIC PRESSURE [N/M**2]
!*** PET               TRUE INDICATES POTENTIAL EVAPOTRANSPIRATION ON
!*** PVAP              VAPOR PRESSURE OF AMBIENT AIR [N/M**2]
!*** QCL               CONVECTION OF LATENT HEAT [W/M**2]
!*** QEV               SENSIBLE HEAT TRANSFERRED TO THE SURFACE BY
!***                      RADIATION AND CONDUCTION THAT IS CONVERTED TO
!***                      LATENT HEAT [W/M**2]
!*** QPET              LATENT HEAT TRANSFER BY POTENTIAL
!***                      EVAPOTRANSPIRATION [W/M**2]
!*** QCS                  CONVECTION OF SENSIBLE HEAT [W/M**2]
!*** RBEAM             BEAM SOLAR RADIATION RECEIVED ON A HORIZONTAL
!***                      SURFACE DURING THE PRECEEDING HOUR [W/M**2]
!*** RDIFH             DIFFUSE (GLOBAL) SOLAR RADIATION INCIDENT ON A
!***                      HORIZONTAL SURFACE DURING THE PRECEEDING HOUR
!***                      [W/M**2]
!*** RDIFHO            PREVIOUS HOUR'S DIFFUSE SOLAR RADIATION
!***                      INCIDENT ON A HORIZONTAL SURFACE [W/M**2]
!*** RDIRH             DIRECT (GLOBAL) SOLAR RADIATION INCIDENT ON A
!***                      HORIZONTAL SURFACE DURING THE PRECEEDING HOUR
!***                      [W/M**2]
!*** RDIRHO            PREVIOUS HOUR'S DIRECT SOLAR RADIATION
!***                      INCIDENT ON A HORIZONTAL SURFACE [W/M**2]
!*** RGRND             GROUND SURFACE INFRARED RADIATION [W/M**2]
!*** RHOA              DENSITY OF AIR [KG/M**3]
!*** RSKY              INCOMING INFRARED SKY RADIATION [W/M**2]
!*** RSNOW             TRUE IF SNOW COVER MODEL IS ON
!*** RSNW              THERMAL RESISTANCE OF GROUND SNOW COVER
!***                      [K/(W/M**2)]
!*** RSOLH             TOTAL ABSORBED SOLAR RADIATION ON A HORIZONTAL
!***                      SURFACE [W/M**2]
!*** RSOLV             TOTAL ABSORBED SOLAR RADIATION ON A VERTICAL
!***                      SURFACE [W/M**2]
!*** RTOT              EFFECTIVE INCOMING RADIATION AT EARTH'S
!***                      SURFACE [W/M**2]
!*** SIGMA             STEFAN-BOLTZMAN CONSTANT [W/M**2/K**4]
!*** SoilDens          DENSITY OF GROUND (SOIL) [KG/M**3]
!*** TCOND             THERMAL CONDUCTIVITY OF GROUND (SOIL) [W/M/K]
!*** TDB               AMBIENT DRY BULB TEMPERATURE [C]
!*** TDEEP             DEEP GROUND TEMPERATURE FOR FIXED TEMPERATURE
!***                      LOWER BOUNDARY CONDITION [C]
!*** TG                CURRENT GROUND TEMPERATURE [C]
!*** TGCVG             1-D TEMPERATURE FIELD SAVED FOR CONVERGENCE
!***                      TEST
!*** TGDAVG            DAILY AVERAGE GROUND TEMPERATURE PROFILE [C]
!*** TGSUM             DAILY SUM OF GROUND TEMPERATURES IN
!***                      Z-DIRECTION [C]
!*** TWB                  AMBIENT WET BULB TEMPERATURE [C]
!*** VEGHT             VEGETATION HEIGHT 1)NO SNOW  2)SNOW [CM]
!*** VHT                  CURRENT HOUR'S VEGETATION HEIGHT [CM]
!*** WND                  WIND SPEED [M/S]

     CVG=.FALSE.
!*** CALCULATE SOME CONSTANTS USED IN FINITE-DIFFERENCE MATRIX
!*** CONSTANT IN NEGATIVE CELL DIRECTION
!!     REWIND (Weather)

     DO COUNT1=1,NZBG-1
       CONST(COUNT1,1)=TCOND*3600.d0/SoilDens/CG/DZ(COUNT1)/DZP(COUNT1-1)
     END DO
!*** CONSTANTS IN POSITIVE CELL DIRECTION
     DO COUNT1=0,NZBG-2
       CONST(COUNT1,2)=TCOND*3600.d0/SoilDens/CG/DZ(COUNT1)/DZP(COUNT1)
     END DO

!*** CONSTANT IN POSITIVE CELL DIRECTION FOR DEEP GROUND CONDITION
     CONST(NZBG-1,2)=TCOND*7200.d0/SoilDens/CG/DZ(NZBG-1)/DZ(NZBG-1)
!*** FOR FIXED TEMPERATURE LOWER BOUNDARY CONDITION, SET
!*** BOUNDARY VALUE
     IF (.not. SameString(FIXBC,'FALSE')) TDEEP=TG(NZBG)

!*** ESTIMATE CONDUCTION TO GROUND FOR FIRST STEP OF CALCULATION
     GOLD=TCOND*(TG(0)-TG(1))/DZP(0)

!*** POSITION BLAST ASCII WEATHER FILE
!     CALL SkipHeader
!!     REWIND (Weather)
!*** TIME LOOP:
!*** YEARS
     DO IYR=1,MAXYR
!*** AT BEGINNING OF YEAR, UPDATE CONVERGENCE TEST TEMPERATURES
       DO COUNT1=0,NZBG
         TGCVG(COUNT1)=TG(COUNT1)
       END DO

!*** INITIALIZE DAY AND MONTHLY COUNT
       IDAYSUM=0
       IMONSUM=0
!*** MONTHS
       DO IMON=1,12
!*** CALCULATE THE DAY OF THE YEAR FROM 1 TO 365
         IMONSUM=IDAYSUM
!*** DAYS
         DO IDAY=1,NDIM(IMON)
!*** CALCULATE THE DAY OF THE YEAR FROM 1 TO 365
           IDAYSUM=IMONSUM + IDAY
!*** READ ONE DAY OF WEATHER FROM BLAST ASCII FILE
           CALL GetWeatherData(IDAYSUM) !(DSNOW)
           TDB=TodaysWeather%TDB
           TWB=TodaysWeather%TWB
           PBAR=TodaysWeather%PBAR
           HRAT=TodaysWeather%HRAT
           WND=TodaysWeather%WND
           RBEAM=TodaysWeather%RBEAM
           RDIFH=TodaysWeather%RDIFH
           DSNOW=TodaysWeather%DSNOW

!*** CALCULATE AVERAGE WIND SPEED FOR DAY
           AVGWND=0.
           DO COUNT1=1,24
             AVGWND=AVGWND+WND(COUNT1)/24.
           END DO
           IF (AVGWND == 0.0) AVGWND=.001

!*** HOURS
           DO IHR=1,24
!*** SET SURFACE PROPERTIES FOR THIS HOUR
             IF (DSNOW(IHR).EQ.0) THEN
               ISNW=1
             ELSE
               ISNW=2
             END IF
             IF (SameString(RSNOW,'FALSE')) ISNW=1
             ALB=ALBEDO(ISNW)
             EPS=EPSLN(ISNW)
             VHT=VEGHT(ISNW)
             RSNW=DSNOW(IHR)/100.d0/1.55d0
!*** CALCULATE PROPERTIES OF AMBIENT AIR FOR THIS HOUR
             CALL AIRPROPS (HRAT(IHR),PBAR(IHR),TDB(IHR),ELEV,PVAP,RHOA,CPA,DODPG)

!*** CALCULATE CONVECTIVE HEAT & MASS TRANSFER COEFFICIENTS DH AND DW
             CALL CalcHeatMassTransCoeffs (VHT,WND(IHR),AVGWND,TDB(IHR),TG(0),DH,DW)

!*** CALCULATE THE DIRECT SOLAR RADIATION INCIDENT ON A HORIZONTAL
!*** AND ON A VERTICAL SURFACE, RDIRH AND RDIRV, RESPECTIVELY.
             CALL SOLAR(LONG,LAT,MSTD,ALB,EPS,RBEAM(IHR),RDIFH(IHR),RDIRH(IHR), &
             &    RSOLV(IHR),IEXT,JEXT,DZ,IDAYSUM,IHR,TDB(IHR),PVAP,TG(0))

!*** SAVE OLD VALUES OF R FOR LAGGED G(T) CALCULATION
             IF (IHR.EQ.1) THEN
               RDIRHO=RDIRH(IHR)
               RDIFHO=RDIFH(IHR)
             ELSE
               RDIRHO=RDIRH(IHR-1)
               RDIFHO=RDIFH(IHR-1)
             END IF
!*** SET UP COEFFICIENT MATRIX WHERE A, B, AND C ARE, RESPECTIVELY,
!*** THE LOWER, MAJOR, AND UPPER DIAGONAL COEFFICIENT VALUES.  R IS
!*** THE RIGHT-HAND SIDE VECTOR OF KNOWN GROUND TEMPERATURES (OR
!*** KNOWN).  X IS THE UNKNOWN VECTOR OF TEMPERATURES AT THE NEXT
!*** TIME STEP.

!***  INTERIOR CELLS
             DO COUNT1=1,NZBG-2
               II=COUNT1+1
               A(II)=-CONST(COUNT1,1)
               B(II)=1.+CONST(COUNT1,1)+CONST(COUNT1,2)
               C(II)=-CONST(COUNT1,2)
               R(II)=TG(COUNT1)
             END DO
!*** LOWER BOUNDARY (2 CASES:  FIXED TEMPERATURE (FIXBC=T) AND
!*** ZERO HEAT FLUX (FIXBC=F))
             IF (.not. SameString(FIXBC,'FALSE')) THEN
               A(NZBG)=-CONST(NZBG-1,1)
               B(NZBG)=1.+CONST(NZBG-1,1)+CONST(NZBG-1,2)
               R(NZBG)=CONST(NZBG-1,2)*TDEEP+TG(NZBG-1)
             ELSE
               A(NZBG)=-CONST(NZBG-1,1)
               B(NZBG)=1.+CONST(NZBG-1,1)
               R(NZBG)=TG(NZBG-1)
             END IF
!*** UPPER BOUNDARY (GROUND SURFACE)
!*** CALCULATE G(T)
!*** SKY RADIATION (RSKY) FROM ANGSTROM/GEIGER EQUATION
!*** EMISSIVITY OF THE SKY IS APPROX. 0.96 (INCROPERA AND DEWITT)
             RSKY=0.96d0*SIGMA*((TDB(IHR)+273.15d0)**4)*(0.820d0-0.250d0*       &
             &    EXP(-2.3d0*0.094d0*0.01d0*PVAP))
!*** GROUND SURFACE LONGWAVE RADIATION (RG) FROM STEFAN-BOLZTMAN
!*** EQUATION
             RGRND=EPS*SIGMA*(TG(0)+273.15d0)**4
!*** TOTAL SOLAR RADIATION (DIRECT AND DIFFUSE) ABSORBED BY THE
!*** EARTH'S SURFACE - AVERAGED OVER THE LAST HOUR
             RSOLH=(1.-ALB)*(RDIRHO+RDIFHO+RDIRH(IHR)+RDIFH(IHR))/2.
!*** EFFECTIVE INCOMING RADIATION AT THE EARTH'S SURFACE (RT)
             RTOT=RSOLH+RSKY-RGRND
!*** SENSIBLE CONVECTIVE LOSSES
             QCS=RHOA*CPA*DH*(TG(0)-TDB(IHR))
!*** COMPUTE LATENT HEAT LOSSES IF PET=T
             IF (.not. SameString(PET,'FALSE')) THEN
               QEV=DODPG*(RTOT-GOLD)
               QCL=RHOA*CPA*DW*(TDB(IHR)-TWB(IHR))
             ELSE
               QEV=0.
               QCL=0.
             END IF
!*** COMPUTE HEAT LOSS BY POTENTIAL EVAPOTRANSPIRATION, QPET
             QPET=QEV+QCL
!*** COMPUTE NET FLUX CONDUCTED INTO THE GROUND, G(T)
             GOFT=RTOT-QCS-QPET
!*** RESET GOLD
             GOLD=GOFT
!*** COMPUTE COEFFICIENTS FOR SURFACE CELL
             IF (RSNW.EQ.0.) THEN
               B(1)=1.+CONST(0,2)
               R(1)=TG(0)+GOFT*3600.d0/SoilDens/CG/DZ(0)
             ELSE
               B(1)=1.d0+(1.d0/RSNW+TCOND/DZP(0))*3600.d0/SoilDens/CG/DZ(0)
               R(1)=TG(0)+(GOFT+TDB(IHR)/RSNW)*3600.d0/SoilDens/CG/DZ(0)
             END IF
             C(1)=-CONST(0,2)
!*** SOLVE SYSTEM WITH TRIDIAGONAL MATRIX ALGORITHM
             CALL TRIDI1D (A,B,C,X,R,NZBG)
             DO COUNT1=0,NZBG-1
               TG(COUNT1)=X(COUNT1+1)
             END DO
             IF (SameString(FIXBC,'FALSE')) TG(NZBG)=TG(NZBG-1)
!*** IF TEMPERATURE FIELD HAS CONVERGED, RESULTS ARE WRITTEN TO
!*** THE BOUNDARY CONDITION FILE (RSKY, CONVECTIVE HEAT AND MASS
!*** TRANSFER COEFFICIENTS (COUNT1.E., RHOA*CPA*DH & RHOA*CPA*DW),
!*** AND TG(COUNT1))
             IF (CVG) THEN
               WRITE (GroundTemp,*) RSKY, RHOA*CPA*DH, RHOA*CPA*DW, DODPG
               WRITE (GroundTemp,*) (TG(COUNT1), COUNT1=0,NZBG)
             END IF
!*** COMPUTE THE SUM OF HOURLY GROUND TEMPERATURES FOR ONE DAY
             DO COUNT1=0,NZBG
               TGSUM(COUNT1)=TGSUM(COUNT1) + TG(COUNT1)
             END DO
!*** HOURS
           END DO
!*** COMPUTE DAILY AVERAGE GROUND TEMPERATURE PROFILE
           DO COUNT1=0,NZBG
             TGDAVG(COUNT1)=TGSUM(COUNT1)/24.
           END DO
!*** RESET TGSUM(COUNT1)
           DO COUNT1=0,NZBG
             TGSUM(COUNT1)=0.
           END DO
!*** WRITE THE AREA-WEIGHTED AVERAGE TOTAL ABSORBED SOLAR
!*** RADIATION ON A VERTICAL SURFACE (RSOLV) AND THE DIRECT SOLAR
!*** RADIATION INCIDENT ON A HORIZONTAL SURFACE (RDIRVH) FOR THE
!*** PRECEEDING HOUR TO A BOUNDARY CONDITION FILE FOR
!*** ABOVE-GRADE SURFACES
           IF (CVG) THEN
             DO IHR=1,24
               WRITE (SolarFile,*) RSOLV(IHR), RDIRH(IHR)
             ENDDO
           END IF
500        FORMAT (2F12.4/)
!*** COMPUTE THE SUM OF AVERAGE DAILY GROUND TEMPERATURES FOR ONE MONTH
           DO COUNT1=0,NZBG
             TGDAVGSUM(COUNT1)=TGDAVGSUM(COUNT1) + TGDAVG(COUNT1)
           END DO
!*** DAYS
         END DO
!*** COMPUTE THE MONTHLY AVERAGE GROUND TEMPERATURE PROFILE
         DO COUNT1=0,NZBG
           TGMAVG(COUNT1)=TGDAVGSUM(COUNT1)/NDIM(IMON)
         END DO
!*** RESET TGDAVGSUM(COUNT1)
         DO COUNT1=0,NZBG
           TGDAVGSUM(COUNT1)=0.
         END DO
!*** OUTPUT MONTHLY AVERAGE GROUND TEMPERATURE PROFILE,
!*** OTHERWISE RESET TGMAVG(COUNT1) AND TGDAVGSUM(COUNT1) TO ZERO
         IF (CVG) THEN
           DO COUNT1=0,NZBG
             WRITE (AvgTG,*) TGMAVG(COUNT1)
           ENDDO
         ELSE
           DO COUNT1=0,NZBG
             TGMAVG(COUNT1)=0.
           END DO
         END IF
600      FORMAT (F8.4)
!*** MONTHS
       END DO
!*** TEST FOR CONVERGENCE AT 2400 HRS ON 31 DECEMBER
       IF (.NOT.CVG) THEN
         PRINT*,'1-D solution has not converged, Delta T>0.05'
         WRITE(DebugOutFile,*) '1-D solution has not converged, Delta T>0.05'
         IF (IYR.EQ.MAXYR-1) THEN
           PRINT*, 'IYR=MAXYR-1, PROGRAM TERMINATED'
           WRITE(DebugOutFile,*) 'IYR=MAXYR-1, PROGRAM TERMINATED'
           RETURN
         ELSE
           CVG=.TRUE.
           MaxDeltaT=-99999.d0
           DO COUNT1=0,NZBG
             IF (ABS(TG(COUNT1)-TGCVG(COUNT1)).GT.0.05d0) CVG=.FALSE.
             MaxDeltaT=MAX(MaxDeltaT,ABS(TG(COUNT1)-TGCVG(COUNT1)))
           END DO
         END IF
       ELSE
         PRINT*, '1-D solution has converged, MaxDeltaT=',MaxDeltaT
         WRITE(DebugOutFile,*) '1-D solution has converged, MaxDeltaT=',MaxDeltaT
!!         REWIND (Weather)
!         REWIND (Weather2)
         REWIND (GroundTemp)
         RETURN
       END IF
!*** REWIND AND POSITION BLAST ASCII WEATHER FILE AT YEAR END
!!       REWIND (Weather)
!       CALL SkipHeader
!!*** REWIND AND POSITIONS TMY2 WEATHER FILE AT YEAR END
!       REWIND (Weather2)
!*** YEARS
     END DO
END SUBROUTINE CalcTearth

!******************************  Air Properties  ****************************************
SUBROUTINE AIRPROPS (HRAT,PBAR,TDB,ELEV,PVAP,RHOA,CPA,DODPG)
! SUBROUTINE INFORMATION:
     !       AUTHOR         Edward Clements
     !       DATE WRITTEN   December 7, 1999
     !       MODIFIED       na
     !       MODIFIED BY    na
     !       RE-ENGINEERED  na
     !      VERSION NUMBER 2.0
     !
     ! PURPOSE OF THIS MODULE:
     ! This subroutine calculates the following air properties: vapor pressure, density,
     ! and constant pressure specific heat using relations published in the ASHRAE
     ! Handbook of Fundamentals, 1997 SI Version. The evaporation parameter
     ! DELTA/(DELTA+GAMMA) (DODPG) is computed by a second order curve fir to data
     ! published in "Consumptive Use of Water," ASCE, 1973.

     ! METHODOLOGY EMPLOYED:
     ! ASHRAE and ASCE relations as described above.

     ! REFERENCES: AIRPROPS Version 1.0, Cynthia A. Cogil. 30 July, 1996.
     !
     ! Bahnfleth, W.  1989.  Three-dimensional modelling of heat transfer from slab
     ! floors. Ph.D. diss., University of Illinois at Urbana-Champaign.  Also
     ! published as US Army CERL Technical Manuscript E-89/11.
     !
     ! 1997 ASHRAE Handbook of Fundamentals (SI Version)
     !
     ! "Consumptive Use of Water," ASCE, 1973.

     ! OTHER NOTES: none

     IMPLICIT NONE

!*** DECLARATIONS:
     REAL(r64) CPA,DODPG,ELEV,HRAT,PBAR,PVAP,RHOA,TDB

!*** VARIABLE DESCRIPTIONS:

!*** CPA         CONSTANT PRESSURE SPECIFIC HEAT OF AIR                [J/kg/K]
!*** DODPG       DELTA/(DELTA + GAMMA)                                       []
!*** ELEV        ELEVATION ABOVE SEA LEVEL                                  [m]
!*** HRAT        HUMIDITY RATIO                                              []
!*** PBAR        BAROMETRIC PRESSURE                                       [Pa]
!*** PVAP        VAPOR PRESSURE OF AMBIENT AIR                             [Pa]
!*** RHOA        DENSITY OF AIR                                        [kg/m^3]
!*** TDB         AMBIENT DRY BULB TEMPERATURE                               [C]

     PVAP=(HRAT/(HRAT+0.62198d0))*PBAR
     RHOA=(PBAR-0.3780d0*PVAP)/(287.055d0*(TDB+273.15d0))
     CPA=1007.d0+863.d0*PVAP/PBAR
     DODPG=0.395643d0+0.17092d-01*TDB-0.140959d-03*TDB*TDB+0.309091d-04*ELEV &
     &     +0.822511d-09*ELEV*ELEV-0.472208d-06*TDB*ELEV
     RETURN
END SUBROUTINE AIRPROPS


!*************  Calculate Heat and Mass Transfer Coefficients****************************
SUBROUTINE CalcHeatMassTransCoeffs (VEGHTCM,WND,AVGWND,TDB,TG,DH,DW)
! SUBROUTINE INFORMATION:
     !       AUTHOR         Edward Clements
     !       DATE WRITTEN   December 7, 1999
     !       MODIFIED       na
     !       MODIFIED BY    na
     !       RE-ENGINEERED  na
     !       VERSION NUMBER 2.0
     !
     ! PURPOSE OF THIS MODULE:
     ! This subroutine computes turbulent heat and mass transfer coefficients for one hour
     ! using the correlation presented in "Evapotranspiration and Irrigation Water
     ! Requirements" by Jensen, Burman, and Allen, ASCE, 1990. A stability adjustment
     ! factor for DH and DW under diabatic (unstable) and adiabatic (stable) conditions
     ! is given by Bartholic, 1970, and in Kreith and Sellers, 1975.

     ! METHODOLOGY EMPLOYED:
     ! Standard EnergyPlus Subroutine formatting

     ! REFERENCES: CHMTC Version 1.0, Cynthia A. Cogil. 31 July, 1996.
     !
     ! Bahnfleth, W.  1989.  Three-dimensional modelling of heat transfer from slab
     ! floors. Ph.D. diss., University of Illinois at Urbana-Champaign.  Also
     ! published as US Army CERL Technical Manuscript E-89/11.
     !
     ! 1997 ASHRAE Handbook of Fundamentals (SI Version)
     !
     ! "Consumptive Use of Water," ASCE, 1973.

     ! OTHER NOTES: none

     USE BasementSimData
     IMPLICIT NONE

!*** DECLARATIONS:
     real(r64), parameter :: monethird=-1.0d0/3.0d0
     REAL(r64) AVGWND,CPHI,DH,DM,DW,PHI,RI,TDB,TG,VEGHTCM,WND,WND2,ZEROD,ZOM,ZOV

!*** VARIABLE DESCRIPTIONS:

!*** AVGWND      AVERAGE WIND SPEED FOR A DAY                             [m/s]
!*** CPHI        INTEGRATION OF (PHI-1)/ZOM FROM ZOM TO Z-ZEROD           [   ]
!*** DH          TURBULENT HEAT TRANSFER COEFFICIENT                      [m/s]
!*** DM          NEUTRAL STABILITY MOMENTUM TRANSFER COEFFICIENT          [m/s]
!*** DW          TURBULENT MASS TRANSFER COEFFICIENT                      [m/s]
!*** G           ACCELERATION OF GRAVITY                                [m/s^2]
!*** PHI         STABILITY PROFILE INFLUENCE FUNCTION                        []
!*** RI          RICHARDSON NUMBER                                           []
!*** TDB            AMBIENT DRY BULB TEMPERATURE                            [C]
!*** TG          GROUND SURFACE TEMPERTURE                                  [C]
!*** VEGHTCM     VEGETATION HEIGHT AS INPUT                                [cm]
!*** VONKAR =0.41, VON KARMAN CONSTANT                                       []
!*** WND            WIND SPEED                                            [m/s]
!*** WND2        WIND SPEED AT HEIGHT 2M                                  [m/s]
!*** ZEROD       ZERO-DISPLACEMENT PARAMETER                               [cm]
!*** ZOM            ROUGHNESS PARAMETER FOR MOMENTUM                       [cm]
!*** ZOV            ROUGHNESS PARAMETER FOR HEAT AND VAPOR TRANSFER        [cm]

!***  CALCULATE THE ZERO-DISPLACEMENT PARAMETER AND THE ROUGHNESS
!*** PARAMETERS FOR MOMENTUM AND FOR HEAT AND VAPOR TRANSFER
     ZEROD=0.67d0*VEGHTCM
     ZOM=0.123d0*VEGHTCM
     ZOV=0.1d0*ZOM

!*** ESTIMATE 2M WIND SPEED FROM 10M SPEED AS MEASURED AT WEATHER
!*** STATION BY LOGARITHMIC BOUNDARY LAYER ASSUMPTION (IF 10M WIND
!*** SPEED IS ZERO, USE DAILY AVERAGE WIND)
     IF (WND.EQ.0.) THEN
       WND2=AVGWND*(LOG(200.d0-ZEROD)/ZOM)/(LOG(1000.d0-ZEROD)/ZOM)
     ELSE
       WND2=WND*(LOG(200.d0-ZEROD)/ZOM)/(LOG(1000.d0-ZEROD)/ZOM)
     END IF

!*** COMPUTE THE NEUTRAL STABILITY MOMENTUM TRANSFER COEFFICIENT
!*** COMPUTE THE RICHARDSON NUMBER

     RI=2.d0*(G/(TDB+273.15d0))*LOG((200.d0-ZEROD)/ZOM)*(TDB-TG)/WND2/WND2

!*** COMPUTE THE STABILITY PROFILE INFLUENCE FUNCTION, PHI,
!*** FOR UNSTABLE CONDITIONS (i.e., TG>TDB)
     IF (TDB.LE.TG) THEN
       PHI=(1.d0-18.d0*RI)**(-0.250d0)
     END IF

!*** INTEGRATE (PHI-1)/ZOM FROM ZOM TO Z-ZEROD FOR UNSTABLE CONDITIONS
     IF (TDB.LE.TG) THEN
       CPHI=(PHI-1.d0)*LOG((200.d0-ZEROD)/ZOM)
     ELSE
       CPHI=0.
     END IF
     DM=VONKAR*VONKAR*WND2/(CPHI+LOG((200.d0-ZEROD)/ZOM))/(CPHI+LOG((200.d0-ZEROD)/ZOV))

!*** COMPUTE THE TURBULENT HEAT AND MASS TRANSFER COEFFICIENTS
     IF (TDB.LE.TG) THEN
       DH=DM
       DW=DM
     ELSE
       DH=DM*(1.d0-14.d0*(TG-TDB)/WND2/WND2)**(monethird)
       DW=DH
     END IF
     RETURN
END SUBROUTINE CalcHeatMassTransCoeffs


!********************************  SOLAR  ***********************************************
SUBROUTINE SOLAR(LONG,LAT,MSTD,ALB,EPS,RBEAM,RDIFH,RDIRH,RSOLV,IEXT,JEXT,DZ,IDAY_LOCAL,IHR,   &
&          TDB,PVAP,TG)
! SUBROUTINE INFORMATION:
     !       AUTHOR         Cynthia A. Cogil
     !       DATE WRITTEN   January 22, 1997
     !       MODIFIED       May 23, 2000
     !       MODIFIED BY    Edward Clements
     !       RE-ENGINEERED  na
     !       VERSION NUMBER 2.0
     !
     ! PURPOSE OF THIS MODULE:
     ! This subroutine calculates the solar radiation incident on a horizontal and
     ! a vertical surface from reported beam radiation data.

     ! METHODOLOGY EMPLOYED:
     ! Standard EnergyPlus Subroutine formatting

     ! REFERENCES: CHMTC Version 1.0, Cynthia A. Cogil. 31 July, 1996.
     !

     ! OTHER NOTES: none

     USE BasementSimData
     IMPLICIT NONE

!*** DECLARATIONS:

     REAL(r64) ALB,B,BETA,DELTA,DZ(-35:100),DZAG,EPS,ET,GAMMAE,GAMMAN,GAMMAS,GAMMAW,H,   &
     &    IEXT,JEXT,LAT, LONG,MSTD,PHI,PVAP,RBEAM,RDIFH,RDIFVE,RDIFVN, RDIFVS,      &
     &    RDIFVW,RDIRH,RDIRVE,RDIRVN,RDIRVS,RDIRVW,RGRND, RREFL,RSKY, RSOLV, TDB,   &
     &    TG, THETAH, THETAVE, THETAVN, THETAVS, THETAVW, TSOL,PHIARG

     INTEGER IHR  ,IDAY_LOCAL !,NZAG

!*** VARIABLE DESCRIPTIONS:

!*** ALB               SURFACE SOLAR ALBEDO FOR THIS HOUR
!*** B                 CONSTANT USED IN EQUATION OF TIME [MIN]
!*** BETA              SOLAR ALTITUDE ANGLE [DEGREES]
!*** DELTA             SOLAR DECLINATION ANGLE [DEGREES]
!*** DZ                ARRAYS OF CELL DIMENSIONS [M]
!*** DZAG              SUM OF CELL DIMENSIONS ABOVE-GRADE [M]
!*** EPS
!*** ET                EQUATION OF TIME [MIN]
!*** F                 ANGLE FACTOR BETWEEN A THE GROUND AND THE WALL
!***                      [DIMENSIONLESS]
!*** H                 HOUR ANGLE [DEGREES]
!*** COUNT1                 INTEGER COUNTER FOR DO LOOP
!*** IEXT,JEXT         COORDINATES OF BASEMENT EXTERIOR AS MEASURED
!***                      FROM ORIGIN [M]
!*** IDAY              DAY OF THE YEAR FROM 1 TO 365 [DAY]
!*** IHR               INTEGER COUNTER FOR DO LOOP (HOUR)
!*** LAT                  SITE LATITUDE [DEGREES]
!*** LONG              SITE LONGITUDE [DEGREES]
!*** MSTD              STANDARD TIME MERIDIAN [DEGREES]
!*** NZAG              NUMBER OF CELL FACES IN NEGATIVE Z DIRECTION ABOVE GRADE
!*** PHI               SOLAR AZIMUTH ANGLE [DEGREES]
!*** PVAP              VAPOR PRESSURE OF AMBIENT AIR [N/M**2]
!*** RBEAM             BEAM SOLAR RADIATION (NORMAL TO RAYS) [W/M**2]
!*** RDIFH             DIFFUSE SOLAR RADIATION INCIDENT ON A HORIZONTAL
!***                      SURFACE DURING THE PRECEEDING HOUR [W/M**2]
!*** RDIFVN, RDIFVE,
!*** RDIFVS, RDIFVW    DIFFUSE SOLAR RADIATION INCIDENT ON A VERTICAL
!***                      SURFACE DURING THE PRECEEDING HOUR FOR THE
!***                      CARDINAL POINTS [W/M**2]
!*** RDIRH             DIRECT SOLAR RADIATION INCIDENT ON A HORIZONTAL
!***                      SURFACE DURING THE PRECEEDING HOUR [W/M**2]
!*** RDIRVN, RDIRVE,
!*** RDIRVS, RDIRVW    DIRECT SOLAR RADIATION INCIDENT ON A VERTICAL
!***                      SURFACE DURING THE PRECEEDING HOUR FOR THE
!***                      CARDINAL POINTS [W/M**2]
!*** RREFL             REFLECTED SOLAR RADIATION FROM THE GROUND
!***                      SURFACE TO A VERTICAL SURFACE [W/M**2]
!*** RSKY              INFRARED SKY RADIATION [W/M**2]
!*** RSOLV             AREA-WEIGHTED TOTAL ABSORBED RADIATION ON A
!***                      VERTICAL SURFACE [W/M**2]
!*** TDB               AMBIENT DRY BULB TEMPERATURE [C]
!*** TG
!*** THETAH            ANGLE OF INCIDENCE FOR A HORIZONTAL SURFACE
!***                      [DEGREES]
!*** THETAVN, THETAVE
!*** THETAVS, THETAVW  ANGLE OF INCIDENCE FOR A VERTICAL SURFACE
!***                      FOR THE CARDIANL POINTS [DEGREES]
!*** TSOL              LOCAL SOLAR TIME [HOURS]


!*** CALCULATE THE LOCAL SOLAR TIME FROM THE STANDARD TIME
     B=360.d0*(IDAY_LOCAL-81.d0)/364.d0
     ET=(9.87d0*SIND(2.d0*B)-7.53d0*COSD(B)-1.5d0*SIND(B))/60.d0
     TSOL=IHR+(MSTD-LONG)/15.d0+ET

!*** CALCULATE THE SOLAR DECLINATION ANGLE
     DELTA=ASIND(-SIND(23.45d0)*COSD(360.d0*(IDAY_LOCAL+10.d0)/365.25d0))/15.d0

!*** CALCULATE THE HOUR ANGLE
     H=ABS(TSOL-12.d0)*15.d0

!*** CALCULATE THE SOLAR ALTITUDE ANGLE
     BETA=ASIND(COSD(LAT)*COSD(H)*COSD(DELTA)+SIND(LAT)*SIND(DELTA))

!*** CALCULATE THE SOLAR AZIMUTH ANGLE  (COMMENT OUT LINES 1674-1678 AND
!**** Modified by Edward Clements, May 23, 2000
     PHIARG=(SIND(BETA)*SIND(LAT)-SIND(DELTA))/(COSD(BETA)*COSD(LAT))
     IF(PHIARG.GT.1.d0) THEN
        PHIARG=1.
     ELSE IF (PHIARG.LT.-1.d0) THEN
        PHIARG=-1.
     END IF
     PHI=ACOSD(PHIARG)

     IF (TSOL.LE.12.d0) PHI=-PHI

!*** CALCULATE THE SURFACE SOLAR AZIMUTH ANGLE FOR THE
!*** CARDINAL POINTS:  NORTH, EAST, SOUTH, AND WEST
     GAMMAN=PHI-180.d0
     GAMMAE=PHI-(-90.d0)
     GAMMAS=PHI-0.d0
     GAMMAW=PHI-90.d0

!*** CALCULATE THE ANGLE OF INCIDENCE FOR A HORIZONTAL AND VERTICAL
!*** SURFACE FOR THE CARDINAL POINTS
     THETAH= ACOSD(SIND(BETA))
     THETAVN=ACOSD(COSD(BETA)*COSD(GAMMAN))
     THETAVE=ACOSD(COSD(BETA)*COSD(GAMMAE))
     THETAVS=ACOSD(COSD(BETA)*COSD(GAMMAS))
     THETAVW=ACOSD(COSD(BETA)*COSD(GAMMAW))

!*** CALCULATE THE DIRECT SOLAR RADIATION INCIDENT ON A HORIZONTAL
!*** SURFACE
     RDIRH=RBEAM*COSD(THETAH)

!*** CALCULATE THE DIRECT SOLAR RADIATION INCIDENT ON A VERTICAL
!*** SURFACE FOR THE CARDINAL POINTS
     IF (COSD(THETAVN).GT.0.) THEN
       RDIRVN=RBEAM*COSD(THETAVN)
     ELSE
       RDIRVN=0.
     END IF
     IF (COSD(THETAVE).GT.0.) THEN
       RDIRVE=RBEAM*COSD(THETAVE)
     ELSE
       RDIRVE=0.
     END IF
     IF (COSD(THETAVS).GT.0.) THEN
       RDIRVS=RBEAM*COSD(THETAVS)
     ELSE
       RDIRVS=0.
     END IF
     IF (COSD(THETAVW).GT.0.) THEN
       RDIRVW=RBEAM*COSD(THETAVW)
     ELSE
       RDIRVW=0.
     END IF

!*** CALCULATE THE REFLECTED SOLAR RADIATION FROM THE GROUND TO A
!*** VERTICAL SURFACE
     RREFL=(RDIRH+RDIFH)*ALB/2.d0
!*** CALCULATE THE DIFFUSE SOLAR RADIATION INCIDENT ON A VERTICAL
!*** SURFACE
     IF (COSD(THETAVN).GT.-0.2d0) THEN
       RDIFVN=RDIFH*(0.55d0+0.437d0*COSD(THETAVN)+0.313d0*(COSD(THETAVN))**2)
     ELSE
       RDIFVN=RDIFH*0.45d0
     END IF
     IF (COSD(THETAVE).GT.-0.2d0) THEN
       RDIFVE=RDIFH*(0.55d0+0.437d0*COSD(THETAVE)+0.313d0*(COSD(THETAVE))**2)
     ELSE
       RDIFVE=RDIFH*0.45d0
     END IF
     IF (COSD(THETAVS).GT.-0.2d0) THEN
       RDIFVS=RDIFH*(0.55d0+0.437d0*COSD(THETAVS)+0.313d0*(COSD(THETAVS))**2)
     ELSE
       RDIFVS=RDIFH*0.45d0
     END IF
     IF (COSD(THETAVW).GT.-0.2d0) THEN
       RDIFVW=RDIFH*(0.55d0+0.437d0*COSD(THETAVW)+0.313d0*(COSD(THETAVW))**2)
     ELSE
       RDIFVW=RDIFH*0.45d0
     END IF

!*** CALCULATE THE INCIDENT INFRARED SKY RADIATION BY MEANS OF
!*** ANGSTROM'S EMPIRICAL CLEAR-SKY CORRELATION
     RSKY=0.96d0*SIGMA*((TDB+273.15d0)**4)*(0.820d0-0.250d0*EXP(-2.3d0*0.094d0*0.01d0*PVAP))/2.d0

!*** CALCULATE THE EMITTED INFRARED RADIATION FROM THE
!*** GROUND INCIDENT ON THE WALL
     RGRND=(EPS*SIGMA*(TG+273.15d0)**4)/2.d0

!*** CALCULATE THE AREA-WEIGHTED AVERAGE TOTAL ABSORBED SOLAR
!*** RADIATION ON A VERTICAL SURFACE FOR THE PRECEEDING HOUR
!*** ASSUMING THE ABSORBTIVITY OF THE ABOVE-GRADE PORTION OF THE
!*** FOUNDATION WALL IS 0.6.
     DZAG=0.
     DO COUNT1=-NZAG,-1
       DZAG=DZAG+DZ(COUNT1)
     END DO
     RSOLV=(IEXT*(0.6d0*(RDIRVE+RDIRVW+RDIFVE+RDIFVW+2.d0*RREFL)/2.d0+RSKY+    &
     & RGRND)+JEXT*(0.6d0*(RDIRVN+RDIRVS+RDIFVN+RDIFVS+2.d0*RREFL)/2.d0+ &
     & RSKY+RGRND))/(IEXT+JEXT)
     RETURN
END SUBROUTINE SOLAR

!*********************  One Dimensional Tridiagonal Matrix Solver  **********************
SUBROUTINE TRIDI1D (A,B,C,X,R,N)

!*** THIS SUBROUTINE SOLVES A SYSTEM OF LINEAR EQUATIONS HAVING A
!*** TRIDIAGONAL COEFFICIENT MATRIX BY THE THOMAS ALGORITHM.  THIS
!*** VERSION IS TAKEN FROM "NUMERICAL MARCHING TECHNIQUES FOR FLUID
!*** FLOWS WITH HEAT TRANSFER" BY ROBERT W. HORNBECK, NASA, 1973.

!*** A, B, AND C ARE, RESPECTIVELY, THE LOWER, MAJOR, AND UPPER
!*** DIAGONAL COEFFICIENT VALUES.  FOR A SYSTEM OF N EQUATIONS,
!*** INDICES OF A RUN FROM 2 TO N, INDICES OF B FROM 1 TO N, AND
!*** INDICES OF C FROM 1 TO N-1.  R IS THE RIGHT-HAND SIDE VECTOR
!*** OF THE SYSTEM.  THE UNKNOWN VECTOR IS RETURNED AS X.

!*** DECLARATIONS:

     !REAL A(50),B(50),C(50),R(50),X(50), BN

     INTEGER II, N
     REAL(r64) A, B, C, R, X, BN
     DIMENSION A(N)
     DIMENSION B(N)
     DIMENSION C(N)
     DIMENSION R(N)
     DIMENSION X(N)

!*** VARIABLE DESCRIPTIONS:

!*** A,B,C,X,R   ARRAYS OF COEFFICIENTS AND VARIABLES OF
!***             TRIDIAGONAL MATIX
!*** BN
!*** I           INTERGER COUNTER FOR DO LOOP
!*** II          -I+N-2
!*** N           NUMBER OF CELL FACES IN POSITIVE Z DIRECTION

     A(N)=A(N)/B(N)
     R(N)=R(N)/B(N)
     DO COUNT1=2,N
       II=-COUNT1+N+2
       BN=1.d0/(B(II-1)-A(II)*C(II-1))
       A(II-1)=A(II-1)*BN
       R(II-1)=(R(II-1)-C(II-1)*R(II))*BN
     END DO
     X(1)=R(1)
     DO COUNT1=2,N
       X(COUNT1)=R(COUNT1)-A(COUNT1)*X(COUNT1-1)
     END DO
END SUBROUTINE TRIDI1D

!********************************  PRELIMINARY OUTPUT  **********************************
SUBROUTINE PrelimOutput(ACEIL,AFLOOR,ARIM,ASILL,AWALL,PERIM,RUNID,TDBH,TDBC)
! SUBROUTINE INFORMATION:
     !       AUTHOR         Edward Clements
     !       DATE WRITTEN   December 7, 1999
     !       MODIFIED       August 2000
     !       MODIFIED BY
     !       RE-ENGINEERED  na
     !       VERSION NUMBER 1.0
     !
     ! PURPOSE OF THIS MODULE:
     ! This subroutine writes an input echo file for debugging purposes.

     ! METHODOLOGY EMPLOYED:
     ! Standard EnergyPlus Subroutine formatting

     ! REFERENCES: Base3d Version 1.0, Cynthia A. Cogil. 23 July, 1996.
     !

     ! OTHER NOTES: none

     USE BasementSimData
     USE EPWRead, ONLY: LocationName
     IMPLICIT NONE
!*** Variable Declarations
     CHARACTER *3 RUNID      ! Run identifier for this run                             []

     REAL(r64) TDBH               ! Ambient temperature below which heating on             [C]
     REAL(r64) TDBC               ! Ambient temperature above which cooling on             [C]
     REAL(r64) ACEIL              ! Surface area of the basement ceiling                 [m^2]
     REAL(r64) AFLOOR             ! Surface area of the basement floor slab              [m^2]
     REAL(r64) PERIM              ! Floor slab perimeter                                   [m]
     REAL(r64) ARIM               ! Total surface area of the rim joist                  [m^2]
     REAL(r64) ASILL              ! Total surface area of the sill box                   [m^2]
     REAL(r64) AWALL              ! Total surface area of the foundation wall            [m^2]

!***  Variables contained in derived types
     REAL(r64) TSTEP              ! Simulation time step                                 [Hrs]
     REAL(r64) F                  ! F factor for the F factor ADI method                    []
     REAL(r64) LONG               ! Site Longitude                                       [Deg]
     REAL(r64) LAT                ! Site Latitude                                        [Deg]
     REAL(r64) MSTD               ! Standard Time Zone Meridian                          [Deg]
     REAL(r64) ELEV               ! Site Elevation                                         [m]
     REAL(r64) REXT               ! Thermal resistance of exterior foundation      [K/(W/m^2)]
                             ! wall insulation
     REAL(r64) RINT               ! Thermal resistance of interior foundation      [K/(W/m^2)]
                             ! wall insulation
     REAL(r64) RSID               ! Thermal resistance of above grade exterior     [K/(W/m^2)]
                             ! siding
     REAL(r64) RSILL              ! Thermal resistance of sill box insulation      [K/(W/m^2)]
                             ! (For interior insulation case)
     REAL(r64) RCEIL              ! Thermal resistance of ceiling insulation       [K/(W/m^2)]
     REAL(r64) ALBEDO(2)          ! Surface solar albedo array 1)No Snow 2)Snow             []
     REAL(r64) EPSLN(2)           ! Surface emissivity array 1)No snow 2)Snow               []
     REAL(r64) VEGHT(2)           ! Vegetation height 1)No snow 2)Snow                    [cm]
     REAL(r64) TIN(2)             ! Indoor set point temperature in either                 [C]
                             ! heating or cooling mode
     REAL(r64) HIN(6)             ! Inside heat transfer coefficients                [W/m^2/K]
                             ! Convection Only:
                             ! 1)Q Downward 2)Q Upward 3)Q Horizontal
                             ! Convection and Radiation
                             ! 4)Q Downward 5)Q Upward 6)Q Horizontal


     CHARACTER *5 INSFULL    ! T/F: If insulated on the exterior, does                 []
                             ! insulation extend the full length of the
                             ! foundation wall?
     CHARACTER *5 RSNOW      ! True if snow cover model is on                          []
     CHARACTER *5 COND       ! True if the basement is conditioned mechanically        []
     CHARACTER *5 PET        ! True if evapotranspiration is on                        []
     CHARACTER *5 FIXBC      ! True if a fixed lower boundary condition is on          []

     INTEGER AHH             ! Annual Heating Hours for a given location               []
     INTEGER ACH             ! Annual Cooling Hours for a given location               []

     TSTEP=SimParams%TSTEP
     F=SimParams%F

     FIXBC=BCS%FIXBC

     PET=SP%PET
     VEGHT=SP%VEGHT
     EPSLN=SP%EPSLN
     ALBEDO=SP%ALBEDO

     RSNOW=Insul%RSNOW
     RCEIL=Insul%RCEIL
     RSILL=Insul%RSILL
     RSID=Insul%RSID
     RINT=Insul%RINT
     REXT=Insul%REXT
     INSFULL=Insul%INSFULL

     HIN=Interior%HIN
     TIN=Interior%TIN
     COND=Interior%COND

     ACH=SiteInfo%ACH
     AHH=SiteInfo%AHH
     ELEV=SiteInfo%ELEV
     MSTD=SiteInfo%MSTD
     LAT=SiteInfo%LAT
     LONG=SiteInfo%LONG
     IF (.not. SameString(ComBldg,'FALSE')) THEN
       WRITE (InputEcho,300) RUNID,TSTEP,F,trim(LocationName),LONG,LAT,MSTD,ELEV,TBasement
300    FORMAT ('3-D BASEMENT MODEL INPUT SUMMARY:'//                         &
       & 5X,'RUN IDENTIFIER:',2X,A6,5X,'TIME-STEP:',2X,F4.0,5X,              &
       & 'F:',2X,F4.2/5X,'WEATHER FILE ID:',2X,A/                           &
       & 5X,'LONGITUDE (DEG):',2X,F6.1,5X,'LATITUDE (DEG):',2X,              &
       & F5.1/                                                               &
       & 5X,'STANDARD TIME MERIDIAN (DEG):',2X,F5.1,                         &
       & 5X,'ELEVATION (M):',2X,F6.1/                                        &
       & 5X,'OPERATING TEMPERATURE (C):',(2X,F5.2))
     ELSE
     WRITE (InputEcho,301) RUNID,TSTEP,F,trim(LocationName),LONG,LAT,MSTD,ELEV,AHH, &
       &     ACH,TDBH,TDBC
301    FORMAT ('3-D BASEMENT MODEL INPUT SUMMARY:'//                         &
       & 5X,'RUN IDENTIFIER:',2X,A6,5X,'TIME-STEP:',2X,F4.0,5X,              &
       & 'F:',2X,F4.2/5X,'WEATHER FILE ID:',2X,A/                           &
       & 5X,'LONGITUDE (DEG):',2X,F5.1,5X,'LATITUDE (DEG):',2X,              &
       & F5.1/                                                               &
       & 5X,'STANDARD TIME MERIDIAN (DEG):',2X,F5.1,                         &
       & 5X,'ELEVATION (M):',2X,F6.1/                                        &
       & 5X,'ANNUAL HEATING/COOLING HOURS:',2(2X,I4)/                        &
       & 5X,'OPERATING TEMPERATURES (WERLING) (C):',2(2X,F5.2))
     END IF

     WRITE (InputEcho,400) REXT, RINT, RSID, RSILL, RCEIL
400  FORMAT (/'CHARATERISTICS OF FOUNDATION INSULATION:'//                   &
     & 5X,'R-VALUE OF EXTERIOR FOUNDATION WALL INSULATION:',2X,              &
     & F5.2/                                                                 &
     & 5X,'R-VALUE OF INTERIOR FOUNDATION WALL INSULATION:',2X,              &
     & F5.2/                                                                 &
     & 5X,'R-VALUE OF EXTERIOR SIDING:',2X,F5.2/                             &
     & 5X,'R-VALUE OF SILL BOX INSULATION:',2X,F5.2/                         &
     & 5X,'R-VALUE OF BASEMENT CEILING INSULATION:',2X,F5.2)

     IF (.not. SameString(INSFULL,'FALSE')) THEN
       WRITE (InputEcho,500)
     ELSE
       WRITE (InputEcho,600)
     END IF
500  FORMAT (/5X,'EXTERIOR INSULATION IS FULL LENGTH')
600  FORMAT (/5X,'IF INSULATED ON EXTERIOR, ','INSULATION IS PARTIAL LENGTH')

     WRITE (InputEcho,700) ALBEDO(1), ALBEDO(2), EPSLN(1), EPSLN(2),         &
           & VEGHT(1), VEGHT(2), RHO(4), CP(4), TCON(4)
700  FORMAT (/'SOIL AND SURFACE PROPERTIES:'//                               &
     & 5X,'ALBEDO:'/5X,'NO SNOW:',2X,F5.2,5X,'SNOW:',2X,F5.2//               &
     & 5X,'LONG-WAVE EMISSIVITY:'/5X,'NO SNOW:',2X,F5.2,5X,                  &
     & 'SNOW:',2X,F5.2//5X,'VEGETATION HEIGHT (CM):'/                         &
     & 5X,'NO SNOW:',2X,F5.2,5X,'SNOW:',2X,F5.2//                            &
     & 5X,'DENSITY (KG/M**3):',2X,F7.2,                                      &
     & 5X,'SPECIFIC HEAT (J/KG/K):',2X,F6.2/                                 &
     & 5X,'THERMAL CONDUCTIVITY (W/M/K):',2X,F6.2)

     IF (.not. SameString(RSNOW,'FALSE')) THEN
       WRITE (InputEcho,800)
     ELSE
       WRITE (InputEcho,900)
     END IF
800  FORMAT (/5X,'GROUND SNOW COVER MODEL ON')
900  FORMAT (/5X,'GROUND SNOW COVER MODEL OFF')

     WRITE (InputEcho,1000) RHO(1), CP(1), TCON(1), ACEIL, AFLOOR, PERIM,    &
     & ARIM, ASILL, AWALL, XFACE(0), XFACE(IBASE+3), YFACE(0),               &
     & YFACE(JBASE+3), ZFACE(KBASE)

1000 FORMAT (/'FOUNDATION WALL MATERIAL PROPERTIES AND ',                    &
     & 'BASEMENT DIMENSIONS:'//5X,'DENSITY (KG/M**3):',2X,                   &
     & F7.2,                                                                 &
     & 5X,'SPECIFIC HEAT (J/KG/K):',2X,F6.2/                                 &
     & 5X,'THERMAL CONDUCTIVITY (W/M/K):',2X,F6.2/                           &
     & 5X,'CEILING AREA (M**2):',2X,F9.2/                                    &
     & 5X,'FLOOR AREA (M**2):',2X,F9.2,5X,'PERIMETER (M):',2X,               &
     & F9.2/5X,'RIM JOIST AREA (M**2):',2X,F9.2/                             &
     & 5X,'SILL PLATE AREA (M**2):',2X,F9.2/                                 &
     & 5X,'WALL AREA (M**2):',2X,F9.2/                                       &
     & 5X,'MIN X (M):',2X,F6.1,5X,'MAX X (M):',2X,F6.1/                      &
     & 5X,'MIN Y (M):',2X,F6.1,5X,'MAX Y (M):',2X,F6.1,/                     &
     & 5X,'HEIGHT OF BASEMENT BELOW-GRADE (M):',2X,F6.1)

     IF(.not. SameString(ComBldg,'FALSE'))THEN
       WRITE (InputEcho,1099) TBasement
     ELSE
       IF (.not. SameString(COND,'FALSE')) THEN
         WRITE (InputEcho,1100) TIN(1), TIN(2)
       ELSE
         WRITE (InputEcho,1200)
       END IF
     END IF
1099 FORMAT (/'CONSTANT BASEMENT TEMPERATURE (C):',5X,F5.2)
1100 FORMAT (/'CONDITIONED BASEMENT INDOOR TEMPERATURE (C):'                 &
     & //5X,'HEATING SEASON:',2X,F5.2,5X,'COOLING SEASON:',2X,F5.2)
1200 FORMAT (/'UNCONDITIONED BASEMENT,                                       &
     & INDOOR TEMPERATURE DETERMINED BY HEAT BALANCE')

     WRITE (InputEcho,1300) HIN(4), HIN(5), HIN(6)

1300 FORMAT (/'BOUNDARY CONDITIONS:'//5X,'INSIDE HEAT TRANSFER',             &
     & ' COEFFICIENTS, CONVECTION AND RADIATION (W/M**2/K):'/                &
     & 5X,'Q DOWNWARD:',2X,F6.2,5X,'Q UPWARD:',2X,F6.2,5X,                   &
     &'Q HORIZONTAL:',2X,F6.2)

     WRITE (InputEcho,1500)

1500 FORMAT (/5X,'BASEMENT RADIATION MODEL OFF')

     IF (.not. SameString(PET,'FALSE')) THEN
       WRITE (InputEcho,1600)
     ELSE
       WRITE (InputEcho,1700)
     END IF
1600 FORMAT (5X,'POTENTIAL EVAPOTRANSPIRATION SURFACE BOUNDARY')
1700 FORMAT (5X,'ZERO EVAPORATION SURFACE BOUNDARY')

     IF (.not. SameString(FIXBC,'FALSE')) THEN
       WRITE (InputEcho,1800)
     ELSE
       WRITE (InputEcho,1900)
     END IF

1800 FORMAT (5X,'FIXED TEMPERATURE LOWER BOUNDARY')
1900 FORMAT (5X,'ZERO FLUX LOWER BOUNDARY')

     WRITE(InputEcho,2000) (XFACE(COUNT1), COUNT1=0,NX)
2000 FORMAT (/'CELL X-COORDINATES (M):'/10(10F10.2/))
     WRITE(InputEcho,2100) (YFACE(COUNT1), COUNT1=0,NY)
2100 FORMAT (/'CELL Y-COORDINATES (M):'/10(10F10.2/))
     WRITE(InputEcho,2200) (ZFACE(COUNT1), COUNT1=-NZAG,NZBG)
2200 FORMAT (/'CELL Z-COORDINATES (M):'/10(10F10.2/))

     CLOSE(InputEcho)
     RETURN
END SUBROUTINE PrelimOutput

!******************************  Retrieve Weather Data **********************************
SUBROUTINE GetWeatherData(Today)
! SUBROUTINE INFORMATION:
      !       AUTHOR         Edward Clements
      !       DATE WRITTEN   December 7, 1999
      !       MODIFIED       April 28, 2001
      !       MODIFIED BY    na
      !       RE-ENGINEERED  na
      !     VERSION NUMBER 2.0
      !
      ! PURPOSE OF THIS MODULE:
      ! This subroutine initializes variables for output
      ! in the final year of the simulation

      ! METHODOLOGY EMPLOYED:
      ! Standard EnergyPlus Subroutine formatting

      ! REFERENCES: BASE3D Version 1.0, Cynthia A. Cogil. 23 July, 1996.
      !

      ! OTHER NOTES: none
USE BasementSimData
IMPLICIT NONE
INTEGER Today
!     READ(Weather,*) TodaysWeather
     IF (Today > 365) RETURN

     TodaysWeather%TDB=FullYearWeather(Today)%TDB
     TodaysWeather%TWB=FullYearWeather(Today)%TWB
     TodaysWeather%PBAR=FullYearWeather(Today)%PBAR
     TodaysWeather%HRAT=FullYearWeather(Today)%HRAT
     TodaysWeather%WND=FullYearWeather(Today)%WND
     TodaysWeather%RBEAM=FullYearWeather(Today)%RBEAM
     TodaysWeather%RDIFH=FullYearWeather(Today)%RDIFH
     TodaysWeather%ISNW=FullYearWeather(Today)%ISNW
     TodaysWeather%DSNOW=FullYearWeather(Today)%DSNOW

     RETURN
END SUBROUTINE GetWeatherData

!****************************  Basement Heat Balance  ***********************************
SUBROUTINE BasementHeatBalance(TB,TC,TF,TRS,TRW,TSS,TSW,TWS,TWW,HIN,DX,DY,DZ,       &
&          XDIM,YDIM,ZDIM)
! SUBROUTINE INFORMATION:
     !       AUTHOR         Cynthia Cogil
     !       DATE WRITTEN   May 22, 1997
     !       MODIFIED       December 9, 1999, August 17, 2000
     !       MODIFIED BY    Edward Clements
     !       RE-ENGINEERED  na
     !       VERSION NUMBER 2.0
     !
     ! PURPOSE OF THIS SUBROUTINE:
     ! This subroutine calculates the basement temperature when the basement is
     ! either unconditioned or in free cooling. In order to develop a generalized
     ! model, only convection is considered.

     ! METHODOLOGY EMPLOYED:
     ! Standard EnergyPlus Subroutine formatting

     ! REFERENCES: HEATBAL Version 1.0, Cynthia A. Cogil. 22 May, 1997.
     !

     ! OTHER NOTES: none
     USE BasementSimData
     !USE MSFLIB
     IMPLICIT NONE

!*** DECLARATIONS:
     INTEGER XDIM,YDIM,ZDIM
     REAL(r64) C1,C2,DX(0:100),DY(0:100),DZ(-35:100),F1,F2,HIN(6),HINC,HINF,    &
          & HINSS,HINSW,RS1,RS2,RW1,RW2,SS1,SS2,SW1,SW2,TB,WS1,WS2,WW1,WW2
     REAL(r64) QC1
     DIMENSION QC1(0:XDIM,0:YDIM)
     REAL(r64) QC2
     DIMENSION QC2(0:XDIM,0:YDIM)
     REAL(r64) QF1
     DIMENSION QF1(0:XDIM,0:YDIM)
     REAL(r64) QF2
     DIMENSION QF2(0:XDIM,0:YDIM)
     REAL(r64) QRS1
     DIMENSION QRS1(0:YDIM)
     REAL(r64) QRS2
     DIMENSION QRS2(0:YDIM)
     REAL(r64) QRW1
     DIMENSION QRW1(0:XDIM)
     REAL(r64) QRW2
     DIMENSION QRW2(0:XDIM)
     REAL(r64) QSS1
     DIMENSION QSS1(0:XDIM,0:YDIM)
     REAL(r64) QSS2
     DIMENSION QSS2(0:XDIM,0:YDIM)
     REAL(r64) QSW1
     DIMENSION QSW1(0:XDIM,0:YDIM)
     REAL(r64) QSW2
     DIMENSION QSW2(0:XDIM,0:YDIM)
     REAL(r64) QWS1
     DIMENSION QWS1(0:YDIM,-35:ZDIM)
     REAL(r64) QWS2
     DIMENSION QWS2(0:YDIM,-35:ZDIM)
     REAL(r64) QWW1
     DIMENSION QWW1(0:XDIM,-35:ZDIM)
     REAL(r64) QWW2
     DIMENSION QWW2(0:XDIM,-35:ZDIM)
     REAL(r64) TC
     DIMENSION TC(0:XDIM,0:YDIM)
     REAL(r64) TF
     DIMENSION TF(0:XDIM,0:YDIM)
     REAL(r64) TRS
     DIMENSION TRS(0:YDIM)
     REAL(r64) TRW
     DIMENSION TRW(0:XDIM)
     REAL(r64) TSS
     DIMENSION TSS(0:XDIM,0:YDIM)
     REAL(r64) TSW
     DIMENSION TSW(0:XDIM,0:YDIM)
     REAL(r64) TWS
     DIMENSION TWS(0:YDIM,-35:ZDIM)
     REAL(r64) TWW
     DIMENSION TWW(0:XDIM,-35:ZDIM)

!*** INITIALIZE THE SUM OF THE SURFACE HEAT FLUXES

     C1=0.
     C2=0.
     F1=0.
     F2=0.
     RS1=0.
     RS2=0.
     RW1=0.
     RW2=0.
     SS1=0.
     SS2=0.
     SW1=0.
     SW2=0.
     WS1=0.
     WS2=0.
     WW1=0.
     WW2=0.
     DO COUNT1=0,IBASE+1
       DO COUNT2=0,JBASE+1
         IF (TB.GE.TC(COUNT1,COUNT2)) THEN
           HINC=HIN(2)
         ELSE
           HINC=HIN(1)
         END IF
         QC1(COUNT1,COUNT2)=HINC*DX(COUNT1)*DY(COUNT2)*TC(COUNT1,COUNT2)
         QC2(COUNT1,COUNT2)=HINC*DX(COUNT1)*DY(COUNT2)
         C1=C1+QC1(COUNT1,COUNT2)
         C2=C2+QC2(COUNT1,COUNT2)
       END DO
     END DO
     DO COUNT1=0,IBASE-1
       DO COUNT2=0,JBASE-1
         IF (TB.GT.TF(COUNT1,COUNT2)) THEN
           HINF=HIN(1)
         ELSE
           HINF=HIN(2)
         END IF
         QF1(COUNT1,COUNT2)=HINF*DX(COUNT1)*DY(COUNT2)*TF(COUNT1,COUNT2)
         QF2(COUNT1,COUNT2)=HINF*DX(COUNT1)*DY(COUNT2)
         F1=F1+QF1(COUNT1,COUNT2)
         F2=F2+QF2(COUNT1,COUNT2)
       END DO
     END DO
     DO COUNT2=0,JBASE+1
       QRS1(COUNT2)=HIN(3)*DY(COUNT2)*DZ(-NZAG+1)*TRS(COUNT2)
       QRS2(COUNT2)=HIN(3)*DY(COUNT2)*DZ(-NZAG+1)
       RS1=RS1+QRS1(COUNT2)
       RS2=RS2+QRS2(COUNT2)
     END DO
     DO COUNT1=0,IBASE+1
       QRW1(COUNT1)=HIN(3)*DX(COUNT1)*DZ(-NZAG+1)*TRW(COUNT1)
       QRW2(COUNT1)=HIN(3)*DX(COUNT1)*DZ(-NZAG+1)
       RW1=RW1+QRW1(COUNT1)
       RW2=RW2+QRW2(COUNT1)
     END DO
     DO COUNT1=IBASE,IBASE+1
       DO COUNT2=0,JBASE+1
         IF (TB.GT.TSS(COUNT1,COUNT2)) THEN
            HINSS=HIN(1)
          ELSE
            HINSS=HIN(2)
          END IF
          QSS1(COUNT1,COUNT2)=HINSS*DX(COUNT1)*DY(COUNT2)*TSS(COUNT1,COUNT2)
          QSS2(COUNT1,COUNT2)=HINSS*DX(COUNT1)*DY(COUNT2)
          SS1=SS1+QSS1(COUNT1,COUNT2)
          SS2=SS2+QSS2(COUNT1,COUNT2)
        END DO
      END DO

      DO COUNT2=JBASE,JBASE+1
        DO COUNT1=0,IBASE-1
          IF (TB.GT.TSW(COUNT1,COUNT2)) THEN
            HINSW=HIN(1)
          ELSE
            HINSW=HIN(2)
          END IF
          QSW1(COUNT1,COUNT2)=HINSW*DX(COUNT1)*DY(COUNT2)*TSW(COUNT1,COUNT2)
          QSW2(COUNT1,COUNT2)=HINSW*DX(COUNT1)*DY(COUNT2)
          SW1=SW1+QSW1(COUNT1,COUNT2)
          SW2=SW2+QSW2(COUNT1,COUNT2)
        END DO
      END DO

      DO COUNT2=0,JBASE-1
        DO COUNT3=-NZAG+2,KBASE-1
          QWS1(COUNT2,COUNT3)=HIN(3)*DY(COUNT2)*DZ(COUNT3)*TWS(COUNT2,COUNT3)
          QWS2(COUNT2,COUNT3)=HIN(3)*DY(COUNT2)*DZ(COUNT3)
          WS1=WS1+QWS1(COUNT2,COUNT3)
          WS2=WS2+QWS2(COUNT2,COUNT3)
        END DO
      END DO

      DO COUNT1=0,IBASE-1
        DO COUNT3=-NZAG+2,KBASE-1
          QWW1(COUNT1,COUNT3)=HIN(3)*DX(COUNT1)*DZ(COUNT3)*TWW(COUNT1,COUNT3)
          QWW2(COUNT1,COUNT3)=HIN(3)*DX(COUNT1)*DZ(COUNT3)
          WW1=WW1+QWW1(COUNT1,COUNT3)
          WW2=WW2+QWW2(COUNT1,COUNT3)
        END DO
      END DO

      TB=(C1+F1+RS1+RW1+SS1+SW1+WS1+WW1)/(C2+F2+RS2+RW2+SS2+SW2+WS2+WW2)
      RETURN

END SUBROUTINE BasementHeatBalance

!**********************  Three Dimensional Tridiagonal Matrix Solver  *******************
SUBROUTINE TRIDI3D (AA, BB, CC, RR, N, X)

!***  THIS SUBROUTINE SOLVES A SYSTEM OF LINEAR EQUATIONS HAVING A
!***  TRIDIAGONAL COEFFICIENT MATRIX BY THE THOMAS ALGORITHM.  THIS
!***  VERSION IS TAKEN FROM "NUMERICAL MARCHING TECHNIQUES FOR FLUID
!***  FLOWS WITH HEAT TRANSFER" BY ROBERT W. HORNBECK, NASA, 1973.

!***  A, B, AND C ARE, RESPECTIVELY, THE LOWER, MAJOR, AND UPPER
!***  DIAGONAL COEFFICIENT VALUES.  FOR A SYSTEM OF N EQUATIONS,
!***  INDICES OF A RUN FROM 2 TO N, INDICES OF B FROM 1 TO N, AND
!***  INDICES OF C FROM 1 TO N-1.  R IS THE RIGHT-HAND SIDE VECTOR
!***  OF THE SYSTEM.  THE UNKNOWN VECTOR IS RETURNED AS X.

!***  DECLARATIONS:

      !REAL(r64) AA(50), BB(50), CC(50), RR(50), X(50), BN
     REAL(r64) AA, BB, CC, RR, X, BN
      INTEGER N
      DIMENSION AA(N)
      DIMENSION BB(N)
      DIMENSION CC(N)
      DIMENSION RR(N)
      DIMENSION X(N)

      INTEGER L, LL

!***  VARIABLE DESCRIPTIONS:

!***  A,B,C,X,R     ARRAYS OF COEFFICIENTS AND VARIABLES OF
!***                TRIDIAGONAL MATIX
!***  BN
!***  I             INTERGER COUNTER FOR DO LOOP
!***  II            -I+N-2
!***  N             NUMBER OF CELL FACES IN POSITIVE Z DIRECTION

     AA(N)=AA(N)/BB(N)
     RR(N)=RR(N)/BB(N)
     DO L=2,N
       LL=-L+N+2
       BN=1.d0/(BB(LL-1)-AA(LL)*CC(LL-1))
       AA(LL-1)=AA(LL-1)*BN
       RR(LL-1)=(RR(LL-1)-CC(LL-1)*RR(LL))*BN
     END DO

     X(1)=RR(1)
     DO L=2,N
       X(L)=RR(L)-AA(L)*X(L-1)
     END DO

END SUBROUTINE TRIDI3D

!************************************ January 21st Output  ******************************
SUBROUTINE Jan21Output(IHR,TC,TF,TRS,TRW,TSS,TSW,TWS,TWW,XDIM,YDIM,ZDIM,XC,YC,ZC)
! SUBROUTINE INFORMATION:
      !       AUTHOR         Edward Clements
      !       DATE WRITTEN   December 13, 1999
      !       MODIFIED       August 17, 2000
      !       MODIFIED BY    na
      !       RE-ENGINEERED  na
      !     VERSION NUMBER 2.0
      !
      ! PURPOSE OF THIS SUBROUTINE:
      ! This subroutine writes the January 21st output from the BasementSimulator
      ! subroutine

      ! METHODOLOGY EMPLOYED:
      ! Standard EnergyPlus Subroutine formatting

      ! REFERENCES: BASE3D Version 1.0, Cynthia A. Cogil. 22 May, 1997.
      !

      ! OTHER NOTES: none
USE BasementSimData
IMPLICIT NONE

!*** Declarations:  For explanations, see BasementSimulator
     INTEGER IHR,XDIM,YDIM,ZDIM !IBASE,JBASE,KBASE,NZAG,XDIM,YDIM,ZDIM
     REAL(r64) TC
     DIMENSION TC(0:XDIM,0:YDIM)
     REAL(r64) TF
     DIMENSION TF(0:XDIM,0:YDIM)
     REAL(r64) TRS
     DIMENSION TRS(0:YDIM)
     REAL(r64) TRW
     DIMENSION TRW(0:XDIM)
     REAL(r64) TSS
     DIMENSION TSS(0:XDIM,0:YDIM)
     REAL(r64) TSW
     DIMENSION TSW(0:XDIM,0:YDIM)
     REAL(r64) TWS
     DIMENSION TWS(0:YDIM,-35:ZDIM)
     REAL(r64) TWW
     DIMENSION TWW(0:XDIM,-35:ZDIM)
     REAL(r64) XC(0:100)
     REAL(r64) YC(0:100)
     REAL(r64) ZC(-35:100)


!*** January 21st Ceiling cell output
     DO COUNT1=0,IBASE+1
       DO COUNT2=0,JBASE+1
         WRITE (Ceil121,*) IHR,XC(COUNT1),YC(COUNT2),TC(COUNT1,COUNT2)
       END DO
     END DO

!*** January 21st Floor cell output
     DO COUNT1=0,IBASE-1
       DO COUNT2=0,JBASE-1
         WRITE (Flor121,*) IHR,XC(COUNT1),YC(COUNT2),TF(COUNT1,COUNT2)
       END DO
     END DO

!*** January 21st south rim joist output
     DO COUNT2=0,JBASE+1
       WRITE (RMJS121,*) IHR,YC(COUNT2),TRS(COUNT2)

     END DO

!*** January 21st west rim joist output
     DO COUNT1=0,IBASE+1
       WRITE (RMJW121,*) IHR,XC(COUNT1),TRW(COUNT1)
     END DO

!*** January 21st south sill plate output
     DO COUNT1=IBASE,IBASE+1
       DO COUNT2=0,JBASE+1
         WRITE (SILS121,*) IHR,XC(COUNT1),YC(COUNT2),TSS(COUNT1,COUNT2)
       END DO
     END DO

!*** January 21st west sill plate output
     DO COUNT1=0,IBASE-1
       DO COUNT2=JBASE,JBASE+1
         WRITE (SILW121,*) IHR,XC(COUNT1),YC(COUNT2),TSW(COUNT1,COUNT2)
       END DO
     END DO

!*** January 21st south foundation wall temperature array output
     DO COUNT2=0,JBASE-1
       DO COUNT3=-NZAG+2,KBASE-1
         WRITE (WALS121,*) IHR,YC(COUNT2),ZC(COUNT3),TWS(COUNT2,COUNT3)
       END DO
     END DO

!*** January 21st west foundation wall temperature array output
     DO COUNT1=0,IBASE-1
       DO COUNT3=-NZAG+2,KBASE-1
         WRITE (WALW121,*) IHR,XC(COUNT1),ZC(COUNT3),TWW(COUNT1,COUNT3)
       END DO
     END DO
     RETURN
END SUBROUTINE Jan21Output

!**************************  Building Load Output  **************************************
SUBROUTINE OutputLoads(HLOAD,CLOAD,CONDITION)
! SUBROUTINE INFORMATION:
      !       AUTHOR         Edward Clements
      !       DATE WRITTEN   December 13, 1999
      !       MODIFIED       na
      !       MODIFIED BY    na
      !       RE-ENGINEERED  na
      !     VERSION NUMBER 1.0
      !
      ! PURPOSE OF THIS SUBROUTINE:
      ! This subroutine writes the basement load output from the BasementSimulator
      ! subroutine

      ! METHODOLOGY EMPLOYED:
      ! Standard EnergyPlus Subroutine formatting

      ! REFERENCES: BASE3D Version 1.0, Cynthia A. Cogil. 22 May, 1997.
      !

      ! OTHER NOTES: none
USE BasementSimData
IMPLICIT NONE

!*** Variables definitions
     REAL(r64) HLOAD,CLOAD
     INTEGER CONDITION

     WRITE (LOADFile,3500) HLOAD, CLOAD, CONDITION
3500 FORMAT ('HLOAD:  ',F7.1,2X,'CLOAD:  ',F7.1,2X,'CONDITION:  ',I2)

END SUBROUTINE OutputLoads

!**************************  Main Output  ***********************************************
SUBROUTINE MainOutput(TCMN,TCMX,TFMN,TFMX,TRMN,TRMX,TSMN,TSMX,TWMN,TWMX,QCMN,QCMX, &
&                     QFMN,QFMX,QRMN,QRMX,QSMN,QSMX,QWMN,QWMX,DTCA,DTFA,DTRA,DTSA,DTWA, &
&                     DQCA,DQFA,DQRA,DQSA,DQWA,DTDBA,DTBA)
! SUBROUTINE INFORMATION:
      !       AUTHOR         Edward Clements
      !       DATE WRITTEN   December 13, 1999
      !       MODIFIED       na
      !       MODIFIED BY    na
      !       RE-ENGINEERED  na
      !     VERSION NUMBER 1.0
      !
      ! PURPOSE OF THIS SUBROUTINE:
      ! This subroutine writes the main output from the BasementSimulator
      ! subroutine

      ! METHODOLOGY EMPLOYED:
      ! Standard EnergyPlus Subroutine formatting

      ! REFERENCES: BASE3D Version 1.0, Cynthia A. Cogil. 22 May, 1997.
      !

      ! OTHER NOTES: none
USE BasementSimData
IMPLICIT NONE

!***  Declarations: for explanations, see BasementSimulator

     REAL(r64) TCMN,TCMX,TFMN,TFMX,TRMN,TRMX,TSMN,TSMX,TWMN,TWMX,QCMN,QCMX,QFMN,QFMX,QRMN, &
     &    QRMX,QSMN,QSMX,QWMN,QWMX,DTCA,DTFA,DTRA,DTSA,DTWA,DQCA,DQFA,DQRA,DQSA,DQWA, &
     &    DTDBA,DTBA

 !    INTEGER IDAY



     WRITE(DOUT,3600) IDAY,TCMN,TCMX,TFMN,TFMX,TRMN,TRMX,TSMN,TSMX,      &
     & TWMN,TWMX,QCMN,QCMX,QFMN,QFMX,QRMN,QRMX,QSMN,QSMX,              &
     & QWMN,QWMX,DTCA,DTFA,DTRA,DTSA,DTWA,DQCA,DQFA,DQRA,DQSA,DQWA,    &
     & DTDBA,DTBA
3600 FORMAT(I3/10(2X,F9.1)/10(2X,F9.1)/10(2X,F9.1)/2(2X,F5.1))
     RETURN
END SUBROUTINE MainOutput

!***************** Output of House Loads Attributable to Foundation HT  *****************
SUBROUTINE HouseLoadOutput(QHOUSE,EFFECT)
! SUBROUTINE INFORMATION:
      !       AUTHOR         Edward Clements
      !       DATE WRITTEN   April 8, 1999
      !       MODIFIED       na
      !       MODIFIED BY    na
      !       RE-ENGINEERED  na
      !     VERSION NUMBER 1.0
      !
      ! PURPOSE OF THIS SUBROUTINE:
      ! This subroutine writes the house load file

      ! METHODOLOGY EMPLOYED:
      ! Standard EnergyPlus Subroutine formatting

      ! REFERENCES: BASE3D Version 1.0, Cynthia A. Cogil. 22 May, 1997.
      !

      ! OTHER NOTES: none
IMPLICIT NONE
!***  Declarations: for explanations, see BasementSimulator
     REAL(r64) QHOUSE
     INTEGER EFFECT

     WRITE (QHouseFile,*) QHOUSE, EFFECT
     RETURN
END SUBROUTINE HouseLoadOutput

!***************************** 21st of each month: Output  ******************************
SUBROUTINE Day21Output(IMON,IBASE,JBASE,KBASE,NZAG,DTRW21,DTRS21,DTC21,         &
           & DTWW21,DTWS21,DTSW21,DTSS21,DQWW21,DQWS21,DQSW21,DQSS21,DQRW21,    &
           & DQRS21,DQF21,DQC21,DTF21,TV1,TV2,TV3,NXM1,NZBGM1,XDIM,YDIM,ZDIM,   &
           & XC,YC,ZC)
! SUBROUTINE INFORMATION:
      !       AUTHOR         Edward Clements
      !       DATE WRITTEN   December 13, 1999
      !       MODIFIED       June 14, 2000
      !       MODIFIED BY    na
      !       RE-ENGINEERED  na
      !     VERSION NUMBER 1.0
      !
      ! PURPOSE OF THIS SUBROUTINE:
      ! This subroutine writes the main output from the BasementSimulator
      ! subroutine

      ! METHODOLOGY EMPLOYED:
      ! Standard EnergyPlus Subroutine formatting

      ! REFERENCES: BASE3D Version 1.0, Cynthia A. Cogil. 22 May, 1997.
      !

      ! OTHER NOTES: none
IMPLICIT NONE

!***  Declarations: for explanations, see BasementSimulator
     INTEGER IMON,IBASE,JBASE,KBASE,NZAG,NZBGM1,NXM1,XDIM,YDIM,ZDIM
     REAL(r64) DTRW21
     DIMENSION DTRW21(0:XDIM)
     REAL(r64) DTRS21
     DIMENSION DTRS21(0:YDIM)
     REAL(r64) DTF21
     DIMENSION DTF21(0:XDIM,0:YDIM)
     REAL(r64) DTC21
     DIMENSION DTC21(0:XDIM,0:YDIM)
     REAL(r64) DTWW21
     DIMENSION DTWW21(0:XDIM,-35:ZDIM)
     REAL(r64) DTWS21
     DIMENSION DTWS21(0:YDIM,-35:ZDIM)
     REAL(r64) DTSW21
     DIMENSION DTSW21(0:YDIM,0:ZDIM)
     REAL(r64) DTSS21
     DIMENSION DTSS21(0:XDIM,0:ZDIM)
     REAL(r64) DQWW21
     DIMENSION DQWW21(0:XDIM,-35:ZDIM)
     REAL(r64) DQWS21
     DIMENSION DQWS21(0:YDIM,-35:ZDIM)
     REAL(r64) DQSW21
     DIMENSION DQSW21(0:XDIM,0:ZDIM)
     REAL(r64) DQSS21
     DIMENSION DQSS21(0:YDIM,0:ZDIM)
     REAL(r64) DQRW21
     DIMENSION DQRW21(0:XDIM)
     REAL(r64) DQRS21
     DIMENSION DQRS21(0:YDIM)
     REAL(r64) DQF21
     DIMENSION DQF21(0:XDIM,0:YDIM)
     REAL(r64) DQC21
     DIMENSION DQC21(0:XDIM,0:YDIM)
     REAL(r64) TV1(0:100,-35:100)
     REAL(r64) TV2(0:100,-35:100)
     REAL(r64) TV3(0:100,-35:100)
     REAL(r64) XC(0:100)
     REAL(r64) YC(0:100)
     REAL(r64) ZC(-35:100)



!*** Daily average surface temperatures on the 21st day of each month
!*** Ceiling
     DO COUNT1=0,IBASE+1
       DO COUNT2=0,JBASE+1
         WRITE(CeilD21,*) IMON,XC(COUNT1),YC(COUNT2),   &
         &    DTC21(COUNT1,COUNT2),DQC21(COUNT1,COUNT2)
       END DO
     END DO

!*** Floor
     DO COUNT1=0,IBASE-1
       DO COUNT2=0,JBASE-1
         WRITE(FlorD21,*) IMON,XC(COUNT1),YC(COUNT2),   &
         &    DTF21(COUNT1,COUNT2),DQF21(COUNT1,COUNT2)
       END DO
     END DO

!*** South rim joist
     DO COUNT2=0,JBASE+1
       WRITE(RMJSD21,*) IMON,YC(COUNT2),DTRS21(COUNT2),DQRS21(COUNT2)
     END DO

!*** West rim joist
     DO COUNT1=0,IBASE+1
       WRITE(RMJWD21,*) IMON,XC(COUNT1),DTRW21(COUNT1),DQRW21(COUNT1)
     END DO

!*** South sill box
     DO COUNT1=IBASE,IBASE+1
       DO COUNT2=0,JBASE+1
         WRITE(SILSD21,*) IMON,XC(COUNT1),YC(COUNT2),   &
         &    DTSS21(COUNT1,COUNT2),DQSS21(COUNT1,COUNT2)
       END DO
     END DO

!*** West sill box
     DO COUNT1=0,IBASE-1
       DO COUNT2=JBASE,JBASE+1
         WRITE(SILWD21,*) IMON,XC(COUNT1),YC(COUNT2),   &
         &    DTSW21(COUNT1,COUNT2),DQSW21(COUNT1,COUNT2)
       END DO
     END DO

!*** South foundation wall
     DO COUNT2=0,JBASE-1
       DO COUNT3=-NZAG+2,KBASE-1
         WRITE(WALSD21,*) IMON,YC(COUNT2),ZC(COUNT3),   &
         &    DTWS21(COUNT2,COUNT3),DQWS21(COUNT2,COUNT3)
       END DO
     END DO

!*** West foundation wall
     DO COUNT1=0,IBASE-1
       DO COUNT3=-NZAG+2,KBASE-1
         WRITE(WALWD21,*) IMON,YC(COUNT2),ZC(COUNT3),   &
         &    DTWW21(COUNT1,COUNT3),DQWW21(COUNT1,COUNT3)
       END DO
     END DO

!*** OUTPUT X-Z CELL TEMPERATURES ON THE 21ST OF EACH MONTH
!*** Temperatures at Y=0
     DO COUNT1=0,NXM1
       DO COUNT3=-NZAG,NZBGM1
         WRITE(XZYZero,*) IMON,XC(COUNT1),ZC(COUNT3),TV1(COUNT1,COUNT3)
       END DO
     END DO

!*** Temperatures at Y=INT(JBASE/2)
     DO COUNT1=0,NXM1
       DO COUNT3=-NZAG,NZBGM1
         WRITE(XZYHalf,*) IMON,XC(COUNT1),ZC(COUNT3),TV2(COUNT1,COUNT3)
       END DO
     END DO

!*** Temperatures at Y=JBASE
     DO COUNT1=0,NXM1
       DO COUNT3=-NZAG,NZBGM1
         WRITE(XZYFull,*) IMON,XC(COUNT1),ZC(COUNT3),TV3(COUNT1,COUNT3)
       END DO
     END DO
     RETURN
END SUBROUTINE Day21Output

!********************************  Daily Output  ****************************************
SUBROUTINE DailyOutput(DQCSUM,DQFSUM,DQRSUM,DQSSUM,DQWSUM)
! SUBROUTINE INFORMATION:
      !       AUTHOR         Edward Clements
      !       DATE WRITTEN   December 13, 1999
      !       MODIFIED       na
      !       MODIFIED BY    na
      !       RE-ENGINEERED  na
      !     VERSION NUMBER 1.0
      !
      ! PURPOSE OF THIS SUBROUTINE:
      ! This subroutine writes the daily output from the BasementSimulator
      ! subroutine

      ! METHODOLOGY EMPLOYED:
      ! Standard EnergyPlus Subroutine formatting

      ! REFERENCES: BASE3D Version 1.0, Cynthia A. Cogil. 22 May, 1997.
      !

      ! OTHER NOTES: none
USE BasementSimData
IMPLICIT NONE

!***  Declarations
 !     INTEGER IDAY
      REAL(r64) DQCSUM,DQFSUM,DQRSUM,DQSSUM,DQWSUM

!***  OUTPUT DAILY NET SURFACE HEAT LOSSES
      WRITE(DYFLX,4900) IDAY
4900  FORMAT('DAILY NET SURFACE HEAT LOSSES [W-H] DAY=  ',I3)

      WRITE(DYFLX,5000) DQCSUM,DQFSUM,DQRSUM,DQSSUM,DQWSUM
5000  FORMAT('CEILING:  ',8X,F9.1/,'FLOOR:  ',10X,F9.1/,        &
      & 'RIM JOIST:  ',6X,F9.1/,'SILL PLATE:  ',5X,F9.1/,       &
      & 'FOUNDATION WALL:  ',F9.1/)
      RETURN
END SUBROUTINE DailyOutput

!*********************************  Yearly Output  **************************************
SUBROUTINE YearlyOutput(YHLOAD,YCLOAD,YQCSUM,YQFSUM,YQRSUM,YQSSUM,YQWSUM,YQBSUM)
! SUBROUTINE INFORMATION:
      !       AUTHOR         Edward Clements
      !       DATE WRITTEN   December 13, 1999
      !       MODIFIED       na
      !       MODIFIED BY    na
      !       RE-ENGINEERED  na
      !     VERSION NUMBER 1.0
      !
      ! PURPOSE OF THIS SUBROUTINE:
      ! This subroutine writes the yearly output from the BasementSimulator
      ! subroutine

      ! METHODOLOGY EMPLOYED:
      ! Standard EnergyPlus Subroutine formatting

      ! REFERENCES: BASE3D Version 1.0, Cynthia A. Cogil. 22 May, 1997.
      !

      ! OTHER NOTES: none
USE BasementSimData
IMPLICIT NONE

!***  Declarations: for explanations, see BasementSimulator
      REAL(r64) YHLOAD,YCLOAD,YQCSUM,YQFSUM,YQRSUM,YQSSUM,YQWSUM,YQBSUM
      WRITE(LOADFile,5100) YHLOAD, YCLOAD
5100  FORMAT('YHLOAD:  ',F15.1,2X,'YCLOAD:  ',F15.1)

!***  OUTPUT THE YEARLY NET SURFACE HEAT LOSSES (W-H)
      WRITE(DYFLX,5200) YQCSUM,YQFSUM,YQRSUM,YQSSUM,YQWSUM,YQBSUM
5200  FORMAT('YEARLY NET SURFACE HEAT LOSSES [W-H]'/                 &
      &  'CEILING:  ',8X,F15.1/,'FLOOR:  ',10X,F15.1/,               &
      &  'RIM JOIST:  ',6X,F15.1/,'SILL PLATE:  ',5X,F15.1/,         &
      &  'FOUNDATION WALL:  ',F15.1/,'BASEMENT:  ',7X,F15.1)
     RETURN
END SUBROUTINE YearlyOutput

!******************************** Initializing Temperatures  ****************************
SUBROUTINE InitializeTemps(NXM1,NZBGM1,NYM1,T)
! SUBROUTINE INFORMATION:
      !       AUTHOR         Edward Clements
      !       DATE WRITTEN   December 13, 1999
      !       MODIFIED       na
      !       MODIFIED BY    na
      !       RE-ENGINEERED  na
      !     VERSION NUMBER 1.0
      !
      ! PURPOSE OF THIS SUBROUTINE:
      ! This subroutine initializes the temperature file used if TREAD='T'

      ! METHODOLOGY EMPLOYED:
      ! Standard EnergyPlus Subroutine formatting

      ! REFERENCES: BASE3D Version 1.0, Cynthia A. Cogil. 22 May, 1997.
      !

      ! OTHER NOTES: none
USE BasementSimData
IMPLICIT NONE
     INTEGER NXM1,NYM1,NZBGM1
     REAL(r64) T(0:100,0:100,-35:100)
!***  Declarations: for explanations, see BasementSimulator

      DO COUNT1=0,NXM1
        DO COUNT2=0,NYM1
          DO COUNT3=-NZAG,NZBGM1
            WRITE (75,*) T(COUNT1,COUNT2,COUNT3)
          END DO
        END DO
      END DO
      RETURN
END SUBROUTINE InitializeTemps

!********************************   AUTOGRIDDING  ***************************************
SUBROUTINE AutoGridding
USE BasementSimData
USE DataGlobals, ONLY: ShowSevereError,ShowContinueError,ShowFatalError
USE General, ONLY: RoundSigDigits
IMPLICIT NONE
! THIS PROGRAM WILL ESTABLISH THE SIMULATION GRID FOR A BASEMENT FOUNDATION
! WHOSE DIMENSIONS ARE INPUT BY THE USER

! MODULE INFORMATION:
        !       AUTHOR         Edward D. Clements
        !       DATE WRITTEN   March 6-7, 2000
        !       MODIFIED       na
        !       MODIFIED       na
        !       RE-ENGINEERED  na

        ! PURPOSE OF THIS MODULE:
        ! This module will establish the simulation grid for a slab on grade foundation
        ! whose dimensions are input by the user.

        ! METHODOLOGY EMPLOYED:
        ! Standard EnergyPlus modular coding

        ! REFERENCES:
        ! na

        ! OTHER NOTES:
        ! na

!*** VARIABLES DECLARATIONS
     REAL(r64) EDGE1          ! X DIRECTION SLAB EDGE                                [m]
     REAL(r64) EDGE1M3        ! X DIRECTION SLAB EDGE - 3 Meters                     [m]
     REAL(r64) EDGE2          ! Y DIRECTION SLAB EDGE                                [m]
     REAL(r64) EDGE2M3        ! Y DIRECTION SLAB EDGE - 3 Meters                     [m]
     REAL(r64) DOMAINEDGEX    ! SOLUTION DOMAIN EDGE (PLACED A CERTAIN DISTANCE      [m]
                         ! AWAY FROM THE SLAB EDGE, TYPICALLY 15m)
     REAL(r64) DOMAINEDGEY    ! SOLUTION DOMAIN EDGE (PLACED A CERTAIN DISTANCE      [m]
                         ! AWAY FROM THE SLAB EDGE, TYPICALLY 15m)

     INTEGER NX1         ! NUMBER OF CELLS BETWEEN THE ORIGIN AND THE SLAB EDGE  []
                         ! MINUS THREE METERS
     INTEGER NX2         ! NUMBER OF CELLS BETWEEN 3m AND 1m AWAY FROM THE WALL  []
                         ! (ON THE INSIDE), CELLS HERE ARE SPACED AT 0.2m
     INTEGER NX3         ! 1 CELL @ 0.6m AWAY FROM THE INSIDE FACE OF THE        []
                         ! BASEMENT WALL
     INTEGER NX4         ! NUMBER OF CELLS BETWEEN 0.6m AND THE WALL FACE @ 0.2m []
     INTEGER NX5         ! THE BASEMENT WALL IS 1 CELL THICK                     []
     INTEGER NX6         ! THE GRAVEL BED IS 1 CELL THICK                        []
     INTEGER NX7         ! CELLS @ 0.25m FOR 1m OUTSIDE OF THE GRAVEL BED        []
     INTEGER NX8         ! CELLS @ 0.5m FOR 1m OUTSIDE OF NX7                    []
     INTEGER NX9         ! 1 CELL @ 3m AWAY FROM THE GRAVEL BED                  []
     INTEGER NX10        ! OUTSIDE 3m AWAY FROM THE GRAVEL BED, CELLS ARE @ 2m   []
     INTEGER NY1         ! NUMBER OF CELLS BETWEEN THE ORIGIN AND THE SLAB EDGE  []
                         ! MINUS THREE METERS
     INTEGER NY2         ! NUMBER OF CELLS BETWEEN 3m AND 1m AWAY FROM THE WALL  []
                         ! (ON THE INSIDE), CELLS HERE ARE SPACED AT 0.2m
     INTEGER NY3         ! 1 CELL @ 0.6m AWAY FROM THE INSIDE FACE OF THE        []
                         ! BASEMENT WALL
     INTEGER NY4         ! NUMBER OF CELLS BETWEEN 0.6m AND THE WALL FACE @ 0.2m []
     INTEGER NY5         ! THE BASEMENT WALL IS 1 CELL THICK                     []
     INTEGER NY6         ! THE GRAVEL BED IS 1 CELL THICK                        []
     INTEGER NY7         ! CELLS @ 0.25m FOR 1m OUTSIDE OF THE GRAVEL BED        []
     INTEGER NY8         ! CELLS @ 0.5m FOR 1m OUTSIDE OF NX7                    []
     INTEGER NY9         ! 1 CELL @ 3m AWAY FROM THE GRAVEL BED                  []
     INTEGER NY10        ! OUTSIDE 3m AWAY FROM THE GRAVEL BED, CELLS ARE @ 2m   []

     INTEGER NZ2         ! FLOOR SLAB CELL                                       []
     INTEGER NZ3         ! GRAVEL BED CELL                                       []
     INTEGER NZ4         ! NUMBER OF CELLS IN THE FIRST METER BELOW GRAVEL BED   []
     INTEGER NZ5         ! NUMBER OF CELLS IN THE NEXT METER                     []
     INTEGER NZ6         ! NUMBER OF CELLS TO THE EDGE OF THE DOMAIN             []


     REAL(r64) DWALL          ! BASEMENT WALL THICKNESS                              [m]
     REAL(r64) DGRAVXY        ! GRAVEL LAYER ALONG BASEMENT WALL THICKNESS           [m]
     REAL(r64) DGRAVZN        ! GRAVEL BED UNDER FLOOR SLAB THICKNESS                [m]
     REAL(r64) DSLAB          ! FLOOR SLAB THICKNESS                                 [m]
     REAL(r64) CeilThick      ! CEILING MATERIAL THICKNESS (CURRENTLY FIXED)         [m]
     REAL(r64) RimJoistHeight ! RIM JOIST HEIGHT  (CURRENTLY FIXED)                  [m]
     REAL(r64) SillPlateHeight ! SILL PLATE HEIGHT (CURRENTLY FIXED)                 [m]


     LOGICAL ODD         ! ODD NUMBER FLAG FOR EDGEM3. USED TO SPACE THE GRID    []
                         ! INSIDE THE SLAB.


!**** ADDED WITH ADDITION OF SHALLOW BASEMENT CAPABILITIES
     INTEGER NZP         ! Number of cells in the above grade foundation wall    []


!*** SALUTATION
 700 Format(/,' You have selected to have the solution grid sized automatically.',/)
!     PRINT 700
!     PRINT *,''
!     PRINT *,'YOU HAVE SELECTED TO HAVE THE SOLUTION GRID SIZED AUTOMATICALLY'
!     PRINT *,''
     WRITE(DebugOutFile,700)

!*** ASSIGN VALUES FROM DERIVED TYPES
     DWALL  =BuildingData%DWALL
     DGRAVXY=BuildingData%DGRAVXY
     DGRAVZN=BuildingData%DGRAVZN
     DSLAB  =BuildingData%DSLAB

!*** ASSIGN CONSTANTS
     EDGE1=SLABX/2.
     EDGE2=SLABY/2.
     EDGE1M3=EDGE1-3.
     EDGE2M3=EDGE2-3.
     DOMAINEDGEX=EDGE1+CLEARANCE+DWALL+DGRAVXY
     DOMAINEDGEY=EDGE2+CLEARANCE+DWALL+DGRAVXY

!*** SET NX
!*** CHECK TO SEE IF THE SLAB X DIMENSION IS ODD OR EVEN (THIS AFFECTS NX1)
     ! IF EDGE1M3 IS ODD, CELL FACES ARE PLACED AT 1, 3, 5....m UNTIL 3m BEFORE
     ! THE FACE OF THE BASEMENT WALL

     IF(MOD(EDGE1M3,2.).NE.0.) THEN
       NX1=INT(EDGE1M3)/2+1
       ODD=.TRUE.
     ELSE
       NX1=EDGE1M3/2
       ODD=.FALSE.
     END IF
     NX2=4          ! BETWEEN 3m AND 1m AWAY FROM THE WALL (ON THE INSIDE), CELLS
                    ! ARE SPACED AT 0.5m
     NX3=1          ! 1 CELL @ 0.6m AWAY FROM THE INSIDE FACE OF THE BASEMENT WALL
     NX4=3          ! NUMBER OF CELLS BETWEEN 0.6m AND THE WALL FACE @ 0.2m
     NX5=3          ! THE BASEMENT WALL IS 3 CELLS THICK
     NX6=2          ! THE GRAVEL BED IS 2 CELLS THICK
     NX7=4          ! CELLS @ 0.25m FOR 1m OUTSIDE OF THE GRAVEL BED
     NX8=2          ! CELLS @ 0.5m FOR 1m OUTSIDE OF NX7
     NX9=1          ! 1 CELL @ 3m
     NX10=(CLEARANCE-3)/2     ! OUTSIDE 3m AWAY FROM THE GRAVEL BED, CELLS ARE 2m

     IBASE=NX1+NX2+NX3+NX4

!*** Set NX
     NX=NX1+NX2+NX3+NX4+NX5+NX6+NX7+NX8+NX9+NX10

!*** SET XFACE VALUES
!*** The first cell is always in the origin
     XFACE(0)=0.
!*** The next cells (numbered 1 through NX1) are spaced @ 2m, according to whether
!*** the value of EDGE-3 is odd or not. If EDGEM3 is odd, the first cell face is at
!*** 1.0m. If not, it's at 2.0m
     DO COUNT1=1,NX1
       IF (COUNT1.EQ.1) THEN
         IF (ODD) THEN
           XFACE(COUNT1)=MOD(EDGE1M3,2.d0)!1.0
         ELSE
           XFACE(COUNT1)=2.0d0
         END IF
       ELSE
         XFACE(COUNT1)=XFACE(COUNT1-1)+2.0d0
       END IF
     END DO
!*** The next group of cells are spaced at 0.5m
     DO COUNT1=NX1+1,NX1+NX2
       XFACE(COUNT1)=XFACE(COUNT1-1)+0.500d0
     END DO
!*** This cell is placed at 0.6m from the basement wall, allowing
!*** for an even spacing closer in
     DO COUNT1=NX1+NX2+1,NX1+NX2+NX3
       XFACE(COUNT1)=EDGE1-0.6000d0
     END DO
!*** In the last 60cm, the cells are spaced at 20cm.
     DO COUNT1=NX1+NX2+NX3+1,IBASE
       XFACE(COUNT1)=XFACE(COUNT1-1)+0.2000d0
     END DO
!*** The wall is three cells thick
     XFACE(IBASE+1)=XFACE(IBASE)+0.078d0
     XFACE(IBASE+2)=XFACE(IBASE)+0.156d0
     XFACE(IBASE+3)=XFACE(IBASE)+DWALL

!*** The gravel bed cell is, by definition, two cells thick.
     DO COUNT1=IBASE+NX5+1,IBASE+NX5+NX6
       XFACE(COUNT1)=XFACE(COUNT1-1)+DGRAVXY/2.d0
     END DO
!*** For the first meter past the gravel bed, the cells are spaced at 0.25m
     DO COUNT1=IBASE+NX5+NX6+1,IBASE+NX5+NX6+NX7
       XFACE(COUNT1)=XFACE(COUNT1-1)+0.250d0
     END DO
!*** For the next meter, the cells are spaced at 0.5m
     DO COUNT1=IBASE+NX5+NX6+NX7+1,IBASE+NX5+NX6+NX7+NX8
       XFACE(COUNT1)=XFACE(COUNT1-1)+0.500d0
     END DO
!*** A cell is placed at 3.0m outside of the building zone
     DO COUNT1=IBASE+NX5+NX6+NX7+NX8+1,IBASE+NX5+NX6+NX7+NX8+NX9
       XFACE(COUNT1)=3.000+EDGE1+DWALL+DGRAVXY
     END DO
!*** For the rest of the domain, the cells are spaced at 2.0m
     DO COUNT1=IBASE+NX5+NX6+NX7+NX8+NX9+1,NX
       XFACE(COUNT1)=XFACE(COUNT1-1)+2.000d0
     END DO
     IF (XFACE(NX).GT.DOMAINEDGEX.OR.XFACE(NX).LT.DOMAINEDGEX) THEN
       XFACE(NX)=DOMAINEDGEX
     END IF
     DO COUNT1=1,NX
       IF (XFACE(COUNT1).LT.XFACE(COUNT1-1)) THEN
         NX=NX-1
       END IF
     END DO
     DO COUNT1=IBASE+NX5+NX6+NX7+NX8+NX9+1,NX-1
       XFACE(COUNT1)=XFACE(COUNT1-1)+2.000d0
     END DO
     DO COUNT1=NX+1,50
       XFACE(COUNT1)=0.0
     END DO

!*** SET NY
!*** CHECK TO SEE IF THE SLAB X DIMENSION IS ODD OR EVEN (THIS AFFECTS NY1)

!*** IF EDGE2M3 IS ODD, CELL FACES ARE PLACED AT 1, 3, 5....m UNTIL 3m BEFORE
!*** THE FACE OF THE BASEMENT WALL
     IF(MOD(EDGE2M3,2.).NE.0.) THEN
       NY1=INT(EDGE2M3)/2+1
       ODD=.TRUE.
     ELSE
       NY1=EDGE2M3/2.d0
       ODD=.FALSE.
     END IF
     NY2=4          ! BETWEEN 3m AND 1m AWAY FROM THE WALL (ON THE INSIDE), CELLS
                    ! ARE SPACED AT 0.5m
     NY3=1          ! 1 CELL @ 0.6m AWAY FROM THE INSIDE FACE OF THE BASEMENT WALL
     NY4=3          ! NUMBER OF CELLS BETWEEN 0.6m AND THE WALL FACE @ 0.2m
     NY5=3          ! THE BASEMENT WALL IS 3 CELLS THICK
     NY6=2          ! THE GRAVEL BED IS 2 CELLS THICK
     NY7=4          ! CELLS @ 0.25m FOR 1m OUTSIDE OF THE GRAVEL BED
     NY8=2          ! CELLS @ 0.5m FOR 1m OUTSIDE OF NY7
     NY9=1          ! 1 CELL @ 3m
     NY10=(CLEARANCE-3)/2     ! OUTSIDE 3m AWAY FROM THE GRAVEL BED, CELLS ARE 2m

     JBASE=NY1+NY2+NY3+NY4

!*** Set NY
     NY=NY1+NY2+NY3+NY4+NY5+NY6+NY7+NY8+NY9+NY10

!*** SET YFACE VALUES

!*** The first cell is always in the origin
     YFACE(0)=0.
!*** The next cells (numbered 1 through NY1) are spaced @ 2m, according to whether
!*** the value of EDGE-3 is odd or not. If EDGEM3 is odd, the first cell face is at
!*** 1.0m. If not, it's at 2.0m

     DO COUNT1=1,NY1
       IF (COUNT1.EQ.1) THEN
         IF (ODD) THEN
           YFACE(COUNT1)=MOD(EDGE2M3,2.)!1.0
         ELSE
           YFACE(COUNT1)=2.0d0
         END IF
       ELSE
         YFACE(COUNT1)=YFACE(COUNT1-1)+2.0d0
       END IF
     END DO

!*** The next group of cells are spaced at 0.5m
     DO COUNT1=NY1+1,NY1+NY2
       YFACE(COUNT1)=YFACE(COUNT1-1)+0.500d0
     END DO
!*** This cell is placed at 0.6m from the basement wall, allowing
!*** for an even spacing closer in
     DO COUNT1=NY1+NY2+1,NY1+NY2+NY3
       YFACE(COUNT1)=EDGE2-0.6000d0
     END DO
!*** In the last 60cm, the cells are spaced at 20cm.
     DO COUNT1=NY1+NY2+NY3+1,JBASE
       YFACE(COUNT1)=YFACE(COUNT1-1)+0.2000d0
     END DO

!*** The wall is three cells thick
     YFACE(JBASE+1)=YFACE(JBASE)+0.078d0
     YFACE(JBASE+2)=YFACE(JBASE)+0.156d0
     YFACE(JBASE+3)=YFACE(JBASE)+DWALL

!*** The gravel bed cell is, by definition, two cells thick.
     DO COUNT1=JBASE+NY5+1,JBASE+NY5+NY6
       YFACE(COUNT1)=YFACE(COUNT1-1)+DGRAVXY/2.d0
     END DO

!*** For the first meter past the gravel bed, the cells are spaced at 0.25m
     DO COUNT1=JBASE+NY5+NY6+1,JBASE+NY5+NY6+NY7
       YFACE(COUNT1)=YFACE(COUNT1-1)+0.250d0
     END DO

!*** For the next meter, the cells are spaced at 0.5m
     DO COUNT1=JBASE+NY5+NY6+NY7+1,JBASE+NY5+NY6+NY7+NY8
       YFACE(COUNT1)=YFACE(COUNT1-1)+0.500d0
     END DO

!*** A cell is placed at 3.0m outside of the building zone
     DO COUNT1=JBASE+NY5+NY6+NY7+NY8+1,JBASE+NY5+NY6+NY7+NY8+NY9
       YFACE(COUNT1)=3.000d0+EDGE2+DWALL+DGRAVXY
     END DO

!*** For the rest of the domain, the cells are spaced at 2.0m
     DO COUNT1=JBASE+NY5+NY6+NY7+NY8+NY9+1,NY
       YFACE(COUNT1)=YFACE(COUNT1-1)+2.000d0
     END DO
     IF (YFACE(NY).GT.DOMAINEDGEY.OR.YFACE(NY).LT.DOMAINEDGEY) THEN
       YFACE(NY)=DOMAINEDGEY
     END IF
     DO COUNT1=1,NY
       IF (YFACE(COUNT1).LT.YFACE(COUNT1-1)) THEN
         NY=NY-1
       END IF
     END DO
     DO COUNT1=JBASE+NX5+NY6+NY7+NY8+NY9+1,NY-1
       YFACE(COUNT1)=YFACE(COUNT1-1)+2.000d0
     END DO
     DO COUNT1=NY+1,50
       YFACE(COUNT1)=0.0
     END DO


!*** Z cells
     CeilThick=0.044d0
     RimJoistHeight=0.235d0
     SillPlateHeight=0.038d0
     IF(((ConcAGHeight/.2d0)-INT(ConcAGHeight/0.2d0)).GT.0.001d0) THEN
       NZP=INT(ConcAGHeight/0.2d0)+1
     ELSE
       NZP=INT(ConcAGHeight/0.2d0)
     END IF
     IF (NZP.EQ.0) THEN
       NZAG=4
     ELSE
       NZAG=NZP+3
     END IF

     IF(MOD(BaseDepth,.200).GT.0.0005d0) THEN
       NZ1=INT(BaseDepth/0.2d0)+1 ! NZ1 is the number of cells in the basement wall that
                                ! are below grade
     ELSE
       NZ1=INT(BaseDepth/.2d0)
     END IF

     NZ2=1          ! NZ2 is the basement floor slab, one cell thick, by definition
     NZ3=1          ! NZ3 is the gravel bed below the floor slab. It is one cell thick
                    ! by definition
     NZ4=4          ! For the 1st meter below the gravel bed, cells are spaced at 0.25m
     NZ5=2          ! For the next meter, cells are spaced at 0.5m
     NZ6=7          ! To the edge of the domain, cells are spaced at 2m
     NZBG=NZ1+NZ2+NZ3+NZ4+NZ5+NZ6

     IF(NZBG.GT.100) THEN
      CALL ShowSevereError('AutoGrid BaseDepth is too high, reduce it below 17.0 meters')
      CALL ShowContinueError('BaseDepth=['//trim(RoundSigDigits(BaseDepth,4))//'], '&
        'resulting  NZBG=['//trim(RoundSigDigits(NZBG,0))//'] (max 100).')
      CALL ShowFatalError('Program terminates due to preceding condition(s).')
     END IF

     ZFACEINIT(-NZAG+3)=-ConcAGHeight
     ZFACEINIT(-NZAG+2)=ZFACEINIT(-NZAG+3)-SillPlateHeight
     ZFACEINIT(-NZAG+1)=ZFACEINIT(-NZAG+2)-RimJoistHeight
     ZFACEINIT(-NZAG)=ZFACEINIT(-NZAG+1)-CeilThick
     DO COUNT3=-NZAG+4,0
       IF (NZAG.EQ.4) THEN
         ZFACEINIT(COUNT3)=0.0
       ELSE IF(COUNT3.EQ.-NZAG+4) THEN
         ZFACEINIT(COUNT3)=ZFACEINIT(-NZAG+3)+MOD(ConcAGHeight,0.2d0)
       ELSE
         ZFACEINIT(COUNT3)=ZFACEINIT(COUNT3-1)+0.2d0
       END IF
       IF (COUNT3.EQ.0) ZFACEINIT(COUNT3)=0.0
     END DO


     !*** BELOW GRADE CELLS

     DO COUNT1=1,NZ1
       ZFACEINIT(COUNT1)=ZFACEINIT(COUNT1-1)+0.2d0
       IF (COUNT1.EQ.NZ1) ZFACEINIT(COUNT1)=BaseDepth
       IF (COUNT1.EQ.NZ1) KBASE=COUNT1 !+NZAG
     END DO
     DO COUNT1=NZ1+1,NZ1+NZ2
       ZFACEINIT(COUNT1)=ZFACEINIT(COUNT1-1)+Slabdepth
     END DO
     DO COUNT1=NZ1+NZ2+1,NZ1+NZ2+NZ3
       ZFACEINIT(COUNT1)=ZFACEINIT(COUNT1-1)+DGRAVZN
     END DO
     DO COUNT1=NZ1+NZ2+NZ3+1,NZ1+NZ2+NZ3+NZ4
       ZFACEINIT(COUNT1)=ZFACEINIT(COUNT1-1)+0.25d0
     END DO
     DO COUNT1=NZ1+NZ2+NZ3+NZ4+1,NZ1+NZ2+NZ3+NZ4+NZ5
       ZFACEINIT(COUNT1)=ZFACEINIT(COUNT1-1)+0.5d0
     END DO
     DO COUNT1=NZ1+NZ2+NZ3+NZ4+NZ5+1,NZBG
       ZFACEINIT(COUNT1)=ZFACEINIT(COUNT1-1)+2.0d0
     END DO
END SUBROUTINE Autogridding

!******************************  MINIMUM DZ (PART OF AUTOGRIDDING)  *********************
SUBROUTINE CalcDZmin(DX,DY,DZINIT)
!USE MSFLIB
USE BasementSimData
USE DataGlobals, ONLY: ShowSevereError,ShowContinueError,ShowFatalError
USE General, ONLY: RoundSigDigits
IMPLICIT NONE
! MODULE INFORMATION:
        !       AUTHOR         Edward D. Clements
        !       DATE WRITTEN   March 07, 2000
        !       MODIFIED       na
        !       MODIFIED       na
        !       RE-ENGINEERED  na

        ! PURPOSE OF THIS MODULE:
        ! This module will establish the simulation grid for a slab on grade foundation
        ! whose dimensions are input by the user.

        ! METHODOLOGY EMPLOYED:
        ! Standard EnergyPlus modular coding

        ! REFERENCES:
        ! Chang, Chow, and Chang, 1991

        ! OTHER NOTES:
        ! na

     REAL(r64) F         ! F FACTOR FOR THE F-FACTOR ADI METHOD                         []
     REAL(r64) TSTEP     ! SOLUTION TIME STEP                                          [s]
     REAL(r64) DX(0:100)  ! ARRAY OF CELL DIMENSIONS                                [m**2]
     REAL(r64) DY(0:100)  ! ARRAY OF CELL DIMENSIONS                                [m**2]
     REAL(r64) DZMIN(0:100,0:100,-35:100) ! ARRAY OF MINIMUM CELL DIMENSIONS TO     [m**2]
                                     ! SATISFY NUMERICAL STABILITY
     REAL(r64) DZINIT(-35:100) ! ARRAY OF INITIAL CELL DIMENSIONS                   [m**2]
     REAL(r64) DZACT(-35:100)  ! ARRAY OF ACTUAL CELL DIMENSIONS                    [m**2]
     REAL(r64) RHOUSED   ! VALUE FOR DENSITY USED FOR CALCULATING STABILITY      [kg/m**3]
     REAL(r64) CPUSED    ! VALUE OF SPECIFIC HEAT USED FOR STABILITY             [kg/m**3]
     REAL(r64) TCONUSED  ! VALUE OF THERMAL CONDUCTIVITY USED FOR STABILITY      [kg/m**3]

     REAL(r64) SqrtArg

!*** Setting values of derived type variables
     TSTEP=SimParams%TSTEP*3600.
     F=SimParams%F
     RHOUSED =RHO(4)
     CPUSED  =CP(4)
     TCONUSED=TCON(4)
     DO COUNT1=0,NX-1
       DO COUNT2=0,NY-1
         DO COUNT3=-NZAG,NZBG-1
!         print *,'rho/cp=',.75d0*RHOUSED*CPUSED
!         print *,'ftcon=',F*TCONUSED*TSTEP
!         print *,'dx=',1.d0/DX(COUNT1)**2
!         print *,'dy=',1.0d0/DY(COUNT2)**2
!         print *,'sqrt arg=',(1.d0/( (.75d0*RHOUSED*CPUSED)/           &
!               & (F*TCONUSED*TSTEP)- (1.d0/DX(COUNT1)**2) - (1.0d0/DY(COUNT2)**2)) )
           SqrtArg=(1.d0/( (.75d0*RHOUSED*CPUSED)/           &
               & (F*TCONUSED*TSTEP)- (1.d0/DX(COUNT1)**2) - (1.0d0/DY(COUNT2)**2)) )
           IF (SqrtArg < 0.0d0 .and. ABS(SqrtArg) <= .2d0) THEN
             SqrtArg=0.0d0
           ELSEIF (SqrtArg < 0.0d0) THEN
             CALL ShowSevereError('CalcDZmin: Argument ['//trim(RoundSigDigits(SqrtArg,3))//'] to Sqrt < min threshold.')
             CALL ShowContinueError('Check autogridding and ADI factor inputs for accuracy.')
             CALL ShowFatalError('Program terminates due to preceding condition.')
           ENDIF
           DZMIN(COUNT1,COUNT2,COUNT3)=SQRT (SqrtArg)
!           DZMIN(COUNT1,COUNT2,COUNT3)=SQRT (1.d0/( (.75d0*RHOUSED*CPUSED)/           &
!               & (F*TCONUSED*TSTEP)- (1.d0/DX(COUNT1)**2) - (1.0d0/DY(COUNT2)**2)) )
           IF (DZINIT(COUNT3).LT.DZMIN(COUNT1,COUNT2,COUNT3)) THEN
!             PRINT *,'STABILITY CHECK FAILED, CELL: ',COUNT1,COUNT2,            &
!                   & COUNT3,' REASSIGNING VALUES'
!             Write(DebugOutFile,*) 'Stability Check Failed, Cell: ',COUNT1,COUNT2,            &
!                   & COUNT3,' Reassigning Values'
!              Write(DebugOutFile,*) 'DZINIT < DZMIN=',DZINIT(COUNT3),DZMIN(COUNT1,COUNT2,COUNT3)
               DZACT(COUNT3)=DZMIN(COUNT1,COUNT2,COUNT3)
           ELSE
             DZACT(COUNT3)=DZINIT(COUNT3)
           END IF
         END DO
       END DO
     END DO
     ZFACE(-NZAG)=ZFACEINIT(-NZAG)
     ZFACE(-NZAG+1)=ZFACEINIT(-NZAG+1)
     ZFACE(-NZAG+2)=ZFACEINIT(-NZAG+2)
     ZFACE(-NZAG+3)=ZFACEINIT(-NZAG+3)
     ZFACE(0)=0.0
     DO COUNT3=-NZAG,NZBG
       IF (DZACT(COUNT3).NE.DZINIT(COUNT3)) THEN
         ZFACE(COUNT3)=ZFACE(COUNT3-1)+DZACT(COUNT3)
         IF (COUNT3.EQ.NZ1) ZFACE(COUNT3)=ZFACEINIT(NZ1)
       ELSE
         ZFACE(COUNT3)=ZFACEINIT(COUNT3)
       END IF
     END DO
     RETURN
END SUBROUTINE  CalcDZmin

!***************************  EnergyPlus Surface Temperatures  **************************
SUBROUTINE SurfaceTemps(T,DX,DY,DZ,MTYPE,INS,TSurfWallXZ,TSurfWallYZ,           &
           & TSurfFloor,TSWallYZIn,TSWallXZIn,TSFloorIn,TSYZCL,TSXZCL,TSFXCL,   &
           & TSFYCL,XC,YC,ZC,TSurfWallYZUpper,TSurfWallYZUpperIn,               &
           & TSurfWallXZUpper,TSurfWallXZUpperIn,TSurfWallYZLower,              &
           & TSurfWallYZLowerIn,TSurfWallXZLower,TSurfWallXZLowerIn,DAPerim,    &
           & DACore,DAYZUpperSum,DAYZLowerSum,DAXZUpperSum,DAXZLowerSum,        &
           & TSurfFloorPerim,TSurfFloorPerimIn,TSurfFloorCore,TSurfFloorCoreIn, &
           & TWW,TWS,TF,XDIM,YDIM,ZDIM,DAXZSum,DAYZSum,DAXYSum)
! SUBROUTINE INFORMATION:
        !       AUTHOR         Edward D. Clements
        !       DATE WRITTEN   April 2, 2000
        !       MODIFIED       April 3, 2000, August 21, 2000
        !       MODIFIED       na
        !       RE-ENGINEERED  na

        ! PURPOSE OF THIS MODULE:
        ! This subroutine will calculate the outside foundation surface temperatures
        ! required by EnergyPlus for integration with the above grade building model.

        ! METHODOLOGY EMPLOYED:
        ! This subroutine was developed using standard EnergyPlus modular formatting.

        ! MATHEMATIC METHODOLOGY
        ! The surface temperatures are calculated using the pre-calculated three
        ! dimensional temperature field around the building. The temperatures are
        ! calculated at the outside surface using the premise that the heat flux
        ! on both sides of the surface must be equal. q"(i-) = q"(i+), where
        ! q"(i-)=k(i-1)*((Ts-T(i-1))/(DX(i-1)/2)) and
        ! q"(i+)=k(i+1)*((T(i+1)-Ts)/(DX(i+1)/2)) and Ts is the outer foundation
        ! surface temperature

        ! REFERENCES:
        ! na

        ! OTHER NOTES:
        ! na
!USE MSFLIB
USE BasementSimData
USE DataGlobals, ONLY: ShowSevereError
USE General, ONLY: TrimSigDigits
IMPLICIT NONE

!*** Variable Declarations
     REAL(r64) T(-1:100,-1:100,-35:100)   ! 3D temperature field around building             [C]
     REAL(r64) TSWYZ(-1:100,-35:100)     ! Wall surface temperature array (non-averaged)    [C]
     REAL(r64) TSWXZ(-1:100,-35:100)     ! Wall surface temperature array (non-averaged)    [C]
     REAL(r64) TSF(-1:100,-1:100)         ! Floor surface temperature array                  [C]
     REAL(r64) DX(0:100)                ! Array of X direction cell dimensions             [m]
     REAL(r64) DY(0:100)                ! Array of Y direction cell dimensions             [m]
     REAL(r64) DZ(-35:100)              ! Array of Z direction cell dimensions             [m]
     REAL(r64) REXT                     ! R value of basement wall insulation      [K/(W/m^2)]
     REAL(r64) TSWYZSum                 ! Dummy summation variable for computing      [m**2-C]
                                   ! area weighted average YZ wall surface temperature
     REAL(r64) TSWXZSum                 ! Dummy summation variable for computing      [m**2-C]
                                   ! area weighted average XZ wall surface temperature
     REAL(r64) TSFSum                   ! Dummy summation variable for computing      [m**2-C]
                                   ! area weighted average floor surface temperature
     REAL(r64) DAYZSum                  ! Dummy area summation variable for computing   [m**2]
                                   ! area weighted average YZ wall surface temperature
     REAL(r64) DAXZSum                  ! Dummy area summation variable for computing   [m**2]
                                   ! area weighted average XZ wall surface temperature
     REAL(r64) DAXYSum                  ! Dummy area summation variable for computing   [m**2]
                                   ! area weighted average floor surface temperature
     REAL(r64) TSurfWallYZ              ! Area weighted avg YZ plane wall temperature      [C]
     REAL(r64) TSurfWallXZ              ! Area weighted avg XZ plane wall temperature      [C]
     REAL(r64) TSurfFloor               ! Area weighted average floor temperature          [C]
     REAL(r64) TSWallYZIn               ! Area weighted avg YZ plane wall temperature      [C]
     REAL(r64) TSWallXZIn               ! Area weighted avg XZ plane wall temperature      [C]
     REAL(r64) TSFloorIn                ! Area weighted average slab temperatrue           [C]
     REAL(r64) TYZSumIn                 ! Dummy summation variable for averaging      [m**2-C]
     REAL(r64) TXZSumIn                 ! Dummy summation variable for averaging      [m**2-C]
     REAL(r64) TXYSumIn                 ! Dummy summation variable for averaging      [m**2-C]
     REAL(r64) TSYZCL                   ! YZ plane average wall centerline temperature     [C]
     REAL(r64) TSYZCLSum                ! Dummy summation variable for averaging      [m**2-C]
     REAL(r64) TSXZCL                   ! XZ plane average wall centerline temperature     [C]
     REAL(r64) TSXZCLSum                ! Dummy summation variable for averaging      [m**2-C]
     REAL(r64) TSFXCL                   ! Floor X direction avg centerline temperature     [C]
     REAL(r64) TSFXCLSum                ! Dummy summation variable for averaging      [m**2-C]
     REAL(r64) TSFYCL                   ! Floor Y direction avg centerline temperature     [C]
     REAL(r64) TSFYCLSum                ! Dummy summation variable for averaging      [m**2-C]
     REAL(r64) DGRAVZP
     REAL(r64) DGRAVZN

     INTEGER INS(0:100,0:100,-35:100) ! Insulation indicator for cells                 []
                                   ! 1= insulated cell, 0= non-insulated cell          []
     INTEGER MTYPE(0:100,0:100,-35:100)    ! Array of material types for entire domain []

!*** Variables added with a split surface temperature calcultation
     INTEGER XDIM                  ! Array dimensioning constant for use with surface  []
                                   ! temperature and flux calculation variables
     INTEGER YDIM                  ! Array dimensioning constant for use with surface  []
                                   ! temperature and flux calculation variables
     INTEGER ZDIM                  ! Array dimensioning constant for use with surface  []
                                   ! temperature and flux calculation variables
!     INTEGER NZAG                  ! Number of cells above grade                       []

     REAL(r64) TSWYZLowerSum            ! Dummy summation variable                    [m**2-C]
     REAL(r64) TSWYZLowerSumIn          ! Dummy summation variable                    [m**2-C]
     REAL(r64) TSWYZUpperSum            ! Dummy summation variable                    [m**2-C]
     REAL(r64) TSWYZUpperSumIn          ! Dummy summation variable                    [m**2-C]
     REAL(r64) TSWXZLowerSum            ! Dummy summation variable                    [m**2-C]
     REAL(r64) TSWXZLowerSumIn          ! Dummy summation variable                    [m**2-C]
     REAL(r64) TSWXZUpperSum            ! Dummy summation variable                    [m**2-C]
     REAL(r64) TSWXZUpperSumIn          ! Dummy summation variable                    [m**2-C]
     REAL(r64) TSurfPerimSum            ! Dummy summation variable                    [m**2-C]
     REAL(r64) TSFPerimInSum            ! Dummy summation variable                    [m**2-C]
     REAL(r64) TSurfCoreSum             ! Dummy summation variable                    [m**2-C]
     REAL(r64) TSFCoreInSum             ! Dummy summation variable                    [m**2-C]

     REAL(r64) DAYZUpperSum             ! Upper band YZ wall area                       [m**2]
     REAL(r64) DAXZUpperSum             ! Upper band XZ wall area                       [m**2]
     REAL(r64) DAYZLowerSum             ! Lower band YZ wall area                       [m**2]
     REAL(r64) DAXZLowerSum             ! Lower band XZ wall area                       [m**2]
     REAL(r64) DAPerim                  ! Perimeter zone floor area                     [m**2]
     REAL(r64) DACore                   ! Core zone floor area                          [m**2]
     REAL(r64) TSurfWallYZUpper         ! Upper band YZ wall temperature                   [C]
     REAL(r64) TSurfWallYZUpperIn       ! Upper band YZ inside wall temperature            [C]
     REAL(r64) TSurfWallYZLower         ! Lower band YZ wall tepmerature                   [C]
     REAL(r64) TSurfWallYZLowerIn       ! Lower band YZ inside wall temperature            [C]
     REAL(r64) TSurfWallXZLower         ! Lower band XZ wall temperature                   [C]
     REAL(r64) TSurfWallXZLowerIn       ! Lower band XZ inside wall temperature            [C]
     REAL(r64) TSurfWallXZUpper         ! Upper band XZ wall temperature                   [C]
     REAL(r64) TSurfWallXZUpperIn       ! Upper band XZ wall inside temperature            [C]

     REAL(r64) TSurfFloorPerim          ! Perimeter area floor surface temperature         [C]
     REAL(r64) TSurfFloorPerimIn        ! Perimeter area floor inside surface temperature  [C]
     REAL(r64) TSurfFloorCore           ! Core area floor surface temperature              [C]
     REAL(r64) TSurfFloorCoreIn         ! Core area floor inside surface temperature       [C]
     REAL(r64) XC(0:100)                ! Array of cell center coordinates                 [m]
     REAL(r64) YC(0:100)                ! Array of cell center coordinates                 [m]
     REAL(r64) ZC(-35:100)              ! Array of cell center coordinates                 [m]
     REAL(r64) TWW
     DIMENSION TWW(0:XDIM,-35:ZDIM)
     REAL(r64) TWS
     DIMENSION TWS(0:YDIM,-35:ZDIM)
     REAL(r64) TF
     DIMENSION TF(0:XDIM,0:YDIM)
     REAL(r64) KEXT
     REAL(r64) DSLAB

     REAL(r64) Rleft,Rright,Tleft,Tright !  Local temporary variables to simplify equations.
     character(len=25) indexerr

!*** Assigning derived type variables
     REXT   = Insul%REXT
     DGRAVZP=BuildingData%DGRAVZP
     DGRAVZN=BuildingData%DGRAVZN
     DSLAB  =BuildingData%DSLAB

!*** Building Constant
     KEXT=ZFACE(KBASE)+DSLAB
!*** Initializing all variables
     TSWYZSum          =0.0
     TSWXZSum          =0.0
     TSFSum            =0.0
     DAYZSum           =0.0
     DAXZSum           =0.0
     DAXYSum           =0.0
     TSurfWallYZ       =0.0
     TSurfWallXZ       =0.0
     TSurfFloor        =0.0
     TSWallYZIn        =0.0
     TSWallXZIn        =0.0
     TSFloorIn         =0.0
     TYZSumIn          =0.0
     TXZSumIn          =0.0
     TXYSumIn          =0.0
     TSYZCL            =0.0
     TSYZCLSum         =0.0
     TSXZCL            =0.0
     TSXZCLSum         =0.0
     TSFXCL            =0.0
     TSFXCLSum         =0.0
     TSFYCL            =0.0
     TSFYCLSum         =0.0
     TSWYZLowerSum     =0.0
     TSWYZLowerSumIn   =0.0
     TSWYZUpperSum     =0.0
     TSWYZUpperSumIn   =0.0
     TSWXZLowerSum     =0.0
     TSWXZLowerSumIn   =0.0
     TSWXZUpperSum     =0.0
     TSWXZUpperSumIn   =0.0
     TSurfPerimSum     =0.0
     TSFPerimInSum     =0.0
     TSurfCoreSum      =0.0
     TSFCoreInSum      =0.0
     DAYZUpperSum      =0.0
     DAXZUpperSum      =0.0
     DAYZLowerSum      =0.0
     DAXZLowerSum      =0.0
     DAPerim           =0.0
     DACore            =0.0
     TSurfWallYZUpper  =0.0
     TSurfWallYZUpperIn=0.0
     TSurfWallYZLower  =0.0
     TSurfWallYZLowerIn=0.0
     TSurfWallXZLower  =0.0
     TSurfWallXZLowerIn=0.0
     TSurfWallXZUpper  =0.0
     TSurfWallXZUpperIn=0.0
     TSurfFloorPerim   =0.0
     TSurfFloorPerimIn =0.0
     TSurfFloorCore    =0.0
     TSurfFloorCoreIn  =0.0

!*** Calculate the surface temperature field for the wall cells in the YZ plane
     !open (unit=124,file='shit.txt')

     DO COUNT2=0,JBASE-1
       DO COUNT3=0,KBASE-1
 !        TSWYZ(COUNT2,COUNT3)=(T(IBASE+3,COUNT2,COUNT3)*(1/                     &
 !        & (INS(IBASE+3,COUNT2,COUNT3)*REXT+((DX(IBASE+3)/2.)/                  &
 !        & TCON(MTYPE(IBASE+3,COUNT2,COUNT3)))))+T(IBASE+4,COUNT2,COUNT3)*      &
 !        & (TCON(MTYPE(IBASE+4,COUNT2,COUNT3))/(DX(IBASE+4)/2.)))/              &
 !        & ((1/(INS(IBASE+3,COUNT2,COUNT3)*REXT+((DX(IBASE+3)/2.)/              &
 !        & TCON(MTYPE(IBASE+3,COUNT2,COUNT3)))))+                               &
 !        & (TCON(MTYPE(IBASE+4,COUNT2,COUNT3))/(DX(IBASE+4)/2.)))
        ! write (124,*) COUNT2,COUNT3,INS(IBASE+3,COUNT2,COUNT3),TSWYZ(COUNT2,COUNT3),TCON(MTYPE(IBASE+3,COUNT2,COUNT3)),TCON(MTYPE(IBASE+4,COUNT2,COUNT3)),DY(COUNT2),DZ(COUNT3)
 !
 !  Modified to correct surface temperature problems 9/2004 COP
 !
 !  Wall is visualized as having left side to basement, right side to ground

 IF (ABS(TCON(MTYPE(IBASE+3,COUNT2,COUNT3))) > 1.E-10) THEN
   Rleft = (DX(IBASE+3)/2.)/TCON(MTYPE(IBASE+3,COUNT2,COUNT3))  !  Wall resistance from last node to interface
 ELSE
   !write(*,*) IBASE+3,COUNT2,COUNT3
   write(indexerr,*) MTYPE(IBASE+3,COUNT2,COUNT3)
   indexerr=adjustl(indexerr)
   CALL ShowSevereError('Thermal conductivity of item ['//trim(indexerr)//' - '//  &
      trim(MatlTypes(MTYPE(IBASE+3,COUNT2,COUNT3)))//  &
      '] too small. Safe divide used.')
   Rleft = (DX(IBASE+3)/2.)/1.E-10 !  Wall resistance from last node to interface
 ENDIF
 IF (ABS(TCON(MTYPE(IBASE+4,COUNT2,COUNT3))) > 1.E-10) THEN
   Rright = (DX(IBASE+4)/2.)/TCON(MTYPE(IBASE+4,COUNT2,COUNT3)) + INS(IBASE+3,COUNT2,COUNT3)*REXT  ! Right resistance includes external insulation if present
 ELSE
!   write(*,*) IBASE+4,COUNT2,COUNT3
   write(indexerr,*) MTYPE(IBASE+4,COUNT2,COUNT3)
   indexerr=adjustl(indexerr)
   CALL ShowSevereError('Thermal conductivity of item ['//trim(indexerr)//  &
      trim(MatlTypes(MTYPE(IBASE+4,COUNT2,COUNT3)))//  &
      '] too small. Safe divide used.')
   Rleft = (DX(IBASE+4)/2.)/1.E-10 !  Wall resistance from last node to interface
 ENDIF
 Tleft = T(IBASE+3,COUNT2,COUNT3)
 Tright = T(IBASE+4,COUNT2,COUNT3)


 TSWYZ(COUNT2,COUNT3)= (Tleft*Rright + Tright*Rleft)/(Rleft+Rright)


!***     Calculate some summation variables for an area weighted average surface temp
         TSWYZSum=TSWYZSum+TSWYZ(COUNT2,COUNT3)*DY(COUNT2)*DZ(COUNT3)
         TYZSumIn=TYZSumIn+TWS(COUNT2,COUNT3)*DY(COUNT2)*DZ(COUNT3)
         DAYZSum =DAYZSum+DY(COUNT2)*DZ(COUNT3)

!***     Split surface temperature calcualtion
         IF (NZAG.EQ.4) THEN
           IF (ZC(COUNT3).LE.ZC(1+(KBASE-NZAG)/2))THEN
             TSWYZUpperSum=TSWYZUpperSum+TSWYZ(COUNT2,COUNT3)*DY(COUNT2)*DZ(COUNT3)
             TSWYZUpperSumIn=TSWYZUpperSumIn+TWS(COUNT2,COUNT3)*DY(COUNT2)*DZ(COUNT3)
             DAYZUpperSum=DAYZUpperSum+DY(COUNT2)*DZ(COUNT3)
           ELSE
             TSWYZLowerSum=TSWYZLowerSum+TSWYZ(COUNT2,COUNT3)*DY(COUNT2)*DZ(COUNT3)
             TSWYZLowerSumIn=TSWYZLowerSumIn+TWS(COUNT2,COUNT3)*DY(COUNT2)*DZ(COUNT3)
             DAYZLowerSum=DAYZLowerSum+DY(COUNT2)*DZ(COUNT3)
           END IF
         ELSE
           IF (ZC(COUNT3).LE.(KEXT+DGRAVZP-DGRAVZN)/2.)THEN
             TSWYZUpperSum=TSWYZUpperSum+TSWYZ(COUNT2,COUNT3)*DY(COUNT2)*DZ(COUNT3)
             TSWYZUpperSumIn=TSWYZUpperSumIn+TWS(COUNT2,COUNT3)*DY(COUNT2)*DZ(COUNT3)
             DAYZUpperSum=DAYZUpperSum+DY(COUNT2)*DZ(COUNT3)
           ELSE
             TSWYZLowerSum=TSWYZLowerSum+TSWYZ(COUNT2,COUNT3)*DY(COUNT2)*DZ(COUNT3)
             TSWYZLowerSumIn=TSWYZLowerSumIn+TWS(COUNT2,COUNT3)*DY(COUNT2)*DZ(COUNT3)
             DAYZLowerSum=DAYZLowerSum+DY(COUNT2)*DZ(COUNT3)
           END IF
         END IF
       END DO
     END DO
     !DAYZSum=DAYZSum
     !DAYZUpperSum=DAYZUpperSum
     TSurfWallYZ=TSWYZSum/DAYZSum
     TSWallYZIn =TYZSumIn/DAYZSum
     TSurfWallYZUpper=TSWYZUpperSum/DAYZUpperSum
     TSurfWallYZUpperIn=TSWYZUpperSumIn/DAYZUpperSum
     TSurfWallYZLower=TSWYZLowerSum/DAYZLowerSum
     TSurfWallYZLowerIn=TSWYZLowerSumIn/DAYZLowerSum


!*** Initialize the variables for the centerline temperature calculation
     TSYZCL=0.0
     TSYZCLSum=0.0
!*** Calculate the average wall centerline temperature for comparisons
     DO COUNT3=0,KBASE-1
       TSYZCLSum=TSYZCLSum+TSWYZ((INT(JBASE/2)),COUNT3)
     END DO
     TSYZCL=TSYZCLSum/(KBASE)

!*** Calculate the surface temperature field for the wall cells in the XZ plane
     DO COUNT1=0,IBASE-1 !+2
       DO COUNT3=0,KBASE-1 !1
         TSWXZ(COUNT1,COUNT3)=(T(COUNT1,JBASE+3,COUNT3)*(1.d0/                     &
         & (INS(COUNT1,JBASE+3,COUNT3)*REXT+((DY(JBASE+3)/2.d0)/                  &
         & TCON(MTYPE(COUNT1,JBASE+3,COUNT3)))))+T(COUNT1,JBASE+4,COUNT3)*      &
         & (TCON(MTYPE(COUNT1,JBASE+4,COUNT3))/(DY(JBASE+4)/2.d0)))/              &
         & ((1/(INS(COUNT1,JBASE+3,COUNT3)*REXT+((DY(JBASE+3)/2.d0)/              &
         & TCON(MTYPE(COUNT1,JBASE+3,COUNT3)))))+                               &
         & (TCON(MTYPE(COUNT1,JBASE+4,COUNT3))/(DY(JBASE+4)/2.d0)))
         TSWXZSum=TSWXZSum+TSWXZ(COUNT1,COUNT3)*DX(COUNT1)*DZ(COUNT3)
         TXZSumIn=TXZSumIn+TWW(COUNT1,COUNT3)*DX(COUNT1)*DZ(COUNT3)
         DAXZSum =DAXZSum +DX(COUNT1)*DZ(COUNT3)

!*** Split surface temperature calculation
         IF (NZAG.EQ.4) THEN
           IF (ZC(COUNT3).LE.ZC(1+(KBASE-NZAG)/2))THEN
             TSWXZUpperSum=TSWXZUpperSum+TSWXZ(COUNT1,COUNT3)*DX(COUNT1)*DZ(COUNT3)
             TSWXZUpperSumIn=TSWXZUpperSumIn+TWW(COUNT1,COUNT3)*DX(COUNT1)*DZ(COUNT3)
             DAXZUpperSum=DAXZUpperSum+DX(COUNT1)*DZ(COUNT3)
           ELSE
             TSWXZLowerSum=TSWXZLowerSum+TSWXZ(COUNT1,COUNT3)*DX(COUNT1)*DZ(COUNT3)
             TSWXZLowerSumIn=TSWXZLowerSumIn+TWW(COUNT1,COUNT3)*DX(COUNT1)*DZ(COUNT3)
             DAXZLowerSum=DAXZLowerSum+DX(COUNT1)*DZ(COUNT3)
           END IF
         ELSE
           IF (ZC(COUNT3).LE.(KEXT+DGRAVZP-DGRAVZN)/2.d0)THEN
             TSWXZUpperSum=TSWXZUpperSum+TSWXZ(COUNT1,COUNT3)*DX(COUNT1)*DZ(COUNT3)
             TSWXZUpperSumIn=TSWXZUpperSumIn+TWW(COUNT1,COUNT3)*DX(COUNT1)*DZ(COUNT3)
             DAXZUpperSum=DAXZUpperSum+DX(COUNT1)*DZ(COUNT3)
           ELSE
             TSWXZLowerSum=TSWXZLowerSum+TSWXZ(COUNT1,COUNT3)*DX(COUNT1)*DZ(COUNT3)
             TSWXZLowerSumIn=TSWXZLowerSumIn+TWW(COUNT1,COUNT3)*DX(COUNT1)*DZ(COUNT3)
             DAXZLowerSum=DAXZLowerSum+DX(COUNT1)*DZ(COUNT3)
           END IF
         END IF
       END DO
     END DO

     TSurfWallXZ=TSWXZSum/DAXZSum
     TSWallXZIn =TXZSumIn/DAXZSum
     TSurfWallXZUpper=TSWXZUpperSum/DAXZUpperSum
     TSurfWallXZUpperIn=TSWXZUpperSumIn/DAXZUpperSum
     TSurfWallXZLower=TSWXZLowerSum/DAXZLowerSum
     TSurfWallXZLowerIn=TSWXZLowerSumIn/DAXZLowerSum
!*** Initialize the variables for the centerline temperature calculation
     TSXZCL=0.0
     TSXZCLSum=0.0
!*** Calculate the average wall centerline temperature for comparisons
     DO COUNT3=0,KBASE-1
       TSXZCLSum=TSXZCLSum+TSWXZ(INT(IBASE/2),COUNT3)
     END DO
     TSXZCL=TSXZCLSum/(KBASE+1)

!*** Calculate the surface temperature field for the floor slab cells
     DO COUNT1=0,IBASE-1
       DO COUNT2=0,JBASE-1
         TSF(COUNT1,COUNT2)=((T(COUNT1,COUNT2,KBASE+1)*                          &
         & TCON(MTYPE(COUNT1,COUNT2,KBASE+1)))/(DZ(KBASE+1)/2.d0)+                 &
         & (T(COUNT1,COUNT2,KBASE+2)*TCON(MTYPE(COUNT1,COUNT2,KBASE+2)))/        &
         & (DZ(KBASE+2)/2.d0))/((TCON(MTYPE(COUNT1,COUNT2,KBASE+1))/               &
         & (DZ(KBASE+1)/2.d0))+(TCON(MTYPE(COUNT1,COUNT2,KBASE+2))/(DZ(KBASE+2)/2.d0)))
         TSFSum=TSFSum+TSF(COUNT1,COUNT2)*DX(COUNT1)*DY(COUNT2)
         TXYSumIn=TXYSumIn+TF(COUNT1,COUNT2)*DX(COUNT1)*DY(COUNT2)
         DAXYSum =DAXYSum+DX(COUNT1)*DY(COUNT2)
         IF(ABS(XC(COUNT1)).GT.(XFACE(IBASE)-2.d0).OR.ABS(YC(COUNT2)).GT.          &
           & (YFACE(JBASE)-2.)) THEN
           TSurfPerimSum=TSurfPerimSum+TSF(COUNT1,COUNT2)*DX(COUNT1)*DY(COUNT2)
           TSFPerimInSum=TSFPerimInSum+TF(COUNT1,COUNT2)*DX(COUNT1)*DY(COUNT2)
           DAPerim=DAPerim+DX(COUNT1)*DY(COUNT2)
         ELSE
           TSurfCoreSum=TSurfCoreSum+TSF(COUNT1,COUNT2)*DX(COUNT1)*DY(COUNT2)
           TSFCoreInSum=TSFCoreInSum+TF(COUNT1,COUNT2)*DX(COUNT1)*DY(COUNT2)
           DACore=DACore+DX(COUNT1)*DY(COUNT2)
         END IF
       END DO
     END DO
     TSurfFloor=TSFSum/DAXYSum
     TSFloorIn=TXYSumIn/DAXYSum
     TSurfFloorPerim=TSurfPerimSum/DAPerim
     TSurfFloorPerimIn=TSFPerimInSum/DAPerim
     TSurfFloorCore=TSurfCoreSum/DACore
     TSurfFloorCoreIn=TSFCoreInSum/DACore

!*** Initialize the variables for the centerline temperature calculation
     TSFXCL=0.0
     TSFXCLSum=0.0
     TSFYCL=0.0
     TSFYCLSum=0.0
!*** Calculate the average floor centerline temperature along the Y axis for comparisons
     DO COUNT2=0,JBASE-1
       TSFXCLSum=TSFXCLSum+TSF(0,COUNT2)
     END DO
     TSFXCL=TSFXCLSum/(JBASE)
!*** Calculate the average floor centerline temperature along the X axis for comparisons
     DO COUNT1=0,IBASE-1
       TSFYCLSum=TSFYCLSum+TSF(COUNT1,0)
     END DO
     TSFYCL=TSFYCLSum/(IBASE)
     RETURN
END SUBROUTINE SurfaceTemps

!***************************     EnergyPlus Output Stage       **************************
SUBROUTINE EPlusOutput(IHR,IDAY_LOCAL,TSurfWallXZ,TSurfWallYZ,TSurfFloor,                      &
                    & TSWallYZIn,TSWallXZIn,TSFloorIn,TSYZCL,TSXZCL,TSFXCL,     &
                    & TSFYCL,TSurfWallYZUpper,TSurfWallYZUpperIn,               &
                    & TSurfWallXZUpper,TSurfWallXZUpperIn,TSurfWallYZLower,     &
                    & TSurfWallYZLowerIn,TSurfWallXZLower,TSurfWallXZLowerIn,   &
                    & TSurfFloorPerim,TSurfFloorPerimIn,TSurfFloorCore,         &
                    & TSurfFloorCoreIn,FloorHeatFlux,CoreHeatFlux,              &
                    & PerimHeatFlux,XZWallHeatFlux,YZWallHeatFlux,              &
                    & UpperXZWallFlux,UpperYZWallFlux,LowerXZWallFlux,          &
                    & LowerYZWallFlux,TB,TCON_LOCAL)
! SUBROUTINE INFORMATION:
        !       AUTHOR         Edward D. Clements
        !       DATE WRITTEN   April 3, 2000
        !       MODIFIED       June 13, 2000
        !       MODIFIED       na
        !       RE-ENGINEERED  C O Pedersen Sept 2004

        ! PURPOSE OF THIS MODULE:
        ! This subroutine will write the output files of surface temperatures required
        ! by EnergyPlus

        ! METHODOLOGY EMPLOYED:
        ! This subroutine was developed using standard EnergyPlus modular formatting.

        ! MATHEMATIC METHODOLOGY
        ! na

        ! REFERENCES:
        ! na

        ! OTHER NOTES:
        ! na
USE BasementSimData
USE EPWRead, ONLY: LocationName
USE General

USE DataGlobals, ONLY: ShowSevereError,ShowWarningError,ShowFatalError

IMPLICIT NONE
     REAL(r64) TSurfWallYZ              ! Area weighted avg YZ wall surface temperature  [C]
     REAL(r64) TSurfWallXZ              ! Area weighted avg XZ wall surface temperature  [C]
     REAL(r64) TSurfFloor               ! Area weighted avg floor surface temperature    [C]
     REAL(r64) TSFloorIn                ! Checking output of T(k=k-1)                    [C]
     REAL(r64) TSWallYZIn               ! Checking output of T(i=i-1)                    [C]
     REAL(r64) TSWallXZIn               ! Checking output of T(j=j-1)                    [C]
     REAL(r64) TSFXCL                   ! Floor surface temperature on the Y centerline  [C]
     REAL(r64) TSFYCL                   ! Floor surface temperature on the X centerline  [C]
     REAL(r64) TSYZCL                   ! YZ Wall surface temp on the centerline         [C]
     REAL(r64) TSXZCL                   ! XZ Wall surface temp on the centerline         [C]
     REAL(r64) TSurfWallYZUpper         ! Upper band YZ wall temperature                 [C]
     REAL(r64) TSurfWallYZUpperIn       ! Upper band YZ inside wall temperature          [C]
     REAL(r64) TSurfWallYZLower         ! Lower band YZ wall tepmerature                 [C]
     REAL(r64) TSurfWallYZLowerIn       ! Lower band YZ inside wall temperature          [C]
     REAL(r64) TSurfWallXZLower         ! Lower band XZ wall temperature                 [C]
     REAL(r64) TSurfWallXZLowerIn       ! Lower band XZ inside wall temperature          [C]
     REAL(r64) TSurfWallXZUpper         ! Upper band XZ wall temperature                 [C]
     REAL(r64) TSurfWallXZUpperIn       ! Upper band XZ wall inside temperature          [C]
     REAL(r64) TSurfFloorPerim          ! Perimeter area floor surface temperature       [C]
     REAL(r64) TSurfFloorPerimIn        ! Perimeter area floor inside surface temp       [C]
     REAL(r64) TSurfFloorCore           ! Core area floor surface temperature            [C]
     REAL(r64) TSurfFloorCoreIn         ! Core area floor inside surface temperature     [C]
     REAL(r64) FloorHeatFlux            ! Area weighted average floor heat flux     [W/m**2]
     REAL(r64) CoreHeatFlux             ! Area weighted average core zone floor q"  [W/m**2]
     REAL(r64) PerimHeatFlux            ! Area weighted average floor perimeter q"  [W/m**2]
     REAL(r64) XZWallHeatFlux           ! Area weighted average XZ wall q"          [W/m**2]
     REAL(r64) YZWallHeatFlux           ! Area weighted average YZ wall q"          [W/m**2]
     REAL(r64) UpperXZWallFlux          ! Area weighted avg upper band XZ wall q"   [W/m**2]
     REAL(r64) UpperYZWallFlux          ! Area weighted avg upper band YZ wall q"   [W/m**2]
     REAL(r64) LowerXZWallFlux          ! Area weighted avg lower band XZ wall q"   [W/m**2]
     REAL(r64) LowerYZWallFlux          ! Area weighted avg lower band YZ wall q"   [W/m**2]
     REAL(r64) TB                       ! Basement temperature for a given hour          [C]
     REAL(r64) RWall                    ! Basement  bare wall thermal resistance    [ C/(W/m**2)]
     REAL(r64) RFloor                   ! Basement bare floor thermal resistance    [ C/(W/m**2)]
     REAL(r64) RradPlusConvHor          ! Horrizontal radiation plus convection thermal resistance [ C/(W/m**2)]
     REAL(r64) RradPlusConvVert         ! Vertical radiation plus convection thermal resistance [ C/(W/m**2)]
     REAL(r64) TCON_LOCAL(6)                  ! Thermal conductivites of materials  [ W/m-C]
     !REAL HIN (6)                  ! Inside convection coefficients  [W/(m**2 C]
     !REAL DWALL
     !REAL DSLAB
     Integer IDAY_LOCAL

!*** Set up monthly averages

     REAL(r64),SAVE ::MonthlyTSurfWallYZ               ! Area weighted avg YZ wall surface temperature  [C]
     REAL(r64),SAVE ::MonthlyTSurfWallXZ               ! Area weighted avg XZ wall surface temperature  [C]
     REAL(r64),SAVE ::MonthlyTSurfFloor                ! Area weighted avg floor surface temperature    [C]
     REAL(r64),SAVE ::MonthlyTSFloorIn                 ! Checking output of T(k=k-1)                    [C]
     REAL(r64),SAVE ::MonthlyTSWallYZIn                ! Checking output of T(i=i-1)                    [C]
     REAL(r64),SAVE ::MonthlyTSWallXZIn                ! Checking output of T(j=j-1)                    [C]
     REAL(r64),SAVE ::MonthlyTSFXCL                    ! Floor surface temperature on the Y centerline  [C]
     REAL(r64),SAVE ::MonthlyTSFYCL                    ! Floor surface temperature on the X centerline  [C]
     REAL(r64),SAVE ::MonthlyTSYZCL                    ! YZ Wall surface temp on the centerline         [C]
     REAL(r64),SAVE ::MonthlyTSXZCL                    ! XZ Wall surface temp on the centerline         [C]
     REAL(r64),SAVE ::MonthlyTSurfWallYZUpper          ! Upper band YZ wall temperature                 [C]
     REAL(r64),SAVE ::MonthlyTSurfWallYZUpperIn        ! Upper band YZ inside wall temperature          [C]
     REAL(r64),SAVE ::MonthlyTSurfWallYZLower          ! Lower band YZ wall tepmerature                 [C]
     REAL(r64),SAVE ::MonthlyTSurfWallYZLowerIn        ! Lower band YZ inside wall temperature          [C]
     REAL(r64),SAVE ::MonthlyTSurfWallXZLower          ! Lower band XZ wall temperature                 [C]
     REAL(r64),SAVE ::MonthlyTSurfWallXZLowerIn        ! Lower band XZ inside wall temperature          [C]
     REAL(r64),SAVE ::MonthlyTSurfWallXZUpper          ! Upper band XZ wall temperature                 [C]
     REAL(r64),SAVE ::MonthlyTSurfWallXZUpperIn        ! Upper band XZ wall inside temperature          [C]
     REAL(r64),SAVE ::MonthlyTSurfFloorPerim           ! Perimeter area floor surface temperature       [C]
     REAL(r64),SAVE ::MonthlyTSurfFloorPerimIn         ! Perimeter area floor inside surface temp       [C]
     REAL(r64),SAVE ::MonthlyTSurfFloorCore            ! Core area floor surface temperature            [C]
     REAL(r64),SAVE ::MonthlyTSurfFloorCoreIn          ! Core area floor inside surface temperature     [C]
     REAL(r64),SAVE ::MonthlyFloorHeatFlux             ! Area weighted average floor heat flux     [W/m**2]
     REAL(r64),SAVE ::MonthlyCoreHeatFlux              ! Area weighted average core zone floor q"  [W/m**2]
     REAL(r64),SAVE ::MonthlyPerimHeatFlux             ! Area weighted average floor perimeter q"  [W/m**2]
     REAL(r64),SAVE ::MonthlyXZWallHeatFlux            ! Area weighted average XZ wall q"          [W/m**2]
     REAL(r64),SAVE ::MonthlyYZWallHeatFlux            ! Area weighted average YZ wall q"          [W/m**2]
     REAL(r64),SAVE ::MonthlyUpperXZWallFlux           ! Area weighted avg upper band XZ wall q"   [W/m**2]
     REAL(r64),SAVE ::MonthlyUpperYZWallFlux           ! Area weighted avg upper band YZ wall q"   [W/m**2]
     REAL(r64),SAVE ::MonthlyLowerXZWallFlux           ! Area weighted avg lower band XZ wall q"   [W/m**2]
     REAL(r64),SAVE ::MonthlyLowerYZWallFlux           ! Area weighted avg lower band YZ wall q"   [W/m**2]
     REAL(r64),SAVE ::MonthlyTempArray(12,4)           ! Saved values of surface temps for OSC input generatrion
                                                  ! j1= wallave,j2=floor,j3=wallupper,j4=walllower



    INTEGER IHR                   ! Hour number counter
     Integer LastDayInMonth(12)       ! Number of days in each month                     []
     Integer DaysInMonth(12)
     INTEGER CurrentMonth           !  Current Month number
     INTEGER MonthIndex
     INTEGER I,J
     INTEGER OldCurrentMonth
     CHARACTER*1 Delimit
     CHARACTER*35 ScheduleName(4)
     CHARACTER(LEN=35) :: surfpOSCName(4)
     CHARACTER(LEN=40) :: stringOut
     Data  LastDayInMonth / 31,59,90,120,151,181,212,243,273,304,334,365/
     Data DaysInMonth / 31,28,31,30,31,30,31,31,30,31,30,31/

     Data OldCurrentMonth /1/

!  Find current month
      Do MonthIndex = 1,12
        IF (IDAY_LOCAL <= LastDayInMonth(MonthIndex)) Then
            CurrentMonth = MonthIndex
            Exit
        End IF
      End Do

    IF (isnan(TSurfWallYZ) .or.    &
        isnan(TSurfWallXZ) .or.    &
        isnan(TSurfFloor) .or.    &
        isnan(TSFloorIn) .or.    &
        isnan(TSWallYZIn) .or.    &
        isnan(TSWallXZIn) .or.    &
        isnan(TSFXCL) .or.    &
        isnan(TSFYCL) .or.    &
        isnan(TSYZCL) .or.    &
        isnan(TSXZCL) .or.    &
        isnan(TSurfWallYZUpper) .or.    &
        isnan(TSurfWallYZUpperIn) .or.    &
        isnan(TSurfWallYZLower) .or.    &
        isnan(TSurfWallYZLowerIn) .or.    &
        isnan(TSurfWallXZLower) .or.    &
        isnan(TSurfWallXZLowerIn) .or.    &
        isnan(TSurfWallXZUpper) .or.    &
        isnan(TSurfWallXZUpperIn) .or.    &
        isnan(TSurfFloorPerim) .or.    &
        isnan(TSurfFloorPerimIn) .or.    &
        isnan(TSurfFloorCore) .or.    &
        isnan(TSurfFloorCoreIn) .or.    &
        isnan(FloorHeatFlux) .or.    &
        isnan(CoreHeatFlux) .or.    &
        isnan(PerimHeatFlux) .or.    &
        isnan(XZWallHeatFlux) .or.    &
        isnan(YZWallHeatFlux) .or.    &
        isnan(UpperXZWallFlux) .or.    &
        isnan(UpperYZWallFlux) .or.    &
        isnan(LowerXZWallFlux) .or.    &
        isnan(LowerYZWallFlux) ) THEN

      CALL ShowSevereError('EPlusOutput: Calculations have resulted in invalid results (NAN).'//  &
         ' (Could be divide by zero. Overflows. Other causes.) Check Input Parameters. '//  &
         ' Hint: Start from defaults for each object and work forward.')
      IF (ABS(SimParams%F-.2d0) <= .04d0) THEN
        CALL ShowWarningError('EPlusOutput: ADI multiplier=['//trim(RoundSigDigits(SimParams%F,4))//  &
          ']. Some runs have shown errors with this result.  Try a higher (>=.25 or lower <=.19) value.')
      ENDIF
      CALL ShowFatalError('Program terminates due to preceding condition(s).')

    ENDIF

     IF (CurrentMonth > OldCurrentMonth) Then
         MonthlyTSurfWallYZ = TSurfWallYZ/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyTSurfWallXZ = TSurfWallXZ/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyTSurfFloor = TSurfFloor/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyTSFloorIn = TSFloorIn/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyTSWallYZIn = TSWallYZIn/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyTSWallXZIn = TSWallXZIn/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyTSFXCL = TSFXCL/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyTSFYCL = TSFYCL/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyTSYZCL = TSYZCL/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyTSXZCL = TSXZCL/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyTSurfWallYZUpper = TSurfWallYZUpper/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyTSurfWallYZUpperIn  =TSurfWallYZUpperIn/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyTSurfWallYZLower = TSurfWallYZLower/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyTSurfWallYZLowerIn = TSurfWallYZLowerIn/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyTSurfWallXZLower = TSurfWallXZLower/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyTSurfWallXZLowerIn = TSurfWallXZLowerIn/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyTSurfWallXZUpper = TSurfWallXZUpper/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyTSurfWallXZUpperIn  =TSurfWallXZUpperIn/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyTSurfFloorPerim = TSurfFloorPerim/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyTSurfFloorPerimIn = TSurfFloorPerimIn/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyTSurfFloorCore = TSurfFloorCore/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyTSurfFloorCoreIn = TSurfFloorCoreIn/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyFloorHeatFlux  = FloorHeatFlux/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyCoreHeatFlux  = CoreHeatFlux/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyPerimHeatFlux = PerimHeatFlux/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyXZWallHeatFlux = XZWallHeatFlux/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyYZWallHeatFlux  = YZWallHeatFlux/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyUpperXZWallFlux = UpperXZWallFlux/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyUpperYZWallFlux = UpperYZWallFlux/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyLowerXZWallFlux = LowerXZWallFlux/Float(DaysInMonth(CurrentMonth)*24)
         MonthlyLowerYZWallFlux = LowerYZWallFlux/Float(DaysInMonth(CurrentMonth)*24)


          OldCurrentMonth=CurrentMonth
    ELSE

        MonthlyTSurfWallYZ=MonthlyTSurfWallYZ + TSurfWallYZ/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyTSurfWallXZ=MonthlyTSurfWallXZ + TSurfWallXZ/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyTSurfFloor=MonthlyTSurfFloor + TSurfFloor/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyTSFloorIn=MonthlyTSFloorIn + TSFloorIn/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyTSWallYZIn=MonthlyTSWallYZIn + TSWallYZIn/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyTSWallXZIn=MonthlyTSWallXZIn + TSWallXZIn/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyTSFXCL=MonthlyTSFXCL + TSFXCL/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyTSFYCL=MonthlyTSFYCL + TSFYCL/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyTSYZCL=MonthlyTSYZCL + TSYZCL/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyTSXZCL=MonthlyTSXZCL + TSXZCL/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyTSurfWallYZUpper=MonthlyTSurfWallYZUpper + TSurfWallYZUpper/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyTSurfWallYZUpperIn=MonthlyTSurfWallYZUpperIn  +TSurfWallYZUpperIn/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyTSurfWallYZLower=MonthlyTSurfWallYZLower + TSurfWallYZLower/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyTSurfWallYZLowerIn=MonthlyTSurfWallYZLowerIn + TSurfWallYZLowerIn/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyTSurfWallXZLower=MonthlyTSurfWallXZLower + TSurfWallXZLower/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyTSurfWallXZLowerIn=MonthlyTSurfWallXZLowerIn + TSurfWallXZLowerIn/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyTSurfWallXZUpper=MonthlyTSurfWallXZUpper + TSurfWallXZUpper/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyTSurfWallXZUpperIn=MonthlyTSurfWallXZUpperIn  +TSurfWallXZUpperIn/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyTSurfFloorPerim=MonthlyTSurfFloorPerim + TSurfFloorPerim/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyTSurfFloorPerimIn=MonthlyTSurfFloorPerimIn + TSurfFloorPerimIn/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyTSurfFloorCore=MonthlyTSurfFloorCore + TSurfFloorCore/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyTSurfFloorCoreIn=MonthlyTSurfFloorCoreIn + TSurfFloorCoreIn/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyFloorHeatFlux=MonthlyFloorHeatFlux  + FloorHeatFlux/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyCoreHeatFlux=MonthlyCoreHeatFlux  + CoreHeatFlux/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyPerimHeatFlux=MonthlyPerimHeatFlux + PerimHeatFlux/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyXZWallHeatFlux=MonthlyXZWallHeatFlux + XZWallHeatFlux/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyYZWallHeatFlux=MonthlyYZWallHeatFlux  + YZWallHeatFlux/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyUpperXZWallFlux=MonthlyUpperXZWallFlux + UpperXZWallFlux/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyUpperYZWallFlux=MonthlyUpperYZWallFlux + UpperYZWallFlux/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyLowerXZWallFlux=MonthlyLowerXZWallFlux + LowerXZWallFlux/Float(DaysInMonth(CurrentMonth)*24)
        MonthlyLowerYZWallFlux=MonthlyLowerYZWallFlux + LowerYZWallFlux/Float(DaysInMonth(CurrentMonth)*24)


     End IF

     IF(IDAY_LOCAL == LastDayInMonth(OldCurrentMonth) .and. IHR == 24) Then
          If (OldCurrentMonth == 1) Then   !write heading in csv form
               Write (EPMonthly,'(A)') 'Weather File Location='//trim(LocationName)
               Write (EPMonthly,'(64(A))') 'Month,',' ZoneTemp,',  &
                      'MonthlyTSurfWall[C] ,' ,  &
                      'MonthlyTSWallIn[C],',  &
!                      'MonthlyTSurfWallXZ,',  &
                       'MonthlyTSurfFloor[C],',  &
                       'MonthlyTSFloorIn[C],',  &
!                       'MonthlyTSWallXZIn,',  &
!                       'MonthlyTSFXCL,',  &
!                       'MonthlyTSFYCL,',  &
!                        'MonthlyTSYZCL,' ,  &
!                        'MonthlyTSXZCL,',  &
                        'MonthlyTSurfWallUpper[C],',  &
                        'MonthlyTSurfWallUpperIn[C],',  &
                        'MonthlyTSurfWallLower[C],',  &
                        'MonthlyTSurfWallLowerIn[C],',  &
!                        'MonthlyTSurfWallXZLower,',  &
!                        'MonthlyTSurfWallXZLowerIn,' ,  &
!                       'MonthlyTSurfWallXZUpper,' ,  &
!                        'MonthlyTSurfWallXZUpperIn,' ,  &
                       ! 'MonthlyTSurfFloorPerim ,' ,  &
                       ! 'MonthlyTSurfFloorPerimIn,' ,  &
                       ! 'MonthlyTSurfFloorCore,' ,  &
                       ! 'MonthlyTSurfFloorCoreIn,' ,  &
                        'MonthlyFloorHeatFlux[W/m**2] ,' ,  &
                      !  'MonthlyCoreHeatFlux ,' ,  &
                       ! 'MonthlyPerimHeatFlux ,' ,  &
 !                       'MonthlyXZWallHeatFlux ,' ,  &
                        'MonthlyWallHeatFlux[W/m**2] ,' ,  &
 !                       'MonthlyUpperXZWallFlux,' ,  &
                        'MonthlyUpperWallFlux[W/m**2] ,' ,  &
 !                       'MonthlyLowerXZWallFlux ,' ,  &
                        'MonthlyLowerWallFlux[W/m**2] ,'
          end if   ! For heading
!
!         Write(SurfaceTemps,'(9X,I2,2X,2X,A7,3X,A7,4X,A7,4x,F7.1)') OldCurrentMonth,TRIM(TrimSigDigits(MonthlySurfaceTemp,2)),  &
 !                            TRIM(TrimSigDigits(MonthlyPerimeterTemp,2)),TRIM(TrimSigDigits(MonthlyCoreTemp,2)), &
!                            TIN
!  Override the previous calculations with monthly averages using heat flux and wall/floor resistance.
!  Heat flux is assumed to be correct since it is based on a simple surface calculation
!  Outside surface temps are based on a more detailed calculation, and depend on grid details that are
!   not available at this time.  (10/1/04  COP)
!  For EnergyPlus monthly details, this calculation is much more straightforward and simple.

     RWall = BuildingData%DWALL/TCON_LOCAL(1)
     RFloor = BuildingData%DSLAB/TCON_LOCAL(2)
     RradPlusConvHor = 1/Interior%HIN(6)
     RradPlusConvVert = 1/Interior%HIN(4)

     MonthlyTSurfWallYZ = MonthlyTSWallYZIn - MonthlyYZWallHeatFlux*RWall
     MonthlyTSurfFloor = MonthlyTSFloorIn - MonthlyFloorHeatFlux*RFloor
     MonthlyTSurfWallYZUpper = MonthlyTSurfWallYZUpperIn - MonthlyUpperYZWallFlux*RWall
     MonthlyTSurfWallYZLower = MonthlyTSurfWallYZLowerIn -MonthlyLowerYZWallFlux * RWall
     MonthlyTempArray(OldCurrentMonth,1) = MonthlyTSurfWallYZ
     MonthlyTempArray(OldCurrentMonth,2) = MonthlyTSurfFloor
     MonthlyTempArray(OldCurrentMonth,3) = MonthlyTSurfWallYZUpper
     MonthlyTempArray(OldCurrentMonth,4) = MonthlyTSurfWallYZLower
     !                1234567890123456789012345678901234567890
     ScheduleName(1)='scheduleOSCBasementWallSurfaceTemp,'
     ScheduleName(2)='scheduleOSCBasementFloorTemp,      '
     ScheduleName(3)='scheduleOSCBasementUpperWallTemp,  '
     ScheduleName(4)='scheduleOSCBasementLowerWallTemp,  '

     surfpOSCName(1)='surfPropOthSdCoefBasementAvgWall,  '
     surfpOSCName(2)='surfPropOthSdCoefBasementAvgFloor, '
     surfpOSCName(3)='surfPropOthSdCoefBasementUpperWall,'
     surfpOSCName(4)='surfPropOthSdCoefBasementLowerWall,'


           WRITE (EPMonthly,'(I7,A,31(F10.2,A),F10.2)') OldCurrentMonth,',', TB,',', &
                         MonthlyTSurfWallYZ , ',',  &
                         MonthlyTSWallYZIn , ',',  &
                      !   MonthlyTSurfWallXZ , ',',  &
                         MonthlyTSurfFloor , ',',  &
                         MonthlyTSFloorIn , ',',  &
                       !  MonthlyTSWallXZIn , ',',  &
                       !  MonthlyTSFXCL ,',',  &
                       !  MonthlyTSFYCL , ',',  &
                       !  MonthlyTSYZCL  , ',',  &
                       !  MonthlyTSXZCL , ',',  &
                         MonthlyTSurfWallYZUpper , ',',  &
                         MonthlyTSurfWallYZUpperIn , ',',  &
                         MonthlyTSurfWallYZLower  , ',',  &
                         MonthlyTSurfWallYZLowerIn , ',',  &
                      !   MonthlyTSurfWallXZLower  , ',',  &
                       !  MonthlyTSurfWallXZLowerIn , ',',  &
                       !  MonthlyTSurfWallXZUpper , ',',  &
                       !  MonthlyTSurfWallXZUpperIn , ',',  &
                       !  MonthlyTSurfFloorPerim  , ',',  &
                       !  MonthlyTSurfFloorPerimIn , ',',  &
                       !  MonthlyTSurfFloorCore , ',',  &
                       !  MonthlyTSurfFloorCoreIn , ',',  &
                         MonthlyFloorHeatFlux  , ',',  &
                       !  MonthlyCoreHeatFlux  , ',',  &
                       !  MonthlyPerimHeatFlux  , ',',  &

                      !   MonthlyXZWallHeatFlux  , ',',  &
                         MonthlyYZWallHeatFlux  , ',',  &
                      !   MonthlyUpperXZWallFlux , ',',  &
                         MonthlyUpperYZWallFlux  , ',',  &
                      !   MonthlyLowerXZWallFlux  , ',',  &
                         MonthlyLowerYZWallFlux

          IF (OldCurrentMonth == 12)Then
! The following lines were present prior to Glazer reworking the output to use specific objects Jan 2010
!
!               Write(EPObjects, '(A,4/)')
!               Write(EPObjects, '(A)') '! Weather File Location='//trim(LocationName)
!               Write(EPObjects, '(A)') '! OTHER SIDE COEFFICIENT OBJECT EXAMPLE FOR IDF FILE', &
!                   ' SurfaceProperty:OtherSideCoefficients,',     &
!                   ' ExampleOSC,                !- OtherSideCoeff Name ***CHANGE THIS!*** ', &
!                   ' 0,                         !- Combined convective/radiative film coefficient',  &
!                   ' 1,                         !- N2,User selected Constant Temperature {C}',  &
!                   ' 1,                         !- Coefficient modifying the user selected constant temperature', &
!                   ' 0,                         !- Coefficient modifying the external dry bulb temperature', &
!                   ' 0,                         !- Coefficient modifying the ground temperature ', &
!                   ' 0,                         !- Coefficient modifying the wind speed term (s/m)', &
!                   ' 0,                         !- Coefficient modifying the zone air temperature ', &
!                   '                            !  part of the equation',  &
!                   ' GroundTempCompactSchedName; !- Name of Schedule for values of const',  &
!                   '                            ! temperature. Schedule values replace N2.', &
!                   '                            !  ***REPLACE WITH CORRECT NAME***'
!             Do J = 1,4
!               Write(EPObjects,'(//)')
!               Write(EPObjects, '(A)') 'Schedule:Compact,', &
!                     ScheduleName(j), &
!                   'Temperature ,            !- ScheduleType'
!               Delimit = ','
!                 DO I= 1,12
!                    IF( I == 12) Delimit = ';'
!                    Write(EPObjects,'(A,I4,A,I2,A)')'Through:',I,'/',DaysInMonth(i),','
!                    Write(EPObjects,'(A)')'For:AllDays,'
!                    Write(EPObjects,'(A)')'Until:24:00,'
!                    Write(EPObjects,'(G10.4,A)')MonthlyTempArray(I,J),Delimit
!                 END DO
!             END DO

              WRITE(EPObjects, '(A)') ' '
              WRITE(EPObjects, '(A)') ' ! ========================================================================'
              WRITE(EPObjects, '(A)') ' ! The following was created by the Basement preprocessor program.'
              WRITE(EPObjects, '(A)') ' ! Weather File Location=' // TRIM(LocationName)
              WRITE(EPObjects, '(A)') ' ! '
              DO J = 1,4
                WRITE(EPObjects, '(A)') ' '
                WRITE(EPObjects, '(A)') 'SurfaceProperty:OtherSideCoefficients,'
                WRITE(EPObjects, '(A)') '  ' // surfpOSCName(J) // '  !- Name'
                WRITE(EPObjects, '(A)') '  0.0,                                 !- Combined Convective Radiative Film Coefficient'
                WRITE(EPObjects, '(A)') '  1.0,                                 !- Constant Temperature'
                WRITE(EPObjects, '(A)') '  1.0,                                 !- Constant Temperature Coefficient'
                WRITE(EPObjects, '(A)') '  0.0,                                 !- External Dry-Bulb Temperature Coefficient'
                WRITE(EPObjects, '(A)') '  0.0,                                 !- Ground Temperature Coefficient'
                WRITE(EPObjects, '(A)') '  0.0,                                 !- Wind Speed Coefficient'
                WRITE(EPObjects, '(A)') '  0.0,                                 !- Zone Air Temperature Coefficient'
                WRITE(EPObjects, '(A)') '  ' // ScheduleName(J) // '  !- Constant Temperature Schedule Name'
                WRITE(EPObjects, '(A)') '  No,                                  !- Sinusoidal Variation of Constant Temperature Coefficient'
                WRITE(EPObjects, '(A)') '  24;                                  !- Period of Sinusoidal Variation'
                WRITE(EPObjects, '(A)') ' '
                WRITE(EPObjects, '(A)') 'Schedule:Compact,'
                WRITE(EPObjects, '(A)') '  ' // ScheduleName(J) // '  !- Name'
                WRITE(EPObjects, '(A)') '  Temperature,                         !- ScheduleType'
                Delimit = ','
                DO I = 1 , 12
                  IF (I .EQ. 12) Delimit = ';'
                  WRITE(EPObjects,'(A,I4,A,I2,A)')'  Through:',I,'/',DaysInMonth(i),',                     !- Field'
                  WRITE(EPObjects,'(A)')'  For:AllDays,                         !- Field'
                  WRITE(EPObjects,'(A)')'  Until:24:00,                         !- Field'
                  WRITE(FMT='(G10.4)', UNIT=stringOut) MonthlyTempArray(I,J)
                  WRITE(EPObjects,'(2X,A,A,A)') TRIM(ADJUSTL(stringOut)),Delimit,'                               !- Field'
                END DO
              END DO
          END IF   !  OldCurrentMonth == 12




!    Reset to zero
        MonthlyTSurfWallYZ =0
        MonthlyTSurfWallXZ =0
        MonthlyTSurfFloor =0
        MonthlyTSFloorIn =0
        MonthlyTSWallYZIn =0
        MonthlyTSWallXZIn =0
        MonthlyTSFXCL = 0
        MonthlyTSFYCL =0
        MonthlyTSYZCL  =0
        MonthlyTSXZCL =0
        MonthlyTSurfWallYZUpper =0
        MonthlyTSurfWallYZUpperIn =0
        MonthlyTSurfWallYZLower  =0
        MonthlyTSurfWallYZLowerIn =0
        MonthlyTSurfWallXZLower  =0
        MonthlyTSurfWallXZLowerIn =0
        MonthlyTSurfWallXZUpper =0
        MonthlyTSurfWallXZUpperIn =0
        MonthlyTSurfFloorPerim  =0
        MonthlyTSurfFloorPerimIn =0
        MonthlyTSurfFloorCore =0
        MonthlyTSurfFloorCoreIn =0
        MonthlyFloorHeatFlux  =0
        MonthlyCoreHeatFlux  =0
        MonthlyPerimHeatFlux  =0
        MonthlyXZWallHeatFlux  =0
        MonthlyYZWallHeatFlux  =0
        MonthlyUpperXZWallFlux =0
        MonthlyUpperYZWallFlux  =0
        MonthlyLowerXZWallFlux  =0
        MonthlyLowerYZWallFlux  =0

      END IF




!*** WRITE THE SURFACE TEMPERATURES   (original hourly output)
!*** YZ Plane Wall
 !    WRITE (XZWallTs,*) IDAY,TSurfWallYZ,TSWallYZIn,YZWallHeatFlux,TB
!*** XZ Plane Wall
 !    WRITE (YZWallTs,*) IDAY,TSurfWallXZ,TSWallXZIn,XZWallHeatFlux,TB
!*** Floor
!     WRITE (FloorTs,*) IDAY,TSurfFloor,TSFloorIn,FloorHeatFlux,TB
!*** YZ and XZ Plane wall, and Floor Centerline temperatures for comparison
!     WRITE (Centerline,*) IDAY,TSYZCL,TSXZCL,TSFXCL,TSFYCL

!*** WRITE THE SPLIT SURFACE TEMPERATURES
!*** YZ Plane Wall
!     WRITE (YZWallSplit,6131) IDAY,TSurfWallYZUpper,TSurfWallYZUpperIn,   &
 !         & TSurfWallYZLower,TSurfWallYZLowerIn,UpperYZWallFlux,LowerYZWallFlux,TB
!6131 FORMAT (I4,F9.4,3X,F9.4,3X,F9.4,3X,F9.4,3X,F9.4,3X,F9.4,3X,F9.4)
!*** XZ Plane Wall
 !    WRITE (XZWallSplit,6131) IDAY,TSurfWallXZUpper,TSurfWallXZUpperIn,   &
!          & TSurfWallXZLower,TSurfWallXZLowerIn,UpperXZWallFlux,LowerXZWallFlux,TB
!*** Floor
 !    WRITE (FloorSplit,6131) IDAY,TSurfFloorPerim,TSurfFloorPerimIn,     &
  !        & TSurfFloorCore,TSurfFloorCoreIn,PerimHeatFlux,CoreHeatFlux,TB

     RETURN
END SUBROUTINE EPlusOutput

REAL(r64) FUNCTION COSD(degree_value)

  REAL(r64), PARAMETER :: pi = 3.1415926535
  REAL(r64) degree_value
  ! Coding this manually since it was a library dependent function in Developer Studio
  COSD = COS(degree_value*pi/180)

END FUNCTION

REAL(r64) FUNCTION ACOSD(degree_value)

  REAL(r64), PARAMETER :: pi = 3.1415926535
  REAL(r64) degree_value
  ! Coding this manually since it was a library dependent function in Developer Studio
  ACOSD = ACOS(degree_value*pi/180)

END FUNCTION

REAL(r64) FUNCTION SIND(degree_value)

  REAL(r64), PARAMETER :: pi = 3.1415926535
  REAL(r64) degree_value
  ! Coding this manually since it was a library dependent function in Developer Studio
  SIND = SIN(degree_value*pi/180)

END FUNCTION

REAL(r64) FUNCTION ASIND(degree_value)

  REAL(r64), PARAMETER :: pi = 3.1415926535
  REAL(r64) degree_value
  ! Coding this manually since it was a library dependent function in Developer Studio
  ASIND = ASIN(degree_value*pi/180)

END FUNCTION

!***********************  WRITING HEADERS IN THE EPlus OUTPUT FILE  *********************
SUBROUTINE EPlusHeader
! SUBROUTINE INFORMATION:
        !       AUTHOR         Edward D. Clements
        !       DATE WRITTEN   April 4, 2000
        !       MODIFIED       June 13, 2000
        !       MODIFIED       na
        !       RE-ENGINEERED  na

        ! PURPOSE OF THIS MODULE:
        ! This subroutine will format the output files of surface temperatures required
        ! by EnergyPlus

        ! METHODOLOGY EMPLOYED:
        ! This subroutine was developed using standard EnergyPlus modular formatting.

        ! MATHEMATIC METHODOLOGY
        ! na

        ! REFERENCES:
        ! na

        ! OTHER NOTES:
        ! na
IMPLICIT NONE
     WRITE  (XZWallTs,4300)
4300 FORMAT (7X,'YZ Wall TS',9X,'Inside',6X,'YZ Heat flux',5X,'Base T')
     WRITE  (YZWallTs,4301)
4301 FORMAT (7X,'XZ Wall TS',9X,'Inside',6X,'XZ Heat flux',5X,'Base T')
     WRITE  (FloorTs,4302)
4302 FORMAT (7X,'Floor TS',7X,'Inside',6X,'Floor heat flux',5X,'Base T')
     WRITE  (Centerline,4303)
4303 FORMAT (7X,'YZ CL',11X,'XZ CL',11X,'FX CL',11X,'FY CL')
     WRITE  (YZWallSplit,4304)
4304 FORMAT (2X,'TSYZUp',4X,'TSYZUpIn',5X,'TSYZLo',5X,'TSYZLoIn',      &
            & 4X,'Upper q',5X,'Lower q',5X,'Base T')
     WRITE  (XZWallSplit,4305)
4305 FORMAT (2X,'TSXZUp',4X,'TSXZUpIn',5X,'TSXZLo',5X,'TSXZLoIn',4X,   &
            & 'Upper q',5X,'Lower q',5X,'Base T')
     WRITE  (FloorSplit,4306)
4306 FORMAT (2X,'TFPerim',3X,'TFPerimIn',5X,'TFCore',4X,'TFCoreIn',6X, &
            & 'Perim q',3X,'Core q',5X,'Base T')
     RETURN
END SUBROUTINE EPlusHeader

!*******************  CALCULATE THE AVERAGE HEAT FLUXES IN EACH ZONE  *******************
SUBROUTINE AvgHeatFlux(DACore,DAPerim,XC,YC,ZC,DX,DY,DZ,QWS,QWW,QF,XDIM,YDIM,   &
           & ZDIM,FloorHeatFlux,CoreHeatFlux,PerimHeatFlux,XZWallHeatFlux,      &
           & YZWallHeatFlux,UpperXZWallFlux,UpperYZWallFlux,LowerXZWallFlux,    &
           & LowerYZWallFlux,DAYZUpperSum,DAYZLowerSum,DAXZUpperSum,            &
           & DAXZLowerSum,DAXZSum,DAYZSum,DAXYSum)
! SUBROUTINE INFORMATION:
     !       AUTHOR         Edward Clements
     !       DATE WRITTEN   June 13, 2000
     !       MODIFIED       August 17, 2000
     !       RE-ENGINEERED  na
     !      VERSION NUMBER 1.0
     !
     ! PURPOSE OF THIS MODULE:
     ! This subroutine will close all input and output files not closed in the main
     ! simulation block.

     ! METHODOLOGY EMPLOYED:
     ! Standard EnergyPlus subroutine formatting for subroutines

     ! REFERENCES: none

     ! OTHER NOTES: none
USE BasementSimData
IMPLICIT NONE

!*** Variable declarations
! Pass through variables
     INTEGER XDIM           ! Array dimensioning constant for use with surface  []
                            ! temperature and flux calculation variables
     INTEGER YDIM           ! Array dimensioning constant for use with surface  []
                            ! temperature and flux calculation variables
     INTEGER ZDIM           ! Array dimensioning constant for use with surface  []
                            ! temperature and flux calculation variables

     REAL(r64) DACore            ! Core zone floor area                          [m**2]
     REAL(r64) DAPerim           ! Perimeter zone floor area                     [m**2]
     REAL(r64) XC(0:100)         ! X Direction cell center coordinates              [m]
     REAL(r64) YC(0:100)         ! Y Direction cell center coordinates              [m]
     REAL(r64) ZC(-35:100)       ! Z Direction cell center coordinates              [m]
     REAL(r64) DX(0:100)         ! Array of cell dimensions                         [m]
     REAL(r64) DY(0:100)         ! Array of cell dimensions                         [m]
     REAL(r64) DZ(-35:100)       ! Array of cell dimensions                         [m]
     REAL(r64) QWS
     DIMENSION QWS(0:YDIM,-35:ZDIM) ! South wall cell surface heat flux    [W/m^2]
                                    ! for one time step
     REAL(r64) QWW
     DIMENSION QWW(0:XDIM,-35:ZDIM) ! West wall cell surface heat flux     [W/m^2]
                                    ! for one time step

     REAL(r64) QF
     DIMENSION QF(0:XDIM,0:YDIM)    ! Floor cell surface heat flux         [W/m^2]
                                    ! for one time step

! Calculated variables
     REAL(r64) QFloorSum         ! Dummy summation variable                         [W]
     REAL(r64) QFloorPerimSum    ! Dummy summation variable                         [W]
     REAL(r64) QFloorCoreSum     ! Dummy summation variable                         [W]
     REAL(r64) QXZWallUpperSum   ! Dummy summation variable                         [W]
     REAL(r64) QXZWallLowerSum   ! Dummy summation variable                         [W]
     REAL(r64) QYZWallUpperSum   ! Dummy summation variable                         [W]
     REAL(r64) QYZWallLowerSum   ! Dummy summation variable                         [W]
     REAL(r64) QXZWallSum        ! Dummy summation variable                         [W]
     REAL(r64) QYZWallSum        ! Dummy summation variable                         [W]

     REAL(r64) DAXYSum           ! Floor Area                                    [m**2]
     REAL(r64) DAXZSum           ! XZ Wall Area                                  [m**2]
     REAL(r64) DAYZSum           ! YZ Wall Area                                  [m**2]
     REAL(r64) DAXZUpperSum      ! Upper XZ wall area                            [m**2]
     REAL(r64) DAYZUpperSum      ! Upper YZ wall area                            [m**2]
     REAL(r64) DAXZLowerSum      ! Lower XZ wall area                            [m**2]
     REAL(r64) DAYZLowerSum      ! Lower YZ wall area                            [m**2]

     REAL(r64) FloorHeatFlux     ! Area weighted average floor heat flux       [W/m**2]
     REAL(r64) CoreHeatFlux      ! Area weighted average core zone floor q"    [W/m**2]
     REAL(r64) PerimHeatFlux     ! Area weighted average floor perimeter q"    [W/m**2]
     REAL(r64) XZWallHeatFlux    ! Area weighted average XZ wall q"            [W/m**2]
     REAL(r64) YZWallHeatFlux    ! Area weighted average YZ wall q"            [W/m**2]
     REAL(r64) UpperXZWallFlux   ! Area weighted avg upper band XZ wall q"     [W/m**2]
     REAL(r64) UpperYZWallFlux   ! Area weighted avg upper band YZ wall q"     [W/m**2]
     REAL(r64) LowerXZWallFlux   ! Area weighted avg lower band XZ wall q"     [W/m**2]
     REAL(r64) LowerYZWallFlux   ! Area weighted avg lower band YZ wall q"     [W/m**2]
     REAL(r64) DGRAVZP           ! Thickness of the gravel bed above the slab       [m]
     REAL(r64) DGRAVZN           ! Thickness of the gravel bed below the slab       [m]
     REAL(r64) KEXT              ! Dimension of the outside of the slab             [m]
     REAL(r64) DSLAB             ! Slab thickness


!*** ASSIGNING DERIVED TYPES
     DGRAVZP=BuildingData%DGRAVZP
     DGRAVZN=BuildingData%DGRAVZN
     DSLAB  =BuildingData%DSLAB
!*** Building Constant
     KEXT=ZFACE(KBASE)+DSLAB

!*** INITIALIZATION STAGE
     QFloorSum      =0.0
     QFloorPerimSum =0.0
     QFloorCoreSum  =0.0
     QXZWallUpperSum=0.0
     QXZWallLowerSum=0.0
     QYZWallUpperSum=0.0
     QYZWallLowerSum=0.0
     QXZWallSum     =0.0
     QYZWallSum     =0.0

     FloorHeatFlux  =0.0
     CoreHeatFlux   =0.0
     PerimHeatFlux  =0.0
     XZWallHeatFlux =0.0
     YZWallHeatFlux =0.0
     UpperXZWallFlux=0.0
     UpperYZWallFlux=0.0
     LowerXZWallFlux=0.0
     LowerYZWallFlux=0.0


!***************** Floor surface fluxes
     DO COUNT1=0,IBASE-1
       DO COUNT2=0,JBASE-1
         QFloorSum=QFloorSum+QF(COUNT1,COUNT2)*DX(COUNT1)*DY(COUNT2)
!*** Assign heat fluxes to the proper zones
         IF(ABS(XC(COUNT1)).GT.(XFACE(IBASE)-2.).OR.ABS(YC(COUNT2)).GT.          &
           & (YFACE(JBASE)-2.)) THEN
           QFloorPerimSum=QFloorPerimSum+QF(COUNT1,COUNT2)*DX(COUNT1)*DY(COUNT2)
         ELSE
           QFloorCoreSum=QFloorCoreSum+QF(COUNT1,COUNT2)*DX(COUNT1)*DY(COUNT2)
         END IF
       END DO
     END DO
!*** Compute weighted averages
     FloorHeatFlux=QFloorSum/DAXYSum
     CoreHeatFlux =QFloorCoreSum/DACore
     PerimHeatFlux=QFloorPerimSum/DAPerim

!***************** XZ Wall surface fluxes
     DO COUNT1=0,IBASE-1
       DO COUNT3=0,KBASE-1
         QXZWallSum=QXZWallSum+QWW(COUNT1,COUNT3)*DX(COUNT1)*DZ(COUNT3)
         IF (NZAG.EQ.4) THEN
           IF (ZC(COUNT3).LE.ZC(1+(KBASE-NZAG)/2))THEN
             QXZWallUpperSum=QXZWallUpperSum+QWW(COUNT1,COUNT3)*DX(COUNT1)*DZ(COUNT3)
           ELSE
             QXZWallLowerSum=QXZWallLowerSum+QWW(COUNT1,COUNT3)*DX(COUNT1)*DZ(COUNT3)
           END IF
         ELSE
           IF (ZC(COUNT3).LE.(KEXT+DGRAVZP-DGRAVZN)/2.d0)THEN
             QXZWallUpperSum=QXZWallUpperSum+QWW(COUNT1,COUNT3)*DX(COUNT1)*DZ(COUNT3)
           ELSE
             QXZWallLowerSum=QXZWallLowerSum+QWW(COUNT1,COUNT3)*DX(COUNT1)*DZ(COUNT3)
           END IF
         END IF
       END DO
     END DO
     XZWallHeatFlux=QXZWallSum/DAXZSum
     UpperXZWallFlux=QXZWallUpperSum/DAXZUpperSum
     LowerXZWallFlux=QXZWallLowerSum/DAXZLowerSum

!***************** YZ Wall surface fluxes
     DO COUNT2=0,JBASE-1
       DO COUNT3=0,KBASE-1
         QYZWallSum=QYZWallSum+QWS(COUNT2,COUNT3)*DY(COUNT2)*DZ(COUNT3)
         IF(NZAG.EQ.4)THEN
           IF (ZC(COUNT3).LE.ZC(1+(KBASE-NZAG)/2))THEN
             QYZWallUpperSum=QYZWallUpperSum+QWS(COUNT2,COUNT3)*DY(COUNT2)*DZ(COUNT3)
           ELSE
             QYZWallLowerSum=QYZWallLowerSum+QWS(COUNT2,COUNT3)*DY(COUNT2)*DZ(COUNT3)
           END IF
         ELSE
           IF (ZC(COUNT3).LE.(KEXT+DGRAVZP-DGRAVZN)/2.d0)THEN
             QYZWallUpperSum=QYZWallUpperSum+QWS(COUNT2,COUNT3)*DY(COUNT2)*DZ(COUNT3)
           ELSE
             QYZWallLowerSum=QYZWallLowerSum+QWS(COUNT2,COUNT3)*DY(COUNT2)*DZ(COUNT3)
           END IF
         END IF
       END DO
     END DO
     YZWallHeatFlux=QYZWallSum/DAYZSum
     UpperYZWallFlux=QYZWallUpperSum/DAYZUpperSum
     LowerYZWallFlux=QYZWallLowerSum/DAYZLowerSum
     RETURN
END SUBROUTINE AvgHeatFlux

!**********************  CLOSE ALL IO FILES AT THE END OF EACH RUN  *********************
SUBROUTINE CloseIO
! SUBROUTINE INFORMATION:
     !       AUTHOR         Edward Clements
     !       DATE WRITTEN   May 24, 2000
     !       MODIFIED       June 9, 2000, August 2000
     !       RE-ENGINEERED  na
     !      VERSION NUMBER 1.0
     !
     ! PURPOSE OF THIS MODULE:
     ! This subroutine will close all input and output files not closed in the main
     ! simulation block.

     ! METHODOLOGY EMPLOYED:
     ! Standard EnergyPlus subroutine formatting for subroutines

     ! REFERENCES: none

     ! OTHER NOTES: none

USE BasementSimData
IMPLICIT NONE
!*** CLOSE ALL INPUT AND OUTPUT FILES
!!     CLOSE (Weather)
!     CLOSE (Weather2)
     CLOSE (GroundTemp,STATUS='DELETE')
     CLOSE (SolarFile,STATUS='DELETE')
     CLOSE (InputEcho)
     CLOSE (AvgTG,STATUS='DELETE')
     CLOSE (QHouseFile)
     CLOSE (DOUT)
     CLOSE (DYFLX)
!     CLOSE (YTDBFile)
     CLOSE (75)
     CLOSE (Debugoutfile)
     CLOSE (LOADFile)
     CLOSE (Ceil121)
     CLOSE (Flor121)
     CLOSE (RMJS121)
     CLOSE (RMJW121)
     CLOSE (SILS121)
     CLOSE (SILW121)
     CLOSE (WALS121)
     CLOSE (WALW121)
     CLOSE (CeilD21)
     CLOSE (FlorD21)
     CLOSE (RMJSD21)
     CLOSE (RMJWD21)
     CLOSE (SILSD21)
     CLOSE (SILWD21)
     CLOSE (WALSD21)
     CLOSE (WALWD21)
     CLOSE (XZYZero)
     CLOSE (XZYHalf)
     CLOSE (XZYFull)
     CLOSE (XZWallTs)
     CLOSE (YZWallTs)
     CLOSE (FloorTs)
     CLOSE (Centerline)
     CLOSE (YZWallSplit)
     CLOSE (XZWallSplit)
     CLOSE (FloorSplit)
     CLOSE (EPMonthly)
     CLOSE (EPObjects)
     RETURN
END SUBROUTINE CloseIO

!*************************  GROUND TEMPERATURE INITIALIZATION  **************************
SUBROUTINE InitializeTG(TG)
USE BasementSimData
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   June 7, 2000
     !***       MODIFIED       August 2000
     !***       RE-ENGINEERED  na

     !*** PURPOSE OF THIS SUBROUTINE:
     !*** This subroutine will create a 1D ground temperature profile to
     !*** initialize the program automatically,
     !*** NOT included in the weather file.

     !*** METHODOLOGY EMPLOYED:
     !*** Standard EnergyPlus "manager" methodology.

     !*** REFERENCES:
     !*** na
USE BasementSimData
USE InputProcessor
IMPLICIT NONE

!*** Variable Declarations
     REAL(r64) TDB(24)               ! Outside air dry bulb temperature                  [C]
     REAL(r64) HTDB(8760)            ! Array of annual dry bulb temperatures             [C]
     REAL(r64) Tm                    ! Average of the monthly average temperatures       [C]
     REAL(r64) TG(0:100)             ! 1D ground temperature initialization array        [C]
     REAL(r64) As                    ! Amplitude of the ground surface temp wave         [C]
     REAL(r64) TempSum               ! Dummy summation variable for averaging            [C]
     REAL(r64) TAVG(12)              ! Monthly average air temperatures                  [C]
     REAL(r64) TAvgMax               ! Maximum monthly average air temperature           [C]
     REAL(r64) TAvgMin               ! Minimum monthly average air temperature           [C]
     REAL(r64) TmSum                 ! Dummy summation variable for averaging            [C]
     REAL(r64) ZFACEUsed(0:100)      ! Dummy array of z direction cell face coordinates  [m]

!     INTEGER IDAY               ! Day counter                                        []
     INTEGER IHR                ! Hour counter (1-24)                                []
     INTEGER HourNum            ! Hour number counter (1-8760)                       []
     INTEGER IMON
     INTEGER ACH                ! Annual cooling hours                               []
     INTEGER AHH                ! Annual heating hours                               []
     INTEGER ACHSum
     INTEGER AHHSum

!*** UNUSED WEATHER VARIABLES
     REAL(r64) TWB(24),PBAR(24),HRAT(24),WND(24),RBEAM(24),RDIFH(24),DSNOW(24)
     INTEGER ISNW(24),TempInit
     INTEGER IHrStart
     INTEGER IHrEnd

!*** Set local variables
     ZFACEUsed=ZFACEINIT(0:100)


!*** Open a scratch file
!     TempInit=GetNewUnitNumber()
!     OPEN(UNIT=TempInit,FILE='TempInit.TXT')

!*** Retrieve weather information for the year
!!     Weather=GetNewUnitNumber()
!!     OPEN(UNIT=Weather,FILE=WeatherFile//'.TXT',STATUS='OLD')
!!     REWIND (Weather)
!     CALL SkipHeader
     IHrStart=1
     IHrEnd=24
     DO IDAY=1,365
!       READ (Weather,*) TodaysWeather
       CALL GetWeatherData(IDAY)
       TDB=TodaysWeather%TDB
       HTDB(IHrStart:IHrEnd)=TDB
       IHrStart=IHrStart+24
       IHrEnd=IHrEnd+24

!!*** Retrieve weather information for the year
!     OPEN(UNIT=Weather,FILE=WeatherFile//'.DAT',STATUS='OLD')
!     REWIND (Weather)
!!     CALL SkipHeader
!     DO IDAY=1,365
!       READ (Weather,*) TDB
!6700   FORMAT (/,3(8F10.6,/),19/)
!       WRITE(TempInit,*) (TDB(IHR),IHR=1,24)
     END DO
!     CLOSE(Weather)
!     REWIND(TempInit)
!     READ (TempInit,*) HTDB

!*** Compute monthly average air temperatures
     HourNum=0.0
     ACHSum=0
     AHHSum=0
     DO IHR=1,8759
       IF (HTDB(IHR).GT.TDeadBandUp) THEN
         ACHSum=ACHSum+1
       ELSE IF (HTDB(IHR).LT.TDeadBandLow) THEN
        AHHSum=AHHSum+1
       END IF
     END DO
     SiteInfo%ACH=ACHSum
     SiteInfo%AHH=AHHSum
     ACH=SiteInfo%ACH
     AHH=SiteInfo%AHH

     DO IMON=1,12
       TempSum=0.0
       DO IDAY=1,NDIM(IMON)
         DO IHR=1,24
           HourNum=HourNum+1
           TempSum=TempSum+HTDB(HourNum)
         END DO
         TAVG(IMON)=TempSum/(IDAY*24)
       END DO
     END DO
     TmSum=0.0
     TAvgMax=-99999.
     TAvgMin=99999.
     DO IMON=1,12
       TmSum=TmSum+TAVG(IMON)
       TAvgMax=MAX(TAvgMax,TAVG(IMON))
       TAvgMin=MIN(TAvgMin,TAVG(IMON))
     END DO

!*** Compute the average of the average monthly air temperatures
     Tm=TmSum/12

!*** Calculate the annual ground surface temperature wave amplitude
     As=(TAvgMax-TAvgMin)/2.

!*** Set up a dummy cell face coordinate matrix (to prevent errors in the main program)
     DO COUNT1=0,NZBG
!*** Calculate the Temperature profile
       TG(COUNT1)=Tm-As*EXP(-0.4464*ZFACEUsed(COUNT1))*         &
         & COSD(.5236*(-1.-.8525*ZFACEUsed(COUNT1)))
       IF (COUNT1.EQ.20) TG(COUNT1)=Tm
     END DO
!     CLOSE(TempInit)
     RETURN
END SUBROUTINE InitializeTG



!************************ ENERGYPLUS WEATHER FILE PARSING ROUTINE *******************
SUBROUTINE WeatherServer !(WeatherFile,EPWFile)
! MODULE INFORMATION:
      !       AUTHOR         Edward Clements
      !       DATE WRITTEN   April 9, 2001
      !       MODIFIED       April 28,2001
      !       RE-ENGINEERED  na
      !       VERSION NUMBER 1.0
      !
      ! PURPOSE OF THIS MODULE:
      ! This module parses the Energy Plus weather file to recreate a full year
      ! TMY weather file for the foundation heat transfer modules

      ! METHODOLOGY EMPLOYED:
      ! Standard EnergyPlus module formatting.
      ! Derived Types used wherever applicable.

      ! REFERENCES: na

      ! OTHER NOTES: none
USE DataGlobals, ONLY: ShowFatalError
USE BasementSimData
USE InputProcessor
USE EPWRead

IMPLICIT NONE   ! Enforce explicit typing of all variables

!     Variable definitions
!      INTEGER WYEAR,WDAY,WHOUR,WMINUTE,WMONTH,EPWeather
!      INTEGER, DIMENSION(9) :: PRESWEATHCONDS
!      CHARACTER (LEN=20) PRESWEATHCODES
!      CHARACTER (LEN=50) SOURCEJUNK
!      CHARACTER (LEN=20) EplusWeather
!      CHARACTER *6 WeatherFile
!      CHARACTER *7 EPWFile
!       REAL(r64) Latitude
!       REAL(r64) Longitude
!       REAL(r64) TimeZone
!       REAL(r64) Altitude
!       Character(len=50) Skip
!       character(len=500) wthline
!!     EPlus weather file variables
!      REAL(r64) DRYBULB,DEWPOINT,RELHUM,ATMPRESS,ETHORIZ,ETDIRECT,IRHORIZ,GLBHORIZ,DIRECTRAD,  &
!      & DIFFUSERAD,GLBHORIZILLUM,DIRECTNORMILLUM,ZENLUM,WINDDIR,WIND,TOTALSKYCOVER,       &
!      & OPAQUESKYCOVER, VISIBILITY,CEILHEIGHT,PRESWEATHOBS,PRECIPWATER,AEROSOLOPTDEPTH,   &
!      & SNOWDEPTH,DAYSSINCELASTSNOW,DIFFUSEHORIZILLUM,T2,PWS,WSSTAR,SATUPT,PDEW

!!     TMY Format weather file variables
!      INTEGER ISNW(24),IHR,IDAY
!      REAL(r64) TDB(24),TWB(24),PBAR(24),HRAT(24),WND(24),RBEAM(24),RDIFH(24),DSNOW(24)
      INTEGER ISNW(24),IHR !,IDAY
      REAL(r64) DSNOW(24)

      LOGICAL ErrorsFound
      LOGICAL FileExist
      CHARACTER(len=120) :: ErrorMessage
      INTEGER NDays

      INQUIRE(FILE=TRIM(EPWFile)//'.epw',EXIST=FileExist)
      IF (.not. FileExist) CALL ShowFatalError('No in.epw file found',EPMonthly,InputEcho)

      CALL ReadEPW(TRIM(EPWFile)//'.epw',ErrorsFound,ErrorMessage,NDays)

      IF (ErrorsFound) THEN
        PRINT *,trim(ErrorMessage)
        CALL ShowFatalError('Errors found getting weather file. Program Terminates.')
      ELSE
        PRINT*, 'Completed Reading Weather File'
      ENDIF

!      EPWeather=GetNewUnitNumber()
!      OPEN(unit=EPWeather,file=TRIM(EPWFile)//'.EPW', STATUS='OLD')
!      Weather=GetNewUnitNumber()
!      OPEN(unit=Weather,file=WeatherFile//'.txt',STATUS='REPLACE')
!  Use the EP weather file for the SiteInfo
!      READ(EPWeather,'(A)') wthline
      ! skip first 6 fields on Location line
!      CALL GetField(wthline,7,Skip,',')
!      READ(Skip,*) Latitude
!      CALL GetField(wthline,8,Skip,',')
!      READ(Skip,*) Longitude
!      CALL GetField(wthline,9,Skip,',')
!      READ(Skip,*) TimeZone
!      CALL GetField(wthline,10,Skip,',')
!      READ(Skip,*) Altitude

      SiteInfo%LONG = Longitude
      SiteInfo%LAT =Latitude
      SiteInfo%MSTD = TimeZone
      SiteInfo%ELEV = Elevation

!      READ (EPWeather,41001)
!41001 FORMAT (6/)
 ! Loop through a year, then 24 hours/day
      DO IDAY=1,NDays
        DO IHR=1,24
!          print *, IDAY,IHR
!          READ (EPWeather,*) WYEAR,WMONTH,WDAY,WHOUR,WMINUTE,SOURCEJUNK,DRYBULB,DEWPOINT, &
!          &    RELHUM,ATMPRESS,ETHORIZ,ETDIRECT,IRHORIZ,GLBHORIZ,DIRECTRAD,DIFFUSERAD,    &
!          &    GLBHORIZILLUM,DIRECTNORMILLUM,DIFFUSEHORIZILLUM,ZENLUM,WINDDIR,WIND,       &
!          &    TOTALSKYCOVER,OPAQUESKYCOVER,VISIBILITY,CEILHEIGHT,PRESWEATHOBS,           &
!          &    PRESWEATHCODES,PRECIPWATER,AEROSOLOPTDEPTH,SNOWDEPTH,DAYSSINCELASTSNOW

!          TDB(IHR)  =DRYBULB
!          PBAR(IHR) =WDAY(IDAY)%StnPres(IHR,1)
!          CALL DrySatPt(SATUPT,TDB(IHR))
!          PDEW=RELHUM/100.*SATUPT
!          HRAT(IHR)=PDEW*0.62198/(PBAR(IHR)-PDEW)
!          T2=TDB(IHR)+273.15
!          IF (TDB(IHR).LT.0.)THEN
!            PWS=EXP(-5.6745359E+03/T2+6.3925247-9.677843E-03*T2+6.2215701E-07*T2**2+      &
!            &   2.0747825E-09*T2**3-9.4840240E-13*T2**4+4.1635019E00*LOG(T2))
!          ELSE
!            PWS=EXP(-5.8002206E+03/T2+1.3914993E+00-4.8640239E-02*T2+4.176476E-05*T2**2-  &
!            &   1.4452093E-08*T2**3+6.5459673E+00*LOG(T2))
!          END IF
!          WSSTAR=0.62198*(PWS/(PBAR(IHR)-PWS))
!          TWB(IHR)=((2501.+1.805*TDB(IHR))*HRAT(IHR)-2501*WSSTAR+TDB(IHR))/               &
!          &        (4.186*HRAT(IHR)-2.381*WSSTAR+1.)
!          TWB(IHR)=WDAY(IDAY)%WetBulb(IHR,1)
!          WND(IHR)  =WIND
!          RBEAM(IHR)=DIRECTRAD
!          RDIFH(IHR)=DIFFUSERAD
          IF(WDAY(IDAY)%SnowDepth(IHR,1) > 0 .and. WDAY(IDAY)%SnowDepth(IHR,1) < 999.) THEN  ! 999. is missing
            ISNW(IHR)=1
            DSNOW(IHR)=WDAY(IDAY)%SnowDepth(IHR,1)
          ELSE
            ISNW(IHR)=0
            DSNOW(IHR)=0.0
          END IF
        END DO
        IF (WDAY(IDAY)%Month == 2 .and. WDAY(IDAY)%Day == 29) CYCLE
        FullYearWeather(IDAY)%TDB=WDAY(IDAY)%DryBulb(:,1)
        FullYearWeather(IDAY)%TWB=WDAY(IDAY)%WetBulb(:,1)
        FullYearWeather(IDAY)%PBAR=WDAY(IDAY)%StnPres(:,1)
        FullYearWeather(IDAY)%HRAT=WDAY(IDAY)%HumRat(:,1)
        FullYearWeather(IDAY)%WND=WDAY(IDAY)%WindSpd(:,1)
        FullYearWeather(IDAY)%RBEAM=WDAY(IDAY)%DirNormRad(:,1)
        FullYearWeather(IDAY)%RDIFH=WDAY(IDAY)%DifHorzRad(:,1)
        FullYearWeather(IDAY)%ISNW=ISNW
        FullYearWeather(IDAY)%DSNOW=DSNOW
      END DO
!      PRINT*, 'DONE PARSING THE EnergyPlus WEATHER FILE'
      Write(DebugOutFile,*) 'Done Parsing The Energyplus Weather File'
!      REWIND (Weather)


END SUBROUTINE WeatherServer

!**********************  DRY AIR SATURATION PRESSURE CALCULATION ************************
SUBROUTINE DrySatPt (SATUPT,TDB)
      IMPLICIT NONE
! MODULE INFORMATION:
      !       AUTHOR         George Shih
      !       DATE WRITTEN   May, 1976
      !       MODIFIED       April 21, 2001
      !       RE-ENGINEERED  Edward D. Clements
      !       VERSION NUMBER 1.0
      !
      ! PURPOSE OF THIS MODULE:
      ! This subroutine calculates the dry air saturation pressure given the dry bulb
      ! temperature and barometric reading

      ! METHODOLOGY EMPLOYED:
      ! Standard EnergyPlus module formatting.
      ! Derived Types used wherever applicable.

      ! REFERENCES: na

      ! OTHER NOTES: none

      REAL(r64)     PSAT,TDB,SATUPT
      Double Precision  TT

!     TDB    - argument  - SATURATION TEMPERATURE (C)
!     SATUPT - funcval - CALCULATE SATURATION PRESSURE
!     TT     -
!     PSAT   - SATURATION PRESSURE (N/M**2)
      TT = TDB
      IF (TDB > 20) GO TO 20
      IF (TDB > 10.) GO TO 70
      IF (TDB > 0.) GO TO 60
      IF (TDB > -20.) GO TO 50
      IF (TDB > -40.) GO TO 40
      GO TO 30
   20 CONTINUE
      IF (TDB < 30.) GO TO 80
      IF (TDB < 40.) GO TO 90
      IF (TDB < 80.) GO TO 100
      GO TO 110
!                                      TEMP. IS FROM -60 C  TO  -40 C
   30 CONTINUE
      PSAT=Y5(TT,4.9752D2,35.3452D0,1.04398D0,1.5962D-2,1.2578D-4,4.0683D-7)
      GO TO 120
!                                      TEMP. IS FROM -40 C  TO  -20 C
   40 CONTINUE
      PSAT=Y4(TT,5.69275D2,42.5035D0,1.29301D0,1.88391D-2,1.0961D-4)
      GO TO 120
!                                      TEMP. IS FROM  -20 C TO  0 C
   50 CONTINUE
      PSAT=Y4(TT,6.10860D2,50.1255D0,1.83622D0,3.67769D-2,3.41421D-4)
      GO TO 120
!                                      TEMP. IS FROM 0. C TO  10 C
   60 CONTINUE
      PSAT=Y3(TT,6.10775D2,44.4502D0,1.38578D0,3.3106D-2)
      GO TO 120
!                                      TEMP. IS FROM 10 C  TO  20 C
   70 CONTINUE
      PSAT=Y3(TT,5.9088D2,49.8847D0,8.74643D-1,4.97621D-2)
      GO TO 120
!                                      TEMP. IS FROM 20 C  TO  30 C
   80 CONTINUE
      PSAT=Y3(TT,4.05663D2,76.8637D0,-4.47857D-1,7.15905D-2)
      GO TO 120
!                                      TEMP. IS FROM 30 C  TO  40 C
   90 CONTINUE
      PSAT=Y3(TT,-3.58332D2,1.52167D2,-2.93294D0,9.90514D-2)
      GO TO 120
!                                      TEMP. IS FROM 40 C TO 80 C
  100 CONTINUE
      PSAT=Y5(TT,7.30208D2,32.987D0,1.84658D0,1.95497D-2,3.33617D-4,2.59343D-6)
      GO TO 120
!                                      TEMP. IS FROM 80 C TO 100 C
  110 CONTINUE
      PSAT=Y5(TT,6.91607D2,10.703D0,3.01092D0,-2.57247D-3,5.19714D-4,2.00552D-6)
!
  120 CONTINUE
      SATUPT=PSAT
      RETURN

      CONTAINS
      DOUBLE PRECISION FUNCTION Y3(X,A0,A1,A2,A3)
      IMPLICIT NONE
      DOUBLE PRECISION X
      DOUBLE PRECISION A0,A1,A2,A3
      Y3=A0+X*(A1+X*(A2+X*A3))
      RETURN
      END FUNCTION Y3

      DOUBLE PRECISION FUNCTION Y4(X,A0,A1,A2,A3,A4)
      IMPLICIT NONE
      DOUBLE PRECISION X
      DOUBLE PRECISION A0,A1,A2,A3,A4
      Y4=A0+X*(A1+X*(A2+X*(A3+X*A4)))
      RETURN
      END FUNCTION Y4

      DOUBLE PRECISION FUNCTION Y5(X,A0,A1,A2,A3,A4,A5)
      IMPLICIT NONE
      DOUBLE PRECISION X
      DOUBLE PRECISION A0,A1,A2,A3,A4,A5
      Y5=A0+X*(A1+X*(A2+X*(A3+X*(A4+X*A5))))
      RETURN
      END FUNCTION Y5

END SUBROUTINE DrySatPt

SUBROUTINE GetField(InputString,Fldno,ReturnString,Delimiter)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   September 2005
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! Gets field no (CSV line or Delimiter) "Fldno" from input string.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  CHARACTER(len=*), INTENT(IN) :: InputString
  INTEGER, INTENT(IN)          :: Fldno
  CHARACTER(len=*), INTENT(OUT):: ReturnString
  CHARACTER(len=1), INTENT(IN), OPTIONAL :: Delimiter

          ! SUBROUTINE PARAMETER DEFINITIONS:
  CHARACTER(len=1), PARAMETER :: Comma=','

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  INTEGER :: Pos
  INTEGER :: Fld
  INTEGER :: LastPos
  CHARACTER(len=1) :: StringDelimiter

  ReturnString=' '
  Fld=1
  LastPos=1
  IF (PRESENT(Delimiter)) THEN
    StringDelimiter=Delimiter
  ELSE
    StringDelimiter=Comma
  ENDIF
  DO WHILE (Fld <= Fldno)
    Pos=INDEX(InputString(LastPos:),StringDelimiter)
    IF (Fld < Fldno) LastPos=LastPos+Pos
    Fld=Fld+1
  ENDDO
  IF (Pos > 0) THEN
    ReturnString=InputString(LastPos:LastPos+Pos-2)
  ELSE
    ReturnString=InputString(LastPos:)
  ENDIF

  RETURN

END SUBROUTINE GetField

END MODULE BASE3D

!****************************************************************************************
!****************************************************************************************
!************************              MAIN PROGRAM                **********************
!****************************************************************************************
!****************************************************************************************
PROGRAM BasementModel
    USE BasementSimData
    USE BASE3D, ONLY:Base3Ddriver
    USE DataStringGlobals
    REAL(r64) Time_Start
    REAL(r64) Time_Finish
    CALL CPU_TIME(Time_Start)
    CALL Base3Ddriver
    CALL CPU_TIME(Time_Finish)
    Elapsed_Time=Time_Finish-Time_Start
    CALL EndEnergyPlus
END PROGRAM BasementModel

!****************************************************************************************
!****************************************************************************************
!************************              MAIN PROGRAM                **********************
!****************************************************************************************
!****************************************************************************************
