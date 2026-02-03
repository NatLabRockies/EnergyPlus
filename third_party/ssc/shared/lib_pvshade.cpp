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

#include "lib_pvshade.h"
#include "lib_util.h"

#include <math.h>
#include <limits>
#include <vector>


#ifndef M_EPS
#define M_EPS 0.00001
#endif

// SUPPORTING MATH AND INTEGRATION FUNCTIONS
/*
static double atand( double radians )
{
	// return angle in radians (-pi/2, pi/2) or in degrees (-90,90)
	double arctan_val = atan( radians );
	while ( arctan_val > M_PI/2.0) arctan_val -= M_PI;
	while ( arctan_val < -M_PI/2.0) arctan_val += M_PI;
	return (180.0/ M_PI) * arctan_val; //convert angle from radians to degrees
}
*/

/*static double round(double number)
{
    return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
}*/



// mask angle function for integration from documentation\PV\Shading\ChrisDeline\2012.4.23\SAM shade geometry_v2.docx
double mask_angle_func(double x, double R, double B, double tilt_eff)
{
	return atan2( (B-x) * sind(tilt_eff), (R-B*cosd(tilt_eff) + x*cosd(tilt_eff)) );
}



// Romberg integration for phi_bar from Numerical Recipes in C
#define EPS 1.0e-6
#define JMAX 20
#define JMAXP (JMAX+1)
#define K 5
#define FUNC(x,R,B,tilt) ((*func)(x,R,B,tilt))

double trapzd(double (*func)(double,double,double,double), double a, double b, double R, double B, double tilt, int n)
{
	double x,tnm,sum,del;
	static double s;
	int it,j;
	if (n == 1)
	{
		return (s=0.5*(b-a)*(FUNC(a,R,B,tilt)+FUNC(b,R,B,tilt)));
	}
	else
	{
		for (it=1,j=1;j<n-1;j++) it <<= 1;
		tnm=it;
		del=(b-a)/tnm; /*This is the spacing of points to be added. */
		x=a+0.5*del;
		for (sum=0.0,j=1;j<=it;j++,x+=del) sum += FUNC(x,R,B,tilt);
		s=0.5*(s+(b-a)*sum/tnm); /*This replaces s by its refined value.*/
		return s;
	}
}
/* ********************************************************************* */
void polint(double xa[], double ya[], int n, double x, double *y, double *dy)
{
	int i,m,ns=1;
	double den,dif,dift,ho,hp,w;
//	double *c,*d;
//	c=vector(1,n);
//	d=vector(1,n);
	size_t size = n+1;
	std::vector<double> c(size);
	std::vector<double> d(size);
	dif=fabs(x-xa[1]);

	for (i=1;i<=n;i++)
	{
		if ( (dift=fabs(x-xa[i])) < dif)
		{
			ns=i;
			dif=dift;
		}
		c[i]=ya[i];
		d[i]=ya[i];
	}
	*y=ya[ns--];
	for (m=1;m<n;m++)
	{
		for (i=1;i<=n-m;i++)
		{
			ho=xa[i]-x;
			hp=xa[i+m]-x;
			w=c[i+1]-d[i];
			den = ho-hp;
			//if ( (den=ho-hp) == 0.0) nrerror("Error in routine polint");
			if (den != 0) den=w/den;
			d[i]=hp*den;
			c[i]=ho*den;
		}
		*y += (*dy=(2*ns < (n-m) ? c[ns+1] : d[ns--]));
	}
//free_vector(d,1,n);
//free_vector(c,1,n);
}
/* ********************************************************************* */
double qromb(double (*func)(double,double,double,double), double a, double b, double R, double B, double tilt)
{
	void polint(double xa[], double ya[], int n, double x, double *y, double *dy);
	double trapzd(double (*func)(double,double,double,double), double a, double b, double R, double B, double tilt, int n);
	void nrerror(char error_text[]);
	double ss,dss;
	double s[JMAXP],h[JMAXP+1];
	int j;
	h[1]=1.0;
	for (j=1;j<=JMAX;j++)
	{
		s[j]=trapzd(func,a,b,R,B,tilt,j);
		if (j >= K)
		{
			polint(&h[j-K],&s[j-K],K,0.0,&ss,&dss);
			if (fabs(dss) <= EPS*fabs(ss)) return ss;
		}
		h[j+1]=0.25*h[j];
	}
	//nrerror("Too many steps in routine qromb");
	return 0.0;
}
// end of Romberg integration functions

// SUPPORTING FUNCTION DEFINITIONS

