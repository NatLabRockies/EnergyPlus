/**
BSD-3-Clause
Copyright 2019 Alliance for Sustainable Energy, LLC
Redistribution and use in source and binary forms, with or without modification, are permitted provided 
that the following conditions are met :
1.	Redistributions of source code must retain the above copyright notice, this list of conditions 
and the following disclaimer.
2.	Redistributions in binary form must reproduce the above copyright notice, this list of conditions 
and the following disclaimer in the documentation and/or other materials provided with the distribution.
3.	Neither the name of the copyright holder nor the names of its contributors may be used to endorse 
or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
ARE DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER, CONTRIBUTORS, UNITED STATES GOVERNMENT OR UNITED STATES 
DEPARTMENT OF ENERGY, NOR ANY OF THEIR EMPLOYEES, BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Flux.h"
#include "math.h"
#include "Toolbox.h"
#include "Heliostat.h"
#include "Ambient.h"
#include "Receiver.h"
#include "SolarField.h"
#include "Land.h"
//#include <vector>
#include <iostream>
#include <algorithm>

#include <iostream>
#include <fstream>

using namespace std;
using namespace Toolbox;

/* 
Note:

Several algorithms in this class are based on the Hermite expansion technique for characterizing
Gaussian flux distributions which was developed by Dellin, Walzel, Lipps, et al, and implemented 
in DELSOL3. These algorithms are publicly available through the following publications:

[1] T. A. Dellin, "An improved Hermite expansion calculation of the flux distribution from
	heliostats", Sandia National Laboratory, Livermore, CA, 1979. SAND79-8619.
[2] M. D. Walzel, F. W. Lipps, and Vant-Hull, "A solar flux density calculation for a solar
	tower concentrator using a two-dimensional Hermite function expansion", Solar Energy,
	vol. 19, pp. 239-256, 1977.
[3] B. L. Kistler, "A user's manual for DELSOL3: A computer code for calculating the optical
	performance and optimal system design for solar thermal central receiver plants", Sandia
	National Laboratory, Albuquerque, NM, 1986. SAND86-8018.


*/


// ----------------- random class -------------
Random::Random()
{ 
	srand( (unsigned int)time( (time_t*)NULL) ); //Seed with time on init
	rmax = RAND_MAX;
}	
double Random::uniform()
{ 
	return double( rand() )/double(rmax); 
}

double Random::triangular()
{
	return pow(uniform(), .5);	//1.0 is more likely, 0.0 is less likely
}	

double Random::normal(double stddev)
{
	//Sample from normal distribution - recursive call
	//Distribution mean is 0., half-tail is positive
	double 
		u = uniform()*2. - 1.,
		v = uniform()*2. - 1.,
		r = u*u + v*v;
	if(r == 0 || r > 1) return normal(stddev);
	double c = sqrt(-2 * log(r)/r);
	return u*c*stddev;
}

double Random::sign()
{
	return uniform()<.5 ? -1. : 1.;
}

int Random::integer(int min, int max)
{
	return int(floor(uniform()*(max-min)));
}
//--------------------------------------------
Random *Flux::getRandomObject(){return _random; }

//Constructor 
Flux::Flux(){ 
	_random = new Random();
	_jmin = 0;
	_jmax = 0;
}; 

Flux::~Flux(){ 
	delete _random;
	if( _jmin != 0 ) delete[] _jmin;
	if( _jmax != 0 ) delete[] _jmax;
		
};
	
//Copy constructor
Flux::Flux(Flux &f) :
	_hermitePoly(f._hermitePoly),
	_fact_odds(f._fact_odds),
	_fact_d(f._fact_d),
	_binomials(f._binomials),
	_binomials_hxn(f._binomials_hxn),
	_mu_SN(f._mu_SN),
	_mu_GN(f._mu_GN),
	_n_order(f._n_order),
	_n_terms(f._n_terms),
	pi(f.pi),
	Pi(f.Pi)
{
	//Create a new random object
	//if(_random != (Random*)NULL) delete _random;
	_random = new Random();
	for(int i=0; i<4; i++)
		_ci[i] = f._ci[i];
	for(int i=0; i<16; i++){
			_ag[i] = f._ag[i];
			_xg[i] = f._xg[i];

		_jmax = new int[_n_terms];
		_jmin = new int[_n_terms];
		for(int i=0; i<_n_terms; i++){
			_jmax[i] = f._jmax[i];
			_jmin[i] = f._jmin[i];
		}
	}
};


void Flux::Setup() 
{	
	_n_order = 6;
	_n_terms = 7;
	//Pi
	pi = 4.*atan(1.); Pi = pi;
	//Calculate the array of factorial terms in the Hermite expansion
	factOdds();
	_fact_d.resize(_n_terms*2);
	for(int i=0;i<_n_terms*2; i++){_fact_d.at(i) = factorial_d(i);}
	Binomials();
	Binomials_hxn();

	//set up coefficient weighting arrays for Hermite integral
	double cit[] = {.196584,.115194,.000344,.019527};
	double agt[] = {.02715246, .06225352, .09515851, .12462897,
					.14959599, .16915652, .18260341, .18945061,
					.14959599, .16915652, .18260341, .18945061,
					.02715246, .06225352, .09515851, .12462897};
	double xgt[] = {.98940093, .94457502, .86563120, .75540441,
					.61787624, .45801678, .28160355, .09501251,
					-.61787624,-.45801678,-.28160355,-.09501251,
					-.98940093,-.94457502,-.86563120,-.75540441};
	for(int i=0; i<4; i++)
		_ci[i] = cit[i];
	for(int i=0; i<16; i++){
		_ag[i] = agt[i];
		_xg[i] = xgt[i];
	}

	//Create jmin and jmax arrays
	_jmin = new int[_n_terms];
	_jmax = new int[_n_terms];

	for(int i=0; i<_n_terms; i++){
		_jmin[i] = i%2+1;
		_jmax[i] = _n_terms - i;
	}

}

void Flux::factOdds(){
	//Calculate the factorial values for the odd terms in the hermite expansion
	int i;

	//factorial of odds.. 1*3*5... etc
	_fact_odds.resize_fill(_n_terms*2, 0.0);
	
    _fact_odds[1] = 1.;	//first value is 1
	i=0;
	for (i=3; i<_n_terms*2; i=i+2) {
		_fact_odds[i] = _fact_odds[i-2]*double(i);
	}
	//Return
	return;
}

int Flux::JMN(int i){
	//Frequently used bounds calculation
	//Treat as an array, should return [1,2,1,2,1,2]...
	//return i%2+1;
	return _jmin[i];
}

int Flux::JMX(int i){
	//Frequently used bounds calculation
	//Treat JMX like an array for conventions to match DELSOL. i.e. where in DELSOL we would call JMX(1) and 
	//expect to get back 7, here we call JMX(0) and get back 7.
	//return _n_terms - i;
	return _jmax[i];
}

int Flux::IMN(int i){
	//Frequently used bounds calculation
	//return JMN(i);
	return _jmin[i];
}

void Flux::Binomials(){
	_binomials.resize_fill(_n_terms,_n_terms,0.0);
	//Calculate the binomial coefficients
	for(int i=1; i<_n_terms+1; i++){
		for(int j=1; j<i+1; j++){
			_binomials.at(i-1,j-1) = _fact_d.at(i-1)/_fact_d.at(j-1)/_fact_d.at(i-j);
		}
	}
}

void Flux::Binomials_hxn(){
	/*
	Calculation of "binomial coefficients" (different from previously calculated array) assigned
	to the array HXN, DELSOL lines 741-754.
	
	I don't know where this comes from mathematically.
	*/

	_binomials_hxn.resize_fill(_n_terms,_n_terms,0.0);
	_binomials_hxn.at(0,0) = 1.;
	_binomials_hxn.at(1,1) = 1.;

	int i,j;
	double fi;
	for(i=3;i<_n_terms+1;i++){
		fi = float(i-2);
		_binomials_hxn.at(i-1,0) = -fi*_binomials_hxn.at(i-3,0);

		for(j=2; j<_n_terms+1; j++){
			_binomials_hxn.at(i-1,j-1) = _binomials_hxn.at(i-2,j-2) - fi*_binomials_hxn.at(i-3,j-1);
		}
	}
}

matrix_t<double> Flux::hermitePoly( double x) {
//THIS ISN"T USED
	/*Evaluate the set of Hermite polynomials described by
    H_n(x)=SUM_(p=0)^(n){h_p^n*x^n}
    Where the only non-zero h_p^n are those for which p+n=even
	
	This method is used to calculate the polynomial coefficients for the Hermite series, given a slant range
	'x' and desired order of the equation. DELSOL3 recommends N_order=6 (or 7 polynomial terms).
	
	*/
        
    //Evaluate the polynomial equations
	matrix_t<double> hermitePoly(1,_n_terms+1,0.0);
	//for (int i=0; i<N_order+1; i+=1) {_hermitePoly.push_back(0.); }
	hermitePoly[0] = 1.;
	hermitePoly[1] = x;		//Need to set H0 and H1
    for(int n=1; n<_n_terms+1; n++) {
		hermitePoly[n+1] = x*hermitePoly[n] - double(n)*hermitePoly[n-1];
	}
    
	/*
	Equations:
    H[0]=1.
    H[1]=x
    H[2]=x**2-1.
    H[3]=x**3-3*x
    H[4]=x**4-6*x**2+3
    H[5]=x**5-10*x**3+15*x
    H[6]=x**6-15*x**4+45*x**2-15.
    */
	
	return hermitePoly;
}

void Flux::initHermiteCoefs(var_map &V){
	/*
	Fills out the constant coefficients that don't change during the simulation
	*/

	//Sun shape
	hermiteSunCoefs(V, _mu_SN);

	//Error distribution coefficients
	hermiteErrDistCoefs(_mu_GN);
	
	return;
}

void Flux::hermiteSunCoefs(var_map &V, matrix_t<double> &mSun) {
	/*
	###############################################################################################
	-------WHEN TO CALL------
	Call this subroutine once to determine the sunshape coefficients. The coefficients do not 
	currently depend on solar position, time of day, weather, etc. These coefficients will be used
	in the subroutine "imagePlaneIntercept" to determine the moments of sunshape.

	---INFORMATION REQUIRED--
	* Requires the sunshape model type -> Ambient
	* the _fact_odds array must have been calculated
	* for a user-specified sunshape, the sunshape array must be filled out -> Ambient.getUserSun()

	---------OUTPUT----------
	Fills out the "mSun" (NxN) array
	###############################################################################################
	
	This function calculates the coefficients of sunshape distribution. The moments are used
    in the analytical Hermite polynomial formulation. Each sunshape moment
    is of form [Dellin, 1979]:
    mu_S_(i,j) = (1/(i+j+2) - 0.5138/(i+j+6))/(1/2 - .5138/6) * (i!!*j!!/((i+j)/2)!)*2^((i+j)/2)r_0^(i+j)
    where r_0 = (4.65e-3 mrad) * (slant range)
        
    The sun-shape can take one of several forms. The limb-darkening expres-
    sion is given by [Dellin, 1979]:
    S(r) = 1 - .5138 * (r/r_0)^4
    where 'r' is the radius about the aim poing in the image plane,
    and r_0 is given above. The r_0 value is not actually applied in this
    algorithm.
        
    Other options for sunshape include point-source (model=0), square-wave
    sun (model=2), or user-defined sunshape (model=3).
        
    For the user-defined sunshape, the user must provide a 2-D array of
    sun intensity and corresponding angular deviation from the center of the
    solar disc.
    suntype = [[angle0, intens0], [angle1, intens1] ...]
        
    Otherwise, sunshape can be declared using:
    suntype = <option number>
        
    Default sunshape is limb-darkened (option 1)    
	*/

	
	double factdum1, factdum2, dfact;

	//--Arrays and values used later
	if(mSun.ncols() != static_cast<size_t>(_n_terms) ||
	   mSun.nrows() != static_cast<size_t>(_n_terms)) {
		mSun.resize_fill(_n_terms, _n_terms, 0.0);
	}
	
	//Get the sun type from the ambient settings
	int suntype = V.amb.sun_type.mapval(); //A.getSunType();
	double sun_rad_limit = V.amb.sun_rad_limit.val; //A.getSunRadLimit();
	//Select a suntype case here 
	switch(suntype) {
	//case 0:	
    case var_ambient::SUN_TYPE::POINT_SUN:
		//---Point of sun unit intensity---
        
        for (int i=1; i<_n_terms+1; i+=2) {    //iterate 'i' from 1 to N_order by 2
            int jmax = _n_terms-i+1;              //Set the upper bound on the iteration limit for j
            for (int j=1; j<jmax+1; j+=2) {
				mSun.at(i-1,j-1) = 0.;
			}
		}
		mSun.at(0,0) = 1.;
		break;
        
	//case 1:
    case var_ambient::SUN_TYPE::LIMBDARKENED_SUN:
        //---Limb-darkened sunshape--- see DELSOL3 lines 6399-6414
		for (int i=1; i<_n_terms+1; i+=2){		//iterate 'i' from 1 to N_order by 2
            int jmax = _n_terms-i+1;			//Set the upper bound on the iteration limit for j
            factdum1 = 1.; 
			if (i>1) { factdum1 = _fact_odds[i-2]; }   //Hold on to the factorial i-1 value to avoid constant recalculation. If i==1, set to 1.
            
			for (int j=1; j<jmax+1; j+=2){ 
                factdum2 = 1.;
				if (j>1) { factdum2 = _fact_odds[j-2]; }		//Hold on to the factorial j-1 value to avoid recalc. if j==1, set to 1.
                int ij = i+j;									//Hold on to the i+j value to avoid multiple recalculations
                //Calculate the moment for this i-j combination. Algorithm taken from DELSOL3, lines 6399-6414
				dfact = double(factorial((ij)/2-1));
				mSun.at(i-1,j-1) = ((1./double(ij)-.5138/double(ij+4))/(.5-.5138/6.) * factdum1 * factdum2 / dfact /pow(2.,(ij-2)/2)) * pow(4.65e-3, ij-2) ; 
			}
		}

		break;
		
	//case 2:
    case var_ambient::SUN_TYPE::PILLBOX_SUN:
		//---Square-wave sunshape--- see DELSOL3 lines 6416-6425
		for (int i=1; i<_n_terms+1; i+=2) {	//Iterate 'i' from 1 to N_order by 2
            factdum1 = 1.; 
			if (i>1) { factdum1 = _fact_odds[i-2]; }		//Hold on to the factorial i-1 value to avoid constant recalculation. If i==1, set to 1.
            for (int j=1; j<_n_terms+1; j+=2) {
                factdum2 = 1.;
				if (j>1) {factdum2 = _fact_odds[j-2]; }		//Hold on to the factorial j-1 value to avoid recalc. if j==1, set to 1.
                int ij = i+j;									//Hold on to the i+j value to avoid multiple recalculations
                mSun.at(i-1,j-1) = (2.*factdum1*factdum2/factorial(ij/2-1)/pow(2.,(ij-2)/2)/double(ij))*pow(sun_rad_limit/1000., ij-2); //pow(4.645e-3,ij-2);
			}
		}
		break;
	//case 4: //Gaussian
	//case 5: //Buie model
	//case 3:
    case var_ambient::SUN_TYPE::GAUSSIAN_SUN:
    case var_ambient::SUN_TYPE::BUIE_CSR:
    case var_ambient::SUN_TYPE::USER_SUN:
		//---user-defined sunshape --- see DELSOL3 lines 6432-6454
            //User provides array of angle (radians) and intensity
		matrix_t<double> *user_sun;
		matrix_t<double> temp_sun;
		if(suntype == 4){	//Create a gaussian distribution
			int npt = 50;
			temp_sun.resize(npt,2);
			double ffact = 1./sqrt(2.*pi*sun_rad_limit);
			for(int i=0; i<npt; i++){
				double theta = (double)i*25./(double)npt;	//25 mrad is the limit of most pyroheliometers
				temp_sun.at(i, 0) = theta;	//mrad -> later converted to rad
				temp_sun.at(i, 1) = ffact * exp(-0.5 * pow(theta/sun_rad_limit, 2));	//Gaussian with standard deviation of sun_rad_limit
			}
			user_sun = &temp_sun;		//Assign
		}
		else if(suntype == 5){	//Create the Buie (2003) sun shape based on CSR
			//[1] Buie, D., Dey, C., & Bosi, S. (2003). The effective size of the solar cone for solar concentrating systems. Solar energy, 74(2003), 417-427.
			//[2] Buie, D., Monger, A., & Dey, C. (2003). Sunshape distributions for terrestrial solar simulations. Solar Energy, 74(March 2003), 113-122.

			double
				kappa, gamma, theta, chi;
			//calculate coefficients
			chi = V.amb.sun_csr.val; //A.getSunCSR();
			kappa = 0.9*log(13.5 * chi)*pow(chi, -0.3);
			gamma = 2.2*log(0.52 * chi)*pow(chi, 0.43) - 0.1;	//0.43 exponent is positive. See reference [2] above.

			int npt = 50;
			temp_sun.resize(npt, 2);
			for(int i=0; i<npt; i++){
				theta = (double)i*25./(double)npt;
				temp_sun.at(i, 0) = theta;
				if(theta > 4.65){
					temp_sun.at(i,1) = exp(kappa)*pow(theta, gamma)*.1;
				}
				else
				{
					temp_sun.at(i,1) = cos(0.326 * theta)/cos(0.308 * theta)*.1;
				}
			}
			user_sun = &temp_sun;
		}
		else		//Use the user-defined distribution
		{
			user_sun = &V.amb.user_sun.val; //A.getUserSun();
		}
		
		std::vector<double> azmin;
        azmin.resize(12,0.);

		//for(int i=0; i<8; i++) azmin[i] = 0.0;;			//Set up an array

        int nn = (int)user_sun->nrows()-1;		
        for (int n=1; n<nn+1; n+=1) {		//DELSOL goes 1..nn
            //The disc angle and corresponding intensity
			double disc_angle, intens; 
			disc_angle = user_sun->at(n-1,0)/1000.;
			intens = user_sun->at(n-1,1);
			//The next disc angle and intensity pair in the array
            double disc_angle_next, intens_next;
			disc_angle_next = user_sun->at(n,0)/1000.;
			intens_next = user_sun->at(n,1);
            
			 //fractional step size
			double rel_step = 1./(disc_angle_next - disc_angle);    
            //Relative to the step size, how far away are we from the centroid?
            double r_steps = disc_angle*rel_step; 
           
            for (int m=1; m<8; m+=2) {
                double fm = double(m+1);
                int L = m+1;
                double temp1 = (pow(disc_angle_next,L) - pow(disc_angle,L))/fm;
                double temp2 = (pow(disc_angle_next,m+2) - pow(disc_angle,m+2))/(fm+1.);
                azmin.at(m-1) += intens*(temp1*(1.+r_steps) - temp2*rel_step)+intens_next*(-temp1*r_steps + temp2*rel_step);
			}
		}

        double xnorm = 1.;      //Initialize the normalizing variable.. it will be reset once the new value is calculated below
        //Also initialize an array that's needed for this calculation - see DELSOL3 lines 6238-6244
		double RSPA[7][7] = {{2.,0.,1.,0.,.75,0.,.625},
                             {0.,0.,0.,0.,0.,0.,0.},
                             {1.,0.,.25,0.,.125,0.,0.},
							 {0.,0.,0.,0.,0.,0.,0.},
                             {.75,0.,.125,0.,0.,0.,0.},
                             {0.,0.,0.,0.,0.,0.,0.},
							 {.625,0.,0.,0.,0.,0.,0.}};
        for(int i=1; i<_n_terms+1; i+=2){
		    int jmax = _n_terms - i+1; 
            for (int j=1; j<jmax+1; j+=2) {
                int ij = i+j;
                mSun.at(i-1,j-1) = azmin.at(ij-2)*RSPA[i-1][j-1]/xnorm*pi; 
                xnorm = mSun.at(0,0); 
			}
		}
        mSun.at(0,0) = 1.;
		break;
		
	//default:
	//	;
	}
	
	return;
} 

