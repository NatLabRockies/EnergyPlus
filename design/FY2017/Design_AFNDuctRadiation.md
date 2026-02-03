Air Duct Radiation Heat Transfer
===============

**Anthony Fontanini, Matt Mitchell, and Jan Kosny: Fraunhofer CSE**

- December 6, 2016

## New Feature Proposal
Details on the justification, overview, and IDD changes of the project are provided in the NFP.

## Overview

The current source files that are related to the following objects:

- AirflowNetwork:Distribution:Component:Duct
- AirflowNetwork:Distribution:Linkage

are primarily:

- AirflowNetworkSolver.cc
- AirflowNetworkBalanceManager.cc
- DataAirflowNetwork.cc

Most of the data structures will remain unchanged. New data structures will be added for the new object, and a few others for surfaces, ducts, and linkages will be modified to accommodate the duct radiation and user view factor changes. Little effort is expected to be given to refactorization. Changes and additions to data structures, and code changes are outlined below.

## Approach

### Modifications to DataAirFlowNetwork.hh

Modify the ```DisSysLinkageProp``` struct by adding the following:

``` C++
struct DisSysLinkageProp // Distribution system linkage data
{
	// New Member
	int viewFactorID;

	// Default Constructor
	AirflowNetworkLinkage() :
		viewFactorID(0)
	{}
};
```

Modify the ```AirflowNetworkExchangeProp``` struct by adding the following:

```C++
// New member
Real64 DuctRadEnergy;

// Default constructor
AirflowNetworkExchangeProp() :
	DuctRadEnergy(0.0)
{}
```

Add the ```AirflowNetworkLinkageViewFactorProp``` and ```LinkageSurfaceData``` structs as follows:

```C++
struct LinkageSurfaceProp
{
	// Members
	std::string surfaceName;
	int surfaceID;
	Real64 viewFactor;

	// Default Constructor
	LinkageSurfaceProp() :
		surfaceID(0),
		viewFactor(0)
	{}
};

struct AirflowNetworkLinkageViewFactorProp
{
	// Members
	std::string name;
	Real64 surfaceExposureFraction;
	Real64 surfaceEmittance;
	Array1D< LinkageSurfaceProp > linkageSurfaceData;

	// Default Constructor
	AirflowNetworkLinkageViewFactorProp() :
		surfaceExposureFraction(0),
		surfaceEmittance(0)
	{}
};
```

It appears that the nomenclature throughout AFN has been to name the struct with the ```Prop``` suffix, and the instance with the ```Data``` suffix. This was replicated in these changes.

### Modifications to DataAirFlowNetwork.cc

Add the ```CompTypeNum``` as follows:

```C++
// ~ Line 107
int const CompTypeNum_DVF(21); // Distribution system linkage view factor
```

Add an array of ```AirflowNetworkLinkageViewFactorData```, and in the ```clear_state()``` as follows:


``` C++
// ~ Line 244
Array1D< AirflowNetworkLinkageViewFactorProp > AirflowNetworkLinkageViewFactorData;

// ~ Line 315
AirflowNetworkLinkageViewFactorData.deallocate();
```

### Modifications to AirFlowNetworkBalanceManger.cc

Add to ```clear_state()``` the following:

```C++
// ~ Line 338
DisSysNumOfDuctViewFactors = 0;
```

Add to ```GetAirflowNetworkInput()``` the following:

```C++
// ~ Line 679
GetObjectDefMaxArgs( "AirflowNetwork:Distribution:DuctViewFactors", TotalArgs, NumAlphas, NumNumbers );
MaxNums = max( MaxNums, NumNumbers );
MaxAlphas = max( MaxAlphas, NumAlphas );

// ~ Line 2750
// Read AirflowNetwork distribution system component: DuctViewFactors
CurrentModuleObject = "AirflowNetwork:Distribution:DuctViewFactors";
DisSysNumOfDuctViewFactors = GetNumObjectsFound( CurrentModuleObject );
if ( DisSysNumOfDuctViewFactors > 0 ) {
	AirflowNetworkLinkageViewFactorData.allocate( DisSysNumOfDuctViewFactors );
	for ( i = 1; i <= DisSysNumOfDucts; ++i ) {
		GetObjectItem( CurrentModuleObject, i, Alphas, NumAlphas, Numbers, NumNumbers, IOStatus, lNumericBlanks, lAlphaBlanks, cAlphaFields, cNumericFields );
		IsNotOK = false;
		IsBlank = false;
		VerifyName( Alphas( 1 ), DisSysCompDuctData, i - 1, IsNotOK, IsBlank, CurrentModuleObject + " Name" );
		if ( IsNotOK ) {
			ErrorsFound = true;
			if ( IsBlank ) Alphas( 1 ) = "xxxxx";
		}
		AirFlowNetworkLinkageViewFactorData( i ).name = Alphas( 1 ); // Name duct view factor component
		AirFlowNetworkLinkageViewFactorData( i ).surfaceExposureFraction = Numbers( 1 ); // Surface exposure fraction []
		AirFlowNetworkLinkageViewFactorData( i ).surfaceEmittance = Numbers( 2 ); // Duct surface emittance

		numSurfaces = NumAlphas - 1;
		AirflowNetworkLinkageViewFactorData( i ).linkageSurfaceData.allocate(numSurfaces)
		for (int i = 0; i < numAlphas; ++i)
		{
			AirFlowNetworkLinkageViewFactorData( i ).surfaceName = Alphas( i + 1 ); // Surface name
			AirFlowNetworkLinkageViewFactorData( i ).surfaceNum = FindItemInList( Alphas( i + 1 ), Surface );
			AirFlowNetworkLinkageViewFactorData( i ).viewFactor = Numbers( i + 2 ); // Surface view factor
		}
	}
}

// ~ Line 3749
if ( AirflowNetworkCompData( i ).CompTypeNum == CompTypeNum_DVF ) CompName( 1 ) = "AirflowNetwork:Distribution:DuctViewFactors";
```

