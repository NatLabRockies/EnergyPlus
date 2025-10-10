# OLD NFP - FOR REFERENCE ONLY #

# Adding "Spaces" to EnergyPlus #

**Michael J. Witte, GARD Analytics, Inc.**
**Jason W. DeGraw, ORNL**
**With input from many more . . .**

 -  Original, November 22, 2019
 -  Revised, April 1, 2020
    * Keep the current definition of Zone
    * Add new objects for Space, SpaceType, CompoundSpaceType, and Enclosure

## Justification for New Feature ##

*The set-up (Amir Roth, Oct 11, 2019):*

There are a number of parallel initiatives in EnergyPlus and EnergyPlus 10X that I think need to
be tied together.

On the 10X side, there are several efforts to clean up and speed up various surface-related
calculations including heat-balance and radiant exchange. LBNL, GARD, and ORNL have been working
on these. On the  EnergyPlus side, there is a task to add the OpenStudio Space/SpaceType concept
and load definitions to EnergyPlus  to both enable space-level book-keeping and reporting and to
facilitate round-tripping with OSM. Of course,  epJSON is implicated as well.

Given the connections, I think it would be good to get together and do a more comprehensive
planning exercise,  to see exactly what the scope of the changes might be. Even if we don't
tackle all of the changes right now,  we should at least know what they are so that we don't do
things that run counter to them.

Right now, the scope appears to be:

i) a Space object that could be used for standards-style load definitions

ii) determination of the right granularity at which to do radiant calculations (may overlap with
the Space object)

iii) implications of doing radiant calculations at a smaller granularity than we currently are
(e.g., do we now need to do conduction calculations on intra-zone walls? do we do this already?)

iv) view factor calculations

v) choice of radiant exchange algorithm

vi) surface data structure reorganization, specifically do we continue to use a single large
surface array? How are surfaces ordered within this array?

vii) epJSON issues.

There are probably more that I am not thinking of.


## Overview ##

### Current EnergyPlus model ###
EnergyPlus input currently has the following (very flat) heierarchy:

    Zone
        Surfaces (references a zone name)
        People, Lights, etc. (references a zone name or zone list name)
        Thermostat (one per zone)
        HVAC equipment (attaches to a zone)

Zones represent an air mass (air node) that exchanges heat with surfaces, internal loads, HVAC equipment, etc.
A Zone is essentially the "atomic unit" of the building. By default a zone has a uniform air temperature, but there are room air models
that allow modeling of stratification and other non-uniform temperature effects. Thermostats and HVAC equipment are assigned per Zone.


The internal data model adds another layer:

    Enclosure
        Zone
            Surfaces
            People, Lights, etc.
            Thermostat
            HVAC equipment
        Zone
            Surfaces
            People, Lights, etc.
            Thermostat
            HVAC equipment

Enclosures are used for radiant and solar/daylighting
exchange. Enclosures are primarily concerned with Surfaces, but they
are also related to internal gains (which cast radiant and visible
energy into the enclosure). By default there is one Enclosure for each
Zone.  By using Construction:AirBoundary, multiple zones may be
grouped into a single larger Enclosure:

    Construction:AirBoundary,
      Air Wall,                !- Name
      GroupedZones,            !- Solar and Daylighting Method (Options are GroupedZones or InteriorWindow)
      GroupedZones,            !- Radiant Exchange Method (Options are GroupedZones or IRTSurface)
      SimpleMixing,            !- Air Exchange Method
      0.5,                     !- Simple Mixing Air Changes per Hour {1/hr}
      ;                        !- Simple Mixing Schedule Name

Presently, enclosures are assigned automatically. They can be listed
(Output:Surfaces:List,ViewFactorInfo;) but there are no explicit input
objects to assign zones to enclosures. Because
Construction:AirBoundary has separate options for solar and radiant
exchange across the air boundary, it is possible that the solar and
radiant enclosures have different zone groupings.

     * AR1: Enclosures are assigned pseudo-automatically using the
       air-boundary construction.  They are not assigned by analyzing
       geometry.  Are we keeping the AirBoundary object in the
       redesign?

