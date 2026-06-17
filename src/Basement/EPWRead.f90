MODULE EPWRead

          ! Module containing the routines dealing with reading the raw weather (source) data

          ! MODULE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   March 2001 - November 2009
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS MODULE:
          ! This module contains all the source reading data routines.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! OTHER NOTES:
          ! na

          ! USE STATEMENTS:
  USE EPWPrecisionGlobals

IMPLICIT NONE ! Enforce explicit typing of all variables

PRIVATE ! Everything private unless explicitly made public

          ! MODULE PARAMETER DEFINITIONS:
  CHARACTER(len=*),  PARAMETER :: AFormat='(A)'
  CHARACTER(len=1),  PARAMETER :: BlankString=' '
  INTEGER,           PARAMETER :: MaxNameLength=60
  REAL(r64),         PARAMETER :: Sigma=5.6697d-8 ! Stefan-Boltzmann constant
  REAL(r64),         PARAMETER :: TKelvin=273.15d0  ! conversion from Kelvin to Celsius
  REAL(r64),         PARAMETER :: PI= 3.141592653589793d0   ! Pi

          ! DERIVED TYPE DEFINITIONS:
  TYPE, PUBLIC :: WeatherDataDetails
    ! one for each day
    INTEGER Year                  ! Year from source
    INTEGER Month                 ! Month from source
    INTEGER Day                   ! Day from source
    INTEGER DayOfYear             ! Day of Year for this date
    INTEGER IntervalMinute(60)    ! Number of Intervals -- Minutes for each
    CHARACTER(len=50) DataSourceFlags(24,60) ! Combined list of DataSourceFlags
                                  ! taken directly from weather file or "*" for calculated
                                  ! in this program
    REAL(r64) DryBulb(24,60)           ! Dry-Bulb Temp (C)
    REAL(r64) DewPoint(24,60)          ! Dew-Point Temp (C)
    REAL(r64) RelHum(24,60)            ! Relative Humidity (%)
    REAL(r64) StnPres(24,60)           ! Station Pressure (Pa)
    REAL(r64) xHorzRad(24,60)           ! Extraterrestial Horizontal Radiation (Wh/m2)
    REAL(r64) xDirNormRad(24,60)        ! Extraterrestial Direct Normal Radiation (Wh/m2)
    REAL(r64) HorzIRSky(24,60)          ! Horizontal Infrared Radiation from Sky (Wh/m2)
    REAL(r64) GlobHorzRad(24,60)        ! Global Horizontal Radiation (Wh/m2)
    REAL(r64) DirNormRad(24,60)         ! Direct Normal Radiation (Wh/m2)
    REAL(r64) DifHorzRad(24,60)         ! Diffuse Horizontal Radiation (Wh/m2)
    REAL(r64) GlobHorzIllum(24,60)      ! Global Horizontal Illuminance (lux)
    REAL(r64) DirNormIllum(24,60)       ! Direct Normal Illuminance (lux)
    REAL(r64) DifHorzIllum(24,60)       ! Diffuse Horizontal Illuminance (lux)
    REAL(r64) ZenLum(24,60)            ! Zenith Luminance (CD/m2)
    INTEGER WindDir(24,60)        ! Wind Direction (degrees)
    REAL(r64) WindSpd(24,60)           ! Wind Speed (m/s)
    INTEGER TotSkyCvr(24,60)      ! Total Sky Cover
    INTEGER OpaqSkyCvr(24,60)     ! Opaque Sky Cover
    REAL(r64) Visibility(24,60)        ! Visibility (km)
    INTEGER Ceiling(24,60)        ! Ceiling Height (m)
    INTEGER PresWthObs(24,60)     ! Present Weather Observations
    CHARACTER(len=9) PresWthCodes(24,60) ! Present Weather Codes
    INTEGER PrecipWater(24,60)    ! Precipitable Water (mm)
    REAL(r64) AerOptDepth(24,60)       ! Aerosol Optical Depth
    INTEGER SnowDepth(24,60)      ! Snow Depth (cm)
    INTEGER DaysLastSnow(24,60)   ! Number of Days since last snow
    REAL Albedo(24,60)            ! Albedo
    REAL LiquidPrecipDepth(24,60) ! mm
    REAL LiquidPrecipRate(24,60)  ! Period of accumulation
    INTEGER SnowInd(24,60)
    REAL(r64) HumRat(24,60)
    REAL(r64) WetBulb(24,60)           ! Wet-Bulb Temp (C)
    LOGICAL DeltaDBRange          ! Day has a delta DB bigger than normal
    REAL DeltaChgDB(24,60)        ! Changed intervals
    LOGICAL DeltaDPRange          ! Day has a delta DP bigger than normal
    REAL DeltaChgDP(24,60)        ! Changed intervals
  END TYPE

  TYPE MissingData          ! This Derived type carries the default missing data
                            ! for those data elements that would be best replaced
                            ! with the previous hour's data for missing data.
    REAL(r64) DryBulb            ! Dry Bulb Temperature (C)
    REAL(r64) DewPoint           ! Dew Point Temperature (C)
    INTEGER RelHumid        ! Relative Humidity (%)
    REAL(r64) StnPres            ! Atmospheric Pressure (Pa)
    INTEGER WindDir         ! Wind Direction (deg)
    REAL(r64) WindSpd            ! Wind Speed/Velocity (m/s)
    INTEGER TotSkyCvr       ! Total Sky Cover (tenths)
    INTEGER OpaqSkyCvr      ! Opaque Sky Cover (tenths)
    REAL(r64) Visibility         ! Visibility (km)
    INTEGER Ceiling         ! Ceiling Height (m)
    INTEGER PrecipWater     ! Precipitable Water (mm)
    REAL(r64) AerOptDepth        ! Aerosol Optical Depth
    INTEGER SnowDepth       ! Snow Depth (cm)
    INTEGER DaysLastSnow    ! Number of Days since last snow
    REAL :: Albedo       =0.0   ! Albedo
    REAL :: LiquidPrecip =0.0   ! Rain/Liquid Precipitation (mm)
  END TYPE

  TYPE MissingDataCounts    ! This Derived type carries the counts of missing data
                            ! items in the weather conversion process.  It will count
                            ! only items that are on the source file -- not those that
                            ! are derived from data on the source file.
                            ! Comments below illustrate the data that is being counted:
    INTEGER :: xHorzRad         ! Extraterrestrial Horizontal Radiation
    INTEGER :: xDirNormRad      ! Extraterrestrial Direct Normal Radiation
    INTEGER :: GloHorRad
    INTEGER :: DirNormRad
    INTEGER :: DifHorzRad
    INTEGER :: DryBulb         ! Dry Bulb Temperature (C)
    INTEGER :: DewPoint        ! Dew Point Temperature (C)
    INTEGER :: RelHumid        ! Relative Humidity (%)
    INTEGER :: StnPres         ! Atmospheric Pressure (Pa)
    INTEGER :: WindDir         ! Wind Direction (deg)
    INTEGER :: WindSpd         ! Wind Speed/Velocity (m/s)
    INTEGER :: TotSkyCvr       ! Total Sky Cover (tenths)
    INTEGER :: OpaqSkyCvr      ! Opaque Sky Cover (tenths)
    INTEGER :: Visibility      ! Visibility (km)
    INTEGER :: Ceiling         ! Ceiling Height (m)
    INTEGER :: PrecipWater     ! Precipitable Water (mm)
    INTEGER :: AerOptDepth     ! Aerosol Optical Depth
    INTEGER :: SnowDepth       ! Snow Depth (cm)
    INTEGER :: DaysLastSnow    ! Number of Days since last snow
    INTEGER :: Albedo       =0 ! Albedo
    INTEGER :: LiquidPrecip =0 ! Liquid Precip Depth
  END TYPE

          ! MODULE VARIABLE DECLARATIONS:
  TYPE (WeatherDataDetails), PUBLIC, DIMENSION(366) :: WDay  ! Allocate for each possible day of weather data
  TYPE (MissingData) :: Missing=MissingData(0.0,0.0,0,0.0,0,0.0,0,0,0.0,0,0,0.0,0,0,0.0,0.0)
  TYPE (MissingDataCounts),SAVE :: Missed=MissingDataCounts(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0)
  CHARACTER(len=MaxNameLength*2), PUBLIC :: LocationName=' ' ! Location name from weather file

  INTEGER :: NumDataPeriods=0  ! Number of data periods indicated on weather file
  REAL(r64),PUBLIC :: Latitude=0.0     ! Latitude (decimal equivalent) for weather file (N+/S-)
  REAL(r64),PUBLIC :: Longitude=0.0    ! Longitude (decimal equivalent) for weather file (W-/E+)
  REAL(r64),PUBLIC :: TimeZone=0.0     ! Time Zone (decimal equivalent - hours) GMT {+/-}
  REAL(r64),PUBLIC :: Elevation=0.0    ! Elevation of weather file station {meters}
  REAL(r64),PUBLIC :: StdBaroPress=0.0 ! Standard Barometric Pressure based on Elevation
  CHARACTER(len=10) :: StnWMO=' '  ! WMO Station Number (from LOCATION Header)
  INTEGER :: NumIntervalsPerHour=0 ! Number of intervals per hour on weather file
  INTEGER :: NumDays=0        ! Number of days on weather file

          ! SUBROUTINE SPECIFICATIONS FOR MODULE:
PUBLIC  ReadEPW
PRIVATE ProcessEPWHeaders
PRIVATE ProcessEPWHeader
PUBLIC  GetLocData
PUBLIC  GetSTM

CONTAINS

