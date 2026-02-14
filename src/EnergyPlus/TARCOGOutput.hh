// EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Energy Innovation, LLC, and other
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

#ifndef TARCOGOutput_hh_INCLUDED
#define TARCOGOutput_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array2A.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/FileSystem.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/TARCOGGassesParams.hh>
#include <EnergyPlus/TARCOGParams.hh>

namespace EnergyPlus {

namespace TARCOGOutput {

    struct Files
    {
        fs::path DBGD; // Debug directory
        bool WriteDebugOutput{false};

        fs::path WINCogFilePath{"test.w7"};
        InputOutputFile WINCogFile{WINCogFilePath};

        // Intermediate debug files
        fs::path TarcogIterationsFilePath{"TarcogIterations.dbg"};
        InputOutputFile TarcogIterationsFile{TarcogIterationsFilePath};

        fs::path IterationCSVFilePath{"IterationResults.csv"};
        InputOutputFile IterationCSVFile{IterationCSVFilePath};

        fs::path DebugOutputFilePath{"Tarcog.dbg"};
        InputOutputFile DebugOutputFile{DebugOutputFilePath};
    };

    void WriteInputArguments(EnergyPlusData &state,
                             InputOutputFile &InArgumentsFile,
                             fs::path const &DBGC,
                             Real64 tout,
                             Real64 tind,
                             Real64 trmin,
                             Real64 wso,
                             int iwd,
                             Real64 wsi,
                             Real64 dir,
                             Real64 outir,
                             int isky,
                             Real64 tsky,
                             Real64 esky,
                             Real64 fclr,
                             Real64 VacuumPressure,
                             Real64 VacuumMaxGapThickness,
                             const Array1D_int &ibc,
                             Real64 hout,
                             Real64 hin,
                             TARCOGGassesParams::Stdrd standard,
                             TARCOGParams::TARCOGThermalModel ThermalMod,
                             Real64 SDScalar,
                             Real64 height,
                             Real64 heightt,
                             Real64 width,
                             Real64 tilt,
                             Real64 totsol,
                             int nlayer,
                             const Array1D<TARCOGParams::TARCOGLayerType> &LayerType,
                             const Array1D<Real64> &thick,
                             const Array1D<Real64> &scon,
                             const Array1D<Real64> &asol,
                             const Array1D<Real64> &tir,
                             const Array1D<Real64> &emis,
                             const Array1D<Real64> &Atop,
                             const Array1D<Real64> &Abot,
                             const Array1D<Real64> &Al,
                             const Array1D<Real64> &Ar,
                             const Array1D<Real64> &Ah,
                             const Array1D<Real64> &SlatThick,
                             const Array1D<Real64> &SlatWidth,
                             const Array1D<Real64> &SlatAngle,
                             const Array1D<Real64> &SlatCond,
                             const Array1D<Real64> &SlatSpacing,
                             const Array1D<Real64> &SlatCurve,
                             const Array1D_int &nslice,
                             const Array1D<Real64> &LaminateA,
                             const Array1D<Real64> &LaminateB,
                             const Array1D<Real64> &sumsol,
                             const Array1D<Real64> &gap,
                             const Array1D<Real64> &vvent,
                             const Array1D<Real64> &tvent,
                             const Array1D<Real64> &presure,
                             const Array1D_int &nmix,
                             Array2A_int iprop,
                             Array2A<Real64> frct,
                             Array2A<Real64> xgcon,
                             Array2A<Real64> xgvis,
                             Array2A<Real64> xgcp,
                             const Array1D<Real64> &xwght);

    void WriteModifiedArguments(InputOutputFile &InArgumentsFile,
                                fs::path const &DBGC,
                                Real64 esky,
                                Real64 trmout,
                                Real64 trmin,
                                Real64 ebsky,
                                Real64 ebroom,
                                Real64 Gout,
                                Real64 Gin,
                                int nlayer,
                                const Array1D<TARCOGParams::TARCOGLayerType> &LayerType,
                                const Array1D_int &nmix,
                                Array2A<Real64> frct,
                                const Array1D<Real64> &thick,
                                const Array1D<Real64> &scon,
                                const Array1D<Real64> &gap,
                                Array2A<Real64> xgcon,
                                Array2A<Real64> xgvis,
                                Array2A<Real64> xgcp,
                                const Array1D<Real64> &xwght);