void Flux::hermiteErrDistCoefs(block_t<double> &errDM)
{
	/*
	###############################################################################################
	-------WHEN TO CALL------
	Call once. Coefficients will be used to calculate the moments of the error distribution in the
	subroutine "imagePlaneIntercept" below.

	---INFORMATION REQUIRED--
	No dependencies.

	---------OUTPUT----------
	Fills in the "errDM" (N x N x 4) array for the coefficients of the error distribution moments.
	###############################################################################################

	This method calculates the moments of error distribution "G". 

	The array is N_terms x N_terms x 4 (3D array)

	From Dellin (1979), pp14:
		G represents the normalized probability distribution that the reflected
		vector t_hat will be displaced from its nominal value by and amount dt in the image
		plane (i_hat_t, j_hat_t) due to errors in the system. These displacements result 
		from the cumulative effect of many individual error sources and the magnitude
		depends on the detailed design of the system.
	
	DELSOL3 lines 6461-6483
	*/

	int i, ii, j, k, jmax, jmin;
	double temp1;

	//resize
	errDM.resize(_n_terms, _n_terms, 4);
	//zeros
	errDM.fill(0.0); 
	
	//Calculate each moment
	temp1 = 1.;
	for (i=1; i<_n_terms+1; i+=2) {
		if(i>1) { temp1 = _fact_odds[i]; }
	}
	for (i=1; i<_n_terms+1; i++) {
		jmax = JMX(i-1); 
		jmin = JMN(i-1); 
		for (j=jmin; j<jmax+1; j+=2) {
			ii = (i-1)/2+1;
			for (k=1; k<ii+1; k++) {
				temp1 = 1.;
				if(i+j > 2*k) {temp1 = _fact_odds[i+j-2*k-1]; }
				errDM.at(i-1,j-1,k-1) = temp1 * _fact_d.at(i-1)/(_fact_d.at(i-2*k+1)*_fact_d.at(k-1));
			}
		}
	}

	return;
}

void Flux::hermiteMirrorCoefs(Heliostat &H, double tht) {
	/*
	###############################################################################################
	-------WHEN TO CALL------
	This method should be called once for each heliostat template (NOT for each heliostat!).

	---INFORMATION REQUIRED--
	Heliostat geometry, tower height (to normalize information). The heliostat templates should be
	previously initialized, then when flux initialization occurs, set the mirror coefficients

	---------OUTPUT----------
	Fills out the mirror moment coefficient array "errMM", which is normalized by the tower height
	and applies to each unique heliostat geometry.
	###############################################################################################

	This method calculates the moments of mirror shape, 'M'. Moments are based on heliostat
	dimensions that are normalized by the tower height.

	This method references an established heliostat geometry and requires a receiver 
	object with an associated tower height.
	-> Heliostat
	-> Receiver

	From Dellin (1979), pp 17:
		M represents the flux produced by a perfect heliostat reflecting a point sun.
		For a flat heliostat M is given by the geometrical projection of all point
		on the mirror that are neither shaded nor blocked. WLV showed that the moments
		of a flat rectangular heliostat (including the effects of shading and blocking)
		can be evaluated analytically.

	DELSOL3 lines 6494-6525

	Round heliostats: 
	DELSOL3 lines 6529-6540

	*/

  	double wm2s, hm2s, wm, hm;
	int kl, k, l, ncantx, ncanty;
	
    var_heliostat *V = H.getVarMap();

	//Assign some values from the heliostat instance
	wm = V->width.val; 
	hm = V->height.val;
	ncantx = V->n_cant_x.val;
	ncanty = V->n_cant_y.val; 
	
	//Calculate the effective mirror width to use for 
	if(V->is_faceted.val){
		//Use the smaller cant panel dimensions for image calculations
		wm2s = 0.; hm2s = 0.;
		//Use average panel width/height
		wm2s = 0.; hm2s = 0.;
		double ff = 1./((double)(ncantx*ncanty)*2.*tht);
		for(int r=0; r<ncanty; r++){
			for(int c=0; c<ncantx; c++){
				wm2s += H.getPanel(r,c)->getWidth()*ff;
				hm2s += H.getPanel(r,c)->getHeight()*ff;
			}
		}
	}
	else
	{
		//Use the large structure width (or no canting is specified) for image calculations
		wm2s = wm/(tht * 2.);
		hm2s = hm/(tht * 2.);
	}
	
	matrix_t<double> *errMM = H.getMirrorShapeNormCoefObject();
	errMM->resize(_n_terms, _n_terms);
	errMM->fill(0.0);
	
	//Calculate the moments depending on whether the heliostats are circular or rectangular
	if (V->is_round.mapval() == var_heliostat::IS_ROUND::ROUND) {
		//----Round heliostats----
		double temp1=1.;
		for (k=1; k<_n_terms+1; k+=2) {
			for (l=1; l<_n_terms+1; l+=2) {
				if(k > 1) {temp1 = _fact_odds[k-2];}
				if(l > 1) {temp1 = temp1 * _fact_odds[l-2];}
				kl = k+l;
				//Calculate the moments
				errMM->at(k-1,l-1) = pow(wm2s, kl)*pi/double(kl)*temp1/_fact_d.at(kl/2-1)*pow(2., 1-(kl-2)/2);
			}
		}
	}
	else {
		//----Rectangular heliostats ----
		
        double wm2sk = wm2s;        //is pow(wm2s,k)
        double wm2s2 = wm2s*wm2s;
		for(k=1; k<_n_terms+1; k+=2)
        {
            wm2sk *= wm2s2;

            double hm2sl = hm2s;    //is pos(hm2s,l)
            double hm2s2 = hm2s*hm2s;
            for(l=1; l<_n_terms+1; l+=2){
				kl = k*l;
				//Calculate the moments
                hm2sl *= hm2s2;
				errMM->at(k-1,l-1) = 4./double(kl)*wm2sk*hm2sl;
			}
		}
		//Potentially add other geometries here. No other geometries are defined in DELSOL3.
	}
	return;
	
}

