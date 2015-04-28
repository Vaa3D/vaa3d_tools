#include "Random.h"
#include "stdlib.h"
#include "math.h"

 double Random::rnd01(){
	

	double ii=((double) rand ()) / ((double)RAND_MAX);

	return  ii;
} /* end rnd01 */

;

void Random::setSeed (int seed) {
		srand (seed);
	} /* end rndseed */

double Random::rnduniform (double min, double max) {
	return min + (max - min) * rnd01 ();
} /* end rnduniform */


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@                                                                            @
//@     Title:  double gaussdev ()                                             @
//@                                                                            @
//@     Action: returns a random number with a gaussian distribution (0,1)     @
//@             "Algorithm from Numerical Recipes in C", Second Ed.            @
//@             Section 7.2, Page 289                                          @
//@                                                                            @
//@     Input:  none    						       @
//@     Output: random number with the gaussian distribution (0,1).            @
//@                                                                            @
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

double Random::gaussdev () {
	
	double fac, rsq, v1, v2;
	
	if (!iset) {
		do {
			v1 = 2.0 * rnd01 () - 1.0;		
			v2 = 2.0 * rnd01 () - 1.0;
			rsq = v1*v1 + v2*v2;
		} while (rsq >= 1.0 || rsq == 0.0);
	
		fac = sqrt (-2.0*log(rsq)/rsq);
		gset = v1*fac;
		iset = 1;
		return v2*fac;
	}
	else {
		iset = 0;
		return gset;
	}
	
} /* end gaussdev */

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@                                                                            @
//@     Title:  double rndgauss (double mean, double stdev)                    @
//@                                                                            @
//@     Action: returns a random number with a gaussian distribution.          @
//@             and max.  "Algorithm from Numerical Recipes in C", Second Ed.  @
//@             Section 7.2, Page 289                                          @
//@                                                                            @
//@     Input:  mean - the mean value of the gaussian distribution.            @
//@     		stdev - the standard deviation of the gaussian distribution.   @
//@     Output: random number with the given gaussian distribution.            @
//@                                                                            @
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

double Random::rndgauss (double mean, double stdev) {
	
	return mean + stdev * (gaussdev ());
	
} /* end rndgauss */





/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@                                                                            @
//@     Title:  double gamdev (int ia mean)                                    @
//@                                                                            @
//@     Action: returns a random number with a gammaa distribution.            @
//@             and max.  "Algorithm from Numerical Recipes in C", Second Ed.  @
//@                                                                            @
//@                                                                            @
//@     Input:  ia - order of the distribution                                 @
//@     		                                                               @
//@     Output: random number with the given  distribution.                    @
//@                                                                            @
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

