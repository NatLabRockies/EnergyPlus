PROGRAM CalcSoilSurfTemp

          ! PROGRAM INFORMATION:
          !       AUTHOR         Kwang Ho Lee
          !       DATE WRITTEN   January 2006
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This program is for calculation of annual average soil surface temperature, amplitude of soil surface
          ! temperature, and phase constant of soil surface temperature.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
USE EPWRead


IMPLICIT NONE    ! Enforce explicit typing of all variables in this program

          ! PROGRAM ARGUMENT DEFINITIONS:
          ! Filename

          ! PROGRAM PARAMETER DEFINITIONS:
REAL, PARAMETER :: a=103.     ! Constant
REAL, PARAMETER :: b=609.     ! Constant
REAL, PARAMETER :: GroundEmit=0.95     ! Hemishperical Emittance of the Ground Surface
REAL, PARAMETER :: RadConstant=63.     ! Radiation Constant
CHARACTER(len=*), PARAMETER :: blank=' '

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! PROGRAM LOCAL VARIABLE DECLARATIONS:
CHARACTER(len=255) :: InputFileNameWithPath   ! InputFile Name (Full path)
CHARACTER(len=120) :: ErrorMessage=' '
LOGICAL  :: ErrorsFound=.false.
REAL :: SoilThermDiff                     =0.0     ! Soil Thermal Diffusivity
REAL :: SoilThermCond                     =0.0     ! Soil Thermal Conductivity
REAL :: AbsorpCoef                        =0.0     ! Absorption Coefficient
REAL :: EvapFrac                          =0.0     ! Fraction of Evaporation Rate
INTEGER :: JulDay     ! Julian Date
INTEGER :: hh     ! Hour of Each Day
INTEGER :: INTERVAL     ! Interval of Each Hour
INTEGER :: i
INTEGER :: j
INTEGER :: SoilCondition    ! Indicator of the actual soil condition surrounding the Earth Tube
INTEGER :: GroundSurface    ! Indicator of the condition of the ground surface above the Earth Tube
INTEGER :: PhaseConAirTemp   ! time from the beginning of the year at which air temp. reaches minimum value
INTEGER :: PhaseConSolarRad  ! time from the beginning of the year when Solar Radiation reaches minimum value
REAL, DIMENSION(:), ALLOCATABLE :: DailyWindVel     ! Daily Wind Velocity
REAL, DIMENSION(:), ALLOCATABLE :: DailyRelatHum     ! Daily Relative Humidity
REAL, DIMENSION(:), ALLOCATABLE :: DailyAirTemp     ! Daily Air Temperature
REAL, DIMENSION(:), ALLOCATABLE :: DailySolarRad     ! Daily Solar Radiation
REAL :: w
REAL :: r
REAL :: MeanWindVel     ! Annual Mean Wind Velocity
REAL :: MeanRelatHum     ! Annual Mean Relative Humidity
REAL :: SoilHeatTranCoef     ! Convective Heat Transfer Coefficient at the Soil Surface
REAL :: hr
REAL :: he
REAL :: AngularFreq     ! Annual Angular Frequency
REAL :: DampDepth     ! Dampening Depth
REAL :: s
REAL :: g
REAL :: MeanAirTemp     ! Annual Mean Air Temperature
REAL :: MaxAirTemp     ! Maximum Air Temperature
REAL :: MinAirTemp     ! Minimum Air Temperature
REAL :: AmplAirTemp     ! Amplitude of the Air Temperature
REAL :: MeanSolarRad     ! Annual Mean Solar Radiation
REAL :: MaxSolarRad     ! Maximum Solar Radiation
REAL :: MinSolarRad     ! Minimum Solar Radiation
REAL :: AmplSolarRad     ! Amplitude of Solar Radiation
REAL :: PhaseAngle     ! Phase Angle between Insolation and Air Temperature
REAL :: Process1     ! Variable Used in the Middle of the Calculation
REAL :: Process2     ! Variable Used in the Middle of the Calculation
REAL :: Process3     ! Variable Used in the Middle of the Calculation
REAL :: Process4     ! Variable Used in the Middle of the Calculation
REAL :: Process5     ! Variable Used in the Middle of the Calculation
REAL :: RealPart     ! Real Part of the Complex Number
REAL :: ImagPart     ! Imaginary Part of the Complex Number
REAL :: PhaseAngleAirSoil     ! Phase Angle Difference between Air and Soil surface temperature
REAL :: AverSoilSurTemp     ! Average Soil Surface Temperature
REAL :: ApmlSoilSurTemp     ! Amplitude of Soil Surface Temperature
INTEGER :: SoilSurPhaseConst     ! Phase constant of Soil Surface
INTEGER :: numcmdargs
INTEGER :: arg

  numcmdargs=Command_Argument_Count()
  if (numcmdargs == 0) then
    InputFileNameWithPath='in.epw'
  else  ! there are command args, at least one...
    arg=1
    Call Get_Command_Argument(arg,InputFileNameWithPath)
    InputFileNameWithPath=ADJUSTL(InputFileNameWithPath)
    if (InputFileNameWithPath == blank) then
      InputFileNameWithPath='in.epw'
    endif
  endif

