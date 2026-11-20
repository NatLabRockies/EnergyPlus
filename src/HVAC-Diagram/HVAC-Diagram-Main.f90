PROGRAM Diagram
          ! MODULE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS MODULE:
          ! Converts the EnergyPlus BND file into a visual diagram of the HVAC
          ! objects present.  The diagram shows the name of the objects, the
          ! type of object and uses lines to show how it is connected to other
          ! objects.

          ! METHODOLOGY EMPLOYED:
          ! Divide the diagram into complete loops. Each loop starts with a fan
          ! or pump and contains all objects that are connected to it. Each loop
          ! has only one pump or fan.  Go through each component until the fan is
          ! reached then backtrack to each splitter and follow other connections
          ! and repeat until all splitter paths have been followed.

          ! REFERENCES:
          ! na

          ! OTHER NOTES:
          ! na

          ! USE STATEMENTS:
          ! <use statements for data only modules>

          ! <use statements for access to subroutines in other modules>

IMPLICIT NONE ! Enforce explicit typing of all variables

          ! MODULE PARAMETER DEFINITIONS:

! =========================================================================
! =========================================================================
!   MAIN ARRAYS AND VARIABLES
! =========================================================================
! =========================================================================

INTEGER, PARAMETER ::  MaxNameLength =  100
INTEGER, PARAMETER ::  eof = -1

INTEGER, PARAMETER ::  flagPassThru = 0
INTEGER, PARAMETER ::  flagSplitter = 1
INTEGER, PARAMETER ::  flagMixer = 2

INTEGER, PARAMETER ::  boxWidth = 200
INTEGER, PARAMETER ::  boxHeight = 20
INTEGER, PARAMETER ::  originX = 20
INTEGER, PARAMETER ::  originY = 20
!INTEGER, PARAMETER ::  gapWidth = 1000   !good for debugging
INTEGER, PARAMETER ::  gapWidth = 50
INTEGER, PARAMETER ::  gapHeight = 20

          ! DERIVED TYPE DEFINITIONS:

TYPE BoxType
  CHARACTER(len=MaxNameLength)   :: ObjName         ! The name of the object represented by the box
  CHARACTER(len=MaxNameLength)   :: TypeObj         ! the type of object
  INTEGER                        :: colorToUse      ! index of the color to use
  INTEGER                        :: FluidStream     ! The fluid stream from BND file
  INTEGER                        :: splitMix        ! 1 = splitter, 2 = mixer, 0 = neither
  INTEGER                        :: countRelatives  ! number of ascendants for mixer
                                                    ! or number of descendants for splitter
  INTEGER                        :: countRows       ! count of boxes used to represent the single object
                                                    ! (usually one except for coils and chillers)
  INTEGER                        :: SubDiagramNum   ! the number of the subdiagram (loop)
  INTEGER                        :: row             ! rough location based on grid (y or height)
  INTEGER                        :: col             ! rough location based on grid (x or width)
  LOGICAL                        :: isAssigned      ! if used by the AssignBoxToGrid routine
  INTEGER                        :: xUpperLeft      ! position on the canvas of the upper left corner of the box
  INTEGER                        :: yUpperLeft      ! position on the canvas of the upper left corner of the box
  INTEGER                        :: xLowerRight     ! position on the canvas of the upper left corner of the box
  INTEGER                        :: yLowerRight     ! position on the canvas of the upper left corner of the box
  LOGICAL                        :: isOrigin        ! if the box is the origin for a subdiagram (the first box)
END TYPE
TYPE (BoxType),          ALLOCATABLE, DIMENSION(:)   :: box                        !main array
TYPE (BoxType),          ALLOCATABLE, DIMENSION(:)   :: boxCopy                    !copy is used only when resizing array
INTEGER                                              :: lastBox=0
INTEGER                                              :: maxSizeBox = 500
INTEGER                                              :: sizeIncrementBox = 100

TYPE ConnectType
  CHARACTER(len=MaxNameLength)   :: NodeName        ! the name of the node that connects the boxes
  LOGICAL                        :: isInterLink     ! TRUE if it is a link between diagrams for the same boxes
  LOGICAL                        :: drawLine        ! TRUE if the line should be drawn (almost all except "return" line)
  INTEGER                        :: BoxFrom         ! the index of the box the connector comes from
  INTEGER                        :: BoxTo           ! the index of the box the connector goes to
  LOGICAL                        :: FollowedBoxTo   ! if the BoxTo has been used then it is set to true.
  INTEGER                        :: StartX          ! position of start of line
  INTEGER                        :: StartY          ! position of start of line
  INTEGER                        :: EndX            ! position of end of line
  INTEGER                        :: EndY            ! position of end of line
  INTEGER                        :: NodeX           ! position of the node symbol
  INTEGER                        :: NodeY           ! position of the node symbol
  LOGICAL                        :: useMidPoint     ! indicator if connecter bends at a mid point
  INTEGER                        :: MidX1           ! if connector needs to bend, the mid point
  INTEGER                        :: MidY1           ! if connector needs to bend, the mid point
  INTEGER                        :: MidX2           ! if connector needs to bend, the mid point
  INTEGER                        :: MidY2           ! if connector needs to bend, the mid point
END TYPE
TYPE (ConnectType)    ,  ALLOCATABLE, DIMENSION(:)   :: connect
TYPE (ConnectType)    ,  ALLOCATABLE, DIMENSION(:)   :: connectCopy
INTEGER                                              :: lastConnect=0
INTEGER                                              :: maxSizeConnect = 500
INTEGER                                              :: sizeIncrementConnect = 100

TYPE SubDiagramType
  INTEGER                        :: RootBox      =0   ! reference to box that is the beginning of a loop (usually a pump or fan)
  INTEGER                        :: FinalBox     =0   ! reference to the last box drawn in each subdiagram
  INTEGER                        :: OffsetX      =0   ! offset of this sub-diagram
  INTEGER                        :: OffsetY      =0   ! offset of this sub-diagram
  INTEGER                        :: maxY         =0   ! the largest Y value in subdiagram
  INTEGER                        :: gapWidth     =0   ! the gaps between columns
  LOGICAL                        :: drawBridge   =.false.   ! draw the bridge shown below
  INTEGER                        :: bridge1X     =0   ! coordinate of the bridge that cicles back from last box to the first box
  INTEGER                        :: bridge1Y     =0   !
  INTEGER                        :: bridge2X     =0   !       _______________________________________________________
  INTEGER                        :: bridge2Y     =0   !      |3                                                    4|
  INTEGER                        :: bridge3X     =0   !      |____                                                  |
  INTEGER                        :: bridge3Y     =0   !      2    1                                                 |
  INTEGER                        :: bridge4X     =0   !                                                             |
  INTEGER                        :: bridge4Y     =0   !                                                             |
  INTEGER                        :: bridge5X     =0   !                                                      _______|
  INTEGER                        :: bridge5Y     =0   !                                                      6      5
  INTEGER                        :: bridge6X     =0
  INTEGER                        :: bridge6Y     =0
END TYPE
TYPE (SubDiagramType),   ALLOCATABLE, DIMENSION(:)   :: subDiagram
TYPE (SubDiagramType),   ALLOCATABLE, DIMENSION(:)   :: subDiagramCopy
INTEGER                                              :: lastSubDiagram=0
INTEGER                                              :: maxSizeSubDiagram = 500
INTEGER                                              :: sizeIncrementSubDiagram = 100

          ! MODULE VARIABLE DECLARATIONS:

INTEGER, ALLOCATABLE, DIMENSION(:)                   :: boxStack   !used in layout routine
INTEGER, ALLOCATABLE, DIMENSION(:)                   :: boxStackCopy
INTEGER                                              :: boxStackTop=0
INTEGER                                              :: maxSizeBoxStack = 500
INTEGER                                              :: sizeFactorBoxStack = 2
INTEGER, PARAMETER                                   :: emptyStack = -1

INTEGER, ALLOCATABLE, DIMENSION(:,:)                 :: grid
INTEGER, ALLOCATABLE, DIMENSION(:,:)                 :: gridCopy
INTEGER                                              :: maxSizeGridRow =  100
INTEGER                                              :: maxSizeGridCol =  100
INTEGER                                              :: sizeIncrementGrid = 50

INTEGER                                              :: canvasMaximumX=0
INTEGER                                              :: canvasMaximumY=0


TYPE ToColorType
  CHARACTER(len=MaxNameLength)   :: ObjType         ! the type of object
  CHARACTER(len=MaxNameLength)   :: ColorName       ! SVG color of the object
END TYPE
INTEGER, PARAMETER                         :: maxToColor = 158
TYPE (ToColorType), DIMENSION(maxToColor)  :: toColor

! =========================================================================
! =========================================================================
!   ARRAYS FROM READING AND PROCESSING BND FILE
! =========================================================================
! =========================================================================

TYPE bndNodeType
  CHARACTER(len=MaxNameLength)  :: id                  = ''
  LOGICAL                       :: usedInConnect       = .FALSE.
  LOGICAL                       :: isEliminated        = .FALSE.
  LOGICAL                       :: mayUseAsOrigin      = .TRUE.
END TYPE
TYPE (bndNodeType),ALLOCATABLE,DIMENSION(:)             ::  bndNode
INTEGER                                                 ::  lastBndNode
! <Non-Parent Node Connection>,<Node Name>,<Node ObjectType>,<Node ObjectName>,<Node ConnectionType>,<Node FluidStream>

TYPE bndNodeConnectType
  LOGICAL                       :: isParent            = .FALSE.
  !items read from BND
  CHARACTER(len=MaxNameLength)  :: NodeID              = ''
  CHARACTER(len=MaxNameLength)  :: ObjectType          = ''
  CHARACTER(len=MaxNameLength)  :: ObjectID            = ''
  CHARACTER(len=MaxNameLength)  :: ConnectType         = ''
  INTEGER                       :: FluidStream         = 0
  ! items that are computed
  INTEGER                       :: NodeNum             = 0
  INTEGER                       :: ConnectTypeNum      = 0
  ! flag if it has been used in a connect already
  LOGICAL                       :: usedInConnect       = .FALSE.
  ! flag if the node connection is an inlet or an outlet
  !  An outlet is: NodeConnect_Outlet, NodeConnect_ZoneReturn, NodeConnect_ZoneExhaust,
  !                NodeConnect_ReliefAir, NodeConnect_OutsideAir
  !  An inlet is:  NodeConnect_Inlet,  NodeConnect_ZoneInlet
  LOGICAL                       :: isOutlet            = .FALSE.
  LOGICAL                       :: isExamined          = .FALSE.
  INTEGER                       :: BoxNum              = 0
  !
  LOGICAL                       :: isMatchedExhaust    = .FALSE.
END TYPE
TYPE (bndNodeConnectType), ALLOCATABLE, DIMENSION(:)    :: bndNodeConnect
TYPE (bndNodeConnectType), ALLOCATABLE, DIMENSION(:)    :: bndNodeConnectCopy
INTEGER                                                 :: lastBndNodeConnect=0
INTEGER                                                 :: maxSizeBndNodeConnect = 500
INTEGER                                                 :: sizeIncrementBndNodeConnect = 100
INTEGER, PARAMETER :: NodeConnect_Inlet = 1
INTEGER, PARAMETER :: NodeConnect_Outlet = 2
INTEGER, PARAMETER :: NodeConnect_Internal = 3
INTEGER, PARAMETER :: NodeConnect_ZoneNode = 4
INTEGER, PARAMETER :: NodeConnect_Sensor = 5
INTEGER, PARAMETER :: NodeConnect_Actuator = 6
INTEGER, PARAMETER :: NodeConnect_OutsideAir = 7
INTEGER, PARAMETER :: NodeConnect_ReliefAir = 8
INTEGER, PARAMETER :: NodeConnect_ZoneInlet = 9
INTEGER, PARAMETER :: NodeConnect_ZoneReturn = 10
INTEGER, PARAMETER :: NodeConnect_ZoneExhaust = 11
INTEGER, PARAMETER :: NodeConnect_Setpoint = 12
INTEGER, PARAMETER :: NodeConnect_Electric = 13
INTEGER, PARAMETER :: NodeConnect_OutsideAirReference = 14

TYPE bndAirLoopConnectType
  LOGICAL                       :: isSupply            = .FALSE.
  !items read from BND
  CHARACTER(len=MaxNameLength)  :: PrimLoopName        = ''
  CHARACTER(len=MaxNameLength)  :: ZnEqpNodeID         = ''
  CHARACTER(len=MaxNameLength)  :: AirLoopNodeID       = ''
  ! remaining items are computed
  INTEGER                       :: ZnEqpNodeNum        = 0
  INTEGER                       :: AirLoopNodeNum      = 0
END TYPE
TYPE (bndAirLoopConnectType), ALLOCATABLE, DIMENSION(:) :: bndAirLoopConnect
TYPE (bndAirLoopConnectType), ALLOCATABLE, DIMENSION(:) :: bndAirLoopConnectCopy
INTEGER                                                 :: lastBndAirLoopConnect=0
INTEGER                                                 :: maxSizeBndAirLoopConnect = 500
INTEGER                                                 :: sizeIncrementBndAirLoopConnect = 100

TYPE bndLoopConnectionType
  LOGICAL                       :: isPlant             = .FALSE.
  LOGICAL                       :: isSupply            = .FALSE.
  !items read from BND
  CHARACTER(len=MaxNameLength)  :: LoopName            = ''
  CHARACTER(len=MaxNameLength)  :: OutletNodeID        = ''
  CHARACTER(len=MaxNameLength)  :: InletNodeID         = ''
  ! remaining items are computed
  INTEGER                       :: InletNodeNum        = 0
  INTEGER                       :: OutletNodeNum       = 0
END TYPE
TYPE (bndLoopConnectionType), ALLOCATABLE, DIMENSION(:) :: bndLoopConnection
TYPE (bndLoopConnectionType), ALLOCATABLE, DIMENSION(:) :: bndLoopConnectionCopy
INTEGER                                                 :: lastBndLoopConnection=0
INTEGER                                                 :: maxSizeBndLoopConnection = 500
INTEGER                                                 :: sizeIncrementBndLoopConnection = 100

TYPE bndContZoneInletType
  !items read from BND
  CHARACTER(len=MaxNameLength)  :: ZoneName            = ''
  CHARACTER(len=MaxNameLength)  :: SupInletNodeID      = ''
  CHARACTER(len=MaxNameLength)  :: SDSysCoolNodeID     = ''
  ! remaining items are computed
  INTEGER                       :: SupInletNodeNum     = 0
  INTEGER                       :: SDSysCoolNodeNum     = 0
  LOGICAL                       :: isDefinedSDSysCool  = .FALSE.
END TYPE
TYPE (bndContZoneInletType), ALLOCATABLE, DIMENSION(:) :: bndContZoneInlet
TYPE (bndContZoneInletType), ALLOCATABLE, DIMENSION(:) :: bndContZoneInletCopy
INTEGER                                                :: lastBndContZoneInlet=0
INTEGER                                                :: maxSizeBndContZoneInlet = 100
INTEGER                                                :: sizeIncrementBndContZoneInlet = 20


TYPE bndContZoneExhaustType
  !items read from BND
  CHARACTER(len=MaxNameLength)  :: ZoneName            = ''
  CHARACTER(len=MaxNameLength)  :: ExhaustNodeID       = ''
  ! remaining items are computed
  INTEGER                       :: ExhaustNodeNum     = 0
END TYPE
TYPE (bndContZoneExhaustType), ALLOCATABLE, DIMENSION(:) :: bndContZoneExhaust
TYPE (bndContZoneExhaustType), ALLOCATABLE, DIMENSION(:) :: bndContZoneExhaustCopy
INTEGER                                                  :: lastBndContZoneExhaust=0
INTEGER                                                  :: maxSizeBndContZoneExhaust = 100
INTEGER                                                  :: sizeIncrementBndContZoneExhaust = 20

LOGICAL  :: errorFoundInDrawing = .FALSE.
CHARACTER(len=300)             :: lastErrorMessage = '.'
INTEGER :: maxNumRelatives=0       !used to keep track after splitters to see how big the gap between columns should be
LOGICAL ::  dumpDetails = .FALSE.
LOGICAL :: doesExist

! =========================================================================
! =========================================================================
!   START OF MAIN PROGRAM
! =========================================================================
! =========================================================================

PRINT "(A)", "  Started HVAC Diagram"
! set the initial size of the main arrays
ALLOCATE(box(maxSizeBox))
CALL initializeBox(box)
ALLOCATE(connect(maxSizeConnect))
CALL initializeConnect(connect)
ALLOCATE(subDiagram(maxSizeSubDiagram))
CALL initializeSubDiagram(subDiagram)
ALLOCATE(boxStack(maxSizeBoxStack))
boxStack = 0
ALLOCATE(grid(maxSizeGridRow,maxSizeGridCol))
grid = 0
! prior to any objects being defined zero the pointers to the arrays
lastBox = 0
lastConnect = 0
lastSubDiagram = 0
! set the variables that size the canvas to zero
canvasMaximumX = 0
canvasMaximumY = 0
! Check if the outflag.txt file is present and if it is then
! produce output file hvacdiag.dbg.
INQUIRE (FILE="dbgflag.txt",EXIST = doesExist)
IF (doesExist) THEN
  PRINT "(A)","  Create debug output."
  dumpDetails = .TRUE.
  OPEN (UNIT=30, FILE="hvacdiag.dbg", ACTION="WRITE") !debug file
  WRITE(UNIT=30, FMT="(A)") "  Started HVAC Diagram"
  WRITE(UNIT=30, FMT="(A)") "  Create debug output."
END IF
! call the main routines (many of these call other routines)
CALL ReadBND
CALL SetZoneInletsOutlets
CALL ObjectsToBoxes
CALL ClassifyNodeConnectTypes
CALL CheckLoopConnections
CALL NodeToConnect
IF (dumpDetails) THEN
  CALL ReportNodesNotUsed
END IF
!CALL CreateDebugSystem   !just for debugging purposes - usually commented out
CALL IdentifyMixerSplitter
IF (dumpDetails) THEN
  CALL ReportConnect
END IF
CALL FindSubdiagramOrigins
CALL OrganizeBoxes
CALL LocateConnectors
CALL LocateLoopBackBridges
CALL WriteSVG
! get rid of the memory allocations for the main arrays
DEALLOCATE(box)
DEALLOCATE(connect)
DEALLOCATE(subDiagram)
DEALLOCATE(boxStack)
DEALLOCATE(grid)
IF (dumpDetails) THEN
  WRITE(UNIT=30, FMT="(A)") "  Complete"
  CLOSE (UNIT=30)
END IF
PRINT "(A)", "  Complete"
STOP
! =========================================================================
! =========================================================================
!   END OF MAIN PROGRAM
! =========================================================================
! =========================================================================

CONTAINS

! =========================================================================
! =========================================================================
!   START OF SUBROUTINES AND FUNCTIONS
! =========================================================================
! =========================================================================

! =========================================================================
SUBROUTINE ReadBND

          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! Read the lines of the BND file that are used.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
INTEGER                                  :: status
CHARACTER(500)                           :: textLine
CHARACTER(MaxNameLength), DIMENSION(50)  :: words
INTEGER                                  :: numWords
LOGICAL                                  :: doesExist

INQUIRE (FILE="eplusout.bnd",EXIST = doesExist)
IF (.NOT. doesExist) THEN
  PRINT "(A)", "No eplusout.bnd file found. Stop processing."
  IF (dumpDetails) THEN
    WRITE(UNIT=30, FMT="(A)") "No eplusout.bnd file found. Stop processing."
  END IF
  STOP
END IF
OPEN (UNIT=10, FILE="eplusout.bnd", POSITION="REWIND", ACTION="READ") !input file
status = 0
DO WHILE (status /= eof)
  textLine = ""
  READ(UNIT=10, FMT="(A)", IOSTAT=status) textLine
  textLine = ADJUSTL(textLine)
!  WRITE(UNIT=30, FMT="(A)") TRIM(textLine) !for debugging only
  IF (TRIM(textLine) .EQ. '') EXIT
  IF (textLine(1:1) .NE. '!') THEN
    CALL commaParse(textLine,words,numWords)
    SELECT CASE (makeUpperCase(TRIM(words(1))))
! used
! #Nodes,<Number of Unique Nodes>
      CASE ('#NODES')
        lastBndNode = ConvertTextToInteger(words(2))
        ALLOCATE(bndNode(lastBndNode))
! <Node>,<NodeNumber>,<Node Name>,<# Times Node Referenced After Definition>
      CASE ('NODE')  !nodes are always at the top of the BND file
        bndNode(ConvertTextToInteger(words(2)))%id = words(3)
! <Non-Parent Node Connection>,<Node Name>,<Node ObjectType>,<Node ObjectName>,<Node ConnectionType>,<Node FluidStream>
      CASE ('NON-PARENT NODE CONNECTION')
        CALL incrementNodeConnection
        bndNodeConnect(lastBndNodeConnect)%isParent = .FALSE.
        bndNodeConnect(lastBndNodeConnect)%NodeID = words(2)
        bndNodeConnect(lastBndNodeConnect)%ObjectType = words(3)
        bndNodeConnect(lastBndNodeConnect)%ObjectID = words(4)
        bndNodeConnect(lastBndNodeConnect)%ConnectType = words(5)
        bndNodeConnect(lastBndNodeConnect)%FluidStream = ConvertTextToInteger(words(6))
        !set node numbers
        bndNodeConnect(lastBndNodeConnect)%NodeNum = LookupNodeNumber(words(2))
        !set connect type number
        bndNodeConnect(lastBndNodeConnect)%ConnectTypeNum = LookupConnectType(words(5))
! <Parent Node Connection>,<Node Name>,<Node ObjectType>,<Node ObjectName>,<Node ConnectionType>,<Node FluidStream>
      CASE ('PARENT NODE CONNECTION')
        CALL incrementNodeConnection
        bndNodeConnect(lastBndNodeConnect)%isParent = .TRUE.
        bndNodeConnect(lastBndNodeConnect)%NodeID = words(2)
        bndNodeConnect(lastBndNodeConnect)%ObjectType = words(3)
        bndNodeConnect(lastBndNodeConnect)%ObjectID = words(4)
        bndNodeConnect(lastBndNodeConnect)%ConnectType = words(5)
        bndNodeConnect(lastBndNodeConnect)%FluidStream = ConvertTextToInteger(words(6))
        !set node numbers
        bndNodeConnect(lastBndNodeConnect)%NodeNum = LookupNodeNumber(words(2))
        !set connect type number
        bndNodeConnect(lastBndNodeConnect)%ConnectTypeNum = LookupConnectType(words(5))

