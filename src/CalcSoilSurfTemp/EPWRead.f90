MODULE EPWRead

          ! Module containing the routines dealing with reading the raw weather (source) data

          ! MODULE INFORMATION:
          !       AUTHOR         Linda Lawrie
          !       DATE WRITTEN   March 2001 - February 2005
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
          ! <use statements for data only modules>
          ! na
          ! <use statements for access to subroutines in other modules>
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables

PUBLIC ! Everything private unless explicitly made public

          ! MODULE PARAMETER DEFINITIONS:
  CHARACTER(len=*),  PARAMETER :: AFormat='(A)'
!  CHARACTER(len=*),  PARAMETER :: SkipLineFormat='(/)'
!  CHARACTER(len=26), PARAMETER :: UpperCase='ABCDEFGHIJKLMNOPQRSTUVWXYZ'
!  CHARACTER(len=26), PARAMETER :: LowerCase='abcdefghijklmnopqrstuvwxyz'
!  CHARACTER(len=1),  PARAMETER :: PathChar='\'
!  INTEGER,           PARAMETER :: PathLimit=255
  CHARACTER(len=1),  PARAMETER :: BlankString=' '
!  CHARACTER(len=1),  PARAMETER :: NullString='-'
!  CHARACTER(len=*),  PARAMETER :: BlankLineFmt="(1X)"
!  CHARACTER(len=1),  PARAMETER :: Tab=CHAR(9)
  INTEGER,           PARAMETER :: MaxNameLength=60
  DOUBLE PRECISION,  PARAMETER :: Sigma=5.6697d-8 ! Stefan-Boltzmann constant
  REAL,              PARAMETER :: TKelvin=273.15  ! conversion from Kelvin to Celsius
  DOUBLE PRECISION,  PARAMETER :: PI= 3.141592653589793   ! Pi
