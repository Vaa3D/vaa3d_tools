/* C++ IMPLEMENTATION OF PRINCIPAL CURVE TRACING USING MEX INTERFACE

Code developed by: Nikhila Srikanth
				   M.S. Project
				   Cognitive Systems Laboratory
				   Electrical and Computer Engineering Department
				   Northeastern University, Boston, MA

Under the guidance of: Prof. Deniz Erdogmus and Dr. Erhan Bas	

Date Created: Jan 20, 2011
Date Last Updated: April 29, 2011

This code aims at converting the MATLAB implementation of 2-D eigen decomposition 
of Principal Curve Tracing algorithm into C++. This code is meant to run on
the Windows platform
*/

#include <mex.h>
#include <matrix.h>
#include <math.h>

#include "EigenDec_2D.h"

void Compute_Eig_Dec_2D (int iNum_of_pixels, double *adDxx, double *adDxy, double *adDyy, 
		double *adLambda1, double *adLambda2, double *adV1x, double *adV1y, double *adV2x, double *adV2y)
{

	double dCur_Dxx, dCur_Dxy, dCur_Dyy;
	double dInterim, dMag, dCur_Lamda1, dCur_Lamda2;
	double dCur_V1x, dCur_V1y, dCur_V2x, dCur_V2y;
	int iIter;

	//FILE *fp_cur_lambda = fopen ("mex_cur_lambda.txt", "w");

	for (iIter = 0 ; iIter < iNum_of_pixels ; iIter++)
	{
		dCur_Dxx = adDxx[iIter]; 
		dCur_Dxy = adDxy[iIter]; 
		dCur_Dyy = adDyy[iIter]; 

		dInterim = sqrt ((dCur_Dxx-dCur_Dyy)*(dCur_Dxx-dCur_Dyy) + (4*dCur_Dxy*dCur_Dxy)); // tmp = sqrt((Dxx - Dyy).^2 + 4*Dxy.^2);		
		dCur_V2x = 2*dCur_Dxy; // v2x = 2*Dxy; 
		dCur_V2y = dCur_Dyy - dCur_Dxx + dInterim; // v2y = Dyy - Dxx + tmp;

		// Normalize
		dMag = sqrt(dCur_V2x*dCur_V2x + dCur_V2y*dCur_V2y); // mag = sqrt(v2x.^2 + v2y.^2);
		if (dMag != 0) // i = (mag ~= 0);
		{
			dCur_V2x = dCur_V2x/dMag; // v2x(i) = v2x(i)./mag(i);
			dCur_V2y = dCur_V2y/dMag; // v2y(i) = v2y(i)./mag(i);
		}
		
		// The eigenvectors are orthogonal
		dCur_V1x = -dCur_V2y; // v1x = -v2y; 
		dCur_V1y = dCur_V2x; // v1y = v2x;

		// Compute Eigen Values.
		dCur_Lamda1 = (dCur_Dxx + dCur_Dyy - dInterim)/2.0; //  mu1 = 0.5*(Dxx + Dyy - tmp);
		dCur_Lamda2 = (dCur_Dxx + dCur_Dyy + dInterim)/2.0; //  mu2 = 0.5*(Dxx + Dyy + tmp);

		// Sort Eigen Values/Vectors and populate the output arrays.
		if (fabs(dCur_Lamda1) > fabs(dCur_Lamda2)) // check=abs(mu1)>abs(mu2);
		{
			// Swap Lambda1 and Lamda2 and populate arrays.
			adLambda1[iIter] = dCur_Lamda2;
			adLambda2[iIter] = dCur_Lamda1;

			// Swap V1 and V2 and populate arrays.
			adV1x[iIter] = dCur_V2x;
			adV1y[iIter] = dCur_V2y;
			adV2x[iIter] = dCur_V1x;
			adV2y[iIter] = dCur_V1y;
		}
		else
		{
			// In proper order, so no need to swap.
			adLambda1[iIter] = dCur_Lamda1;
			adLambda2[iIter] = dCur_Lamda2;

			// Swap V1 and V2 and populate arrays.
			adV1x[iIter] = dCur_V1x;
			adV1y[iIter] = dCur_V1y;
			adV2x[iIter] = dCur_V2x;
			adV2y[iIter] = dCur_V2y;
		}
	//	fprintf (fp_cur_lambda, "%g %g\n", adLambda1[iIter], adLambda2[iIter]);
	}
	//fclose (fp_cur_lambda);
}