SUBROUTINE ReadEPW(InputFile,ErrorsFound,ErrorMessage,NumberOfDays)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   March 2001
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine reads the EPW files.  Since these are text files, people can, potentially,
          ! hand modify them for their own purposes.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  CHARACTER(len=*), INTENT(IN)    :: InputFile     ! Name of Input File
  LOGICAL, INTENT(INOUT)          :: ErrorsFound   ! True if errors found while processing
  CHARACTER(len=*), INTENT(INOUT) :: ErrorMessage ! Error message causing halt
  INTEGER, INTENT(INOUT)          :: NumberOfDays

          ! SUBROUTINE PARAMETER DEFINITIONS:
  CHARACTER(len=*), PARAMETER :: WFmt="(1X,A,'[',A,']',A,I2,A,I2,A,I2)"
  CHARACTER(len=*), PARAMETER :: ValidDigits="0123456789"

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  INTEGER UnitNo  ! Unit for reading data
  INTEGER ios     ! IOStat variable
  CHARACTER(len=500) :: InputLine=BlankString
  INTEGER Com5(5) ! Position of first five commas (to get to DataSource flags
  CHARACTER(len=50) SourceFlags
  INTEGER Count
  INTEGER Pos
  INTEGER Lenth
  INTEGER Comma6
  CHARACTER(len=22) City
  CHARACTER(len=22) State
  CHARACTER(len=10) Country
  CHARACTER(len=6) WMO
  CHARACTER(len=25) String
  ! Data variables
  INTEGER Hour
  INTEGER Interval
  INTEGER WYear,WMonth,WDayOfMonth,WHour
  REAL(r64) WMinute
  REAL(r64) DryBulb,DewPoint,RelHumid,AtmPress,ExtHorzRad,ExtDirNormRad,IRHoriz,GloHorzRad,  &
       DirNormRad,DifHorzRad,GloHorzIllum,DirNormIllum,DifHorzIllum,ZenithLum, &
       WindDir,WindSpd,  &
       TotSkyCvr,OpqSkyCvr,Visibility,CeilHgt,PrecWtr,    &
       AerOptDepth,SnowDepth,DaysLastSnow,Albedo,Rain,RainRate
  INTEGER I_WindDir
  INTEGER PresWeathObs
  CHARACTER(len=15) PresWeathCodes
  ! Uncertainty flags
  CHARACTER(len=2) FldIR,Fld1,Fld2,Fld3,Fld4,Fld5,Fld6,Fld7,Fld8,Fld9,Fld10, &
                   Fld11,Fld12,Fld13,Fld14,Fld15,Fld16,Fld17,Fld18,Fld19,Fld20,Fld21,Fld22,Fld23,Fld24
  REAL(r64) TDewK
  REAL(r64) TDryK
  REAL(r64) OSky
  REAL(r64) ESky
  INTEGER I_rh
  LOGICAL RelHumidMissing
  REAL(r64) Pw,Pws
  REAL(r64) WSSTAR,T2
  REAL(r64) PDEW
  REAL(r64) SATUPT
  REAL(r64) HumRat
  LOGICAL LocDataDef
  LOGICAL MiscDataDef
  CHARACTER(len=60) SourceData

  UnitNo=GetNewUnitNumber()
  ErrorsFound=.false.
  ErrorMessage = ' '

  ! Initialize Missing
  Missing%StnPres      = 101325.  ! Initial "missing" value
  Missing%DryBulb      = 6.       ! Initial "missing" value
  Missing%DewPoint     = 3.       ! Initial "missing" value
  Missing%RelHumid     = 50       ! Initial "missing" value
  Missing%WindSpd      = 2.5      ! Initial "missing" value
  Missing%WindDir      = 180      ! Initial "missing" value
  Missing%TotSkyCvr    = 5        ! Initial "missing" value
  Missing%OpaqSkyCvr   = 5        ! Initial "missing" value
  Missing%Visibility   = 777.7    ! Initial "missing" value
  Missing%Ceiling      = 77777    ! Initial "missing" value
  Missing%PrecipWater  = 0        ! Initial "missing" value
  Missing%AerOptDepth  = 0        ! Initial "missing" value
  Missing%SnowDepth    = 0        ! Initial "missing" value
  Missing%DaysLastSnow = 88       ! Initial "missing" value
  Missing%Albedo       = 0.0
  Missing%LiquidPrecip = 0.0

  Missed%xHorzRad     = 0
  Missed%xDirNormRad  = 0
  Missed%GloHorRad    = 0
  Missed%DirNormRad   = 0
  Missed%DifHorzRad   = 0
  Missed%StnPres      = 0
  Missed%DryBulb      = 0
  Missed%DewPoint     = 0
  Missed%RelHumid     = 0
  Missed%WindSpd      = 0
  Missed%WindDir      = 0
  Missed%TotSkyCvr    = 0
  Missed%OpaqSkyCvr   = 0
  Missed%Visibility   = 0
  Missed%Ceiling      = 0
  Missed%PrecipWater  = 0
  Missed%AerOptDepth  = 0
  Missed%SnowDepth    = 0
  Missed%DaysLastSnow = 0
  Missed%Albedo       = 0
  Missed%LiquidPrecip = 0

! Initialize WDay
  NumDays=0
  NumIntervalsPerHour=0

  DO Count=1,366
    WDay(Count)%Year=0
    WDay(Count)%Month=0
    WDay(Count)%Day=0
    WDay(Count)%DayOfYear=0
    WDay(Count)%IntervalMinute=0
    WDay(Count)%DataSourceFlags=BlankString
    WDay(Count)%DryBulb=0.
    WDay(Count)%DewPoint=0.
    WDay(Count)%RelHum=0.
    WDay(Count)%StnPres=0.
    WDay(Count)%xHorzRad=0.
    WDay(Count)%xDirNormRad=0.
    WDay(Count)%HorzIRSky=0.
    WDay(Count)%GlobHorzRad=0.
    WDay(Count)%DirNormRad=0.
    WDay(Count)%DifHorzRad=0.
    WDay(Count)%GlobHorzIllum=0.
    WDay(Count)%DirNormIllum=0.
    WDay(Count)%DifHorzIllum=0.
    WDay(Count)%ZenLum=0.
    WDay(Count)%WindDir=0.
    WDay(Count)%WindSpd=0.
    WDay(Count)%TotSkyCvr=0
    WDay(Count)%OpaqSkyCvr=0
    WDay(Count)%Visibility=0.
    WDay(Count)%Ceiling=0
    WDay(Count)%PresWthObs=0
    WDay(Count)%PresWthCodes=BlankString
    WDay(Count)%PrecipWater=0
    WDay(Count)%AerOptDepth=0.
    WDay(Count)%SnowDepth=0
    WDay(Count)%DaysLastSnow=0
    WDay(Count)%Albedo=0.
    WDay(Count)%LiquidPrecipDepth=0.
    WDay(Count)%LiquidPrecipRate=0.
  ENDDO

!TODO-- ! Need to check if file is open already, etc.

  OPEN(UnitNo,FILE=InputFile,IOSTAT=ios,STATUS='OLD',ERR=900)
  IF (ios /=0) THEN
    goto 900
  ENDIF
  NumIntervalsPerHour=1

  ! EPW file has header records.  (If you need to process more information than shown here, feel free to modify the
  ! EPW Header routines and check back in.

  CALL ProcessEPWHeaders(UnitNo,ErrorsFound,ErrorMessage)
  IF (ErrorsFound) RETURN

  StdBaroPress=(101.325d0*(1.d0-2.25577d-05*Elevation)**5.2559d0)*1000.d0
  Missing%StnPres=StdBaroPress

  NumDays=0
  Hour=24
  Interval=NumIntervalsPerHour
  DO
    READ(UnitNo,AFormat,IOSTAT=ios,ERR=905) InputLine   ! Here we will allow data source flags to have spaces.
    IF (IOS /= 0) EXIT
    IF (InputLine == BlankString) EXIT
    Com5=0
    DO Count=1,5
      Com5(Count)=INDEX(InputLine,',')
      InputLine(Com5(Count):Com5(Count))='$'
    ENDDO
    ! And one more
    Comma6=INDEX(InputLine,',')
    SourceFlags=InputLine(Com5(5)+1:Comma6-1)
    SourceFlags=ADJUSTL(SourceFlags)
    Lenth=LEN_TRIM(SourceFlags)
    Pos=INDEX(SourceFlags(1:Lenth),' ')
    DO WHILE (Pos > 0)
      SourceFlags(Pos:Pos)='_'
      Pos=INDEX(SourceFlags(1:Lenth),' ')
    ENDDO
    ! Put it back
    InputLine(Com5(5)+1:Comma6-1)=SourceFlags
    ! Replace commas
    DO Count=1,5
      InputLine(Com5(Count):Com5(Count))=','
    ENDDO
    !  Now can read the line

    READ(InputLine,*,IOSTAT=ios,ERR=905) WYear,WMonth,WDayOfMonth,WHour,WMinute,SourceFlags
    ! Inputline is positioned at start of data (Dry Bulb)
    InputLine=InputLine(Comma6+1:)
    CALL ReadAndInterpretEPWWeatherLine(IOS,InputLine,  &
             DryBulb,DewPoint,RelHumid, &
             AtmPress,ExtHorzRad,ExtDirNormRad,IRHoriz,GloHorzRad,  &
             DirNormRad,DifHorzRad,GloHorzIllum,  &
             DirNormIllum,DifHorzIllum,ZenithLum,WindDir,WindSpd,  &
             TotSkyCvr,OpqSkyCvr,Visibility,CeilHgt,  &
             PresWeathObs,PresWeathCodes,PrecWtr,    &
             AerOptDepth,SnowDepth,DaysLastSnow,Albedo,Rain,RainRate)
      Pos=INDEX(PresWeathCodes,'''')
      DO WHILE (Pos > 0)
        PresWeathCodes(Pos:Pos)=' '
        Pos=INDEX(PresWeathCodes,'''')
      ENDDO
      Pos=INDEX(PresWeathCodes,'"')
      DO WHILE (Pos > 0)
        PresWeathCodes(Pos:Pos)=' '
        Pos=INDEX(PresWeathCodes,'"')
      ENDDO
      PresWeathCodes=ADJUSTL(PresWeathCodes)
      IF (LEN_TRIM(PresWeathCodes) == 9) THEN
        DO Pos=1,9
          IF (INDEX(ValidDigits,PresWeathCodes(Pos:Pos)) == 0) PresWeathCodes(Pos:Pos)='9'
        ENDDO
      ELSE
!        WRITE(AuditFile,WFmt) 'Warning ** WeatherCodes (not equal 9 digits)=',TRIM(PresWeathCodes),  &
!                              ' on date=',WMonth,'/',WDayOfMonth,' at hour=',Hour
!        NumOfWarnings=NumOfWarnings+1
      ENDIF

    Interval=Interval+1
    IF (Interval > NumIntervalsPerHour) THEN
      Interval=1
      Hour=Hour+1
    ENDIF
    IF (Hour > 24) THEN
      Hour=1
      NumDays=NumDays+1
    ENDIF
    IF (Hour == 1) THEN
      WDay(NumDays)%Year=WYear
      WDay(NumDays)%Month=WMonth
      WDay(NumDays)%Day=WDayOfMonth
    ENDIF
    WDay(NumDays)%IntervalMinute(Interval)=NINT(WMinute)
    READ(SourceFlags,'(25A2)') Fld1,Fld2,Fld3,Fld4,FldIR,Fld5,Fld6,Fld7,Fld8,Fld9,Fld10,  &
                       Fld11,Fld12,Fld13,Fld14,Fld15,Fld16,Fld17,Fld18,Fld19,Fld20,Fld21,Fld22,Fld23,Fld24
    ! Check missing values
    ! Check missing values
    IF (ExtHorzRad >= 9999) THEN
      Missed%xHorzRad=Missed%xHorzRad+1
    ENDIF
    IF (ExtDirNormRad >= 9999) THEN
      Missed%xDirNormRad=Missed%xDirNormRad+1
    ENDIF
    IF (GloHorzRad >= 9999) THEN
      Missed%GloHorRad=Missed%GloHorRad+1
    ENDIF
    IF (DirNormRad >= 9999) THEN
      Missed%DirNormRad=Missed%DirNormRad+1
    ENDIF
    IF (DifHorzRad >= 9999) THEN
      Missed%DifHorzRad=Missed%DifHorzRad+1
    ENDIF

    IF (DryBulb >= 99.9) THEN
      DryBulb=Missing%DryBulb
      Fld10(1:1)='*'
      Missed%DryBulb=Missed%DryBulb+1
    ENDIF

    IF (AtmPress >= 999999.) THEN
      AtmPress=Missing%StnPres
      Fld13='*'
      Missed%StnPres=Missed%StnPres+1
    ENDIF

    IF (RelHumid >= 999.) THEN
      RelHumid=Missing%RelHumid
      Fld12(1:1)='*'
      Missed%RelHumid=Missed%RelHumid+1
      RelHumidMissing=.true.
    ELSE
      RelHumidMissing=.false.
    ENDIF

    IF (DewPoint >= 99.9) THEN   ! Correlate missing Dewpoint to DryBulb, Pressure, Relative Humidity?
      Missed%DewPoint=Missed%DewPoint+1
      IF (RelHumidMissing) THEN
        DewPoint=Missing%DewPoint
        Fld11(1:1)='*'
        IF (DewPoint > DryBulb) THEN
          DewPoint=DryBulb - 3.
        ENDIF
      ELSE
    !   Calculate DewPoint
        Pws=Psat(REAL(DryBulb,8))                        !Saturation pressure at temperature
        Pw=MAX(RelHumid,1.)*.01d0*Pws
        DewPoint=SATUTP(Pw)
        Fld11(1:1)='*'
      ENDIF
    ENDIF

    IF (WindSpd >= 999.) THEN
      WindSpd=Missing%WindSpd
      Fld15(1:1)='*'
      Missed%WindSpd=Missed%WindSpd+1
    ENDIF

    IF (WindDir >= 999.) THEN
      WindDir=Missing%WindDir
      Fld14(1:1)='*'
      Missed%WindDir=Missed%WindDir+1
    ENDIF
    I_WindDir=WindDir

    IF (Visibility >= 9999.) THEN
      Visibility=Missing%Visibility
      Fld16(1:1)='*'
      Missed%Visibility=Missed%Visibility+1
    ENDIF

    IF (AerOptDepth >= .999) THEN
      AerOptDepth=Missing%AerOptDepth
      Fld19='*'
      Missed%AerOptDepth=Missed%AerOptDepth+1
    ENDIF

    IF (TotSkyCvr == 99.) THEN
      TotSkyCvr=Missing%TotSkyCvr
      Fld8(1:1)='*'
      Missed%TotSkyCvr=Missed%TotSkyCvr+1
    ENDIF
    IF (OpqSkyCvr == 99.) THEN
      OpqSkyCvr=Missing%OpaqSkyCvr
      Fld9(1:1)='*'
      Missed%OpaqSkyCvr=Missed%OpaqSkyCvr+1
    ENDIF
    IF (CeilHgt >= 99999.) THEN
      CeilHgt=Missing%Ceiling
      Fld17(1:1)='*'
      Missed%Ceiling=Missed%Ceiling+1
    ENDIF
    IF (PrecWtr >= 999) THEN
      PrecWtr=Missing%PrecipWater
      FLd18(1:1)='*'
      Missed%PrecipWater=Missed%PrecipWater+1
    ENDIF
    IF (SnowDepth >= 999) THEN
      SnowDepth=Missing%SnowDepth
      Fld20(1:1)='*'
      Missed%SnowDepth=Missed%SnowDepth+1
    ENDIF
    IF (DaysLastSnow >= 99) THEN
      DaysLastSnow=Missing%DaysLastSnow
      Fld21(1:1)='*'
      Missed%DaysLastSnow=Missed%DaysLastSnow+1
    ENDIF
    IF (Albedo >= 999.) THEN
      Albedo=0.0
      Fld22(1:1)='*'
      Missed%Albedo=Missed%Albedo+1
    ENDIF
    IF (Rain >= 999.) THEN
      Rain=0.0
      Fld23(1:1)='*'
      Missed%LiquidPrecip=Missed%LiquidPrecip+1
    ENDIF
    IF (RainRate >= 99.) THEN
      RainRate=99.
      Fld24(1:1)='*'
    ENDIF
!    IF (LEN_TRIM(PresWeathCodes) /= 9) THEN
!        WRITE(AuditFile,WFmt) 'Warning ** WeatherCodes (not equal 9 digits)=',TRIM(PresWeathCodes),  &
!                              ' on date=',WMonth,'/',WDayOfMonth,' at hour=',Hour
!        NumOfWarnings=NumOfWarnings+1
!    ENDIF

    ! Transfer to data structure

    WDay(NumDays)%xHorzRad(Hour,Interval)=ExtHorzRad
    WDay(NumDays)%xDirNormRad(Hour,Interval)=ExtDirNormRad
    WDay(NumDays)%GlobHorzRad(Hour,Interval)=GloHorzRad
    WDay(NumDays)%DirNormRad(Hour,Interval)=DirNormRad
    WDay(NumDays)%DifHorzRad(Hour,Interval)=DifHorzRad
    WDay(NumDays)%GlobHorzIllum(Hour,Interval)=GloHorzIllum
    WDay(NumDays)%DirNormIllum(Hour,Interval)=DirNormIllum
    WDay(NumDays)%DifHorzIllum(Hour,Interval)=DifHorzIllum
    WDay(NumDays)%ZenLum(Hour,Interval)=ZenithLum
    WDay(NumDays)%TotSkyCvr(Hour,Interval)=TotSkyCvr
    WDay(NumDays)%OpaqSkyCvr(Hour,Interval)=OpqSkyCvr
    WDay(NumDays)%DryBulb(Hour,Interval)=DryBulb
    WDay(NumDays)%DewPoint(Hour,Interval)=DewPoint
    WDay(NumDays)%RelHum(Hour,Interval)=RelHumid
    WDay(NumDays)%StnPres(Hour,Interval)=AtmPress
    WDay(NumDays)%WindDir(Hour,Interval)=I_WindDir
    WDay(NumDays)%WindSpd(Hour,Interval)=WindSpd
    WDay(NumDays)%Visibility(Hour,Interval)=Visibility
    WDay(NumDays)%Ceiling(Hour,Interval)=CeilHgt
    WDay(NumDays)%PresWthObs(Hour,Interval)=PresWeathObs
    WDay(NumDays)%PresWthCodes(Hour,Interval)=PresWeathCodes
    WDay(NumDays)%PrecipWater(Hour,Interval)=PrecWtr
    WDay(NumDays)%AerOptDepth(Hour,Interval)=AerOptDepth
    WDay(NumDays)%SnowDepth(Hour,Interval)=SnowDepth
    WDay(NumDays)%DaysLastSnow(Hour,Interval)=DaysLastSnow
    WDay(NumDays)%Albedo(Hour,Interval)=DaysLastSnow
    WDay(NumDays)%LiquidPrecipDepth(Hour,Interval)=Rain
    WDay(NumDays)%LiquidPrecipRate(Hour,Interval)=RainRate

    CALL DrySatPt(SATUPT,DryBulb)
    PDEW=RelHumid/100.d0*SATUPT
    HumRat=PDEW*0.62198d0/(AtmPress-PDEW)
    WDay(NumDays)%HumRat(Hour,Interval)=HumRat
    T2=DryBulb+273.15d0
    IF (DryBulb < 0.)THEN
      PWS=EXP(-5.6745359d+03/T2+6.3925247d0-9.677843d-03*T2+6.2215701d-07*T2**2+      &
            2.0747825d-09*T2**3-9.4840240d-13*T2**4+4.1635019d00*LOG(T2))
    ELSE
      PWS=EXP(-5.8002206d+03/T2+1.3914993d+00-4.8640239d-02*T2+4.176476d-05*T2**2-  &
            1.4452093d-08*T2**3+6.5459673d+00*LOG(T2))
    END IF
    WSSTAR=0.62198d0*(PWS/(AtmPress-PWS))
    WDay(NumDays)%WetBulb(Hour,Interval)=((2501.d0+1.805d0*DryBulb)*HumRat-2501.d0*WSSTAR+DryBulb)/               &
          &        (4.186d0*HumRat-2.381d0*WSSTAR+1.)

    IF (SnowDepth > 0)THEN
      WDay(NumDays)%SnowInd(Hour,Interval)=1
    ELSE
      WDay(NumDays)%SnowInd(Hour,Interval)=0
    END IF

    ! Recalc HorzIRSky only if <=0 or missing (9999.)
    IF (IRHoriz <= 0.0 .or. IRHoriz >= 9999.) THEN

      ! Calculate Sky IR
      !HIR = ESKY * SIGMA * (TOUT**4)
      !
      !where
      !
      !HIR = horizontal IR intensity (W/m2)
      !ESKY = sky emissivity
      !SIGMA = Stefan-Boltzmann constant = 5.6697e-8 W/m2-K4
      !TOUT = drybulb temperature (K)
      !
      !The sky emissivity is given by
      !
      !ESKY = [0.787 + 0.764*ln(TDEW/273)]*[1 + 0.0224*N - 0.0035*(N**2) + 0.00028*(N**3)]
      !
      !where
      !
      !TDEW = dewpoint temperature (K)
      !N = opaque sky cover (tenths)
      !
      !Example: Clear sky (N=0), TOUT = 273+20=293K, TDEW = 273+10=283K:
      !
      !ESKY = 0.787 + 0.764*0.036 = 0.815
      !HIR = 0.815*5.6697e-8*(293**4) = 340.6 W/m^2

      !References:
      !
      !George N. Walton, "Thermal Analysis Research Program Reference Manual,"
      !NBSIR 83-2655, March 1983, p. 21.
      !
      !G. Clark and C. Allen, "The Estimation of Atmospheric Radiation for Clear and
      !Cloudy Skies," Proc. 2nd National Passive Solar Conference (AS/ISES), 1978, pp. 675-678.
       !!!must calcWDay(NumDays)%HorzIRSky(Hour,Interval)
      TDewK=WDay(NumDays)%DewPoint(Hour,Interval)+TKelvin
      TDryK=WDay(NumDays)%DryBulb(Hour,Interval)+TKelvin
      OSky=WDay(NumDays)%OpaqSkyCvr(Hour,Interval)
      ESky= (.787d0 +.764d0*LOG(TDewK/TKelvin))*(1.d0 + .0224d0*OSky - 0.0035d0*(OSky**2) + .00028d0*(OSky**3))

      WDay(NumDays)%HorzIRSky(Hour,Interval)=ESky*Sigma*(TDryK**4)
    ELSE
      WDay(NumDays)%HorzIRSky(Hour,Interval)=IRHoriz
    ENDIF

    WDay(NumDays)%DataSourceFlags(Hour,Interval)=Fld1//Fld2//Fld3//Fld4//FldIR//Fld5//Fld6//Fld7//Fld8//Fld9//Fld10// &
                             Fld11//Fld12//Fld13//Fld14//Fld15//Fld16//Fld17//Fld18//Fld19//Fld20//Fld21
    !  Cannot have spaces in DataSourceFlags
    Lenth=LEN_TRIM(WDay(NumDays)%DataSourceFlags(Hour,Interval))
    Pos=INDEX(WDay(NumDays)%DataSourceFlags(Hour,Interval)(1:Lenth),' ')
    DO WHILE (Pos > 0)
      WDay(NumDays)%DataSourceFlags(Hour,Interval)(Pos:Pos)='_'
      Pos=INDEX(WDay(NumDays)%DataSourceFlags(Hour,Interval)(1:Lenth),' ')
    ENDDO
    !  Cannot have commas in DataSourceFlags
    Lenth=LEN_TRIM(WDay(NumDays)%DataSourceFlags(Hour,Interval))
    Pos=INDEX(WDay(NumDays)%DataSourceFlags(Hour,Interval)(1:Lenth),',')
    DO WHILE (Pos > 0)
      WDay(NumDays)%DataSourceFlags(Hour,Interval)(Pos:Pos)='_'
      Pos=INDEX(WDay(NumDays)%DataSourceFlags(Hour,Interval)(1:Lenth),',')
    ENDDO

   ! Reset missing values
   Missing%StnPres      = AtmPress
   Missing%DryBulb      = DryBulb
   Missing%DewPoint     = DewPoint
   Missing%WindSpd      = WindSpd
   Missing%WindDir      = WindDir
   Missing%TotSkyCvr    = TotSkyCvr
   Missing%OpaqSkyCvr   = OpqSkyCvr
   Missing%Visibility   = Visibility
   Missing%Ceiling      = CeilHgt
   Missing%PrecipWater  = PrecWtr
   Missing%AerOptDepth  = AerOptDepth
   Missing%SnowDepth    = SnowDepth
   Missing%DaysLastSnow = DaysLastSnow

  ENDDO

  NumberOfDays=NumDays

  CLOSE(UnitNo)

  RETURN

900 ErrorMessage=' *** Could not open EPW file='//TRIM(InputFile)
    ErrorsFound=.true.
    NumberOfDays=0
    RETURN

904 ErrorMessage='Severe ** Error during processing EPW header'
    ErrorsFound=.true.
    NumberOfDays=0
    RETURN

905 write(ErrorMessage,'(A,A,I5)') ' *** Line in error='//TRIM(InputLine), &
                               ' ** Error during processing day #',NumDays
    ErrorsFound=.true.
    NumberOfDays=0
    RETURN

END SUBROUTINE ReadEPW

SUBROUTINE ProcessEPWHeaders(WeatherFileUnitNumber,ErrorsFound,ErrorMessage)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   June 1999
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine reads the EnergyPlus Weather File (in.epw) and processes
          ! the initial header records.

          ! METHODOLOGY EMPLOYED:
          ! List directed reads, as possible.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  INTEGER, INTENT(IN)    :: WeatherFileUnitNumber ! Unit number for EPW file
  LOGICAL, INTENT(INOUT) :: ErrorsFound     ! Will be set to true if errors found
  CHARACTER(len=*), INTENT(INOUT) :: ErrorMessage ! Error message causing halt

          ! SUBROUTINE PARAMETER DEFINITIONS:
  CHARACTER(len=*), PARAMETER, DIMENSION(8) :: Header=(/"LOCATION                ","DESIGN CONDITIONS       ",  &
                                                        "TYPICAL/EXTREME PERIODS ","GROUND TEMPERATURES     ",  &
                                                        "HOLIDAYS/DAYLIGHT SAVING","COMMENTS 1              ",  &
                                                        "COMMENTS 2              ","DATA PERIODS            "/)

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  INTEGER Pos
  CHARACTER(len=800) Line
  integer HdPos,HdLine
  logical StillLooking

  ! Read in Header Information

  ! Headers should come in order
    HdLine=1   ! Look for first Header
    StillLooking=.true.
    DO WHILE (StillLooking)
      READ(WeatherFileUnitNumber,AFormat,END=9998) line
      Pos=FindNonSpace(line)
      HdPos=INDEX(line,TRIM(Header(HdLine)))
      IF (Pos /= HdPos) CYCLE
      CALL ProcessEPWHeader(WeatherFileUnitNumber,Header(HdLine),line,ErrorsFound,ErrorMessage)
      IF (ErrorsFound) THEN
        RETURN
      ENDIF
      HdLine=HdLine+1
      IF (HdLine == 9) StillLooking=.false.
    ENDDO

  RETURN

  9998 ErrorMessage='Unexpected End-of-File on EPW Weather file, while reading header information, looking for header='// &
                            TRIM(Header(HdLine))
  ErrorsFound=.true.

  RETURN

END SUBROUTINE ProcessEPWHeaders

SUBROUTINE ProcessEPWHeader(WeatherFileUnitNumber,HeaderString,Line,ErrorsFound,ErrorMessage)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   December 1999
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine processes each header line in the EPW weather file.

          ! METHODOLOGY EMPLOYED:
          ! File is positioned to the correct line, then backspaced.  This routine
          ! reads in the line and processes as appropriate.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  INTEGER, INTENT(IN)             :: WeatherFileUnitNumber
  CHARACTER(len=*), INTENT(IN)    :: HeaderString
  CHARACTER(len=*), INTENT(INOUT) :: Line
  LOGICAL, INTENT(INOUT)          :: ErrorsFound
  CHARACTER(len=*), INTENT(INOUT) :: ErrorMessage ! Error message causing halt

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  CHARACTER(len=300) :: Title=' '
  INTEGER :: Count
  CHARACTER(len=6) CharWMO
  INTEGER Pos
  REAL(r64) Number
  LOGICAL IOStatus
  INTEGER NumHdArgs
  LOGICAL ErrFlag
  CHARACTER(len=20) ErrNum
  INTEGER CurCount
  INTEGER CurOne
  INTEGER NumEPWHolidays
  INTEGER PMonth
  INTEGER PDay
  INTEGER PWeekDay
  INTEGER DateType
  INTEGER DataStJDay
  INTEGER DataEnJDay
  INTEGER Loop
  INTEGER M


  ! Strip off Header value from Line
  Pos=INDEX(Line,',')
  Line=Line(Pos+1:)

  SELECT CASE(HeaderString)

    CASE ('LOCATION')

  !
  ! LOCATION, A1 [City], A2 [State/Province/Region], A3 [Country],
  ! A4 [Source], N1 [WMO], N2 [Latitude],
  ! N3 [Longitude], N4 [Time Zone], N5 [Elevation {m}]

!      EPWLocLine='LOCATION,'//TRIM(Line)

      NumHdArgs=9
      Count=1
      DO WHILE (Count <= NumHdArgs)
        Line=ADJUSTL(Line)
        Pos=INDEX(Line,',')
        IF (Pos == 0) THEN
          IF (LEN_TRIM(Line) == 0) THEN
            DO WHILE (Pos == 0)
              READ(WeatherFileUnitNumber,AFormat) Line
              Line=ADJUSTL(Line)
              Pos=INDEX(Line,',')
            ENDDO
          ELSE
            Pos=LEN_TRIM(Line)+1
          ENDIF
        ENDIF

        SELECT CASE(Count)

          CASE (1)
            Title=TRIM(Line(1:Pos-1))

          CASE (2,3,4)
            Title=TRIM(Title)//' '//TRIM(Line(1:Pos-1))

          CASE (5,6,7,8,9)
            IF (Count == 5) THEN
              CharWMO=Line(1:Pos-1)
              StnWMO=CharWMO
            ELSE
              Number=ProcessNumber(Line(1:Pos-1),ErrFlag)
              IF (.not. ErrFlag) THEN
                SELECT CASE (Count)
                  CASE (6)
                    Latitude=Number
                  CASE (7)
                    Longitude=Number
                  CASE (8)
                    TimeZone=Number
                  CASE (9)
                    Elevation=Number
                END SELECT
              ELSE
                ErrorMessage='GetEPWHeader:LOCATION, invalid numeric='//Line(1:Pos-1)
                ErrorsFound=.true.
                RETURN
              ENDIF
            ENDIF
        END SELECT
        Line=Line(Pos+1:)
        Count=Count+1
      ENDDO
      LocationName=ADJUSTL(Title)

    CASE ('DESIGN CONDITIONS')
!      EPWDesCondLine='DESIGN CONDITIONS,'//TRIM(Line)
      Line=ADJUSTL(Line)
      Pos=INDEX(Line,',')
      IF (Pos == 0) THEN
        IF (LEN_TRIM(Line) == 0) THEN
          DO WHILE (Pos == 0)
            READ(WeatherFileUnitNumber,AFormat) Line
            Line=ADJUSTL(Line)
            Pos=INDEX(Line,',')
          ENDDO
        ELSE
          Pos=LEN_TRIM(Line)+1
        ENDIF
      ENDIF
!      NumEPWDesCondSets=ProcessNumber(Line(1:Pos-1),IOStatus)

    CASE ('TYPICAL/EXTREME PERIODS')
!      EPWTYPEXTLine='TYPICAL/EXTREME PERIODS,'//TRIM(Line)
      Line=ADJUSTL(Line)
      Pos=INDEX(Line,',')
      IF (Pos == 0) THEN
        IF (LEN_TRIM(Line) == 0) THEN
          DO WHILE (Pos == 0)
            READ(WeatherFileUnitNumber,AFormat) Line
            Line=ADJUSTL(Line)
            Pos=INDEX(Line,',')
          ENDDO
        ELSE
          Pos=LEN_TRIM(Line)+1
        ENDIF
      ENDIF
!      NumEPWTypExtSets=ProcessNumber(Line(1:Pos-1),IOStatus)

    CASE ('GROUND TEMPERATURES')
!      EPWGRNDLine='GROUND TEMPERATURES,'//TRIM(Line)
      Line=ADJUSTL(Line)
      Pos=INDEX(Line,',')
      IF (Pos == 0) THEN
        IF (LEN_TRIM(Line) == 0) THEN
          DO WHILE (Pos == 0)
            READ(WeatherFileUnitNumber,AFormat) Line
            Line=ADJUSTL(Line)
            Pos=INDEX(Line,',')
          ENDDO
        ELSE
          Pos=LEN_TRIM(Line)+1
        ENDIF
      ENDIF
!      NumInputEPWGrndSets=ProcessNumber(Line(1:Pos-1),IOStatus)
!      NumEPWGrndSets=NumInputEPWGrndSets
!      IF (NumInputEPWGrndSets > 0) THEN
!        ALLOCATE(MonthlyGroundTemps(NumInputEPWGrndSets))
!        Line=Line(Pos+1:)
!        READ(Line,*) (MonthlyGroundTemps(Loop)%Depth,MonthlyGroundTemps(Loop)%Conductivity,  &
!           MonthlyGroundTemps(Loop)%Density,MonthlyGroundTemps(Loop)%SpecHeat,               &
!           (MonthlyGroundTemps(Loop)%Temps(M),M=1,12),Loop=1,NumInputEPWGrndSets)
!      ENDIF


    CASE ('HOLIDAYS/DAYLIGHT SAVING')
     !A1, \field LeapYear Observed
     ! \type choice
     ! \key Yes
     ! \key No
     ! \note Yes if Leap Year will be observed for this file
     ! \note No if Leap Year days (29 Feb) should be ignored in this file
     !A2, \field Daylight Saving Start Day
     !A3, \field Daylight Saving End Day
     !N1, \field Number of Holidays
     !A4, \field Holiday 1 Name
     !A5, \field Holiday 1 Day
     ! etc.
     ! Start with Minimum number of NumHdArgs
!      EPWHOLDSTLine='HOLIDAYS/DAYLIGHT SAVINGS,'//TRIM(Line)
      NumHdArgs=4
      Count=1
      DO WHILE (Count <= NumHdArgs)
        Line=ADJUSTL(Line)
        Pos=INDEX(Line,',')
        IF (Pos == 0) THEN
          IF (LEN_TRIM(Line) == 0) THEN
            DO WHILE (Pos == 0)
              READ(WeatherFileUnitNumber,AFormat) Line
              Line=ADJUSTL(Line)
              !Line=MakeUPPERCase(Line)
              Pos=INDEX(Line,',')
            ENDDO
          ELSE
            Pos=LEN_TRIM(Line)+1
          ENDIF
        ENDIF

!        SELECT CASE(Count)
!
!          CASE(1)
!            IF (Line(1:1) == 'Y') THEN
!              LeapYear=.true.
!              WFLeapYearInd=1
!            ELSE
!              LeapYear=.false.
!              WFLeapYearInd=0
!            ENDIF
!
!          CASE(2)
!            EPWDST%TypeString=Line(1:Pos-1)
!
!          CASE(3)
!            EPWDST%TypeString=TRIM(EPWDST%TypeString)//','//Line(1:Pos-1)
!
!          CASE(4)
!            NumEPWHolidays=ProcessNumber(Line(1:Pos-1),IOStatus)
!            NumSpecialDays=NumEPWHolidays
!            ALLOCATE(SpecialDays(NumSpecialDays))
!            NumHdArgs=4+NumEPWHolidays*2
!            CurCount=0
!
!          CASE(5:)
!          IF (MOD(Count,2) /= 0) THEN
!            CurCount=CurCount+1
!            IF (CurCount > NumSpecialDays) THEN
!              WRITE(AuditFile,AFormat) 'Too many SpecialDays'
!              ErrorsFound=.true.
!            ELSE
!              SpecialDays(CurCount)%Name=Line(1:Pos-1)
!            ENDIF
!            ! Process name
!          ELSE
!            IF (CurCount <= NumSpecialDays) THEN
!              SpecialDays(CurCount)%TypeString=Line(1:Pos-1)
!            ENDIF
!          ENDIF
!        END SELECT
        Line=Line(Pos+1:)
        Count=Count+1
      ENDDO

    CASE ('COMMENTS 1')
!      EPWCmt1Line='COMMENTS 1,'//TRIM(Line)

    CASE ('COMMENTS 2')
!      EPWCmt2Line='COMMENTS 2,'//TRIM(Line)

    CASE ('DATA PERIODS')
!     N1, \field Number of Data Periods
!     N2, \field Number of Records per hour
!     A1, \field Data Period 1 Name/Description
!     A2, \field Data Period 1 Start Day of Week
!       \type choice
!       \key  Sunday
!       \key  Monday
!       \key  Tuesday
!       \key  Wednesday
!       \key  Thursday
!       \key  Friday
!       \key  Saturday
!     A3, \field Data Period 1 Start Day
!     A4, \field Data Period 1 End Day
!      EPWDataLine='DATA PERIODS,'//TRIM(Line)
      NumHdArgs=2
      Count=1
      DO WHILE (Count <= NumHdArgs)
        Line=ADJUSTL(Line)
        Pos=INDEX(Line,',')
        IF (Pos == 0) THEN
          IF (LEN_TRIM(Line) == 0) THEN
            DO WHILE (Pos == 0)
              READ(WeatherFileUnitNumber,AFormat) Line
              Line=ADJUSTL(Line)
              Pos=INDEX(Line,',')
            ENDDO
          ELSE
            Pos=LEN_TRIM(Line)+1
          ENDIF
        ENDIF

        SELECT CASE(Count)

        CASE(1)
          NumDataPeriods=ProcessNumber(Line(1:Pos-1),IOStatus)
!          ALLOCATE(DataPeriods(NumDataPeriods))
!          NumHdArgs=NumHdArgs+4*NumDataPeriods
!          CurCount=0

        CASE(2)
          NumIntervalsPerHour=ProcessNumber(Line(1:Pos-1),IOStatus)

        CASE(3:)
!          CurOne=MOD(Count-3,4)
!
!          SELECT CASE(CurOne)
!
!            CASE(0)
!              ! Description of Data Period
!              CurCount=CurCount+1
!              IF (CurCount > NumDataPeriods) THEN
!                WRITE(AuditFile,AFormat) 'Too many data periods'
!                ErrorsFound=.true.
!              ELSE
!                DataPeriods(CurCount)%Name=Line(1:Pos-1)
!              ENDIF
!
!            CASE(1)
!              ! Start Day of Week
!              IF (CurCount <= NumDataPeriods) THEN
!                DataPeriods(CurCount)%DayOfWeek=Line(1:Pos-1)
!                DataPeriods(CurCount)%WeekDay=FindItem(DataPeriods(CurCount)%DayOfWeek,DaysOfWeek,7)
!                IF (DataPeriods(CurCount)%WeekDay == 0) THEN
!                  WRITE(ErrNum,*) CurCount
!                  ErrNum=ADJUSTL(ErrNum)
!                  WRITE(AuditFile,AFormat) 'Weather File -- Invalid Start Day of Week for Data Period #'//TRIM(ErrNum)// &
!                                       ', Invalid day='//TRIM(DataPeriods(CurCount)%DayOfWeek)
!                  ErrorsFound=.true.
!                ENDIF
!              ENDIF
!
!
!            CASE(2)
!              ! DataPeriod Start Day
!              IF (CurCount <= NumDataPeriods) THEN
!                DataPeriods(CurCount)%TypeString=Line(1:Pos-1)
!                CALL ProcessDateString(Line(1:Pos-1),PMonth,PDay,PWeekDay,DateType,ErrFlag)
!                IF (DateType == MonthDay) THEN
!                  DataPeriods(CurCount)%StMon=PMonth
!                  DataPeriods(CurCount)%StDay=PDay
!                ELSE
!                  WRITE(AuditFile,AFormat) 'Data Periods must be of the form <DayOfYear> or <Month Day> (WeatherFile), found='  &
!                                       //TRIM(Line(1:Pos-1))
!                  ErrorsFound=.true.
!                ENDIF
!              ENDIF
!
!            CASE(3)
!              IF (CurCount <= NumDataPeriods) THEN
!                DataPeriods(CurCount)%TypeString=TRIM(DataPeriods(CurCount)%TypeString)//','//Line(1:Pos-1)
!                CALL ProcessDateString(Line(1:Pos-1),PMonth,PDay,PWeekDay,DateType,ErrFlag)
!                IF (DateType == MonthDay) THEN
!                  DataPeriods(CurCount)%EnMon=PMonth
!                  DataPeriods(CurCount)%EnDay=PDay
!                ELSE
!                  WRITE(AuditFile,AFormat) 'Data Periods must be of the form <DayOfYear> or <Month Day>, (WeatherFile) found='  &
!                                       //TRIM(Line(1:Pos-1))
!                  ErrorsFound=.true.
!                ENDIF
!              ENDIF
!              DataStJDay=JulianDay(DataPeriods(CurCount)%StMon,DataPeriods(CurCount)%StDay,WFLeapYearInd)
!              DataEnJDay=JulianDay(DataPeriods(CurCount)%EnMon,DataPeriods(CurCount)%EnDay,WFLeapYearInd)
!              IF (DataStJDay <= DataEnJDay) THEN
!                DataPeriods(CurCount)%NumDays=DataEnJDay-DataStJDay+1
!              ELSE
!                DataPeriods(CurCount)%NumDays=(365-DataStJDay+1)+(DataEnJDay-1+1)
!              ENDIF
!
!          END SELECT
        END SELECT
        Line=Line(Pos+1:)
        Count=Count+1
       ENDDO

    CASE DEFAULT
      ErrorMessage='Invalid EPW Header designation found='//TRIM(HeaderString)
      ErrorsFound=.true.
      RETURN

  END SELECT
  RETURN

END SUBROUTINE ProcessEPWHeader

SUBROUTINE ReadAndInterpretEPWWeatherLine(IOS,InputLine,DryBulb,DewPoint,RelHum, &
              AtmPress,ExtHorzRad,ExtDirNormRad,IRHoriz,GloHorzRad,  &
              DirNormRad,DifHorzRad,GloHorzIllum,  &
              DirNormIllum,DifHorzIllum,ZenithLum,WindDir,WindSpd,  &
              TotSkyCvr,OpqSkyCvr,Visibility,CeilHgt,  &
              PresWeathObs,PresWeathCodes,PrecWtr,     &
              AerOptDepth,SnowDepth,DaysLastSnow,Albedo,Rain,RainRate)


          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   June 2009
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This routine allows processing of the EPW or EPW-CSV lines from the Dry-Bulb field through
          ! the end of the data line.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  integer, intent(inout) :: IOS
  character(len=*), intent(inout) :: InputLine
  real(r64), intent(inout) :: DryBulb
  real(r64), intent(inout) :: DewPoint
  real(r64), intent(inout) :: RelHum
  real(r64), intent(inout) :: AtmPress
  real(r64), intent(inout) :: ExtHorzRad
  real(r64), intent(inout) :: ExtDirNormRad
  real(r64), intent(inout) :: IRHoriz
  real(r64), intent(inout) :: GloHorzRad
  real(r64), intent(inout) :: DirNormRad
  real(r64), intent(inout) :: DifHorzRad
  real(r64), intent(inout) :: GloHorzIllum
  real(r64), intent(inout) :: DirNormIllum
  real(r64), intent(inout) :: DifHorzIllum
  real(r64), intent(inout) :: ZenithLum
  real(r64), intent(inout) :: WindDir
  real(r64), intent(inout) :: WindSpd
  real(r64), intent(inout) :: TotSkyCvr
  real(r64), intent(inout) :: OpqSkyCvr
  real(r64), intent(inout) :: Visibility
  real(r64), intent(inout) :: CeilHgt
  integer, intent(inout) :: PresWeathObs
  character(len=*), intent(inout) :: PresWeathCodes
  real(r64), intent(inout) :: PrecWtr
  real(r64), intent(inout) :: AerOptDepth
  real(r64), intent(inout) :: SnowDepth
  real(r64), intent(inout) :: DaysLastSnow
  real(r64), intent(inout) :: Albedo
  real(r64), intent(inout) :: Rain
  real(r64), intent(inout) :: RainRate

          ! SUBROUTINE PARAMETER DEFINITIONS:
  character(len=*), parameter :: fmta="(A)"
  character(len=*), parameter :: BlankString=' '

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  CHARACTER(len=25), DIMENSION(29) :: Fields
  CHARACTER(len=80) FieldString   ! Field indicator in case of errors
  INTEGER FieldNo
  integer :: count
  integer :: pos
  integer :: Ios1

    Fields=BlankString
    DO Count=1,25
      Pos=INDEX(InputLine,',')
      Fields(Count)=Inputline(1:Pos-1)
      InputLine=InputLine(Pos+1:)
    ENDDO
    Pos=INDEX(InputLine,',')
    IF (Pos == 0) THEN
      Fields(26)=InputLine
    ELSE
      Fields(26)=InputLine(1:Pos-1)
      InputLine=InputLine(Pos+1:)
      Pos=INDEX(InputLine,',')
      IF (Pos == 0) THEN
        Fields(27)=InputLine
      ELSE
        Fields(27)=InputLine(1:Pos-1)
        InputLine=InputLine(Pos+1:)
        Pos=INDEX(InputLine,',')
        IF (Pos == 0) THEN
          Fields(28)=InputLine
        ELSE
          Fields(28)=InputLine(1:Pos-1)
          InputLine=InputLine(Pos+1:)
          Fields(29)=InputLine
        ENDIF
      ENDIF
    ENDIF

    IF (Fields(1) /= BlankString) THEN
      FieldNo=1
      FieldString='Dry-Bulb Temperature'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=101) DryBulb
 101  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=2
      endif
    ELSE
      DryBulb=999.  ! Blank -- missing
    ENDIF
    IF (Fields(2) /= BlankString) THEN
      FieldNo=2
      FieldString='Dew-Point Temperature'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=102) DewPoint
 102  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=2
      endif
    ELSE
      DewPoint=999.  ! Blank -- missing
    ENDIF
    IF (Fields(3) /= BlankString) THEN
      FieldNo=3
      FieldString='Relative Humidity'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=103) RelHum
 103  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=3
      endif
    ELSE
      RelHum=999.  ! Blank -- missing
    ENDIF
    IF (Fields(4) /= BlankString) THEN
      FieldNo=4
      FieldString='Atmospheric/Station Pressure'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=104) AtmPress
 104  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=4
      endif
    ELSE
      AtmPress=999999.  ! Blank -- missing
    ENDIF
    IF (Fields(5) /= BlankString) THEN
      FieldNo=5
      FieldString='Extraterrestrial Horizontal Radiation'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=105) ExtHorzRad
 105  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=1
      endif
    ELSE
      ExtHorzRad=9999.  ! Blank -- missing
    ENDIF
    IF (Fields(6) /= BlankString) THEN
      FieldNo=6
      FieldString='Extraterrestrial Direct Normal Radiation'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=106) ExtDirNormRad
 106  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=1
      endif
    ELSE
      ExtDirNormRad=9999.  ! Blank -- missing
    ENDIF
    IF (Fields(7) /= BlankString) THEN
      FieldNo=7
      FieldString='Horizontal Infrared Radiation Intensity from Sky'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=107) IRHoriz
 107  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=7
      endif
    ELSE
      IRHoriz=9999.  ! Blank -- missing
    ENDIF
    IF (Fields(8) /= BlankString) THEN
      FieldNo=8
      FieldString='Global Horizontal Radiation'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=108) GloHorzRad
 108  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=1
      endif
    ELSE
      GloHorzRad=9999.  ! Blank -- missing
    ENDIF
    IF (Fields(9) /= BlankString) THEN
      FieldNo=9
      FieldString='Direct Normal Radiation'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=109) DirNormRad
 109  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=9
      endif
    ELSE
      DirNormRad=9999.  ! Blank -- missing
    ENDIF
    IF (Fields(10) /= BlankString) THEN
      FieldNo=10
      FieldString='Diffuse Horizontal Radiation'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=110) DifHorzRad
 110  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=10
      endif
    ELSE
      DifHorzRad=9999.  ! Blank -- missing
    ENDIF
    IF (Fields(11) /= BlankString) THEN
      FieldNo=11
      FieldString='Global Horizontal Illuminance'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=111) GloHorzIllum
 111  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=1
      endif
    ELSE
      GloHorzIllum=999999.  ! Blank -- missing
    ENDIF
    IF (Fields(12) /= BlankString) THEN
      FieldNo=12
      FieldString='Direct Normal Illuminance'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=112) DirNormIllum
 112  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=1
      endif
    ELSE
      DirNormIllum=999999.  ! Blank -- missing
    ENDIF
    IF (Fields(13) /= BlankString) THEN
      FieldNo=13
      FieldString='Diffuse Horizontal Illuminance'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=113) DifHorzIllum
 113  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=1
      endif
    ELSE
      DifHorzIllum=999999.  ! Blank -- missing
    ENDIF
    IF (Fields(14) /= BlankString) THEN
      FieldNo=14
      FieldString='Zenith Luminance'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=114) ZenithLum
 114  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=1
      endif
    ELSE
      ZenithLum=99999.  ! Blank -- missing
    ENDIF
    IF (Fields(15) /= BlankString) THEN
      FieldNo=15
      FieldString='Wind Direction'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=115) WindDir
 115  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=15
      endif
    ELSE
      WindDir=999.  ! Blank -- missing
    ENDIF
    IF (Fields(16) /= BlankString) THEN
      FieldNo=16
      FieldString='Wind Speed'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=116) WindSpd
 116  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=16
      endif
    ELSE
      WindSpd=999.  ! Blank -- missing
    ENDIF
    IF (Fields(17) /= BlankString) THEN
      FieldNo=17
      FieldString='Total Sky Cover'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=117) TotSkyCvr
 117  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=17
      endif
    ELSE
      TotSkyCvr=99.  ! Blank -- missing
    ENDIF
    IF (Fields(18) /= BlankString) THEN
      FieldNo=18
      FieldString='Opaque Sky Cover'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=118) OpqSkyCvr
 118  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=18
      endif
    ELSE
      OpqSkyCvr=99.  ! Blank -- missing
    ENDIF
    IF (Fields(19) /= BlankString) THEN
      FieldNo=19
      FieldString='Visibility'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=119) Visibility
 119  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=1
      endif
    ELSE
      Visibility=9999.  ! Blank -- missing
    ENDIF
    IF (Fields(20) /= BlankString) THEN
      FieldNo=20
      FieldString='Ceiling Height'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=120) CeilHgt
 120  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=1
      endif
    ELSE
      CeilHgt=99999.  ! Blank -- missing
    ENDIF
    IF (Fields(21) /= BlankString) THEN
      FieldNo=21
      FieldString='Present Weather Observation'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=121) PresWeathObs
 121  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=21
      endif
    ELSE
      PresWeathObs=9  ! Blank -- missing
    ENDIF
    IF (Fields(22) /= BlankString) THEN
      FieldNo=22
      FieldString='Present Weather Codes'
      PresWeathCodes=Fields(22)
    ELSE
      PresWeathCodes='999999999'  ! Blank -- missing
    ENDIF
    IF (Fields(23) /= BlankString) THEN
      FieldNo=23
      FieldString='Precipitable Water'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=123) PrecWtr
 123  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=23
      endif
    ELSE
      PrecWtr=999  ! Blank -- missing
    ENDIF
    IF (Fields(24) /= BlankString) THEN
      FieldNo=24
      FieldString='Aerosol Optical Depth'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=124) AerOptDepth
 124  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=1
      endif
    ELSE
      AerOptDepth=.999  ! Blank -- missing
    ENDIF
    IF (Fields(25) /= BlankString) THEN
      FieldNo=25
      FieldString='Snow Depth'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=125) SnowDepth
 125  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=25
      endif
    ELSE
      SnowDepth=999  ! Blank -- missing
    ENDIF
    IF (Fields(26) /= BlankString) THEN
      FieldNo=26
      FieldString='Days Since Last Snow'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=126) DaysLastSnow
 126  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=1
      endif
    ELSE
      DaysLastSnow=99  ! Blank -- missing
    ENDIF
    ! Following are defaulted because of transition from old EPW to new.
    IF (Fields(27) /= BlankString) THEN
      FieldNo=27
      FieldString='Albedo'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=127) Albedo
 127  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=1
      endif
    ELSE
      Albedo=.100
    ENDIF
    IF (Fields(28) /= BlankString) THEN
      FieldNo=28
      FieldString='Rain'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=128) Rain
 128  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=28
      endif
    ELSE
      Rain=0.0
    ENDIF
    IF (Fields(29) /= BlankString) THEN
      FieldNo=29
      FieldString='Rain Rate'
      READ(Fields(FieldNo),*,IOSTAT=Ios1,ERR=129) RainRate
 129  if (ios1 /= 0) then
