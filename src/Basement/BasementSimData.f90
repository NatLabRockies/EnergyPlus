MODULE BasementSimData
! MODULE INFORMATION:
     !       AUTHOR         Edward Clements
     !       DATE WRITTEN   December 6, 1999
     !       MODIFIED       March 9, 2000
     !       RE-ENGINEERED  na
     !       VERSION NUMBER 2.0
     !
     ! PURPOSE OF THIS MODULE:
     ! This module provides the input structure for the Base3D module

     ! METHODOLOGY EMPLOYED:
     ! Standard EnergyPlus module formatting.
     ! Derived Types used wherever applicable.

     ! REFERENCES: BASE 3D Version 1.0, Cynthia A. Cogil. 23 July, 1996.

     ! OTHER NOTES: none

     ! USE STATEMENTS:
     USE DataPrecisionGlobals

     IMPLICIT NONE   ! Enforce explicit typing of all variables

          ! MODULE PARAMETER DEFINITIONS:
     CHARACTER(len=*), PARAMETER, DIMENSION(7) :: MatlTypes=  &
         (/'Foundation Wall',  &
           'Floor Slab     ',  &
           'Ceiling        ',  &
           'Soil           ',  &
           'Gravel         ',  &
           'Wood           ',  &
           'Air            '/)
     REAL(r64), PARAMETER:: SIGMA=5.6697D-8
     REAL(r64), PARAMETER:: VONKAR=0.41d0
     REAL(r64), PARAMETER:: G=9.806d0


          ! DERIVED TYPE DEFINITIONS:
     TYPE WeatherData     ! Derived Type for Storing Weather Data Locally
       REAL(r64) TDB(24)           ! Hourly Outdoor Air Dry Bulb Temperature             [C]
       REAL(r64) TWB(24)           ! Hourly Outdoor Air Wet Bulb Temperature             [C]
       REAL(r64) PBAR(24)          ! Hourly Outdoor Barometric Pressure                  [C]
       REAL(r64) HRAT(24)          ! Hourly Outdoor Air Humidity Ratio                   [C]
       REAL(r64) WND(24)           ! Hourly Wind Speed (Outdoors)                      [m/s]
       REAL(r64) RBEAM(24)         ! Hourly direct normal solar irradiance           [W/m^2]
       REAL(r64) RDIFH(24)         ! Hourly diffuse horizontal solar irradiance      [W/m^2]
       INTEGER ISNW(24)       ! Is there snow this hour? 1=Yes, 0=No                 []
       REAL(r64) DSNOW(24)         ! Hourly snow depth data                             [cm]
     END TYPE WeatherData

     TYPE SiteParameters
       REAL(r64) LONG             ! Site Longitude                                 [Degrees]
       REAL(r64) LAT              ! Site Latitude                                  [Degrees]
       REAL(r64) MSTD             ! Standard Time Zone Meridian                    [Degrees]
       REAL(r64) ELEV             ! Site Elevation                                       [m]
       INTEGER AHH           ! Annual Heating Hours for a given location             []
       INTEGER ACH           ! Annual Cooling Hours for a given location             []
     END TYPE SiteParameters

     TYPE Boundaries
       INTEGER NMAT          ! Number of material types in this simulation           []
       CHARACTER *5 OLDTG    ! Is there an old temperature file?                     []
                             ! If not, a new one will be generated using a 1-D model
       CHARACTER *6 TGNAM    ! Name of the ground temperature file                   []
       CHARACTER *5 TWRITE   ! True indicates that cell temperatures will            []
                             ! be written to a file to be saved for domain
                             ! initialization
       CHARACTER *5 TREAD    ! True indicates that intialized temperatures           []
                             ! will be read from a file
       CHARACTER *5 TINIT    ! Name of initialization temperature file               []
       CHARACTER *5 FIXBC    ! True indicates that a fixed temperature lower         []
                             ! condition will be applied
     END TYPE Boundaries

     TYPE Insulation
       REAL(r64) REXT             ! Thermal resistance of exterior foundation    [K/(W/m^2)]
                             ! wall insulation
       REAL(r64) RINT             ! Thermal resistance of interior foundation    [K/(W/m^2)]
                             ! wall insulation
       CHARACTER *5 INSFULL  ! T/F: If insulated on the exterior, does               []
                             ! insulation extend the full length of the
                             ! foundation wall?
       REAL(r64) RSID             ! Thermal resistance of above grade exterior   [K/(W/m^2)]
                             ! siding
       REAL(r64) RSILL            ! Thermal resistance of sill box insulation    [K/(W/m^2)]
                             ! (For interior insulation case)
       REAL(r64) RCEIL            ! Thermal resistance of ceiling insulation     [K/(W/m^2)]
       CHARACTER *5 RSNOW    ! True if snow cover model is on                        []
     END TYPE Insulation

     TYPE SurfaceProperties
       REAL(r64) ALBEDO(2)        ! Surface solar albedo array 1)No Snow 2)Snow           []
       REAL(r64) EPSLN(2)         ! Surface emissivity array 1)No snow 2)Snow             []
       REAL(r64) VEGHT(2)         ! Vegetation height 1)No snow 2)Snow                  [cm]
       CHARACTER *5 PET      ! True indicates potential evapotranspiration on        []
     END TYPE SurfaceProperties

     TYPE BuildingParameters
       REAL(r64) DWALL            ! Thickness of foundation wall                         [m]
       REAL(r64) DSLAB            ! Thickness of floor slab                              [m]
       REAL(r64) DGRAVXY          ! Thickness of gravel in positive X and Y              [m]
                             ! directions from the wall
       REAL(r64) DGRAVZN          ! Thickness of gravel in negative Z direction          [m]
                             ! from the top of floor slab
       REAL(r64) DGRAVZP          ! Thickness of gravel in positive Z direction          [m]
                             ! from beneath slab
     END TYPE BuildingParameters

     TYPE InteriorParameters
       CHARACTER *5 COND     ! True if the basement is conditioned mechanically      []
       REAL(r64) TIN(2)           ! Indoor set point temperature in either               [C]
                             ! heating or cooling mode
       REAL(r64) HIN(6)           ! Inside heat transfer coefficients              [W/m^2/K]
                             ! Convection Only
                             ! 1)Q Downward 2)Q Upward 3)Q Horizontal
                             ! Convection and Radiation
                             ! 4)Q Downward 5)Q Upward 6)Q Horizontal
     END TYPE InteriorParameters

     TYPE SimulationParameters
       REAL(r64) F                ! F-factor for modified ADI solution method            []
       INTEGER IYRS          ! Maximum number of years to iterate for 3D            []
       REAL(r64) TSTEP            ! Time Step                                         [Hrs]
     END TYPE SimulationParameters


          ! MODULE VARIABLE DECLARATIONS:
     CHARACTER *8 WeatherFile     ! Name of the weather file for this run           []
     CHARACTER *7 SNOW            ! Name of the TMY2 file for snow info             []
     CHARACTER *5 EquivSizing     ! Equivalent sizing flag: if equivalent sizing is []
                                  ! to be used in this run
     CHARACTER *5 AUTOGRID
     CHARACTER *5 Eplus
     CHARACTER *5 ComBldg
     CHARACTER *7 EPWFile         ! Energy Plus format weather file name            []


     REAL(r64) RHO(7)              ! Material Density array                        [kg/m^3]
     REAL(r64) CP(7)               ! Material Specific heat array                  [J/kg-K]
     REAL(r64) TCON(7)             ! Material thermal conductivity array            [W/m-K]

     REAL(r64) SLABX               ! X Dimension of the floor slab                      [m]
     REAL(r64) SLABY               ! Y Dimension of the floor slab                      [m]
     REAL(r64) SlabDepth           ! Thickness of the floor slab                        [m]
     REAL(r64) ConcAGHeight        ! Height of the basement wall above grade            [m]
     REAL(r64) BaseDepth           ! Depth of the basement wall below grade             [m]
     REAL(r64) APRatio             ! Area to perimeter ratio for calculating an         [m]
                              ! equivalent foundation
     REAL(r64) XFACE(0:100)        ! Array of X direction cell face coordinates         [m]
     REAL(r64) YFACE(0:100)        ! Array of Y direction cell face coordinates         [m]
     REAL(r64) ZFACE(-35:100)      ! Array of finalized Z dir. cell face coordinates    [m]
     REAL(r64) ZFACEINIT(-35:100)  ! Array of initial Z direction cell face coordinates [m]
     REAL(r64) TBasement           ! Basement temperature (Commercial cases)   [C]
     REAL(r64) TBasementAve(12)    ! Monthly average basement temp
     REAL(r64) TBasementDailyAmp   ! Daily amplitude of sine wave added to TBasement
     REAL(r64) TDeadBandUp         ! Upper bound of the conditioning sys dead band zone [C]
     REAL(r64) TDeadBandLow        ! Lower bound of the conditioning sys dead band zone [C]

     INTEGER CLEARANCE        ! Distance from the foundation edge to domain edge   [m]
     INTEGER COUNT1           ! Dummy counter variable                              []
     INTEGER COUNT2           ! Dummy counter variable                              []
     INTEGER COUNT3           ! Dummy counter variable                              []
     INTEGER NDIM(12)         ! Number of days in each month                        []
     INTEGER NFDM(12)         ! Number of the first day of each month               []
     INTEGER NX               ! Number of cells in the X direction                  []
     INTEGER NY               ! Number of cells in the Y direction                  []
     INTEGER NZAG             ! Number of cells above grade in the Z direction      []
     INTEGER NZBG             ! Number of cells below grade in the Z direction      []
     INTEGER NZ1              ! Number of cells between grade & the top of the slab []
     INTEGER IBASE            ! Cell index of the inside wall face, X direction     []
     INTEGER JBASE            ! Cell index of the inside wall face, Y direction     []
     INTEGER KBASE            ! Cell index of the top of the floor slab             []
     INTEGER NUM              ! Batching counter
     INTEGER IDAY             ! Day counter number moved to BasementSimData
