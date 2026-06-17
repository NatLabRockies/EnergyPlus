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

#ifndef _ST_SIMTHREAD_
#define _ST_SIMTHREAD_ 1
#include "definitions.h"

#ifdef SP_USE_SOLTRACE

#include <thread>
#include <mutex>
#include <vector>
#include "stapi.h"

typedef void* st_context_t;

class STSimThread 
{
	
	bool
		Finished,
		CancelFlag,
        SaveStage0Data,
        LoadStage0Data;
	int NToTrace, 
		NTraced, 
		NTraceTotal, 
		ResultCode, 
		SeedVal, 
		CurStage, 
		NStages, 
		ThreadNum;
	
	st_context_t ContextId;
	std::mutex
		StatusLock,
		CancelLock,
		FinishedLock;

    std::vector<std::vector<double> > raydata_st0;
    std::vector<std::vector<double> > raydata_st1;

public:

	STSimThread(){};

	void Setup( st_context_t spcxt, int thd_num, int seed, bool is_load_st0data = false, bool is_save_st0data = false );
	
    void CopyStageRayData( std::vector<std::vector<double> > &src_dat, int which_stage /*0 or 1*/, int ind_start, int ind_end );
    
    std::vector<std::vector< double > > *GetStage0RayDataObject();
    std::vector<std::vector< double > > *GetStage1RayDataObject();


	~STSimThread();

	void CancelTrace();

	bool IsTraceCancelled();

	bool IsFinished();

	void UpdateStatus(int ntracedtotal, int ntraced, int ntotrace, int curstage, int nstages);

	void GetStatus(int *total, int *traced, int *ntotrace, int *stage, int *nstages);
	
	int GetResultCode();

	st_context_t GetContextId();

	void StartThread();

};

//Multithreaded callback 
extern int STCallback_MT(st_uint_t ntracedtotal, st_uint_t ntraced, st_uint_t ntotrace, st_uint_t curstage, st_uint_t nstages, void *data);

#endif

#endif