!        write(AuditFile,fmta) ' *** Field in error ('//TRIM(FieldString)//')='//TRIM(Fields(FieldNo))
        IOS=1
      endif
    ELSE
      RainRate=1.0
    ENDIF

  RETURN

END SUBROUTINE ReadAndInterpretEPWWeatherLine

REAL(r64) FUNCTION ProcessNumber(String,ErrorFlag)

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


  ProcessNumber=0.0
  !  Make sure the string has all what we think numerics should have
  PString=ADJUSTL(String)
  StringLen=LEN_TRIM(PString)
  ErrorFlag=.false.
  IF (StringLen == 0) RETURN
  VerNumber=VERIFY(PString(1:StringLen),ValidNumerics)
  IF (VerNumber == 0) THEN
    Read(PString,*,IOSTAT=IoStatus) Temp
    ProcessNumber=Temp
    ErrorFlag=.false.
  ELSE
    ProcessNumber=0.0
    ErrorFlag=.true.
  ENDIF
  IF (IoStatus /= 0) THEN
    ProcessNumber=0.0
    ErrorFlag=.true.
  ENDIF

RETURN

END FUNCTION ProcessNumber

INTEGER FUNCTION FindNonSpace(String)

          ! FUNCTION INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   September 1997
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS FUNCTION:
          ! This function finds the first non-space character in the passed string
          ! and returns that position as the result to the calling program.

          ! METHODOLOGY EMPLOYED:
          ! Scan string for character not equal to blank.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! FUNCTION ARGUMENT DEFINITIONS:
  CHARACTER(len=*), INTENT(IN) :: String  ! String to be scanned

          ! FUNCTION PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! FUNCTION LOCAL VARIABLE DECLARATIONS:
      INTEGER I,ILEN

      FindNonSpace=0
      ILEN=LEN_TRIM(String)
      DO I=1,ILEN
        IF (String(I:I) .NE. ' ') THEN
          FindNonSpace=I
          EXIT
        END IF
      END DO

      RETURN