! <Air Loop Return Connections>,<Connection Count>,<Air Primary Loop Name>,<Zn Eqp Return Node #>,<Zn Eqp Return Node Name>,
!                <Air Loop Return Node #>,<Air Loop Return Node Name>
      CASE ('AIRLOOP RETURN CONNECTIONS')
        CALL incrementAirLoopConnect
        bndAirLoopConnect(lastBndAirLoopConnect)%isSupply = .FALSE.
        bndAirLoopConnect(lastBndAirLoopConnect)%PrimLoopName = words(3)
        bndAirLoopConnect(lastBndAirLoopConnect)%ZnEqpNodeID = words(5)
        bndAirLoopConnect(lastBndAirLoopConnect)%AirLoopNodeID = words(7)
        !set node numbers
        bndAirLoopConnect(lastBndAirLoopConnect)%ZnEqpNodeNum =  LookupNodeNumber(words(5))
        bndAirLoopConnect(lastBndAirLoopConnect)%AirLoopNodeNum =  LookupNodeNumber(words(7))
! <Air Loop Supply Connections>,<Connection Count>,<Air Primary Loop Name>,<Zn Eqp Supply Node #>,<Zn Eqp Supply Node Name>,
!                <Air Loop Supply Node #>,<Air Loop Supply Node Name>
      CASE ('AIRLOOP SUPPLY CONNECTIONS')
        CALL incrementAirLoopConnect
        bndAirLoopConnect(lastBndAirLoopConnect)%isSupply = .TRUE.
        bndAirLoopConnect(lastBndAirLoopConnect)%PrimLoopName = words(3)
        bndAirLoopConnect(lastBndAirLoopConnect)%ZnEqpNodeID = words(5)
        bndAirLoopConnect(lastBndAirLoopConnect)%AirLoopNodeID = words(7)
        !set node numbers
        bndAirLoopConnect(lastBndAirLoopConnect)%ZnEqpNodeNum =  LookupNodeNumber(words(5))
        bndAirLoopConnect(lastBndAirLoopConnect)%AirLoopNodeNum =  LookupNodeNumber(words(7))
! <Plant Loop Supply Connnection>,<Plant Loop Name>,<Supply Side Outlet Node Name>,<Demand Side Inlet Node Name>
      CASE ('PLANT LOOP SUPPLY CONNECTION')
        CALL incrementLoopConnection
        bndLoopConnection(lastBndLoopConnection)%isPlant = .TRUE.
        bndLoopConnection(lastBndLoopConnection)%isSupply = .TRUE.
        bndLoopConnection(lastBndLoopConnection)%LoopName = words(2)
        bndLoopConnection(lastBndLoopConnection)%OutletNodeID = words(3)
        bndLoopConnection(lastBndLoopConnection)%InletNodeID = words(4)
        !set node numbers
        bndLoopConnection(lastBndLoopConnection)%OutletNodeNum = LookupNodeNumber(words(3))
        bndLoopConnection(lastBndLoopConnection)%InletNodeNum = LookupNodeNumber(words(4))
! <Plant Loop Return Connnection>,<Plant Loop Name>,<Demand Side Outlet Node Name>,<Supply Side Inlet Node Name>
      CASE ('PLANT LOOP RETURN CONNECTION')
        CALL incrementLoopConnection
        bndLoopConnection(lastBndLoopConnection)%isPlant = .TRUE.
        bndLoopConnection(lastBndLoopConnection)%isSupply = .FALSE.
        bndLoopConnection(lastBndLoopConnection)%LoopName = words(2)
        bndLoopConnection(lastBndLoopConnection)%OutletNodeID = words(3)
        bndLoopConnection(lastBndLoopConnection)%InletNodeID = words(4)
        !set node numbers
        bndLoopConnection(lastBndLoopConnection)%OutletNodeNum = LookupNodeNumber(words(3))
        bndLoopConnection(lastBndLoopConnection)%InletNodeNum = LookupNodeNumber(words(4))
! <Condenser Loop Supply Connnection>,<Condenser Loop Name>,<Supply Side Outlet Node Name>,<Demand Side Inlet Node Name>
      CASE ('CONDENSER LOOP SUPPLY CONNECTION')
        CALL incrementLoopConnection
        bndLoopConnection(lastBndLoopConnection)%isPlant = .FALSE.
        bndLoopConnection(lastBndLoopConnection)%isSupply = .TRUE.
        bndLoopConnection(lastBndLoopConnection)%LoopName = words(2)
        bndLoopConnection(lastBndLoopConnection)%OutletNodeID = words(3)
        bndLoopConnection(lastBndLoopConnection)%InletNodeID = words(4)
        !set node numbers
        bndLoopConnection(lastBndLoopConnection)%OutletNodeNum = LookupNodeNumber(words(3))
        bndLoopConnection(lastBndLoopConnection)%InletNodeNum = LookupNodeNumber(words(4))
! <Condenser Loop Return Connnection>,<Condenser Loop Name>,<Demand Side Outlet Node Name>,<Supply Side Inlet Node Name>
      CASE ('CONDENSER LOOP RETURN CONNECTION')
        CALL incrementLoopConnection
        bndLoopConnection(lastBndLoopConnection)%isPlant = .FALSE.
        bndLoopConnection(lastBndLoopConnection)%isSupply = .FALSE.
        bndLoopConnection(lastBndLoopConnection)%LoopName = words(2)
        bndLoopConnection(lastBndLoopConnection)%OutletNodeID = words(3)
        bndLoopConnection(lastBndLoopConnection)%InletNodeID = words(4)
        !set node numbers
        bndLoopConnection(lastBndLoopConnection)%OutletNodeNum = LookupNodeNumber(words(3))
        bndLoopConnection(lastBndLoopConnection)%InletNodeNum = LookupNodeNumber(words(4))
! <Controlled Zone Inlet>,<Inlet Node Count>,<Controlled Zone Name>,<Supply Air Inlet Node Name>,
!          <SD Sys:Cooling/Heating [DD:Cooling] Inlet Node Name>,<DD Sys:Heating Inlet Node Name>
      CASE ('CONTROLLED ZONE INLET')
        CALL incrementContZoneInlet
        bndContZoneInlet(lastBndContZoneInlet)%ZoneName = words(3)
        bndContZoneInlet(lastBndContZoneInlet)%SupInletNodeID = words(4)
        bndContZoneInlet(lastBndContZoneInlet)%SDSysCoolNodeID = words(5)
        !set node number
        bndContZoneInlet(lastBndContZoneInlet)%SupInletNodeNum = LookupNodeNumber(words(4))
        bndContZoneInlet(lastBndContZoneInlet)%SDSysCoolNodeNum = LookupNodeNumber(words(5))
        !check for 'undefined'
        IF (bndContZoneInlet(lastBndContZoneInlet)%SDSysCoolNodeNum .GT. 0) THEN  !if a node is found and is not "undefined"
          bndContZoneInlet(lastBndContZoneInlet)%isDefinedSDSysCool = .TRUE.
        ELSE
          bndContZoneInlet(lastBndContZoneInlet)%isDefinedSDSysCool = .FALSE.
        END IF
! <Controlled Zone Exhaust>,<Exhaust Node Count>,<Controlled Zone Name>,<Exhaust Air Node Name>
      CASE ('CONTROLLED ZONE EXHAUST')
        CALL incrementContZoneExhaust
        bndContZoneExhaust(lastBndContZoneExhaust)%ZoneName = words(3)
        bndContZoneExhaust(lastBndContZoneExhaust)%ExhaustNodeID = words(4)
        !set node number
        bndContZoneExhaust(lastBndContZoneExhaust)%ExhaustNodeNum = LookupNodeNumber(words(4))
! not used
      CASE ('#AIRLOOPHVACS')
      CASE ('#BRANCH LISTS')
      CASE ('#COMPONENT SETS')
      CASE ('#COMPONENTS ON RETURN AIR PATH')
      CASE ('#COMPONENTS ON SUPPLY AIR PATH')
      CASE ('#CONDENSER LOOPS')
      CASE ('#CONTROLLED ZONES')
      CASE ('#DUAL DUCT DAMPER CONNECTIONS')
      CASE ('#HEAT RECOVERY LOOPS')
      CASE ('#INLET NODES ON RETURN AIR PATH COMPONENT')
      CASE ('#NODES ON RETURN AIR PATH')
      CASE ('#NODES ON SUPPLY AIR PATH')
      CASE ('#NONCONNECTED NODES')
      CASE ('#NON-PARENT NODE CONNECTIONS')
      CASE ('#OUTLET NODES ON SUPPLY AIR PATH COMPONENT')
      CASE ('#OUTDOOR AIR NODES')
      CASE ('#PARENT NODE CONNECTIONS')
      CASE ('#PLANT LOOPS')
      CASE ('#RETURN AIR PATHS')
      CASE ('#SUPPLY AIR PATHS')
      CASE ('#ZONE EQUIPMENT LISTS')
      CASE ('AIRLOOPHVAC')
      CASE ('AIRLOOPHVAC CONNECTOR')
      CASE ('AIRLOOPHVAC CONNECTOR BRANCHES')
      CASE ('AIRLOOPHVAC CONNECTOR NODES')
      CASE ('BRANCH')
      CASE ('BRANCH LIST')
      CASE ('COMPONENT SET')
      CASE ('CONDENSER LOOP','COND LOOP')
      CASE ('CONDENSER LOOP CONNECTOR','COND LOOP CONNECTOR')
      CASE ('CONDENSER LOOP CONNECTOR BRANCHES','COND LOOP CONNECTOR BRANCHES')
      CASE ('CONDENSER LOOP CONNECTOR NODES','COND LOOP CONNECTOR NODES')
      CASE ('CONTROLLED ZONE')
      CASE ('CONTROLLED ZONE RETURN')
      CASE ('COOLED ZONE INFO')
      CASE ('DUAL DUCT DAMPER')
      CASE ('HEATED ZONE INFO')
      CASE ('NONCONNECTED NODE')
      CASE ('PLANT LOOP')
      CASE ('PLANT LOOP CONNECTOR')
      CASE ('PLANT LOOP CONNECTOR BRANCHES')
      CASE ('PLANT LOOP CONNECTOR NODES')
      CASE ('PROGRAM VERSION')
      CASE ('OUTDOOR AIR CONNECTIONS','OA SYS CONNECTIONS') !FOR LATER
      CASE ('OUTDOOR AIR NODE NAME')  !FOR LATER
      CASE ('OUTDOOR AIR NODE')  !FOR LATER
      CASE ('RETURN AIR PATH')
      CASE ('RETURN AIR PATH COMPONENT')
      CASE ('RETURN AIR PATH COMPONENT NODES')
      CASE ('RETURN AIR PATH NODE')
      CASE ('SUPPLY AIR PATH')
      CASE ('SUPPLY AIR PATH COMPONENT')
      CASE ('SUPPLY AIR PATH COMPONENT NODES')
      CASE ('SUPPLY AIR PATH NODE')
      CASE ('SUSPICIOUS NODE')
      CASE ('ZONE EQUIPMENT LIST')
      CASE ('ZONE EQUIPMENT COMPONENT')
! other
      CASE DEFAULT
        PRINT "(A,A)","  ERROR: ReadBND, Did not recognize: ",textLine
        IF (dumpDetails) THEN
          WRITE(UNIT=30,FMT="(A,A)") "  ERROR: ReadBND, Did not recognize: ",trim(textLine)
        END IF
        lastErrorMessage = "ReadBND, Did not recognize: "// trim(textLine)
    END SELECT
  END IF !comment line
END DO
CLOSE (UNIT=10)
END SUBROUTINE ReadBND

! =========================================================================
INTEGER FUNCTION LookupNodeNumber(nameOfNode)
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   March 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   Returns the node number for the node name that is specified

          ! METHODOLOGY EMPLOYED:
          !   Loop through array looking for same string

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! Note these need to match the ReadBnd routine definitions
CHARACTER(MaxNameLength),INTENT(IN)                  :: nameOfNode

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
INTEGER :: iNode

LookupNodeNumber = 0
DO iNode = 1, lastBndNode
  IF (SameString(bndNode(iNode)%id,nameOfNode)) THEN
    LookupNodeNumber = iNode
    EXIT
  END IF
END DO
END FUNCTION LookupNodeNumber


! =========================================================================
INTEGER FUNCTION LookupConnectType(typeOfConnection)
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   March 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   Returns the integer value that corresponds to the constants
          !   for the different types of node connections.

          ! METHODOLOGY EMPLOYED:
          !   Case

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! Note these need to match the ReadBnd routine definitions
CHARACTER(*),INTENT(IN)                  :: typeOfConnection

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
SELECT CASE (MakeUPPERCase(typeOfConnection))
  CASE ('INLET')
    LookupConnectType = NodeConnect_Inlet
  CASE ('OUTLET')
    LookupConnectType = NodeConnect_Outlet
  CASE ('INTERNAL')
    LookupConnectType = NodeConnect_Internal
  CASE ('ZONENODE')
    LookupConnectType = NodeConnect_ZoneNode
  CASE ('SENSOR')
    LookupConnectType = NodeConnect_Sensor
  CASE ('ACTUATOR')
    LookupConnectType = NodeConnect_Actuator
  CASE ('OUTDOORAIR')
    LookupConnectType = NodeConnect_OutsideAir
  CASE ('RELIEFAIR')
    LookupConnectType = NodeConnect_ReliefAir
  CASE ('ZONEINLET')
    LookupConnectType = NodeConnect_ZoneInlet
  CASE ('ZONERETURN')
    LookupConnectType = NodeConnect_ZoneReturn
  CASE ('ZONEEXHAUST')
    LookupConnectType = NodeConnect_ZoneExhaust
  CASE ('SETPOINT')
    LookupConnectType = NodeConnect_Setpoint
  CASE DEFAULT
    LookupConnectType = 0
END SELECT
END FUNCTION

! =========================================================================
SUBROUTINE commaParse(lineToParse,subStrings,cntSubstrings)
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   February 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   Seperates the string into subscrings based on commas

          ! METHODOLOGY EMPLOYED:
          !   Use index

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! Note these need to match the ReadBnd routine definitions
CHARACTER(500),INTENT(IN)                            :: lineToParse
CHARACTER(*), DIMENSION(50),INTENT(OUT)  :: subStrings
INTEGER,INTENT(OUT)                                  :: cntSubstrings


          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:

INTEGER         :: nextSubstringLoc
CHARACTER(500)  :: workString

subStrings = ''
cntSubstrings = 0
workString = ADJUSTL(lineToParse)
! loop through the string looking for commas
DO
  nextSubstringLoc = INDEX(workString,',')
  IF (nextSubstringLoc .GT. 0) THEN
    cntSubstrings = cntSubstrings + 1
    subStrings(cntSubstrings) = ADJUSTL(workString(:nextSubstringLoc - 1))
    workString = workString(nextSubstringLoc + 1:)
  ELSE
    EXIT
  END IF
END DO
! if any of the string is left after the last comma add it
IF (LEN_TRIM(workString) .GT. 0) THEN
  cntSubstrings = cntSubstrings + 1
  subStrings(cntSubstrings) = ADJUSTL(workString)
END IF
END SUBROUTINE commaParse


! =========================================================================
SUBROUTINE SetZoneInletsOutlets
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   March 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   Since zones can have multiple inlets and multiple outlets
          !   this routine uses other data from the BND file to associate
          !   each zone inlet with its appropriate zone outlet.  It adds new
          !   bndNodeConnects for the zone with regular input and outputs
          !   and a slighly different zone name

          ! METHODOLOGY EMPLOYED:
          !   Two different types of things can be attached to a zone.  Air
          !   loops and zone equipment. Only one air loop should be present
          !   at a time but multiple zone equipment may be present. To find
          !   the zone equipment look at the Parent Node Connection data and
          !   use the inlet and outlet that are associated with the zone
          !   exhaust to find the zoneinlet for that zone equipment.  For
          !   air loops look in the Controlled Zone Inlet data in the BND
          !   file and look for the controlled zone and use the "Supply Air
          !   Inlet Node Name" if the "SD Sys:Cooling Heating [DD:Cooling]
          !   Inlet Node Name" field is not "undefined".

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
INTEGER :: iNodeConnect
INTEGER :: jNodeConnect
INTEGER :: kContZoneInlet
INTEGER :: curExhaustNode
TYPE (bndNodeConnectType) :: curEquipInlet
INTEGER :: foundEquipInletNodeConnect
TYPE (bndNodeConnectType) :: curEquipOutlet
INTEGER :: foundEquipOutletNodeConnect
INTEGER :: nextFluidStream
CHARACTER(len=MaxNameLength)  :: curZoneName
INTEGER :: curSupInletNode
INTEGER :: foundZoneInletNodeConnect
INTEGER :: origLastBndNodeConnect
LOGICAL :: isUniqueInlet

nextFluidStream = 0
! now do the air loop connections to the zone
! these use the Controlled Zone Inlet data
origLastBndNodeConnect = lastBndNodeConnect
DO iNodeConnect = 1, lastBndNodeConnect
  !look through non-parents
  IF (.NOT. bndNodeConnect(iNodeConnect)%isParent) THEN
    IF (bndNodeConnect(iNodeConnect)%ConnectTypeNum .EQ. NodeConnect_ZoneReturn) THEN
      !now look through the Controlled Zone Inlet to match up the zoneexhaust node
      !with the inlet of some type of zone equipment
      curZoneName = bndNodeConnect(iNodeConnect)%ObjectID
      DO kContZoneInlet = 1, lastBndContZoneInlet
        IF (SameString(bndContZoneInlet(kContZoneInlet)%ZoneName, curZoneName)) THEN
          IF (bndContZoneInlet(kContZoneInlet)%isDefinedSDSysCool) THEN
            curSupInletNode = bndContZoneInlet(kContZoneInlet)%SupInletNodeNum
            ! now go batck and find the bndNodeConnect that corresponds to this case
            ! just to get the object type
            foundZoneInletNodeConnect = 0
            DO jNodeConnect = 1, lastBndNodeConnect
              IF (.NOT. bndNodeConnect(jNodeConnect)%isParent) THEN
                IF (bndNodeConnect(jNodeConnect)%ConnectTypeNum .EQ. NodeConnect_ZoneInlet) THEN
                  IF (bndNodeConnect(jNodeConnect)%NodeNum .EQ. curSupInletNode) THEN
                    foundZoneInletNodeConnect = jNodeConnect
                    EXIT
                  END IF
                END IF
              END IF
            END DO
            IF (foundZoneInletNodeConnect .GT. 0) THEN
              !Now we have found the inlet and outlets for the air loop that serves the zone
              !create new items in the nodeConnect array that reflect this specific flow through the
              !zone.  Create a new fluid stream value for this nodeConnection
              nextFluidStream = nextFluidStream + 1
              !First the zone outlet
              CALL incrementNodeConnection
              bndNodeConnect(lastBndNodeConnect)%isParent = .FALSE.
              bndNodeConnect(lastBndNodeConnect)%NodeID = bndNodeConnect(iNodeConnect)%NodeID
              bndNodeConnect(lastBndNodeConnect)%ObjectType = bndNodeConnect(iNodeConnect)%ObjectType
              bndNodeConnect(lastBndNodeConnect)%ObjectID = bndNodeConnect(iNodeConnect)%ObjectID
              bndNodeConnect(lastBndNodeConnect)%ConnectType = 'Outlet'
              bndNodeConnect(lastBndNodeConnect)%FluidStream = nextFluidStream
              bndNodeConnect(lastBndNodeConnect)%NodeNum = bndNodeConnect(iNodeConnect)%NodeNum
              bndNodeConnect(lastBndNodeConnect)%ConnectTypeNum = NodeConnect_Outlet
              !Next the zone inlet
              CALL incrementNodeConnection
              bndNodeConnect(lastBndNodeConnect)%isParent = .FALSE.
              bndNodeConnect(lastBndNodeConnect)%NodeID = bndNodeConnect(foundZoneInletNodeConnect)%NodeID
              bndNodeConnect(lastBndNodeConnect)%ObjectType = bndNodeConnect(foundZoneInletNodeConnect)%ObjectType
              bndNodeConnect(lastBndNodeConnect)%ObjectID = bndNodeConnect(foundZoneInletNodeConnect)%ObjectID
              bndNodeConnect(lastBndNodeConnect)%ConnectType = 'Inlet'
              bndNodeConnect(lastBndNodeConnect)%FluidStream = nextFluidStream
              bndNodeConnect(lastBndNodeConnect)%NodeNum = bndNodeConnect(foundZoneInletNodeConnect)%NodeNum
              bndNodeConnect(lastBndNodeConnect)%ConnectTypeNum = NodeConnect_Inlet
            ELSE
              PRINT "(A)","    ERROR: Could not find matching inlet node for: " // TRIM(curZoneName)
              IF (dumpDetails) THEN
                WRITE(UNIT=30, FMT="(A)")  "    ERROR: Could not find matching inlet node for: " // TRIM(curZoneName)
              END IF
              lastErrorMessage = "Could not find matching inlet node for: " // TRIM(curZoneName)
            END IF
          END IF
        END IF
      END DO
    END IF
  END IF
