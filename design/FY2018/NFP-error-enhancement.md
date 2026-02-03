# NFP: EnergyPlus Error Reporting Enhancement #

## Justification for New Feature ##
As more and more users adopt EnergyPlus and place ever greater demands upon EnergyPlus for both traditional single-simulation
investigations and multiple-simulation parametric efforts, it is imperative that information, warnings, and errors be reported
in a systematic and scalable way. The present error reporting policy, while providing sufficient information for experienced
users, presents new users with a great deal of information.

## Overview ##
EnergyPlus will be modified to replace the current set of logging functions (`Show*Error`, `Show*Message`, etc.) with replacements
that regularize logging to the following three severity levels:

* Warning - EnergyPlus has encountered conditions that may result in simulation problems or unexpected results. Warning logging events indicate that user action may be needed to correct an issue (e.g. no floor in a zone), but that the simulation is able to continue to a successful conclusion. Same as the current "warning" level of severity.
* Error - EnergyPlus has encountered conditions that will likely result in the termination of the simulation. Error logging events indicated that user action will be needed to correct an issue (e.g. ERL syntax errors), and that while EnergyPlus is able to continue to a graceful exit of the program, the simulation may soon be aborted. EnergyPlus may in some situations be able to continue a simulation, but completion of the simulation should not be considered a success. Replaces the current "severe" level of severity.
* Fatal - EnergyPlus has encountered conditions that require immediate termination of the simulation. Fatal logging events indicate something is terribly wrong (e.g. memory allocation failure, accumulation of errors) and the program will exit immediately. Same as the current "fatal" level of severity levels.

Events will have an alphanumeric identifier associated with the logging event (an event code). Output options beyond the currently available text and SQLite forms will be considered once the JSON/CBOR feature has been merged. Logging output will be uniform across all available options in that events will be represented in all options (though possibly in different levels of detail).

## Approach ##

Events will be assigned a seven-character identifier of the following form:

```
[W|E|F]XXX###
```

The first character indicates the severity: 'W' for warnings, 'E' for errors, and 'F' for fatals. The second, third, and fourth
characters make a three-character module code that indicates which module the event is coming from (a module code). At the current
time, only a few module codes have been chosen:

* LGC for legacy events that have not been transitioned to the new scheme (see below).
* GEN for general use
* AFN for AirflowNetwork
* EMS for EMS
* PLT for Plant
* DXC for DXCoils
* PSY for psychrometrics

Additional module codes will need to be selected as progress on the work is made.

This approach is needed because the number of error messages in the code base is very, very large. A survey of version 8.8 resulted in the following function call
counts:

|  Function Name                            |  Approximate Usages  |
|-------------------------------------------|:--------------------:|
|  Show[Warning/Severe/Fatal]Error          |       11,000         |
|  ShowContinueError                        |       13,000         |
|  ShowRecurring[Warning/Severe]ErrorAtEnd  |          600         |
|  Show[Warning/Severe]Message              |          300         |
|  Total                                    |       25,000         |

In order to replace all of these calls with a new approach, a phased approach will be used:

1. Refactor the current error calls to use new code, tagging warnings with the code `WLGC000`, errors with `ELGC000`, and fatals with
`FLGC000`. Outputs will remain as-is (codes will not be output).
2. Modify event calls to use the new code, but without modifying the output (codes will not be output).
3. Once a sufficient proportion of the events have been transitioned to new code, modify output to report event codes.

Modification of event calls requires translating each call, such as

```
ShowSevereError(RoutineName + "An AirLoop branch, " + PrimaryAirSystem(1).Branch(BranchNum).Name +
                ", has two or more fans: " + FanNames);
ShowContinueError("The AirflowNetwork model allows a single supply fan in an AirLoop only. Please make changes in the input file accordingly.");
```

into a different call:

```
tracker.error(ErrorTracking::ErrorCode::AFN001,
              __FILE__,
              __LINE__,
              epfmt::arg("RoutineName", routineName),
              epfmt::arg("BranchName", PrimaryAirSystem(1).Branch(BranchNum).Name),
              epfmt::arg("Fans", FanNames));
```

that is coupled with an error definition in a JSON-formatted file:

