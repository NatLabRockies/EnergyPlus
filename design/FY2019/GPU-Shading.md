GPU-based Shading Calculations
==============================

**Neal Kruis, Big Ladder Software, LLC**

## Justification for New Feature ##

GPU architecture is optimized for performing geometry operations. Leveraging methodologies used in the gaming industry to represent shadows, the GPU can (in theory) improve computational performance of shading calculations and provide greater flexibility (e.g., handle concave geometries and solar transmission through interior glazing).

## Approach ##

The general approach is to use the Pixel Counting methodology described by Jones et al. In this approach, the building is rendered using OpenGL. Each surface is then viewed from the (parallel projection) perspective of the sun. The number of visible pixels for that surface is a proxy for the projected sunlit surface area. Dividing by the cosign of incidence for that surface gives the total sunlit surface area.

### Exterior Shading ###

Pixel counting for exterior shading can be handled using Big Ladder's [Penumbra](https://github.com/bigladder/penumbra) library (a C++ implementation of Jones's pixel counting method).

### Interior Solar Distribution ###

We will count pixels when viewing the from the perspective of the sun through a window with each internal surface assigned a different color, then use the histogramming functions in OpenGL to report the number of pixels of each color that are visible. This functionality will be added to Penumbra.

### Pitfalls ###

#### Hardware requirements ####

This approach only works if there are graphics drivers (or emulators) that support OpenGL version 2.1 (or higher) on the machine running the code. This introduces a new hardware requirement for EnergyPlus. If GPU-based shading is requested and the required hardware (or emulated hardware) is not present, EnergyPlus will issue a warning and revert to the CPU-based polygon clipping method.

#### Transparent shading surfaces ####

Non-opaque shading surfaces are difficult to characterize under this approach (although, from early testing of SolarShadingTest.idf it doesn't appear to be working properly with the CPU calculations either).

There are a couple potential solutions to this problem:

1. We use the perforated approach described by Jones.
2. We introduce transparency to the OpenGL rendering and calculate the color of the pixels returned.
3. We render with and without transparent surfaces to evaluate their impact.

## Input Output Reference Documentation ##

See proposed changes in [Energy+.idd.in](https://github.com/NatLabRockies/EnergyPlus/pull/7302/files#diff-23ccf090b80d26e885712256b9a6d888). Will draft document once IDD is reviewed.

## Engineering Reference ##

Mainly a reference to Jones's papers.

## References ##

[Fast computer graphics techniques for calculating direct solar radiation on complex building surfaces](http://dx.doi.org/10.1080/19401493.2011.582154)

Nathaniel L. Jones, Donald P. Greenberg, and Kevin B. Pratt. Journal of Building Performance Simulation. Volume 5, Issue 5, Pages 300-312. June 26, 2011.

[Hardware accelerated computation of direct solar radiation through transparent shades and screens](https://nljones.github.io/publications/SB12_TS09b_3_Jones.pdf)

Nathaniel L. Jones and Donald P. Greenberg. 5th National Conference of the International Building Performance Simulation Association-USA. Madison, Wisconsin. August 1-3, 2012