END FUNCTION FindNonSpace

FUNCTION GetNewUnitNumber ()  RESULT (UnitNumber)

          ! FUNCTION INFORMATION:
          !       AUTHOR         Linda K. Lawrie, adapted from reference
          !       DATE WRITTEN   September 1997
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS FUNCTION:
          ! Returns a unit number of a unit that can exist and is not connected.  Note
          ! this routine does not magically mark that unit number in use.  In order to
          ! have the unit "used", the source code must OPEN the file.

          ! METHODOLOGY EMPLOYED:
          ! Use Inquire function to find out if proposed unit: exists or is opened.
          ! If not, can be used for a new unit number.

          ! REFERENCES:
          ! Copyright (c) 1994 Unicomp, Inc.  All rights reserved.
          !
          ! Developed at Unicomp, Inc.
          !
          ! Permission to use, copy, modify, and distribute this
          ! software is freely granted, provided that this notice
          ! is preserved.

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! FUNCTION ARGUMENT DEFINITIONS:
  INTEGER UnitNumber  ! Result from scanning currently open files

          ! FUNCTION PARAMETER DEFINITIONS:
!  IO Status Values:

  INTEGER, PARAMETER :: END_OF_RECORD = -2
  INTEGER, PARAMETER :: END_OF_FILE = -1