double Flux::imagePlaneIntercept(var_map &V, Heliostat &H, Receiver *Rec, Vect *Sun) {
	/* 
	###############################################################################################
	-------WHEN TO CALL------
	Call this method to evaluate the sunshape, heliostat error distribution, and heliostat mirror
	shape distribution for EACH HELIOSTAT IN THE FIELD. Output will change when any input except
	tower height changes. 

	*This is the main optical intercept algorithm that should be called during simulation. This 
	calls the other Hermite flux characterization algorithms.*

	---INFORMATION REQUIRED--
	* Heliostat geometry
	* Tower height
	* Receiver geometry
	* Sun position
	* Evaluated Hermite flux coefficients (mirror geometry, error distribution, sunshape)
	
	---------OUTPUT----------
	Calculates:
		* moments of sunshape "_mu_S"
		* moments of mirror shape "_mu_M"
		* moments of error distribution "_mu_G"
		* convolved moments "_mu_F"
		--> All of these are normalized by the tower height
	Calls the subroutine chain:
		hermiteIntEval() -> hermiteIntegralSetup() -> hermiteIntegral()
	...and returns the result (double) of the chain.
	###############################################################################################	

	This subroutine calculates the moments of the sunshape, heliostat error
	distribution, and heliostat shape on the image plane and then convolves
	these to find the moments of the heliostat image. 

	This method is derived from the DELSOL3 subroutine "MOMENT", beginning on 
	line 8249.
	*/

	//First check to make sure the heliostat can see the aperture plane. If not, don't bother.
	PointVect NV;
	Rec->CalculateNormalVector(*H.getLocation(), NV);
	bool view_ok = false;
	
    double h_rad = H.getRadialPos();		//[m] Get the heliostat radius from the tower

    /*if(Rec->getReceiverType() == Receiver::REC_TYPE::CYLINDRICAL
        && h_rad < Rec->getReceiverWidth()/2.)
        return 0.0;
   */     
	Vect rnorm;
	rnorm.Set(NV.i, NV.j, NV.k);
	if(Toolbox::dotprod(rnorm, *H.getTowerVector()) < 0.) view_ok = true;	//We actually want the opposite of the tower vector, so take negative dot products to be ok.
	if(! view_ok) return 0.;
	//Otherwise, there's a satisfactory option so continue


	//Variable declarations
	int i, j, jmax, jmin;
	double slant, tht;

	//--------Calculate the moments of the sun-----------
	tht = V.sf.tht.val; 

	//vector to store calculated sun moments
	matrix_t<double> *mu_S = H.getSunShapeCoefObject();
	mu_S->resize_fill(_n_terms, _n_terms, 0.0);
	
	//Pointer to the relevant receiver
	//Receiver *Rec = SF.getReceivers()->at(rec);
    var_receiver* Rv = Rec->getVarMap();
    int rec_type = Rv->rec_type.mapval();

	//Calculate the range-dependent expansion coefficients
	//Calculate the slant range
	if(rec_type == var_receiver::REC_TYPE::EXTERNAL_CYLINDRICAL ){	//External cylindrical 
		double rec_width = Receiver::getReceiverWidth( *Rv );;
		double rec_opt_ht = Rv->optical_height.Val();
        double hmr = h_rad - rec_width*0.5;
		slant = sqrt( hmr*hmr + rec_opt_ht*rec_opt_ht ) / tht;	//[tht] the distance to the receiver
	}
	else{
		double rec_opt_ht = Rv->optical_height.Val();
		slant = sqrt( h_rad*h_rad + rec_opt_ht*rec_opt_ht )/tht;	//normalized. This is the default for all cavity/flat-plate type receivers
	}

	matrix_t<double> srange; srange.resize_fill(1, _n_terms, 0.);
	srange[0] = 1.;
	//..and the actual array (DELSOL line 7349):
	for(i=1; i<_n_terms; i++) { srange[i] = srange[i-1]*slant; }
	
	for(i=1; i<_n_terms+1; i+=2) {
		jmax = JMX(i-1);
		jmin = 2-i+2*(i/2);

		for(j=jmin; j<jmax+1; j+=2) {
			//RUS -> hmSun
			mu_S->at(i-1,j-1) = srange[i+j-2] * _mu_SN.at(i-1,j-1); //The sun moments, DELSOL 8299
		}
	}
	
	//---------Calculate moments of the error distribution -------------
	//DELSOL 8304-8339


	/*
	The error distribution moments are evaluated using the formulation from Dellin (1979)
	equations 3-6 and 3-7 on page 16. The angles for this equation are defined according
	to the figures on page 38-39 of Dellin, and also on page 26-27 of the DELSOL manual.

	Equation 3-6 contains terms for sigma_x^2, sigma_y^2, and rho, each of which are evaluated
	by their formulations described in Eq. 3-7 of Dellin. These three relationships contain 
	several phrases that are identical, so to save computational effort, they are assigned
	unique variable names.

	The expansion coefficients are calculated in the "B(:,:)" array.

	Description of variables and angles from DELSOL:
	---Nomenclature system	---
	-Char 1	
	C	Cosine
	S	Sine
	-Char 2	
	S	Sun vector
	T	Helio-to-receiver vector
	N	Heliostat tracking vector
	-Char 3	
	P	with reference to zenith (polar)
	A	with reference to azimuth
	Suffix	
	ANG	Array
	Variable	Primary description---
	AZMANG		Heliostat aiming azimuth angle
	CBNS		CSA*CNA+SNA*SSA
	CBNT		j_hat component of the normal tracking vector
	CDEC		Cosine of the declination angle
	CLAT		cosine of latitude
	CNA			check CTA
	CNP			Cosine of the tracking normal zenith angle
	CSA			Equals CSAANG(I,J)
	CSAANG		Cosine of solar azimuth angle
	CSP			Equals CSPANG(I,J)
	CSPANG		Cosine of solar zenith angle
	CT			Cosine of hour angle
	CTA			Equals CTAANG(L)
	CTAANG		cosine of heliostat location azimuth angle
	CTP			Equals CTPANG(K)
	CTPANG		cosine of TPANG
	ELVANG		Heliostat aiming elevation (?) angle
	HCOS		Heliostat cosine loss
	HINSOL	
	I			Iterator, 1 -> Number of azimuth angles in table
	J			Iterator, 1-> Number of zenith angles in table
	K			Iterator, 1-> Number of azimuthal zones
	L			Iterator, 1-> Number of radial zones
	SBNS		SNA*CSA-CNA*SSA
	SBNT		i_hat component of the normal tracking vector
	SDEC		Sine of the declination angle
	SLAT		sine of latitude
	SNA			Sine of the tracking azimuth angle
	SNP			Sine of the tracking zenith angle
	SSA			Equals SSAANG(I,J)
	SSAANG		Sine of solar azimuth angle
	SSP			Equals SSPANG(I,J)
	SSPANG		Sine of solar zenith angle
	STA			Equals STAANG(L)
	STAANG		sine of heliostat location azimuth angle
	STP			Equals STPANG(K)
	STPANG		sin of TPANG
	T			Hour angle
	TPANG		Heliostat to receiver zenith angle
	UAZ			Array of user-defined solar azimuth angles
	UEL			Array of user-defined solar ZENITH angles
	*/


	//calculate relevant angles

	Vect 
		n_hat, //tracking vector
		t_hat, //heliostat to tower vector
		z_hat, //zenith unit vector - values are 0,0,1 by default
		s_hat;	//Solar position vector
	z_hat.Set(0.,0.,1.);
	
	n_hat = *H.getTrackVector();		//heliostat tracking vector
	t_hat = *H.getTowerVector();		//heliostat-to-receiver vector
	s_hat = *Sun;		//Sun position vector
	
    double cos_s_zen = s_hat.k; //cos(theta_s_zen),			//Cosine of the solar zenith angle (CSP)
	double sin_s_zen = sqrt(1.-s_hat.k*s_hat.k); //sin(theta_s_zen),			//Sine of the solar zenith angle (SSP)
    sin_s_zen = sin_s_zen == 0. ? 1.e-6 : sin_s_zen;
	double cos_s_az = s_hat.j/sin_s_zen; //cos(theta_s_az),				//Cosine of the solar azimuth angle (CSA)
	double sin_s_az = s_hat.i/sin_s_zen; //sin(theta_s_az);				//Sine of the solar azimuth angle (SSA)
	
	////double theta_n_zen= acos(n_hat.k);	//zenith angle of the tracking vector
	//double cos_n_zen = n_hat.k;				//cos of the zenith angle of tracking vector (CNP)
	//double sin_n_zen = sqrt(1.-n_hat.k*n_hat.k); //sin(theta_n_zen);				//sine of the zenith angle of the tracking vector (SNP)
 //   sin_n_zen = sin_n_zen == 0. ? 1.e-6 : sin_n_zen;

	////theta_n_az = atan2(n_hat.i;n_hat.j);			//Azimuth angle of the tracking vector (0..360)
	//double sin_n_az = n_hat.i/sin_n_zen; //sin(theta_n_az);
	//double cos_n_az = n_hat.j/sin_n_zen; //cos(theta_n_az);

	////double theta_t_zen = acos(dotprod(t_hat, z_hat)); 
	//double cos_t_zen = dotprod(t_hat, z_hat); //cos of zenith of helio-tower vector
	//double sin_t_zen = sqrt(1.-cos_t_zen*cos_t_zen); //sin(theta_t_zen);
 //   sin_t_zen = sin_t_zen == 0. ? 1.e-6 : sin_t_zen;
	////double theta_t_az = atan2(t_hat.i,t_hat.j); //azimuth angle of the heliostat-to-receiver vector
	//double sin_t_az = t_hat.i/sin_t_zen; //(theta_t_az);
	//double cos_t_az = t_hat.j/sin_t_zen; //cos(theta_t_az);

    //-----------------------------------------------------------------------------

    double theta_n_zen= acos(n_hat.k);	//zenith angle of the tracking vector
	double cos_n_zen = n_hat.k;				//cos of the zenith angle of tracking vector (CNP)
	double sin_n_zen = sin(theta_n_zen);				//sine of the zenith angle of the tracking vector (SNP)
    sin_n_zen = sin_n_zen == 0. ? 1.e-6 : sin_n_zen;

	double theta_n_az = atan2(n_hat.i,n_hat.j);			//Azimuth angle of the tracking vector (0..360)
	double sin_n_az = sin(theta_n_az);
	double cos_n_az = cos(theta_n_az);

	double theta_t_zen = acos(dotprod(t_hat, z_hat)); 
	double cos_t_zen = dotprod(t_hat, z_hat); //cos of zenith of helio-tower vector
	double sin_t_zen = sin(theta_t_zen);
    sin_t_zen = sin_t_zen == 0. ? 1.e-6 : sin_t_zen;
	double theta_t_az = atan2(t_hat.i,t_hat.j); //azimuth angle of the heliostat-to-receiver vector
	double sin_t_az = sin(theta_t_az);
	double cos_t_az = cos(theta_t_az);
	
	//---------------------------------------------------	
	//Calculate the heliostat cosine loss
	double              // sqrt(2)/2
		eta_cosine = 0.7071067811865*sqrt(1.+cos_s_zen*cos_t_zen+sin_s_zen*sin_t_zen*(cos_t_az*cos_s_az + sin_t_az*sin_s_az));
	//double eta_test = dotprod(s_hat, n_hat);

    //get heliostat inputs
    var_heliostat *Hv = H.getVarMap();

	//get error terms - See Kistler pp. 184 for definition
	double err_angular[2], err_surface[2], err_reflected[2];
	
    err_angular[0] = Hv->err_azimuth.val;
    err_angular[1] = Hv->err_elevation.val;

    err_surface[0] = Hv->err_surface_x.val;
    err_surface[1] = Hv->err_surface_y.val;

    err_reflected[0] = Hv->err_reflect_x.val;
    err_reflected[1] = Hv->err_reflect_y.val;
    
	//Depending on the canting method, calculate the A[], B[] arrays differently.
	double A11, A12, A21, A22, B11, B12, B21, B22;
	//---Are the heliostats canted?
	int cant_method = Hv->cant_method.mapval(); //{0=none, -1=on-axis at slant, 1=on-axis at user def., 3=off-axis at hour-day}
	
	//reused terms:
	//SAVE=SIGAZ2*SNP**2+SIGSX2 | 8304
	double term1 = err_angular[0] * sin_n_zen; //first reused term
    term1 *= term1;
    term1 += err_surface[0]*err_surface[0];

	//SAVE2=SIGEL2+SIGSY2
	double term2 = err_angular[1]*err_angular[1] + err_surface[1] * err_surface[1];  //second reused term
    
    switch (cant_method)
    {
    //case Heliostat::CANT_TYPE::FLAT:
    //case Heliostat::CANT_TYPE::AT_SLANT:
    //case Heliostat::CANT_TYPE::ON_AXIS_USER:
    case var_heliostat::CANT_METHOD::NO_CANTING:
    case var_heliostat::CANT_METHOD::ONAXIS_AT_SLANT:
    case var_heliostat::CANT_METHOD::ONAXIS_USERDEFINED:
    {
        //Everything except individual off-axis canting and vector canting

		//A(1,1)=CBNT 
		A11 = cos_t_az * cos_n_az + sin_n_az * sin_t_az;
		//A(1,2)=CNP*SBNT
		A12 = cos_n_zen * (sin_n_az * cos_t_az - cos_n_az * sin_t_az);
		//A(2,1)=-CTP*SBNT
		A21 = -cos_t_zen * A12/cos_n_zen;
		//A(2,2)=SNP*STP+CNP*CTP*CBNT
		A22 = sin_n_zen * sin_t_zen + cos_n_zen * cos_t_zen * A11;
		
		//Calculate the "B" coefficient array, uses "A" array
		B11 = 2.*A22;
		B12 = -2.*A21;
		B21 = -2.*A12;
		B22 = 2.*A11;

        break;
    }
    //case Heliostat::CANT_TYPE::AT_DAY_HOUR:
    case var_heliostat::CANT_METHOD::OFFAXIS_DAY_AND_HOUR:
    {
        //case 3: individual off-axis canting at defined time
		//Calculate sun angles at canting time 7097
		double cant_day = Hv->cant_day.val;
		double cant_hour = Hv->cant_hour.val;
		double pi2 = pi/180.;

		double dtemp = 2. * Pi / 365.24 * (cant_day + 284.);
		dtemp += 0.007133*sin(dtemp) + 0.032680*cos(dtemp) - 0.00318*sin(2.*dtemp)+0.000145*cos(2.*dtemp);
		
        double sdec = sin(23.442274*pi2)*sin(dtemp);
        double cdec = sqrt(1.-sdec*sdec);
        double ct = cos(cant_hour*15.*pi2);
        double st = sin(cant_hour*15.*pi2);
        double lat = V.amb.latitude.val * D2R; 
        double slat = sin(lat);
        double clat = cos(lat);
        double cspcan =  slat * sdec + clat * cdec * ct;
        double sspcan = sqrt(1. - cspcan*cspcan);
        double ssacan = 0.;
        double csacan;		

		if(sspcan > 1.e-5) ssacan = st * cdec / sspcan;
		csacan = sqrt(1.-ssacan*ssacan);
		if(ssacan*ssacan > 1.e-4) csacan = ssacan * (slat*ct - clat*sdec/cdec)/(st+1.e-8);
				
		//7352
		//	CSP=CSPCAN
		cos_s_zen = cspcan;
		//	SSP=SSPCAN
		sin_s_zen = sspcan;
		//	SSA=SSACAN
		sin_s_az = ssacan;
		//	CSA=CSACAN
		cos_s_az = csacan;
		//	HCOS=SQ2*SQRT(1.+CSP*CTP+SSP*STP*(CTA*CSA+STA*SSA))
		eta_cosine = sqrt(2.)*sqrt(1.+cos_s_zen*cos_t_zen + sin_s_zen*sin_t_zen*(cos_t_az*cos_s_az + sin_t_az*sin_s_az));
		//	CNP=(CSP+CTP)/HCOS/2.
		cos_n_zen = (cos_s_zen + cos_t_zen)/eta_cosine/2.;
		//	SNP=SQRT(1.-CNP**2)
		sin_n_zen = sqrt(1.-cos_n_zen*cos_n_zen);
		//	CNA=CTA
		cos_n_az = cos_t_az;
		//	SNA=STA
		sin_n_az = sin_t_az;
		//	IF (CNP.GT.0.999) GO TO 1750
		if(!(cos_n_zen > 0.999)){
			//	CNA=(SSP*CSA+STP*CTA)/HCOS/SNP/2.
			cos_n_az = (sin_s_zen*cos_s_az + sin_t_zen*cos_t_az)/eta_cosine/sin_n_zen/2.;
			//	SNA=(SSP*SSA+STP*STA)/HCOS/SNP/2.
			sin_n_az = (sin_s_zen*sin_s_az + sin_t_zen*sin_t_az)/eta_cosine/sin_n_zen/2.;
		//1750  CONTINUE
		}
		//	SBNT=SNA*CTA-CNA*STA
		//	CBNT=CTA*CNA+SNA*STA
		//	CBNS=CSA*CNA+SNA*SSA
		//	SBNS=SNA*CSA-CNA*SSA
		//	A(1,1)=CBNT
		A11 = cos_t_az*cos_n_az + sin_n_az*sin_t_az;
		//	A(1,2)=CNP*SBNT
		A12 = sin_n_zen*(sin_n_az*cos_t_az - cos_n_az*sin_t_az);
		//	A(2,2)=SNP*STP+CNP*CTP*CBNT
		A22 = sin_n_zen*sin_t_zen + cos_n_zen*cos_t_zen*A11;
		//	A(2,1)=-CTP*SBNT
		A21 = -cos_t_zen*sin_n_zen*(sin_n_az*cos_t_az - cos_n_az*sin_t_az);
		//	B(1,1)=2.*A(2,2)
		B11 = 2.*A22;
		//	B(1,2)=-A(2,1)*2.
		B12 = A21*2.;
		//	B(2,1)=-A(1,2)*2.
		B21 = -A12*2.;
		//	B(2,2)=2.*A(1,1)
		B22 = 2.*A11;


        break;
    }
    //case Heliostat::CANT_TYPE::USER_VECTOR:
    case var_heliostat::CANT_METHOD::USERDEFINED_VECTOR:
        throw spexception("User-vector cant method is not fully implemented (imagePlaneIntercept()).");






        break;
    default:
        throw spexception("Unspecified cant method is not implemented (imagePlaneIntercept()).");
        break;
    }
    	
	//if the focal distance of the heliostat is not equal to the slant range, do additional 
	//calculations here
	//FLAG 

	double delta_1, delta_2, delta_a, sigma_x, sigma_y;
	//DELTA1=SIGTX2+SAVE*B(1,1)**2+SAVE2*B(1,2)**2
	delta_1 = err_reflected[0]*err_reflected[0] + term1 * B11 * B11 + term2 * B12 * B12;
	//DELTA2=SIGTY2+SAVE*B(2,1)**2+SAVE2*B(2,2)**2
	delta_2 = err_reflected[1]*err_reflected[1] + term1 * B21 * B21 + term2 * B22 * B22;
	//DELTAA=SAVE*B(1,1)*B(2,1)+SAVE2*B(2,2)*B(1,2)
	delta_a = term1 * B11 * B21 + term2 * B22 * B12;
	//SIGX=SRANGE(2)*SQRT(DELTA1)
	sigma_x = srange.at(1) * sqrt(delta_1);	//Standard deviation of the image error in the X-direction
	//SIGY=SRANGE(2)*SQRT(DELTA2)
	sigma_y = srange.at(1) * sqrt(delta_2);	//Standard deviation of the image error in the Y-direction
	
	//The argument to part of the RHO equation (sigma_a) determines the remaining calculations. 
	//If the value of sigma_a^2 is zero, handle with a separate method.
	double rho, rho_tmp, rs2, rs, rho_sq;
	matrix_t<double>* mu_G = H.getErrorDistCoefObject();
	mu_G->resize_fill(_n_terms, _n_terms, 0.0);

	if(delta_a*delta_a < 1.e-20 ) {
		//The argument delta_a^2 is very close to zero, handle separately
		double fact1, fact2;
		//Calculate the factorial terms
		matrix_t<double> factarr;
		factarr.resize_fill(_n_terms, _n_terms, 0.0);
		fact1 = 1.;	//use for calculating the factorial terms || 6461-6470
		for(i=1; i<_n_terms+1; i+=2) {
			if(i>1) {fact1 = _fact_odds[i-2]; }
			fact2 = 1.;
			for(j=1; j<_n_terms+1; j+=2) {
				if(j>1) {fact2 = _fact_odds[j-2]; }
				factarr.at(i-1,j-1) = fact1 * fact2;
			}
		}
		
		//Calculate the moments
		for(i=1; i<_n_terms+1; i++) {	//8331-8337
			term2 = double(i - 2*(i/2));
			term1 = pow(sigma_x, i-1);
			jmax = JMX(i-1);
			jmin = JMN(i-1);
			
			//Calculate the moments of mu_G
			for(j=jmin; j<jmax+1; j+=2) {
				mu_G->at(i-1,j-1) = factarr.at(i-1,j-1) * term1 * pow(sigma_y, j-1) * term2;
			}
		}
	}
	else {
		rho_tmp = delta_a / sqrt(delta_1 * delta_2) + 1.e-10;
		rho_sq = rho_tmp*rho_tmp;	//used later
		rho = (1. - rho_sq)/2.;
		
		for(i=1; i<_n_terms+1; i++) {
			//set iteration bounds
			jmax = JMX(i-1); 
			jmin = JMN(i-1);	//Array of 1,2,1,2...

			for(j=jmin; j<jmax+1; j+=2) {
				rs2 = rho_sq;	//initialize variable, used in moment calculation
				rs = 1./rho;
				term1 = 0.;		//Another temp variable, initialize here
				int k_ct = (i-1)/2 + 1; //Iteration limit for moment calculation

				for(int k=1; k<k_ct+1; k++){
					rs = rs*rho;
					rs2 = rs2/rho_sq;
					term1 += _mu_GN.at(i-1,j-1,k-1) * rs * rs2;		//_mu_GN are the constant error distribution coefficients calculated once (see hermiteErrDistCoefs() )
				}

				//Calculate the moments mu_G
				mu_G->at(i-1,j-1) = pow(sigma_x * rho_tmp, i-1) * pow(sigma_y, j-1) * term1 ;
			}
		}
	}
	//-----end moments of error distribution ---------------


	//------ begin moments of the mirror projection "M"-----
	/*
	8343-8377	Flat, Focused, and Canted Heliostats

	For a flat heliostat, projection along t_hat of the differential mirror element (dxm, dym)
	at (xm,ym) onto the image plane located at the receiver is:
		xt = A11*xm + A12*ym
		yt = A21*xm + A22*ym
	Aij are defined by Eq (A-2) (Dellin) and are functions of time and position with respect to 
	the tower. By definition, the center of the heliostat (xm=0, ym=0) is projected onto the 
	origin of the image plane.

	Focusing is induced by displacements delta_x and delta_y in the mirror along the i_n j_n 
	directions. With displacements, the size of the image becomes:
		x = A11*xm + A12*ym + B11*R*delta_x + B12*R*delta_y
		y = A21*xm + A22*ym + B21*R*delta_x + B22*R*delta_y
	where R is the slant range from heliostat and Bij are defined in Eq A-6 (Dellin). 

	*/
	matrix_t<double> 
		mu_Msave, //copy of mu_M to save for other calculations
		*errm_M;	//Hermite polynomial moments of the mirror projection array
	
	matrix_t<double>* mu_M = H.getMirrorShapeCoefObject();
	mu_M->resize_fill(_n_terms,_n_terms,0.0);
	mu_Msave.resize_fill(_n_terms,_n_terms,0.0);

	//hermiteMirrorCoefs(errm_M, H, tht);	//Get the moments
	errm_M = H.getMirrorShapeNormCoefObject();
	
	double 
		xfocal  =-tht/H.getFocalX(),		//in Delsol, this value is also multiplied by XFOCUS, (=0 for no focusing, =1 for focusing)
		yfocal = -tht/H.getFocalY(),		//same as above line, but YFOCUS
		E11 = A11+B11*xfocal*srange[1]/2.+1.e-10,
		E12 = A12+B12*yfocal*srange[1]/2.+1.e-10,
		E21 = A21+B21*xfocal*srange[1]/2.+1.e-10,
		E22 = A22+B22*yfocal*srange[1]/2.+1.e-10;

	double step = E21*E21/(E22*E22);
	double temp_res, e_ratio, start[2], binoms;	//temporary result, updated in iteration
	double S11, S22;	//s terms, not sure what these represent but seem to be cross terms- check Dellin
	
	int k, l, l_min, ij, ijk, mstep;
	//Loop over each term in the polynomial
	for(i=1; i<_n_terms+1; i++){
		int jmax = JMX(i - 1) + 1;
		int jmin = JMN(i - 1);
		for(j=jmin; j<jmax; j+=2){		//the bounds on j are the actual calculations for the jmax, jmin values in the DELSOL code 8349-8350
			temp_res = 0.;
			ij = i+j+1;
			if(j>1){mstep=1;} else {mstep=2;}

			S11 = pow(E12, i-1);
			e_ratio = pow(E11/E12, mstep);
			start[0] = pow(E22, j-1);
			start[1] = start[0]*E21/E22;

			l_min = IMN(i - 1);

			for(k=1; k<i+1; k+=mstep){
				ijk = ij - k;
				binoms = _binomials.at(i-1,k-1)*S11; //e_ratio;  moved the s11 calculation after this to reduce number of operations
				S11 *= e_ratio;
				S22 = start[l_min - 1];
				for(l=l_min; l<j+1; l+=2){
					S22 *= step;
					temp_res += binoms*S22*_binomials.at(j-1,l-1)*errm_M->at(k+l-2,ijk-l-1)/step;
				}
			}

			//calculate the moments to save
			mu_M->at(i-1,j-1) = temp_res * eta_cosine;
			mu_Msave.at(i-1,j-1) = mu_M->at(i-1,j-1);	//save a copy for other calcs
		}
	}
	matrix_t<Reflector> *panels = H.getPanels();	//Get the matrix of all of the panels
	//Reflector *panel;	//A pointer to a single panel in the matrix

    int ncanty = Hv->n_cant_y.val;
	int ncantx = Hv->n_cant_x.val;

	double gcanta, gcanty, gcantx, gcantb, tempmult;
	
	if(Hv->is_faceted.val){
		//No canting=0;On-axis at slant=-1;On-axis, user-defined=1;Off-axis, day and hour=3; User-defined vector=4
		//Are the heliostats canted on or off axis?
        switch (cant_method)
        {
        //case Heliostat::CANT_TYPE::FLAT:
        case var_heliostat::CANT_METHOD::NO_CANTING:
            //No canting
			gcanta = 0.;
			gcantx = 0.;
			gcantb = 0.;
			gcanty = 0.;
            break;
        //case Heliostat::CANT_TYPE::AT_SLANT:
        case var_heliostat::CANT_METHOD::ONAXIS_AT_SLANT:
            //method -1 for on-axis at default slant range (6 tht)
			gcanta = 0.;
			gcanty = 0.;
			//gcantx = -.5/6.*srange[1];	//default range is 6.0 -- see DELSOL 835
			gcantx = -.5*tht/H.getSlantRange()*srange[1];
			gcantb = gcantx;
            break;
        //case Heliostat::CANT_TYPE::ON_AXIS_USER:
        case var_heliostat::CANT_METHOD::ONAXIS_USERDEFINED:
            //method 1 for on-axis at user defined length
			gcanta = 0.;
			gcanty = 0.;
			gcantx = -.5*tht/Hv->cant_radius.Val()*srange[1];	
			gcantb = gcantx;
            break;
        //case Heliostat::CANT_TYPE::AT_DAY_HOUR:
        case var_heliostat::CANT_METHOD::OFFAXIS_DAY_AND_HOUR:
            //off-axis user defined time
			tempmult = 1./(4.*eta_cosine*slant)*srange[1];
			gcantx = (A21*B12 - A11*B22)*tempmult;
			gcanta = (A22*B12 - A12*B22)*tempmult;
			gcanty = (A11*B21 - A21*B11)*tempmult;
			gcantb = (A12*B21 - A22*B11)*tempmult;
            break;
        //case Heliostat::CANT_TYPE::USER_VECTOR:
        case var_heliostat::CANT_METHOD::USERDEFINED_VECTOR:
            throw spexception("User-vector cant method is not fully implemented (imagePlaneIntercept()).");
        default:
            throw spexception("Unspecified cant method is not implemented (imagePlaneIntercept()).");
        }
        
		matrix_t<double> xcent, ycent;
		xcent.resize(ncanty,ncantx);
		ycent.resize(ncanty,ncantx);
		block_t<double> xc, yc;
		xc.resize(ncanty,ncantx,_n_terms);
		yc.resize(ncanty,ncantx,_n_terms);
		PointVect *ploc;
		
		//calculate optical terms for each canted panel
		double
			ploc_x, ploc_y, xcentij, ycentij;
		double thtinv = 1./tht;
		for(i=0; i<ncanty; i++){	//over the rows of panels
			for(j=0; j<ncantx; j++){	//over the columns of panels
				//panel = &panels->at(i,j);
				ploc = panels->at(i, j).getOrientation();
				ploc_x = ploc->x * thtinv;
				ploc_y = ploc->y * thtinv;
				xcent.at(i,j) = (A11 + B11*gcantx + B12*gcanty)*ploc_x + (A12 + B11*gcanta + B12*gcantb)*ploc_y;
				ycent.at(i,j) = (A21 + B21*gcantx + B22*gcanty)*ploc_x + (A22 + B21*gcanta + B22*gcantb)*ploc_y;
				xc.at(i,j,0) = 1.;
				yc.at(i,j,0) = 1.;
			
				//loop over each term in the Hermite expansion
				xcentij = xcent.at(i, j);
				ycentij = ycent.at(i, j);
				for(k=1; k<_n_terms; k++){
					xc.at(i,j,k) = xc.at(i,j,k-1)*xcentij;
					yc.at(i,j,k) = yc.at(i,j,k-1)*ycentij;
				}
			}
		}
		
		//Adjust the image for multiple facets
		//DELSOL 8393-8407
		int m, n, ii, jj, jp1, ip1, im1, iim1, jm1, jjm1;		//reused indices
		for(i=1; i<_n_terms+1; i++){
			jmin = JMN(i-1);
			jmax = JMX(i-1);
			im1 = i-1;
			ip1 = i+1;
			for(j=jmin; j<jmax+1; j+=2){
				jp1 = j+1;
				jm1 = j-1;
							
				temp_res = 0.;
				if(j>1){mstep=1;} else {mstep=2;}

				for(m=0; m<ncanty; m++){
					for(n=0; n<ncantx; n++){
						for(ii=1; ii<ip1; ii+=mstep){
							iim1 = ii-1;
							S11 = _binomials.at(im1, iim1)*xc.at(m,n,i-ii);
							//for(jj=JMN(ii-1); jj<jp1; jj+=2){
							for(jj=iim1%2+1; jj<jp1; jj+=2){
								jjm1 = jj-1;
								temp_res += S11*_binomials.at(jm1,jjm1)*mu_Msave.at(iim1,jjm1)*yc.at(m,n,j-jj);
							}
						}
					}
				}
				//moments of M
				mu_M->at(im1,jm1) = temp_res;
			}
		}
		
	}
	//------------end of mirror projection moments
	
	//-----Combine moments of S, G, and M to get moments of F---------
	//DELSOL3 8412-8450
	matrix_t<double>* mu_F = H.getFluxMomentsObject();
	mu_F->resize_fill(_n_terms, _n_terms, 0.0);
	/*int nrf=0;
	for(i=1;i<_n_terms+1;i++){ for(j=JMN(i-1); j<JMX(i-1)+1; j+=2){nrf++;} }
	matrix_t<double>* hc_tht = H.getHermiteNormCoefObject();
	hc_tht->resize_fill(nrf,4,0.0);	//Hermite coef. dependence on tower height*/

	//double tsave[7];
	double binom_temp0, binom_temp1, ugs;
	int ipak=0, nmin, nmax, lmin, mk, ki, kp1, km1;
	double comb_ord = mu_S->at(0,0) * mu_G->at(0,0) * mu_M->at(0,0);	//combined moments at the ordinate
	//
	double muS, muM,
		comb_ord_inv = 1./comb_ord;
	//
	for(int m=1; m<_n_terms+1; m++){
		nmin = JMN(m-1);
		nmax = JMX(m-1);

		for(int n=nmin; n<nmax+1; n+=2){
			ipak ++;
			temp_res = 0.;

			/*tsave[0] = 0.;
			tsave[2] = 0.;
			tsave[4] = 0.;
			tsave[6] = 0.;*/

			if(n>1){mstep=1;} else {mstep=2;}

			for(int k=1; k<m+1; k+=mstep){
				lmin = JMN(k-1);
				binom_temp0 = _binomials.at(m-1,k-1);
				mk = m-k+1;
				kp1 = k+1;
				km1 = k-1;
				for(int l=lmin; l<n+1; l+=2){
					ugs = mu_G->at(mk-1,n-l)*binom_temp0*_binomials.at(n-1,l-1);

					for(i=1; i<kp1; i+=2){
						ki = kp1-i;
						binom_temp1 = _binomials.at(km1,i-1);

						for(j=1; j<l+1; j+=2){
							muS = mu_S->at(i-1,j-1);
							muM = mu_M->at(ki-1,l-j);
							term1 = _binomials.at(l-1,j-1)*binom_temp1*muS*muM*ugs;
							//tsave[ki+l-j-1] += term1*comb_ord_inv;
							temp_res += term1;
						}
					}
				}
			}
			//normalize for unit flux
			mu_F->at(m-1,n-1) = temp_res*comb_ord_inv;
			/*hc_tht->at(ipak-1, 0) = tsave[0];
			hc_tht->at(ipak-1, 1) = tsave[2];
			hc_tht->at(ipak-1, 2) = tsave[4];
			hc_tht->at(ipak-1, 3) = tsave[6];*/	//save these coefficients for use in other optimization runs
		}
	}
	
	//-------end combination-------------


	//Now evaluate the hermite coefficients and assign the spillage intercept value
	double heval = hermiteIntEval(H, Rec);

    if(Rv->rec_type.mapval() == var_receiver::REC_TYPE::EXTERNAL_CYLINDRICAL 
        && h_rad < Receiver::getReceiverWidth( *Rv )/2.)
        return 0.0;
    else
        return heval;

	// ** probably here is where we should call the flux density calculation. The flux will depend on 
	// information calculated in the hermiteIntEval method. _hcoef array.
	// D311
}

