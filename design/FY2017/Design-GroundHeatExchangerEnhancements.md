## Design- Ground Heat Exchanger Enhancements

**Matt Mitchell, Jeffrey Spitler**

**Oklahoma State University**

- Original Date: Aug 15, 2017

## New Feature Proposal

See the file NFP-GroundHeatExchangerEnhancments.md  for details on the justification, overview, and output description.



## Overview

All ground heat exchanger related code is located in the ```GroundHeatExchangers.cc``` and ```GroundHeatExchangers.hh``` files. The current project will keep all GLHE code contained within these files.

The GLHE code was modified to an object oriented design several years ago when the GLHE Slinky model was incorporated into EnergyPlus. As a result, the same routines and data structures will largely be reused.

## Approach

### Modifications to GroundHeatExchangers.cc

The function ```GLHEVert::calcGFunctions()``` will be where the g-function generation code is added. This function is currently empty and was stubbed out during previous work for this purpose.

The function ```GLHEVert::calcHXResistance()``` will be where the short timestep borehole model will be added.

The function ```GLHEBase::calcAggregateLoad``` will be modified to calculate the aggregated load using the updated method described in the NFP.

The function ```GLHEBase::calcGroundHeatExchanger``` will be simplified and will call the functions necessary to calculate the current temperature response of the GLHE. The function currently performs some of the load aggregation functions, as well as borehole resistance functions. This will be moved the corresponding code blocks where these calculations are performed.

### Modifications to GroundHeatExchangers.hh

The data structures will see minimal modifications. The GLHE code uses a factory method to create new GLHE objects when called. These new objects will continue to be stored on the ```verticalGLHE``` and ```slinkyGLHE``` arrays. Pointers to the objects will continued to be referenced by the plant loop solver, as is currently done.

### Object-Oriented Programming/Refactorization

As has been mentioned, the code already largely adheres to an OOP design, however, there may be some refactorization which will occur. The input scheme is moving from idf to json format, so those changes will be incorporated at a minimum.