!  Indicate default input and output units:

  INTEGER, PARAMETER :: DEFAULT_INPUT_UNIT = 5
  INTEGER, PARAMETER :: DEFAULT_OUTPUT_UNIT = 6

!  Indicate number and value of preconnected units

  INTEGER, PARAMETER :: NUMBER_OF_PRECONNECTED_UNITS = 2
  INTEGER, PARAMETER :: PRECONNECTED_UNITS (NUMBER_OF_PRECONNECTED_UNITS) = (/ 5, 6 /)

!  Largest allowed unit number (or a large number, if none)
  INTEGER, PARAMETER :: MaxUnitNumber = 1000

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! FUNCTION LOCAL VARIABLE DECLARATIONS:
  LOGICAL :: exists  ! File exists
  LOGICAL :: opened  ! Unit is open
  INTEGER :: ios     ! return value from Inquire intrinsic

  DO UnitNumber = 1, MaxUnitNumber
    IF (UnitNumber == DEFAULT_INPUT_UNIT .or. &
        UnitNumber == DEFAULT_OUTPUT_UNIT) CYCLE
    IF (ANY (UnitNumber == PRECONNECTED_UNITS)) CYCLE
    INQUIRE (UNIT = UnitNumber, EXIST = exists,  OPENED = opened, IOSTAT = ios)
    IF (exists .and. .not. opened .and. ios == 0) RETURN      ! result is set in UnitNumber
  END DO

  UnitNumber = -1