double Random::gamdev(double a,double lambda)
{
///////////////////////////////////////////////////////////////
//	double  RandGamma::genGamma( HepRandomEngine *anEngine,
 //                              double  a, double  lambda ) {
/*************************************************************************
 *         Gamma Distribution - Rejection algorithm gs combined with     *
 *                              Acceptance complement method gd          *
 *************************************************************************/

static double  aa = -1.0, aaa = -1.0, b, c, d, e, r, s, si, ss, q0,
       q1 = 0.0416666664, q2 =  0.0208333723, q3 = 0.0079849875,
       q4 = 0.0015746717, q5 = -0.0003349403, q6 = 0.0003340332,
       q7 = 0.0006053049, q8 = -0.0004701849, q9 = 0.0001710320,
       a1 = 0.333333333,  a2 = -0.249999949,  a3 = 0.199999867,
       a4 =-0.166677482,  a5 =  0.142873973,  a6 =-0.124385581,
       a7 = 0.110368310,  a8 = -0.112750886,  a9 = 0.104089866,
       e1 = 1.000000000,  e2 =  0.499999994,  e3 = 0.166666848,
       e4 = 0.041664508,  e5 =  0.008345522,  e6 = 0.001353826,
       e7 = 0.000247453;

double  gds,p,q,t,sign_u,u,v,w,x;
double  v1,v2,v12;

// Check for invalid input values

 if( a <= 0.0 ) return (-1.0);
 if( lambda <= 0.0 ) return (-1.0);

 if (a < 1.0)
   {          // CASE A: Acceptance rejection algorithm gs
    b = 1.0 + 0.36788794412 * a;       // Step 1
    for(;;)
      {
       p = b * rnd01();
       if (p <= 1.0)
	  {                            // Step 2. Case gds <= 1
	   gds = exp(log(p) / a);
	   if (log(rnd01()) <= -gds) return(gds/lambda);
	  }
       else
	  {                            // Step 3. Case gds > 1
	   gds = - log ((b - p) / a);
	   if (log(rnd01()) <= ((a - 1.0) * log(gds))) return(gds/lambda);
	  }
      }
   }
 else
   {          // CASE B: Acceptance complement algorithm gd
    if (a != aa)
       {                               // Step 1. Preparations
	aa = a;
	ss = a - 0.5;
	s = sqrt(ss);
	d = 5.656854249 - 12.0 * s;
       }
                                              // Step 2. Normal deviate
    do {
      v1 = 2.0 * rnd01() - 1.0;
      v2 = 2.0 * rnd01() - 1.0;
      v12 = v1*v1 + v2*v2;
    } while ( v12 > 1.0 );
    t = v1*sqrt(-2.0*log(v12)/v12);
    x = s + 0.5 * t;
    gds = x * x;
    if (t >= 0.0) return(gds/lambda);         // Immediate acceptance

    u = rnd01();            // Step 3. Uniform random number
    if (d * u <= t * t * t) return(gds/lambda); // Squeeze acceptance

    if (a != aaa)
       {                               // Step 4. Set-up for hat case
	aaa = a;
	r = 1.0 / a;
	q0 = ((((((((q9 * r + q8) * r + q7) * r + q6) * r + q5) * r + q4) *
			  r + q3) * r + q2) * r + q1) * r;
	if (a > 3.686)
	   {
	    if (a > 13.022)
	       {
		b = 1.77;
		si = 0.75;
		c = 0.1515 / s;
	       }
	    else
	       {
		b = 1.654 + 0.0076 * ss;
		si = 1.68 / s + 0.275;
		c = 0.062 / s + 0.024;
	       }
	   }
	else
	   {
	    b = 0.463 + s - 0.178 * ss;
	    si = 1.235;
	    c = 0.195 / s - 0.079 + 0.016 * s;
	   }
       }
    if (x > 0.0)                       // Step 5. Calculation of q
       {
	v = t / (s + s);               // Step 6.
	if (fabs(v) > 0.25)
	   {
	    q = q0 - s * t + 0.25 * t * t + (ss + ss) * log(1.0 + v);
	   }
	else
	   {
	    q = q0 + 0.5 * t * t * ((((((((a9 * v + a8) * v + a7) * v + a6) *
			    v + a5) * v + a4) * v + a3) * v + a2) * v + a1) * v;
	   }                // Step 7. Quotient acceptance
	if (log(1.0 - u) <= q) return(gds/lambda);
       }

    for(;;)
       {                    // Step 8. Double exponential deviate t
	do
	{
	 e = -log(rnd01());
	 u = rnd01();
	 u = u + u - 1.0;
	 sign_u = (u > 0)? 1.0 : -1.0;
	 t = b + (e * si) * sign_u;
	}
	while (t <= -0.71874483771719);   // Step 9. Rejection of t
	v = t / (s + s);                  // Step 10. New q(t)
	if (fabs(v) > 0.25)
	   {
	    q = q0 - s * t + 0.25 * t * t + (ss + ss) * log(1.0 + v);
	   }
	else
	   {
	    q = q0 + 0.5 * t * t * ((((((((a9 * v + a8) * v + a7) * v + a6) *
			    v + a5) * v + a4) * v + a3) * v + a2) * v + a1) * v;
	   }
	if (q <= 0.0) continue;           // Step 11.
	if (q > 0.5)
	   {
	    w = exp(q) - 1.0;
	   }
	else
	   {
	    w = ((((((e7 * q + e6) * q + e5) * q + e4) * q + e3) * q + e2) *
				     q + e1) * q;
	   }                    // Step 12. Hat acceptance
	if ( c * u * sign_u <= w * exp(e - 0.5 * t * t))
	   {
	    x = s + 0.5 * t;
	    return(x*x/lambda);
	   }
       }
   }



}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@                                                                            @
//@     Title:  double rndgamma (double mean, double stdev)                    @
//@                                                                            @
//@     Action: returns a random number with a gamma distribution.          @
//@             and max.  "Algorithm from Numerical Recipes in C", Second Ed.  @
//@             Section 7.2, Page 289                                          @
//@                                                                            @
//@     Input:  mean - the mean value of the gaussian distribution.            @
//@     		stdev - the standard deviation of the gaussian distribution.   @
//@     Output: random number with the given gaussian distribution.            @
//@                                                                            @
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

double Random::rndgamma(double alpha, double lambda, double offset) {
	
	return offset*(gamdev (alpha,lambda));
	
} /* end rndgauss */


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@                                                                            @
//@     Title:  double expdev (int ia mean)                                    @
//@                                                                            @
//@     Action: returns a random number with a exp distribution.            @
//@             and max.  "Algorithm from Numerical Recipes in C", Second Ed.  @
//@                                                                            @
//@                                                                            @
//@                             @
//@     		                                                               @
//@     Output: random number with the given  distribution.            @
//@                                                                            @
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

double Random::expdev()
{
       
        double dum;

        do
                dum=rnd01();
        while (dum == 0.0);
        return -log(dum);
}
;
