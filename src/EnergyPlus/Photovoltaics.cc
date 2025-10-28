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
#include <cassert>
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataPhotovoltaics.hh>
#include <EnergyPlus/DataPrecisionGlobals.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/PhotovoltaicThermalCollectors.hh>
#include <EnergyPlus/Photovoltaics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/TranspiredCollector.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace Photovoltaics {
    //       MODULE INFORMATION:
    //       AUTHOR         David Bradley
    //       DATE WRITTEN   January 2003
    //       MODIFIED       B. Griffith, dec2003 - Jan2004
    //                      added Sandia PV model loosely based on G. Barker's implementation for TRNSYS type
    //                      added Simple PV efficiency model for early design phases
    //       RE-ENGINEERED  added case statement to allow selecting and mixing between different models
    //                      moved derived types to DataPhotovoltaics
    //                      B. Griffith, Aug. 2008, refactored PV data structures and input objects to
    //                       so that there is one Generator:Photovoltaics object with 3 different model options.

    // PURPOSE OF THIS MODULE:
    // This module collects routines used to simulate the timestep by timestep performance of a
    // photovoltaic arrays.  The user can select between different models by choosing an a model and performance input object
    // Using the input object "PhotovoltaicPerformance:Simple" will lead to modeling the PV system using
    // crude model that just applies a power conversion efficiency factor, much simpler to specify
    // Using the input object "PhotovoltaicPerformance:EquivalentOne-Diode" will lead to modeling the PV system using
    // The PV model used as the basis for this module is Type180 from the HYDROGEMS library developed by
    // Oystein Ulleberg at the IFE Institute for Energy Technology in Norway and also work by Eckstein

    // Using the input object, "PhotovoltaicPerformance:SANDIA"  will lead to modeling a PV array
    //  using models developed by David King, Sandia National lab.  These models appear to provide
    //  improved prediction of PV performance at low radiance and incident angles.

    // METHODOLOGY EMPLOYED: This module contains routines to manage PV system models.
    //  There are two options for what model to use and this duality of modeling approaches is
    //  reflected in there being two groups of routines for each PV model, The original model is
    //  referred to as Equivalent one-diode model and has origins as a TRNSYS type180 from the Hydrogems library
    //  A newer model with more involved input has been developed by Sandia National Lab (SNL) by David King.
    //  The TRNSYS type180 model include the use of numerical routines to minimize a multivariate function

    // Using/Aliasing
    using namespace DataPhotovoltaics;

    constexpr std::string_view cPVGeneratorObjectName = "Generator:Photovoltaic";

    constexpr std::array<std::string_view, (int)PVModel::Num> pvModelNames = {
        "PhotovoltaicPerformance:Simple", "PhotovoltaicPerformance:EquivalentOne-Diode", "PhotovoltaicPerformance:Sandia"};
    constexpr std::array<std::string_view, (int)PVModel::Num> pvModelNamesUC = {
        "PHOTOVOLTAICPERFORMANCE:SIMPLE", "PHOTOVOLTAICPERFORMANCE:EQUIVALENTONE-DIODE", "PHOTOVOLTAICPERFORMANCE:SANDIA"};

    [[maybe_unused]] constexpr std::array<std::string_view, (int)CellIntegration::Num> cellIntegrationNames = {"Decoupled",
                                                                                                               "DecoupledUllebergDynamic",
                                                                                                               "IntegratedSurfaceOutsideFace",
                                                                                                               "IntegratedTranspiredCollector",
                                                                                                               "IntegratedExteriorVentedCavity",
                                                                                                               "PhotovoltaicThermalSolarCollector"};
    constexpr std::array<std::string_view, (int)CellIntegration::Num> cellIntegrationNamesUC = {"DECOUPLED",
                                                                                                "DECOUPLEDULLEBERGDYNAMIC",
                                                                                                "INTEGRATEDSURFACEOUTSIDEFACE",
                                                                                                "INTEGRATEDTRANSPIREDCOLLECTOR",
                                                                                                "INTEGRATEDEXTERIORVENTEDCAVITY",
                                                                                                "PHOTOVOLTAICTHERMALSOLARCOLLECTOR"};

    [[maybe_unused]] constexpr std::array<std::string_view, (int)Efficiency::Num> efficiencyNames = {"Fixed", "Scheduled"};
    constexpr std::array<std::string_view, (int)Efficiency::Num> efficiencyNamesUC = {"FIXED", "SCHEDULED"};

    [[maybe_unused]] constexpr std::array<std::string_view, (int)SiPVCells::Num> siPVCellsNames = {"CrystallineSilicon", "AmorphousSilicon"};
    constexpr std::array<std::string_view, (int)SiPVCells::Num> siPVCellsNamesUC = {"CRYSTALLINESILICON", "AMORPHOUSSILICON"};

    void SimPVGenerator(EnergyPlusData &state,
                        [[maybe_unused]] GeneratorType const GeneratorType, // type of Generator !unused1208
                        std::string const &GeneratorName,                   // user specified name of Generator
                        int &GeneratorIndex,
                        bool const RunFlag,                  // is PV ON or OFF as determined by schedules in ElecLoadCenter
                        [[maybe_unused]] Real64 const PVLoad // electrical load on the PV (not really used... PV models assume "full on" !unused1208
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         David Bradley
        //       DATE WRITTEN   April 2003
        //       MODIFIED       B. Griffith Jan 2004
        //                      B. Griffith Aug. 2008 Rework for new structure
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is in charge of all the rest of the subroutines contained
        // in this module. provides common entry point for all the models

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int PVnum; // index of unit in PV array for Equivalent one-diode model

        // Get PV data from input file
        if (state.dataPhotovoltaicState->GetInputFlag) {
            GetPVInput(state); // for all three types of models
            state.dataPhotovoltaicState->GetInputFlag = false;
        }

        if (GeneratorIndex == 0) {
            PVnum = Util::FindItemInList(GeneratorName, state.dataPhotovoltaic->PVarray);
            if (PVnum == 0) {
                ShowFatalError(state, format("SimPhotovoltaicGenerator: Specified PV not one of valid Photovoltaic Generators {}", GeneratorName));
            }
            GeneratorIndex = PVnum;
        } else {
            PVnum = GeneratorIndex;
            if (PVnum > state.dataPhotovoltaic->NumPVs || PVnum < 1) {
                ShowFatalError(state,
                               format("SimPhotovoltaicGenerator: Invalid GeneratorIndex passed={}, Number of PVs={}, Generator name={}",
                                      PVnum,
                                      state.dataPhotovoltaic->NumPVs,
                                      GeneratorName));
            }
            if (state.dataPhotovoltaicState->CheckEquipName(PVnum)) {
                if (GeneratorName != state.dataPhotovoltaic->PVarray(PVnum).Name) {
                    ShowFatalError(
                        state,
                        format("SimPhotovoltaicGenerator: Invalid GeneratorIndex passed={}, Generator name={}, stored PV Name for that index={}",
                               PVnum,
                               GeneratorName,
                               state.dataPhotovoltaic->PVarray(PVnum).Name));
                }
                state.dataPhotovoltaicState->CheckEquipName(PVnum) = false;
            }
        }

        switch (state.dataPhotovoltaic->PVarray(PVnum).PVModelType) {
        case PVModel::Simple: {
            CalcSimplePV(state, PVnum);
        } break;
        case PVModel::TRNSYS: {
            // 'PhotovoltaicPerformance:EquivalentOne-Diode' (aka. 5-parameter TRNSYS type 180 model)
            InitTRNSYSPV(state, PVnum);

            CalcTRNSYSPV(state, PVnum, RunFlag);
        } break;
        case PVModel::Sandia: {
            // 'PhotovoltaicPerformance:Sandia' (aka. King model, Sandia Nat. Labs.)
            CalcSandiaPV(state, PVnum, RunFlag);
        } break;
        default: {
            ShowFatalError(state, format("Specified generator model type not found for PV generator = {}", GeneratorName));
        } break;
        }

        ReportPV(state, PVnum);
    }

    void GetPVGeneratorResults(EnergyPlusData &state,
                               [[maybe_unused]] GeneratorType const GeneratorType, // type of Generator !unused1208
                               int const GeneratorIndex,
                               Real64 &GeneratorPower,  // electrical power
                               Real64 &GeneratorEnergy, // electrical energy
                               Real64 &ThermalPower,
                               Real64 &ThermalEnergy)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Aug. 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // provide a "get" method to collect results for individual electric load centers.

        // Using/Aliasing
        using PhotovoltaicThermalCollectors::GetPVTThermalPowerProduction;

        GeneratorPower = state.dataPhotovoltaic->PVarray(GeneratorIndex).Report.DCPower;
        GeneratorEnergy = state.dataPhotovoltaic->PVarray(GeneratorIndex).Report.DCEnergy;
        auto const &thisPVarray = state.dataPhotovoltaic->PVarray(GeneratorIndex);
        // PVT may add thermal
        if (thisPVarray.CellIntegrationMode == CellIntegration::PVTSolarCollector) {
            // get result for thermal power generation
            GetPVTThermalPowerProduction(state, GeneratorIndex, ThermalPower, ThermalEnergy);
        } else {
            ThermalPower = 0.0;
            ThermalEnergy = 0.0;
        }
    }

    // *************

    void GetPVInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         David Bradley
        //       DATE WRITTEN   January 2003
        //       MODIFIED       B.Griffith Dec. 2003 - Jan 2004 added input for Simple and Sandia PV model
        //                      B. Griffith Feb. 2008 - revised input for TRNSYS pv model for BIPV and inverter
        //                      B. Griffith Aug. 2008 - revised input for new organization and naming convention
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the input for the Photovoltaic units saving it in
        // the data structures defined in DataPhotovoltaics.cc.

        // METHODOLOGY EMPLOYED:
        // subroutine structure taken from Beta2 BaseboardRadiator.cc

        // Using/Aliasing
        using namespace DataHeatBalance;

        using PhotovoltaicThermalCollectors::GetPVTmodelIndex;
        using TranspiredCollector::GetTranspiredCollectorIndex;

        static constexpr std::string_view routineName = "GetPVInput";
        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int PVnum;     // working variable for do loop through pv arrays
        int SurfNum;   // working variable for surface id in Heat Balance domain
        int ModNum;    // working variable for do loop through Sandia model parameter input
        int NumAlphas; // Number of PV Array parameter alpha names being passed
        int NumNums;   // Number of PV Array numeric parameters are being passed
        int IOStat;
        bool ErrorsFound(false); // if errors detected in input
        int ThisParamObj;
        int dupPtr;

        // Object Data
        Array1D<SimplePVParamsStruct> tmpSimpleModuleParams;       // temporary, for processing input data
        Array1D<TRNSYSPVModuleParamsStruct> tmpTRNSYSModuleParams; // temporary, for processing input data
        Array1D<SNLModuleParamsStuct> tmpSNLModuleParams;          // temporary, for processing input data

        auto &s_ipsc = state.dataIPShortCut;

        // count how many photovoltaic arrays of different types are in the .idf
        state.dataPhotovoltaic->NumPVs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cPVGeneratorObjectName);
        state.dataPhotovoltaic->NumSimplePVModuleTypes =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, pvModelNames[(int)PVModel::Simple]);
        state.dataPhotovoltaic->Num1DiodePVModuleTypes =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, pvModelNames[(int)PVModel::TRNSYS]);
        state.dataPhotovoltaic->NumSNLPVModuleTypes =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, pvModelNames[(int)PVModel::Sandia]);

        if (state.dataPhotovoltaic->NumPVs <= 0) {
            ShowSevereError(state, format("Did not find any {}", cPVGeneratorObjectName));
            return;
        }

        if (!allocated(state.dataPhotovoltaic->PVarray)) {
            state.dataPhotovoltaic->PVarray.allocate(state.dataPhotovoltaic->NumPVs);
        }
        state.dataPhotovoltaicState->CheckEquipName.dimension(state.dataPhotovoltaic->NumPVs, true);

        s_ipsc->cCurrentModuleObject = cPVGeneratorObjectName;
        for (PVnum = 1; PVnum <= state.dataPhotovoltaic->NumPVs; ++PVnum) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     PVnum,
                                                                     s_ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     s_ipsc->rNumericArgs,
                                                                     NumNums,
                                                                     IOStat,
                                                                     _,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

            Util::IsNameEmpty(state, s_ipsc->cAlphaArgs(1), s_ipsc->cCurrentModuleObject, ErrorsFound);
            state.dataPhotovoltaic->PVarray(PVnum).Name = s_ipsc->cAlphaArgs(1);

            state.dataPhotovoltaic->PVarray(PVnum).SurfaceName = s_ipsc->cAlphaArgs(2);
            state.dataPhotovoltaic->PVarray(PVnum).SurfacePtr = Util::FindItemInList(s_ipsc->cAlphaArgs(2), state.dataSurface->Surface);
            // required-surface
            if (s_ipsc->lAlphaFieldBlanks(2)) {
                ShowSevereError(state, format("Invalid {} = {}", s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2)));
                ShowContinueError(state, format("Entered in {} = {}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "Surface name cannot be blank");
                ErrorsFound = true;
            }
            if (state.dataPhotovoltaic->PVarray(PVnum).SurfacePtr == 0) {
                ShowSevereError(state, format("Invalid {} = {}", s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2)));
                ShowContinueError(state, format("Entered in {} = {}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ErrorsFound = true;
            } else {
                // Found one -- make sure has right parameters for PV
                SurfNum = state.dataPhotovoltaic->PVarray(PVnum).SurfacePtr;
                state.dataSurface->SurfIsPV(SurfNum) = true;

                if (!state.dataSurface->Surface(SurfNum).ExtSolar) {
                    ShowWarningError(state, format("Invalid {} = {}", s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2)));
                    ShowContinueError(state, format("Entered in {} = {}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                    ShowContinueError(state, "Surface is not exposed to solar, check surface boundary condition");
                }
                state.dataPhotovoltaic->PVarray(PVnum).Zone = GetPVZone(state, state.dataPhotovoltaic->PVarray(PVnum).SurfacePtr);

                // check surface orientation, warn if upside down
                if ((state.dataSurface->Surface(SurfNum).Tilt < -95.0) || (state.dataSurface->Surface(SurfNum).Tilt > 95.0)) {
                    ShowWarningError(state, format("Suspected input problem with {} = {}", s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2)));
                    ShowContinueError(state, format("Entered in {} = {}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                    ShowContinueError(state, "Surface used for solar collector faces down");
                    ShowContinueError(
                        state, format("Surface tilt angle (degrees from ground outward normal) = {:.2R}", state.dataSurface->Surface(SurfNum).Tilt));
                }
            }

            if (s_ipsc->lAlphaFieldBlanks(3)) {
                ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
                ErrorsFound = true;
            } else if ((state.dataPhotovoltaic->PVarray(PVnum).PVModelType =
                            static_cast<PVModel>(getEnumValue(pvModelNamesUC, s_ipsc->cAlphaArgs(3)))) == PVModel::Invalid) {
                ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
                ErrorsFound = true;
            }

            state.dataPhotovoltaic->PVarray(PVnum).PerfObjName = s_ipsc->cAlphaArgs(4); // check later once perf objects are loaded

            if (s_ipsc->lAlphaFieldBlanks(5)) {
                ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5));
                ErrorsFound = true;
            } else if ((state.dataPhotovoltaic->PVarray(PVnum).CellIntegrationMode =
                            static_cast<CellIntegration>(getEnumValue(cellIntegrationNamesUC, s_ipsc->cAlphaArgs(5)))) == CellIntegration::Invalid) {
                ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5));
                ErrorsFound = true;
            }

            state.dataPhotovoltaic->PVarray(PVnum).NumSeriesNParall = s_ipsc->rNumericArgs(1);
            state.dataPhotovoltaic->PVarray(PVnum).NumModNSeries = s_ipsc->rNumericArgs(2);

        } // main PV array objects

        // search for duplicate PV arrays on integrated heat transfer surfaces, accumulating source terms across arrays is not supported
        for (PVnum = 1; PVnum <= state.dataPhotovoltaic->NumPVs; ++PVnum) {
            switch (state.dataPhotovoltaic->PVarray(PVnum).CellIntegrationMode) {
            case CellIntegration::SurfaceOutsideFace:
            case CellIntegration::TranspiredCollector:
            case CellIntegration::ExteriorVentedCavity: {
                dupPtr = Util::FindItemInList(state.dataPhotovoltaic->PVarray(PVnum).SurfaceName,
                                              state.dataPhotovoltaic->PVarray({PVnum + 1, state.dataPhotovoltaic->NumPVs}),
                                              &PVArrayStruct::SurfaceName);
                if (dupPtr != 0) {
                    dupPtr += PVnum; // to correct for shortened array in find item
                }
                if (dupPtr != 0) {
                    auto &thisPVarray = state.dataPhotovoltaic->PVarray(dupPtr);
                    if (thisPVarray.CellIntegrationMode == CellIntegration::SurfaceOutsideFace) {
                        ShowSevereError(state, format("{}: problem detected with multiple PV arrays.", s_ipsc->cCurrentModuleObject));
                        ShowContinueError(state, "When using IntegratedSurfaceOutsideFace heat transfer mode, only one PV array can be coupled");
                        ShowContinueError(state,
                                          format("Both {} and {} are using surface {}",
                                                 state.dataPhotovoltaic->PVarray(PVnum).Name,
                                                 thisPVarray.Name,
                                                 state.dataPhotovoltaic->PVarray(PVnum).SurfaceName));
                        ErrorsFound = true;
                    } else if (thisPVarray.CellIntegrationMode == CellIntegration::TranspiredCollector) {
                        ShowSevereError(state, format("{}: problem detected with multiple PV arrays.", s_ipsc->cCurrentModuleObject));
                        ShowContinueError(state, "When using IntegratedTranspiredCollector heat transfer mode, only one PV array can be coupled");
                        ShowContinueError(state,
                                          format("Both {} and {} are using UTSC surface = {}",
                                                 state.dataPhotovoltaic->PVarray(PVnum).Name,
                                                 thisPVarray.Name,
                                                 state.dataPhotovoltaic->PVarray(PVnum).SurfaceName));
                        ErrorsFound = true;
                    } else if (thisPVarray.CellIntegrationMode == CellIntegration::ExteriorVentedCavity) {
                        ShowSevereError(state, format("{}: problem detected with multiple PV arrays.", s_ipsc->cCurrentModuleObject));
                        ShowContinueError(state, "When using IntegratedExteriorVentedCavity heat transfer mode, only one PV array can be coupled");
                        ShowContinueError(state,
                                          format("Both {} and {} are using exterior vented surface = {}",
                                                 state.dataPhotovoltaic->PVarray(PVnum).Name,
                                                 thisPVarray.Name,
                                                 state.dataPhotovoltaic->PVarray(PVnum).SurfaceName));
                        ErrorsFound = true;
                    } else if (thisPVarray.CellIntegrationMode == CellIntegration::PVTSolarCollector) {
                        ShowSevereError(
                            state,
                            format("Problem detected with multiple PV arrays for={}. When using PhotovoltaicThermalSolarCollector heat transfer "
                                   "mode, only one PV array can be coupled. Both this PV array={} and this PV array={} are using PVT surface={}",
                                   s_ipsc->cCurrentModuleObject,
                                   state.dataPhotovoltaic->PVarray(PVnum).Name,
                                   thisPVarray.Name,
                                   state.dataPhotovoltaic->PVarray(PVnum).SurfaceName));
                        ErrorsFound = true;
                    }
                }
            } break;
            default:
                break;
            }
        }

        if (state.dataPhotovoltaic->NumSimplePVModuleTypes > 0) {
            tmpSimpleModuleParams.allocate(state.dataPhotovoltaic->NumSimplePVModuleTypes);
            s_ipsc->cCurrentModuleObject = pvModelNames[(int)PVModel::Simple];
            for (ModNum = 1; ModNum <= state.dataPhotovoltaic->NumSimplePVModuleTypes; ++ModNum) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         s_ipsc->cCurrentModuleObject,
                                                                         ModNum,
                                                                         s_ipsc->cAlphaArgs,
                                                                         NumAlphas,
                                                                         s_ipsc->rNumericArgs,
                                                                         NumNums,
                                                                         IOStat,
                                                                         _,
                                                                         s_ipsc->lAlphaFieldBlanks,
                                                                         s_ipsc->cAlphaFieldNames,
                                                                         s_ipsc->cNumericFieldNames);

                ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

                if (Util::IsNameEmpty(state, s_ipsc->cAlphaArgs(1), s_ipsc->cCurrentModuleObject, ErrorsFound)) {
                    continue;
                }
                tmpSimpleModuleParams(ModNum).Name = s_ipsc->cAlphaArgs(1);
                tmpSimpleModuleParams(ModNum).ActiveFraction = s_ipsc->rNumericArgs(1);

                if (s_ipsc->lAlphaFieldBlanks(2)) {
                    ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(2));
                    ErrorsFound = true;
                } else if ((tmpSimpleModuleParams(ModNum).EfficencyInputMode =
                                static_cast<Efficiency>(getEnumValue(efficiencyNamesUC, s_ipsc->cAlphaArgs(2)))) == Efficiency::Invalid) {
                    ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
                    ErrorsFound = true;
                }
                tmpSimpleModuleParams(ModNum).PVEfficiency = s_ipsc->rNumericArgs(2);

                if (tmpSimpleModuleParams(ModNum).EfficencyInputMode == Efficiency::Scheduled) {
                    if (s_ipsc->lAlphaFieldBlanks(3)) {
                        ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(3));
                        ErrorsFound = true;
                    } else if ((tmpSimpleModuleParams(ModNum).effSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(3))) == nullptr) {
                        ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
                        ErrorsFound = true;
                    }
                }
            }
        }

        if (state.dataPhotovoltaic->Num1DiodePVModuleTypes > 0) {
            tmpTRNSYSModuleParams.allocate(state.dataPhotovoltaic->Num1DiodePVModuleTypes);
            s_ipsc->cCurrentModuleObject = pvModelNames[(int)PVModel::TRNSYS];
            for (ModNum = 1; ModNum <= state.dataPhotovoltaic->Num1DiodePVModuleTypes; ++ModNum) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         s_ipsc->cCurrentModuleObject,
                                                                         ModNum,
                                                                         s_ipsc->cAlphaArgs,
                                                                         NumAlphas,
                                                                         s_ipsc->rNumericArgs,
                                                                         NumNums,
                                                                         IOStat,
                                                                         _,
                                                                         s_ipsc->lAlphaFieldBlanks,
                                                                         s_ipsc->cAlphaFieldNames,
                                                                         s_ipsc->cNumericFieldNames);

                ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};
                if (Util::IsNameEmpty(state, s_ipsc->cAlphaArgs(1), s_ipsc->cCurrentModuleObject, ErrorsFound)) {
                    continue;
                }
                tmpTRNSYSModuleParams(ModNum).Name = s_ipsc->cAlphaArgs(1);

                if (s_ipsc->lAlphaFieldBlanks(2)) {
                    ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
                    ErrorsFound = true;
                } else if ((tmpTRNSYSModuleParams(ModNum).CellType = static_cast<SiPVCells>(getEnumValue(siPVCellsNamesUC, s_ipsc->cAlphaArgs(2)))) ==
                           SiPVCells::Invalid) {
                    ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
                    ErrorsFound = true;
                }

                tmpTRNSYSModuleParams(ModNum).CellsInSeries = int(s_ipsc->rNumericArgs(1));
                tmpTRNSYSModuleParams(ModNum).Area = s_ipsc->rNumericArgs(2);
                tmpTRNSYSModuleParams(ModNum).TauAlpha = s_ipsc->rNumericArgs(3);
                tmpTRNSYSModuleParams(ModNum).SemiConductorBandgap = s_ipsc->rNumericArgs(4);
                tmpTRNSYSModuleParams(ModNum).ShuntResistance = s_ipsc->rNumericArgs(5);
                tmpTRNSYSModuleParams(ModNum).RefIsc = s_ipsc->rNumericArgs(6);
                tmpTRNSYSModuleParams(ModNum).RefVoc = s_ipsc->rNumericArgs(7);
                tmpTRNSYSModuleParams(ModNum).RefTemperature = s_ipsc->rNumericArgs(8) + Constant::Kelvin;
                tmpTRNSYSModuleParams(ModNum).RefInsolation = s_ipsc->rNumericArgs(9);
                tmpTRNSYSModuleParams(ModNum).Imp = s_ipsc->rNumericArgs(10);
                tmpTRNSYSModuleParams(ModNum).Vmp = s_ipsc->rNumericArgs(11);
                tmpTRNSYSModuleParams(ModNum).TempCoefIsc = s_ipsc->rNumericArgs(12);
                tmpTRNSYSModuleParams(ModNum).TempCoefVoc = s_ipsc->rNumericArgs(13);
                tmpTRNSYSModuleParams(ModNum).NOCTAmbTemp = s_ipsc->rNumericArgs(14) + Constant::Kelvin;
                tmpTRNSYSModuleParams(ModNum).NOCTCellTemp = s_ipsc->rNumericArgs(15) + Constant::Kelvin;
                tmpTRNSYSModuleParams(ModNum).NOCTInsolation = s_ipsc->rNumericArgs(16);
                tmpTRNSYSModuleParams(ModNum).HeatLossCoef = s_ipsc->rNumericArgs(17);
                tmpTRNSYSModuleParams(ModNum).HeatCapacity = s_ipsc->rNumericArgs(18);
            }
        }

        if (state.dataPhotovoltaic->NumSNLPVModuleTypes > 0) {
            tmpSNLModuleParams.allocate(state.dataPhotovoltaic->NumSNLPVModuleTypes);
            s_ipsc->cCurrentModuleObject = pvModelNames[(int)PVModel::Sandia];
            for (ModNum = 1; ModNum <= state.dataPhotovoltaic->NumSNLPVModuleTypes; ++ModNum) {

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         s_ipsc->cCurrentModuleObject,
                                                                         ModNum,
                                                                         s_ipsc->cAlphaArgs,
                                                                         NumAlphas,
                                                                         s_ipsc->rNumericArgs,
                                                                         NumNums,
                                                                         IOStat,
                                                                         _,
                                                                         s_ipsc->lAlphaFieldBlanks,
                                                                         s_ipsc->cAlphaFieldNames,
                                                                         s_ipsc->cNumericFieldNames);
                if (Util::IsNameEmpty(state, s_ipsc->cAlphaArgs(1), s_ipsc->cCurrentModuleObject, ErrorsFound)) {
                    continue;
                }

                tmpSNLModuleParams(ModNum).name = s_ipsc->cAlphaArgs(1);
                tmpSNLModuleParams(ModNum).Acoll = s_ipsc->rNumericArgs(1);
                tmpSNLModuleParams(ModNum).NcellSer = s_ipsc->rNumericArgs(2);
                tmpSNLModuleParams(ModNum).NparSerCells = s_ipsc->rNumericArgs(3);
                tmpSNLModuleParams(ModNum).Isc0 = s_ipsc->rNumericArgs(4);
                tmpSNLModuleParams(ModNum).Voc0 = s_ipsc->rNumericArgs(5);
                tmpSNLModuleParams(ModNum).Imp0 = s_ipsc->rNumericArgs(6);
                tmpSNLModuleParams(ModNum).Vmp0 = s_ipsc->rNumericArgs(7);
                tmpSNLModuleParams(ModNum).aIsc = s_ipsc->rNumericArgs(8);
                tmpSNLModuleParams(ModNum).aImp = s_ipsc->rNumericArgs(9);
                tmpSNLModuleParams(ModNum).c_0 = s_ipsc->rNumericArgs(10);
                tmpSNLModuleParams(ModNum).c_1 = s_ipsc->rNumericArgs(11);
                tmpSNLModuleParams(ModNum).BVoc0 = s_ipsc->rNumericArgs(12);
                tmpSNLModuleParams(ModNum).mBVoc = s_ipsc->rNumericArgs(13);
                tmpSNLModuleParams(ModNum).BVmp0 = s_ipsc->rNumericArgs(14);
                tmpSNLModuleParams(ModNum).mBVmp = s_ipsc->rNumericArgs(15);
                tmpSNLModuleParams(ModNum).DiodeFactor = s_ipsc->rNumericArgs(16);
                tmpSNLModuleParams(ModNum).c_2 = s_ipsc->rNumericArgs(17);
                tmpSNLModuleParams(ModNum).c_3 = s_ipsc->rNumericArgs(18);
                tmpSNLModuleParams(ModNum).a_0 = s_ipsc->rNumericArgs(19);
                tmpSNLModuleParams(ModNum).a_1 = s_ipsc->rNumericArgs(20);
                tmpSNLModuleParams(ModNum).a_2 = s_ipsc->rNumericArgs(21);
                tmpSNLModuleParams(ModNum).a_3 = s_ipsc->rNumericArgs(22);
                tmpSNLModuleParams(ModNum).a_4 = s_ipsc->rNumericArgs(23);
                tmpSNLModuleParams(ModNum).b_0 = s_ipsc->rNumericArgs(24);
                tmpSNLModuleParams(ModNum).b_1 = s_ipsc->rNumericArgs(25);
                tmpSNLModuleParams(ModNum).b_2 = s_ipsc->rNumericArgs(26);
                tmpSNLModuleParams(ModNum).b_3 = s_ipsc->rNumericArgs(27);
                tmpSNLModuleParams(ModNum).b_4 = s_ipsc->rNumericArgs(28);
                tmpSNLModuleParams(ModNum).b_5 = s_ipsc->rNumericArgs(29);
                tmpSNLModuleParams(ModNum).DT0 = s_ipsc->rNumericArgs(30);
                tmpSNLModuleParams(ModNum).fd = s_ipsc->rNumericArgs(31);
                tmpSNLModuleParams(ModNum).a = s_ipsc->rNumericArgs(32);
                tmpSNLModuleParams(ModNum).b = s_ipsc->rNumericArgs(33);
                tmpSNLModuleParams(ModNum).c_4 = s_ipsc->rNumericArgs(34);
                tmpSNLModuleParams(ModNum).c_5 = s_ipsc->rNumericArgs(35);
                tmpSNLModuleParams(ModNum).Ix0 = s_ipsc->rNumericArgs(36);
                tmpSNLModuleParams(ModNum).Ixx0 = s_ipsc->rNumericArgs(37);
                tmpSNLModuleParams(ModNum).c_6 = s_ipsc->rNumericArgs(38);
                tmpSNLModuleParams(ModNum).c_7 = s_ipsc->rNumericArgs(39);
            }
        }

        // now fill collector performance data into main PV structure
        for (PVnum = 1; PVnum <= state.dataPhotovoltaic->NumPVs; ++PVnum) {

            switch (state.dataPhotovoltaic->PVarray(PVnum).PVModelType) {
            case PVModel::Simple: {
                ThisParamObj = Util::FindItemInList(state.dataPhotovoltaic->PVarray(PVnum).PerfObjName, tmpSimpleModuleParams);
                if (ThisParamObj > 0) {
                    state.dataPhotovoltaic->PVarray(PVnum).SimplePVModule = tmpSimpleModuleParams(ThisParamObj); // entire structure assignment

                    // do one-time setups on input data
                    state.dataPhotovoltaic->PVarray(PVnum).SimplePVModule.AreaCol =
                        state.dataSurface->Surface(state.dataPhotovoltaic->PVarray(PVnum).SurfacePtr).Area *
                        state.dataPhotovoltaic->PVarray(PVnum).SimplePVModule.ActiveFraction;
                } else {
                    ShowSevereError(state, format("Invalid PV performance object name of {}", state.dataPhotovoltaic->PVarray(PVnum).PerfObjName));
                    ShowContinueError(state, format("Entered in {} = {}", cPVGeneratorObjectName, state.dataPhotovoltaic->PVarray(PVnum).Name));
                    ErrorsFound = true;
                }
            } break;
            case PVModel::TRNSYS: {
                ThisParamObj = Util::FindItemInList(state.dataPhotovoltaic->PVarray(PVnum).PerfObjName, tmpTRNSYSModuleParams);
                if (ThisParamObj > 0) {
                    state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule = tmpTRNSYSModuleParams(ThisParamObj); // entire structure assignment
                } else {
                    ShowSevereError(state, format("Invalid PV performance object name of {}", state.dataPhotovoltaic->PVarray(PVnum).PerfObjName));
                    ShowContinueError(state, format("Entered in {} = {}", cPVGeneratorObjectName, state.dataPhotovoltaic->PVarray(PVnum).Name));
                    ErrorsFound = true;
                }
            } break;
            case PVModel::Sandia: {
                ThisParamObj =
                    Util::FindItemInList(state.dataPhotovoltaic->PVarray(PVnum).PerfObjName, tmpSNLModuleParams, &SNLModuleParamsStuct::name);
                if (ThisParamObj > 0) {
                    state.dataPhotovoltaic->PVarray(PVnum).SNLPVModule = tmpSNLModuleParams(ThisParamObj); // entire structure assignment
                } else {
                    ShowSevereError(state, format("Invalid PV performance object name of {}", state.dataPhotovoltaic->PVarray(PVnum).PerfObjName));
                    ShowContinueError(state, format("Entered in {} = {}", cPVGeneratorObjectName, state.dataPhotovoltaic->PVarray(PVnum).Name));
                    ErrorsFound = true;
                }
            } break;
            default:
                break;
            }

            // set up report variables CurrentModuleObject='Photovoltaics'
            SetupOutputVariable(state,
                                "Generator Produced DC Electricity Rate",
                                Constant::Units::W,
                                state.dataPhotovoltaic->PVarray(PVnum).Report.DCPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataPhotovoltaic->PVarray(PVnum).Name);
            SetupOutputVariable(state,
                                "Generator Produced DC Electricity Energy",
                                Constant::Units::J,
                                state.dataPhotovoltaic->PVarray(PVnum).Report.DCEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataPhotovoltaic->PVarray(PVnum).Name,
                                Constant::eResource::ElectricityProduced,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::Photovoltaic);
            SetupOutputVariable(state,
                                "Generator PV Array Efficiency",
                                Constant::Units::None,
                                state.dataPhotovoltaic->PVarray(PVnum).Report.ArrayEfficiency,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataPhotovoltaic->PVarray(PVnum).Name);

            // CurrentModuleObject='Equiv1Diode or Sandia Photovoltaics'
            if ((state.dataPhotovoltaic->PVarray(PVnum).PVModelType == PVModel::TRNSYS) ||
                (state.dataPhotovoltaic->PVarray(PVnum).PVModelType == PVModel::Sandia)) {
                SetupOutputVariable(state,
                                    "Generator PV Cell Temperature",
                                    Constant::Units::C,
                                    state.dataPhotovoltaic->PVarray(PVnum).Report.CellTemp,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataPhotovoltaic->PVarray(PVnum).Name);
                SetupOutputVariable(state,
                                    "Generator PV Short Circuit Current",
                                    Constant::Units::A,
                                    state.dataPhotovoltaic->PVarray(PVnum).Report.ArrayIsc,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataPhotovoltaic->PVarray(PVnum).Name);
                SetupOutputVariable(state,
                                    "Generator PV Open Circuit Voltage",
                                    Constant::Units::V,
                                    state.dataPhotovoltaic->PVarray(PVnum).Report.ArrayVoc,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataPhotovoltaic->PVarray(PVnum).Name);
            }

            // do some checks and setup
            if (state.dataPhotovoltaic->PVarray(PVnum).CellIntegrationMode == CellIntegration::SurfaceOutsideFace) {
                // check that surface is HeatTransfer and a Construction with Internal Source was used
                if (!state.dataSurface->Surface(state.dataPhotovoltaic->PVarray(PVnum).SurfacePtr).HeatTransSurf) {
                    ShowSevereError(state,
                                    format("Must use a surface with heat transfer for IntegratedSurfaceOutsideFace mode in {}",
                                           state.dataPhotovoltaic->PVarray(PVnum).Name));
                    ErrorsFound = true;
                } else if (!state.dataConstruction
                                ->Construct(state.dataSurface->Surface(state.dataPhotovoltaic->PVarray(PVnum).SurfacePtr).Construction)
                                .SourceSinkPresent) {
                    ShowSevereError(state,
                                    format("Must use a surface with internal source construction for IntegratedSurfaceOutsideFace mode in {}",
                                           state.dataPhotovoltaic->PVarray(PVnum).Name));
                    ErrorsFound = true;
                }
            }

            if (state.dataPhotovoltaic->PVarray(PVnum).CellIntegrationMode == CellIntegration::TranspiredCollector) {
                GetTranspiredCollectorIndex(state, state.dataPhotovoltaic->PVarray(PVnum).SurfacePtr, state.dataPhotovoltaic->PVarray(PVnum).UTSCPtr);
            }

            if (state.dataPhotovoltaic->PVarray(PVnum).CellIntegrationMode == CellIntegration::ExteriorVentedCavity) {
                GetExtVentedCavityIndex(
                    state, state.dataPhotovoltaic->PVarray(PVnum).SurfacePtr, state.dataPhotovoltaic->PVarray(PVnum).ExtVentCavPtr);
            }

            if (state.dataPhotovoltaic->PVarray(PVnum).CellIntegrationMode == CellIntegration::PVTSolarCollector) {
                state.dataPhotovoltaic->PVarray(PVnum).PVTPtr = GetPVTmodelIndex(state, state.dataPhotovoltaic->PVarray(PVnum).SurfacePtr);
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, "Errors found in getting photovoltaic input");
        }
    }

    int GetPVZone(EnergyPlusData &state, int const SurfNum)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   Sept 2017

        // PURPOSE OF THIS SUBROUTINE:
        // Get the zone number for this PV array for use when zone multipliers are applied

        int GetPVZone(0);

        if (SurfNum > 0) {
            GetPVZone = state.dataSurface->Surface(SurfNum).Zone;
            if (GetPVZone == 0) { // might need to get the zone number from the name
                GetPVZone = Util::FindItemInList(state.dataSurface->Surface(SurfNum).ZoneName, state.dataHeatBal->Zone, state.dataGlobal->NumOfZones);
            }
        }

        return GetPVZone;
    }

    // **************************************

    void CalcSimplePV(EnergyPlusData &state, int const thisPV)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Jan. 2004
        //       MODIFIED       B. Griffith, Aug. 2008
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // calculate the electricity production using a simple PV model

        // Using/Aliasing
        Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ThisSurf; // working index ptr to Surface arrays
        Real64 Eff;   // working variable for solar electric efficiency

        ThisSurf = state.dataPhotovoltaic->PVarray(thisPV).SurfacePtr;

        if (state.dataHeatBal->SurfQRadSWOutIncident(ThisSurf) > DataPhotovoltaics::MinIrradiance) {

            // get efficiency
            switch (state.dataPhotovoltaic->PVarray(thisPV).SimplePVModule.EfficencyInputMode) {
            case Efficiency::Fixed: {
                Eff = state.dataPhotovoltaic->PVarray(thisPV).SimplePVModule.PVEfficiency;
            } break;
            case Efficiency::Scheduled: { // get from schedule
                Eff = state.dataPhotovoltaic->PVarray(thisPV).SimplePVModule.effSched->getCurrentVal();
                state.dataPhotovoltaic->PVarray(thisPV).SimplePVModule.PVEfficiency = Eff;
            } break;
            default: {
                Eff = 0.0; // Suppress uninitialized warning
                ShowSevereError(state, "caught bad Mode in Generator:Photovoltaic:Simple use FIXED or SCHEDULED efficiency mode");
            } break;
            }

            state.dataPhotovoltaic->PVarray(thisPV).Report.DCPower =
                state.dataPhotovoltaic->PVarray(thisPV).SimplePVModule.AreaCol * Eff *
                state.dataHeatBal->SurfQRadSWOutIncident(
                    ThisSurf); // active solar cellsurface net area | solar conversion efficiency | solar incident

            // store sink term in appropriate place for surface heat transfer integration
            state.dataPhotovoltaic->PVarray(thisPV).SurfaceSink = state.dataPhotovoltaic->PVarray(thisPV).Report.DCPower;

            // array energy, power * timestep
            state.dataPhotovoltaic->PVarray(thisPV).Report.DCEnergy = state.dataPhotovoltaic->PVarray(thisPV).Report.DCPower * TimeStepSysSec;
            state.dataPhotovoltaic->PVarray(thisPV).Report.ArrayEfficiency = Eff;
        } else { // not enough incident solar, zero things out

            state.dataPhotovoltaic->PVarray(thisPV).SurfaceSink = 0.0;
            state.dataPhotovoltaic->PVarray(thisPV).Report.DCEnergy = 0.0;
            state.dataPhotovoltaic->PVarray(thisPV).Report.DCPower = 0.0;
            state.dataPhotovoltaic->PVarray(thisPV).Report.ArrayEfficiency = 0.0;
        }
    }

    void ReportPV(EnergyPlusData &state, int const PVnum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Jan. 2004
        //       MODIFIED       B. Griffith, Aug. 2008

        // PURPOSE OF THIS SUBROUTINE:
        // collect statements that assign to variables tied to output variables

        // Using/Aliasing
        Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;
        using TranspiredCollector::SetUTSCQdotSource;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int thisZone; // working index for zones

        state.dataPhotovoltaic->PVarray(PVnum).Report.DCEnergy = state.dataPhotovoltaic->PVarray(PVnum).Report.DCPower * TimeStepSysSec;

        // add check for multiplier.  if surface is attached to a zone that is on a multiplier
        // then PV production should be multiplied out as well

        thisZone = state.dataPhotovoltaic->PVarray(PVnum).Zone;
        if (thisZone != 0) { // might need to apply multiplier
            state.dataPhotovoltaic->PVarray(PVnum).Report.DCEnergy *=
                (state.dataHeatBal->Zone(thisZone).Multiplier * state.dataHeatBal->Zone(thisZone).ListMultiplier);
            state.dataPhotovoltaic->PVarray(PVnum).Report.DCPower *=
                (state.dataHeatBal->Zone(thisZone).Multiplier * state.dataHeatBal->Zone(thisZone).ListMultiplier);
        }

        switch (state.dataPhotovoltaic->PVarray(PVnum).CellIntegrationMode) {
            // SurfaceSink is not multiplied...
        case CellIntegration::SurfaceOutsideFace: {
            state.dataHeatBalFanSys->QPVSysSource(state.dataPhotovoltaic->PVarray(PVnum).SurfacePtr) =
                -1.0 * state.dataPhotovoltaic->PVarray(PVnum).SurfaceSink;
        } break;
        case CellIntegration::TranspiredCollector: {
            SetUTSCQdotSource(state, state.dataPhotovoltaic->PVarray(PVnum).UTSCPtr, -1.0 * state.dataPhotovoltaic->PVarray(PVnum).SurfaceSink);
        } break;
        case CellIntegration::ExteriorVentedCavity: {
            SetVentedModuleQdotSource(
                state, state.dataPhotovoltaic->PVarray(PVnum).ExtVentCavPtr, -1.0 * state.dataPhotovoltaic->PVarray(PVnum).SurfaceSink);
        } break;
        case CellIntegration::PVTSolarCollector: {
            PhotovoltaicThermalCollectors::SetPVTQdotSource(
                state, state.dataPhotovoltaic->PVarray(PVnum).PVTPtr, -1.0 * state.dataPhotovoltaic->PVarray(PVnum).SurfaceSink);
        } break;
        default:
            break;
        }
    }

    // *************

    void CalcSandiaPV(EnergyPlusData &state,
                      int const PVnum,   // ptr to current PV system
                      bool const RunFlag // controls if generator is scheduled *ON*
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith , (derived from Greg Barker's TRNSYS type101 for SANDIA PV model)
        //       DATE WRITTEN   Jan 2004
        //       MODIFIED       B. Griffith, Aug. 2008 reworked for new, single-PV-generator data structure
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculate various PV system performance indices at the current timestep

        // METHODOLOGY EMPLOYED:
        //  adapted code from a set of F77 routines by G. Barker that implement the model
        //  This routines works on a single photovoltaic object of the type 'GENERATOR:PV:SANDIA'
        //  Each major model equation has its own function (in this module)

        // REFERENCES:
        // King, David L. . Photovoltaic module and array performance characterization methods for all
        //   system operating conditions. Pro. NREL/SNL Photovoltaics Program Review, AIP Press, Lakewood CO
        //   Sandia National Laboratories

        // Davis, M.W., A.H. Fanney, and B.P. Dougherty. Measured versus predicted performance of Building
        //    integrated photovoltaics. Solar 2002, Sunrise on the Reliable Energy Economy, June 15-19, 2002 Reno, NV

        // Using/Aliasing
        using PhotovoltaicThermalCollectors::GetPVTTsColl;
        using TranspiredCollector::GetUTSCTsColl;

        int ThisSurf; // working variable for indexing surfaces
        Real64 Ee;

        ThisSurf = state.dataPhotovoltaic->PVarray(PVnum).SurfacePtr;
        auto &thisPVarray = state.dataPhotovoltaic->PVarray(PVnum);

        //   get input from elsewhere in Energyplus for the current point in the simulation
        thisPVarray.SNLPVinto.IcBeam = state.dataHeatBal->SurfQRadSWOutIncidentBeam(ThisSurf); //(W/m2)from DataHeatBalance
        thisPVarray.SNLPVinto.IcDiffuse =
            state.dataHeatBal->SurfQRadSWOutIncident(ThisSurf) - state.dataHeatBal->SurfQRadSWOutIncidentBeam(ThisSurf); //(W/ m2)(was kJ/hr m2)
        thisPVarray.SNLPVinto.IncidenceAngle =
            std::acos(state.dataHeatBal->SurfCosIncidenceAngle(ThisSurf)) / Constant::DegToRad;         // (deg) from dataHeatBalance
        thisPVarray.SNLPVinto.ZenithAngle = std::acos(state.dataEnvrn->SOLCOS(3)) / Constant::DegToRad; //(degrees),
        thisPVarray.SNLPVinto.Tamb = state.dataSurface->SurfOutDryBulbTemp(ThisSurf);                   //(deg. C)
        thisPVarray.SNLPVinto.WindSpeed = state.dataSurface->SurfOutWindSpeed(ThisSurf);                // (m/s)
        thisPVarray.SNLPVinto.Altitude = state.dataEnvrn->Elevation;                                    // from DataEnvironment via USE

        if (((thisPVarray.SNLPVinto.IcBeam + thisPVarray.SNLPVinto.IcDiffuse) > DataPhotovoltaics::MinIrradiance) && (RunFlag)) {

            // first determine PV cell temperatures depending on model
            switch (thisPVarray.CellIntegrationMode) {
            case CellIntegration::Decoupled: { // Sandia module temperature model for rack mounted PVs
                // Calculate back-of-module temperature:
                thisPVarray.SNLPVCalc.Tback = SandiaModuleTemperature(thisPVarray.SNLPVinto.IcBeam,
                                                                      thisPVarray.SNLPVinto.IcDiffuse,
                                                                      thisPVarray.SNLPVinto.WindSpeed,
                                                                      thisPVarray.SNLPVinto.Tamb,
                                                                      thisPVarray.SNLPVModule.fd,
                                                                      thisPVarray.SNLPVModule.a,
                                                                      thisPVarray.SNLPVModule.b);

                // Calculate cell temperature:
                thisPVarray.SNLPVCalc.Tcell = SandiaTcellFromTmodule(thisPVarray.SNLPVCalc.Tback,
                                                                     thisPVarray.SNLPVinto.IcBeam,
                                                                     thisPVarray.SNLPVinto.IcDiffuse,
                                                                     thisPVarray.SNLPVModule.fd,
                                                                     thisPVarray.SNLPVModule.DT0);

            } break;
            case CellIntegration::SurfaceOutsideFace: {
                // get back-of-module temperature from elsewhere in EnergyPlus
                thisPVarray.SNLPVCalc.Tback = state.dataHeatBalSurf->SurfTempOut(thisPVarray.SurfacePtr);

                thisPVarray.SNLPVCalc.Tcell = SandiaTcellFromTmodule(thisPVarray.SNLPVCalc.Tback,
                                                                     thisPVarray.SNLPVinto.IcBeam,
                                                                     thisPVarray.SNLPVinto.IcDiffuse,
                                                                     thisPVarray.SNLPVModule.fd,
                                                                     thisPVarray.SNLPVModule.DT0);

            } break;
            case CellIntegration::TranspiredCollector: {
                GetUTSCTsColl(state, thisPVarray.UTSCPtr, thisPVarray.SNLPVCalc.Tback);

                thisPVarray.SNLPVCalc.Tcell = SandiaTcellFromTmodule(thisPVarray.SNLPVCalc.Tback,
                                                                     thisPVarray.SNLPVinto.IcBeam,
                                                                     thisPVarray.SNLPVinto.IcDiffuse,
                                                                     thisPVarray.SNLPVModule.fd,
                                                                     thisPVarray.SNLPVModule.DT0);

            } break;
            case CellIntegration::ExteriorVentedCavity: {
                GetExtVentedCavityTsColl(state, thisPVarray.ExtVentCavPtr, thisPVarray.SNLPVCalc.Tback);

                thisPVarray.SNLPVCalc.Tcell = SandiaTcellFromTmodule(thisPVarray.SNLPVCalc.Tback,
                                                                     thisPVarray.SNLPVinto.IcBeam,
                                                                     thisPVarray.SNLPVinto.IcDiffuse,
                                                                     thisPVarray.SNLPVModule.fd,
                                                                     thisPVarray.SNLPVModule.DT0);

            } break;
            case CellIntegration::PVTSolarCollector: {
                GetPVTTsColl(state, thisPVarray.PVTPtr, thisPVarray.SNLPVCalc.Tback);
                thisPVarray.SNLPVCalc.Tcell = SandiaTcellFromTmodule(thisPVarray.SNLPVCalc.Tback,
                                                                     thisPVarray.SNLPVinto.IcBeam,
                                                                     thisPVarray.SNLPVinto.IcDiffuse,
                                                                     thisPVarray.SNLPVModule.fd,
                                                                     thisPVarray.SNLPVModule.DT0);
            } break;
            default: {
                ShowSevereError(state, format("Sandia PV Simulation Temperature Modeling Mode Error in {}", thisPVarray.Name));
            } break;
            }

            // Calculate Air Mass function
            thisPVarray.SNLPVCalc.AMa = AbsoluteAirMass(thisPVarray.SNLPVinto.ZenithAngle, thisPVarray.SNLPVinto.Altitude);

            // Calculate F1 polynomial function:
            thisPVarray.SNLPVCalc.F1 = SandiaF1(thisPVarray.SNLPVCalc.AMa,
                                                thisPVarray.SNLPVModule.a_0,
                                                thisPVarray.SNLPVModule.a_1,
                                                thisPVarray.SNLPVModule.a_2,
                                                thisPVarray.SNLPVModule.a_3,
                                                thisPVarray.SNLPVModule.a_4);

            // Calculate F2 polynomial function:
            thisPVarray.SNLPVCalc.F2 = SandiaF2(thisPVarray.SNLPVinto.IncidenceAngle,
                                                thisPVarray.SNLPVModule.b_0,
                                                thisPVarray.SNLPVModule.b_1,
                                                thisPVarray.SNLPVModule.b_2,
                                                thisPVarray.SNLPVModule.b_3,
                                                thisPVarray.SNLPVModule.b_4,
                                                thisPVarray.SNLPVModule.b_5);

            // Calculate short-circuit current function:
            thisPVarray.SNLPVCalc.Isc = SandiaIsc(thisPVarray.SNLPVCalc.Tcell,
                                                  thisPVarray.SNLPVModule.Isc0,
                                                  thisPVarray.SNLPVinto.IcBeam,
                                                  thisPVarray.SNLPVinto.IcDiffuse,
                                                  thisPVarray.SNLPVCalc.F1,
                                                  thisPVarray.SNLPVCalc.F2,
                                                  thisPVarray.SNLPVModule.fd,
                                                  thisPVarray.SNLPVModule.aIsc);

            // Calculate effective irradiance function:
            Ee = SandiaEffectiveIrradiance(
                thisPVarray.SNLPVCalc.Tcell, thisPVarray.SNLPVCalc.Isc, thisPVarray.SNLPVModule.Isc0, thisPVarray.SNLPVModule.aIsc);
            // Calculate Imp function:
            thisPVarray.SNLPVCalc.Imp = SandiaImp(thisPVarray.SNLPVCalc.Tcell,
                                                  Ee,
                                                  thisPVarray.SNLPVModule.Imp0,
                                                  thisPVarray.SNLPVModule.aImp,
                                                  thisPVarray.SNLPVModule.c_0,
                                                  thisPVarray.SNLPVModule.c_1);

            // Calculate Voc function:
            thisPVarray.SNLPVCalc.Voc = SandiaVoc(thisPVarray.SNLPVCalc.Tcell,
                                                  Ee,
                                                  thisPVarray.SNLPVModule.Voc0,
                                                  thisPVarray.SNLPVModule.NcellSer,
                                                  thisPVarray.SNLPVModule.DiodeFactor,
                                                  thisPVarray.SNLPVModule.BVoc0,
                                                  thisPVarray.SNLPVModule.mBVoc);

            // Calculate Vmp: voltage at maximum powerpoint
            thisPVarray.SNLPVCalc.Vmp = SandiaVmp(thisPVarray.SNLPVCalc.Tcell,
                                                  Ee,
                                                  thisPVarray.SNLPVModule.Vmp0,
                                                  thisPVarray.SNLPVModule.NcellSer,
                                                  thisPVarray.SNLPVModule.DiodeFactor,
                                                  thisPVarray.SNLPVModule.BVmp0,
                                                  thisPVarray.SNLPVModule.mBVmp,
                                                  thisPVarray.SNLPVModule.c_2,
                                                  thisPVarray.SNLPVModule.c_3);

            // Calculate Ix function:
            thisPVarray.SNLPVCalc.Ix = SandiaIx(thisPVarray.SNLPVCalc.Tcell,
                                                Ee,
                                                thisPVarray.SNLPVModule.Ix0,
                                                thisPVarray.SNLPVModule.aIsc,
                                                thisPVarray.SNLPVModule.aImp,
                                                thisPVarray.SNLPVModule.c_4,
                                                thisPVarray.SNLPVModule.c_5);

            // Calculate Vx function:
            thisPVarray.SNLPVCalc.Vx = thisPVarray.SNLPVCalc.Voc / 2.0;

            // Calculate Ixx function:
            thisPVarray.SNLPVCalc.Ixx = SandiaIxx(thisPVarray.SNLPVCalc.Tcell,
                                                  Ee,
                                                  thisPVarray.SNLPVModule.Ixx0,
                                                  thisPVarray.SNLPVModule.aImp,
                                                  thisPVarray.SNLPVModule.c_6,
                                                  thisPVarray.SNLPVModule.c_7);
            // Calculate Vxx :
            thisPVarray.SNLPVCalc.Vxx = 0.5 * (thisPVarray.SNLPVCalc.Voc + thisPVarray.SNLPVCalc.Vmp);

            // Calculate Pmp, single module: power at maximum powerpoint
            thisPVarray.SNLPVCalc.Pmp = thisPVarray.SNLPVCalc.Imp * thisPVarray.SNLPVCalc.Vmp; // W

            // Calculate PV efficiency at maximum power point
            thisPVarray.SNLPVCalc.EffMax =
                thisPVarray.SNLPVCalc.Pmp / (thisPVarray.SNLPVinto.IcBeam + thisPVarray.SNLPVinto.IcDiffuse) / thisPVarray.SNLPVModule.Acoll;

            // Scale to NumStrings and NumSeries:
            thisPVarray.SNLPVCalc.Pmp *= thisPVarray.NumSeriesNParall * thisPVarray.NumModNSeries;
            thisPVarray.SNLPVCalc.Imp *= thisPVarray.NumModNSeries;
            thisPVarray.SNLPVCalc.Vmp *= thisPVarray.NumModNSeries;
            thisPVarray.SNLPVCalc.Isc *= thisPVarray.NumSeriesNParall;
            thisPVarray.SNLPVCalc.Voc *= thisPVarray.NumModNSeries;
            thisPVarray.SNLPVCalc.Ix *= thisPVarray.NumSeriesNParall;
            thisPVarray.SNLPVCalc.Ixx *= thisPVarray.NumSeriesNParall;
            thisPVarray.SNLPVCalc.Vx *= thisPVarray.NumModNSeries;
            thisPVarray.SNLPVCalc.Vxx *= thisPVarray.NumModNSeries;
            thisPVarray.SNLPVCalc.SurfaceSink = thisPVarray.SNLPVCalc.Pmp;
        } else { // Ibeam+Idiff < MaxIrradiance or not RunFlag
            // so zero things.
            thisPVarray.SNLPVCalc.Vmp = 0.0;
            thisPVarray.SNLPVCalc.Imp = 0.0;
            thisPVarray.SNLPVCalc.Pmp = 0.0;
            thisPVarray.SNLPVCalc.EffMax = 0.0;
            thisPVarray.SNLPVCalc.Isc = 0.0;
            thisPVarray.SNLPVCalc.Voc = 0.0;
            thisPVarray.SNLPVCalc.Tcell = thisPVarray.SNLPVinto.Tamb;
            thisPVarray.SNLPVCalc.Tback = thisPVarray.SNLPVinto.Tamb;
            thisPVarray.SNLPVCalc.AMa = 999.0;
            thisPVarray.SNLPVCalc.F1 = 0.0;
            thisPVarray.SNLPVCalc.F2 = 0.0;
            thisPVarray.SNLPVCalc.Ix = 0.0;
            thisPVarray.SNLPVCalc.Vx = 0.0;
            thisPVarray.SNLPVCalc.Ixx = 0.0;
            thisPVarray.SNLPVCalc.Vxx = 0.0;
            thisPVarray.SNLPVCalc.SurfaceSink = 0.0;
        } // Ibeam+Idiff > MinIrradiance and runflag

        // update calculations to report variables
        thisPVarray.Report.DCPower = thisPVarray.SNLPVCalc.Pmp;
        thisPVarray.Report.ArrayIsc = thisPVarray.SNLPVCalc.Isc;
        thisPVarray.Report.ArrayVoc = thisPVarray.SNLPVCalc.Voc;
        thisPVarray.Report.CellTemp = thisPVarray.SNLPVCalc.Tcell;
        thisPVarray.Report.ArrayEfficiency = thisPVarray.SNLPVCalc.EffMax;
        thisPVarray.SurfaceSink = thisPVarray.SNLPVCalc.SurfaceSink;
    }

    // ********************
    // begin routines for Equivalent one-diode model by Bradley/Ulleberg

    void InitTRNSYSPV(EnergyPlusData &state, int const PVnum) // the number of the GENERATOR:PHOTOVOLTAICS (passed in)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         David Bradley
        //       DATE WRITTEN   April 2003
        //       MODIFIED       BG March 2007 reworked for CR7109 (reverse DD testing)
        //                      B. Griffith, Aug. 2008 reworked for new, single-PV-generator data structure
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine initializes the PV arrays during simulation. It performs both start of
        // simulation initializations and start of timestep initializations. The structure of the
        // subroutine was taken from InitBaseboard.

        // Using/Aliasing
        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 TimeElapsed; // Fraction of the current hour that has elapsed (h)

        // perform the one time initializations
        if (state.dataPhotovoltaicState->MyOneTimeFlag) {
            // initialize the environment and sizing flags
            state.dataPhotovoltaicState->MyEnvrnFlag.dimension(state.dataPhotovoltaic->NumPVs, true);
            state.dataPhotovoltaicState->MyOneTimeFlag = false;
        }

        // Do the Begin Environment initializations
        if (state.dataGlobal->BeginEnvrnFlag && state.dataPhotovoltaicState->MyEnvrnFlag(PVnum)) {
            state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVcalc.CellTempK =
                state.dataSurface->SurfOutDryBulbTemp(state.dataPhotovoltaic->PVarray(PVnum).SurfacePtr) + Constant::Kelvin;
            state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVcalc.LastCellTempK =
                state.dataSurface->SurfOutDryBulbTemp(state.dataPhotovoltaic->PVarray(PVnum).SurfacePtr) + Constant::Kelvin;
            state.dataPhotovoltaicState->MyEnvrnFlag(PVnum) = false;
        }

        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataPhotovoltaicState->MyEnvrnFlag(PVnum) = true;
        }

        // Do the beginning of every time step initializations
        TimeElapsed =
            state.dataGlobal->HourOfDay + state.dataGlobal->TimeStep * state.dataGlobal->TimeStepZone + state.dataHVACGlobal->SysTimeElapsed;
        if (state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVcalc.TimeElapsed != TimeElapsed) {
            // The simulation has advanced to the next system timestep.  Save conditions from the end of the previous system
            state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVcalc.LastCellTempK = state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVcalc.CellTempK;
            state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVcalc.TimeElapsed = TimeElapsed;
        }

        if (any_gt(state.dataHeatBal->SurfQRadSWOutIncident, 0.0)) {
            //  Determine the amount of radiation incident on each PV
            state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVcalc.Insolation =
                state.dataHeatBal->SurfQRadSWOutIncident(state.dataPhotovoltaic->PVarray(PVnum).SurfacePtr); //[W/m2]
        } else {
            state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVcalc.Insolation = 0.0;
        }
    }

    // *************

    void CalcTRNSYSPV(EnergyPlusData &state,
                      int const PVnum,   // BTG added intent
                      bool const RunFlag // BTG added intent    !flag tells whether the PV is ON or OFF
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         D. Bradley
        //       DATE WRITTEN   April 2003
        //       MODIFIED       B. Griffith, February 2008-- added support for inverter
        //                      multipliers, and building integrated heat transfer
        //                      B. Griffith, Aug. 2008 reworked for new, single-PV-generator data structure
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine simulates the PV performance.

        using PhotovoltaicThermalCollectors::GetPVTTsColl;
        using TranspiredCollector::GetUTSCTsColl;

        Real64 constexpr EPS(0.001);
        Real64 constexpr ERR(0.001);
        Real64 constexpr MinInsolation(30.0);
        int constexpr KMAX(100);
        Real64 constexpr EtaIni(0.10); // initial value of eta
        Real64 DummyErr;
        Real64 ETA;
        Real64 Tambient;
        Real64 EtaOld;
        Real64 ILRef;
        Real64 AARef;
        Real64 IORef;
        Real64 SeriesResistance;
        Real64 IL;
        Real64 AA;
        Real64 IO;
        Real64 ISCG1;
        Real64 ISC;
        Real64 VOCG1;
        Real64 VOC;
        Real64 VLEFT;
        Real64 VRIGHT;
        Real64 VM;
        Real64 IM;
        Real64 PM;
        Real64 IA;
        Real64 ISCA;
        Real64 VA;
        Real64 VOCA;
        Real64 PA;
        Real64 CellTemp(0.0); // cell temperature in Kelvin
        Real64 CellTempC;     // cell temperature in degrees C

        // if the cell temperature mode is 2, convert the timestep to seconds
        if (state.dataPhotovoltaicState->firstTime &&
            state.dataPhotovoltaic->PVarray(PVnum).CellIntegrationMode == CellIntegration::DecoupledUllebergDynamic) {
            state.dataPhotovoltaicState->PVTimeStep = double(state.dataGlobal->MinutesInTimeStep) * 60.0; // Seconds per time step
        }
        state.dataPhotovoltaicState->firstTime = false;

        // place the shunt resistance into its common block
        state.dataPhotovoltaic->ShuntResistance = state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.ShuntResistance;

        // convert ambient temperature from C to K
        Tambient = state.dataSurface->SurfOutDryBulbTemp(state.dataPhotovoltaic->PVarray(PVnum).SurfacePtr) + Constant::Kelvin;

        auto const &thisPVarray = state.dataPhotovoltaic->PVarray(PVnum);

        if ((state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVcalc.Insolation > MinInsolation) && (RunFlag)) {

            // set initial values for eta iteration loop
            DummyErr = 2.0 * ERR;
            EtaOld = EtaIni;
            int K;

            // Begin DO WHILE loop - until the error tolerance is reached.
            ETA = 0.0;
            while (DummyErr > ERR) {
                switch (state.dataPhotovoltaic->PVarray(PVnum).CellIntegrationMode) {
                case CellIntegration::Decoupled: {
                    //  cell temperature based on energy balance
                    state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.HeatLossCoef =
                        state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.TauAlpha *
                        state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.NOCTInsolation /
                        (state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.NOCTCellTemp -
                         state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.NOCTAmbTemp);
                    CellTemp = Tambient + (state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVcalc.Insolation *
                                           state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.TauAlpha /
                                           state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.HeatLossCoef) *
                                              (1.0 - ETA / state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.TauAlpha);
                } break;
                case CellIntegration::DecoupledUllebergDynamic: {
                    //  cell temperature based on energy balance with thermal capacity effects
                    CellTemp =
                        Tambient +
                        (state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVcalc.LastCellTempK - Tambient) *
                            std::exp(-state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.HeatLossCoef /
                                     state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.HeatCapacity * state.dataPhotovoltaicState->PVTimeStep) +
                        (state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.TauAlpha - ETA) *
                            state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVcalc.Insolation /
                            state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.HeatLossCoef *
                            (1.0 -
                             std::exp(-state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.HeatLossCoef /
                                      state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.HeatCapacity * state.dataPhotovoltaicState->PVTimeStep));
                } break;
                case CellIntegration::SurfaceOutsideFace: {
                    CellTemp = state.dataHeatBalSurf->SurfTempOut(state.dataPhotovoltaic->PVarray(PVnum).SurfacePtr) + Constant::Kelvin;
                } break;
                case CellIntegration::TranspiredCollector: {
                    GetUTSCTsColl(state, state.dataPhotovoltaic->PVarray(PVnum).UTSCPtr, CellTemp);
                    CellTemp += Constant::Kelvin;
                } break;
                case CellIntegration::ExteriorVentedCavity: {
                    GetExtVentedCavityTsColl(state, state.dataPhotovoltaic->PVarray(PVnum).ExtVentCavPtr, CellTemp);
                    CellTemp += Constant::Kelvin;
                } break;
                case CellIntegration::PVTSolarCollector: {
                    GetPVTTsColl(state, thisPVarray.PVTPtr, CellTemp);
                    CellTemp += Constant::Kelvin;
                } break;
                default:
                    break;
                }

                //  reference parameters
                ILRef = state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.RefIsc;
                AARef = (state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.TempCoefVoc *
                             state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.RefTemperature -
                         state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.RefVoc +
                         state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.SemiConductorBandgap *
                             state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.CellsInSeries) /
                        (state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.TempCoefIsc *
                             state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.RefTemperature / ILRef -
                         3.0);
                IORef = ILRef * std::exp(-state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.RefVoc / AARef);

                //  series resistance
                SeriesResistance =
                    (AARef * std::log(1.0 - state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.Imp / ILRef) -
                     state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.Vmp + state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.RefVoc) /
                    state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.Imp;

                //  temperature dependence
                IL = state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVcalc.Insolation /
                     state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.RefInsolation *
                     (ILRef + state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.TempCoefIsc *
                                  (CellTemp - state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.RefTemperature));
                Real64 const cell_temp_ratio(CellTemp / state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.RefTemperature);
                AA = AARef * cell_temp_ratio;
                IO = IORef * pow_3(cell_temp_ratio) *
                     std::exp(state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.SemiConductorBandgap *
                              state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.CellsInSeries / AARef *
                              (1.0 - state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.RefTemperature / CellTemp));

                //  compute short circuit current and open circuit voltage

                //   NEWTON --> ISC  (STARTVALUE: ISCG1 - BASED ON IL=ISC)
                ISCG1 = IL;
                NEWTON(state, ISC, FUN, FI, ISC, DataPrecisionGlobals::constant_zero, IO, IL, SeriesResistance, AA, ISCG1, EPS);

                //   NEWTON --> VOC  (STARTVALUE: VOCG1 - BASED ON IM=0.0)
                VOCG1 = (std::log(IL / IO) + 1.0) * AA;
                NEWTON(state, VOC, FUN, FV, DataPrecisionGlobals::constant_zero, VOC, IO, IL, SeriesResistance, AA, VOCG1, EPS);

                //  maximum power point tracking

                //   SEARCH --> VM AT MAXIMUM POWER POINT
                VLEFT = 0.0;
                VRIGHT = VOC;
                SEARCH(state, VLEFT, VRIGHT, VM, K, IO, IL, SeriesResistance, AA, EPS, KMAX);

                //   POWER --> IM & PM AT MAXIMUM POWER POINT
                POWER(state, IO, IL, SeriesResistance, AA, EPS, IM, VM, PM);

                // calculate overall PV module efficiency
                ETA =
                    PM / state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVcalc.Insolation / state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.Area;
                DummyErr = std::abs((ETA - EtaOld) / EtaOld);
                EtaOld = ETA;

            } // while

        } else {
            // if there is no incident radiation or if the control switch is 'Off'
            switch (state.dataPhotovoltaic->PVarray(PVnum).CellIntegrationMode) {
            case CellIntegration::Decoupled: {
                CellTemp = Tambient;
            } break;
            case CellIntegration::DecoupledUllebergDynamic: {
                CellTemp = Tambient +
                           (state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVcalc.LastCellTempK - Tambient) *
                               std::exp(-state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.HeatLossCoef /
                                        state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVModule.HeatCapacity * state.dataPhotovoltaicState->PVTimeStep);
            } break;
            case CellIntegration::SurfaceOutsideFace: {
                CellTemp = state.dataHeatBalSurf->SurfTempOut(state.dataPhotovoltaic->PVarray(PVnum).SurfacePtr) + Constant::Kelvin;
            } break;
            case CellIntegration::TranspiredCollector: {
                GetUTSCTsColl(state, state.dataPhotovoltaic->PVarray(PVnum).UTSCPtr, CellTemp);
                CellTemp += Constant::Kelvin;
            } break;
            case CellIntegration::ExteriorVentedCavity: {
                GetExtVentedCavityTsColl(state, state.dataPhotovoltaic->PVarray(PVnum).ExtVentCavPtr, CellTemp);
                CellTemp += Constant::Kelvin;
            } break;
            case CellIntegration::PVTSolarCollector: {
                GetPVTTsColl(state, thisPVarray.PVTPtr, CellTemp);
                CellTemp += Constant::Kelvin;
            } break;
            default: {
                assert(false);
            } break;
            }

            state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVcalc.Insolation = 0.0;
            IM = 0.0;  // module current
            VM = 0.0;  // module voltage
            PM = 0.0;  // module power
            ETA = 0.0; // module efficiency
            ISC = 0.0;
            VOC = 0.0;
        }

        // convert cell temperature back to C
        CellTempC = CellTemp - Constant::Kelvin;

        // calculate array based outputs (so far, the outputs are module based
        IA = state.dataPhotovoltaic->PVarray(PVnum).NumSeriesNParall * IM;
        ISCA = state.dataPhotovoltaic->PVarray(PVnum).NumSeriesNParall * ISC;
        VA = state.dataPhotovoltaic->PVarray(PVnum).NumModNSeries * VM;
        VOCA = state.dataPhotovoltaic->PVarray(PVnum).NumModNSeries * VOC;
        PA = IA * VA;

        // Place local variables into the reporting structure
        state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVcalc.ArrayCurrent = IA;
        state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVcalc.ArrayVoltage = VA;
        state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVcalc.ArrayPower = PA;
        state.dataPhotovoltaic->PVarray(PVnum).Report.DCPower = PA;
        state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVcalc.ArrayEfficiency = ETA;
        state.dataPhotovoltaic->PVarray(PVnum).Report.ArrayEfficiency = ETA;
        state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVcalc.CellTemp = CellTempC;
        state.dataPhotovoltaic->PVarray(PVnum).Report.CellTemp = CellTempC;
        state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVcalc.CellTempK = CellTemp;
        state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVcalc.ArrayIsc = ISCA;
        state.dataPhotovoltaic->PVarray(PVnum).Report.ArrayIsc = ISCA;
        state.dataPhotovoltaic->PVarray(PVnum).TRNSYSPVcalc.ArrayVoc = VOCA;
        state.dataPhotovoltaic->PVarray(PVnum).Report.ArrayVoc = VOCA;
        state.dataPhotovoltaic->PVarray(PVnum).SurfaceSink = PA;
    }

    void POWER(EnergyPlusData &state,
               Real64 const IO,   // passed in from CalcPV
               Real64 const IL,   // passed in from CalcPV
               Real64 const RSER, // passed in from CalcPV
               Real64 const AA,   // passed in from CalcPV
               Real64 const EPS,  // passed in from CalcPV
               Real64 &II,        // current [A]
               Real64 const VV,   // voltage [V]
               Real64 &PP         // power [W]
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         O. Ulleberg, IFE Norway for Hydrogems
        //       DATE WRITTEN   March 2001
        //       MODIFIED       D. Bradley for use with EnergyPlus
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine calculates the power produced by the PV.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 IG1;

        // NEWTON --> II (STARTVALUE: IG1 BASED ON SIMPLIFIED I(I,V) EQUATION)
        IG1 = IL - IO * std::exp(VV / AA - 1.0);
        NEWTON(state, II, FUN, FI, II, VV, IO, IL, RSER, AA, IG1, EPS);
        PP = II * VV;
    }

    void NEWTON(EnergyPlusData &state,
                Real64 &XX,
                std::function<Real64(EnergyPlusData &state, Real64 const, Real64 const, Real64 const, Real64 const, Real64 const, Real64 const)> FXX,
                std::function<Real64(EnergyPlusData &state, Real64 const, Real64 const, Real64 const, Real64 const, Real64 const)> DER,
                Real64 const &II, // Autodesk Aliased to XX in some calls
                Real64 const &VV, // Autodesk Aliased to XX in some calls
                Real64 const IO,
                Real64 const IL,
                Real64 const RSER,
                Real64 const AA,
                Real64 const XS,
                Real64 const EPS)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         O. Ulleberg, IFE Norway for Hydrogems
        //       DATE WRITTEN   March 2001
        //       MODIFIED       D. Bradley for use with EnergyPlus
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine uses the Newton-Raphson method to solve a non linear equation with one variable.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int COUNT;
        Real64 ERR;
        Real64 X0;

        COUNT = 0;
        XX = XS;
        ERR = 1.0;
        while ((ERR > EPS) && (COUNT <= 10)) {
            X0 = XX;
            XX -= FXX(state, II, VV, IL, IO, RSER, AA) / DER(state, II, VV, IO, RSER, AA);
            ++COUNT;
            ERR = std::abs((XX - X0) / X0);
        }
    }

    void SEARCH(EnergyPlusData &state,
                Real64 &A,
                Real64 &B,
                Real64 &P,
                int &K,
                Real64 const IO,
                Real64 const IL,
                Real64 const RSER,
                Real64 const AA,
                Real64 const EPS,
                int const KMAX)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         O. Ulleberg, IFE Norway for Hydrogems
        //       DATE WRITTEN   March 2001
        //       MODIFIED       D. Bradley for use with EnergyPlus
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine minimum of an unimodal function with one variable. The algorithm was
        // adapted to find the maximum power point of a PV module. The changes to the original
        // algorithm are the following:
        // 1. a subroutine "POWER" is called in order to calculate the power output of the PV module
        // 2. the negative of the power of the PV module is taken so that the optimum can be found.

        // REFERENCES:
        //   /1/ MATHEWS, JOHN H.  NUMERICAL METHODS:  FORTRAN PROGRAMS. 1992, PP 413.
        //   /2/ NUMERICAL METHODS FOR MATHEMATICS, SCIENCE AND ENGINEERING, 2ND EDITION,
        //       PRENTICE HALL, NEW JERSEY, 1992.

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr DELTA(1.e-3);
        Real64 constexpr EPSILON(1.e-3);
        static Real64 const RONE((std::sqrt(5.0) - 1.0) / 2.0);
        static Real64 const RTWO(RONE * RONE);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 C;
        Real64 D;
        Real64 H;
        Real64 YP;
        Real64 YA;
        Real64 YB;
        Real64 YC;
        Real64 YD;
        Real64 IM;
        Real64 PM;

        H = B - A;
        POWER(state, IO, IL, RSER, AA, EPS, IM, A, PM);
        YA = -1.0 * PM;
        POWER(state, IO, IL, RSER, AA, EPS, IM, B, PM);
        YB = -1.0 * PM;
        C = A + RTWO * H;
        D = A + RONE * H;
        POWER(state, IO, IL, RSER, AA, EPS, IM, C, PM);
        YC = -1.0 * PM;
        POWER(state, IO, IL, RSER, AA, EPS, IM, D, PM);
        YD = -1.0 * PM;
        K = 1;
        while (std::abs(YB - YA) > EPSILON || H > DELTA) {
            if (YC < YD) {
                B = D;
                YB = YD;
                D = C;
                YD = YC;
                H = B - A;
                C = A + RTWO * H;
                POWER(state, IO, IL, RSER, AA, EPS, IM, C, PM);
                YC = -1.0 * PM;
            } else {
                A = C;
                YA = YC;
                C = D;
                YC = YD;
                H = B - A;
                D = A + RONE * H;
                POWER(state, IO, IL, RSER, AA, EPS, IM, D, PM);
                YD = -1.0 * PM;
            }
            ++K;
        }
        if (K < KMAX) {
            P = A;
            YP = YA;
            if (YB < YA) {
                P = B;
                YP = YB;
            }
            return;
        } else {
            return;
        }
    }

    Real64 FUN(EnergyPlusData &state, Real64 const II, Real64 const VV, Real64 const IL, Real64 const IO, Real64 const RSER, Real64 const AA)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         O. Ulleberg, IFE Norway for Hydrogems
        //       DATE WRITTEN   March 2001
        //       MODIFIED       D. Bradley for EnergyPlus
        //       RE-ENGINEERED

        // PURPOSE OF THIS FUNCTION:
        // This function is based on the current-voltage characteristic of the PV module and is of the
        // form f(I,V)=0

        // Return value
        Real64 FUN(0.0);

        if (((VV + II * RSER) / AA) < 700.0) {
            FUN = II - IL + IO * (std::exp((VV + II * RSER) / AA) - 1.0) - ((VV + II * RSER) / state.dataPhotovoltaic->ShuntResistance);
        } else {
            ShowSevereError(state, "EquivalentOneDiode Photovoltaic model failed to find maximum power point");
            ShowContinueError(state, "Numerical solver failed trying to take exponential of too large a number");
            ShowContinueError(state, format("Check input data in {}", pvModelNames[(int)PVModel::TRNSYS]));
            ShowContinueError(state, format("VV (voltage) = {:.5R}", VV));
            ShowContinueError(state, format("II (current) = {:.5R}", II));
            ShowFatalError(state, "FUN: EnergyPlus terminates because of numerical problem in EquivalentOne-Diode PV model");
        }

        return FUN;
    }

    Real64 FI(EnergyPlusData &state, Real64 const II, Real64 const VV, Real64 const IO, Real64 const RSER, Real64 const AA)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         O. Ulleberg, IFE Norway for Hydrogems
        //       DATE WRITTEN   March 2001
        //       MODIFIED       D. Bradley for EnergyPlus
        //       RE-ENGINEERED

        // PURPOSE OF THIS FUNCTION:
        // partial differential of I=I(I,V)

        // METHODOLOGY EMPLOYED:
        // the function is based on the current voltage characteristic of the PV module and is of
        // the form dF(I,V)/dI=0

        // Return value
        Real64 FI(0.0);

        if (((VV + II * RSER) / AA) < 700.0) {
            FI = 1.0 + IO * std::exp((VV + II * RSER) / AA) * RSER / AA + (RSER / state.dataPhotovoltaic->ShuntResistance);
        } else {
            ShowSevereError(state, "EquivalentOneDiode Photovoltaic model failed to find maximum power point");
            ShowContinueError(state, "Numerical solver failed trying to take exponential of too large a number");
            ShowContinueError(state, format("Check input data in {}", pvModelNames[(int)PVModel::TRNSYS]));
            ShowContinueError(state, format("VV (voltage) = {:.5R}", VV));
            ShowContinueError(state, format("II (current) = {:.5R}", II));
            ShowFatalError(state, "FI: EnergyPlus terminates because of numerical problem in EquivalentOne-Diode PV model");
        }

        return FI;
    }

    Real64 FV(EnergyPlusData &state, Real64 const II, Real64 const VV, Real64 const IO, Real64 const RSER, Real64 const AA)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         O. Ulleberg, IFE Norway for Hydrogems
        //       DATE WRITTEN   March 2001
        //       MODIFIED       D. Bradley for EnergyPlus
        //       RE-ENGINEERED

        // PURPOSE OF THIS FUNCTION:
        // partial differential of V=I(I,V)

        // METHODOLOGY EMPLOYED:
        // the function is based on the current voltage characteristic of the PV module and is of
        // the form dF(I,V)/dV=0

        // Return value
        Real64 FV(0.0);

        if (((VV + II * RSER) / AA) < 700.0) {
            FV = IO * std::exp((VV + II * RSER) / AA) / AA + (1.0 / state.dataPhotovoltaic->ShuntResistance);
        } else {
            ShowSevereError(state, "EquivalentOneDiode Photovoltaic model failed to find maximum power point");
            ShowContinueError(state, "Numerical solver failed trying to take exponential of too large a number");
            ShowContinueError(state, format("Check input data in {}", pvModelNames[(int)PVModel::TRNSYS]));
            ShowContinueError(state, format("VV (voltage) = {:.5R}", VV));
            ShowContinueError(state, format("II (current) = {:.5R}", II));
            ShowFatalError(state, "FI: EnergyPlus terminates because of numerical problem in EquivalentOne-Diode PV model");
        }

        return FV;
    }

    // End routines for Equivalent One-Diode model as implemented by Bradley
    //************************************************************************

    // Begin supporting routines for Sandia PV model
    // -------------------------------------------------------------------------------

    Real64 SandiaModuleTemperature(Real64 const Ibc, // beam radiation on collector plane, W/m2
                                   Real64 const Idc, // Diffuse radiation on collector plane, W/m2
                                   Real64 const Ws,  // wind speed, m/s
                                   Real64 const Ta,  // ambient temperature, degC
                                   Real64 const fd,  // fraction of Idc used (empirical constant)
                                   Real64 const a,   // empirical constant
                                   Real64 const b    // empirical constant
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         G. Barker
        //       DATE WRITTEN   unknown
        //       MODIFIED       na
        //       RE-ENGINEERED  B.Griffith December 2003

        // PURPOSE OF THIS FUNCTION:
        // Returns back-of-module temperature, deg C

        // METHODOLOGY EMPLOYED:
        // apply sandia temperature model, This is module temp or back of
        // of the panel.  A separate correction handles delta T for actual cell

        // REFERENCES:
        // from G. Barker's TRNSYS implementation
        // Equations (10)  in Davis, M.W., A.H. Fanney, B.P. Dougherty. Measured versus
        //   predicted performance of building integrated photovoltaics,
        //   Solar 2002, Sunrise on the Reliable Energy Economy,
        //   June 15-19, 2002, Reno, NV.

        // Return value
        Real64 SandiaModuleTemperature;

        Real64 E; // total irradiance working variable

        E = Ibc + fd * Idc;

        SandiaModuleTemperature = E * std::exp(a + b * Ws) + Ta;

        return SandiaModuleTemperature;
    }

    // -------------------------------------------------------------------------------
    // -------------------------------------------------------------------------------

    Real64 SandiaTcellFromTmodule(Real64 const Tm,  // module temperature (deg C)
                                  Real64 const Ibc, // beam radiation on collector plane, W/m2
                                  Real64 const Idc, // Diffuse radiation on collector plane, W/m2
                                  Real64 const fd,  // fraction of Idc used (empirical constant)
                                  Real64 const DT0  // (Tc-Tm) at E=1000 W/m2 (empirical constant known as delta T), deg C
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         G. Barker
        //       DATE WRITTEN   unknown
        //       MODIFIED       na
        //       RE-ENGINEERED  B. Griffith Jan 2004 F77 -> f90

        // PURPOSE OF THIS FUNCTION:
        // Returns cell temperature, deg C

        // METHODOLOGY EMPLOYED:
        // This is for the Sandia model method of determining cell temperatures
        // module temperature differs from solar cell temperature
        // because panel temperatures are not uniform

        // REFERENCES:
        // Equations (11) in Davis, M.W., A.H. Fanney, B.P. Dougherty. Measured versus
        //   predicted performance of building integrated photovoltaics,
        //   Solar 2002, Sunrise on the Reliable Energy Economy,
        //   June 15-19, 2002, Reno, NV.

        // Return value
        Real64 SandiaTcellFromTmodule;

        Real64 E; // total irradiance working variable

        E = Ibc + fd * Idc;

        SandiaTcellFromTmodule = Tm + (E / 1000.0) * DT0;

        return SandiaTcellFromTmodule;
    }

    // -------------------------------------------------------------------------------

    Real64 SandiaCellTemperature(Real64 const Ibc, // beam radiation on collector plane W/m2
                                 Real64 const Idc, // Diffuse radiation on collector plane W/m2
                                 Real64 const Ws,  // wind speed, m/s
                                 Real64 const Ta,  // ambient temperature, degC
                                 Real64 const fd,  // fraction of Idc used (empirical constant)
                                 Real64 const a,   // empirical constant
                                 Real64 const b,   // empirical constant
                                 Real64 const DT0  // (Tc-Tm) at E=1000 W/m2 (empirical constant known as dTc), deg C
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         G. Barker
        //       DATE WRITTEN   unknown
        //       MODIFIED
        //       RE-ENGINEERED  B. Griffith, Jan 2004 F77-> f90

        // PURPOSE OF THIS FUNCTION:
        //  Returns cell temperature, deg C
        // METHODOLOGY EMPLOYED:
        // is this even used?  duplicates separate functions above.
        // combines function SandiaTcellFromTmodule with
        //  SandiaModuleTemperature

        // REFERENCES:
        // Equations (10) and (11) in Davis, M.W., A.H. Fanney, B.P. Dougherty. Measured versus
        //   predicted performance of building integrated photovoltaics,
        //   Solar 2002, Sunrise on the Reliable Energy Economy,
        //   June 15-19, 2002, Reno, NV.

        // Return value
        Real64 SandiaCellTemperature;

        Real64 E; // irradiance working variable
        Real64 Tm;

        E = Ibc + fd * Idc;

        Tm = E * std::exp(a + b * Ws) + Ta;

        SandiaCellTemperature = Tm + (E / 1000.0) * DT0; // E0=1000.0 W/m2

        return SandiaCellTemperature;
    }

    // -------------------------------------------------------------------------------

    Real64 SandiaEffectiveIrradiance(Real64 const Tc,   // cell temperature (deg C)
                                     Real64 const Isc,  // short-circuit current under operating conditions (A)
                                     Real64 const Isc0, // reference Isc at Tc=25 C, Ic=1000 W/m2 (A)
                                     Real64 const aIsc  // Isc temperature coefficient (degC^-1)
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         G. Barker
        //       DATE WRITTEN   <unknown>
        //       MODIFIED       na
        //       RE-ENGINEERED  B. Griffith Jan 2004, F77 to f90

        // PURPOSE OF THIS FUNCTION:
        // Returns "effective irradiance", used in calculation of Imp, Voc, Ix, Ixx

        // Return value
        Real64 SandiaEffectiveIrradiance;

        SandiaEffectiveIrradiance = Isc / (1.0 + aIsc * (Tc - 25.0)) / Isc0;

        return SandiaEffectiveIrradiance;
    }

    // -------------------------------------------------------------------------------

    Real64 AbsoluteAirMass(Real64 const SolZen,  // solar zenith angle (deg)
                           Real64 const Altitude // site altitude (m)
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         G. Barker
        //       DATE WRITTEN   <unknown>
        //       MODIFIED       na
        //       RE-ENGINEERED  B. Griffith Jan 2004 F77 -> f90

        // PURPOSE OF THIS FUNCTION:
        // Returns absolute air mass

        // Return value
        Real64 AbsoluteAirMass;

        if (SolZen < 89.9) {
            Real64 const AM(1.0 / (std::cos(SolZen * Constant::DegToRad) + 0.5057 * std::pow(96.08 - SolZen, -1.634)));
            AbsoluteAirMass = std::exp(-0.0001184 * Altitude) * AM;
        } else {
            Real64 constexpr AM(36.32); // evaluated above at SolZen = 89.9 issue #5528
            AbsoluteAirMass = std::exp(-0.0001184 * Altitude) * AM;
        }

        return AbsoluteAirMass;
    }

    // -------------------------------------------------------------------------------

    Real64 SandiaF1(Real64 const AMa, // absolute air mass
                    Real64 const a0,  // empirical constant, module-specific
                    Real64 const a1,  // empirical constant, module-specific
                    Real64 const a2,  // empirical constant, module-specific
                    Real64 const a3,  // empirical constant, module-specific
                    Real64 const a4   // empirical constant, module-specific
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         G. Barker
        //       DATE WRITTEN   <unknown>
        //       MODIFIED       na
        //       RE-ENGINEERED  B. Griffith F77-> f90

        // PURPOSE OF THIS FUNCTION:
        // Returns the result of Sandia Air Mass function
        //  "AMa-Function" for solar spectral influence

        // METHODOLOGY EMPLOYED:
        // <description>

        // REFERENCES:
        // Equation (8) in Davis, M.W., A.H. Fanney, B.P. Dougherty. Measured versus
        //   predicted performance of building integrated photovoltaics,
        //   Solar 2002, Sunrise on the Reliable Energy Economy,
        //   June 15-19, 2002, Reno, NV.

        // Return value
        Real64 SandiaF1;

        Real64 const F1(a0 + a1 * AMa + a2 * pow_2(AMa) + a3 * pow_3(AMa) + a4 * pow_4(AMa));

        if (F1 > 0.0) {
            SandiaF1 = F1;
        } else {
            SandiaF1 = 0.0;
        }

        return SandiaF1;
    }

    // -------------------------------------------------------------------------------

    Real64 SandiaF2(Real64 const IncAng, // incidence angle (deg)
                    Real64 const b0,     // empirical module-specific constants
                    Real64 const b1,     // empirical module-specific constants
                    Real64 const b2,     // empirical module-specific constants
                    Real64 const b3,     // empirical module-specific constants
                    Real64 const b4,     // empirical module-specific constants
                    Real64 const b5      // empirical module-specific constants
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         G. Barker
        //       DATE WRITTEN   <unknown>
        //       MODIFIED       na
        //       RE-ENGINEERED  B. Griffith Jan 2004 F77-> f90

        // PURPOSE OF THIS FUNCTION:
        // C Returns Sandia F2 function

        // REFERENCES:
        // Equation (9) in Davis, M.W., A.H. Fanney, B.P. Dougherty. Measured versus
        //   predicted performance of building integrated photovoltaics,
        //   Solar 2002, Sunrise on the Reliable Energy Economy,
        //   June 15-19, 2002, Reno, NV.

        // Return value
        Real64 SandiaF2;

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        Real64 F2; // working variable for function result

        F2 = b0 + b1 * IncAng + b2 * pow_2(IncAng) + b3 * pow_3(IncAng) + b4 * pow_4(IncAng) + b5 * pow_5(IncAng);

        if (F2 > 0.0) {
            SandiaF2 = F2;
        } else {
            SandiaF2 = 0.0;
        }

        return SandiaF2;
    }

    // -------------------------------------------------------------------------------

    Real64 SandiaImp(Real64 const Tc,   // cell temperature (degC)
                     Real64 const Ee,   // effective irradiance (W/m2)
                     Real64 const Imp0, // current at MPP at SRC (1000 W/m2, 25 C) (A)
                     Real64 const aImp, // Imp temperature coefficient (degC^-1)
                     Real64 const C0,   // empirical module-specific constants
                     Real64 const C1    // empirical module-specific constants
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         G. Barker
        //       DATE WRITTEN   <unknown>
        //       MODIFIED       na
        //       RE-ENGINEERED  B. Griffith F77 -> f90

        // PURPOSE OF THIS FUNCTION:
        // Returns current at maximum power point (A)

        // REFERENCES:
        // Equation (3) in Davis, M.W., A.H. Fanney, B.P. Dougherty. Measured versus
        //   predicted performance of building integrated photovoltaics,
        //   Solar 2002, Sunrise on the Reliable Energy Economy,
        //   June 15-19, 2002, Reno, NV.

        // Return value
        Real64 SandiaImp;

        SandiaImp = Imp0 * (C0 * Ee + C1 * pow_2(Ee)) * (1.0 + aImp * (Tc - 25));
        // why hardwire T0 at 25.0?  can this change? seems okay, fewer args
        return SandiaImp;
    }

    // -------------------------------------------------------------------------------

    Real64 SandiaIsc(Real64 const Tc,   // cell temperature (deg C)
                     Real64 const Isc0, // Isc at Tc=25 C, Ic=1000 W/m2 (A)
                     Real64 const Ibc,  // beam radiation on collector plane (W/m2)
                     Real64 const Idc,  // Diffuse radiation on collector plane (W/m2)
                     Real64 const F1,   // Sandia F1 function for air mass effects
                     Real64 const F2,   // Sandia F2 function of incidence angle
                     Real64 const fd,   // module-specific empirical constant
                     Real64 const aIsc  // Isc temperature coefficient (degC^-1)
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         G. Barker
        //       DATE WRITTEN   <date_written>
        //       MODIFIED       na
        //       RE-ENGINEERED  B. Griffith Jan 2004 F77 -> f90

        // PURPOSE OF THIS FUNCTION:
        // Returns Short-Circuit Current

        // REFERENCES:
        // Equation (1) in Davis, M.W., A.H. Fanney, B.P. Dougherty. Measured versus
        //   predicted performance of building integrated photovoltaics,
        //   Solar 2002, Sunrise on the Reliable Energy Economy,
        //   June 15-19, 2002, Reno, NV.

        // Return value
        Real64 SandiaIsc;

        // SandiaIsc=Isc0*((Ibc*F1*F2+fd*Idc)/1000.0)*(1.0+aIsc*(Tc-25.0))
        // Barkers original (above) changed to match publish eq. (1) in reference
        SandiaIsc = Isc0 * F1 * ((Ibc * F2 + fd * Idc) / 1000.0) * (1.0 + aIsc * (Tc - 25.0));

        // why hardwire E0 at 1000.0 ?, can this change? seems okay

        return SandiaIsc;
    }

    // -------------------------------------------------------------------------------

    Real64 SandiaIx(Real64 const Tc,   // cell temperature (deg C)
                    Real64 const Ee,   // effective irradiance
                    Real64 const Ix0,  // Ix at SRC (1000 W/m2, 25 C) (A)
                    Real64 const aIsc, // Isc temp coefficient (/C)
                    Real64 const aImp, // Imp temp coefficient (/C)
                    Real64 const C4,   // empirical module-specific constants
                    Real64 const C5    // empirical module-specific constants
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         G. Barker
        //       DATE WRITTEN   <unknown>
        //       MODIFIED       na
        //       RE-ENGINEERED  B. Griffith, Jan 2004 F77 -> f90

        // PURPOSE OF THIS FUNCTION:
        // Returns current "Ix" at V=0.5*Voc (A)

        // REFERENCES:
        // Equation 9 in King et al. nov 20003

        // Return value
        Real64 SandiaIx;

        SandiaIx = Ix0 * (C4 * Ee + C5 * pow_2(Ee)) * (1.0 + ((aIsc + aImp) / 2.0 * (Tc - 25.0)));

        return SandiaIx;
    }

    // -------------------------------------------------------------------------------

    Real64 SandiaIxx(Real64 const Tc,   // cell temperature (deg C)
                     Real64 const Ee,   // effective irradiance (W/m2 ?)
                     Real64 const Ixx0, // Ixx at SRC (1000 W/m2, 25 C) (A)
                     Real64 const aImp, // Imp temp coefficient (/C)
                     Real64 const C6,   // empirical module-specific constants
                     Real64 const C7    // empirical module-specific constants
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         G. Barker
        //       DATE WRITTEN   <unknown>
        //       MODIFIED       na
        //       RE-ENGINEERED  B. Griffith Jan2004 F77 to f90

        // PURPOSE OF THIS FUNCTION:
        // Returns current "Ix" at V=0.5*(Voc+Vmp) (A)

        // REFERENCES:
        // Equation 10 in King et al nov. 2003

        // Return value
        Real64 SandiaIxx;

        SandiaIxx = Ixx0 * (C6 * Ee + C7 * pow_2(Ee)) * (1.0 + aImp * (Tc - 25.0));

        return SandiaIxx;
    }

    // -------------------------------------------------------------------------------

    Real64 SandiaVmp(Real64 const Tc,          // cell temperature (deg C)
                     Real64 const Ee,          // effective irradiance
                     Real64 const Vmp0,        // Vmp at SRC (1000 W/m2, 25 C) (V)
                     Real64 const NcellSer,    // # cells in series
                     Real64 const DiodeFactor, // module-specIFic empirical constant
                     Real64 const BVmp0,       // Vmp temperature coefficient (V/C)
                     Real64 const mBVmp,       // change in BVmp with irradiance
                     Real64 const C2,          // empirical module-specific constants
                     Real64 const C3           // empirical module-specific constants
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         G. Barker
        //       DATE WRITTEN   <unknown>
        //       MODIFIED       na
        //       RE-ENGINEERED  B. Griffith, Jan 2004, F77 -> f90

        // PURPOSE OF THIS FUNCTION:
        // Returns Voltage at Max. Power Point (V)

        // METHODOLOGY EMPLOYED:
        // <description>

        // REFERENCES:
        // Equation 4 in King et al Nov. 2003

        // Return value
        Real64 SandiaVmp;

        Real64 dTc;
        Real64 BVmpEe;

        if (Ee > 0.0) {
            // following is equation 8 in King et al. nov. 2003
            dTc = DiodeFactor * ((1.38066e-23 * (Tc + Constant::Kelvin)) / 1.60218e-19);

            BVmpEe = BVmp0 + mBVmp * (1.0 - Ee);

            SandiaVmp = Vmp0 + C2 * NcellSer * dTc * std::log(Ee) + C3 * NcellSer * pow_2(dTc * std::log(Ee)) + BVmpEe * (Tc - 25.0);
        } else {
            SandiaVmp = 0.0;
        }

        return SandiaVmp;
    }

    // -------------------------------------------------------------------------------

    Real64 SandiaVoc(Real64 const Tc,          // cell temperature (deg C)
                     Real64 const Ee,          // effective irradiance
                     Real64 const Voc0,        // Voc at SRC (1000 W/m2, 25 C) (V)
                     Real64 const NcellSer,    // # cells in series
                     Real64 const DiodeFactor, // module-specIFic empirical constant
                     Real64 const BVoc0,       // Voc temperature coefficient (V/C)
                     Real64 const mBVoc        // change in BVoc with irradiance
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         G Barker
        //       DATE WRITTEN   <unknown>
        //       MODIFIED       na
        //       RE-ENGINEERED  B Griffith Jan 2004 F77 -> f90

        // PURPOSE OF THIS FUNCTION:
        // Returns Open-Circuit Voltage (V)

        // Return value
        Real64 SandiaVoc;

        Real64 dTc;    // working variable
        Real64 BVocEe; // working variable

        if (Ee > 0.0) {
            dTc = DiodeFactor * ((1.38066e-23 * (Tc + Constant::Kelvin)) / 1.60218e-19);
            BVocEe = BVoc0 + mBVoc * (1.0 - Ee);

            SandiaVoc = Voc0 + NcellSer * dTc * std::log(Ee) + BVocEe * (Tc - 25.0);
        } else {
            SandiaVoc = 0.0;
        }

        return SandiaVoc;
    }

    void SetVentedModuleQdotSource(EnergyPlusData &state,
                                   int const VentModNum,
                                   Real64 const QSource // source term in Watts
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   January 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // object oriented "Set" routine for updating sink term without exposing variables

        // METHODOLOGY EMPLOYED:
        // update derived type with new data , turn power into W/m2

        // Using/Aliasing
        using namespace DataSurfaces;

        state.dataHeatBal->ExtVentedCavity(VentModNum).QdotSource = QSource / state.dataHeatBal->ExtVentedCavity(VentModNum).ProjArea;
    }

    void GetExtVentedCavityIndex(EnergyPlusData &state, int const SurfacePtr, int &VentCavIndex)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   January 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // object oriented "Get" routine for establishing correct integer index from outside this module

        // METHODOLOGY EMPLOYED:
        // mine Surface derived type for correct index/number of surface
        // mine  ExtVentedCavity derived type that has the surface.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int CavNum;   // temporary
        int ThisSurf; // temporary
        int thisCav;
        bool Found;

        if (SurfacePtr == 0) {
            // should be trapped already
            ShowFatalError(state, "Invalid surface passed to GetExtVentedCavityIndex");
        }

        CavNum = 0;
        Found = false;
        for (thisCav = 1; thisCav <= state.dataSurface->TotExtVentCav; ++thisCav) {
            for (ThisSurf = 1; ThisSurf <= state.dataHeatBal->ExtVentedCavity(thisCav).NumSurfs; ++ThisSurf) {
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

    void GetExtVentedCavityTsColl(EnergyPlusData &state, int const VentModNum, Real64 &TsColl)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         <author>
        //       DATE WRITTEN   <date_written>
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // object oriented "Get" routine for collector surface temperature

        // METHODOLOGY EMPLOYED:
        // access derived type

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        TsColl = state.dataHeatBal->ExtVentedCavity(VentModNum).Tbaffle;
    }

    // -------------------------------------------------------------------------------

    //     EnergyPlus V1.2 and beyond include models for photovoltaic calculations called
    //     Generator:Photovoltaic:Simple and Generator:PV:Sandia implemented by the Center for
    //     Buildings and Thermal Systems, National Renewable Energy Laboratory, 1617 Cole Blvd
    //     MS 2722, Golden, CO, 80401

    //     EnergyPlus v1.1.1 and beyond includes model for Photovoltaic calculations, now
    //     referred to as the Generator:PV:Equivalent One-Diode model developed by Thermal Energy
    //     System Specialists, 2916 Marketplace Drive, Suite 104, Madison, WI 53719;
    //     Tel: (608) 274-2577

} // namespace Photovoltaics

} // namespace EnergyPlus
