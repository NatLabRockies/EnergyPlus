# Ground Heat Exchanger Simulation Code

The code in this source directory used to live in a single source file called "GroundHeatExchangers.cc".
This file contained an ever growing amount of classes and functions, and it was getting difficult to maintain.
So it was split up into a few useful files.  
The ultimate goal is roughly one class per file, but there are some very small classes that don't justify their own file.

Here is some historical and reference information about the ground heat exchangers simulated in this code:

## Author Information
 - Original Authors:
   - Arun Murugappan and Dan Fisher, September 2000
 - Modifications:
   - Brent Griffith, September 2010 for plant upgrades
   - Matt Mitchell, February 2015 for slinky heat exchangers and object-oriented code

## Purpose and Methodology
The module contains the data structures and routines to simulate the operation of vertical closed-loop ground heat exchangers (GLHE) typically used in low temperature geothermal heat pump systems.

The borehole and fluid temperatures are calculated from the response to the current heat transfer rate and the response to the history of past applied heat pulses. 
The response to each pulse is calculated from a nondimensionalized response function, or G-function, that is specific to the given borehole field arrangement, depth and spacing.
The data defining this function is read from input.
The heat pulse histories need to be recorded over an extended period (months).
To aid computational efficiency past pulses are continuously aggregated into equivalent heat pulses of longer duration, as each pulse becomes less recent.

## References
- Eskilson, P. 'Thermal Analysis of Heat Extraction Boreholes' Ph.D. Thesis: Dept. of Mathematical Physics, University of Lund, Sweden, June 1987.
- Yavuzturk, C., J.D. Spitler. 1999. 'A Short Time Step Response Factor Model for Vertical Ground Loop Heat Exchangers. ASHRAE Transactions. 105(2): 475-485.
- Xiong, Z., D.E. Fisher, J.D. Spitler. 2015. 'Development and Validation of a Slinky Ground Heat Exchanger.' Applied Energy. Vol 114, 57-69.