END FUNCTION GetNewUnitNumber

REAL(r64) Function GetSTM(Longitude)
      REAL(r64) Longitude
!D    TITLE:= GetSTM - Get Standard Time Meridian from User Entered
!D                     Longitude
!D    PURPOSE:= Calculate the proper Meridian from Longitude.  This
!D    value is needed for weather calculations so that the sun comes
!D    up and goes down at the right times.
!D----- GetSTM variables ------------------
!D VARIABLE DICTIONARY:=
!D    LONGTD   - argument - Input variable, Longitude entered by user
!D    LONGH  - Upper Longitude value for a Time Zone
!D    LONGL  - Lower Longitude value for a Time Zone
!D    I      - loop variable
!D    TEMP   - temporary value to use to determine TZ
!D    STM     - calculated Standard Time Meridian -- Longitude
!D            .le. longh(STM) .and. gt. longl(STM)
!
      REAL(r64) longl(-12:12),longh(-12:12)

!
      INTEGER I
      REAL(r64) TEMP
      REAL(r64) GetTZ, tz
!
      GetTZ=-999.d0
      longl(0)=-7.5d0
      longh(0)=7.5d0
      do i=1,12
        longl(i)=longl(i-1)+15.d0
        longh(i)=longh(i-1)+15.d0
      enddo
      do i=1,12
        longl(-i)=longl(-i+1)-15.d0
        longh(-i)=longh(-i+1)-15.d0
      enddo
      temp=Longitude
      temp=mod(temp,360.d0)
      if (temp > 180.d0) temp=temp-180.d0
      do i=-12,12
        if (temp .gt. longl(i) .and. temp .le. longh(i)) goto 125
      enddo
