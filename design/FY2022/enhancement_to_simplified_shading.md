Simplified Shading
================

**Yujie Xu, Tianzhen Hong**

**Lawrence Berkeley National Laboratory**

- Original Date: April 25, 2022
- Modified Date: April 27, 2022

## Justification for New Feature ##

Currently, EnergyPlus models shading devices in *WindowMaterial:Shade* and *WindowMaterial:ComplexShade*. Even for the non-complex shade, users need to specify at least nine input fields including solar and visible transmittance and reflectance, infrared hemispherical emissivity and transmittance, thickness, conductivity, etc. This level of complexity could hinder the application of the shading model in EnergyPlus: when the required input fields are difficult to acquire, users might choose to not model shading devices at all. One such case could be converting models from other simulation tools with less detailed shading methods. The omission of shading devices could lead to an overestimation of cooling loads in the summer and an underestimation of the heating load in winter.

The proposed new feature will enable a simplified way to specify the properties of a shading device with a multiplier for solar transmittance and visible transmittance. The multiplier can be a single fixed value or a schedule of values. An idd object *SurfaceProperty:IncidentSolarMultiplier* will be added to enable users to specify the multiplier. This multiplier resembles the SHADING-SCHEDULE in DOE-2, which can adjust the solar heat gain through windows with a multiplier schedule.

This post on unmethours.com shows a need for such a simplified shading method in EnergyPlus: https://unmethours.com/question/342/what-assumptions-do-you-make-for-modeling-blinds/

## Overview ##

Shading devices in EnergyPlus are modeled as a physical layer inside or outside of a window, or between glass layers within a window. Currently, EnergyPlus models in great detail the convective and radiative interaction between the shading layer and the glass layers and the indoor or outdoor environment, as well as the heat absorbed by the shading device.

The most commonly used *SurfaceProperty:IncidentSolarMultiplier* requires nine inputs, which might not be readily available. For example, DOE-2 or eQUEST can model shading devices as a fractional adjustment to reduce the solar transmission from the outside to the inside. When converting such DOE-2 or eQUEST models into an EnergyPlus model, users have to go through a trial-and-error process, as there is no corresponding fractional adjustment in EnergyPlus.

This feature will provide a simplified way of discounting indoor-outdoor solar and visual transmission using a similar fractional adjustment as DOE-2. This allows easier translation between EnergyPlus and other building energy modeling tools. In this feature, the shading will not be modeled as a physical layer as EnergyPlus currently does. It only adjusts the solar and visible transmittance of the corresponding window, without considering the detailed interactions between shading devices and other adjacent objects or environments.

## Approach	##

With the added *SurfaceProperty:IncidentSolarMultiplier* object, users will be able to input a fractional adjustment α between 0 and 1, either as a constant value or as a schedule. The fractional adjustment will be multiplied to the incident solar of the outside surface of an exterior window. This adjustment is compatible with building shading structures like overhangs, but not with the following regular shading objects in EnergyPlus: WindowMaterial:Shade, WindowMaterial:Blind, WindowMaterial:Screen, WindowShadingControl, or WindowMaterial:*:EquivalentLayer objects. If any of them are defined for a window in addition to the SurfaceProperty:IncidentSolarMultiplier, a warning will be thrown.

## Testing/Validation/Data Source(s) ##

The feature will be tested and demonstrated with a test file derived from *UnitarySystem_DXCoilSystemAuto.idf* by adding a *SurfaceProperty:IncidentSolarMultiplier* object. The time-step output of window solar heat gains and transmitted visible illuminance will be manually checked (also using extreme values: 0 for perfect reflective shade and 1 for no shade or perfect transparent shade). Comparison between a model with *SurfaceProperty:IncidentSolarMultiplier* defined and a model with no *SurfaceProperty:IncidentSolarMultiplier* but with epw solar radiation adjusted.

## IDD Object changes ##

A new object *SurfaceProperty:IncidentSolarMultiplier* will be created.

    SurfaceProperty:IncidentSolarMultiplier,
          \min-fields 3
      A1, \field Surface Name
          \required-field
          \type object-list
          \object-list SurfaceNames
          \note Enter the name of an exterior window outside surface object
      N1, \field Incident Solar Multiplier
          \note a constant multiplier for window solar transmittance
          \note and visible transmittance. If the Incident Solar Multiplier Schedule Name is
          \note defined, the product of these two will be the final Incident Solar Multiplier.
          \type real
          \units dimensionless
          \minimum 0
          \maximum 1
          \default 1.0
      A2; \field Incident Solar Multiplier Schedule Name
          \note The schedule values should be greater than or equal
          \note to 0 and less than or equal to 1.
          \type object-list
          \object-list ScheduleNames

## Proposed additions to Meters ##

N/A

## Proposed Report Variables ##

N/A

## References ##
