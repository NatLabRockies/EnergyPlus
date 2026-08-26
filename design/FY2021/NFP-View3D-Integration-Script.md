# View3D Integration Scripts #

Jason DeGraw, Niraj Kunwar
Oak Ridge National Laboratory

Original Date: February 12, 2021

## Justification for New Feature ##
The View3D program that ships with EnergyPlus allows users to compute much more accurate view factors but use of the tool is not well automated.
EnergyPlus is shipping with Python scripts that do not come with the same security risks associated with the macro driven spreadsheets that are
currently shipped. Similar functionality can be obtained with Python scripts that extract the relevant information from the inputs and outputs of
EnergyPlus and View3D to get what EnergyPlus needs to use the externally calculated view factors.

## Overview ##
A Python script will be developed to read in the EnergyPlus epJSON representation of a model and the generate the appropriate View3D input file.
Optionally, the script will execute View3D to compute the factors and insert the factors into the epJSON.

## Approach ##
The Python script will be written as script that can also be used as a module. Only features of the Python standard library will be used in order
to preserve maximum portability. Geometry rules will be respected by the script.

## Testing/Validation/Data Sources ##

N/A

## Reference Documentation ##

Appropriate documentation of the script will be provided, potentially in the auxiliary programs documentation. 

## Input Description ##

The script will be used as follows:

`$ python generateView3D.py <options> FILE`

where `FILE` is the epJSON to be processed. By default, if the script is asked to add the view factors into the model, the original
file will be overwritten. Options will include

`-z ZONE` or `-–zone ZONE`: process zone `ZONE`

`-r` or `--run`: Run View3D to generate view factors and insert values into the epJSON file

`-o OUTFILE` or `-–output OUTFILE`: Write epJSON output to `OUTFILE` so that the original file is left unchanged

Additional options will be added as needed.

## Outputs Description ##

N/A

## Engineering Reference ##

N/A

## Example File and Transition Changes ##

N/A

## References ##

N/A