double Flux::hermiteIntEval(Heliostat &H, Receiver *Rec)
{
	/*
	###############################################################################################
	-------WHEN TO CALL------
	Automatically called by imagePlaneIntercept. Should be called for each flux intercept calculation.

	---INFORMATION REQUIRED--
	Managed by imagePlaneIntercept. Requires all heliostat, flux, and receiver geometry; ambient

	---------OUTPUT----------
	Returns spillage efficiency (1=no spillage) {type = double}
	###############################################################################################

	"HERMIT"

	This subroutine evaluates the Hermite coefficients.

	Returns spillage efficiency (1=no spillage)

	*/
	matrix_t<double> *mu_F = H.getFluxMomentsObject();
	//matrix_t<double> *hc_tht = H.getHermiteNormCoefObject();  //this structure isn't actually used here

    int nrf=0;
	for(int i=1;i<_n_terms+1;i++)
    { 
        for(int j=JMN(i-1); j<JMX(i-1)+1; j+=2)
        {
            nrf++;
        } 
    }

	double 
		eta_spill,		//Spillage efficiency
		sig_x2 = sqrt(mu_F->at(2,0)),	//x-direction image standard deviation
		sig_y2 = sqrt(mu_F->at(0,2)),	//y-direction image standard deviation
		SigXY[] = {sig_x2, sig_y2};			//Put in an array
	//Set the standard deviation of the image (scaled by tower height) for the heliostat
	H.setImageSize(sig_x2, sig_y2);

	int npak = nrf*4; //(int)hc_tht->ncells();

	//for optimization runs, don't do spillage calculations
	matrix_t<double> h_spill, axi, ayi;
	h_spill.resize_fill(1, nrf, 0.0); // hc_tht->nrows(), 0.0);
	axi.resize_fill(1, _n_terms, 0.0);
	ayi.resize_fill(1, _n_terms, 0.0);

	matrix_t<double>* hcoef = H.getHermiteCoefObject();
	hcoef->resize_fill(1, npak/4, 0.0);

	//detailed spillage calculations
	hermiteIntegralSetup(SigXY, H, h_spill, Rec);
	eta_spill = 0.;
	
	axi.at(0) = 1.;
	ayi.at(0) = 1.;
	for(int i=1; i<_n_terms; i++){
		axi.at(i) = axi.at(i-1)/sig_x2;
		ayi.at(i) = ayi.at(i-1)/sig_y2;
	}
	//Evaluate the hermite coefs
	int jmin, jmax, kmin, lmin, i, j, k, l;
	double temp_res;
	int ipak = 0;
    double save1 = axi.at(1)*ayi.at(1)/6.2832;
	for(i=1; i<_n_terms+1; i++){
		jmin = JMN(i-1);
		jmax = JMX(i-1);
		kmin = jmin;	//small difference in calculation method, but the hard-coded values appear to always give the same results. DELSOL 1393

        double save2 = save1 / _fact_d.at(i-1);

		for(j=jmin; j<jmax+1; j+=2){
			lmin = JMN(j-1);
			ipak ++;
			temp_res = 0.;

			for(k=kmin; k<i+1; k+=2){
				for(l=lmin; l<j+1; l+=2){
					temp_res += _binomials_hxn.at(i-1,k-1) * mu_F->at(k-1,l-1)*axi.at(k-1) * ayi.at(l-1) * _binomials_hxn.at(j-1,l-1);
				}
			}
			
			temp_res = temp_res / _fact_d.at(j-1) * save2;
			eta_spill += h_spill.at(ipak-1)*temp_res;
			hcoef->at(ipak-1) = temp_res;	//This array is used to evaluate the flux density
		}
	}
	
	return eta_spill;
	
}