10 PRINT *, 'Select the soil condition surrounding the Earth Tube'
   PRINT *, '1. HEAVY AND SATURATED'
   PRINT *, '2. HEAVY AND DAMP'
   PRINT *, '3. HEAVY AND DRY'
   PRINT *, '4. LIGHT AND DRY'
   READ *, SoilCondition

   PRINT *, 'Select the condition of the ground surface above the Earth Tube'
   PRINT *, '1. BARE AND WET'
   PRINT *, '2. BARE AND MOIST'
   PRINT *, '3. BARE AND ARID'
   PRINT *, '4. BARE AND DRY'
   PRINT *, '5. COVERED AND WET'
   PRINT *, '6. COVERED AND MOIST'
   PRINT *, '7. COVERED AND ARID'
   PRINT *, '8. COVERED AND DRY'
   READ *, GroundSurface

    SELECT CASE (SoilCondition)
      CASE (1)
        SoilThermDiff = 0.0032544
        SoilThermCond = 2.42
      CASE (2)
        SoilThermDiff = 0.002322
        SoilThermCond = 1.3
      CASE (3)
        SoilThermDiff = 0.0018576
        SoilThermCond = 0.865
      CASE (4)
        SoilThermDiff = 0.001008
        SoilThermCond = 0.346
      CASE DEFAULT
        PRINT *, 'Invalid value for the soil condition surrounding the Earth Tube'
        GOTO 10
    END SELECT

    SELECT CASE (GroundSurface)
      CASE (1)
        AbsorpCoef = 0.9
        EvapFrac   = 0.7
      CASE (2)
        AbsorpCoef = 0.8
        EvapFrac   = 0.45
      CASE (3)
        AbsorpCoef = 0.8
        EvapFrac   = 0.15
      CASE (4)
        AbsorpCoef = 0.7
        EvapFrac   = 0.
      CASE (5)
        AbsorpCoef = 0.9
        EvapFrac   = 0.49
      CASE (6)
        AbsorpCoef = 0.8
        EvapFrac   = 0.315
      CASE (7)
        AbsorpCoef = 0.8
        EvapFrac   = 0.105
      CASE (8)
        AbsorpCoef = 0.7
        EvapFrac   = 0.
      CASE DEFAULT
        PRINT *, 'Invalid value for the condition of the ground surface above the Earth Tube'
        GOTO 10
    END SELECT

CALL ReadEPW(InputFileNameWithPath,ErrorsFound,ErrorMessage)
IF (ErrorsFound) THEN
  OPEN(17,FILE='CalcSoilSurfTemp.out',STATUS='REPLACE')
  WRITE (17,'(A)') ' Weather file used='//TRIM(InputFileNameWithPath)
  WRITE(17,'(A)') 'Errors occured in reading weather file='//TRIM(ErrorMessage)
  CLOSE(17)
  STOP
