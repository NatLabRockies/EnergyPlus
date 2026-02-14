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

#ifndef PVWatts_hh_INCLUDED
#define PVWatts_hh_INCLUDED

// C++ Headers
#include <map>
#include <memory>
#include <string>

// ObjexxFCL Headers

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/EnergyPlus.hh>

// SAM Headers
// #include <../third_party/ssc/shared/lib_irradproc.h>
// #include <../third_party/ssc/shared/lib_pvwatts.h>
// #include <../third_party/ssc/shared/lib_pvshade.h>
// #include <../third_party/ssc/shared/lib_pv_incidence_modifier.h>
#include <../third_party/ssc/ssc/sscapi.h>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace PVWatts {

    enum class ModuleType
    {
        Invalid = -1,
        STANDARD,
        PREMIUM,
        THIN_FILM,
        Num
    };

    enum class ArrayType
    {
        Invalid = -1,
        FIXED_OPEN_RACK,
        FIXED_ROOF_MOUNTED,
        ONE_AXIS,
        ONE_AXIS_BACKTRACKING,
        TWO_AXIS,
        Num
    };

    enum class GeometryType
    {
        Invalid = -1,
        TILT_AZIMUTH,
        SURFACE,
        Num
    };

    struct DCPowerOutput
    {
        Real64 poa;  // Plane of array irradiance
        Real64 tpoa; // Transmitted plane of array irradiance
        Real64 pvt;  // PV Cell temperature
        Real64 dc;   // DC power output
    };

    struct IrradianceOutput
    {
        Real64 solazi;
        Real64 solzen;
        Real64 solalt;
        Real64 aoi;
        Real64 stilt;
        Real64 sazi;
        Real64 rot;
        Real64 btd;
        Real64 ibeam;
        Real64 iskydiff;
        Real64 ignddiff;
        int sunup;
    };

    class PVWattsGenerator
    {
    private:
        enum AlphaFields
        {
            NAME = 1,
            VERSION = 2,
            MODULE_TYPE = 3,
            ARRAY_TYPE = 4,
            GEOMETRY_TYPE = 5,
            SURFACE_NAME = 6,
        };

        enum NumFields
        {
            DC_SYSTEM_CAPACITY = 1,
            SYSTEM_LOSSES = 2,
            TILT_ANGLE = 3,
            AZIMUTH_ANGLE = 4,
            GROUND_COVERAGE_RATIO = 5,
        };

        // User inputs
        std::string name_;
        Real64 dcSystemCapacity_;
        ModuleType moduleType_;
        ArrayType arrayType_;
        Real64 systemLosses_;
        GeometryType geometryType_;
        Real64 tilt_;
        Real64 azimuth_;
        int surfaceNum_;
        Real64 groundCoverageRatio_;
        Real64 DCtoACRatio_;
        Real64 inverterEfficiency_;

        // Output variables
        Real64 outputDCPower_;
        Real64 outputDCEnergy_;
        Real64 outputACPower_;
        Real64 outputACEnergy_;
        Real64 cellTemperature_;
        Real64 planeOfArrayIrradiance_;
        Real64 shadedPercent_;

        ssc_module_t pvwattsModule_;
        ssc_data_t pvwattsData_;
        Real64 NumTimeStepsToday_;

    public:
        static std::unique_ptr<PVWattsGenerator> createFromIdfObj(EnergyPlusData &state, int objNum);

        PVWattsGenerator(EnergyPlusData &state,
                         const std::string &name,
                         const Real64 dcSystemCapacity,
                         ModuleType moduleType,
                         ArrayType arrayType,
                         Real64 systemLosses = 0.14,
                         GeometryType geometryType = GeometryType::TILT_AZIMUTH,
                         Real64 tilt = 20.0,
                         Real64 azimuth = 180.0,
                         size_t surfaceNum = 0,
                         Real64 groundCoverageRatio = 0.4);

        void setupOutputVariables(EnergyPlusData &state);

        Real64 getDCSystemCapacity();
        ModuleType getModuleType();
        ArrayType getArrayType();
        Real64 getSystemLosses();
        GeometryType getGeometryType();
        Real64 getTilt();
        Real64 getAzimuth();
        DataSurfaces::SurfaceData &getSurface(EnergyPlusData &state);
        Real64 getGroundCoverageRatio();

        Real64 getCellTemperature();
        Real64 getPlaneOfArrayIrradiance();
        void setCellTemperature(Real64 cellTemp);
        void setPlaneOfArrayIrradiance(Real64 poa);

        // Setters so we can store properties of the attached inverter.
        void setDCtoACRatio(Real64 dc2ac);
        void setInverterEfficiency(Real64 inverterEfficiency);

        void calc(EnergyPlusData &state);
        void getResults(Real64 &GeneratorPower, Real64 &GeneratorEnergy, Real64 &ThermalPower, Real64 &ThermalEnergy);
    };

} // namespace PVWatts

} // namespace EnergyPlus

#endif