END DO
! do the zone equipment connections to the zone
! these use the parent node connections to of the equipment serving the zone
DO iNodeConnect = 1, lastBndNodeConnect
  !look through non-parents
  IF (.NOT. bndNodeConnect(iNodeConnect)%isParent) THEN
    IF (bndNodeConnect(iNodeConnect)%ConnectTypeNum .EQ. NodeConnect_ZoneExhaust) THEN
      !now look through the parent node connections to match up the zoneexhaust node
      !with the inlet of some type of zone equipment
      curExhaustNode = bndNodeConnect(iNodeConnect)%NodeNum
      foundEquipInletNodeConnect = 0
      DO jNodeConnect = 1, lastBndNodeConnect
        IF (bndNodeConnect(jNodeConnect)%isParent) THEN
          IF (bndNodeConnect(jNodeConnect)%ConnectTypeNum .EQ. NodeConnect_Inlet) THEN
            IF (bndNodeConnect(jNodeConnect)%NodeNum .EQ. curExhaustNode) THEN
              curEquipInlet = bndNodeConnect(jNodeConnect)
              foundEquipInletNodeConnect = jNodeConnect
              EXIT
            END IF
          END IF
        END IF
      END DO
      IF (foundEquipInletNodeConnect .GT. 0) THEN
        !now that the equipment has been found scan for this equipment type but look for its
        !outlet node which will be the inlet node for the current zone
        foundEquipOutletNodeConnect = 0
        DO jNodeConnect = 1, lastBndNodeConnect
          IF (bndNodeConnect(jNodeConnect)%isParent) THEN
            IF (bndNodeConnect(jNodeConnect)%ConnectTypeNum .EQ. NodeConnect_Outlet) THEN
              IF (SameString(bndNodeConnect(jNodeConnect)%ObjectID,curEquipInlet%ObjectID)) THEN
                IF (bndNodeConnect(jNodeConnect)%FluidStream .EQ. curEquipInlet%FluidStream) THEN
                  curEquipOutlet = bndNodeConnect(jNodeConnect)
                  foundEquipOutletNodeConnect = jNodeConnect
                  EXIT
                END IF
              END IF
            END IF
          END IF
        END DO
        IF (foundEquipOutletNodeConnect .GT.0) THEN
          isUniqueInlet = .TRUE.
          ! check if the inlet has already been used in a bndNodeConnect
          DO jNodeConnect = 1, lastBndNodeConnect
            IF (.NOT. bndNodeConnect(jNodeConnect)%isParent) THEN
              IF (bndNodeConnect(jNodeConnect)%NodeNum .EQ. bndNodeConnect(foundEquipOutletNodeConnect)%NodeNum) THEN
                IF (bndNodeConnect(jNodeConnect)%ConnectTypeNum .EQ. NodeConnect_Inlet) THEN
                  isUniqueInlet = .FALSE.
                END IF
              END IF
            END IF
          END DO
          IF (isUniqueInlet) THEN
            !Now we have found the inlet and outlets for the zone equipment that serves the zone
            !create new items in the nodeConnect array that reflect this specific flow through the
            !zone.  Create a new fluid stream value for this nodeConnection
            nextFluidStream = nextFluidStream + 1
            !I'm messing with the nodeConnect array in a DO loop that examines the array. This
            !is usually not a good idea but I checked the CVF language reference manual (Sep 99) and
            !on page 7-19 it says that I can change the "terminal" value of a DO loop without causing
            !a problem.
            !
            !First the zone outlet
            CALL incrementNodeConnection
            bndNodeConnect(lastBndNodeConnect)%isParent = .FALSE.
            bndNodeConnect(lastBndNodeConnect)%NodeID = bndNodeConnect(iNodeConnect)%NodeID
            bndNodeConnect(lastBndNodeConnect)%ObjectType = bndNodeConnect(iNodeConnect)%ObjectType
            bndNodeConnect(lastBndNodeConnect)%ObjectID = bndNodeConnect(iNodeConnect)%ObjectID
            bndNodeConnect(lastBndNodeConnect)%ConnectType = 'Outlet'
            bndNodeConnect(lastBndNodeConnect)%FluidStream = nextFluidStream
            bndNodeConnect(lastBndNodeConnect)%NodeNum = bndNodeConnect(iNodeConnect)%NodeNum
            bndNodeConnect(lastBndNodeConnect)%ConnectTypeNum = NodeConnect_Outlet
            bndNodeConnect(lastBndNodeConnect)%isOutlet = .TRUE.
            ! to address CR6789 flags exhaust bndNodeConnects that have matching inlet so that ones
            ! that don't are not used in the Find Subdiagram Origins routine
            bndNodeConnect(lastBndNodeConnect)%isMatchedExhaust = .TRUE.
            bndNodeConnect(lastBndNodeConnect)%isExamined = .TRUE.
            !Next the zone inlet
            CALL incrementNodeConnection
            bndNodeConnect(lastBndNodeConnect)%isParent = .FALSE.
            bndNodeConnect(lastBndNodeConnect)%NodeID = bndNodeConnect(foundEquipOutletNodeConnect)%NodeID
            bndNodeConnect(lastBndNodeConnect)%ObjectType = bndNodeConnect(iNodeConnect)%ObjectType
            bndNodeConnect(lastBndNodeConnect)%ObjectID = bndNodeConnect(iNodeConnect)%ObjectID
            bndNodeConnect(lastBndNodeConnect)%ConnectType = 'Inlet'
            bndNodeConnect(lastBndNodeConnect)%FluidStream = nextFluidStream
            bndNodeConnect(lastBndNodeConnect)%NodeNum = bndNodeConnect(foundEquipOutletNodeConnect)%NodeNum
            bndNodeConnect(lastBndNodeConnect)%ConnectTypeNum = NodeConnect_Inlet
            bndNodeConnect(lastBndNodeConnect)%isOutlet = .FALSE.
            bndNodeConnect(lastBndNodeConnect)%isExamined = .TRUE.
          END IF
        ELSE ! from (foundEquipInletNodeConnect .GT.0)
          PRINT "(A)","    ERROR: Equipment not found with outlet from equipment: " // TRIM(curEquipInlet%ObjectID)
          IF (dumpDetails) THEN
            WRITE(UNIT=30, FMT="(A)") "    ERROR: Equipment not found with outlet from equipment: " // TRIM(curEquipInlet%ObjectID)
          END IF
          lastErrorMessage = "Equipment not found with outlet from equipment: " // TRIM(curEquipInlet%ObjectID)
        END IF
      ELSE ! from (curEquip .GT. 0)
        PRINT "(A)","    ERROR: Equipment not found with inlet from node: " // TRIM(bndNode(curExhaustNode)%id)
        IF (dumpDetails) THEN
          WRITE(UNIT=30, FMT="(A)") "    ERROR: Equipment not found with inlet from node: " // TRIM(bndNode(curExhaustNode)%id)
        END IF
        lastErrorMessage = "Equipment not found with inlet from node: " // TRIM(bndNode(curExhaustNode)%id)
        bndNode(curExhaustNode)%mayUseAsOrigin = .FALSE.
      END IF
    END IF
  END IF
END DO
! see what was added
IF (dumpDetails) THEN
  WRITE(UNIT=30, FMT="(A)") "SetZoneInletsOutlets"
  DO iNodeConnect = origLastBndNodeConnect + 1, lastBndNodeConnect
    WRITE(UNIT=30, FMT="(2X,A,A,A,A,I4,A,A,I4)") TRIM(bndNodeConnect(iNodeConnect)%NodeID), " :: ", &
        TRIM(bndNodeConnect(iNodeConnect)%ObjectID), "(", iNodeConnect, ") :: ", &
        TRIM(bndNodeConnect(iNodeConnect)%ConnectType),bndNodeConnect(iNodeConnect)%NodeNum
  END DO
END IF
END SUBROUTINE SetZoneInletsOutlets

! =========================================================================
SUBROUTINE ObjectsToBoxes
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   Examine the array holding the node connections for all of the
          !   objects in the file.  Create a list of unique combinations of
          !
          !       object name, object type, fluid stream
          !
          !   And each of these is a 'box'

          ! METHODOLOGY EMPLOYED:
          !   Loop through the array holding the node connections

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
INTEGER                      :: iNodeConnect
INTEGER                      :: found
INTEGER                      :: jBox
CHARACTER(len=MaxNameLength) :: curObjectID
CHARACTER(len=MaxNameLength) :: curObjectType
INTEGER                      :: curFluidStream

DO iNodeConnect = 1, lastBndNodeConnect
  IF (.NOT. bndNodeConnect(iNodeConnect)%isParent) THEN
    found = 0
    curObjectID = bndNodeConnect(iNodeConnect)%ObjectID
    curObjectType = bndNodeConnect(iNodeConnect)%ObjectType
    curFluidStream = bndNodeConnect(iNodeConnect)%FluidStream
    DO jBox = 1, lastBox
      ! the object id, type and fluid stream all have to be the same
      ! for it to be unique
      IF (SameString(box(jBox)%ObjName,curObjectID)) THEN
        IF (SameString(box(jBox)%TypeObj,curObjectType)) THEN
          IF (box(jBox)%FluidStream .EQ. curFluidStream) THEN
            found = jBox
            EXIT
          END IF
        END IF
      END IF
    END DO
    ! if it was not found then add it to the box array
    IF (found .EQ. 0) THEN
      lastBox = lastBox + 1
      CALL expandBox(lastBox)
      box(lastBox)%ObjName = curObjectID
      box(lastBox)%TypeObj = curObjectType
      box(lastBox)%FluidStream = curFluidStream
      bndNodeConnect(iNodeConnect)%BoxNum = lastBox
    ELSE
      bndNodeConnect(iNodeConnect)%BoxNum = found
    END IF
  END IF
END DO
! For debugging only
IF (dumpDetails) THEN
  WRITE(UNIT=30, FMT="(A)") " "
  WRITE(UNIT=30, FMT="(A)") "Identified boxes below     Name :: Type :: Number "
  DO jBox = 1, lastBox
    WRITE(UNIT=30, FMT="(2X,A,A,A,2X,I4)") TRIM(box(jBox)%ObjName)," :: ",TRIM(box(jBox)%TypeObj),jBox
  END DO
END IF
END SUBROUTINE ObjectsToBoxes


! =========================================================================
SUBROUTINE ClassifyNodeConnectTypes
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   March 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   Scans through the node connection array from the bnd file
          !   and classifies the types of node connections

          ! METHODOLOGY EMPLOYED:
          !   Loop through the arrays.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
INTEGER :: iNodeConnect

! Valid connections are between objects with the following outlets
!
!    NodeConnect_Outlet
!    NodeConnect_ZoneReturn  (but handled in SetZoneInletsOutlets)
!    NodeConnect_ZoneExhaust (but handled in SetZoneInletsOutlets)
!    NodeConnect_ReliefAir (may not have object after it)
!
! The following is an outlet but is not supported yet
!
!    NodeConnect_OutsideAir (not included if not drawing outside air box)
!
! And objects with the following inlets
!
!    NodeConnect_Inlet
!    NodeConnect_ZoneInlet (but handled in SetZoneInletsOutlets)
!
! This is for non-parent objects only (internal can be inlet and outlet)

! Loop through the node connection array and classify as outlet or inlet.
DO iNodeConnect = 1, lastBndNodeConnect
  IF (bndNodeConnect(iNodeConnect)%isParent) THEN
    bndNodeConnect(iNodeConnect)%isExamined = .FALSE.
  ELSE
    SELECT CASE (bndNodeConnect(iNodeConnect)%ConnectTypeNum)
! classified as outlets
      CASE (NodeConnect_Outlet)
        bndNodeConnect(iNodeConnect)%isOutlet   = .TRUE.
        bndNodeConnect(iNodeConnect)%isExamined = .TRUE.
      CASE (NodeConnect_ReliefAir)
        bndNodeConnect(iNodeConnect)%isOutlet   = .TRUE.
        bndNodeConnect(iNodeConnect)%isExamined = .TRUE.
      CASE (NodeConnect_OutsideAir)
        bndNodeConnect(iNodeConnect)%isOutlet   = .TRUE.
        bndNodeConnect(iNodeConnect)%isExamined = .TRUE.
! classified as inlets
      CASE (NodeConnect_Inlet)
        bndNodeConnect(iNodeConnect)%isOutlet   = .FALSE.
        bndNodeConnect(iNodeConnect)%isExamined = .TRUE.
! not scanned (although used in SetZoneInletsOutlets)
      CASE (NodeConnect_ZoneInlet)
        bndNodeConnect(iNodeConnect)%isExamined = .FALSE.
      CASE (NodeConnect_ZoneReturn)
        bndNodeConnect(iNodeConnect)%isExamined = .FALSE.
      CASE (NodeConnect_ZoneExhaust)
        bndNodeConnect(iNodeConnect)%isExamined = .FALSE.
! not scanned at all
      CASE (NodeConnect_Setpoint)
        bndNodeConnect(iNodeConnect)%isExamined = .FALSE.
      CASE (NodeConnect_Internal)
        bndNodeConnect(iNodeConnect)%isExamined = .FALSE.
      CASE (NodeConnect_ZoneNode)
        bndNodeConnect(iNodeConnect)%isExamined = .FALSE.
      CASE (NodeConnect_Sensor)
        bndNodeConnect(iNodeConnect)%isExamined = .FALSE.
      CASE (NodeConnect_Actuator)
        bndNodeConnect(iNodeConnect)%isExamined = .FALSE.
      CASE DEFAULT
        bndNodeConnect(iNodeConnect)%isExamined = .FALSE.
    END SELECT
  END IF
END DO
END SUBROUTINE ClassifyNodeConnectTypes


! =========================================================================
SUBROUTINE CheckLoopConnections
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   March 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   Checks for loop connections from the demand and supply
          !   sides of each loop and creates alias nodes that have a
          !   combined name and sets the connections to that node.

          ! METHODOLOGY EMPLOYED:
          !   Loop through the arrays and find the connections that are
          !   shared. When node is present in the airloopconnection or
          !   plantconnection, or condenserloopconnection then the 'alias'
          !   for the node needs to be used.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
INTEGER :: iNodeConnect
INTEGER :: kLoopConnect

INTEGER :: nodeToEliminate
INTEGER :: nodeToUseInstead
CHARACTER(len=MaxNameLength*3)  ::  newNodeName

! go through the loop connections and match together alias nodes
! first the air loop connections
DO kLoopConnect = 1, lastBndAirLoopConnect
  nodeToEliminate = bndAirLoopConnect(kLoopConnect)%ZnEqpNodeNum
  nodeToUseInstead = bndAirLoopConnect(kLoopConnect)%AirLoopNodeNum
  bndNode(nodeToEliminate)%isEliminated = .TRUE.
  newNodeName = TRIM(bndNode(nodeToUseInstead)%id) // "/" // TRIM(bndNode(nodeToEliminate)%id)
  bndNode(nodeToUseInstead)%id = newNodeName(1:MaxNameLength) !trim off the end of the node name if it is too long
  ! now go through the bndNodeConnect array and make the substitution
  DO iNodeConnect = 1, lastBndNodeConnect
    IF (bndNodeConnect(iNodeConnect)%NodeNum .EQ. nodeToEliminate) THEN
      bndNodeConnect(iNodeConnect)%NodeNum = nodeToUseInstead
      IF (dumpDetails) THEN
        WRITE(UNIT=30, FMT="(A)") "Fixed Air Loop Connect: " // TRIM(bndNodeConnect(iNodeConnect)%NodeID) // "::"// TRIM(bndNodeConnect(iNodeConnect)%ObjectID) &
                            // "::" // TRIM(bndNodeConnect(iNodeConnect)%ConnectType)
        WRITE(UNIT=30, FMT="(A)") "      From: " // TRIM(bndNode(nodeToEliminate)%id) // " to " // TRIM(bndNode(nodeToUseInstead)%id)
      END IF
    END IF
  END DO
END DO
! now water loop connections
DO kLoopConnect = 1, lastBndLoopConnection
  nodeToEliminate = bndLoopConnection(kLoopConnect)%InletNodeNum
  nodeToUseInstead = bndLoopConnection(kLoopConnect)%OutletNodeNum
  bndNode(nodeToEliminate)%isEliminated = .TRUE.
  newNodeName = TRIM(bndNode(nodeToUseInstead)%id) // "/" // TRIM(bndNode(nodeToEliminate)%id)
  bndNode(nodeToUseInstead)%id = newNodeName(1:MaxNameLength) !trim off the end of the node name if it is too long
  ! now go through the bndNodeConnect array and make the substitution
  DO iNodeConnect = 1, lastBndNodeConnect
    IF (bndNodeConnect(iNodeConnect)%NodeNum .EQ. nodeToEliminate) THEN
      bndNodeConnect(iNodeConnect)%NodeNum = nodeToUseInstead
      IF (dumpDetails) THEN
        WRITE(UNIT=30, FMT="(A)") "Fixed Water Loop Connect: " // TRIM(bndNodeConnect(iNodeConnect)%NodeID) // "::"// TRIM(bndNodeConnect(iNodeConnect)%ObjectID) &
                              // "::" // TRIM(bndNodeConnect(iNodeConnect)%ConnectType)
        WRITE(UNIT=30, FMT="(A)") "      From: " // TRIM(bndNode(nodeToEliminate)%id) // " to " // TRIM(bndNode(nodeToUseInstead)%id)
      END IF
    END IF
  END DO
END DO
END SUBROUTINE CheckLoopConnections

! =========================================================================
SUBROUTINE NodeToConnect
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   March 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   Scans through the node connection array from the bnd file
          !   and creates the connect array that is used to create the
          !   diagram.

          ! METHODOLOGY EMPLOYED:
          !   Loop through the arrays and find the connections that are
          !   shared.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
INTEGER :: iNodeConnect
INTEGER :: jNodeConnect

INTEGER :: curNode

! loop through node connection array and try to match up
! inlets with outlets
IF (dumpDetails) THEN
  DO iNodeConnect = 1, lastBndNodeConnect
    IF (bndNodeConnect(iNodeConnect)%isExamined) THEN
      WRITE(UNIT=30, FMT="(A,A)")  "  NodeConnect BoxNums: ",TRIM(box(bndNodeConnect(iNodeConnect)%BoxNum)%ObjName)
      WRITE(UNIT=30, FMT="(A,A)") "                       ",TRIM(bndNodeConnect(iNodeConnect)%NodeID)
    END IF
  END DO
END IF
DO iNodeConnect = 1, lastBndNodeConnect
  IF (bndNodeConnect(iNodeConnect)%isExamined) THEN
    IF (.NOT. bndNodeConnect(iNodeConnect)%usedInConnect) THEN
      IF (bndNodeConnect(iNodeConnect)%isOutlet) THEN
        curNode = bndNodeConnect(iNodeConnect)%NodeNum
        ! now search through the array looking for the same
        ! node number but this time being used as an inlet
        IF (dumpDetails) THEN
          WRITE(UNIT=30, FMT="(A,A)") "  curNode in NodeToConnect: ",bndNode(curNode)%id
        END IF
        DO jNodeConnect = 1, lastBndNodeConnect
          IF (bndNodeConnect(jNodeConnect)%isExamined) THEN
            IF (.NOT. bndNodeConnect(jNodeConnect)%usedInConnect) THEN
              IF (.NOT. bndNodeConnect(jNodeConnect)%isOutlet) THEN
                IF (bndNodeConnect(jNodeConnect)%NodeNum .EQ. curNode) THEN
                  !matching node found - make a connect
                  lastConnect = lastConnect + 1
                  CALL expandConnect(lastConnect)
                  connect(lastConnect)%NodeName = bndNode(curNode)%id
                  connect(lastConnect)%BoxFrom = bndNodeConnect(iNodeConnect)%BoxNum
                  connect(lastConnect)%BoxTo =   bndNodeConnect(jNodeConnect)%BoxNum
                  bndNode(curNode)%usedInConnect = .TRUE.
                  bndNodeConnect(jNodeConnect)%usedInConnect = .TRUE.
                  bndNodeConnect(iNodeConnect)%usedInConnect = .TRUE.
                  IF (bndNodeConnect(iNodeConnect)%BoxNum .EQ. 0) THEN
                    PRINT "(A)","     WARNING: Zero box number. " // TRIM(connect(lastConnect)%NodeName)
                    IF (dumpDetails) THEN
                      WRITE(UNIT=30, FMT="(A)") "     WARNING: Zero box number. " // TRIM(connect(lastConnect)%NodeName)
                    END IF
                  END IF
                END IF
              END IF
            END IF
          END IF
        END DO !jNodeConnect - inlets
      END IF
    END IF
  END IF
END DO !iNodeConnect - outlets
curnode = 1 !just for debugging
END SUBROUTINE NodeToConnect

! =========================================================================
SUBROUTINE ReportNodesNotUsed
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   March 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   Scans through the node connection array and node array and
          !   reports items not used after the NodeToConnect routine is
          !   complete

          ! METHODOLOGY EMPLOYED:
          !   Loop through the arrays

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
INTEGER :: iNodeConnect
INTEGER :: iNode

! for debugging purposes - node connects that aren't used
DO iNodeConnect = 1, lastBndNodeConnect
  IF (bndNodeConnect(iNodeConnect)%isExamined) THEN
    IF (.NOT. bndNodeConnect(iNodeConnect)%usedInConnect) THEN
      WRITE(UNIT=30, FMT="(A)") "    Node connection not used: " // TRIM(bndNodeConnect(iNodeConnect)%NodeID) // &
                  "," // TRIM(bndNodeConnect(iNodeConnect)%NodeID) // &
                  "," // TRIM(bndNodeConnect(iNodeConnect)%ObjectType) // &
                  "," // TRIM(bndNodeConnect(iNodeConnect)%ObjectID) // &
                  "," // TRIM(bndNodeConnect(iNodeConnect)%ConnectType)
    END IF
  END IF
END DO
! for debugging purposes - nodes that aren't used
DO iNode = 1, lastBndNode
  IF (.NOT. bndNode(iNode)%isEliminated) THEN
    IF (.NOT. bndNode(iNode)%usedInConnect) THEN
      WRITE(UNIT=30, FMT="(A)")  "    Node not used: " // TRIM(bndNode(iNode)%id)
    END IF
  END IF
END DO
END SUBROUTINE ReportNodesNotUsed


! =========================================================================
SUBROUTINE ReportConnect
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   Scans through the boxes and figures out which ones act as splitter
          !   and which ones as mixers.

          ! METHODOLOGY EMPLOYED:
          !   Use built in COUNT function from F90 to see when the connector
          !   array references that box more than once.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
INTEGER :: iConnect
CHARACTER(len=MaxNameLength)  ::  boxFromName
CHARACTER(len=MaxNameLength)  ::  boxToName

DO iConnect = 1, lastConnect
  WRITE(UNIT=30, FMT="(A,I4,2X,A)") "----  ", iConnect, connect(iConnect)%NodeName
  IF (connect(iConnect)%BoxFrom .GT. 0) THEN
    boxFromName = box(connect(iConnect)%BoxFrom)%ObjName
  ELSE
    boxFromName = 'undefined'
  ENDIF
  IF (connect(iConnect)%BoxTo .GT. 0) THEN
    boxToName = box(connect(iConnect)%BoxTo)%ObjName
  ELSE
    boxToName = 'undefined'
  ENDIF
  WRITE(UNIT=30, FMT="(A,I4,2X,A)") "BoxFrom: ",connect(iConnect)%BoxFrom,TRIM(boxFromName)
  WRITE(UNIT=30, FMT="(A,I4,2X,A)") "  BoxTo: ",connect(iConnect)%BoxTo,TRIM(boxToName)
END DO
END SUBROUTINE ReportConnect


! =========================================================================
SUBROUTINE IdentifyMixerSplitter
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   Scans through the boxes and figures out which ones act as splitter
          !   and which ones as mixers.

          ! METHODOLOGY EMPLOYED:
          !   Use built in COUNT function from F90 to see when the connector
          !   array references that box more than once.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
INTEGER    :: iBox
INTEGER    :: numFrom
INTEGER    :: numTo

DO iBox = 1,lastBox
  numFrom = COUNT(connect%BoxFrom .EQ. iBox)
  numTo = COUNT(connect%BoxTo .EQ. iBox)
  IF (numTo .GT. 1) THEN
    box(iBox)%splitMix = flagMixer
    box(iBox)%countRelatives = numTo
  ELSEIF (numFrom .GT. 1) THEN
    box(iBox)%splitMix = flagSplitter
    box(iBox)%countRelatives = numFrom
  ELSE
    box(iBox)%splitMix = flagPassThru
    box(iBox)%countRelatives = 1
  END IF
END DO
END SUBROUTINE IdentifyMixerSplitter


! =========================================================================
SUBROUTINE CreateDebugSystem
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   Creates a defined set of boxes and their interconnections. Used to debug
          !   only and should usually not be called

          ! METHODOLOGY EMPLOYED:
          ! <description>

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:

! based on 3 Zone Dual Duct VAV System Diagram from
! training course

! air side components

lastBox = 40
lastConnect = 49

box(1)%objName ='Zone 1'
box(1)%TypeObj = 'Zone'