ENDIF
IF (NumDataPeriods >1) THEN
  OPEN(17,FILE='CalcSoilSurfTemp.out',STATUS='REPLACE')
  WRITE (17,'(A)') ' Weather file used='//TRIM(InputFileNameWithPath)
  WRITE(17,'(A)') 'Number of Data Periods on Weather File not = 1.'
  CLOSE(17)
  STOP
ENDIF

! Daily Wind Velocity and Annual Mean Wind Velocity Calculation

ALLOCATE (DailyWindVel(NumDays))
DO JulDay=1, NumDays
 w=0
 DO hh=1, 24
  DO INTERVAL=1, NumIntervalsPerHour
   w=w+WDay(JulDay)%WindSpd(hh, INTERVAL)
  END DO
 END DO
 DailyWindVel(JulDay)=w/24./Real(NumIntervalsPerHour)
END DO

r=0
 DO JulDay=1, NumDays
  r=r+DailyWindVel(JulDay)
 END DO
MeanWindVel=r/Real(NumDays)


! Daily Relative Humidity and Annual Mean Relative Humidity Calculation

ALLOCATE (DailyRelatHum(NumDays))
DO JulDay=1, NumDays
 w=0
 DO hh=1, 24
  DO INTERVAL=1, NumIntervalsPerHour
   w=w+WDay(JulDay)%RelHum(hh, INTERVAL)
  END DO
 END DO
 DailyRelatHum(JulDay)=w/24./Real(NumIntervalsPerHour)
END DO

r=0
 DO JulDay=1, NumDays
  r=r+DailyRelatHum(JulDay)
 END DO
MeanRelatHum=r/Real(NumDays)


! Calculation of Convective Heat Transfer Coefficient at the Soil Surface, hr and he

SoilHeatTranCoef=3.8*MeanWindVel+5.7
he=(0.0168*a*EvapFrac+1.)*SoilHeatTranCoef
hr=(0.0168*a*EvapFrac*MeanRelatHum/100.+1.)*SoilHeatTranCoef


! Calculation of Annual Angular Frequency and Dampening Depth

AngularFreq=2.*pi/(NumDays*24.)
DampDepth=SQRT(2.*SoilThermDiff/AngularFreq)



! Daily Air Temperature and Anuual Mean Air Temperature Calculation

ALLOCATE (DailyAirTemp(NumDays))
DO JulDay=1, NumDays
 s=0
 DO hh=1, 24
  DO INTERVAL=1, NumIntervalsPerHour
   s=s+WDay(JulDay)%DryBulb(hh, INTERVAL)
  END DO
 END DO
 DailyAirTemp(JulDay)=s/24./Real(NumIntervalsPerHour)
END DO

g=0
 DO JulDay=1, NumDays
  g=g+DailyAirTemp(JulDay)
 END DO
MeanAirTemp=g/Real(NumDays)


! Maximum Daily Temperature

MaxAirTemp=DailyAirTemp(1)
DO i=2, NumDays
 IF (DailyAirTemp(i) > MaxAirTemp) THEN
  MaxAirTemp=DailyAirTemp(i)
 END IF
END DO

! Minimum Daily Temperature, Phase Constant of Air Temperature

MinAirTemp=DailyAirTemp(1)
PhaseConAirTemp=1
DO j=2, NumDays
 IF (DailyAirTemp(j) < MinAirTemp) THEN
  MinAirTemp=DailyAirTemp(j)
  PhaseConAirTemp=j
 END IF
END DO

! Amplitude of the Air Temperature
AmplAirTemp=(MaxAirTemp-MinAirTemp)/2.



! Daily Solar Radiation Calculation