```
{
  "type": "error",
  "code": {
    "prefix": "AFN",
    "number": "003"
   },
   "name": {
     "en" : Too Many Fans on AirLoop"
   },
   "message": {
     "en" : [
       "{RoutineName}: An AirLoop branch, {BranchName}, has two or more fans: {Fans}",
       "The AirflowNetwork model allows a single supply fan in an AirLoop only. Please make changes in the input file accordingly."
     ]
   },
   "documentation": {
     "en" : "The specified opening factor is larger than 1, which is not allowed."
   }
}
```

The JSON file is used to generate the necessary C++ code to allow for the above call signature. The format anticipates the inclusion of internationalized messages, but such a  capability will not be implemented at this time. The information is converted from the JSON form into an element of an array inside the tracker object:

```
Event(Event::Type::Error, "EAFN001", {
  "{RoutineName}: An AirLoop branch, {BranchName}, has two or more fans: {Fans}",
      "The AirflowNetwork model allows a single supply fan in an AirLoop only. Please make changes in the input file accordingly."},
   { "RoutineName", "BranchName", "Fans" })
```

When `tracker.error` is called, the error information is looked up and the parameters listed are used to format the messages that are given to the user. A similar mechanism is used for both warnings and fatals.

Recurring errors are handled slightly differently. A recurring error handle is created to an object that the tracker manages, and optionally allows for the caller to manage the shown/not shown decision. The JSON representation of a recurring error is:

```
{
  "type": "error",
  "code": {
    "prefix": "AFN",
    "number": "002"
  },
  "name": {
    "en" : "Opening Factor Too Large"
  }
  "message": {
    "en" : [
      "AirflowNetwork: The window or door opening factor is greater than 1.0 for Multizone:Surface \"{SurfaceName}\"",
      "The window or door opening factor is {OpeningFactor}"
    ]
  },
  "recurring": {
    "message": {
      "en" : [
        "AirflowNetwork: The window or door opening factor is greater than 1.0 for Multizone:Surface \"{SurfaceName}\", continues"
      ]
    },
    "tracking": [
      {
        "parameter": "OpeningFactor",
        "type": "minmax"
      }
    ]
  },
  "documentation" : {
    "en" : "A window or door opening factor is out of range and will be truncated to 1.0"
  }
}
```

Recurring errors are created and used in two steps. First, an error object is created:

```
RecurringHandle handle = tracker.create_recurring(RecurringCode::AFNA0A, 2,
    epfmt::arg("SurfaceName", "Srf-001"));
```

The integer `2` in the call determines the number of times the error will be displayed. After creation, the error can be repeatedly used:

```
 tracker.recurring(handle, __FILE__, __LINE__, epfmt::arg("SurfaceName", "Srf-001"),
    epfmt::arg("OpeningFactor", 1.0));
```

In this situation, the error will be shown twice and then silently tracked. Determination of the shown/not shown decision can be handled in caller code if the alternate creation function is used:

```
RecurringHandle handle = tracker.create_infinite_recurring(RecurringCode::AFNA0A, 2,
    epfmt::arg("SurfaceName", "Srf-001"));
```

Depending on feedback of the development team, preprocessor macros that simplify the calls may be made available.

## Input Output Reference Documentation ##

No new input changes will be made at this time. After inclusion of the JSON output feature, new options for output will be revisited.

Once phase three of the work is reached, a structured table of the event codes will be inclued in as a separate document alongside the IORef that lists the codes and gives a description of the event. For example:

| Code | Type | Name | Reported Parameters | Description |
|:----:|:----:|------|---------------------|-------------|
| FGEN000 | fatal |	Conditions Require Termination | RoutineName | One or more errors have been encountered that require the simulation to be terminated. Check preceding errors and warnings, address the conditions that caused those issues, and try again. |

Given the potentially large number of warnings, errors, and fatals, the tabular form may need to be made available as a separate document as more codes are added.

## Engineering Reference ##
This feature is not planned to impact the calculations performed by EnergyPlus, so additions or changes to the Engineering Reference will be minimal.

## Output Details and Examples ##
The section of this document describing error output will be modified to reflect the changes determined upon at the design stage.

## Example File and Transition Changes ##
No example file or transition changes are envisioned as a result of this new feature.

## References ##
n/a