!  DOUBLE PRECISION,  PARAMETER :: PiOvr2 = PI/2.          ! Pi/2
!  DOUBLE PRECISION,  PARAMETER :: DegreesToRadians = PI/180.  ! Conversion for Degrees to Radians

          ! DERIVED TYPE DEFINITIONS:
  TYPE WeatherDataDetails
    ! one for each day
    INTEGER Year                  ! Year from source
    INTEGER Month                 ! Month from source
    INTEGER Day                   ! Day from source
    INTEGER DayOfYear             ! Day of Year for this date
    INTEGER IntervalMinute(60)    ! Number of Intervals -- Minutes for each
    CHARACTER(len=50) DataSourceFlags(24,60) ! Combined list of DataSourceFlags
                                  ! taken directly from weather file or "*" for calculated
                                  ! in this program
    REAL DryBulb(24,60)           ! Dry-Bulb Temp (C)
    REAL DewPoint(24,60)          ! Dew-Point Temp (C)
    REAL RelHum(24,60)            ! Relative Humidity (%)
    REAL StnPres(24,60)           ! Station Pressure (Pa)
    REAL xHorzRad(24,60)          ! Extraterrestial Horizontal Radiation (Wh/m2)
    REAL xDirNormRad(24,60)       ! Extraterrestial Direct Normal Radiation (Wh/m2)
    REAL HorzIRSky(24,60)          ! Horizontal Infrared Radiation from Sky (Wh/m2)
    REAL GlobHorzRad(24,60)       ! Global Horizontal Radiation (Wh/m2)
    REAL DirNormRad(24,60)        ! Direct Normal Radiation (Wh/m2)
    REAL DifHorzRad(24,60)        ! Diffuse Horizontal Radiation (Wh/m2)
    REAL GlobHorzIllum(24,60)     ! Global Horizontal Illuminance (lux)
    REAL DirNormIllum(24,60)      ! Direct Normal Illuminance (lux)
    REAL DifHorzIllum(24,60)      ! Diffuse Horizontal Illuminance (lux)
    REAL ZenLum(24,60)            ! Zenith Luminance (CD/m2)
    INTEGER WindDir(24,60)        ! Wind Direction (degrees)
    REAL WindSpd(24,60)           ! Wind Speed (m/s)
    INTEGER TotSkyCvr(24,60)      ! Total Sky Cover
    INTEGER OpaqSkyCvr(24,60)     ! Opaque Sky Cover
    REAL Visibility(24,60)        ! Visibility (km)
    INTEGER Ceiling(24,60)        ! Ceiling Height (m)
    INTEGER PresWthObs(24,60)     ! Present Weather Observations
    CHARACTER(len=9) PresWthCodes(24,60) ! Present Weather Codes
    INTEGER PrecipWater(24,60)    ! Precipitable Water (mm)
    REAL AerOptDepth(24,60)       ! Aerosol Optical Depth
    INTEGER SnowDepth(24,60)      ! Snow Depth (cm)
    INTEGER DaysLastSnow(24,60)   ! Number of Days since last snow
    LOGICAL DeltaDBRange          ! Day has a delta DB bigger than normal
    REAL DeltaChgDB(24,60)        ! Changed intervals
    LOGICAL DeltaDPRange          ! Day has a delta DP bigger than normal
    REAL DeltaChgDP(24,60)        ! Changed intervals
  END TYPE

  TYPE MissingData          ! This Derived type carries the default missing data
                            ! for those data elements that would be best replaced
                            ! with the previous hour's data for missing data.
    REAL DryBulb            ! Dry Bulb Temperature (C)
    REAL DewPoint           ! Dew Point Temperature (C)
    INTEGER RelHumid        ! Relative Humidity (%)
    REAL StnPres            ! Atmospheric Pressure (Pa)
    INTEGER WindDir         ! Wind Direction (deg)
    REAL WindSpd            ! Wind Speed/Velocity (m/s)
    INTEGER TotSkyCvr       ! Total Sky Cover (tenths)
    INTEGER OpaqSkyCvr      ! Opaque Sky Cover (tenths)
    REAL Visibility         ! Visibility (km)
    INTEGER Ceiling         ! Ceiling Height (m)
    INTEGER PrecipWater     ! Precipitable Water (mm)
    REAL AerOptDepth        ! Aerosol Optical Depth
    INTEGER SnowDepth       ! Snow Depth (cm)
    INTEGER DaysLastSnow    ! Number of Days since last snow
  END TYPE

  TYPE MissingDataCounts    ! This Derived type carries the counts of missing data
                            ! items in the weather conversion process.  It will count
                            ! only items that are on the source file -- not those that
                            ! are derived from data on the source file.
                            ! Comments below illustrate the data that is being counted:
    INTEGER xHorzRad         ! Extraterrestrial Horizontal Radiation
    INTEGER xDirNormRad      ! Extraterrestrial Direct Normal Radiation
    INTEGER DirNormRad       ! Direct Normal Radiation
    INTEGER DifHorzRad       ! Diffuse Normal Radiation
    INTEGER DryBulb         ! Dry Bulb Temperature (C)
    INTEGER DewPoint        ! Dew Point Temperature (C)
    INTEGER RelHumid        ! Relative Humidity (%)
    INTEGER StnPres         ! Atmospheric Pressure (Pa)
    INTEGER WindDir         ! Wind Direction (deg)
    INTEGER WindSpd         ! Wind Speed/Velocity (m/s)
    INTEGER TotSkyCvr       ! Total Sky Cover (tenths)
    INTEGER OpaqSkyCvr      ! Opaque Sky Cover (tenths)
    INTEGER Visibility      ! Visibility (km)
    INTEGER Ceiling         ! Ceiling Height (m)
    INTEGER PrecipWater     ! Precipitable Water (mm)
    INTEGER AerOptDepth     ! Aerosol Optical Depth
    INTEGER SnowDepth       ! Snow Depth (cm)
    INTEGER DaysLastSnow    ! Number of Days since last snow
  END TYPE

          ! MODULE VARIABLE DECLARATIONS:
  TYPE (WeatherDataDetails), DIMENSION(366) :: WDay  ! Allocate for each possible day of weather data
  TYPE (MissingData) :: Missing
  TYPE (MissingDataCounts) :: Missed

  INTEGER :: NumDataPeriods=0  ! Number of data periods indicated on weather file
  REAL :: Latitude=0.0     ! Latitude (decimal equivalent) for weather file (N+/S-)
  REAL :: Longitude=0.0    ! Longitude (decimal equivalent) for weather file (W-/E+)
  REAL :: TimeZone=0.0     ! Time Zone (decimal equivalent - hours) GMT {+/-}
  REAL :: Elevation=0.0    ! Elevation of weather file station {meters}
  REAL :: StdBaroPress=0.0 ! Standard Barometric Pressure based on Elevation
  CHARACTER(len=10) :: StnWMO=' '  ! WMO Station Number (from LOCATION Header)
  INTEGER :: NumIntervalsPerHour=0 ! Number of intervals per hour on weather file
  INTEGER :: NumDays=0        ! Number of days on weather file

          ! SUBROUTINE SPECIFICATIONS FOR MODULE:
PUBLIC  ReadEPW
PRIVATE ProcessEPWHeaders
PRIVATE ProcessEPWHeader

CONTAINS

SUBROUTINE ReadEPW(InputFile,ErrorsFound,ErrorMessage)

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

          ! SUBROUTINE PARAMETER DEFINITIONS:
  CHARACTER(len=*), PARAMETER :: WFmt="(1X,A,'[',A,']',A,I2,A,I2,A,I2)"

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
  REAL WMinute
  REAL DryBulb,DewPoint,RelHumid,AtmPress,ExtHorzRad,ExtDirNormRad,IRHoriz,GloHorzRad,  &
       DirNormRad,DifHorzRad,GloHorzIllum,DirNormIllum,DifHorzIllum,ZenithLum, &
       WindDir,WindSpd,  &
       TotSkyCvr,OpqSkyCvr,Visibility,CeilHgt,PresWeathObs,PrecWtr,    &
       AerOptDepth,SnowDepth,DaysLastSnow
  INTEGER I_WindDir
  CHARACTER(len=15) PresWeathCodes
  ! Uncertainty flags
  CHARACTER(len=2) FldIR,Fld1,Fld2,Fld3,Fld4,Fld5,Fld6,Fld7,Fld8,Fld9,Fld10, &
                   Fld11,Fld12,Fld13,Fld14,Fld15,Fld16,Fld17,Fld18,Fld19,Fld20,Fld21
  REAL TDewK
  REAL TDryK
  REAL OSky
  REAL ESky
  INTEGER I_rh
  LOGICAL RelHumidMissing
  REAL Pw,Pws
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

  StdBaroPress=(101.325*(1-2.25577E-05*Elevation)**5.2559)*1000.
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

    READ(InputLine,*,IOSTAT=ios,ERR=905) WYear,WMonth,WDayOfMonth,WHour,WMinute,SourceFlags, &
             DryBulb,DewPoint,RelHumid, &
             AtmPress,ExtHorzRad,ExtDirNormRad,IRHoriz,GloHorzRad,  &
             DirNormRad,DifHorzRad,GloHorzIllum,  &
             DirNormIllum,DifHorzIllum,ZenithLum,WindDir,WindSpd,  &
             TotSkyCvr,OpqSkyCvr,Visibility,CeilHgt,  &
             PresWeathObs,PresWeathCodes,PrecWtr,    &
             AerOptDepth,SnowDepth,DaysLastSnow
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
    READ(SourceFlags,'(22A2)') Fld1,Fld2,Fld3,Fld4,FldIR,Fld5,Fld6,Fld7,Fld8,Fld9,Fld10,  &
                       Fld11,Fld12,Fld13,Fld14,Fld15,Fld16,Fld17,Fld18,Fld19,Fld20,Fld21
    ! Check missing values
    IF (ExtHorzRad >= 9999) THEN
      Missed%xHorzRad=Missed%xHorzRad+1
    ENDIF
    IF (ExtDirNormRad >= 9999) THEN
      Missed%xDirNormRad=Missed%xDirNormRad+1
    ENDIF
    IF (DirNormRad >= 9999) THEN
      Missed%DirNormRad=Missed%DirNormRad+1
    ENDIF
    IF (DifHorzRad >= 9999) THEN
      Missed%DifHorzRad=Missed%DifHorzRad+1
    ENDIF

    IF (DryBulb >= 99.9) THEN
      DryBulb=Missing%DryBulb
      Missed%DryBulb=Missed%DryBulb+1
    ENDIF

    IF (AtmPress >= 999999.) THEN
      AtmPress=Missing%StnPres
      Missed%StnPres=Missed%StnPres+1
    ENDIF

    IF (RelHumid >= 999.) THEN
      RelHumid=Missing%RelHumid
      Missed%RelHumid=Missed%RelHumid+1
      RelHumidMissing=.true.
    ELSE
      RelHumidMissing=.false.
    ENDIF

    IF (DewPoint >= 99.9) THEN   ! Correlate missing Dewpoint to DryBulb, Pressure, Relative Humidity?
      Missed%DewPoint=Missed%DewPoint+1
      IF (RelHumidMissing) THEN
        DewPoint=Missing%DewPoint
        IF (DewPoint > DryBulb) THEN
          DewPoint=DryBulb - 3.
        ENDIF
      ELSE
    !   Calculate DewPoint
        Pws=Psat(DryBulb)                        !Saturation pressure at temperature
        Pw=MAX(RelHumid,1.)*.01*Pws
        DewPoint=SATUTP(Pw)
      ENDIF
    ENDIF

    IF (WindSpd >= 999.) THEN
      WindSpd=Missing%WindSpd
      Missed%WindSpd=Missed%WindSpd+1
    ENDIF

    IF (WindDir >= 999.) THEN
      WindDir=Missing%WindDir
      Missed%WindDir=Missed%WindDir+1
    ENDIF
    I_WindDir=WindDir

    IF (Visibility >= 9999.) THEN
      Visibility=Missing%Visibility
      Missed%Visibility=Missed%Visibility+1
    ENDIF

    IF (AerOptDepth >= .999) THEN
      AerOptDepth=Missing%AerOptDepth
      Missed%AerOptDepth=Missed%AerOptDepth+1
    ENDIF

    IF (TotSkyCvr == 99.) THEN
      TotSkyCvr=Missing%TotSkyCvr
      Missed%TotSkyCvr=Missed%TotSkyCvr+1
    ENDIF
    IF (OpqSkyCvr == 99.) THEN
      OpqSkyCvr=Missing%OpaqSkyCvr
      Missed%OpaqSkyCvr=Missed%OpaqSkyCvr+1
    ENDIF
    IF (CeilHgt >= 99999.) THEN
      CeilHgt=Missing%Ceiling
      Missed%Ceiling=Missed%Ceiling+1
    ENDIF
    IF (PrecWtr >= 999) THEN
      PrecWtr=Missing%PrecipWater
      Missed%PrecipWater=Missed%PrecipWater+1
    ENDIF
    IF (SnowDepth >= 999) THEN
      SnowDepth=Missing%SnowDepth
      Missed%SnowDepth=Missed%SnowDepth+1
    ENDIF
    IF (DaysLastSnow >= 99) THEN
      DaysLastSnow=Missing%DaysLastSnow
      Missed%DaysLastSnow=Missed%DaysLastSnow+1
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
      ESky= (.787 +.764*LOG(TDewK/TKelvin))*(1. + .0224*OSky - 0.0035*(OSky**2) + .00028*(OSky**3))

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

  CLOSE(UnitNo)

