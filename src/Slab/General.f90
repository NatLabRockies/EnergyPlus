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


END MODULE General