box(2)%objName ='Zone 2'
box(2)%TypeObj = 'Zone'

box(3)%objName ='Zone 3'
box(3)%TypeObj = 'Zone'

box(4)%objName ='VAV Box 1'
box(4)%TypeObj ='VAV Unit'

box(5)%objName ='VAV Box 2'
box(5)%TypeObj ='VAV Unit'

box(6)%objName ='VAV Box 3'
box(6)%TypeObj ='VAV Unit'

box(7)%objName ='Hot Air Splitter'
box(7)%TypeObj ='Splitter'

box(8)%objName ='Cold Air Splitter'
box(8)%TypeObj ='Splitter'

box(9)%objName ='HC'
box(9)%TypeObj ='Coil:Simple'

box(10)%objName ='CC'
box(10)%TypeObj ='Coil:Simple'

box(11)%objName ='Supply Air Splitter'
box(11)%TypeObj ='Splitter:Look Ahead'

box(12)%objName ='Supply Fan(VAV)'
box(12)%TypeObj ='Fan:Variable'

box(19)%objName ='Return Air Mixer'
box(19)%TypeObj ='Mixer'

! heating plant loop components

box(13)%objName ='HC'
box(13)%TypeObj ='Coil:Simple'

box(15)%objName ='Heating Demand Splitter'
box(15)%TypeObj ='Splitter'

!box(16)%objName ='PurchHeat1'
!box(16)%TypeObj ='Purchased Heating'

box(17)%objName ='HeatDemandBypass'
box(17)%TypeObj ='Pipe'

box(18)%objName ='HeatingPump1'
box(18)%TypeObj ='Pump:Constant Speed'

box(21)%objName ='Heating Demand Mixer'
box(21)%TypeObj ='Mixer'

!box(23)%objName ='Heating Coil Bypass'
!box(23)%TypeObj ='Pipe'

box(37)%objName ='MixerExitPipe'
box(37)%TypeObj ='Pipe'

box(38)%objName ='SplitterEntrancePipe'
box(38)%TypeObj ='Pipe'

! cooling loop

box(14)%objName ='CC'
box(14)%TypeObj ='Coil:Simple'

box(20)%objName ='CoolCoilSplitter'
box(20)%TypeObj ='Splitter'

box(22)%objName ='CoolCoilMixer'
box(22)%TypeObj ='Mixer'

box(24)%objName ='Cooling Coil Bypass'
box(24)%TypeObj ='Pipe'

box(25)%objName ='CoolingPump1'
box(25)%TypeObj ='Pump:Constant Speed'

box(26)%objName ='ChillerSplitter'
box(26)%TypeObj ='Splitter'

box(27)%objName ='Chiller#1'
box(27)%TypeObj ='Chiller:Recip'

box(28)%objName ='Chiller#2'
box(28)%TypeObj ='Chiller:Recip'

box(29)%objName ='PurchCool1'
box(29)%TypeObj ='Purchased Cooling'

box(30)%objName ='Chiller Bypass'
box(30)%TypeObj ='Pipe'

box(31)%objName ='ChillerMixer'
box(31)%TypeObj ='Mixer'

box(39)%objName ='ChillerExitPipe'
box(39)%TypeObj ='Pipe'

! condenser loop

box(32)%objName ='Chiller#1'
box(32)%TypeObj ='Chiller:Recip'

box(40)%objName ='Chiller#2'
box(40)%TypeObj ='Chiller:Recip'

box(33)%objName ='CondenserMixer'
box(33)%TypeObj ='Mixer'

box(34)%objName ='CondenserPump1'
box(34)%TypeObj ='Pump:Constant Speed'

box(35)%objName ='CoolingTower1'
box(35)%TypeObj ='CoolingTower:Water'

box(36)%objName ='CondenserSplitter'
box(36)%TypeObj ='Splitter'

! air side connection

! VAV to Zone
connect(1)%NodeName  = 'Node 15'
connect(1)%BoxFrom  = 4
connect(1)%BoxTo  = 1

connect(2)%NodeName  = 'Node 18'
connect(2)%BoxFrom  = 5
connect(2)%BoxTo  = 2

connect(3)%NodeName  = 'Node 21'
connect(3)%BoxFrom  = 6
connect(3)%BoxTo  = 3

! hot air splitter to vav
connect(4)%NodeName  = 'Node 8'
connect(4)%BoxFrom  = 7
connect(4)%BoxTo  = 4

connect(5)%NodeName  = 'Node 9'
connect(5)%BoxFrom  = 7
connect(5)%BoxTo  = 5

connect(6)%NodeName  = 'Node 10'
connect(6)%BoxFrom  = 7
connect(6)%BoxTo  = 6

! cold air splitter to vav
connect(7)%NodeName  = 'Node 12'
connect(7)%BoxFrom  = 8
connect(7)%BoxTo  = 4

connect(8)%NodeName  = 'Node 13'
connect(8)%BoxFrom  = 8
connect(8)%BoxTo  = 5

connect(9)%NodeName  = 'Node 14'
connect(9)%BoxFrom  = 8
connect(9)%BoxTo  = 6

!coils
connect(10)%NodeName  = 'Node 4/Node 7'
connect(10)%BoxFrom  = 9
connect(10)%BoxTo  = 7

connect(11)%NodeName  = 'Node 6/Node 11'
connect(11)%BoxFrom  = 10
connect(11)%BoxTo  = 8

!supply splitter
connect(12)%NodeName  = 'Node 3'
connect(12)%BoxFrom  = 11
connect(12)%BoxTo  = 9

connect(13)%NodeName  = 'Node 5'
connect(13)%BoxFrom  = 11
connect(13)%BoxTo  = 10

connect(14)%NodeName  = 'Node 2'
connect(14)%BoxFrom  = 12
connect(14)%BoxTo  = 11

!return mixer to fan
connect(15)%NodeName  = 'Node 24/Node 1'
connect(15)%BoxFrom  = 19
connect(15)%BoxTo  = 12

connect(16)%NodeName  = 'Node 17'
connect(16)%BoxFrom  = 1
connect(16)%BoxTo  = 19

connect(17)%NodeName  = 'Node 20'
connect(17)%BoxFrom  = 2
connect(17)%BoxTo  = 19

connect(18)%NodeName  = 'Node 23'
connect(18)%BoxFrom  = 3
connect(18)%BoxTo  = 19

! heating connections
connect(19)%NodeName  = 'Node 26'
connect(19)%BoxFrom  = 13
connect(19)%BoxTo  = 21

connect(20)%NodeName  = 'Node 72'
connect(20)%BoxFrom  = 17
connect(20)%BoxTo  = 21

connect(21)%NodeName  = 'Node 69'
connect(21)%BoxFrom  = 21
connect(21)%BoxTo  = 37

connect(22)%NodeName  = 'Node 25/30'
connect(22)%BoxFrom  = 37
connect(22)%BoxTo  = 18

connect(23)%NodeName  = 'Node 29/Node 28'
connect(23)%BoxFrom  = 18
connect(23)%BoxTo  = 38

connect(24)%NodeName  = 'Node 66'
connect(24)%BoxFrom  = 38
connect(24)%BoxTo  = 15

connect(25)%NodeName  = 'Node 73'
connect(25)%BoxFrom  = 15
connect(25)%BoxTo  = 17

connect(26)%NodeName  = 'Node 27'
connect(26)%BoxFrom  = 15
connect(26)%BoxTo  = 13

! chilled water connections

connect(27)%NodeName  = 'Node 33'
connect(27)%BoxFrom  = 14
connect(27)%BoxTo  = 22

connect(28)%NodeName  = 'Node 70'
connect(28)%BoxFrom  = 24
connect(28)%BoxTo  = 22

connect(29)%NodeName  = 'Node 50/Node 51'
connect(29)%BoxFrom  = 22
connect(29)%BoxTo  = 25

connect(30)%NodeName  = 'Node 34'
connect(30)%BoxFrom  = 25
connect(30)%BoxTo  = 26

connect(31)%NodeName  = 'Node 36'
connect(31)%BoxFrom  = 26
connect(31)%BoxTo  = 27

connect(32)%NodeName  = 'Node 37'
connect(32)%BoxFrom  = 26
connect(32)%BoxTo  = 28

connect(33)%NodeName  = 'Node 37'
connect(33)%BoxFrom  = 26
connect(33)%BoxTo  = 29

connect(34)%NodeName  = 'Node 64'
connect(34)%BoxFrom  = 26
connect(34)%BoxTo  = 30

connect(35)%NodeName  = 'Node 38'
connect(35)%BoxFrom  = 27
connect(35)%BoxTo  = 31

connect(36)%NodeName  = 'Node 39'
connect(36)%BoxFrom  = 28
connect(36)%BoxTo  = 31

connect(37)%NodeName  = 'Node 49'
connect(37)%BoxFrom  = 29
connect(37)%BoxTo  = 31

connect(38)%NodeName  = 'Node 65'
connect(38)%BoxFrom  = 30
connect(38)%BoxTo  = 31

connect(39)%NodeName  = 'Node 74'
connect(39)%BoxFrom  = 31
connect(39)%BoxTo  = 39

connect(40)%NodeName  = 'Node 40/Node 31'
connect(40)%BoxFrom  = 39
connect(40)%BoxTo  = 20

connect(41)%NodeName  = 'Node 71'
connect(41)%BoxFrom  = 20
connect(41)%BoxTo  = 24

connect(42)%NodeName  = 'Node 32'
connect(42)%BoxFrom  = 20
connect(42)%BoxTo  = 14

! condensor loop
connect(43)%NodeName  = 'A'
connect(43)%BoxFrom  = 34
connect(43)%BoxTo  = 36

connect(44)%NodeName  = 'B'
connect(44)%BoxFrom  = 36
connect(44)%BoxTo  = 32

connect(45)%NodeName  = 'C'
connect(45)%BoxFrom  = 36
connect(45)%BoxTo  = 40

connect(46)%NodeName  = 'D'
connect(46)%BoxFrom  = 32
connect(46)%BoxTo  = 33

connect(47)%NodeName  = 'E'
connect(47)%BoxFrom  = 40
connect(47)%BoxTo  = 33

connect(48)%NodeName  = 'F'
connect(48)%BoxFrom  = 33
connect(48)%BoxTo  = 35

connect(49)%NodeName  = 'G'
connect(49)%BoxFrom  = 35
connect(49)%BoxTo  = 34

END SUBROUTINE CreateDebugSystem


! =========================================================================
SUBROUTINE FindSubdiagramOrigins
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   The first box for each diagram is identified.

          ! METHODOLOGY EMPLOYED:
          !   This routine replaces the old method of just looking for fans
          !   and pumps. Instead this routine looks for certain nodes and
          !   finds the next object that goes with that node.

          ! REFERENCES:
          !   na

          ! USE STATEMENTS:
          !   na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:

INTEGER :: iContZoneExhaust
INTEGER :: iLoopConnection
INTEGER :: jNodeConnect
INTEGER :: nodeForOrigin

!first look in the bndAirLoopConnections for a return node that
!corresponds to the first box for an air loop
!Use "Air Loop Return Connections" item "Air Loop Return Node Name" where it is an inlet
!
! In other words:
!     Search through the Air Loop Return Connections for the Air Loop Return Node
!     Name.  This node name is then searched for in the Non-Parent Node Connection
!     that are Inlet. If the node matches it is an origin.
!
DO iLoopConnection = 1, lastBndAirLoopConnect
  IF (.NOT. bndAirLoopConnect(iLoopConnection)%isSupply) THEN
    nodeForOrigin = bndAirLoopConnect(iLoopConnection)%AirLoopNodeNum
    DO jNodeConnect =  1, lastBndNodeConnect
      IF ((bndNodeConnect(jNodeConnect)%isExamined) .AND. (.NOT. bndNodeConnect(jNodeConnect)%isOutlet)) THEN
        IF (bndNodeConnect(jNodeConnect)%NodeNum .EQ. nodeForOrigin) THEN
          box(bndNodeConnect(jNodeConnect)%BoxNum)%isOrigin = .TRUE.
          IF (dumpDetails) THEN
            WRITE(UNIT=30, FMT="(A)") " Found air loop origin: " // box(bndNodeConnect(jNodeConnect)%BoxNum)%ObjName
          END IF
        END IF
      END IF
    END DO
  END IF
END DO
! Go through each Controlled Zone Exhaust and assume the Exhaust Air Node
! Name is the node to be used for the origin. It is a zero (not found) skip.
! If equipment is not found with inlet from that node then skip. This node
! name is then searched for in the Non-Parent Node Connection that are Inlet.
! If the node matches it is an origin.
DO iContZoneExhaust = 1, lastBndContZoneExhaust
  nodeForOrigin = bndContZoneExhaust(iContZoneExhaust)%ExhaustNodeNum
  IF (nodeForOrigin .EQ. 0) CYCLE
  !if it cannot be used as an origin, skip it (only when exhaust fan system like TermReheatZOneExh.idf)
  IF (.NOT. bndNode(nodeForOrigin)%mayUseAsOrigin) CYCLE
  !now search through the node connections and look for the use of this
  !node as an inlet
  DO jNodeConnect =  1,lastBndNodeConnect
    IF ((bndNodeConnect(jNodeConnect)%isExamined) .AND. (.NOT. bndNodeConnect(jNodeConnect)%isOutlet)) THEN
      ! to address CR6789 dont use exhausts from the zone that were not followed in the Set Zone Inlets Outlets routine
      !IF (bndNodeConnect(jNodeConnect)%isMatchedExhaust) THEN
        IF (bndNodeConnect(jNodeConnect)%NodeNum .EQ. nodeForOrigin) THEN
          box(bndNodeConnect(jNodeConnect)%BoxNum)%isOrigin = .TRUE.
          IF (dumpDetails) THEN
            WRITE(UNIT=30, FMT="(A,A,A,I4,A,I4,A)") " Found zone equipment origin: " , TRIM(box(bndNodeConnect(jNodeConnect)%BoxNum)%ObjName),"(", &
                          bndNodeConnect(jNodeConnect)%BoxNum," <- ",jNodeConnect,")"
          END IF
        END IF
      !END IF
    END IF
  END DO
END DO
!now look for plant loops - use the Supply Side Inlet Node Name of the Plant Loop Return Connnection
!Now look for the - use the Supply Side Inlet Node Name of the Condenser Loop Return Connnection
!
! For each Plant Loop Return Connection, the Demand Side Outlet Node Name.
! This node name is then searched for in the Non-Parent Node Connection that
! are Inlet. If the node matches it is an origin.
!
DO iLoopConnection = 1, lastBndLoopConnection
  !only returns
  IF (.NOT. bndLoopConnection(iLoopConnection)%isSupply) THEN
    nodeForOrigin = bndLoopConnection(iLoopConnection)%OutletNodeNum
    !now search through the node connections and look for the use of this
    !node as an inlet
    DO jNodeConnect =  1, lastBndNodeConnect
      IF ((bndNodeConnect(jNodeConnect)%isExamined) .AND. (.NOT. bndNodeConnect(jNodeConnect)%isOutlet)) THEN
        IF (bndNodeConnect(jNodeConnect)%NodeNum .EQ. nodeForOrigin) THEN
          box(bndNodeConnect(jNodeConnect)%BoxNum)%isOrigin = .TRUE.
          IF (dumpDetails) THEN
            WRITE(UNIT=30, FMT="(A)") " Found water loop origin: " // box(bndNodeConnect(jNodeConnect)%BoxNum)%ObjName
          END IF
        END IF
      END IF
    END DO
  END IF
END DO
END SUBROUTINE FindSubdiagramOrigins


! =========================================================================
SUBROUTINE OrganizeBoxes
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! The boxes and connections exist when this routine is called but
          ! are not organized into loops.  This routine scans through the boxes
          ! and connections and creates a series of sub-diagrams and then puts all
          ! of the subdiagrams on the canvas.

          ! METHODOLOGY EMPLOYED:
          ! see purpose

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
INTEGER    :: originBoxOfDangling
INTEGER    :: iBox
!organize the closed loop subdiagrams
DO iBox = 1,lastBox
  IF (box(iBox)%isOrigin) THEN
    IF (dumpDetails) THEN
      WRITE(UNIT=30, FMT="(A,A)") "Processing origin box: ",box(iBox)%objName
      WRITE(UNIT=30, FMT="(A,I4)") "  Subdiagram should be zero: ", box(iBox)%SubDiagramNum
    END IF
    ! if it is not already in another sub-diagram
    IF (box(iBox)%SubDiagramNum .EQ. 0) THEN
      !make sure the origin has not been used already
      IF (.NOT. box(iBox)%isAssigned) THEN
        !check if its ancestors are a splitter
        IF (.NOT. ancestorIsSplitter(iBox)) THEN
          CALL OrganizeSubDiagram(iBox)
          CALL RowColIntoXY(lastSubDiagram)
        ELSE
          PRINT "(A,A)", "  Ancestor to the following box is a splitter: ",box(iBox)%objName
          IF (dumpDetails) THEN
            WRITE(UNIT=30, FMT="(A,A)") "  Ancestor to the following box is a splitter: ",box(iBox)%objName
          END IF
        END IF
      END IF
    END IF
  END IF
END DO
!now draw the dangling boxes if they can be connected up with each other
DO iBox = 1,lastBox !counter just to leave if stuck in loop
  CALL GetNextDanglingOriginBox(originBoxOfDangling)
  IF (originBoxOfDangling .GT. 0) THEN
    IF (dumpDetails) THEN
      WRITE(UNIT=30, FMT="(A,A)") "Processing dangling origin box: ",box(originBoxOfDangling)%objName
    END IF
    CALL OrganizeDanglingSubDiagram(originBoxOfDangling)
    CALL RowColIntoXY(lastSubDiagram)
  ELSE
    EXIT
  END IF
END DO
!finish up organization
CALL LocateSubDiagrams
CALL InterSubDiagramLinks
IF (dumpDetails) THEN
  DO iBox =1, lastBox
    IF (.NOT. box(iBox)%isAssigned) THEN
      WRITE(UNIT=30, FMT="(A,I6,3X,A)") "    Boxes that are not assigned: ",ibox, box(ibox)%ObjName
    END IF
  END DO
END IF
END SUBROUTINE OrganizeBoxes


! =========================================================================
LOGICAL FUNCTION ancestorIsSplitter(boxNum)
          ! FUNCTION INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! Returns true if an ancestor of the box is a splitter and a
          ! false if a mixer is found instead.

          ! METHODOLOGY EMPLOYED:
          ! Searchs through the boxes connected to it on the "boxFrom" side and
          ! looks for the first splitter or mixer. If a splitter is found

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine


          ! FUNCTION ARGUMENT DEFINITIONS:
INTEGER,INTENT(IN)  :: boxNum

          ! FUNCTION PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! FUNCTION LOCAL VARIABLE DECLARATIONS:

INTEGER   :: curBox
INTEGER   :: safetyCounter
INTEGER   :: prevBox
INTEGER   :: originalCurBox

curBox = boxNum
originalCurBox = curBox
safetyCounter = 0
DO
  IF (dumpDetails) THEN
    WRITE(UNIT=30, FMT="(A,I4,2X,A,2X,I4)") "  Ancestor: ",curBox,box(curBox)%ObjName,box(curBox)%splitMix
  END IF
  IF (box(curBox)%splitMix .EQ. flagSplitter) THEN
    ancestorIsSplitter = .TRUE.
    EXIT
  ELSEIF (box(curBox)%splitMix .EQ. flagMixer) THEN
    ancestorIsSplitter = .FALSE.
    EXIT
  ELSE
    prevBox = curBox
    curBox = getFirstAncestorOf(curBox)
    IF (curBox .EQ. originalCurBox) THEN
      ancestorIsSplitter = .FALSE.
      EXIT
    END IF
    IF (curBox .EQ. 0) THEN  !error condition
      IF (dumpDetails) THEN
        WRITE(UNIT=30, FMT="(A,I4,2X,A)") "  ERROR: Diagram pieces not linked (upstream): ",prevBox, box(prevBox)%ObjName
      END IF
      PRINT "(A,I4,2X,A)","  ERROR: Diagram pieces not linked (upstream): ",prevBox, box(prevBox)%ObjName
      lastErrorMessage = "Diagram pieces not linked (upstream): " // box(prevBox)%ObjName
      ! attempt to draw whatever has been figured out so far prior to quiting
      errorFoundInDrawing = .TRUE.
      ! trying to fix the 5zoneFPIU bug return a result even if error is found instead of writing out the SVG right away
      ancestorIsSplitter = .FALSE.
      EXIT
      !CALL LocateSubDiagrams
      !CALL InterSubDiagramLinks
      !CALL LocateConnectors
      !CALL WriteSVG
      !STOP
      !EXIT
    END IF
  END IF
  safetyCounter = safetyCounter + 1
  IF (safetyCounter .GT. 1000) THEN
    PRINT "(A)", "  ERROR: Safety couter exceeded."
    IF (dumpDetails) THEN
      WRITE(UNIT=30, FMT="(A)") "  ERROR: Safety couter exceeded."
    END IF
    lastErrorMessage = "Safety couter exceeded."
    errorFoundInDrawing = .TRUE.
    !changing the way to deal with a loop, go ahead and say no splitter
    !and allow to continue instead of drawing current diagram
    !ancestorIsSplitter = .FALSE.
    !EXIT
    CALL LocateSubDiagrams
    CALL InterSubDiagramLinks
    CALL LocateConnectors
    CALL WriteSVG
    STOP
  END IF
END DO
END FUNCTION ancestorIsSplitter


! =========================================================================
SUBROUTINE OrganizeSubDiagram(originBox)
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   layout a single loop

          ! METHODOLOGY EMPLOYED:
          !   This routine is really the heart of the entire software
          !   it uses a stack to keep track of undrawn objects.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:

INTEGER, INTENT(IN)   :: originBox

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
INTEGER, ALLOCATABLE, DIMENSION(:)          :: relatives
INTEGER                                     :: iRel
INTEGER                                     :: curBox
INTEGER                                     :: nextBox
INTEGER                                     :: countCurBoxes