!  IF (NumDataPeriods == 1) THEN
!    DataPeriods(1)%StMon=WDay(1)%Month
!    DataPeriods(1)%StDay=WDay(1)%Day
!    DataPeriods(1)%EnMon=WDay(NumDays)%Month
!    DataPeriods(1)%EnDay=WDay(NumDays)%Day
!  ELSE   ! For now, just assume only 1st and last might have been changed
!    DataPeriods(1)%StMon=WDay(1)%Month
!    DataPeriods(1)%StDay=WDay(1)%Day
!    DataPeriods(NumDataPeriods)%EnMon=WDay(NumDays)%Month
!    DataPeriods(NumDataPeriods)%EnDay=WDay(NumDays)%Day
!    WRITE(AuditFile,*) 'Warning ** Multiple Data Period file -- check periods for accuracy'
!  ENDIF

!  EPWDataLine='DATA PERIODS,'
!  CALL CreateEPWDataLine(EPWDataLine)

  RETURN

900 ErrorMessage=' *** Could not open EPW file='//TRIM(InputFile)
    ErrorsFound=.true.
    RETURN

904 ErrorMessage='Severe ** Error during processing EPW header'
    ErrorsFound=.true.
    RETURN

905 write(ErrorMessage,'(A,A,I5)') ' *** Line in error='//TRIM(InputLine), &
                               ' ** Error during processing day #',NumDays
    ErrorsFound=.true.
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
  CHARACTER(len=40) :: Title=' '
  INTEGER :: Count
  CHARACTER(len=6) CharWMO
  INTEGER Pos
  REAL Number
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
!      LocationTitle=ADJUSTL(Title)

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

REAL FUNCTION ProcessNumber(String,ErrorFlag)

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

  REAL Temp
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