void diffuse_reduce(
	// inputs (angles in degrees)
	double solzen,
    double stilt,
    double Gb_nor,
    double Gdh,
    double poa_sky,
    double poa_gnd,
    double gcr,
//	double phi0, // mask angle
	double alb,
    double nrows,
    sssky_diffuse_table &skydiffderates,
	// outputs
	double &reduced_skydiff,
    double &Fskydiff,  // derate factor on sky diffuse
	double &reduced_gnddiff,
    double &Fgnddiff) // derate factor on ground diffuse
{
	double Gd_poa = poa_sky + poa_gnd;
	if (Gd_poa < 0.1)
	{
		Fskydiff = Fgnddiff = 1.0;
		return;
	}

	// view factor  and shade derate calculations assume isotropic sky
	double Gbh = Gb_nor * cosd(solzen); // beam irradiance on horizontal surface
	// double poa_sky_iso = Gdh * (1 + cosd(stilt)) / 2;

	double B = 1.0;
	double R = B / gcr;

    // sky diffuse reduction
    Fskydiff = skydiffderates.lookup(stilt);
    reduced_skydiff = Fskydiff * poa_sky;

	double solalt = 90 - solzen;

	// ground reflected reduction
	double F1 = alb * pow(sind(stilt / 2.0), 2);
	double Y1 = R - B * sind(180.0 - solalt - stilt) / sind(solalt);
	Y1 = fmax(0.00001, Y1); // constraint per Chris 4/23/12
	double F2 = 0.5 * (1.0 + Y1 / B - sqrt(pow(Y1, 2) / pow(B, 2) - 2 * Y1 / B * cosd(180 - stilt) + 1.0));
	double F3 = 0.5 * (1.0 + R / B - sqrt(pow(R, 2) / pow(B, 2) - 2 * R / B * cosd(180 - stilt) + 1.0));

	double Gr1 = F1 * (Gbh + Gdh);
	double reduced_gnddiff_iso = ((F1 + (nrows - 1) * F1 * F2) / nrows) * Gbh
		                          + ((F1 + (nrows - 1) * F1 * F3) / nrows) * Gdh;

	Fgnddiff = 1.0;
	if (Gr1 > 0)
		Fgnddiff = reduced_gnddiff_iso / Gr1;
	reduced_gnddiff = Fgnddiff * reduced_gnddiff_iso;
}

double selfshade_dc_derate(double X, double S, double FF0, double dbh_ratio, double m_d, double Vmp)
{
	double Xtemp = fmin(X, 0.65);  // X is limited to 0.65 for c2 calculation

	double c1 = (109 * FF0 - 54.3) * exp(-4.5 * X); // new c1 on 1/18/13
	double c2 = -6 * pow(Xtemp, 2) + 5 * Xtemp + 0.28; // new c2 on 1/18/13
	double c3_0 = (-0.05 * dbh_ratio - 0.01) * X + (0.85 * FF0 - 0.7) * dbh_ratio - 0.085 * FF0 + 0.05;  //new c3_0 on 1/18/13
	double c3 = fmax(c3_0, (dbh_ratio)-1.0);
	double eqn5 = 1.0 - c1 * pow(S, 2) - c2 * S;  // new eqn5 on 1/18/13
	double eqn9 = 0;

	if (X != 0) eqn9 = (X - S * (1 + 0.5 / (Vmp / m_d))) / X; //updated 8/7/14 jmf per Chris Deline to match published paper

	double eqn10 = c3 * (S - 1.0) + (dbh_ratio);

	double reduc = fmax(eqn5, eqn9);
	reduc = fmax(reduc, eqn10);
	reduc = X * reduc + (1.0 - X);

	// check limits
	if (reduc > 1) reduc = 1.0;
	if (reduc < 0) reduc = 0.0;

	return reduc;
}

void selfshade_xs_horstr(bool landscape,
	double W,   // module width (short side)
	double L,   // module length (long side)
	int r,      // number of rows
	int m,      // number of modules along row edge (short side of assembly)
	int n,      // number of modules along (long side of assembly)
	int ndiode, // number of bypass diodes
	double Fshad, // Fraction of assembly shaded up from long edge

	// outputs (pass by reference)
	double &X, double &S)
{
	// if number of modules across bottom > number in string and horizontal wiring then g=0
	double g = 0; // assume very long rows

	if (landscape) // Landscape mode
	{
		double Hs = Fshad * m * W;


		if (Hs <= W)
		{ // Situation 1a
			X = (ceil(Hs / W) / (m * r)) * (r - 1.0);
			// updated to more conservative approach - email from Chris 4/10/12
			S = (ceil(Hs * ndiode / W) / ndiode) * (1.0 - floor(g / L) / n);
		}
		else // Hs > width
		{  // Situation 1b
			X = (ceil(Hs / W) / (m * r)) * (r - 1.0);
			S = 1.0;
		}
	}
	else // Portrait mode
	{  // Situation 2
		double Hs = Fshad * m * L;

		X = (ceil(Hs / L) / (m * r)) * (r - 1.0);
		S = 1.0 - (floor(g * ndiode / W) / (ndiode * n));
	}
}

