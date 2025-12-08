MODULE SimData
     ! MODULE INFORMATION:
        !       AUTHOR         Edward D. Clements
        !       DATE WRITTEN   October 1999 - April 2001
        !       MODIFIED       na
        !       RE-ENGINEERED  na
        !       VERSION 8.0
        !
        ! PURPOSE OF THIS MODULE:
        ! This module provides the data required by the Three Dimensional Slab On Grade
        ! Heat Flux Modelling program it is a part of

        ! METHODOLOGY EMPLOYED:
        ! This module was converted from legacy code

        ! REFERENCES:
        ! Bahnfleth, W.  1989.  Three-dimensional modelling of heat transfer from slab
        ! floors. Ph.D. diss., University of Illinois at Urbana-Champaign.  Also
        ! published as US Army CERL Technical Manuscript E-89/11.
        !
        ! "Numerical Methods with Numerous Examples and Solved Illustrative Problems"
        ! by Robert W. Hornbeck, Quantum, 1975.

        ! OTHER NOTES:
        ! na

     ! USE STATEMENTS:
     USE DataPrecisionGlobals

IMPLICIT NONE   ! Enforce explicit typing of all variables

PUBLIC

     CHARACTER(len=*), PARAMETER :: ProgramName='Slab'

!*** DERIVED TYPE DEFINITIONS
     TYPE WeatherData     ! Derived Type for Storing Weather Data Locally
       REAL(r64) TDB(24)           ! Hourly Outdoor Air Dry Bulb Temperature             [C]
       REAL(r64) TWB(24)           ! Hourly Outdoor Air Wet Bulb Temperature             [C]
       REAL(r64) PBAR(24)          ! Hourly Outdoor Barometric Pressure                  [C]
       REAL(r64) HRAT(24)          ! Hourly Outdoor Air Humidity Ratio                   [C]
       REAL(r64) WND(24)           ! Hourly Wind Speed (Outdoors)                      [m/s]
       REAL(r64) RBEAM(24)         ! Hourly direct normal solar irradiance           [W/m^2]
       REAL(r64) RDIF(24)          ! Hourly diffuse horizontal solar irradiance      [W/m^2]
       INTEGER ISNW(24)       ! Is there snow this hour? 1=Yes, 0=No                 []
       REAL(r64) DSNOW(24)         ! Hourly snow depth data                             [cm]
     END TYPE WeatherData

     TYPE SiteParameters  ! Derived Type for Site Parameters
       REAL(r64) LONG              ! Site Longitude                                [Degrees]
       REAL(r64) LAT               ! Site Latitude                                 [Degrees]
       REAL(r64) MSTD              ! Standard Time Zone Meridian                   [Degrees]
       REAL(r64) ELEV              ! Site Elevation                                      [m]
     END TYPE SiteParameters

      TYPE ETemp
       REAL(r64) RSKY
       REAL(r64) HHEAT
       REAL(r64) HMASS
       REAL(r64) DODPG
       REAL(r64) TG(0:35)
     END TYPE ETemp

     TYPE Soil_SurfaceProperties  ! Derived Type for Soil and Surface Properties
       INTEGER NumMaterials   ! Number of material types                             []
                              ! (2 currently (1=soil,2=bldg)
       REAL(r64) ALBEDO(2)         ! Surface Albedo Array                                 []
       REAL(r64) EPSLW(2)          ! Surface Emissivity Array                             []
       REAL(r64) Z0(2)             ! Surface Roughness Array                            [cm]
       REAL(r64) HIN(2)             ! Indoor Convective Heat Transfer Coefficient  [W/m^2-K]
     END TYPE Soil_SurfaceProperties

     TYPE BoundaryConds   ! Derived Type for Boundary Conditions
       CHARACTER *5 EVTR      ! Evapotranspiration Simulation Status Flag             []
       CHARACTER *5 FIXBC     ! Fixed Temperature Lower Boundary Condition Flag TRUE = Fixed Temp      []
   !    CHARACTER *5 OLDTG     ! Is there an old temperature file?                     []
                               ! If not, a new one will be generated using a 1-D model
       REAL(r64) TDEEPin           !  User specified lower region boundary temp, C
                              !  Zero input reverts to using the TDEEP from the 1-D ground temp.
       CHARACTER *5  USERHFlag !   If True, a user input heat transfer coefficient
                               !    that follows is used on the ground surface.
       REAL(r64) USERH              !  Heat transfer coeffcient that will be used on ground surf W/m^2C

     END TYPE BoundaryConds

     TYPE InputGridProps  ! Derived Type for Simulation Grid Properties
       INTEGER NX             ! Number of cells in the X direction (N-S)             []
       INTEGER NY             ! Number of cells in the Y direction (E-W)             []
       INTEGER NZ             ! Number of cells in the Z direction (into soil)       []
     END TYPE InputGridProps

     TYPE BldgProps       ! Derived Type for Building Properties
         INTEGER IYRS           ! Maximum Number of years to iterate for soln.         []
       INTEGER Shape          ! Building Shape: 0 For Rectangular Floor Plan         []
                              ! 1 For L-Shape with 1st Quadrant Empty
                              ! 2  "  "   "     "  2nd    "       "
                              ! 3  "  "   "     "  3rd    "       "
                              ! 4  "  "   "     "  4th    "       "
       REAL(r64) HBLDG             ! Building Height                                     [m]
       REAL(r64) TINave(12)        ! Inside Monthly Average Dry bulb temperature    [C]
       REAL(r64) TIN               ! Actual Inside Dry Bulb Temperature             [C]
       REAL(r64) TINAmp            ! Amplitude of sine wave variation of daily TIN  [C]
       INTEGER NumberOfTIN    !  Number of TIN values (either 12 or 1)
       REAL(r64) ConvTol           ! Convergence Tolerance
     END TYPE BldgProps

     TYPE Insulation      ! Derived Type for Building Insulation Properties
       REAL(r64) RINS              ! R value of the horizontal slab insulation       [W/m-C]
       REAL(r64) DINS              ! Strip width of perimeter insulation                 [m]
       REAL(r64) RVINS             ! R value of the foundation wall insulation       [W/m-C]
       REAL(r64) ZVINS             ! Depth of the foundation wall insulation             [m]
       INTEGER IVINS          ! Switch denoting modelling of vertical insulation     []
     END TYPE Insulation

     TYPE BuildingDimensions
       INTEGER IBOX           ! One index that describes the box in which            []
                              ! the building floor lies (e.g.XFACE(IBOX)
                              ! and XFACE(-IBOX) bound X)
       INTEGER JBOX           ! The other index that describes the box in            []
                              ! which the building floor lies (e.g.YFACE(JBOX)
                              ! and YFACE(-JBOX) bound Y)
     END TYPE BuildingDimensions
     TYPE GroundTemp
       REAL(r64) TG(0:35)          ! Array of ground temperatures for initialization      []
     END TYPE GroundTemp

!*** Variables of Specific Types
     REAL(r64) XFACE(-35:35)     ! Array of face coordinates of cells                    [m]
     REAL(r64) YFACE(-35:35)     ! Array of face coordinates of cells                    [m]
     REAL(r64) ZFACE(0:35)       ! Array of face coordinates of cells                    [m]
     REAL(r64) RHO(2)            ! Soil Density                                    [kg/m^3]
     REAL(r64) TCON(2)           ! Soil Thermal Conductivity                         [W/m-K]
     REAL(r64) CP(2)             ! Soil Specific Heat                               [J/kg-K]
     INTEGER COUNT1         ! Dummy counter variable                                 []
     INTEGER COUNT2         ! Dummy counter variable                                 []
     INTEGER COUNT3         ! Dummy counter variable                                 []



!*** VARIABLES ADDED WITH AUTOGRID
     REAL(r64) SLABX             ! X direction dimension of the slab                     [m]
     REAL(r64) SLABY             ! Y direction dimension of the slab                     [m]
     REAL(r64) CLEARANCE         ! Distance between the edge of the slab and domain edge [m]
     REAL(r64) SLABDEPTH         ! Depth of the floor slab (This is typically 0.1m)      [m]
     REAL(r64) ZCLEARANCE        ! DISTANCE TO THE LOWER BOUNDARY OF THE DOMAIN
     REAL(r64) APRatio           ! Area to perimeter ratio for calculating an equivalent [m]
                            ! foundation
     CHARACTER *5 EquivSizing ! Equivalent sizing flag: if equivalent sizing is      []
                              ! to be used in this run

!*** Derived type variables
     TYPE(WeatherData)::TodaysWeather   ! Derived type variable
     TYPE(SiteParameters)::Site         ! Derived type variable
     TYPE(Soil_SurfaceProperties)::SSP  ! Derived type variable
     TYPE(BoundaryConds)::BCS           ! Derived type variable
     TYPE(InputGridProps)::InitGrid     ! Derived type variable
     TYPE(BldgProps)::BuildingData      ! Derived type variable
     TYPE(Insulation)::Insul            ! Derived type variable
     TYPE(BuildingDimensions)::Slab     ! Derived type variable
     TYPE(GroundTemp):: TGround         ! Derived type variable
     TYPE(ETemp), DIMENSION(24,365) :: EarthTemp

     INTEGER, EXTERNAL :: GetNewUnitNumber  ! External  function to "get" a unit number
     INTEGER DebugInfo       ! Dynamic Unit Number
     INTEGER InputEcho       ! Dynamic Unit Number
     INTEGER DailyFlux       ! Dynamic Unit Number
     INTEGER History         ! Dynamic Unit Number
     INTEGER FluxDistn       ! Dynamic Unit Number
     INTEGER TempDistn       ! Dynamic Unit Number
     INTEGER SurfaceTemps    ! Dynamic Unit Number
     INTEGER CLTemps         ! Dynamic Unit Number
     INTEGER SplitSurfTemps  ! Dynamic Unit Number
!     INTEGER EarthTemp       ! Dynamic Unit Number
     INTEGER Weather         ! Dynamic Unit Number
     INTEGER TempInit        ! Dynamic Unit Number
     INTEGER NUMRUNS         ! Number of runs in a batch
     INTEGER RUNNUM          ! Current run number
END MODULE SimData

!****************************************************************************************
!*********************************  END DATA MODULE  ************************************
!****************************************************************************************
