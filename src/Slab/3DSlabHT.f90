
!****************************************************************************************
!*******************************  BEGIN PROGRAM MODULE  *********************************
!****************************************************************************************

MODULE EPlusSlab3D
USE DataPrecisionGlobals
USE DataGlobals
USE SimData
USE InputProcessor
IMPLICIT NONE
! MODULE INFORMATION:
        !       AUTHOR         Edward D. Clements
        !       DATE WRITTEN   October 1999 - August 2000
        !       MODIFIED       na
        !       MODIFIED       na
        !       RE-ENGINEERED  na
        !       VERSION        8.0
        !
        ! PURPOSE OF THIS MODULE:
        ! This Program calculates the hourly, three-dimensional temperature
        ! distribution beneath a slab on grade building by the explicit method.

        ! METHODOLOGY EMPLOYED:
        ! This module was converted from legacy code which used an explicit numerical
        ! solution of a finite difference model of the Heat Diffusion Equation.

        ! REFERENCES:
        ! Bahnfleth, W.  1989.  Three-dimensional modelling of heat transfer from slab
        ! floors. Ph.D. diss., University of Illinois at Urbana-Champaign. Also
        ! published as US Army CERL Technical Manuscript E-89/11.
        !
        ! "Numerical Methods with Numerous Examples and Solved Illustrative Problems"
        ! by Robert W. Hornbeck, Quantum, 1975.

        ! OTHER NOTES:
        ! Commenting Convention:
        !** Titles and major headings
        !*** General comments

!** CALL THE MAIN PROGRAM SIMULATION CONTROLLER
CONTAINS
SUBROUTINE Driver
IMPLICIT NONE
    CALL MainSimControl
    RETURN
END SUBROUTINE Driver

!*******************************  PRIMARY SIMULATION CONTROL  ***************************
SUBROUTINE MainSimControl
USE SimData
USE InputProcessor
USE EPWRead, ONLY: GetLocData,GetSTM
IMPLICIT NONE
! PROGRAM INFORMATION:
        !       AUTHOR         Edward D. Clements
        !       DATE WRITTEN   October 20, 1999 - June 5, 2000
        !       MODIFIED       June 13, 2000
        !       MODIFIED       na
        !       RE-ENGINEERED  November 4, 1999
        !       VERSION        10.0

        ! PURPOSE OF THIS MODULE:
        ! This Program calculates the hourly, three-dimensional temperature
        ! distribution beneath a slab on grade building by the explicit method.

        ! METHODOLOGY EMPLOYED:
        ! This module was converted from legacy code which used an explicit numerical
        ! solution of a finite difference model of the Heat Diffusion Equation.

        ! REFERENCES:
        ! Bahnfleth, W.  1989.  Three-dimensional modelling of heat transfer from slab
        ! floors. Ph.D. diss., University of Illinois at Urbana-Champaign. Also
        ! published as US Army CERL Technical Manuscript E-89/11.
        !
        ! "Numerical Methods with Numerous Examples and Solved Illustrative Problems"
        ! by Robert W. Hornbeck, Quantum, 1975.

        ! OTHER NOTES:
        ! Commenting Convention:
        !** Titles and major headings
        !*** General comments

!*** Variables Declarations
     LOGICAL CVG1D            ! Has the ground temperature profile converged?         []
     LOGICAL SYM              ! Can symmetry be used in this solution? T/F            []
     LOGICAL CONVERGE         ! Has the 3D solution converged? T/F                    []
     LOGICAL QUIT             ! Should the program quit? T/F                          []

     CHARACTER *5 OLDTG       ! Is there an old temperature file?                     []
     CHARACTER *5 TGNAM       ! Name of the ground temperature output file            []
     CHARACTER *5 RUNID       ! Run identifier for this set                           []
     CHARACTER *6 WeatherFile ! Name of the BLAST weather file for this run           []

     INTEGER NX               ! Number of cells in the X direction (from -NX to NX)   []
     INTEGER NY               ! Number of cells in the Y direction (from -NY to NY)   []
     INTEGER NZ               ! Number of cells in the Z direction (from 0 to NZ)     []
     INTEGER INS(-35:35,-35:35)    ! Array of slab insulation indicators 1=insulated  []
     INTEGER MSURF(-35:35,-35:35)  ! Array of surface material types 1=building       []
     INTEGER MTYPE(-35:35,-35:35,0:35)  ! Array of 3D material types 1=building       []
     INTEGER NXMIN            ! X boundary for calculations (allows use of symmetry)  []
     INTEGER NYMIN            ! Y boundary for calculations (allows use of symmetry)  []
     INTEGER NXM1             ! NX-1                                                  []
     INTEGER NYM1             ! NY-1                                                  []
     INTEGER NZM1             ! NZ-1                                                  []
     INTEGER IYR              ! Counter variable for years                            []
     INTEGER MAXITER          ! Maximum number of iterations allowed for convergence  []
     INTEGER IMON             ! Counter variable for months                           []
     INTEGER IDAY             ! Counter variable for days                             []
     INTEGER IHR              ! Counter variable for hours                            []
     INTEGER NDIM(12)         ! Number of days in each month                          []
     INTEGER NFDM(12)         ! Number of the first day of each month                 []
     DATA    NDIM/31,28,31,30,31,30,31,31,30,31,30,31/,                       &
      &      NFDM/1,32,60,91,121,152,182,213,244,274,305,335/
     INTEGER EDGEX(-35:35)    ! Array of edge indicator variables for edge insulation []
     INTEGER EDGEY(-35:35)    ! Array of edge indicator variables for edge insulation []

     REAL(r64) SMULT                    ! Symmetry multiplier. 1=No symmetry, 4=symmetry   []
     REAL(r64) TG(0:35)                 ! 1D ground temperature profile                   [C]
     REAL(r64) THETAZ(365,24)           ! Array of solar zenith angles              [Radians]
     REAL(r64) COSTHETAZ(365,24)        ! Array of cosine(solar zenith angles)      [       ]
     REAL(r64) CXP(-35:35,-35:35,0:35)  ! Array of FDM interface coefficients (+X)  [       ]
     REAL(r64) CYP(-35:35,-35:35,0:35)  ! Array of FDM interface coefficients (+Y)  [       ]
     REAL(r64) CZP(-35:35,-35:35,0:35)  ! Array of FDM interface coefficients (+Z)  [       ]
     REAL(r64) CXM(-35:35,-35:35,0:35)  ! Array of FDM interface coefficients (-X)  [       ]
     REAL(r64) CYM(-35:35,-35:35,0:35)  ! Array of FDM interface coefficients (-Y)  [       ]
     REAL(r64) CZM(-35:35,-35:35,0:35)  ! Array of FDM interface coefficients (-Z)  [       ]
     REAL(r64) GOFT(-35:35,-35:35,2)    ! Array of surface heat fluxes of exterior cells
                                   ! GOFT(xx,xx,1) = Current hour value         [W/m**2]
                                   ! GOFT(xx,xx,2) = Previous hour value        [W/m**2]
     REAL(r64) TCVG(-35:35,-35:35,0:35) ! Converged ground temperature profile            [C]
     REAL(r64) DA(-35:35,-35:35)        ! Area of each surface cell                    [m**2]
     REAL(r64) PERIM                    ! Slab perimeter                                  [m]
     REAL(r64) AFLOR                    ! Floor Area                                   [m**2]
     REAL(r64) T(-35:35,-35:35,0:35)    ! 3D Temperature field                            [C]
     REAL(r64) DX(-35:35)               ! Array of cell dimensions                        [m]
     REAL(r64) DY(-35:35)               ! Array of cell dimensions                        [m]
     REAL(r64) DZ(0:35)                 ! Array of cell dimensions                        [m]
     REAL(r64) DXP(-35:35)              ! Array of distances between cell centers         [m]
     REAL(r64) DYP(-35:35)              ! Array of distances between cell centers         [m]
     REAL(r64) DZP(0:35)                ! Array of distances between cell centers         [m]
     REAL(r64) XC(-35:35)               ! Array of cell center coordinates                [m]
     REAL(r64) YC(-35:35)               ! Array of cell center coordinates                [m]
     REAL(r64) ZC(0:35)                 ! Array of cell center coordinates                [m]
     REAL(r64) ATOT                     ! Total domain surface area                    [m**2]
     REAL(r64) RSKY                     ! Incomming infrared sky radiation           [W/m**2]
     REAL(r64) HHEAT                    ! Convective heat transfer coefficient     [W/m**2/K]
     REAL(r64) HMASS                    ! Convective mass transfer coefficient    [kg/m**2/K]
     REAL(r64) DODPG                    ! Delta over Delta Plus Gamma                      []
                                   ! Delta is the change in saturation pressure
                                   ! with temperature and gamma is the "psychrometer
                                   ! constant" (the change in vapor pressure)
     REAL(r64) TMXA                     ! Maximum space averaged floor temp for a day      []
     REAL(r64) TMNA                     ! Minimum space averaged floor temp for a day      []
     REAL(r64) QS(-35:35,-35:35)        ! Daily time averaged heat flux              [W/m**2]
     REAL(r64) TS(-35:35,-35:35)        ! Daily time averaged temperature                 [C]
     REAL(r64) TV(-35:35,0:35)          ! Daily time avg'd temperature in the N/S plane   [C]
     REAL(r64) XDIF                     ! X diffusion term of the heat diffusion eqn [      ]
     REAL(r64) YDIF                     ! Y diffusion term of the heat diffusion eqn [      ]
     REAL(r64) ZDIF                     ! Z diffusion term of the heat diffusion eqn [      ]
     REAL(r64) TOLD(-35:35,-35:35,0:35) ! Array of old temperature field values           [C]
     REAL(r64) ALB                      ! Snow-adjusted surface albedo                     []
     REAL(r64) EPS                      ! Snow-adjusted surface emissivity                 []
     REAL(r64) RDO                      ! Previous hour diffuse radiation value      [W/m**2]
     REAL(r64) RBO                      ! Previous hour beam radiation value         [W/m**2]
     REAL(r64) TDMX                     ! Maximum daily outdoor dry bulb temperature      [C]
     REAL(r64) TDMN                     ! Minimum daily outdoor dry bulb temperature      [C]
     REAL(r64) TDBA                     ! Average daily outdoor dry bulb temperature      [C]
     REAL(r64) QMX                      ! Daily Maximum floor heat flux              [W/m**2]
     REAL(r64) QMN                      ! Daily Minimum floor heat flux              [W/m**2]
     REAL(r64) QBAR                     ! Time and Space Avgd Daily floor heat flux  [W/m**2]
     REAL(r64) QMXA                     ! Daily Maximum Space avgd floor heat flux   [W/m**2]
     REAL(r64) QMNA                     ! Daily Minimum space avgd floor heat flux   [W/m**2]
     REAL(r64) TMX                      ! Maximum floor temperature for a day             [C]
     REAL(r64) TMN                      ! Minimum floor temperature for a day             [C]
     REAL(r64) TBAR                     ! Time and space avgd floor temp for a day        [C]

!*** VARIABLES ADDED FOR BATCHING
     INTEGER COUNTER               ! Dummy counter for batching                       []


!*** VARIABLES ADDED FOR THE SLAB OUTSIDE SURFACE TEMPERATURE CALCULATION
     CHARACTER *5 EPlus            ! Will this set be ouputting data for EnergyPlus?  []
     REAL(r64) TSurfFloor               ! Area weighted slab outside surface temp         [C]
     REAL(r64) TSFYCL                   ! Average temperature on the centerline of the    [C]
                                   ! slab (Y axis)
     REAL(r64) TSFXCL                   ! Average temperature on the centerline of the    [C]
                                   ! slab (X axis)
     INTEGER IBOX                  ! One index that describes the box in which        []
                                   ! the building floor lies (e.g.XFACE(IBOX)
                                   ! and XFACE(-IBOX) bound X)
     INTEGER JBOX                  ! The other index that describes the box in        []
                                   ! which the building floor lies (e.g.YFACE(JBOX)
                                   ! and YFACE(-JBOX) bound Y)
     REAL(r64) TSurfFloorPerim          ! Perimeter outside surface area weighted avg T   [C]
     REAL(r64) TSurfFloorCore           ! Core outside surface area weighted average T    [C]
     REAL(r64) CoreArea                 ! Core zone floor area                         [m**2]
     REAL(r64) PerimArea                ! Perimeter zone floor area                    [m**2]
     INTEGER PerimIndex(-35:35,-35:35) ! Cell index array for perimeter zone cells    []
     CHARACTER *5 AUTOGRID         ! Will the grid be sized automatically?
     CHARACTER *5 EPWFile          ! Name of the EnergyPlus weather file to be linked []
     CHARACTER *1 EPGEOM           ! Will the slab geometry be taken from the EnergyPlus
                                   ! input file? Y/N                                  []
     INTEGER I,J
     REAL(r64) AverageTIN               ! Average Value of TIN for current month
     REAL(r64) HourlyTIN                !  Hourly value of TIN used in calculation
     LOGICAL :: ErrorsFound=.false.
     LOGICAL :: recalc=.false.

!*** GET THE NUMBER OF RUNS FOR BATCHING
     CALL ProcessInput('SlabGHT.idd','GHTIn.idf')
     NUMRUNS= 1 !GetNumObjectsFound('Location')
     PRINT *,'Begin Ground Temp Calculations' !There is/are: ',NUMRUNS,' run/s in this batch'
     EarthTemp%RSKY=0.0
     EarthTemp%HHEAT=0.0
     EarthTemp%HMASS=0.0
     EarthTemp%DODPG=0.0
     DO I=1,365
       DO J=1,24
         EarthTemp(J,I)%TG=0.0
       ENDDO
     ENDDO

     TSurfFloorPerim=0.0d0
     TSurfFloorCore=0.0d0
     CoreArea=0.0d0
     PerimArea=0.0d0

!*** Batching loop
     DO COUNTER=1,NUMRUNS
       RUNNUM=RUNNUM+1
       CONVERGE=.FALSE.
       QUIT=.FALSE.

!*** Get the input data from the user input file
       CALL SetDefaults(AUTOGRID,EPlus,RUNID,WeatherFile,TGNAM,EPWFile,EPGEOM,ErrorsFound)

!*** Connect all IO files  (ADDED 5/30/00)
       CALL ConnectIO(RUNID,WeatherFile,EPlus)

!*** Parse the EnergyPlus Weather File and create an annual TMY database
       IF(.not. SameString(EPlus,'FALSE')) CALL WeatherServer(WeatherFile,EPWFile)

!*** Get the rest of the input data from the user input file
       CALL  GetInput2(AUTOGRID,WeatherFile,TGNAM,MAXITER,EPlus,NDIM,NX,NY,NZ,RUNID,EPGEOM,ErrorsFound)

       TG=TGround%TG
!       OLDTG=BCS%OLDTG  !  Always generate a new TG with EnergyPlus
       NX=InitGrid%NX
       NY=InitGrid%NY
       NZ=InitGrid%NZ
       IBOX=Slab%IBOX
       JBOX=Slab%JBOX

       NXM1=NX-1
       NYM1=NY-1
       NZM1=NZ-1


!*** Set up the EnergyPlus output files
       CALL EPlusHeader
       CALL GetLocData(Site%Lat,Site%Long,Site%Elev)
       Site%Long=-Site%Long
       Site%MSTD=GetSTM(Site%Long)*15.

!*** Calculate some basic Cell Geometry Factors
       CALL CellGeom(NX,NXM1,NY,NYM1,NZ,NZM1,XC,YC,ZC,DX,DY,DZ,DXP,DYP,DZP)

!*** Calculte the surface materials properties field (i.e. define the slab)
       CALL DefineSlab(NX,NXM1,NY,NYM1,NZM1,XC,YC,DX,DY,AFLOR,          &
            & DA,ATOT,PERIM,MSURF,MTYPE)

!*** Determine which cells have horizontal insulation (if any)
       CALL DefineInsulation(NX,NXM1,NY,NYM1,MSURF,XC,YC,INS)

!*** Pre-calculate some finite difference constants
       CALL CalculateFEMCoeffs(INS,NX,NXM1,NY,NYM1,NZM1,MTYPE,DXP,DX,   &
            & DYP,DY,DZP,DZ,CXM,CXP,CYM,CYP,CZM,CZP,EDGEX,EDGEY)

!*** Calculate the sun's zenith angle array for 365 days, 24 hours
!*** Note that if the sun is not up, Thetaz=90
       CALL CalcZenith(THETAZ,COSTHETAZ,WeatherFile)

!*** Write a mirror of the input information
       CALL PrelimOutput(RUNID,WeatherFile,AFLOR,PERIM,NX,NY,NZ,MAXITER)

