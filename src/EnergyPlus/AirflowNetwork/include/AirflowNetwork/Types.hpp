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

#ifndef AIRFLOWNETWORK_TYPES_HPP
#define AIRFLOWNETWORK_TYPES_HPP

namespace EnergyPlus {

namespace AirflowNetwork {

    enum VentControlType
    {
        None = 0,  // Wrong input
        Temp = 1,  // Temperature venting control
        Enth = 2,  // Enthalpy venting control
        Const = 3, // Constant venting control
        ASH55 = 4,
        CEN15251 = 5,
        NoVent = 6,    // No venting
        ZoneLevel = 7, // ZoneLevel control for a heat transfer subsurface
        AdjTemp = 8,   // Temperature venting control based on adjacent zone conditions
        AdjEnth = 9    // Enthalpy venting control based on adjacent zone conditions
    };

    enum OpenStatus
    {
        FreeOperation = 0,     // Free operation
        MinCheckForceOpen = 1, // Force open when opening elapsed time is less than minimum opening time
        MinCheckForceClose = 2 // Force open when closing elapsed time is less than minimum closing time
    };

    enum ProbabilityCheck
    {
        NoAction = 0,    // No action from probability check
        ForceChange = 1, // Force open or close from probability check
        KeepStatus = 2   // Keep status at the previous time step from probability check
    };

    enum class EquivRec : int
    {
        Height,          // Effective rectangle polygonal height selection
        BaseAspectRatio, // Effective rectangle base surface aspect ratio selection
        UserAspectRatio  // Effective rectangle user input aspect ratio selection
    };

    enum class DuctLineType : int
    {
        Invalid = -1,
        SupplyTrunk,  // Supply trunk
        SupplyBranch, // Supply branch
        ReturnTrunk,  // Return trunk
        ReturnBranch, // Return branch
    };

    enum class AirflowElementType : int
    {
        Invalid = -1,
        DOP = 1, // Detailed large opening component
        SOP,     // Simple opening component
        SCR,     // Surface crack component
        SEL,     // Surface effective leakage ratio component
        PLR,     // Distribution system crack component
        DWC,     // Distribution system duct component
        CVF,     // Distribution system constant volume fan component
        FAN,     // Distribution system detailed fan component
        MRR,     // Distribution system multiple curve fit power law resistant flow component
        DMP,     // Distribution system damper component
        ELR,     // Distribution system effective leakage ratio component
        CPD,     // Distribution system constant pressure drop component
        COI,     // Distribution system coil component
        TMU,     // Distribution system terminal unit component
        EXF,     // Zone exhaust fan
        HEX,     // Distribution system heat exchanger
        HOP,     // Horizontal opening component
        RVD,     // Reheat VAV terminal damper
        OAF,     // Distribution system OA
        REL,     // Distribution system relief air
        SMF,     // Specified mass flow component
        SVF      // Specified volume flow component
    };

    enum class EPlusComponentType : int
    {
        Invalid = -1,
        SCN, // Supply connection
        RCN, // Return connection
        RHT, // Reheat terminal
        FAN, // Fan
        COI, // Heating or cooling coil
        HEX, // Heat exchanger
        RVD  // Reheat VAV terminal damper
    };

    enum class EPlusNodeType : int
    {
        Invalid = -1,
        ZIN, // Zone inlet node
        ZOU, // Zone outlet node
        SPL, // Splitter node
        MIX, // Mixer node
        OAN, // Outside air system node
        EXT, // OA system inlet node
        FIN, // Fan Inlet node
        FOU, // Fan Outlet Node
        COU, // Coil Outlet Node
        HXO, // Heat exchanger Outlet Node
        DIN, // Damper Inlet node
        DOU, // Damper Outlet Node
        SPI, // Splitter inlet Node
        SPO  // Splitter Outlet Node
    };

    enum class WindPressureType : int
    {
        Input,
        SurfAvg
    };

    int constexpr PressureCtrlExhaust = 1;
    int constexpr PressureCtrlRelief = 2;

} // namespace AirflowNetwork

} // namespace EnergyPlus

#endif
