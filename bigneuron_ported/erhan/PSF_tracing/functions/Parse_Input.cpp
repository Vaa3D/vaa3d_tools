/* C++ IMPLEMENTATION OF PRINCIPAL CURVE TRACING USING MEX INTERFACE

Code developed by: Nikhila Srikanth
				   M.S. Project
				   Cognitive Systems Laboratory
				   Electrical and Computer Engineering Department
				   Northeastern University, Boston, MA

Under the guidance of: Prof. Deniz Erdogmus and Dr. Erhan Bas	

Date Created: Jan 20, 2011
Date Last Updated: April 29, 2011

This code aims at converting the MATLAB implementation of parse_input function
of Principal Curve Tracing algorithm into C++. This code is meant to run on
the Windows platform
*/

#include <math.h>

#include "Parse_Input.h"
#include "FunctionsForMainCode.h"

double max_2 (double dInp1, double dInp2)
{
	double dMax;
	if (dInp1 >= dInp2)
	{
		dMax = dInp1;
	}
	else
	{
		dMax = dInp2;
	}
	return dMax;
}

void parse_input (bool *bAnisotropic, double *K, double *Beta, double *ppmxaWeights_ParseInput, int iNum_of_Dims_of_InputImage,
				  int iNum_of_pixels_InputImage, int iWidth)
{
	/*
	p = inputParser;
	p.addParamValue('K', max(max(ceil((sqrt(N))),dim+1),width^dim), @(x)isnumeric(x) && x>0 && x<=N);
	p.addParamValue('beta', 1, @(x)isnumeric(x) && x>=1 && x<=N);
	p.addParamValue('ani', 0, @(x)isnumeric(x) && x<2&& x>0);
	p.addParamValue('weights', ones(1,N)/N);
	p.parse(inputs{:});
	anisotropic = p.Results.ani;
	k = p.Results.K;
	weights = p.Results.weights;
	beta = p.Results.beta;
*/

	*K = max_2(max_2(ceil(sqrt((double)iNum_of_pixels_InputImage)),iNum_of_Dims_of_InputImage+1),pow((double)iWidth,iNum_of_Dims_of_InputImage));
	*Beta = 1.0;
	*bAnisotropic = DEFAULT_ANISOTROPIC;
#if 0
	*ppmxaWeights_ParseInput = mxCreateDoubleMatrix(1, iNum_of_pixels_InputImage, mxREAL);
	double *pdWeights_ParseInput = mxGetPr (*ppmxaWeights_ParseInput);
	double dWeight_Val = 1.0/iNum_of_pixels_InputImage;

	int iIter;
	for (iIter = 0 ; iIter < iNum_of_pixels_InputImage ; iIter++)
	{
		pdWeights_ParseInput [iIter] = dWeight_Val;
	}
#endif
}




