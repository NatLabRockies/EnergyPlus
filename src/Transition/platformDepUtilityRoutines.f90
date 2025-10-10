SUBROUTINE CopyFile(InFileName,OutFileName,errflag)

          !       SUBROUTINE INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   na
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This routine copies a file from one to another.


          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  CHARACTER(len=*), INTENT(IN) :: InFileName   ! Input file name
  CHARACTER(len=*), INTENT(IN) :: OutFileName  ! Output file name
  LOGICAL, INTENT(INOUT) :: errflag

          ! SUBROUTINE PARAMETER DEFINITIONS:
  character(len=*), parameter :: fmta="(A)"

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  CHARACTER(len=1000) lineBuffer
  INTEGER :: inunit
  INTEGER :: outunit

  INTEGER, EXTERNAL :: GetNewUnitNumber

  INTEGER :: ios
  LOGICAL :: fileExist

  inunit=GetNewUnitNumber()
  INQUIRE(File=trim(InFileName),EXIST=fileExist)

  if (.not. fileExist) then
    errflag=.true.
    return
  endif

  open(inunit,file=trim(InFileName))

  outunit=GetNewUnitNumber()
  open(outunit,file=trim(OutFileName))

  rewind(inunit)

  ios=0
  do while (ios == 0)
    read(inunit,fmta,IOSTAT=ios) lineBuffer
    if (ios /= 0) exit
    write(outunit,fmta) trim(lineBuffer)
  enddo

  rewind(outunit)
  rewind(inunit)
  close(outunit)
  close(inunit)

  return

END SUBROUTINE copyfile