!*** If there is an old boundary condition file for this problem,
!*** connect it and close the input file. Otherwise, read the initial
!*** condition and create a new file with a 1-D model.
!       EarthTemp=GetNewUnitNumber()
!       IF(OLDTG.EQ.'TRUE') THEN
!         OPEN(UNIT=EarthTemp,FILE=TGNAM//'.TXT', STATUS='OLD')
!       ELSE
         TG=TGround%TG
!         OPEN(UNIT=EarthTemp,FILE=TGNAM//'.TXT', STATUS='REPLACE')
         CALL CalcTearth(TG,COSTHETAZ,NZ,DZ,DZP,CVG1D,recalc)
         IF(.NOT. CVG1D) THEN
           IF (recalc) THEN
             CALL CalcTearth(TG,COSTHETAZ,NZ,DZ,DZP,CVG1D,recalc)
           ENDIF
           IF (.not. CVG1D) THEN
             STOP
           ENDIF
         END IF
 !      END IF

!*** Initialize the temperatures in the 3-D domain.
!*** T(X,Y,Z)=TG(Z)(i.e., equal to the boundary condition)
       CALL Initialize3D(NX,NY,NXM1,NYM1,NZM1,DZP,T,TCVG,GOFT)

!*** Check to see if symmetry can be used
       CALL SymCheck(NXMIN,NYMIN,SYM,SMULT)

       PRINT *,'Entering Main Computational Block'


!*** Loop through the main portion of the solution
       DO 2001 IYR=1,MAXITER
         Print *, 'Working on year ',IYR
         IF(CONVERGE) QUIT=.TRUE.
         !CALL SkipHeader
!         REWIND(Weather)
!         REWIND(EarthTemp)   ! Rewind the Ground Temperature Calculation file.
         IMON=1
         DO 2010 IDAY=1,365
!           PRINT *,COUNTER,IYR,IDAY,CONVERGE
           IF(IDAY.EQ.NFDM(IMON)+NDIM(IMON)) IMON=IMON+1
           IF((CONVERGE.OR.IYR.EQ.MAXITER).AND.SameString(EPlus,'FALSE')) THEN
             CALL InitOutVars(NX,NY,NXM1,NYM1,NZM1,TS,QS,TV,TMNA,TMXA,      &
                  & TBAR,TMN,TMX,QMNA,QMXA,QBAR,QMN,QMX,TDBA,TDMN,TDMX)
           END IF
           CALL GetWeather(IDAY)
           DO 3000 IHR=1,24
             RSKY=EarthTemp(IHR,IDAY)%RSKY
             HHEAT=EarthTemp(IHR,IDAY)%HHEAT
             HMASS=EarthTemp(IHR,IDAY)%HMASS
             DODPG=EarthTemp(IHR,IDAY)%DODPG
             TG=EarthTemp(IHR,IDAY)%TG
             IF (BuildingData%NumberOfTIN > 1) Then   !  Set up changing TIN
                AverageTIN=BuildingData%TINave(IMON)  !  Average TIN for this month
                HourlyTIN = AverageTIN + BuildingData%TINAmp*sin(6.28138*IHR/24)
                BuildingData%TIN=HourlyTIN
             ELSE
                BuildingData%TIN=BuildingData%TINave(1)
                AverageTIN = BuildingData%TIN
             EndIF

!             READ (EarthTemp,*) RSKY,HHEAT,HMASS,DODPG,(TG(COUNT1),COUNT1=0,NZ)
             CALL SetCurrentSurfaceProps(IHR,RBO,RDO)

             CALL SetOldBeamDiffRad(IHR,EPS,ALB)
             CALL CalcCurrentHeatFlux(NXMIN,NXM1,NYMIN,NYM1,MSURF,T,ALB,    &
                  & IHR,RBO,COSTHETAZ,IDAY,RDO,EPS,RSKY,HHEAT,HMASS,DODPG,SYM,GOFT)
             CALL CalcSolutionAndUpdate(NXMIN,NYMIN,NX,NY,NZ,NXM1,NYM1,NZM1,DX,DY,DZ,  &
                        & CXP,CYP,CZP,CXM,CYM,CZM,MTYPE,TG,TOLD,GOFT,T,     &
                        & XDIF,YDIF,ZDIF,SYM)
!*** OUTPUT STAGE
             IF(CONVERGE.OR.IYR.EQ.MAXITER)THEN
!*** CALCULATE THE OUTPUT STATISTICS
               IF(SameString(EPlus,'FALSE'))THEN
                 CALL CalcOutputStats(DX,DY,IHR,NX,NY,NXM1,NYM1,NZM1,NXMIN, &
                 & NYMIN,MSURF,T,GOFT,SMULT,DA,AFLOR,IDAY,IMON,TMN,TMX,     &
                 & QMN,QMX,TMNA,TMXA,QMNA,QMXA,TDMN,TDMX,TBAR,QBAR,TDBA,    &
                 & TV,TS,QS,XC,YC,TG)
               ELSE

!*** IF THIS IS AN ENERGY PLUS RUN, CALCULATE AREA WEIGHTED AVERAGE SURFACE
!*** TEMPERATURES AND HEAT FLUXES FOR OUTPUT
                 CALL SurfTemps(DX,DY,DZ,INS,MTYPE,IBOX,JBOX,T,TSurfFloor,  &
                 &    TSFXCL,TSFYCL,TSurfFloorPerim,TSurfFloorCore,         &
                 &    PerimIndex,CoreArea,PerimArea,AFLOR,XC,YC)
                 If (BuildingData%TINAmp > .001d0) Then    !  Only output hourly info if daily sine wave present
                   CALL EPlusOutput(TSurfFloor,TSFXCL,TSFYCL,TSurfFloorPerim, &
                    &    TSurfFloorCore,CoreArea,PerimArea,IDAY,IHR,HourlyTIN)
                 End If
                 CALL MonthlyEPlusOutput(TSurfFloor,TSFXCL,TSFYCL,TSurfFloorPerim, &
                 &    TSurfFloorCore,CoreArea,PerimArea,IDAY,IHR,AverageTIN,ZFACE(1),TCON(1),SSP%HIN)
               END IF
             END IF
3000       CONTINUE

!*** BYPASS THE DAILY OUTPUT STAGE FOR ALL ENERGY PLUS RUNS

           IF((CONVERGE.OR.IYR.EQ.MAXITER).AND.SameString(EPlus,'FALSE')) THEN
             CALL WriteDailyData(IDAY,IMON,NX,NXM1,NY,NYM1,NZM1,TMN,TMX,    &
                  & TMNA,TMXA,TBAR,TDMN,TDMX,TDBA,QMN,QMX,QMNA,QMXA,QBAR,   &
                  & XC,YC,ZC,TS,QS,TV)
           END IF
2010     CONTINUE
!*** Test for convergence at the end of the year
         CONVERGE=.TRUE.
         DO 650 COUNT3=0,NZM1
           IF (.not. CONVERGE) EXIT
           DO 649 COUNT1=-NX,NXM1
             IF (.not. CONVERGE) EXIT
             DO 648 COUNT2=-NY,NYM1
               IF (ABS(T(COUNT1,COUNT2,COUNT3)-TCVG(COUNT1,COUNT2,COUNT3)) >= BuildingData%ConvTol) &
               & CONVERGE=.FALSE.
               ! TCVG(COUNT1,COUNT2,COUNT3)=T(COUNT1,COUNT2,COUNT3)
 648         CONTINUE
 649       CONTINUE
 650     CONTINUE
         TCVG(-NX:NXM1,-NY:NYM1,0:NZM1)=T(-NX:NXM1,-NY:NYM1,0:NZM1)
         WRITE(DebugInfo,*) ' YEAR = ', IYR

!*** Determine whether execution should continue.
!*** If not, close all open files and stop.
         IF(QUIT.OR.IYR.EQ.MAXITER) THEN
!*** Write debugging information to the debug file, then close it.
           WRITE(DebugInfo,*) 'CONVERGE = ',CONVERGE
           IF (CONVERGE) THEN
             WRITE(SurfaceTemps,*) ' '
             WRITE(SurfaceTemps,*) '! Convergence has been gained.'
           ELSE
             WRITE(SurfaceTemps,*) ' '
             WRITE(SurfaceTemps,*) '! Convergence has NOT been gained.'
           ENDIF
           CALL CloseIO
           EXIT
         END IF
2001   CONTINUE
     END DO ! END BATCHING LOOP
     RETURN
END SUBROUTINE MainSimControl

!******************************  GET ALL INPUT  *****************************************
SUBROUTINE SetDefaults(AUTOGRID,EPlus,RUNID,WeatherFile,TGNAM,EPWFile,EPGEOM,ErrorsFound)
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   April 22, 2001
     !***       MODIFIED
     !***       RE-ENGINEERED  na

     !*** PURPOSE OF THIS SUBROUTINE:
     !*** This subroutine is the input retrieval driver.

     !*** METHODOLOGY EMPLOYED:
     !*** Standard EnergyPlus "manager" methodology.

     !*** REFERENCES:
     !*** na
IMPLICIT NONE
     CHARACTER (len = *) WeatherFile ! TMY Weather file name
     CHARACTER (len = *) TGNAM       ! Ground temperature file name
     CHARACTER *5 AUTOGRID    ! Will the grid be sized automatically?
     CHARACTER *5 EPlus       ! Is this run being used for EnergyPlus?
     CHARACTER *5 RUNID       ! Run Identifier
     CHARACTER (len =*) EPWFile     ! EnergyPlus weather file name
     CHARACTER *1 EPGEOM      ! Use EnergyPlusGeometry? Y/N
     LOGICAL, INTENT(INOUT) :: ErrorsFound

!*** GET LOCATION INFORMATION
!     CALL GetLocation(AUTOGRID,Eplus,RUNID,EPGEOM,ErrorsFound)
     EPlus     ='TRUE'
     RUNID     ='SLAB'
     EPGEOM    ='FALSE'
!*** GET ALL WEATHER FILES
!     CALL GetWeatherFiles(WeatherFile,TGNAM,EPWFile,ErrorsFound)
     EPWFile    ='in'
     WeatherFile='in.epw'
     RETURN
END SUBROUTINE SetDefaults

SUBROUTINE GetInput2(AUTOGRID,WeatherFile,TGNAM,MaxIter,EPlus,NDIM,NX,NY,NZ,RUNID,EPGEOM,ErrorsFound)
USE InputProcessor
USE DataGlobals, ONLY: ShowFatalError
IMPLICIT NONE
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   May 8-10, 2000
     !***       MODIFIED       June 14, 2000
     !***       RE-ENGINEERED  na

     !*** PURPOSE OF THIS SUBROUTINE:
     !*** This subroutine is the input retrieval driver.
     !*** All retrieval of input information is launched from this subroutine.

     !*** METHODOLOGY EMPLOYED:
     !*** Standard EnergyPlus "manager" methodology.

     !*** REFERENCES:
     !*** na

!*** VARIABLES DECLARATIONS
     CHARACTER *5 AUTOGRID    ! Will the grid be sized automatically?
     CHARACTER *5 EPlus       ! Is this run being used for EnergyPlus?
     CHARACTER *5 RUNID       ! Run Identifier
     CHARACTER (len = *) WeatherFile ! TMY Weather file name
     CHARACTER (len = *) TGNAM       ! Ground temperature file name
     CHARACTER *5 OLDTG       ! Is there an old ground temperature file?
     CHARACTER *1 EPGEOM      ! Retrieve slab geometry from the EnergyPlus input file?
     LOGICAL, INTENT(INOUT) :: ErrorsFound
     INTEGER MaxIter          ! Maximum number of years to iterate
     INTEGER NDIM(12)         ! Number of days in days in each month
     INTEGER  :: NX           ! Number of cells in the X direction
     INTEGER  :: NY           ! Number of cells in the Y direction
     INTEGER  :: NZ           ! Number of cells in the Z direction
     INTEGER  :: NumEquivSlab = -1   ! Number of EquivSlab Objects
     INTEGER  :: NumEquivAutoGrid = -1  !  Nuber of EquivAutoGrid Objects
     INTEGER  :: NumAutoGrid = -1    ! Number of AutoGrid Objects
     INTEGER  :: NumEquivalentSlab = -1 !  Number of  new EquivalentSlab objects
     INTEGER  :: NumManualGrid =  -1    ! Number of ManualGrid Objects
     LOGICAL  :: NodeSizingDone = .false.  !  Flag to bypass multiple node sizing optione.

!*** PRECALCULATE A/P RATIO BASED ON MAIN EPLUS RUN DATA, IF PROVIDED
!     IF (Eplus.NE.'FALSE'.AND.AUTOGRID.NE.'FALSE') CALL GetEPlusGeom

!*** GET SURFACE PROPERTIES
     CALL GetSurfProps(ErrorsFound)
!*** GET MATERIALS PROPERTIES
     CALL GetMatlsProps(ErrorsFound)
!*** GET THE BOUNDARY CONDITIONS
     CALL GetBCs(ErrorsFound)
!*** GET ALL INFORMATION ON THE BUILDING
     CALL GetBuildingInfo(MaxIter,ErrorsFound)

!*** IF THIS RUN IS TO BE USED WITH ENERGYPLUS, RETRIEVE THE
!*** GEOMETRY FROM THE MAIN INPUT FILE
!     IF  (EPGEOM.EQ.'Y') THEN
!       CALL GetEPlusGeom(EPlus)
!     ELSE

!     END IF

!*** GET THE BUILDING INSULATION CONFIGURATION
     CALL GetInsulationInfo(ErrorsFound)

!*** GET NODE SIZING INFORMATION

!   Check to see which node sizing arrangement is in idf

    NumEquivSlab=GetNumObjectsFound('EquivSlab')
    NumEquivAutoGrid = GetNumObjectsFound('EquivAutoGrid')
	NumAutoGrid = GetNumObjectsFound('AutoGrid')
    NumEquivalentSlab = GetNumObjectsFound('EquivalentSlab')
    NumManualGrid = GetNumObjectsFound('ManualGrid')
      ! Print *,'NumEquivSlab  ',NumEquivSlab
      ! Print *,'NumEquivAutoGrid  ',NumEquivAutoGrid
      ! Print *, 'NumAutoGrid  ',NumAutoGrid
      ! Print *, 'NumEquivalentSlab ',NumEquivalentSlab
      ! Print *, 'NumManualGrid  ', NumManualGrid
    If (NumEquivSlab > 0 .and. NumEquivAutoGrid >0 .and. .not. NodeSizingDone) Then   ! This idf has old objects
      Call GetEquivSlabInfo(ErrorsFound)
      Call  GetEquivAutoGridInfo(NX,NY,NZ,ErrorsFound)
      NodeSizingDone = .true.
    End If
    If (NumEquivalentSlab > 0 .and. .not. NodeSizingDone) Then
      CALL GetEquivalentSlabInfo(ErrorsFound)
      NodeSizingDone = .true.
    End If
    If (NumAutoGrid >0 .and. .not. NodeSizingDone ) Then
      CALL   GetAutoGridInfo(NX,NY,NZ,ErrorsFound)
      NodeSizingDone = .true.
    End If
    If (NumManualGrid > 0 .and. .not. NodeSizingDone)Then
      CALL GetManualGridInfo(NX,NY,NZ,ErrorsFound)
    End If

    IF (ErrorsFound) THEN
      CALL ShowFatalError('Program terminates due to preceding condition(s).',SurfaceTemps,InputEcho)
    ENDIF


!*** INITIALIZE THE GROUND TEMPERATURE
!     OLDTG=BCS%OLDTG
!     IF(OLDTG.EQ.'FALSE')THEN
       CALL InitializeTG(WeatherFile,NDIM)
!     END IF
     RETURN
END SUBROUTINE GetInput2

!**********************************  LOCATION INFORMATION  ******************************
SUBROUTINE GetLocation(AUTOGRID,EPlus,RUNID,EPGEOM,ErrorsFound)
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   May 10, 2000
     !***       MODIFIED       June 14, 2000
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor
USE SimData

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(4) :: AlphArray !character string data
     REAL,DIMENSION(4)              :: NumArray  !numeric data
     CHARACTER *5 AUTOGRID
     CHARACTER *5 EPlus
     CHARACTER *5 RUNID
     CHARACTER *1 EPGEOM
     LOGICAL, INTENT(INOUT) :: ErrorsFound

!*** RETRIEVING THE DATA
!     CALL GetObjectItem('Location',RUNNUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)

!*** ASSIGNING THE DATA TO SPECIFIC SEGMENTS OF THE
!*** DERIVED TYPES IN SimData WHERE APPLICABLE
     AUTOGRID  ='TRUE'  !AlphArray(1)
!     AUTOGRID  ='FALSE'  !AlphArray(1)

     EPlus     ='TRUE'  !AlphArray(2)
     RUNID     ='SLAB'  !AlphArray(1)
     EPGEOM    ='FALSE' !AlphArray(4)
     RETURN
END SUBROUTINE GetLocation

!************************  ENERGY PLUS GEOMETRY RETRIEVAL  ******************************
SUBROUTINE GetEPlusGeom(EPlus,ErrorsFound)
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   March 16, 2001
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor
USE SimData

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(16) :: AlphArray !character string data
     REAL(r64),DIMENSION(31)              :: NumArray  !numeric data
     CHARACTER *7 SurfType ! Surface type of a given surface
     REAL(r64) FloorArea        ! Floor area
     REAL(r64) Perimeter        ! Floor perimeter
     REAL(r64) X1,X2,X3,X4,Y1,Y2,Y3,Y4,DIMX1,DIMX2,DIMY1,DIMY2

!*** LOCAL VARIABLE
     REAL(r64) L
     CHARACTER *5 EPlus
     LOGICAL, INTENT(INOUT) :: ErrorsFound
!*** RETRIEVING THE DATA
     CALL GetObjectItem('SURFACE',RUNNUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)

!*** ASSIGNING THE DATA TO SPECIFIC SEGMENTS OF THE
!*** DERIVED TYPES IN SimData WHERE APPLICABLE
     SurfType  =MakeUPPERCase(AlphArray(2))
     FloorArea =NumArray(1)
     IF (SurfType.EQ.'FLOOR')THEN
       X1        =NumArray(7)
       X2        =NumArray(10)
       X3        =NumArray(13)
       X4        =NumArray(16)
       Y1        =NumArray(8)
       Y2        =NumArray(11)
       Y3        =NumArray(14)
       Y4        =NumArray(17)
       DIMX1     =MAX(X2,X1)-MIN(X2,X1)
       DIMX2     =MAX(X3,X4)-MIN(X3,X4)
       DIMY1     =MAX(Y2,Y1)-MIN(Y2,Y1)
       DIMY2     =MAX(Y4,Y3)-MIN(Y4,Y3)
       Perimeter =DIMX1+DIMX2+DIMY1+DIMY2
       APRatio   =FloorArea/Perimeter
     END IF

!*** RETRIEVING THE DATA
     IF (.not. SameString(EPlus,'FALSE').AND.APRatio.NE.0.)THEN
       L=4.*APRatio
       IF(L.LT.6.0)THEN
         SLABX=6.0
         SLABY=(2.*APRatio*SLABX)/(1-(2.*APRatio))
       ELSE
         SLABX=L
         SLABY=L
       END IF
     ELSE
       CALL GetObjectItem('EquivSlab',RUNNUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
       APRatio    =NumArray(1)
       EquivSizing=MakeUPPERCase(AlphArray(1))
       L=4.*APRatio
       IF(L.LT.6.0)THEN
         SLABX=6.0
         SLABY=(2.*APRatio*SLABX)/(1-(2.*APRatio))
       ELSE
         SLABX=L
         SLABY=L
       END IF
     END IF
     RETURN
END SUBROUTINE GetEPlusGeom

!********************************  SURFACE PROPERTIES  **********************************
SUBROUTINE GetSurfProps(ErrorsFound)
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   May 10, 2000
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor !, ONLY: GetObjectItem

USE SimData

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
     LOGICAL, INTENT(INOUT) :: ErrorsFound
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(1) :: AlphArray !character string data
     REAL(r64),DIMENSION(9)              :: NumArray  !numeric data
     INTEGER :: NMAT       ! Number of materials types

!*** RETRIEVING THE DATA
     CALL GetObjectItem('Materials',RUNNUM,AlphArray,NumAlphas, NumArray,NumNums,IOSTAT)
!*** ASSIGNING THE DATA TO SPECIFIC SEGMENTS OF THE
!*** DERIVED TYPES IN SimData WHERE APPLICABLE
     SSP%NumMaterials =NumArray(1)
     NMAT=SSP%NumMaterials
     SSP%ALBEDO(1)=NumArray(2)
     SSP%ALBEDO(2)=NumArray(3)
     SSP%EPSLW(1) =NumArray(4)
     SSP%EPSLW(2) =NumArray(5)
     SSP%Z0(1)    =NumArray(6)
     SSP%Z0(2)    =NumArray(7)
     SSP%HIN(1)   =NumArray(8)
     SSP%HIN(2)   =NumArray(9)
     RETURN
END SUBROUTINE GetSurfProps

!******************************  MATERIALS PROPERTIES  **********************************
SUBROUTINE GetMatlsProps(ErrorsFound)
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   May 10, 2000
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor !, ONLY: GetObjectItem
USE SimData
USE DataGlobals, ONLY: ShowSevereError
USE General, ONLY: RoundSigDigits

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
     LOGICAL, INTENT(INOUT) :: ErrorsFound
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(1) :: AlphArray !character string data
     REAL(r64),DIMENSION(6)              :: NumArray  !numeric data

!*** RETRIEVING THE DATA
     CALL GetObjectItem('MatlProps',RUNNUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
     RHO(1)  =NumArray(1)
     RHO(2)  =NumArray(2)
     CP(1)   =NumArray(3)
     CP(2)   =NumArray(4)
     TCON(1) =NumArray(5)
     IF (NumArray(5) > 7.0) THEN
       CALL ShowSevereError('Slab Thermal Conductivity ['//trim(RoundSigDigits(NumArray(5),3))// &
          '] is greater than max allowed=7.0',SurfaceTemps,InputEcho)
       ErrorsFound=.true.
     ENDIF
     TCON(2) =NumArray(6)
     RETURN
END SUBROUTINE GetMatlsProps

!****************************  BOUNDARY CONDITIONS  *************************************
SUBROUTINE GetBCs(ErrorsFound)
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   May 10, 2000
     !***       MODIFIED       June 14, 2000
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor !, ONLY: GetObjectItem
USE SimData

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
     LOGICAL, INTENT(INOUT) :: ErrorsFound
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(3) :: AlphArray !character string data
     REAL(r64),DIMENSION(2)              :: NumArray  !numeric data


!*** RETRIEVING THE DATA
     CALL GetObjectItem('BoundConds',RUNNUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
     BCS%EVTR =MakeUPPERCase(AlphArray(1))
     IF (.not. (SameString(AlphArray(1),'TRUE' ).or. SameString(AlphArray(1),'FALSE'))) THEN
       CALL ShowWarningError('GetBCs: Entered "EVTR: Is surface evapotranspiration modeled" choice is not valid.'//  &
          ' Entered value="'//trim(AlphArray(1))//'", FALSE will be used.',SurfaceTemps,InputEcho)
       BCS%EVTR='FALSE'
     ENDIF
     BCS%FIXBC=MakeUPPERCase(AlphArray(2))
     IF (.not. (SameString(AlphArray(2),'TRUE') .or. SameString(AlphArray(2),'FALSE')) ) THEN
       CALL ShowWarningError('GetBCs: Entered "FIXBC: is the lower boundary at a fixed temperature" choice is not valid.'//  &
          ' Entered value="'//trim(AlphArray(2))//'", FALSE will be used.',SurfaceTemps,InputEcho)
       BCS%FIXBC='FALSE'
     ENDIF
     BCS%USERHFlag = MakeUPPERCase(AlphArray(3))
     IF (.not. (SameString(AlphArray(3),'TRUE' ).or. SameString(AlphArray(3),'FALSE'))) THEN
       CALL ShowWarningError('GetBCs: Entered "USRHflag: Is the ground surface h specified by the user?" choice is not valid.'//  &
          ' Entered value="'//trim(AlphArray(3))//'", FALSE will be used.',SurfaceTemps,InputEcho)
       BCS%USERHFlag='FALSE'
     ENDIF
     BCS%TDEEPin = NumArray(1)
     BCS%USERH = NumArray(2)
     RETURN
END SUBROUTINE GetBCs

!***********************************  BUILDING INFORMATION  *****************************
SUBROUTINE GetBuildingInfo(MaxIter,ErrorsFound)
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   May 10, 2000
     !***       MODIFIED       June 14, 2000
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor !, ONLY: GetObjectItem
USE SimData

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
     LOGICAL, INTENT(INOUT) :: ErrorsFound
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(1) :: AlphArray !character string data
     REAL(r64),DIMENSION(17)              :: NumArray  !numeric data
     INTEGER :: MaxIter    ! Maximum number of iterations for a solution
     INTEGER :: index

!*** RETRIEVING THE DATA
     CALL GetObjectItem('BldgProps',RUNNUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
     BuildingData%IYRS =NumArray(1)
     BuildingData%SHAPE=NumArray(2)
     BuildingData%HBLDG=NumArray(3)
     BuildingData%NumberOfTIN =12   !  Monthly TIN's
     If (NumNums < 16 ) Then
       BuildingData%NumberOfTIN = 1  !  Old Input file with singlt TIN
     EndIF
     do index = 1,BuildingData%NumberOfTIN
         BuildingData%TINave(index)  =NumArray(3+index)
     end do
     BuildingData%TINAmp = NumArray(NumNums-1)  ! Pick second tolast Number, will not be used if only one TIN.

     BuildingData%ConvTol=NumArray(NumNums)    ! Last number is ConvTol even if only one TIN
     IF (BuildingData%ConvTol <= 0.0) BuildingData%ConvTol=.1d0
!*** Global Variable Assignments
     Maxiter=BuildingData%IYRS
     RETURN
END SUBROUTINE GetBuildingInfo

!*******************************  INSULATION   ******************************************
SUBROUTINE GetInsulationInfo(ErrorsFound)
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   May 10, 2000
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor !, ONLY: GetObjectItem
USE SimData

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
     LOGICAL, INTENT(INOUT) :: ErrorsFound
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(1) :: AlphArray !character string data
     REAL(r64),DIMENSION(5)              :: NumArray  !numeric data

!*** RETRIEVING THE DATA
     Insul%RINS =0
     Insul%DINS =0
     Insul%RVINS=0
     Insul%ZVINS=0
     Insul%IVINS=0

     CALL GetObjectItem('Insulation',RUNNUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
     Insul%RINS =NumArray(1)
     Insul%DINS =NumArray(2)
     Insul%RVINS=NumArray(3)
     Insul%ZVINS=NumArray(4)
     Insul%IVINS=NumArray(5)
     RETURN
END SUBROUTINE GetInsulationInfo

!*************  CALCULATION OF EQUIVALENT RECTANGLE OR SQUARE  **************************
SUBROUTINE GetEquivalentSlabInfo(ErrorsFound)
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   August 16,2000
     !***       MODIFIED       COP This now gets an expanded EquivalentSlab object that includes the old EquivAutoGrid
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor !, ONLY: GetObjectItem
USE DataGlobals, ONLY: ShowWarningError
USE SimData
USE General, ONLY: RoundSigDigits

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
     LOGICAL, INTENT(INOUT) :: ErrorsFound

!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     INTEGER  :: NX           ! Number of cells in the X direction
     INTEGER  :: NY           ! Number of cells in the Y direction
     INTEGER  :: NZ           ! Number of cells in the Z direction
     Logical  :: EquivSlab
     CHARACTER(len=MaxNameLength),DIMENSION(1) :: AlphArray !character string data
     REAL(r64),DIMENSION(4)              :: NumArray  !numeric data
     CHARACTER *5 EPlus

!*** LOCAL VARIABLE ASSIGNMENTS
     REAL(r64) L                ! Square slab dimension from A/P ratio

!*** RETRIEVING THE DATA

       CALL GetObjectItem('EquivalentSlab',RUNNUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
       APRatio    =NumArray(1)
       IF (APRatio < 1.5) Then
          CALL ShowWarningError('GetEquivalentSlabInfo: APRatio =['//TRIM(RoundSigDigits(APRatio,3))//  &
             '] too small. Will be set to 1.50',SurfaceTemps,InputEcho)
          APRatio = 1.5d0
       END IF
       IF (APRatio > 22.) Then
          CALL ShowWarningError('GetEquivalentSlabInfo: APRatio =['//TRIM(RoundSigDigits(APRatio,3))//  &
             '] too big (maximum=22). Will be set to 22.0',SurfaceTemps,InputEcho)
          APRatio = 22.
       END IF
       L=4.*APRatio

        SLABX=L
        SLABY=L

       SLABDEPTH=NumArray(2)
       If(SLABDEPTH > 0.308d0) THEN
         CALL ShowWarningError('Slab Depth ='//TRIM(RoundSigDigits(SLABDEPTH,3))//' is too large.  '//  &
           'Set to .308 (12 inches)',SurfaceTemps,InputEcho)
         SLABDEPTH = 0.308d0  !  12 inches
       END IF
      CLEARANCE=NumArray(3)
      IF (NumNums > 3) Then
        ZCLEARANCE = NumArray(4)
      ELSE
        ZCLEARANCE = CLEARANCE
      End If
       CALL AutoGridding()
       NX=InitGrid%NX
       NY=InitGrid%NY
       NZ=InitGrid%NZ

     RETURN
END SUBROUTINE GetEquivalentSlabInfo



!**************************  AUTOMATED GRID INFORMATION  ********************************
SUBROUTINE GetAutoGridInfo(NX,NY,NZ,ErrorsFound)
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   May 10, 2000
     !***       MODIFIED       June 14, 2000
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE DataGlobals,   ONLY: ShowWarningError, ShowContinueError
USE InputProcessor !, ONLY: GetObjectItem
USE SimData
USE General, ONLY: RoundSigDigits

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
     LOGICAL, INTENT(INOUT) :: ErrorsFound
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas    ! Number of elements in the alpha array
     INTEGER  :: NumNums      ! Number of elements in the numeric array
     INTEGER  :: IOStat       ! IO Status when calling get input subroutine
     INTEGER  :: NX           ! Number of cells in the X direction
     INTEGER  :: NY           ! Number of cells in the Y direction
     INTEGER  :: NZ           ! Number of cells in the Z direction
     CHARACTER(len=MaxNameLength),DIMENSION(0) :: AlphArray !character string data
     REAL(r64),DIMENSION(5)              :: NumArray  !numeric data
!     Logical AutoGridFlag
!*** RETRIEVING THE DATA
     CALL ShowWarningError('You have selected to use the actual Building Geometry.',SurfaceTemps,InputEcho)
     CALL ShowWarningError('If your building is not a square or rectangle, please reselect to use '//  &
        'an equivalent square foundation based on the area to perimeter ratio of the actual geometry.',SurfaceTemps,InputEcho)
     PRINT *,'You have selected to use the actual building geometry.'
     PRINT *,'If your building is not a square or rectangle, please reselect '
     PRINT *,'to use an equivalent square foundation based on the area to'
     PRINT *,'perimeter ratio of the actual geometry.'

     CALL GetObjectItem('AutoGrid',RUNNUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
     SLABX    =NumArray(1)
     IF(SLABX <6.0 ) Then
       CALL ShowWarningError('Entered Slab X=['//trim(roundsigDigits(SLABX,1))//' m'//'] is too small, set to 6.0 '// &
         'and the program continues.',SurfaceTemps,InputEcho)
       slabx =6.
       print *, 'SLABX RESET TO 6.0 m.'
     END IF
     SLABY    =NumArray(2)
     IF(SLABY <6.0 ) Then
       CALL ShowWarningError('Entered Slab Y=['//trim(roundsigDigits(SLABY,1))//' m'//'] is too small, set to 6.0 '// &
         'and the program continues.',SurfaceTemps,InputEcho)
       SLABY  =6.
       print *, 'SLABY RESET TO 6.0 m.'
     END IF
     SLABDEPTH=NumArray(3)
     If(SLABDEPTH > 0.308) THEN
         CALL ShowWarningError('Slab Depth =['//TRIM(RoundSigDigits(SLABDEPTH,3))//'] is too large.  '//  &
           'Set to .308 (12 inches) and the program continues.',SurfaceTemps,InputEcho)
         SLABDEPTH = 0.308  !  12 inches
     END IF
     CLEARANCE=NumArray(4)
      IF (NumNums > 4) Then
        ZCLEARANCE = NumArray(5)
      ELSE
        ZCLEARANCE = CLEARANCE
      End If

     CALL AutoGridding()
     NX=InitGrid%NX
     NY=InitGrid%NY
     NZ=InitGrid%NZ
!  print *, ' In GetAutoGrid', slabx,slaby,NX,NY
     RETURN
END SUBROUTINE GetAutoGridInfo
!*************  CALCULATION OF EQUIVALENT RECTANGLE OR SQUARE (Outdated Version  **************************
SUBROUTINE GetEquivSlabInfo(ErrorsFound)
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   August 16,2000
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor !, ONLY: GetObjectItem
USE SimData
USE General, ONLY: RoundSigDigits
USE DataGlobals, ONLY: ShowWarningError

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
     LOGICAL, INTENT(INOUT) :: ErrorsFound
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(1) :: AlphArray !character string data
     REAL(r64),DIMENSION(1)              :: NumArray  !numeric data
     CHARACTER *5    EPlus

!*** LOCAL VARIABLE ASSIGNMENTS
     REAL(r64) L                ! Square slab dimension from A/P ratio

!*** RETRIEVING THE DATA

       CALL GetObjectItem('EquivSlab',RUNNUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
       APRatio    =NumArray(1)
       IF (APRatio < 1.5d0) Then
          CALL ShowWarningError('APRatio =['//TRIM(RoundSigDigits(APRatio,3))//'] is too small. Reset to minimum=1.50' //  &
             ' and program continues.',SurfaceTemps,InputEcho)
          print *,  ' APRatio too small, reset to 1.5  '
          APRatio = 1.5d0
       END IF
       IF (APRatio >22.) Then
          CALL ShowWarningError('APRatio =['//TRIM(RoundSigDigits(APRatio,3))//'] is too big. Reset to maximum=22.0' //  &
             ' and program continues.',SurfaceTemps,InputEcho)
          print *, 'APRatio too large, reset to 22.'
          APRatio = 22.
       END IF

       EquivSizing=AlphArray(1)
       L=4.*APRatio

       SLABX=L
       SLABY=L


     RETURN
END SUBROUTINE GetEquivSlabInfo
!************************** EQUIVALENT AUTOMATED GRID INFORMATION  **********************
SUBROUTINE GetEquivAutoGridInfo(NX,NY,NZ,ErrorsFound)
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   August 16,2000
     !***       MODIFIED
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE DataGlobals, ONLY: ShowWarningError
USE InputProcessor !, ONLY: GetObjectItem
USE SimData
USE General, ONLY: RoundSigDigits

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
     LOGICAL, INTENT(INOUT) :: ErrorsFound
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas    ! Number of elements in the alpha array
     INTEGER  :: NumNums      ! Number of elements in the numeric array
     INTEGER  :: IOStat       ! IO Status when calling get input subroutine
     INTEGER  :: NX           ! Number of cells in the X direction
     INTEGER  :: NY           ! Number of cells in the Y direction
     INTEGER  :: NZ           ! Number of cells in the Z direction
     CHARACTER(len=MaxNameLength),DIMENSION(0) :: AlphArray !character string data
     REAL(r64),DIMENSION(3)              :: NumArray  !numeric data

!*** RETRIEVING THE DATA
     PRINT *,'Area to Perimeter Ratio selected'

     CALL GetObjectItem('EquivAutoGrid',RUNNUM,AlphArray,  &
     &    NumAlphas,NumArray,NumNums,IOSTAT)
     SLABDEPTH=NumArray(1)
     If(SLABDEPTH > 0.308d0) THEN
         CALL ShowWarningError('Slab Depth ='//TRIM(RoundSigDigits(SLABDEPTH,3))//' is too large.  '//  &
           'Set to .308 (12 inches)',SurfaceTemps,InputEcho)
         SLABDEPTH = 0.308d0  !  12 inches
     END IF
     CLEARANCE=NumArray(2)
     IF (NumNums > 2) Then
        ZCLEARANCE = NumArray(3)
      ELSE
        ZCLEARANCE = CLEARANCE
      End If

     CALL AutoGridding()
     NX=InitGrid%NX
     NY=InitGrid%NY
     NZ=InitGrid%NZ
     RETURN
END SUBROUTINE GetEquivAutoGridInfo

!*************************  MANUAL GRID INFORMATION  ************************************
SUBROUTINE GetManualGridInfo(NX,NY,NZ,ErrorsFound)
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   May 10, 2000
     !***       MODIFIED       June 14, 2000
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor !, ONLY: GetObjectItem
USE SimData

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
     LOGICAL, INTENT(INOUT) :: ErrorsFound
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(1) :: AlphArray !character string data
     REAL(r64),DIMENSION(5)              :: NumArray  !numeric data

!*** PROGRAM VARIABLES
     INTEGER  :: NX           ! Number of cells in the X direction
     INTEGER  :: NY           ! Number of cells in the Y direction
     INTEGER  :: NZ           ! Number of cells in the Z direction

!*** RETRIEVING THE DATA
     CALL GetObjectItem('ManualGrid',RUNNUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
     InitGrid%NX  =NumArray(1)
     InitGrid%NY  =NumArray(2)
     InitGrid%NZ  =NumArray(3)
     Slab%IBOX=NumArray(4)
     Slab%JBOX=NumArray(5)
     NX=InitGrid%NX
     NY=InitGrid%NY
     NZ=InitGrid%NZ
     CALL GetXFACEData(NX)
     CALL GetYFACEData(NY)
     CALL GetZFACEData(NZ)
     RETURN
END SUBROUTINE GetManualGridInfo

!***********************************  XFACE VALUES  *************************************
SUBROUTINE GetXFACEData(NX)
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   May 10, 2000
     !***       MODIFIED       June 14, 2000
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor !, ONLY: GetObjectItem
USE SimData

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NX
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(1) :: AlphArray !character string data
     REAL(r64),DIMENSION(-NX:NX)         :: NumArray  !numeric data

!*** RETRIEVING THE DATA
     CALL GetObjectItem('XFACE',RUNNUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
     XFACE=NumArray
     RETURN
END SUBROUTINE GetXFACEData

!***********************************  YFACE VALUES  *************************************
SUBROUTINE GetYFACEData(NY)
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   May 10, 2000
     !***       MODIFIED       June 14, 2000
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor !, ONLY: GetObjectItem
USE SimData

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NY
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(1) :: AlphArray !character string data
     REAL(r64),DIMENSION(-NY:NY)         :: NumArray  !numeric data


!*** RETRIEVING THE DATA
     CALL GetObjectItem('YFACE',RUNNUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
       YFACE=NumArray
     RETURN
END SUBROUTINE GetYFACEData

!***********************************  ZFACE VALUES  *************************************
SUBROUTINE GetZFACEData(NZ)
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   May 10, 2000
     !***       MODIFIED       June 14, 2000
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor !, ONLY: GetObjectItem
USE SimData

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NZ
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(1) :: AlphArray !character string data
     REAL(r64),DIMENSION(0:NZ)           :: NumArray  !numeric data

!*** RETRIEVING THE DATA
     CALL GetObjectItem('ZFACE',RUNNUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
       ZFACE=NumArray
     RETURN
END SUBROUTINE GetZFACEData

!*****************************  WEATHER FILES  ******************************************
SUBROUTINE GetWeatherFiles(WeatherFile,TGNAM,EPWFile,ErrorsFound)
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   May 10, 2000
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

!*** USE STATEMENTS
USE InputProcessor !, ONLY: GetObjectItem
USE SimData

!*** ENFORCE EXPLICIT VARIABLE TYPING
IMPLICIT NONE
LOGICAL, INTENT(INOUT) :: ErrorsFound
!*** LOCAL VARIABLE DECLARATIONS
     INTEGER  :: NumAlphas ! Number of elements in the alpha array
     INTEGER  :: NumNums   ! Number of elements in the numeric array
     INTEGER  :: IOStat    ! IO Status when calling get input subroutine
     CHARACTER(len=MaxNameLength),DIMENSION(3) :: AlphArray !character string data
     REAL(r64),DIMENSION(0)         :: NumArray  !numeric data
     CHARACTER (Len =*) WeatherFile
     CHARACTER (len = *) TGNAM
     CHARACTER (len = *) EPWFile

!*** RETRIEVING THE DATA
!     CALL  GetObjectItem('Weather',RUNNUM,AlphArray,NumAlphas,NumArray,NumNums,IOSTAT)
!     WeatherFile=TRIM(AlphArray(1))
!     TGNAM      =TRIM(AlphArray(2))
     EPWFile    ='in' !TRIM(AlphArray(1))
     WeatherFile='in.epw'
     RETURN
END SUBROUTINE GetWeatherFiles
!****************************************************************************************
!*******************************  END ENERGY PLUS INPUT STAGE  **************************
!****************************************************************************************

!*******************  CALCULATE SOME CELL GEOMETRY CONSTANTS   **************************
SUBROUTINE CellGeom(NX,NXM1,NY,NYM1,NZ,NZM1,XC,YC,ZC,DX,DY,DZ,DXP,DYP,DZP)
USE SimData
IMPLICIT NONE
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   October 12, 1999
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

     !*** PURPOSE OF THIS SUBROUTINE:
     !*** This subroutine will compute some basic cell geometry factors. This was
     !*** originally in the
     !*** main Slab3D program.

     !*** METHODOLOGY EMPLOYED:
     !*** Standard EnergyPlus "manager" methodology.

     !*** REFERENCES:
        ! Bahnfleth, W.  1989.  Three-dimensional modelling of heat transfer from slab
        ! floors. Ph.D. diss., University of Illinois at Urbana-Champaign.  Also
        ! published as US Army CERL Technical Manuscript E-89/11.
     !*** Compute some cell geometry factors
     INTEGER NX          ! Number of cells in the X direction (from -NX to NX)      []
     INTEGER NXM1        ! NX-1                                                     []
     INTEGER NY          ! Number of cells in the Y direction (from -NY to NY)      []
     INTEGER NYM1        ! NY-1                                                     []
     INTEGER NZ          ! Number of cells in the Z direction (from -NZ to NZ)      []
     INTEGER NZM1        ! NZ-1                                                     []
     REAL(r64) XC(-35:35)     ! Array of X direction cell center coordinates            [m]
     REAL(r64) DX(-35:35)     ! Array of cell dimensions                                [m]
     REAL(r64) DXP(-35:35)    ! Array of distances between X direction cell centers     [m]
     REAL(r64) YC (-35:35)    ! Array of Y direction cell center coordinates            [m]
     REAL(r64) DY(-35:35)     ! Array of cell dimensions                                [m]
     REAL(r64) DYP(-35:35)    ! Array of distances between Y direction cell centers     [m]
     REAL(r64) ZC(0:35)       ! Array of Z direction cell center coordinates            [m]
     REAL(r64) DZ(0:35)       ! Array of cell dimensions                                [m]
     REAL(r64) DZP(0:35)      ! Array of distances between Z direction cell centers     [m]

     NZ=InitGrid%NZ
     NY=InitGrid%NY
     NZ=InitGrid%NZ
     NXM1=NX-1
     NYM1=NY-1
     NZM1=NZ-1
     DO 100 COUNT1=-NX,NXM1
       XC(COUNT1)=(XFACE(COUNT1)+XFACE(COUNT1+1))/2.0d0
       DX(COUNT1)=XFACE(COUNT1+1)- XFACE(COUNT1)
100  CONTINUE
     DO 101 COUNT1=-NX,NX-2
       DXP(COUNT1)=XC(COUNT1+1)-XC(COUNT1)
101  CONTINUE
     DO 102 COUNT1=-NY,NYM1
       YC(COUNT1)=(YFACE(COUNT1)+YFACE(COUNT1+1))/2.0d0
       DY(COUNT1)=YFACE(COUNT1+1)- YFACE(COUNT1)
102  CONTINUE
     DO 103 COUNT1=-NY,NY-2
       DYP(COUNT1)=YC(COUNT1+1)-YC(COUNT1)
103  CONTINUE
     DO 104 COUNT1=0,NZM1
       ZC(COUNT1)=(ZFACE(COUNT1)+ZFACE(COUNT1+1))/2.0d0
       DZ(COUNT1)=ZFACE(COUNT1+1)- ZFACE(COUNT1)
104  CONTINUE
     ZC(0)=0.
     DO 105 COUNT1=0,NZ-2
       DZP(COUNT1)=ZC(COUNT1+1)-ZC(COUNT1)
105  CONTINUE
     RETURN
END SUBROUTINE CellGeom

!******************************  DEFINE THE FLOOR SLAB  *********************************
SUBROUTINE DefineSlab(NX,NXM1,NY,NYM1,NZM1,XC,YC,DX,DY,AFLOR,DA,ATOT,PERIM,MSURF,MTYPE)
USE SimData
IMPLICIT NONE
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   October 29, 1999
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

     !*** PURPOSE OF THIS SUBROUTINE:
     !*** This subroutine will define the building slab in the 3-D domain. The slab is,
     !*** by definition, one cell thick. The options for MTYPE are as follows:
     !*** MTYPE=1 for building cells, MTYPE=2 for soil cells.
     !*** At this point, the building floor area and perimeter are also calculated.

     !*** METHODOLOGY EMPLOYED:
     !*** Standard EnergyPlus "manager" methodology.

     !*** REFERENCES:
        ! Bahnfleth, W.  1989.  Three-dimensional modelling of heat transfer from slab
        ! floors. Ph.D. diss., University of Illinois at Urbana-Champaign.  Also
        ! published as US Army CERL Technical Manuscript E-89/11.
     !*** Compute some cell geometry factors
     REAL(r64) AFLOR                         ! Building Floor Area                        [m2]
     REAL(r64) ATOT                          ! Total building Area                        [m2]
     REAL(r64) XC(-35:35)                    ! Array of cell center coordinates           [m]
     REAL(r64) YC (-35:35)                   ! Array of cell center coordinates           [m]
     REAL(r64) PERIM                         ! Slab perimeter                             [m]
     REAL(r64) DA(-35:35,-35:35)             ! Array of Differential cell areas           [m2]
     REAL(r64) DY(-35:35)                    ! Array of differences between cell centers  [m]
     REAL(r64) DX(-35:35)                    ! Array of differences between cell centers  [m]
     INTEGER NX                         ! Number of cells in the X direction          []
     INTEGER NY                         ! Number of cells in the Y direction          []
     INTEGER NXM1                       ! NX-1                                        []
     INTEGER NYM1                       ! NY-1                                        []
     INTEGER NZM1                       ! NZ-1                                        []
     INTEGER MTYPE(-35:35,-35:35,0:35)  ! Array of material types 1=bldg, 2=soil      []
     INTEGER MSURF(-35:35,-35:35)       ! Array of material types (surface only)      []
     INTEGER IBOX                       ! Cell number of the edge of the slab         []
     INTEGER JBOX                       ! Cell number of the edge of the slab         []
     INTEGER SHAPE                      ! Building shape indicator                    []

     IBOX=Slab%IBOX
     JBOX=Slab%JBOX
     SHAPE=BuildingData%Shape

     AFLOR=0.
     ATOT=(XFACE(NX)-XFACE(-NX))*(YFACE(NY)-YFACE(-NY))
!*** Assign materials properties to the surface cells (2=soil, 1=building)
     DO 200 COUNT1=-NX,NXM1
       DO 190 COUNT2=-NY,NYM1
         DO 180 COUNT3=0,NZM1
          IF(COUNT3.GE.1) THEN
             MTYPE(COUNT1,COUNT2,COUNT3)=2
           ELSE
             IF(XC(COUNT1).LT.XFACE(-IBOX).OR.XC(COUNT1).GT.XFACE(IBOX).OR. &
             & YC(COUNT2).LT.YFACE(-JBOX).OR.YC(COUNT2).GT.YFACE(JBOX)) THEN
               MTYPE(COUNT1,COUNT2,0)=2
             ELSE
               MTYPE(COUNT1,COUNT2,0)=1
               IF(SHAPE.EQ.1) THEN
                 IF(XC(COUNT1).GT.0..AND.YC(COUNT2).GT.0.) THEN
                   MTYPE(COUNT1,COUNT2,0)=2
                 END IF
               ELSE IF(SHAPE.EQ.2) THEN
                 IF(XC(COUNT1).LT.0..AND.YC(COUNT2).GT.0.) THEN
                   MTYPE(COUNT1,COUNT2,0)=2
                 END IF
               ELSE IF(SHAPE.EQ.3) THEN
                 IF(XC(COUNT1).LT.0..AND.YC(COUNT2).LT.0.) THEN
                   MTYPE(COUNT1,COUNT2,0)=2
                 END IF
               ELSE IF(SHAPE.EQ.4) THEN
                 IF(XC(COUNT1).GT.0..AND.YC(COUNT2).LT.0.) THEN
                   MTYPE(COUNT1,COUNT2,0)=2
                 END IF
               END IF
             END IF
           END IF
           IF(COUNT3.EQ.0) MSURF(COUNT1,COUNT2)=MTYPE(COUNT1,COUNT2,0)
180      CONTINUE
!*** Calculate the floor area of the building.
         DA(COUNT1,COUNT2)=DX(COUNT1)*DY(COUNT2)
         IF(MSURF(COUNT1,COUNT2).EQ.1) AFLOR=AFLOR+DA(COUNT1,COUNT2)
190    CONTINUE
200  CONTINUE

!*** Calculate the perimeter of the building.
     PERIM=2.*(XFACE(IBOX)-XFACE(-IBOX)+YFACE(JBOX)-YFACE(-JBOX))
END SUBROUTINE DefineSlab

!*****************************  DEFINE INSULATED CELLS  *********************************
SUBROUTINE DefineInsulation(NX,NXM1,NY,NYM1,MSURF,XC,YC,INS)
USE SimData
IMPLICIT NONE
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   October 31, 1999
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

     !*** PURPOSE OF THIS SUBROUTINE:
     !*** This subroutine will determine which cells have horizontal (slab) insulation
     !*** on them. Vertical (Foundation wall) insulation will be applied to the edges
     !*** of the slab and is assigned at another stage in the program.

     !*** METHODOLOGY EMPLOYED:
     !*** Standard EnergyPlus "manager" methodology.

     !*** REFERENCES:
        ! Bahnfleth, W.  1989.  Three-dimensional modelling of heat transfer from slab
        ! floors. Ph.D. diss., University of Illinois at Urbana-Champaign.  Also
        ! published as US Army CERL Technical Manuscript E-89/11.

     INTEGER NX,NXM1          ! Number of cells in the X direction, NX-1         []
     INTEGER NY,NYM1          ! Number of cells in the Y direction, NY-1         []
     INTEGER IBOX             ! Cell number of the X edge of the slab            []
     INTEGER JBOX             ! Cell number of the Y edge of the slab            []
     INTEGER SHAPE            ! Shape indicator for the slab                     []
     INTEGER INS(-35:35,-35:35) ! Array of insulation indicators for the domain  []
     REAL(r64) DINS                ! Width of slab insulation strip                  [m]
     REAL(r64) XC(-35:35)          ! Array of cell center coordinates (X direction)  [m]
     REAL(r64) YC(-35:35)          ! Array of cell center coordinates (Y direction)  [m]
     INTEGER MSURF(-35:35,-35:35)  ! Array of surface material properties        []
     DINS=Insul%DINS
     IBOX=Slab%IBOX
     JBOX=Slab%JBOX
     SHAPE=BuildingData%Shape
     DO 210 COUNT1=-NX,NXM1
       DO 209 COUNT2=-NY,NYM1
         IF(MSURF(COUNT1,COUNT2).EQ.1) THEN
           IF(XC(COUNT1).LT.(XFACE(-IBOX)+DINS).OR.XC(COUNT1).GT.   &
             & (XFACE(IBOX)-DINS).OR.YC(COUNT2).LT.                 &
             & (YFACE(-JBOX)+DINS).OR.YC(COUNT2).GT.                &
             & (YFACE(JBOX)-DINS)) THEN
             INS(COUNT1,COUNT2)=1
           ELSE IF(SHAPE.EQ.1) THEN
             IF(XC(COUNT1).GT.-DINS.OR.YC(COUNT2).GT.-DINS) INS(COUNT1,COUNT2)=1
           ELSE IF(SHAPE.EQ.2) THEN
             IF(XC(COUNT1).LT.DINS.OR.YC(COUNT2).GT.-DINS) INS(COUNT1,COUNT2)=1
           ELSE IF(SHAPE.EQ.3) THEN
             IF(XC(COUNT1).LT.DINS.OR.YC(COUNT2).LT.DINS) INS(COUNT1,COUNT2)=1
           ELSE IF(SHAPE.EQ.4) THEN
             IF(XC(COUNT1).GT.-DINS.OR.YC(COUNT2).LT.DINS) INS(COUNT1,COUNT2)=1
           END IF
         ELSE
           INS(COUNT1,COUNT2)=0
         END IF
209    CONTINUE
210  CONTINUE
     RETURN
END SUBROUTINE DefineInsulation

!*******************  CALCULATE FEM COEFFICIENTS FOR THE DOMAIN  ************************
SUBROUTINE CalculateFEMCoeffs(INS,NX,NXM1,NY,NYM1,NZM1,MTYPE,DXP,   &
           & DX,DYP,DY,DZP,DZ,CXM,CXP,CYM,CYP,CZM,CZP,EDGEX,EDGEY)
USE SimData
IMPLICIT NONE

!** THIS SUBROUTINE COMPUTES CONSTANTS USED IN THE FINITE
!** DIFFERENCE EQUATIONS SO THAT THEY DO NOT NEED TO BE REGENERATED
!** AT EVERY TIME STEP. THEY ARE FUNCTIONS OF CELL DIMENSIONS AND
!** OF THERMAL CONDUCTIVITY. THEY MUST BE RECOMPUTED EVERY TIME
!** MATERIAL PROPERTIES ARE VARIED, BUT NEED BE CALCULATED ONLY
!** ONCE IN A CONSTANT PROPERTY RUN. SEE SLAB3D FOR MOST VARIABLE DEFS.

!*** 1 AUGUST 1988

!*** WM. BAHNFLETH
!**** MODIFIED EXTENSIVELY JUNE-JULY 1999 BY E. CLEMENTS

     REAL(r64) DX(-35:35)     ! Array of cell dimensions                             [m]
     REAL(r64) DY(-35:35)     ! Array of cell dimensions                             [m]
     REAL(r64) DZ(0:35)       ! Array of cell dimensions                             [m]
     REAL(r64) DXP(-35:35)    ! Array of distances between X direction cells         [m]
     REAL(r64) DYP(-35:35)    ! Array of distances between Y direction cells         [m]
     REAL(r64) DZP(0:35)      ! Array of distances between Z direction cells         [m]
     REAL(r64) CXM(-35:35,-35:35,0:35)  ! Array of FDM coefficients (-X)       [       ]
     REAL(r64) CXP(-35:35,-35:35,0:35)  ! Array of FDM coefficients (+X)       [       ]
     REAL(r64) CYM(-35:35,-35:35,0:35)  ! Array of FDM coefficients (-Y)       [       ]
     REAL(r64) CYP(-35:35,-35:35,0:35)  ! Array of FDM coefficients (+Y)       [       ]
     REAL(r64) CZM(-35:35,-35:35,0:35)  ! Array of FDM coefficients (-Z)       [       ]
     REAL(r64) CZP(-35:35,-35:35,0:35)  ! Array of FDM coefficients (+Z)       [       ]
     REAL(r64) RINS           ! R value of the slab insulation              [K/(W/m**2)]
     REAL(r64) RVINS          ! R value of the foundation wall insulation   [K/(W/m**2)]
     REAL(r64) ZVINS          ! Depth of the foundation wall insulation              [m]
     INTEGER INS(-35:35,-35:35)    ! Array of slab insulation indicators         []
     INTEGER MTYPE(-35:35,-35:35,0:35)  ! Array of domain material properties    []
     INTEGER IVINS       ! Integer indicator of foundation wall insulation 1=Y   []
     INTEGER EDGEX(-35:35),EDGEY(-35:35) ! Slab edge indicators                  []
     INTEGER NX          ! Number of cells in the X direction (from -NX to NX)   []
     INTEGER NXM1        ! NX-1                                                  []
     INTEGER NY          ! Number of cells in the Y direction (from -NY to NY)   []
     INTEGER NYM1        ! NY-1                                                  []
     INTEGER NZM1        ! Number of cells in the Z direction minus 1            []

     REAL(r64) XK             ! Effective thermal conductivity in X direction    [W/m-K]
     REAL(r64) YK             ! Effective thermal conductivity in Y direction    [W/m-K]
     REAL(r64) ZK             ! Effective thermal conductivity in Z direction    [W/m-K]

     RINS=Insul%RINS
     RVINS=Insul%RVINS
     ZVINS=Insul%ZVINS
     IVINS=Insul%IVINS

!*** Initialize edge cell indicator variables
     DO 3030 COUNT1=-NX,NXM1
       DO 3040 COUNT2=-NY,NYM1
         EDGEX(COUNT1)=0
         EDGEY(COUNT2)=0
3040   CONTINUE
3030 CONTINUE

!*** CXM, CYM, CZM are coefficients referring to cell face in the negative
!*** coordinate direction indicated for the the center cell node
!*** CXP, CYP, AND CZP refer to faces in the positive direction from the center
!*** node
     DO 100 COUNT1=-NXM1,NXM1
       DO 90 COUNT2=-NY,NYM1
         DO 80 COUNT3=0,NZM1
!*** Determine the effective conductivity at the X interface (COUNT1-1/COUNT1)
!*** Unless neighbor cells are of different materials or there is a surface
!*** surface resistance due to insulation, the effective conductivity at the
!*** interface is the same as the actual cell conductivity.
           IF (IVINS.EQ.1) THEN
             IF(COUNT3.EQ.0)THEN
               IF(MTYPE(COUNT1,COUNT2,COUNT3).EQ.MTYPE(COUNT1-1,COUNT2,COUNT3)) THEN
                 XK=TCON(MTYPE(COUNT1,COUNT2,COUNT3))
               ELSE IF(MTYPE(COUNT1,COUNT2,COUNT3).EQ.1.OR.&
                 & MTYPE(COUNT1-1,COUNT2,COUNT3).EQ.1) THEN
                 XK=DXP(COUNT1-1)/(DX(COUNT1-1)/                        &
                 & TCON(MTYPE(COUNT1-1,COUNT2,COUNT3))/2.0d0+              &
                 & DX(COUNT1)/TCON(MTYPE(COUNT1,COUNT2,COUNT3))/2.0d0+RINS+IVINS*RVINS)
                 IF(MTYPE(COUNT1,COUNT2,COUNT3).EQ.1.)THEN
                   EDGEX(COUNT1)=1
                 ELSE IF(MTYPE(COUNT1-1,COUNT2,COUNT3).EQ.1)THEN
                   EDGEX(COUNT1-1)=1
                 END IF
               ELSE
                   XK=DXP(COUNT1-1)/(DX(COUNT1-1)/                      &
                   & TCON(MTYPE(COUNT1-1,COUNT2,COUNT3))/2.0d0+            &
                   & DX(COUNT1)/TCON(MTYPE(COUNT1,COUNT2,COUNT3))/2.0d0)
               END IF
             ELSE IF(COUNT3.GT.0.AND.ZFACE(COUNT3).LE.ZVINS)THEN
               IF(MTYPE(COUNT1,COUNT2,0).EQ.MTYPE(COUNT1-1,COUNT2,0))THEN
                 XK=TCON(MTYPE(COUNT1,COUNT2,COUNT3))
               ELSE IF (MTYPE(COUNT1,COUNT2,0).EQ.1.OR.                 &
                 & MTYPE(COUNT1-1,COUNT2,0).EQ.1) THEN
                 XK=DXP(COUNT1-1)/(DX(COUNT1-1)/                        &
                 & TCON(MTYPE(COUNT1-1,COUNT2,COUNT3))/2.0d0+              &
                 & DX(COUNT1)/TCON(MTYPE(COUNT1,COUNT2,COUNT3))/2.0d0+     &
                 & RINS+IVINS*RVINS)
               ELSE
                 XK=DXP(COUNT1-1)/(DX(COUNT1-1)/                        &
                    & TCON(MTYPE(COUNT1-1,COUNT2,COUNT3))/2.0d0+           &
                    & DX(COUNT1)/TCON(MTYPE(COUNT1,COUNT2,COUNT3))/2.0d0)
               END IF
             ELSE
               XK=DXP(COUNT1-1)/(DX(COUNT1-1)/                          &
                 & TCON(MTYPE(COUNT1-1,COUNT2,COUNT3))/2.0d0+              &
                 & DX(COUNT1)/TCON(MTYPE(COUNT1,COUNT2,COUNT3))/2.0d0)
             END IF
           ELSE
             IF(MTYPE(COUNT1,COUNT2,COUNT3).EQ.MTYPE(COUNT1-1,COUNT2,COUNT3)) THEN
               XK=TCON(MTYPE(COUNT1,COUNT2,COUNT3))
             ELSE IF(MTYPE(COUNT1,COUNT2,COUNT3).EQ.1.OR.               &
                    & MTYPE(COUNT1-1,COUNT2,COUNT3).EQ.1) THEN
               XK=DXP(COUNT1-1)/(DX(COUNT1-1)/                          &
               & TCON(MTYPE(COUNT1-1,COUNT2,COUNT3))/2.0d0+DX(COUNT1)/     &
               & TCON(MTYPE(COUNT1,COUNT2,COUNT3))/2.0d0+RINS)
             ELSE
               XK=DXP(COUNT1-1)/(DX(COUNT1-1)/                          &
               & TCON(MTYPE(COUNT1-1,COUNT2,COUNT3))/2.0d0+DX(COUNT1)/     &
               & TCON(MTYPE(COUNT1,COUNT2,COUNT3))/2.0d0)
             END IF
           END IF
!*** Calculate the coefficients CXM and CXP
           CXM(COUNT1,COUNT2,COUNT3)=XK/DX(COUNT1)/DXP(COUNT1-1)
           CXP(COUNT1-1,COUNT2,COUNT3)=XK/DX(COUNT1-1)/DXP(COUNT1-1)
  80     CONTINUE
  90   CONTINUE
 100 CONTINUE
!*** Repeat the previous steps at the Y interface (COUNT2-1/COUNT2)
     DO 200 COUNT1=-NX,NXM1
       DO 190 COUNT2=-NYM1,NYM1
         DO 180 COUNT3=0,NZM1
!*** Calculate the Y interface effective conductivity
           IF (IVINS.EQ.1) THEN
             IF(COUNT3.EQ.0)THEN
               IF(MTYPE(COUNT1,COUNT2,COUNT3).EQ.                       &
                  & MTYPE(COUNT1,COUNT2-1,COUNT3)) THEN
                 YK=TCON(MTYPE(COUNT1,COUNT2,COUNT3))
               ELSE IF(MTYPE(COUNT1,COUNT2,COUNT3).EQ.1.OR.             &
                    & MTYPE(COUNT1,COUNT2-1,COUNT3)&
                 &.EQ.1) THEN
                 YK=DYP(COUNT1-1)/(DY(COUNT2-1)/                        &
                 & TCON(MTYPE(COUNT1,COUNT2-1,COUNT3))/2.0d0+              &
                 & DY(COUNT2)/TCON(MTYPE(COUNT1,COUNT2,COUNT3))/2.0d0+     &
                 & RINS+IVINS*RVINS)
                 IF(MTYPE(COUNT1,COUNT2,COUNT3).EQ.1.)THEN
                     EDGEY(COUNT2)=1
                 ELSE IF(MTYPE(COUNT1,COUNT2-1,COUNT3).EQ.1) THEN
                     EDGEY(COUNT2-1)=1
                 END IF
               ELSE
                 YK=DYP(COUNT2-1)/(DY(COUNT2-1)/                        &
                 & TCON(MTYPE(COUNT1,COUNT2-1,COUNT3))/2.0d0+              &
                 & DY(COUNT2)/TCON(MTYPE(COUNT1,COUNT2,COUNT3))/2.0d0)
               END IF
             ELSE IF(COUNT3.GT.0.AND.ZFACE(COUNT3).LE.ZVINS)THEN
               IF(MTYPE(COUNT1,COUNT2,0).EQ.MTYPE(COUNT1,COUNT2-1,0))THEN
                 YK=TCON(MTYPE(COUNT1,COUNT2,COUNT3))
               ELSE IF (MTYPE(COUNT1,COUNT2,0).EQ.1.OR.                 &
               & MTYPE(COUNT1,COUNT2-1,0).EQ.1) THEN
                 YK=DYP(COUNT2-1)/(DY(COUNT2-1)/                        &
                 & TCON(MTYPE(COUNT1,COUNT2-1,COUNT3))/2.0d0+              &
                 & DY(COUNT2)/TCON(MTYPE(COUNT1,COUNT2,COUNT3))/2.0d0+     &
                 & RINS+IVINS*RVINS)
               ELSE
                 YK=DYP(COUNT2-1)/(DY(COUNT2-1)/                        &
                 & TCON(MTYPE(COUNT1,COUNT2-1,COUNT3))/2.0d0+              &
                 & DY(COUNT2)/TCON(MTYPE(COUNT1,COUNT2,COUNT3))/2.0d0)
               END IF
             ELSE
               YK=DYP(COUNT2-1)/(DY(COUNT2-1)/                          &
               & TCON(MTYPE(COUNT1,COUNT2-1,COUNT3))/2.0d0+                &
               & DY(COUNT2)/TCON(MTYPE(COUNT1,COUNT2,COUNT3))/2.0d0)
             END IF
           ELSE
             IF(MTYPE(COUNT1,COUNT2,COUNT3).EQ.                         &
             & MTYPE(COUNT1,COUNT2-1,COUNT3)) THEN
               YK=TCON(MTYPE(COUNT1,COUNT2,COUNT3))
             ELSE IF(MTYPE(COUNT1,COUNT2,COUNT3).EQ.1.OR.               &
                & MTYPE(COUNT1,COUNT2-1,COUNT3).EQ.1) THEN
               YK=DYP(COUNT2-1)/(DY(COUNT2-1)/                          &
               & TCON(MTYPE(COUNT1,COUNT2-1,COUNT3))/2.0d0+ DY(COUNT2)/    &
               & TCON(MTYPE(COUNT1,COUNT2,COUNT3))/2.0d0+RINS)
             ELSE
               YK=DYP(COUNT2-1)/(DY(COUNT2-1)/                          &
               & TCON(MTYPE(COUNT1,COUNT2-1,COUNT3))/2.0d0+ DY(COUNT2)/    &
               & TCON(MTYPE(COUNT1,COUNT2,COUNT3))/2.0d0)
             END IF
           END IF
!*** Calculate the coefficients CYM and CYP
           CYM(COUNT1,COUNT2,COUNT3)=YK/DY(COUNT2)/DYP(COUNT2-1)
           CYP(COUNT1,COUNT2-1,COUNT3)=YK/DY(COUNT2-1)/DYP(COUNT2-1)
180      CONTINUE
190    CONTINUE
200  CONTINUE
!*** One more time for the Z interface (COUNT3-1/COUNT3)
     DO 300 COUNT1=-NX,NXM1
       DO 290 COUNT2=-NY,NYM1
         DO 280 COUNT3=1,NZM1
!*** Determine the interface effective conductivity
           IF (IVINS.EQ.1)THEN
             IF(COUNT3.EQ.1) THEN
               IF(MTYPE(COUNT1,COUNT2,1).EQ.MTYPE(COUNT1,COUNT2,0)) THEN
                 ZK=TCON(MTYPE(COUNT1,COUNT2,1))
               ELSE IF(MTYPE(COUNT1,COUNT2,0).EQ.1) THEN
                 IF(EDGEX(COUNT1).EQ.1.OR.EDGEY(COUNT2).EQ.1) THEN
                   ZK=DZP(0)/(DZ(0)/TCON(1)+DZ(1)/                      &
                   & TCON(MTYPE(COUNT1,COUNT2,1))/2.0d0+                   &
                   & RINS*INS(COUNT1,COUNT2)+IVINS*RVINS)
                 ELSE
                   ZK=DZP(0)/(DZ(0)/TCON(1)+DZ(1)/                      &
                   & TCON(MTYPE(COUNT1,COUNT2,1))/2.0d0+                   &
                   & RINS*INS(COUNT1,COUNT2))
                 END IF
               END IF
             ELSE IF(COUNT3.GT.1.AND.ZFACE(COUNT3).LT.ZVINS)THEN
               IF(EDGEX(COUNT1).EQ.1.OR.EDGEY(COUNT2).EQ.1.AND.         &
               & MTYPE(COUNT1,COUNT2,0).EQ.1)THEN
                 ZK=DZP(COUNT3-1)/(DZ(COUNT3-1)/                        &
                 & TCON(MTYPE(COUNT1,COUNT2,COUNT3-1))/2.0d0               &
                 & + DZ(COUNT3)/TCON(MTYPE(COUNT1,COUNT2,COUNT3))/2.0d0+   &
                 & IVINS*RVINS)
               ELSE
                 ZK=DZP(COUNT3-1)/(DZ(COUNT3-1)/                        &
                 & TCON(MTYPE(COUNT1,COUNT2,COUNT3-1))/2.0d0               &
                 & + DZ(COUNT3)/TCON(MTYPE(COUNT1,COUNT2,COUNT3))/2.0d0)
               END IF
             ELSE
               IF(MTYPE(COUNT1,COUNT2,COUNT3).EQ.                       &
               & MTYPE(COUNT1,COUNT2,COUNT3-1))THEN
                 ZK=TCON(MTYPE(COUNT1,COUNT2,COUNT3))
               ELSE
                 ZK=DZP(COUNT3-1)/(DZ(COUNT3-1)/                        &
                 & TCON(MTYPE(COUNT1,COUNT2,COUNT3-1))/2.0d0               &
                 & +DZ(COUNT3)/TCON(MTYPE(COUNT1,COUNT2,COUNT3))/2.0d0)
               END IF
             END IF
           ELSE
             IF(COUNT3.EQ.1) THEN
               IF(MTYPE(COUNT1,COUNT2,1).EQ.MTYPE(COUNT1,COUNT2,0)) THEN
                 ZK=TCON(MTYPE(COUNT1,COUNT2,1))
               ELSE IF(MTYPE(COUNT1,COUNT2,0).EQ.1) THEN
                 ZK=DZP(0)/(DZ(0)/TCON(1)+DZ(1)/                        &
                 & TCON(MTYPE(COUNT1,COUNT2,1))/2.0d0+                     &
                 & RINS*INS(COUNT1,COUNT2))
               END IF
             ELSE
               IF(MTYPE(COUNT1,COUNT2,COUNT3).EQ.                       &
                 & MTYPE(COUNT1,COUNT2,COUNT3-1)) THEN
                 ZK=TCON(MTYPE(COUNT1,COUNT2,COUNT3))
               ELSE
                 ZK=DZP(COUNT3-1)/(DZ(COUNT3-1)/                        &
                    & TCON(MTYPE(COUNT1,COUNT2,COUNT3-1))/2.0d0+           &
                    & DZ(COUNT3)/TCON(MTYPE(COUNT1,COUNT2,COUNT3))/2.0d0)
               END IF
             END IF
           END IF
!*** Calculate the coefficients CZM, CZP
           CZM(COUNT1,COUNT2,COUNT3)=ZK/DZ(COUNT3)/DZP(COUNT3-1)
           CZP(COUNT1,COUNT2,COUNT3-1)=ZK/DZ(COUNT3-1)/DZP(COUNT3-1)
280      CONTINUE
290      CONTINUE
300      CONTINUE
     RETURN
END SUBROUTINE CalculateFEMCoeffs

!************************  CALCULATE THE SOLAR AZIMUTH ANGLE  ***************************
SUBROUTINE CalcZenith(THETAZ,COSTHETAZ,WeatherFile)
USE SimData
USE EPWRead
IMPLICIT NONE
!** THIS SUBROUTINE COMPUTES THE SUN'S ZENITH ANGLE FOR EACH HOUR OF
!** EACH DAY OF THE YEAR. THIS VALUE IS USED TO CORRECT THE BEAM
!** RADIATION DATA READ FROM THE BLAST WEATHER FILES TO HORIZONTAL
!** RADIATION VALUES REQUIRED FOR THIS ANALYSIS.

!** VERSION 4.5
!** EDWARD D. CLEMENTS
!** SEPTEMBER 27, 1999

!*** LONG:      LONGITUDE [DEG]
!*** LAT:       LATITUDE [DEG]
!*** MSTD:      STANDARD TIME MERIDIAN [DEG]
!*** THETAZ:    ZENITH ANGLE [RADIANS]
!*** RBEAM:     BEAM RADIATION READ FROM WEATHER FILE
!*** TCORR:     CORRECTION FROM LOCAL STANDARD TIME TO SOLAR TIME [HRS]
!*** DELTA:     SOLAR DECLINATION ANGLE [RADIANS]
!*** ET:        SOLAR "EQUATION OF TIME" [MIN]
!*** B:         CONSTANT USED IN CALCULATING THE EQUATION OF TIME
!*** OMEGA:     HOUR ANGLE [RADIANS]
!*** TSOL:      SOLAR TIME [HRS]

!*** Declarations
     REAL(r64) MSTD,LONG,LAT,RBEAM(24),THETAZ(365,24),B,ET,TCORR,DELTA,TSOL
     REAL(r64) COSTHETAZ(365,24)
!     REAL(r64) PI  ! from DataGlobals
     REAL(r64) OMEGA
!     DATA PI/3.14159265359/
     INTEGER IDAY,IHR,ISS,ISR
     REAL(r64) TDB(24),TWB(24),PBAR(24),HRAT(24),WND(24),RDIF(24),DSNOW(24)
     INTEGER ISNW(24)
     CHARACTER *6 WeatherFile
     MSTD=Site%MSTD
     LONG=Site%LONG
     LAT=Site%Lat

!*** Rewind weather file
!     REWIND (Weather)
!     PRINT *,Weather


!*** Loop through one year of weather data
!     open (unit=Weather,file=WeatherFile//'.txt')

!     READ (Weather,5000) !  Skip first line
!5000 FORMAT (/)

     DO 6000 IDAY=1,365

!*** Read one day of beam radiation values from weather file

       RBEAM(1:24)=WDay(IDAY)%DirNormRad(:,1)
!       READ (Weather,*) TDB,TWB,PBAR,HRAT,WND,RBEAM,RDIF,ISNW,DSNOW

!      READ (Weather,5001)RBEAM
!5001   FORMAT (18(/),3(8F10.6),///)

!*** Extract the beam radiation
 !      RBEAM=TodaysWeather%RBEAM
!     IF (IDAY < 3 )PRINT *,RBEAM
!*** Calculate correction from local standard time to solar time (in hours)
       B=2.d0*PI*(IDAY-81.d0)/364.d0
       ET=9.87d0*SIN(2.d0*B)-7.53d0*COS(B)-1.5d0*SIN(B)
       TCORR=(4.d0*(MSTD-LONG)+ET)/60.d0
!*** Calculate declination (in radians)
       DELTA=PI*(23.45d0*SIN(2.d0*PI*(284.d0+IDAY)/365.d0))/180.d0
!*** Find local sunrise and sunset hours
       ISR=24
       ISS=1
       DO 6001 IHR=1,24
         IF (RBEAM(IHR).NE.0.)THEN
           IF(IHR.LT.ISR)THEN
             ISR=IHR
           ELSE IF (IHR.GT.ISS)THEN
             ISS=IHR
           END IF
         END IF
6001   CONTINUE
!*** Compute raw azimuth values
       DO 6002 IHR=1,24
         TSOL=IHR+TCORR
         IF (IHR.LT.ISR.OR.IHR.GT.ISS)THEN
           THETAZ(IDAY,IHR)=0
         ELSE
           IF (IHR.LT.12)THEN
             OMEGA=(-1.d0*PI/12.d0*(12.d0-TSOL))
           ELSE
             OMEGA=PI/12.d0*(TSOL-12.d0)
           END IF
           THETAZ(IDAY,IHR)=ACOS(COS(DELTA)*COS(PI*LAT/180.)*COS(OMEGA)+SIN(DELTA)* &
           & SIN(PI*LAT/180.d0))
           IF (THETAZ(IDAY,IHR).GT.PiOvr2)THEN
             THETAZ(IDAY,IHR)=PI-THETAZ(IDAY,IHR)
           ELSE
             THETAZ(IDAY,IHR)=THETAZ(IDAY,IHR)
           END IF
         END IF
         COSTHETAZ(IDAY,IHR)=COS(THETAZ(IDAY,IHR))
6002   CONTINUE
6000 CONTINUE
     RETURN
END SUBROUTINE CalcZenith

!***************************  WRITE AN INPUT ECHO FILE  *********************************
SUBROUTINE PrelimOutput(RUNID,WeatherFile,AFLOR,PERIM,NX,NY,NZ,MAXITER)
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   October 31, 1999
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

     !*** PURPOSE OF THIS SUBROUTINE:
     !*** This subroutine writes an input echo file that is useful for debugging.

     !*** METHODOLOGY EMPLOYED:
     !*** Standard EnergyPlus "manager" methodology.

     !*** REFERENCES:
        ! Bahnfleth, W.  1989.  Three-dimensional modelling of heat transfer from slab
        ! floors. Ph.D. diss., University of Illinois at Urbana-Champaign.  Also
        ! published as US Army CERL Technical Manuscript E-89/11.

USE SimData
USE EPWRead, ONLY: LocationName
IMPLICIT NONE

     CHARACTER *5 RUNID       ! Run identifier for this set                     []
     CHARACTER *6 WeatherFile ! Name of the BLAST weather file for this run     []
     CHARACTER *5 EVTR      ! Evapotranspiration Simulation Status Flag         []
     CHARACTER *5 FIXBC     ! Fixed Temperature Lower Boundary Condition Flag   []

     REAL(r64) LONG           ! Site Longitude                                [Degrees]
     REAL(r64) LAT            ! Site Latitude                                 [Degrees]
     REAL(r64) MSTD           ! Site Standard Meridian                        [Degrees]
     REAL(r64) ELEV           ! Site Elevation                                      [m]
     REAL(r64) RINS           ! Slab insulation R value                    [K/(W/m**2)]
     REAL(r64) DINS           ! Slab insulation strip width                         [m]
     REAL(r64) AFLOR          ! Slab area                                        [m**2]
     REAL(r64) PERIM          ! Slab perimeter                                      [m]
     REAL(r64) HBLDG          ! Building Height                                     [m]
     REAL(r64) TIN            ! Indoor air dry bulb temperature                     [C]
     REAL(r64) Z0(2)          ! Surface roughness                                  [cm]
     REAL(r64) EPSLW(2)       ! Surface emissivity                                   []
     REAL(r64) ALBEDO(2)      ! Surface albedo                                       []
     REAL(r64) HIN(2)         ! Indoor convective heat transfer coefficient          []
     REAL(r64) RVINS          ! Foundation wall insulation R value         [K/(W/m**2)]
     REAL(r64) ZVINS          ! Foundation wall insulation depth                    [m]

     INTEGER IBOX        ! Cell number of the edge of the slab (X)              []
     INTEGER JBOX        ! Cell number of the edge of the slab (Y)              []
     INTEGER SHAPE       ! Slab Shape indicator                                 []
     INTEGER NX          ! Number of cells in the X direction (from -NX to NX)  []
     INTEGER NY          ! Number of cells in the Y direction (from -NY to NY)  []
     INTEGER NZ          ! Number of cells in the Z direction                   []
     INTEGER MAXITER     ! Maximum number of iterations allowed for convergence []

     IBOX  =Slab%IBOX
     JBOX  =Slab%JBOX
     SHAPE  =BuildingData%Shape
     EVTR  =BCS%EVTR
     FIXBC =BCS%FIXBC
     LONG  =Site%LONG
     LAT   =Site%LAT
     MSTD  =Site%MSTD
     ELEV  =Site%ELEV
     RINS  =Insul%RINS
     DINS  =Insul%DINS
     RVINS =Insul%RVINS
     ZVINS =Insul%ZVINS
     HBLDG =BuildingData%HBLDG
     TIN   =BuildingData%TINave(6)
     Z0    =SSP%Z0
     EPSLW =SSP%EPSLW
     ALBEDO=SSP%ALBEDO
     HIN   =SSP%HIN

     WRITE(InputEcho,1100) RUNID,TRIM(LocationName),LONG,LAT,MSTD,ELEV
1100 FORMAT( ' 3-D SLAB MODEL INPUT SUMMARY',//,                &
     & '	RUN IDENTIFIER: ',A,/,'	WEATHER FILE ID: ',A,/,    &
     & '	LONGITUDE (DEG): ',F6.1,' LATITUDE (DEG): ',F5.1,/, &
     & '	STANDARD TIME MERIDIAN (DEG): ',F6.1,/,             &
     & '	ELEVATION (M): ',F6.1)
     WRITE(InputEcho,1101) ALBEDO(1),ALBEDO(2),EPSLW(1),        &
     & EPSLW(2),Z0(1),Z0(2),RHO(2),CP(2),TCON(2)
1101 FORMAT(/,' SOIL AND SURFACE PROPERTIES:',//,               &
     & ' ALBEDO:',/,' NO SNOW: ',G10.4,' SNOW: ',G10.4,/,         &
     &' LONG-WAVE EMISSIVITY:',/,' NO SNOW: ',G10.4,             &
     & ' SNOW: ',G10.4,/,' ROUGHNESS LENGTH (CM):',/,            &
     & ' NO SNOW: ',G10.4,'	SNOW: ',G10.4,/,                     &
     & ' DENSITY (KG/M**3): ',G10.4,/,                           &
     & ' SPECIFIC HEAT (J/KG/K): ',G10.4,/,                      &
     & ' THERMAL CONDUCTIVTTY (W/M/K): ',G10.4)
     WRITE(InputEcho,1102) RHO(1),CP(1),TCON(1),RINS,DINS,RVINS,ZVINS, &
     & SHAPE,AFLOR,PERIM,XFACE(-IBOX),XFACE(IBOX),YFACE(-JBOX), &
     & YFACE(JBOX),ZFACE(1), HBLDG
1102 FORMAT(/,' FLOOR MATERIAL PROPERTIES AND DIMENSIONS:',//,  &
     & ' DENSITY: ',G10.4,/,' SPECIFIC HEAT: ',G10.4,/,           &
     & ' THERMAL CONDUCTIVITY: ',G10.4,/,' UNDER-SLAB INSULATION:',   &
     & '	RESISTANCE (M**2 K/W): ',G10.4,' WIDTH (M): ',G10.4,/,     &
     & ' FOUNDATION WALL INSULATION RESISTANCE (M**2 K/W): ',   &
     & G10.4,' DEPTH (m): ', G10.4,/,                             &
     & ' FLOOR SHAPE: ',I1,/,' FLOOR AREA (M**2): ',G10.4,/,     &
     & ' PERIMETER (M): ',G10.4,/,                               &
     & ' MIN X (M): ',G10.4,' MAX X: ',G10.4,/,                   &
     & ' MIN Y (M): ',G10.4,' MAX Y: ',G10.4,/,                   &
     & ' FLOOR THICKNESS (M): ',G10.4,/,' BUILDING HEIGHT (M): ',G10.4)
     WRITE(InputEcho,1103) TIN,HIN(1),HIN(2)
1103 FORMAT(/,' BOUNDARY CONDITIONS:',//,' INSIDE AIR TEMP(June). (C):',   &
     & G10.4,/,' FLOOR-AIR HEAT TRANSFER COEFF. (W/M**2/K): ',/,      &
     & ' Q INTO FLOOR: ',G10.4,' Q OUT OF FLOOR: ',G10.4)

     IF(.not. SameString(EVTR,'FALSE')) THEN
       WRITE(InputEcho,1104)
1104   FORMAT(' POTENTIAL EVAPOTRANSPIRATION SURFACE BOUNDARY')
     ELSE
       WRITE(InputEcho,1105)
1105   FORMAT(' ZERO EVAPORATION SURFACE BOUNDARY')
     END IF

     WRITE(InputEcho,1106)
1106 FORMAT(' NO SHADING OF GROUND BY BUILDING INCLUDED')
     IF(.not. SameString(FIXBC,'FALSE')) THEN
       WRITE(InputEcho,1108)
1108   FORMAT(' FIXED TEMPERATURE LOWER BOUNDARY')
     ELSE
       WRITE(InputEcho,1109)
1109   FORMAT(' ZERO FLUX LOWER BOUNDARY')
     END IF
     WRITE(InputEcho,1110) (XFACE(COUNT1),COUNT1=-NX,NX)
1110 FORMAT( ' CELL X-COORDINATES (M):',/,4(10F7.2,/))
     WRITE(InputEcho,1111) (YFACE(COUNT2),COUNT2=-NY,NY)
1111 FORMAT( ' CELL Y-COORDINATES (M):',/,4(10F7.2,/))
     WRITE(InputEcho,1112) (ZFACE(COUNT3),COUNT3=0,NZ)
1112 FORMAT( ' CELL Z-COORDINATES (M):',/,3(10F7.2,/))
     WRITE(InputEcho,1113) MAXITER
1113 FORMAT('MAXITER= ',I5)
     WRITE(InputEcho,1114) BuildingData%ConvTol
1114 FORMAT('Convergence Tolerance= ',1PG14.3)
     IF (NX == 0 .or. NY == 0 .or. NZ == 0) THEN
       CALL ShowFatalError('PrelimOutput: Improper slab entered.  Describe slab using EquivalentSlab object or similar.')
     ENDIF
!     CLOSE(InputEcho)
     RETURN
END SUBROUTINE PrelimOutput

!*********************  CALCULATE 1D GROUND TEMPERATURE PROFILE  ************************
SUBROUTINE CalcTearth(TG,COSTHETAZ,NZ,DZ,DZP,CVG1D,recalc)
USE SimData
USE EPWRead
USE General
IMPLICIT NONE

!** THIS SUBROUTINE CALCULATES THE 1-D, HOURLY TEMPERATURE
!** DISTRIBUTION IN THE GROUND. IT WRITES ONE YEAR OF  TEMPERATURES,
!** SKY RADIATION, AND SURFACE CONVECTION COEFFICIENTS TO LFN 8

!** VERSION 1.5, 20 JULY, 1999
!** EDWARD CLEMENTS

!** REVISED FROM:
!** VERSION 1.0, 20 JULY, 1988
!** WILLIAM BAHNFLETH

!*** DECLARATIONS:
     REAL(r64) ELEV           ! Site Elevation                                           [m]
     REAL(r64) TCOND          ! Soil thermal conductivity                            [W/m-K]
     REAL(r64) CG             ! Soil specific heat                                  [J/kg-K]
     REAL(r64) ALBEDO(2)      ! Surface albedo                                            []
     REAL(r64) EPSLW(2)       ! Surface emissivity                                        []
     REAL(r64) Z0(2)          ! Surface roughness                                       [cm]
     REAL(r64) DZ(0:35)       ! Array of cell dimensions                                 [m]
     REAL(r64) DZP(0:35)      ! Array of distances between cell centers in the Z dir.    [m]
     REAL(r64) TG(0:35)       ! Soil temperature array                                   [C]
     REAL(r64) CONST(0:35,2)  ! Simplifying constant k/Rho*Cp                            [ ]
     REAL(r64) TDB(24)        ! Air dry bulb temperature                                 [C]
     REAL(r64) TWB(24)        ! Air wet bulb temperature                                 [C]
     REAL(r64) PBAR(24)       ! Barometric pressure                                     [Pa]
     REAL(r64) HRAT(24)       ! Humidity Ratio                                            []
     REAL(r64) WND(24)        ! Wind Speed                                             [m/s]
     REAL(r64) RBEAM(24)      ! Beam Solar Radiation                                [W/m**2]
     REAL(r64) RDIF(24)       ! Diffuse Solar Radiation                             [W/m**2]
     REAL(r64) A(21)          ! Array of variables to go into the tridiagonal matrix [     ]
     REAL(r64) B(21)          ! Array of variables to go into the tridiagonal matrix [     ]
     REAL(r64) C(21)          ! Array of variables to go into the tridiagonal matrix [     ]
     REAL(r64) X(21)          ! Array of solutions to the tridiagonal matrix             [C]
     REAL(r64) R(21)          ! Array of constants to go into the tridiagonal matrix [     ]
     REAL(r64) COSTHETAZ(365,24) ! Array of cosine(solar zenith angles)              [       ]
     REAL(r64) TDEEP          ! Deep ground temperature (if fixed lower temp BC)         [C]
     REAL(r64) GOLD           ! Old value of surface heat conduction into the soil  [W/m**2]
     REAL(r64) AVGWND         ! Daily averaged wind speed                              [m/s]
     REAL(r64) TDBO           ! Previous hour value of dry bulb temperature              [C]
     REAL(r64) RBMO           ! Previous hour value of beam solar radiation         [W/m**2]
     REAL(r64) RDFO           ! Previous hour value of diffuse solar radiation      [W/m**2]
     REAL(r64) ALB            ! Snow-modified value of surface albedo                     []
     REAL(r64) EPS            ! Snow-modified value of surface emissivity                 []
     REAL(r64) ZZER           ! Surface roughness                                         []
     REAL(r64) RSKY           ! Infrared sky radiation                              [W/m**2]
     REAL(r64) PVAP           ! Vapor pressure                                          [Pa]
     REAL(r64) RLW            !
     REAL(r64) RSW            !
     REAL(r64) QCS            !
     REAL(r64) RHOA           ! Air Density                                        [kg/m**3]
     REAL(r64) CPA            ! Air Specific Heat                                   [J/kg-K]
     REAL(r64) DH             ! Stability corrected heat transfer coefficient          [m/s]
     REAL(r64) DW             ! Stability corrected mass transfer coefficient          [m/s]
     REAL(r64) QEV            ! Evaporative heat flux                               [W/m**2]
     REAL(r64) DODPG          ! Delta over delta plus gamma                               []
     REAL(r64) QCL            ! Latent heat loss at the surface                     [W/m**2]
     REAL(r64) GOFT           ! Conduction into the ground                          [W/m**2]
     REAL(r64) TGCVG(0:35)    ! Converged temperature profile in the soil                [C]
     REAL(r64) RHOG           ! Soil Density                                       [kg/m**3]
     REAL(r64) DSNOW(24)      ! Snow depth                                              [cm]

     INTEGER IYR         ! Counter Variable: Years                                   []
     INTEGER IMON        ! Counter Variable: Months                                  []
     INTEGER IDAY        ! Counter Variable: Days                                    []
     INTEGER IHR         ! Counter Variable: Hours                                   []
     INTEGER NZ          ! Number of cells in the Z direction                        []
     INTEGER ISP         ! Modified Snow variable                                    []
     INTEGER COUNT4      ! Dummy Counter                                             []
     INTEGER NDIM(12)    ! Number of days in a month                                 []
     INTEGER ISNW(24)    ! Is there snow in this hour? 1=Yes, 0=No                   []
     INTEGER NUMDAYS     ! Dummy Day counter for use with ThetaZ                     []
     INTEGER MAXYR       ! Maximum number of iterations for the 1D solution          []
     DATA NDIM/31,28,31,30,31,30,31,31,30,31,30,31/, &
          & MAXYR/20/

     CHARACTER *5 EVTR   ! Evapotranspiration Simulation Status Flag                 []
     CHARACTER *5 FIXBC  ! Fixed Temperature Lower Boundary Condition Flag           []
     LOGICAL CVG1D       ! Has the 1D ground temprature solution converged? T/F      []
     REAL(r64) CVG1DFail
     INTEGER Hr
     LOGICAL recalc

     EVTR=BCS%EVTR
     FIXBC=BCS%FIXBC
     CG=CP(2)
     ALBEDO=SSP%ALBEDO
     EPSLW=SSP%EPSLW
     Z0=SSP%Z0
     TG=TGround%TG
     ELEV=Site%ELEV
     TCOND=TCON(2)
     RHOG=RHO(2)

     CVG1D=.FALSE.
!*** For variable definitions, consult the list in the main program
!*** NZ: The number of cell face in the 3-D model.
!*** There are NZ+1 temperatures in the boundary condition file.

!*** Calculate some constants in the the finite difference matrix
     DO 110 COUNT1=1,NZ-1
       CONST(COUNT1,1)=TCOND*3600.d0/RHOG/CG/DZ(COUNT1)/DZP(COUNT1-1)
110  CONTINUE

     DO 120 COUNT1=0,NZ-2
       CONST(COUNT1,2)=TCOND*3600.d0/RHOG/CG/DZ(COUNT1)/DZP(COUNT1)
120  CONTINUE

     CONST(NZ-1,2)=TCOND*7200.d0/RHOG/CG/DZ(NZ-1)/DZ(NZ-1)
!*** For a fixed temperature lower boundary condition, set the boundary value
     IF( SameString(BCS%FIXBC,'TRUE') ) Then
        IF (BCS%TDEEPin .EQ. 0.0) Then
          TDEEP=TG(NZ)
        ELSE
         TDEEP=BCS%TDEEPin
         TG(NZ)=TDEEP
        END IF
      END IF
!*** Estimate the conduction into the ground for the first step of the calculation.
       GOLD=TCOND*(TG(0)-TG(1))/DZP(0)
!*** Position the weather file
!       REWIND (Weather)
!      READ (Weather,1000)
!1000   FORMAT(/)
!*** Time loop:
!*** Years:
       DO 800 IYR=1,MAXYR
!*** At the beginning of each year, update the convergence test temperatures.
         DO 100 COUNT1=0,NZ
           TGCVG (COUNT1) =TG (COUNT1)
100      CONTINUE
!**** Initializing Dummy day counter for use with THETAZ
         NUMDAYS=0
!*** Time Loop, months:
         DO 600 IMON=1,12
!*** Time Loop, days:
           DO 400 IDAY=1,NDIM(IMON)
!**** Dummy day counter for use with THETAZ
             NUMDAYS=NUMDAYS+1
!*** Read one day of weather from the weather file
             TDB(1:24)=WDay(NUMDAYS)%DryBulb(:,1)
             TWB(1:24)=WDay(NUMDAYS)%WetBulb(:,1)
             PBAR(1:24)=WDay(NUMDAYS)%StnPres(:,1)
             HRAT(1:24)=WDay(NUMDAYS)%HumRat(:,1)
             WND(1:24)=WDay(NUMDAYS)%WindSpd(:,1)
             RBEAM(1:24)=WDay(NUMDAYS)%DirNormRad(:,1)
             RDIF(1:24)=WDay(NUMDAYS)%DifHorzRad(:,1)
             ISNW(1:24)=WDay(NUMDAYS)%SnowInd(:,1)
             DO Hr=1,24
               IF (WDay(NUMDAYS)%SnowDepth(Hr,1) < 999.) THEN
                 DSNOW(Hr)=WDay(NUMDAYS)%SnowDepth(Hr,1)
               ELSE
                 DSNOW(Hr)=0.0
               ENDIF
             ENDDO
!             READ(Weather,*) TDB,TWB,PBAR,HRAT,WND,RBEAM,RDIF,ISNW,DSNOW
                  !             READ(Weather,1100) ISNW,TDB,TWB,PBAR,HRAT,WND,RBEAM,RDIF
!1100         FORMAT(33X,24I1,/,6(8F10.6,/),4(6F12.5,/),2(12F6.4,/),&
!             & 3(8F10.5,/),//,5(8F10.6,/),8F10.6)

!*** Calculate the average windspeed for a day
             AVGWND=0.
             DO 150 COUNT1=1,24
               AVGWND=AVGWND+WND(COUNT1)/24.
150          CONTINUE
!*** Time Loop, hours:
             DO 200 IHR=1,24
!*** Save old values of TDB and R for a lagged G(T) calculation.
               IF(IHR.EQ.1) THEN
                 TDBO=TDB(IHR)
                 RBMO=RBEAM(IHR)
                 RDFO=RDIF(IHR)
               ELSE
                 TDBO=TDB(IHR-1)
                 RBMO=RBEAM(IHR-1)
                 RDFO=RDIF(IHR-1)
               END IF
!*** Set the surface properties for this hour.
               ISP=1+ISNW(IHR)
               ALB=ALBEDO(ISP)
               EPS=EPSLW(ISP)
               ZZER=Z0(ISP)

!*** Calculate the properties of the ambient air for this hour.
               CALL CalcAirProps(HRAT(IHR),PBAR(IHR),TDB(IHR),PVAP,RHOA,CPA,DODPG)

!*** Calculate the convective heat and mass transfer coefficients DH and DW
               CALL CalcHeatMassTransCoeffs(ZZER,WND(IHR),AVGWND,TDB(IHR),TG(0),DH,DW)

!*** Set up the coefficient matrix
!*** Interior cells
               DO 160 COUNT1=1,NZ-2
                 COUNT4=COUNT1+1
                 A(COUNT4)=-CONST(COUNT1,1)
                 B(COUNT4)=1.d0 + CONST(COUNT1,1)+CONST(COUNT1,2)
                 C(COUNT4)=-CONST(COUNT1,2)
                 R(COUNT4)=TG(COUNT1)
160            CONTINUE
!*** Lower boundary (2 Cases: Fixed temperature (FIXBC=T)
!*** and zero heat flux (FIXBC=F))
               IF(.not. SameString(FIXBC,'FALSE')) THEN
                 A(NZ)=-CONST(NZ-1,1)
                 B(NZ)=1.d0+ CONST(NZ-1,1)+CONST(NZ-1,2)

                 R(NZ)=CONST(NZ-1,2)*TDEEP+TG(NZ-1)
               ELSE
                 A(NZ)=-CONST(NZ-1,1)
                 B(NZ) =1.d0+CONST (NZ-1,1)
                 R(NZ)=TG(NZ-1)
               END IF
!*** Upper Boundary (Ground Surface)
!*** Calculate G(T)
!*** Infrared radiation
!*** Sky radtiation from the Angstrom / Geiger Equation
               RSKY=StefanBoltzmann*((TDBO+273.15d0)**4)*(0.820d0-0.250d0*EXP(-0.002162d0*PVAP))
!*** Net infrared radiation to the ground
               RLW=EPS*(RSKY-StefanBoltzmann*(TG(0)+273.15d0)**4)
!*** Net solar radiation
               RSW=(1.d0-ALB)*(RBMO+RDFO+RBEAM(IHR)*COSTHETAZ(NUMDAYS,IHR)+RDIF(IHR))/2.0d0
!*** Sensible convective heat loss.  User specified h option added.
               IF (SameString(BCS%USERHFlag,'TRUE') ) Then
                 QCS = BCS%USERH*(TG(0)-TDBO)
               ELSE
!               print *,'rhoa=',rhoa
!               print *,'cpa=',cpa
!               print *,'dh=',dh
!               print *,'tg(0)=',tg(0)
!               print *,'tdbo=',tdbo
                 QCS=RHOA*CPA*DH*(TG(0)-TDBO)
               END IF
!*** Compute latent heat losses if evapotranspiration is on.
               !IF(EVTR) THEN       ! Changed b/c of addition of EPlus input stage
               recalc=.false.
               IF(EVTR.NE.'FALSE') THEN
                 QEV=DODPG*(RLW+RSW-GOLD)
                 QCL=RHOA*CPA*DW*(TDB(IHR)-TWB(IHR))
               ELSE
                 QEV=0.
                 QCL=0.
               END IF
!*** Compute the net heat flux into the ground (G(T))
               GOFT=RSW+RLW-QCS-QEV-QCL
!*** Reset GOLD
               GOLD=GOFT
!*** Compute coefficients for surface cells
               B(1)=1.d0 +CONST(0,2)
               C(1)=-CONST(0,2)
               R(1)=TG(0)+GOFT*3600.d0/RHOG/CG/DZ(0)

!*** Solve the system of equations with a tridiagonal matrix algorithm
               CALL TridiagonalMatrixSolver(A,B,C,X,R,NZ)
               DO 165 COUNT1=0,NZ-1
                 TG(COUNT1) =X(COUNT1+1)

                IF (TG(COUNT1)>200) THEN
                  CALL ShowSevereError('The calculated ground temperature in exceeds 200 C [' // trim(RoundSigDigits(TG(COUNT1),2)) // ' C]')
                  CALL ShowContinueError(' The solution may not be computationally stable. Please verify material properties')
                  IF (EVTR == 'TRUE') THEN
                    CALL ShowContinueError(' Will retry calculations with Evapotranspiration = FALSE')
                    BCS%EVTR='FALSE'
                    CVG1D=.FALSE.
                    recalc=.true.
                    RETURN
                  ELSE
                    CALL ShowFatalError('Program terminates due to preceding condition.')
                  ENDIF
                ENDIF
                IF (TG(COUNT1)<-100) THEN
                  CALL ShowSevereError('The calculated ground temperature exceeds -100 C [' // trim(RoundSigDigits(TG(COUNT1),2)) // ' C]')
                  CALL ShowContinueError(' The solution may not be computationally stable. Please verify material properties')
                  IF (EVTR == 'TRUE') THEN
                    CALL ShowContinueError(' Will retry calculations with Evapotranspiration = FALSE')
                    BCS%EVTR='FALSE'
                    CVG1D=.FALSE.
                    recalc=.true.
                    RETURN
                  ELSE
                    CALL ShowFatalError('Program terminates due to preceding condition.')
                  ENDIF
                ENDIF

165            CONTINUE
                IF(SameString(FIXBC,'FALSE')) TG(NZ)=TG(NZ-1)
!*** If the temperature field has converged, the results are written
!*** (LFN 8) (RSKY, the convective heat and mass transfer coefficients*, and TG(COUNT1))
!*** RHO*CPA*DH & RHO*CPA*DW
               IF(CVG1D) THEN
                 EarthTemp(IHR,NUMDAYS)%RSKY=RSKY
                 EarthTemp(IHR,NUMDAYS)%HHEAT=RHOA*CPA*DH
                 EarthTemp(IHR,NUMDAYS)%HMASS=RHOA*CPA*DW
                 EarthTemp(IHR,NUMDAYS)%DODPG=DODPG
                 EarthTemp(IHR,NUMDAYS)%TG=TG
!                 WRITE(EarthTemp,*) RSKY,RHOA*CPA*DH,RHOA*CPA*DW,DODPG,  &
!                                     (TG(COUNT1),COUNT1=0,NZ)
               END IF
200          CONTINUE
400        CONTINUE
600      CONTINUE
!*** Test for convergence at midnight on December 31st.
         IF(.NOT.CVG1D) THEN
           IF(IYR.EQ.MAXYR-1) THEN
             CVG1D=.TRUE.
             CVG1DFail=0.0
             DO 650 COUNT1=0,NZ
               IF(ABS(TG(COUNT1)-TGCVG(COUNT1)).GT.0.05d0) THEN
                 CVG1D=.FALSE.
                 CVG1DFail=MAX(CVG1DFail,ABS(TG(COUNT1)-TGCVG(COUNT1)))
               ENDIF
650          CONTINUE
             IF (.not. CVG1D) THEN
               CALL ShowSevereError('1D Convergence Fails after '//trim(RoundSigDigits(IYR))//  &
                    ' years.  Max Convergence Diff='//trim(RoundSigDigits(CVG1DFail,5)))
               CALL ShowContinueError('...Convergence should be <= .05, Difference='//  &
                  trim(roundsigdigits(abs(CVG1DFail-.05d0),5)))
               CALL ShowContinueError('1 more year will be simulated to try to gain convergence.')
             ENDIF
           END IF
         ELSE
           RETURN  ! Just return if converged.
         END IF
800    CONTINUE  ! End MaxYr
     IF (.not. CVG1D) THEN
       IF (EVTR == 'TRUE') THEN
         CALL ShowContinueError(' Will retry calculations with Evapotranspiration = FALSE')
         BCS%EVTR='FALSE'
         CVG1D=.FALSE.
         recalc=.true.
       ENDIF
     ENDIF

     RETURN
END SUBROUTINE CalcTearth

!*************************  AIR PROPERTY CALCULATION  ***********************************
SUBROUTINE CalcAirProps(HRAT,PBAR,TDB,PVAP,RHOA,CPA,DODPG)
USE SimData
IMPLICIT NONE

!** THIS SUBROUTINE CALCULATES PROPERTIES OF AIR.
!** VAPOR PRESSURE, DENSITY, AND CONSTANT PRESSURE SPECIFIC HEAT
!** ARE COMPUTED WITH RELATIONS PUBLISHED IN THE ASHRAE HANDBOOK
!** OF FUNDAMENTALS, 1985 SI VERSION. THE EVAPORATION PARAMETER
!** DELTA/(DELTA + GAMMA) IS COMPUTED BY A 2ND ORDER CURVE FIT TO
!** DATA PUBLISHED IN "CONSUMPTIVE USE OF WATER," ASCE, 1973.

!** VERSION 2.0, 20 SEPTEMBER, 1999
!** EDWARD CLEMENTS

!** REVISED FROM:
!** VERSION 1.0, 21 JULY, 1988
!** WM. BAHNFLETH


!*** VARIABLES:
     REAL(r64) PVAP      ! Vapor pressure of ambient air            [Pa]
     REAL(r64) HRAT      ! Humidity ratio of the ambient air          []
     REAL(r64) PBAR      ! Barometric pressure                      [Pa]
     REAL(r64) RHOA      ! Air Density                         [kg/m**3]
     REAL(r64) TDB       ! Outdoor air temperature                   [C]
     REAL(r64) DODPG     ! Delta over delta plus gamma                []
     REAL(r64) ELEV      ! Site elevation                            [m]
     REAL(r64) CPA       ! Air Specific Heat                    [J/kg-K]

     ELEV=Site%ELEV

!*** Initialize the variables
     PVAP=0.
     RHOA=0.
     CPA=0.
     DODPG=0.

     PVAP=(HRAT/(HRAT+0.62198d0))*PBAR
     RHOA=(PBAR-0.3780d0*PVAP)/(287.055d0*(TDB+273.15d0))
     CPA=1007.d0+863d0*PVAP/PBAR
     DODPG=0.395643d0+0.170926d-01*TDB-0.140959d-03*TDB*TDB+0.309091d-04*ELEV+  &
     &  0.822511d-09*ELEV*ELEV-0.472208d-06*TDB*ELEV
     RETURN
END SUBROUTINE CalcAirProps

!*******************  CALC HEAT AND MASS TRANSFER COEFFICIENTS **************************
SUBROUTINE CalcHeatMassTransCoeffs(ZZER,WND,AVGWND,TDB,TG,DH,DW)
USE SimData
IMPLICIT NONE

!*** THIS SUBROUTINE COMPUTES TURBULENT HEAT AND MASS TRANSFER COEFFICIENTS
!*** FOR ONE HOUR USING THE CORELLATION OF SELLERS AND DRYDEN

!*** VERSION 2.0, 10 JULY, 1999
!*** EDWARD CLEMENTS

!*** REVISED FROM:
!*** VERSION 1.0 21 JULY 1988
!*** WM. BAHNFLETH

    REAL(r64), PARAMETER :: onethird=(1.0d0/3.0d0)
    REAL(r64), PARAMETER :: monethird=(-1.0d0/3.0d0)


!*** VARIABLES
     REAL(r64) Z0used         ! Surface roughness                                     [m]
     REAL(r64) ZZER           ! Surface roughness as entered in the input file       [cm]
     REAL(r64) ALGZ0          ! Dummy variable ALOG(Z0used)                           [m]
     REAL(r64) WND            ! Wind Speed                                          [m/s]
     REAL(r64) AVGWND         ! Average Wind Speed                                  [m/s]
     REAL(r64) WND2           ! Wind speed at 2 m height                            [m/s]
     REAL(r64) DM             ! Neutral Stability coefficient                       [m/s]
     REAL(r64) DTV2           ! Stability parameter analogous to Richardson No.        []
     REAL(r64) TG             ! Ground Surface Temperature                            [C]
     REAL(r64) TDB            ! Outdoor air dry bulb temperature                      [C]
     REAL(r64) DH             ! Stability corrected heat transfer coefficient       [m/s]
     REAL(r64) DW             ! Stability corrected mass transfer coefficient       [m/s]

!*** Initialize the variables
     DM=0.
     DTV2=0.
     DH=0.
     DW=0.

!*** Some constants
     Z0used=ZZER/100.d0
     ALGZ0=LOG(2.d0/Z0used)

!*** Estimate the wind speed at a 2 meter height from the 10 meter speed
!*** by using a logarithmic boundary layer assumption.
!*** (If the 10m windspeed is zero, then use the daily average wind speed)

     IF(WND.EQ.0.) THEN
       WND2=AVGWND*ALGZ0/LOG(10.d0/Z0used)
     ELSE
       WND2=WND*ALGZ0/LOG(10.d0/Z0used)
     END IF
     DM=0.164d0*WND2/ALGZ0/ALGZ0
     IF (WND2 > 1.d-6) THEN
       DTV2=(TG-TDB)/WND2/WND2
     ELSE
       DTV2=0.0
     ENDIF

!*** Select the appropriate form for the correction term (either stable or unstable)
!*** and compute the coefficients
     IF(DTV2.GE.0.) THEN
       DH=DM*(1.d0+14.d0*DTV2)**onethird
       DW=DM*(1.d0+10.5d0*DTV2)**onethird
     ELSE
       DH=DM*(1.d0-14.d0*DTV2)**(monethird)
       DW=DM*(1.d0-10.5d0*DTV2)**(monethird)
     END IF
     RETURN
END SUBROUTINE CalcHeatMassTransCoeffs

!***************************  TRIDIAGONAL MATRIX SOLVER  ********************************
SUBROUTINE TridiagonalMatrixSolver(A,B,C,X,R,N)
USE SimData
IMPLICIT NONE

!** THIS SUBROUTINE SOLVES A TRIDIAGONAL SYSTEM OF EQUATIONS BY THE
!** THOMAS ALGORITHM. THIS VERSION IS TAKEN FROM "NUMERICAL MARCHING
!** TECHNIQUES FOR FLUID FLOWS WITH HEAT TRANSFER" BY ROBERT W.
!** HORNBECK, NASA, 1973.

!** A,B,AND C ARE, RESPECTIVELY, THE LOWER, MAJOR, AND UPPER
!** DIAGONAL COEFFICIENT VALUES. FOR A SYSTEM OF N EQUATIONS,
!** INDICES OF A RUN FROM 2 TO N, INDICES OF B FROM 1 TO N, AND
!** INDICES OF C FROM 1 TO N-1. R IS THE RIGHT-HAND SIDE VECTOR
!** OF THE SYSTEM. THE UNKNOWN VECTOR IS RETURNED AS X.

!*** VARIABLES
     REAL(r64) A
     REAL(r64) B
     REAL(r64) C
     REAL(r64) X
     REAL(r64) R
     REAL(r64) BN

     INTEGER N
     INTEGER NZ
     INTEGER COUNT4
     DIMENSION A(N)
     DIMENSION B(N)
     DIMENSION C(N)
     DIMENSION X(N)
     DIMENSION R(N)

     NZ=InitGrid%NZ
     N=NZ

     A(N)=A(N)/B(N)
     R(N)=R(N)/B(N)

     DO 1100 COUNT1=2,N
       COUNT4=-COUNT1+N+2
       BN=1./(B(COUNT4-1)-A(COUNT4)*C(COUNT4-1))
       A(COUNT4-1)=A(COUNT4-1)*BN
       R(COUNT4-1)=(R(COUNT4-1)-C(COUNT4-1)*R(COUNT4))*BN
1100 CONTINUE
     X(1)=R(1)
     DO 1101 COUNT1=2,N
       X(COUNT1)=R(COUNT1)-A(COUNT1)*X(COUNT1-1)
1101 CONTINUE
     RETURN
END SUBROUTINE TridiagonalMatrixSolver


!**************************   INITIALIZE THE 3D TEMPERATURE FIELD  **********************
SUBROUTINE Initialize3D(NX,NY,NXM1,NYM1,NZM1,DZP,T,TCVG,GOFT)
USE SimData
IMPLICIT NONE
     REAL(r64) RSKY           ! Infrared sky radiation                        [W/m**2]
     REAL(r64) HHEAT          ! Convective heat transfer coefficient        [W/m**2-K]
     REAL(r64) HMASS          ! Convective mass transfer coefficient        [W/m**2-K]
     REAL(r64) DODPG          ! Delta over delta plus gamma                         []
     REAL(r64) TG(0:35)       ! 1D Ground temperature profile                      [C]
     REAL(r64) T(-35:35,-35:35,0:35)    ! 3D ground temperature array              [C]
     REAL(r64) TCVG(-35:35,-35:35,0:35) ! 3d converged ground temperature array    [C]
     REAL(r64) GINIT          ! Initial assumption of surface heat conduction [W/m**2]
     REAL(r64) GOFT(-35:35,-35:35,2)    ! Surface heat conduction array       [W/m**2]
     REAL(r64) DZP(0:35)      ! Array of distances between cell centers            [m]

     INTEGER NX,NY       ! Number of cells in the X, Y, and Z directions, resp.[]
     INTEGER NXM1,NYM1,NZM1   ! NX-1, NY-1, and NZ-1, respectively             []

!*** Initialize temperatures in the 3-D domain.
!*** T(X,Y,Z)=TG(Z)(i.e., equal to the boundary condition)
!     READ(EarthTemp,*) RSKY,HHEAT,HMASS,DODPG,(TG(COUNT1),COUNT1=0,NZM1)
     RSKY=EarthTemp(1,1)%RSKY
     HHEAT=EarthTemp(1,1)%HHEAT
     HMASS=EarthTemp(1,1)%HMASS
     DODPG=EarthTemp(1,1)%DODPG
     TG=EarthTemp(1,1)%TG
     DO 220 COUNT1=-NX,NXM1
       DO 219 COUNT2=-NY,NYM1
         DO 218 COUNT3=0,NZM1
           T (COUNT1,COUNT2,COUNT3)=TG (COUNT3)

           TCVG(COUNT1,COUNT2,COUNT3)=TG(COUNT3)
218      CONTINUE
219    CONTINUE
220  CONTINUE

!*** Initialize surface heat fluxes

     GINIT=TCON(2)*(TG(0)-TG(1))/DZP(0)
     GOFT(-NX:NXM1,-NY:NYM1,2)=GINIT
!     DO 222 COUNT1=-NX,NXM1
!       DO 221 COUNT2=-NY,NYM1
!         GOFT(COUNT1,COUNT2,2)=GINIT
!221    CONTINUE
!222  CONTINUE
     RETURN
END SUBROUTINE Initialize3D

!******************************  CHECK IF SYMMETRY CAN BE USED  *************************
SUBROUTINE SymCheck(NXMIN,NYMIN,SYM,SMULT)
USE SimData
IMPLICIT NONE
     INTEGER SHAPE       ! Building shape indicator                             []
     INTEGER NXMIN       ! Minimum X value used for calcs (allows use of sym)   []
     INTEGER NYMIN       ! Minimum Y value used for calcs (allows use of sym)   []
     LOGICAL SYM         ! Symmetry indicator (T/F)                             []
     INTEGER NX          ! Number of cells in the X direction (from -NX to NX)  []
     INTEGER NY          ! Number of cells in the Y direction (from -NY to NY)  []
     REAL(r64) SMULT          ! Multiplication factor for use with symmetry          []
     NX=InitGrid%NX
     NY=InitGrid%NY
     SHAPE=BuildingData%Shape

     IF(SHAPE.EQ.0.) THEN
       NXMIN=0
       NYMIN=0
       SYM=.TRUE.
       SMULT=4.d0
     ELSE
       NXMIN=-NX
       NYMIN=-NY
       SYM=.FALSE.
       SMULT=1.d0
     END IF
     RETURN
END SUBROUTINE SymCheck


!*********************************  SKIP HEADER  ****************************************
SUBROUTINE SkipHeader
USE SimData
IMPLICIT NONE
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   October 12, 1999
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

     !*** PURPOSE OF THIS SUBROUTINE:
     !*** This subroutine will skip the header written into BLAST weather Files.

     !*** METHODOLOGY EMPLOYED:
     !*** Standard EnergyPlus "manager" methodology.

     !*** REFERENCES:
     !*** na

!*** Skip the header row at the beginning of the weather file
     REWIND (Weather)
     READ (Weather,1000)
1000 FORMAT (/)
     RETURN
END SUBROUTINE SkipHeader

!*************************  INITIALIZE OUTPUT VARIABLES  ********************************
SUBROUTINE InitOutVars(NX,NY,NXM1,NYM1,NZM1,TS,QS,TV,TMNA,TMXA,     &
           & TBAR,TMN,TMX,QMNA,QMXA,QBAR,QMN,QMX,TDBA,TDMN,TDMX)

USE SimData
IMPLICIT NONE
     REAL(r64) TMNA      ! Minimum Daily Space averaged floor temperature            [C]
     REAL(r64) TMXA      ! Maximum Daily Space averaged floor temperature            [C]
     REAL(r64) TBAR      ! Time and space averaged floor temperature for a day       [C]
     REAL(r64) TMN       ! Daily minimum floor temperature                           [C]
     REAL(r64) TMX       ! Daily maximum floor temperature                           [C]
     REAL(r64) QMNA      ! Minimum Daily Space averaged floor heat flux         [W/m**2]
     REAL(r64) QMXA      ! Maximum Daily Space averaged floor heat flux         [W/m**2]
     REAL(r64) QBAR      ! Time and space averaged floor heat flux for a day    [W/m**2]
     REAL(r64) QMN       ! Minimum daily floor heat flux                        [W/m**2]
     REAL(r64) QMX       ! Maximum daily floor heat flux                        [W/m**2]
     REAL(r64) TDBA      ! Daily averaged outdoor air dry bulb temperature           [C]
     REAL(r64) TDMN      ! Daily minimum outdoor air dry bulb temperature            [C]
     REAL(r64) TDMX      ! Daily maximum outdoor air dry bulb temperature            [C]
     REAL(r64) TS(-35:35,-35:35)   ! Ground Surface Temperature                      [C]
     REAL(r64) TV(-35:35,0:35)     ! Ground Temperature in the N-S plane             [C]
     REAL(r64) QS(-35:35,-35:35)   ! Surface Heat Flux                          [W/m**2]
     INTEGER NX     ! Number of cells in the X direction (from -NX to NX)        []
     INTEGER NY     ! Number of cells in the Y direction (from -NY to NY)        []
     INTEGER NXM1   ! NX-1                                                       []
     INTEGER NYM1   ! NY-1                                                       []
     INTEGER NZM1   ! Number of cells in the Z direction minus one               []

     TMNA=999.d0
     TMXA=-999.d0
     TBAR=0.d0
     TMN=999.d0
     TMX=-999.d0
     QMNA=999999.d0
     QMXA=-999999.d0
     QBAR=0.d0
     QMN=999999d0
     QMX=-999999d0
     TDBA=0.d0
     TDMN=999.d0
     TDMX=-999.d0
     TS(-NX:NXM1,-NY:NYM1)=0.0
     QS(-NX:NXM1,-NY:NYM1)=0.0
     TV(-NX:NXM1,0:NZM1)=0.0
!     DO 6500 COUNT1=-NX,NXM1
!       DO 6550 COUNT2=-NY,NYM1
!         TS(COUNT1,COUNT2)=0.0
!         QS(COUNT1,COUNT2)=0.0
!6550   CONTINUE
!       DO 6560 COUNT3=0,NZM1
!         TV(COUNT1,COUNT3)=0.0
!6560   CONTINUE
!6500 CONTINUE
     RETURN
END SUBROUTINE InitOutVars

!*********************************  GET WEATHER  ****************************************
SUBROUTINE GetWeather(DayNo)
USE SimData
USE EPWRead
IMPLICIT NONE
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   October 12, 1999
     !***       MODIFIED       April 22, 2001
     !***       RE-ENGINEERED  na

     !*** PURPOSE OF THIS SUBROUTINE:
     !*** This subroutine will retrieve a day of weather from the weather file.

     !*** METHODOLOGY EMPLOYED:
     !*** na

     !*** REFERENCES:
        ! Bahnfleth, W.  1989.  Three-dimensional modelling of heat transfer from slab
        ! floors. Ph.D. diss., University of Illinois at Urbana-Champaign.  Also
        ! published as US Army CERL Technical Manuscript E-89/11.
 INTEGER DayNo


       TodaysWeather%TDB(1:24)=WDay(DayNo)%DryBulb(:,1)
       TodaysWeather%TWB(1:24)=WDay(DayNo)%WetBulb(:,1)
       TodaysWeather%PBAR(1:24)=WDay(DayNo)%StnPres(:,1)
       TodaysWeather%HRAT(1:24)=WDay(DayNo)%HumRat(:,1)
       TodaysWeather%WND(1:24)=WDay(DayNo)%WindSpd(:,1)
       TodaysWeather%RBEAM(1:24)=WDay(DayNo)%DirNormRad(:,1)
       TodaysWeather%RDIF(1:24)=WDay(DayNo)%DifHorzRad(:,1)
       TodaysWeather%ISNW(1:24)=WDay(DayNo)%SnowInd(:,1)
       TodaysWeather%DSNOW(1:24)=WDay(DayNo)%SnowDepth(:,1)
!     READ(Weather,*) TodaysWeather
!1100 FORMAT(33X,24I1,/,6(8F10.6,/),4(6F12.5,/),2(12F6.4,/),&
!            & 3(8F10.5,/),//,5(8F10.6,/),8F10.6)
     RETURN
END SUBROUTINE GetWeather

!**********************  SET THIS HOUR'S SURFACE PROPERTIES  ****************************
SUBROUTINE SetCurrentSurfaceProps(IHR,RBO,RDO)
USE SimData
IMPLICIT NONE

!*** Variables declarations
     REAL(r64) RBO       ! Old value of beam solar radiation                 [W/m**2]
!     REAL RBEAM(24) ! Beam solar radiation                              [W/m**2]
     REAL(r64) RDO       ! Old value of diffuse solar radiation              [W/m**2]
!     REAL RDIF(24)  ! Diffuse solar radiation                           [W/m**2]

     INTEGER IHR    ! Counter variable: hours                                 []

!     RBEAM=TodaysWeather%RBEAM
!     RDIF=TodaysWeather%RDIF

!*** Set old values of beam and diffuse solar radiation
     IF(IHR.GT.1) THEN
       RBO=TodaysWeather%RBEAM(IHR-1)
       RDO=TodaysWeather%RDIF(IHR-1)
     ELSE
       RBO=TodaysWeather%RBEAM(1)
       RDO=TodaysWeather%RDIF(1)
     END IF
     RETURN
END SUBROUTINE SetCurrentSurfaceProps

!**************  SET OLD VALUES OF BEAM AN DIFFUSE RADIATION FOR THIS HOUR  *************
SUBROUTINE SetOldBeamDiffRad(IHR,EPS,ALB)
USE SimData
IMPLICIT NONE

!*** Variables declarations
!     INTEGER ISP         ! Modified snow indicator variable                   []
!     INTEGER ISNW(24)    ! Snow indicator variable 1=Yes,0=No                 []
     INTEGER IHR         ! Counter variable: hours                            []

     REAL(r64) ALB            ! Snow modified surface albedo                       []
     REAL(r64) EPS            ! Snow modified surface emissivity                   []
!     REAL ALBEDO(2)      ! Surface albedo array                               []
!     REAL EPSLW(2)       ! Surface emissivity array                           []

     IF (TodaysWeather%ISNW(IHR) /= 1) THEN
       ALB=SSP%ALBEDO(1)
       EPS=SSP%EPSLW(1)
     ELSE
       ALB=SSP%ALBEDO(2)
       EPS=SSP%EPSLW(2)
     ENDIF
!     ALBEDO=SSP%ALBEDO
!     EPSLW=SSP%EPSLW
!     ISNW=TodaysWeather%ISNW
!     WRITE (68,*) ISNW
!*** Set surface properties for this hour
!     ISP=1+ISNW(IHR)
     !PRINT *,1+ISNW(IHR)
!     EPS=EPSLW(ISP)
!     ALB=ALBEDO(ISP)
     RETURN
END SUBROUTINE SetOldBeamDiffRad

!*******************  CALCULATE THE CURRENT SURFACE HEAT FLUX G(T)  *********************
SUBROUTINE CalcCurrentHeatFlux(NXMIN,NXM1,NYMIN,NYM1,MSURF,T,ALB,       &
           & IHR,RBO,COSTHETAZ,IDAY,RDO,EPS,RSKY,HHEAT,HMASS,DODPG,SYM,GOFT)
USE SimData
IMPLICIT NONE

!*** Variables declarations
     REAL(r64) RGRND          ! Heat radiated from the ground                  [W/m**2]
     REAL(r64) T(-35:35,-35:35,0:35)    ! 3D soil temperature field                 [C]
     REAL(r64) RTOT           ! Total incident radiation on the surface        [W/m**2]
     REAL(r64) RDIF(24)       ! Diffuse solar radiation on the surface         [W/m**2]
     REAL(r64) ALB            ! Snow modified surface albedo                         []
     REAL(r64) RBEAM(24)      ! Beam solar radiation on the surface            [W/m**2]
     REAL(r64) RBO            ! Old value of beam solar radiation              [W/m**2]
     REAL(r64) RDO            ! Old value of diffuse solar radiation           [W/m**2]
     REAL(r64) COSTHETAZ(365,24) ! Array of cosine(solar zenith angles)        [      ]
     REAL(r64) RSKY           ! Infrared sky radiation                         [W/m**2]
     REAL(r64) EPS            ! Snow modified surface emissivity                     []
     REAL(r64) GOFT(-35:35,-35:35,2)    ! Surface heat conduction
                         !         into the ground                        [W/m**2]
     REAL(r64) HHEAT          ! Convective heat transfer coefficient         [W/m**2-K]
     REAL(r64) HMASS          ! Convective mass transfer coefficient         [W/m**2-K]
     REAL(r64) DODPG          ! Delta over delta plus gamma                          []
     REAL(r64) TWB(24)        ! Outdoor air wet bulb temperature                    [C]
     REAL(r64) TDB(24)        ! Outdoor air dry bulb temperature                    [C]

     INTEGER NXMIN       ! Minimum X value for calculations (allows use of sym) []
     INTEGER NXM1        ! Number of cells in the X direction minus 1           []
     INTEGER NYMIN       ! Minimum Y value for calculations (allows use of sym) []
     INTEGER NYM1        ! Number of cells in the Y direction minus 1           []
     INTEGER MSURF(-35:35,-35:35)  ! Array of surface materials properties      []
     INTEGER IDAY        ! Counter variable: days                               []
     INTEGER IHR         ! Counter variable: hours                              []

     CHARACTER *5 EVTR   ! Evapotranspiration Simulation Status Flag            []
     LOGICAL SYM         ! Can symmetry be used in these calculations? T/F      []

     EVTR=BCS%EVTR
     RBEAM=TodaysWeather%RBEAM
     RDIF=TodaysWeather%RDIF
     TWB=TodaysWeather%TWB
     TDB=TodaysWeather%TDB

!*** Loop through exterior surface cells
     DO 250 COUNT1=NXMIN,NXM1
       DO 249 COUNT2=NYMIN,NYM1
         IF(MSURF(COUNT1,COUNT2).EQ.2) THEN
!*** Determine radiative flux to cell surface
           RGRND=StefanBoltzmann*(T(COUNT1,COUNT2,0)+273.15d0)**4
           RTOT=(1.-ALB)*((RBEAM(IHR)+RBO)*COSTHETAZ(IDAY,IHR)+ &
           & RDIF(IHR)+RDO)/2.0d0+EPS*(RSKY-RGRND)
!*** Calculate G(T) for evaporation off/on cases
           GOFT(COUNT1,COUNT2,1)=RTOT-HHEAT*(T(COUNT1,COUNT2,0)-TDB(IHR))
           !IF(EVTR) THEN   ! Changed b/c of addition of EPlus input
           IF (.not. SameString(EVTR,'FALSE'))THEN
             GOFT(COUNT1,COUNT2,1)=GOFT(COUNT1,COUNT2,1)- &
             & DODPG*(RTOT-GOFT(COUNT1,COUNT2,2)) &
             & -HMASS*(TDB(IHR)-TWB(IHR))
           END IF
         END IF
!*** If the domain is symmetric, assign values by reflection
         IF(SYM) THEN
           GOFT(COUNT1,-COUNT2-1,1)=GOFT(COUNT1,COUNT2,1)
           GOFT(-COUNT1-1,COUNT2,1)=GOFT(COUNT1,COUNT2,1)
           GOFT(-COUNT1-1, -COUNT2-1,1)=GOFT (COUNT1,COUNT2,1)
         END IF

249    CONTINUE
250  CONTINUE
     RETURN
END SUBROUTINE CalcCurrentHeatFlux


!********  CALCULATE AN EXPLICIT FINITE DIFFERENCE SOLUTION FOR THE HEAT EQUATION  ******
SUBROUTINE CalcSolutionAndUpdate(NXMIN,NYMIN,NX,NY,NZ,NXM1,NYM1,NZM1,DX,DY,DZ,CXP,CYP,   &
           & CZP,CXM,CYM,CZM,MTYPE,TG,TOLD,GOFT,T,XDIF,YDIF,ZDIF,SYM)
USE SimData
USE General
IMPLICIT NONE

!*** Variables Declarations
     REAL(r64) DX(-35:35)     ! Array of cell dimensions                             [m]
     REAL(r64) DY(-35:35)     ! Array of cell dimensions                             [m]
     REAL(r64) DZ(0:35)       ! Array of cell dimensions                             [m]
     REAL(r64) CXP(-35:35,-35:35,0:35)  ! Array of FDM constants (+X)             [    ]
     REAL(r64) CXM(-35:35,-35:35,0:35)  ! Array of FDM constants (-X)             [    ]
     REAL(r64) CYP(-35:35,-35:35,0:35)  ! Array of FDM constants (+Y)             [    ]
     REAL(r64) CYM(-35:35,-35:35,0:35)  ! Array of FDM constants (-Y)             [    ]
     REAL(r64) CZP(-35:35,-35:35,0:35)  ! Array of FDM constants (+Z)             [    ]
     REAL(r64) CZM(-35:35,-35:35,0:35)  ! Array of FDM constants (-Z)             [    ]
     REAL(r64) XDIF           ! X diffusion term in the heat diffusion equation    [   ]
     REAL(r64) YDIF           ! Y diffusion term in the heat diffusion equation    [   ]
     REAL(r64) ZDIF           ! Z diffusion term in the heat diffusion equation    [   ]
     REAL(r64) TG(0:35)       ! 1D ground temperature profile                        [C]
     REAL(r64) TOLD(-35:35,-35:35,0:35) ! Old values of the 3D temperature profile   [C]
     REAL(r64) GOFT(-35:35,-35:35,2)    ! Conduction into the surface           [W/m**2]
     REAL(r64) TIN            ! Indoor air temperature                               [C]
     REAL(r64) T(-35:35,-35:35,0:35)    ! Current values of the 3D temperature field [C]
     REAL(r64) HIN(2)         ! Indoor convective heat transfer coefficient   [W/m**2-K]
     REAL(r64) HROOM          ! Indoor heat transfer coefficient (selected)   [W/m**2-K]

     INTEGER NX          ! Number of cells in the X direction (from -NX to NX)   []
     INTEGER NY          ! Number of cells in the Y direction (from -NY to NY)   []
     INTEGER NZ          ! Number of cells in the Z direction                    []
     INTEGER NXM1        ! NX-1                                                  []
     INTEGER NYM1        ! NY-1                                                  []
     INTEGER NZM1        ! NZ-1                                                  []
     INTEGER MTYPE(-35:35,-35:35,0:35)  ! Array of domain material types         []
     INTEGER NXMIN
     INTEGER NYMIN

     LOGICAL SYM    ! Can symmetry be used in these calculations? T/F            []


     TIN=BuildingData%TIN
     HIN=SSP%HIN


!*** X-Diffusion term
DO COUNT1=NXMIN,NXM1
  DO COUNT2=NYMIN,NYM1
    DO COUNT3=0,NZM1

     IF(COUNT1.EQ.-NX) THEN
       XDIF=TCON(MTYPE(-NX,COUNT2,COUNT3))*(TG(COUNT3)-TOLD(-NX,COUNT2,     &
            & COUNT3))/DX(-NX)/DX(-NX)+CXP(-NX,COUNT2,COUNT3)*              &
            & (TOLD(-NX+1,COUNT2,COUNT3)-TOLD(-NX,COUNT2,COUNT3))
     ELSE IF(COUNT1.EQ.NXM1) THEN
       XDIF=CXM(NXM1,COUNT2,COUNT3)*(TOLD(NX-2,COUNT2,COUNT3)-              &
            & TOLD(NXM1,COUNT2,COUNT3))+TCON(MTYPE(NXM1,COUNT2,COUNT3))*    &
            & (TG(COUNT3)-TOLD(NXM1,COUNT2,COUNT3))/                        &
            & DX(NXM1)/DX(NXM1)
     ELSE
       XDIF=CXM(COUNT1,COUNT2,COUNT3)*TOLD(COUNT1-1,COUNT2,COUNT3)          &
       & -(CXM(COUNT1,COUNT2,COUNT3)+CXP(COUNT1,COUNT2,COUNT3))*            &
       & TOLD(COUNT1,COUNT2,COUNT3)+CXP (COUNT1,COUNT2,COUNT3)*             &
       & TOLD(COUNT1+1,COUNT2,COUNT3)
     END IF
!*** Y-Diffusion term
     IF(COUNT2.EQ.-NY) THEN
       YDIF=TCON(MTYPE(COUNT1,-NY,COUNT3))*(TG(COUNT3)-                     &
       & TOLD(COUNT1,-NY,COUNT3))/DY(-NY)/DY(-NY)+CYP(COUNT1,-NY,COUNT3)*   &
       & (TOLD(COUNT1,-NY+1,COUNT3)-TOLD(COUNT1,-NY,COUNT3))
     ELSE IF(COUNT2.EQ.NYM1) THEN
       YDIF=CYM(COUNT1,NYM1,COUNT3)*(TOLD(COUNT1,NY-2,COUNT3)-              &
       & TOLD(COUNT1,NYM1,COUNT3))                                          &
       & +TCON(MTYPE(COUNT1,NYM1,COUNT3))*(TG(COUNT3)-                      &
       & TOLD(COUNT1,NYM1,COUNT3))/DY(NYM1)/DY(NYM1)
     ELSE
       YDIF=CYM(COUNT1,COUNT2,COUNT3)*TOLD(COUNT1,COUNT2-1,COUNT3)          &
       & -(CYM(COUNT1,COUNT2,COUNT3)+CYP(COUNT1,COUNT2,COUNT3))*            &
       & TOLD(COUNT1,COUNT2,COUNT3)+CYP(COUNT1,COUNT2,COUNT3)*              &
       & TOLD(COUNT1,COUNT2+1,COUNT3)
     END IF

!*** Z-Diffusion term
     IF(COUNT3.EQ.0) THEN
       IF(MTYPE(COUNT1,COUNT2,0).EQ.2) THEN
         ZDIF=GOFT(COUNT1,COUNT2,1)/DZ(0)+CZP(COUNT1,COUNT2,0)*             &
         & (TOLD(COUNT1,COUNT2,1)-TOLD(COUNT1,COUNT2,0))
       ELSE IF(MTYPE(COUNT1,COUNT2,0).EQ.1) THEN
         IF(TIN.GT.T(COUNT1,COUNT2,0)) THEN
           HROOM=HIN(1)
         ELSE
           HROOM=HIN(2)
         END IF
           ZDIF=HROOM*(TIN-TOLD(COUNT1,COUNT2,0))/DZ(0)+CZP(COUNT1,COUNT2,0)* &
           & (TOLD(COUNT1,COUNT2,1)-TOLD(COUNT1,COUNT2,0))
       END IF
     ELSE IF(COUNT3.EQ.NZM1) THEN
       ZDIF=CZM(COUNT1,COUNT2,NZM1)*(TOLD(COUNT1,COUNT2,NZ-2)-              &
       & TOLD(COUNT1,COUNT2,NZM1))+TCON(MTYPE(COUNT1,COUNT2,NZM1))*2.*      &
       & (TG(NZ)-TOLD(COUNT1,COUNT2,NZM1))/DZ(NZM1)/DZ(NZM1)
     ELSE
       ZDIF=CZM(COUNT1,COUNT2,COUNT3)*TOLD(COUNT1,COUNT2,COUNT3-1)          &
       & -(CZM(COUNT1,COUNT2,COUNT3)+CZP(COUNT1,COUNT2,COUNT3))*            &
       & TOLD(COUNT1,COUNT2,COUNT3)+ CZP(COUNT1,COUNT2,COUNT3)*             &
       & TOLD(COUNT1,COUNT2,COUNT3+1)
     END IF

!*** Update the temperature at this node.
     T(COUNT1,COUNT2,COUNT3)=TOLD(COUNT1,COUNT2,COUNT3)+(XDIF+YDIF+ZDIF)*3600.d0/ &
     & RHO(MTYPE(COUNT1,COUNT2,COUNT3))/CP(MTYPE(COUNT1,COUNT2,COUNT3))


!*** If the domain is symmetric, assign values by reflection.
     IF(SYM) THEN
       T (COUNT1,-COUNT2-1,COUNT3)=T(COUNT1,COUNT2,COUNT3)
       T(-COUNT1-1,COUNT2,COUNT3)=T(COUNT1,COUNT2,COUNT3)
       T(-COUNT1-1,-COUNT2-1,COUNT3)=T(COUNT1,COUNT2,COUNT3)
     END IF

    IF (T(COUNT1,COUNT2,COUNT3)>200) THEN
      CALL ShowSevereError('The calculated ground temperature exceeds 200 C [' // trim(RoundSigDigits(T(COUNT1,COUNT2,COUNT3),2)) // ' C]')
      CALL ShowContinueError(' The solution may not be computationally stable. Please verify material properties')
      CALL ShowFatalError('Program terminates due to preceding condition.')
    ENDIF
    IF (T(COUNT1,COUNT2,COUNT3)<-100) THEN
      CALL ShowSevereError('The calculated ground temperature exceeds -100 C [' // trim(RoundSigDigits(T(COUNT1,COUNT2,COUNT3),2)) // ' C]')
      CALL ShowContinueError(' The solution may not be computationally stable. Please verify material properties')
      CALL ShowFatalError('Program terminates due to preceding condition.')
    ENDIF

    ENDDO
  ENDDO
ENDDO

  GOFT(-NX:NXM1,-NY:NYM1,2)=GOFT(-NX:NXM1,-NY:NYM1,1)
  TOLD(-NX:NYM1,-NY:NYM1,0:NZM1)=T(-NX:NYM1,-NY:NYM1,0:NZM1)

     RETURN
END SUBROUTINE CalcSolutionAndUpdate


!* RESET OLD VALUES OF THE TEMPERATURE FIELD AND SURFACE HEAT FLUX FOR THE NEXT TIME STEP*
SUBROUTINE ResetTOLD_GOFT(NX,NY,NXM1,NYM1,NZM1,T,TOLD,GOFT)
USE SimData
IMPLICIT NONE

!*** Variables declarations
     REAL(r64) GOFT(-35:35,-35:35,2)    ! Surface heat conduction array         [W/m**2]
     REAL(r64) T(-35:35,-35:35,0:35)    ! Domain temperature field                   [C]
     REAL(r64) TOLD(-35:35,-35:35,0:35) ! Previous hour domain temperature field     [C]
     INTEGER NX     ! Number of cells in the X direction (from -NX to NX)        []
     INTEGER NY     ! Number of cells in the Y direction (from -NY to NY)        []
     INTEGER NYM1   ! NY-1                                                       []
     INTEGER NXM1   ! NX-1                                                       []
     INTEGER NZM1   ! Number of cells in the Z direction minus one               []

!*** Reset the TOLD and GOFT(COUNT1,COUNT2) variables.
     GOFT(-NX:NXM1,-NY:NYM1,2)=GOFT(-NX:NXM1,-NY:NYM1,1)
     TOLD(-NX:NYM1,-NY:NYM1,0:NZM1)=T(-NX:NYM1,-NY:NYM1,0:NZM1)
!     DO 303 COUNT1=-NX,NXM1
!       DO 302 COUNT2=-NY,NYM1
!         GOFT(COUNT1,COUNT2,2)=GOFT(COUNT1,COUNT2,1)
!         DO 301 COUNT3=0,NZM1
!           TOLD(COUNT1,COUNT2,COUNT3)=T(COUNT1,COUNT2,COUNT3)
!301      CONTINUE
!302    CONTINUE
!303  CONTINUE
     RETURN
END SUBROUTINE ResetTOLD_GOFT

!*************************** CALCULATE STATISTICS FOR OUTPUT ****************************
SUBROUTINE CalcOutputStats(DX,DY,IHR,NX,NY,NXM1,NYM1,NZM1,NXMIN,NYMIN,MSURF,T,GOFT,     &
& SMULT,DA,AFLOR,IDAY,IMON,TMN,TMX,QMN,QMX,TMNA,TMXA,QMNA,QMXA,TDMN,TDMX,TBAR,QBAR,     &
& TDBA,TV,TS,QS,XC,YC,TG)
USE SimData
IMPLICIT NONE

!*** Variables declarations
     REAL(r64) TSUM      ! Dummy summation variable to be used in temp. averaging      [C]
     REAL(r64) QSUM      ! Dummy summation variable to be used in flux avging     [W/M**2]
     REAL(r64) T(-35:35,-35:35,0:35)    ! Domain temperature field                     [C]
     REAL(r64) TMN       ! Daily minimum floor temperature                             [C]
     REAL(r64) DX(-35:35)     ! Array of cell dimensions                               [m]
     REAL(r64) DY(-35:35)     ! Array of cell dimensions                               [m]
     REAL(r64) DA(-35:35,-35:35)   ! Array of cell areas                            [m**2]
     REAL(r64) TMX       ! Daily maximum floor temperature                             [C]
     REAL(r64) TIN       ! Indoor temperature                                          [C]
     REAL(r64) HROOM     ! Indoor convective heat transfer coefficient          [W/m**2-K]
     REAL(r64) QQ        ! Dummy heat flux variable                               [W/m**2]
     REAL(r64) QMN       ! Minimum daily floor heat flux                          [W/m**2]
     REAL(r64) QMX       ! Maximum daily floor heat flux                          [W/m**2]
     REAL(r64) TMNA      ! Minimum space averaged floor temp for a day                 [C]
     REAL(r64) QMNA      ! Minimum space averaged floor heat flux for a day       [W/m**2]
     REAL(r64) QMXA      ! Maximum space averaged floor heat flux for a day       [W/m**2]
     REAL(r64) AFLOR     ! Slab floor area                                          [m**2]
     REAL(r64) TMXA      ! Maximum space averaged floor temp for a day                 [C]
     REAL(r64) TDMN      ! Minimum daily outdoor air dry bulb temperature              [C]
     REAL(r64) TDMX      ! Maximum daily outdoor air dry bulb temperature              [C]
     REAL(r64) TBAR      ! Time and space averaged floor temperature for a day         [C]
     REAL(r64) QBAR      ! Time and space averaged floor heat flux for a day           [C]
     REAL(r64) TDBA      ! Daily average dry bulb temperature                          [C]
     REAL(r64) TS(-35:35,-35:35)   ! Surface temperature array                         [C]
     REAL(r64) QS(-35:35,-35:35)   ! Surface heat flux array                      [W/m**2]
     REAL(r64) GOFT(-35:35,-35:35,2)    ! Array of conduction into the ground     [W/m**2]
     REAL(r64) TG(0:35)  ! 1D ground temperature profile                               [C]
     REAL(r64) HIN(2)    ! Indoor convective heat transfer coefficient          [W/m**2-K]
     REAL(r64) TDB(24)   ! Indoor dry bulb temperature                                 [C]
     REAL(r64) XC(-35:35)     ! Array of coordinates of cell centers in the X dir.     [m]
     REAL(r64) YC(-35:35)     ! Array of coordinates of cell centers in the Y dir.     [m]
     REAL(r64) TV(-35:35,0:35)     ! Array of temperatures in the N-S plane            [C]
     REAL(r64) SMULT     ! Multiplier that allows the use of symmetry in the domain     []

     INTEGER IDAY   ! Counter variable: days                                       []
     INTEGER IMON   ! Counter variable: months                                     []
     INTEGER IHR    ! Counter variable: hours                                      []
     INTEGER NX     ! Number of cells in the X direction (from -NX to NX)          []
     INTEGER NY     ! Number of cells in the Y direction (from -NY to NY)          []
     INTEGER NXM1   ! NX-1                                                         []
     INTEGER NYM1   ! NY-1                                                         []
     INTEGER NZM1   ! Number of cells in the Z direction minus one                 []
     INTEGER NXMIN  ! Minimum X value used in the calculations (reflects sym)      []
     INTEGER NYMIN  ! Minimum Y value used in the calculations (reflects sym)      []
     INTEGER MSURF(-35:35,-35:35)  ! Array of surface materials types              []
     INTEGER NFDM(12)    ! Number of the first day of the month                    []
     DATA NFDM/1,32,60,91,121,152,182,213,244,274,305,335/

     TIN=BuildingData%TIN
     HIN=SSP%HIN
     TDB=TodaysWeather%TDB

!*** Compute spatial averages (weighted by area) and check for
!*** new local min/max of floor temperature and heat flux.
!*** Create output of this information.
     TSUM=0.
     QSUM=0.
     DO 310 COUNT1=NXMIN,NXM1
       DO 309 COUNT2=NYMIN,NYM1
         IF(MSURF(COUNT1,COUNT2).EQ.1) THEN
           TSUM=TSUM+T(COUNT1,COUNT2,0)*SMULT*DA(COUNT1,COUNT2)
           TMN=MIN(TMN,T(COUNT1,COUNT2,0))
           TMX=MAX(TMX,T(COUNT1,COUNT2,0))
           IF(TIN.GT.T(COUNT1,COUNT2,0)) THEN
             HROOM=HIN(1)
           ELSE
             HROOM=HIN(2)
           END IF
           QQ=HROOM*(TIN-T(COUNT1,COUNT2,0))
           QSUM=QSUM+QQ*SMULT*DA(COUNT1,COUNT2)
           QMN=MIN(QMN,QQ)
           QMX=MAX(QMX,QQ)
         END IF
309    CONTINUE
310    CONTINUE

!*** Check for max/min hourly spatial values.
     TMNA=MIN(TMNA,TSUM/AFLOR)
     TMXA=MAX(TMXA,TSUM/AFLOR)
     QMNA=MIN(QMNA,QSUM/AFLOR)
     QMXA=MAX(QMXA,QSUM/AFLOR)

!*** Check for hourly min/max outdoor dry bulb temperatures.
     TDMN=MIN(TDMN,TDB(IHR))
     TDMX=MAX(TDMX,TDB(IHR))

!*** Calculate daily, spatially averaged floor temperature and heat flux.
     TBAR=TBAR+TSUM/AFLOR/24.
     QBAR=QBAR+QSUM/AFLOR/24.
     TDBA=TDBA+TDB(IHR)/24.

!*** Calculate time-averaged surface and X-Z plane information on
!*** the 21st of each month.
     IF(IDAY.EQ.NFDM(IMON)+20) THEN
       DO 320 COUNT1=-NX,NXM1
!*** Surface Cells
         DO 318 COUNT2=-NY,NYM1
           TS(COUNT1,COUNT2)=TS(COUNT1,COUNT2)+T(COUNT1,COUNT2,0)/24.
           IF(MSURF(COUNT1,COUNT2).EQ.1) THEN
             IF(TIN.GT.T(COUNT1,COUNT2,0)) THEN
               HROOM=HIN(1)
             ELSE
               HROOM = HIN(2)
             END IF
             QS(COUNT1,COUNT2)=QS(COUNT1,COUNT2)+HROOM*(TIN-T(COUNT1,COUNT2,0))/24.
           ELSE IF(MSURF(COUNT1,COUNT2).EQ.2) THEN
             QS(COUNT1,COUNT2)=QS(COUNT1,COUNT2)+GOFT(COUNT1,COUNT2,1)/24.
           END IF
318      CONTINUE
!*** Vertical plane
         DO 319 COUNT3=0,NZM1
           TV(COUNT1,COUNT3)=TV(COUNT1,COUNT3)+T(COUNT1,0,COUNT3)/24.

319    CONTINUE
320    CONTINUE
     END IF
     CALL WriteJan21Data(IHR,IDAY,NX,NXM1,NY,NYM1,XC,YC,TG,T,DX,DY)
     !(IHR,IDAY,NX,NXM1,NY,NYM1,XC,YC,TG)
     RETURN
END SUBROUTINE CalcOutputStats

!*******************************  WRITE JANUARY 21ST DATA  ******************************
SUBROUTINE WriteJan21Data(IHR,IDAY,NX,NXM1,NY,NYM1,XC,YC,TG,T,DX,DY)
USE SimData
IMPLICIT NONE

!*** Variables declarations
     REAL(r64) XH             ! X coordinate variable                                [m]
     REAL(r64) YH             ! Y coordinate variable                                [m]
     REAL(r64) XC(-35:35)     ! Array of coordinates of cell centers in the x dir.   [m]
     REAL(r64) YC(-35:35)     ! Array of coordinates of cell centers in the y dir.   [m]
     REAL(r64) DX(-35:35)     ! Array of cell dimensions                             [m]
     REAL(r64) DY(-35:35)     ! Array of cell dimensions                             [m]
     REAL(r64) TH             ! Temperature variable                                 [C]
     REAL(r64) TG(0:35)       ! 1D Ground temperature profile                        [C]
     REAL(r64) T(-35:35,-35:35,0:35)    ! 3D temperature field                       [C]

     INTEGER IHR         ! Counter variable: hours                               []
     INTEGER IDAY        ! Counter variable: days                                []
     INTEGER NX          ! Number of cells in the X direction (from -NX to NX)   []
     INTEGER NXM1        ! NX-1                                                  []
     INTEGER NY          ! Number of cells in the Y direction (from -NY to NY)   []
     INTEGER NYM1        ! NY-1                                                  []

!*** Make hourly surface temperature distribution output on Jan 21.
     IF(IDAY.EQ.21) THEN
       DO 330 COUNT1=-NX-1,NX
         IF(COUNT1.EQ.-NX-1) THEN
           XH=XC(-NX)-DX(-NX)
         ELSE IF(COUNT1.EQ.NX) THEN
           XH=XC(NXM1)+DX(NXM1)
         ELSE
           XH=XC(COUNT1)
         END IF
         DO 329 COUNT2=-NY-1,NY
           IF(COUNT2.EQ.-NY-1) THEN
             YH=YC(-NY)-DY(-NY)
           ELSE IF(COUNT2.EQ.NY) THEN
             YH=YC(NYM1)+DY(NYM1)
           ELSE
             YH=YC(COUNT2)
           END IF
           IF(COUNT1.EQ.-NX-1.OR.COUNT1.EQ.NX.OR.COUNT2.EQ.-NY-1.OR.COUNT2.EQ.NY) THEN
             TH=TG(0)
           ELSE
             TH=T(COUNT1,COUNT2,0)
           END IF
           WRITE(History,1300) IHR,XH,YH,TH
1300       FORMAT(I3,2(2X,F7.2),2X,F5.1)
329      CONTINUE
330    CONTINUE
     END IF
     RETURN
END SUBROUTINE WriteJan21Data

!******************************  WRITE DAILY OUTPUTS  ***********************************
SUBROUTINE WriteDailyData(IDAY,IMON,NX,NXM1,NY,NYM1,NZM1,TMN,           &
          & TMX,TMNA,TMXA,TBAR,TDMN,TDMX,TDBA,QMN,QMX,QMNA,QMXA,QBAR,   &
          & XC,YC,ZC,TS,QS,TV)
USE SimData
IMPLICIT NONE

!*** Variables declarations

     REAL(r64) TIN       ! Indoor air dry bulb temperature                            [C]
     REAL(r64) TMN       ! Daily minimum floor temperature                            [C]
     REAL(r64) TMX       ! Daily maximum floor temperature                            [C]
     REAL(r64) TMNA      ! Minimum space averaged floor temperature for a day         [C]
     REAL(r64) TMXA      ! Maximum space averaged floor temperature for a day         [C]
     REAL(r64) TBAR      ! Time and space averaged floor temperature for a day        [C]
     REAL(r64) TDMN      ! Minimum daily outdoor dry bulb temperature                 [C]
     REAL(r64) TDMX      ! Maximum daily outdoor dry bulb temperature                 [C]
     REAL(r64) TDBA      ! Average daily outdoor dry bulb temperature                 [C]
     REAL(r64) QMN       ! Daily minimum floor heat flux                         [W/m**2]
     REAL(r64) QMX       ! Daily maximum floor heat flux                         [W/m**2]
     REAL(r64) QMXA      ! Maximum space averaged floor heat flux                [W/m**2]
     REAL(r64) QMNA      ! Minimum space averaged floor heat flux                [W/m**2]
     REAL(r64) QBAR      ! Time and space averaged floor heat flux for a day     [W/m**2]
     REAL(r64) XC(-35:35)     ! Array of cell center coordinates                      [m]
     REAL(r64) YC(-35:35)     ! Array of cell center coordinates                      [m]
     REAL(r64) ZC(0:35)       ! Array of cell center coordinates                      [m]
     REAL(r64) TS(-35:35,-35:35)   ! Surface temperature array                        [C]
     REAL(r64) QS(-35:35,-35:35)   ! Surface heat flux array                     [W/m**2]
     REAL(r64) TV(-35:35,0:35)     ! Temperature array in the N-S plane into the soil [C]

     INTEGER IDAY   ! Counter variable: days                                      []
     INTEGER NX     ! Number of cells in the X direction (from -NX to NX)         []
     INTEGER NY     ! Number of cells in the Y direction (from -NY to NY)         []
     INTEGER NXM1   ! NX-1                                                        []
     INTEGER NYM1   ! NY-1                                                        []
     INTEGER NZM1   ! Number of cells in the Z direction minus one                []
     INTEGER IMON   ! Counter variable: months                                    []
     INTEGER NFDM(12)    ! Number of the first day in each month                  []
     DATA NFDM/1,32,60,91,121,152,182,213,244,274,305,335/

     TIN=BuildingData%TIN

     WRITE(DailyFlux,1200) IDAY,TMN,TMX,TMNA,TMXA,TBAR,TIN,TDMN,TDMX,&
     &  TDBA,QMN,QMX,QMNA,QMXA,QBAR
1200 FORMAT( I4,14(2X,F5.1))
     IF(IDAY.EQ.NFDM(IMON)+20) THEN
       DO 450 COUNT1=-NX,NXM1
         DO 448 COUNT2=-NY,NYM1
           WRITE(FluxDistn,1400) IMON,XC(COUNT1),YC(COUNT2),TS(COUNT1,COUNT2), &
           & QS(COUNT1,COUNT2)
1400       FORMAT( I2,2(2X,F7.2),F5.1,F6.1)
 448     CONTINUE
         DO 449 COUNT3=0,NZM1
           WRITE(TempDistn,1500) IMON,XC(COUNT1),ZC(COUNT3),TV(COUNT1,COUNT3)
1500       FORMAT( I2,2X,F7.2,2X,F6.2,F5.1)
 449     CONTINUE
 450   CONTINUE
     END IF
     RETURN
END SUBROUTINE WriteDailyData

!************************  Connect Input / Output Files  ********************************
SUBROUTINE ConnectIO(RUNID,WeatherFile,EPlus)
! SUBROUTINE INFORMATION:
        !       AUTHOR         Edward D. Clements
        !       DATE WRITTEN   April 5, 2000
        !       MODIFIED       na
        !       MODIFIED       na
        !       RE-ENGINEERED  na

        ! PURPOSE OF THIS MODULE:
        ! This subroutine will connect the input and output files and assign them
        ! unit numbers

        ! METHODOLOGY EMPLOYED:
        ! This subroutine was developed using standard EnergyPlus modular formatting.

        ! MATHEMATIC METHODOLOGY
        ! na

        ! REFERENCES:
        ! na

        ! OTHER NOTES:
        ! na

USE SimData
USE InputProcessor
IMPLICIT NONE

!*** Variables

     CHARACTER *5 RUNID         ! Run identifier for this set                        []
     CHARACTER *6 WeatherFile   ! Name of the BLAST weather file for this run        []
     CHARACTER *5 EPlus         ! Is this simulation to be used with EnergyPlus?     []

!*** Open file for debugging output
     DebugInfo=GetNewUnitNumber()
     OPEN(unit=DebugInfo,file=TRIM(RUNID)//'DBOUT.TXT', STATUS='REPLACE')

!*** Connect the output files
     IF (SameString(EPlus,'FALSE')) THEN
!*** Detailed output files: Non EnergyPlus simulations only
       DailyFlux=GetNewUnitNumber()
         OPEN(unit=DailyFlux,file=RUNID//'DLY.TXT',STATUS='REPLACE')
       History=GetNewUnitNumber()
         OPEN(unit=History,file=RUNID//'HST.TXT',STATUS='REPLACE')
       FluxDistn=GetNewUnitNumber()
         OPEN(unit=FluxDistn,file=RUNID//'MTQ.TXT',STATUS='REPLACE')
       TempDistn=GetNewUnitNumber()
         OPEN(unit=TempDistn,file=RUNID//'MTV.TXT',STATUS='REPLACE')
     END IF

!*** Echo input data to logical unit InputEcho.
     InputEcho=GetNewUnitNumber()
     OPEN(UNIT=InputEcho,FILE=TRIM(RUNID)//'INP.TXT',STATUS='REPLACE')

!*** Open the weather file
!     Weather=GetNewUnitNumber()
!     OPEN(UNIT=Weather,FILE=WeatherFile//'.TXT',STATUS='REPLACE')

!*** Connect the EnergyPlus output files
     SurfaceTemps=GetNewUnitNumber()
       OPEN(UNIT=SurfaceTemps,FILE=TRIM(RUNID)//'SurfaceTemps.TXT',STATUS='REPLACE')
     CLTemps=GetNewUnitNumber()
!       OPEN(UNIT=CLTemps,FILE=TRIM(RUNID)//'CenterlineTemps.TXT',STATUS='SCRATCH')
       OPEN(UNIT=CLTemps,STATUS='SCRATCH')
     SplitSurfTemps=GetNewUnitNumber()
       OPEN(UNIT=SplitSurfTemps,FILE=TRIM(RUNID)//'Split Surface Temps.TXT',STATUS='REPLACE')
     RETURN
END SUBROUTINE ConnectIO

!****************************************************************************************
!*************************  SURFACE TEMPERATURE CALCULATION  ****************************
!****************************************************************************************

!***************  Slab Surface Temperature Calculation for EnergyPlus  ******************
SUBROUTINE SurfTemps(DX,DY,DZ,INS,MTYPE,IBOX,JBOX,T,TSurfFloor,TSFXCL,      &
           & TSFYCL,TSurfFloorPerim,TSurfFloorCore,PerimIndex,CoreArea,     &
           & PerimArea,AFLOR,XC,YC)
! SUBROUTINE INFORMATION:
        !       AUTHOR         Edward D. Clements
        !       DATE WRITTEN   April 4, 2000
        !       MODIFIED       Dec 9, 2001
        !       MODIFIED       na
        !       RE-ENGINEERED  na

        ! PURPOSE OF THIS MODULE:
        ! This subroutine will calculate the outside foundation surface temperatures
        ! required by EnergyPlus for integration with the above grade building model.

        ! METHODOLOGY EMPLOYED:
        ! This subroutine was developed using standard EnergyPlus modular formatting.

        ! MATHEMATIC METHODOLOGY
        ! The surface temperatures are calculated using the pre-calculated three
        ! dimensional temperature field around the building. The temperatures are
        ! calculated at the outside surface using the premise that the heat flux
        ! on both sides of the surface must be equal. q"(i-) = q"(i+), where
        ! q"(k-)=TCON(k-1)*((Ts-T(k-1))/(DZ(k-1)/2)) and
        ! q"(k+)=TCON(k+1)*((T(k+1)-Ts)/(DZ(k+1)/2)) and Ts is the outer foundation
        ! surface temperature

        ! REFERENCES:
        ! na

        ! OTHER NOTES:
        ! na

USE SimData
USE General
IMPLICIT NONE
     REAL(r64) RINS           ! R value of the insulation under the slab         [W/m-C]
     REAL(r64) TSFSum         ! Dummy summation variable for computing time     [m**2-C]
                         ! averaged floor temperature
     REAL(r64) TSFInSum       ! Dummy summation variable for computing time     [m**2-C]
                         ! averaged floor temperature
     REAL(r64) TSurfFloor     ! Area weighted average slab surface temperature       [C]
     REAL(r64) TSFYCL         ! Average temperature on the centerline of the slab    [C]
                         ! X Axis
     REAL(r64) TSFYCLSum      ! Summation varable for calculating TSFYCL       [m**2-C]
     REAL(r64) TSFXCL         ! Average temperature on the centerline of the slab    [C]
                         ! Y Axis
     REAL(r64) TSFXCLSum      ! Summation variable for calculating TSFYCL       [m**2-C]
     REAL(r64) TSF(-35:35,-35:35)  ! Array of surface temperatures beneath slab      [C]
     REAL(r64) T(-35:35,-35:35,0:35)    ! 3D Temperature field from main program     [C]
     REAL(r64) DX(-35:35)     ! Array of X cell dimensions                           [m]
     REAL(r64) DY(-35:35)     ! Array of Y cell dimensions                           [m]
     REAL(r64) DZ(0:35)       ! Array of Z cell dimensions                           [m]


     INTEGER IBOX        ! One index that describes the box in which             []
                         ! the building floor lies (e.g.XFACE(IBOX)
                         ! and XFACE(-IBOX) bound X)
     INTEGER JBOX        ! The other index that describes the box in             []
                         !  which the building floor lies (e.g.YFAC(JBOX)
                         ! and YFACE(-JBOX) bound Y)
     INTEGER INS(-35:35,-35:35) ! Integer indicator for insulated cells          []
                         ! (1 = insulated, 0 = not insulated)
     INTEGER MTYPE(-35:35,-35:35,0:35)  ! Array of material types                []

!*** Variables added 6/5/00 to allow for a split surface temperature calculation
     REAL(r64) TSurfPerimSum  ! Dummy summation variable for calculating the    [m**2-C]
                         ! outside surface temperature in the perimeter zone
     REAL(r64) TSurfCoreSum   ! Dummy summation variable for calculating the    [m**2-C]
                         ! outside surface temperature in the core zone
     REAL(r64) TSurfFloorPerim   ! Perimeter outside surface area weighted avg temp  [C]
     REAL(r64) TSurfFloorCore    ! Core outside surface area weighted average temp   [C]
     INTEGER PerimIndex(-35:35,-35:35)   ! Cell index array for the perim zone   []
     REAL(r64) PerimArea      ! Floor area of the perimeter zone                 [m**2]
     REAL(r64) CoreArea       ! Floor area of the core zone                       [m**2]
     REAL(r64) DINS           ! Width of underfloor insulation strip                 [m]
     REAL(r64) AFLOR          ! Floor area                                        [m**2]
     REAL(r64) DAPerim
     REAL(r64) DACore
     REAL(r64) XC(-35:35)
     REAL(r64) YC(-35:35)
     LOGICAL, SAVE :: perimerror=.false.

     RINS=Insul%RINS
     DINS=Insul%DINS
     IBOX=Slab%IBOX
     JBOX=Slab%JBOX
     TSFSum=0.0
     TSFInSum=0.0
     TSurfPerimSum=0.0
     TSurfCoreSum=0.0
     DAPerim=0.0
     DACore=0.0

!*** Calculate the surface temperature field for the floor slab cells
     DO COUNT1=-IBOX,IBOX-1
       DO COUNT2=-JBOX,JBOX-1
         IF (MTYPE(COUNT1,COUNT2,0).EQ.1) THEN
           TSF(COUNT1,COUNT2)=(T(COUNT1,COUNT2,1)/(RINS*INS(COUNT1,COUNT2)+   &
           & (DZ(1)/2.0d0/TCON(MTYPE(COUNT1,COUNT2,1))))+(T(COUNT1,COUNT2,0)*    &
           & TCON(MTYPE(COUNT1,COUNT2,0))/DZ(0)/2.0d0))/(1/(RINS*                &
           & INS(COUNT1,COUNT2)+DZ(1)/2.0d0/TCON(MTYPE(COUNT1,COUNT2,1)))+       &
           & TCON(MTYPE(COUNT1,COUNT2,0))/DZ(0)/2.0d0)

           TSFSum=TSFSum+TSF(COUNT1,COUNT2)*DX(COUNT1)*DY(COUNT2)
           IF (RINS.NE.0.0) THEN
             IF(INS(COUNT1,COUNT2).EQ.1) THEN
               TSurfPerimSum=TSurfPerimSum+TSF(COUNT1,COUNT2)*DX(COUNT1)*DY(COUNT2)
               PerimIndex(COUNT1,COUNT2)=1
               DAPerim=DAPerim+DX(COUNT1)*DY(COUNT2)
             ELSE
               TSurfCoreSum=TSurfCoreSum+TSF(COUNT1,COUNT2)*DX(COUNT1)*DY(COUNT2)
               PerimIndex(COUNT1,COUNT2)=0
               DACore=DACore+DX(COUNT1)*DY(COUNT2)
             END IF
           ELSE
             IF(ABS(XC(COUNT1)).GT.(XFACE(IBOX)-2.).OR.ABS(YC(COUNT2)).GT.      &
               & (YFACE(JBOX)-2.)) THEN
               TSurfPerimSum=TSurfPerimSum+TSF(COUNT1,COUNT2)*DX(COUNT1)*DY(COUNT2)
               DAPerim=DAPerim+DX(COUNT1)*DY(COUNT2)
               PerimIndex(COUNT1,COUNT2)=1
             ELSE
               TSurfCoreSum=TSurfCoreSum+TSF(COUNT1,COUNT2)*DX(COUNT1)*DY(COUNT2)
               PerimIndex(COUNT1,COUNT2)=0
               DACore=DACore+DX(COUNT1)*DY(COUNT2)
             END IF
           END IF
         END IF
       END DO
     END DO
     TSurfFloor=TSFSum/AFLOR

!*** Compute the separated strip floor areas
     IF (RINS.NE.0) THEN
       CoreArea=2*(XFACE(IBOX)-DINS)*2*(YFACE(JBOX)-DINS)
     ELSE
       CoreArea=2*(XFACE(IBOX)-2.)*2*(YFACE(JBOX)-2.)
     END IF
     PerimArea=AFLOR-CoreArea
     IF (DAPerim < 1.d-10 .and. .not. perimerror) then
       CALL ShowSevereError('SurfTemps: Perimeter is zero, perimeter schedules values will be zero and should not be used.'//  &
          ' Use average or core temperatures in EnergyPlus input files.')
       perimerror=.true.
     ENDIF
     TSurfFloorPerim=SafeDivide(TSurfPerimSum,DAPerim)
     If (CoreArea < 1.0d0) Then
        TSurfFloorCore= TSurfFloorPerim  ! account for user covering most of surface with insulation, Tiny core area
     Else
        TSurfFloorCore=TSurfCoreSum/DACore
     End If
!*** Initialize the variables for the centerline temperature calculation
     TSFYCL=0.0
     TSFYCLSum=0.0
     TSFXCL=0.0
     TSFXCLSum=0.0

!*** Calculate the average floor centerline (along the Y axis)
!*** temperature for comparisons
     DO COUNT1=-IBOX,IBOX-1
       TSFYCLSum=TSFYCLSum+TSF(COUNT1,0)
     END DO
     TSFYCL=TSFYCLSum/(2*IBOX)

!*** Calculate the average floor centerline (along the X axis)
!*** temperature for comparisons
     DO COUNT2=-JBOX,JBOX-1

TSFXCLSum=TSFXCLSum+TSF(0,COUNT2)
     END DO
     TSFXCL=TSFXCLSum/(2*JBOX)
END SUBROUTINE SurfTemps


!***************************     EnergyPlus Output Stage       **************************
SUBROUTINE EPlusOutput(TSurfFloor,TSFXCL,TSFYCL,TSurfFloorPerim,TSurfFloorCore,     &
           & CoreArea,PerimArea,IDAY,IHR,TIN)
! SUBROUTINE INFORMATION:
        !       AUTHOR         Edward D. Clements
        !       DATE WRITTEN   April 3, 2000
        !       MODIFIED       June 14, 2000
        !       MODIFIED       na
        !       RE-ENGINEERED  na

        ! PURPOSE OF THIS MODULE:
        ! This subroutine will write the output files of surface temperatures required
        ! by EnergyPlus

        ! METHODOLOGY EMPLOYED:
        ! This subroutine was developed using standard EnergyPlus modular formatting.

        ! MATHEMATIC METHODOLOGY
        ! na

        ! REFERENCES:
        ! na

        ! OTHER NOTES:
        ! na
USE SimData
IMPLICIT NONE
     REAL(r64) TSurfFloor               ! Area weighted avg floor surface temperature     [C]
     REAL(r64) TSFXCL                   ! Floor surface temperature on the Y centerline   [C]
     REAL(r64) TSFYCL                   ! Floor surface temperature on the X centerline   [C]
     REAL(r64) TSurfFloorPerim          ! Perimeter outside surface area weighted avg T   [C]
     REAL(r64) TSurfFloorCore           ! Core outside surface area weighted average T    [C]
     REAL(r64) CoreArea                 ! Core zone floor area                         [m**2]
     REAL(r64) PerimArea                ! Perimeter zone floor area                    [m**2]
     INTEGER IDAY                  ! Day number counter                               []
     INTEGER IHR                   ! Hour number couneter                             []
     REAL(r64) TIN
!*** WRITE THE SURFACE TEMPERATURES
!*** Floor
     !WRITE (SurfaceTemps,*)IDAY,IHR, TSurfFloor
     WRITE (SplitSurfTemps,*)IDAY,IHR,TSurfFloorPerim,TSurfFloorCore,TIN
     IF (IDAY.EQ.365.AND.IHR.EQ.24) WRITE (SplitSurfTemps,6500) CoreArea,PerimArea
6500 FORMAT('Core zone area: ',F10.2,'    Perimeter zone area: ',F10.2)

!*** Floor Centerline temperatures for comparison
     WRITE (CLTemps,*) TSFXCL,TSFYCL
     RETURN
END SUBROUTINE EPlusOutput

SUBROUTINE MonthlyEPlusOutput(TSurfFloor,TSFXCL,TSFYCL,TSurfFloorPerim,TSurfFloorCore,     &
           & CoreArea,PerimArea,IDAY,IHR,TIN,SlabThickness,Slabk,Insideh)
! SUBROUTINE INFORMATION:
        !       AUTHOR         Edward D. Clements
        !       DATE WRITTEN   April 3, 2000
        !       MODIFIED       June 14, 2000
        !       MODIFIED       C Pedersen Nov 2001
        !       RE-ENGINEERED  na

        ! PURPOSE OF THIS MODULE:
        ! This subroutine will write the output files of monthly surface temperatures required
        ! by EnergyPlus

        ! METHODOLOGY EMPLOYED:
        ! This subroutine was developed using standard EnergyPlus modular formatting.

        ! MATHEMATIC METHODOLOGY
        ! na

        ! REFERENCES:
        ! na

        ! OTHER NOTES:
        ! na
USE SimData
USE EPWRead, ONLY: LocationName
USE General, ONLY: RoundSigDigits

IMPLICIT NONE

     REAL(r64) TSurfFloor               ! Area weighted avg floor surface temperature     [C]
     REAL(r64) TSFXCL                   ! Floor surface temperature on the Y centerline   [C]
     REAL(r64) TSFYCL                   ! Floor surface temperature on the X centerline   [C]
     REAL(r64) TSurfFloorPerim          ! Perimeter outside surface area weighted avg T   [C]
     REAL(r64) TSurfFloorCore           ! Core outside surface area weighted average T    [C]
     REAL(r64) CoreArea                 ! Core zone floor area                         [m**2]
     REAL(r64) PerimArea                ! Perimeter zone floor area                    [m**2]
     REAL(r64) SlabThickness            ! Slab hickness, m
     REAL(r64) Slabk                    ! Slab Thermal Conductivity W/(mK)
     REAL(r64) Insideh(2)               ! Inside heat transfer Coefficient 1 = Qdown, 2= Qup
     REAL(r64) Activeh                  ! heat transfer coefficient matching flux directin
     REAL(r64) CoreFlux                 ! Core heat flux W/m^2
     REAL(r64) PerimFlux                ! Perimeter heat flux W/m^2
     REAL(r64) AveFlux                  ! Average slab heat flux
     REAL(r64) Rtop                     ! Thermal resistance between air and slab bottom.
     INTEGER IDAY                  ! Day number counter                               []
     INTEGER IHR                   ! Hour number counter
     Integer LastDayInMonth(12)       ! Number of days in each month                     []
     Integer DaysInMonth(12)
     REAL, SAVE :: MonthlySurfaceTemp        ! Average surface temperature for month
     REAL,SAVE :: MonthlyCoreTemp            ! Average slab core temp for month
     REAL,SAVE  :: MonthlyPerimeterTemp      ! Average slab perimeter temp for month
     REAL,SAVE  :: MonthlyTempArray(12,3)    ! Saved valuse of the three surface temps i1=ave,i2=perim,i3=core
     INTEGER, SAVE ::  OldCurrentMonth          ! Saved Current month number
     INTEGER CurrentMonth           !  Current Month number
     INTEGER MonthIndex
     INTEGER I
     INTEGER J
     CHARACTER*1 Delimit
     CHARACTER*35 ScheduleName(3)
     CHARACTER(LEN=35) :: surfpOSCName(3)
     CHARACTER(LEN=40) :: stringOut

     Data  LastDayInMonth / 31,59,90,120,151,181,212,243,273,304,334,365/
     Data DaysInMonth / 31,28,31,30,31,30,31,31,30,31,30,31/
     Data MonthlysurfaceTemp /0.0/
     Data MonthlyPerimeterTemp /0.0/
     Data MonthlyCoreTemp /0.0/
     Data OldCurrentMonth /1/
     REAL(r64) TIN
!  Find current month
      Do MonthIndex = 1,12
        IF (IDAY <= LastDayInMonth(MonthIndex)) Then
          CurrentMonth = MonthIndex
          Exit
        End IF
      End Do


     IF (CurrentMonth > OldCurrentMonth) Then
       MonthlySurfaceTemp=TsurfFloor/Float(DaysInMonth(CurrentMonth)*24)  ! First Day in new month
       MonthlyPerimeterTemp=TsurfFloorPerim/Float(DaysInMonth(CurrentMonth)*24)  ! First Day in new month
       MonthlyCoreTemp=TsurfFloorCore/Float(DaysInMonth(CurrentMonth)*24)  ! First Day in new month


       OldCurrentMonth=CurrentMonth
     ELSE
       MonthlySurfaceTemp = MonthlySurfaceTemp + TsurfFloor/Float(DaysInMonth(CurrentMonth)*24)
       MonthlyPerimeterTemp = MonthlyPerimeterTemp + TsurfFloorPerim/Float(DaysInMonth(CurrentMonth)*24)
       MonthlyCoreTemp = MonthlyCoreTemp + TsurfFloorCore/Float(DaysInMonth(CurrentMonth)*24)


     End IF

     !WRITE (SurfaceTemps,*)IDAY,IHR, TSurfFloor  !Origingal Write

     !Write(SurfaceTemps,*)'Debug: Month ',OldCurrentMonth,'  Average Surf Temp ',MonthlySurfaceTemp



    IF(IDAY == LastDayInMonth(OldCurrentMonth) .and. IHR == 24) Then
       Activeh = Insideh(1)  !  downward Q
       If( TIN .LT. MonthlySurfaceTemp) Activeh = Insideh(2)
       Rtop = 1./Activeh + SlabThickness/Slabk
       CoreFlux=(TIN-MonthlyCoreTemp)/Rtop
       PerimFlux = (TIN-MonthlyPerimeterTemp)/Rtop
       AveFlux = (TIN-MonthlySurfaceTemp)/Rtop

! The following lines were present prior to Glazer reworking the output to use specific objects Jan 2010
!
!       If (OldCurrentMonth == 1) Then
!         Write (SurfaceTemps,*)  '================================================'
!         Write (SurfaceTemps,'(A)') ' CHECK CONVERGENCE MESSAGE AT END OF THIS FILE!'
!         Write (SurfaceTemps,*)  '================================================'
!         WRITE (SurfaceTemps,'(A)')'   Weather File Location='//trim(LocationName)
!         Write (SurfaceTemps,*)  '================================================'
!         Write (SurfaceTemps,'(A)')'   Monthly Slab Outside Face Temperatures, C and Heat Fluxes(loss), W/(m^2)'
!         Write (SurfaceTemps,'(A)') ' Perimeter Area: '//TRIM(RoundSigDigits(PerimArea,2))//  &
!                                          '  Core Area: '//TRIM(RoundSigDigits(CoreArea,2))
!         Write (SurfaceTemps,'(A)') '       Month '//'  TAverage '//'  TPerimeter'//'    TCore'//'      TInside' &
!                                      //' AverageFlux '// 'PerimeterFlux '// 'CoreFlux '!
!
!      end if
!      Write(SurfaceTemps,'(9X,I2,2X,2X,A7,3X,A7,6X,A7,5X,A7,4X,A7,5X,A7,5X,A9)') OldCurrentMonth, &
!                         TRIM(RoundSigDigits(MonthlySurfaceTemp,2)),  &
!                         TRIM(RoundSigDigits(MonthlyPerimeterTemp,2)),TRIM(RoundSigDigits(MonthlyCoreTemp,2)), &
!                         TRIM(RoundSigDigits(TIN,2)),TRIM(RoundSigDigits(AveFlux,3)),TRIM(RoundSigDigits(PerimFlux,3)), &
!                         TRIM(RoundSigDigits(CoreFlux,3))
!          MonthlyTempArray(OldCurrentMonth,1) = MonthlySurfaceTemp
!          MonthlyTempArray(OldCurrentMonth,2) = MonthlyPerimeterTemp
!          MonthlyTempArray(OldCurrentMonth,3) = MonthlyCoreTemp
!          ScheduleName(1)='MonthlyAveSurfaceTemp, !Name'
!          ScheduleName(2)='MonthlyPerimeterTemp, !Name'
!          ScheduleName(3)='MonthlyCoreTemp, !Name'
!          IF (OldCurrentMonth == 12)Then
!               Write(SurfaceTemps, '(A,4/)')
!               Write(SurfaceTemps, '(A)') 'OTHER SIDE COEFFICIENT OBJECT EXAMPLE FOR IDF FILE', &
!                   ' SurfaceProperty:OtherSideCoefficients,', &
!                   ' ExampleOSC,                !- OtherSideCoeff Name ***CHANGE THIS!*** ', &
!                   ' 0,                         !- Combined convective/radiative film coefficient',  &
!                   ' 1,                         !- N2,User selected Constant Temperature {C}',  &
!                   ' 1,                         !- Coefficient modifying the user selected constant temperature', &
!                   ' 0,                         !- Coefficient modifying the external dry bulb temperature', &
!                   ' 0,                         !- Coefficient modifying the ground temperature ', &
!                   ' 0,                         !- Coefficient modifying the wind speed term (s/m)', &
!                   ' 0,                         !- Coefficient modifying the zone air temperature ', &
!                   '                            !  part of the equation',  &
!                   ' GroundTempCompactSchedName; !- Name of Schedule for values of const', 	&
!                   '                            ! temperature. Schedule values replace N2.', &
!                   '                            !  ***REPLACE WITH CORRECT NAME***'
!             Do J = 1,3
!               Write(SurfaceTemps,'(//)')
!               Write(SurfaceTemps, '(A)') 'SCHEDULE:COMPACT,', &
!                     ScheduleName(j), &
!                   'Temperature ,            !- ScheduleType'
!               Delimit = ','
!               DO I= 1,12
!                 IF( I == 12) Delimit = ';'
!                 Write(SurfaceTemps,'(A,I4,A,I2,A)')'Through:',I,'/',DaysInMonth(i),','
!                 Write(SurfaceTemps,'(A)')'For:AllDays,'
!                 Write(SurfaceTemps,'(A)')'Until:24:00,'
!                 Write(SurfaceTemps,'(2x,A,A)')TRIM(RoundSigDigits(MonthlyTempArray(I,J),3)),Delimit
!               END DO
!             END DO
!          END IF

      ! Setup for the file that contains the resulting EnergyPlus objects.
      MonthlyTempArray(OldCurrentMonth,1) = MonthlySurfaceTemp
      MonthlyTempArray(OldCurrentMonth,2) = MonthlyPerimeterTemp
      MonthlyTempArray(OldCurrentMonth,3) = MonthlyCoreTemp
      !                1234567890123456789012345678901234567890
      ScheduleName(1)='scheduleOSCSlabAverageSurfaceTemp, '
      ScheduleName(2)='scheduleOSCSlabPerimeterTemp,      '
      ScheduleName(3)='scheduleOSCSlabCoreTemp,           '

      surfpOSCName(1)='surfPropOthSdCoefSlabAverage,      '
      surfpOSCName(2)='surfPropOthSdCoefSlabPerimeter,    '
      surfpOSCName(3)='surfPropOthSdCoefSlabCore,         '

      ! Write the headings of the intermediate file that include the generated EnergyPlus objects.
      IF (OldCurrentMonth == 1) THEN
         WRITE(SurfaceTemps, '(A)') ''
         WRITE(SurfaceTemps, '(A)') '! ========================================================================'
         WRITE(SurfaceTemps, '(A)') '! The following was created by the Slab preprocessor program.'
         WRITE(SurfaceTemps, '(A)') '! Check the convergence message at the end of this file.'
         WRITE(SurfaceTemps, '(A)') '! Weather File Location=' // TRIM(LocationName)
         WRITE(SurfaceTemps, '(A)') '!'
         WRITE(SurfaceTemps, '(A)') '!  Monthly Slab Outside Face Temperatures, C and Heat Fluxes(loss), W/(m^2)'
         WRITE(SurfaceTemps, '(A)') '!  Perimeter Area: '//TRIM(RoundSigDigits(PerimArea,2))
         WRITE(SurfaceTemps, '(A)') '!  Core Area:      '//TRIM(RoundSigDigits(CoreArea,2))
         WRITE(SurfaceTemps, '(A)') '!'
         WRITE(SurfaceTemps, '(A)') '!       Month '//'  TAverage '//'  TPerimeter'//'    TCore'//'      TInside' &
                                      //' AverageFlux '// 'PerimeterFlux '// 'CoreFlux '!
      END IF
      WRITE(SurfaceTemps,'(A,9X,I2,2X,2X,A7,3X,A7,6X,A7,5X,A7,4X,A7,5X,A7,5X,A)') '! ',OldCurrentMonth, &
                         TRIM(RoundSigDigits(MonthlySurfaceTemp,2)),  &
                         TRIM(RoundSigDigits(MonthlyPerimeterTemp,2)),TRIM(RoundSigDigits(MonthlyCoreTemp,2)), &
                         TRIM(RoundSigDigits(TIN,2)),TRIM(RoundSigDigits(AveFlux,3)),TRIM(RoundSigDigits(PerimFlux,3)), &
                         TRIM(RoundSigDigits(CoreFlux,3))
      IF (OldCurrentMonth == 12)Then
        DO J = 1,3
          WRITE(SurfaceTemps, '(A)') ''
          WRITE(SurfaceTemps, '(A)') 'SurfaceProperty:OtherSideCoefficients,'
          WRITE(SurfaceTemps, '(A)') '  ' // surfpOSCName(J) // '  !- Name'
          WRITE(SurfaceTemps, '(A)') '  0.0,                                 !- Combined Convective Radiative Film Coefficient'
          WRITE(SurfaceTemps, '(A)') '  1.0,                                 !- Constant Temperature'
          WRITE(SurfaceTemps, '(A)') '  1.0,                                 !- Constant Temperature Coefficient'
          WRITE(SurfaceTemps, '(A)') '  0.0,                                 !- External Dry-Bulb Temperature Coefficient'
          WRITE(SurfaceTemps, '(A)') '  0.0,                                 !- Ground Temperature Coefficient'
          WRITE(SurfaceTemps, '(A)') '  0.0,                                 !- Wind Speed Coefficient'
          WRITE(SurfaceTemps, '(A)') '  0.0,                                 !- Zone Air Temperature Coefficient'
          WRITE(SurfaceTemps, '(A)') '  ' // ScheduleName(J) // '  !- Constant Temperature Schedule Name'
          WRITE(SurfaceTemps, '(A)') '  No,                                  !- Sinusoidal Variation of Constant Temperature Coefficient'
          WRITE(SurfaceTemps, '(A)') '  24;                                  !- Period of Sinusoidal Variation'
          WRITE(SurfaceTemps, '(A)') ' '
          WRITE(SurfaceTemps, '(A)') 'Schedule:Compact,'
          WRITE(SurfaceTemps, '(A)') '  ' // ScheduleName(J) // '  !- Name'
          WRITE(SurfaceTemps, '(A)') '  Temperature,                         !- ScheduleType'
          Delimit = ','
          DO I = 1 , 12
            IF (I .EQ. 12) Delimit = ';'
            WRITE(SurfaceTemps,'(A,I4,A,I2,A)')'  Through:',I,'/',DaysInMonth(i),',                     !- Field'
            WRITE(SurfaceTemps,'(A)')'  For:AllDays,                         !- Field'
            WRITE(SurfaceTemps,'(A)')'  Until:24:00,                         !- Field'
            WRITE(FMT='(F10.3)', UNIT=stringOut) MonthlyTempArray(I,J)
            WRITE(SurfaceTemps,'(2X,A,A,A)') TRIM(ADJUSTL(stringOut)),Delimit,'                              !- Field'
          END DO
  !       Write(SurfaceTemps,'(3G10.4)')((MonthlyTempArray(i,j),j=1,3),i=1,12)
        END DO
      END IF
      MonthlySurfaceTemp = 0.0
      MonthlyPerimeterTemp=0.0
      MonthlyCoreTemp = 0.0
    END IF






!*** WRITE THE SURFACE TEMPERATURES
!*** Floor

    ! WRITE (SplitSurfTemps,*)IDAY,IHR,TSurfFloorPerim,TSurfFloorCore
    ! IF (IDAY.EQ.365.AND.IHR.EQ.24) WRITE (SplitSurfTemps,6500) CoreArea,PerimArea
!6500 FORMAT('Core zone area: ',F10.2,'    Perimeter zone area: ',F10.2)

!*** Floor Centerline temperatures for comparison
  !   WRITE (CLTemps,*) TSFXCL,TSFYCL
     RETURN
END SUBROUTINE MonthlyEPlusOutput

!***********************  WRITING HEADERS IN THE EPlus OUTPUT FILE  *********************
SUBROUTINE EPlusHeader
! SUBROUTINE INFORMATION:
        !       AUTHOR         Edward D. Clements
        !       DATE WRITTEN   April 4, 2000
        !       MODIFIED       June 14, 2000
        !       MODIFIED       na
        !       RE-ENGINEERED  na

        ! PURPOSE OF THIS MODULE:
        ! This subroutine will format the output files of surface temperatures required
        ! by EnergyPlus

        ! METHODOLOGY EMPLOYED:
        ! This subroutine was developed using standard EnergyPlus modular formatting.

        ! MATHEMATIC METHODOLOGY
        ! na

        ! REFERENCES:
        ! na

        ! OTHER NOTES:
        ! na
IMPLICIT NONE
    ! WRITE (SurfaceTemps,4302)
4302 FORMAT (7X,'Day',7X,'Hour',5X,'Floor TS')
     WRITE (SplitSurfTemps,4303)
4303 FORMAT (7X,'Day',7X,'Hour',3X,'Perim Out Ts', 4X,'Core Out Ts',4X,'Inside Temp')
     WRITE (CLTemps,4304)
4304 FORMAT (7X,'FXCL',12X,'FYCL')
     RETURN
END SUBROUTINE EPlusHeader


!****************************************************************************************
!*******************************  AUTOMATED GRID SELECTION  *****************************
!****************************************************************************************
SUBROUTINE AutoGridding()

USE General
USE DataGlobals, ONLY: ShowWarningError, ShowSevereError, ShowContinueError, ShowFatalError
IMPLICIT NONE
! THIS PROGRAM WILL ESTABLISH THE SIMULATION GRID FOR A SLAB ON GRADE FOUNDATION
! WHOSE DIMENSIONS ARE INPUT BY THE USER

! MODULE INFORMATION:
        !       AUTHOR         Edward D. Clements
        !       DATE WRITTEN   February 08, 2000
        !       MODIFIED       June 12, 2000
        !       MODIFIED       na
        !       RE-ENGINEERED  na

        ! PURPOSE OF THIS MODULE:
        ! This module will establish the simulation grid for a slab on grade foundation
        ! whose dimensions are input by the user.

        ! METHODOLOGY EMPLOYED:
        ! Standard EnergyPlus modular coding

        ! REFERENCES:
        ! na

        ! OTHER NOTES:
        ! na

!*** VARIABLES DECLARATIONS
     REAL(r64) EDGE1          ! X DIRECTION SLAB EDGE                                [m]
     REAL(r64) EDGE1M3        ! X DIRECTION SLAB EDGE - 3 Meters                     [m]
     REAL(r64) EDGE2          ! Y DIRECTION SLAB EDGE                                [m]
     REAL(r64) EDGE2M3        ! Y DIRECTION SLAB EDGE - 3 Meters                     [m]
     REAL(r64) DOMAINEDGEX    ! SOLUTION DOMAIN EDGE (PLACED A CERTAIN DISTANCE      [m]
                         ! AWAY FROM THE SLAB EDGE, TYPICALLY 15m)
     REAL(r64) DOMAINEDGEY    ! SOLUTION DOMAIN EDGE (PLACED A CERTAIN DISTANCE      [m]
                         ! AWAY FROM THE SLAB EDGE, TYPICALLY 15m)
     REAL(r64) ZFACEINIT(0:35) ! ARRAY OF CELL FACE COORDINATES FOR INITIALIZATION   [m]
     !REAL(r64) ZCLEARANCE     ! DISTANCE TO THE LOWER BOUNDARY OF THE DOMAIN         [m]
     REAL(r64) DZINIT(0:35)   ! ARRAY OF CELL DIMENSIONS FOR INITIALIZATION          [m]
     REAL(r64) DZMIN(-35:35,-35:35) ! ARRAY OF MINIMUM Z DIMENSIONS FOR         [m]
                                    ! NUMERICAL STABILITY
     REAL(r64) DZACT(0:35)    ! ARRAY OF CELL DIMENSIONS INCLUDING STABILITY         [m]
     REAL(r64) ZVINS          ! DEPTH OF VERTICAL INSULATION                         [m]
     REAL(r64) MINDZ          ! MINIMUM VALUE OF DZ(0) THAT WILL MEET STABILITY      [m]

     INTEGER NX          ! NUMBER OF CELLS IN THE POSITIVE X DIRECTION           []
                         ! (DOMAIN RUNS FROM -NX TO NX)
     INTEGER NXM1        ! NX-1                                                  []
     INTEGER NX1         ! NUMBER OF CELLS BETWEEN X=0 AND EDGE1M3 (SPACED AT 2m)[]
     INTEGER NX2         ! NUMBER OF CELLS BETWEEN EDGE1M3 AND EDGE-1            []
     INTEGER NX3         ! CELL AT EDGE-0.5m                                     []
     INTEGER NX4         ! CELL AT EDGE-0.2m                                     []
     INTEGER NX5         ! NUMBER OF CELLS BETWEEN EDGE-0.2 AND EDGE+0.2m        []
     INTEGER NX6         ! CELL AT EDGE+0.5m                                     []
     INTEGER NX7         ! CELL AT EDGE+1.0m                                     []
     INTEGER NX8         ! NUMBER OF CELLS BETWEEN EDGE+1 AND EDGE+3             []
     INTEGER NX9         ! NUMBER OF CELLS BETWEEN EDGE+3 AND DOMAINEDGEX        []
     INTEGER NY          ! NUMBER OF CELLS IN THE POSITIVE X DIRECTION           []
                         ! (DOMAIN RUNS FROM -NX TO NX)
     INTEGER NYM1        ! NX-1                                                  []
     INTEGER NY1         ! NUMBER OF CELLS BETWEEN X=0 AND EDGE1M3               []
     INTEGER NY2         ! NUMBER OF CELLS BETWEEN EDGE1M3 AND EDGE-1            []
     INTEGER NY3         ! CELL AT EDGE-0.5m                                     []
     INTEGER NY4         ! CELL AT EDGE-0.2m                                     []
     INTEGER NY5         ! NUMBER OF CELLS BETWEEN EDGE-0.2 AND EDGE+0.2m        []
     INTEGER NY6         ! CELL AT EDGE+0.5m                                     []
     INTEGER NY7         ! CELL AT EDGE+1.0m                                     []
     INTEGER NY8         ! NUMBER OF CELLS BETWEEN EDGE+1 AND EDGE+3             []
     INTEGER NY9         ! NUMBER OF CELLS BETWEEN EDGE+3 AND DOMAINEDGEY        []
     INTEGER NZ1
     INTEGER NZ2
     INTEGER NZ          ! NUMBER OF CELLS IN THE POSITIVE Z DIRECTION           []
                         ! (DOMAIN RUNS FROM 0 TO NZ)                            []
     INTEGER NZM1        ! NZ-1                                                  []
     INTEGER IBOX        ! CELL INDEX OF SLAB EDGE IN THE X DIRECTION            []
                         ! (SLAB IS BOUNDED BY +/- IBOX IN THE X DIRECTION)
     INTEGER JBOX        ! CELL INDEX OF SLAB EDGE IN THE Y DIRECTION            []
                         ! (SLAB IS BOUNDED BY +/- JBOX IN THE Y DIRECTION)
     INTEGER SHAPE       ! SLAB SHAPE INDICATOR. 0=RECTANGULAR (DEFAULT)         []
     INTEGER IVINS       ! INTEGER VERTICAL INSULATION FLAG: 1=YES,0=NO          []
     INTEGER VALIDZVINS  ! CHECK FLAG FOR ZVINS. THIS FORCES THE INSULATION      []
                         ! DEPTH TO BE CONSISTENT WITH THE PREDETERMINED Z
                         ! DIRECTION SOLUTION GRID

     LOGICAL XODD         ! ODD NUMBER FLAG FOR EDGE1M3. USED TO SPACE THE X-GRID    []
                         ! INSIDE THE SLAB.
     LOGICAL YODD         ! ODD NUMBER FLAG FOR EDGE2M3. USED TO SPACE THE Y-GRID    []
                         ! INSIDE THE SLAB.

!     PRINT *,''
!     PRINT *,'YOU HAVE SELECTED TO HAVE THE SOLUTION GRID SIZED AUTOMATICALLY'
!     PRINT *,''
      PRINT *,'Auto Size Grid Selected'

!*** IMPORTANT BUILDING PARAMETERS
     IVINS=Insul%IVINS
     ZVINS=Insul%ZVINS
     SHAPE=BuildingData%SHAPE

!*** MAKE SURE THAT THE VALUE FOR ZVINS IN THE INPUT FILE IS VALID
     IF (IVINS.EQ.1) THEN
       IF (ZVINS.NE.0.2d0.AND.ZVINS.NE.0.4d0.AND.ZVINS.NE.0.6d0.AND.ZVINS.NE.0.8d0.AND. &
       & ZVINS.NE.1d0.AND.ZVINS.NE.1.5d0.AND.ZVINS.NE.2.0d0.AND.ZVINS.NE.2.5d0.AND.     &
       & ZVINS.NE.3.0d0) THEN
         VALIDZVINS=0
       ELSE
         VALIDZVINS=1
       END IF
       DO WHILE (VALIDZVINS.NE.1)
         CALL ShowSevereError('Invalid value for Vertical Insulation Depth=['//  &
           trim(RoundSigDigits(ZVINS,2))//']'// &
             ' should be: .2, .4, .6, .8, 1.0, 1.5, 2.0, 2.5, or 3.0m')
         CALL ShowFatalError('Program terminates due to preceding condition.')
!         PRINT *,'INVALID VALUE FOR VERTICAL INSULATION DEPTH. PLEASE RESELECT.'
!         PRINT *,'(.2, .4, .6, .8, 1.0, 1.5, 2.0, 2.5, OR 3.0m)'
!         READ *,ZVINS
         IF (ZVINS.NE.0.2d0.AND.ZVINS.NE.0.4d0.AND.ZVINS.NE.0.6d0.AND.ZVINS.NE.0.8d0    &
         & .AND.ZVINS.NE.1d0.AND.ZVINS.NE.1.5d0.AND.ZVINS.NE.2.0d0.AND.ZVINS.NE.2.5d0   &
         & .AND.ZVINS.NE.3.0d0) THEN
           VALIDZVINS=0
         ELSE
           VALIDZVINS=1
         END IF
       END DO
     END IF

!*** ASSIGN LOCAL VARIABLE VALUES
     EDGE1=SLABX/2.0d0
     EDGE2=SLABY/2.0d0
     EDGE1M3=EDGE1-3.
     EDGE2M3=EDGE2-3.
     DOMAINEDGEX=EDGE1+CLEARANCE
     DOMAINEDGEY=EDGE2+CLEARANCE
     ! Set the lower boundary for the domain
 !    ZCLEARANCE=MAX(MIN(MAX(SLABX,SLABY)+3.,20.),15.)  ZCLEARAnCE now passed in

!*** SET NX
!*** CHECK TO SEE IF THE SLAB X DIMENSION IS ODD OR EVEN (THIS AFFECTS NX1)
     IF(MOD(EDGE1M3,2.).NE.0.) THEN
       NX1=INT(EDGE1M3/2.0d0+1.001d0)
       XODD=.TRUE.
     ELSE
       NX1=INT(EDGE1M3/2.0d0 +.001d0)
       XODD=.FALSE.
     END IF

!Print *, 'NX1'  ,NX1, 'Edge1M3  ',Edge1M3, XODD

     NX2=2
     NX3=1
     NX4=1
     NX5=2
     NX6=1
     NX7=1
     NX8=1
     NX9=(CLEARANCE-3)/3
     NX=NX1+NX2+NX3+NX4+NX5+NX6+NX7+NX8+NX9+1

!*** SET LOCAL CONSTANTS
     NXM1=NX-1

!*** SET NY
!*** CHECK TO SEE IF THE SLAB Y DIMENSION IS ODD OR EVEN (THIS AFFECTS NY1)
     IF(MOD(EDGE2M3,2.).NE.0.) THEN
       NY1=INT(EDGE2M3/2.0d0+1.001d0)
       YODD=.TRUE.
     ELSE
       NY1=EDGE2M3/2
       YODD=.FALSE.
     END IF

!print *, 'NY1  ',  NY1, '  EDGE2M3  ',EDGE2M3, YODD

     NY2=2
     NY3=1
     NY4=1
     NY5=2
     NY6=1
     NY7=1
     NY8=1
     NY9=(CLEARANCE-3)/3
     NY=NY1+NY2+NY3+NY4+NY5+NY6+NY7+NY8+NY9+1


!*** SET LOCAL CONSTANTS
     NYM1=NY-1


!print *, 'NX1            NX2            NX3           NX4           NX5           NX6           NX7          NX8           NX9'
!PRINT *,  NX1,NX2,NX3,NX4,NX5,NX6,NX7,NX8,NX9







!*** ASSIGN XFACE VALUES
     IBOX=NX1+NX2+NX3+NX4+1
     XFACE(IBOX)=EDGE1
     DO COUNT1=1,NX1
       IF (COUNT1.EQ.1) THEN
         IF (XODD) THEN
           XFACE(COUNT1)=MOD(EDGE1M3,2.)
         ELSE
           XFACE(COUNT1)=2.0
         END IF
       ELSE
         XFACE(COUNT1)=XFACE(COUNT1-1)+2.0
       END IF
     END DO
     DO COUNT1=NX1+1,NX1+NX2
        XFACE(COUNT1)=XFACE(COUNT1-1)+1.0
     END DO
     XFACE(NX1+NX2+NX3)=XFACE(NX1+NX2)+0.5d0
     XFACE(NX1+NX2+NX3+NX4)=XFACE(NX1+NX2+NX3)+0.3d0
     DO COUNT1=IBOX,IBOX+1
        XFACE(COUNT1)=XFACE(COUNT1-1)+0.2d0
     END DO
     XFACE(IBOX+2)=XFACE(IBOX+1)+0.3d0
     XFACE(IBOX+3)=XFACE(IBOX+2)+0.5d0
     DO COUNT1=IBOX+4,IBOX+4+NX8
       XFACE(COUNT1)=XFACE(COUNT1-1)+1.0
     END DO
     DO COUNT1=IBOX+4+NX8+1,IBOX+4+NX8+NX9
       XFACE(COUNT1)=XFACE(COUNT1-1)+3.0
     END DO

     IF (XFACE(IBOX+4+NX8+NX9).GT.DOMAINEDGEX.OR.          &
        & XFACE(IBOX+4+NX8+NX9).LT.DOMAINEDGEX)THEN
       XFACE(IBOX+4+NX8+NX9)=DOMAINEDGEX
     END IF

     DO COUNT1=-NX,0
       IF (COUNT1.EQ.0) THEN
         XFACE(COUNT1)=0.0
       ELSE
         XFACE(COUNT1)=(-1)*XFACE(-1*(COUNT1))
       END IF
     END DO

!  Debug print of XFACE values ******************************************

!Print *, 'XFace Values'
!Print *, ( Count1, XFACE(Count1), Count1 = 1, NX)

!********************************************************

!*** ASSIGN YFACE VALUES
     JBOX=NY1+NY2+NY3+NY4+1
     YFACE(JBOX)=EDGE2
     DO COUNT1=1,NY1
       IF (COUNT1.EQ.1) THEN
         IF (YODD) THEN
            YFACE(COUNT1)=MOD(EDGE2M3,2.)

         ELSE
           YFACE(COUNT1)=2.0
         END IF
       ELSE
         YFACE(COUNT1)=YFACE(COUNT1-1)+2.0
       END IF
     END DO
     DO COUNT1=NY1+1,NY1+NY2
        YFACE(COUNT1)=YFACE(COUNT1-1)+1.0
     END DO
     YFACE(NY1+NY2+NY3)=YFACE(NY1+NY2)+0.5
     YFACE(NY1+NY2+NY3+NY4)=YFACE(NY1+NY2+NY3)+0.3
     DO COUNT1=JBOX,JBOX+1
        YFACE(COUNT1)=YFACE(COUNT1-1)+0.2
     END DO
     YFACE(JBOX+2)=YFACE(JBOX+1)+0.3
     YFACE(JBOX+3)=YFACE(JBOX+2)+0.5
     DO COUNT1=JBOX+4,JBOX+4+NY8
       YFACE(COUNT1)=YFACE(COUNT1-1)+1.0
     END DO
     DO COUNT1=JBOX+4+NY8+1,JBOX+4+NY8+NY9
       YFACE(COUNT1)=YFACE(COUNT1-1)+3.0
     END DO

     IF (YFACE(JBOX+4+NY8+NY9).GT.DOMAINEDGEY.OR.          &
     & YFACE(JBOX+4+NY8+NY9).LT.DOMAINEDGEY)THEN
       YFACE(JBOX+4+NY8+NY9)=DOMAINEDGEY
     END IF

     DO COUNT1=-NY,0
       IF (COUNT1.EQ.0) THEN
         YFACE(COUNT1)=0.0
       ELSE
         YFACE(COUNT1)=(-1)*YFACE(-1*(COUNT1))
       END IF
     END DO
!  Debug print of YFACE values ******************************************

!Print *, 'YFace Values'
!Print *, ( Count1, YFACE(Count1), Count1 = 1, NY)

!********************************************************
!*** ASSIGN INITIAL ZFACE VALUES

!*** SOLVING ZMIN MATRIX
     MINDZ=-999999.
     DO COUNT1=-NX,NXM1
       DO COUNT2=-NY,NYM1
         DZMIN(COUNT1,COUNT2)=(MAX(1/((0.5d0*RHO(2)*                          &
         & CP(2))/(TCON(2)*3600)-1/                                                &
         & ABS(XFACE(COUNT1+1)-XFACE(COUNT1))**2-1/ABS(YFACE(COUNT2+1)-YFACE(COUNT2))**2),0.0))**0.5d0
         MINDZ=MAX(MINDZ,DZMIN(COUNT1,COUNT2) )
       END DO
     END DO

!*** SET NZ
!
     NZ1=MAX(MIN(9,INT(5-4.5*MINDZ)),1)  ! Previously set to 9, but needs to be reduced if stability requires a thicker slab
     NZ2=MAX(MIN(INT((ZCLEARANCE-5.)/2.0d0),INT((ZCLEARANCE-5.)/MINDZ)),1)
     NZ=NZ1+NZ2
     IF(NZ*MINDZ > ZCLEARANCE) THEN
       CALL ShowSevereError('The minimum Z direction cell length [' // trim(RoundSigDigits(MINDZ,2)) // ' m] is too large to fit' //&
         ' a sufficient number of cells within the domain')
       CALL ShowContinueError(' consider increasing the ZCLEARANCE to be greater than' // trim(RoundSigDigits(MINDZ*2,2)) // 'm')
       CALL ShowFatalError('Program terminates due to preceding condition.')
     END IF

!*** SET LOCAL CONSTANTS
     NZM1=NZ-1

     ZFACEINIT(0)=0.0
     ZFACEINIT(1)=ZFACEINIT(0)+SLABDEPTH
     ZFACEINIT(2)=ZFACEINIT(1)+.2d0
     ZFACEINIT(3)=ZFACEINIT(2)+.2d0
     DO COUNT1=4,MIN(6,NZ)
       IF(COUNT1 == NZ .OR. ZFACEINIT(COUNT1-1)+0.5d0 > ZCLEARANCE) THEN
         ZFACEINIT(COUNT1)=ZCLEARANCE
         NZ = COUNT1
         EXIT
       END IF
       ZFACEINIT(COUNT1)=ZFACEINIT(COUNT1-1)+0.5d0
     END DO
     DO COUNT1=MIN(7,NZ),MIN(9,NZ)
       IF(COUNT1 == NZ .OR. ZFACEINIT(COUNT1-1)+1.0 > ZCLEARANCE) THEN
         ZFACEINIT(COUNT1)=ZCLEARANCE
         NZ = COUNT1
         EXIT
       END IF
       ZFACEINIT(COUNT1)=ZFACEINIT(COUNT1-1)+1.0
     END DO
     DO COUNT1=MIN(10,NZ),NZ
       IF(COUNT1 < NZ .AND.ZFACEINIT(COUNT1-1)+2.0 < ZCLEARANCE ) THEN
            ZFACEINIT(COUNT1)=ZFACEINIT(COUNT1-1)+2.0
       ELSE

            ZFACEINIT(COUNT1)=ZCLEARANCE
            NZ = COUNT1
            EXIT
       END IF
     END DO


!*** SETTING UP THE INITIAL DZ MATRIX

     DO COUNT3=0,NZM1
       DZINIT(COUNT3)=0.0
       DZINIT(COUNT3)=ZFACEINIT(COUNT3+1)-ZFACEINIT(COUNT3)
     END DO

!*** ASSIGNING USEABLE ZFACE VALUES
     ZFACE(0)=0.
     IF (DZINIT(0).LT. 1.5d0*MINDZ) THEN
       ZFACE(1)=ZFACE(0)+1.5d0*MINDZ
       IF (SLABDEPTH < ZFACE(1)) THEN
         CALL ShowWarningError(' Slab thickness ['//trim(roundsigdigits(SLABDEPTH,3))//' m] reset to '//  &
            trim(roundsigdigits(ZFACE(1),3))// ' m  for computational stability.', &
            SurfaceTemps,InputEcho)
         PRINT *, ' Slab thickness reset to ',trim(roundsigdigits(ZFACE(1),3)), ' m  for computational stability.'
       SLABDEPTH = ZFACE(1)  !  Slab depth is reset to maintain stability if it's too small.
       ENDIF
       DZACT(0)=1.5d0*MINDZ
     ELSE
       DZACT(0)=DZINIT(0)
       ZFACE(1)=ZFACEINIT(1)
     END IF

     ZFACE(2)=ZFACE(1)+3.d0*MINDZ  !  Set twice as thick as the smallest permissible slab thickness
     DZACT(1)=ZFACE(2)-ZFACE(1)

     IF(NZ>=3) THEN
       DO COUNT3=3,NZ
         ZFACE(COUNT3)=MAX(ZFACEINIT(COUNT3),ZFACE(Count3-1)+2*MINDZ)
         IF (COUNT3.EQ.NZ) ZFACE(COUNT3)=ZCLEARANCE
         DZACT(COUNT3-1)=ZFACE(COUNT3)-ZFACE(COUNT3-1)
       END DO
     END IF

!*** PUT GRID INFORMATION INTO THE PROPER DERIVED TYPES
     InitGrid%NX=NX
     InitGrid%NY=NY
     InitGrid%NZ=NZ
     Slab%IBOX=IBOX
     Slab%JBOX=JBOX
     RETURN
END SUBROUTINE Autogridding

!*************************  GROUND TEMPERATURE INITIALIZATION  **************************
SUBROUTINE InitializeTG(WeatherFile,NDIM)
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   June 7, 2000
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

     !*** PURPOSE OF THIS SUBROUTINE:
     !*** This subroutine will create a 1D ground temperature profile to
     !*** initialize the program automatically,
     !*** NOT included in the weather file.

     !*** METHODOLOGY EMPLOYED:
     !*** Standard EnergyPlus "manager" methodology.

     !*** REFERENCES:
     !*** na
USE SimData
USE EPWRead
IMPLICIT NONE

!*** Variable Declarations
     REAL(r64) TDB(24)               ! Outside air dry bulb temperature                  [C]
     REAL(r64) HTDB(8760)            ! Array of annual dry bulb temperatures             [C]
     REAL(r64) Tm                    ! Average of the monthly average temperatures       [C]
     REAL(r64) TG(0:35)              ! 1D ground temperature initialization array        [C]
     REAL(r64) As                    ! Amplitude of the ground surface temp wave         [C]
     REAL(r64) TempSum               ! Dummy summation variable for averaging            [C]
     REAL(r64) TAVG(12)              ! Monthly average air temperatures                  [C]
     REAL(r64) TAvgMax               ! Maximum monthly average air temperature           [C]
     REAL(r64) TAvgMin               ! Minimum monthly average air temperature           [C]
     REAL(r64) TmSum                 ! Dummy summation variable for averaging            [C]
     REAL(r64) ZFACEUsed(0:35)       ! Dummy array of z direction cell face coordinates  [m]

     CHARACTER *6 WeatherFile   ! Name of the weather file being used
     CHARACTER *1 HeaderLine    !  Dummy variable for skiping header line

     INTEGER IDAY               ! Day counter                                        []
     INTEGER IHR                ! Hour counter (1-24)                                []
     INTEGER HourNum            ! Hour number counter (1-8760)                       []
     INTEGER IMON
     INTEGER NDIM(12)           ! Number of days in each month
     INTEGER TempUnit           ! Temporary unit number
     INTEGER HrStrt
     INTEGER HrEnd

     REAL, PARAMETER ::PI=3.14159265359  ! Physical constant                         []


!*** UNUSED WEATHER VARIABLES
!     REAL TWB(24),PBAR(24),HRAT(24),WND(24),RBEAM(24),RDIFH(24),DSNOW(24)
!     INTEGER ISNW(24)


!*** Open a scratch file
!     TempUnit=GetNewUnitNumber()
!     OPEN(UNIT=TempUnit,FILE='TempInit.TXT',STATUS = 'REPLACE')

!*** Retrieve weather information for the year
!     OPEN(UNIT=Weather,FILE=WeatherFile//'.TXT',STATUS='OLD')
!     REWIND (Weather)
     !***   Skip the header line


!     READ (Weather,*) HeaderLine
     !***


    HrStrt=1
    HrEnd=24
    DO IDAY=1,365
       TDB(:)=WDay(IDAY)%DryBulb(:,1)
!       READ (Weather,*) TDB,TWB,PBAR,HRAT,WND,RBEAM,RDIFH,ISNW,DSNOW
!      READ (Weather,6700) TDB
!6700   FORMAT (/,3(8F10.6,/),19/)

       HTDB(HrStrt:HrEnd)=TDB(:)
       HrStrt=HrStrt+24
       HrEnd=HrEnd+24

!       WRITE(TempUnit,*) (TDB(IHR),IHR=1,24)
     END DO
!     CLOSE(Weather)
!     REWIND(TempUnit)
!     READ (TempUnit,*) HTDB
!     CLOSE (TempUnit,Status='DELETE')
!*** Compute monthly average air temperatures
     HourNum=0.0
     DO IMON=1,12
       TempSum=0.0
       DO IDAY=1,NDIM(IMON)
         DO IHR=1,24
           HourNum=HourNum+1
           TempSum=TempSum+HTDB(HourNum)
         END DO
       TAVG(IMON)=TempSum/(IDAY*24)
       END DO
     END DO
     TmSum=0.0
     TAvgMax=-9999.
     TAvgMin=9999.
     DO IMON=1,12
       TmSum=TmSum+TAVG(IMON)
       TAvgMax=MAX(TAvgMax,TAVG(IMON))
       TAvgMin=MIN(TAvgMin,TAVG(IMON))
     END DO

!*** Compute the average of the average monthly air temperatures
     Tm=TmSum/12

!*** Calculate the annual ground surface temperature wave amplitude
     As=(TAvgMax-TAvgMin)/2.0d0

!*** Set up a dummy cell face coordinate matrix (to prevent errors in the main program)
     DO COUNT1=1,20
       ZFACEUsed(COUNT1)=ZFACE(COUNT1)
       IF(ZFACEUsed(COUNT1).LT.ZFACEUsed(COUNT1-1)) THEN
         ZFACEUsed(COUNT1)=ZFACEUsed(COUNT1-1)
       END IF
!*** Calculate the Temperature profile
       TG(COUNT1)=Tm-As*EXP(-0.4464*ZFACEUsed(COUNT1))*         &
         & COSD(.5236*(-1.-.8525*ZFACEUsed(COUNT1)))
       IF (COUNT1.EQ.20) TG(COUNT1)=Tm
     END DO
!*** Assign the profile into its derived type
     TGround%TG=TG

     RETURN
END SUBROUTINE InitializeTG

REAL(r64) FUNCTION COSD(degree_value)

  REAL(r64) degree_value
  ! Coding this manually since it was a library dependent function in Developer Studio
  COSD = COS(degree_value*pi/180)

END FUNCTION

!******************  CLOSE ALL INPUT AND OUTPUT FILES AND RUN END  **********************
SUBROUTINE CloseIO
!*** SUBROUTINE INFORMATION:
     !***       AUTHOR         Edward D. Clements
     !***       DATE WRITTEN   June 9, 2000
     !***       MODIFIED       na
     !***       RE-ENGINEERED  na

     !*** PURPOSE OF THIS SUBROUTINE:
     !*** This subroutine will close all open files at the end of each run to ensure
     !*** that there are no problems during batching

     !*** METHODOLOGY EMPLOYED:
     !*** Standard EnergyPlus "manager" methodology.

     !*** REFERENCES:
     !*** na
IMPLICIT NONE
!     CLOSE (EarthTemp,Status='DELETE')
!     CLOSE (Weather)
!     CLOSE (InputEcho)
     CLOSE (DailyFlux)
     CLOSE (History)
     CLOSE (FluxDistn)
     CLOSE (TempDistn)
     ! remove status = delete in following to retain debug output
     CLOSE (DebugInfo,STATUS='DELETE')
 !    CLOSE (TempUnit)
     CLOSE (SurfaceTemps)
     CLOSE (CLTemps)
     CLOSE (SplitSurfTemps)
     RETURN
END SUBROUTINE CloseIO


!************************ ENERGYPLUS WEATHER FILE PARSING ROUTINE *******************
SUBROUTINE WeatherServer(WeatherFile,EPWFile)
! MODULE INFORMATION:
      !       AUTHOR         Edward Clements
      !       DATE WRITTEN   April 9, 2001
      !       MODIFIED       April 28,2001
      !       RE-ENGINEERED  na
      !       VERSION NUMBER 1.0
      !
      ! PURPOSE OF THIS MODULE:
      ! This module parses the Energy Plus weather file to recreate a full year
      ! TMY weather file for the foundation heat transfer modules

      ! METHODOLOGY EMPLOYED:
      ! Standard EnergyPlus module formatting.
      ! Derived Types used wherever applicable.

      ! REFERENCES: na

      ! OTHER NOTES: none
USE SimData
USE InputProcessor
USE EPWRead
USE DataGlobals, ONLY: ShowFatalError
IMPLICIT NONE   ! Enforce explicit typing of all variables


!     Variable definitions
!      INTEGER WYEAR,WDAY,WHOUR,WMINUTE,WMONTH,EPWeather
!      INTEGER, DIMENSION(9) :: PRESWEATHCONDS
!      CHARACTER (LEN=20) PRESWEATHCODES
!      CHARACTER (LEN=50) SOURCEJUNK
      CHARACTER (LEN=20) EplusWeather
      CHARACTER(len=120) ErrorMessage
      CHARACTER *6 WeatherFile
      CHARACTER *5 EPWFile
      INTEGER NDays

!     EPlus weather file variables
!      REAL DRYBULB,DEWPOINT,RELHUM,ATMPRESS,ETHORIZ,ETDIRECT,IRHORIZ,GLBHORIZ,DIRECTRAD,  &
!      & DIFFUSERAD,GLBHORIZILLUM,DIRECTNORMILLUM,ZENLUM,WINDDIR,WIND,TOTALSKYCOVER,       &
!      & OPAQUESKYCOVER, VISIBILITY,CEILHEIGHT,PRESWEATHOBS,PRECIPWATER,AEROSOLOPTDEPTH,   &
!      & SNOWDEPTH,DAYSSINCELASTSNOW,DIFFUSEHORIZILLUM,T2,PWS,WSSTAR,SATUPT,PDEW

!     TMY Format weather file variables
!      INTEGER ISNW(24),IHR,IDAY
!      REAL TDB(24),TWB(24),PBAR(24),HRAT(24),WND(24),RBEAM(24),RDIFH(24),DSNOW(24)
      LOGICAL ErrorsFound
      LOGICAL FileExist

      INQUIRE(FILE=TRIM(EPWFile)//'.epw',EXIST=FileExist)
      IF (.not. FileExist) CALL ShowFatalError('No in.epw file found',SurfaceTemps,InputEcho)

      CALL ReadEPW(TRIM(EPWFile)//'.epw',ErrorsFound,ErrorMessage,NDays)

      IF (ErrorsFound) THEN
        PRINT *,trim(ErrorMessage)
        CALL ShowFatalError('Errors found getting weather file. Program Terminates.')
      ELSE
        PRINT*, 'Completed Reading Weather File'
      ENDIF

END SUBROUTINE WeatherServer

END MODULE EPlusSlab3D
!****************************************************************************************
!*****************************  END OF SIMULATION MODULE  *******************************
!****************************************************************************************
