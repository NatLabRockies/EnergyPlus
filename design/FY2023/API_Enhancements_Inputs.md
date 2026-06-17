# Enhancing the Usability of the EnergyPlus API

## Document Tracking

- 2023-06-21 Initial draft
- 2023-08-02 Revised draft with more design detail
- 2024-02-07 Note that some of these enhancements are now being completed for 24.1, while the remaining pieces will be pushed to 24.2.

## Background

- API activity is continuing to rise, and a new discussion was opened on E+ to perpetually track user activity/requests
- Every API demo I give hints at ways users want to interact and difficulties they encounter. A few aspects have recurred:
  - Being able to tie together multiple simulation engines and keep E+ in sync (out of scope for this -- improved mechanization)
  - Being able to easily deploy and embed E+ into workflows (out of scope for this -- pip install)
  - Being able to create reusable functionality (I already did this one)
  - Odd nature of hardwired discrete EMS integer values (that's here!)
  - Cannot actuate User-Defined Components with callback functions, only with Python-Plugins (that's here too!)

## Problem Statement

I had always hoped that users would develop their own reusable scripts and packages, which is one reason that users provide custom search paths for Python in their input files.
However, API callback functionality is tied very closely to a single input model because of hardwired object names.
Users can create fully functional generic code and then create thin wrappers that tie the functionality to a single model, but this can be improved.

By providing a set of API functions that provide the current input structure to the user, they can directly write generic code in more situations.
Most users still operate on IDF, so providing the already-interpreted input deck as a Python object will lead to a lower bar for writing generic code.

In addition, there are "magic numbers" in the EMS interface which allow certain control strategies or settings to be applied discretely based on integer values.
Internally to EnergyPlus, these are often stored as an enumeration, and as the numbers have meaning, they must be hardcoded there as well.
These are not currently exposed as symbols through the EnergyPlus API, which means user scripts must hardcode these there too.
To alleviate this, it is proposed that an automated method be created which either:
 - programmatically exposes these internal values through symbols in the API, or
 - allows a call into an API function to get symbolic values for certain discrete settings or configurations.

The final problem here relates to the user-defined components and the API.
As of right now, the user-defined components in EnergyPlus require the user to specify an initialization routine and a simulation routine.
These are either ERL-based programs, or Python-Plugin instance names, and there is no way to utilize an API callback function for either actuation.
Users who want to take advantage of the API/callback workflow are limited in this and cannot combine the workflow with user-defined component functionality.

## Design of API Functions for Accessing Current Inputs

I want to provide an API endpoint that provides input data as Python objects.
Any functions here would belong in the data exchange API group, as it relies on a currently running simulation.
The inputs, whether EpJSON or IDF (or CBOR, or...), are already interpreted into a JSON object in the EnergyPlus C++ at runtime.
The new endpoints would provide methods to access a list of names for object types, for example:

I already completed this as a small side task in https://github.com/NatLabRockies/EnergyPlus/pull/9973, so this is done.

## Design of EMS Magic Number Solution

The solution here is a bit less clear.

If the agreed solution is to expose the internal enumerations through a set of symbolic constants in the API layer, then
the result may be a small script that scrapes the EnergyPlus code as a part of the libenergyplusapi target and creates an API file full of symbols.
This file would be packaged like all the other API files and users could import the symbols and avoid having to track magic numbers.
This solution could help future proof the user code in case the symbols changed for some reason, although it is doubtful we would make that much of a breaking change anytime soon.
I would expect the symbols to remain an enum class in the core C++ code, but it would still need to be determined how to pass this information to both C and Python.

Consider the before:

```python
api.exchange.set_actuator_value(state, actuator_handle, 3)
```

versus the after:

```python
api.exchange.set_actuator_value(state, actuator_handle, api.constants.operation_mode.cooling)
```

If the agreed upon solution is to create a worker function inside the EnergyPlus API to "mine" out the magic numbers based on symbolic entries, it actually becomes a bit easier.
Any enumeration that is exposed through EMS actuation would need to be given a symbolic class and key name.
Then a single worker function would be created that takes string views of the class and key name.
Internally, there could be a nested switch block that simply returns the enumerated value as an integer back to the user.
The user would then avoid magic numbers.
Consider the before:

```python
api.exchange.set_actuator_value(state, actuator_handle, 3)
```

versus the after:

```python
operation_mode = api.functional.get_enum_value("OperationMode", "Cooling")
api.exchange.set_actuator_value(state, actuator_handle, operation_mode)
```

This would still require a string in the Python code, but feels a ton better than a magic number.
A quick scan of all the calls to SetupEMSActuator reveals a few possible candidates for this new paradigm.

There are several plant-based supervisory control actuators that were expected to only carry discrete 0 (off) and 1 (on) values.
However, the component supervisory actuators respond to that actuator in a continuous nature, allowing the user to scale the available load capacity for that component.
So for now at least, those plant actuators will be left alone, and not captured in this magic-number cleanup.

The remaining candidates include:

| Actuator "Object"                          | Control Type        |
|--------------------------------------------|---------------------|
| AirLoopHVAC                                | Availability Status |
| Window Shading Control                     | Control Status      |
| Variable Refrigerant Flow Heat Pump        | Operating Mode      |
| Coil:Cooling:DX:SingleSpeed:ThermalStorage | Operating Mode      |

The "AirLoopHVAC" actuator with control type "Availability Status" accepts the following discrete values, currently as magic numbers, as described in the EMSApplicationGuide document:

- 0.0 ( = NoAction). This tells the air system to do whatever it would usually do without any special override status.
- 1.0 ( = ForceOff). This overrides the air system to shut down when it would normally want to run.
- 2.0 ( = CycleOn). This overrides the air system to start up when it would normally be off.
- 3.0 ( = CycleOnZoneFansOnly).

There "Window Shading Control" actuator with control type "Control Status" has a relatively long list of magic numbers with the following meanings:

- –1.0: No shading device.
- 0.0: Shading device is off (applies to shades and blinds).
- 1.0: Interior shade is on.
- 2.0: Glazing is switched to a darker state (switchable glazing only).
- 3.0: Exterior shade is on.
- 4.0: Exterior screen is on.
- 6.0: Interior blind is on.
- 7.0: Exterior blind is on.
- 8.0: Between-glass shade is on.
- 9.0: Between-glass blind is on.

The "Variable Refrigerant Flow Heat Pump" actuator with control type "Operating Mode" accepts the following discrete values:

- 0.0: System is OFF
- 1.0: System is in Cooling Mode
- 2.0: System is in Heating Mode

The "Coil:Cooling:DX:SingleSpeed:ThermalStorage" actuator with control type "Operating Mode" accepts the following discrete values:

- 0 = Off Mode
- 1 = Cooling Only Mode
- 2 = Cooling and Charge Mode
- 3 = Cooling and Discharge Mode
- 4 = Charge Only Mode
- 5 = Discharge Only Mode

As we can see, the magic number use is not consistent, and can lead to confusing, difficult to maintain Python code.
The primary goal for this work will be to add a worker function that allows the user to capture these control signals by meaningful name, eliminating the magic numbers.
As further actuators are added to the program, if they are managing discrete control signals, they can be added to this worker function.
The implementation of this function will almost certainly be easiest to maintain as a simple IF ladder, although it is possible that the strings could be hashed into integral values that could be SWITCHed.

As of right now, my preferred path forward would be the following steps:
- Each of the above discrete EMS actuators should have an enum class that they are using internally, move these to a single header file just containing the enums
- Create an API worker function that just relies on this one enum header, the basic GetEnumValue (sp?) function, and the MakeUPPER function, to return a meaningful integer value or ::Invalid
- Evaluate the -1 magic number being used on the window shading control type, and how to proceed there.

## Improvements to User-Defined Components

The API callbacks do not have "names" in the input file, which immediately breaks any option to use callbacks to handle user defined initialization and simulation.
In order to get the user defined components working with the API, we need to make two changes:
 - Modify the input structure and code of the user-defined components so that we allow a name input that is not directly found in the input file
 - Add a callback registration function where an external function can be registered with a name argument, that can be paired up to the user-defined component function name

The input file changes would look like this:

```
UserDefinedComponentFoo,
  My User Defined Component,   !- Name
  ...other inputs...,          !- Etc
  MyInitRoutine,               !- Name of the init routine, could be an EMS:Program, a PythonPlugin:Instance, or the name of a registered user-defined init callback
  MySimRoutine,                !- Name of the sim routine, could be an EMS:Program, a PythonPlugin:Instance, or the name of a registered user-defined sim callback
  ...etc...;
```

And the new callback registration function would be something like:

```python
def my_user_defined_init_function(state):
  pass  # foo

def my_user_defined_sim_function(state):
  pass  # foo

api.runtime.callback_user_defined_init(state, "MyInitRoutine", my_user_defined_init_function)
api.runtime.callback_user_defined_sim(state, "MySimRoutine", my_user_defined_sim_function)
```

An API user will register the callback function ahead of the call to run EnergyPlus.
The input processing for the user defined component will need to check the name entered for each function and determine if it is an ERL program, Python Plugin, or API callback.
If there is a duplicate, or none match, errors/warnings will be emitted.