### Where do Spaces fit in? ###
It is generally agreed that a Space (or Room) is equal to or smaller than a Zone.

Some relevant definitions from ASHRAE Std 90.1:

 * Enclosed Space: a volume substantially surrounded by solid
surfaces, such as walls, floors, roofs, and openable devices, such as
doors and operable windows.

 * Space: an enclosed space within a building. The classifications of
spaces are as follows for the purpose of determining building envelope
requirements: (and further defines conditioned space, cooled space,
heated space, indirectly conditioned space, semiheated space,
unconditioned space).

 * HVAC Zone: a space or group of spaces within a building with
heating and cooling requirements that are sufficiently similar so that
desired conditions (e.g., temperature) can be maintained throughout
using a single sensor (e.g., thermostat or temperature sensor).

 * Thermal Block: a collection of one or more HVAC zones grouped
together for simulation purposes. Spaces need not be contiguous to be
combined within a single thermal block.

The relationship of Zones and Spaces for HVAC systems and control are
less clear. The concept of one active thermostat per zone is generally
accepted. Spaces may have HVAC equipment which shares some control
with other Spaces in the Zone. For example, the thermostat in one
office may control the airflow to a VAV terminal unit that supplies
air proportionately to two or more offices (Spaces).  This implies the
need for Spaces to have a separate air heat balance and separate zone
temperatures - this currently happens at the Zone level.

The relationship of Enclosures to Spaces is less certain. In an
open-plan setting, an Enclosure could include multiples zones and
spaces. In a subdivided setting (e.g., row of private offices) an
Enclosure would be a single private office which could correspond to a
Space or multiple offices may comprise the Space.

There is also a commonly used concept of "Space" as a non-geometric
means to communicate allocations of internal load calculations (are
there other use cases, e.g., HVAC systems?). Internal loads typically
have convective and radiant components and thus would ultimately need
to be assigned to both a zone (air mass or air node, see Zone above)
and an enclosure (with surfaces receiving radiant heat).  A "space"
might be a convenient way of associating certain loads with a
combination of a zone and an enclosure.

Related to this is the concept of SpaceType, such as office or
conference room, which would specify densities for occupancy,
lighting, equipment, etc., and possibly other attributes aligned with
specific codes and standards. Non-geometric connection of loads to
zones and enclosures is supported below with a compound space type
that uses a fractional weighting to distribute energy.

## New Space Object/Layer

### Relationship assumptions ##

First, let's ignore input structures at the moment and potential transition issues.

If we keep the current definition of "Zone" and add the concept of
"Space" (<= Zone), then here's the proposed model:

    Enclosure
        Space 1
            Surfaces
            Internal Gains (People, Lights, etc.)
        Space 2
            Surfaces
            Internal Gains (People, Lights, etc.)

    Zone
        Thermostat
        HVAC equipment
        Space 2
            Surfaces
            Internal Gains (People, Lights, etc.)
        Space 3
            Surfaces
            Internal Gains (People, Lights, etc.)

The concept of a "SpaceType" object would allow for convenient assignment of internal gains to Spaces.

