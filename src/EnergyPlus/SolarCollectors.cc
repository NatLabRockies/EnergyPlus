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

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/SolarCollectors.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace SolarCollectors {

    // MODULE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   December 2003
    //       MODIFIED       B. Nigusse, FSEC/UCF, March 2012, added ICS Collector
    //       RE-ENGINEERED  Brent Griffith, for plant upgrade, general fluid props

    // PURPOSE OF THIS MODULE:
    // Simulates solar collectors as a component on the plant loop.  Currently only flat-plate collectors (glazed and
    // unglazed) are implemented.

    // METHODOLOGY EMPLOYED:
    // Solar collectors are called as non-zone equipment on the demand side of the plant loop.  The collector object
    // must be connected to a WATER HEATER object on the supply side of the plant loop.  Water is assumed to be
    // the heat transfer fluid.

    PlantComponent *CollectorData::factory(EnergyPlusData &state, std::string const &objectName)
    {
        // Process the input data
        if (state.dataSolarCollectors->GetInputFlag) {
            GetSolarCollectorInput(state);
            state.dataSolarCollectors->GetInputFlag = false;
        }
        // Now look for this particular object
        for (auto &thisSC : state.dataSolarCollectors->Collector) {
            if (thisSC.Name == objectName) {
                return &thisSC;
            }
        }
        // If we didn't find it, fatal
        ShowFatalError(state, format("LocalSolarCollectorFactory: Error getting inputs for object named: {}", objectName)); // LCOV_EXCL_LINE
        // Shut up the compiler
        return nullptr; // LCOV_EXCL_LINE
    }

    void GetSolarCollectorInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   December 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Gets the solar collector input from the input file and sets up the parameters and collector objects.

        bool ErrorsFound(false);              // Set to true if errors in input, fatal at end of routine
        int IOStatus;                         // Used in GetObjectItem
        int NumAlphas;                        // Number of Alphas for each GetObjectItem call
        int NumNumbers;                       // Number of Numbers for each GetObjectItem call
        std::string CurrentModuleObject;      // for ease in renaming.
        std::string CurrentModuleParamObject; // for ease in renaming.

        int NumFields;  // Total number of fields in object
        int MaxAlphas;  // Maximum number of alpha fields in all objects
        int MaxNumbers; // Maximum number of numeric fields in all objects

        Array1D<Real64> Numbers;       // Numeric data
        Array1D_string Alphas;         // Alpha data
        Array1D_string cAlphaFields;   // Alpha field names
        Array1D_string cNumericFields; // Numeric field names
        Array1D_bool lAlphaBlanks;     // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks;   // Logical array, numeric field input BLANK = .TRUE.

        MaxNumbers = 0;
        MaxAlphas = 0;

        CurrentModuleParamObject = "SolarCollectorPerformance:FlatPlate";
        int NumOfFlatPlateParam = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleParamObject);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleParamObject, NumFields, NumAlphas, NumNumbers);
        MaxNumbers = max(MaxNumbers, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);

        CurrentModuleObject = "SolarCollector:FlatPlate:Water";
        int NumFlatPlateUnits = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumFields, NumAlphas, NumNumbers);
        MaxNumbers = max(MaxNumbers, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);

        CurrentModuleParamObject = "SolarCollectorPerformance:IntegralCollectorStorage";
        int NumOfICSParam = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleParamObject);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleParamObject, NumFields, NumAlphas, NumNumbers);
        MaxNumbers = max(MaxNumbers, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);

        CurrentModuleObject = "SolarCollector:IntegralCollectorStorage";
        int NumOfICSUnits = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumFields, NumAlphas, NumNumbers);
        MaxNumbers = max(MaxNumbers, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);

        Alphas.allocate(MaxAlphas);
        Numbers.dimension(MaxNumbers, 0.0);
        cAlphaFields.allocate(MaxAlphas);
        cNumericFields.allocate(MaxNumbers);
        lAlphaBlanks.dimension(MaxAlphas, true);
        lNumericBlanks.dimension(MaxNumbers, true);

        state.dataSolarCollectors->NumOfCollectors = NumFlatPlateUnits + NumOfICSUnits;
        state.dataSolarCollectors->NumOfParameters = NumOfFlatPlateParam + NumOfICSParam;

        if (state.dataSolarCollectors->NumOfParameters > 0) {
            state.dataSolarCollectors->Parameters.allocate(state.dataSolarCollectors->NumOfParameters);

            CurrentModuleParamObject = "SolarCollectorPerformance:FlatPlate";

            for (int FlatPlateParamNum = 1; FlatPlateParamNum <= NumOfFlatPlateParam; ++FlatPlateParamNum) {

                int ParametersNum = FlatPlateParamNum;
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         CurrentModuleParamObject,
                                                                         ParametersNum,
                                                                         state.dataIPShortCut->cAlphaArgs,
                                                                         NumAlphas,
                                                                         state.dataIPShortCut->rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         state.dataIPShortCut->lNumericFieldBlanks,
                                                                         _,
                                                                         state.dataIPShortCut->cAlphaFieldNames,
                                                                         state.dataIPShortCut->cNumericFieldNames);

                // Collector module parameters name
                GlobalNames::VerifyUniqueInterObjectName(state,
                                                         state.dataSolarCollectors->UniqueParametersNames,
                                                         state.dataIPShortCut->cAlphaArgs(1),
                                                         CurrentModuleObject,
                                                         state.dataIPShortCut->cAlphaFieldNames(1),
                                                         ErrorsFound);
                state.dataSolarCollectors->Parameters(ParametersNum).Name = state.dataIPShortCut->cAlphaArgs(1);

                // NOTE:  This values serves mainly as a reference.  The area of the associated surface object is used in all calculations.
                state.dataSolarCollectors->Parameters(ParametersNum).Area = state.dataIPShortCut->rNumericArgs(1);

                //                The TestFluid member variable was never accessed, so this input field seems to not do anything as of right now
                //                if (state.dataIPShortCut->cAlphaArgs(2) == "WATER") {
                //                    state.dataSolarCollectors->Parameters(ParametersNum).TestFluid = FluidEnum::WATER;
                //                    // CASE('AIR')
                //                    //  Parameters(ParametersNum)%TestFluid = AIR
                //                } else {
                //                    ShowSevereError(state, format("{}{} = {}:  {}{} is an unsupported Test Fluid for {}{}", //,
                //                    CurrentModuleParamObject, state.dataIPShortCut->cAlphaArgs(1), //, state.dataIPShortCut->cAlphaArgs(2), //,
                //                    state.dataIPShortCut->cAlphaFieldNames(2))); ErrorsFound = true;
                //                }

                if (state.dataIPShortCut->rNumericArgs(2) > 0.0) {
                    state.dataSolarCollectors->Parameters(ParametersNum).TestMassFlowRate =
                        state.dataIPShortCut->rNumericArgs(2) * Psychrometrics::RhoH2O(Constant::InitConvTemp);
                } else {
                    ShowSevereError(state,
                                    format("{} = {}:  flow rate must be greater than zero for {}",
                                           CurrentModuleParamObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           state.dataIPShortCut->cNumericFieldNames(2)));
                    ErrorsFound = true;
                }

                std::string_view const key = state.dataIPShortCut->cAlphaArgs(3);
                state.dataSolarCollectors->Parameters(ParametersNum).TestType = static_cast<TestTypeEnum>(getEnumValue(testTypesUC, key));
                if (state.dataSolarCollectors->Parameters(ParametersNum).TestType == TestTypeEnum::INVALID) {
                    ShowSevereError(state,
                                    format("{} = {}: {} is not supported for {}",
                                           CurrentModuleParamObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           key,
                                           state.dataIPShortCut->cAlphaFieldNames(3)));
                    ErrorsFound = true;
                }

                // Efficiency equation coefficients
                state.dataSolarCollectors->Parameters(ParametersNum).eff0 = state.dataIPShortCut->rNumericArgs(3);
                state.dataSolarCollectors->Parameters(ParametersNum).eff1 = state.dataIPShortCut->rNumericArgs(4);

                if (NumNumbers > 4) {
                    state.dataSolarCollectors->Parameters(ParametersNum).eff2 = state.dataIPShortCut->rNumericArgs(5);
                } else {
                    state.dataSolarCollectors->Parameters(ParametersNum).eff2 = 0.0;
                }

                // Incident angle modifier coefficients
                if (NumNumbers > 5) {
                    state.dataSolarCollectors->Parameters(ParametersNum).iam1 = state.dataIPShortCut->rNumericArgs(6);
                } else {
                    state.dataSolarCollectors->Parameters(ParametersNum).iam1 = 0.0;
                }

                if (NumNumbers > 6) {
                    state.dataSolarCollectors->Parameters(FlatPlateParamNum).iam2 = state.dataIPShortCut->rNumericArgs(7);
                } else {
                    state.dataSolarCollectors->Parameters(ParametersNum).iam2 = 0.0;
                }
            } // ParametersNum

            if (ErrorsFound) {
                ShowFatalError(state, format("Errors in {} input.", CurrentModuleParamObject));
            }
        }

        if (state.dataSolarCollectors->NumOfCollectors > 0) {
            state.dataSolarCollectors->Collector.allocate(state.dataSolarCollectors->NumOfCollectors);

            CurrentModuleObject = "SolarCollector:FlatPlate:Water";

            for (int FlatPlateUnitsNum = 1; FlatPlateUnitsNum <= NumFlatPlateUnits; ++FlatPlateUnitsNum) {

                int CollectorNum = FlatPlateUnitsNum;

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         CurrentModuleObject,
                                                                         CollectorNum,
                                                                         state.dataIPShortCut->cAlphaArgs,
                                                                         NumAlphas,
                                                                         state.dataIPShortCut->rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStatus);

                // Collector name
                GlobalNames::VerifyUniqueInterObjectName(
                    state, state.dataSolarCollectors->UniqueCollectorNames, state.dataIPShortCut->cAlphaArgs(1), CurrentModuleObject, ErrorsFound);
                state.dataSolarCollectors->Collector(CollectorNum).Name = state.dataIPShortCut->cAlphaArgs(1);
                state.dataSolarCollectors->Collector(CollectorNum).Type =
                    DataPlant::PlantEquipmentType::SolarCollectorFlatPlate; // parameter assigned in DataPlant

                // Get parameters object
                int ParametersNum = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(2), state.dataSolarCollectors->Parameters);

                if (ParametersNum == 0) {
                    ShowSevereError(state,
                                    format("{} = {}: {} object called {} not found.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           CurrentModuleParamObject,
                                           state.dataIPShortCut->cAlphaArgs(2)));
                    ErrorsFound = true;
                } else {
                    state.dataSolarCollectors->Collector(CollectorNum).Parameters = ParametersNum;
                }

                // Get surface object
                int SurfNum = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(3), state.dataSurface->Surface);

                if (SurfNum == 0) {
                    ShowSevereError(state,
                                    format("{} = {}:  Surface {} not found.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           state.dataIPShortCut->cAlphaArgs(3)));
                    ErrorsFound = true;
                    continue; // avoid hard crash
                } else {

                    if (!state.dataSurface->Surface(SurfNum).ExtSolar) {
                        ShowWarningError(state,
                                         format("{} = {}:  Surface {} is not exposed to exterior radiation.",
                                                CurrentModuleObject,
                                                state.dataIPShortCut->cAlphaArgs(1),
                                                state.dataIPShortCut->cAlphaArgs(3)));
                    }

                    // check surface orientation, warn if upside down
                    if ((state.dataSurface->Surface(SurfNum).Tilt < -95.0) || (state.dataSurface->Surface(SurfNum).Tilt > 95.0)) {
                        ShowWarningError(state,
                                         format("Suspected input problem with {} = {}",
                                                state.dataIPShortCut->cAlphaFieldNames(3),
                                                state.dataIPShortCut->cAlphaArgs(3)));
                        ShowContinueError(
                            state, format("Entered in {} = {}", state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                        ShowContinueError(state, "Surface used for solar collector faces down");
                        ShowContinueError(
                            state,
                            format("Surface tilt angle (degrees from ground outward normal) = {:.2R}", state.dataSurface->Surface(SurfNum).Tilt));
                    }

                    // Check to make sure other solar collectors are not using the same surface
                    // NOTE:  Must search over all solar collector types
                    for (int CollectorNum2 = 1; CollectorNum2 <= NumFlatPlateUnits; ++CollectorNum2) {
                        if (state.dataSolarCollectors->Collector(CollectorNum2).Surface == SurfNum) {
                            ShowSevereError(state,
                                            format("{} = {}:  Surface {} is referenced by more than one {}",
                                                   CurrentModuleObject,
                                                   state.dataIPShortCut->cAlphaArgs(1),
                                                   state.dataIPShortCut->cAlphaArgs(3),
                                                   CurrentModuleObject));
                            ErrorsFound = true;
                            break;
                        }
                    } // CollectorNum2

                    state.dataSolarCollectors->Collector(CollectorNum).Surface = SurfNum;
                }

                // Give warning if surface area and gross area do not match within tolerance
                if (SurfNum > 0 && ParametersNum > 0 && state.dataSolarCollectors->Parameters(ParametersNum).Area > 0.0 &&
                    std::abs(state.dataSolarCollectors->Parameters(ParametersNum).Area - state.dataSurface->Surface(SurfNum).Area) /
                            state.dataSurface->Surface(SurfNum).Area >
                        0.01) {

                    ShowWarningError(state,
                                     format("{} = {}:  Gross Area of solar collector parameters and surface object differ by more than 1%.",
                                            CurrentModuleObject,
                                            state.dataIPShortCut->cAlphaArgs(1)));
                    ShowContinueError(state, "Area of surface object will be used in all calculations.");
                }

                state.dataSolarCollectors->Collector(CollectorNum).InletNode =
                    NodeInputManager::GetOnlySingleNode(state,
                                                        state.dataIPShortCut->cAlphaArgs(4),
                                                        ErrorsFound,
                                                        DataLoopNode::ConnectionObjectType::SolarCollectorFlatPlateWater,
                                                        state.dataIPShortCut->cAlphaArgs(1),
                                                        DataLoopNode::NodeFluidType::Water,
                                                        DataLoopNode::ConnectionType::Inlet,
                                                        NodeInputManager::CompFluidStream::Primary,
                                                        DataLoopNode::ObjectIsNotParent);
                state.dataSolarCollectors->Collector(CollectorNum).OutletNode =
                    NodeInputManager::GetOnlySingleNode(state,
                                                        state.dataIPShortCut->cAlphaArgs(5),
                                                        ErrorsFound,
                                                        DataLoopNode::ConnectionObjectType::SolarCollectorFlatPlateWater,
                                                        state.dataIPShortCut->cAlphaArgs(1),
                                                        DataLoopNode::NodeFluidType::Water,
                                                        DataLoopNode::ConnectionType::Outlet,
                                                        NodeInputManager::CompFluidStream::Primary,
                                                        DataLoopNode::ObjectIsNotParent);

                if (NumNumbers > 0) {
                    state.dataSolarCollectors->Collector(CollectorNum).VolFlowRateMax =
                        state.dataIPShortCut->rNumericArgs(1); // Max volumetric flow rate used for plant sizing calculation
                } else {
                    state.dataSolarCollectors->Collector(CollectorNum).VolFlowRateMax =
                        0.0; // Max vol flow rate is not specified; no flow for plant sizing calculation
                    state.dataSolarCollectors->Collector(CollectorNum).MassFlowRateMax =
                        999999.9; // But...set a very high value so that it demands as much as possible
                }

                BranchNodeConnections::TestCompSet(state,
                                                   CurrentModuleObject,
                                                   state.dataIPShortCut->cAlphaArgs(1),
                                                   state.dataIPShortCut->cAlphaArgs(4),
                                                   state.dataIPShortCut->cAlphaArgs(5),
                                                   "Water Nodes");

            } // FlatPlateUnitsNum

            // Get data for ICS collector
            CurrentModuleParamObject = "SolarCollectorPerformance:IntegralCollectorStorage";

            for (int ICSParamNum = 1; ICSParamNum <= NumOfICSParam; ++ICSParamNum) {

                int ParametersNum = ICSParamNum + NumOfFlatPlateParam;

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         CurrentModuleParamObject,
                                                                         ICSParamNum,
                                                                         state.dataIPShortCut->cAlphaArgs,
                                                                         NumAlphas,
                                                                         state.dataIPShortCut->rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         state.dataIPShortCut->lNumericFieldBlanks,
                                                                         _,
                                                                         state.dataIPShortCut->cAlphaFieldNames,
                                                                         state.dataIPShortCut->cNumericFieldNames);

                // Collector module parameters name
                GlobalNames::VerifyUniqueInterObjectName(state,
                                                         state.dataSolarCollectors->UniqueParametersNames,
                                                         state.dataIPShortCut->cAlphaArgs(1),
                                                         CurrentModuleObject,
                                                         state.dataIPShortCut->cAlphaFieldNames(1),
                                                         ErrorsFound);
                state.dataSolarCollectors->Parameters(ParametersNum).Name = state.dataIPShortCut->cAlphaArgs(1);
                // NOTE:  currently the only available choice is RectangularTank.  In the future progressive tube type will be
                //        added
                //                if (Util::SameString(state.dataIPShortCut->cAlphaArgs(2), "RectangularTank")) {
                //                    state.dataSolarCollectors->Parameters(ParametersNum).ICSType_Num = TankTypeEnum::ICSRectangularTank;
                //                } else {
                //                    ShowSevereError(state, format("{}{} not found={}{} in {}{} ={}{}", //,
                //                    state.dataIPShortCut->cAlphaFieldNames(2), state.dataIPShortCut->cAlphaArgs(2), //, //,
                //                    CurrentModuleParamObject, //, state.dataSolarCollectors->Parameters(ParametersNum).Name)); ErrorsFound = true;
                //                }
                // NOTE:  This collector gross area is used in all the calculations.
                state.dataSolarCollectors->Parameters(ParametersNum).Area = state.dataIPShortCut->rNumericArgs(1);
                if (state.dataIPShortCut->rNumericArgs(1) <= 0.0) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleParamObject, state.dataIPShortCut->cAlphaArgs(1)));
                    ShowContinueError(
                        state, format("Illegal {} = {:.2R}", state.dataIPShortCut->cNumericFieldNames(1), state.dataIPShortCut->rNumericArgs(1)));
                    ShowContinueError(state, " Collector gross area must be always greater than zero.");
                    ErrorsFound = true;
                }
                state.dataSolarCollectors->Parameters(ParametersNum).Volume = state.dataIPShortCut->rNumericArgs(2);
                if (state.dataIPShortCut->rNumericArgs(2) <= 0.0) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleParamObject, state.dataIPShortCut->cAlphaArgs(1)));
                    ShowContinueError(
                        state, format("Illegal {} = {:.2R}", state.dataIPShortCut->cNumericFieldNames(2), state.dataIPShortCut->rNumericArgs(2)));
                    ShowContinueError(state, " Collector water volume must be always greater than zero.");
                    ErrorsFound = true;
                }
                // Note: this value is used to calculate the heat loss through the bottom and side of the collector
                state.dataSolarCollectors->Parameters(ParametersNum).ULossBottom = state.dataIPShortCut->rNumericArgs(3);
                state.dataSolarCollectors->Parameters(ParametersNum).ULossSide = state.dataIPShortCut->rNumericArgs(4);
                state.dataSolarCollectors->Parameters(ParametersNum).AspectRatio = state.dataIPShortCut->rNumericArgs(5);
                state.dataSolarCollectors->Parameters(ParametersNum).SideHeight = state.dataIPShortCut->rNumericArgs(6);
                state.dataSolarCollectors->Parameters(ParametersNum).ThermalMass = state.dataIPShortCut->rNumericArgs(7);
                state.dataSolarCollectors->Parameters(ParametersNum).NumOfCovers = state.dataIPShortCut->rNumericArgs(8);
                state.dataSolarCollectors->Parameters(ParametersNum).CoverSpacing = state.dataIPShortCut->rNumericArgs(9);

                if (state.dataSolarCollectors->Parameters(ParametersNum).NumOfCovers == 2) {
                    // Outer cover refractive index
                    state.dataSolarCollectors->Parameters(ParametersNum).RefractiveIndex[0] = state.dataIPShortCut->rNumericArgs(10);
                    // Outer cover extinction coefficient times thickness of the cover
                    state.dataSolarCollectors->Parameters(ParametersNum).ExtCoefTimesThickness[0] = state.dataIPShortCut->rNumericArgs(11);
                    // Outer cover Emissivity
                    state.dataSolarCollectors->Parameters(ParametersNum).EmissOfCover[0] = state.dataIPShortCut->rNumericArgs(12);

                    if (!state.dataIPShortCut->lNumericFieldBlanks(13) || !state.dataIPShortCut->lNumericFieldBlanks(14) ||
                        !state.dataIPShortCut->lNumericFieldBlanks(15)) {
                        state.dataSolarCollectors->Parameters(ParametersNum).RefractiveIndex[1] = state.dataIPShortCut->rNumericArgs(13);
                        state.dataSolarCollectors->Parameters(ParametersNum).ExtCoefTimesThickness[1] = state.dataIPShortCut->rNumericArgs(14);
                        state.dataSolarCollectors->Parameters(ParametersNum).EmissOfCover[1] = state.dataIPShortCut->rNumericArgs(15);
                    } else {
                        ShowSevereError(state, format("{} = {}", CurrentModuleParamObject, state.dataIPShortCut->cAlphaArgs(1)));
                        ShowContinueError(state, "Illegal input for one of the three inputs of the inner cover optical properties");
                        ErrorsFound = true;
                    }
                } else if (state.dataSolarCollectors->Parameters(ParametersNum).NumOfCovers == 1) {
                    // Outer cover refractive index
                    state.dataSolarCollectors->Parameters(ParametersNum).RefractiveIndex[0] = state.dataIPShortCut->rNumericArgs(10);
                    // Outer cover extinction coefficient times thickness of the cover
                    state.dataSolarCollectors->Parameters(ParametersNum).ExtCoefTimesThickness[0] = state.dataIPShortCut->rNumericArgs(11);
                    // Outer cover emissivity
                    state.dataSolarCollectors->Parameters(ParametersNum).EmissOfCover[0] = state.dataIPShortCut->rNumericArgs(12);
                } else {
                    ShowSevereError(state, format("{} = {}", CurrentModuleParamObject, state.dataIPShortCut->cAlphaArgs(1)));
                    ShowContinueError(
                        state, format("Illegal {} = {:.2R}", state.dataIPShortCut->cNumericFieldNames(8), state.dataIPShortCut->rNumericArgs(8)));
                    ErrorsFound = true;
                }
                // Solar absorptance of the absorber plate
                state.dataSolarCollectors->Parameters(ParametersNum).AbsorOfAbsPlate = state.dataIPShortCut->rNumericArgs(16);
                // thermal emissivity of the absorber plate
                state.dataSolarCollectors->Parameters(ParametersNum).EmissOfAbsPlate = state.dataIPShortCut->rNumericArgs(17);

            } // end of ParametersNum

            if (ErrorsFound) {
                ShowFatalError(state, format("Errors in {} input.", CurrentModuleParamObject));
            }

            CurrentModuleObject = "SolarCollector:IntegralCollectorStorage";

            for (int ICSUnitsNum = 1; ICSUnitsNum <= NumOfICSUnits; ++ICSUnitsNum) {

                int CollectorNum = ICSUnitsNum + NumFlatPlateUnits;

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         CurrentModuleObject,
                                                                         ICSUnitsNum,
                                                                         state.dataIPShortCut->cAlphaArgs,
                                                                         NumAlphas,
                                                                         state.dataIPShortCut->rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         state.dataIPShortCut->lNumericFieldBlanks,
                                                                         lAlphaBlanks,
                                                                         state.dataIPShortCut->cAlphaFieldNames,
                                                                         state.dataIPShortCut->cNumericFieldNames);

                // Collector name
                GlobalNames::VerifyUniqueInterObjectName(state,
                                                         state.dataSolarCollectors->UniqueCollectorNames,
                                                         state.dataIPShortCut->cAlphaArgs(1),
                                                         CurrentModuleObject,
                                                         state.dataIPShortCut->cAlphaFieldNames(1),
                                                         ErrorsFound);
                state.dataSolarCollectors->Collector(CollectorNum).Name = state.dataIPShortCut->cAlphaArgs(1);
                state.dataSolarCollectors->Collector(CollectorNum).Type =
                    DataPlant::PlantEquipmentType::SolarCollectorICS; // parameter assigned in DataPlant

                state.dataSolarCollectors->Collector(CollectorNum).InitICS = true;

                // Get parameters object
                int ParametersNum = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(2), state.dataSolarCollectors->Parameters);

                if (ParametersNum == 0) {
                    ShowSevereError(state,
                                    format("{} = {}: {} object called {} not found.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           CurrentModuleParamObject,
                                           state.dataIPShortCut->cAlphaArgs(2)));
                    ErrorsFound = true;
                } else {
                    state.dataSolarCollectors->Collector(CollectorNum).Parameters = ParametersNum;
                }

                if (ParametersNum > 0) {
                    // Calculate constant collector parameters only once
                    Real64 Perimeter = 2.0 * std::sqrt(state.dataSolarCollectors->Parameters(ParametersNum).Area) *
                                       (std::sqrt(state.dataSolarCollectors->Parameters(ParametersNum).AspectRatio) +
                                        1.0 / std::sqrt(state.dataSolarCollectors->Parameters(ParametersNum).AspectRatio));
                    state.dataSolarCollectors->Collector(CollectorNum).Length = std::sqrt(
                        state.dataSolarCollectors->Parameters(ParametersNum).Area / state.dataSolarCollectors->Parameters(ParametersNum).AspectRatio);

                    // calculate the collector side heat transfer area and loss coefficient
                    state.dataSolarCollectors->Collector(CollectorNum).Area = state.dataSolarCollectors->Parameters(ParametersNum).Area;
                    state.dataSolarCollectors->Collector(CollectorNum).Volume = state.dataSolarCollectors->Parameters(ParametersNum).Volume;
                    state.dataSolarCollectors->Collector(CollectorNum).SideArea =
                        Perimeter * state.dataSolarCollectors->Parameters(ParametersNum).SideHeight;
                    state.dataSolarCollectors->Collector(CollectorNum).AreaRatio =
                        state.dataSolarCollectors->Collector(CollectorNum).SideArea / state.dataSolarCollectors->Collector(CollectorNum).Area;
                }
                // Get surface object
                int SurfNum = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(3), state.dataSurface->Surface);

                if (SurfNum == 0) {
                    ShowSevereError(state,
                                    format("{} = {}:  Surface {} not found.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           state.dataIPShortCut->cAlphaArgs(3)));
                    ErrorsFound = true;
                    continue; // avoid hard crash
                } else {

                    if (!state.dataSurface->Surface(SurfNum).ExtSolar) {
                        ShowWarningError(state,
                                         format("{} = {}:  Surface {} is not exposed to exterior radiation.",
                                                CurrentModuleObject,
                                                state.dataIPShortCut->cAlphaArgs(1),
                                                state.dataIPShortCut->cAlphaArgs(3)));
                    }

                    // check surface orientation, warn if upside down
                    if ((state.dataSurface->Surface(SurfNum).Tilt < -95.0) || (state.dataSurface->Surface(SurfNum).Tilt > 95.0)) {
                        ShowWarningError(state,
                                         format("Suspected input problem with {} = {}",
                                                state.dataIPShortCut->cAlphaFieldNames(3),
                                                state.dataIPShortCut->cAlphaArgs(3)));
                        ShowContinueError(
                            state, format("Entered in {} = {}", state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                        ShowContinueError(state, "Surface used for solar collector faces down");
                        ShowContinueError(
                            state,
                            format("Surface tilt angle (degrees from ground outward normal) = {:.2R}", state.dataSurface->Surface(SurfNum).Tilt));
                    }

                    // Check to make sure other solar collectors are not using the same surface
                    // NOTE:  Must search over all solar collector types
                    for (int CollectorNum2 = 1; CollectorNum2 <= state.dataSolarCollectors->NumOfCollectors; ++CollectorNum2) {
                        if (state.dataSolarCollectors->Collector(CollectorNum2).Surface == SurfNum) {
                            ShowSevereError(state,
                                            format("{} = {}:  Surface {} is referenced by more than one {}",
                                                   CurrentModuleObject,
                                                   state.dataIPShortCut->cAlphaArgs(1),
                                                   state.dataIPShortCut->cAlphaArgs(3),
                                                   CurrentModuleObject));
                            ErrorsFound = true;
                            break;
                        }
                    } // ICSNum2

                    state.dataSolarCollectors->Collector(CollectorNum).Surface = SurfNum;
                }

                // Give warning if surface area and gross area do not match within tolerance
                if (SurfNum > 0 && ParametersNum > 0 && state.dataSolarCollectors->Parameters(ParametersNum).Area > 0.0 &&
                    std::abs(state.dataSolarCollectors->Parameters(ParametersNum).Area - state.dataSurface->Surface(SurfNum).Area) /
                            state.dataSurface->Surface(SurfNum).Area >
                        0.01) {

                    ShowWarningError(state, format("{} = {}: ", CurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                    ShowContinueError(state, "Gross area of solar collector parameters and surface object differ by more than 1%.");
                    ShowContinueError(state, "Gross collector area is always used in the calculation.  Modify the surface ");
                    ShowContinueError(state, "coordinates to match its area with collector gross area. Otherwise, the underlying ");
                    ShowContinueError(state, "surface is assumed to be fully shaded when it is not.");
                }

                state.dataSolarCollectors->Collector(CollectorNum).BCType = state.dataIPShortCut->cAlphaArgs(4);
                if (Util::SameString(state.dataIPShortCut->cAlphaArgs(4), "AmbientAir")) {
                    state.dataSolarCollectors->Collector(CollectorNum).OSCMName = "";
                } else if (Util::SameString(state.dataIPShortCut->cAlphaArgs(4), "OtherSideConditionsModel")) {
                    state.dataSolarCollectors->Collector(CollectorNum).OSCMName = state.dataIPShortCut->cAlphaArgs(5);
                    state.dataSolarCollectors->Collector(CollectorNum).OSCM_ON = true;
                    int Found = Util::FindItemInList(state.dataSolarCollectors->Collector(CollectorNum).OSCMName, state.dataSurface->OSCM);
                    if (Found == 0) {
                        ShowSevereError(state,
                                        format("{} not found={} in {} ={}",
                                               state.dataIPShortCut->cAlphaFieldNames(5),
                                               state.dataSolarCollectors->Collector(CollectorNum).OSCMName,
                                               CurrentModuleObject,
                                               state.dataSolarCollectors->Collector(CollectorNum).Name));
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(state,
                                    format("{} not found={} in {} ={}",
                                           state.dataIPShortCut->cAlphaFieldNames(5),
                                           state.dataSolarCollectors->Collector(CollectorNum).BCType,
                                           CurrentModuleObject,
                                           state.dataSolarCollectors->Collector(CollectorNum).Name));
                    ErrorsFound = true;
                }

                if (state.dataSolarCollectors->Collector(CollectorNum).OSCM_ON) {
                    // get index of ventilated cavity object
                    int VentCavIndex = 0;
                    SolarCollectors::CollectorData::GetExtVentedCavityIndex(state, SurfNum, VentCavIndex);
                    state.dataSolarCollectors->Collector(CollectorNum).VentCavIndex = VentCavIndex;
                }

                state.dataSolarCollectors->Collector(CollectorNum).InletNode =
                    NodeInputManager::GetOnlySingleNode(state,
                                                        state.dataIPShortCut->cAlphaArgs(6),
                                                        ErrorsFound,
                                                        DataLoopNode::ConnectionObjectType::SolarCollectorIntegralCollectorStorage,
                                                        state.dataIPShortCut->cAlphaArgs(1),
                                                        DataLoopNode::NodeFluidType::Water,
                                                        DataLoopNode::ConnectionType::Inlet,
                                                        NodeInputManager::CompFluidStream::Primary,
                                                        DataLoopNode::ObjectIsNotParent);
                state.dataSolarCollectors->Collector(CollectorNum).OutletNode =
                    NodeInputManager::GetOnlySingleNode(state,
                                                        state.dataIPShortCut->cAlphaArgs(7),
                                                        ErrorsFound,
                                                        DataLoopNode::ConnectionObjectType::SolarCollectorIntegralCollectorStorage,
                                                        state.dataIPShortCut->cAlphaArgs(1),
                                                        DataLoopNode::NodeFluidType::Water,
                                                        DataLoopNode::ConnectionType::Outlet,
                                                        NodeInputManager::CompFluidStream::Primary,
                                                        DataLoopNode::ObjectIsNotParent);

                if (NumNumbers > 0) {
                    state.dataSolarCollectors->Collector(CollectorNum).VolFlowRateMax =
                        state.dataIPShortCut->rNumericArgs(1); // Max volumetric flow rate used for plant sizing calculation
                } else {
                    state.dataSolarCollectors->Collector(CollectorNum).VolFlowRateMax =
                        0.0; // Max vol flow rate is not specified; no flow for plant sizing calculation
                    state.dataSolarCollectors->Collector(CollectorNum).MassFlowRateMax =
                        999999.9; // But...set a very high value so that it demands as much as possible
                }

                BranchNodeConnections::TestCompSet(state,
                                                   CurrentModuleObject,
                                                   state.dataIPShortCut->cAlphaArgs(1),
                                                   state.dataIPShortCut->cAlphaArgs(6),
                                                   state.dataIPShortCut->cAlphaArgs(7),
                                                   "Water Nodes");

            } // ICSNum

            if (ErrorsFound) {
                ShowFatalError(state, format("Errors in {} input.", CurrentModuleObject));
            }
        }
    }

    void CollectorData::setupOutputVars(EnergyPlusData &state)
    {
        if (this->Type == DataPlant::PlantEquipmentType::SolarCollectorFlatPlate) {
            // Setup report variables
            SetupOutputVariable(state,
                                "Solar Collector Incident Angle Modifier",
                                Constant::Units::None,
                                this->IncidentAngleModifier,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);

            SetupOutputVariable(state,
                                "Solar Collector Efficiency",
                                Constant::Units::None,
                                this->Efficiency,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);

            SetupOutputVariable(state,
                                "Solar Collector Heat Transfer Rate",
                                Constant::Units::W,
                                this->Power,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);

            SetupOutputVariable(state,
                                "Solar Collector Heat Gain Rate",
                                Constant::Units::W,
                                this->HeatGain,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);

            SetupOutputVariable(state,
                                "Solar Collector Heat Loss Rate",
                                Constant::Units::W,
                                this->HeatLoss,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);

            SetupOutputVariable(state,
                                "Solar Collector Heat Transfer Energy",
                                Constant::Units::J,
                                this->Energy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::SolarWater,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::HeatProduced);
        } else if (this->Type == DataPlant::PlantEquipmentType::SolarCollectorICS) {

            SetupOutputVariable(state,
                                "Solar Collector Transmittance Absorptance Product",
                                Constant::Units::None,
                                this->TauAlpha,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);

            SetupOutputVariable(state,
                                "Solar Collector Overall Top Heat Loss Coefficient",
                                Constant::Units::W_m2C,
                                this->UTopLoss,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);

            SetupOutputVariable(state,
                                "Solar Collector Absorber Plate Temperature",
                                Constant::Units::C,
                                this->TempOfAbsPlate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);

            SetupOutputVariable(state,
                                "Solar Collector Storage Water Temperature",
                                Constant::Units::C,
                                this->TempOfWater,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);

            SetupOutputVariable(state,
                                "Solar Collector Thermal Efficiency",
                                Constant::Units::None,
                                this->Efficiency,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);

            SetupOutputVariable(state,
                                "Solar Collector Storage Heat Transfer Rate",
                                Constant::Units::W,
                                this->StoredHeatRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);

            SetupOutputVariable(state,
                                "Solar Collector Storage Heat Transfer Energy",
                                Constant::Units::J,
                                this->StoredHeatEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::SolarWater,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::HeatProduced);

            SetupOutputVariable(state,
                                "Solar Collector Skin Heat Transfer Rate",
                                Constant::Units::W,
                                this->SkinHeatLossRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);

            SetupOutputVariable(state,
                                "Solar Collector Skin Heat Transfer Energy",
                                Constant::Units::J,
                                this->CollHeatLossEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::SolarWater,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::HeatProduced);

            SetupOutputVariable(state,
                                "Solar Collector Heat Transfer Rate",
                                Constant::Units::W,
                                this->HeatRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);

            SetupOutputVariable(state,
                                "Solar Collector Heat Transfer Energy",
                                Constant::Units::J,
                                this->HeatEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::SolarWater,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::HeatProduced);
        }
    }

    void CollectorData::simulate(EnergyPlusData &state,
                                 [[maybe_unused]] const PlantLocation &calledFromLocation,
                                 [[maybe_unused]] bool const FirstHVACIteration,
                                 [[maybe_unused]] Real64 &CurLoad,
                                 [[maybe_unused]] bool const RunFlag)
    {
        this->initialize(state);

        switch (this->Type) {
            // Select and CALL models based on collector type
        case DataPlant::PlantEquipmentType::SolarCollectorFlatPlate: {
            this->CalcSolarCollector(state);
        } break;
        case DataPlant::PlantEquipmentType::SolarCollectorICS: {
            this->CalcICSSolarCollector(state);
        } break;
        default: {
            assert(false); // LCOV_EXCL_LINE
        } break;
        }

        this->update(state);

        this->report(state);
    }

    void CollectorData::initialize(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   January 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Initializes the solar collector object during the plant simulation.

        // METHODOLOGY EMPLOYED:
        // Inlet and outlet nodes are initialized.  The maximum collector flow rate is requested.

        static constexpr std::string_view RoutineName("InitSolarCollector");
        Real64 constexpr BigNumber(9999.9); // Component desired mass flow rate

        if (!state.dataGlobal->SysSizingCalc && this->InitSizing) {
            PlantUtilities::RegisterPlantCompDesignFlow(state, this->InletNode, this->VolFlowRateMax);
            this->InitSizing = false;
        }

        if (state.dataGlobal->BeginEnvrnFlag && this->Init) {
            // Clear node initial conditions
            if (this->VolFlowRateMax > 0) {
                Real64 rho = this->plantLoc.loop->glycol->getDensity(state, Constant::InitConvTemp, RoutineName);

                this->MassFlowRateMax = this->VolFlowRateMax * rho;
            } else {
                this->MassFlowRateMax = BigNumber;
            }

            PlantUtilities::InitComponentNodes(state, 0.0, this->MassFlowRateMax, this->InletNode, this->OutletNode);

            this->Init = false;

            if (this->InitICS) {
                this->TempOfWater = 20.0;
                this->SavedTempOfWater = this->TempOfWater;
                this->SavedTempOfAbsPlate = this->TempOfWater;
                this->TempOfAbsPlate = this->TempOfWater;
                this->TempOfInnerCover = this->TempOfWater;
                this->TempOfOuterCover = this->TempOfWater;
                this->SavedTempOfInnerCover = this->TempOfWater;
                this->SavedTempOfOuterCover = this->TempOfWater;
                this->SavedTempCollectorOSCM = this->TempOfWater;
            }
        }

        if (!state.dataGlobal->BeginEnvrnFlag) {
            this->Init = true;
        }

        if (this->SetDiffRadFlag && this->InitICS) {
            // calculates the sky and ground reflective diffuse radiation optical properties (only one time)
            int SurfNum = this->Surface;
            int ParamNum = this->Parameters;

            this->Tilt = state.dataSurface->Surface(SurfNum).Tilt;
            this->TiltR2V = std::abs(90.0 - Tilt);
            this->CosTilt = std::cos(Tilt * Constant::DegToRad);
            this->SinTilt = std::sin(1.8 * Tilt * Constant::DegToRad);

            // Diffuse reflectance of the cover for solar radiation diffusely reflected back from the absorber
            // plate to the cover.  The diffuse solar radiation reflected back from the absorber plate to the
            // cover is represented by the 60 degree equivalent incident angle.  This diffuse reflectance is
            // used to calculate the transmittance - absorptance product (Duffie and Beckman, 1991)
            Real64 Theta = 60.0 * Constant::DegToRad;
            Real64 TransSys = 0.0;
            Real64 RefSys = 0.0;
            Real64 AbsCover1 = 0.0;
            Real64 AbsCover2 = 0.0;
            Real64 RefSysDiffuse = 0.0;
            this->CalcTransRefAbsOfCover(state, Theta, TransSys, RefSys, AbsCover1, AbsCover2, true, RefSysDiffuse);
            this->RefDiffInnerCover = RefSysDiffuse;

            // transmittance-absorptance product normal incident:
            Theta = 0.0;
            this->CalcTransRefAbsOfCover(state, Theta, TransSys, RefSys, AbsCover1, AbsCover2);

            // transmittance-absorptance product for sky diffuse radiation.  Uses equivalent incident angle
            // of sky radiation (radians), and is calculated according to Brandemuehl and Beckman (1980):
            Theta = (59.68 - 0.1388 * Tilt + 0.001497 * pow_2(Tilt)) * Constant::DegToRad;
            this->CalcTransRefAbsOfCover(state, Theta, TransSys, RefSys, AbsCover1, AbsCover2);
            this->TauAlphaSkyDiffuse = TransSys * state.dataSolarCollectors->Parameters(ParamNum).AbsorOfAbsPlate /
                                       (1.0 - (1.0 - state.dataSolarCollectors->Parameters(ParamNum).AbsorOfAbsPlate) * this->RefDiffInnerCover);
            this->CoversAbsSkyDiffuse[0] = AbsCover1;
            this->CoversAbsSkyDiffuse[1] = AbsCover2;

            // transmittance-absorptance product for ground diffuse radiation.  Uses equivalent incident angle
            // of ground radiation (radians), and is calculated according to Brandemuehl and Beckman (1980):
            Theta = (90.0 - 0.5788 * Tilt + 0.002693 * pow_2(Tilt)) * Constant::DegToRad;
            this->CalcTransRefAbsOfCover(state, Theta, TransSys, RefSys, AbsCover1, AbsCover2);
            this->TauAlphaGndDiffuse = TransSys * state.dataSolarCollectors->Parameters(ParamNum).AbsorOfAbsPlate /
                                       (1.0 - (1.0 - state.dataSolarCollectors->Parameters(ParamNum).AbsorOfAbsPlate) * this->RefDiffInnerCover);
            this->CoversAbsGndDiffuse[0] = AbsCover1;
            this->CoversAbsGndDiffuse[1] = AbsCover2;

            this->SetDiffRadFlag = false;
        }

        this->InletTemp = state.dataLoopNodes->Node(this->InletNode).Temp;

        this->MassFlowRate = this->MassFlowRateMax;

        // Request the mass flow rate from the plant component flow utility routine
        PlantUtilities::SetComponentFlowRate(state, this->MassFlowRate, this->InletNode, this->OutletNode, this->plantLoc);

        if (this->InitICS) {

            Real64 timeElapsed =
                state.dataGlobal->HourOfDay + state.dataGlobal->TimeStep * state.dataGlobal->TimeStepZone + state.dataHVACGlobal->SysTimeElapsed;

            if (this->TimeElapsed != timeElapsed) {
                // The simulation has advanced to the next system timestep.  Save conditions from the end of the previous
                // system timestep for use as initial condition of each iteration that does not advance system timestep.
                this->SavedTempOfWater = this->TempOfWater;
                this->SavedTempOfAbsPlate = this->TempOfAbsPlate;
                this->SavedTempOfInnerCover = this->TempOfInnerCover;
                this->SavedTempOfOuterCover = this->TempOfOuterCover;
                if (this->OSCM_ON) {
                    this->SavedTempCollectorOSCM = state.dataHeatBal->ExtVentedCavity(this->VentCavIndex).Tbaffle;
                }
                this->TimeElapsed = timeElapsed;
            }
        }
    }

    void CollectorData::CalcSolarCollector(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   January 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates the heat gain (or loss), outlet temperature, and solar energy conversion efficiency for a flat-plate
        // solar collector when there is a fluid flow.  For the no flow condition, the fluid stagnation temperature is
        // calculated as the outlet temperature.  Glazed and unglazed collectors are both handled.

        // METHODOLOGY EMPLOYED:
        // Calculation is performed using the methodology described in the ASHRAE standards and references below.  Measured
        // collector performance coefficients (available from the Solar Rating & Certification Corporation, for example)
        // are modified from the test conditions to match the actual optical (incident angle modifier) and thermal (flow rate
        // modifier) conditions.  Water is assumed to be the heat transfer fluid.

        // REFERENCES:
        // ASHRAE Standard 93-1986 (RA 91), "Methods of Testing to Determine the Thermal Performance of Solar Collectors".
        // ASHRAE Standard 96-1980 (RA 89), "Methods of Testing to Determine the Thermal Performance of Unglazed Flat-Plate
        //   Liquid-Type Solar Collectors".
        // Duffie, J. A., and Beckman, W. A.  Solar Engineering of Thermal Processes, Second Edition.  Wiley-Interscience:
        //   New York (1991).

        // NOTES:
        // This subroutine has been validated against the TRNSYS Type 1 flat-plate solar collector module.  Results are
        // identical except for slight differences at extreme incident angles (>80 degrees) and extreme surface tilts (<20
        // degrees).  The differences are due to the fact that Type 1 does not prevent the *component* incident angle
        // modifiers from being less than zero.  There is an effect on the net incident angle modifier if one or more
        // components are less than zero but the net adds up to greater than zero.  The EnergyPlus subroutine, on the other
        // hand, requires each component incident angle modifier always to be greater than zero.

        static constexpr std::string_view RoutineName("CalcSolarCollector");
        Real64 efficiency = 0.0; // Thermal efficiency of solar energy conversion

        int SurfNum = this->Surface;
        int ParamNum = this->Parameters;
        Real64 incidentAngleModifier; // Net incident angle modifier combining beam, sky, and ground radiation

        // Calculate incident angle modifier
        if (state.dataHeatBal->SurfQRadSWOutIncident(SurfNum) > 0.0) {
            // Equivalent incident angle of sky radiation (radians)
            Real64 ThetaBeam = std::acos(state.dataHeatBal->SurfCosIncidenceAngle(SurfNum));

            // Calculate equivalent incident angles for sky and ground radiation according to Brandemuehl and Beckman (1980)
            // Surface tilt angle (degrees)
            Real64 tilt = state.dataSurface->Surface(SurfNum).Tilt;

            // Equivalent incident angle of sky radiation (radians)
            Real64 ThetaSky = (59.68 - 0.1388 * tilt + 0.001497 * pow_2(tilt)) * Constant::DegToRad;

            // Equivalent incident angle of ground radiation (radians)
            Real64 ThetaGnd = (90.0 - 0.5788 * tilt + 0.002693 * pow_2(tilt)) * Constant::DegToRad;

            incidentAngleModifier =
                (state.dataHeatBal->SurfQRadSWOutIncidentBeam(SurfNum) * state.dataSolarCollectors->Parameters(ParamNum).IAM(state, ThetaBeam) +
                 state.dataHeatBal->SurfQRadSWOutIncidentSkyDiffuse(SurfNum) * state.dataSolarCollectors->Parameters(ParamNum).IAM(state, ThetaSky) +
                 state.dataHeatBal->SurfQRadSWOutIncidentGndDiffuse(SurfNum) * state.dataSolarCollectors->Parameters(ParamNum).IAM(state, ThetaGnd)) /
                state.dataHeatBal->SurfQRadSWOutIncident(SurfNum);
        } else {
            incidentAngleModifier = 0.0;
        }

        // Inlet temperature from plant (C)
        Real64 inletTemp = this->InletTemp;

        // Mass flow rate through collector (kg/s)
        Real64 massFlowRate = this->MassFlowRate;

        // Specific heat of collector fluid (J/kg-K)
        Real64 Cp = this->plantLoc.loop->glycol->getSpecificHeat(state, inletTemp, RoutineName);

        // Gross area of collector (m2)
        Real64 area = state.dataSurface->Surface(SurfNum).Area;

        // = MassFlowRate * Cp / Area
        Real64 mCpA = massFlowRate * Cp / area;

        // = MassFlowRateTest * Cp / Area (tested area)
        Real64 mCpATest =
            state.dataSolarCollectors->Parameters(ParamNum).TestMassFlowRate * Cp / state.dataSolarCollectors->Parameters(this->Parameters).Area;

        int Iteration = 1;

        // Outlet temperature or stagnation temperature in the collector (C)
        Real64 outletTemp = 0.0;

        // Outlet temperature saved from previous iteration for convergence check (C)
        Real64 OutletTempPrev = 999.9; // Set to a ridiculous number so that DO loop runs at least once

        // Heat gain or loss to collector fluid (W)
        Real64 Q = 0.0;

        while (std::abs(outletTemp - OutletTempPrev) > state.dataHeatBal->TempConvergTol) { // Check for temperature convergence

            OutletTempPrev = outletTemp; // Save previous outlet temperature

            // Modifier for test correlation type:  INLET, AVERAGE, or OUTLET
            Real64 TestTypeMod = 0.0;

            // FR * ULoss "prime" for test conditions = (eff1 + eff2 * deltaT)
            Real64 FRULpTest = 0.0;

            // Modify coefficients depending on test correlation type
            switch (state.dataSolarCollectors->Parameters(ParamNum).TestType) {
            case TestTypeEnum::INLET: {
                FRULpTest = state.dataSolarCollectors->Parameters(ParamNum).eff1 +
                            state.dataSolarCollectors->Parameters(ParamNum).eff2 * (inletTemp - state.dataSurface->SurfOutDryBulbTemp(SurfNum));
                TestTypeMod = 1.0;
            } break;
            case TestTypeEnum::AVERAGE: {
                FRULpTest = state.dataSolarCollectors->Parameters(ParamNum).eff1 +
                            state.dataSolarCollectors->Parameters(ParamNum).eff2 *
                                ((inletTemp + outletTemp) * 0.5 - state.dataSurface->SurfOutDryBulbTemp(SurfNum));
                TestTypeMod = 1.0 / (1.0 - FRULpTest / (2.0 * mCpATest));
            } break;
            case TestTypeEnum::OUTLET: {
                FRULpTest = state.dataSolarCollectors->Parameters(ParamNum).eff1 +
                            state.dataSolarCollectors->Parameters(ParamNum).eff2 * (outletTemp - state.dataSurface->SurfOutDryBulbTemp(SurfNum));
                TestTypeMod = 1.0 / (1.0 - FRULpTest / mCpATest);
            } break;
            default:
                break;
            }

            // FR * tau * alpha at normal incidence = Y-intercept of collector efficiency
            Real64 FRTAN = state.dataSolarCollectors->Parameters(ParamNum).eff0 * TestTypeMod;

            // FR * ULoss = 1st order coefficient of collector efficiency
            Real64 FRUL = state.dataSolarCollectors->Parameters(ParamNum).eff1 * TestTypeMod;

            // FR * ULoss / T = 2nd order coefficient of collector efficiency
            Real64 FRULT = state.dataSolarCollectors->Parameters(ParamNum).eff2 * TestTypeMod;
            FRULpTest *= TestTypeMod;

            if (massFlowRate > 0.0) { // Calculate efficiency and heat transfer with flow

                // Modifier for flow rate different from test flow rate
                Real64 FlowMod = 0.0;

                // F prime * ULoss for test conditions = collector efficiency factor * overall loss coefficient
                Real64 FpULTest;

                if ((1.0 + FRULpTest / mCpATest) > 0.0) {
                    FpULTest = -mCpATest * std::log(1.0 + FRULpTest / mCpATest);
                } else {
                    FpULTest = FRULpTest; // Avoid LOG( <0 )
                }

                if ((-FpULTest / mCpA) < 700.0) {
                    FlowMod = mCpA * (1.0 - std::exp(-FpULTest / mCpA));
                } else { // avoid EXP(too large #)
                         // FlowMod = FlowMod; // Self-assignment commented out
                }
                if ((-FpULTest / mCpATest) < 700.0) {
                    FlowMod /= (mCpATest * (1.0 - std::exp(-FpULTest / mCpATest)));
                } else {
                    // FlowMod = FlowMod; // Self-assignment commented out
                }

                // Calculate fluid heat gain (or loss)
                // Heat loss is possible if there is no incident radiation and fluid is still flowing.
                Q = (FRTAN * incidentAngleModifier * state.dataHeatBal->SurfQRadSWOutIncident(SurfNum) +
                     FRULpTest * (inletTemp - state.dataSurface->SurfOutDryBulbTemp(SurfNum))) *
                    area * FlowMod;

                outletTemp = inletTemp + Q / (massFlowRate * Cp);

                // CR 7877 bound unreasonable result
                if (outletTemp < -100) {
                    outletTemp = -100.0;
                    Q = massFlowRate * Cp * (outletTemp - inletTemp);
                }
                if (outletTemp > 200) {
                    outletTemp = 200.0;
                    Q = massFlowRate * Cp * (outletTemp - inletTemp);
                }

                if (state.dataHeatBal->SurfQRadSWOutIncident(SurfNum) > 0.0) { // Calculate thermal efficiency
                    // NOTE: Efficiency can be > 1 if Q > SurfQRadSWOutIncident because of favorable delta T, i.e. warm outdoor temperature
                    efficiency =
                        Q / (state.dataHeatBal->SurfQRadSWOutIncident(SurfNum) * area); // Q has units of W; SurfQRadSWOutIncident has units of W/m2
                } else {
                    efficiency = 0.0;
                }

            } else { // Calculate stagnation temperature of fluid in collector (no flow)
                Q = 0.0;
                efficiency = 0.0;

                // Calculate temperature of stagnant fluid in collector
                Real64 A = -FRULT;
                Real64 B = -FRUL + 2.0 * FRULT * state.dataSurface->SurfOutDryBulbTemp(SurfNum);
                Real64 C = -FRULT * pow_2(state.dataSurface->SurfOutDryBulbTemp(SurfNum)) + FRUL * state.dataSurface->SurfOutDryBulbTemp(SurfNum) -
                           FRTAN * incidentAngleModifier * state.dataHeatBal->SurfQRadSWOutIncident(SurfNum);
                Real64 qEquation = (pow_2(B) - 4.0 * A * C);
                if (qEquation < 0.0) {
                    if (this->ErrIndex == 0) {
                        ShowSevereMessage(state,
                                          format("CalcSolarCollector: {}=\"{}\", possible bad input coefficients.",
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->Type)],
                                                 this->Name));
                        ShowContinueError(state,
                                          "...coefficients cause negative quadratic equation part in calculating temperature of stagnant fluid.");
                        ShowContinueError(state, "...examine input coefficients for accuracy. Calculation will be treated as linear.");
                    }
                    ShowRecurringSevereErrorAtEnd(state,
                                                  format("CalcSolarCollector: {}=\"{}\", coefficient error continues.",
                                                         DataPlant::PlantEquipTypeNames[static_cast<int>(this->Type)],
                                                         this->Name),
                                                  this->ErrIndex,
                                                  qEquation,
                                                  qEquation);
                }
                if (FRULT == 0.0 || qEquation < 0.0) { // Linear, 1st order solution
                    outletTemp = state.dataSurface->SurfOutDryBulbTemp(SurfNum) -
                                 FRTAN * incidentAngleModifier * state.dataHeatBal->SurfQRadSWOutIncident(SurfNum) / FRUL;
                } else { // Quadratic, 2nd order solution
                    outletTemp = (-B + std::sqrt(qEquation)) / (2.0 * A);
                }
            }

            if (state.dataSolarCollectors->Parameters(ParamNum).TestType == TestTypeEnum::INLET) {
                break; // Inlet temperature test correlations do not need to iterate
            }

            if (Iteration > 100) {
                if (this->IterErrIndex == 0) {
                    ShowWarningMessage(state,
                                       format("CalcSolarCollector: {}=\"{}\":  Solution did not converge.",
                                              DataPlant::PlantEquipTypeNames[static_cast<int>(this->Type)],
                                              this->Name));
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               format("CalcSolarCollector: {}=\"{}\", solution not converge error continues.",
                                                      DataPlant::PlantEquipTypeNames[static_cast<int>(this->Type)],
                                                      this->Name),
                                               this->IterErrIndex);
                break;
            } else {
                ++Iteration;
            }

        } // Check for temperature convergence

        this->IncidentAngleModifier = incidentAngleModifier;
        this->Power = Q;
        this->HeatGain = max(Q, 0.0);
        this->HeatLoss = min(Q, 0.0);
        this->OutletTemp = outletTemp;
        this->Efficiency = efficiency;
    }

    Real64 ParametersData::IAM(EnergyPlusData &state, Real64 const IncidentAngle)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   December 2003
        //       MODIFIED       Sept 2008, BG cut off IAM beyond 60 degrees.
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates the incident angle modifier based on the solar collector parameters.  Both first and second order
        // correlations are allowed.

        // METHODOLOGY EMPLOYED:
        // A simple function.

        // REFERENCES:
        // ASHRAE Standard 93-1986 (RA 91), "Methods of Testing to Determine the Thermal Performance of Solar Collectors".
        // ASHRAE Standard 96-1980 (RA 89), "Methods of Testing to Determine the Thermal Performance of Unglazed Flat-Plate
        //   Liquid-Type Solar Collectors".
        // Duffie, J. A., and Beckman, W. A.  Solar Engineering of Thermal Processes, Second Edition.  Wiley-Interscience:
        //   New York (1991).

        Real64 IAM;

        // cut off IAM for angles greater than 60 degrees. (CR 7534)
        Real64 CutoffAngle = 60.0 * Constant::DegToRad;
        if (std::abs(IncidentAngle) > CutoffAngle) { // cut off, model curves not robust beyond cutoff
            // curves from FSEC/SRCC testing are only certified to 60 degrees, larger angles can cause numerical problems in curves
            IAM = 0.0;
        } else {

            Real64 s = (1.0 / std::cos(IncidentAngle)) - 1.0;

            IAM = 1.0 + this->iam1 * s + this->iam2 * pow_2(s);
            IAM = max(IAM, 0.0); // Never allow to be less than zero, but greater than one is a possibility

            if (IAM > 10.0) { // Greater than 10 is probably not a possibility
                ShowSevereError(
                    state,
                    format(
                        "IAM Function: SolarCollectorPerformance:FlatPlate = {}:  Incident Angle Modifier is out of bounds due to bad coefficients.",
                        this->Name));
                ShowContinueError(state, format("Coefficient 2 of Incident Angle Modifier = {}", this->iam1));
                ShowContinueError(state, format("Coefficient 3 of Incident Angle Modifier = {}", this->iam2));
                ShowContinueError(state, format("Calculated Incident Angle Modifier = {}", IAM));
                ShowContinueError(state, "Expected Incident Angle Modifier should be approximately 1.5 or less.");
                ShowFatalError(state, "Errors in SolarCollectorPerformance:FlatPlate input.");
            }

        } // not greater than cut off angle

        return IAM;
    }

    void CollectorData::CalcICSSolarCollector(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Bereket Nigusse, FSEC/UCF
        //       DATE WRITTEN   February 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates the heat transferred (gain or loss), energy stored, skin heat loss, outlet temperature, solar energy
        // conversion efficiency, and transmittance-absorptance product of an ICS solar collector.

        // METHODOLOGY EMPLOYED:
        // The governing equations for the absorber and collector water heat balance equations are solved simultaneously.
        // The two coupled first ODE are solved analytically.
        // The transmittance-absorptance product of the collector cover-absorber system is calculated using ray tracing
        // method according to Duffie and Beckman(1991).
        // REFERENCES:
        // Duffie, J. A., and Beckman, W. A.  Solar Engineering of Thermal Processes, 2nd. Edition.  Wiley-Interscience:
        // New York (1991).
        // NOTES:

        static constexpr std::string_view RoutineName("CalcICSSolarCollector");

        int SurfNum = this->Surface;
        int ParamNum = this->Parameters;
        Real64 SecInTimeStep = state.dataHVACGlobal->TimeStepSysSec;
        Real64 TempWater = this->SavedTempOfWater;
        Real64 TempAbsPlate = this->SavedTempOfAbsPlate;
        Real64 TempOutdoorAir = state.dataSurface->SurfOutDryBulbTemp(SurfNum);

        Real64 TempOSCM; // Otherside condition model temperature [C]
        if (this->OSCM_ON) {
            TempOSCM = this->SavedTempCollectorOSCM;
        } else {
            TempOSCM = TempOutdoorAir;
        }

        // Calculate transmittance-absorptance product of the system
        // Incident angle of beam radiation (radians)
        Real64 ThetaBeam = std::acos(state.dataHeatBal->SurfCosIncidenceAngle(SurfNum));
        this->CalcTransAbsorProduct(state, ThetaBeam);

        Real64 inletTemp = this->InletTemp;

        Real64 massFlowRate = this->MassFlowRate;

        // Specific heat of collector fluid (J/kg-K)
        Real64 Cpw = this->plantLoc.loop->glycol->getSpecificHeat(state, inletTemp, RoutineName);

        // density of collector fluid (kg/m3)
        Real64 Rhow = this->plantLoc.loop->glycol->getDensity(state, inletTemp, RoutineName);

        // calculate heat transfer coefficients and covers temperature:
        this->CalcHeatTransCoeffAndCoverTemp(state);

        // Calc convection heat transfer coefficient between the absorber plate and water:

        // convection coeff between absorber plate and water [W/m2K]
        Real64 hConvCoefA2W =
            EnergyPlus::SolarCollectors::CollectorData::CalcConvCoeffAbsPlateAndWater(state, TempAbsPlate, TempWater, this->Length, this->TiltR2V);
        Real64 TempWaterOld = TempWater;
        Real64 TempAbsPlateOld = TempAbsPlate;

        // flag if the absorber has thermal mass or not
        bool AbsPlateMassFlag;

        Real64 a1; // coefficient of ODE for absorber temperature Tp
        Real64 a2; // coefficient of ODE for absorber temperature Tw
        Real64 a3; // constant term of ODE for absorber temperature

        // Gross area of collector (m2)
        Real64 area = state.dataSolarCollectors->Parameters(ParamNum).Area;

        if (state.dataSolarCollectors->Parameters(ParamNum).ThermalMass > 0.0) {
            AbsPlateMassFlag = true;

            // thermal mass of the absorber plate [J/K]
            Real64 ap = state.dataSolarCollectors->Parameters(ParamNum).ThermalMass * area;
            a1 = -area * (hConvCoefA2W + this->UTopLoss) / ap;
            a2 = area * hConvCoefA2W / ap;
            a3 = area * (this->TauAlpha * state.dataHeatBal->SurfQRadSWOutIncident(SurfNum) + this->UTopLoss * TempOutdoorAir) / ap;
        } else {
            AbsPlateMassFlag = false;
            a1 = -area * (hConvCoefA2W + this->UTopLoss);
            a2 = area * hConvCoefA2W;
            a3 = area * (this->TauAlpha * state.dataHeatBal->SurfQRadSWOutIncident(SurfNum) + this->UTopLoss * TempOutdoorAir);
        }

        // thermal mass of the collector water [J/K]
        Real64 aw = state.dataSolarCollectors->Parameters(ParamNum).Volume * Rhow * Cpw;

        // coefficient of ODE for water temperature Tp
        Real64 b1 = area * hConvCoefA2W / aw;

        // coefficient of ODE for water temperature Tw
        Real64 b2 = -(area * (hConvCoefA2W + this->UbLoss + this->UsLoss) + massFlowRate * Cpw) / aw;

        // constant term of ODE for water temperature
        Real64 b3 = (area * (this->UbLoss * TempOSCM + this->UsLoss * TempOutdoorAir) + massFlowRate * Cpw * inletTemp) / aw;

        EnergyPlus::SolarCollectors::CollectorData::ICSCollectorAnalyticalSolution(
            state, SecInTimeStep, a1, a2, a3, b1, b2, b3, TempAbsPlateOld, TempWaterOld, TempAbsPlate, TempWater, AbsPlateMassFlag);

        this->SkinHeatLossRate = area * (this->UTopLoss * (TempOutdoorAir - TempAbsPlate) + this->UsLoss * (TempOutdoorAir - TempWater) +
                                         this->UbLoss * (TempOSCM - TempWater));
        this->StoredHeatRate = aw * (TempWater - TempWaterOld) / SecInTimeStep;

        // heat gain rate (W)
        Real64 QHeatRate = massFlowRate * Cpw * (TempWater - inletTemp);
        this->HeatRate = QHeatRate;
        this->HeatGainRate = max(0.0, QHeatRate);

        Real64 outletTemp = TempWater;
        this->OutletTemp = outletTemp;
        this->TempOfWater = TempWater;
        this->TempOfAbsPlate = TempAbsPlate;

        Real64 efficiency = 0.0; // Thermal efficiency of solar energy conversion
        if (state.dataHeatBal->SurfQRadSWOutIncident(SurfNum) > 0.0) {
            efficiency = (this->HeatGainRate + this->StoredHeatRate) / (state.dataHeatBal->SurfQRadSWOutIncident(SurfNum) * area);
            if (efficiency < 0.0) {
                efficiency = 0.0;
            }
        }
        this->Efficiency = efficiency;
    }

    void CollectorData::ICSCollectorAnalyticalSolution(EnergyPlusData &state,
                                                       Real64 const SecInTimeStep,     // seconds in a time step
                                                       Real64 const a1,                // coefficient of ODE for Tp
                                                       Real64 const a2,                // coefficient of ODE for Tp
                                                       Real64 const a3,                // coefficient of ODE for Tp
                                                       Real64 const b1,                // coefficient of ODE for TW
                                                       Real64 const b2,                // coefficient of ODE for TW
                                                       Real64 const b3,                // coefficient of ODE for TW
                                                       Real64 const TempAbsPlateOld,   // absorber plate temperature at previous time step [C]
                                                       Real64 const TempWaterOld,      // collector water temperature at previous time step [C]
                                                       Real64 &TempAbsPlate,           // absorber plate temperature at current time step [C]
                                                       Real64 &TempWater,              // collector water temperature at current time step [C]
                                                       bool const AbsorberPlateHasMass // flag for absorber thermal mass
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Bereket Nigusse, FSEC/UCF
        //       DATE WRITTEN   February 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates the absorber plate and collector water temperatures.
        // METHODOLOGY EMPLOYED:
        // Analytical method: Solves the coupled absorber plate and collector water energy balance
        // equations.  The two non-homogeneous ordinary differential equations of the form.
        //          Tp' = a1*Tp + a2*Tw + a3.
        //          Tw' = b1*Tp + b2*Tw + b3.
        // The general solution of these coupled equation with real routes has the following form:
        //          Tp = ConstantC1*exp(lamda1*t) + ConstantC2*exp(lamda2*t) + ConstOfTpSln
        //          Tw = r1*ConstantC2*exp(lamda1*t) + r2*ConstantC2*exp(lamda2*t) + ConstOfTwSln

        if (AbsorberPlateHasMass) {

            // coefficients of quadratic equation a*m2+b*m+c=0
            Real64 a = 1.0;
            Real64 b = -(a1 + b2);
            Real64 c = a1 * b2 - a2 * b1;
            Real64 BSquareM4TimesATimesC = pow_2(b) - 4.0 * a * c;

            if (BSquareM4TimesATimesC > 0.0) {

                // the real roots of the quadratic equation
                Real64 lamda1 = (-b + std::sqrt(BSquareM4TimesATimesC)) / (2.0 * a);
                Real64 lamda2 = (-b - std::sqrt(BSquareM4TimesATimesC)) / (2.0 * a);

                // the particular solution for the ODE
                Real64 ConstOfTpSln = (-a3 * b2 + b3 * a2) / c;
                Real64 ConstOfTwSln = (-a1 * b3 + b1 * a3) / c;

                // ratio of the ODE solution constant coefficients
                Real64 r1 = (lamda1 - a1) / a2;
                Real64 r2 = (lamda2 - a1) / a2;

                // coefficients of the ODE solution
                Real64 ConstantC2 = (TempWaterOld + r1 * ConstOfTpSln - r1 * TempAbsPlateOld - ConstOfTwSln) / (r2 - r1);
                Real64 ConstantC1 = (TempAbsPlateOld - ConstOfTpSln - ConstantC2);

                TempAbsPlate = ConstantC1 * std::exp(lamda1 * SecInTimeStep) + ConstantC2 * std::exp(lamda2 * SecInTimeStep) + ConstOfTpSln;
                TempWater = r1 * ConstantC1 * std::exp(lamda1 * SecInTimeStep) + r2 * ConstantC2 * std::exp(lamda2 * SecInTimeStep) + ConstOfTwSln;

            } else { // this should never occur
                ShowSevereError(
                    state, "ICSCollectorAnalyticalSoluton: Unanticipated differential equation coefficient - report to EnergyPlus Development Team");
                ShowFatalError(state, "Program terminates due to above conditions.");
            }
        } else {
            // In the absence of absorber plate thermal mass, only the collector water heat balance has a
            // differential equation of the form: Tw' = b1*Tp + b2*Tw + b3. The absorber plate energy balance
            // equation in the absence of thermal mass is a steady state form:  b1*Tp + b2*Tw + b3 = 0
            Real64 b = b2 - b1 * (a2 / a1);
            Real64 c = b3 - b1 * (a3 / a1);
            TempWater = (TempWaterOld + c / b) * std::exp(b * SecInTimeStep) - c / b;
            TempAbsPlate = -(a2 * TempWater + a3) / a1;
        }
    }

    void CollectorData::CalcTransAbsorProduct(EnergyPlusData &state, Real64 const IncidAngle)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Bereket A Nigusse
        //       DATE WRITTEN   February 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates transmittance-absorptance product and the fraction of total solar radiation
        // absorbed by each cover of a multicover ICS solar collector.

        // METHODOLOGY EMPLOYED:
        // Uses a ray tracing method.

        // REFERENCES:
        // Duffie, J. A., and Beckman, W. A.  Solar Engineering of Thermal Processes, Second Edition.
        // Wiley-Interscience: New York (1991).

        Real64 TransSys = 1.0;  // cover system solar transmittance
        Real64 ReflSys = 0.0;   // cover system solar reflectance
        Real64 AbsCover1 = 0.0; // Inner cover solar absorbtance
        Real64 AbsCover2 = 0.0; // Outer cover solar absorbtance
        Real64 TuaAlpha;        // weighted trans-abs product of system
        Real64 TuaAlphaBeam;    // trans-abs product of beam radiation
        this->CoverAbs[0] = 0.0;
        this->CoverAbs[1] = 0.0;

        int SurfNum = this->Surface;
        int ParamNum = this->Parameters;

        if (state.dataHeatBal->SurfQRadSWOutIncident(SurfNum) > 0.0) {

            // cover system transmittance and reflectance from outer to inner cover
            this->CalcTransRefAbsOfCover(state, IncidAngle, TransSys, ReflSys, AbsCover1, AbsCover2);

            TuaAlphaBeam = TransSys * state.dataSolarCollectors->Parameters(ParamNum).AbsorOfAbsPlate /
                           (1.0 - (1.0 - state.dataSolarCollectors->Parameters(ParamNum).AbsorOfAbsPlate) * this->RefDiffInnerCover);

            this->TauAlphaBeam = max(0.0, TuaAlphaBeam);

            Array1D<Real64> CoversAbsBeam(2); // Inner and Outer Cover absorptance
            CoversAbsBeam(1) = AbsCover1;
            CoversAbsBeam(2) = AbsCover2;

            // calc total solar radiation weighted transmittance-absorptance product
            TuaAlpha = (state.dataHeatBal->SurfQRadSWOutIncidentBeam(SurfNum) * this->TauAlphaBeam +
                        state.dataHeatBal->SurfQRadSWOutIncidentSkyDiffuse(SurfNum) * this->TauAlphaSkyDiffuse +
                        state.dataHeatBal->SurfQRadSWOutIncidentGndDiffuse(SurfNum) * this->TauAlphaGndDiffuse) /
                       state.dataHeatBal->SurfQRadSWOutIncident(SurfNum);

            if (state.dataSolarCollectors->Parameters(ParamNum).NumOfCovers == 1) {
                // calc total solar radiation weighted cover absorptance
                this->CoverAbs[0] = (state.dataHeatBal->SurfQRadSWOutIncidentBeam(SurfNum) * CoversAbsBeam(1) +
                                     state.dataHeatBal->SurfQRadSWOutIncidentSkyDiffuse(SurfNum) * this->CoversAbsSkyDiffuse[0] +
                                     state.dataHeatBal->SurfQRadSWOutIncidentGndDiffuse(SurfNum) * this->CoversAbsGndDiffuse[0]) /
                                    state.dataHeatBal->SurfQRadSWOutIncident(SurfNum);

            } else if (state.dataSolarCollectors->Parameters(ParamNum).NumOfCovers == 2) {
                // Num = 1 represents outer cover and Num = 2 represents inner cover
                for (int Num = 0; Num < state.dataSolarCollectors->Parameters(ParamNum).NumOfCovers; ++Num) {
                    this->CoverAbs[Num] = (state.dataHeatBal->SurfQRadSWOutIncidentBeam(SurfNum) * CoversAbsBeam(Num + 1) +
                                           state.dataHeatBal->SurfQRadSWOutIncidentSkyDiffuse(SurfNum) * this->CoversAbsSkyDiffuse[Num] +
                                           state.dataHeatBal->SurfQRadSWOutIncidentGndDiffuse(SurfNum) * this->CoversAbsGndDiffuse[Num]) /
                                          state.dataHeatBal->SurfQRadSWOutIncident(SurfNum);
                }
            }

        } else {
            TuaAlpha = 0.0;
        }
        this->TauAlpha = TuaAlpha;
    }

    void CollectorData::CalcTransRefAbsOfCover(EnergyPlusData &state,
                                               Real64 const IncidentAngle,               // Angle of incidence (radians)
                                               Real64 &TransSys,                         // cover system solar transmittance
                                               Real64 &ReflSys,                          // cover system solar reflectance
                                               Real64 &AbsCover1,                        // Inner cover solar absorbtance
                                               Real64 &AbsCover2,                        // Outer cover solar absorbtance
                                               ObjexxFCL::Optional_bool_const InOUTFlag, // flag for calc. diffuse solar refl of cover from inside out
                                               ObjexxFCL::Optional<Real64> RefSysDiffuse // cover system solar reflectance from inner to outer cover
    ) const
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Bereket A Nigusse
        //       DATE WRITTEN   February 2012

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates the transmittance, reflectance, and absorptance of the collector covers based on
        // solar collector optical parameters specified.

        // METHODOLOGY EMPLOYED:
        // Uses a ray tracing method.

        // REFERENCES:
        // Duffie, J. A., and Beckman, W. A.  Solar Engineering of Thermal Processes, Second Edition.
        // Wiley-Interscience: New York (1991).

        Real64 constexpr AirRefIndex(1.0003); // refractive index of air

        Array1D<Real64> TransPara(2);    // cover transmittance parallel component
        Array1D<Real64> TransPerp(2);    // cover transmittance perpendicular component
        Array1D<Real64> ReflPara(2);     // cover reflectance parallel component
        Array1D<Real64> ReflPerp(2);     // cover reflectance Perpendicular component
        Array1D<Real64> AbsorPara(2);    // cover absorbtance parallel component
        Array1D<Real64> AbsorPerp(2);    // cover absorbtance Perpendicular component
        Array1D<Real64> TransAbsOnly(2); // cover transmittance with absorptance only considered

        TransPerp = 1.0;
        TransPara = 1.0;
        ReflPerp = 0.0;
        ReflPara = 0.0;
        AbsorPerp = 0.0;
        AbsorPara = 0.0;
        TransAbsOnly = 1.0;
        TransSys = 0.0;
        ReflSys = 0.0;
        AbsCover1 = 0.0;
        AbsCover2 = 0.0;

        bool DiffRefFlag; // flag for calc. diffuse refl of cover from inside to outside
        if (present(InOUTFlag)) {
            DiffRefFlag = InOUTFlag;
        } else {
            DiffRefFlag = false;
        }

        // get the incidence and refraction angles
        int ParamNum = this->Parameters;
        Real64 const sin_IncAngle(std::sin(IncidentAngle));

        for (int nCover = 1; nCover <= state.dataSolarCollectors->Parameters(ParamNum).NumOfCovers; ++nCover) {

            // refractive index of collector cover
            Real64 CoverRefrIndex = state.dataSolarCollectors->Parameters(ParamNum).RefractiveIndex[nCover - 1];

            // angle of refraction
            Real64 RefrAngle = std::asin(sin_IncAngle * AirRefIndex / CoverRefrIndex);

            // transmitted component with absorption only considered:
            TransAbsOnly(nCover) = std::exp(-state.dataSolarCollectors->Parameters(ParamNum).ExtCoefTimesThickness[nCover - 1] / std::cos(RefrAngle));

            // parallel reflected component of unpolarized solar radiation
            Real64 ParaRad;

            // Perpendicular reflected component of unpolarized solar radiation
            Real64 PerpRad;

            // parallel and perpendicular reflection components:
            if (IncidentAngle == 0.0) {
                ParaRad = pow_2((CoverRefrIndex - AirRefIndex) / (CoverRefrIndex + AirRefIndex));
                PerpRad = pow_2((CoverRefrIndex - AirRefIndex) / (CoverRefrIndex + AirRefIndex));
            } else {
                ParaRad = pow_2(std::tan(RefrAngle - IncidentAngle)) / pow_2(std::tan(RefrAngle + IncidentAngle));
                PerpRad = pow_2(std::sin(RefrAngle - IncidentAngle)) / pow_2(std::sin(RefrAngle + IncidentAngle));
            }

            // parallel and perpendicular transmitted components:
            TransPerp(nCover) =
                TransAbsOnly(nCover) * ((1.0 - PerpRad) / (1.0 + PerpRad)) * ((1.0 - pow_2(PerpRad)) / (1.0 - pow_2(PerpRad * TransAbsOnly(nCover))));
            TransPara(nCover) =
                TransAbsOnly(nCover) * ((1.0 - ParaRad) / (1.0 + ParaRad)) * ((1.0 - pow_2(ParaRad)) / (1.0 - pow_2(ParaRad * TransAbsOnly(nCover))));

            ReflPerp(nCover) =
                (PerpRad + (pow_2(1.0 - PerpRad) * pow_2(TransAbsOnly(nCover)) * PerpRad) / (1.0 - pow_2(PerpRad * TransAbsOnly(nCover))));
            ReflPara(nCover) =
                (ParaRad + (pow_2(1.0 - ParaRad) * pow_2(TransAbsOnly(nCover)) * ParaRad) / (1.0 - pow_2(ParaRad * TransAbsOnly(nCover))));

            AbsorPerp(nCover) = 1.0 - TransPerp(nCover) - ReflPerp(nCover);
            AbsorPara(nCover) = 1.0 - TransPara(nCover) - ReflPara(nCover);
        }

        // solar absorptance of the individual cover
        AbsCover1 = 0.5 * (AbsorPerp(1) + AbsorPara(1));
        if (state.dataSolarCollectors->Parameters(ParamNum).NumOfCovers == 2) {
            AbsCover2 = 0.5 * (AbsorPerp(2) + AbsorPara(2));
        }

        // calculate from outer to inner cover:
        TransSys =
            0.5 * (TransPerp(1) * TransPerp(2) / (1.0 - ReflPerp(1) * ReflPerp(2)) + TransPara(1) * TransPara(2) / (1.0 - ReflPara(1) * ReflPara(2)));
        ReflSys = 0.5 * (ReflPerp(1) + TransSys * ReflPerp(2) * TransPerp(1) / TransPerp(2) + ReflPara(1) +
                         TransSys * ReflPara(2) * TransPara(1) / TransPara(2));
        if (DiffRefFlag) {
            // calculate from inner to outer cover:

            // cover system solar transmittance from inner to outer cover
            Real64 TransSysDiff = 0.5 * (TransPerp(2) * TransPerp(1) / (1.0 - ReflPerp(2) * ReflPerp(1)) +
                                         TransPara(2) * TransPara(1) / (1.0 - ReflPara(2) * ReflPara(1)));
            RefSysDiffuse = 0.5 * (ReflPerp(2) + TransSysDiff * ReflPerp(1) * TransPerp(2) / TransPerp(1) + ReflPara(2) +
                                   TransSysDiff * ReflPara(1) * TransPara(2) / TransPara(1));
        }
    }

    void CollectorData::CalcHeatTransCoeffAndCoverTemp(EnergyPlusData &state) // Collector object number
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Bereket A Nigusse, FSEC/UCF
        //       DATE WRITTEN   February 2012

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates the various heat transfer coefficients, and collector cover temperatures.

        // METHODOLOGY EMPLOYED:

        // REFERENCES:
        // Duffie, J. A., and Beckman, W. A.  Solar Engineering of Thermal Processes, Second Edition.
        // Wiley-Interscience: New York (1991).

        Real64 tempnom;             // intermediate variable
        Real64 tempdenom;           // intermediate variable
        Real64 hRadCoefC2Sky;       // radiation coeff from collector to the sky [W/m2C]
        Real64 hRadCoefC2Gnd = 0.0; // radiation coeff from collector to the ground [W/m2C]
        Real64 hConvCoefA2C = 0.0;  // convection coeff. between abs plate and cover [W/m2C]
        Real64 hConvCoefC2C = 0.0;  // convection coeff. between covers [W/m2C]
        Real64 hConvCoefC2O = 0.0;  // convection coeff. between outer cover and the ambient [W/m2C]
        Real64 hRadCoefA2C = 0.0;   // radiation coeff. between abs plate and cover [W/m2C]
        Real64 hRadCoefC2C = 0.0;   // radiation coeff. between covers [W/m2C]
        Real64 hRadCoefC2O = 0.0;   // radiation coeff. between outer covers and the ambient [W/m2C]

        int ParamNum = this->Parameters;
        int NumCovers = state.dataSolarCollectors->Parameters(ParamNum).NumOfCovers;
        int SurfNum = this->Surface;

        Real64 TempAbsPlate = this->SavedTempOfAbsPlate;                        // absorber plate average temperature [C]
        Real64 TempInnerCover = this->SavedTempOfInnerCover;                    // inner cover average temperature [C]
        Real64 TempOuterCover = this->SavedTempOfOuterCover;                    // outer cover average temperature [C]
        Real64 TempOutdoorAir = state.dataSurface->SurfOutDryBulbTemp(SurfNum); // outdoor air temperature [C]

        Real64 EmissOfAbsPlate = state.dataSolarCollectors->Parameters(ParamNum).EmissOfAbsPlate;   // emissivity of absorber plate
        Real64 EmissOfOuterCover = state.dataSolarCollectors->Parameters(ParamNum).EmissOfCover[0]; // emissivity of outer cover
        Real64 EmissOfInnerCover = state.dataSolarCollectors->Parameters(ParamNum).EmissOfCover[1]; // emissivity of inner cover
        Real64 AirGapDepth = state.dataSolarCollectors->Parameters(ParamNum).CoverSpacing;          // characteristic length [m]

        if (NumCovers == 1) {
            // calc linearized radiation coefficient
            tempnom = Constant::StefanBoltzmann * ((TempAbsPlate + Constant::Kelvin) + (TempOuterCover + Constant::Kelvin)) *
                      (pow_2(TempAbsPlate + Constant::Kelvin) + pow_2(TempOuterCover + Constant::Kelvin));
            tempdenom = 1.0 / EmissOfAbsPlate + 1.0 / EmissOfOuterCover - 1.0;
            hRadCoefA2C = tempnom / tempdenom;
            hRadCoefC2C = 0.0;
            hConvCoefC2C = 0.0;
            // Calc convection heat transfer coefficient:
            hConvCoefA2C = EnergyPlus::SolarCollectors::CollectorData::CalcConvCoeffBetweenPlates(
                TempAbsPlate, TempOuterCover, AirGapDepth, this->CosTilt, this->SinTilt);
        } else if (NumCovers == 2) {
            for (int CoverNum = 1; CoverNum <= NumCovers; ++CoverNum) {
                if (CoverNum == 1) {
                    // calc linearized radiation coefficient
                    tempnom = Constant::StefanBoltzmann * ((TempAbsPlate + Constant::Kelvin) + (TempInnerCover + Constant::Kelvin)) *
                              (pow_2(TempAbsPlate + Constant::Kelvin) + pow_2(TempInnerCover + Constant::Kelvin));
                    tempdenom = 1.0 / EmissOfAbsPlate + 1.0 / EmissOfInnerCover - 1.0;
                    hRadCoefA2C = tempnom / tempdenom;
                    // Calc convection heat transfer coefficient:
                    hConvCoefA2C = EnergyPlus::SolarCollectors::CollectorData::CalcConvCoeffBetweenPlates(
                        TempAbsPlate, TempOuterCover, AirGapDepth, this->CosTilt, this->SinTilt);
                } else {
                    // calculate the linearized radiation coeff.
                    tempnom = Constant::StefanBoltzmann * ((TempInnerCover + Constant::Kelvin) + (TempOuterCover + Constant::Kelvin)) *
                              (pow_2(TempInnerCover + Constant::Kelvin) + pow_2(TempOuterCover + Constant::Kelvin));
                    tempdenom = 1.0 / EmissOfInnerCover + 1.0 / EmissOfOuterCover - 1.0;
                    hRadCoefC2C = tempnom / tempdenom;
                    // Calc convection heat transfer coefficient:
                    hConvCoefC2C = EnergyPlus::SolarCollectors::CollectorData::CalcConvCoeffBetweenPlates(
                        TempInnerCover, TempOuterCover, AirGapDepth, this->CosTilt, this->SinTilt);
                }
            }
        }

        // Calc collector outside surface convection heat transfer coefficient:
        hConvCoefC2O = 2.8 + 3.0 * state.dataSurface->SurfOutWindSpeed(SurfNum);

        // Calc linearized radiation coefficient between outer cover and the surrounding:
        tempnom = state.dataSurface->Surface(SurfNum).ViewFactorSky * EmissOfOuterCover * Constant::StefanBoltzmann *
                  ((TempOuterCover + Constant::Kelvin) + state.dataEnvrn->SkyTempKelvin) *
                  (pow_2(TempOuterCover + Constant::Kelvin) + pow_2(state.dataEnvrn->SkyTempKelvin));
        tempdenom = (TempOuterCover - TempOutdoorAir) / (TempOuterCover - state.dataEnvrn->SkyTemp);
        if (tempdenom < 0.0) {
            // use approximate linearized radiation coefficient
            hRadCoefC2Sky = tempnom;
        } else if (tempdenom == 0.0) {
            // if temperature difference is zero, no radiation exchange
            hRadCoefC2Sky = 0.0;
        } else {
            hRadCoefC2Sky = tempnom / tempdenom;
        }

        tempnom = state.dataSurface->Surface(SurfNum).ViewFactorGround * EmissOfOuterCover * Constant::StefanBoltzmann *
                  ((TempOuterCover + Constant::Kelvin) + state.dataEnvrn->GroundTempKelvin) *
                  (pow_2(TempOuterCover + Constant::Kelvin) + pow_2(state.dataEnvrn->GroundTempKelvin));
        tempdenom =
            (TempOuterCover - TempOutdoorAir) / (TempOuterCover - state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::BuildingSurface]);
        if (tempdenom < 0.0) {
            // use approximate linearized radiation coefficient
            hRadCoefC2Gnd = tempnom;
        } else if (tempdenom == 0.0) {
            // if temperature difference is zero, no radiation exchange
            hRadCoefC2Gnd = 0.0;
        } else {
            hRadCoefC2Gnd = tempnom / tempdenom;
        }

        // combine the radiation coefficients
        hRadCoefC2O = hRadCoefC2Sky + hRadCoefC2Gnd;

        // calculate the overall top heat loss coefficient:

        if (NumCovers == 1) {
            this->UTopLoss = 1.0 / (1.0 / (hRadCoefA2C + hConvCoefA2C) + 1.0 / (hRadCoefC2O + hConvCoefC2O));
        } else {
            this->UTopLoss = 1.0 / (1.0 / (hRadCoefA2C + hConvCoefA2C) + 1.0 / (hRadCoefC2C + hConvCoefC2C) + 1.0 / (hRadCoefC2O + hConvCoefC2O));
        }

        // calculate the side loss coefficient.  Adds the insulation resistance and the combined
        // convection-radiation coefficients in series.
        Real64 hRadConvOut = 5.7 + 3.8 * state.dataSurface->SurfOutWindSpeed(SurfNum);
        this->UsLoss =
            1.0 / (1.0 / (state.dataSolarCollectors->Parameters(ParamNum).ULossSide * this->AreaRatio) + 1.0 / (hRadConvOut * this->AreaRatio));

        // the bottom loss coefficient calculation depends on the boundary condition
        if (this->OSCM_ON) { // OtherSideConditionsModel
            this->UbLoss = state.dataSolarCollectors->Parameters(ParamNum).ULossBottom;
        } else { // AmbientAir
            this->UbLoss = 1.0 / (1.0 / state.dataSolarCollectors->Parameters(ParamNum).ULossBottom + 1.0 / hRadConvOut);
        }

        // Calculate current timestep covers temperature
        if (NumCovers == 1) {
            tempnom = this->CoverAbs[0] * state.dataHeatBal->SurfQRadSWOutIncident(SurfNum) + TempOutdoorAir * (hConvCoefC2O + hRadCoefC2O) +
                      TempAbsPlate * (hConvCoefA2C + hRadCoefA2C);
            tempdenom = (hConvCoefC2O + hRadCoefC2O) + (hConvCoefA2C + hRadCoefA2C);
            TempOuterCover = tempnom / tempdenom;
        } else if (NumCovers == 2) {
            for (int Num = 0; Num < NumCovers; ++Num) {
                if (Num == 0) {
                    tempnom = this->CoverAbs[Num] * state.dataHeatBal->SurfQRadSWOutIncident(SurfNum) +
                              TempOutdoorAir * (hConvCoefC2O + hRadCoefC2O) + TempInnerCover * (hConvCoefC2C + hRadCoefC2C);
                    tempdenom = (hConvCoefC2O + hRadCoefC2O) + (hConvCoefC2C + hRadCoefC2C);
                    TempOuterCover = tempnom / tempdenom;
                } else if (Num == 1) {
                    tempnom = this->CoverAbs[Num] * state.dataHeatBal->SurfQRadSWOutIncident(SurfNum) + TempAbsPlate * (hConvCoefA2C + hRadCoefA2C) +
                              TempOuterCover * (hConvCoefC2C + hRadCoefC2C);
                    tempdenom = (hConvCoefC2C + hRadCoefC2C + hConvCoefA2C + hRadCoefA2C);
                    TempInnerCover = tempnom / tempdenom;
                }
            }
        }
        this->TempOfInnerCover = TempInnerCover;
        this->TempOfOuterCover = TempOuterCover;
    }

    Real64 CollectorData::CalcConvCoeffBetweenPlates(Real64 const TempSurf1, // temperature of surface 1
                                                     Real64 const TempSurf2, // temperature of surface 1
                                                     Real64 const AirGap,    // characteristic length [m]
                                                     Real64 const CosTilt,   // cosine of surface tilt angle relative to the horizontal
                                                     Real64 const SinTilt    // sine of surface tilt angle relative to the horizontal
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Bereket Nigusse, FSEC/UCF
        //       DATE WRITTEN   February 2012

        // PURPOSE OF THIS FUNCTION:
        //   Calculates the convection coefficient for an enclosure between two parallel surfaces
        //   at different temperatures.
        // METHODOLOGY EMPLOYED:
        //   Uses empirical correlation by Holands et al (1976) to determine free convection between
        //   inclined parallel plates at different temperature.
        // REFERENCES:
        //   Duffie, J. A., and Beckman, W. A.  Solar Engineering of Thermal Processes, 2nd. Edition.
        //   Wiley-Interscience: New York (1991).
        //   Property data for air at atmospheric pressure were taken from Table A-11, Yunus A Cengel
        //   Heat Transfer: A Practical Approach, McGraw-Hill, Boston, MA, 1998.

        Real64 constexpr gravity(9.806); // gravitational constant [m/s^2]

        int constexpr NumOfPropDivisions(11);
        static constexpr std::array<Real64, NumOfPropDivisions> Temps = {
            -23.15, 6.85, 16.85, 24.85, 26.85, 36.85, 46.85, 56.85, 66.85, 76.85, 126.85}; // Temperature, in C
        static constexpr std::array<Real64, NumOfPropDivisions> Mu = {
            0.0000161, 0.0000175, 0.000018, 0.0000184, 0.0000185, 0.000019, 0.0000194, 0.0000199, 0.0000203, 0.0000208, 0.0000229}; // Viscosity, in
                                                                                                                                    // kg/(m.s)
        static constexpr std::array<Real64, NumOfPropDivisions> Conductivity = {
            0.0223, 0.0246, 0.0253, 0.0259, 0.0261, 0.0268, 0.0275, 0.0283, 0.0290, 0.0297, 0.0331}; // Conductivity, in W/mK
        static constexpr std::array<Real64, NumOfPropDivisions> Pr = {
            0.724, 0.717, 0.714, 0.712, 0.712, 0.711, 0.71, 0.708, 0.707, 0.706, 0.703}; // Prandtl number (dimensionless)
        static constexpr std::array<Real64, NumOfPropDivisions> Density = {
            1.413, 1.271, 1.224, 1.186, 1.177, 1.143, 1.110, 1.076, 1.043, 1.009, 0.883}; // Density, in kg/m3

        Real64 CondOfAir; // thermal conductivity of air [W/mK]
        Real64 VisDOfAir; // dynamic viscosity of air [kg/m.s]
        Real64 DensOfAir; // density of air [W/mK]
        Real64 PrOfAir;   // Prandtl number of air [W/mK]
        Real64 VolExpAir; // volumetric expansion of air [1/K]

        Real64 DeltaT = std::abs(TempSurf1 - TempSurf2);
        Real64 Tref = 0.5 * (TempSurf1 + TempSurf2);
        int Index = 0;
        while (Index < NumOfPropDivisions) {
            if (Tref < Temps[Index]) {
                break; // DO loop
            }
            ++Index;
        }

        // Initialize thermal properties of air
        if (Index == 0) {
            VisDOfAir = Mu[Index];
            CondOfAir = Conductivity[Index];
            PrOfAir = Pr[Index];
            DensOfAir = Density[Index];
        } else if (Index >= NumOfPropDivisions) { // 0-index, hence MaxIndex = NumOfPropDivisions - 1
            Index = NumOfPropDivisions - 1;
            VisDOfAir = Mu[Index];
            CondOfAir = Conductivity[Index];
            PrOfAir = Pr[Index];
            DensOfAir = Density[Index];
        } else {
            Real64 InterpFrac = (Tref - Temps[Index - 1]) / (Temps[Index] - Temps[Index - 1]);
            VisDOfAir = Mu[Index - 1] + InterpFrac * (Mu[Index] - Mu[Index - 1]);
            CondOfAir = Conductivity[Index - 1] + InterpFrac * (Conductivity[Index] - Conductivity[Index - 1]);
            PrOfAir = Pr[Index - 1] + InterpFrac * (Pr[Index] - Pr[Index - 1]);
            DensOfAir = Density[Index - 1] + InterpFrac * (Density[Index] - Density[Index - 1]);
        }

        VolExpAir = 1.0 / (Tref + Constant::Kelvin);

        // Rayleigh number
        Real64 RaNum = gravity * pow_2(DensOfAir) * VolExpAir * PrOfAir * DeltaT * pow_3(AirGap) / pow_2(VisDOfAir);

        // Rayleigh number of air times cosine of collector tilt []
        Real64 RaNumCosTilt = RaNum * CosTilt;

        Real64 NuL; // Nusselt number
        if (RaNum == 0.0) {
            NuL = 0.0;
        } else {
            if (RaNumCosTilt > 1708.0) {
                NuL = 1.44 * (1.0 - 1708.0 * std::pow(SinTilt, 1.6) / (RaNum * CosTilt)) * (1.0 - 1708.0 / RaNumCosTilt);
            } else {
                NuL = 0.0;
            }
        }
        if (RaNumCosTilt > 5830.0) {
            NuL += std::pow(RaNumCosTilt / 5830.0 - 1.0, 1.0 / 3.0);
        }
        ++NuL;
        Real64 hConvCoef = NuL * CondOfAir / AirGap;

        return hConvCoef;
    }

    Real64 CollectorData::CalcConvCoeffAbsPlateAndWater(EnergyPlusData &state,
                                                        Real64 const TAbsorber, // temperature of absorber plate [C]
                                                        Real64 const TWater,    // temperature of water [C]
                                                        Real64 const Lc,        // characteristic length [m]
                                                        Real64 const TiltR2V    // collector tilt angle relative to the vertical [degree]
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Bereket Nigusse, FSEC/UCF
        //       DATE WRITTEN   February 2012

        // PURPOSE OF THIS FUNCTION:
        //  Calculates the free convection coefficient between the absorber plate and water.
        // METHODOLOGY EMPLOYED:
        //  The convection coefficient calculation were based on the Fujii and Imura empirical correlations
        // REFERENCES:
        //  T.Fujii, and H.Imura,Natural convection heat transfer from a plate with arbitrary inclination.
        //  International Journal of Heat and Mass Transfer: 15(4), (1972), 755-764.

        Real64 hConvA2W; // convection coefficient, [W/m2K]

        Real64 constexpr gravity(9.806); // gravitational constant [m/s^2]
        static constexpr std::string_view CalledFrom("SolarCollectors:CalcConvCoeffAbsPlateAndWater");

        Real64 DeltaT = std::abs(TAbsorber - TWater);
        Real64 TReference = TAbsorber - 0.25 * (TAbsorber - TWater);
        // record fluid prop index for water

        auto *water = Fluid::GetWater(state);
        // find properties of water - always assume water
        Real64 WaterSpecHeat = water->getSpecificHeat(state, max(TReference, 0.0), CalledFrom);
        Real64 CondOfWater = water->getConductivity(state, max(TReference, 0.0), CalledFrom);
        Real64 VisOfWater = water->getViscosity(state, max(TReference, 0.0), CalledFrom);
        Real64 DensOfWater = water->getDensity(state, max(TReference, 0.0), CalledFrom);
        Real64 PrOfWater = VisOfWater * WaterSpecHeat / CondOfWater;
        // Requires a different reference temperature for volumetric expansion coefficient
        TReference = TWater - 0.25 * (TWater - TAbsorber);
        Real64 VolExpWater =
            -(water->getDensity(state, max(TReference, 10.0) + 5.0, CalledFrom) - water->getDensity(state, max(TReference, 10.0) - 5.0, CalledFrom)) /
            (10.0 * DensOfWater);

        // Grashof number
        Real64 GrNum = gravity * VolExpWater * DensOfWater * DensOfWater * PrOfWater * DeltaT * pow_3(Lc) / pow_2(VisOfWater);
        Real64 CosTilt = std::cos(TiltR2V * Constant::DegToRad);

        Real64 RaNum; // Raleigh number
        Real64 NuL;   // Nusselt number
        if (TAbsorber > TWater) {
            // hot absorber plate facing down
            if (std::abs(TiltR2V - 90.0) < 1.0) {
                // It is a horizontal surface
                RaNum = GrNum * PrOfWater;
                if (RaNum <= 1708.0) {
                    NuL = 1.0;
                } else {
                    NuL = 0.58 * std::pow(RaNum, 0.20);
                }
            } else {
                RaNum = GrNum * PrOfWater * CosTilt;
                if (RaNum <= 1708.0) {
                    NuL = 1.0;
                } else {
                    NuL = 0.56 * root_4(RaNum);
                }
            }
        } else {
            // cold plate facing down or hot plate facing up
            RaNum = GrNum * PrOfWater;
            if (RaNum > 5.0e8) {
                NuL = 0.13 * std::pow(RaNum, 1.0 / 3.0);
            } else {
                NuL = 0.16 * std::pow(RaNum, 1.0 / 3.0);
                if (RaNum <= 1708.0) {
                    NuL = 1.0;
                }
            }
        }
        hConvA2W = NuL * CondOfWater / Lc;

        return hConvA2W;
    }

    void CollectorData::update(EnergyPlusData &state) // NOLINT(readability-make-member-function-const)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   January 2004

        // PURPOSE OF THIS SUBROUTINE:
        // Updates the node variables with local variables.

        static constexpr std::string_view RoutineName("UpdateSolarCollector");

        PlantUtilities::SafeCopyPlantNode(state, this->InletNode, this->OutletNode);
        // Set outlet node variables that are possibly changed
        state.dataLoopNodes->Node(this->OutletNode).Temp = this->OutletTemp;
        Real64 Cp = this->plantLoc.loop->glycol->getSpecificHeat(state, this->OutletTemp, RoutineName);
        state.dataLoopNodes->Node(this->OutletNode).Enthalpy = Cp * state.dataLoopNodes->Node(this->OutletNode).Temp;
    }

    void CollectorData::report(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   January 2004

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates report variables.

        Real64 TimeStepInSecond = state.dataHVACGlobal->TimeStepSysSec;

        this->Energy = this->Power * TimeStepInSecond;
        this->HeatEnergy = this->HeatRate * TimeStepInSecond;
        this->CollHeatLossEnergy = this->SkinHeatLossRate * TimeStepInSecond;
        this->StoredHeatEnergy = this->StoredHeatRate * TimeStepInSecond;
    }

    void CollectorData::GetExtVentedCavityIndex(EnergyPlusData &state, int const SurfacePtr, int &VentCavIndex)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Nigusse, FSEC. Adopted from Photovoltaics module
        //       DATE WRITTEN   February 2012

        // PURPOSE OF THIS SUBROUTINE:
        // object oriented "Get" routine for establishing correct integer index from outside this module

        // METHODOLOGY EMPLOYED:
        // mine Surface derived type for correct index/number of surface
        // mine  ExtVentedCavity derived type that has the surface.
        // Adapted from Photovoltaics module, originally developed by Brent G. (2004)

        bool Found;

        if (SurfacePtr == 0) {
            // should be trapped already
            ShowFatalError(state, "Invalid surface passed to GetExtVentedCavityIndex");
        }

        int CavNum = 0;
        Found = false;
        for (int thisCav = 1; thisCav <= state.dataSurface->TotExtVentCav; ++thisCav) {
            for (int ThisSurf = 1; ThisSurf <= state.dataHeatBal->ExtVentedCavity(thisCav).NumSurfs; ++ThisSurf) {
                if (SurfacePtr == state.dataHeatBal->ExtVentedCavity(thisCav).SurfPtrs(ThisSurf)) {
                    Found = true;
                    CavNum = thisCav;
                }
            }
        }

        if (!Found) {
            ShowFatalError(state,
                           format("Did not find surface in Exterior Vented Cavity description in GetExtVentedCavityIndex, Surface name = {}",
                                  state.dataSurface->Surface(SurfacePtr).Name));
        } else {

            VentCavIndex = CavNum;
        }
    }
    void CollectorData::oneTimeInit_new(EnergyPlusData &state)
    {

        this->setupOutputVars(state);

        if (this->SetLoopIndexFlag) {
            if (allocated(state.dataPlnt->PlantLoop)) {
                bool errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(state, this->Name, this->Type, this->plantLoc, errFlag, _, _, _, _, _);
                if (errFlag) {
                    ShowFatalError(state, "InitSolarCollector: Program terminated due to previous condition(s).");
                }
                this->SetLoopIndexFlag = false;
            }
        }
    }
    void CollectorData::oneTimeInit([[maybe_unused]] EnergyPlusData &state)
    {
    }

} // namespace SolarCollectors

} // namespace EnergyPlus
