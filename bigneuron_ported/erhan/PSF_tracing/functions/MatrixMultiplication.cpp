/* C++ IMPLEMENTATION OF PRINCIPAL CURVE TRACING USING MEX INTERFACE

Code developed by: Nikhila Srikanth
				   M.S. Project
				   Cognitive Systems Laboratory
				   Electrical and Computer Engineering Department
				   Northeastern University, Boston, MA

Under the guidance of: Prof. Deniz Erdogmus and Dr. Erhan Bas	

Date Created: Jan 20, 2011
Date Last Updated: April 29, 2011

This code performs matrix multiplication in C. This code is meant to run on
the Windows platform
*/

#include <math.h>

#include "MatrixMultiplication.h"

#define ROWCOL(row,col,tot_rows) (row + col*tot_rows)
//pdInput1[ROWCOL(2,1,20)]
#define TRUE 1
#define FALSE 0
double *Matrix_Multiply(double *pdInput1, double *pdInput2, int iRows_Input1, int iCols_Input1, int iRows_Input2, int iCols_Input2,
						 bool bUse_Transpose_of_Inp_2)
{
	int iSwapVar;
	double dProduct;
	if (bUse_Transpose_of_Inp_2 == TRUE)
	{
		iSwapVar = iRows_Input2;
		iRows_Input2 = iCols_Input2;
		iCols_Input2 = iSwapVar;
	}

	//A --> 10x20
	//B --> 40x20
	// When taking transpose, B becomes 20x40
	//AB` --> 10x40

	if(iCols_Input1 != iRows_Input2)
	{
        printf("\n Error:Matrix dimensions mismatch for multiplication");
		exit (1);
	}

    double *pdOutput = new double[iRows_Input1*iCols_Input2];

	int iIter_i, iIter_j, iIter_k;
	for(iIter_i = 0; iIter_i < iRows_Input1; iIter_i++)  // 0
	{
		for(iIter_j = 0; iIter_j < iCols_Input2; iIter_j++)  // 39
		{
			dProduct = 0;
			for(iIter_k = 0; iIter_k < iRows_Input2; iIter_k++) // 0 to start with..
			{
				//dProduct += pdInput1[iIter_i + iIter_k*iRows_Input1] * pdInput2[iIter_k + iIter_j*iRows_Input2];
				if (bUse_Transpose_of_Inp_2 == TRUE)
				{
					dProduct += pdInput1[ROWCOL(iIter_i, iIter_k, iRows_Input1)] * pdInput2[ROWCOL(iIter_j, iIter_k, iCols_Input2)];
				}
				else
				{
					dProduct += pdInput1[ROWCOL(iIter_i, iIter_k, iRows_Input1)] * pdInput2[ROWCOL(iIter_k, iIter_j, iRows_Input2)];
				}
			}
			pdOutput[ROWCOL(iIter_i, iIter_j, iRows_Input1)] = dProduct;
		}
	}
	//   A:[10x20]   B:[20x40]  -- P = AB
	// to generate: P(1,1) --> a(1,1)*b(1,1) + a(1,2)*b(2,1) ...... a(1,20)*b(20,1)
	//				P(1,40) --> a(1,1)*b(1,40) + a(1,2)*b(2,40) ....... a(1,20)*b(20,40)

	//   A:[10x20]   B:[40x20]  -- P = AB`
	//   iRows_Input2 = 20, iCols_Input2 = 40
	// to generate: P(1,1) --> a(1,1)*b(1,1) + a(1,2)*b(1,2) ...... a(1,20)*b(1,20)
	//				P(1,40) --> a(1,1)*b(40,1) + a(1,2)*b(40,2) ....... a(1,20)*b(40,20)

    return pdOutput;
}


void Int_Mat_Multiply(int *piInput1, int *piInput2, int iRows_Input1, int iCols_Input1, int iRows_Input2, int iCols_Input2, int **ppiOutput)
{
	int iProduct;

	if(iCols_Input1 != iRows_Input2)
	{
        printf("\n Error:Matrix dimensions mismatch for multiplication");
		exit (1);
	}

	*ppiOutput = (int*) malloc (iRows_Input1*iCols_Input2*sizeof(int));

	int iIter_i, iIter_j, iIter_k;
	for(iIter_i = 0; iIter_i < iRows_Input1; iIter_i++)  // 0
	{
		for(iIter_j = 0; iIter_j < iCols_Input2; iIter_j++)  // 39
		{
			iProduct = 0;
			for(iIter_k = 0; iIter_k < iRows_Input2; iIter_k++) // 0 to start with..
			{
				iProduct += piInput1[ROWCOL(iIter_i, iIter_k, iRows_Input1)] * piInput2[ROWCOL(iIter_k, iIter_j, iRows_Input2)];
			}
			*ppiOutput[ROWCOL(iIter_i, iIter_j, iRows_Input1)] = iProduct;
		}
	}
	//   A:[10x20]   B:[20x40]  -- P = AB
	// to generate: P(1,1) --> a(1,1)*b(1,1) + a(1,2)*b(2,1) ...... a(1,20)*b(20,1)
	//				P(1,40) --> a(1,1)*b(1,40) + a(1,2)*b(2,40) ....... a(1,20)*b(20,40)
}


void Double_Mat_Multiply(double *pdInput1, double *pdInput2, int iRows_Input1, int iCols_Input1, int iRows_Input2, int iCols_Input2, double *pdOutput)
{
	double dProduct;

	if(iCols_Input1 != iRows_Input2)
	{
        printf("\n Error:Matrix dimensions mismatch for multiplication");
		exit (1);
	}

	int iIter_i, iIter_j, iIter_k;
	for(iIter_i = 0; iIter_i < iRows_Input1; iIter_i++)  // 0
	{
		for(iIter_j = 0; iIter_j < iCols_Input2; iIter_j++)  // 39
		{
			dProduct = 0;
			for(iIter_k = 0; iIter_k < iRows_Input2; iIter_k++) // 0 to start with..
			{
				dProduct += pdInput1[ROWCOL(iIter_i, iIter_k, iRows_Input1)] * pdInput2[ROWCOL(iIter_k, iIter_j, iRows_Input2)];
			}
			pdOutput[ROWCOL(iIter_i, iIter_j, iRows_Input1)] = dProduct;
		}
	}
	//   A:[10x20]   B:[20x40]  -- P = AB
	// to generate: P(1,1) --> a(1,1)*b(1,1) + a(1,2)*b(2,1) ...... a(1,20)*b(20,1)
	//				P(1,40) --> a(1,1)*b(1,40) + a(1,2)*b(2,40) ....... a(1,20)*b(20,40)
}