### Definitions and constraints

 * Zone - An air mass connecting surfaces, internal gains, and HVAC equipment for heat balance and HVAC control.
   * *Each Zone is comprised of one or more Spaces.*
   * *All surfaces and internal gains associated with the Spaces are included in the Zone.*
   * *The relationship between Zone and Enclosure is not constrained.*
   * *The Zone heat balance does not change: it has an air node (or a Room Air Model) and includes all surfaces and internal gains from its Spaces
    plus any HVAC which is attached to the zone.*

 * Enclosure - A continuous volume connecting surfaces for radiant and solar exchange.
   * *Each Enclosure is comprised of one or more Spaces.*
   * *All surfaces and internal radiant gains associated with the Spaces are included in the Enclosure.*
   * *The relationship between Zone and Enclosure is not constrained.*
   * *Enclosures only distribute radiant and solar (visible) energy to the surfaces.*
   * *There is no full heat balance at the enclosure level. Each enclosure only balances the radiant/solar flux on each surface. These
   fluxes then become part of the surface inside heat balance.*

 * Surface - A geometric plane which is attached to a Space.
   * *A surface can be opaque, transparent, or an air boundary.*
   * *Each Surface belongs to one Space.*
   * *Each Surface belongs to one Zone.*
   * *Each Surface belongs to one Enclosure.*

 * Space - A collection of one or more surfaces and internal gains.
   * *Each Space belongs to one Zone.*
   * *Each Space belongs to one Enclosure.*
   * *Internal gains may be assigned by a SpaceType object.*
   * *The Spaces in a Zone are lumped together for the Zone heat balance.*
   * *Optionally allow Space-level heat balances.*

 * SpaceType - List of internal load characteristics, ventilation requirements, etc. (details TBD).

 * CompoundSpaceType - List of SpaceTypes with fractions

## Proposed Input Approach ##
Very minimal changes to current inputs.

 * Zone remains as-is.
 * ZoneHVAC and Thermostats remain as-is.
 * Add Space object
 * Add SpaceType and CompoundSpaceType objects
 * Consider adding HVAC connections and thermostats at the Space level (future)

### Proposed objects:

    Zone,  !- Same as before, but maybe shorter
      Name,
      !- Move some or all of these fields to the Space object?

    * AR1: I think so

      Origin,
      Multiplier,
      Ceiling Height,
      Volume,
      Floor Area,
      Convection algorithms,
      Part of Total Floor Area;

    * AR1: should this be an IDF object or will this be an internal construct?
    Ditch this - keep it internal
    Enclosure, !- New object
      Name;

    SpaceType,
      Name,

    * AR1: In addition to the name of the SpaceType, which can be
      anything, there also needs to be a standard enumeration (e.g.,
      Office, Kitchen, Corridor) as well as a reference to that
      standard, either Name and Version (e.g., ASHRAE90.1 and 2004) or
      NameVersion (e.g., ASHRAE90.1-2004)

      SpaceTypeStandardEnum,
      SpaceTypeStandardName,
      SpaceTypeStandardVersion,

!- The components of this space type are determined through references in Lighting/People/Equipment
      !- (makes it easier to maintain as we add more associated gain object types [e.g., all the types of equipment]
      !- or need more flexibility than just schedules and power densities)
      !- This should probably be a separate discussion.

    * AR1: yes

    CompoundSpaceType,
      Name,
      SpaceType Name 1,
      Fraction 1,
      ...

    Lighting/People/Equipment,
      Name,
      SpaceType, !- Replace current Zone or ZoneList Name with Space Type (or allow all three?)
      Schedule Name,
      ...

    Space,
      Name,
      SpaceType Name or CompoundSpaceType Name,  !- If blank, there is no "gain aggregation" from this space
                                                    !- in the corresponding Zones/Enclosures
      Zone Name,

      !- Move some or all of these fields from the Zone object?
      Origin,
      Multiplier,
      Ceiling Height,
      Volume,
      Floor Area,
      Convection algorithms,
      Part of Total Floor Area;

    Surface,
      Name,
      ...
      Space Name,  !- Replace current Zone Name (or allow both?)
                   !- This provides "surface assignment" functionality for all surfaces
                   !- and potentially "gain aggregation" for floor surfaces.
      ...

    * AR1: What about AirBoundaries in the new formulation.

### Options/Questions for Discussion

    * AR1: A lot of these are correlated

 * Require the Space object or make it an optional layer?
 * If Space is required, then move most Zone fields to the Space object?
 * Allow internal gains to continue referencing Zone or ZoneList names directly?

    * AR1: If it were optional, could we support Zones both with an
      without Spaces in the same model?  For Zones without Spaces, we
      would internally create a Space that is one-to-one with the
      Zone.  From an IDF standpoint, it would be cleaner to require
      Spaces and then middleware or applications would have to
      create the one-to-one corresponding Space on their side.

