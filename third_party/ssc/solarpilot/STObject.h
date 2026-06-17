/*******************************************************************************************************
*  Copyright 2017 Alliance for Sustainable Energy, LLC
*
*  NOTICE: This software was developed at least in part by Alliance for Sustainable Energy, LLC
*  (“Alliance”) under Contract No. DE-AC36-08GO28308 with the U.S. Department of Energy and the U.S.
*  The Government retains for itself and others acting on its behalf a nonexclusive, paid-up,
*  irrevocable worldwide license in the software to reproduce, prepare derivative works, distribute
*  copies to the public, perform publicly and display publicly, and to permit others to do so.
* 
*  Redistribution and use in source and binary forms, with or without modification, are permitted
*  provided that the following conditions are met:
* 
*  1. Redistributions of source code must retain the above copyright notice, the above government
*  rights notice, this list of conditions and the following disclaimer.
* 
*  2. Redistributions in binary form must reproduce the above copyright notice, the above government
*  rights notice, this list of conditions and the following disclaimer in the documentation and/or
*  other materials provided with the distribution.
* 
*  3. The entire corresponding source code of any redistribution, with or without modification, by a
*  research entity, including but not limited to any contracting manager/operator of a United States
*  National Laboratory, any institution of higher learning, and any non-profit organization, must be
*  made publicly available under this license for as long as the redistribution is made available by
*  the research entity.
* 
*  4. Redistribution of this software, without modification, must refer to the software by the same
*  designation. Redistribution of a modified version of this software (i) may not refer to the modified
*  version by the same designation, or by any confusingly similar designation, and (ii) must refer to
*  the underlying software originally provided by Alliance as “System Advisor Model” or “SAM”. Except
*  to comply with the foregoing, the terms “System Advisor Model”, “SAM”, or any confusingly similar
*  designation may not be used to refer to any modified version of this software or any modified
*  version of the underlying software originally provided by Alliance without the prior written consent
*  of Alliance.
* 
*  5. The name of the copyright holder, contributors, the United States Government, the United States
*  Department of Energy, or any of their employees may not be used to endorse or promote products
*  derived from this software without specific prior written permission.
* 
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
*  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
*  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER,
*  CONTRIBUTORS, UNITED STATES GOVERNMENT OR UNITED STATES DEPARTMENT OF ENERGY, NOR ANY OF THEIR
*  EMPLOYEES, BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
*  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
*  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
*  THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************************************/

#ifndef _ST_OBJECT_
#define _ST_OBJECT_ 1
#include "definitions.h"

#ifdef SP_USE_SOLTRACE

#include <vector>
#include <string>
#include <exception>

#include "stapi.h"
#include "mtrand.h"
#include "SolarField.h"

class ST_OpticalProperties
{
public:
	ST_OpticalProperties();
	ST_OpticalProperties &operator=(const ST_OpticalProperties &rhs);
	
	char DistributionType;
	int OpticSurfNumber;
	int ApertureStopOrGratingType;
	int DiffractionOrder;
	double Reflectivity;
	double Transmissivity;
	double RMSSlopeError;
	double RMSSpecError;

	double Grating[4];	
	void Write(FILE *fdat);
};

class ST_OpticalPropertySet
{
public:
	std::string Name;
	ST_OpticalProperties Front;
	ST_OpticalProperties Back;	
	void Write(FILE *fdat);
};

struct ST_Element
{
	ST_Element();
	
	bool Enabled;
	void Write(FILE *fdat);
	/////////// ORIENTATION PARAMETERS ///////////////
	double Origin[3];
	double AimPoint[3];
	double ZRot;
	double Euler[3]; // calculated
	double RRefToLoc[3][3]; // calculated
	double RLocToRef[3][3]; // calculated
	void UpdateRotationMatrix();
	
	/////////// APERTURE PARAMETERS ///////////////
	char ShapeIndex;
	double Ap_A, Ap_B, Ap_C, Ap_D, Ap_E, Ap_F, Ap_G, Ap_H;	//APERTURE PARAMETERS
	
	double ApertureArea; // calculated
	double ZAperture; // calculated 
	
	/////////// SURFACE PARAMETERS ///////////////
	char SurfaceIndex;
	double Su_A, Su_B, Su_C, Su_D, Su_E, Su_F, Su_G, Su_H;
	
	/////////// OPTICAL PARAMETERS ///////////////
	int InteractionType;
	ST_OpticalPropertySet *Optics;
	std::string OpticName;

	std::string Comment;	
};

struct ST_Sun
{
	ST_Sun();
	void Reset();
	
	char ShapeIndex;	
	double Sigma;

	std::vector<double> SunShapeAngle;
	std::vector<double> SunShapeIntensity;
	
	double Origin[3];
	void Write(FILE *fdat);
};

class ST_RayData
{
public:
	ST_RayData();
	~ST_RayData();

	struct ray_t
	{
		double pos[3];
		double cos[3];
		int element;
		int stage;
		unsigned int raynum;
	};

	ray_t *Append( double pos[3],
					 double cos[3],
					 int element,
					 int stage,
					 unsigned int raynum );

	bool Overwrite( unsigned int idx,
					double pos[3],
					double cos[3],
					int element,
					int stage,
					unsigned int raynum);

	bool Query( unsigned int idx,
					double pos[3],
					double cos[3],
					int *element,
					int *stage,
					unsigned int *raynum);

	void Merge( ST_RayData &dest );

	void Clear();

	void Print();

	st_uint_t Count();

	ray_t *Index(st_uint_t i, bool write_access);

private:
	static const unsigned int block_size = 16384;


	struct block_t
	{
		ray_t data[block_size];
		st_uint_t count;
	};

	std::vector<block_t*> m_blockList;
	st_uint_t m_dataCount;
	st_uint_t m_dataCapacity;
};


class ST_IntersectionData
{
public:
    double *hitx;
	double *hity;
	double *hitz;
	double *cosx;
	double *cosy;
	double *cosz;
	int *emap;	//corresponding element number
	int *smap;	//corresponding stage number
	int *rnum;	//ray numbers
    int nint;   //number of intersections
    int nsunrays;
    double q_ray;   //power per ray
    double bounds[5]; //land bounds

    ST_IntersectionData();
    ~ST_IntersectionData();
    void AllocateArrays(int size);
    void DeallocateArrays();
};


struct ST_Stage
{
	ST_Stage();
	~ST_Stage();
		
	bool MultiHitsPerRay;
	bool Virtual;
	bool TraceThrough;
	
	double Origin[3];
	double AimPoint[3];
	double ZRot;

	std::vector<ST_Element*> ElementList;
	
	// calculated
	double Euler[3];
	double RRefToLoc[3][3];
	double RLocToRef[3][3];
	std::string Name;
	void Write(FILE *fdat);
};

struct ST_System
{
	ST_System();
	~ST_System();

	void ClearAll();
	
	ST_Sun Sun;
	std::vector<ST_OpticalPropertySet*> OpticsList;
	std::vector<ST_Stage*> StageList;


	// system simulation context data
	int sim_raycount;
	int sim_raymax;
	bool sim_errors_sunshape;
	bool sim_errors_optical;

	// simulation outputs
	ST_RayData AllRayData;
	st_uint_t SunRayCount;
    ST_IntersectionData IntData;

	//method for loading the solar field geometry into the ST_System object
	bool CreateSTSystem(SolarField &SF, Hvector &helios, Vect &sunvect);

	static void LoadIntoContext(ST_System *System, st_context_t spcxt);
	void Write(FILE *fdat);

};

#endif

#endif