// EnergyPlus, Copyright (c) 1996-2025, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// C++ Headers
#include <cmath>
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataGenerators.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneratorFuelSupply.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace GeneratorFuelSupply {

    //_______________________________________________
    // Utility modules used by other generators.
    //
    // GeneratorFuelSupply
    //   reused among some generators to define gaseous fuel chemistry, optional compressor)

    // Module containing the routines dealing with the fuel supply for some generators
    // different generator modules can reuse the same fuel supply code, hence a separate module

    // MODULE INFORMATION:
    //       AUTHOR         B Griffith
    //       DATE WRITTEN   July 2006

    // METHODOLOGY EMPLOYED:
    // data defined in DataGenerators.cc
    // this module only provides input and subroutines for other component simulations
    //  no specific energyplus component is modeled here.  it is used by other generators

    // REFERENCES:
    // Annex 42 documentation

    void GetGeneratorFuelSupplyInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B Griffith
        //       DATE WRITTEN   July 2006,
        //       RE-ENGINEERED  this module extracted from older SOFC module for
        //                      reuse with both Annex 42 models,

        static constexpr std::string_view routineName = "GetGeneratorFuelSupplyInput";
        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        //  INTEGER                     :: GeneratorNum !Generator counter
        Array1D_string AlphArray(25);  // character string data
        Array1D<Real64> NumArray(200); // numeric data TODO deal with allocatable for extensible

        if (state.dataGeneratorFuelSupply->MyOneTimeFlag) {
            int NumAlphas; // Number of elements in the alpha array
            int NumNums;   // Number of elements in the numeric array
            int IOStat;    // IO Status when calling get input subroutine
            bool ErrorsFound = false;
            std::string const cCurrentModuleObject = "Generator:FuelSupply";
            int NumGeneratorFuelSups = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

            if (NumGeneratorFuelSups <= 0) {
                ShowSevereError(state, format("No {} equipment specified in input file", cCurrentModuleObject));
                ErrorsFound = true;
            }

            state.dataGenerator->FuelSupply.allocate(NumGeneratorFuelSups);

            for (int FuelSupNum = 1; FuelSupNum <= NumGeneratorFuelSups; ++FuelSupNum) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         FuelSupNum,
                                                                         AlphArray,
                                                                         NumAlphas,
                                                                         NumArray,
                                                                         NumNums,
                                                                         IOStat,
                                                                         _,
                                                                         _,
                                                                         state.dataIPShortCut->cAlphaFieldNames,
                                                                         state.dataIPShortCut->cNumericFieldNames);

                ErrorObjectHeader eoh{routineName, cCurrentModuleObject, AlphArray(1)};
                state.dataGenerator->FuelSupply(FuelSupNum).Name = AlphArray(1);
                if (Util::SameString("TemperatureFromAirNode", AlphArray(2))) {
                    state.dataGenerator->FuelSupply(FuelSupNum).FuelTempMode = DataGenerators::FuelTemperatureMode::FuelInTempFromNode;
                } else if (Util::SameString("Scheduled", AlphArray(2))) {
                    state.dataGenerator->FuelSupply(FuelSupNum).FuelTempMode = DataGenerators::FuelTemperatureMode::FuelInTempSchedule;
                } else {
                    ShowSevereError(state, format("Invalid, {} = {}", state.dataIPShortCut->cAlphaFieldNames(2), AlphArray(2)));
                    ShowContinueError(state, format("Entered in {}={}", cCurrentModuleObject, AlphArray(1)));
                    ErrorsFound = true;
                }

                state.dataGenerator->FuelSupply(FuelSupNum).NodeName = AlphArray(3);
                state.dataGenerator->FuelSupply(FuelSupNum).NodeNum =
                    NodeInputManager::GetOnlySingleNode(state,
                                                        AlphArray(3),
                                                        ErrorsFound,
                                                        DataLoopNode::ConnectionObjectType::GeneratorFuelSupply,
                                                        AlphArray(1),
                                                        DataLoopNode::NodeFluidType::Air,
                                                        DataLoopNode::ConnectionType::Sensor,
                                                        NodeInputManager::CompFluidStream::Primary,
                                                        DataLoopNode::ObjectIsNotParent);

                if (state.dataGenerator->FuelSupply(FuelSupNum).FuelTempMode == DataGenerators::FuelTemperatureMode::FuelInTempSchedule) {
                    if ((state.dataGenerator->FuelSupply(FuelSupNum).sched = Sched::GetSchedule(state, AlphArray(4))) == nullptr) {
                        ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(4), AlphArray(4));
                        ErrorsFound = true;
                    }
                }

                state.dataGenerator->FuelSupply(FuelSupNum).CompPowerCurveID = Curve::GetCurveIndex(state, AlphArray(5));
                if (state.dataGenerator->FuelSupply(FuelSupNum).CompPowerCurveID == 0) {
                    ShowSevereError(state, format("Invalid, {} = {}", state.dataIPShortCut->cAlphaFieldNames(5), AlphArray(5)));
                    ShowContinueError(state, format("Entered in {}={}", cCurrentModuleObject, AlphArray(1)));
                    ShowContinueError(state, "Curve named was not found ");
                    ErrorsFound = true;
                }

                for (auto &e : state.dataGenerator->FuelSupply) {
                    e.CompPowerLossFactor = NumArray(1);
                }

                if (Util::SameString(AlphArray(6), "GaseousConstituents")) {
                    state.dataGenerator->FuelSupply(FuelSupNum).FuelTypeMode = DataGenerators::FuelMode::GaseousConstituents;
                } else if (Util::SameString(AlphArray(6), "LiquidGeneric")) {
                    state.dataGenerator->FuelSupply(FuelSupNum).FuelTypeMode = DataGenerators::FuelMode::GenericLiquid;
                } else {
                    ShowSevereError(state, format("Invalid, {} = {}", state.dataIPShortCut->cAlphaFieldNames(6), AlphArray(6)));
                    ShowContinueError(state, format("Entered in {}={}", cCurrentModuleObject, AlphArray(1)));
                    ErrorsFound = true;
                }

                state.dataGenerator->FuelSupply(FuelSupNum).LHVliquid = NumArray(2) * 1000.0; // generic liquid LHV  (kJ/kG input converted to J/kG )
                state.dataGenerator->FuelSupply(FuelSupNum).HHV = NumArray(3) * 1000.0;       // generic liquid HHV (kJ/kG input converted to J/kG )
                state.dataGenerator->FuelSupply(FuelSupNum).MW = NumArray(4);
                state.dataGenerator->FuelSupply(FuelSupNum).eCO2 = NumArray(5);

                if (state.dataGenerator->FuelSupply(FuelSupNum).FuelTypeMode == DataGenerators::FuelMode::GaseousConstituents) {
                    int NumFuelConstit = NumArray(6);
                    state.dataGenerator->FuelSupply(FuelSupNum).NumConstituents = NumFuelConstit;

                    if (NumFuelConstit > 12) {
                        ShowSevereError(state, format("{} model not set up for more than 12 fuel constituents", cCurrentModuleObject));
                        ErrorsFound = true;
                    }
                    if (NumFuelConstit < 1) {
                        ShowSevereError(state, format("{} model needs at least one fuel constituent", cCurrentModuleObject));
                        ErrorsFound = true;
                    }

                    for (int ConstitNum = 1; ConstitNum <= NumFuelConstit; ++ConstitNum) {
                        state.dataGenerator->FuelSupply(FuelSupNum).ConstitName(ConstitNum) = AlphArray(ConstitNum + 6);
                        state.dataGenerator->FuelSupply(FuelSupNum).ConstitMolalFract(ConstitNum) = NumArray(ConstitNum + 6);
                    }

                    // check for molar fractions summing to 1.0.
                    if (std::abs(sum(state.dataGenerator->FuelSupply(FuelSupNum).ConstitMolalFract) - 1.0) > 0.0001) {
                        ShowSevereError(state, format("{} molar fractions do not sum to 1.0", cCurrentModuleObject));
                        ShowContinueError(state, format("Sum was={:.5R}", sum(state.dataGenerator->FuelSupply(FuelSupNum).ConstitMolalFract)));
                        ShowContinueError(state, format("Entered in {} = {}", cCurrentModuleObject, AlphArray(1)));
                        ErrorsFound = true;
                    }
                }
            }

            // now make calls to Setup

            for (int FuelSupNum = 1; FuelSupNum <= NumGeneratorFuelSups; ++FuelSupNum) {
                SetupFuelConstituentData(state, FuelSupNum, ErrorsFound);
            }

            if (ErrorsFound) {
                ShowFatalError(state, format("Problem found processing input for {}", cCurrentModuleObject));
            }

            state.dataGeneratorFuelSupply->MyOneTimeFlag = false;
        } // MyOneTimeFlag
    }

    //******************************************************************************

    void SetupFuelConstituentData(EnergyPlusData &state, int const FuelSupplyNum, bool &ErrorsFound)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B Griffith
        //       DATE WRITTEN   Aug 2005,
        //       RE-ENGINEERED  July/Aug 2006, extracted to own module. added liquid fuel option

        // PURPOSE OF THIS SUBROUTINE:
        // Fill data structure for gas phase thermochemistry

        // METHODOLOGY EMPLOYED:
        // Hardcoded data from NIST is filled into data structure one time only

        int constexpr NumHardCodedConstituents = 14; // number of gases included in data

        bool first_time = false;
        if (!allocated(state.dataGenerator->GasPhaseThermoChemistryData)) {
            state.dataGenerator->GasPhaseThermoChemistryData.allocate(NumHardCodedConstituents);
            first_time = true;
        }
        // Carbon Dioxide (CO2) Temp K 298-1200 (Chase 1998)
        state.dataGenerator->GasPhaseThermoChemistryData(1).ConstituentName = "CarbonDioxide";
        state.dataGenerator->GasPhaseThermoChemistryData(1).ConstituentFormula = "CO2";
        state.dataGenerator->GasPhaseThermoChemistryData(1).StdRefMolarEnthOfForm = -393.5224; // KJ/mol
        state.dataGenerator->GasPhaseThermoChemistryData(1).ThermoMode = DataGenerators::ThermodynamicMode::NISTShomate;
        state.dataGenerator->GasPhaseThermoChemistryData(1).ShomateA = 24.99735;
        state.dataGenerator->GasPhaseThermoChemistryData(1).ShomateB = 55.18696;
        state.dataGenerator->GasPhaseThermoChemistryData(1).ShomateC = -33.69137;
        state.dataGenerator->GasPhaseThermoChemistryData(1).ShomateD = 7.948387;
        state.dataGenerator->GasPhaseThermoChemistryData(1).ShomateE = -0.136638;
        state.dataGenerator->GasPhaseThermoChemistryData(1).ShomateF = -403.6075;
        state.dataGenerator->GasPhaseThermoChemistryData(1).ShomateG = 228.2431;
        state.dataGenerator->GasPhaseThermoChemistryData(1).ShomateH = -393.5224;
        state.dataGenerator->GasPhaseThermoChemistryData(1).NumCarbons = 1.0;
        state.dataGenerator->GasPhaseThermoChemistryData(1).NumHydrogens = 0.0;
        state.dataGenerator->GasPhaseThermoChemistryData(1).NumOxygens = 2.0;
        state.dataGenerator->GasPhaseThermoChemistryData(1).MolecularWeight = 44.01;

        // Nitrogen (N2) Temp (K) 298-6000
        state.dataGenerator->GasPhaseThermoChemistryData(2).ConstituentName = "Nitrogen";
        state.dataGenerator->GasPhaseThermoChemistryData(2).ConstituentFormula = "N2";
        state.dataGenerator->GasPhaseThermoChemistryData(2).StdRefMolarEnthOfForm = 0.0;
        state.dataGenerator->GasPhaseThermoChemistryData(2).ThermoMode = DataGenerators::ThermodynamicMode::NISTShomate;
        state.dataGenerator->GasPhaseThermoChemistryData(2).ShomateA = 26.092;
        state.dataGenerator->GasPhaseThermoChemistryData(2).ShomateB = 8.218801;
        state.dataGenerator->GasPhaseThermoChemistryData(2).ShomateC = -1.976141;
        state.dataGenerator->GasPhaseThermoChemistryData(2).ShomateD = 0.159274;
        state.dataGenerator->GasPhaseThermoChemistryData(2).ShomateE = 0.044434;
        state.dataGenerator->GasPhaseThermoChemistryData(2).ShomateF = -7.98923;
        state.dataGenerator->GasPhaseThermoChemistryData(2).ShomateG = 221.02;
        state.dataGenerator->GasPhaseThermoChemistryData(2).ShomateH = 0.000;
        state.dataGenerator->GasPhaseThermoChemistryData(2).NumCarbons = 0.0;
        state.dataGenerator->GasPhaseThermoChemistryData(2).NumHydrogens = 0.0;
        state.dataGenerator->GasPhaseThermoChemistryData(2).NumOxygens = 0.0;
        state.dataGenerator->GasPhaseThermoChemistryData(2).MolecularWeight = 28.01;

        // Oxygen (O2) Temp (K) 298-6000
        state.dataGenerator->GasPhaseThermoChemistryData(3).ConstituentName = "Oxygen";
        state.dataGenerator->GasPhaseThermoChemistryData(3).ConstituentFormula = "O2";
        state.dataGenerator->GasPhaseThermoChemistryData(3).StdRefMolarEnthOfForm = 0.0;
        state.dataGenerator->GasPhaseThermoChemistryData(3).ThermoMode = DataGenerators::ThermodynamicMode::NISTShomate;
        state.dataGenerator->GasPhaseThermoChemistryData(3).ShomateA = 29.659;
        state.dataGenerator->GasPhaseThermoChemistryData(3).ShomateB = 6.137261;
        state.dataGenerator->GasPhaseThermoChemistryData(3).ShomateC = -1.186521;
        state.dataGenerator->GasPhaseThermoChemistryData(3).ShomateD = 0.095780;
        state.dataGenerator->GasPhaseThermoChemistryData(3).ShomateE = -0.219663;
        state.dataGenerator->GasPhaseThermoChemistryData(3).ShomateF = -9.861391;
        state.dataGenerator->GasPhaseThermoChemistryData(3).ShomateG = 237.948;
        state.dataGenerator->GasPhaseThermoChemistryData(3).ShomateH = 0.0;
        state.dataGenerator->GasPhaseThermoChemistryData(3).NumCarbons = 0.0;
        state.dataGenerator->GasPhaseThermoChemistryData(3).NumHydrogens = 0.0;
        state.dataGenerator->GasPhaseThermoChemistryData(3).NumOxygens = 2.0;
        state.dataGenerator->GasPhaseThermoChemistryData(3).MolecularWeight = 32.00;

        // Water (H2O) Temp K 300-1700
        // need lower temperature range for Shomate coef for Water Vapor..
        state.dataGenerator->GasPhaseThermoChemistryData(4).ConstituentName = "Water";
        state.dataGenerator->GasPhaseThermoChemistryData(4).ConstituentFormula = "H2O";
        state.dataGenerator->GasPhaseThermoChemistryData(4).StdRefMolarEnthOfForm = -241.8264; // KJ/mol
        state.dataGenerator->GasPhaseThermoChemistryData(4).ThermoMode = DataGenerators::ThermodynamicMode::NISTShomate;
        state.dataGenerator->GasPhaseThermoChemistryData(4).ShomateA = 29.0373;
        state.dataGenerator->GasPhaseThermoChemistryData(4).ShomateB = 10.2573;
        state.dataGenerator->GasPhaseThermoChemistryData(4).ShomateC = 2.81048;
        state.dataGenerator->GasPhaseThermoChemistryData(4).ShomateD = -0.95914;
        state.dataGenerator->GasPhaseThermoChemistryData(4).ShomateE = 0.11725;
        state.dataGenerator->GasPhaseThermoChemistryData(4).ShomateF = -250.569;
        state.dataGenerator->GasPhaseThermoChemistryData(4).ShomateG = 223.3967;
        state.dataGenerator->GasPhaseThermoChemistryData(4).ShomateH = -241.8264;
        state.dataGenerator->GasPhaseThermoChemistryData(4).NumCarbons = 0.0;
        state.dataGenerator->GasPhaseThermoChemistryData(4).NumHydrogens = 2.0;
        state.dataGenerator->GasPhaseThermoChemistryData(4).NumOxygens = 1.0;
        state.dataGenerator->GasPhaseThermoChemistryData(4).MolecularWeight = 18.02;

        // Argon (Ar)  Temp K 298-600

        state.dataGenerator->GasPhaseThermoChemistryData(5).ConstituentName = "Argon";
        state.dataGenerator->GasPhaseThermoChemistryData(5).ConstituentFormula = "Ar";
        state.dataGenerator->GasPhaseThermoChemistryData(5).StdRefMolarEnthOfForm = 0.0;
        state.dataGenerator->GasPhaseThermoChemistryData(5).ThermoMode = DataGenerators::ThermodynamicMode::NISTShomate;
        state.dataGenerator->GasPhaseThermoChemistryData(5).ShomateA = 20.786;
        state.dataGenerator->GasPhaseThermoChemistryData(5).ShomateB = 2.825911e-07;
        state.dataGenerator->GasPhaseThermoChemistryData(5).ShomateC = -1.464191e-07;
        state.dataGenerator->GasPhaseThermoChemistryData(5).ShomateD = 1.092131e-08;
        state.dataGenerator->GasPhaseThermoChemistryData(5).ShomateE = -3.661371e-08;
        state.dataGenerator->GasPhaseThermoChemistryData(5).ShomateF = -6.19735;
        state.dataGenerator->GasPhaseThermoChemistryData(5).ShomateG = 179.999;
        state.dataGenerator->GasPhaseThermoChemistryData(5).ShomateH = 0.0;
        state.dataGenerator->GasPhaseThermoChemistryData(5).NumCarbons = 0.0;
        state.dataGenerator->GasPhaseThermoChemistryData(5).NumHydrogens = 0.0;
        state.dataGenerator->GasPhaseThermoChemistryData(5).NumOxygens = 0.0;
        state.dataGenerator->GasPhaseThermoChemistryData(5).MolecularWeight = 39.95;

        // Hydrogen (H2) Temp K 298-1000
        state.dataGenerator->GasPhaseThermoChemistryData(6).ConstituentName = "Hydrogen";
        state.dataGenerator->GasPhaseThermoChemistryData(6).ConstituentFormula = "H2";
        state.dataGenerator->GasPhaseThermoChemistryData(6).StdRefMolarEnthOfForm = 0.0;
        state.dataGenerator->GasPhaseThermoChemistryData(6).ThermoMode = DataGenerators::ThermodynamicMode::NISTShomate;
        state.dataGenerator->GasPhaseThermoChemistryData(6).ShomateA = 33.066178;
        state.dataGenerator->GasPhaseThermoChemistryData(6).ShomateB = -11.363417;
        state.dataGenerator->GasPhaseThermoChemistryData(6).ShomateC = 11.432816;
        state.dataGenerator->GasPhaseThermoChemistryData(6).ShomateD = -2.772874;
        state.dataGenerator->GasPhaseThermoChemistryData(6).ShomateE = -0.158558;
        state.dataGenerator->GasPhaseThermoChemistryData(6).ShomateF = -9.980797;
        state.dataGenerator->GasPhaseThermoChemistryData(6).ShomateG = 172.707974;
        state.dataGenerator->GasPhaseThermoChemistryData(6).ShomateH = 0.0;
        state.dataGenerator->GasPhaseThermoChemistryData(6).NumCarbons = 0.0;
        state.dataGenerator->GasPhaseThermoChemistryData(6).NumHydrogens = 2.0;
        state.dataGenerator->GasPhaseThermoChemistryData(6).NumOxygens = 0.0;
        state.dataGenerator->GasPhaseThermoChemistryData(6).MolecularWeight = 2.02;

        // Methane (CH4) Temp K 298-1300
        state.dataGenerator->GasPhaseThermoChemistryData(7).ConstituentName = "Methane";
        state.dataGenerator->GasPhaseThermoChemistryData(7).ConstituentFormula = "CH4";
        state.dataGenerator->GasPhaseThermoChemistryData(7).StdRefMolarEnthOfForm = -74.8731; // KJ/mol (Chase 1998)
        state.dataGenerator->GasPhaseThermoChemistryData(7).ThermoMode = DataGenerators::ThermodynamicMode::NISTShomate;
        state.dataGenerator->GasPhaseThermoChemistryData(7).ShomateA = -0.703029;
        state.dataGenerator->GasPhaseThermoChemistryData(7).ShomateB = 108.4773;
        state.dataGenerator->GasPhaseThermoChemistryData(7).ShomateC = -42.52157;
        state.dataGenerator->GasPhaseThermoChemistryData(7).ShomateD = 5.862788;
        state.dataGenerator->GasPhaseThermoChemistryData(7).ShomateE = 0.678565;
        state.dataGenerator->GasPhaseThermoChemistryData(7).ShomateF = -76.84376;
        state.dataGenerator->GasPhaseThermoChemistryData(7).ShomateG = 158.7163;
        state.dataGenerator->GasPhaseThermoChemistryData(7).ShomateH = -74.87310;
        state.dataGenerator->GasPhaseThermoChemistryData(7).NumCarbons = 1.0;
        state.dataGenerator->GasPhaseThermoChemistryData(7).NumHydrogens = 4.0;
        state.dataGenerator->GasPhaseThermoChemistryData(7).NumOxygens = 0.0;
        state.dataGenerator->GasPhaseThermoChemistryData(7).MolecularWeight = 16.04;

        // Ethane (C2H6)
        state.dataGenerator->GasPhaseThermoChemistryData(8).ConstituentName = "Ethane";
        state.dataGenerator->GasPhaseThermoChemistryData(8).ConstituentFormula = "C2H6";
        state.dataGenerator->GasPhaseThermoChemistryData(8).StdRefMolarEnthOfForm = -83.8605; // -83.8 !KJ/mol (Pittam and Pilcher 1972)
        state.dataGenerator->GasPhaseThermoChemistryData(8).ThermoMode = DataGenerators::ThermodynamicMode::NISTShomate;
        state.dataGenerator->GasPhaseThermoChemistryData(8).ShomateA = -3.03849;
        state.dataGenerator->GasPhaseThermoChemistryData(8).ShomateB = 199.202;
        state.dataGenerator->GasPhaseThermoChemistryData(8).ShomateC = -84.9812;
        state.dataGenerator->GasPhaseThermoChemistryData(8).ShomateD = 11.0348;
        state.dataGenerator->GasPhaseThermoChemistryData(8).ShomateE = 0.30348;
        state.dataGenerator->GasPhaseThermoChemistryData(8).ShomateF = -90.0633;
        state.dataGenerator->GasPhaseThermoChemistryData(8).ShomateG = -999.0;
        state.dataGenerator->GasPhaseThermoChemistryData(8).ShomateH = -83.8605;
        state.dataGenerator->GasPhaseThermoChemistryData(8).NumCarbons = 2.0;
        state.dataGenerator->GasPhaseThermoChemistryData(8).NumHydrogens = 6.0;
        state.dataGenerator->GasPhaseThermoChemistryData(8).NumOxygens = 0.0;
        state.dataGenerator->GasPhaseThermoChemistryData(8).MolecularWeight = 30.07;
        state.dataGenerator->GasPhaseThermoChemistryData(8).NASA_A1 = 0.14625388e+01;
        state.dataGenerator->GasPhaseThermoChemistryData(8).NASA_A2 = 0.15494667e-01;
        state.dataGenerator->GasPhaseThermoChemistryData(8).NASA_A3 = 0.05780507e-04;
        state.dataGenerator->GasPhaseThermoChemistryData(8).NASA_A4 = -0.12578319e-07;
        state.dataGenerator->GasPhaseThermoChemistryData(8).NASA_A5 = 0.04586267e-10;
        state.dataGenerator->GasPhaseThermoChemistryData(8).NASA_A6 = -0.11239176e+05;
        state.dataGenerator->GasPhaseThermoChemistryData(8).NASA_A7 = 0.14432295e+02;

        // Propane (C3H8)
        state.dataGenerator->GasPhaseThermoChemistryData(9).ConstituentName = "Propane";
        state.dataGenerator->GasPhaseThermoChemistryData(9).ConstituentFormula = "C3H8";
        state.dataGenerator->GasPhaseThermoChemistryData(9).StdRefMolarEnthOfForm = -103.855; //  -104.7 !kJ/mol  (Pittam and Pilcher 1972)
        state.dataGenerator->GasPhaseThermoChemistryData(9).ThermoMode = DataGenerators::ThermodynamicMode::NISTShomate;
        state.dataGenerator->GasPhaseThermoChemistryData(9).ShomateA = -23.1747;
        state.dataGenerator->GasPhaseThermoChemistryData(9).ShomateB = 363.742;
        state.dataGenerator->GasPhaseThermoChemistryData(9).ShomateC = -222.981;
        state.dataGenerator->GasPhaseThermoChemistryData(9).ShomateD = 56.253;
        state.dataGenerator->GasPhaseThermoChemistryData(9).ShomateE = 0.61164;
        state.dataGenerator->GasPhaseThermoChemistryData(9).ShomateF = -109.206;
        state.dataGenerator->GasPhaseThermoChemistryData(9).ShomateG = -999.0;
        state.dataGenerator->GasPhaseThermoChemistryData(9).ShomateH = -103.855;
        state.dataGenerator->GasPhaseThermoChemistryData(9).NumCarbons = 3.0;
        state.dataGenerator->GasPhaseThermoChemistryData(9).NumHydrogens = 8.0;
        state.dataGenerator->GasPhaseThermoChemistryData(9).NumOxygens = 0.0;
        state.dataGenerator->GasPhaseThermoChemistryData(9).MolecularWeight = 44.10;
        state.dataGenerator->GasPhaseThermoChemistryData(9).NASA_A1 = 0.08969208e+01;
        state.dataGenerator->GasPhaseThermoChemistryData(9).NASA_A2 = 0.02668986e+00;
        state.dataGenerator->GasPhaseThermoChemistryData(9).NASA_A3 = 0.05431425e-04;
        state.dataGenerator->GasPhaseThermoChemistryData(9).NASA_A4 = -0.02126000e-06;
        state.dataGenerator->GasPhaseThermoChemistryData(9).NASA_A5 = 0.09243330e-10;
        state.dataGenerator->GasPhaseThermoChemistryData(9).NASA_A6 = -0.13954918e+05;
        state.dataGenerator->GasPhaseThermoChemistryData(9).NASA_A7 = 0.01935533e+03;

        // Butane (C4H10)
        state.dataGenerator->GasPhaseThermoChemistryData(10).ConstituentName = "Butane";
        state.dataGenerator->GasPhaseThermoChemistryData(10).ConstituentFormula = "C4H10";
        state.dataGenerator->GasPhaseThermoChemistryData(10).StdRefMolarEnthOfForm = -133.218; // -125.6 !kJ/mol  (Pittam and Pilcher 1972)
        state.dataGenerator->GasPhaseThermoChemistryData(10).ThermoMode = DataGenerators::ThermodynamicMode::NISTShomate;
        state.dataGenerator->GasPhaseThermoChemistryData(10).ShomateA = -5.24343;
        state.dataGenerator->GasPhaseThermoChemistryData(10).ShomateB = 426.442;
        state.dataGenerator->GasPhaseThermoChemistryData(10).ShomateC = -257.955;
        state.dataGenerator->GasPhaseThermoChemistryData(10).ShomateD = 66.535;
        state.dataGenerator->GasPhaseThermoChemistryData(10).ShomateE = -0.26994;
        state.dataGenerator->GasPhaseThermoChemistryData(10).ShomateF = -149.365;
        state.dataGenerator->GasPhaseThermoChemistryData(10).ShomateG = -999.0;
        state.dataGenerator->GasPhaseThermoChemistryData(10).ShomateH = -133.218;
        state.dataGenerator->GasPhaseThermoChemistryData(10).NumCarbons = 4.0;
        state.dataGenerator->GasPhaseThermoChemistryData(10).NumHydrogens = 10.0;
        state.dataGenerator->GasPhaseThermoChemistryData(10).NumOxygens = 0.0;
        state.dataGenerator->GasPhaseThermoChemistryData(10).MolecularWeight = 58.12;
        state.dataGenerator->GasPhaseThermoChemistryData(10).NASA_A1 = -0.02256618e+02;
        state.dataGenerator->GasPhaseThermoChemistryData(10).NASA_A2 = 0.05881732e+00;
        state.dataGenerator->GasPhaseThermoChemistryData(10).NASA_A3 = -0.04525782e-03;
        state.dataGenerator->GasPhaseThermoChemistryData(10).NASA_A4 = 0.02037115e-06;
        state.dataGenerator->GasPhaseThermoChemistryData(10).NASA_A5 = -0.04079458e-10;
        state.dataGenerator->GasPhaseThermoChemistryData(10).NASA_A6 = -0.01760233e+06;
        state.dataGenerator->GasPhaseThermoChemistryData(10).NASA_A7 = 0.03329595e+03;

        // Pentane (C5H12)
        state.dataGenerator->GasPhaseThermoChemistryData(11).ConstituentName = "Pentane";
        state.dataGenerator->GasPhaseThermoChemistryData(11).ConstituentFormula = "C5H12";
        state.dataGenerator->GasPhaseThermoChemistryData(11).StdRefMolarEnthOfForm = -146.348; // -146.8 !kJ/mol (Good 1970)
        state.dataGenerator->GasPhaseThermoChemistryData(11).ThermoMode = DataGenerators::ThermodynamicMode::NISTShomate;
        state.dataGenerator->GasPhaseThermoChemistryData(11).ShomateA = -34.9431;
        state.dataGenerator->GasPhaseThermoChemistryData(11).ShomateB = 576.777;
        state.dataGenerator->GasPhaseThermoChemistryData(11).ShomateC = -338.353;
        state.dataGenerator->GasPhaseThermoChemistryData(11).ShomateD = 76.8232;
        state.dataGenerator->GasPhaseThermoChemistryData(11).ShomateE = 1.00948;
        state.dataGenerator->GasPhaseThermoChemistryData(11).ShomateF = -155.348;
        state.dataGenerator->GasPhaseThermoChemistryData(11).ShomateG = -999.0;
        state.dataGenerator->GasPhaseThermoChemistryData(11).ShomateH = -146.348;
        state.dataGenerator->GasPhaseThermoChemistryData(11).NumCarbons = 5.0;
        state.dataGenerator->GasPhaseThermoChemistryData(11).NumHydrogens = 12.0;
        state.dataGenerator->GasPhaseThermoChemistryData(11).NumOxygens = 0.0;
        state.dataGenerator->GasPhaseThermoChemistryData(11).MolecularWeight = 72.15;
        state.dataGenerator->GasPhaseThermoChemistryData(11).NASA_A1 = 0.01877907e+02;
        state.dataGenerator->GasPhaseThermoChemistryData(11).NASA_A2 = 0.04121645e+00;
        state.dataGenerator->GasPhaseThermoChemistryData(11).NASA_A3 = 0.12532337e-04;
        state.dataGenerator->GasPhaseThermoChemistryData(11).NASA_A4 = -0.03701536e-06;
        state.dataGenerator->GasPhaseThermoChemistryData(11).NASA_A5 = 0.15255685e-10;
        state.dataGenerator->GasPhaseThermoChemistryData(11).NASA_A6 = -0.02003815e+06;
        state.dataGenerator->GasPhaseThermoChemistryData(11).NASA_A7 = 0.01877256e+03;

        // Hexane  (C6H14)
        state.dataGenerator->GasPhaseThermoChemistryData(12).ConstituentName = "Hexane";
        state.dataGenerator->GasPhaseThermoChemistryData(12).ConstituentFormula = "C6H14";
        state.dataGenerator->GasPhaseThermoChemistryData(12).StdRefMolarEnthOfForm = -166.966; // -167.2 !kJ/mol (Prosen and Rossini 1945)
        state.dataGenerator->GasPhaseThermoChemistryData(12).ThermoMode = DataGenerators::ThermodynamicMode::NISTShomate;
        state.dataGenerator->GasPhaseThermoChemistryData(12).ShomateA = -46.7786;
        state.dataGenerator->GasPhaseThermoChemistryData(12).ShomateB = 711.187;
        state.dataGenerator->GasPhaseThermoChemistryData(12).ShomateC = -438.39;
        state.dataGenerator->GasPhaseThermoChemistryData(12).ShomateD = 103.784;
        state.dataGenerator->GasPhaseThermoChemistryData(12).ShomateE = 1.23887;
        state.dataGenerator->GasPhaseThermoChemistryData(12).ShomateF = -176.813;
        state.dataGenerator->GasPhaseThermoChemistryData(12).ShomateG = -999.0;
        state.dataGenerator->GasPhaseThermoChemistryData(12).ShomateH = -166.966;
        state.dataGenerator->GasPhaseThermoChemistryData(12).NumCarbons = 6.0;
        state.dataGenerator->GasPhaseThermoChemistryData(12).NumHydrogens = 14.0;
        state.dataGenerator->GasPhaseThermoChemistryData(12).NumOxygens = 0.0;
        state.dataGenerator->GasPhaseThermoChemistryData(12).MolecularWeight = 86.18;
        state.dataGenerator->GasPhaseThermoChemistryData(12).NASA_A1 = 0.01836174e+02;
        state.dataGenerator->GasPhaseThermoChemistryData(12).NASA_A2 = 0.05098461e+00;
        state.dataGenerator->GasPhaseThermoChemistryData(12).NASA_A3 = 0.12595857e-04;
        state.dataGenerator->GasPhaseThermoChemistryData(12).NASA_A4 = -0.04428362e-06;
        state.dataGenerator->GasPhaseThermoChemistryData(12).NASA_A5 = 0.01872237e-09;
        state.dataGenerator->GasPhaseThermoChemistryData(12).NASA_A6 = -0.02292749e+06;
        state.dataGenerator->GasPhaseThermoChemistryData(12).NASA_A7 = 0.02088145e+03;

        // Methanol (CH3OH)
        // No Shomate coefficients???
        state.dataGenerator->GasPhaseThermoChemistryData(13).ConstituentName = "Methanol";
        state.dataGenerator->GasPhaseThermoChemistryData(13).ConstituentFormula = "CH3OH";
        state.dataGenerator->GasPhaseThermoChemistryData(13).StdRefMolarEnthOfForm = -201.102; // -201.0 !kJ/mol (Hine and Arata 1976)
        state.dataGenerator->GasPhaseThermoChemistryData(13).ThermoMode = DataGenerators::ThermodynamicMode::NISTShomate;
        state.dataGenerator->GasPhaseThermoChemistryData(13).ShomateA = 14.1952;
        state.dataGenerator->GasPhaseThermoChemistryData(13).ShomateB = 97.7218;
        state.dataGenerator->GasPhaseThermoChemistryData(13).ShomateC = -9.73279;
        state.dataGenerator->GasPhaseThermoChemistryData(13).ShomateD = -12.8461;
        state.dataGenerator->GasPhaseThermoChemistryData(13).ShomateE = 0.15819;
        state.dataGenerator->GasPhaseThermoChemistryData(13).ShomateF = -209.037;
        state.dataGenerator->GasPhaseThermoChemistryData(13).ShomateG = -999.0;
        state.dataGenerator->GasPhaseThermoChemistryData(13).ShomateH = -201.102;
        state.dataGenerator->GasPhaseThermoChemistryData(13).NumCarbons = 1.0;
        state.dataGenerator->GasPhaseThermoChemistryData(13).NumHydrogens = 4.0;
        state.dataGenerator->GasPhaseThermoChemistryData(13).NumOxygens = 1.0;
        state.dataGenerator->GasPhaseThermoChemistryData(13).MolecularWeight = 32.04;
        state.dataGenerator->GasPhaseThermoChemistryData(13).NASA_A1 = 0.02660115e+02;
        state.dataGenerator->GasPhaseThermoChemistryData(13).NASA_A2 = 0.07341508e-01;
        state.dataGenerator->GasPhaseThermoChemistryData(13).NASA_A3 = 0.07170050e-04;
        state.dataGenerator->GasPhaseThermoChemistryData(13).NASA_A4 = -0.08793194e-07;
        state.dataGenerator->GasPhaseThermoChemistryData(13).NASA_A5 = 0.02390570e-10;
        state.dataGenerator->GasPhaseThermoChemistryData(13).NASA_A6 = -0.02535348e+06;
        state.dataGenerator->GasPhaseThermoChemistryData(13).NASA_A7 = 0.11232631e+02;

        // Ethanol (C2H5OH)
        // No Shomate coefficients???
        state.dataGenerator->GasPhaseThermoChemistryData(14).ConstituentName = "Ethanol";
        state.dataGenerator->GasPhaseThermoChemistryData(14).ConstituentFormula = "C2H5OH";
        state.dataGenerator->GasPhaseThermoChemistryData(14).StdRefMolarEnthOfForm = -234.441; //  -235.3 !kJ/mol (Green 1960)
        state.dataGenerator->GasPhaseThermoChemistryData(14).ThermoMode = DataGenerators::ThermodynamicMode::NISTShomate;
        state.dataGenerator->GasPhaseThermoChemistryData(14).ShomateA = -8.87256;
        state.dataGenerator->GasPhaseThermoChemistryData(14).ShomateB = 282.389;
        state.dataGenerator->GasPhaseThermoChemistryData(14).ShomateC = -178.85;
        state.dataGenerator->GasPhaseThermoChemistryData(14).ShomateD = 46.3528;
        state.dataGenerator->GasPhaseThermoChemistryData(14).ShomateE = 0.48364;
        state.dataGenerator->GasPhaseThermoChemistryData(14).ShomateF = -241.239;
        state.dataGenerator->GasPhaseThermoChemistryData(14).ShomateG = -999.0;
        state.dataGenerator->GasPhaseThermoChemistryData(14).ShomateH = -234.441;
        state.dataGenerator->GasPhaseThermoChemistryData(14).NumCarbons = 2.0;
        state.dataGenerator->GasPhaseThermoChemistryData(14).NumHydrogens = 6.0;
        state.dataGenerator->GasPhaseThermoChemistryData(14).NumOxygens = 1.0;
        state.dataGenerator->GasPhaseThermoChemistryData(14).MolecularWeight = 46.07;
        state.dataGenerator->GasPhaseThermoChemistryData(14).NASA_A1 = 0.18461027e+01;
        state.dataGenerator->GasPhaseThermoChemistryData(14).NASA_A2 = 0.20475008e-01;
        state.dataGenerator->GasPhaseThermoChemistryData(14).NASA_A3 = 0.39904089e-05;
        state.dataGenerator->GasPhaseThermoChemistryData(14).NASA_A4 = -0.16585986e-07;
        state.dataGenerator->GasPhaseThermoChemistryData(14).NASA_A5 = 0.73090440e-11;
        state.dataGenerator->GasPhaseThermoChemistryData(14).NASA_A6 = -0.29663086e+05;
        state.dataGenerator->GasPhaseThermoChemistryData(14).NASA_A7 = 0.17289993e+02;

        if (state.dataGenerator->FuelSupply(FuelSupplyNum).FuelTypeMode == DataGenerators::FuelMode::GaseousConstituents) {
            // now calculate LHV of fuel for entire simulation

            // sum over each constituent
            Real64 O2Stoic = 0.0;      // stochiometric oxygen coef in chemical equation (15)
            Real64 CO2ProdStoic = 0.0; // product gases carbon dioxide coeff
            Real64 H2OProdStoic = 0.0; // product gases water coeff
            int CO2dataID = 1;         // hard-coded above
            int WaterDataID = 4;       // hard-coded above
            // Loop over fuel constituents and do one-time setup
            for (int i = 1; i <= state.dataGenerator->FuelSupply(FuelSupplyNum).NumConstituents; ++i) {

                std::string const &thisName = state.dataGenerator->FuelSupply(FuelSupplyNum).ConstitName(i);
                int thisGasID = Util::FindItem(
                    thisName, state.dataGenerator->GasPhaseThermoChemistryData, &DataGenerators::GasPropertyDataStruct::ConstituentName);
                state.dataGenerator->FuelSupply(FuelSupplyNum).GasLibID(i) = thisGasID;

                if (thisGasID == 0) {
                    ShowSevereError(state, format("Fuel constituent not found in thermochemistry data: {}", thisName));
                    ErrorsFound = true;
                }

                // for this fuel mixture, figure stoichiometric oxygen requirement
                O2Stoic += state.dataGenerator->FuelSupply(FuelSupplyNum).ConstitMolalFract(i) *
                           (state.dataGenerator->GasPhaseThermoChemistryData(thisGasID).NumCarbons +
                            state.dataGenerator->GasPhaseThermoChemistryData(thisGasID).NumHydrogens / 4.0 -
                            state.dataGenerator->GasPhaseThermoChemistryData(thisGasID).NumOxygens / 2.0);
                // for this fuel mixture, figure stoichiometric Carbon Dioxide in Product Gases

                CO2ProdStoic += state.dataGenerator->FuelSupply(FuelSupplyNum).ConstitMolalFract(i) *
                                state.dataGenerator->GasPhaseThermoChemistryData(thisGasID).NumCarbons;

                H2OProdStoic += state.dataGenerator->FuelSupply(FuelSupplyNum).ConstitMolalFract(i) *
                                state.dataGenerator->GasPhaseThermoChemistryData(thisGasID).NumHydrogens / 2.0;
            }

            state.dataGenerator->FuelSupply(FuelSupplyNum).StoicOxygenRate = O2Stoic;
            state.dataGenerator->FuelSupply(FuelSupplyNum).CO2ProductGasCoef = CO2ProdStoic;
            state.dataGenerator->FuelSupply(FuelSupplyNum).H2OProductGasCoef = H2OProdStoic;

            // Calculate LHV for an NdotFuel of 1.0
            Real64 LHVfuel = 0.0;
            Real64 LHVi; // working var for lower heating value calc
            for (int i = 1; i <= state.dataGenerator->FuelSupply(FuelSupplyNum).NumConstituents; ++i) {
                int thisGasID = state.dataGenerator->FuelSupply(FuelSupplyNum).GasLibID(i);
                if (state.dataGenerator->GasPhaseThermoChemistryData(thisGasID).NumHydrogens == 0.0) {
                    LHVi = 0.0;
                } else {
                    LHVi = state.dataGenerator->GasPhaseThermoChemistryData(thisGasID).StdRefMolarEnthOfForm -
                           state.dataGenerator->GasPhaseThermoChemistryData(thisGasID).NumCarbons *
                               state.dataGenerator->GasPhaseThermoChemistryData(CO2dataID).StdRefMolarEnthOfForm -
                           (state.dataGenerator->GasPhaseThermoChemistryData(thisGasID).NumHydrogens / 2.0) *
                               state.dataGenerator->GasPhaseThermoChemistryData(WaterDataID).StdRefMolarEnthOfForm;
                }
                LHVfuel += LHVi * state.dataGenerator->FuelSupply(FuelSupplyNum).ConstitMolalFract(i);
            }
            state.dataGenerator->FuelSupply(FuelSupplyNum).LHV = LHVfuel;

            // Calculate HHV for an NdotFuel of 1.0
            Real64 HHVfuel = 0.0;
            Real64 HHVi; // working var for higher heating value calc
            for (int i = 1; i <= state.dataGenerator->FuelSupply(FuelSupplyNum).NumConstituents; ++i) {
                int thisGasID = state.dataGenerator->FuelSupply(FuelSupplyNum).GasLibID(i);
                if (state.dataGenerator->GasPhaseThermoChemistryData(thisGasID).NumHydrogens == 0.0) {
                    HHVi = 0.0;
                } else {
                    HHVi = state.dataGenerator->GasPhaseThermoChemistryData(thisGasID).StdRefMolarEnthOfForm -
                           state.dataGenerator->GasPhaseThermoChemistryData(thisGasID).NumCarbons *
                               state.dataGenerator->GasPhaseThermoChemistryData(CO2dataID).StdRefMolarEnthOfForm -
                           (state.dataGenerator->GasPhaseThermoChemistryData(thisGasID).NumHydrogens / 2.0) *
                               state.dataGenerator->GasPhaseThermoChemistryData(WaterDataID).StdRefMolarEnthOfForm +
                           (state.dataGenerator->GasPhaseThermoChemistryData(thisGasID).NumHydrogens / 2.0) *
                               (state.dataGenerator->GasPhaseThermoChemistryData(WaterDataID).StdRefMolarEnthOfForm + 285.8304);
                }
                HHVfuel += HHVi * state.dataGenerator->FuelSupply(FuelSupplyNum).ConstitMolalFract(i);
            }

            // Calculate Molecular Weight for this fuel
            Real64 MWfuel = 0.0;
            for (int i = 1; i <= state.dataGenerator->FuelSupply(FuelSupplyNum).NumConstituents; ++i) {
                int thisGasID = state.dataGenerator->FuelSupply(FuelSupplyNum).GasLibID(i);
                MWfuel += state.dataGenerator->FuelSupply(FuelSupplyNum).ConstitMolalFract(i) *
                          state.dataGenerator->GasPhaseThermoChemistryData(thisGasID).MolecularWeight;
            }
            state.dataGenerator->FuelSupply(FuelSupplyNum).MW = MWfuel;
            state.dataGenerator->FuelSupply(FuelSupplyNum).KmolPerSecToKgPerSec = MWfuel;      // TODO check this, guessing on conversion...
            state.dataGenerator->FuelSupply(FuelSupplyNum).HHV = 1000000.0 * HHVfuel / MWfuel; // (1000/k) (1000/k) (kJ/mol)/(g/mol) = J/kg
            state.dataGenerator->FuelSupply(FuelSupplyNum).LHVJperkg =
                state.dataGenerator->FuelSupply(FuelSupplyNum).LHV * 1000000.0 / state.dataGenerator->FuelSupply(FuelSupplyNum).MW;

        } else if (state.dataGenerator->FuelSupply(FuelSupplyNum).FuelTypeMode == DataGenerators::FuelMode::GenericLiquid) {
            state.dataGenerator->FuelSupply(FuelSupplyNum).LHV = state.dataGenerator->FuelSupply(FuelSupplyNum).LHVliquid *
                                                                 state.dataGenerator->FuelSupply(FuelSupplyNum).MW /
                                                                 1000000.0; // J/kg * g/mol (k/1000) (k/10000)

        } else {
        }

        // report Heating Values in EIO.
        if (first_time) {
            print(state.files.eio,
                  "! <Fuel Supply>, Fuel Supply Name, Lower Heating Value [J/kmol], Lower Heating Value [kJ/kg], Higher "
                  "Heating Value [KJ/kg],  Molecular Weight [g/mol] \n");
        }
        static constexpr std::string_view Format_501(" Fuel Supply, {},{:13.6N},{:13.6N},{:13.6N},{:13.6N}\n");
        print(state.files.eio,
              Format_501,
              state.dataGenerator->FuelSupply(FuelSupplyNum).Name,
              state.dataGenerator->FuelSupply(FuelSupplyNum).LHV * 1000000.0,
              state.dataGenerator->FuelSupply(FuelSupplyNum).LHVJperkg / 1000.0,
              state.dataGenerator->FuelSupply(FuelSupplyNum).HHV / 1000.0,
              state.dataGenerator->FuelSupply(FuelSupplyNum).MW);
    }

} // namespace GeneratorFuelSupply

} // namespace EnergyPlus