* Allow Thermostats and ZoneHVAC:EquipmentConnections to reference Spaces or Zones?

    * AR1: Wouldn't moving these to the Space elvel imply that we are
      doing heat-balance at that level?  What is the point of having a
      Zone then?  Alternately, what have we done by adding a Space
      object other than create a cleaner bookkeeping mechanism for
      Zone and Enclosure?

* Omit the Enclosure object and "Enclosure Name" field in the Space object and assign Enclosures automatically (as is currently done), one-to-one with Spaces unless there are air boundaries.
     - Yes, leave this out
 * ~If Enclosure is required, then collapse solar and radiant enclosures into a single enclosure
 (separate enclosure options were introduced with Construction:AirBoundary)?~
 * Some of the options result in objects that only have a single field for Name, e.g. Enclosure - are we ok with that?

     * AR1: I think we should keep Enclosure internal. From a design
       standpoint, there is no point in asking the user or application
       to figure something out that you can figure out yourself.  It's
       just another opportunity for them to screw things up.  A
       potential exception is if it is very difficult to figure out
       Space-to-Enclosure assignment.

## Data Structure Considerations ##
 * Keep existing zone-based data structures and use Spaces to simply map surfaces and internal gains to Zones?

      * AR1: Don't you need some of this data at the Space level so
        that you can also aggregate it up these to Enclosures?

 * Remember Space-level assignments for surfaces and internal gains to allow for Space-level heat balance?
 (This comment in the setup implies we need to keep track: "enable space-level book-keeping and reporting").

 * Surface ordering:
   - Group by Enclosure (because enclosure radiant exchange is more computationally intensive than zone heat balance)
   - Group by Space within each Enclosure.
    -Group by surface type within each Space

e.g., Enclosure1 contains Space1 and Space3, Enclosure2 contains Space2, etc.

      Space1-OpaqueSurface1 (begin surfaces in Enclosure1)
      Space1-OpaqueSurface2
      Space1-Window1
      Space1-Window2
      Space3-OpaqueSurface1
      Space3-OpaqueSurface2
      Space2-OpaqueSurface1 (begin surfaces in Enclosure2)
      Space2-OpaqueSurface2
      Space2-Window1

## Testing/Validation/Data Sources ##

insert text

## Input Output Reference Documentation ##

insert text

## Input Description ##

insert text

## Outputs Description ##

insert text

## Engineering Reference ##

insert text

## Example File and Transition Changes ##

* If Space, SpaceType, and Enclosure are optional, then no transition required.
* If Space is required, then transition required to add Space and change Surfaces to reference the new Space names.
* If Enclosure is required, then transition required to add Enclosure and determine Space-Enclosure relationships in old idf.
* If SpaceType is required, then transition required to add SpaceType and change internal gains to reference the new SpaceType names.

## References ##

insert text

## E-mail and  Conference Call Conclusions ##
### Oct 11 - Nov 14, 2019
E-mail thread with Roth, Hong, Lee, Adams, Witte, Glazer, Parker, DeGraw, Merket,
Benne, Horowitz, and Kruis is condensed into this NFP. Major points of concensus/divergence are
highlighted.

### Nov-Dec 2019 Comments

Amir - I am agnostic about the IDF/epJSON implementation (child-points-to-parent, parent-points-to-children, hybrid), I
can see the benefits of doing things either way. Parent-points-to-children is more intuitive (to me) and most calculations will go this way. Child-point-to-
parent is easier in terms of sanity and invariant checking and 1-to-1 relationships are generally easier to deal with than 1-to-many. I think that
decision should be made to simplify model development and editing and if there is no clear favorite there then it should be made to preserve the current
structure as much as possible.