!                                   error
      goto 999
 125  tz=i
      tz=mod(tz,24.d0)
      GetSTM=tz
!
 999  RETURN
!
END FUNCTION GetSTM

SUBROUTINE GetLocData(Lat,Long,Elev)

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   April 2002
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This subroutine gives the location data (latitude, longitude, elevation) to the
          ! calling program.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
  REAL(r64), INTENT(OUT) :: Lat
  REAL(r64), INTENT(OUT) :: Long
  REAL(r64), INTENT(OUT) :: Elev

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
          ! na

  Lat=Latitude
  Long=Longitude
  Elev=Elevation

  RETURN

END SUBROUTINE GetLocData

real(r64) FUNCTION Psat(T)
!  Function: Calculates water vapour saturation pressure at temperature T øC
!  Eqns from [Hyland & Wexler 1983] cited in ASHRAE FUNDAMENTALS 6.7
!  Created: P.G.Schild 1999May18
!  IO #  Name      unit             description
!  O  -  Psat      [Pa]             Saturated vapour pressure
!  I  1  T         [deg.C]          Air dry bulb temperature
!  ..........................................................................
      IMPLICIT NONE
      REAL(r64), INTENT(IN) :: T
      REAL(r64) :: dummy
      dummy=T+273.15
      IF (T<0.0) THEN