void Flux::hermiteIntegralSetup(double SigXY[2], Heliostat &H, matrix_t<double> &hspill, Receiver *Rec){
	/*
	###############################################################################################
	-------WHEN TO CALL------
	Automatically called by the hermiteIntEval method. Should be called each time the flux 
	intercept is calculated.

	---INFORMATION REQUIRED--
	Automatically managed by hermiteIntEval

	---------OUTPUT----------
	Passes data from hermiteIntegral through 'hspill' matrix
	###############################################################################################	
	
	Calculate the amount of flux intercepted by the receiver. The receiver number is assigned 
	using the variable "rec", by default = 0.

	This subroutine is called by the hermiteIntEval method in evaluating the Hermite coefficients.

	DELSOL code 2041-2506

	The code manipulates and returns the hspill array of length(npak) ~typically x16

	*/

	//Choose the analytical coefficients based on the receiver type
	double G[5], F[5], X[2], A[2], TA[2];

    var_receiver *Rv = Rec->getVarMap();

    double optical_height = Rv->optical_height.Val();

	//Calculate required values and angles
	Vect *T = H.getTowerVector();
	double
		tht = optical_height,
		sig_x = SigXY[0],		//Standard deviation of the image in the x-direction
		sig_y = SigXY[1],		//Standard deviation of the image in the y-direction
		theta_t_zen = acos(T->k),	//Heliostat to receiver zenith angle
		cos_t_zen = T->k,
		sin_t_zen = sin(theta_t_zen);

	A[0] = 1./sig_x;
	A[1] = 1./sig_y;
	
	switch (Rec->getGeometryType())
	{
	case Receiver::REC_GEOM_TYPE::CYLINDRICAL_CLOSED:
	case Receiver::REC_GEOM_TYPE::POLYGON_CLOSED:
	{	//External receiver
		double
			w = Rec->CalculateApparentDiameter(*H.getLocation()),
			w2 = w/2./tht,
			h = Rv->rec_height.val,
			h2 = h/2./tht;

		//External cylindrical receiver
		G[4] = -1.;
		F[4] = -1.;
		G[1] = 0.;
		F[1] = 0.;
		G[2] = cos_t_zen;		//CTP Cosine of tower vector zenith angle
		F[2] = G[2];
		X[1] = w2;	//xmax Normalized maximum x-extent of the receiver
		X[0] = -X[1];		//xmin Normalized minimum x-extent of the receiver

		//Assign the aim points - relative to the flux plane
		TA[0] = H.getAimPointFluxPlane()->x/tht;		//aim at the center of the flux plane (horizontally)
		TA[1] = H.getAimPointFluxPlane()->y * sin_t_zen / tht;		//DELSOL -> YTAP(I)*sin_t_zen | the aim point is vertically scaled according to the severity of the view angle

        //DELSOL limits the integral based on the size of the receiver vs image size (sig_x)
		//if(X[1]/sig_x > 4.) {X[1] = 4.*sig_x;}
		//if(X[0]/sig_x < -4.) {X[0] = -4.*sig_x;}

        double siglim = 4.*sqrt(sig_x*sig_x + sig_y*sig_y);

        if( siglim < w2 )
        {
            double xprime = (TA[0] < 0. ? -1. : 1.) * fmin(fabs(TA[0]), w2 - siglim);
            X[0] = - siglim;
            X[1] =   siglim; 
            TA[0] += -xprime;
        }
        				
        G[0] = X[0] * cos_t_zen - h2 * sin_t_zen;
		F[0] = X[0] * cos_t_zen + h2 * sin_t_zen;

		G[3] = F[3] = X[1]*X[1];

		//Call the flux integral
		hermiteIntegral(G, F, X, A, TA, 1.0, hspill);
		break;

	}
	case Receiver::REC_GEOM_TYPE::CYLINDRICAL_CAV:
	case Receiver::REC_GEOM_TYPE::PLANE_RECT:
	case Receiver::REC_GEOM_TYPE::PLANE_ELLIPSE:
		//Cavity receiver | 2362
	{		
		//2	|	Continuous open cylinder - internal cavity
		//3	|	Planar rectangle
		//4	|	Planar ellipse
	
		//Get the aperture shape
		bool is_elliptical = /*Rv->aperture_type.mapval() == var_receiver::APERTURE_TYPE::ELLIPTICAL ||*/
						Rec->getGeometryType() == Receiver::REC_GEOM_TYPE::PLANE_ELLIPSE;	//0=Rectangular, 1=elliptical
		
		//Set up for cavity model | 2362
        sp_point *hloc = H.getLocation();
		double hloc_az = atan2(hloc->x, hloc->y);	//Azimuth angle of the heliostat location, receiver is abscissa
		double rxn = Rv->rec_width.val/tht/2.;		//Normalized half-width of the aperture
		double ryn = Rv->rec_height.val/tht/2.;		//Normalized half-height of the aperture
		double rec_az = Rv->rec_azimuth.val*D2R+pi;			//In DELSOL; user input for RAZM is 180=N; but later adjusted to 0=N
		double rec_zen = pi/2. - Rv->rec_elevation.val*D2R;		//Receiver zenith angle {90 = horizontal; >90 downward facing
		double hloc_az_i = cos(hloc_az);		//Cosine of the heliostat position azimuth angle
		double hloc_az_j = sin(hloc_az);		//Sine ""
		double rec_az_i = cos(rec_az);		//Cosine receiver aperture azimuth angle
		double rec_az_j = sin(rec_az);		//Sine ""
		double rec_zen_k = cos(rec_zen);	//Cosine receiver aperture zenith angle
		double rec_zen_j = sin(rec_zen);
		double cos_view_az = hloc_az_i*rec_az_i + hloc_az_j*rec_az_j;		//cosine of angle between rec. azimuth and helio position azimuth
		double sin_view_az = hloc_az_i*rec_az_j - hloc_az_j*rec_az_i;		//sine of angle between rec. azimuth and helio position azimuth - sign maintained
		
        /* 
        Coordinate system transform factors 'r'
        See Eq. A-2 in Dellin (1979)
        */
        
        double r11 = cos_view_az;       //base apparent width due to non-normal view of receiver
		double r12 = sin_view_az * rec_zen_k;   //extension of apparent width at corners due to non-normal view of a tilted receiver plane                              
                                                                                                                                                            //             /|
		double r21 = -cos_t_zen * sin_view_az;  //apparent vertical displacement of image plane corners due to azimuthal and upward severity of heliostat's viewpoint.  <-|-|
                                                                                                                                                            //            |/
		double r22 = sin_t_zen * rec_zen_j + cos_t_zen * rec_zen_k * cos_view_az;   //apparent vertical height of the receiver at the centerline
		
		sp_point *aim = H.getAimPointFluxPlane();		//In X and Y coordinates, Y being vertical


		if(is_elliptical){
			//Elliptical receiver aperture | 2382
			TA[0] = aim->x*r11 + r12 * aim->y;
            TA[1] = aim->x*r21 + r22 * aim->y;
			double
				ar2 = rxn*rxn,
				br2 = ryn*ryn,
				aa = r12*r12/ar2 + r11*r11/br2,
				bb = -2.*(r12*r22/ar2 + r11*r21/br2),
                bb2 = bb*bb,
				cc = r22*r22/ar2 + r21*r21/br2,
				dd = -pow(r11*r22-r12*r21,2),
				term_1 = bb2 - 4.*aa*cc,
				xmax = dd*bb2/(aa*term_1),
				xmin = sqrt(xmax);
			if(bb > 0) xmin = - xmin;
			xmax = fabs(-bb*xmin/2./cc + sqrt(xmax*term_1 - 4.*cc*dd)/2./cc);
			xmin = -xmax;
			G[0] = 0.;
			F[0] = 0.;
			G[1] = -bb/aa/2.;
			F[1] = G[1];
			G[2] = -.5/aa;
			F[2] = -G[2];
			G[4] = bb2 - 4.*aa*cc;
			F[4] = G[4];
			G[3] = -4.*aa*dd;
			F[3] = G[3];

			X[0] = xmin;
			X[1] = xmax;

			//Call the flux integral
			hermiteIntegral(G, F, X, A, TA, 1.0, hspill);
		}
		else{
			/*
			Rectangular receiver aperture		2410
			This uses an X[] and Y[] array. Map to U[] and V[]
			*/
			double 
				rxn_temp = rxn,
				ryn_temp = ryn,
				U[4], V[4];
			if(r11 < 0.) rxn_temp = -rxn;
			if(r12 < 0.) ryn_temp = -ryn;
            


            /* 
		    The DELSOL Hermite integral is inaccurate for heliostat images that are significantly smaller than
		    the receiver. Perform the integral over a subsection of the receiver if the image is much smaller than 
		    the receiver.

		    More information on the coordinate system rotation matrix rXX can be found in Dellin, T. A. (1979), pp 28.
		    */
		    double sig_x5 = sig_x * 5;        //6/23/2015 -- Seems to work better with 10x. Adjusted for bug-fixed algorithm below
		    double sig_y5 = sig_y * 5;
		    double sig_max5 = max(sig_y5, sig_x5);
            

            /* 
            First calculate the original image plane receiver corner points.

            The U and V vectors represent the corner positions of the receiver in the heliostat view plane. U are x coords, V are Y.
            In other words, these coordinates map the receiver corner positions when projected onto a plane normal to the heliostat-
            to-receiver vector. The points are subject to rotation and scaling according to the orientation of the heliostat and the
            receiver.
            */

            //                                                                                                                                         ____ 
			U[0] = r11 * rxn_temp + r12 * ryn_temp;     //apparent width at outer (or inner) corner of flat plate receiver projected onto image plane /_|_/ ^^
            //                                                                                                                                         ____ 
            U[1] = -r11 * rxn_temp + r12 * ryn_temp;    //apparent width at outer (or inner) corner of flat plate receiver projected onto image plane /_|_/ vv
			U[2] = -U[0];                               //opposite corner from U[0] (x)
			U[3] = -U[1];                               //opposite corner from U[1] (x)

			V[0] = r21 * rxn_temp + r22 * ryn_temp;     //apparent height at outer (or inner) corner of flat plate receiver projected onto image plane
			V[1] = -r21 * rxn_temp + r22 * ryn_temp;    //apparent height at inner (or outer) corner of flat plate receiver projected onto image plane
			V[2] = -V[0];                               //opposite corner from V[0] (y)
			V[3] = -V[1];                               //opposite corner from V[1] (y)

            /* 
            We now want to determine whether the image is likely to be much smaller than the projected receiver quadrilateral. 
            Calculate the apparent space between parallel edges.
            */

            //Dot product of 0-3-2 scales receiver height and width
            double lx03 = U[0]-U[3];
            double ly03 = V[0]-V[3];
            double lx23 = U[2]-U[3];
            double ly23 = V[2]-V[3];
            double l03 = sqrt( ly03*ly03 + lx03*lx03 );     //height component
            double l23 = sqrt( ly23*ly23 + lx23*lx23 );     //width component
            //take dot product
            double dp = (lx03 * lx23 + ly03 * ly23)/(l03 * l23);
            //sine product gives final scale due to skewness
            double sdp = sqrt(1. - dp*dp);

            //if the scaled width/height are greater than the maximum image size, move into receiver scaling mode
            double aimy_adj = aim->y/tht;   //aimpoint in global coordinate system
            double aimx_adj = aim->x/tht;
            if( sdp * l03 > 2.*sig_max5 || sdp * l23 > 2.*sig_max5 )
            {

                /* 
                The image of the heliostat can be modeled approximately using the normalizing constants sig_x and sig_y as an
                ellipsoid. When viewed from the heliostat, the image appears to be oriented such that sig_y falls along the
                vertical axis and sig_x along the horizontal axis. 

                When projected on to the receiver, the ellipsoid must be scaled and rotated according to the orientation of 
                the receiver relative to the heliostat image plane. For this analysis, we only care about scaling down the 
                receiver size (virtually) so that the integral can be evaluated using quadrature. Therefore, we can avoid having 
                to project and rotate the ellipsoid onto the receiver plane by solving for tangent lines to the ellipsoid in 
                the heliostat image plane where the tangent lines are parallel with the edge lines of the projected receiver. 
                These tangent lines form the new scaled receiver bounds.

                Use the equation of an ellipse:
                (x/sig_x)^2 + (y/sig_y)^2 = 1
                
                We wish to know where the derivative of this equation dy/dx equals the slope of the tangent line:
                dy/dx = - (sig_x * x) / sqrt(b - a b x^2 )
                In terms of x-position:
                x = -(sig_x^2 * dy/dx) / sqrt( sig_y^2 + sig_x^2 (dy/dx)^2 )

                The apparent half-width and half-height of the scaled receiver in projected coordinates are given by the 
                radius of the ellipse at x,y. Substituting the above expression for x and the equation of the ellipse solved 
                for y into the radius equation:
                r = sqrt(x^2 + y^2)
                
                we have: 
                r = sqrt[ (sig_y^4 + sig_x^4 * (dy/dx)^2 ) / (sig_y^2 + sig_x^2 * (dy/dx)^2 ) ]
                (valid first quadrant)
                */

                double dydx_w = fabs(lx23)>1e-6 ? ly23/lx23 : 1e6;
                if(U[3] > U[0]) dydx_w *= -1.;  //always work 3->0 positive

                double dydx_w2 = dydx_w * dydx_w;

                double dydx_h = fabs(lx03)>1e-6 ? ly03/lx03 : 1e6;
                if(U[3] > U[2]) dydx_h *= -1;   //always work 3->2 positive

                double dydx_h2 = dydx_h * dydx_h;

                //translation assumes coef a = sig_x / sig_y, b = 1 = sig_y / sig_y
                double a = sig_x / sig_y;
                double a2 = a*a;
                double a4 = a2*a2;

                double radw = sqrt( (1. + a4*dydx_w2)/(1. + a2*dydx_w2) );
                double radh = sqrt( (1. + a4*dydx_h2)/(1. + a2*dydx_h2) );

                //Translate back to original receiver coordinates
                double rx_ip = 2. * radw * sig_y5 * rxn / (sdp * l23);
                double ry_ip = 2. * radh * sig_y5 * ryn / (sdp * l03);

                if(ry_ip < ryn || rx_ip < rxn)
                {
                    //move the quadrature rectangle to be as close as possible to the aim point without moving outside
                    //the original reciever bounds
                    double delta_x_q = aimx_adj; 
                    double signx = aimx_adj < 0 ? -1. : 1.;
                    double dshiftx = max(rxn - rx_ip, 0.);      //maximum allowable shift in x, zero if rx_ip > rxn
                    if( fabs(delta_x_q) > rxn - rx_ip ) delta_x_q = signx * dshiftx;

                    double delta_y_q = aimy_adj; 
                    double signy = aimy_adj < 0 ? -1. : 1.;
                    double dshifty = max(ryn - ry_ip, 0.);
                    if( fabs(delta_y_q) > ryn - ry_ip ) delta_y_q = signy * dshifty;

                    //the aim point is now closer to the (0,0) centroid of the quadrature grid. Adjust
                    aimx_adj += -delta_x_q;
                    aimy_adj += -delta_y_q;

                    //update receiver width
                    if( rx_ip < rxn ) rxn = rx_ip;
                    if( ry_ip < ryn ) ryn = ry_ip;
                }
                
                //Recalculate U and V vectors with new widths
                rxn_temp = rxn;
				ryn_temp = ryn;
			    if(r11 < 0.) rxn_temp = -rxn;
			    if(r12 < 0.) ryn_temp = -ryn;
                
                U[0] = r11 * rxn_temp + r12 * ryn_temp;     
                U[1] = -r11 * rxn_temp + r12 * ryn_temp;    
			    U[2] = -U[0];                               
			    U[3] = -U[1];                               

			    V[0] = r21 * rxn_temp + r22 * ryn_temp;     
			    V[1] = -r21 * rxn_temp + r22 * ryn_temp;    
			    V[2] = -V[0];                               
			    V[3] = -V[1];                               

            }
            else
            {
                aimy_adj = aim->y/tht;
                aimx_adj = aim->x/tht;
            }

        
            //Set aimpoints
            TA[0] = aimx_adj*r11 + r12 * aimy_adj;
            TA[1] = aimx_adj*r21 + r22 * aimy_adj;
            

			//finish setting up arrays
            G[2] = G[3] = G[4] = 0.;
			F[2] = F[3] = F[4] = 0.;

            double
				csmall = 1.e-8;

            //if the X position of the 2nd point is greater than the 4th point, assume symmetry and swap points to make the sign of the integral consistent.
			if(! (U[1] +csmall < U[3])){
				swap(U[1], U[3]);
				swap(V[1], V[3]);
			}

			//Evaluate the integral based on the values of X
			if(! (pow(U[0] - U[3],2) > csmall)){
                X[0] = U[2];
				X[1] = U[0];
				G[0] = (U[1]*V[0] - U[0]*V[1])/(U[1]-U[0]);
				G[1] = (V[1] - V[0])/(U[1] - U[0]);
				F[1] = (V[2] - V[3])/(U[2] - U[3]);
				F[0] = (U[2]*V[3] - U[3]*V[2])/(U[2]-U[3]);

                if(!( (F[0] + F[1]*U[1]) >= (G[0] + G[1]*U[1]) ) ){
					//Switch stuff
					swap(G[0], F[0]);
					swap(G[1], F[1]);
				}

				//Call the flux integral		2451
				hermiteIntegral(G, F, X, A, TA, 1.0, hspill);
				return;		//Return here if this call is executed
			}

			//**** Other cases ... Evaluate all that are applicable without immediate return. *****
			
			if(! (pow(U[1] - U[2],2) < csmall)){
				X[0] = U[2];
				X[1] = U[1];
				F[0] = (U[2]*V[3]-U[3]*V[2])/(U[2]-U[3]);
				F[1] = (V[2]-V[3])/(U[2]-U[3]);
				G[0] = (U[1]*V[2]-U[2]*V[1])/(U[1]-U[2]);
				G[1] = (V[1]-V[2])/(U[1]-U[2]);
				if((F[0] + F[1]*U[1]) < (G[0] + G[1]*U[1]) ){
					swap(G[0], F[0]);
					swap(G[1], F[1]);
				}
                
				//Call the flux integral 2469
				hermiteIntegral(G, F, X, A, TA, 1.0, hspill);
			}
			
			if(! (pow(U[1]-U[3],2) < csmall)){
				X[0] = U[1];
				X[1] = U[3];
				G[0] = (U[1]*V[0]-U[0]*V[1])/(U[1]-U[0]);
				G[1] = (V[1]-V[0])/(U[1]-U[0]);
				F[1] = (V[2]-V[3])/(U[2]-U[3]);
				F[0] = (U[2]*V[3]-U[3]*V[2])/(U[2]-U[3]);
				if(!( (F[0] + F[1]*U[1]) >= (G[0] + G[1]*U[1]) ) ){
					swap(G[0], F[0]);
					swap(G[1], F[1]);
				}

				//Call the flux integral 2486
				hermiteIntegral(G, F, X, A, TA, 1.0, hspill);
			}

			//always use this final case...
			X[0] = U[3];
			X[1] = U[0];
			F[0] = (U[3]*V[0] - V[3]*U[0])/(U[3]-U[0]);
			F[2] = (V[3]-V[0])/(U[3]-U[0]);
			G[0] = (U[1]*V[0]-U[0]*V[1])/(U[1]-U[0]);
			G[1] = (V[1]-V[0])/(U[1]-U[0]);
			if(! ( (F[0]+F[1]*U[3]) >= (G[0]+G[1]*U[3]) )){
				swap(G[0], F[0]);
				swap(G[1], F[1]);
			}

			//Call the flux integral 2469
			hermiteIntegral(G, F, X, A, TA, 1.0, hspill);
		}

		break;
	}
	case Receiver::REC_GEOM_TYPE::POLYGON_OPEN:
	case Receiver::REC_GEOM_TYPE::POLYGON_CAV:
	case Receiver::REC_GEOM_TYPE::CYLINDRICAL_OPEN:
		throw spexception("Unsupported receiver type in Hermite Integral calculations.");
		break;
	default:
		break;
	}
	

}
//#define _WRITE_FILE 

