MODULE EPWPrecisionGlobals

          ! Module containing the routines dealing with the precision of data in EnergyPlus

          ! MODULE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   January 2008
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS MODULE:
          ! This module allows for setting the default precision to "double precision" using
          ! F95 KIND and parameters.  Should it ever be necessary to try a higher precision, it
          ! will be easy to switch for testing.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! OTHER NOTES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables

PUBLIC ! This data only module is public.

          ! MODULE PARAMETER DEFINITIONS:
    INTEGER, PARAMETER :: i32=Selected_Int_Kind(6)  ! 6 digits
    INTEGER, PARAMETER :: i64=Selected_Int_Kind(12) ! 12 digits
    INTEGER, PARAMETER :: r32=KIND(1.0)
    INTEGER, PARAMETER :: r64=KIND(1.0D0)
    INTEGER, PARAMETER :: default_prec=r64
    REAL(r64), PARAMETER :: constant_zero=0.0d0
    REAL(r64), PARAMETER :: constant_one=1.0d0
    REAL(r64), PARAMETER :: constant_minusone=-1.0d0
    REAL(r64), PARAMETER :: constant_twenty=20.0d0
    REAL(r64), PARAMETER :: constant_pointfive=.5d0
    REAL(r64), PARAMETER :: EXP_LowerLimit=-20.d0  ! In IVF=2.061153622438558E-009 - used 20
                                                   ! because it's already used in other parts of the code
    REAL(r64), PARAMETER :: EXP_UpperLimit= 40.d0  ! In IVF=2.353852668370200E+017

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! MODULE VARIABLE DECLARATIONS:
          ! na

          ! SUBROUTINE SPECIFICATIONS FOR MODULE:
          ! na

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
END MODULE EPWPrecisionGlobals
