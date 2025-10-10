A new algorithm for polygon clipping in solar calculations for rectangular surfaces

================

**Xuan Luo, Jerome Wei, Tianzhen Hong**

**Lawrence Berkeley National Laboratory**

 - October 21, 2019

## Justification for New Feature

There exists an extensive amount of literature on polygon clipping when constrained to rectangular surfaces. The current polygon clipping method implemented in EnergyPlus, using the *Sutherland-Hodgman* algorithm, is a performance hotspot. The method is generalized for clipping area of all shapes, but in EnergyPlus simulations runs, are often called with only rectangular surfaces.

We propose a negligible amount of code to check if the clipping surface is rectangular. And if so,
we redirect to a new method using the `Slater & Barsky` algorithm.


## Overview

To clip a polygon against a rectangular region, we can go segment by segment and clip each line individually, saving the unique points of intersection as we go. Then, we add the necessary corners of the clipping rectangle making sure that clockwise order is preserved.

The `Slater & Barsky` algorithm relies on converting the line segment to be clipped into a parametric equation. Below is the general form for the x- and y- components of a parametric line:

- <img src="https://latex.codecogs.com/svg.latex?  x = x_0 + t \Delta x" />

- <img src="https://latex.codecogs.com/svg.latex?  y = y_0 + t \Delta y" />

Letting our subject line be starting at (x_0, y_0) and ending at (x_1, x_1):

- <img src="https://latex.codecogs.com/svg.latex?  \Delta x = x_1 - x_0" />

- <img src="https://latex.codecogs.com/svg.latex?  \Delta y = y_1 - y_0" />

Distances from the endpoints to the edges of the clipping rectangle that collide with the line are used to obtain two values of *t* that parameterize the equation, representing the pair of new endpoints.

Once we have the two values of t, t_1 and t_2, we can calculate the clipped line endpoints (Note that t_2 > t_1).

- <img src="https://latex.codecogs.com/svg.latex?  x'_1 = x_0 + t_1 \Delta x" />

- <img src="https://latex.codecogs.com/svg.latex?  y'_1 = y_0 + t_1 \Delta y" />

- <img src="https://latex.codecogs.com/svg.latex?  x'_2 = x_0 + t_2 \Delta x" />

- <img src="https://latex.codecogs.com/svg.latex?  y'_2 = y_0 + t_2 \Delta y" />

The `Slater & Barsky` algorithm uses space subdivision to reduce the number of pre-emptive calculations. We break the plane into 9 parts, where region (4) is the clipping region.


| 6 | 7 | 8 |

| 3 | 4 | 5 |

| 0 | 1 | 2 |

With this method, calculating deltas and plugging in the final parametric equations can be skipped if the subject line obeys certain conditions. For example, if the line begins in region (0) and ends in region (6), then no new endpoints need to be calculated.

Below are performance results for the calls to the solar clipping function `CLIPPOLY` in EnergyPlus. We see that the method provided by Slater & Barsky 1994 has a 31% speedup over all rectangular inputs in the **SolarShadingTest.idf** test file. Note that this performance gain is limited to rectangular inputs, so an estimate of the performance gain to the existing algorithm overall then is a function of the percentage of calls which apply a rectangular window. For example, if 33% of the calls are rectangles then the overall speedup to `CLIPPOLY` would be diminished by a factor 0.33.

#### Best case taken from 5 trials (SolarShadingTest.idf)

|                            | Slater-Barsky 1994 |
|----------------------------|--------------------|
| Rectangular inputs         | 31% speedup        |
| Overall assuming 50% rects | 15% speedup        |

#### Worst case taken from 5 trials (SolarShadingTest.idf)

|                            | Slater-Barsky 1994 |
|----------------------------|--------------------|
| Rectangular inputs         | 23% speedup        |
| Overall assuming 50% rects | 11% speedup        |


## Approach ##

We propose to add a new key, `SlaterBarskyandSutherlHodgman`, to the existing `ShadowCalculation` object, `Polygon Clipping Algorithm` field, for global shadow calculation control：

	ShadowCalculation,
       \unique-object
       \memo This object is used to control details of the solar, shading, and daylighting models
       \extensible:1
       ...
	  A2 , \field Polygon Clipping Algorithm
       \note Advanced Feature.  Internal default is SutherlandHodgman
       \note Refer to InputOutput Reference and Engineering Reference for more information
       \type choice
       \key ConvexWeilerAtherton
       \key SutherlandHodgman
       \key SlaterBarskyandSutherlHodgman
       \default SutherlandHodgman

If `SlaterBarskyandSutherlHodgman` is chosen, the polygon clipping for rectangular surfaces will be calculated using the *Slater and Barsky* algorithm, while the rest adopts the default *Sutherl and Hodgman* algorithm.

## Testing/Validation/Data Sources ##

The current test file, "SolarShadingTest.idf", is used for performance and accuracy evaluation and validation.

## Input Output Reference Documentation ##

TBD

## Input Description ##

N/A

## Outputs Description ##

N/A

## Engineering Reference ##

TBD

## Example File and Transition Changes ##

An example file "SolarShadingTest-Slater-Barsky.idf" will be developed based on the existing test case for testing the new algorithm implementation.

No transition change is required.

## References ##

insert text



