void Flux::hermiteIntegral(double G[5], double F[5], double X[2], double A[2], double TA[2], double WT, matrix_t<double> &hspill){
	/*
	###############################################################################################
	-------WHEN TO CALL------
	Automatically called by the hermiteIntegralSetup method.

	---INFORMATION REQUIRED--
	Defined in the hermiteIntegralSetup method

	---------OUTPUT----------
	Fills the 'hspill' matrix, which is used to determine flux intercept based on receiver 
	geometry and aim point
	###############################################################################################

	"HINT"
	
	Inputs are:
	G[1-5]
	F[1-5]
	X[xmax,xmin]
	A[axi,ayi]
	WT				|	Multiplier weight for this intercept - by default should be 1.0
	&hspill			|	location of the result array

	Calculate the integral of the Hermite polynomial - determines
	the intercept of the flux image on the receiver. Work with the hspill 
	array.

	This subroutine calculates the integral of:
	HI(X/AX)*HJ(Y/AY)*EXP(-.5*(X**2/AX2+Y**2/AY2)) 
    over the projection of teh receiver on the image plane

    8 point gaussian integration formulae are used.
	
	1540-1627
	*/
	/*double ci[] = {.196584,.115194,.000344,.019527};
	double ag[] = {.02715246, .06225352, .09515851, .12462897,
					.14959599, .16915652, .18260341, .18945061,
					.14959599, .16915652, .18260341, .18945061,
					.02715246, .06225352, .09515851, .12462897};
	double xg[] = {.98940093, .94457502, .86563120, .75540441,
					.61787624, .45801678, .28160355, .09501251,
					-.61787624,-.45801678,-.28160355,-.09501251,
					-.98940093,-.94457502,-.86563120,-.75540441};*/
	double
		xmax = X[1],
		xmin = X[0],
		xdiff = (xmax - xmin)/2.;
	double 
		ss = 2.50663,
		dsmall = 1.23456789e-10;
	double
		xta = TA[0],
		yta = TA[1];
	matrix_t<double> h;
	h.resize_fill(3,9,0.0);

	//declare other temp variables
	double xx, x12, x[3], xsq[3], fk, s2, s3, sign2, sign3;
	int ipak, i, j, k, n, jmin, jmax;
#ifdef _WRITE_FILE
    string name = "C:/Users/mwagner/Documents/NREL/Field optimization/Misc sim files/int/hdat.csv";
    ofstream fout(name.c_str());
    fout << "x0,x1,x2,xsq1,2,3,sign2,3,s2,s3,h-->\n";
    fout.clear();
#endif 
    //Evaluate each of the quadrature points
	for(n=1; n<17; n++){
        //move through abscissas (positive receiver extent to negative). Each _xg is Gauss-Hermite abscissa.
		xx = xmin+xdiff*(1.+_xg[n-1]);
		x12 = xx * xx;
        //I don't know what these values represent

        /* 
        Cylindrical
        -------------
        G[0] - More negative the smaller either width/height are, closer theta_t_zen to 45deg
        F[0] - Positive if width*cos_t_zen is smaller than height*sin_t_zen
        G[1] = F[1] - 0
        G[2] = F[2] - helio-to-tower zenith angle cosine (cos_t_zen)
        G[3] = F[3] - w2^2
        G[4] = F[4] - -1

        Plate
        -------------
        */

		x[1] = (G[0]+G[1]*xx+G[2]*sqrt(G[3]+G[4]*x12))*A[1]-yta*A[1];
		x[2] = (F[0]+F[1]*xx+F[2]*sqrt(F[3]+F[4]*x12))*A[1]-yta*A[1];
        //The distance between the current abscissa and the aim point (X) in # of std-dev's
		x[0] = (xx-xta)*A[0];
        //Initialize the Hermite coefficient array
		h.at(0,0) = 0.;
		h.at(1,0) = 0.;
		h.at(2,0) = 0.;
		for(i=0; i<3; i++){xsq[i] = x[i]*x[i];}
        //Calculate the quadrature weights. If the xsq value is large, don't bother since it will be zero.
		if(xsq[0] < 100.) {h.at(0,0) = exp( -xsq[0]/2. )*WT/A[1]*xdiff;}
		if(xsq[1] < 100.) {h.at(1,0) = exp( -xsq[1]/2. );}
		if(xsq[2] < 100.) {h.at(2,0) = exp( -xsq[2]/2. );}
#ifdef _WRITE_FILE
        fout << x[0] << "," << x[1] << "," << x[2] << ",";
        fout << xsq[0] << "," << xsq[1] << "," << xsq[2] << ",";
#endif
        //Initialize
		h.at(1,1) = 0.;
		fk = -2.;
        /* 
        Evaluate the hermite polynomial series coefficients. 
        
        The recurrence relationship is used: 
        H_n = x * H_n-1 - (n-1)*H_n-2
        */
		for(k=3; k<_n_terms+3; k++){
			fk += 1.;
			h.at(0,k-1) = x[0]*h.at(0,k-2) - fk*h.at(0,k-3);
			h.at(1,k-1) = x[1]*h.at(1,k-2) - fk*h.at(1,k-3);
			h.at(2,k-1) = x[2]*h.at(2,k-2) - fk*h.at(2,k-3);
		}
		s2 = 1.; s3 = 1.;
		sign2 = (x[1]+dsmall)/fabs(x[1]+dsmall);
		sign3 = (x[2]+dsmall)/fabs(x[2]+dsmall);
		x[1] = fabs(x[1]);
		x[2] = fabs(x[2]);
        //
		for(j=1; j<5; j++){
			s2 += _ci[j-1]*pow(x[1], j);
			s3 += _ci[j-1]*pow(x[2], j);
		}
		s2 = ss*((sign2 - 1.)/(-2.) + sign2*(1.-.5*pow(s2, -4)));
		s3 = ss*((sign3 - 1.)/(-2.) + sign3*(1.-.5*pow(s3, -4)));
		
		h.at(1,1) = s3 - s2;

#ifdef _WRITE_FILE
        fout << sign2 << "," << sign3 << "," << s2 << "," << s3 << ",";

        for(int rr =0; rr<h.nrows(); rr++)
            for(int cc=0; cc<h.ncols(); cc++)
               fout << h.at(rr,cc) << ",";
        
        fout << "\n";
#endif
		ipak = 0;
		for(i=1; i<_n_terms+1; i++){
			int im1 = i-1;
			jmin = im1%2+1; //jmin = JMN(i-1);
			jmax = _n_terms - im1; //jmax = JMX(i-1);
			for(j=jmin; j<jmax+1; j+=2){
				ipak++;
				hspill.at(ipak-1) += _ag[n-1]*h.at(0,i+1)*(h.at(1,j) - h.at(2,j));
			}
		}	
	}
#ifdef _WRITE_FILE
    fout.close();
#endif
}

void Flux::fluxDensity(simulation_info *siminfo, FluxSurface &flux_surface, Hvector &helios, bool clear_grid, bool norm_grid, bool show_progress){
	/* 
	Take a set of points defining the flux plane within the flux_surface object, a solar field geometry, 
	and calculate the flux intensity at each point. Fills and returns these values into the FluxSurface
	structure. 
	
	The returned value is unitless, but is to be used in the form:
		Q_rec_{i,j} [W] = Q''_rec_{i,j} [-] * DNI [W/m2] * A_node_{i,j}
	where:
		Q_rec_{i,j} [W] is the power delivered to node i,j on the receiver
		Q''_rec_{i,j} [-] is the value returned by this method
		DNI [W/m2] is the direct normal irradiation incident on the solar field
		A_node_{i,j} is the area of node i,j on the receiver
			

	FluxGrid == vector<vector<FluxPoint>> 

	If a limited set of heliostats should be evaluated instead of the whole field, use the h_ids[] array
	which contains N+1 entries. If not null, the array should be constructed as:
	[<# of entries>, <heliostat id #1>, <heliostat id #2>, ... , <heliostat id #N>]

	The method evaluates the flux for each heliostat at each tested point on the receiver.

	---------------

	The flux intensity on a particular point at x,y is:
	
	SUM(k=1)^(nh){ r <dot> h * FLUX(x,y) * exp(1/2 (x^2 - y^2)) }
	
	This equation appears in DELSOL Line 553. The FLUX subroutine in DELSOL evaluates the Hermite 
	series for each heliostat zone (individual heliostats here) based on the previously calculated
	image error coefficients. The magnitude of the flux is scaled by the dot product of the 
	receiver normal and heliostat-to-receiver vectors and by the distance that the point (x,y) is
	away from the image center on the receiver. Scaling of the latter form is handled in the 
	exp() call. 

	Additional terms in DELSOL are empirical scaling coefficients based on aim point algorithms and
	multiple receivers. 

	Here, (x,y) is normalized by the standard deviation of the image error in x and y respectively.

	*/
	
	//get the flux grid
	FluxGrid* grid = flux_surface.getFluxMap();
	int 
		nfx = (int)grid->size(),
		nfy = (int)grid->at(0).size();
	//Zero the grid values
	if(clear_grid) 
    {
        flux_surface.ClearFluxGrid();
        flux_surface.setMaxObservedFlux(0.);
    }

	//Get the flux surface offset
	sp_point *offset = flux_surface.getSurfaceOffset();
	
	int nh = (int)helios.size();
	if(show_progress){
		siminfo->setTotalSimulationCount(nh);
	}
	//Loop through each heliostat
	int update_every = max(nh/20,1);

	for(int i=0; i<nh; i++){
		if(show_progress && i % update_every == 0)
			siminfo->setCurrentSimulation(i+1);
		
        if(! helios.at(i)->IsEnabled() )
            continue;

		//Get the image error std dev's
		double sigx, sigy;	
		helios.at(i)->getImageSize(sigx, sigy);	//Image size is normalized by the tower height
		
		//Get the heliostat aim point
		sp_point *aim = helios.at(i)->getAimPoint();
		//Get the height of the receiver that the heliostat is aiming at
		double tht = helios.at(i)->getWhichReceiver()->getVarMap()->optical_height.Val();

		//Calculate the normalizing constant. This is equal to the normalized power delivered by the heliostat to the
		//reciever divided by the tower height squared. (the tht^2 term falls out of the normalizing procedure
		//that we previously used in defining the Hermite moments). See DELSOL 7634.
		double cnorm = helios.at(i)->getArea() * helios.at(i)->getEfficiencyTotal()/(tht*tht);
		//Loop through each flux point
		//Rows
		for(int j=0; j<nfx; j++){
			//Cols
			for(int k=0; k<nfy; k++){
				//Get the flux point
				FluxPoint *pt = &grid->at(j).at(k);
				//Calculate the dot product between the flux point normal and the helio->tower vector
				Vect *tv = helios.at(i)->getTowerVector();
				Vect tvr;
				tvr.Set( -tv->i, -tv->j, -tv->k );	//Reverse

				double f_dot_t = Toolbox::dotprod(pt->normal, tvr);	
				//If the dot product is negative, the point is not in view of the heliostat, so continue.
				if(f_dot_t < 0.) continue;
				if(f_dot_t>1.){
					continue;
				}
				//Translate the flux point location into global coordinates
				sp_point pt_g;
				pt_g.Set(pt->location.x + offset->x, pt->location.y + offset->y, pt->location.z + tht); //tht include z offset

				//Project the current flux point into the image plane as defined by the 
				//aim point and the heliostat-to-receiver vector.
				sp_point pt_ip;
				Toolbox::plane_intersect(*aim, tvr, pt_g, tvr, pt_ip); 
				
				//Now the point pt_ip indicates in global coordinates the projection of the flux point onto the image plane.
				
				//Translate the flux point into coordinates relative to the aim point
				pt_ip.Subtract( *aim );
				
				//Express this point in image plane coordinates
                double azpt = atan2(tvr.i, tvr.j);
                double zenpt = acos(tvr.k);

				Toolbox::rotation(pi-azpt, 2, pt_ip);
				Toolbox::rotation(zenpt, 0, pt_ip);

				//This rotation now expresses pt_ip in x,y coordinates of the image plane.

				//Normalize the x,y coordinates with respect to the image error size
				double
					xn = -pt_ip.x/tht / sigx,       //with delsol formulation, image is flipped in x direction. Not sure why.
					yn = pt_ip.y/tht / sigy;
				
				//Calculate the flux
                double hfe = hermiteFluxEval(helios.at(i), xn, yn) * exp( -0.5 *( xn*xn + yn*yn) );
                //if( hfe != hfe )
                //{
                //    double xxxxx = 0.;
                //}
				pt->flux += f_dot_t * hfe * cnorm;
			}
		}
	}
	if(show_progress){
		siminfo->Reset();
		siminfo->setCurrentSimulation(0);
	}
	if(norm_grid){
		//Normalize the flux to sum to 1
		double fsum=0.;
		for(int i=0; i<nfx; i++){
			for(int j=0; j<nfy; j++){
				fsum += grid->at(i).at(j).flux;
			}
		}
		//make sure fsum is positive
		fsum = max(fsum, 1.e-6);
		for(int i=0; i<nfx; i++){
			for(int j=0; j<nfy; j++){
				grid->at(i).at(j).flux *= 1./fsum;
			}
		}
	}

}

double Flux::hermiteFluxEval(Heliostat *H, double xs, double ys){
	/* 
	Evaluate the flux density at point (x,y) in the image plane for the give heliostat H
	*/

	//get the hermite coef array from the heliostat
	matrix_t<double> *hc = H->getHermiteCoefObject();

	double HX[9], HY[9];
	HX[0] = 1.;
	HX[1] = 0.;
	HY[0] = 1.;
	HY[1] = 0.;

	double FX = -2.;

	for(int i=1; i<_n_terms+1; i++){
		FX ++;
		HX[i+1] = xs*HX[i] - FX*HX[i-1];
		HY[i+1] = ys*HY[i] - FX*HY[i-1];
	}
	int ipak = 0;
	double flux = 0.;
	for(int i=1; i<_n_terms+1; i++){
		int
			jmin = JMN(i-1),
			jmax = JMX(i-1);
		for(int j=jmin; j<jmax+1; j+=2){
			flux += hc->at(ipak)*HX[i+1]*HY[j+1];
			ipak++;
		}
	}
	if(flux < 0.) flux = 0.;
	return flux;
}

/* 
-----------------------------------------------------------------
						Aim point methods  
-----------------------------------------------------------------
*/

