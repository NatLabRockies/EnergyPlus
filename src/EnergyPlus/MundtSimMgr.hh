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

#ifndef MundtSimMgr_hh_INCLUDED
#define MundtSimMgr_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace RoomAir {

    // Types

    struct DefineLinearModelNode
    {
        // Members
        std::string AirNodeName;                                        // Name of air nodes
        RoomAir::AirNodeType ClassType = RoomAir::AirNodeType::Invalid; // Type of air nodes
        Real64 Height = 0.0;                                            // Z coordinates [m] node's Control Vol. center
        Real64 Temp = 0.0;                                              // Surface temperature BC
        Array1D_bool SurfMask;                                          // Limit of 60 surfaces at current sizing
    };

    struct DefineSurfaceSettings
    {
        // Members
        Real64 Area = 0.0;     // m2
        Real64 Temp = 0.0;     // surface temperature BC
        Real64 Hc = 0.0;       // convective film coeff BC
        Real64 TMeanAir = 0.0; // effective near-surface air temp from air model solution
    };

    struct DefineZoneData
    {
        // Members
        int NumOfSurfs = 0;             // number of surfaces in the zone
        int MundtZoneIndex = 0;         // index for zones using Mundt model
        EPVector<int> HBsurfaceIndexes; // list of surface indexes in this Mundt model zone
    };

    // Functions

    void ManageDispVent1Node(EnergyPlusData &state, int ZoneNum); // index number for the specified zone

    //*****************************************************************************************

    void InitDispVent1Node(EnergyPlusData &state);

    //*****************************************************************************************

    void GetSurfHBDataForDispVent1Node(EnergyPlusData &state, int ZoneNum); // index number for the specified zone

    //*****************************************************************************************

    void SetupDispVent1Node(EnergyPlusData &state,
                            int ZoneNum,      // index number for the specified zone
                            bool &ErrorsFound // true if problems setting up model
    );

    //*****************************************************************************************

    void CalcDispVent1Node(EnergyPlusData &state, int const ZoneNum); // index number for the specified zone

    //*****************************************************************************************

    void SetNodeResult(EnergyPlusData &state,
                       int NodeID,       // node ID
                       Real64 TempResult // temperature for the specified air node
    );

    //*****************************************************************************************

    void SetSurfTmeanAir(EnergyPlusData &state,
                         int SurfID,    // surface ID
                         Real64 TeffAir // temperature of air node adjacent to the specified surface
    );

    //*****************************************************************************************

    void SetSurfHBDataForDispVent1Node(EnergyPlusData &state, int ZoneNum); // index number for the specified zone

    //*****************************************************************************************

} // namespace RoomAir

struct MundtSimMgrData : BaseGlobalStruct
{

    Array1D_int FloorSurfSetIDs;      // fixed variable for floors
    Array1D_int TheseSurfIDs;         // temporary working variable
    int MundtCeilAirID = 0;           // air node index in AirDataManager
    int MundtFootAirID = 0;           // air node index in AirDataManager
    int SupplyNodeID = 0;             // air node index in AirDataManager
    int TstatNodeID = 0;              // air node index in AirDataManager
    int ReturnNodeID = 0;             // air node index in AirDataManager
    int NumRoomNodes = 0;             // number of nodes connected to walls
    int NumFloorSurfs = 0;            // total number of surfaces for floor
    Array1D_int RoomNodeIDs;          // ids of the first NumRoomNode Air Nodes
    Array1D_int ID1dSurf;             // numbers used to identify surfaces
    int MundtZoneNum = 0;             // index of zones using Mundt model
    Real64 ZoneHeight = 0.0;          // zone height
    Real64 ZoneFloorArea = 0.0;       // zone floor area
    Real64 QventCool = 0.0;           // heat gain due to ventilation
    Real64 ConvIntGain = 0.0;         // heat gain due to internal gains
    Real64 SupplyAirTemp = 0.0;       // supply air temperature
    Real64 SupplyAirVolumeRate = 0.0; // supply air volume flowrate
    Real64 ZoneAirDensity = 0.0;      // zone air density
    Real64 QsysCoolTot = 0.0;         // zone sensible cooling load

    // Object Data
    Array1D<RoomAir::DefineZoneData> ZoneData;            // zone data
    Array2D<RoomAir::DefineLinearModelNode> LineNode;     // air nodes
    Array2D<RoomAir::DefineSurfaceSettings> MundtAirSurf; // surfaces
    Array1D<RoomAir::DefineSurfaceSettings> FloorSurf;    // floor

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->FloorSurfSetIDs.clear();   // fixed variable for floors
        this->TheseSurfIDs.clear();      // temporary working variable
        this->MundtCeilAirID = 0;        // air node index in AirDataManager
        this->MundtFootAirID = 0;        // air node index in AirDataManager
        this->SupplyNodeID = 0;          // air node index in AirDataManager
        this->TstatNodeID = 0;           // air node index in AirDataManager
        this->ReturnNodeID = 0;          // air node index in AirDataManager
        this->NumRoomNodes = 0;          // number of nodes connected to walls
        this->NumFloorSurfs = 0;         // total number of surfaces for floor
        this->RoomNodeIDs.clear();       // ids of the first NumRoomNode Air Nodes
        this->ID1dSurf.clear();          // numbers used to identify surfaces
        this->MundtZoneNum = 0;          // index of zones using Mundt model
        this->ZoneHeight = 0.0;          // zone height
        this->ZoneFloorArea = 0.0;       // zone floor area
        this->QventCool = 0.0;           // heat gain due to ventilation
        this->ConvIntGain = 0.0;         // heat gain due to internal gains
        this->SupplyAirTemp = 0.0;       // supply air temperature
        this->SupplyAirVolumeRate = 0.0; // supply air volume flowrate
        this->ZoneAirDensity = 0.0;      // zone air density
        this->QsysCoolTot = 0.0;         // zone sensible cooling load
        this->ZoneData.clear();          // zone data
        this->LineNode.clear();          // air nodes
        this->MundtAirSurf.clear();      // surfaces
        this->FloorSurf.clear();         // floor
    }
};

} // namespace EnergyPlus

#endif