// Accessor for sky diffuse derates for the given surface_tilt. If the value doesn't exist in the table, it is computed.
double sssky_diffuse_table::lookup(double surface_tilt) {
    char buf[8];
    sprintf(buf, "%.3f", surface_tilt);
    if (derates_table.find(buf) != derates_table.end())
        return derates_table[buf];
    return compute(surface_tilt);
}

double sssky_diffuse_table::compute(double surface_tilt) {
    if (gcr == 0)
        throw std::runtime_error("sssky_diffuse_table::compute error: gcr required in initialization");
    // sky diffuse reduction
    double step = 1.0 / 1000.0;
    double skydiff = 0.0;
    double tand_stilt = tand(surface_tilt);
    double sind_stilt = sind(surface_tilt);
    double Asky = M_PI + M_PI / pow((1 + pow(tand_stilt, 2)), 0.5);
    double arg[1000];
    double gamma[1000];
    double tan_tilt_gamma[1000];
    double Asky_shade[1000];
    for (int n = 0; n < 1000; n++)
    {
        if (surface_tilt != 0) {
            arg[n] = (1 / tand_stilt) - (1 / (gcr * sind_stilt * (1 - n * step)));
            gamma[n] = (-M_PI / 2) + atan(arg[n]);
            tan_tilt_gamma[n] = tan(surface_tilt * DTOR + gamma[n]);
            Asky_shade[n] = M_PI + M_PI / pow((1 + tan_tilt_gamma[n] * tan_tilt_gamma[n]), 0.5);
            if ((surface_tilt * DTOR + gamma[n]) > (M_PI / 2))
            {
                Asky_shade[n] = 2 * M_PI - Asky_shade[n];
            }
            skydiff += (Asky_shade[n] / Asky) * step;
        } else {
            arg[n] = std::numeric_limits<double>::quiet_NaN();
            gamma[n]  = std::numeric_limits<double>::quiet_NaN();
            tan_tilt_gamma[n] = std::numeric_limits<double>::quiet_NaN();
            Asky_shade[n] = Asky;
            skydiff += step;
        }
    }
    char buf[8];
    sprintf(buf, "%.3f", surface_tilt);
    derates_table[buf] = skydiff;
    return skydiff;
}