void Flux::simpleAimPoint(Heliostat &H, SolarField &SF){
	/* 
	This call sets the aim point local to the Heliostat object "H"
	*/
	simpleAimPoint(H.getAimPoint(), H.getAimPointFluxPlane(), H, SF);
}

void Flux::simpleAimPoint(sp_point *Aim, sp_point *AimF, Heliostat &H, SolarField &SF)
{
	/*
	This script takes a pointer to the result "Aim" point, the heliostat doing the aiming, and the SolarField that has information 
	on the receivers. The purpose of the script is to quickly assess the recievers and the heliostat and determine the best aim 
	point that maximizes the intercept factor. The following are considered:
	* Type of receiver
    * Number of receivers
    * Incidence angle of the incoming flux, what's the projected area?
    
	Total flux limitations are NOT considered here. For detailed flux mapping, use the calculateAimPoints() method.

	This call returns the relevant information within "Aim" and does not set the local value of the aimpoint for "H"
	This call also returns relevant information for "AimF" - the aim point in flux plan coordinates

	*/

	vector<Receiver*> *Recs = SF.getReceivers();

	double tht = SF.getVarMap()->sf.tht.val;
	
	int isave;
	Vect rtoh;	//receiver to heliostat vector

	calcBestReceiverTarget(&H, Recs, tht, isave, &rtoh);

	//For the selected receiver, determine the aim point
	Receiver *rec = Recs->at(isave);
	//Associate the receiver with the heliostat
	H.setWhichReceiver(rec);

    var_receiver *Rv = rec->getVarMap();
	double
		opt_height = Rv->optical_height.Val(), // + rec->getOffsetZ(),       << optical height already includes Z offset
		y_offset = Rv->rec_offset_y.val,
		x_offset = Rv->rec_offset_x.val;
	int recgeom = rec->getGeometryType();

    double view_az, w2;
	switch (recgeom)
	{
	case Receiver::REC_GEOM_TYPE::CYLINDRICAL_CLOSED:
	case Receiver::REC_GEOM_TYPE::POLYGON_CLOSED:
		view_az = atan2(rtoh.i , rtoh.j);	//The azimuth angle of the view from the receiver to the heliostat
		w2 = rec->CalculateApparentDiameter(*H.getLocation())/2.;	//half of the receiver diameter
		Aim->x = x_offset + w2*sin(view_az);		//x component of the aim point
		Aim->y = y_offset + w2*cos(view_az);		//y component of the aim point
		Aim->z = opt_height;						//z component of the aim point

		//Calculate the flux plane position (i.e. the position in the plane that represents the viewable portion of the receiver w/r/t the heliostat)
		//By definition using the simple aim point, flux plane position is in the center of the receiver.
		//ONLY X AND Y ARE USED
		AimF->Set(0., 0., 0.);

		break;
	case Receiver::REC_GEOM_TYPE::CYLINDRICAL_OPEN:
		break;
	case Receiver::REC_GEOM_TYPE::CYLINDRICAL_CAV:
	case Receiver::REC_GEOM_TYPE::PLANE_RECT:
	case Receiver::REC_GEOM_TYPE::PLANE_ELLIPSE:
		//2	|	Continuous open cylinder - internal cavity
		//3	|	Planar rectangle
		//4	|	Planar ellipse

		//For any of these options with planar apertures, select the center of the aperture as the aim point
		Aim->x = x_offset;
		Aim->y = y_offset;
		Aim->z = opt_height;

		AimF->Set(0., 0., 0.);
		break;
	case Receiver::REC_GEOM_TYPE::POLYGON_OPEN:
	case Receiver::REC_GEOM_TYPE::POLYGON_CAV:
	default:
		throw spexception("The specified receiver geometry is not currently supported.");
	}
	

	return;

}

void Flux::zenithAimPoint(Heliostat &H, Vect &Sun)
{
    sp_point *Aim = H.getAimPoint();
    sp_point *AimF = H.getAimPointFluxPlane();

    //the aimpoint bisects the sun and zenith
    Aim->x = Sun.i / 2. * 1000.;
    Aim->y = Sun.j / 2. * 1000.;
    Aim->z = Sun.k / 2. * 1000.;

    AimF->Set(0., 0., 9.e9);

}

void Flux::sigmaAimPoint(Heliostat &H, SolarField &SF, double args[]){
	/* 
	This method calculates aim points based on the standard deviation of the image error in the plane of the 
	receiver aperture in both the X and Y directions.	

	The aimpoint is set within the heliostat 'H' that is passed to the method.

	Each heliostat should have stored the standard deviations normalized in meters (normalized by the tower
	height) calculated from the Hermite evaluation methods.
	*/

	
	vector<Receiver*> *Recs = SF.getReceivers();

	sp_point *Aim = H.getAimPoint();

	double tht = SF.getVarMap()->sf.tht.val;

	int isave;
	Vect rtoh;
	calcBestReceiverTarget(&H, Recs, tht, isave, &rtoh);

	//For the selected receiver, determine the aim point
	Receiver *rec = Recs->at(isave);
	//Associate the receiver with the heliostat
	H.setWhichReceiver(rec);

    var_receiver *Rv = rec->getVarMap();
	double
		opt_height = Rv->optical_height.Val(), // + rec->getOffsetZ(),       << optical height already includes Z offset
		y_offset = Rv->rec_offset_y.val,
		x_offset = Rv->rec_offset_x.val;
	int recgeom = rec->getGeometryType();

	double view_az, w2, h2;

	//Get the simple aim point
	sp_point saim, saimf;
	simpleAimPoint(&saim, &saimf, H, SF);
	
	double sigx,sigy;
	sp_point aimpos;

	switch (recgeom)
	{
	case Receiver::REC_GEOM_TYPE::CYLINDRICAL_CLOSED:
	case Receiver::REC_GEOM_TYPE::POLYGON_CLOSED:

		//0	|	Continuous closed cylinder - external

		//Aim at the closest part of the receiver
		view_az = atan2(rtoh.i, rtoh.j);	//The azimuth angle of the view from the receiver to the heliostat
		w2 = rec->CalculateApparentDiameter(*H.getLocation())/2.;		//half of the receiver diameter
		h2 = Rv->rec_height.val/2.;		//Half of the receiver height
		Aim->x = x_offset + w2*sin(view_az);		//x component of the aim point
		Aim->y = y_offset + w2*cos(view_az);		//y component of the aim point

		//Calculate the z position based on the image size in the height direction
		H.getImageSize(sigx, sigy);
		sigy *= tht; 

		Aim->z = opt_height + (h2 - fmin(args[0]*sigy, h2) )*args[1];		//args[1] is an alternating flag, = -1 or 1
		
		//-- now calculate the aim point position in the flux plane
		//vector from simple aim point to mod aim point
		aimpos.Set(Aim->x - saim.x, Aim->y - saim.y, Aim->z - saim.z);

        H.calcAndSetAimPointFluxPlane(aimpos, *rec, H);

		break;
		/*
		else if(recgeom == 1){		//1	|	Continuous open cylinder - external	
		}
		else if(recgeom > 1 && recgeom < 5){		
			//2	|	Continuous open cylinder - internal cavity
			//3	|	Planar rectangle
			//4	|	Planar ellipse
							
			//is the aperture elliptical?
			bool is_elliptical = rec->getReceiverApertureType() == 1 || recgeom == 4;



		}
		else if(recgeom == 5){		//5	|	Discrete closed N-polygon - external	
		}
		else if(recgeom == 6){		//6	|	Discrete open N-polygon - external
		}
		else if(recgeom == 7){		//7	|	Discrete open N-polygon - internal cavity
		} */
	default:

		throw spexception( "Receiver geometry not supported for Sigma Aim Point calculation" );
		
	}

	return;


}

void Flux::probabilityShiftAimPoint(Heliostat &H, SolarField &SF, double args[]){
	/* 
	Information is set within heliostat 'H' that is passed to this method

	This method calculates the aim point of the heliostat on the best receiver option. The aim point
	calculation is sampled randomly within the available range for the "sigma multiplier" parameter (args[0]).
	The steps for calculating the aim point is as follows:
	1) Calculate the simple aim point representing the center of the flux plane.
	2) Calculate the range within the receiver flux plane that would result in at least "sigma" standard
	   deviations of the flux image size between the receiver edges and the center of the flux image. 
	   This calculation uses the second coefficients of X and Y in the Hermite expansion as representative
	   of the standard deviation.
	3) Pick randomly to determine the actual aim point within the window calculated in (2).The random number
	   can be sampled from several distributions as indicated by args[1]:
	   0 = Triangular
	   1 = Normal
	   2 = Uniform
	   Non-uniform distributions are "reflected", or weighted such that the higher probability regions approach
	   the receiver edges rather than the center.

	   If the distribution is normal, the third parameter (args[2] ) will be the standard deviation of the 
	   sampling distribution in the range (0,1].

	Note: if the sigma multiplier times the standard deviation is larger than the receiver dimensions, the
		  aim point will automatically be chosen at the simple aim point where intercept is maximized. Thus,
		  typical heliostats at the outer regions of the solar field will generally be aimed at the center of
		  the receiver.

	*/

	
	vector<Receiver*> *Recs = SF.getReceivers();

	sp_point *Aim = H.getAimPoint();

	double tht = SF.getVarMap()->sf.tht.val;
	
	int isave;
	Vect r_to_h;	//receiver to heliostat vector

	calcBestReceiverTarget(&H, Recs, tht, isave, &r_to_h);

	
	//For the selected receiver, determine the aim point
	Receiver *rec = Recs->at(isave);
	//Associate the receiver with the heliostat
	H.setWhichReceiver(rec);

	var_receiver *Rv = rec->getVarMap();
	double
		opt_height = Rv->optical_height.Val(), // + rec->getOffsetZ(),       << optical height already includes Z offset
		y_offset = Rv->rec_offset_y.val,
		x_offset = Rv->rec_offset_x.val;
	int recgeom = rec->getGeometryType();

    double view_az, w2, h2;

	//Get the simple aim point
	sp_point saim, saimf;
	simpleAimPoint(&saim, &saimf, H, SF);

	double window2, rand, sigx, sigy;
	switch (recgeom)
	{
	case Receiver::REC_GEOM_TYPE::CYLINDRICAL_CLOSED:
	case Receiver::REC_GEOM_TYPE::POLYGON_CLOSED:
    {
		//Aim at the closest part of the receiver
		view_az = atan2(r_to_h.i, r_to_h.j);	//The azimuth angle of the view from the receiver to the heliostat
		w2 = rec->CalculateApparentDiameter(*H.getLocation())/2.;		//half of the receiver diameter
		h2 = Rv->rec_height.val/2.;		//Half of the receiver height
		Aim->x = x_offset + w2*sin(view_az);		//x component of the aim point
		Aim->y = y_offset + w2*cos(view_az);		//y component of the aim point

		//Calculate the z position based on the image size in the height direction
		H.getImageSize(sigx, sigy);
		sigy *= tht;
		//Calculate the allowable window for sampling 
		window2 = max(h2 - args[0]*sigy,0.);
		if(args[1] == 0.){	//Triangular
			rand = _random->triangular() * _random->sign();
		}
		else if(args[1] == 1.){	//Normal
			//Sample with std dev of 0.25, so high probability of falling within range 0..1 
			//Note 1-rand will tend towards 1 for small standard deviations
			rand = _random->sign() * max(1.0-_random->normal(args[2]), 0.);	
		}
		else if(args[1] == 2.){	//uniform
			rand = 2. * _random->uniform() - 1.;
		}
        else
        {
            throw spexception("Internal error: Invalid argument #1 provided to probability shift aim point algorithm.");
        }
		Aim->z = opt_height + window2 * rand;

		//-- now calculate the aim point position in the flux plane
		//vector from simple aim point to mod aim point
		sp_point aimpos;
		aimpos.Set(Aim->x - saim.x, Aim->y - saim.y, Aim->z - saim.z);
		H.calcAndSetAimPointFluxPlane(aimpos, *rec, H);
		break;
	
		/*else if(recgeom == 1){		//1	|	Continuous open cylinder - external	
		}
		else if(recgeom == 2){		//2	|	Continuous open cylinder - internal cavity
		}
		else if(recgeom == 3){		//3	|	Planar rectangle
		}
		else if(recgeom == 4){		//4	|	Planar ellipse
		}
		else if(recgeom == 5){		//5	|	Discrete closed N-polygon - external	
		}
		else if(recgeom == 6){		//6	|	Discrete open N-polygon - external
		}
		else if(recgeom == 7){		//7	|	Discrete open N-polygon - internal cavity
		}*/
    }
	default:
		throw spexception("Receiver geometry not supported for Probability Shift Aim Point calculation"); 
	}

	return;
}

