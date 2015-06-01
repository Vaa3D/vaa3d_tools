/* C++ IMPLEMENTATION OF PRINCIPAL CURVE TRACING USING MEX INTERFACE

Code developed by: Nikhila Srikanth
				   M.S. Project
				   Cognitive Systems Laboratory
				   Electrical and Computer Engineering Department
				   Northeastern University, Boston, MA

Under the guidance of: Prof. Deniz Erdogmus and Dr. Erhan Bas	

Date Created: Jan 20, 2011
Date Last Updated: April 29, 2011

This code aims at converting the MATLAB implementation of create3DLookUpTable 
of Principal Curve Tracing algorithm into C++. This code is meant to run on
the Windows platform
*/

#include <math.h>
#include <v3d_interface.h>


/*
	function: create3DLookupTable, return two arrays:
	LookupTable [some array]
	DistTable [some array]

	calling function code:
	mxArray *pmxLookupTable;
	mxArray *pmxDistTable;

	create3DLookupTable (input args, &pmxLookupTable, &pmxDistTable);


*/

void create3DLookupTable(int *aiWindowDims, int iNumWinDims, int iVolHeight, int iVolWidth, int **ppiLookUpTable, double **ppdDistTable)
{
	int iImageWidth, iImageHeight;
	int iWindowWidth, iWindowLength, iWindowHeight;
	int iD2WindowWidth, iD2WindowLength, iD2WindowHeight;
	int i, j, k;
	
	iImageWidth = iVolWidth;
	iImageHeight = iVolHeight;

	if(iNumWinDims == 1)
	{
		iWindowHeight = aiWindowDims[0];
		iWindowWidth = aiWindowDims[0];
		iWindowLength = aiWindowDims[0];
	}
	else
	{
		iWindowHeight = aiWindowDims[0];
		iWindowWidth = aiWindowDims[1];
		iWindowLength = aiWindowDims[2];
	}

	if ((iWindowWidth % 2 == 0) || (iWindowLength % 2 == 0) || (iWindowHeight % 2 == 0))
	{
        printf("\n Window sizes must be odd\n");
		*ppiLookUpTable = NULL;
		*ppdDistTable = NULL;
        return;
	}

    *ppiLookUpTable = new int[iWindowLength * iWindowWidth * iWindowHeight];
    *ppdDistTable = new double[iWindowLength * iWindowWidth * iWindowHeight];

	iD2WindowHeight = (iWindowHeight )/2;
	iD2WindowWidth = (iWindowWidth )/2;
	iD2WindowLength = (iWindowLength )/2;
    printf ("\nCreating Lookup Table: [%d %d %d]", iWindowHeight, iWindowLength, iWindowWidth);

	for (k = 0; k < iWindowLength; k++)
	{
		for (j = 0; j < iWindowWidth; j++)
		{
			for (i = 0; i < iWindowHeight; i++)
			{
				(*ppiLookUpTable)[i + j*iWindowHeight + k*iWindowWidth*iWindowHeight] = (k-iD2WindowLength)*iImageHeight*iImageWidth + 
																					   (j-iD2WindowWidth)*iImageHeight+(i-iD2WindowHeight);

				(*ppdDistTable)[i + j*iWindowHeight + k*iWindowWidth*iWindowHeight] = sqrt((double)((i-iD2WindowHeight)*(i-iD2WindowHeight) +
																					  (j-iD2WindowWidth)*(j-iD2WindowWidth) +
																					  (k-iD2WindowLength)*(k-iD2WindowLength)));
			}
		}
	}
#if 0
	for (i = 0; i < iWindowHeight; i++)
	{
        printf ("\n\nLookup (%d,:,:)\n", i+1);
		for (j = 0; j < iWindowWidth; j++)
		{
            printf ("\n\t");
			for (k = 0; k < iWindowLength; k++)
			{
                printf ("%d ", (*ppiLookUpTable)[i + j*iWindowHeight + k*iWindowWidth*iWindowHeight]);
			}
		}
	}
#endif
}