Add to ```AllocateAndInitData()``` the following:

```C++
// Radiation losses due to the forced air systems
SetupOutputVariable( "AFN Zone Duct Radiation Heat Gain Rate [W]", AirflowNetworkReportData( i ).RadGainW, "System", "Average", Zone( i ).Name );
SetupOutputVariable( "AFN Zone Duct Radiation Sensible Heat Gain Energy [J]", AirflowNetworkReportData( i ).RadGainJ, "System", "Sum", Zone( i ).Name );
SetupOutputVariable( "AFN Zone Duct Radiation Heat Loss Rate [W]", AirflowNetworkReportData( i ).RadLossW, "System", "Average", Zone( i ).Name );
SetupOutputVariable( "AFN Zone Duct Radiation Sensible Heat Loss Energy [J]", AirflowNetworkReportData( i ).RadLossJ, "System", "Sum", Zone( i ).Name );
```

Add to ```ReportAirflowNetwork()``` the following:

```C++
if ( AirflowNetworkExchangeData( i ).RadGain > 0.0 ) {
	AirflowNetworkReportData( i ).RadGainW = AirflowNetworkExchangeData( i ).RadGain;
	AirflowNetworkReportData( i ).RadGainJ = AirflowNetworkExchangeData( i ).RadGain * ReportingConstant;
} else {
	AirflowNetworkReportData( i ).RadLossW = -AirflowNetworkExchangeData( i ).RadGain;
	AirflowNetworkReportData( i ).RadLossJ = -AirflowNetworkExchangeData( i ).RadGain * ReportingConstant;
}
```

Add to ```UpdateAirFlowNetwork()``` the following:

```C++
// ~ Line 6973
AirflowNetworkExchangeData( i ).TotalSen = AirflowNetworkExchangeData( i ).LeakSen + AirflowNetworkExchangeData( i ).CondSen + AirflowNetworkExchangeData( i ).RadGain;

// ~ Line 6984
AirflowNetworkExchangeData( i ).RadGain *= OnOffFanRunTimeFraction;
```

Add to ```ManageAirflowNetworkBalance()``` the following:

```C++
// ~ Line 493
CalcAirflowNetworkRadiation();
```

Currenty, it anticpated that this will be called before the heat and moisture balance's are calculated. The function will stand alone and be capable of being called by either of the other heat balance methods if deemed necessary. This will require some futher investigation to determine what works best in practice.

Add ```CalcAirflowNetworkRadiation()```:

```C++
for ( i = 1; i <= AirflowNetworkNumOfLinks; ++i ) {
	CompNum = AirflowNetworkLinkageData( i ).CompNum;
	CompTypeNum = AirflowNetworkCompData( CompNum ).CompTypeNum;
	CompName = AirflowNetworkCompData( CompNum ).EPlusName;
	// Calculate duct radiation
	if ( CompTypeNum == CompTypeNum_DWC && CompName == BlankString ) { // Duct element only
		// Duct radiation calcs here
	}
}
```


### Modifications to DataSurfaceHeatBalance.cc

This will follow the pattern set out by objects such as the ```ZoneHVAC:Baseboard:RadiantConvective:Electric``` object for ducts to place heat gains/losses on the interior surface heat balance.


## OOP/Refactorization
Since the new duct view factor object described here and in the NFP are intended as a *first* step at evaluating the effects of duct radiation, no major refactorization will occur during this phase of the project. If in subsequent phases duct radiation view factors are to be calculated automatically by EnergyPlus, refactorization of the surface heat balance and airflow network will be further evaluated. Converting some of the affected list of component types, as shown [here](https://github.com/NatLabRockies/EnergyPlus/blob/ea65c6e0c28a9e40c4846062fadfbf198ede5681/src/EnergyPlus/DataAirflowNetwork.cc#L87) to ```enum``` types could be tasked given team support, though little effort for code refactorization is expected.

Lots of spelling errors and end-of-line whitespace--correct those at a minimum.