!       .Saturation vapour pressure over ice [Pa] (-100 to 0 deg.C)
         Psat=EXP(-5.6745359d+03/dummy    &
                  -5.1523058d-01          &
                  -9.6778430d-03*dummy    &
                  +6.2215701d-07*dummy**2 &
                  +2.0747825d-09*dummy**3 &
                  -9.4840240d-13*dummy**4 &
                  +4.1635019    *LOG(dummy))
      ELSE
!       .Saturation vap. press. over liquid water [Pa] (0 to +200 deg.C)
         Psat=EXP(-5.8002206d+03/dummy    &
                  -5.5162560              &
                  -4.8640239d-02*dummy    &
                  +4.1764768d-05*dummy**2 &
                  -1.4452093d-08*dummy**3 &
                  +6.5459673    *LOG(dummy))
      ENDIF
!     Convert kPa to Pa
      Psat=Psat*1000.0
   END FUNCTION Psat

      REAL(r64) FUNCTION SATUTP(P)
      Implicit NONE
      real(r64), intent(in) :: P
!D    TITLE:= SATUTP - CALCULATE SATURATION TEMPERATURE
!D    AUTHOR:=  GEORGE SHIH
!D    DATE WRITTEN:=  MAY 76
!D    PURPOSE:=
!
!     LOCAL VARIABLES
!
      REAL(r64)     T
      REAL(r64)  PP

! 700  FORMAT(10X,'DATE (MM/DD/HH)= ',I2,2('/',I2.2),' PRESSURE= ',F10.2)
!
!D----- SATUTP variables ------------------
!D VARIABLE DICTIONARY:=
!D    P      - argument  - SATURATION PRESSURE (N/M**2)
!D    CNVP   - exfunc
!D    SATUTP - funcval - CALCULATE SATURATION TEMPERATURE
!D    PP     -
!D    T      - SATURATION TEMPERATURE (C)

!                                      CHECK P IN RANGE.
      IF (P <= 1.0813d0 .or. P >= 1.0133d5) THEN
!        IF (NTIMES(11) <= RFREQ) THEN
!          CALL ShowWarningError('Pressure out of range (SATUTP)')
!          WRITE(String,700) Month,DayOfMonth,HourOfDay,P
!          CALL ShowContinueError(String)
!        ENDIF
      ENDIF

      PP= P
!
      IF (P > 2.3366d3) GO TO 20
      IF (P > 1.227d3) GO TO 70
      IF (P > 6.108d2) GO TO 60
      IF (P > 1.0325d2) GO TO 50
      IF (P > 12.842d0) GO TO 40
      GO TO 30
   20 CONTINUE
      IF (P < 4.2415d3) GO TO 80
      IF (P < 7.375d3) GO TO 90
      IF (P < 1.992d4) GO TO 100
      IF (P < 1.0133d5) GO TO 110
      GO TO 120
!
!                                      TEMP. IS FROM -60 C  TO  -40C
   30 CONTINUE
      T=Y5(PP,-67.8912D0,9.21677D0,-1.90385D0,2.35588D-1,-1.48075D-2,3.64517D-4)
      GO TO 130
!                                      THE FOLLOWING EQUATIONS IN SI
!                                      UNITS ARE OBTAINED BY CURVE
!                                      FITTING
!                                      TEMP. IS FROM  -40 C TO  -20 C
   40 CONTINUE
      T=Y7(PP,-5.35428D1,1.59311D0,-5.70202D-2,1.44012D-3,-2.30578D-5,2.22628D-7,-1.17867D-9,2.62131D-12)
      GO TO 130
!                                      TEMP. IS FROM  -20 C TO    0 C
   50 CONTINUE
      T=Y6(PP,-3.59131D1,2.31311D-1,-1.00453D-3,2.99919D-6,-5.38184D-9,5.22567D-12,-2.10354D-15)
      GO TO 130
!                                      TEMP. IS FROM    0 C TO  10 C
   60 CONTINUE
      T=Y3(PP,-19.7816D0,4.46963D-2,-2.36037D-5,5.67281D-9)
      GO TO 130
!                                      TEMP. IS FROM   10 C TO  20 C
   70 CONTINUE
      T=Y3(PP,-11.7426D0,2.4662D-2,-6.66598D-6,8.24255D-10)
      GO TO 130
!                                      TEMP. IS FROM   20 C TO  30 C
   80 CONTINUE
      T=Y3(PP,-3.78423D0,1.42713D-2,-2.07467D-6,1.38642D-10)
      GO TO 130
!                                      TEMP. IS FROM   30 C TO  40 C
   90 CONTINUE
      T=Y3(PP,4.09671D0,8.61614D-3,-7.04051D-7,2.65621D-11)
      GO TO 130
!                                      TEMP. IS FROM   40 C TO   60 C
  100 CONTINUE
      T=Y5(PP,8.65676D0,6.86019D-3,-5.07998D-7,2.57958D-11,-7.28305D-16,8.62156D-21)
      GO TO 130
!                                      TEMP. IS FROM   60 C TO  100 C
  110 CONTINUE
      T=Y6(PP,2.66453D1,2.54217D-3,-6.00185D-8,1.01356D-12,-1.04474D-17,5.88844D-23,-1.38705D-28)
      GO TO 130
!                                      TEMP. IS FROM 100 C TO 140 C
  120 CONTINUE
      T=Y5(PP,5.69919D1,6.37817D-4,-2.85187D-9,8.77453D-15,-1.48739D-20,1.04699D-26)
!
  130 CONTINUE
      SATUTP=T
!
      RETURN

      CONTAINS

      REAL(r64) FUNCTION Y3(X,A0,A1,A2,A3)
      IMPLICIT NONE
      REAL(r64) X
      REAL(r64) A0,A1,A2,A3

      Y3=A0+X*(A1+X*(A2+X*A3))

      RETURN
      END FUNCTION Y3

      REAL(r64) FUNCTION  Y5(X,A0,A1,A2,A3,A4,A5)
      IMPLICIT NONE
      REAL(r64) X
      REAL(r64) A0,A1,A2,A3,A4,A5

      Y5=A0+X*(A1+X*(A2+X*(A3+X*(A4+X*A5))))

      RETURN
      END FUNCTION Y5

      REAL(r64) FUNCTION  Y6(X,A0,A1,A2,A3,A4,A5,A6)
      IMPLICIT NONE
      REAL(r64) X
      REAL(r64) A0,A1,A2,A3,A4,A5,A6

      Y6=A0+X*(A1+X*(A2+X*(A3+X*(A4+X*(A5+X*A6)))))

      RETURN
      END FUNCTION Y6

      REAL(r64) FUNCTION  Y7(X,A0,A1,A2,A3,A4,A5,A6,A7)
      IMPLICIT NONE
      REAL(r64) X
      REAL(r64) A0,A1,A2,A3,A4,A5,A6,A7

      Y7=A0+X*(A1+X*(A2+X*(A3+X*(A4+X*(A5+X*(A6+X*A7))))))

      RETURN
      END FUNCTION Y7

      END FUNCTION SATUTP

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
      REAL(r64)  TT

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
      REAL(r64) FUNCTION Y3(X,A0,A1,A2,A3)
      IMPLICIT NONE
      REAL(r64) X
      REAL(r64) A0,A1,A2,A3
      Y3=A0+X*(A1+X*(A2+X*A3))
      RETURN
      END FUNCTION Y3

      REAL(r64) FUNCTION Y4(X,A0,A1,A2,A3,A4)
      IMPLICIT NONE
      REAL(r64) X
      REAL(r64) A0,A1,A2,A3,A4
      Y4=A0+X*(A1+X*(A2+X*(A3+X*A4)))
      RETURN
      END FUNCTION Y4

      REAL(r64) FUNCTION Y5(X,A0,A1,A2,A3,A4,A5)
      IMPLICIT NONE
      REAL(r64) X
      REAL(r64) A0,A1,A2,A3,A4,A5
      Y5=A0+X*(A1+X*(A2+X*(A3+X*(A4+X*A5))))
      RETURN
      END FUNCTION Y5

END SUBROUTINE DrySatPt

END MODULE EPWRead