!keep track of the maximum number of children to see how far apart the columns of
!boxes should be
maxNumRelatives = 0
! increment the counter on number of subdiagrams and assign to the origin box
lastSubDiagram = lastSubDiagram + 1
! if the array needs to be made larger this call will expand it
CALL expandSubDiagram(lastSubDiagram)
box(originBox)%SubDiagramNum = lastSubDiagram
subDiagram(lastSubDiagram)%RootBox = originBox
!start the final box as the origin box but will be replaced later since
!further to the right
subDiagram(lastSubDiagram)%FinalBox = originBox
! Use a counter to stop run-away looping using the counter and compare against
! the square of the number of boxes (lastBox * lastBox) so that the routine
! can handle a large number of cases gracefully.
countCurBoxes = 0
! place the origin box in the first row and column
grid = 0  !clear the grid
box(originBox)%row = 1
box(originBox)%col = 1
CALL AssignBoxToGrid(originBox)
CALL eraseLineToFirstAncestor(originBox)
! set the current box the first decendant of the origin box
! the origin box cannot be a splitter or a mixer and can only
! have one descendant.
nextBox = getFirstDescendantOf(originBox)
IF (nextBox .EQ. 0) RETURN
box(nextBox)%row = 1
box(nextBox)%col = 2
! push the first box onto the stack.
CALL PushBoxOnStack(nextBox)
DO
  curBox = PopBoxFromStack()  !get the top value of the stack
  IF (curBox .EQ. originBox) CYCLE
  ! loop until back to the origin and stack is empty
  IF (curBox .EQ. emptyStack)  EXIT
  IF (dumpDetails) THEN
    WRITE(UNIT=30, FMT="(A)")  "   Working on box in OrganizeSubDiagram: " // TRIM(box(curBox)%ObjName)
  END IF
  box(curBox)%SubDiagramNum = lastSubDiagram
  SELECT CASE (box(curBox)%splitMix)
    CASE (flagPassThru)
      CALL AssignBoxToGrid(curBox)
      nextBox = getFirstDescendantOf(curBox)
      IF (nextBox .EQ. 0) THEN
        errorFoundInDrawing = .TRUE.
        IF (dumpDetails) THEN
          WRITE(UNIT=30, FMT="(A)")  "Error: No node connection found after the following object: " // TRIM(box(curBox)%ObjName)
        END IF
        lastErrorMessage = 'Error: No node connection found after the following object: ' // box(curBox)%ObjName
        CALL LocateSubDiagrams
        CALL InterSubDiagramLinks
        CALL LocateConnectors
        CALL WriteSVG
        STOP
      END IF
      IF (nextBox .EQ. originBox) THEN
        subDiagram(lastSubDiagram)%FinalBox = curBox
        CYCLE
      END IF
      IF (.NOT. box(nextBox)%isAssigned) THEN
        box(nextBox)%col = box(curBox)%col + 1
        box(nextBox)%row = box(curBox)%row
      END IF
      CALL pushBoxOnStack(nextBox)
    CASE (flagSplitter)
      CALL AssignBoxToGrid(curBox)
      ! for splitters must make the top descendant
      ! the next box but for the rest, need to
      ! push them onto the stack for later work
      ALLOCATE(relatives(box(curBox)%countRelatives))
      CALL getAllDescendantsOf(curBox,relatives)
      ! push all descendants to the stack the first
      ! descendant will be next.  Put the descendants
      ! in reverse order so that the one on top is
      ! the next one that should be done.
      DO iRel = 1,box(curBox)%countRelatives
        nextBox = relatives(iRel)
        IF (.NOT. box(nextBox)%isAssigned) THEN
          box(nextBox)%row = box(curBox)%row + iRel - 1
          box(nextBox)%col = box(curBox)%col + 1
        END IF
        CALL PushBoxOnStack(nextBox)
      END DO
      DEALLOCATE(relatives)
      !keep track of how many relatives the biggest splitter
      !has to size the gap between columns correctly.
      IF (maxNumRelatives .LT. box(curBox)%countRelatives) THEN
        maxNumRelatives = box(curBox)%countRelatives
      END IF
    CASE (flagMixer)
      CALL AssignBoxToGrid(curBox)
      nextBox = getFirstDescendantOf(curBox)
      IF (nextBox .EQ. originBox) THEN
        subDiagram(lastSubDiagram)%FinalBox = curBox
        CYCLE
      END IF
      IF (.NOT. box(nextBox)%isAssigned) THEN
        box(nextBox)%col = box(curBox)%col + 1
        box(nextBox)%row = box(curBox)%row
      END IF
      CALL pushBoxOnStack(nextBox)
  END SELECT
  !catch run away condition
  countCurBoxes = countCurBoxes + 1
  IF (countCurBoxes .GT. (lastBox ** 1.5)) THEN
    IF (dumpDetails) THEN
       WRITE(UNIT=30, FMT="(A)")  "Error: OrganizeSubDiagram stopped from runaway condition."
    END IF
    lastErrorMessage = 'Error: OrganizeSubDiagram stopped from runaway condition.'
    EXIT
  END IF
END DO
CALL alignMixers !move mixers over so that inputs are on left side
END SUBROUTINE OrganizeSubDiagram


! =========================================================================
SUBROUTINE GetNextDanglingOriginBox(originBox)
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   September 2005
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! Returns the box number of the origin for a "dangling" string of objects
          ! that is left after normal loops are drawn.  Return zero if no origin
          ! found.

          ! METHODOLOGY EMPLOYED:
          ! Searchs through the boxes that haven't been used yet and check if
          ! they have a ancestor that also is not used. Keep tracing back
          ! ancestors until come to end or to one that has used.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine


          ! FUNCTION ARGUMENT DEFINITIONS:
INTEGER,INTENT(OUT)  :: originBox

          ! FUNCTION PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! FUNCTION LOCAL VARIABLE DECLARATIONS:
INTEGER :: possibleOrigin
INTEGER :: childBox
INTEGER :: parentBox
INTEGER :: iBox

originBox = 0
possibleOrigin = 0
DO iBox = 1,lastBox
  IF (.NOT. box(iBox)%isAssigned) THEN
    IF (getFirstDescendantOf(iBox) .GT. 0) THEN
      possibleOrigin = iBox
      EXIT
    END IF
  END IF
END DO
! a possible origin was found - trace backwards to find first
! box that is also unassigned
IF (possibleOrigin .GT. 0) THEN
  childBox = possibleOrigin
  ! use fixed loop just to prevent runaway condition
  DO iBox = 1,lastBox
    parentBox = getFirstAncestorOf(childBox)
    IF (parentBox .GT. 0) THEN
      IF (.NOT. box(parentBox)%isAssigned) THEN
        childBox = parentBox
      ELSE
        EXIT
      END IF
    ELSE
      EXIT
    END IF
  END DO
  originBox = childBox
END IF
END SUBROUTINE GetNextDanglingOriginBox

! =========================================================================
SUBROUTINE OrganizeDanglingSubDiagram(originBox)
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   September 2005
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   Layout the subdiagram of dangling boxes that were not
          !   already assigned as part of loops.

          ! METHODOLOGY EMPLOYED:
          !   Assumes that no splitters or mixers and layout is a single
          !   list of boxes connected to one another.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:

INTEGER, INTENT(IN)   :: originBox

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
INTEGER :: parentBox
INTEGER :: childBox
INTEGER :: iBox

IF (dumpDetails) THEN
  WRITE(UNIT=30, FMT="(A,A)") "organize dangling subdiagrams ",box(originBox)%objName
END IF
! increment the counter on number of subdiagrams and assign to the origin box
lastSubDiagram = lastSubDiagram + 1
! if the array needs to be made larger this call will expand it
CALL expandSubDiagram(lastSubDiagram)
childBox = originBox
!set the origin
box(childBox)%SubDiagramNum = lastSubDiagram
subDiagram(lastSubDiagram)%RootBox = childBox
! place the origin box in the first row and column
grid = 0  !clear the grid
box(childBox)%row = 1
box(childBox)%col = 1
CALL AssignBoxToGrid(childBox)
parentBox = childBox
! use do loop to stop runaway condition - not really
! expecting to loop for this many times
DO iBox = 1,lastBox
  childBox = getFirstDescendantOf(parentBox)
  !test if need loop back bridge
  IF (childBox .EQ. originBox) THEN
    subDiagram(lastSubDiagram)%FinalBox = parentBox
  END IF
  !place next box
  IF (childBox .GT. 0) THEN
    IF (.NOT. box(childBox)%isAssigned) THEN
      box(childBox)%SubDiagramNum = lastSubDiagram
      box(childBox)%col = box(parentBox)%col + 1
      box(childBox)%row = box(parentBox)%row
      CALL AssignBoxToGrid(childBox)
      parentBox = childBox
    ELSE
      EXIT
    END IF
  ELSE
    EXIT
  END IF
END DO
END SUBROUTINE OrganizeDanglingSubDiagram

! =========================================================================
SUBROUTINE newConnection(fromBox,toBox)
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   September 2005
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   Make a new connection between two boxes

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:

INTEGER, INTENT(IN)   :: fromBox
INTEGER, INTENT(IN)   :: toBox

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
          ! na
lastConnect = lastConnect + 1
CALL expandConnect(lastConnect)
connect(lastConnect)%BoxFrom = fromBox
connect(lastConnect)%BoxTo = toBox
END SUBROUTINE newConnection

! =========================================================================
SUBROUTINE eraseConnection(fromBox,toBox)
          ! FUNCTION INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! Disables drawing of the line to the first ancestor box

          ! METHODOLOGY EMPLOYED:
          ! Searchs through the connect to and returns the BoxFrom if the
          ! boxTo matches boxNum.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine


          ! FUNCTION ARGUMENT DEFINITIONS:
INTEGER, INTENT(IN)   :: fromBox
INTEGER, INTENT(IN)   :: toBox

          ! FUNCTION PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! FUNCTION LOCAL VARIABLE DECLARATIONS:
INTEGER    :: iConnect

DO iConnect = 1, lastConnect
  IF (connect(iConnect)%BoxTo .EQ. toBox) THEN
    IF (connect(iConnect)%BoxFrom .EQ. fromBox) THEN
      connect(iConnect)%drawLine = .FALSE.
      EXIT
    END IF
  END IF
END DO !iConnect
END SUBROUTINE eraseConnection


! =========================================================================
INTEGER FUNCTION MakeDuplicateBox(boxNum)
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   September 2005
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   Create a copy of a box as the last box in the array

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:

INTEGER, INTENT(IN)   :: boxNum

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
          ! na

lastBox = lastBox + 1
CALL expandBox(lastBox)
box(lastBox)%ObjName = box(boxNum)%ObjName
box(lastBox)%TypeObj = box(boxNum)%TypeObj
box(lastBox)%colorToUse = box(boxNum)%colorToUse
box(lastBox)%FluidStream = box(boxNum)%FluidStream
box(lastBox)%splitMix = box(boxNum)%splitMix
box(lastBox)%countRelatives = box(boxNum)%countRelatives
box(lastBox)%countRows = box(boxNum)%countRows
box(lastBox)%SubDiagramNum = 0
box(lastBox)%row = 0
box(lastBox)%col = 0
box(lastBox)%isAssigned = .FALSE.
box(lastBox)%xUpperLeft = 0
box(lastBox)%yUpperLeft = 0
box(lastBox)%xLowerRight = 0
box(lastBox)%yLowerRight = 0
box(lastBox)%isOrigin = .FALSE.
MakeDuplicateBox = lastBox
END FUNCTION MakeDuplicateBox

! =========================================================================
SUBROUTINE AssignBoxToGrid(BoxNum)
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   February 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! Assign a value to the grid.

          ! METHODOLOGY EMPLOYED:
          ! Check if the grid needs to be expanded prior
          ! to assigning value.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
INTEGER               :: rowNum
INTEGER               :: colNum
INTEGER, INTENT(IN)   :: BoxNum

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:

IF (.NOT. box(BoxNum)%isAssigned) THEN
  !resize the grid if necessary
  rowNum = Box(BoxNum)%row
  colNum = Box(BoxNum)%col
  CALL ExpandGrid(rowNum,colNum)
  ! loop through succeeding rows until an empty location is found
  DO WHILE (grid(rowNum,colNum) .NE. 0)
    !increment row
    rowNum = rowNum +1
    CALL ExpandGrid(rowNum,colNum)
  END DO
  grid(rowNum,colNum) = BoxNum
  box(BoxNum)%row = rowNum
  box(BoxNum)%col = colNum
  box(BoxNum)%countRows = 1 !for now all boxes the same size
  box(BoxNum)%isAssigned = .TRUE.
END IF
END SUBROUTINE AssignBoxToGrid

! =========================================================================
SUBROUTINE RowColIntoXY(curSubDiagram)
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! Convert the row and column placement of the box into coordinates
          ! that are used in the diagram (x,y).

          ! METHODOLOGY EMPLOYED:
          ! Use the size of the boxes and the gaps to place the boxes and
          ! connectors on the diagram.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
INTEGER, INTENT(IN)   :: curSubDiagram

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
INTEGER :: iBox
INTEGER :: xPosUpLeft
INTEGER :: yPosUpLeft
INTEGER :: maximumY
INTEGER :: curGapWidth

curGapWidth = gapWidth * (1 + maxNumRelatives / 4)
IF (curGapWidth .GT. 200) THEN
  curGapWidth = 200
END IF
subDiagram(curSubDiagram)%gapWidth = curGapWidth
maximumY = 0
DO iBox = 1, lastBox
  IF (box(iBox)%SubDiagramNum .EQ. curSubDiagram) THEN
    xPosUpLeft = (box(iBox)%col - 1) * (boxWidth + curGapWidth)
    yPosUpLeft = (box(iBox)%row - 1) * (boxHeight + gapHeight)
    box(iBox)%xUpperLeft = xPosUpLeft
    box(iBox)%yUpperLeft = yPosUpLeft
    box(iBox)%xLowerRight = xPosUpLeft + boxWidth
    box(iBox)%yLowerRight = yPosUpLeft &
                            + boxHeight * box(iBox)%countRows &
                            + gapHeight * (box(iBox)%countRows - 1)
    IF (box(iBox)%yLowerRight .GT. maximumY) THEN
      maximumY = box(iBox)%yLowerRight
    END IF
  END IF
END DO
subDiagram(curSubDiagram)%maxY = maximumY
END SUBROUTINE RowColIntoXY

! =========================================================================
SUBROUTINE PushBoxOnStack(boxNum)
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! The stack is used during the layout of boxes to keep track of
          ! which boxes are still needed to be laid out.

          ! METHODOLOGY EMPLOYED:
          ! Simple LIFO stack.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
INTEGER, INTENT(IN)   :: boxNum

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:

boxStackTop = boxStackTop + 1
!if the stack is larger than the maxSize then resize the stack
!by copying the stack to a copy array
IF (BoxStackTop .GT. maxSizeBoxStack) THEN
  ALLOCATE(boxStackCopy(maxSizeBoxStack * sizeFactorBoxStack))
  boxStackCopy = 0
  boxStackCopy(1:maxSizeBoxStack) = boxStack
  DEALLOCATE(boxStack)
  ALLOCATE(boxStack(maxSizeBoxStack * sizeFactorBoxStack))
  boxStack = boxStackCopy
  DEALLOCATE(boxStackCopy)
  maxSizeBoxStack = maxSizeBoxStack * sizeFactorBoxStack
END IF
boxStack(boxStackTop) = boxNum
END SUBROUTINE PushBoxOnStack

! =========================================================================
INTEGER FUNCTION PopBoxFromStack()
          ! FUNCTION INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! Returns the index to the box that is the ancester of the box index
          ! 'boxNum'.

          ! METHODOLOGY EMPLOYED:
          ! Searchs through the connect to and returns the BoxFrom if the
          ! boxTo matches boxNum.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine


          ! FUNCTION ARGUMENT DEFINITIONS:
          ! na

          ! FUNCTION PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! FUNCTION LOCAL VARIABLE DECLARATIONS:

IF (boxStackTop .GE. 1) THEN
  PopBoxFromStack = boxStack(boxStackTop)
  boxStackTop = boxStackTop - 1
ELSE
  PopBoxFromStack = emptyStack
END IF
END FUNCTION PopBoxFromStack

! =========================================================================
INTEGER FUNCTION GetValueGrid(rowNum,colNum)
          ! FUNCTION INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! Get value from the grid

          ! METHODOLOGY EMPLOYED:
          ! Check if grid is large enough prior to pulling value out of it.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine


          ! FUNCTION ARGUMENT DEFINITIONS:
INTEGER, INTENT(IN)   :: rowNum
INTEGER, INTENT(IN)   :: colNum

          ! FUNCTION PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! FUNCTION LOCAL VARIABLE DECLARATIONS:
CALL ExpandGrid(rowNum,colNum)
GetValueGrid = grid(rowNum,colNum)
END FUNCTION GetValueGrid

! =========================================================================
SUBROUTINE ExpandGrid(possRow,possCol)
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! Check if the grid array is large enough and make it bigger
          ! if necesary

          ! METHODOLOGY EMPLOYED:
          ! Allocate a temporary array and copy the values while
          ! reallocating the main array.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
INTEGER, INTENT(IN)   :: possRow
INTEGER, INTENT(IN)   :: possCol

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
INTEGER :: increaseRow = 0
INTEGER :: increaseCol = 0
LOGICAL :: needIncrease

! first assume that no increase in size is needed
needIncrease = .FALSE.
! check the row
IF (possRow .GE. maxSizeGridRow) THEN
  increaseRow = sizeIncrementGrid
  IF (possRow .GE. maxSizeGridRow + increaseRow) THEN  !if it is still not big enough then add the new size
    increaseRow = sizeIncrementGrid + (possRow - maxSizeGridRow)
  END IF
  needIncrease = .TRUE.
END IF
! check the column
IF (possCol .GE. maxSizeGridCol) THEN
  increaseCol = sizeIncrementGrid
  IF (possCol .GE. maxSizeGridCol + increaseCol) THEN  !if it is still not big enough then add the new size
    increaseCol = sizeIncrementGrid + (possCol - maxSizeGridCol)
  END IF
  needIncrease = .TRUE.
END IF
! if either the row or column needs to be changed
IF (needIncrease) THEN
  ALLOCATE(gridCopy(maxSizeGridRow + increaseRow, maxSizeGridCol + increaseCol))
  gridCopy = 0 !initialize
  gridCopy(1:maxSizeGridRow,1:maxSizeGridCol) = grid
  DEALLOCATE(grid)
  ALLOCATE(grid(maxSizeGridRow + increaseRow, maxSizeGridCol + increaseCol))
  grid = gridCopy
  DEALLOCATE(gridCopy)
  maxSizeGridRow = maxSizeGridRow + increaseRow
  maxSizeGridCol = maxSizeGridCol + increaseCol
END IF
END SUBROUTINE ExpandGrid

! =========================================================================
SUBROUTINE alignMixers
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   September 2005
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! If mixers are found that are not alone in a column, move over
          ! the rest of the diagram until the mixer is alone in the column

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
LOGICAL :: foundColWithMixAndOther
INTEGER :: curSubDiagram
INTEGER :: curColumn
INTEGER :: mixerCount
INTEGER :: iBox
INTEGER :: jBox
! count the number of mixers in the subdiagram
mixerCount = 0
DO iBox = 1, lastBox
  IF (box(iBox)%isAssigned) THEN
    !find a mixer
    IF  (box(iBox)%splitMix .EQ. flagMixer) THEN
      mixerCount = mixerCount + 1
    END IF
  END IF
END DO
! if the number of mixers is too large, do not shift them but leave drawing alone
! the threshold is arbitrary but tries to balance the benefit of seeing the mixers
! separately with exceptionally wide drawings.
IF (mixerCount .LE. 12) THEN
  !keep repeating until no mixers are found in columns
  !with other boxes
  DO
    !reset the flag that found a mixer with another box in same column
    foundColWithMixAndOther = .FALSE.
    DO iBox = 1, lastBox
      IF (box(iBox)%isAssigned) THEN
        !find a mixer
        IF  (box(iBox)%splitMix .EQ. flagMixer) THEN
          curSubDiagram = box(iBox)%SubDiagramNum
          curColumn = box(iBox)%col
          !search for other boxes in the same column in the
          !same subdiagram
          DO jBox = 1,lastBox
            IF (box(jBox)%SubDiagramNum .EQ. curSubDiagram) THEN
              IF (box(jBox)%col .EQ. curColumn) THEN
                IF (iBox .NE. jBox) THEN
                  !found something else in current column
                  foundColWithMixAndOther = .TRUE.
                  !move everything that is to the right of the box
                  !one column further to the right
                  CALL moveRightBoxesRight(iBox)
                  !first unassign box on grid
                  grid(box(iBox)%row,box(iBox)%col) = 0
                  !now move the mixer itself
                  box(iBox)%col = box(iBox)%col + 1
                  !reassign the box to the grid
                  CALL AssignBoxToGrid(iBox)
                  EXIT
                END IF
              END IF
            END IF
          END DO
        END IF
      END IF
    END DO
    IF (.NOT. foundColWithMixAndOther) EXIT
  END DO
END IF
END SUBROUTINE alignMixers

! =========================================================================
SUBROUTINE moveRightBoxesRight(curBox)
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   September 2005
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! Move any boxes that are in the same diagram as the current
          ! box that are in a column to the right, one additional column
          ! to the right.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
INTEGER, INTENT(IN)   :: curBox

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
INTEGER :: curSubDiagram
INTEGER :: curCol
INTEGER :: iBox

curSubDiagram = box(curBox)%SubDiagramNum
curCol = box(curBox)%col
DO iBox = 1, lastBox
  IF (box(iBox)%SubDiagramNum .EQ. curSubDiagram) THEN
    IF (box(iBox)%col .GT. curCol) THEN
      !first unassign box on grid
      grid(box(iBox)%row,box(iBox)%col) = 0
      !move the box over one column
      box(iBox)%col = box(iBox)%col + 1
      !reassign the box to the grid
      CALL AssignBoxToGrid(iBox)
    END IF
  END IF
END DO
END SUBROUTINE moveRightBoxesRight


! =========================================================================
SUBROUTINE LocateSubDiagrams
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   February 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! Locate each of the subdiagrams within a single coordinate space.

          ! METHODOLOGY EMPLOYED:
          ! Each subdiagram has its own coordinate system but the intention
          ! is to stack them up vertically. Use the maxY value for each
          ! subdiagram as an offset to the next subdiagram and revise the
          ! Y values of all of the referenced boxes to include the new offset.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
INTEGER :: iSub
INTEGER :: jBox
INTEGER :: xOffset
INTEGER :: yOffset

xOffset = boxWidth  ! leave small gap at the left of the diagram
yOffset = boxHeight  ! leave small gap at top of the diagram
DO iSub = 1, lastSubDiagram
  subDiagram(iSub)%OffsetX = xOffset
  subDiagram(iSub)%OffsetY = yOffset
  DO jBox = 1, lastBox
    IF (box(jBox)%SubDiagramNum .EQ. iSub) THEN !for all boxes in the subdiagram
      box(jBox)%xUpperLeft =  box(jBox)%xUpperLeft  + xOffset
      box(jBox)%yUpperLeft =  box(jBox)%yUpperLeft  + yOffset
      box(jBox)%xLowerRight = box(jBox)%xLowerRight + xOffset
      box(jBox)%yLowerRight = box(jBox)%yLowerRight + yOffset
      ! check if new lower right coordinates are larger than the maximum
      IF (box(jBox)%xLowerRight .GT. canvasMaximumX) THEN
        canvasMaximumX = box(jBox)%xLowerRight
      END IF
      IF (box(jBox)%yLowerRight .GT. canvasMaximumY) THEN
        canvasMaximumY = box(jBox)%yLowerRight
      END IF
    END IF
  END DO
  !for now don't increment the X offset since all diagrams simply stacked
  yOffset = yOffset + subDiagram(iSub)%maxY + boxHeight * 3
