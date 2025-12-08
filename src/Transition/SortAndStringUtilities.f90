MODULE SortAndStringUtilities

          ! Module containing the routines dealing with Sorting

          ! MODULE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   March 2009
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS MODULE:
          ! <description>

          ! METHODOLOGY EMPLOYED:
          ! <description>

          ! REFERENCES:
          ! na

          ! OTHER NOTES:
          ! na

          ! USE STATEMENTS:
          ! <use statements for data only modules>
          ! <use statements for access to subroutines in other modules>

IMPLICIT NONE ! Enforce explicit typing of all variables

PRIVATE ! Everything private unless explicitly made public

          ! MODULE PARAMETER DEFINITIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! MODULE VARIABLE DECLARATIONS:
          ! na

          ! SUBROUTINE SPECIFICATIONS FOR MODULE SortUtilities
PUBLIC  SetupAndSort
PRIVATE QSortC
PRIVATE QSortPartition

CONTAINS

SUBROUTINE SetUpAndSort(Alphas,iAlphas)

         ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   March 2009
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! Set up and call sort routine for Alphas

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  CHARACTER(len=*), INTENT(INOUT),DIMENSION(:) :: Alphas  ! Alphas to be sorted
  INTEGER, INTENT(INOUT),DIMENSION(:)       :: iAlphas ! Pointers -- this is the array that is actually sorted

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  INTEGER :: Loop

  DO Loop=1,SIZE(Alphas)
    iAlphas(Loop)=Loop
  ENDDO

  CALL QSortC(Alphas,iAlphas)

  RETURN

END SUBROUTINE SetupAndSort

RECURSIVE SUBROUTINE QsortC(Alphas,iAlphas)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   March 2009
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! Make sort order for an Alpha Array but store the pointers in an
          ! accompanying integer array which must be filled prior to the first call
          ! as this routine is recursive and called from within.

          ! METHODOLOGY EMPLOYED:
          ! recursion and quick-sort methodology

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  CHARACTER(len=*), INTENT(INOUT),DIMENSION(:) :: Alphas  ! Alphas to be sorted
  INTEGER, INTENT(INOUT),DIMENSION(:)       :: iAlphas ! Pointers -- this is the array that is actually sorted

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  integer :: iq

  if(size(Alphas) > 1) then
     call QsortPartition(Alphas,iAlphas,iq)
     call QsortC(Alphas(:iq-1),iAlphas(:iq-1))
     call QsortC(Alphas(iq:),iAlphas(iq:))
  endif

  RETURN

END SUBROUTINE QsortC

SUBROUTINE QsortPartition(Alphas,iAlphas,marker)

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
  USE DataGlobals, ONLY: MaxNameLength

  IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  CHARACTER(len=*), INTENT(INOUT),DIMENSION(:) :: Alphas  ! Alphas to be sorted
  INTEGER, INTENT(INOUT),DIMENSION(:)       :: iAlphas ! Pointers -- this is the array that is actually sorted
  INTEGER, INTENT(INOUT)       :: marker

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  integer :: i, j
  integer :: itemp
  CHARACTER(len=MaxNameLength*2) :: ctemp
  CHARACTER(len=MaxNameLength*2) :: cpivot      ! pivot point

  cpivot = Alphas(1)
  i= 0
  j= size(Alphas) + 1

  do
     j = j-1
     do
        if (Alphas(j) <= cpivot) exit
        j = j-1
     end do
     i = i+1
     do
        if (Alphas(i) >= cpivot) exit
        i = i+1
     end do
     if (i < j) then
        ! exchange iAlphas(i) and iAlphas(j)
        ctemp=Alphas(i)
        Alphas(i)=Alphas(j)
        Alphas(j)=ctemp
        itemp = iAlphas(i)
        iAlphas(i) = iAlphas(j)
        iAlphas(j) = itemp
     elseif (i == j) then
        marker = i+1
        return
     else
        marker = i
        return
     endif
  end do

END SUBROUTINE QsortPartition

!     NOTICE
!
!     Copyright © 1996-2009 The Board of Trustees of the University of Illinois
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

END MODULE SortAndStringUtilities