Amir - Maybe/probably because I am the least informed person on this thread, this doesn�t make sense to me. In my mind, the current definition
of Zone is not broken. What is broken (or at least not clean) is the relationship between Zone and Enclosure. The use of Space as an atom that
can tie Zones and Enclosures together cleanly and flexibly (and also serve as an anchor for internal loads) fixes that.

Amir - The problem with �keeping things really simple and removing the Space concept� is that it reverts the ability to have a clean and
flexible relationship between Zone and Enclosure, which is one of the things we wanted to have. Again, I think it would be possible to add
Space objects but keep our current definition of Zone�the Zone �owns� the air-volume, the thermostat, and the HVAC system. The Zone internal
loads are aggregated over all the Spaces in the Zone.  The Zone heat balance includes those loads, the air-volume, and all Surfaces attached
fto all Spaces in the Zone. What I hear you saying is not that we have to change heat-balance to work at the Space level, it�s that we actually
*want to change because with improper zoning, spaces within a zone can have highly varying conditions and thermostat placement matters*.
In other words, EnergyPlus currently does a poor job of simulating poorly zoned buildings and as a result does not support evaluation of proper zoning.
 Is that right? (Yes)

Neal - Before we try to tackle terminology, we should make sure we are clear on the conceptual components. I propose we talk about it as:

  * Air nodes: Distinctly unique temperatures of air within the building, each with their own heat balance (based on surface convection,
    forced air systems, convective internal gains). These can be referenced by traditional thermostats to control HVAC systems.

  * Enclosures: A collection of components that participate in radiant exchange (based on surface long wave temperatures, radiant internal gains,
    transmitted solar). These can be referenced by thermal comfort based thermostats to control HVAC systems.

  * Gain aggregators: A mechanism to aggregate internal gains to air nodes and enclosures (often based on a floor surface area).
    I think this is what most people associate with the term "spaces"

  * Surface assignments: A mechanism to assign surfaces to air nodes and enclosures.

I think we are relatively set on the term "Enclosures". Not much confusion there. But people are using combinations of "Zone" and "Space"
to describe combinations of the other three, and I think it's important to keep these roles separate in our discussion. I think it makes
sense to use the same construct and term for both "gain aggregators" and "surface assignments". My initial inclination is to use the term "Space"
for both of these and to use "Zone" for "air nodes" (for historical consistency), but I could be persuaded otherwise. Generally, I don't like the
term "HVACZone" except as a mechanism to associate air nodes served by the same system for reporting or visualization.

We should enumerate the use cases in the NFP so we don't lose sight of anything in these discussions (feel free to add):

  1. Many air nodes within a single enclosure (e.g., open offices

  2. Many enclosures within a single air node

  3. Aggregating gains assigned to a floor surface to the appropriate air nodes and enclosures

Amir - The terminology Neal describes is close to my initial understanding:

  * Space holds internal gains, surfaces (potentially airwalls), and an air-node

  * Enclosure aggregates and operates on the non-airwall surfaces of its child spaces

  * Zone aggregates and operates on the internal gains, surfaces, and air-nodes of its child Spaces, and also holds the thermostat and
HVAC system.

I think the object design he describes is also cleanest design in terms of transition from the current schema.

I do want to point out, however, that modeling poorly-zoned buildings using slave zones is not the same as modeling heat-balance at the space-level.
The former is a work-around that requires you to understand a priori that the building is poorly-zoned. The second will tell you that the building is
poorly zoned regardless of what you do. There�s a big conceptual difference between those two. We need to consider this carefully and
I don�t think we can defer the decision because moving to space-level heat-balance requires that we move the association of HVAC and Thermostat objects
 from Zone to Space.

Jason G. - I think you are heading in the right direction but I do wonder if the value of making this change is worth the effort for users and
GUI developers. What if these changes are incorporated into new objects and the existing ones continue to work (for a few releases?) and mapping
from the old to new is done internally? Don't underestimate the hassle this will cause to many users and GUI developers so the value that is being
 gained from the user perspective should be spelled out explicitly.