real FUNCTION Psat(T)
!  Function: Calculates water vapour saturation pressure at temperature T øC
!  Eqns from [Hyland & Wexler 1983] cited in ASHRAE FUNDAMENTALS 6.7
!  Created: P.G.Schild 1999May18
!  IO #  Name      unit             description
!  O  -  Psat      [Pa]             Saturated vapour pressure
!  I  1  T         [deg.C]          Air dry bulb temperature
!  ..........................................................................
      IMPLICIT NONE
      REAL, INTENT(IN) :: T
      REAL :: dummy
      dummy=T+273.15
      IF (T<0.0) THEN
!       .Saturation vapour pressure over ice [Pa] (-100 to 0 deg.C)
         Psat=EXP(-5.6745359E+03/dummy    &
                  -5.1523058E-01          &
                  -9.6778430E-03*dummy    &
                  +6.2215701E-07*dummy**2 &
                  +2.0747825E-09*dummy**3 &
                  -9.4840240E-13*dummy**4 &
                  +4.1635019    *LOG(dummy))
      ELSE
!       .Saturation vap. press. over liquid water [Pa] (0 to +200 deg.C)
         Psat=EXP(-5.8002206E+03/dummy    &
                  -5.5162560              &
                  -4.8640239E-02*dummy    &
                  +4.1764768E-05*dummy**2 &
                  -1.4452093E-08*dummy**3 &
                  +6.5459673    *LOG(dummy))
      ENDIF
!     Convert kPa to Pa
      Psat=Psat*1000.0
   END FUNCTION Psat

      REAL FUNCTION SATUTP(P)
      Implicit NONE
      real, intent(in) :: P
!D    TITLE:= SATUTP - CALCULATE SATURATION TEMPERATURE
!D    AUTHOR:=  GEORGE SHIH
!D    DATE WRITTEN:=  MAY 76
!D    PURPOSE:=
!
!     LOCAL VARIABLES
!
      REAL     T
      Double Precision  PP

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
      IF (P <= 1.0813 .or. P >= 1.0133E5) THEN
!        IF (NTIMES(11) <= RFREQ) THEN
!          CALL ShowWarningError('Pressure out of range (SATUTP)')
!          WRITE(String,700) Month,DayOfMonth,HourOfDay,P
!          CALL ShowContinueError(String)
!        ENDIF
      ENDIF

      PP= P
!
      IF (P > 2.3366E3) GO TO 20
      IF (P > 1.227E3) GO TO 70
      IF (P > 6.108E2) GO TO 60
      IF (P > 1.0325E2) GO TO 50
      IF (P > 12.842) GO TO 40
      GO TO 30
   20 CONTINUE
      IF (P < 4.2415E3) GO TO 80
      IF (P < 7.375E3) GO TO 90
      IF (P < 1.992E4) GO TO 100
      IF (P < 1.0133E5) GO TO 110
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

      DOUBLE PRECISION FUNCTION Y3(X,A0,A1,A2,A3)
      IMPLICIT NONE
      DOUBLE PRECISION X
      DOUBLE PRECISION A0,A1,A2,A3

      Y3=A0+X*(A1+X*(A2+X*A3))

      RETURN
      END FUNCTION Y3

      DOUBLE PRECISION FUNCTION Y5(X,A0,A1,A2,A3,A4,A5)
      IMPLICIT NONE
      DOUBLE PRECISION X
      DOUBLE PRECISION A0,A1,A2,A3,A4,A5

      Y5=A0+X*(A1+X*(A2+X*(A3+X*(A4+X*A5))))

      RETURN
      END FUNCTION Y5

      DOUBLE PRECISION FUNCTION Y6(X,A0,A1,A2,A3,A4,A5,A6)
      IMPLICIT NONE
      DOUBLE PRECISION X
      DOUBLE PRECISION A0,A1,A2,A3,A4,A5,A6

      Y6=A0+X*(A1+X*(A2+X*(A3+X*(A4+X*(A5+X*A6)))))

      RETURN
      END FUNCTION Y6

      DOUBLE PRECISION FUNCTION Y7(X,A0,A1,A2,A3,A4,A5,A6,A7)
      IMPLICIT NONE
      DOUBLE PRECISION X
      DOUBLE PRECISION A0,A1,A2,A3,A4,A5,A6,A7

      Y7=A0+X*(A1+X*(A2+X*(A3+X*(A4+X*(A5+X*(A6+X*A7))))))

      RETURN
      END FUNCTION Y7

      END FUNCTION SATUTP

END MODULE EPWRead