END DO
END SUBROUTINE LocateSubDiagrams

! =========================================================================
SUBROUTINE InterSubDiagramLinks
          ! FUNCTION INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   September 2005
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! When boxes appear in more than one diagram that represent
          ! the same piece of equipment a line is created to go between
          ! them.

          ! METHODOLOGY EMPLOYED:
          ! Brute force compare all boxes on name and type

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine


          ! FUNCTION ARGUMENT DEFINITIONS:
          ! na

          ! FUNCTION PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! FUNCTION LOCAL VARIABLE DECLARATIONS:
INTEGER :: iBox
INTEGER :: jBox
DO iBox = 1, lastBox - 1
  DO jBox = iBox + 1, lastBox
    IF (box(iBox)%ObjName .EQ. box(jBox)%objName) THEN
      IF (box(iBox)%TypeObj .EQ. box(jBox)%TypeObj) THEN
        !found a pair that should be connected
        lastConnect = lastConnect + 1
        CALL expandConnect(lastConnect)
        connect(lastConnect)%BoxFrom = iBox
        connect(lastConnect)%BoxFrom = jBox
        connect(lastConnect)%isInterLink = .TRUE.
        connect(lastConnect)%drawLine = .TRUE.
        !locate start and end of line in the center of the box
        connect(lastConnect)%StartX = (box(ibox)%xUpperLeft + box(ibox)%xLowerRight) / 2
        connect(lastConnect)%StartY = (box(ibox)%yUpperLeft + box(ibox)%yLowerRight) / 2
        connect(lastConnect)%EndX = (box(jbox)%xUpperLeft + box(jbox)%xLowerRight) / 2
        connect(lastConnect)%EndY = (box(jbox)%yUpperLeft + box(jbox)%yLowerRight) / 2
      END IF
    END IF
  END DO
END DO
END SUBROUTINE InterSubDiagramLinks

! =========================================================================
INTEGER FUNCTION getFirstAncestorOf(boxNum)
          ! FUNCTION INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! Returns the index to the box that is the ancester of the box index
          ! 'boxNum'.

          ! METHODOLOGY EMPLOYED:
          ! Searchs through the connect to and returns the BoxFrom if the
          ! boxTo matches boxNum.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine


          ! FUNCTION ARGUMENT DEFINITIONS:
INTEGER, INTENT(IN)   :: boxNum

          ! FUNCTION PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! FUNCTION LOCAL VARIABLE DECLARATIONS:
INTEGER    :: iConnect

getFirstAncestorOf = 0
DO iConnect = 1, lastConnect
  IF (connect(iConnect)%BoxTo .EQ. boxNum) THEN
    getFirstAncestorOf = connect(iConnect)%BoxFrom
    EXIT
  END IF
END DO !iConnect
IF (dumpDetails) THEN
  IF (getFirstAncestorOf .NE. 0) THEN
    WRITE(UNIT=30, FMT="(A,A,2X,A)") "  getFirstAncestorOf and result: ", TRIM(box(boxNum)%ObjName), TRIM(box(getFirstAncestorOf)%ObjName)
  ELSE
    WRITE(UNIT=30, FMT="(A,A)") "  getFirstAncestorOf is zero: ", TRIM(box(boxNum)%ObjName)
  ENDIF
END IF
END FUNCTION getFirstAncestorOf

! =========================================================================
SUBROUTINE eraseLineToFirstAncestor(boxNum)
          ! FUNCTION INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! Disables drawing of the line to the first ancestor box

          ! METHODOLOGY EMPLOYED:
          ! Searchs through the connect to and returns the BoxFrom if the
          ! boxTo matches boxNum.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine


          ! FUNCTION ARGUMENT DEFINITIONS:
INTEGER, INTENT(IN)   :: boxNum

          ! FUNCTION PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! FUNCTION LOCAL VARIABLE DECLARATIONS:
INTEGER    :: iConnect

DO iConnect = 1, lastConnect
  IF (connect(iConnect)%BoxTo .EQ. boxNum) THEN
    connect(iConnect)%drawLine = .FALSE.
    EXIT
  END IF
END DO !iConnect
END SUBROUTINE eraseLineToFirstAncestor


! =========================================================================
INTEGER FUNCTION getFirstDescendantOf(boxNum)
          ! FUNCTION INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! Returns the index to the box that is the descendant of the box index
          ! 'boxNum'.

          ! METHODOLOGY EMPLOYED:
          ! Searchs through the connect to and returns the BoxTo if the
          ! boxFrom matches boxNum.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine


          ! FUNCTION ARGUMENT DEFINITIONS:
INTEGER, INTENT(IN)   :: boxNum

          ! FUNCTION PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! FUNCTION LOCAL VARIABLE DECLARATIONS:
INTEGER    :: iConnect

getFirstDescendantOf = 0
DO iConnect = 1, lastConnect
  IF (connect(iConnect)%BoxFrom .EQ. boxNum) THEN
    getFirstDescendantOf = connect(iConnect)%BoxTo
    EXIT
  END IF
END DO !iConnect
END FUNCTION getFirstDescendantOf

! =========================================================================
SUBROUTINE getAllDescendantsOf(boxNum,listOfBoxes)
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! Fill the listOfBoxes array with all the references to boxes

          ! METHODOLOGY EMPLOYED:
          ! Scan the connect array.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
INTEGER, INTENT(IN)                 :: boxNum
INTEGER, INTENT(OUT), DIMENSION(:)  :: listOfBoxes

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
INTEGER :: sizeOfListOfBoxes
INTEGER :: cnt
INTEGER :: iConnect

cnt = 0
sizeOfListOfBoxes = SIZE(listOfBoxes)
DO iConnect = 1, lastConnect
  IF (connect(iConnect)%BoxFrom .EQ. boxNum) THEN
    cnt = cnt + 1
    IF (cnt .GT. sizeOfListOfBoxes) THEN
      PRINT "(A) ","  ERROR: Size mismatch in getAllDescendantsOf"
      IF (dumpDetails) THEN
        WRITE(UNIT=30,FMT="(A)") "  ERROR: Size mismatch in getAllDescendantsOf"
      END IF
      lastErrorMessage = "Size mismatch in getAllDescendantsOf"
      EXIT
    END IF
    listOfBoxes(cnt) = connect(iConnect)%BoxTo
  END IF
END DO !iConnect
! check if this matches the count
IF (cnt .NE. box(boxNum)%countRelatives) THEN
  PRINT "(A) ","  ERROR: Number of descendants do not match"
  IF (dumpDetails) THEN
    WRITE(UNIT=30,FMT="(A)") "  ERROR: Number of descendants do not match"
  END IF
  lastErrorMessage = "Number of descendants do not match"
END IF
END SUBROUTINE getAllDescendantsOf


! =========================================================================
SUBROUTINE LocateConnectors
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   February 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   Assign x,y locations to the start and end of all
          !   connecting lines

          ! METHODOLOGY EMPLOYED:
          ! <description>

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
INTEGER :: iConnect
INTEGER :: curFrom
INTEGER :: curTo
INTEGER :: curGapWidth

DO iConnect = 1, lastConnect
  IF (connect(iConnect)%drawLine) THEN
    IF (.NOT. connect(iConnect)%isInterLink) THEN
      curFrom = connect(iConnect)%BoxFrom
      curTo = connect(iConnect)%BoxTo
      !locate the beginning of the connector
      ! the x value of the start is the right edge of box
      connect(iConnect)%StartX = box(curFrom)%xLowerRight
      ! put the height of the start at half the height from bottom to top of box
      connect(iConnect)%StartY = (box(curFrom)%yUpperLeft + box(curFrom)%yLowerRight ) / 2
      !locate the end of the connector
      ! the x value of the end is the left edge of box
      connect(iConnect)%EndX = box(curTo)%xUpperLeft
      ! put the height of the end at half the height from bottom to top of box
      connect(iConnect)%EndY = (box(curTo)%yUpperLeft + box(curTo)%yLowerRight ) / 2
      ! check if the line should bend to avoid going through another box
      ! assume no mid point is needed
      connect(iConnect)%useMidPoint = .FALSE.
      ! just check if the columns are more than one apart
      IF (box(curTo)%col .GT. (box(curFrom)%col + 1)) THEN
        connect(iConnect)%useMidPoint = .TRUE.
        !find out how big the gap width is
        IF (box(curTo)%SubDiagramNum .NE. 0) THEN
          curGapWidth = subDiagram(box(curTo)%SubDiagramNum)%gapWidth
          !draw the line that is between a row of boxes
          !if "to" box is lower than "from" box snake the line below
          IF (box(curTo)%yUpperLeft .GT. box(curFrom)%yUpperLeft) THEN
            connect(iConnect)%MidX1 = connect(iConnect)%StartX + curGapWidth
            connect(iConnect)%MidY1 = box(curFrom)%yLowerRight + gapHeight/2
            connect(iConnect)%MidX2 = connect(iConnect)%EndX - curGapWidth
            connect(iConnect)%MidY2 = box(curFrom)%yLowerRight + gapHeight/2
          ELSE
            connect(iConnect)%MidX1 = connect(iConnect)%StartX
            connect(iConnect)%MidY1 = connect(iConnect)%StartY
            connect(iConnect)%MidX2 = connect(iConnect)%StartX
            connect(iConnect)%MidY2 = connect(iConnect)%StartY
          END IF
        END IF
      END IF
    END IF
  END IF
END DO
END SUBROUTINE LocateConnectors

! =========================================================================
SUBROUTINE LocateLoopBackBridges
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   September 2005
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! Create lines that loop around the diagram from the last box
          ! to the first box.

          ! METHODOLOGY EMPLOYED:
          ! na

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
INTEGER :: originBox
INTEGER :: endBox
INTEGER :: iDiagram

DO iDiagram = 1, lastSubDiagram
  originBox = subDiagram(iDiagram)%RootBox
  endBox = subDiagram(iDiagram)%FinalBox
  IF ((endBox .GT.0) .AND. (originBox .NE. endBox)) THEN
    subDiagram(iDiagram)%drawBridge = .TRUE.
    ! make sure no direct line is drawn
    !CALL eraseConnection(originBox,endBox)
    CALL eraseConnection(endBox,originBox)
    ! the x value of the start is the right edge of first box in the diagram
    ! put the height of the start at half the height from bottom to top of box
    subDiagram(iDiagram)%bridge1X = box(originBox)%xUpperLeft
    subDiagram(iDiagram)%bridge1Y = (box(originBox)%yUpperLeft + box(originBox)%yLowerRight ) / 2
    ! now move left but keep the height the same
    subDiagram(iDiagram)%bridge2X = subDiagram(iDiagram)%bridge1X -  gapWidth / 2
    subDiagram(iDiagram)%bridge2Y = subDiagram(iDiagram)%bridge1Y
    ! move above the diagram but keep the X the same
    subDiagram(iDiagram)%bridge3X = subDiagram(iDiagram)%bridge2X
    subDiagram(iDiagram)%bridge3Y = box(originBox)%yUpperLeft - boxHeight
    ! move above the diagram but keep the Y the same
    subDiagram(iDiagram)%bridge4X = box(endBox)%xLowerRight +  gapWidth / 2
    subDiagram(iDiagram)%bridge4Y = subDiagram(iDiagram)%bridge3Y
    ! move down left side of diagram and keep the X the same
    subDiagram(iDiagram)%bridge5X = subDiagram(iDiagram)%bridge4X
    subDiagram(iDiagram)%bridge5Y = (box(endBox)%yUpperLeft + box(endBox)%yLowerRight ) / 2
    ! now connect to the last box and keep the Y the same
    subDiagram(iDiagram)%bridge6X = box(endBox)%xLowerRight
    subDiagram(iDiagram)%bridge6Y = subDiagram(iDiagram)%bridge5Y
  END IF
END DO
!add some to edge of canvas to draw lines
canvasMaximumX =  canvasMaximumX + 2 * gapWidth
END SUBROUTINE LocateLoopBackBridges


! =========================================================================
SUBROUTINE WriteSVG
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !  Create the SVG file by writing simple text file with
          !  XML markup for the SVG specification.

          ! METHODOLOGY EMPLOYED:
          ! <description>

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
INTEGER,PARAMETER :: maxSplit = 36

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
INTEGER :: iBox
INTEGER :: jBox
INTEGER :: jConnect
INTEGER :: unusedY
INTEGER :: splitLong
INTEGER :: iDiagram

unusedY = 0

OPEN (UNIT=20, FILE="eplusout.svg", ACTION="WRITE")
WRITE(UNIT=20, FMT="(A)") '<?xml version="1.0"?>'
IF (canvasMaximumX .LT. 2500)   canvasMaximumX = 2500
IF (canvasMaximumY .LT. 1500)   canvasMaximumY = 1500
WRITE(UNIT=20, FMT="(A,I0,1X,I0,A)") '<svg width="250mm" height="200mm" viewBox="0 0 ',canvasMaximumX, canvasMaximumY,'"'
WRITE(UNIT=20, FMT="(A)") '     xmlns="http://www.w3.org/2000/svg"'
WRITE(UNIT=20, FMT="(A)") '     xmlns:xlink="http://www.w3.org/1999/xlink"'
WRITE(UNIT=20, FMT="(A)") '     xmlns:ev="http://www.w3.org/2001/xml-events">'
! set the font size
WRITE(UNIT=20, FMT="(A)") '<g font-family="Verdana" font-size="8">'
IF (errorFoundInDrawing) THEN
  WRITE(UNIT=20, FMT="(A,I0,A,I0,A,A,A)") &
         '  <text x="',1, &
         '" y="',1, &
         '">', &
         'ERROR - COULD NOT COMPLETE DRAWING! Examine ERR file for possible explanation. Last error message is:   "'  &
         // TRIM(lastErrorMessage) //'"','</text>'
END IF
!draw interdiagram linking lines first - they should be behind everything else
DO jConnect = 1, lastConnect
  IF (connect(jConnect)%drawLine .AND. connect(jConnect)%isInterLink) THEN
    IF ((connect(jConnect)%EndX .GT. 2) .AND. (connect(jConnect)%StartX .GT. 2)) THEN
      !normal direct line with no mid point
      CALL WriteLineSegmentSVG(20,connect(jConnect)%StartX,connect(jConnect)%StartY, &
                                    connect(jConnect)%EndX,  connect(jConnect)%EndY,.FALSE.)
    END IF
  END IF
END DO
! Draw the boxes
CALL colorizeBoxes
DO iBox = 1, lastBox
  IF (box(iBox)%isAssigned) THEN
    WRITE(UNIT=20, FMT="(A,I0,A,I0,A,I0,A,I0,A,A,A,I0)") &
         '  <rect x="',box(iBox)%xUpperLeft, &
         '" y="',      box(iBox)%yUpperLeft, &
         '" width="',  box(iBox)%xLowerRight - box(iBox)%xUpperLeft, &
         '" height="', box(iBox)%yLowerRight - box(iBox)%yUpperLeft, &
         '" style="fill:', &
         TRIM(toColor(box(iBox)%colorToUse)%ColorName), &
         '; stroke: black;"/>',ibox
    !
    ! Enhancement on September 2005 by J. Glazer
    !
    ! If the name in the box is long split it into two
    ! lines instead of a single line.
    !
    IF (LEN_TRIM(box(iBox)%ObjName) .LE. maxSplit) THEN
      ! Draw the box name for single line of text
      WRITE(UNIT=20, FMT="(A,I0,A,I0,A,A,A)") &
         '  <text x="',box(iBox)%xUpperLeft + 2, &
         '" y="',      box(iBox)%yLowerRight - 7, &
         '">', &
         TRIM(box(iBox)%ObjName), &
         '</text>'
    ELSE
      ! Draw the box name for split line of text
      ! into two lines
      splitLong = INDEX(box(iBox)%ObjName(1:maxSplit),' ',.TRUE.)
      IF (splitLong .EQ. 0) THEN
        splitLong = maxSplit
      ELSEIF (splitLong .GE. maxSplit) THEN
        splitLong = maxSplit
      END IF
      WRITE(UNIT=20, FMT="(A,I0,A,I0,A,A)") &
         '  <text  x="',box(iBox)%xUpperLeft + 2, &
         '" y="',      box(iBox)%yLowerRight - 12, &
         '">', &
         TRIM(box(iBox)%ObjName(1:splitLong))
      WRITE(UNIT=20, FMT="(A,I0,A,I0,A,A,A)") &
         '  <tspan x="',box(iBox)%xUpperLeft + 2, &
         '" y="',      box(iBox)%yLowerRight - 4, &
         '">', &
         TRIM(box(iBox)%ObjName((splitLong+1):)), &
         '</tspan></text>'
    END IF
    IF (unusedY .LT. box(iBox)%yLowerRight) unusedY = box(iBox)%yLowerRight
  END IF
END DO
! Draw the connectors within a subdiagram
DO jConnect = 1, lastConnect
  IF (connect(jConnect)%drawLine .AND. .NOT. connect(jConnect)%isInterLink) THEN
    IF ((connect(jConnect)%EndX .GT. 2) .AND. (connect(jConnect)%StartX .GT. 2)) THEN
      IF (.NOT. connect(jConnect)%useMidPoint) THEN
        !normal direct line with no mid point
        CALL WriteLineSegmentSVG(20,connect(jConnect)%StartX,connect(jConnect)%StartY, &
                                    connect(jConnect)%EndX,  connect(jConnect)%EndY,.TRUE.)
      ELSE
        !bent line segments
        CALL WriteLineSegmentSVG(20,connect(jConnect)%StartX,connect(jConnect)%StartY, &
                                    connect(jConnect)%MidX1, connect(jConnect)%MidY1,.TRUE.)
        CALL WriteLineSegmentSVG(20,connect(jConnect)%MidX1, connect(jConnect)%MidY1, &
                                    connect(jConnect)%MidX2, connect(jConnect)%MidY2,.TRUE.)
        CALL WriteLineSegmentSVG(20,connect(jConnect)%MidX2, connect(jConnect)%MidY2, &
                                    connect(jConnect)%EndX,  connect(jConnect)%EndY,.TRUE.)
      END IF
    END IF
  END IF
END DO
!Draw loop back bridges for diagrams that go from the last box to the orgin
DO iDiagram = 1, lastSubDiagram
  IF (subDiagram(iDiagram)%drawBridge) THEN
    CALL WriteLineSegmentSVG(20,subDiagram(iDiagram)%bridge1X,subDiagram(iDiagram)%bridge1Y, &
                                subDiagram(iDiagram)%bridge2X,subDiagram(iDiagram)%bridge2Y,.TRUE.)
    CALL WriteLineSegmentSVG(20,subDiagram(iDiagram)%bridge2X,subDiagram(iDiagram)%bridge2Y, &
                                subDiagram(iDiagram)%bridge3X,subDiagram(iDiagram)%bridge3Y,.TRUE.)
    CALL WriteLineSegmentSVG(20,subDiagram(iDiagram)%bridge3X,subDiagram(iDiagram)%bridge3Y, &
                                subDiagram(iDiagram)%bridge4X,subDiagram(iDiagram)%bridge4Y,.TRUE.)
    CALL WriteLineSegmentSVG(20,subDiagram(iDiagram)%bridge4X,subDiagram(iDiagram)%bridge4Y, &
                                subDiagram(iDiagram)%bridge5X,subDiagram(iDiagram)%bridge5Y,.TRUE.)
    CALL WriteLineSegmentSVG(20,subDiagram(iDiagram)%bridge5X,subDiagram(iDiagram)%bridge5Y, &
                                subDiagram(iDiagram)%bridge6X,subDiagram(iDiagram)%bridge6Y,.TRUE.)
  END IF
END DO
!confirm unused boxes are used elsewhere in the diagram
DO iBox = 1, lastBox
  IF ((.NOT. box(iBox)%isAssigned) .AND. (LEN_TRIM(box(iBox)%ObjName) .GT. 0)) THEN
    !check if it has been assigned previously
    DO jBox = 1, lastBox
      IF (box(iBox)%ObjName .EQ. box(jBox)%ObjName) THEN
        IF (box(jBox)%isAssigned) THEN
          box(iBox)%isAssigned = .TRUE.
        END IF
      END IF
    END DO
  END IF
END DO
!write unused boxes
unusedY = unusedY +  2 * boxHeight
WRITE(UNIT=20, FMT="(A,I0,A,I0,A,A,A)") &
         '  <text x="',boxWidth, &
         '" y="',      unusedY, &
         '">', &
         'Unused Non-Parent Objects:', &
         '</text>'
unusedY = unusedY + boxHeight
DO iBox = 1, lastBox
  IF ((.NOT. box(iBox)%isAssigned) .AND. (LEN_TRIM(box(iBox)%ObjName) .GT. 0)) THEN
    WRITE(UNIT=20, FMT="(A,I0,A,I0,A,A,A)") &
         '  <text x="',boxWidth + 20, &
         '" y="',      unusedY, &
         '">', &
         TRIM(box(iBox)%ObjName) , &
         '</text>'
    unusedY = unusedY +  boxHeight
  END IF
END DO
unusedY = unusedY + boxHeight
WRITE(UNIT=20, FMT="(A,I0,A,I0,A)") &
         '  <text x="',boxWidth, &
         '" y="',      unusedY, &
         '">', &
!         'The following are not supported:  objects related to controls, DIRECT AIR, or  PURCHASED AIR</text>' fixed line to newer names for CR8043
         'The following are not supported: objects related to controls or ZoneHVAC:IdealLoadsAirSystem</text>'

WRITE(UNIT=20, FMT="(A)") '</g>'
WRITE(UNIT=20, FMT="(A)") '</svg>'
CLOSE (UNIT=20)

END SUBROUTINE WriteSVG

! =========================================================================
SUBROUTINE WriteLineSegmentSVG(fileHandle,startX,startY,endX,endY,isBlack)
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   September 2005
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !  Write to the SVG file a line segment

          ! METHODOLOGY EMPLOYED:
          ! <description>

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:

          ! SUBROUTINE PARAMETER DEFINITIONS:
INTEGER, INTENT(IN) :: fileHandle
INTEGER, INTENT(IN) :: startX
INTEGER, INTENT(IN) :: startY
INTEGER, INTENT(IN) :: endX
INTEGER, INTENT(IN) :: endY
LOGICAL, INTENT(IN) :: isBlack

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
IF (isBlack) THEN
  WRITE(UNIT=fileHandle, FMT="(A,I0,A,I0,A,I0,A,I0,A)") &
         '  <line x1="',startX, &
         '" y1="',      startY, &
         '" x2="',      endX, &
         '" y2="',      endY, &
         '" style="stroke:black;"/>'
ELSE
  WRITE(UNIT=fileHandle, FMT="(A,I0,A,I0,A,I0,A,I0,A)") &
         '  <line x1="',startX, &
         '" y1="',      startY, &
         '" x2="',      endX, &
         '" y2="',      endY, &
         '" style="stroke:silver;"/>'    !honeydew  linen        lightyellow
