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
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataMoistureBalance.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HeatBalFiniteDiffManager.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/MoistureBalanceEMPDManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/PhaseChangeModeling/HysteresisModel.hh>
#include <EnergyPlus/PluginManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace HeatBalFiniteDiffManager {

    // Module containing the heat balance simulation routines

    // MODULE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   October 2003
    //       RE-ENGINEERED  Curtis Pedersen, 2006, Changed to Implicit FD calc for conduction.
    //                      and included enthalpy formulations for phase change materials
    // PURPOSE OF THIS MODULE:
    // To encapsulate the data and algorithms required to
    // manage the finite difference heat balance simulation on the building.

    // REFERENCES:
    // The MFD moisture balance method
    //  C. O. Pedersen, Enthalpy Formulation of conduction heat transfer problems
    //    involving latent heat, Simulation, Vol 18, No. 2, February 1972
    // Fan system Source/Sink heat value, and source/sink location temp from CondFD

    void ManageHeatBalFiniteDiff(EnergyPlusData &state,
                                 int const SurfNum,
                                 Real64 &SurfTempInTmp, // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                                 Real64 &TempSurfOutTmp // Outside Surface Temperature of each Heat Transfer Surface
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   May 2000

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine manages the moisture balance method.  It is called
        // from the HeatBalanceManager at the time step level.
        // This driver manages the calls to all of
        // the other drivers and simulation algorithms.

        // Get the moisture balance input at the beginning of the simulation only
        if (state.dataHeatBalFiniteDiffMgr->GetHBFiniteDiffInputFlag) {
            // Obtains conduction FD related parameters from input file
            GetCondFDInput(state);
            state.dataHeatBalFiniteDiffMgr->GetHBFiniteDiffInputFlag = false;
        }
        // Solve the zone heat & moisture balance using a finite difference solution
        CalcHeatBalFiniteDiff(state, SurfNum, SurfTempInTmp, TempSurfOutTmp);
    }

    void GetCondFDInput(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Curtis Pedersen
        //       DATE WRITTEN   July 2006
        //       MODIFIED       Brent Griffith Mar 2011, user settings

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is the main driver for initializations for the variable property CondFD part of the
        // MFD algorithm

        static constexpr std::string_view routineName = "GetCondFDInput";
        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int IOStat;                         // IO Status when calling get input subroutine
        Array1D_string MaterialNames(3);    // Number of Material Alpha names defined
        Array1D_string ConstructionName(3); // Name of Construction with CondFDsimplified
        int MaterialNumAlpha;               // Number of material alpha names being passed
        int MaterialNumProp;                // Number of material properties being passed
        Array1D<Real64> MaterialProps;      // Temporary array to transfer material properties (allocated based on user input)
        bool ErrorsFound(false);            // If errors detected in input
        int propNum;
        int pcMat;
        int vcMat;
        int inegptr;
        bool nonInc;

        auto &s_ip = state.dataInputProcessing->inputProcessor;
        auto &s_ipsc = state.dataIPShortCut;
        auto &s_hbfd = state.dataHeatBalFiniteDiffMgr;
        auto &s_mat = state.dataMaterial;

        // user settings for numerical parameters
        s_ipsc->cCurrentModuleObject = "HeatBalanceSettings:ConductionFiniteDifference";

        if (s_ip->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject) > 0) {
            int NumAlphas;
            int NumNumbers;
            s_ip->getObjectItem(state,
                                s_ipsc->cCurrentModuleObject,
                                1,
                                s_ipsc->cAlphaArgs,
                                NumAlphas,
                                s_ipsc->rNumericArgs,
                                NumNumbers,
                                IOStat,
                                s_ipsc->lNumericFieldBlanks,
                                s_ipsc->lAlphaFieldBlanks,
                                s_ipsc->cAlphaFieldNames,
                                s_ipsc->cNumericFieldNames);

            if (!s_ipsc->lAlphaFieldBlanks(1)) {
                {
                    s_hbfd->CondFDSchemeType =
                        static_cast<CondFDScheme>(getEnumValue(CondFDSchemeTypeNamesUC, Util::makeUPPER(s_ipsc->cAlphaArgs(1))));
                    if (s_hbfd->CondFDSchemeType == CondFDScheme::Invalid) {
                        ShowSevereError(state,
                                        format("{}: invalid {} entered={}, must match CrankNicholsonSecondOrder or FullyImplicitFirstOrder.",
                                               s_ipsc->cCurrentModuleObject,
                                               s_ipsc->cAlphaFieldNames(1),
                                               s_ipsc->cAlphaArgs(1)));
                        ErrorsFound = true;
                    }
                }
            }

            if (!s_ipsc->lNumericFieldBlanks(1)) {
                s_hbfd->SpaceDescritConstant = s_ipsc->rNumericArgs(1);
            }
            if (!s_ipsc->lNumericFieldBlanks(2)) {
                state.dataHeatBal->CondFDRelaxFactorInput = s_ipsc->rNumericArgs(2);
                state.dataHeatBal->CondFDRelaxFactor = state.dataHeatBal->CondFDRelaxFactorInput;
            }
            if (!s_ipsc->lNumericFieldBlanks(3)) {
                state.dataHeatBal->MaxAllowedDelTempCondFD = s_ipsc->rNumericArgs(3);
            }

        } // settings object

        pcMat = s_ip->getNumObjectsFound(state, "MaterialProperty:PhaseChange");
        vcMat = s_ip->getNumObjectsFound(state, "MaterialProperty:VariableThermalConductivity");

        int numProps = setSizeMaxProperties(state);
        MaterialProps.allocate(numProps);

        s_hbfd->MaterialFD.allocate(s_mat->materials.size());

        // Load the additional CondFD Material properties
        s_ipsc->cCurrentModuleObject = "MaterialProperty:PhaseChange"; // Phase Change Information First

        if (pcMat != 0) { //  Get Phase Change info
            //    CondFDVariableProperties = .TRUE.
            for (int Loop = 1; Loop <= pcMat; ++Loop) {

                // Call Input Get routine to retrieve material data
                s_ip->getObjectItem(state,
                                    s_ipsc->cCurrentModuleObject,
                                    Loop,
                                    MaterialNames,
                                    MaterialNumAlpha,
                                    MaterialProps,
                                    MaterialNumProp,
                                    IOStat,
                                    s_ipsc->lNumericFieldBlanks,
                                    s_ipsc->lAlphaFieldBlanks,
                                    s_ipsc->cAlphaFieldNames,
                                    s_ipsc->cNumericFieldNames);

                // Load the material derived type from the input data.
                ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, MaterialNames(1)};
                int matNum = Material::GetMaterialNum(state, MaterialNames(1));
                if (matNum == 0) {
                    ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(1), MaterialNames(1));
                    ErrorsFound = true;
                    continue;
                }
                auto const *mat = state.dataMaterial->materials(matNum);

                if (mat->group != Material::Group::Regular) {
                    ShowSevereError(state,
                                    format("{}: Reference Material is not appropriate type for CondFD properties, material={}, must have regular "
                                           "properties (L,Cp,K,D)",
                                           s_ipsc->cCurrentModuleObject,
                                           mat->Name));
                    ErrorsFound = true;
                }

                // Once the material derived type number is found then load the additional CondFD variable material properties
                //   Some or all may be zero (default).  They will be checked when calculating node temperatures
                auto &matFD = s_hbfd->MaterialFD(matNum);
                matFD.tk1 = MaterialProps(1);
                matFD.numTempEnth = (MaterialNumProp - 1) / 2;
                if (matFD.numTempEnth * 2 != (MaterialNumProp - 1)) {
                    ShowSevereError(state, format("GetCondFDInput: {}=\"{}\", mismatched pairs", s_ipsc->cCurrentModuleObject, MaterialNames(1)));
                    ShowContinueError(state, format("...expected {} pairs, but only entered {} numbers.", matFD.numTempEnth, MaterialNumProp - 1));
                    ErrorsFound = true;
                }
                matFD.TempEnth.dimension(2, matFD.numTempEnth, 0.0);
                propNum = 2;
                // Temperature first
                for (int pcount = 1, pcount_end = matFD.numTempEnth; pcount <= pcount_end; ++pcount) {
                    matFD.TempEnth(1, pcount) = MaterialProps(propNum);
                    propNum += 2;
                }
                propNum = 3;
                // Then Enthalpy
                for (int pcount = 1, pcount_end = matFD.numTempEnth; pcount <= pcount_end; ++pcount) {
                    matFD.TempEnth(2, pcount) = MaterialProps(propNum);
                    propNum += 2;
                }
                nonInc = false;
                inegptr = 0;
                for (int pcount = 1, pcount_end = matFD.numTempEnth - 1; pcount <= pcount_end; ++pcount) {
                    if (matFD.TempEnth(1, pcount) < matFD.TempEnth(1, pcount + 1)) {
                        continue;
                    }
                    nonInc = true;
                    inegptr = pcount + 1;
                    break;
                }
                if (nonInc) {
                    ShowSevereError(state,
                                    format("GetCondFDInput: {}=\"{}\", non increasing Temperatures. Temperatures must be strictly increasing.",
                                           s_ipsc->cCurrentModuleObject,
                                           MaterialNames(1)));
                    ShowContinueError(state,
                                      format("...occurs first at item=[{}], value=[{:.2R}].", fmt::to_string(inegptr), matFD.TempEnth(1, inegptr)));
                    ErrorsFound = true;
                }
                nonInc = false;
                inegptr = 0;
                for (int pcount = 1, pcount_end = matFD.numTempEnth - 1; pcount <= pcount_end; ++pcount) {
                    if (matFD.TempEnth(2, pcount) <= matFD.TempEnth(2, pcount + 1)) {
                        continue;
                    }
                    nonInc = true;
                    inegptr = pcount + 1;
                    break;
                }
                if (nonInc) {
                    ShowSevereError(state,
                                    format("GetCondFDInput: {}=\"{}\", non increasing Enthalpy.", s_ipsc->cCurrentModuleObject, MaterialNames(1)));
                    ShowContinueError(state, format("...occurs first at item=[{}], value=[{:.2R}].", inegptr, matFD.TempEnth(2, inegptr)));
                    ShowContinueError(state, "...These values may be Cp (Specific Heat) rather than Enthalpy.  Please correct.");
                    ErrorsFound = true;
                }
            }
        }
        //   Get CondFD Variable Thermal Conductivity Input

        s_ipsc->cCurrentModuleObject = "MaterialProperty:VariableThermalConductivity"; // Variable Thermal Conductivity Info next
        if (vcMat != 0) {                                                              //  variable k info
            //    CondFDVariableProperties = .TRUE.
            for (int Loop = 1; Loop <= vcMat; ++Loop) {

                // Call Input Get routine to retrieve material data
                s_ip->getObjectItem(state,
                                    s_ipsc->cCurrentModuleObject,
                                    Loop,
                                    MaterialNames,
                                    MaterialNumAlpha,
                                    MaterialProps,
                                    MaterialNumProp,
                                    IOStat,
                                    s_ipsc->lNumericFieldBlanks,
                                    s_ipsc->lAlphaFieldBlanks,
                                    s_ipsc->cAlphaFieldNames,
                                    s_ipsc->cNumericFieldNames);

                ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, MaterialNames(1)};
                // Load the material derived type from the input data.
                int matNum = Material::GetMaterialNum(state, MaterialNames(1));
                if (matNum == 0) {
                    ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(1), MaterialNames(1));
                    ErrorsFound = true;
                    continue;
                }

                auto *mat = s_mat->materials(matNum);

                if (mat->group != Material::Group::Regular) {
                    ShowSevereError(state,
                                    format("{}: Reference Material is not appropriate type for CondFD properties, material={}, must have regular "
                                           "properties (L,Cp,K,D)",
                                           s_ipsc->cCurrentModuleObject,
                                           mat->Name));
                    ErrorsFound = true;
                }

                // Once the material derived type number is found then load the additional CondFD variable material properties
                //   Some or all may be zero (default).  They will be checked when calculating node temperatures

                auto &matFD = s_hbfd->MaterialFD(matNum);
                matFD.numTempCond = MaterialNumProp / 2;
                if (matFD.numTempCond * 2 != MaterialNumProp) {
                    ShowSevereError(state, format("GetCondFDInput: {}=\"{}\", mismatched pairs", s_ipsc->cCurrentModuleObject, MaterialNames(1)));
                    ShowContinueError(state, format("...expected {} pairs, but only entered {} numbers.", matFD.numTempCond, MaterialNumProp));
                    ErrorsFound = true;
                }
                matFD.TempCond.dimension(2, matFD.numTempCond, 0.0);
                propNum = 1;
                // Temperature first
                for (int pcount = 1, pcount_end = matFD.numTempCond; pcount <= pcount_end; ++pcount) {
                    matFD.TempCond(1, pcount) = MaterialProps(propNum);
                    propNum += 2;
                }
                propNum = 2;
                // Then Conductivity
                for (int pcount = 1, pcount_end = matFD.numTempCond; pcount <= pcount_end; ++pcount) {
                    matFD.TempCond(2, pcount) = MaterialProps(propNum);
                    propNum += 2;
                }
                nonInc = false;
                inegptr = 0;
                for (int pcount = 1, pcount_end = matFD.numTempCond - 1; pcount <= pcount_end; ++pcount) {
                    if (matFD.TempCond(1, pcount) < matFD.TempCond(1, pcount + 1)) {
                        continue;
                    }
                    nonInc = true;
                    inegptr = pcount + 1;
                    break;
                }
                if (nonInc) {
                    ShowSevereError(state,
                                    format("GetCondFDInput: {}=\"{}\", non increasing Temperatures. Temperatures must be strictly increasing.",
                                           s_ipsc->cCurrentModuleObject,
                                           MaterialNames(1)));
                    ShowContinueError(state, format("...occurs first at item=[{}], value=[{:.2R}].", inegptr, matFD.TempCond(1, inegptr)));
                    ErrorsFound = true;
                }
            }
        }

        for (auto &matFD : s_hbfd->MaterialFD) {
            if (matFD.numTempEnth == 0) {
                matFD.numTempEnth = 3;
                matFD.TempEnth.dimension(2, 3, -100.0);
            }
            if (matFD.numTempCond == 0) {
                matFD.numTempCond = 3;
                matFD.TempCond.dimension(2, 3, -100.0);
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, "GetCondFDInput: Errors found getting ConductionFiniteDifference properties. Program terminates.");
        }

        InitialInitHeatBalFiniteDiff(state);
    }

    int setSizeMaxProperties(EnergyPlusData &state)
    {
        int numArgs;
        int numAlphas;
        int numNumerics;
        int maxTotalProps = 0;

        auto &s_ip = state.dataInputProcessing->inputProcessor;

        s_ip->getObjectDefMaxArgs(state, "MaterialProperty:PhaseChange", numArgs, numAlphas, numNumerics);
        maxTotalProps = max(maxTotalProps, numNumerics);

        s_ip->getObjectDefMaxArgs(state, "MaterialProperty:VariableThermalConductivity", numArgs, numAlphas, numNumerics);
        maxTotalProps = max(maxTotalProps, numNumerics);

        return maxTotalProps;
    }

    void InitHeatBalFiniteDiff(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard J. Liesen
        //       DATE WRITTEN   Oct 2003
        //       RE-ENGINEERED  C O Pedersen 2006
        //                      B. Griffith May 2011 move begin-environment and every-timestep inits, cleanup formatting

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine sets the initial values for the FD moisture calculation

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool ErrorsFound;

        auto &s_hbfd = state.dataHeatBalFiniteDiffMgr;

        if (s_hbfd->GetHBFiniteDiffInputFlag) {
            // Obtains conduction FD related parameters from input file
            GetCondFDInput(state);
            s_hbfd->GetHBFiniteDiffInputFlag = false;
        }

        auto &SurfaceFD = s_hbfd->SurfaceFD;
        ErrorsFound = false;

        // now do begin environment inits.
        if (state.dataGlobal->BeginEnvrnFlag && s_hbfd->MyEnvrnFlag) {
            for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
                if (state.dataSurface->Surface(SurfNum).HeatTransferAlgorithm != DataSurfaces::HeatTransferModel::CondFD) {
                    continue;
                }
                if (state.dataSurface->Surface(SurfNum).Construction <= 0) {
                    continue; // Shading surface, not really a heat transfer surface
                }
                int ConstrNum = state.dataSurface->Surface(SurfNum).Construction;
                if (state.dataConstruction->Construct(ConstrNum).TypeIsWindow) {
                    continue; //  Windows simulated in Window module
                }
                auto &thisSurface = SurfaceFD(SurfNum);
                thisSurface.T = TempInitValue;
                thisSurface.TOld = TempInitValue;
                thisSurface.TT = TempInitValue;
                thisSurface.Rhov = RhovInitValue;
                thisSurface.RhovOld = RhovInitValue;
                thisSurface.RhoT = RhovInitValue;
                thisSurface.TD = TempInitValue;
                thisSurface.TDT = TempInitValue;
                thisSurface.TDTLast = TempInitValue;
                thisSurface.TDOld = TempInitValue;
                thisSurface.TDreport = TempInitValue;
                thisSurface.RH = 0.0;
                thisSurface.RHreport = 0.0;
                thisSurface.EnthOld = EnthInitValue;
                thisSurface.EnthNew = EnthInitValue;
                thisSurface.EnthLast = EnthInitValue;
                thisSurface.QDreport = 0.0;
                thisSurface.CpDelXRhoS1 = 0.0;
                thisSurface.CpDelXRhoS2 = 0.0;
                thisSurface.TDpriortimestep = 0.0;
                thisSurface.PhaseChangeState = Material::Phase::Transition;
                thisSurface.PhaseChangeStateOld = Material::Phase::Transition;
                thisSurface.PhaseChangeStateOldOld = Material::Phase::Transition;
                thisSurface.PhaseChangeStateRep = Material::phaseInts[(int)Material::Phase::Transition];
                thisSurface.PhaseChangeStateOldRep = Material::phaseInts[(int)Material::Phase::Transition];
                thisSurface.PhaseChangeStateOldOldRep = Material::phaseInts[(int)Material::Phase::Transition];
                thisSurface.PhaseChangeTemperatureReverse = 50;

                state.dataMstBal->TempOutsideAirFD(SurfNum) = 0.0;
                state.dataMstBal->RhoVaporAirOut(SurfNum) = 0.0;
                state.dataMstBal->RhoVaporSurfIn(SurfNum) = 0.0;
                state.dataMstBal->RhoVaporAirIn(SurfNum) = 0.0;
                state.dataMstBal->HConvExtFD(SurfNum) = 0.0;
                state.dataMstBal->HMassConvExtFD(SurfNum) = 0.0;
                state.dataMstBal->HConvInFD(SurfNum) = 0.0;
                state.dataMstBal->HMassConvInFD(SurfNum) = 0.0;
                state.dataMstBal->HSkyFD(SurfNum) = 0.0;
                state.dataMstBal->HGrndFD(SurfNum) = 0.0;
                state.dataMstBal->HAirFD(SurfNum) = 0.0;
            }
            s_hbfd->WarmupSurfTemp = 0;
            s_hbfd->MyEnvrnFlag = false;
        }
        if (!state.dataGlobal->BeginEnvrnFlag) {
            s_hbfd->MyEnvrnFlag = true;
        }

        // now do every timestep inits

        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            if (state.dataSurface->Surface(SurfNum).HeatTransferAlgorithm != DataSurfaces::HeatTransferModel::CondFD) {
                continue;
            }
            if (state.dataSurface->Surface(SurfNum).Construction <= 0) {
                continue; // Shading surface, not really a heat transfer surface
            }
            int ConstrNum = state.dataSurface->Surface(SurfNum).Construction;
            if (state.dataConstruction->Construct(ConstrNum).TypeIsWindow) {
                continue; //  Windows simulated in Window module
            }
            auto &thisSurface = SurfaceFD(SurfNum);
            thisSurface.T = thisSurface.TOld;
            thisSurface.Rhov = thisSurface.RhovOld;
            thisSurface.TD = thisSurface.TDOld;
            thisSurface.TDT = thisSurface.TDreport; // PT changes from TDold to TDreport
            thisSurface.TDTLast = thisSurface.TDOld;
            thisSurface.EnthOld = thisSurface.EnthOld;
            thisSurface.EnthNew = thisSurface.EnthOld;
            thisSurface.EnthLast = thisSurface.EnthOld;
            thisSurface.TDpriortimestep = thisSurface.TDreport; // Save TD for heat flux calc
        }
    }

    void InitialInitHeatBalFiniteDiff(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   March 2012

        // PURPOSE OF THIS SUBROUTINE:
        // This routine performs the original allocate, inits and setup output variables for the
        // module.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 dxn; // Intermediate calculation of nodal spacing. This is the full dx. There is
        // a half dxn thick node at each surface. dxn is the "capacitor" spacing.
        int Ipts1; // Intermediate calculation for number of full thickness nodes per layer. There
        // are always two half nodes at the layer faces.
        int Layer; // Loop counter
        int Delt;

        Real64 Alpha;
        Real64 mAlpha;
        Real64 StabilityTemp;
        Real64 StabilityMoist;
        Real64 kt;
        Real64 RhoS;
        Real64 Por;
        Real64 Cp;
        Real64 Dv;
        Real64 DeltaTimestep;      // zone timestep in seconds, for local check of properties
        Real64 ThicknessThreshold; // min thickness consistent with other thermal properties, for local check

        auto &s_hbfd = state.dataHeatBalFiniteDiffMgr;
        auto &ConstructFD = s_hbfd->ConstructFD;
        auto &SigmaR = s_hbfd->SigmaR;
        auto &SigmaC = s_hbfd->SigmaC;
        auto &SurfaceFD = s_hbfd->SurfaceFD;
        auto &QHeatInFlux = s_hbfd->QHeatInFlux;
        auto &QHeatOutFlux = s_hbfd->QHeatOutFlux;

        ConstructFD.allocate(state.dataHeatBal->TotConstructs);
        SigmaR.allocate(state.dataHeatBal->TotConstructs);
        SigmaC.allocate(state.dataHeatBal->TotConstructs);

        SurfaceFD.allocate(state.dataSurface->TotSurfaces);
        QHeatInFlux.allocate(state.dataSurface->TotSurfaces);
        QHeatOutFlux.allocate(state.dataSurface->TotSurfaces);

        // And then initialize
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            QHeatInFlux(SurfNum) = 0.0;
            QHeatOutFlux(SurfNum) = 0.0;
            state.dataHeatBalSurf->SurfOpaqInsFaceCondFlux(SurfNum) = 0.0;
            state.dataHeatBalSurf->SurfOpaqOutFaceCondFlux(SurfNum) = 0.0;
        }

        // Setup Output Variables

        //  set a Delt that fits the zone time step and keeps it below 200s.

        s_hbfd->fracTimeStepZone_Hour = 1.0 / double(state.dataGlobal->TimeStepsInHour);

        for (int index = 1; index <= 20; ++index) {
            Delt = (s_hbfd->fracTimeStepZone_Hour * Constant::rSecsInHour) / index; // TimeStepZone = Zone time step in fractional hours
            if (Delt <= 200) {
                break;
            }
        }

        for (int ConstrNum = 1; ConstrNum <= state.dataHeatBal->TotConstructs; ++ConstrNum) {
            auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum);
            auto &thisConstructFD = ConstructFD(ConstrNum);
            // Need to skip window constructions, IRT, air wall and construction not in use.
            // Need to also skip constructions for surfaces that do not use CondFD.
            if (thisConstruct.TypeIsWindow) {
                continue;
            }
            if (thisConstruct.TypeIsIRT) {
                continue;
            }
            if (thisConstruct.TypeIsAirBoundary) {
                continue;
            }
            if (!thisConstruct.IsUsed) {
                continue;
            }
            if (!findAnySurfacesUsingConstructionAndCondFD(state, ConstrNum)) {
                continue;
            }

            thisConstructFD.Name.allocate(thisConstruct.TotLayers);
            thisConstructFD.Thickness.allocate(thisConstruct.TotLayers);
            thisConstructFD.NodeNumPoint.allocate(thisConstruct.TotLayers);
            thisConstructFD.DelX.allocate(thisConstruct.TotLayers);
            thisConstructFD.TempStability.allocate(thisConstruct.TotLayers);
            thisConstructFD.MoistStability.allocate(thisConstruct.TotLayers);

            int TotNodes = 0;
            SigmaR(ConstrNum) = 0.0;
            SigmaC(ConstrNum) = 0.0;

            for (Layer = 1; Layer <= thisConstruct.TotLayers; ++Layer) { // Begin layer loop ...

                // Loop through all of the layers in the current construct. The purpose
                // of this loop is to define the thermal properties and to.
                // determine the total number of full size nodes in each layer.
                // The number of temperature points is one more than this
                // because of the two half nodes at the layer faces.
                // The calculation of dxn used here is based on a standard stability
                // criteria for explicit finite difference solutions.  This criteria
                // was chosen not because it is viewed to be correct, but rather for
                // lack of any better criteria at this time.  The use of a Fourier
                // number based criteria such as this is probably physically correct.
                //  Change to implicit formulation still uses explicit stability, but
                // now there are special equations for R-only layers.

                int CurrentLayer = thisConstruct.LayerPoint(Layer);
                auto *mat = state.dataMaterial->materials(CurrentLayer);

                thisConstructFD.Name(Layer) = mat->Name;
                thisConstructFD.Thickness(Layer) = mat->Thickness;

                // Do some quick error checks for this section.

                if (mat->ROnly) { // Rlayer

                    //  These values are only needed temporarily and to calculate flux,
                    //   Layer will be handled
                    //  as a pure R in the temperature calc.
                    // assign other properties based on resistance

                    mat->SpecHeat = 0.0001;
                    mat->Density = 1.0;
                    mat->Thickness = 0.1; //  arbitrary thickness for R layer
                    mat->Conductivity = mat->Thickness / mat->Resistance;
                    kt = mat->Conductivity;
                    thisConstructFD.Thickness(Layer) = mat->Thickness;

                    SigmaR(ConstrNum) += mat->Resistance; // add resistance of R layer
                    SigmaC(ConstrNum) += 0.0;             //  no capacitance for R layer

                    Alpha = kt / (mat->Density * mat->SpecHeat);

                    mAlpha = 0.0;

                } else if (mat->group == Material::Group::AirGap) { //  Group 1 = Air

                    //  Again, these values are only needed temporarily and to calculate flux,
                    //   Air layer will be handled
                    //  as a pure R in the temperature calc.
                    // assign
                    // other properties based on resistance

                    mat->SpecHeat = 0.0001;
                    mat->Density = 1.0;
                    mat->Thickness = 0.1; //  arbitrary thickness for R layer
                    mat->Conductivity = mat->Thickness / mat->Resistance;
                    kt = mat->Conductivity;
                    thisConstructFD.Thickness(Layer) = mat->Thickness;

                    SigmaR(ConstrNum) += mat->Resistance; // add resistance of R layer
                    SigmaC(ConstrNum) += 0.0;             //  no capacitance for R layer

                    Alpha = kt / (mat->Density * mat->SpecHeat);
                    mAlpha = 0.0;
                } else if (thisConstruct.TypeIsIRT) { // make similar to air? (that didn't seem to work well)
                    ShowSevereError(state,
                                    format("InitHeatBalFiniteDiff: Construction =\"{}\" uses Material:InfraredTransparent. Cannot be used currently "
                                           "with finite difference calculations.",
                                           thisConstruct.Name));
                    if (thisConstruct.IsUsed) {
                        ShowContinueError(state, "...since this construction is used in a surface, the simulation is not allowed.");
                    } else {
                        ShowContinueError(state, "...if this construction were used in a surface, the simulation would be terminated.");
                    }
                    continue;
                } else {
                    auto *matReg = mat;
                    assert(matReg != nullptr);
                    //    Regular material Properties
                    kt = matReg->Conductivity;
                    RhoS = matReg->Density;
                    Por = matReg->Porosity;
                    Cp = matReg->SpecHeat;
                    // Need Resistance for reg layer
                    matReg->Resistance = matReg->Thickness / matReg->Conductivity;
                    Dv = matReg->VaporDiffus;
                    SigmaR(ConstrNum) += matReg->Resistance; // add resistance
                    SigmaC(ConstrNum) += matReg->Density * matReg->SpecHeat * matReg->Thickness;
                    Alpha = kt / (RhoS * Cp);
                    mAlpha = 0.0;

                    // check for Material layers that are too thin and highly conductivity (not appropriate for surface models)
                    if (Alpha > DataHeatBalance::HighDiffusivityThreshold) {
                        DeltaTimestep = state.dataGlobal->TimeStepZoneSec;
                        ThicknessThreshold = std::sqrt(Alpha * DeltaTimestep * 3.0);
                        if (mat->Thickness < ThicknessThreshold) {
                            ShowSevereError(
                                state,
                                format(
                                    "InitialInitHeatBalFiniteDiff: Found Material that is too thin and/or too highly conductive, material name = {}",
                                    mat->Name));
                            ShowContinueError(state,
                                              format("High conductivity Material layers are not well supported by Conduction Finite Difference, "
                                                     "material conductivity = {:.3R} [W/m-K]",
                                                     mat->Conductivity));
                            ShowContinueError(state, format("Material thermal diffusivity = {:.3R} [m2/s]", Alpha));
                            ShowContinueError(
                                state, format("Material with this thermal diffusivity should have thickness > {:.5R} [m]", ThicknessThreshold));
                            if (mat->Thickness < DataHeatBalance::ThinMaterialLayerThreshold) {
                                ShowContinueError(state,
                                                  format("Material may be too thin to be modeled well, thickness = {:.5R} [m]", mat->Thickness));
                                ShowContinueError(state,
                                                  format("Material with this thermal diffusivity should have thickness > {:.5R} [m]",
                                                         DataHeatBalance::ThinMaterialLayerThreshold));
                            }
                            ShowFatalError(state, "Preceding conditions cause termination.");
                        }
                    }

                } //  R, Air  or regular material properties and parameters

                // Proceed with setting node sizes in layers

                dxn = std::sqrt(Alpha * Delt * s_hbfd->SpaceDescritConstant); // The Fourier number is set using user constant

                // number of nodes=thickness/spacing.  This is number of full size node spaces across layer.
                Ipts1 = int(mat->Thickness / dxn);
                //  set high conductivity layers to a single full size node thickness. (two half nodes)
                if (Ipts1 <= 1) {
                    Ipts1 = 1;
                }
                if (mat->ROnly || mat->group == Material::Group::AirGap) {

                    Ipts1 = 1; //  single full node in R layers- surfaces of adjacent material or inside/outside layer
                }

                dxn = mat->Thickness / double(Ipts1); // full node thickness

                StabilityTemp = Alpha * Delt / pow_2(dxn);
                StabilityMoist = mAlpha * Delt / pow_2(dxn);
                thisConstructFD.TempStability(Layer) = StabilityTemp;
                thisConstructFD.MoistStability(Layer) = StabilityMoist;
                thisConstructFD.DelX(Layer) = dxn;

                TotNodes += Ipts1;                           //  number of full size nodes
                thisConstructFD.NodeNumPoint(Layer) = Ipts1; //  number of full size nodes
            } //  end of layer loop.

            thisConstructFD.TotNodes = TotNodes;
            thisConstructFD.DeltaTime = Delt;

        } // End of Construction Loop.  TotNodes in each construction now set

        // now determine x location, or distance that nodes are from the outside face in meters
        for (int ConstrNum = 1; ConstrNum <= state.dataHeatBal->TotConstructs; ++ConstrNum) {
            auto &thisConstructFD = ConstructFD(ConstrNum);
            auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum);
            if (thisConstructFD.TotNodes > 0) {
                thisConstructFD.NodeXlocation.allocate(thisConstructFD.TotNodes + 1);
                thisConstructFD.NodeXlocation = 0.0; // init them all
                Ipts1 = 0;                           // init counter
                for (Layer = 1; Layer <= thisConstruct.TotLayers; ++Layer) {
                    int OutwardMatLayerNum = Layer - 1;
                    for (int LayerNode = 1; LayerNode <= thisConstructFD.NodeNumPoint(Layer); ++LayerNode) {
                        ++Ipts1;
                        if (Ipts1 == 1) {
                            thisConstructFD.NodeXlocation(Ipts1) = 0.0; // first node is on outside face

                        } else if (LayerNode == 1) {
                            if (OutwardMatLayerNum > 0 && OutwardMatLayerNum <= thisConstruct.TotLayers) {
                                // later nodes are Delx away from previous, but use Delx from previous layer
                                thisConstructFD.NodeXlocation(Ipts1) =
                                    thisConstructFD.NodeXlocation(Ipts1 - 1) + thisConstructFD.DelX(OutwardMatLayerNum);
                            }
                        } else {
                            // later nodes are Delx away from previous
                            thisConstructFD.NodeXlocation(Ipts1) = thisConstructFD.NodeXlocation(Ipts1 - 1) + thisConstructFD.DelX(Layer);
                        }
                    }
                }
                Layer = thisConstruct.TotLayers;
                ++Ipts1;
                thisConstructFD.NodeXlocation(Ipts1) = thisConstructFD.NodeXlocation(Ipts1 - 1) + thisConstructFD.DelX(Layer);
            }
        }

        for (int Surf = 1; Surf <= state.dataSurface->TotSurfaces; ++Surf) {
            if (!state.dataSurface->Surface(Surf).HeatTransSurf) {
                continue;
            }
            if (state.dataSurface->Surface(Surf).Class == DataSurfaces::SurfaceClass::Window) {
                continue;
            }
            if (state.dataSurface->Surface(Surf).HeatTransferAlgorithm != DataSurfaces::HeatTransferModel::CondFD) {
                continue;
            }
            int ConstrNum = state.dataSurface->Surface(Surf).Construction;
            int TotNodes = ConstructFD(ConstrNum).TotNodes;
            int TotLayers = state.dataConstruction->Construct(ConstrNum).TotLayers;

            // Allocate the Surface Arrays
            SurfaceFD(Surf).T.allocate(TotNodes + 1);
            SurfaceFD(Surf).TOld.allocate(TotNodes + 1);
            SurfaceFD(Surf).TT.allocate(TotNodes + 1);
            SurfaceFD(Surf).Rhov.allocate(TotNodes + 1);
            SurfaceFD(Surf).RhovOld.allocate(TotNodes + 1);
            SurfaceFD(Surf).RhoT.allocate(TotNodes + 1);
            SurfaceFD(Surf).TD.allocate(TotNodes + 1);
            SurfaceFD(Surf).TDT.allocate(TotNodes + 1);
            SurfaceFD(Surf).TDTLast.allocate(TotNodes + 1);
            SurfaceFD(Surf).TDOld.allocate(TotNodes + 1);
            SurfaceFD(Surf).TDreport.allocate(TotNodes + 1);
            SurfaceFD(Surf).RH.allocate(TotNodes + 1);
            SurfaceFD(Surf).RHreport.allocate(TotNodes + 1);
            SurfaceFD(Surf).EnthOld.allocate(TotNodes + 1);
            SurfaceFD(Surf).EnthNew.allocate(TotNodes + 1);
            SurfaceFD(Surf).EnthLast.allocate(TotNodes + 1);
            SurfaceFD(Surf).QDreport.allocate(TotNodes + 1);
            SurfaceFD(Surf).CpDelXRhoS1.allocate(TotNodes + 1);
            SurfaceFD(Surf).CpDelXRhoS2.allocate(TotNodes + 1);
            SurfaceFD(Surf).TDpriortimestep.allocate(TotNodes + 1);
            SurfaceFD(Surf).PhaseChangeState.allocate(TotNodes + 1);
            SurfaceFD(Surf).PhaseChangeStateOld.allocate(TotNodes + 1);
            SurfaceFD(Surf).PhaseChangeStateOldOld.allocate(TotNodes + 1);
            SurfaceFD(Surf).PhaseChangeStateRep.allocate(TotNodes + 1);
            SurfaceFD(Surf).PhaseChangeStateOldRep.allocate(TotNodes + 1);
            SurfaceFD(Surf).PhaseChangeStateOldOldRep.allocate(TotNodes + 1);
            SurfaceFD(Surf).PhaseChangeTemperatureReverse.allocate(TotNodes + 1);
            SurfaceFD(Surf).condMaterialActuators.allocate(TotLayers);
            SurfaceFD(Surf).specHeatMaterialActuators.allocate(TotLayers);
            SurfaceFD(Surf).condNodeReport.allocate(TotNodes + 1);
            SurfaceFD(Surf).specHeatNodeReport.allocate(TotNodes + 1);
            SurfaceFD(Surf).heatSourceFluxMaterialActuators.allocate(TotLayers - 1);
            SurfaceFD(Surf).heatSourceInternalFluxLayerReport.allocate(TotLayers - 1);
            SurfaceFD(Surf).heatSourceInternalFluxEnergyLayerReport.allocate(TotLayers - 1);
            SurfaceFD(Surf).heatSourceEMSFluxLayerReport.allocate(TotLayers - 1);
            SurfaceFD(Surf).heatSourceEMSFluxEnergyLayerReport.allocate(TotLayers - 1);

            // Initialize the allocated arrays.
            SurfaceFD(Surf).T = TempInitValue;
            SurfaceFD(Surf).TOld = TempInitValue;
            SurfaceFD(Surf).TT = TempInitValue;
            SurfaceFD(Surf).Rhov = RhovInitValue;
            SurfaceFD(Surf).RhovOld = RhovInitValue;
            SurfaceFD(Surf).RhoT = RhovInitValue;
            SurfaceFD(Surf).TD = TempInitValue;
            SurfaceFD(Surf).TDT = TempInitValue;
            SurfaceFD(Surf).TDTLast = TempInitValue;
            SurfaceFD(Surf).TDOld = TempInitValue;
            SurfaceFD(Surf).TDreport = TempInitValue;
            SurfaceFD(Surf).RH = 0.0;
            SurfaceFD(Surf).RHreport = 0.0;
            SurfaceFD(Surf).EnthOld = EnthInitValue;
            SurfaceFD(Surf).EnthNew = EnthInitValue;
            SurfaceFD(Surf).EnthLast = EnthInitValue;
            SurfaceFD(Surf).QDreport = 0.0;
            SurfaceFD(Surf).CpDelXRhoS1 = 0.0;
            SurfaceFD(Surf).CpDelXRhoS2 = 0.0;
            SurfaceFD(Surf).TDpriortimestep = 0.0;
            SurfaceFD(Surf).PhaseChangeState = Material::Phase::Transition;
            SurfaceFD(Surf).PhaseChangeStateOld = Material::Phase::Transition;
            SurfaceFD(Surf).PhaseChangeStateOldOld = Material::Phase::Transition;
            SurfaceFD(Surf).PhaseChangeStateRep = Material::phaseInts[(int)Material::Phase::Transition];
            SurfaceFD(Surf).PhaseChangeStateOldRep = Material::phaseInts[(int)Material::Phase::Transition];
            SurfaceFD(Surf).PhaseChangeStateOldOldRep = Material::phaseInts[(int)Material::Phase::Transition];
            SurfaceFD(Surf).PhaseChangeTemperatureReverse = 50;
            SurfaceFD(Surf).condNodeReport = 0.0;
            SurfaceFD(Surf).specHeatNodeReport = 0.0;
            SurfaceFD(Surf).heatSourceInternalFluxLayerReport = 0.0;
            SurfaceFD(Surf).heatSourceInternalFluxEnergyLayerReport = 0.0;
            SurfaceFD(Surf).heatSourceEMSFluxLayerReport = 0.0;
            SurfaceFD(Surf).heatSourceEMSFluxEnergyLayerReport = 0.0;

            // Setup EMS data
            for (int lay = 1; lay <= TotLayers; ++lay) {
                // Setup material layer names actuators
                int matLay = state.dataConstruction->Construct(ConstrNum).LayerPoint(lay);
                // Actuator name format: "{SurfName}:{MaterialLayerName}"
                std::string actName = fmt::format("{}:{}", state.dataSurface->Surface(Surf).Name, state.dataMaterial->materials(matLay)->Name);
                SurfaceFD(Surf).condMaterialActuators(lay).actuatorName = actName;
                SurfaceFD(Surf).specHeatMaterialActuators(lay).actuatorName = actName;

                // only setup for heat source actuator for layers 1 to N-1
                if (lay != TotLayers) {
                    SurfaceFD(Surf).heatSourceFluxMaterialActuators(lay).actuatorName = actName;
                }
            }
        }

        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            if (!state.dataSurface->Surface(SurfNum).HeatTransSurf) {
                continue;
            }
            if (state.dataSurface->Surface(SurfNum).Class == DataSurfaces::SurfaceClass::Window) {
                continue;
            }
            if (state.dataSurface->Surface(SurfNum).HeatTransferAlgorithm != DataSurfaces::HeatTransferModel::CondFD) {
                continue;
            }

            SetupOutputVariable(state,
                                "CondFD Inner Solver Loop Iteration Count",
                                Constant::Units::None,
                                SurfaceFD(SurfNum).GSloopCounter,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataSurface->Surface(SurfNum).Name);

            // Setup EMS Material Actuators for Conductivity and Specific Heat
            int ConstrNum = state.dataSurface->Surface(SurfNum).Construction;

            auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum);
            // Setup internal heat source output variables
            // Only setup for layers 1 to N-1
            for (int lay = 1; lay < thisConstruct.TotLayers; ++lay) {
                SetupOutputVariable(state,
                                    format("CondFD Internal Heat Source Power After Layer {}", lay),
                                    Constant::Units::W,
                                    SurfaceFD(SurfNum).heatSourceInternalFluxLayerReport(lay),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataSurface->Surface(SurfNum).Name);
                SetupOutputVariable(state,
                                    format("CondFD Internal Heat Source Energy After Layer {}", lay),
                                    Constant::Units::J,
                                    SurfaceFD(SurfNum).heatSourceInternalFluxEnergyLayerReport(lay),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataSurface->Surface(SurfNum).Name);
            }

            if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                for (int lay = 1; lay <= thisConstruct.TotLayers; ++lay) {
                    EnergyPlus::SetupEMSActuator(state,
                                                 "CondFD Surface Material Layer",
                                                 SurfaceFD(SurfNum).condMaterialActuators(lay).actuatorName,
                                                 "Thermal Conductivity",
                                                 "[W/m-K]",
                                                 SurfaceFD(SurfNum).condMaterialActuators(lay).isActuated,
                                                 SurfaceFD(SurfNum).condMaterialActuators(lay).actuatedValue);
                    EnergyPlus::SetupEMSActuator(state,
                                                 "CondFD Surface Material Layer",
                                                 SurfaceFD(SurfNum).specHeatMaterialActuators(lay).actuatorName,
                                                 "Specific Heat",
                                                 "[J/kg-C]",
                                                 SurfaceFD(SurfNum).specHeatMaterialActuators(lay).isActuated,
                                                 SurfaceFD(SurfNum).specHeatMaterialActuators(lay).actuatedValue);
                }

                // Setup EMS Actuator and Output Variables for Heat Flux
                // Only setup for layers 1 to N-1
                for (int lay = 1; lay < thisConstruct.TotLayers; ++lay) {
                    EnergyPlus::SetupEMSActuator(state,
                                                 "CondFD Surface Material Layer",
                                                 SurfaceFD(SurfNum).heatSourceFluxMaterialActuators(lay).actuatorName,
                                                 "Heat Flux",
                                                 "[W/m2]",
                                                 SurfaceFD(SurfNum).heatSourceFluxMaterialActuators(lay).isActuated,
                                                 SurfaceFD(SurfNum).heatSourceFluxMaterialActuators(lay).actuatedValue);
                    SetupOutputVariable(state,
                                        format("CondFD EMS Heat Source Power After Layer {}", lay),
                                        Constant::Units::W,
                                        SurfaceFD(SurfNum).heatSourceEMSFluxLayerReport(lay),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        state.dataSurface->Surface(SurfNum).Name);
                    SetupOutputVariable(state,
                                        format("CondFD EMS Heat Source Energy After Layer {}", lay),
                                        Constant::Units::J,
                                        SurfaceFD(SurfNum).heatSourceEMSFluxEnergyLayerReport(lay),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Sum,
                                        state.dataSurface->Surface(SurfNum).Name,
                                        Constant::eResource::Electricity,
                                        OutputProcessor::Group::Building,
                                        OutputProcessor::EndUseCat::Heating);
                }
            }

            int TotNodes = ConstructFD(state.dataSurface->Surface(SurfNum).Construction).TotNodes; // Full size nodes, start with outside face.
            for (int node = 1; node <= TotNodes + 1; ++node) {                                     // include inside face node
                SetupOutputVariable(state,
                                    format("CondFD Surface Temperature Node {}", node),
                                    Constant::Units::C,
                                    SurfaceFD(SurfNum).TDreport(node),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataSurface->Surface(SurfNum).Name);
                SetupOutputVariable(state,
                                    format("CondFD Surface Heat Flux Node {}", node),
                                    Constant::Units::W_m2,
                                    SurfaceFD(SurfNum).QDreport(node),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataSurface->Surface(SurfNum).Name);
                SetupOutputVariable(state,
                                    format("CondFD Phase Change State {}", node),
                                    Constant::Units::None,
                                    (int &)SurfaceFD(SurfNum).PhaseChangeStateRep(node),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataSurface->Surface(SurfNum).Name);
                SetupOutputVariable(state,
                                    format("CondFD Phase Change Previous State {}", node),
                                    Constant::Units::None,
                                    (int &)SurfaceFD(SurfNum).PhaseChangeStateOldRep(node),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataSurface->Surface(SurfNum).Name);
                SetupOutputVariable(state,
                                    format("CondFD Phase Change Node Temperature {}", node),
                                    Constant::Units::C,
                                    SurfaceFD(SurfNum).TDT(node),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataSurface->Surface(SurfNum).Name);
                SetupOutputVariable(state,
                                    format("CondFD Phase Change Node Conductivity {}", node),
                                    Constant::Units::W_mK,
                                    SurfaceFD(SurfNum).condNodeReport(node),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataSurface->Surface(SurfNum).Name);
                SetupOutputVariable(state,
                                    format("CondFD Phase Change Node Specific Heat {}", node),
                                    Constant::Units::J_kgK,
                                    SurfaceFD(SurfNum).specHeatNodeReport(node),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataSurface->Surface(SurfNum).Name);
                if (state.dataGlobal->DisplayAdvancedReportVariables) {
                    SetupOutputVariable(state,
                                        format("CondFD Surface Heat Capacitance Outer Half Node {}", node),
                                        Constant::Units::W_m2K,
                                        SurfaceFD(SurfNum).CpDelXRhoS1(node),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        state.dataSurface->Surface(SurfNum).Name);
                    SetupOutputVariable(state,
                                        format("CondFD Surface Heat Capacitance Inner Half Node {}", node),
                                        Constant::Units::W_m2K,
                                        SurfaceFD(SurfNum).CpDelXRhoS2(node),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        state.dataSurface->Surface(SurfNum).Name);
                }
            }

        } // End of the Surface Loop for Report Variable Setup

        ReportFiniteDiffInits(state); // Report the results from the Finite Diff Inits
    }

    int numNodesInMaterialLayer(EnergyPlusData &state, std::string const &surfName, std::string const &matName)
    {
        auto &s_hbfd = state.dataHeatBalFiniteDiffMgr;
        for (auto const &surface : state.dataSurface->Surface) {
            if (surface.Name == surfName) {
                int constrNum = surface.Construction;
                for (int lay = 1; lay <= state.dataConstruction->Construct(constrNum).TotLayers; ++lay) {
                    int matLay = state.dataConstruction->Construct(constrNum).LayerPoint(lay);
                    if (state.dataMaterial->materials(matLay)->Name == matName) {
                        return s_hbfd->ConstructFD(constrNum).NodeNumPoint(lay);
                    }
                }
            }
        }

        return 0;
    }

    void relax_array(Array1D<Real64> &a,       // Array to relax
                     Array1D<Real64> const &b, // Array to relax towards
                     Real64 const r            // Relaxation factor [0-1]
    )
    {
        assert(equal_dimensions(a, b));
        assert((0.0 <= r) && (r <= 1.0));
        Real64 const q(1.0 - r);
        for (int i = a.l(), e = a.u(); i <= e; ++i) {
            a(i) = r * b(i) + q * a(i);
        }
    }

    Real64 sum_array_diff(Array1D<Real64> const &a, Array1D<Real64> const &b)
    {
        assert(equal_dimensions(a, b));
        Real64 s(0.0);
        for (int i = a.l(), e = a.u(); i <= e; ++i) {
            s += a(i) - b(i); //? Should this be in abs?
        }
        return s;
    }

    void CalcHeatBalFiniteDiff(EnergyPlusData &state,
                               int const Surf,        // Surface number
                               Real64 &SurfTempInTmp, // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                               Real64 &TempSurfOutTmp // Outside Surface Temperature of each Heat Transfer Surface
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard J. Liesen
        //       DATE WRITTEN   Oct 2003
        //       MODIFIED       Aug 2006 by C O Pedersen to include implicit solution and variable properties with
        //                                material enthalpy added for Phase Change Materials.
        //                      Sept 2010 B. Griffith, remove allocate/deallocate, use structure variables
        //                      March 2011 P. Tabares, add relaxation factor and add surfIteration to
        //                                 update TD and TDT, correct interzone partition
        //                      May 2011  B. Griffith add logging and errors when inner GS loop does not converge
        //                      November 2011 P. Tabares fixed problems with adiabatic walls/massless walls and PCM stability problems

        // PURPOSE OF THIS SUBROUTINE:
        // this routine controls the calculation of the fluxes and temperatures using
        //      finite difference procedures for
        //      all building surface constructs.

        int const ConstrNum = state.dataSurface->Surface(Surf).Construction;

        auto &s_hbfd = state.dataHeatBalFiniteDiffMgr;
        auto &constructFD = s_hbfd->ConstructFD(ConstrNum);

        int const TotNodes = constructFD.TotNodes;
        int const TotLayers = state.dataConstruction->Construct(ConstrNum).TotLayers;

        SurfTempInTmp = 0.0;
        TempSurfOutTmp = 0.0;

        int const Delt = constructFD.DeltaTime; //   (seconds)

        // Aliases
        auto &surfaceFD = s_hbfd->SurfaceFD(Surf);

        Real64 HMovInsul = 0;
        if (state.dataSurface->AnyMovableInsulation) {
            HMovInsul = state.dataSurface->extMovInsuls(Surf).H; // Even if this is not a movable insulation surface?
        }
        // Start stepping through the slab with time.
        for (int J = 1, J_end = nint(state.dataGlobal->TimeStepZoneSec / Delt); J <= J_end; ++J) { // PT testing higher time steps

            int GSiter;                                               // iteration counter for implicit repeat calculation
            for (GSiter = 1; GSiter <= s_hbfd->MaxGSiter; ++GSiter) { //  Iterate implicit equations
                surfaceFD.TDTLast = surfaceFD.TDT;                    // Save last iteration's TDT (New temperature) values
                surfaceFD.EnthLast = surfaceFD.EnthNew;               // Last iterations new enthalpy value

                // Original loop version
                int i(1);                                    //  Node counter
                for (int Lay = 1; Lay <= TotLayers; ++Lay) { // Begin layer loop ...

                    // For the exterior surface node with a convective boundary condition
                    if ((i == 1) && (Lay == 1)) {
                        ExteriorBCEqns(state,
                                       Delt,
                                       i,
                                       Lay,
                                       Surf,
                                       surfaceFD.T,
                                       surfaceFD.TT,
                                       surfaceFD.Rhov,
                                       surfaceFD.RhoT,
                                       surfaceFD.RH,
                                       surfaceFD.TD,
                                       surfaceFD.TDT,
                                       surfaceFD.EnthOld,
                                       surfaceFD.EnthNew,
                                       TotNodes,
                                       HMovInsul);
                    }

                    // For the Layer Interior nodes.  Arrive here after exterior surface node or interface node
                    if (TotNodes != 1) {
                        for (int ctr = 2, ctr_end = constructFD.NodeNumPoint(Lay); ctr <= ctr_end; ++ctr) {
                            ++i;
                            InteriorNodeEqns(state,
                                             Delt,
                                             i,
                                             Lay,
                                             Surf,
                                             surfaceFD.T,
                                             surfaceFD.TT,
                                             surfaceFD.Rhov,
                                             surfaceFD.RhoT,
                                             surfaceFD.RH,
                                             surfaceFD.TD,
                                             surfaceFD.TDT,
                                             surfaceFD.EnthOld,
                                             surfaceFD.EnthNew);
                        }
                    }

                    if ((Lay < TotLayers) && (TotNodes != 1)) { // Interface equations for 2 capacitive materials
                        ++i;
                        IntInterfaceNodeEqns(state,
                                             Delt,
                                             i,
                                             Lay,
                                             Surf,
                                             surfaceFD.T,
                                             surfaceFD.TT,
                                             surfaceFD.Rhov,
                                             surfaceFD.RhoT,
                                             surfaceFD.RH,
                                             surfaceFD.TD,
                                             surfaceFD.TDT,
                                             surfaceFD.EnthOld,
                                             surfaceFD.EnthNew,
                                             GSiter);
                    } else if (Lay == TotLayers) { // For the Interior surface node with a convective boundary condition
                        ++i;
                        InteriorBCEqns(state,
                                       Delt,
                                       i,
                                       Lay,
                                       Surf,
                                       surfaceFD.T,
                                       surfaceFD.TT,
                                       surfaceFD.Rhov,
                                       surfaceFD.RhoT,
                                       surfaceFD.RH,
                                       surfaceFD.TD,
                                       surfaceFD.TDT,
                                       surfaceFD.EnthOld,
                                       surfaceFD.EnthNew,
                                       surfaceFD.TDreport);
                    }

                } // layer loop

                // Apply Relaxation factor for stability, use current (TDT) and previous (TDTLast) iteration temperature values
                // to obtain the actual temperature that is going to be used for next iteration. This would mostly happen with PCM
                // Tuned Function call to eliminate array temporaries and multiple relaxation passes
                if (GSiter > 15) {
                    relax_array(surfaceFD.TDT, surfaceFD.TDTLast, 0.9875);
                } else if (GSiter > 10) {
                    relax_array(surfaceFD.TDT, surfaceFD.TDTLast, 0.875);
                } else if (GSiter > 5) {
                    relax_array(surfaceFD.TDT, surfaceFD.TDTLast, 0.5);
                }

                // the following could blow up when all the node temps sum to less than 1.0.  seems poorly formulated for temperature in C.
                // PT delete one zero and decrease number of minimum iterations, from 3 (which actually requires 4 iterations) to 2.

                if ((GSiter > 2) && (std::abs(sum_array_diff(surfaceFD.TDT, surfaceFD.TDTLast) / sum(surfaceFD.TDT)) < 0.00001)) {
                    break;
                }

            } // End of Gauss Seidell iteration loop

            surfaceFD.GSloopCounter = GSiter; // outputs GSloop iterations, useful for pinpointing stability issues with condFD
            if (state.dataHeatBal->CondFDRelaxFactor != 1.0) {
                // Apply Relaxation factor for stability, use current (TDT) and previous (TDreport) temperature values
                //   to obtain the actual temperature that is going to be exported/use
                relax_array(surfaceFD.TDT, surfaceFD.TDreport, 1.0 - state.dataHeatBal->CondFDRelaxFactor);
                surfaceFD.EnthOld = surfaceFD.EnthNew;
            }

            for (int I = 1; I <= (TotNodes + 1); I++) {
                // When the phase change process reverses its direction while melting or freezing (without completing its phase
                // to either liquid or solid), the temperature at which it changes its direction is saved
                // in the variable PhaseChangeTemperatureReverse, and this variable will hold the value of the temperature until
                // the next reverse in the process takes place.
                if (((surfaceFD.PhaseChangeStateOld(I) == Material::Phase::Freezing &&
                      surfaceFD.PhaseChangeState(I) == Material::Phase::Transition) ||
                     (surfaceFD.PhaseChangeStateOld(I) == Material::Phase::Transition &&
                      surfaceFD.PhaseChangeState(I) == Material::Phase::Freezing)) ||
                    ((surfaceFD.PhaseChangeStateOld(I) == Material::Phase::Melting && surfaceFD.PhaseChangeState(I) == Material::Phase::Transition) ||
                     (surfaceFD.PhaseChangeStateOld(I) == Material::Phase::Transition &&
                      surfaceFD.PhaseChangeState(I) == Material::Phase::Melting))) {
                    surfaceFD.PhaseChangeTemperatureReverse(I) = surfaceFD.TDT(I);
                }
            }

            surfaceFD.PhaseChangeStateOldOld = surfaceFD.PhaseChangeStateOld;
            surfaceFD.PhaseChangeStateOld = surfaceFD.PhaseChangeState;

            surfaceFD.PhaseChangeStateOldOldRep = surfaceFD.PhaseChangeStateOldRep;
            surfaceFD.PhaseChangeStateOldRep = surfaceFD.PhaseChangeStateRep;
        } // Time Loop  //PT solving time steps

        TempSurfOutTmp = surfaceFD.TDT(1);
        SurfTempInTmp = surfaceFD.TDT(TotNodes + 1);
        state.dataMstBal->RhoVaporSurfIn(Surf) = 0.0;

        // For ground surfaces or when raining, outside face inner half-node heat capacity was unknown and set to -1 in ExteriorBCEqns
        // Now check for the flag and set equal to the second node's outer half-node heat capacity if needed
        if (surfaceFD.CpDelXRhoS2(1) == -1.0) {
            surfaceFD.CpDelXRhoS2(1) = surfaceFD.CpDelXRhoS1(2); // Set to node 2's outer half node heat capacity
        }
        CalcNodeHeatFlux(state, Surf, TotNodes);

        // Determine largest change in node temps
        Real64 MaxDelTemp = 0.0;
        for (int NodeNum = 1; NodeNum <= TotNodes + 1; ++NodeNum) { // need to consider all nodes
            MaxDelTemp = max(std::abs(surfaceFD.TDT(NodeNum) - surfaceFD.TDreport(NodeNum)), MaxDelTemp);
        }
        surfaceFD.MaxNodeDelTemp = MaxDelTemp;
        surfaceFD.TDreport = surfaceFD.TDT;
        surfaceFD.EnthOld = surfaceFD.EnthNew;
    }

    void ReportFiniteDiffInits(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   November 2003
        //       MODIFIED       B. Griffith, May 2011 add reporting of node x locations

        // PURPOSE OF THIS SUBROUTINE:
        // This routine gives a detailed report to the user about
        // the initializations for the Finite Difference calculations
        // of each construction.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool DoReport;

        auto &s_hbfd = state.dataHeatBalFiniteDiffMgr;

        // Formats
        static constexpr std::string_view Format_702(" ConductionFiniteDifference Node,{},{:.8R},{},{},{}\n");

        print(state.files.eio,
              "! <ConductionFiniteDifference HeatBalanceSettings>,Scheme Type,Space Discretization Constant,Relaxation Factor,Inside Face Surface "
              "Temperature Convergence Criteria\n");
        print(state.files.eio,
              " ConductionFiniteDifference HeatBalanceSettings,{},{:.2R},{:.2R},{:.4R}\n",
              CondFDSchemeTypeNamesCC[static_cast<int>(s_hbfd->CondFDSchemeType)],
              s_hbfd->SpaceDescritConstant,
              state.dataHeatBal->CondFDRelaxFactorInput,
              state.dataHeatBal->MaxAllowedDelTempCondFD);

        General::ScanForReports(state, "Constructions", DoReport, "Constructions");

        if (DoReport) {

            //                                      Write Descriptions
            print(state.files.eio, "{}\n", "! <Construction CondFD>,Construction Name,Index,#Layers,#Nodes,Time Step {hours}");
            print(state.files.eio,
                  "{}\n",
                  "! <Material CondFD Summary>,Material Name,Thickness {m},#Layer Elements,Layer Delta X,Layer Alpha*Delt/Delx**2,Layer Moisture "
                  "Stability");

            // HT Algo issue
            if (state.dataHeatBal->AnyCondFD) {
                print(state.files.eio,
                      "{}\n",
                      "! <ConductionFiniteDifference Node>,Node Identifier, Node Distance From Outside Face {m}, Construction Name, Outward Material "
                      "Name (or Face), Inward Material Name (or Face)");
            }

            for (int ThisNum = 1; ThisNum <= state.dataHeatBal->TotConstructs; ++ThisNum) {
                auto &construct = state.dataConstruction->Construct(ThisNum);

                if (construct.TypeIsWindow) {
                    continue;
                }
                if (construct.TypeIsIRT) {
                    continue;
                }
                if (construct.TypeIsAirBoundary) {
                    continue;
                }
                if (!construct.IsUsed) {
                    continue;
                }
                if (!findAnySurfacesUsingConstructionAndCondFD(state, ThisNum)) {
                    continue;
                }

                auto &constructFD = s_hbfd->ConstructFD(ThisNum);
                static constexpr std::string_view Format_700(" Construction CondFD,{},{},{},{},{:.6R}\n");
                print(state.files.eio,
                      Format_700,
                      construct.Name,
                      ThisNum,
                      construct.TotLayers,
                      int(constructFD.TotNodes + 1),
                      constructFD.DeltaTime / Constant::rSecsInHour);

                for (int Layer = 1; Layer <= construct.TotLayers; ++Layer) {
                    static constexpr std::string_view Format_701(" Material CondFD Summary,{},{:.4R},{},{:.8R},{:.8R},{:.8R}\n");
                    print(state.files.eio,
                          Format_701,
                          constructFD.Name(Layer),
                          constructFD.Thickness(Layer),
                          constructFD.NodeNumPoint(Layer),
                          constructFD.DelX(Layer),
                          constructFD.TempStability(Layer),
                          constructFD.MoistStability(Layer));
                }

                // now list each CondFD Node with its X distance from outside face in m along with other identifiers
                int Inodes = 0;

                for (int Layer = 1; Layer <= construct.TotLayers; ++Layer) {
                    int OutwardMatLayerNum = Layer - 1;
                    for (int LayerNode = 1; LayerNode <= constructFD.NodeNumPoint(Layer); ++LayerNode) {
                        ++Inodes;
                        if (Inodes == 1) {
                            print(state.files.eio,
                                  Format_702,
                                  format("Node #{}", Inodes),
                                  constructFD.NodeXlocation(Inodes),
                                  construct.Name,
                                  "Surface Outside Face",
                                  constructFD.Name(Layer));

                        } else if (LayerNode == 1) {

                            if (OutwardMatLayerNum > 0 && OutwardMatLayerNum <= construct.TotLayers) {
                                print(state.files.eio,
                                      Format_702,
                                      format("Node #{}", Inodes),
                                      constructFD.NodeXlocation(Inodes),
                                      construct.Name,
                                      constructFD.Name(OutwardMatLayerNum),
                                      constructFD.Name(Layer));
                            }
                        } else if (LayerNode > 1) {
                            OutwardMatLayerNum = Layer;
                            print(state.files.eio,
                                  Format_702,
                                  format("Node #{}", Inodes),
                                  constructFD.NodeXlocation(Inodes),
                                  construct.Name,
                                  constructFD.Name(OutwardMatLayerNum),
                                  constructFD.Name(Layer));
                        }
                    }
                }

                int Layer = construct.TotLayers;
                ++Inodes;
                print(state.files.eio,
                      Format_702,
                      format("Node #{}", Inodes),
                      constructFD.NodeXlocation(Inodes),
                      construct.Name,
                      constructFD.Name(Layer),
                      "Surface Inside Face");
            }
        }
    }

    Real64 terpld(Array2<Real64> const &a, Real64 const x1, int const nind, int const ndep)
    {
        // author:c. o. pedersen
        // purpose:
        //   this function performs a linear interpolation
        //     on a two dimensional array containing both
        //     dependent and independent variables.

        // inputs:
        //  a = two dimensional array
        //  nind=row containing independent variable
        //  ndep=row containing the dependent variable
        //   x1 = specific independent variable value for which
        //      interpolated output is wanted
        // outputs:
        //    the value of dependent variable corresponding
        //       to x1
        //    routine returns first or last dependent variable
        //      for out of range x1.

        int const first(a.l2());

        assert(a.size() > 0u);
        Array2<Real64>::size_type l(1);
        Real64 r(a[0]);
        int last(first);
        for (int i1 = first + 1, e1 = a.u2(); i1 <= e1; ++i1, ++l) {
            if (a[l] > r) {
                r = a[l];
                last = i1;
            }
        }

        Array2<Real64>::size_type lind(a.index(nind, 0));
        Array2<Real64>::size_type ldep(a.index(ndep, 0));
        if ((a.size2() == 1u) || (x1 <= a[lind + first])) { // [ lind + first ] == ( nind, first )
            return a[ldep + first];                         // [ ldep + first ] == ( ndep, first )
        } else if (x1 >= a[lind + last]) {                  // [ lind + last ] == ( nind, last )
            return a[ldep + last];                          // [ ldep + last ] == ( ndep, last )
        } else {
            int i;
            int i1(first);
            int i2(last);
            while ((i2 - i1) > 1) {
                i = i1 + ((i2 - i1) >> 1); // Tuned bit shift replaces / 2
                if (x1 < a[lind + i]) {    // [ lind + i ] == ( nind, i )
                    i2 = i;
                } else {
                    i1 = i;
                }
            }
            i = i2;
            lind += i;
            ldep += i;
            Real64 const fract((x1 - a[lind - 1]) / (a[lind] - a[lind - 1])); // [ lind ] == ( nind, i ), [ lind - 1 ] == ( nind, i - 1 )
            return a[ldep - 1] + fract * (a[ldep] - a[ldep - 1]);             // [ ldep ] == ( ndep, i ), [ ldep - 1 ] == ( ndep, i - 1 )
        }
    }

    void ExteriorBCEqns(EnergyPlusData &state,
                        int const Delt,                               // Time Increment
                        int const i,                                  // Node Index
                        int const Lay,                                // Layer Number for Construction
                        int const Surf,                               // Surface number
                        [[maybe_unused]] Array1D<Real64> const &T,    // Old node Temperature in MFD finite difference solution
                        Array1D<Real64> &TT,                          // New node Temperature in MFD finite difference solution.
                        [[maybe_unused]] Array1D<Real64> const &Rhov, // MFD Nodal Vapor Density[kg/m3] and is the old or last time step result.
                        Array1D<Real64> &RhoT,                        // MFD vapor density for the new time step.
                        [[maybe_unused]] Array1D<Real64> &RH,         // Nodal relative humidity
                        Array1D<Real64> const &TD,                    // The old dry Temperature at each node for the CondFD algorithm..
                        Array1D<Real64> &TDT,     // The current or new Temperature at each node location for the CondFD solution..
                        Array1D<Real64> &EnthOld, // Old Nodal enthalpy
                        Array1D<Real64> &EnthNew, // New Nodal enthalpy
                        int const TotNodes,       // Total nodes in layer
                        Real64 const HMovInsul    // Conductance of movable(transparent) insulation.
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   November, 2003
        //       MODIFIED       B. Griffith 2010, fix adiabatic and other side surfaces
        //                      May 2011, B. Griffith, P. Tabares
        //                      November 2011 P. Tabares fixed problems with adiabatic walls/massless walls
        //                      November 2011 P. Tabares fixed problems PCM stability problems
        //       RE-ENGINEERED  Curtis Pedersen 2006

        auto &s_hbfd = state.dataHeatBalFiniteDiffMgr;
        auto const &surface(state.dataSurface->Surface(Surf));
        int const surface_ExtBoundCond(surface.ExtBoundCond);

        Real64 Tsky;
        Real64 QRadSWOutFD;             // Short wave radiation absorbed on outside of opaque surface
        Real64 QRadSWOutMvInsulFD(0.0); // SW radiation at outside of Movable Insulation
        if (surface_ExtBoundCond == DataSurfaces::OtherSideCondModeledExt) {
            // CR8046 switch modeled rad temp for sky temp.
            Tsky = state.dataSurface->OSCM(surface.OSCMPtr).TRad;
            QRadSWOutFD = 0.0; // eliminate incident shortwave on underlying surface
        } else {               // Set the external conditions to local variables
            QRadSWOutFD = state.dataHeatBalSurf->SurfOpaqQRadSWOutAbs(Surf);
            QRadSWOutMvInsulFD = state.dataHeatBalSurf->SurfQRadSWOutMvIns(Surf);
            Tsky = state.dataEnvrn->SkyTemp;
        }

        if (surface_ExtBoundCond == DataSurfaces::Ground || state.dataEnvrn->IsRain) {
            TDT(i) = TT(i) = state.dataMstBal->TempOutsideAirFD(Surf);
            RhoT(i) = state.dataMstBal->RhoVaporAirOut(Surf);
            s_hbfd->SurfaceFD(Surf).CpDelXRhoS1(i) = 0.0;  // Outside face  does not have an outer half node
            s_hbfd->SurfaceFD(Surf).CpDelXRhoS2(i) = -1.0; // Set this to -1 as a flag, then set to node 2's outer half node heat capacity
        } else if (surface_ExtBoundCond > 0) {
            // this is actually the inside face of another surface, or maybe this same surface if adiabatic
            // switch around arguments for the other surf and call routines as for interior side BC from opposite face

            int const ext_bound_construction(state.dataSurface->Surface(surface_ExtBoundCond).Construction);
            int const LayIn(state.dataConstruction->Construct(ext_bound_construction).TotLayers); // layer number for call to interior eqs
            int const NodeIn(s_hbfd->ConstructFD(ext_bound_construction).TotNodes + 1);           // node number "I" for call to interior eqs
            int const TotNodesPlusOne(TotNodes + 1);
            if (surface_ExtBoundCond == Surf) { // adiabatic surface, PT added since it is not the same as interzone wall
                // as Outside Boundary Condition Object can be left blank.

                auto &surfaceFD = s_hbfd->SurfaceFD(Surf);
                InteriorBCEqns(state,
                               Delt,
                               NodeIn,
                               LayIn,
                               Surf,
                               surfaceFD.T,
                               surfaceFD.TT,
                               surfaceFD.Rhov,
                               surfaceFD.RhoT,
                               surfaceFD.RH,
                               surfaceFD.TD,
                               surfaceFD.TDT,
                               surfaceFD.EnthOld,
                               surfaceFD.EnthNew,
                               surfaceFD.TDreport);
                TDT(i) = surfaceFD.TDT(TotNodesPlusOne);
                TT(i) = surfaceFD.TT(TotNodesPlusOne);
                RhoT(i) = surfaceFD.RhoT(TotNodesPlusOne);

                surfaceFD.CpDelXRhoS1(i) = 0.0;                                    // Outside face  does not have an outer half node
                surfaceFD.CpDelXRhoS2(i) = surfaceFD.CpDelXRhoS1(TotNodesPlusOne); // Save this for computing node flux values

            } else {

                // potential-lkl-from old      CALL InteriorBCEqns(Delt,nodeIn,LayIn,Surf,SurfaceFD(Surface(Surf)%ExtBoundCond)%T, &
                auto &surfaceFDEBC = s_hbfd->SurfaceFD(surface_ExtBoundCond);
                InteriorBCEqns(state,
                               Delt,
                               NodeIn,
                               LayIn,
                               surface_ExtBoundCond,
                               surfaceFDEBC.T,
                               surfaceFDEBC.TT,
                               surfaceFDEBC.Rhov,
                               surfaceFDEBC.RhoT,
                               surfaceFDEBC.RH,
                               surfaceFDEBC.TD,
                               surfaceFDEBC.TDT,
                               surfaceFDEBC.EnthOld,
                               surfaceFDEBC.EnthNew,
                               surfaceFDEBC.TDreport);

                TDT(i) = surfaceFDEBC.TDT(TotNodesPlusOne);
                TT(i) = surfaceFDEBC.TT(TotNodesPlusOne);
                RhoT(i) = surfaceFDEBC.RhoT(TotNodesPlusOne);

                s_hbfd->SurfaceFD(Surf).CpDelXRhoS1(i) = 0.0;                                       // Outside face  does not have an outer half node
                s_hbfd->SurfaceFD(Surf).CpDelXRhoS2(i) = surfaceFDEBC.CpDelXRhoS1(TotNodesPlusOne); // Save this for computing node flux values
            }

            Real64 const QNetSurfFromOutside(state.dataHeatBalSurf->SurfOpaqInsFaceCondFlux(surface_ExtBoundCond)); // filled in InteriorBCEqns
            //    QFluxOutsideToOutSurf(Surf)       = QnetSurfFromOutside
            state.dataHeatBalSurf->SurfOpaqOutFaceCondFlux(Surf) = -QNetSurfFromOutside;
            s_hbfd->QHeatOutFlux(Surf) = QNetSurfFromOutside;

        } else { // regular outside conditions
            Real64 TDT_i(TDT(i));
            Real64 const TDT_p(TDT(i + 1));

            Real64 Tgndsurface = 0.0;
            if (state.dataSurface->Surface(Surf).UseSurfPropertyGndSurfTemp) {
                Tgndsurface = state.dataSurface->GroundSurfsProperty(Surf).SurfsTempAvg;
            } else {
                Tgndsurface = state.dataMstBal->TempOutsideAirFD(Surf);
            }

            Real64 TsurrSurface = 0.0;
            if (state.dataSurface->Surface(Surf).SurfHasSurroundingSurfProperty) {
                TsurrSurface = state.dataSurface->Surface(Surf).SrdSurfTemp;
            }

            // Boundary Conditions from Simulation for Exterior
            Real64 const hconvo(state.dataMstBal->HConvExtFD(Surf));

            Real64 const hrad(state.dataMstBal->HAirFD(Surf));
            Real64 const hsky(state.dataMstBal->HSkyFD(Surf));
            Real64 const hgnd(state.dataMstBal->HGrndFD(Surf));
            Real64 const hsurr(state.dataMstBal->HSurrFD(Surf));
            Real64 const Toa(state.dataMstBal->TempOutsideAirFD(Surf));
            Real64 const Tgnd(Tgndsurface);
            Real64 const Tsurr(TsurrSurface);

            if (surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CondFD) {

                int const ConstrNum(surface.Construction);
                int const MatLay(state.dataConstruction->Construct(ConstrNum).LayerPoint(Lay));
                auto *mat = state.dataMaterial->materials(MatLay);
                auto const &matFD(s_hbfd->MaterialFD(MatLay));
                auto const &condActuator = s_hbfd->SurfaceFD(Surf).condMaterialActuators(Lay);
                auto const &specHeatActuator = s_hbfd->SurfaceFD(Surf).specHeatMaterialActuators(Lay);

                // regular outside conditions

                // Calculate the Dry Heat Conduction Equation

                if (mat->ROnly || mat->group == Material::Group::AirGap) { // R Layer or Air Layer  **********
                    // Use algebraic equation for TDT based on R
                    Real64 const Rlayer(mat->Resistance);
                    TDT_i = (TDT_p + (QRadSWOutFD + hgnd * Tgnd + (hconvo + hrad) * Toa + hsky * Tsky + hsurr * Tsurr) * Rlayer) /
                            (1.0 + (hconvo + hgnd + hrad + hsky + hsurr) * Rlayer);

                } else { // Regular or phase change material layer

                    // Set Thermal Conductivity. Can be constant, simple linear temp dep or multiple linear segment temp function dep.
                    auto const &matFD_TempCond(matFD.TempCond);
                    assert(matFD_TempCond.u2() >= 3);
                    Real64 const lTC(matFD_TempCond.index(2, 1));
                    Real64 kt;
                    if (matFD_TempCond[lTC] + matFD_TempCond[lTC + 1] + matFD_TempCond[lTC + 2] >= 0.0) { // Multiple Linear Segment Function
                        // Use average temp of surface and first node for k
                        kt = terpld(matFD_TempCond, (TDT_i + TDT_p) / 2.0, 1, 2); // 1: Temperature, 2: Thermal conductivity
                    } else {
                        kt = mat->Conductivity;      // 20C base conductivity
                        Real64 const kt1(matFD.tk1); // linear coefficient (normally zero)
                        if (kt1 != 0.0) {
                            kt = +kt1 * ((TDT_i + TDT_p) / 2.0 - 20.0);
                        }
                    }

                    // Check for phase change material
                    Real64 const TD_i(TD(i));
                    Real64 const Cpo(mat->SpecHeat); // Specific heat from idf
                    Real64 Cp(Cpo);                  // Specific heat modified if PCM, otherwise equal to Cpo // Will be changed if PCM
                    auto const &matFD_TempEnth(matFD.TempEnth);
                    assert(matFD_TempEnth.u2() >= 3);
                    Real64 const lTE(matFD_TempEnth.index(2, 1));
                    Real64 RhoS(mat->Density);
                    if (mat->hasPCM) {
                        auto *matPC = dynamic_cast<Material::MaterialPhaseChange *>(mat);
                        adjustPropertiesForPhaseChange(state, i, Surf, matPC, TD_i, TDT_i, Cp, RhoS, kt);
                        s_hbfd->SurfaceFD(Surf).EnthalpyF = matPC->enthalpyF;
                        s_hbfd->SurfaceFD(Surf).EnthalpyM = matPC->enthalpyM;
                    } else if (matFD_TempEnth[lTE] + matFD_TempEnth[lTE + 1] + matFD_TempEnth[lTE + 2] >=
                               0.0) { // Phase change material: Use TempEnth data to generate Cp
                        // Enthalpy function used to get average specific heat. Updated by GS so enthalpy function is followed.
                        EnthOld(i) = terpld(matFD_TempEnth, TD_i, 1, 2);  // 1: Temperature, 2: Enthalpy
                        EnthNew(i) = terpld(matFD_TempEnth, TDT_i, 1, 2); // 1: Temperature, 2: Enthalpy
                        if (EnthNew(i) != EnthOld(i)) {
                            Cp = max(Cpo, (EnthNew(i) - EnthOld(i)) / (TDT_i - TD_i));
                        }
                    } // Phase Change Material option

                    // EMS Conductivity Override
                    if (condActuator.isActuated) {
                        kt = condActuator.actuatedValue;
                    }

                    // EMS Specific Heat Override
                    if (specHeatActuator.isActuated) {
                        Cp = specHeatActuator.actuatedValue;
                    }

                    // Update EMS internal variables
                    s_hbfd->SurfaceFD(Surf).condNodeReport(i) = kt;
                    s_hbfd->SurfaceFD(Surf).specHeatNodeReport(i) = Cp;

                    // Choose Regular or Transparent Insulation Case
                    Real64 const DelX = s_hbfd->ConstructFD(ConstrNum).DelX(Lay);
                    Real64 const Delt_DelX(Delt * DelX);
                    s_hbfd->SurfaceFD(Surf).CpDelXRhoS1(i) = 0.0;                      // Outside face  does not have an outer half node
                    s_hbfd->SurfaceFD(Surf).CpDelXRhoS2(i) = (Cp * DelX * RhoS) / 2.0; // Save this for computing node flux values

                    if (HMovInsul <= 0.0) { // Regular  case

                        if (s_hbfd->CondFDSchemeType == CondFDScheme::CrankNicholsonSecondOrder) { // Second Order equation
                            Real64 const Cp_DelX_RhoS_2Delt(Cp * DelX * RhoS / (2.0 * Delt));
                            Real64 const kt_2DelX(kt / (2.0 * DelX));
                            Real64 const hsum(0.5 * (hconvo + hgnd + hrad + hsky + hsurr));
                            TDT_i = (QRadSWOutFD + Cp_DelX_RhoS_2Delt * TD_i + kt_2DelX * (TDT_p - TD_i + TD(i + 1)) + hgnd * Tgnd +
                                     (hconvo + hrad) * Toa + hsky * Tsky + hsurr * Tsurr - hsum * TD_i) /
                                    (hsum + kt_2DelX + Cp_DelX_RhoS_2Delt);
                        } else if (s_hbfd->CondFDSchemeType == CondFDScheme::FullyImplicitFirstOrder) { // First Order
                            Real64 const Two_Delt_DelX(2.0 * Delt_DelX);
                            Real64 const Cp_DelX2_RhoS(Cp * pow_2(DelX) * RhoS);
                            Real64 const Two_Delt_kt(2.0 * Delt * kt);
                            TDT_i = (Two_Delt_DelX * (QRadSWOutFD + hgnd * Tgnd + (hconvo + hrad) * Toa + hsky * Tsky + hsurr * Tsurr) +
                                     Cp_DelX2_RhoS * TD_i + Two_Delt_kt * TDT_p) /
                                    (Two_Delt_DelX * (hconvo + hgnd + hrad + hsky + hsurr) + Two_Delt_kt + Cp_DelX2_RhoS);
                        }

                    } else { // HMovInsul > 0.0: Transparent insulation on outside
                        // Transparent insulation additions

                        // Movable Insulation Layer Outside surface temp

                        Real64 const TInsulOut(
                            (QRadSWOutMvInsulFD + hgnd * Tgnd + HMovInsul * TDT_i + (hconvo + hrad) * Toa + hsky * Tsky + hsurr * Tsurr) /
                            (hconvo + hgnd + HMovInsul + hrad + hsky + hsurr)); // Temperature of outside face of Outside Insulation
                        Real64 const Two_Delt_DelX(2.0 * Delt_DelX);
                        Real64 const Cp_DelX2_RhoS(Cp * pow_2(DelX) * RhoS);
                        Real64 const Two_Delt_kt(2.0 * Delt * kt);

                        // Wall first node temperature behind Movable insulation
                        if (s_hbfd->CondFDSchemeType == CondFDScheme::CrankNicholsonSecondOrder) {
                            TDT_i = (Two_Delt_DelX * (QRadSWOutFD + HMovInsul * TInsulOut) + Cp_DelX2_RhoS * TD_i + Two_Delt_kt * TDT_p) /
                                    (Two_Delt_DelX * HMovInsul + Two_Delt_kt + Cp_DelX2_RhoS);
                        } else if (s_hbfd->CondFDSchemeType == CondFDScheme::FullyImplicitFirstOrder) {
                            // Currently same as Crank Nicholson, need fully implicit formulation
                            TDT_i = (Two_Delt_DelX * (QRadSWOutFD + HMovInsul * TInsulOut) + Cp_DelX2_RhoS * TD_i + Two_Delt_kt * TDT_p) /
                                    (Two_Delt_DelX * HMovInsul + Two_Delt_kt + Cp_DelX2_RhoS);
                        } else {
                            assert(false); // Illegal CondFDSchemeType
                        }

                    } // Regular layer or Movable insulation cases

                } // R layer or Regular layer

                CheckFDNodeTempLimits(state, Surf, i, TDT_i);

                TDT(i) = TDT_i;

            } // regular detailed FD part or SigmaR SigmaC part

            // Determine net heat flux to outside face
            // One formulation that works for Fully Implicit and CrankNicholson and massless wall

            Real64 const Toa_TDT_i(Toa - TDT_i);
            Real64 const QNetSurfFromOutside(
                QRadSWOutFD + (hgnd * (-TDT_i + Tgnd) + (hconvo + hrad) * Toa_TDT_i + hsky * (-TDT_i + Tsky) + hsurr * (-TDT_i + Tsurr)));

            // Same sign convention as CTFs
            state.dataHeatBalSurf->SurfOpaqOutFaceCondFlux(Surf) = -QNetSurfFromOutside;

            // Report all outside BC heat fluxes
            state.dataHeatBalSurf->SurfQdotRadOutRepPerArea(Surf) =
                -(hgnd * (TDT_i - Tgnd) + hrad * (-Toa_TDT_i) + hsky * (TDT_i - Tsky) + hsurr * (TDT_i - Tsurr));
            state.dataHeatBalSurf->SurfQdotRadOutRep(Surf) = surface.Area * state.dataHeatBalSurf->SurfQdotRadOutRepPerArea(Surf);
            state.dataHeatBalSurf->SurfQRadOutReport(Surf) = state.dataHeatBalSurf->SurfQdotRadOutRep(Surf) * state.dataGlobal->TimeStepZoneSec;

        } // regular BC part of the ground and Rain check
    }

    void InteriorNodeEqns(EnergyPlusData &state,
                          int const Delt,                               // Time Increment
                          int const i,                                  // Node Index
                          int const Lay,                                // Layer Number for Construction
                          int const Surf,                               // Surface number
                          [[maybe_unused]] Array1D<Real64> const &T,    // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                          [[maybe_unused]] Array1D<Real64> &TT,         // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                          [[maybe_unused]] Array1D<Real64> const &Rhov, // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                          [[maybe_unused]] Array1D<Real64> &RhoT,       // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                          [[maybe_unused]] Array1D<Real64> &RH,         // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                          Array1D<Real64> const &TD,                    // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                          Array1D<Real64> &TDT,                         // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                          Array1D<Real64> &EnthOld,                     // Old Nodal enthalpy
                          Array1D<Real64> &EnthNew                      // New Nodal enthalpy
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   November, 2003
        //       MODIFIED       May 2011, B. Griffith and P. Tabares
        //       RE-ENGINEERED  C. O. Pedersen, 2006

        int const ConstrNum(state.dataSurface->Surface(Surf).Construction);

        auto &s_hbfd = state.dataHeatBalFiniteDiffMgr;

        int const MatLay(state.dataConstruction->Construct(ConstrNum).LayerPoint(Lay));
        auto *mat = state.dataMaterial->materials(MatLay);
        auto const &matFD = s_hbfd->MaterialFD(MatLay);
        auto const &condActuator = s_hbfd->SurfaceFD(Surf).condMaterialActuators(Lay);
        auto const &specHeatActuator = s_hbfd->SurfaceFD(Surf).specHeatMaterialActuators(Lay);

        Real64 const TD_i(TD(i));

        Real64 const TDT_m(TDT(i - 1));
        Real64 TDT_i(TDT(i));
        Real64 const TDT_p(TDT(i + 1));
        Real64 const TDT_mi((TDT_m + TDT_i) / 2.0);
        Real64 const TDT_ip((TDT_i + TDT_p) / 2.0);

        //  Set Thermal Conductivity.  Can be constant, simple linear temp dep or multiple linear segment temp function dep.
        auto const &matFD_TempCond(matFD.TempCond);
        assert(matFD_TempCond.u2() >= 3);
        Real64 const lTC(matFD_TempCond.index(2, 1));
        Real64 ktA1; // Variable Outer Thermal conductivity in temperature equation
        Real64 ktA2; // Thermal Inner conductivity in temperature equation
        if (matFD_TempCond[lTC] + matFD_TempCond[lTC + 1] + matFD_TempCond[lTC + 2] >= 0.0) { // Multiple Linear Segment Function
            ktA1 = terpld(matFD.TempCond, TDT_ip, 1, 2);                                      // 1: Temperature, 2: Thermal conductivity
            ktA2 = terpld(matFD.TempCond, TDT_mi, 1, 2);                                      // 1: Temperature, 2: Thermal conductivity
        } else {
            ktA1 = ktA2 = mat->Conductivity; // 20C base conductivity
            Real64 const kt1(matFD.tk1);     // temperature coefficient for simple temp dep k. // linear coefficient (normally zero)
            if (kt1 != 0.0) {
                ktA1 += kt1 * (TDT_ip - 20.0);
                ktA2 += kt1 * (TDT_mi - 20.0);
            }
        }

        Real64 const Cpo(mat->SpecHeat); // Const Cp from input
        Real64 Cp(Cpo);                  // Cp used // Will be changed if PCM
        Real64 kt(0.0);
        auto const &matFD_TempEnth(matFD.TempEnth);
        assert(matFD_TempEnth.u2() >= 3);
        Real64 const lTE(matFD_TempEnth.index(2, 1));
        Real64 RhoS(mat->Density);
        if (mat->hasPCM) {
            auto *matPC = dynamic_cast<Material::MaterialPhaseChange *>(mat);
            adjustPropertiesForPhaseChange(state, i, Surf, matPC, TD_i, TDT_i, Cp, RhoS, kt);
            ktA1 = matPC->getConductivity(TDT_ip);
            ktA2 = matPC->getConductivity(TDT_mi);
        } else if (matFD_TempEnth[lTE] + matFD_TempEnth[lTE + 1] + matFD_TempEnth[lTE + 2] >= 0.0) { // Phase change material: Use TempEnth data
            EnthOld(i) = terpld(matFD_TempEnth, TD_i, 1, 2);                                         // 1: Temperature, 2: Enthalpy
            EnthNew(i) = terpld(matFD_TempEnth, TDT_i, 1, 2);                                        // 1: Temperature, 2: Enthalpy
            if (EnthNew(i) != EnthOld(i)) {
                Cp = max(Cpo, (EnthNew(i) - EnthOld(i)) / (TDT_i - TD_i));
            }
        } // Phase Change case

        // EMS Conductivity Override
        if (condActuator.isActuated) {
            kt = condActuator.actuatedValue;
            ktA1 = kt;
            ktA2 = kt;
        }

        // EMS Specific Heat Override
        if (specHeatActuator.isActuated) {
            Cp = specHeatActuator.actuatedValue;
        }

        // Update EMS internal variables
        s_hbfd->SurfaceFD(Surf).condNodeReport(i) = kt;
        s_hbfd->SurfaceFD(Surf).specHeatNodeReport(i) = Cp;

        Real64 const DelX(s_hbfd->ConstructFD(ConstrNum).DelX(Lay));
        Real64 const Cp_DelX_RhoS_Delt(Cp * DelX * RhoS / Delt);

        switch (s_hbfd->CondFDSchemeType) {
        case CondFDScheme::CrankNicholsonSecondOrder: { // Adams-Moulton second order
            Real64 const inv2DelX(1.0 / (2.0 * DelX));
            TDT_i = ((Cp_DelX_RhoS_Delt * TD_i) + ((ktA1 * (TD(i + 1) - TD_i + TDT_p) + ktA2 * (TD(i - 1) - TD_i + TDT_m)) * inv2DelX)) /
                    (((ktA1 + ktA2) * inv2DelX) + Cp_DelX_RhoS_Delt);
        } break;
        case CondFDScheme::FullyImplicitFirstOrder: { // Adams-Moulton First order
            Real64 const invDelX(1.0 / DelX);
            TDT_i = ((Cp_DelX_RhoS_Delt * TD_i) + ((ktA2 * TDT_m) + (ktA1 * TDT_p)) * invDelX) / (((ktA1 + ktA2) * invDelX) + Cp_DelX_RhoS_Delt);
        } break;
        default:
            assert(false); // Illegal CondFDSchemeType
        }

        CheckFDNodeTempLimits(state, Surf, i, TDT_i);

        TDT(i) = TDT_i;
        s_hbfd->SurfaceFD(Surf).CpDelXRhoS1(i) = s_hbfd->SurfaceFD(Surf).CpDelXRhoS2(i) =
            (Cp * DelX * RhoS) / 2.0; // Save this for computing node flux values, half nodes are the same here
    }

    void IntInterfaceNodeEqns(EnergyPlusData &state,
                              int const Delt,                                  // Time Increment
                              int const i,                                     // Node Index
                              int const Lay,                                   // Layer Number for Construction
                              int const SurfNum,                               // Surface number
                              [[maybe_unused]] Array1D<Real64> const &T,       // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                              [[maybe_unused]] Array1D<Real64> &TT,            // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                              [[maybe_unused]] Array1D<Real64> const &Rhov,    // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                              [[maybe_unused]] Array1D<Real64> &RhoT,          // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                              [[maybe_unused]] Array1D<Real64> &RH,            // RELATIVE HUMIDITY.
                              Array1D<Real64> const &TD,                       // OLD NODE TEMPERATURES OF EACH HEAT TRANSFER SURF IN CONDFD.
                              Array1D<Real64> &TDT,                            // NEW NODE TEMPERATURES OF EACH HEAT TRANSFER SURF IN CONDFD.
                              [[maybe_unused]] Array1D<Real64> const &EnthOld, // Old Nodal enthalpy
                              Array1D<Real64> &EnthNew,                        // New Nodal enthalpy
                              [[maybe_unused]] int const GSiter                // Iteration number of Gauss Seidel iteration
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   November, 2003
        //       MODIFIED       May 2011, B. Griffith, P. Tabares,  add first order fully implicit, bug fixes, cleanup
        //       RE-ENGINEERED  Curtis Pedersen, Changed to Implicit mode and included enthalpy.  FY2006

        // PURPOSE OF THIS SUBROUTINE:
        // calculate finite difference heat transfer for nodes that interface two different material layers inside construction

        auto &s_hbfd = state.dataHeatBalFiniteDiffMgr;
        auto const &surface = state.dataSurface->Surface(SurfNum);
        auto &surfFD = s_hbfd->SurfaceFD(SurfNum);

        if (surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CondFD) { // HT Algo issue

            int const ConstrNum(surface.Construction);
            auto const &construct(state.dataConstruction->Construct(ConstrNum));

            int const MatLay(construct.LayerPoint(Lay));
            auto *mat = state.dataMaterial->materials(MatLay);

            int const MatLay2(construct.LayerPoint(Lay + 1));
            auto *mat2 = state.dataMaterial->materials(MatLay2);

            auto const &condActuator1 = surfFD.condMaterialActuators(Lay);
            auto const &condActuator2 = surfFD.condMaterialActuators(Lay + 1);

            auto const &specHeatActuator1 = surfFD.specHeatMaterialActuators(Lay);
            auto const &specHeatActuator2 = surfFD.specHeatMaterialActuators(Lay + 1);

            auto const &heatFluxActuator = surfFD.heatSourceFluxMaterialActuators(Lay);

            Real64 const TDT_m(TDT(i - 1));
            Real64 const TDT_p(TDT(i + 1));

            bool const RLayerPresent(mat->ROnly || mat->group == Material::Group::AirGap);
            bool const RLayer2Present(mat2->ROnly || mat2->group == Material::Group::AirGap);

            Real64 const Rlayer(mat->Resistance);   // Resistance value of R Layer
            Real64 const Rlayer2(mat2->Resistance); // Resistance value of next layer to inside

            if (RLayerPresent && RLayer2Present) {

                TDT(i) = (Rlayer2 * TDT_m + Rlayer * TDT_p) / (Rlayer + Rlayer2); // Two adjacent R layers

            } else {

                auto const &matFD(s_hbfd->MaterialFD(MatLay));
                auto const &matFD2(s_hbfd->MaterialFD(MatLay2));
                Real64 TDT_i(TDT(i));

                // Set Thermal Conductivity. Can be constant, simple linear temp dep or multiple linear segment temp function dep.

                Real64 kt1(0.0);
                if (!RLayerPresent) {
                    auto const &matFD_TempCond(matFD.TempCond);
                    assert(matFD_TempCond.u2() >= 3);
                    Real64 const lTC(matFD_TempCond.index(2, 1));
                    if (matFD_TempCond[lTC] + matFD_TempCond[lTC + 1] + matFD_TempCond[lTC + 2] >= 0.0) { // Multiple Linear Segment Function
                        kt1 = terpld(matFD.TempCond, (TDT_i + TDT_m) / 2.0, 1, 2);                        // 1: Temperature, 2: Thermal conductivity
                    } else {
                        kt1 = mat->Conductivity;      // 20C base conductivity
                        Real64 const kt11(matFD.tk1); // temperature coefficient for simple temp dep k. // linear coefficient (normally zero)
                        if (kt11 != 0.0) {
                            kt1 += kt11 * ((TDT_i + TDT_m) / 2.0 - 20.0);
                        }
                    }
                }

                Real64 kt2(0.0);
                if (!RLayer2Present) {
                    auto const &matFD2_TempCond(matFD2.TempCond);
                    assert(matFD2_TempCond.u2() >= 3);
                    Real64 const lTC2(matFD2_TempCond.index(2, 1));
                    if (matFD2_TempCond[lTC2] + matFD2_TempCond[lTC2 + 1] + matFD2_TempCond[lTC2 + 2] >= 0.0) { // Multiple Linear Segment Function
                        kt2 = terpld(matFD2_TempCond, (TDT_i + TDT_p) / 2.0, 1, 2); // 1: Temperature, 2: Thermal conductivity
                    } else {
                        kt2 = mat2->Conductivity;      // 20C base conductivity
                        Real64 const kt21(matFD2.tk1); // temperature coefficient for simple temp dep k. // linear coefficient (normally zero)
                        if (kt21 != 0.0) {
                            kt2 += kt21 * ((TDT_i + TDT_p) / 2.0 - 20.0);
                        }
                    }
                }

                Real64 RhoS1(mat->Density);
                Real64 const Cpo1(mat->SpecHeat); // constant Cp from input file
                Real64 Cp1(Cpo1);                 // Will be reset if PCM
                Real64 const Delx1(s_hbfd->ConstructFD(ConstrNum).DelX(Lay));

                Real64 RhoS2(mat2->Density);
                Real64 const Cpo2(mat2->SpecHeat);
                Real64 Cp2(Cpo2); // will be reset if PCM
                Real64 const Delx2(s_hbfd->ConstructFD(ConstrNum).DelX(Lay + 1));

                // Calculate the Dry Heat Conduction Equation

                // Source/Sink Flux Capability ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                Real64 QSSFlux = 0.0;
                if ((surface.Area > 0.0) && (construct.SourceSinkPresent && Lay == construct.SourceAfterLayer)) {
                    // Source/Sink flux value at a layer interface // Includes QPV Source
                    QSSFlux = (state.dataHeatBalFanSys->QRadSysSource(SurfNum) + state.dataHeatBalFanSys->QPVSysSource(SurfNum)) / surface.Area;
                }

                // update report variables
                auto &surfFD = s_hbfd->SurfaceFD(SurfNum);

                // only includes internal heat source
                surfFD.heatSourceInternalFluxLayerReport(Lay) = QSSFlux * surface.Area;
                surfFD.heatSourceInternalFluxEnergyLayerReport(Lay) = QSSFlux * surface.Area * state.dataGlobal->TimeStepZoneSec;

                // Add EMS actuated value
                if (heatFluxActuator.isActuated) {
                    Real64 actuatedVal = heatFluxActuator.actuatedValue;
                    if (actuatedVal >= 0) {
                        QSSFlux += heatFluxActuator.actuatedValue;
                    } else {
                        ShowSevereError(state, fmt::format("Surface: {}, Material: {}", surface.Name, mat->Name));
                        ShowContinueError(state, "EMS Actuator does not support negative values");
                        ShowFatalError(state, "Program terminates due to preceding conditions.");
                    }

                    // Update report variables
                    // Only includes the EMS values
                    surfFD.heatSourceEMSFluxLayerReport(Lay) = heatFluxActuator.actuatedValue * surface.Area;
                    surfFD.heatSourceEMSFluxEnergyLayerReport(Lay) =
                        heatFluxActuator.actuatedValue * surface.Area * state.dataGlobal->TimeStepZoneSec;
                }

                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                Real64 const TD_i(TD(i));

                auto const &matFD_TempEnth(matFD.TempEnth);
                assert(matFD_TempEnth.u2() >= 3);
                Real64 const lTE(matFD_TempEnth.index(2, 1));
                Real64 const matFD_sum(matFD_TempEnth[lTE] + matFD_TempEnth[lTE + 1] + matFD_TempEnth[lTE + 2]);

                auto const &matFD2_TempEnth(matFD2.TempEnth);
                assert(matFD2_TempEnth.u2() >= 3);
                Real64 const lTE2(matFD2_TempEnth.index(2, 1));
                Real64 const matFD2_sum(matFD2_TempEnth[lTE2] + matFD2_TempEnth[lTE2 + 1] + matFD2_TempEnth[lTE2 + 2]);

                if (RLayerPresent && !RLayer2Present) { // R-layer first

                    // Check for PCM second layer
                    if (mat2->hasPCM) {
                        auto *matPC2 = dynamic_cast<Material::MaterialPhaseChange *>(mat);
                        assert(matPC2 != nullptr);
                        adjustPropertiesForPhaseChange(state, i, SurfNum, matPC2, TD_i, TDT_i, Cp2, RhoS2, kt2);
                    } else if ((matFD_sum < 0.0) && (matFD2_sum > 0.0)) {            // Phase change material Layer2, Use TempEnth Data
                        Real64 const Enth2Old(terpld(matFD2_TempEnth, TD_i, 1, 2));  // 1: Temperature, 2: Thermal conductivity
                        Real64 const Enth2New(terpld(matFD2_TempEnth, TDT_i, 1, 2)); // 1: Temperature, 2: Thermal conductivity
                        EnthNew(i) = Enth2New; // This node really doesn't have an enthalpy, this gives it a value
                        if ((std::abs(Enth2New - Enth2Old) > smalldiff) && (std::abs(TDT_i - TD_i) > smalldiff)) {
                            Cp2 = max(Cpo2, (Enth2New - Enth2Old) / (TDT_i - TD_i));
                        }
                    }

                    // EMS Conductivity 2 Override
                    if (condActuator2.isActuated) {
                        kt2 = condActuator1.actuatedValue;
                    }

                    // EMS Specific Heat 2 Override
                    if (specHeatActuator2.isActuated) {
                        Cp2 = specHeatActuator1.actuatedValue;
                    }

                    // Update EMS internal variables
                    surfFD.condNodeReport(i) = kt1;
                    surfFD.specHeatNodeReport(i) = Cp1;
                    surfFD.condNodeReport(i + 1) = kt2;
                    surfFD.specHeatNodeReport(i + 1) = Cp2;

                    // R layer first, then PCM or regular layer
                    Real64 const Delt_Delx2(Delt * Delx2);
                    Real64 const Cp2_fac(Cp2 * pow_2(Delx2) * RhoS2 * Rlayer);
                    Real64 const Delt_kt2_Rlayer(Delt * kt2 * Rlayer);
                    if (s_hbfd->CondFDSchemeType == CondFDScheme::CrankNicholsonSecondOrder) {
                        TDT_i = (2.0 * Delt_Delx2 * QSSFlux * Rlayer + (Cp2_fac - Delt_Delx2 - Delt_kt2_Rlayer) * TD_i +
                                 Delt_Delx2 * (TD(i - 1) + TDT_m) + Delt_kt2_Rlayer * (TD(i + 1) + TDT_p)) /
                                (Delt_Delx2 + Delt_kt2_Rlayer + Cp2_fac);
                    } else if (s_hbfd->CondFDSchemeType == CondFDScheme::FullyImplicitFirstOrder) {
                        Real64 const Two_Delt_Delx2(2.0 * Delt_Delx2);
                        Real64 const Two_Delt_kt2_Rlayer(2.0 * Delt_kt2_Rlayer);
                        TDT_i = (Two_Delt_Delx2 * (QSSFlux * Rlayer + TDT_m) + Cp2_fac * TD_i + Two_Delt_kt2_Rlayer * TDT_p) /
                                (Two_Delt_Delx2 + Two_Delt_kt2_Rlayer + Cp2_fac);
                    }

                    CheckFDNodeTempLimits(state, SurfNum, i, TDT_i);

                    surfFD.CpDelXRhoS1(i) = 0.0;                         //  - rlayer has no capacitance, so this is zero
                    surfFD.CpDelXRhoS2(i) = (Cp2 * Delx2 * RhoS2) / 2.0; // Save this for computing node flux values

                } else if (!RLayerPresent && RLayer2Present) { // R-layer second

                    // Check for PCM layer before R layer
                    if (mat->hasPCM) {
                        auto *matPC = dynamic_cast<Material::MaterialPhaseChange *>(mat);
                        adjustPropertiesForPhaseChange(state, i, SurfNum, matPC, TD_i, TDT_i, Cp1, RhoS1, kt1);
                    } else if ((matFD_sum > 0.0) && (matFD2_sum < 0.0)) {           // Phase change material Layer1, Use TempEnth Data
                        Real64 const Enth1Old(terpld(matFD_TempEnth, TD_i, 1, 2));  // 1: Temperature, 2: Thermal conductivity
                        Real64 const Enth1New(terpld(matFD_TempEnth, TDT_i, 1, 2)); // 1: Temperature, 2: Thermal conductivity
                        EnthNew(i) = Enth1New; // This node really doesn't have an enthalpy, this gives it a value
                        if ((std::abs(Enth1New - Enth1Old) > smalldiff) && (std::abs(TDT_i - TD_i) > smalldiff)) {
                            Cp1 = max(Cpo1, (Enth1New - Enth1Old) / (TDT_i - TD_i));
                        }
                    }

                    // EMS Conductivity 1 Override
                    if (condActuator1.isActuated) {
                        kt1 = condActuator1.actuatedValue;
                    }

                    // EMS Specific Heat 1 Override
                    if (specHeatActuator1.isActuated) {
                        Cp1 = specHeatActuator1.actuatedValue;
                    }

                    // Update EMS internal variables
                    surfFD.condNodeReport(i) = kt1;
                    surfFD.specHeatNodeReport(i) = Cp1;
                    surfFD.condNodeReport(i + 1) = kt2;
                    surfFD.specHeatNodeReport(i + 1) = Cp2;

                    Real64 const Delt_Delx1(Delt * Delx1);
                    Real64 const Cp1_fac(Cp1 * pow_2(Delx1) * RhoS1 * Rlayer2);
                    Real64 const Delt_kt1_Rlayer2(Delt * kt1 * Rlayer2);
                    if (s_hbfd->CondFDSchemeType == CondFDScheme::CrankNicholsonSecondOrder) {
                        TDT_i = (2.0 * Delt_Delx1 * QSSFlux * Rlayer2 + (Cp1_fac - Delt_Delx1 - Delt_kt1_Rlayer2) * TD_i +
                                 Delt_Delx1 * (TD(i + 1) + TDT_p) + Delt_kt1_Rlayer2 * (TD(i - 1) + TDT_m)) /
                                (Delt_Delx1 + Delt_kt1_Rlayer2 + Cp1_fac);
                    } else if (s_hbfd->CondFDSchemeType == CondFDScheme::FullyImplicitFirstOrder) {
                        Real64 const Two_Delt_Delx1(2.0 * Delt_Delx1);
                        Real64 const Two_Delt_kt1_Rlayer2(2.0 * Delt_kt1_Rlayer2);
                        TDT_i = (Two_Delt_Delx1 * (QSSFlux * Rlayer2 + TDT_p) + Cp1_fac * TD_i + Two_Delt_kt1_Rlayer2 * TDT_m) /
                                (Two_Delt_Delx1 + Two_Delt_kt1_Rlayer2 + Cp1_fac);
                    }

                    CheckFDNodeTempLimits(state, SurfNum, i, TDT_i);

                    surfFD.CpDelXRhoS1(i) = (Cp1 * Delx1 * RhoS1) / 2.0; // Save this for computing node flux values
                    surfFD.CpDelXRhoS2(i) = 0.0;                         //  - rlayer has no capacitance, so this is zero

                } else { // Regular or Phase Change on both sides of interface

                    // Consider the various PCM material location cases
                    if ((matFD_sum > 0.0) && (matFD2_sum > 0.0)) { // Phase change material both layers, Use TempEnth Data

                        Real64 const Enth1Old(terpld(matFD_TempEnth, TD_i, 1, 2));   // 1: Temperature, 2: Thermal conductivity
                        Real64 const Enth2Old(terpld(matFD2_TempEnth, TD_i, 1, 2));  // 1: Temperature, 2: Thermal conductivity
                        Real64 const Enth1New(terpld(matFD_TempEnth, TDT_i, 1, 2));  // 1: Temperature, 2: Thermal conductivity
                        Real64 const Enth2New(terpld(matFD2_TempEnth, TDT_i, 1, 2)); // 1: Temperature, 2: Thermal conductivity

                        EnthNew(i) = Enth1New; // This node really doesn't have an enthalpy, this gives it a value

                        if ((std::abs(Enth1New - Enth1Old) > smalldiff) && (std::abs(TDT_i - TD_i) > smalldiff)) {
                            Cp1 = max(Cpo1, (Enth1New - Enth1Old) / (TDT_i - TD_i));
                        }

                        if ((std::abs(Enth2New - Enth2Old) > smalldiff) && (std::abs(TDT_i - TD_i) > smalldiff)) {
                            Cp2 = max(Cpo2, (Enth2New - Enth2Old) / (TDT_i - TD_i));
                        }

                        // if

                    } else if ((matFD_sum > 0.0) && (matFD2_sum < 0.0)) { // Phase change material Layer1, Use TempEnth Data

                        Real64 const Enth1Old(terpld(matFD_TempEnth, TD_i, 1, 2));  // 1: Temperature, 2: Thermal conductivity
                        Real64 const Enth1New(terpld(matFD_TempEnth, TDT_i, 1, 2)); // 1: Temperature, 2: Thermal conductivity
                        EnthNew(i) = Enth1New; // This node really doesn't have an enthalpy, this gives it a value

                        if ((std::abs(Enth1New - Enth1Old) > smalldiff) && (std::abs(TDT_i - TD_i) > smalldiff)) {
                            Cp1 = max(Cpo1, (Enth1New - Enth1Old) / (TDT_i - TD_i));
                        }

                    } else if ((matFD_sum < 0.0) && (matFD2_sum > 0.0)) { // Phase change material Layer2, Use TempEnth Data

                        Real64 const Enth2Old(terpld(matFD2_TempEnth, TD_i, 1, 2));  // 1: Temperature, 2: Thermal conductivity
                        Real64 const Enth2New(terpld(matFD2_TempEnth, TDT_i, 1, 2)); // 1: Temperature, 2: Thermal conductivity
                        EnthNew(i) = Enth2New; // This node really doesn't have an enthalpy, this gives it a value

                        if ((std::abs(Enth2New - Enth2Old) > smalldiff) && (std::abs(TDT_i - TD_i) > smalldiff)) {
                            Cp2 = max(Cpo2, (Enth2New - Enth2Old) / (TDT_i - TD_i));
                        }

                    } // Phase change material check

                    if (mat->hasPCM) {
                        auto *matPC = dynamic_cast<Material::MaterialPhaseChange *>(mat);
                        adjustPropertiesForPhaseChange(state, i, SurfNum, matPC, TD_i, TDT_i, Cp1, RhoS1, kt1);
                    }
                    if (mat2->hasPCM) {
                        auto *matPC2 = dynamic_cast<Material::MaterialPhaseChange *>(mat2);
                        adjustPropertiesForPhaseChange(state, i, SurfNum, matPC2, TD_i, TDT_i, Cp2, RhoS2, kt2);
                    }

                    // EMS Conductivity 1 Override
                    if (condActuator1.isActuated) {
                        kt1 = condActuator1.actuatedValue;
                    }

                    // EMS Conductivity 2 Override
                    if (condActuator2.isActuated) {
                        kt2 = condActuator2.actuatedValue;
                    }

                    // EMS Specific Heat 1 Override
                    if (specHeatActuator1.isActuated) {
                        Cp1 = specHeatActuator1.actuatedValue;
                    }

                    // EMS Specific Heat 2 Override
                    if (specHeatActuator2.isActuated) {
                        Cp2 = specHeatActuator2.actuatedValue;
                    }

                    // Update EMS internal variables
                    surfFD.condNodeReport(i) = kt1;
                    surfFD.specHeatNodeReport(i) = Cp1;
                    surfFD.condNodeReport(i + 1) = kt2;
                    surfFD.specHeatNodeReport(i + 1) = Cp2;

                    Real64 const Delt_Delx1(Delt * Delx1);
                    Real64 const Delt_Delx2(Delt * Delx2);
                    Real64 const Delt_Delx1_kt2(Delt_Delx1 * kt2);
                    Real64 const Delt_Delx2_kt1(Delt_Delx2 * kt1);
                    Real64 const Delt_sum(Delt_Delx1_kt2 + Delt_Delx2_kt1);
                    Real64 const Cp1_fac(Cp1 * pow_2(Delx1) * Delx2 * RhoS1);
                    Real64 const Cp2_fac(Cp2 * Delx1 * pow_2(Delx2) * RhoS2);
                    Real64 const Cp_fac(Cp1_fac + Cp2_fac);
                    if (s_hbfd->CondFDSchemeType ==
                        CondFDScheme::CrankNicholsonSecondOrder) { // Regular Internal Interface Node with Source/sink using Adams Moulton second
                        // order
                        TDT_i = (2.0 * Delt_Delx1 * Delx2 * QSSFlux + (Cp_fac - Delt_sum) * TD_i + Delt_Delx1_kt2 * (TD(i + 1) + TDT_p) +
                                 Delt_Delx2_kt1 * (TD(i - 1) + TDT_m)) /
                                (Delt_sum + Cp_fac);
                    } else if (s_hbfd->CondFDSchemeType == CondFDScheme::FullyImplicitFirstOrder) { // First order adams moulton
                        TDT_i = (2.0 * (Delt_Delx1 * Delx2 * QSSFlux + Delt_Delx2_kt1 * TDT_m + Delt_Delx1_kt2 * TDT_p) + Cp_fac * TD_i) /
                                (2.0 * (Delt_Delx2_kt1 + Delt_Delx1_kt2) + Cp_fac);
                    }

                    CheckFDNodeTempLimits(state, SurfNum, i, TDT_i);

                    surfFD.CpDelXRhoS1(i) = (Cp1 * Delx1 * RhoS1) / 2.0; // Save this for computing node flux values
                    surfFD.CpDelXRhoS2(i) = (Cp2 * Delx2 * RhoS2) / 2.0; // Save this for computing node flux values

                    if (construct.SourceSinkPresent && (Lay == construct.SourceAfterLayer)) {
                        state.dataHeatBalFanSys->TCondFDSourceNode(SurfNum) = TDT_i; // Transfer node temp to Radiant System
                        state.dataHeatBalSurf->SurfTempSource(SurfNum) = TDT_i;      // Transfer node temp to DataHeatBalSurface module
                        surfFD.QSource = QSSFlux;
                        surfFD.SourceNodeNum = i;
                    }

                    if (construct.SourceSinkPresent && (Lay == construct.TempAfterLayer)) {
                        state.dataHeatBalSurf->SurfTempUserLoc(SurfNum) = TDT_i; // Transfer node temp to DataHeatBalSurface module
                    }

                } // End of R-layer and Regular check

                TDT(i) = TDT_i;
            }

        } // End of the CondFD if block
    }

    void InteriorBCEqns(EnergyPlusData &state,
                        int const Delt,                               // Time Increment
                        int const i,                                  // Node Index
                        int const Lay,                                // Layer Number for Construction
                        int const SurfNum,                            // Surface number
                        [[maybe_unused]] Array1D<Real64> const &T,    // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF (Old).
                        [[maybe_unused]] Array1D<Real64> &TT,         // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF (New).
                        [[maybe_unused]] Array1D<Real64> const &Rhov, // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                        [[maybe_unused]] Array1D<Real64> &RhoT,       // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                        [[maybe_unused]] Array1D<Real64> &RH,         // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                        Array1D<Real64> const &TD,                    // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                        Array1D<Real64> &TDT,                         // INSIDE SURFACE TEMPERATURE OF EACH HEAT TRANSFER SURF.
                        Array1D<Real64> &EnthOld,                     // Old Nodal enthalpy
                        Array1D<Real64> &EnthNew,                     // New Nodal enthalpy
                        Array1D<Real64> &TDreport                     // Temperature value from previous HeatSurfaceHeatManager iteration's value
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   November, 2003
        //       MODIFIED       B. Griffith, P. Tabares, May 2011, add first order fully implicit, bug fixes, cleanup
        //                      November 2011 P. Tabares fixed problems with adiabatic walls/massless walls
        //                      November 2011 P. Tabares fixed problems PCM stability problems
        //       RE-ENGINEERED  C. O. Pedersen 2006

        // PURPOSE OF THIS SUBROUTINE:
        // Calculate the heat transfer at the node on the surfaces inside face (facing zone)

        auto &s_hbfd = state.dataHeatBalFiniteDiffMgr;

        auto const &surface(state.dataSurface->Surface(SurfNum));
        auto &surfFD = s_hbfd->SurfaceFD(SurfNum);
        int const ConstrNum(surface.Construction);

        // Set the internal conditions to local variables
        Real64 const NetLWRadToSurfFD(
            state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea(SurfNum)); // Net interior long wavelength radiation to surface from other surfaces
        Real64 const QRadSWInFD(state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(SurfNum)); // Short wave radiation absorbed on inside of opaque surface
        Real64 const SurfQdotRadHVACInPerAreaFD(
            state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(SurfNum));                        // Total current radiant heat flux at a surface
        Real64 const QRadThermInFD(state.dataHeatBal->SurfQdotRadIntGainsInPerArea(SurfNum)); // Thermal radiation absorbed on inside surfaces

        // Boundary Conditions from Simulation for Interior
        Real64 hconvi(state.dataMstBal->HConvInFD(SurfNum));

        Real64 const Tia(state.dataZoneTempPredictorCorrector->zoneHeatBalance(surface.Zone).MAT);

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //    Do all the nodes in the surface   Else will switch to SigmaR,SigmaC
        Real64 TDT_i(TDT(i));
        Real64 const QFac(NetLWRadToSurfFD + QRadSWInFD + QRadThermInFD + SurfQdotRadHVACInPerAreaFD);
        if (surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CondFD) {
            int const MatLay(state.dataConstruction->Construct(ConstrNum).LayerPoint(Lay));
            auto *mat = state.dataMaterial->materials(MatLay);
            auto const &matFD(s_hbfd->MaterialFD(MatLay));
            auto const &condActuator = surfFD.condMaterialActuators(Lay);
            auto const &specHeatActuator = surfFD.specHeatMaterialActuators(Lay);

            // Calculate the Dry Heat Conduction Equation

            if (mat->ROnly || mat->group == Material::Group::AirGap) { // R Layer or Air Layer
                // Use algebraic equation for TDT based on R
                Real64 constexpr IterDampConst(
                    5.0); // Damping constant for inside surface temperature iterations. Only used for massless (R-value only) Walls
                Real64 const Rlayer(mat->Resistance);
                if ((i == 1) && (surface.ExtBoundCond > 0)) { // this is for an adiabatic partition
                    TDT_i = (TDT(i + 1) + (QFac + hconvi * Tia + TDreport(i) * IterDampConst) * Rlayer) / (1.0 + (hconvi + IterDampConst) * Rlayer);
                } else { // regular wall
                    TDT_i = (TDT(i - 1) + (QFac + hconvi * Tia + TDreport(i) * IterDampConst) * Rlayer) / (1.0 + (hconvi + IterDampConst) * Rlayer);
                }
                surfFD.CpDelXRhoS1(i) = 0.0; // Save this for computing node flux values - rlayer has no capacitance
                surfFD.CpDelXRhoS2(i) = 0.0; // Inside face  does not have an inner half node

            } else { //  Regular or PCM
                Real64 const TDT_m(TDT(i - 1));

                // Set Thermal Conductivity. Can be constant, simple linear temp dep or multiple linear segment temp function dep.
                auto const &matFD_TempCond(matFD.TempCond);
                assert(matFD_TempCond.u2() >= 3);
                Real64 const lTC(matFD_TempCond.index(2, 1));
                Real64 kt;
                if (matFD_TempCond[lTC] + matFD_TempCond[lTC + 1] + matFD_TempCond[lTC + 2] >= 0.0) { // Multiple Linear Segment Function
                    // Use average of surface and first node temp for determining k
                    kt = terpld(matFD_TempCond, (TDT_i + TDT_m) / 2.0, 1, 2); // 1: Temperature, 2: Thermal conductivity
                } else {
                    kt = mat->Conductivity;      // 20C base conductivity
                    Real64 const kt1(matFD.tk1); // linear coefficient (normally zero)
                    if (kt1 != 0.0) {
                        kt = +kt1 * ((TDT_i + TDT_m) / 2.0 - 20.0);
                    }
                }

                Real64 RhoS(mat->Density);
                Real64 const TD_i(TD(i));
                Real64 const Cpo(mat->SpecHeat);
                Real64 Cp(Cpo); // Will be changed if PCM
                auto const &matFD_TempEnth(matFD.TempEnth);
                assert(matFD_TempEnth.u2() >= 3);
                Real64 const lTE(matFD_TempEnth.index(2, 1));
                if (mat->hasPCM) {
                    auto *matPC = dynamic_cast<Material::MaterialPhaseChange *>(mat);
                    adjustPropertiesForPhaseChange(state, i, SurfNum, matPC, TD_i, TDT_i, Cp, RhoS, kt);
                } else if (matFD_TempEnth[lTE] + matFD_TempEnth[lTE + 1] + matFD_TempEnth[lTE + 2] >=
                           0.0) {                                     // Phase change material: Use TempEnth data
                    EnthOld(i) = terpld(matFD_TempEnth, TD_i, 1, 2);  // 1: Temperature, 2: Enthalpy
                    EnthNew(i) = terpld(matFD_TempEnth, TDT_i, 1, 2); // 1: Temperature, 2: Enthalpy
                    if ((std::abs(EnthNew(i) - EnthOld(i)) > smalldiff) && (std::abs(TDT_i - TD_i) > smalldiff)) {
                        Cp = max(Cpo, (EnthNew(i) - EnthOld(i)) / (TDT_i - TD_i));
                    }
                } // Phase change material check

                // EMS Conductivity Override
                if (condActuator.isActuated) {
                    kt = condActuator.actuatedValue;
                }

                // EMS Specific Heat Override
                if (specHeatActuator.isActuated) {
                    Cp = specHeatActuator.actuatedValue;
                }

                // Update EMS internal variables
                surfFD.condNodeReport(i) = kt;
                surfFD.specHeatNodeReport(i) = Cp;

                Real64 const DelX(s_hbfd->ConstructFD(ConstrNum).DelX(Lay));
                Real64 const Delt_DelX(Delt * DelX);
                Real64 const Two_Delt_DelX(2.0 * Delt_DelX);
                Real64 const Delt_kt(Delt * kt);
                Real64 const Cp_DelX2_RhoS(Cp * pow_2(DelX) * RhoS);
                if ((surface.ExtBoundCond > 0) && (i == 1)) {                                  // this is for an adiabatic or interzone partition
                    if (s_hbfd->CondFDSchemeType == CondFDScheme::CrankNicholsonSecondOrder) { // Adams-Moulton second order
                        TDT_i = (Two_Delt_DelX * (QFac + hconvi * Tia) + (Cp_DelX2_RhoS - Delt_DelX * hconvi - Delt_kt) * TD_i +
                                 Delt_kt * (TD(i + 1) + TDT(i + 1))) /
                                (Delt_DelX * hconvi + Delt_kt + Cp_DelX2_RhoS);
                    } else if (s_hbfd->CondFDSchemeType == CondFDScheme::FullyImplicitFirstOrder) { // Adams-Moulton First order
                        Real64 const Two_Delt_kt(2.0 * Delt_kt);
                        TDT_i = (Two_Delt_DelX * (QFac + hconvi * Tia) + Cp_DelX2_RhoS * TD_i + Two_Delt_kt * TDT(i + 1)) /
                                (Two_Delt_DelX * hconvi + Two_Delt_kt + Cp_DelX2_RhoS);
                    }
                } else { // for regular or interzone walls
                    if (s_hbfd->CondFDSchemeType == CondFDScheme::CrankNicholsonSecondOrder) {
                        TDT_i = (Two_Delt_DelX * (QFac + hconvi * Tia) + (Cp_DelX2_RhoS - Delt_DelX * hconvi - Delt_kt) * TD_i +
                                 Delt_kt * (TD(i - 1) + TDT_m)) /
                                (Delt_DelX * hconvi + Delt_kt + Cp_DelX2_RhoS);
                    } else if (s_hbfd->CondFDSchemeType == CondFDScheme::FullyImplicitFirstOrder) {
                        Real64 const Two_Delt_kt(2.0 * Delt_kt);
                        TDT_i = (Two_Delt_DelX * (QFac + hconvi * Tia) + Cp_DelX2_RhoS * TD_i + Two_Delt_kt * TDT_m) /
                                (Two_Delt_DelX * hconvi + Two_Delt_kt + Cp_DelX2_RhoS);
                    }
                }
                surfFD.CpDelXRhoS1(i) = (Cp * DelX * RhoS) / 2.0; // Save this for computing node flux values
                surfFD.CpDelXRhoS2(i) = 0.0;                      // Inside face  does not have an inner half node

            } // Regular or R layer

            CheckFDNodeTempLimits(state, SurfNum, i, TDT_i);

            TDT(i) = TDT_i;

        } //  End of Regular node or SigmaR SigmaC option

        Real64 const QNetSurfInside(-(QFac + hconvi * (-TDT_i + Tia)));
        //  Pass inside conduction Flux [W/m2] to DataHeatBalanceSurface array
        state.dataHeatBalSurf->SurfOpaqInsFaceCondFlux(SurfNum) = QNetSurfInside;
    }

    // todo - function not used
    void CheckFDSurfaceTempLimits(EnergyPlusData &state,
                                  int const SurfNum,            // surface number
                                  Real64 const CheckTemperature // calculated temperature, not reset
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   August 2012

        // PURPOSE OF THIS SUBROUTINE:
        // Provides a single entry point for checking surface temperature limits as well as
        // setting up for recurring errors if too low or too high.

        // METHODOLOGY EMPLOYED:
        // Use methodology similar to HBSurfaceManager
        auto &s_hbfd = state.dataHeatBalFiniteDiffMgr;

        int ZoneNum = state.dataSurface->Surface(SurfNum).Zone;

        if (state.dataGlobal->WarmupFlag) {
            ++s_hbfd->WarmupSurfTemp;
        }
        if (!state.dataGlobal->WarmupFlag || s_hbfd->WarmupSurfTemp > 10 || state.dataGlobal->DisplayExtraWarnings) {
            if (CheckTemperature < DataHeatBalSurface::MinSurfaceTempLimit) {
                if (state.dataSurface->SurfLowTempErrCount(SurfNum) == 0) {
                    ShowSevereMessage(state,
                                      format("Temperature (low) out of bounds [{:.2R}] for zone=\"{}\", for surface=\"{}\"",
                                             CheckTemperature,
                                             state.dataHeatBal->Zone(ZoneNum).Name,
                                             state.dataSurface->Surface(SurfNum).Name));
                    ShowContinueErrorTimeStamp(state, "");
                    if (!state.dataHeatBal->Zone(ZoneNum).TempOutOfBoundsReported) {
                        ShowContinueError(state, format("Zone=\"{}\", Diagnostic Details:", state.dataHeatBal->Zone(ZoneNum).Name));
                        if (state.dataHeatBal->Zone(ZoneNum).FloorArea > 0.0) {
                            ShowContinueError(
                                state,
                                format("...Internal Heat Gain [{:.3R}] W/m2",
                                       state.dataHeatBal->Zone(ZoneNum).InternalHeatGains / state.dataHeatBal->Zone(ZoneNum).FloorArea));
                        } else {
                            ShowContinueError(
                                state, format("...Internal Heat Gain (no floor) [{:.3R}] W", state.dataHeatBal->Zone(ZoneNum).InternalHeatGains));
                        }
                        if (state.afn->simulation_control.type == AirflowNetwork::ControlType::NoMultizoneOrDistribution) {
                            ShowContinueError(state,
                                              format("...Infiltration/Ventilation [{:.3R}] m3/s", state.dataHeatBal->Zone(ZoneNum).NominalInfilVent));
                            ShowContinueError(state, format("...Mixing/Cross Mixing [{:.3R}] m3/s", state.dataHeatBal->Zone(ZoneNum).NominalMixing));
                        } else {
                            ShowContinueError(state, "...Airflow Network Simulation: Nominal Infiltration/Ventilation/Mixing not available.");
                        }
                        if (state.dataHeatBal->Zone(ZoneNum).IsControlled) {
                            ShowContinueError(state, "...Zone is part of HVAC controlled system.");
                        } else {
                            ShowContinueError(state, "...Zone is not part of HVAC controlled system.");
                        }
                        state.dataHeatBal->Zone(ZoneNum).TempOutOfBoundsReported = true;
                    }
                    ShowRecurringSevereErrorAtEnd(state,
                                                  "Temperature (low) out of bounds for zone=" + state.dataHeatBal->Zone(ZoneNum).Name +
                                                      " for surface=" + state.dataSurface->Surface(SurfNum).Name,
                                                  state.dataSurface->SurfLowTempErrCount(SurfNum),
                                                  CheckTemperature,
                                                  CheckTemperature,
                                                  _,
                                                  "C",
                                                  "C");
                } else {
                    ShowRecurringSevereErrorAtEnd(state,
                                                  "Temperature (low) out of bounds for zone=" + state.dataHeatBal->Zone(ZoneNum).Name +
                                                      " for surface=" + state.dataSurface->Surface(SurfNum).Name,
                                                  state.dataSurface->SurfLowTempErrCount(SurfNum),
                                                  CheckTemperature,
                                                  CheckTemperature,
                                                  _,
                                                  "C",
                                                  "C");
                }
            } else {
                if (state.dataSurface->SurfHighTempErrCount(SurfNum) == 0) {
                    ShowSevereMessage(state,
                                      format("Temperature (high) out of bounds ({:.2R}] for zone=\"{}\", for surface=\"{}\"",
                                             CheckTemperature,
                                             state.dataHeatBal->Zone(ZoneNum).Name,
                                             state.dataSurface->Surface(SurfNum).Name));
                    ShowContinueErrorTimeStamp(state, "");
                    if (!state.dataHeatBal->Zone(ZoneNum).TempOutOfBoundsReported) {
                        ShowContinueError(state, format("Zone=\"{}\", Diagnostic Details:", state.dataHeatBal->Zone(ZoneNum).Name));
                        if (state.dataHeatBal->Zone(ZoneNum).FloorArea > 0.0) {
                            ShowContinueError(
                                state,
                                format("...Internal Heat Gain [{:.3R}] W/m2",
                                       state.dataHeatBal->Zone(ZoneNum).InternalHeatGains / state.dataHeatBal->Zone(ZoneNum).FloorArea));
                        } else {
                            ShowContinueError(
                                state, format("...Internal Heat Gain (no floor) [{:.3R}] W", state.dataHeatBal->Zone(ZoneNum).InternalHeatGains));
                        }
                        if (state.afn->simulation_control.type == AirflowNetwork::ControlType::NoMultizoneOrDistribution) {
                            ShowContinueError(state,
                                              format("...Infiltration/Ventilation [{:.3R}] m3/s", state.dataHeatBal->Zone(ZoneNum).NominalInfilVent));
                            ShowContinueError(state, format("...Mixing/Cross Mixing [{:.3R}] m3/s", state.dataHeatBal->Zone(ZoneNum).NominalMixing));
                        } else {
                            ShowContinueError(state, "...Airflow Network Simulation: Nominal Infiltration/Ventilation/Mixing not available.");
                        }
                        if (state.dataHeatBal->Zone(ZoneNum).IsControlled) {
                            ShowContinueError(state, "...Zone is part of HVAC controlled system.");
                        } else {
                            ShowContinueError(state, "...Zone is not part of HVAC controlled system.");
                        }
                        state.dataHeatBal->Zone(ZoneNum).TempOutOfBoundsReported = true;
                    }
                    ShowRecurringSevereErrorAtEnd(state,
                                                  "Temperature (high) out of bounds for zone=" + state.dataHeatBal->Zone(ZoneNum).Name +
                                                      " for surface=" + state.dataSurface->Surface(SurfNum).Name,
                                                  state.dataSurface->SurfHighTempErrCount(SurfNum),
                                                  CheckTemperature,
                                                  CheckTemperature,
                                                  _,
                                                  "C",
                                                  "C");
                } else {
                    ShowRecurringSevereErrorAtEnd(state,
                                                  "Temperature (high) out of bounds for zone=" + state.dataHeatBal->Zone(ZoneNum).Name +
                                                      " for surface=" + state.dataSurface->Surface(SurfNum).Name,
                                                  state.dataSurface->SurfHighTempErrCount(SurfNum),
                                                  CheckTemperature,
                                                  CheckTemperature,
                                                  _,
                                                  "C",
                                                  "C");
                }
            }
        }
    }

    void CheckFDNodeTempLimits(EnergyPlusData &state,
                               int surfNum,     // surface number
                               int nodeNum,     // node number
                               Real64 &nodeTemp // calculated temperature, not reset
    )
    {
        auto &s_hbfd = state.dataHeatBalFiniteDiffMgr;

        auto &surfFD = s_hbfd->SurfaceFD(surfNum);
        auto &surfName = state.dataSurface->Surface(surfNum).Name;
        auto &minTempLimit = DataHeatBalSurface::MinSurfaceTempLimit;
        auto &maxTempLimit = state.dataHeatBalSurf->MaxSurfaceTempLimit;
        if (nodeTemp < minTempLimit) {
            if (surfFD.indexNodeMinTempLimit == 0) {
                ShowSevereMessage(state,
                                  format("Node temperature (low) out of bounds [{:.2R}] for surface={}, node={}", nodeTemp, surfName, nodeNum));
                ShowContinueErrorTimeStamp(state, "");
                ShowContinueError(state, format("Value has been reset to the lower limit value of {:.2R}.", minTempLimit));
            }
            ShowRecurringSevereErrorAtEnd(
                state, "Node temperature (low) out of bounds for surface=" + surfName, surfFD.indexNodeMinTempLimit, nodeTemp, nodeTemp, _, "C", "C");
            nodeTemp = minTempLimit;
        } else if (nodeTemp > maxTempLimit) {
            if (surfFD.indexNodeMaxTempLimit == 0) {
                ShowSevereMessage(state,
                                  format("Node temperature (high) out of bounds [{:.2R}] for surface={}, node={}", nodeTemp, surfName, nodeNum));
                ShowContinueErrorTimeStamp(state, "");
                ShowContinueError(state, format("Value has been reset to the upper limit value of {:.2R}.", maxTempLimit));
            }
            ShowRecurringSevereErrorAtEnd(state,
                                          "Node temperature (high) out of bounds for surface=" + surfName,
                                          surfFD.indexNodeMaxTempLimit,
                                          nodeTemp,
                                          nodeTemp,
                                          _,
                                          "C",
                                          "C");
            nodeTemp = maxTempLimit;
        }
    }

    void CalcNodeHeatFlux(EnergyPlusData &state,
                          int const SurfNum, // surface number
                          int const TotNodes // number of nodes in surface
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         M.J. Witte
        //       DATE WRITTEN   Sept-Nov 2015
        // PURPOSE OF THIS SUBROUTINE:
        // Calculate flux at each condFD node

        auto &s_hbfd = state.dataHeatBalFiniteDiffMgr;
        auto &surfFD = s_hbfd->SurfaceFD(SurfNum);

        // SurfaceFD.QDreport( n ) is the flux at node n
        // When this is called TDT( NodeNum ) is the new node temp and TDpriortimestep( NodeNum ) holds the previous node temp
        // For the TDT and TDpriortimestep arrays, Node 1 is the outside face, and Node TotNodes+1 is the inside face

        // Last node is always the surface inside face.  Start calculations here because the outside face is not defined for all surfaces.
        // Note that TotNodes is the number of nodes in the surface including the outside face node, but not the inside face node
        // so the arrays are all allocated to Totodes+1

        // Heat flux at the inside face node (TotNodes+1)
        surfFD.QDreport(TotNodes + 1) = state.dataHeatBalSurf->SurfOpaqInsFaceCondFlux(SurfNum);

        // Heat flux for remaining nodes.
        for (int node = TotNodes; node >= 1; --node) {
            // Start with inside face (above) and work outward, positive value is flowing towards the inside face
            // CpDelXRhoS1 is outer half-node heat capacity, CpDelXRhoS2 is inner half node heat capacity
            Real64 interNodeFlux; // heat flux at the plane between node and node+1 [W/m2]
            Real64 sourceFlux;    // Internal source flux [W/m2]
            if (surfFD.SourceNodeNum == node) {
                sourceFlux = surfFD.QSource;
            } else {
                sourceFlux = 0.0;
            }
            interNodeFlux = surfFD.QDreport(node + 1) + surfFD.CpDelXRhoS1(node + 1) * (surfFD.TDT(node + 1) - surfFD.TDpriortimestep(node + 1)) /
                                                            state.dataGlobal->TimeStepZoneSec;
            surfFD.QDreport(node) = interNodeFlux - sourceFlux +
                                    surfFD.CpDelXRhoS2(node) * (surfFD.TDT(node) - surfFD.TDpriortimestep(node)) / state.dataGlobal->TimeStepZoneSec;
        }
        if (state.dataEnvrn->IsRain) {
            state.dataHeatBalSurf->SurfOpaqOutFaceCondFlux(SurfNum) = -surfFD.QDreport(1); // Update the outside flux if it is raining
        }
    }

    void adjustPropertiesForPhaseChange(EnergyPlusData &state,
                                        int finiteDifferenceLayerIndex,
                                        int surfNum,
                                        Material::MaterialPhaseChange *mat,
                                        Real64 temperaturePrevious,
                                        Real64 temperatureUpdated,
                                        Real64 &updatedSpecificHeat,
                                        Real64 &updatedDensity,
                                        Real64 &updatedThermalConductivity)
    {
        auto &s_hbfd = state.dataHeatBalFiniteDiffMgr;
        auto &surfFD = s_hbfd->SurfaceFD(surfNum);

        updatedSpecificHeat = mat->getCurrentSpecificHeat(temperaturePrevious,
                                                          temperatureUpdated,
                                                          surfFD.PhaseChangeTemperatureReverse(finiteDifferenceLayerIndex),
                                                          surfFD.PhaseChangeStateOld(finiteDifferenceLayerIndex),
                                                          surfFD.PhaseChangeState(finiteDifferenceLayerIndex));

        surfFD.PhaseChangeStateRep(finiteDifferenceLayerIndex) = Material::phaseInts[(int)surfFD.PhaseChangeState(finiteDifferenceLayerIndex)];
        updatedDensity = mat->getDensity(temperaturePrevious);
        updatedThermalConductivity = mat->getConductivity(temperatureUpdated);
    }

    bool findAnySurfacesUsingConstructionAndCondFD(EnergyPlusData const &state, int const constructionNum)
    {
        for (auto const &thisSurface : state.dataSurface->Surface) {
            if (thisSurface.Construction == constructionNum) {
                if (thisSurface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CondFD) {
                    return true;
                }
            }
        }
        return false;
    }

} // namespace HeatBalFiniteDiffManager

} // namespace EnergyPlus