ALLOCATE (DailySolarRad(NumDays))
DO JulDay=1,NumDays
 s=0
 DO hh=1, 24
  DO INTERVAL=1, NumIntervalsPerHour
   s=s+WDay(JulDay)%GlobHorzRad(hh, INTERVAL)
  END DO
 END DO
 DailySolarRad(JulDay)=s/24./Real(NumIntervalsPerHour)
END DO

g=0
 DO JulDay=1, NumDays
  g=g+DailySolarRad(JulDay)
 END DO
MeanSolarRad=g/Real(NumDays)


! Maximum Daily Solar Radiation

MaxSolarRad=DailySolarRad(1)
DO i=2, NumDays
 IF (DailySolarRad(i) > MaxSolarRad) THEN
  MaxSolarRad=DailySolarRad(i)
 END IF
END DO

! Minimum Daily Solar Radiation, Phase Constant of Solar Radiation

MinSolarRad=DailySolarRad(1)
PhaseConSolarRad=1
DO j=2, NumDays
 IF (DailySolarRad(j) < MinSolarRad) THEN
  MinSolarRad=DailySolarRad(j)
  PhaseConSolarRad=j
 END IF
END DO

! Amplitude of Solar Radiation
AmplSolarRad=(MaxSolarRad-MinSolarRad)/2.

DEALLOCATE (DailyWindVel, DailyRelatHum, DailyAirTemp, DailySolarRad)

! Phase Angle between Insolation and Air Temperature
PhaseAngle=(PhaseConAirTemp-PhaseConSolarRad)*2.*pi/Real(NumDays)


! Average Soil Surface Temperature Calculation
AverSoilSurTemp=(hr*MeanAirTemp-GroundEmit*RadConstant+AbsorpCoef*MeanSolarRad- &
                0.0168*SoilHeatTranCoef*EvapFrac*b*(1.-MeanRelatHum/100.))/he

! Amplitude of Soil Surface Temperature and Phase Angle Difference between Air and Soil
Process1=he+SoilThermCond/DampDepth
Process2=SoilThermCond/DampDepth
Process3=hr*AmplAirTemp
Process4=AbsorpCoef*AmplSolarRad*cos(PhaseAngle)
Process5=AbsorpCoef*AmplSolarRad*sin(PhaseAngle)
RealPart=(Process1*(Process3-Process4)+Process2*(-Process5))/(Process1**2+Process2**2)
ImagPart=(Process1*(-Process5)-Process2*(Process3-Process4))/(Process1**2+Process2**2)
ApmlSoilSurTemp=SQRT(RealPart**2+ImagPart**2)
PhaseAngleAirSoil=-ATAN(ImagPart/RealPart)

! Phase constant of Soil Surface Calculation
j=PhaseConAirTemp+ INT(PhaseAngleAirSoil/AngularFreq/24.)+1

IF (j<=0) THEN
 i=1
ELSE
 i=j
END IF

IF (i>365) THEN
 SoilSurPhaseConst=i-365
ELSE
 SoilSurPhaseConst=i
END IF

PRINT *, ' '
PRINT *, ' '
PRINT *, 'Annual Average Soil Surface Temperature', AverSoilSurTemp
PRINT *, 'Amplitude of Soil Surface Temperature', ApmlSoilSurTemp
PRINT *, 'Phase Constant of Soil Surface Temperature', SoilSurPhaseConst
PRINT *, 'Output can also be found in CalcSoilSurfTemp.out file'

OPEN(17,FILE='CalcSoilSurfTemp.out',STATUS='REPLACE')

WRITE (17,'(A)') ' Weather file used='//TRIM(InputFileNameWithPath)
WRITE (17, '("Annual Average Soil Surface Temperature", F18.12)') AverSoilSurTemp
WRITE (17, '("Amplitude of Soil Surface Temperature", F18.12)') ApmlSoilSurTemp
WRITE (17, '("Phase Constant of Soil Surface Temperature", I3)') SoilSurPhaseConst

CLOSE(17)

END PROGRAM CalcSoilSurfTemp