END IF
END SUBROUTINE WriteLineSegmentSVG


! =========================================================================
SUBROUTINE colorizeBoxes
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !  Associate colors with different object types

          ! METHODOLOGY EMPLOYED:
          ! <description>

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
INTEGER :: iBox
INTEGER :: jColor


toColor(1)%ObjType = 'EvaporativeCooler:Direct:CelDekPad' !rename by JG
toColor(1)%ColorName = 'aliceblue'
toColor(2)%ObjType = 'EvaporativeCooler:Indirect:CelDekPad' !rename by JG
toColor(2)%ColorName = 'aliceblue'
toColor(3)%ObjType = 'EvaporativeCooler:Indirect:UserEffectiveness' !rename by JG
toColor(3)%ColorName = 'aliceblue'
toColor(4)%ObjType = 'EvaporativeCooler:Indirect:WetCoil' !rename by JG
toColor(4)%ColorName = 'aliceblue'
toColor(5)%ObjType = 'ZoneHVAC:EquipmentConnections' !renamed by script
toColor(5)%ColorName = 'chartreuse'
toColor(6)%ObjType = 'Boiler:HotWater' !renamed by script
toColor(6)%ColorName = 'indianred'
toColor(7)%ObjType = 'Boiler:Steam' !renamed by script
toColor(7)%ColorName = 'indianred'
toColor(8)%ObjType = 'Humidifier:Steam:Electric' !renamed by script
toColor(8)%ColorName = 'lavender'
toColor(9)%ObjType = 'OutdoorAir:Mixer' !renamed by script
toColor(9)%ColorName = 'lawngreen'
toColor(10)%ObjType = 'Connector:Mixer' !rename by JG
toColor(10)%ColorName = 'wheat'
toColor(11)%ObjType = 'AirLoopHVAC:ReturnPlenum'  !renamed by script
toColor(11)%ColorName = 'lightgreen'
toColor(12)%ObjType = 'AirLoopHVAC:SupplyPlenum'  !renamed by script
toColor(12)%ColorName = 'lightgreen'
toColor(13)%ObjType = 'WATERHEATER:HEATPUMP INLET AIR MIXER' !rename by JG see HeatPumpWaterHeater.bnd
toColor(13)%ColorName = 'lightslategray'
toColor(14)%ObjType = 'WATERHEATER:HEATPUMP-OUTLET AIR SPLITTER' !rename by JG see HeatPumpWaterHeater.bnd
toColor(14)%ColorName = 'lightslategray'
toColor(15)%ObjType = 'HEATPUMP:WATERTOWATER COOLING' !still uses this in BND file
toColor(15)%ColorName = 'powderblue'
toColor(16)%ObjType = 'HeatPump:WaterToWater:EquationFit:Cooling' !rename by JG
toColor(16)%ColorName = 'powderblue'
toColor(17)%ObjType = 'HeatPump:WaterToWater:EquationFit:Heating' !rename by JG
toColor(17)%ColorName = 'indianred'
toColor(18)%ObjType = 'HeatPump:WaterToWater:ParameterEstimation:Heating' !rename by JG
toColor(18)%ColorName = 'indianred'
toColor(20)%ObjType = 'AirLoopHVAC:UnitaryHeatPump:WaterToAir' !rename by JG
toColor(20)%ColorName = 'lightslategray'
toColor(21)%ObjType = 'DOMESTIC HOT WATER' !still uses this in BND file
toColor(21)%ColorName = 'orange'
toColor(22)%ObjType = 'Generator:CombustionTurbine'  !renamed by script
toColor(22)%ColorName = 'orange'
toColor(23)%ObjType = 'Generator:FuelCell:AirSupply'  !renamed by script
toColor(23)%ColorName = 'orange'
toColor(24)%ObjType = 'Generator:FuelCell:ExhaustGasToWaterHeatExchanger' !renamed by script
toColor(24)%ColorName = 'orange'
toColor(25)%ObjType = 'Generator:FuelCell:PowerModule' !renamed by script
toColor(25)%ColorName = 'orange'
toColor(26)%ObjType = 'Generator:FuelCell:WaterSupply' !renamed by script
toColor(26)%ColorName = 'orange'
toColor(27)%ObjType = 'Generator:FuelSupply' !renamed by script
toColor(27)%ColorName = 'orange'
toColor(28)%ObjType = 'Generator:InternalCombustionEngine' !renamed by script
toColor(28)%ColorName = 'orange'
toColor(29)%ObjType = 'Generator:MicroCHP' !renamed by script
toColor(29)%ColorName = 'orange'
toColor(31)%ObjType = 'WaterHeater:Mixed' !renamed by script
toColor(31)%ColorName = 'orange'
toColor(32)%ObjType = 'WaterHeater:Stratified' !renamed by script
toColor(32)%ColorName = 'orange'
toColor(33)%ObjType = 'ZoneHVAC:LowTemperatureRadiant:ConstantFlow' !renamed by JG - name change looked up in RadLoTempCFloHeatCool.bnd
toColor(33)%ColorName = 'orangered'
toColor(34)%ObjType = 'ZoneHVAC:LowTemperatureRadiant:VariableFlow' !renamed by JG - name change looked up in RadLoHydrHeatCoolAuto.bnd
toColor(34)%ColorName = 'orangered'
toColor(35)%ObjType = 'HeatExchanger:WatersideEconomizer' !renamed by JG - name change looked up in WaterSideEconomizer_Integrated.bnd
toColor(35)%ColorName = 'paleturquoise'
toColor(36)%ObjType = 'FREE COOLING HEAT EXCHANGER' ! did not need to be changed according to FreeCoolingChiller.bnd
toColor(36)%ColorName = 'paleturquoise'
toColor(37)%ObjType = 'GroundHeatExchanger:Vertical'  !renamed by script
toColor(37)%ColorName = 'paleturquoise'
toColor(38)%ObjType = 'HeatExchanger:AirToAir:FlatPlate' !renamed by script
toColor(38)%ColorName = 'paleturquoise'
toColor(39)%ObjType = 'HeatExchanger:AirToAir:SensibleAndLatent' !renamed by script
toColor(39)%ColorName = 'paleturquoise'
toColor(40)%ObjType = 'HeatExchanger:Desiccant:BalancedFlow' !renamed by script
toColor(40)%ColorName = 'paleturquoise'
toColor(41)%ObjType = 'HeatExchanger:WatersideEconomizer' !renamed by script
toColor(41)%ColorName = 'paleturquoise'
toColor(42)%ObjType = 'HeatExchanger:Plate'  !renamed by JG - name change looked up in PlateHeatExchanger.bnd
toColor(42)%ColorName = 'paleturquoise'
toColor(43)%ObjType = 'GroundHeatExchanger:Pond' !renamed by JG to closest object name
toColor(43)%ColorName = 'paleturquoise'
toColor(44)%ObjType = 'GroundHeatExchanger:Surface' !renamed by JG to closest object name
toColor(44)%ColorName = 'paleturquoise'
toColor(45)%ObjType = 'CoolingTower:SingleSpeed' !renamed by script
toColor(45)%ColorName = 'pink'
toColor(46)%ObjType = 'CoolingTower:TwoSpeed' !renamed by script
toColor(46)%ColorName = 'pink'
toColor(47)%ObjType = 'Chiller:Absorption' !renamed by script
toColor(47)%ColorName = 'powderblue'
toColor(48)%ObjType = 'Chiller:CombustionTurbine' !renamed by script
toColor(48)%ColorName = 'powderblue'
toColor(49)%ObjType = 'Chiller:ConstantCOP' !renamed by JG to closest object name
toColor(49)%ColorName = 'powderblue'
toColor(50)%ObjType = 'ChillerHeater:Absorption:DirectFired'  !renamed by script
toColor(50)%ColorName = 'powderblue'
toColor(51)%ObjType = 'Chiller:Electric' !renamed by script
toColor(51)%ColorName = 'powderblue'
toColor(52)%ObjType = 'Chiller:Electric:EIR' !renamed by script
toColor(52)%ColorName = 'powderblue'
toColor(53)%ObjType = 'Chiller:Electric:ReformulatedEIR' !renamed by script
toColor(53)%ColorName = 'powderblue'
toColor(54)%ObjType = 'Chiller:EngineDriven' !renamed by script
toColor(54)%ColorName = 'powderblue'
toColor(55)%ObjType = 'ZoneHVAC:Baseboard:Convective:Water' !renamed by script
toColor(55)%ColorName = 'salmon'
toColor(56)%ObjType = 'Coil:Heating:Desuperheater' !renamed by script
toColor(56)%ColorName = 'salmon'
toColor(57)%ObjType = 'Coil:Heating:DX:SingleSpeed' !renamed by script
toColor(57)%ColorName = 'salmon'
toColor(59)%ObjType = 'Coil:Heating:DX:MultiSpeed' !renamed by script
toColor(59)%ColorName = 'salmon'
toColor(60)%ObjType = 'Coil:Heating:Electric' !renamed by script
toColor(60)%ColorName = 'salmon'
toColor(61)%ObjType = 'Coil:Heating:Fuel' !renamed by script
toColor(61)%ColorName = 'salmon'
toColor(62)%ObjType = 'Coil:Heating:Steam' !renamed by script
toColor(62)%ColorName = 'salmon'
toColor(63)%ObjType = 'Coil:Heating:Water' !renamed by script
toColor(63)%ColorName = 'salmon'
toColor(64)%ObjType = 'Coil:Heating:WaterToAirHeatPump:EquationFit' !renamed by JG
toColor(64)%ColorName = 'salmon'
toColor(65)%ObjType = 'Coil:Heating:WaterToAirHeatPump:ParameterEstimation' !renamed by JG
toColor(65)%ColorName = 'salmon'
toColor(67)%ObjType = 'ZONEHVAC:UNITVENTILATOR-OA MIXER' !renamed by JG - name change looked up in UnitVent5ZoneAuto.bnd
toColor(67)%ColorName = 'sandybrown'
toColor(68)%ObjType = 'ZoneHVAC:UnitVentilator' !renamed by script
toColor(68)%ColorName = 'sandybrown'
toColor(69)%ObjType = 'Fan:ConstantVolume' !renamed by script
toColor(69)%ColorName = 'silver'
toColor(70)%ObjType = 'Fan:OnOff' !renamed by script
toColor(70)%ColorName = 'silver'
toColor(71)%ObjType = 'Fan:VariableVolume' !renamed by JG
toColor(71)%ColorName = 'silver'
toColor(72)%ObjType = 'Fan:ZoneExhaust' !renamed by script
toColor(72)%ColorName = 'silver'
toColor(73)%ObjType = 'Coil:Cooling:DX:SingleSpeed' !renamed by JG
toColor(73)%ColorName = 'skyblue'
toColor(74)%ObjType = 'Coil:Cooling:DX:TwoStageWithHumidityControlMode' !renamed by script
toColor(74)%ColorName = 'skyblue'
toColor(75)%ObjType = 'Coil:Cooling:DX:MultiSpeed'  !renamed by JG
toColor(75)%ColorName = 'skyblue'
toColor(76)%ObjType = 'Coil:Cooling:DX:TwoSpeed' !renamed by script
toColor(76)%ColorName = 'skyblue'
toColor(77)%ObjType = 'Coil:Cooling:Water'  !renamed by JG
toColor(77)%ColorName = 'skyblue'
toColor(78)%ObjType = 'Coil:Cooling:Water:DetailedGeometry' !renamed by JG
toColor(78)%ColorName = 'skyblue'
toColor(80)%ObjType = 'Coil:Cooling:WaterToAirHeatPump:EquationFit' !renamed by JG
toColor(80)%ColorName = 'skyblue'
toColor(81)%ObjType = 'Coil:Cooling:WaterToAirHeatPump:ParameterEstimation' !renamed by JG
toColor(81)%ColorName = 'skyblue'
toColor(83)%ObjType = 'Refrigeration:CompressorRack' !renamed by JG
toColor(83)%ColorName = 'skyblue'
toColor(84)%ObjType = 'ThermalStorage:Ice:Detailed' !renamed by script
toColor(84)%ColorName = 'skyblue'
toColor(85)%ObjType = 'ThermalStorage:Ice:Simple' !renamed by script
toColor(85)%ColorName = 'skyblue'
toColor(86)%ObjType = 'PlantEquipmentOperation:ComponentSetpoint' !renamed by script
toColor(86)%ColorName = 'snow'
toColor(87)%ObjType = 'Controller:OutdoorAir' !renamed by script
toColor(87)%ColorName = 'snow'
toColor(88)%ObjType = 'Controller:WaterCoil' !renamed by script
toColor(88)%ColorName = 'snow'
toColor(89)%ObjType = 'ZoneHVAC:EnergyRecoveryVentilator:Controller' !renamed by script
toColor(89)%ColorName = 'snow'
toColor(90)%ObjType = 'AirTerminal:SingleDuct:ConstantVolume:NoReheat' !renamed by script
toColor(90)%ColorName = 'snow'
toColor(91)%ObjType = 'PlantEquipmentOperation:OutdoorDewpointDifference' !renamed by script
toColor(91)%ColorName = 'snow'
toColor(92)%ObjType = 'PlantEquipmentOperation:OutdoorDryBulbDifference' !renamed by script
toColor(92)%ColorName = 'snow'
toColor(93)%ObjType = 'PlantEquipmentOperation:OutdoorWetBulbDifference' !renamed by script
toColor(93)%ColorName = 'snow'
toColor(94)%ObjType = 'OutdoorAir:NodeList' !renamed by script
toColor(94)%ColorName = 'snow'
toColor(95)%ObjType = 'OutdoorAir:Node' !renamed by JG
toColor(95)%ColorName = 'snow'
toColor(96)%ObjType = 'LoadProfile:Plant' !renamed by script
toColor(96)%ColorName = 'snow'
toColor(97)%ObjType = 'ZoneHVAC:IdealLoadsAirSystem' !renamed by script
toColor(97)%ColorName = 'snow'
toColor(98)%ObjType = 'DistrictCooling' !renamed by script
toColor(98)%ColorName = 'powderblue'
toColor(99)%ObjType = 'DistrictHeating' !renamed by script
toColor(99)%ColorName = 'indianred'
toColor(100)%ObjType = 'SetpointManager:Coldest' !renamed by script
toColor(100)%ColorName = 'snow'
toColor(101)%ObjType = 'SetpointManager:MixedAir' !renamed by script
toColor(101)%ColorName = 'snow'
toColor(102)%ObjType = 'SetpointManager:OutdoorAirReset' !renamed by script
toColor(102)%ColorName = 'snow'
toColor(103)%ObjType = 'SetpointManager:OutdoorAirPretreat' !renamed by script
toColor(103)%ColorName = 'snow'
toColor(104)%ObjType = 'SetpointManager:Scheduled' !renamed by script
toColor(104)%ColorName = 'snow'
toColor(105)%ObjType = 'SetpointManager:Scheduled:DualSetpoint' !renamed by script
toColor(105)%ColorName = 'snow'
toColor(106)%ObjType = 'SetpointManager:SingleZone:Cooling' !renamed by script
toColor(106)%ColorName = 'snow'
toColor(107)%ObjType = 'SetpointManager:SingleZone:Heating' !renamed by script
toColor(107)%ColorName = 'snow'
toColor(108)%ObjType = 'SetpointManager:SingleZone:Humidity:Maximum' !renamed by script
toColor(108)%ColorName = 'snow'
toColor(109)%ObjType = 'SetpointManager:SingleZone:Humidity:Minimum' !renamed by script
toColor(109)%ColorName = 'snow'
toColor(110)%ObjType = 'SetpointManager:SingleZone:Reheat' !renamed by script
toColor(110)%ColorName = 'snow'
toColor(111)%ObjType = 'SetpointManager:Warmest' !renamed by script
toColor(111)%ColorName = 'snow'
toColor(112)%ObjType = 'SetpointManager:WarmestTemperatureFlow' !renamed by script
toColor(112)%ColorName = 'snow'
toColor(113)%ObjType = 'AvailabilityManager:DifferentialThermostat' !renamed by script
toColor(113)%ColorName = 'snow'
toColor(114)%ObjType = 'AvailabilityManager:HighTemperatureTurnOff' !renamed by script
toColor(114)%ColorName = 'snow'
toColor(115)%ObjType = 'AvailabilityManager:HighTemperatureTurnOn' !renamed by script
toColor(115)%ColorName = 'snow'
toColor(116)%ObjType = 'AvailabilityManager:LowTemperatureTurnOff' !renamed by script
toColor(116)%ColorName = 'snow'
toColor(117)%ObjType = 'AvailabilityManager:LowTemperatureTurnOn' !renamed by script
toColor(117)%ColorName = 'snow'
toColor(118)%ObjType = 'HeaderedPumps:ConstantSpeed' !renamed by script
toColor(118)%ColorName = 'springgreen'
toColor(119)%ObjType = 'HeaderedPumps:VariableSpeed' !renamed by script
toColor(119)%ColorName = 'springgreen'
toColor(120)%ObjType = 'Pump:VariableSpeed:Condensate' !renamed by script
toColor(120)%ColorName = 'springgreen'
toColor(121)%ObjType = 'Pump:ConstantSpeed' !renamed by script
toColor(121)%ColorName = 'springgreen'
toColor(122)%ObjType = 'Pump:VariableSpeed' !renamed by script
toColor(122)%ColorName = 'springgreen'
toColor(123)%ObjType = 'Dehumidifier:Desiccant:System' !renamed by script
toColor(123)%ColorName = 'tan'
toColor(124)%ObjType = 'Dehumidifier:Desiccant:NoFans' !renamed by script
toColor(124)%ColorName = 'tan'
toColor(125)%ObjType = 'PlantLoopConnection' !renamed by script
toColor(125)%ColorName = 'wheat'
toColor(126)%ObjType = 'PlantLoopConnection:Controlled' !renamed by script
toColor(126)%ColorName = 'wheat'
toColor(127)%ObjType = 'AirTerminal:DualDuct:ConstantVolume' !renamed by script
toColor(127)%ColorName = 'wheat'
toColor(128)%ObjType = 'AirTerminal:DualDuct:VAV' !renamed by script
toColor(128)%ColorName = 'wheat'
toColor(129)%ObjType = 'Duct' !renamed by script
toColor(129)%ColorName = 'wheat'
toColor(130)%ObjType = 'Pipe:Adiabatic' !renamed by script
toColor(130)%ColorName = 'wheat'
toColor(131)%ObjType = 'Pipe:Indoor' !renamed by JG
toColor(131)%ColorName = 'wheat'
toColor(132)%ObjType = 'Pipe:Outdoor' !renamed by script
toColor(132)%ColorName = 'wheat'
toColor(133)%ObjType = 'AirTerminal:SingleDuct:VAV:NoReheat' !renamed by script
toColor(133)%ColorName = 'wheat'
toColor(134)%ObjType = 'AirTerminal:SingleDuct:VAV:Reheat' !renamed by script
toColor(134)%ColorName = 'wheat'
toColor(135)%ObjType = 'AirTerminal:SingleDuct:VAV:HeatAndCool:NoReheat' !renamed by script
toColor(135)%ColorName = 'wheat'
toColor(136)%ObjType = 'AirTerminal:SingleDuct:VAV:HeatAndCool:Reheat' !renamed by script
toColor(136)%ColorName = 'wheat'
toColor(137)%ObjType = 'Connector:Splitter' !renamed by JG
toColor(137)%ColorName = 'wheat'
toColor(138)%ObjType = 'TemperingValve' !renamed by script
toColor(138)%ColorName = 'wheat'
toColor(139)%ObjType = 'WaterUse:Connections' !renamed by script
toColor(139)%ColorName = 'wheat'
toColor(140)%ObjType = 'AirLoopHVAC:ZoneMixer' !renamed by script
toColor(140)%ColorName = 'wheat'
toColor(141)%ObjType = 'AirLoopHVAC:ZoneSplitter' !renamed by script
toColor(141)%ColorName = 'wheat'
toColor(142)%ObjType = 'SolarCollector:FlatPlate:Water' !renamed by script
toColor(142)%ColorName = 'yellow'
toColor(143)%ObjType = 'SolarCollector:UnglazedTranspired' !renamed by script
toColor(143)%ColorName = 'yellow'

!items added by others recommendations
toColor(144)%ObjType = 'ZoneHVAC:Baseboard:Convective:Electric' !renamed by script
toColor(144)%ColorName = 'orangered'
toColor(145)%ObjType = 'ZoneHVAC:LowTemperatureRadiant:VariableFlow' !renamed by script
toColor(145)%ColorName = 'orangered'
toColor(146)%ObjType = 'ZoneHVAC:LowTemperatureRadiant:ConstantFlow' !renamed by script
toColor(146)%ColorName = 'orangered'
toColor(147)%ObjType = 'ZoneHVAC:LowTemperatureRadiant:Electric' !renamed by script
toColor(147)%ColorName = 'orangered'
toColor(148)%ObjType = 'Chiller:ConstantCOP' !renamed by script
toColor(148)%ColorName = 'powderblue'
toColor(149)%ObjType = 'Chiller:Electric:SPARK' !renamed by script
toColor(149)%ColorName = 'powderblue'
toColor(150)%ObjType = 'AirTerminal:SingleDuct:ConstantVolume:Reheat' !renamed by script
toColor(150)%ColorName = 'snow'
toColor(151)%ObjType = 'AirTerminal:SingleDuct:VAV:Reheat:VariableSpeedFan' !renamed by script
toColor(151)%ColorName = 'snow'
toColor(152)%ObjType = 'AirTerminal:SingleDuct:SeriesPIU:Reheat' !renamed by JG
toColor(152)%ColorName = 'snow'
toColor(153)%ObjType = 'AirTerminal:SingleDuct:ParallelPIU:Reheat' !renamed by JG
toColor(153)%ColorName = 'snow'
toColor(154)%ObjType = 'AirTerminal:SingleDuct:ConstantVolume:FourPipeInduction' !renamed by script
toColor(154)%ColorName = 'snow'
toColor(155)%ObjType = 'CoolingTower:VariableSpeed' !renamed by script
toColor(155)%ColorName = 'pink'
!fixed to proper name
toColor(156)%ObjType = 'GroundHeatExchanger:Pond' !renamed by script
toColor(156)%ColorName = 'paleturquoise'
toColor(157)%ObjType = 'GroundHeatExchanger:Surface' !renamed by script
toColor(157)%ColorName = 'paleturquoise'
!
toColor(158)%ObjType = 'WATERHEATER:HEATPUMP-INLET AIR MIXER' !rename by JG see HeatPumpWaterHeater.bnd
toColor(158)%ColorName = 'lightslategray'