    void WriteOutputArguments(InputOutputFile &OutArgumentsFile,
                              fs::path const &DBGC,
                              int nlayer,
                              Real64 tamb,
                              const Array1D<Real64> &q,
                              const Array1D<Real64> &qv,
                              const Array1D<Real64> &qcgas,
                              const Array1D<Real64> &qrgas,
                              const Array1D<Real64> &theta,
                              const Array1D<Real64> &vfreevent,
                              const Array1D<Real64> &vvent,
                              const Array1D<Real64> &Keff,
                              const Array1D<Real64> &ShadeGapKeffConv,
                              Real64 troom,
                              Real64 ufactor,
                              Real64 shgc,
                              Real64 sc,
                              Real64 hflux,
                              Real64 shgct,
                              Real64 hcin,
                              Real64 hrin,
                              Real64 hcout,
                              Real64 hrout,
                              const Array1D<Real64> &Ra,
                              const Array1D<Real64> &Nu,
                              const Array1D<TARCOGParams::TARCOGLayerType> &LayerType,
                              const Array1D<Real64> &Ebf,
                              const Array1D<Real64> &Ebb,
                              const Array1D<Real64> &Rf,
                              const Array1D<Real64> &Rb,
                              Real64 ebsky,
                              Real64 Gout,
                              Real64 ebroom,
                              Real64 Gin,
                              Real64 ShadeEmisRatioIn,
                              Real64 ShadeEmisRatioOut,
                              Real64 ShadeHcRatioIn,
                              Real64 ShadeHcRatioOut,
                              Real64 HcUnshadedIn,
                              Real64 HcUnshadedOut,
                              const Array1D<Real64> &hcgas,
                              const Array1D<Real64> &hrgas,
                              Real64 AchievedErrorTolerance,
                              int NumOfIter);

    void WriteOutputEN673(InputOutputFile &OutArgumentsFile,
                          fs::path const &DBGD,
                          int const nlayer,
                          Real64 const ufactor,
                          Real64 const hout,
                          Real64 const hin,
                          const Array1D<Real64> &Ra,
                          const Array1D<Real64> &Nu,
                          const Array1D<Real64> &hg,
                          const Array1D<Real64> &hr,
                          const Array1D<Real64> &hs,
                          int &nperr);

    void WriteTARCOGInputFile(EnergyPlusData &state,
                              Files &files,
                              std::string const &VerNum,
                              Real64 tout,
                              Real64 tind,
                              Real64 trmin,
                              Real64 wso,
                              int iwd,
                              Real64 wsi,
                              Real64 dir,
                              Real64 outir,
                              int isky,
                              Real64 tsky,
                              Real64 esky,
                              Real64 fclr,
                              Real64 VacuumPressure,
                              Real64 VacuumMaxGapThickness,
                              TARCOGParams::DeflectionCalculation CalcDeflection,
                              Real64 Pa,
                              Real64 Pini,
                              Real64 Tini,
                              const Array1D_int &ibc,
                              Real64 hout,
                              Real64 hin,
                              TARCOGGassesParams::Stdrd standard,
                              TARCOGParams::TARCOGThermalModel ThermalMod,
                              Real64 SDScalar,
                              Real64 height,
                              Real64 heightt,
                              Real64 width,
                              Real64 tilt,
                              Real64 totsol,
                              int nlayer,
                              const Array1D<TARCOGParams::TARCOGLayerType> &LayerType,
                              const Array1D<Real64> &thick,
                              const Array1D<Real64> &scon,
                              const Array1D<Real64> &YoungsMod,
                              const Array1D<Real64> &PoissonsRat,
                              const Array1D<Real64> &asol,
                              const Array1D<Real64> &tir,
                              const Array1D<Real64> &emis,
                              const Array1D<Real64> &Atop,
                              const Array1D<Real64> &Abot,
                              const Array1D<Real64> &Al,
                              const Array1D<Real64> &Ar,
                              const Array1D<Real64> &Ah,
                              const Array1D_int &SupportPillar,
                              const Array1D<Real64> &PillarSpacing,
                              const Array1D<Real64> &PillarRadius,
                              const Array1D<Real64> &SlatThick,
                              const Array1D<Real64> &SlatWidth,
                              const Array1D<Real64> &SlatAngle,
                              const Array1D<Real64> &SlatCond,
                              const Array1D<Real64> &SlatSpacing,
                              const Array1D<Real64> &SlatCurve,
                              const Array1D_int &nslice,
                              const Array1D<Real64> &gap,
                              const Array1D<Real64> &GapDef,
                              const Array1D<Real64> &vvent,
                              const Array1D<Real64> &tvent,
                              const Array1D<Real64> &presure,
                              const Array1D_int &nmix,
                              Array2A_int iprop,
                              Array2A<Real64> frct,
                              Array2A<Real64> xgcon,
                              Array2A<Real64> xgvis,
                              Array2A<Real64> xgcp,
                              const Array1D<Real64> &xwght,
                              const Array1D<Real64> &gama);

    void FinishDebugOutputFiles(Files &files, int nperr);

    void PrepDebugFilesAndVariables(
        EnergyPlusData &state, Files &files, fs::path const &Debug_dir, fs::path const &Debug_file, int Debug_mode, int win_ID, int igu_ID);

} // namespace TARCOGOutput

struct TARCOGOutputData : BaseGlobalStruct
{
    int winID = 0;
    int iguID = 0;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->winID = 0;
        this->iguID = 0;
    }
};

} // namespace EnergyPlus

#endif