// self-shading calculation function
/*

Chris Deline 4/9/2012 - updated 4/19/2012 - update 4/23/2012
see SAM shade geometry_v2.docx
Updated 1/18/13 to match new published coefficients in Solar Energy "A simplified model of uniform shading in large photovoltaic arrays"

Definitions of X and S in SAM for the four layout conditions � portrait, landscape and vertical / horizontal strings.
Definitions:
S: Fraction of submodules that are shaded in a given parallel string
X: Fraction of parallel strings in the system that are shaded
m: modules along side of row
n: modules along bottom of row
d:  # of diodes per module
W: module width
L: module length
r: number of rows
Hs: shadow height along inclined plane from Applebaum eqn. A13
g: shadow distance from row edge from Applebaum eq. A12

B: array length along side (m*L in portrait, m*W in landscape configuration) = Appelbaum paper A
beta: effective tilt angle
alpha: solar elevation angle
R: inter-row spacing
phi_bar: average masking angle

*/
bool ss_exec(

        const ssinputs &inputs,

        double tilt,		// module tilt (constant for fixed tilt, varies for one-axis)
	double azimuth,		// module azimuth (constant for fixed tilt, varies for one-axis)
	double solzen,		// solar zenith (deg)
	double solazi,		// solar azimuth (deg)
	double Gb_nor,		// beam normal irradiance (W/m2)
	double Gdh,         // diffuse horizontal irradiance (W/m2)
	double Gb_poa,		// POA beam irradiance (W/m2)
	double poa_sky,		// POA diffuse sky irradiance (W/m2)
	double poa_gnd,     // POA diffuse gnd irradiance (W/m2)
	double albedo,		// used to calculate reduced relected irradiance
	bool trackmode,		// 0 for fixed tilt, 1 for one-axis tracking
	bool linear,		// 0 for non-linear shading (C. Deline's full algorithm), 1 to stop at linear shading
	double shade_frac_1x,	// geometric calculation of the fraction of one-axis row that is shaded (0-1), not used if fixed tilt

	sssky_diffuse_table &skydiffs,
        ssoutputs &outputs)
{

	// ***********************************
	// VARIABLE ASSIGNMENTS
	// ***********************************

	// translate inputs to variable names consistent with C. Deline's self-shading paper for clarity
	double m_m = inputs.nmody;
	double m_n = inputs.nmodx;
	double m_d = inputs.ndiode;
	double m_W = inputs.width;
	double m_L = inputs.length;
	double m_r = inputs.nrows;
	double m_R = inputs.row_space;

	// check for divide by zero issues with Row spacing per email from Chris 5/2/12
	if (m_R < M_EPS) m_R = M_EPS;

	// calculate the mask angle
	// NOTE THAT B HERE IS PER CHRIS DELINE'S PAPER: B IS THE LENGTH OF THE SIDE OF A ROW
	double m_B;
	if (inputs.mod_orient == 0) m_B = m_L * m_m;	// Portrait Mode
	else m_B = m_W * m_m;	// Landscape Mode

	// calculate the length of the row also
	double m_row_length;
	if (inputs.mod_orient == 0) m_row_length = m_n * m_W; //Portrait Mode
	else m_row_length = m_n * m_L; //Landscape Mode

	//double a = 0.0, b = m_B;
	/*
	double mask_angle;
	if (inputs.mask_angle_calc_method == 1)
	{
	// average over entire array
		mask_angle = qromb( mask_angle_func, a, b, m_R, m_B, tilt) / m_B;
	}
	else
	{
	// worst case (default)
	// updated to phi(0) per email from Chris Deline 5/2/12
		mask_angle = atan2( ( m_B * sind( tilt ) ), ( m_R - m_B * cosd( tilt ) ) );
	}
	mask_angle *= 180.0/M_PI; // change to degrees to pass into functions later
	*/
	// ***********************************
	// SHADOW DIMENSION CALCULATIONS
	// ***********************************
	// Reference Appelbaum and Bany "Shadow effect of adjacent solar collectors in large scale systems" Solar Energy 1979 Vol 23. No. 6

	double m_A; //NOTE THAT THIS IS APPLEBAUM A, WHICH IS THE ROW SIDE WIDTH, NOT DELINE A, WHICH IS THE ROW LENGTH
	// APPLEBAUM A IS EQUAL TO DELINE B
	m_A = m_B;

	double px, py;
	double S,X;

	// AppelBaum Appendix A
	double g, Hs;

	/* two assumptions in Applebaum paper:
		1. Azimuth = 0 is facing toward sun (south in northern hemisphere)
		2. Array azimuth is 0 degrees
	   to reconcile these assumptions, use an effective azimuth (az_eff) that is the difference between array az and solar az
	*/
	double az_eff = solazi - azimuth;

	// if no effective tilt, or sun is down, then no array self-shading
	if ((solzen < 90.0) && (tilt != 0) && (fabs(az_eff) < 90.0) )
	{
		// Appelbaum eqn (12)
		py = m_A * (cosd(tilt) + ( cosd(az_eff) * sind(tilt) /tand(90.0-solzen) ) );
		// Appelbaum eqn (11)
		px = m_A * sind(tilt) * sind(az_eff) / tand(90.0-solzen);
	}
	else //! Otherwise the sun has set
	{
		py = 0;
		px = 0;
	}

	// Appelbaum equation A12  Xe = R*Px/Py
	if (py == 0)
		g = 0;
	else
		g = m_R * px / py;

	// Additional constraints from Chris 4/11/12
	g = fmax(g, 0); //fabs(g);	//g must be positive
	g = fmin(g, m_row_length);	//g can't be greater than the length of the row

	// if number of modules across bottom > number in string and horizontal wiring then g=0
	// Chris Deline email 4/19/12
	if ( ( inputs.str_orient == 1 ) && ( inputs.nstrx > 1 ) ) // Horizontal wiring
	{
		g = 0;
	}

	// Appelbaum equation A13  Hs = EF = A(1 - R/Py)
	if (py == 0)
		Hs = 0;
	else
		Hs = m_A * (1.0 - m_R / py);

	//overwrite Hs using geometrically calculated shade fraction for one-axis trackers
	if (trackmode == 1)
	{
		Hs = shade_frac_1x * m_B;
	}

	// Additional constraints from Chris 4/11/12
	Hs = fmax( Hs, 0.0);	// Hs must be positive
	Hs = fmin( Hs, m_B);	// Hs cannot be greater than the height of the row

	//calculate the relative shaded area, Applebaum equation A15
	//this would only apply to linearly shaded systems, but we'll report it as an output
	//in order to be able to distinguish the linear beam shading component from the non-linear dc effect
	//in loss diagrams and such.
	double relative_shaded_area = Hs * (m_row_length - g) / (m_A * m_row_length); //numerator is shadow area, denom is row area
	outputs.m_shade_frac_fixed = relative_shaded_area;

	//now we'll apply the diffuse reduction and return because we're done for linearly shaded systems
	if (linear)
	{
		//determine reduction of diffuse incident on shaded sections due to self-shading (beam is not derated because that shading is taken into account in dc derate)
		diffuse_reduce(solzen, tilt, Gb_nor, Gdh, poa_sky, poa_gnd, m_B / m_R, albedo, m_r, skydiffs,
			// outputs
			outputs.m_reduced_diffuse, outputs.m_diffuse_derate, outputs.m_reduced_reflected, outputs.m_reflected_derate);

		return true;
	}

	// X and S from Chris Deline 4/23/12
	if ( inputs.str_orient == 1 ) // Horizontal wiring
	{
		if ( inputs.mod_orient == 1 ) // Landscape mode
		{
			if ( Hs <= m_W )
			{ // Situation 1a
				X = ( ceil( Hs / m_W ) / (m_m * m_r) ) * ( m_r - 1.0);
				// updated to more conservative approach - email from Chris 4/10/12
				//S = round( Hs * D / W ) / D - floor( Xe / L ) / N;
				S = ( ceil( Hs * m_d / m_W ) / m_d ) * ( 1.0 - floor( g / m_L ) / m_n);
			}
			else // Hs > width
			{  // Situation 1b
				X = ( ceil( Hs / m_W ) / (m_m * m_r) ) * ( m_r - 1.0);
			 	S = 1.0;
			}
		}
		else // Portrait mode
		{  // Situation 2
			X = ( ceil( Hs / m_L ) / (m_m * m_r) ) * ( m_r - 1.0);
			S = 1.0 - ( floor( g * m_d / m_W ) / ( m_d * m_n) );
		}
	}
	else // Vertical wiring
	{  // Situation 3
		if ( inputs.mod_orient == 0 ) // Portrait mode
		{
			X = 1.0 - ( floor( g / m_W ) / m_n );
			S = ( ceil( Hs / m_L ) / ( m_m * m_r ) ) * (m_r - 1.0);
		}
		else // Landscape
		{   // Situation 4
			X = 1.0 - ( floor( g / m_L ) / m_n );
			// updated to more conservative approach - email from Chris 4/10/12
			//S = ( round( Hs * D / W ) / (D * M * R) ) * (R - 1.0);
			S = ( ceil( Hs * m_d / m_W ) / (m_d * m_m * m_r) ) * (m_r - 1.0);
		}
	}

	// overwrite S to be 1 for one-axis trackers- assume entire row is shaded
	if (trackmode == 1)
	{
		S = 1;
	}

	//Chris Deline's self-shading algorithm

	// 1. determine reduction of diffuse incident on shaded sections due to self-shading (beam is not derated because that shading is taken into account in dc derate)
	diffuse_reduce(solzen, tilt, Gb_nor, Gdh, poa_sky, poa_gnd, m_B/m_R, albedo, m_r, skydiffs,
		// outputs
		outputs.m_reduced_diffuse, outputs.m_diffuse_derate, outputs.m_reduced_reflected, outputs.m_reflected_derate );

	// 2. Calculate the "Reduced Irradiance Fraction" (Ee in Chris' paper)
	double inc_total =  ( Gb_poa + outputs.m_reduced_diffuse + outputs.m_reduced_reflected)/1000;
	double inc_diff = (outputs.m_reduced_diffuse + outputs.m_reduced_reflected)/1000;
	double diffuse_globhoriz = 0;
	if (inc_total != 0)
		diffuse_globhoriz = inc_diff / inc_total;

	// 3. Calculate the dc power derate based on C.Deline et al., "A simplified model of uniform shading in large photovoltaic arrays" (Psys/Psys0 in the paper, which is equivalent to a derate)
	outputs.m_dc_derate = selfshade_dc_derate( X, S, inputs.FF0, diffuse_globhoriz, m_d, inputs.Vmp );

	return true;
}