! Odd cases from the object renaming effort
toColor(19)%ObjType = 'HEATPUMP:WATERTOWATER HEATING' !not used by old BND
toColor(19)%ColorName = 'indianred'
toColor(30)%ObjType = 'WATER HEATER:SIMPLE' !not used by old BND
toColor(30)%ColorName = 'orange'
toColor(58)%ObjType = 'Coil:WaterHeating:WaterToWaterHeatPump' !not used by old BND
toColor(58)%ColorName = 'salmon'
toColor(66)%ObjType = 'COIL:WATERTOAIRHP:HEATING' !not used by old BND
toColor(66)%ColorName = 'salmon'
toColor(79)%ObjType = 'COIL:WATER:SIMPLECOOLING' !not used by old BND
toColor(79)%ColorName = 'skyblue'
toColor(82)%ObjType = 'COIL:WATERTOAIRHP:COOLING' !not used by old BND
toColor(82)%ColorName = 'skyblue'


DO iBox = 1, lastBox
  box(iBox)%colorToUse = 1 !default
  DO jColor = 1, maxToColor
    IF (SameString(box(iBox)%TypeObj,toColor(jColor)%ObjType)) THEN
      box(iBox)%colorToUse = jColor
    END IF
  END DO
END DO
END SUBROUTINE colorizeBoxes



! =========================================================================
! =========================================================================
!   ROUTINES TO MANAGE MAIN ARRAYS
! =========================================================================
! =========================================================================

! =========================================================================
SUBROUTINE initializeBox(aBoxArray)
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   Clear the box array
          !   Passing the specific box array since it is also used
          !   in the expandBox routine

          ! METHODOLOGY EMPLOYED:
          ! <description>

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
TYPE (BoxType), DIMENSION(:),INTENT(INOUT)     :: aBoxArray

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:

aBoxArray%ObjName = ''
aBoxArray%TypeObj = ''
aBoxArray%colorToUse = 1
aBoxArray%FluidStream = 0
aBoxArray%splitMix = 0
aBoxArray%countRelatives = 0
aBoxArray%countRows = 1
aBoxArray%SubDiagramNum = 0
aBoxArray%row = 0
aBoxArray%col = 0
aBoxArray%isAssigned = .FALSE.
aBoxArray%xUpperLeft = 0
aBoxArray%yUpperLeft = 0
aBoxArray%xLowerRight = 1
aBoxArray%yLowerRight = 1
aBoxArray%isOrigin = .FALSE.
END SUBROUTINE initializeBox

! =========================================================================
SUBROUTINE expandBox(possSizeBox)
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   If necessary expand the Box array

          ! METHODOLOGY EMPLOYED:
          ! <description>

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
INTEGER,INTENT(IN)     :: possSizeBox

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
INTEGER      :: increaseBox


IF (possSizeBox .GE. maxSizeBox) THEN
  increaseBox = sizeIncrementBox
  IF (possSizeBox .GE. maxSizeBox + increaseBox) THEN  !if it is still not big enough then add the new size
    increaseBox = sizeIncrementBox + (possSizeBox - maxSizeBox)
  END IF
  ALLOCATE(boxCopy(maxSizeBox + increaseBox))
  CALL initializeBox(boxCopy)
  boxCopy(1:maxSizeBox) = box
  DEALLOCATE(box)
  ALLOCATE(box(maxSizeBox + increaseBox))
  box = boxCopy
  DEALLOCATE(boxCopy)
  maxSizeBox = maxSizeBox + increaseBox
END IF
END SUBROUTINE expandBox

! =========================================================================
SUBROUTINE initializeConnect(aConnectArray)
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   Clear the connect array
          !   Passing the specific connect array since it is also used
          !   in the expandConnect routine

          ! METHODOLOGY EMPLOYED:
          ! <description>

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
TYPE (ConnectType), DIMENSION(:),INTENT(INOUT)     :: aConnectArray

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:

aConnectArray%NodeName = ''
aConnectArray%isInterLink = .FALSE.
aConnectArray%drawLine = .TRUE.
aConnectArray%BoxFrom = 0
aConnectArray%BoxTo = 0
aConnectArray%FollowedBoxTo = .FALSE.
aConnectArray%StartX = 0
aConnectArray%StartY = 0
aConnectArray%EndX = 2
aConnectArray%EndY = 2
aConnectArray%NodeX = 1
aConnectArray%NodeY = 1
END SUBROUTINE initializeConnect

! =========================================================================
SUBROUTINE expandConnect(possSizeConnect)
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   If necessary expand the Connect array

          ! METHODOLOGY EMPLOYED:
          ! <description>

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
INTEGER,INTENT(IN)     :: possSizeConnect

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
INTEGER      :: increaseConnect


IF (possSizeConnect .GE. maxSizeConnect) THEN
  increaseConnect = sizeIncrementConnect   !increase by the normal increment
  IF (possSizeConnect .GE. maxSizeConnect + increaseConnect) THEN  !if it is still not big enough then add the new size
    increaseConnect = sizeIncrementConnect + (possSizeConnect - maxSizeConnect)
  END IF
  ALLOCATE(connectCopy(maxSizeConnect + increaseConnect))
  CALL initializeConnect(connectCopy)
  connectCopy(1:maxSizeConnect) = connect
  DEALLOCATE(connect)
  ALLOCATE(connect(maxSizeConnect + increaseConnect))
  connect = connectCopy
  DEALLOCATE(connectCopy)
  maxSizeConnect = maxSizeConnect + increaseConnect
END IF
END SUBROUTINE expandConnect

! =========================================================================
SUBROUTINE initializeSubDiagram(aSubDiagramArray)
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   Clear the connect array
          !   Passing the specific SubDiagram array since it is also used
          !   in the expandSubDiagram routine

          ! METHODOLOGY EMPLOYED:
          ! <description>

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
TYPE (SubDiagramType), DIMENSION(:),INTENT(INOUT)     :: aSubDiagramArray

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:

aSubDiagramArray%RootBox = 0
aSubDiagramArray%FinalBox = 0
aSubDiagramArray%OffsetX = 0
aSubDiagramArray%OffsetY = 0
aSubDiagramArray%maxY = 0
aSubDiagramArray%gapWidth = 0
aSubDiagramArray%drawBridge = .FALSE.
aSubDiagramArray%bridge1X = 0
aSubDiagramArray%bridge1Y = 0
aSubDiagramArray%bridge2X = 0
aSubDiagramArray%bridge2Y = 0
aSubDiagramArray%bridge3X = 0
aSubDiagramArray%bridge3Y = 0
aSubDiagramArray%bridge4X = 0
aSubDiagramArray%bridge4Y = 0
aSubDiagramArray%bridge5X = 0
aSubDiagramArray%bridge5Y = 0
aSubDiagramArray%bridge6X = 0
aSubDiagramArray%bridge6Y = 0
END SUBROUTINE initializeSubDiagram

! =========================================================================
SUBROUTINE expandSubDiagram(possSizeSubDiagram)
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   If necessary expand the SubDiagram array

          ! METHODOLOGY EMPLOYED:
          ! <description>

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
INTEGER,INTENT(IN)     :: possSizeSubDiagram

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:
INTEGER      :: increaseSubDiagram


IF (possSizeSubDiagram .GE. maxSizeSubDiagram) THEN
  increaseSubDiagram = sizeIncrementSubDiagram   !increase by the normal increment
  IF (possSizeSubDiagram .GE. maxSizeSubDiagram + increaseSubDiagram) THEN  !if it is still not big enough then add the new size
    increaseSubDiagram = sizeIncrementSubDiagram + (possSizeSubDiagram - maxSizeSubDiagram)
  END IF
  ALLOCATE(subDiagramCopy(maxSizeSubDiagram + increaseSubDiagram))
  CALL initializeSubDiagram(subDiagramCopy)
  subDiagramCopy(1:maxSizeSubDiagram) = subDiagram
  DEALLOCATE(subDiagram)
  ALLOCATE(subDiagram(maxSizeSubDiagram + increaseSubDiagram))
  subDiagram = subDiagramCopy
  DEALLOCATE(subDiagramCopy)
  maxSizeSubDiagram = maxSizeSubDiagram + increaseSubDiagram
END IF
END SUBROUTINE expandSubDiagram

! =========================================================================
! =========================================================================
!   ROUTINES TO MANAGE BND ARRAYS
! =========================================================================
! =========================================================================

! =========================================================================
SUBROUTINE incrementAirLoopConnect
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   February 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   Increment the counter for this array and if necessary expand
          !   the array.  If the array has not been initialized then
          !   it also initializes the values in the array.

          ! METHODOLOGY EMPLOYED:
          !   Uses a second array to move values while the array is
          !   deallocated and allocated again. Uses the allocated functin
          !   to see if the array has been initialized.

          ! REFERENCES:
          !   na

          ! USE STATEMENTS:
          !   na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:

IF (ALLOCATED(bndAirLoopConnect)) THEN
  lastBndAirLoopConnect = lastBndAirLoopConnect + 1
  IF (lastBndAirLoopConnect .GT. maxSizeBndAirLoopConnect) THEN
    ALLOCATE(bndAirLoopConnectCopy(maxSizeBndAirLoopConnect + sizeIncrementBndAirLoopConnect))
    bndAirLoopConnectCopy(1:maxSizeBndAirLoopConnect) = bndAirLoopConnect
    DEALLOCATE(bndAirLoopConnect)
    ALLOCATE(bndAirLoopConnect(maxSizeBndAirLoopConnect + sizeIncrementBndAirLoopConnect))
    bndAirLoopConnect = bndAirLoopConnectCopy
    DEALLOCATE(bndAirLoopConnectCopy)
    maxSizeBndAirLoopConnect = maxSizeBndAirLoopConnect + sizeIncrementBndAirLoopConnect
  END IF
ELSE
  ALLOCATE(bndAirLoopConnect(maxSizeBndAirLoopConnect))
  lastBndAirLoopConnect = 1
END IF
END SUBROUTINE incrementAirLoopConnect

! =========================================================================
SUBROUTINE incrementNodeConnection
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   February 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   Increment the counter for this array and if necessary expand
          !   the array.  If the array has not been initialized then
          !   it also initializes the values in the array.

          ! METHODOLOGY EMPLOYED:
          !   Uses a second array to move values while the array is
          !   deallocated and allocated again. Uses the allocated functin
          !   to see if the array has been initialized.

          ! REFERENCES:
          !   na

          ! USE STATEMENTS:
          !   na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:

IF (ALLOCATED(bndNodeConnect)) THEN
  lastBndNodeConnect = lastBndNodeConnect + 1
  IF (lastBndNodeConnect .GT. maxSizeBndNodeConnect) THEN
    ALLOCATE(bndNodeConnectCopy(maxSizeBndNodeConnect + sizeIncrementBndNodeConnect))
    bndNodeConnectCopy(1:maxSizeBndNodeConnect) = bndNodeConnect
    DEALLOCATE(bndNodeConnect)
    ALLOCATE(bndNodeConnect(maxSizeBndNodeConnect + sizeIncrementBndNodeConnect))
    bndNodeConnect = bndNodeConnectCopy
    DEALLOCATE(bndNodeConnectCopy)
    maxSizeBndNodeConnect = maxSizeBndNodeConnect + sizeIncrementBndNodeConnect
  END IF
ELSE
  ALLOCATE(bndNodeConnect(maxSizeBndNodeConnect))
  lastBndNodeConnect = 1
END IF
END SUBROUTINE incrementNodeConnection

! =========================================================================
SUBROUTINE incrementLoopConnection
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   February 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   Increment the counter for this array and if necessary expand
          !   the array.  If the array has not been initialized then
          !   it also initializes the values in the array.

          ! METHODOLOGY EMPLOYED:
          !   Uses a second array to move values while the array is
          !   deallocated and allocated again. Uses the allocated functin
          !   to see if the array has been initialized.

          ! REFERENCES:
          !   na

          ! USE STATEMENTS:
          !   na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:

IF (ALLOCATED(bndLoopConnection)) THEN
  lastBndLoopConnection = lastBndLoopConnection + 1
  IF (lastBndLoopConnection .GT. maxSizeBndLoopConnection) THEN
    ALLOCATE(bndLoopConnectionCopy(maxSizeBndLoopConnection + sizeIncrementBndLoopConnection))
    bndLoopConnectionCopy(1:maxSizeBndLoopConnection) = bndLoopConnection
    DEALLOCATE(bndLoopConnection)
    ALLOCATE(bndLoopConnection(maxSizeBndLoopConnection + sizeIncrementBndLoopConnection))
    bndLoopConnection = bndLoopConnectionCopy
    DEALLOCATE(bndLoopConnectionCopy)
    maxSizeBndLoopConnection = maxSizeBndLoopConnection + sizeIncrementBndLoopConnection
  END IF
ELSE
  ALLOCATE(bndLoopConnection(maxSizeBndLoopConnection))
  lastBndLoopConnection = 1
END IF
END SUBROUTINE incrementLoopConnection

! =========================================================================
SUBROUTINE incrementContZoneInlet
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   February 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   Increment the counter for this array and if necessary expand
          !   the array.  If the array has not been initialized then
          !   it also initializes the values in the array.

          ! METHODOLOGY EMPLOYED:
          !   Uses a second array to move values while the array is
          !   deallocated and allocated again. Uses the allocated functin
          !   to see if the array has been initialized.

          ! REFERENCES:
          !   na

          ! USE STATEMENTS:
          !   na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:

IF (ALLOCATED(bndContZoneInlet)) THEN
  lastBndContZoneInlet = lastBndContZoneInlet + 1
  IF (lastBndContZoneInlet .GT. maxSizeBndContZoneInlet) THEN
    ALLOCATE(bndContZoneInletCopy(maxSizeBndContZoneInlet + sizeIncrementBndContZoneInlet))
    bndContZoneInletCopy(1:maxSizeBndContZoneInlet) = bndContZoneInlet
    DEALLOCATE(bndContZoneInlet)
    ALLOCATE(bndContZoneInlet(maxSizeBndContZoneInlet + sizeIncrementBndContZoneInlet))
    bndContZoneInlet = bndContZoneInletCopy
    DEALLOCATE(bndContZoneInletCopy)
    maxSizeBndContZoneInlet = maxSizeBndContZoneInlet + sizeIncrementBndContZoneInlet
  END IF
ELSE
  ALLOCATE(bndContZoneInlet(maxSizeBndContZoneInlet))
  lastBndContZoneInlet = 1
END IF
END SUBROUTINE incrementContZoneInlet

! =========================================================================
SUBROUTINE incrementContZoneExhaust
          ! SUBROUTINE INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   March 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          !   Increment the counter for this array and if necessary expand
          !   the array.  If the array has not been initialized then
          !   it also initializes the values in the array.

          ! METHODOLOGY EMPLOYED:
          !   Uses a second array to move values while the array is
          !   deallocated and allocated again. Uses the allocated functin
          !   to see if the array has been initialized.

          ! REFERENCES:
          !   na

          ! USE STATEMENTS:
          !   na

IMPLICIT NONE ! Enforce explicit typing of all variables in this routine

          ! SUBROUTINE ARGUMENT DEFINITIONS:
          ! na

          ! SUBROUTINE PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS:
          ! na

          ! DERIVED TYPE DEFINITIONS:
          ! na

          ! SUBROUTINE LOCAL VARIABLE DECLARATIONS:

IF (ALLOCATED(bndContZoneExhaust)) THEN
  lastBndContZoneExhaust = lastBndContZoneExhaust + 1
  IF (lastBndContZoneExhaust .GT. maxSizeBndContZoneExhaust) THEN
    ALLOCATE(bndContZoneExhaustCopy(maxSizeBndContZoneExhaust + sizeIncrementBndContZoneExhaust))
    bndContZoneExhaustCopy(1:maxSizeBndContZoneExhaust) = bndContZoneExhaust
    DEALLOCATE(bndContZoneExhaust)
    ALLOCATE(bndContZoneExhaust(maxSizeBndContZoneExhaust + sizeIncrementBndContZoneExhaust))
    bndContZoneExhaust = bndContZoneExhaustCopy
    DEALLOCATE(bndContZoneExhaustCopy)
    maxSizeBndContZoneExhaust = maxSizeBndContZoneExhaust + sizeIncrementBndContZoneExhaust
  END IF
ELSE
  ALLOCATE(bndContZoneExhaust(maxSizeBndContZoneExhaust))
  lastBndContZoneExhaust = 1
END IF
END SUBROUTINE incrementContZoneExhaust


! =========================================================================
! =========================================================================
!   STRING MANIPULATION ROUTINES
! =========================================================================
! =========================================================================

! =========================================================================
FUNCTION MakeUPPERCase(InputString) RESULT (ResultString)

          ! FUNCTION INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   September 1997
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This function returns the Upper Case representation of the InputString.

          ! METHODOLOGY EMPLOYED:
          ! Uses the Intrinsic SCAN function to scan the lowercase representation of
          ! characters (DataStringGlobals) for each character in the given string.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine


          ! FUNCTION ARGUMENT DEFINITIONS:
CHARACTER(len=*), INTENT(IN)      :: InputString    ! Input String
CHARACTER(len=LEN(InputString))  :: ResultString   ! Result String, string is limited to


          ! FUNCTION PARAMETER DEFINITIONS:
CHARACTER(len=55), PARAMETER :: LowerCase='abcdefghijklmnopqrstuvwxyz‡·‚„‰ÂÊÁËÈÍÎÏÌÓÔÒÚÛÙıˆ¯˘˙˚¸˝'
CHARACTER(len=55), PARAMETER :: UpperCase='ABCDEFGHIJKLMNOPQRSTUVWXYZ¿¡¬√ƒ≈∆«»… ÀÃÕŒœ–—“”‘’÷ÿŸ⁄€‹›'


          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! FUNCTION LOCAL VARIABLE DECLARATIONS:
INTEGER Count              ! Loop Counter
INTEGER Pos                ! Position in String representation
INTEGER LengthInputString  ! Length (trimmed) of InputString

ResultString=' '
Pos=SCAN(InputString,LowerCase)
IF (POS /= 0) THEN
  LengthInputString=LEN_TRIM(InputString)
  DO Count=1,LengthInputString
    Pos=SCAN(LowerCase,InputString(Count:Count))
    IF (Pos /= 0) THEN
      ResultString(Count:Count)=UpperCase(Pos:Pos)
    ELSE
      ResultString(Count:Count)=InputString(Count:Count)
    ENDIF
  END DO
  ResultString=TRIM(ResultString)
ELSE
  ! String already in Upper Case
  ResultString=TRIM(InputString)
ENDIF
END FUNCTION MakeUPPERCase

! =========================================================================
LOGICAL FUNCTION SameString(TestString1,TestString2)

          ! FUNCTION INFORMATION:
          !       AUTHOR         Linda K. Lawrie
          !       DATE WRITTEN   November 1997
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! This function returns true if the two strings are equal (case insensitively)

          ! METHODOLOGY EMPLOYED:
          ! Make both strings uppercase.  Do internal compare.

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

  IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine


          ! FUNCTION ARGUMENT DEFINITIONS:
   CHARACTER(len=*), INTENT(IN) :: TestString1  ! First String to Test
   CHARACTER(len=*), INTENT(IN) :: TestString2  ! Second String to Test


          ! FUNCTION PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! FUNCTION LOCAL VARIABLE DECLARATIONS:
          ! na
INTEGER, PARAMETER :: MaxInputLineLength =  500

IF (LEN_TRIM(TestString1) /= LEN_TRIM(TestString2)) THEN
  SameString=.false.
ELSEIF (LEN(TestString1) <= MaxInputLineLength .and. LEN(TestString2) <= MaxInputLineLength) THEN
  ! This test (MaxInputLineLength) is necessary because of PowerStation Compiler
  SameString=MakeUPPERCase(TestString1) == MakeUPPERCase(TestString2)
ELSE
  PRINT "(A)"," ERROR: SameString aborting -- input strings too long"
  IF (dumpDetails) THEN
    WRITE(UNIT=30,FMT="(A)") " ERROR: SameString aborting -- input strings too long"
  END IF
  SameString=.false.
ENDIF
END FUNCTION SameString

! =========================================================================
LOGICAL FUNCTION ContainsString(LookForString,LookInString)

          ! FUNCTION INFORMATION:
          !       AUTHOR         Jason Glazer
          !       DATE WRITTEN   January 2004
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! Check if one string contains another (case insensitively)

          ! METHODOLOGY EMPLOYED:
          ! Make both strings uppercase.  Use Index

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! FUNCTION ARGUMENT DEFINITIONS:
CHARACTER(len=*), INTENT(IN) :: LookForString  ! First String to Test
CHARACTER(len=*), INTENT(IN) :: LookInString  ! Second String to Test

          ! FUNCTION PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! FUNCTION LOCAL VARIABLE DECLARATIONS:
INTEGER  :: locOfSubstring

locOfSubstring=INDEX(MakeUPPERCase(LookInString),MakeUPPERCase(LookForString))
IF (locOfSubstring .GT. 0) THEN
  ContainsString = .TRUE.
ELSE
  ContainsString = .FALSE.
END IF
END FUNCTION ContainsString


! =========================================================================
INTEGER FUNCTION ConvertTextToInteger(String) RESULT (res)

          ! FUNCTION INFORMATION:
          !       AUTHOR         Probably Linda Lawrie
          !       DATE WRITTEN   Unknown - found in DOE2Translator
          !       MODIFIED       na
          !       RE-ENGINEERED  na

          ! PURPOSE OF THIS SUBROUTINE:
          ! Takes a text string and converts it into an integer

          ! METHODOLOGY EMPLOYED:
          ! Use internal read

          ! REFERENCES:
          ! na

          ! USE STATEMENTS:
          ! na

IMPLICIT NONE    ! Enforce explicit typing of all variables in this routine

          ! FUNCTION ARGUMENT DEFINITIONS:
  CHARACTER(len=*), INTENT(IN) :: String

          ! FUNCTION PARAMETER DEFINITIONS:
          ! na

          ! INTERFACE BLOCK SPECIFICATIONS
          ! na

          ! DERIVED TYPE DEFINITIONS
          ! na

          ! FUNCTION LOCAL VARIABLE DECLARATIONS:
  LOGICAL                      :: ErrorFlag
  CHARACTER(len=*), PARAMETER  :: ValidNumerics='0123456789+-'
  INTEGER                      :: Temp
  INTEGER                      :: IoStatus
  INTEGER                      :: VerNumber
  INTEGER                      :: StringLen
  CHARACTER(len=200)           :: PString
  res=0.0
  !  Make sure the string has all what we think numerics should have
  PString=ADJUSTL(String)
  StringLen=LEN_TRIM(PString)
  ErrorFlag=.false.
  IF (StringLen == 0) RETURN
  VerNumber=VERIFY(PString(1:StringLen),ValidNumerics)
  IF (VerNumber == 0) THEN
    Read(PString,*,IOSTAT=IoStatus) Temp
    res=Temp
    ErrorFlag=.false.
  ELSE
    res=0.0
    ErrorFlag=.true.
  ENDIF
  IF (IoStatus /= 0) THEN
    res=0.0
    ErrorFlag=.true.
  ENDIF
RETURN
END FUNCTION


END PROGRAM Diagram

!     NOTICE
!
!     Copyright © 2004-2008 The Board of Trustees of the University of Illinois
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
