CROSS PLATFORM epJSON EDITOR ENHANCEMENTS
================

**Jason Glazer, GARD Analytics**

 - December 20, 2024


## Justification for New Feature ##

The cross-platform epJSON Editor developed in 2020-2021 used Python and the
wxPython library, and was intended to be the replacement for the Windows-only IDF
Editor that uses Visual Basic. Unfortunately, the wxPython library could not be
effectively integrated with the EnergyPlus installer. Also, the existence of the
epJSON Editor was not widely publicized, and currently most users still use the
IDF format. For these reasons, few users have used epJSON Editor. To solve
these problems, epJSON Editor will be refactored to use the Tkinter library
instead of wxPython, which is already part of the EnergyPlus installer. It will
also support opening and saving the IDF format using convertInputFormat. These
enhancements, along with some public notifications, should result in more users
using epJSON Editor and, ultimately, the epJSON format.


## E-mail and Conference Call Conclusions ##

none

## Overview ##

A replacement for the popular Windows-only IDF Editor using a modern programming
language was previously developed called "epJSON Editor." The epJSON Editor
focused on use with the JSON-based epJSON format as an alternative to the IDF
format and used Python and a cross-platform GUI library so that it could be used
on Windows, MacOS, and Linux. It was developed but not widely tested or
publicized, and few users of EnergyPus have tried it even though it includes
useful additional functionality compared to IDF Editor. GARD in conjunction with
the NREL team, will update the epJSON Editor to use the EnergyPlus team's
preferred GUI library called Tkinter instead of the wxPython library. To ease
the transition to the new epJSON format, it is also expected that we will enable
opening IDF files by using the existing convertInputFormat utility. Two small
enhancements to convertInputFormat will be made to help facilitate user
migration to that format and preserve object order and comments provided by the
user, and to add field names. GARD will reach out to users for beta-testing, and
the code will be updated in response to problems and suggestions provided by the
testers, as well as known issues previously identified. A final version of the
release will be included in the EnergyPlus installation packages for each
platform.

The current repository for the project is located here:

https://github.com/ORNL-BTRIC/epJSON-Editor

A list of feature suggestions is being maintained here:

https://github.com/ORNL-BTRIC/epJSON-Editor/issues/15

The origin for editor came from this issue:

https://github.com/NatLabRockies/EnergyPlus/issues/7418

That issue also includes a summary of a meeting with users of IDF Editor and what features they found most important.

Two issues related to convertInputFormat are planned to be addressed as part of this work:

Populate comments (!-) from IDD/schema when converting from epJSON to IDF

https://github.com/NatLabRockies/EnergyPlus/issues/8987

Add ConvertInputFormat options to preserve object order and comments to facilitate round-tripping

https://github.com/NatLabRockies/EnergyPlus/issues/8969

One widget that is not included in Tkinter that will be used is Tksheet

https://pypi.org/project/tksheet/

https://github.com/ragardner/tksheet


## Approach ##

The following steps are expected as part of this development effort:

-	Update the repository to Python 3.12 (or later)
- Create a mock-up of epJSON Editor using Tkinter widgets
-	Refactor existing code to minimize non-GUI code in files using wxPython
-	Remove wxPython and replace with Tkinter based on mock-up
-	Allow multiple open files
-	Possibly replace icons with better ones on hand
-	Test to ensure features are consistent
-	Fix known bugs
-	Add additional unit test coverage
-	Enable opening IDF file either by modifying convertInputFormat to save and use saved user comments and input
 object order and include field names or else create a new Python library to perform these functions
-	Create installers for all types of user groups including part of EnergyPlus installer, pip, and Windows installer
-	Encourage beta testing on all platforms
-	Test by opening a variety of files including large user files
-	Maybe use automated testing for GUI
-	Fix issues found during beta testing
-	Possibly add new features
-	Release testing and make final installers
-	Publicize more widely
-	Maybe add pop-up screen to IDF Editor to encourage the use of epJSON Editor


## Testing/Validation/Data Sources ##

Will add additional unit tests. May add GUI unit tests.

## Input Output Reference Documentation ##

None expected.

## Input Description ##

N/A

## Outputs Description ##

N/A

## Engineering Reference ##

None

## Example File and Transition Changes ##

N/A

## References ##

None.
