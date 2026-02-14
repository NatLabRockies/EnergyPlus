// EnergyPlus, Copyright (c) 1996-present, The Board of Trustees of the University of Illinois,
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

#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/WindowModel.hh>

namespace EnergyPlus {

namespace Window {

    constexpr std::array<std::string_view, (int)OpticalDataModel::Num> opticalDataModelNames = {
        "SpectralAverage", "Spectral", "BSDF", "SpectralAndAngle"};

    constexpr std::array<std::string_view, (int)OpticalDataModel::Num> opticalDataModelNamesUC = {
        "SPECTRALAVERAGE", "SPECTRAL", "BSDF", "SPECTRALANDANGLE"};

    constexpr std::array<std::string_view, (int)WindowsModel::Num> windowsModelNamesUC = {"BUILTINWINDOWSMODEL", "EXTERNALWINDOWSMODEL"};

    /////////////////////////////////////////////////////////////////////////////////////////
    //  CWindowModel
    /////////////////////////////////////////////////////////////////////////////////////////

    CWindowModel::CWindowModel() : m_Model(WindowsModel::BuiltIn)
    {
    }

    std::unique_ptr<CWindowModel> CWindowModel::WindowModelFactory(EnergyPlusData &state, std::string const &objectName)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Vidanovic
        //       DATE WRITTEN   July 2016
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Reads input and creates instance of WindowModel object

        auto aModel = std::make_unique<CWindowModel>(); // (AUTO_OK)
        int numCurrModels = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, objectName);
        if (numCurrModels > 0) {
            int NumNums;
            int NumAlphas;
            int IOStat;
            state.dataInputProcessing->inputProcessor->getObjectItem(
                state, objectName, 1, state.dataIPShortCut->cAlphaArgs, NumAlphas, state.dataIPShortCut->rNumericArgs, NumNums, IOStat);
            // Please consider using getEnumValue pattern here.
            // Consider that you are creating an entire map for the
            // sole purpose of looking up a single element

            aModel->m_Model = static_cast<WindowsModel>(getEnumValue(windowsModelNamesUC, state.dataIPShortCut->cAlphaArgs(1)));
        }

        return aModel;
    }

    WindowsModel CWindowModel::getWindowsModel() const
    {
        return m_Model;
    }

    bool CWindowModel::isExternalLibraryModel() const
    {
        return m_Model == WindowsModel::External;
    }

    void CWindowModel::setExternalLibraryModel(WindowsModel model)
    {
        m_Model = model;
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    //  CWindowOpticalModel
    /////////////////////////////////////////////////////////////////////////////////////////

    CWindowOpticalModel::CWindowOpticalModel() : m_Model(WindowsOpticalModel::Simplified)
    {
    }

    std::unique_ptr<CWindowOpticalModel> CWindowOpticalModel::WindowOpticalModelFactory(EnergyPlusData &state)
    {
        // Process input data and counts if number of complex fenestration objects is greater
        // than zero in which case it will use BSDF window model
        auto aModel = std::make_unique<CWindowOpticalModel>(); // (AUTO_OK)
        int numCurrModels = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Construction:ComplexFenestrationState");

        if (numCurrModels > 0) {
            aModel->m_Model = WindowsOpticalModel::BSDF;
        }

        return aModel;
    }

    WindowsOpticalModel CWindowOpticalModel::getWindowsOpticalModel() const
    {
        return m_Model;
    }

    bool CWindowOpticalModel::isSimplifiedModel() const
    {
        return (m_Model == WindowsOpticalModel::Simplified);
    }

} // namespace Window

} // namespace EnergyPlus
