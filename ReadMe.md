# Occupant Behavior Functional Mock-up Unit (obFMU)
Traditionally, in building energy modeling (BEM) programs, occupant behavior (OB) related inputs are oversimplified and not indicative of real world scenarios, contributing to discrepancies between the simulated and actual energy use in buildings. This project focuses on the development of an OB modeling tool, obFMU, to allow for more detailed and stochastic OB modeling. obFMU is a Functional Mock-up Unit enabling co-simulation with BEM programs implementing Functional Mock-up Interface. This enables more accurate assessment of the impact of occupant behavior on building energy consumption. 


# Installation Guide:
##1. install EnergyPlus-8.3.0-6d97d074ea-Windows-i386.exe to the default folder C:\EnergyPlusV8-3-0
##2. copy the USA_FL_Miami.Intl.AP.722020_TMY3.epw to C:\EnergyPlusV8-3-0\WeatherData\
##3. open C:\EnergyPlusV8-3-0\EP-Launch.exe, set the input file to Medium Office TH.idf, set the weather file to USA_FL_Miami.Intl.AP.722020_TMY3.epw.
##4. Click Simulate.. on EP-Launch.exe
##5. See the results under the same folder
##6. Each file end up with "<obfmu instance name>.csv" is the result file for the associated room. 
##7. Each file end up with "<obfmu instance name>_log.csv" is the log file for the associated room. It shows all the decisions for each time step.
##8. The obMovement_byOccupant.csv is the movment result file for all the rooms.

##9. To use EnergyPlus.exe directly to run the obfmu
###- Create a folder e.g. D:\obFMU_simulation
###- Add the obXML.xml, obCoSim.xml into the foler D:\obFMU_simulation
###- Create a sub-folder e.g. D:\obFMU_simulation\EnergyPlus\
###- Add in.idf, in.epw, obFMU.fmu, Energy+.idd into the sub-folder: D:\obFMU_simulation\EnergyPlus\
###- run CMD, and move to the sub-folder: D:\obFMU_simulation\EnergyPlus\ and run EnergyPlus.exe


# Version History
## Version 0.1
The working version based on Hongsan Sun's code.

## Version 0.2
Create the set of classes based on obXML Schema.
Read the obXML

## Version 0.3
Include the interaction solver

## Version 0.4
Integrate the interaction solver with the movement solver and add them into the ObmFmu classes

## Version 0.5
Add outdoor air dry-bulb temperature into the inputs list
Change the solution form ObmFmu to obFMU

## Version 0.6
	Include the Behaviors only build for Linux
	add the option to enable or disable movment calculation. If the movement calculation is disabled, the user need to provide the movement calculation result file.
	Update the obXML file to match the obXML schema
	Add progresses to the movement calculation

# TODO List
## Overall
### Clean the code, remove the SQLite and boost library
### Add comments and read me files
### Make sure the code is compatible with obXML Schema, no hard code efficiency or parameters

## Occupant Behaviour Functional Mock-up Unit

## Occupant Movement EXE

## The lighting control calculation
### Currently, input 1 is zone illuminance in lux. The input 3 is lighting power in W. The power effect (light effect) is in lm/W.
### The lighting power should be replaced by lighting power density (W/m2) instead.
#### Code location: ObmFmu.cpp (line 204 -218)

## The occupant calculation
### Currently, it is simply turn off the HVAC and lighting system if the room is not occupants.
#### Code location: ObmFMU.cpp (line 247-250)

## The fmiValueReference
### EnergyPlus don't provide the reference ID as expected.
### Currently, the inputs and outputs in EnergyPlus IDF file need to have the same order of the model description file.

## Check the hard code parameters in obm solver
### Current found the zone names are hard coded instead of reading the CoSimParas.xml