void Flux::imageSizeAimPoint(Heliostat &H, SolarField &SF, double args[], bool islast){
	/* 
	This method calculates the aim point of the heliostat "H" based on the existing flux on the receiver.
	The heliostats should be passed to this method having been sorted by image size so that the first heliostats
	will be positioned near the center of the flux plane. 

	The method will search among the possible aim points for the lowest flux portion and choose the minimum point
	that satisfies the image intercept requirement indicated by the standard deviation cutoff (args[0]). 

	For external receivers, the flux points surveyed will be in the vertical line most normal to the heliostat.

	*/
	vector<Receiver*> *Recs = SF.getReceivers();

	sp_point *hpos = H.getLocation();	//heliostat position for reference
	sp_point *Aim = H.getAimPoint();	//Point to object, this will be set below

	double tht = SF.getVarMap()->sf.tht.val;
	int isave;
	Vect r_to_h;

	calcBestReceiverTarget(&H, Recs, tht, isave, &r_to_h);

	//For the selected receiver, determine the aim point
	Receiver *rec = Recs->at(isave);
	PointVect NV;
	rec->CalculateNormalVector(*hpos, NV);	//Get the receiver normal vector
	
	//Associate the receiver with the heliostat
	H.setWhichReceiver(rec);
    
    var_receiver *Rv = rec->getVarMap();
	double opt_height = Rv->optical_height.Val(); // + rec->getOffsetZ(),       << optical height already includes Z offset
	int recgeom = rec->getGeometryType(); 
	
    double h2;

	//Get the simple aim point
	sp_point saim, saimf;
	simpleAimPoint(&saim, &saimf, H, SF);

	//----declare varibles that are used in the switch statement
	FluxSurface *FS;
	FluxGrid *FG;
	FluxPoint *Fp;
	Vect f_to_h, *fnorm, vtemp;
    sp_point *fpos, fint, Fpp, aimpos;
	Hvector HV;
	double dpsave, dprod, sigx, sigy, dx, dy, fsave, imsizex, imsizey, theta_img, rnaz, rnel, stretch_factor, ftmp;
	double e_bound_box[4];
	int nfx, nfy, ny_in, ny_del, istart, jstart, iend, jend, jsave,kk,jspan;


	switch (recgeom)
	{
	case Receiver::REC_GEOM_TYPE::CYLINDRICAL_CLOSED:
	case Receiver::REC_GEOM_TYPE::POLYGON_CLOSED:
	{
		//Get the receiver flux plane data and determine which vertical slice to aim at
		FS = &rec->getFluxSurfaces()->at(0);	//Should be only one flux surface for this type of receiver
		FG = FS->getFluxMap();

		//for the first row, which column is best?
		isave = 0;
		dpsave=-99.;
		for(int i=0; i<FS->getFluxNX(); i++){
			fpos = &FG->at(i).at(0).location;
			fnorm = &FG->at(i).at(0).normal;
			f_to_h.Set(hpos->x - fpos->x, hpos->y - fpos->y, hpos->z - (fpos->z + opt_height));
			Toolbox::unitvect(f_to_h);
			dprod = Toolbox::dotprod(f_to_h, *fnorm);
			if(dprod < 0.) continue;	//not in view
			if(dprod > dpsave){
				dpsave = dprod;
				isave = i;
			}
		}

		//Receiver dimensions
		// w2 = rec->CalculateApparentDiameter(*H.getLocation()) / 2.;		//half of the receiver diameter
		h2 = Rv->rec_height.val/2.;		//Half of the receiver height
		
		//Image size (radial) in Y
		H.getImageSize(sigx, sigy);
		sigy *= tht;
				
		//Based on the image size, determine the flux points that fall within the constrained window
		//Assume the flux points are normally spaced and each represent and equal nodal area in dY
		nfy = FS->getFluxNY();
		dy = h2*2./nfy;
		//How far in should we aim, in terms of discrete DY's, in order to maintain the sigma offset requirement?
		//scale sigy by the dot product since it is in the image plane
		dprod = -Toolbox::dotprod(FG->at(isave).at(0).normal, *H.getTowerVector());
		ny_in = int(ceil(sigy*args[0]/dprod/dy-.5));
		ny_del = (int)(ceil(sigy/dprod/dy));
		ny_in = min(ny_in, nfy/2);	//limit
		jstart = ny_in;
		//ny_del = max(1,min(ny_del, ny_in));
		jend = max(nfy - ny_in, jstart+1);
		//Search within the bounds to find the best flux point
		fsave = 9.e9;
		jsave = nfy/2;	//Pick the midpoint to begin
		ftmp = 0.;
		kk=0;
#ifdef _DEBUG
		double *fvals = new double[nfy];
		for(int i=0; i<nfy; i++){
			fvals[i] = FG->at(isave).at(i).flux;
		}
		delete [] fvals;
#endif
		jspan = 2*ny_del + 1;
		for(int j=jstart; j<jend+ny_del; j++){
			
			if(j<jend){
				ftmp += FG->at(isave).at(j).flux;
				kk++;
			}
			
			if(j>jstart+ny_del-1){
				if(j>jstart+2*ny_del){
					ftmp += -FG->at(isave).at(j-jspan).flux;
					kk--;
				}
				double ftmpave = ftmp/(double)kk;
				if( ftmpave < fsave ){
					fsave = ftmpave;
					jsave = j-ny_del;
				}
				
			}
		}
		Fp = &FG->at(isave).at(jsave);
		//Aim at the selected flux point
		Aim->Set(Fp->location.x, Fp->location.y, Fp->location.z+opt_height);
		
		//The azimuth angle of the view from the receiver to the heliostat
		//view_az = atan2(r_to_h.i, r_to_h.j);	
		
		//-- now calculate the aim point position in the flux plane
		//Bring the flux point into the image plane
		Fpp.Set(*Aim); 

		Toolbox::plane_intersect(saim, *NV.vect(), Fpp, *NV.vect(), fint);
		//vector from simple aim point to mod aim point
		aimpos.Set(fint.x - saim.x, fint.y - saim.y, fint.z - saim.z);
		H.calcAndSetAimPointFluxPlane(aimpos, *rec, H);

		//The flux grid needs to be updated after each heliostat
		HV.clear();
		HV.push_back(&H);
		fluxDensity(SF.getSimInfoObject(), *FS, HV, args[2] == 1.? true : false, islast);
		break;
	}
	case Receiver::REC_GEOM_TYPE::PLANE_RECT:
	{
		//3	|	Planar rectangle

		jsave = 0;

		//Get the receiver that this heliostat is aiming at
		FS = &rec->getFluxSurfaces()->at(0);	//Should be only one flux surface for this type of receiver
		FG = FS->getFluxMap();

		//Get the image size
		H.getImageSize(sigx, sigy);
	    tht = SF.getVarMap()->sf.tht.val;
		
		sigx *= tht;
		sigy *= tht;

		//Receiver dimensions
		//w2 = rec->getReceiverWidth()/2.;		//half of the receiver diameter
		//h2 = rec->getReceiverHeight()/2.;		//Half of the receiver height
		
		//Flux grid dimensions
		nfx = FS->getFluxNX();
		nfy = FS->getFluxNY();
		
        var_receiver* Rv = rec->getVarMap();

		dx = Receiver::getReceiverWidth( *Rv )/nfx;		//Size of each node
		dy = Rv->rec_height.val/nfy;

		/* 
		Approximate the shape of the flux image as an ellipse with dimensions A (x-axis) and B (y-axis). When projecting
		from the image plane onto the receiver plane, these dimensions will stretch according to the angles of incidence.
		*/
				
		//Rotate the heliostat vector into receiver plane coordinates
		rnaz = Rv->rec_azimuth.val*D2R;
		rnel = Rv->rec_elevation.val*D2R;
		vtemp.Set(r_to_h);
		Toolbox::rotation(Pi-rnaz, 2, vtemp);
		Toolbox::rotation(-rnel, 0, vtemp);
		
		//Calculate the skew angle of the image
		theta_img = atan2(-vtemp.i, -vtemp.k);

		//The amount of stretching is calculated using the dot product between the surface normal and the incident flux
		stretch_factor = Toolbox::dotprod(r_to_h, *NV.vect());

		//Stretch the major vertical axis of the ellipse
		imsizey = sigy*args[1]/stretch_factor;	
		imsizex = sigx*args[0];

		/* 
		Calculate the reduced aiming window based on the image size. The window defines the range of nodes that are 
		suitable as aim points given the user's requirement of displacement of the flux image centroid away from the edges
		of the aperture.
		*/
		Toolbox::ellipse_bounding_box(imsizex, imsizey, theta_img, e_bound_box);


		if(nfy > 1){
			//jstart = (int)ceil(imsizey/dy);
			jstart = (int)ceil(e_bound_box[3]/dy);
			jend = nfy - jstart;
			if(jstart > jend-1){
				jstart = nfy/2-1;
				jend = jstart + 1;
			}
		}
		else{
			jstart = 0;
			jend = 1;
		}
		//istart = (int)ceil(imsizex/dx);
		istart = (int)ceil(e_bound_box[1]/dx);
		iend = nfx - istart;
		if(nfx > 1){
			isave = istart;
			jsave = jstart;
			if(istart > iend-1){
				istart = nfx/2-1;
				iend = istart+1;
			}
		}
		else{
			istart = 0;
			iend = 1;
		}
		fsave = 9.e9;
		for(int i=istart; i<iend; i++){
			for(int j=jstart; j<jend; j++){
				if(FG->at(i).at(j).flux < fsave){
					fsave = FG->at(i).at(j).flux;
					isave = i; 
					jsave = j;
				}

			}
		}

		Fp = &FG->at(isave).at(jsave);
		//Aim at the selected flux point
		Aim->Set(Fp->location.x, Fp->location.y, Fp->location.z+opt_height);
				
		//-- now calculate the aim point position in the flux plane
		//vector to aim point in globals
		aimpos.Set(Rv->rec_offset_x.val + FS->getSurfaceOffset()->x - Aim->x, 
			Rv->rec_offset_y.val + FS->getSurfaceOffset()->y - Aim->y, 
			Rv->rec_offset_z.val + FS->getSurfaceOffset()->z +tht - Aim->z);
		
		H.calcAndSetAimPointFluxPlane(aimpos, *rec, H);
		
		//The flux grid needs to be updated after each heliostat
		HV.clear();
		HV.push_back(&H);
		fluxDensity(SF.getSimInfoObject(), *FS, HV, args[2] == 1.? true : false, islast);

		break;
	}
	case Receiver::REC_GEOM_TYPE::CYLINDRICAL_OPEN:
	case Receiver::REC_GEOM_TYPE::CYLINDRICAL_CAV:
	case Receiver::REC_GEOM_TYPE::PLANE_ELLIPSE:
	case Receiver::REC_GEOM_TYPE::POLYGON_OPEN:
	case Receiver::REC_GEOM_TYPE::POLYGON_CAV:
	default:
		throw spexception("Receiver geometry not supported for Image Size Aim Point calculation");
		return;
	}

	return;

}

void Flux::frozenAimPoint(Heliostat &H, double tht, double args[] )
{
    /* 
    The heliostat tracking vector is "frozen", but because of the way the flux density and intercept algorithms 
    implement based on aim point, we just need to update the receiver aim point for the new solar position as
    if the heliostat hadn't moved but the image on the receiver has.

    args [0,1,2] = sun position i,j,k (unit vector)
    */

    //which receiver is the heliostat currently associated with?
    Receiver *Rec = H.getWhichReceiver(); 

    //the current tracking vector
    Vect *track = H.getTrackVector();
    Vect sun;
    sun.Set( args[0], args[1], args[2] );

    //Calculate the reflected vector using the existing tracking as the normal vector
    //R = D - 2(D . N)N
    Vect R;
    Vect D;
    D.Set( -args[0], -args[1], -args[2] );
    Vect N( *track );
    R.Set( D );
    double DdN = 2. * Toolbox::dotprod(D,N);
    Vect arg( N );
    arg.Scale( DdN );
    R.Subtract( arg );

    var_receiver* Rv = Rec->getVarMap();

    //figure out where on the image plane this aim point lies
	int recgeom = Rec->getGeometryType();
    
    //Get the receiver normal vector
    PointVect norm;
    sp_point hloc;
    hloc.Set( *H.getLocation() );
    Rec->CalculateNormalVector(hloc, norm);

    sp_point aim_ip;
    Toolbox::plane_intersect(*norm.point(), *norm.vect(), hloc, R, aim_ip);

    switch (recgeom)
    {
    case Receiver::REC_GEOM_TYPE::CYLINDRICAL_CLOSED:
    case Receiver::REC_GEOM_TYPE::POLYGON_CLOSED:
    {
        
        //Set the heliostat aim point in tower coordinates
        sp_point aim_adj( aim_ip );
        aim_adj.Add( -Rv->rec_offset_x.val, -Rv->rec_offset_y.val, -Rv->rec_offset_z.val - tht );
        H.setAimPoint( aim_adj );

        //Move the aim point into receiver coordinates, accounting for any receiver rotation
        Vect *h_to_r = H.getTowerVector();
        Vect r_to_h( *h_to_r );
        r_to_h.Scale( -1. );
		double view_az = atan2(r_to_h.i, r_to_h.j);	
        Toolbox::rotation( Pi - view_az, 2, aim_adj );
        Toolbox::rotation( Pi/2. - Rv->rec_elevation.val*D2R, 0, aim_adj );
		if( fabs(aim_adj.z) < 1.e-6 ) aim_adj.z = 0.;
		//The X and Y coordinates now indicate the image plane position
		H.setAimPointFluxPlane(aim_adj);
        
        break;
    }
    case Receiver::REC_GEOM_TYPE::CYLINDRICAL_CAV:
    case Receiver::REC_GEOM_TYPE::PLANE_RECT:
    case Receiver::REC_GEOM_TYPE::PLANE_ELLIPSE:
    {

        throw spexception("Specified aim point method is not available for this geometry.");

        break;
    }
    case Receiver::REC_GEOM_TYPE::POLYGON_OPEN:
    case Receiver::REC_GEOM_TYPE::CYLINDRICAL_OPEN:
    case Receiver::REC_GEOM_TYPE::POLYGON_CAV:
    default:
        throw spexception("Specified aim point method is not available for this geometry.");
        break;
    }



}

void Flux::keepExistingAimPoint(Heliostat &H, SolarField &SF, double[] /*args*/)
{

    Receiver *rec = H.getWhichReceiver();
    
    int rec_geom = rec->getGeometryType();

    switch (rec_geom)
    {
    case Receiver::REC_GEOM_TYPE::CYLINDRICAL_CLOSED:
    case Receiver::REC_GEOM_TYPE::POLYGON_CLOSED:
    case Receiver::REC_GEOM_TYPE::PLANE_RECT:
    {
        //get the aimpoint, the heliostat location and vector to the aimpoint, and then calculate
        //the relative position of the intersection on the image plain
        sp_point *aim = H.getAimPoint();       //global coordinates
        sp_point *hloc = H.getLocation();

        Vect h_to_r;
        h_to_r.Set(aim->x - hloc->x, aim->y - hloc->y, aim->z - hloc->z);   //vector from heliostat to receiver -- aimpoint line
        Toolbox::unitvect( h_to_r );    //unit vector

        //calculate the position of the receiver image plane
        Receiver *rec = H.getWhichReceiver();
        PointVect NV;
        rec->CalculateNormalVector(*hloc, NV);
        /*sp_point fluxplanept = *NV.point();
        Vect fluxplanevect = *NV.vect();*/
        
        //calculate the intersection point
        sp_point int_pt;
        Toolbox::plane_intersect(*NV.point() /* global coordinates */, *NV.vect(), *aim, h_to_r, int_pt);

        //Get the simple aim point
	    sp_point saim, saimf;
	    simpleAimPoint(&saim, &saimf, H, SF);

        //Move into receiver coordinates
        sp_point aim_rec(int_pt);
        aim_rec.Subtract(saim);

        //Rotate the intersection point into receiver coordinates
		H.calcAndSetAimPointFluxPlane(aim_rec, *rec, H);


        break;
    }
    case Receiver::REC_GEOM_TYPE::PLANE_ELLIPSE:
    case Receiver::REC_GEOM_TYPE::CYLINDRICAL_OPEN:
    case Receiver::REC_GEOM_TYPE::CYLINDRICAL_CAV:
    case Receiver::REC_GEOM_TYPE::POLYGON_OPEN:
    case Receiver::REC_GEOM_TYPE::POLYGON_CAV:
    default:
		throw spexception("Receiver geometry not supported for Keep Existing Aimpoint calculation");
        break;
    }






    
    //sp_point *fs = rec->getFluxSurfaces()->front().getSurfaceOffset();
    //sp_point *hp = h->getAimPoint();

    //sp_point aim;        //The current aim point in receiver coordinates
    //aim.Set(*hp);
    //aim.z = -aim.z +_tht;
    //aim.Add(*fs);

    ////A point on the image plane
    //       
    ////Get receiver normal vector
    //PointVect NV;
    //rec->CalculateNormalVector(*h->getLocation(), NV);
    //sp_point pint;
    //Toolbox::plane_intersect(aim, *NV.vect(), Fpp, *NV.vect(), pint);

    //Heliostat::calcAndSetAimPointFluxPlane(pint, *h->getWhichReceiver(), *h);

}

void Flux::calcBestReceiverTarget(Heliostat *H, vector<Receiver*> *Recs, double tht, int &rec_index, Vect *rtoh){
	/* 
	Take an existing heliostat 'H' and all possible receivers "Recs" and determine which receiver 'rec_index'
	provides the best view factor between the heliostat and the receiver.

	Optional argument 'rtoh' is a vector from the receiver to the heliostat that is calculated within this algorithm
	*/
	int i, isave, Nrec;
	PointVect NV;	//Normal vector from the receiver
	Vect r_to_h;	//receiver to heliostat vector
	vector<double> projarea(Recs->size(),0.0);	//Vector of stored "normality" efficiencies for each receiver
	double Arec, projarea_max;	//Receiver effective area

	Nrec = (int)Recs->size();	//The number of receivers to choose from
	double slant;
	sp_point *hpos = H->getLocation();

	//If we only have 1 receiver, don't bother
	if(Nrec==1){
		isave = 0;
		slant = sqrt(tht*tht + hpos->x*hpos->x + hpos->y*hpos->y); // A very approximate slant range
		r_to_h.i = hpos->x/slant;
		r_to_h.j = hpos->y/slant;
		r_to_h.k = (hpos->z - tht)/slant;
		Recs->at(0)->CalculateNormalVector(*hpos, NV);	//Get the receiver normal vector
			
	}
	else{
		//Determine the projected area for each receiver
		isave = 0; projarea_max = -9.e99;
		for(i=0; i<Nrec; i++){
			//Calculate a rough receiver-to-heliostat vector
			slant = sqrt(pow(tht-hpos->z,2) + hpos->x*hpos->x + hpos->y*hpos->y); // A very approximate slant range
			r_to_h.i = hpos->x/slant;
			r_to_h.j = hpos->y/slant;
			r_to_h.k = (hpos->z - tht)/slant;

			//Calculate the receiver projected area before accounting for heliostat view
			double width;
			if( Recs->at(i)->getGeometryType() == Receiver::REC_GEOM_TYPE::POLYGON_CLOSED )
				width = Recs->at(i)->CalculateApparentDiameter(*H->getLocation());
			else
				width = Receiver::getReceiverWidth( *Recs->at(i)->getVarMap() );
			Arec = Recs->at(i)->getVarMap()->rec_height.val * width;

			Recs->at(i)->CalculateNormalVector(*hpos, NV);	//Get the receiver normal vector
			projarea.at(i) = dotprod(*NV.vect(), r_to_h) * Arec;	//Calculate the dotproduct of the receiver normal and the receiver to heliostat vector. 
														//This determines how well the heliostat can see the receiver.
			//Is this the best one?
			if(projarea.at(i) > projarea_max){ 
				projarea_max = projarea.at(i);
				isave = i;
			}
		}
	}
	
	if(rtoh != 0){
		rtoh->i = r_to_h.i;
		rtoh->j = r_to_h.j;
		rtoh->k = r_to_h.k;
	}


	//For the selected receiver, determine the aim point
	rec_index = isave;
	return;
	
}
