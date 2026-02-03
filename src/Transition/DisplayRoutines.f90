  subroutine DisplayString(String)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   Version 1.0
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine provides a call to display strings during program execution.

          ! METHODOLOGY EMPLOYED:
          ! usage:=  call DisplayString(string)

          ! REFERENCES:
          ! na


          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  character(len=*), intent(in) :: String  ! String to be displayed

          ! SUBROUTINE PARAMETER DEFINITIONS:
  character(len=*), parameter :: FmtA="(1X,A)"

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
          ! na

  write(*,FmtA) trim(String)

  return
  end subroutine DisplayString

  subroutine DisplayNumberandString(Number,String)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   Version 1.0
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine provides a call to display (at set point on screen for screen positioning models) card images
          ! during program parsing.

          ! METHODOLOGY EMPLOYED:
          ! usage:= call DisplayNumberandString(numbr,string)

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  integer, intent(in) :: Number  ! number to be displayed
  character(len=*), intent(in) :: String  ! String to be displayed

          ! SUBROUTINE PARAMETER DEFINITIONS:
  character(len=*), parameter :: FmtA="(1X,A)"

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
   character(len=25) :: NumString

   write(NumString,*) Number
   NumString=ADJUSTL(NumString)

   write(*,FmtA) trim(String)//trim(NumString)
!
  return
  end subroutine DisplayNumberandString

  SUBROUTINE  DisplaySimDaysProgress(CurrentSimDay,TotalSimDays)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   Version 1.0
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine provides a call for "progress" during simulation.
          ! Progress is percent of current days vs total days.

          ! METHODOLOGY EMPLOYED:
          ! Needs description, as appropriate.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  integer, intent(in) :: CurrentSimDay  ! Current Simulation Day
  integer, intent(in) :: TotalSimdays   ! Total number of Simulation Days

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  integer, save :: percent=0  ! Current percent progress

  if (TotalSimdays > 0) then
    percent=NINT((REAL(CurrentSimDay)/REAL(TotalSimdays))*100.)
    percent=MIN(percent,100)
  else
    percent=0
  endif

  return
  end subroutine DisplaySimDaysProgress

!     NOTICE
!
!     Copyright © 1996-2007 The Board of Trustees of the University of Illinois
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
