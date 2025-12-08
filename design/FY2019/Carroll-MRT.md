Carroll Mean Radiant Temperature option for Interior Radiant Exchange
=====================================================================

**Neal Kruis, Big Ladder Software, LLC**

## Justification for New Feature ##

A well-known performance bottleneck in EnergyPlus is the calculation of interior long-wave radiation exchange. This is a dense-matrix, linear algebra problem with O(n^2 ) complexity. One approach used in other programs with comparable accuracy is the Carroll method (see Carroll 1980, 1980a, & 1981). There are several similar MRT methods with linear complexity including methdods by Walton (used in BLAST) and Seem (used in TRNSYS). Unlike Walton's method, Carroll's method balances radiant heat without an additional term to balance the heat flow between surfaces.

## Approach ##

The Carroll method is an approximation of gray-body long-wave radiation exchange within an enclosure that simplifies the surface-to-surface radiation exchange by using a single, mean radiant temperature node, Tr, that act as a clearinghouse for the radiation heat exchange between surfaces. Instead of solving a dense-matrix, linear algebra problem, the mean radiant temperature can be calculated using a single equation, and subsequently used to determine the net long-wave radiation to/from each surface. Unlike the O(n^2 ) complexity of the current dense-matrix solution, this approach has linear complexity.

The mean radiant temperature is calculated using three steps:

1. Calculation of the mean radiant temperature “view factor”, Fi. These view factors represent each surface’s “view” to the mean radiant temperature node as though all surfaces were part of a spherical enclosure (i.e., they all have equal view of the node regardless of their orientation to each other). Fi is calculated as:

    $$F_i=\frac{1}{1-\frac{A_i F_i}{\sum_1^n{A_j F_j} }}$$

    Because of the circular reference in this equation, the collection of all “view factors” must be solved iteratively, but only once per simulation as surface areas do not change throughout. This converges for realistic enclosures but won’t necessarily converge for “enclosures” having only two or three surfaces, particularly if there are large area disparities.

2. Calculating the gray-body radiation resistance, F’i. This calculation must be computed every time surface emissivity changes. F’i is calculated as:

    $$F'_i=\frac{\sigma\varepsilon_i}{\frac{\varepsilon_i}{F_i} +1-\varepsilon_i}$$

3. Finally, the mean radiant temperature, Tr, is:

    $$T_r=\frac{\sum_1^nA_i F'_i T_i}{\sum_1^nA_i F'_i}$$

Once the mean radiant temperature is known, the net radiation heat transfer for each surface can be calculated as:

$$q=F'_i A_i (T_r^4-T_i^4)$$


## Input Output Reference Documentation ##

See proposed changes in [Energy+.idd.in](https://github.com/NREL/EnergyPlus/pull/7534/files#diff-23ccf090b80d26e885712256b9a6d888). Will draft document once IDD is reviewed.

## Engineering Reference ##

See approach.

## References ##

Carroll, J. A., 1980, An ‘MRT Method’ of Computing Radiant Energy Exchange in Rooms, Proceedings of the Second Systems Simulation and Economic Analysis Conference, San Diego, CA.

Carroll, J. A., 1980a, "An MRT method of computing radiant energy exchange in rooms," Proceedings of the 2nd Systems Simulation and Economic Analysis Conference, San Diego, CA.

Carroll, J. A., 1981, "A Comparison of Radiant Interchange Algorithms," Proceedings of the 3rd Annual Systems Simulation and Economics Analysis/Solar Heating and Cooling Operational Results Conference, Reno. Solar Engineering, Proceedings of the ASME Solar division.