!*** IO File Unit Numbers
     INTEGER, EXTERNAL :: GetNewUnitNumber  ! External  function to "get" a unit number
     INTEGER Weather,SolarFile,InputEcho,GroundTemp,QHouseFile,DOUT,DYFLX,YTDBFile, &
     &       InitT,LOADFile,Ceil121,Flor121,RMJS121,RMJW121,WALS121,WALW121,        &
     &       SILS121,SILW121,CeilD21,FlorD21,RMJSD21,RMJWD21,WALSD21,WALWD21,       &
     &       XZYZero,XZYHalf,XZYFull,XZWallTs,YZWallTs,FloorTs,XZWallSplit,         &
     &       YZWallSplit,FloorSplit,Centerline,SILSD21,SILWD21,Weather2,AvgTG,      &
     &       Debugoutfile,floorflux,xzwallflux,yzwallflux,EPMonthly,EPObjects

     DATA     NDIM/31,28,31,30,31,30,31,31,30,31,30,31/,                 &
            & NFDM/1,32,60,91,121,152,182,213,244,274,305,335/


     TYPE(WeatherData)::TodaysWeather           ! Derived type variable
     TYPE(WeatherData), DIMENSION(365) :: FullYearWeather
     TYPE(SiteParameters)::SiteInfo             ! Derived type variable
     TYPE(Boundaries)::BCS                      ! Derived type variable
     TYPE(Insulation)::Insul=Insulation(.00001d0,.00001d0,'F',.00001d0,.00001d0,.00001d0,'F')  ! Insulation
     TYPE(SurfaceProperties)::SP                ! Derived type variable
     TYPE(BuildingParameters)::BuildingData     ! Derived type variable
     TYPE(InteriorParameters)::Interior         ! Derived type variable
     TYPE(SimulationParameters)::SimParams      ! Derived type variable
END MODULE BasementSimData
