/* C++ IMPLEMENTATION OF PRINCIPAL CURVE TRACING USING MEX INTERFACE

Code developed by: Nikhila Srikanth
				   M.S. Project
				   Cognitive Systems Laboratory
				   Electrical and Computer Engineering Department
				   Northeastern University, Boston, MA

Under the guidance of: Prof. Deniz Erdogmus and Dr. Erhan Bas	

Date Created: Jan 20, 2011
Date Last Updated: April 29, 2011

This code contains all the functions called by main_mexcode.cpp,
PreProcessDataImage.cpp and by some other functions. This code is meant to run on
the Windows platform
*/

//#include <mex.h>
//#include <matrix.h>
#include <math.h>
//#include <varargs.h>
#include <stdarg.h>
#include <cfloat>

#include "FunctionsForMainCode.h"
#include "MatrixMultiplication.h"
#include "ML_Divide.h"

#define P 2


//------------------------------------------------------------------------------------------------------------------------------------
int Compute_3D_Min(int *piLookUpTable, int iWindowWidth, int iWindowLength, int iWindowHeight)
{
	int iLengthIter, iWidthIter, iHeightIter;
	int iMin, iCur_Entry;

	iMin = piLookUpTable[0];
	for (iLengthIter = 0; iLengthIter < iWindowLength; iLengthIter++)
	{
		for (iWidthIter = 0; iWidthIter < iWindowWidth; iWidthIter++)
		{
			for (iHeightIter = 0; iHeightIter < iWindowHeight; iHeightIter++)
			{
				iCur_Entry = piLookUpTable[iHeightIter + iWidthIter*iWindowHeight + iLengthIter*iWindowWidth*iWindowHeight];
				if(iCur_Entry < iMin)
				{
					iMin = iCur_Entry;
				}			
			}
		}
	}
	return iMin;
}

//-------------------------------------------------------------------------------------------------------------------------
int Compute_3D_Max(int *piLookUpTable, int iWindowWidth, int iWindowLength, int iWindowHeight)
{
	int iLengthIter, iWidthIter, iHeightIter;
	int iMax, iCur_Entry;

	iMax = piLookUpTable[0];
	for (iLengthIter = 0; iLengthIter < iWindowLength; iLengthIter++)
	{
		for (iWidthIter = 0; iWidthIter < iWindowWidth; iWidthIter++)
		{
			for (iHeightIter = 0; iHeightIter < iWindowHeight; iHeightIter++)
			{
				iCur_Entry = piLookUpTable[iHeightIter + iWidthIter*iWindowHeight + iLengthIter*iWindowWidth*iWindowHeight];
				if(iCur_Entry > iMax)
				{
					iMax = iCur_Entry;
				}			
			}
		}
	}
	return iMax;
}

//-------------------------------------------------------------------------------------------------------------------------
int Compute_3D_Len(int *piLookUpTable, int iWindowWidth, int iWindowLength, int iWindowHeight)
{
	int iLookupTable_Len = iWindowWidth*iWindowLength*iWindowHeight;
	return iLookupTable_Len ;
}

//-------------------------------------------------------------------------------------------------------------------------

void Get_ImagePixel_Neighbors (int *piLookUpTable, int iLenLookupTable, int iImagePixelIterator, double *pdNormP_EigVal,
							   int *piNeighbors_ImagePixel, int *piNum_of_Neighbors)
{
	int iLookupIter, iValid_Neig_Count, iCur_Neig;
    double dInfinity = DBL_MAX;
	int centerVoxel = int(double(iLenLookupTable+1)/2);

	for(iLookupIter = 0, iValid_Neig_Count = 0 ; iLookupIter < iLenLookupTable; iLookupIter++)
	{		
#if (CV)
		if (!piLookUpTable[iLookupIter])
			continue;
#endif
		
		iCur_Neig =  iImagePixelIterator + piLookUpTable[iLookupIter];
		if ((pdNormP_EigVal[iCur_Neig] > 0) && (pdNormP_EigVal[iCur_Neig] != dInfinity))
		{
			piNeighbors_ImagePixel[iValid_Neig_Count++] = iCur_Neig;
		}
	}
	*piNum_of_Neighbors = iValid_Neig_Count;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void Generate_Weights_for_KDE (double dImagePixelWeight,int *piNeighbors_ImagePixel, int iNum_of_Valid_Neighbors, 
							   double *pdWeights_InputImage, double *pdWeights_for_KDE)
{

	/*
	wjmx = (repmat(weights(:,jj),1,length(indNeig))-weights(:,indNeig)).^2/scaleInt^2; 
    W = weights(:,indNeig).*exp(-wjmx);
    W = W/sum(W); %normalize to make it 1
	*/
	int iCur_Neig, iIter;
	double dWeight_Cur_Neig, dCur_wjmx, dCur_W, dW_sum;
	
	dW_sum = 0;
	for (iIter = 0 ; iIter < iNum_of_Valid_Neighbors ; iIter++)
	{
		iCur_Neig = piNeighbors_ImagePixel[iIter];
		dWeight_Cur_Neig = pdWeights_InputImage[iCur_Neig];

		dCur_wjmx = dImagePixelWeight - dWeight_Cur_Neig;
		dCur_wjmx = (dCur_wjmx*dCur_wjmx)/(ScaleInt*ScaleInt);

		dCur_W = dWeight_Cur_Neig*exp(-dCur_wjmx); // W = weights(:,indNeig).*exp(-wjmx);
		dW_sum = dW_sum + dCur_W ;
		pdWeights_for_KDE[iIter] = dCur_W; // Not the final W.
	}
	
	for (iIter = 0 ; iIter < iNum_of_Valid_Neighbors ; iIter++)
	{
		pdWeights_for_KDE[iIter] = pdWeights_for_KDE[iIter]/dW_sum ; // W = W/sum(W); %normalize to make it 1
	}
}
//-------------------------------------------------------------------------------------------------------------------------
void Generate_Eig_Vecs_For_KDE(double *pdEigVec_Cov_InputImage, int *piNeighbors_ImagePixel, int iNum_of_Valid_Neighbors, 
							   int iNum_of_Dims_of_Input_Image, int iNum_of_pixels, double *pdEig_Vecs_for_KDE )
{
	// Vecs = Vectors(:,:,indNeig);  [ 1 x dData.dData.length(LookupTable) ]
	// Vecs = reshape(Vecs,dData,numel(Vecs)/dData);
	int iNeig_Iter, iIter, iEigVec_Length, iCur_Neig  ;

	iEigVec_Length = iNum_of_Dims_of_Input_Image*iNum_of_Dims_of_Input_Image; // Since EigVec for each pixel = dData x dData
			
	for (iNeig_Iter = 0 ; iNeig_Iter < iNum_of_Valid_Neighbors ; iNeig_Iter++)
	{
		iCur_Neig = piNeighbors_ImagePixel [iNeig_Iter];
		for (iIter = 0 ; iIter < iEigVec_Length ; iIter++)
		{
			pdEig_Vecs_for_KDE  [iNeig_Iter*iEigVec_Length + iIter] = pdEigVec_Cov_InputImage [iCur_Neig*iEigVec_Length + iIter]; // Vecs = Vectors(:,:,indNeig);  
		}
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

/*int Get_Number_of_Elements (mxArray *pmxaInputMatrix)
{
	// Get N and D of the input matrix.
	const mwSize *piDimsInputMatrix = mxGetDimensions(pmxaInputMatrix);
	int iNum_Of_Dims = mxGetNumberOfDimensions(pmxaInputMatrix);

	int iIter;
	int iNum_of_elems = 1;
	for (iIter = 0 ; iIter < iNum_Of_Dims  ; iIter++)
	{
		iNum_of_elems = iNum_of_elems * piDimsInputMatrix[iIter];
	}

	return iNum_of_elems;
}
*/
//--------------------------------------------------------------------------------------------------------------------------------------------
void Compute_H_perp (double *pdEigVecs_of_LogHessians, double *pdEigVals_of_LogHessians_Extracted, int iNum_of_Dims_of_Input_Image, 
					 double *pdH_Perp)
{
	// Goal is to generate: Hperp = Q(:,2:end)*L(2:end,2:end)*Q(:,2:end)';%nEigVal + 1
	double *pdEigVecs_of_LogHessians_Extracted = &pdEigVecs_of_LogHessians[iNum_of_Dims_of_Input_Image*INTRINSIC_DIM]; // Q(:,2:end) [dData x  dData-1]
    // iNum_of_Dims_of_Input_Image*INTRINSIC_DIM is the amount we skip to access to the orthogonal components 
    // note that Q[:,1-3] => L(1)<L(2)<L(3)

	double *pdTranspose_of_EigVecs_of_LogHessians_Extracted = (double *) malloc ((iNum_of_Dims_of_Input_Image-INTRINSIC_DIM) * (iNum_of_Dims_of_Input_Image) * sizeof(double)); // Q(:,2:end)' [dData-1 x  dData]				
	double *pdH_perp_Interim_Output = (double *) malloc ((iNum_of_Dims_of_Input_Image-INTRINSIC_DIM) * iNum_of_Dims_of_Input_Image * sizeof (double));

	// Generate Q(:,2:end)'
	Double_Compute_Transpose (pdEigVecs_of_LogHessians_Extracted, pdTranspose_of_EigVecs_of_LogHessians_Extracted, 
            iNum_of_Dims_of_Input_Image, iNum_of_Dims_of_Input_Image-INTRINSIC_DIM);
		
	//Generate: L(2:end,2:end)*Q(:,2:end)' [dData-1  x  dData]
	Double_Mat_Multiply(pdEigVals_of_LogHessians_Extracted, pdTranspose_of_EigVecs_of_LogHessians_Extracted,
						iNum_of_Dims_of_Input_Image-INTRINSIC_DIM, iNum_of_Dims_of_Input_Image-INTRINSIC_DIM, iNum_of_Dims_of_Input_Image-INTRINSIC_DIM, iNum_of_Dims_of_Input_Image, 
						pdH_perp_Interim_Output);

	// Generate: Q(:,2:end) * (Interim Output) [dData x dData]
	Double_Mat_Multiply (pdEigVecs_of_LogHessians_Extracted , pdH_perp_Interim_Output, 
		iNum_of_Dims_of_Input_Image, iNum_of_Dims_of_Input_Image - INTRINSIC_DIM, iNum_of_Dims_of_Input_Image-INTRINSIC_DIM, iNum_of_Dims_of_Input_Image, pdH_Perp);

	free (pdTranspose_of_EigVecs_of_LogHessians_Extracted); 
	free (pdH_perp_Interim_Output);
}
//---------------------------------------------------------------------------------------------------------------------------------------

void Double_Compute_Transpose(double *pdInput, double *pdOutput, int iRows_Input, int iCols_Input)
{
	int iRows_Output = iCols_Input;
	int iCols_Output = iRows_Input;
		
	int iIter_Col, iIter_Row;

	for (iIter_Col = 0; iIter_Col < iCols_Input; iIter_Col++)
	{
		for (iIter_Row = 0; iIter_Row < iRows_Input; iIter_Row++)
		{
			pdOutput[ROWCOL(iIter_Col, iIter_Row, iRows_Output)]  = pdInput[ROWCOL(iIter_Row, iIter_Col, iRows_Input)];
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------------------------
void SortTangentialSpace(double *pdCurrT, double *pdEigVecs_of_LogHessian, double *pdEigVals_of_LogHessian, int iNum_of_Dims_of_Input_Image )
{
	double *pdTranspose_of_Tangential = (double *) malloc (INTRINSIC_DIM * iNum_of_Dims_of_Input_Image * sizeof(double)); // g' [1 x dData];
	double *pdTransTang_LogHess_Interim_Output = (double *) malloc (INTRINSIC_DIM * iNum_of_Dims_of_Input_Image * sizeof (double));
	double *eigVec = (double *) malloc (iNum_of_Dims_of_Input_Image * iNum_of_Dims_of_Input_Image * sizeof (double));
	double *eigVal = (double *) malloc (iNum_of_Dims_of_Input_Image * sizeof (double));
	int *processedBasis = (int *)malloc (INTRINSIC_DIM* sizeof (int));
	
    int iTangIter, iRowIter, iColIter, iRit, iCit, maxInd, Iter;
	double currEnt, maxVal;

    // initialize basis
	for(Iter = 0; Iter<iNum_of_Dims_of_Input_Image; Iter++)
	{
		processedBasis[Iter] = 0;
	}	
	
	// sort the eigen values and the eigenvectors based on the tangential space of the origin
	// Tangential transpose
	Double_Compute_Transpose (pdCurrT, pdTranspose_of_Tangential, iNum_of_Dims_of_Input_Image, INTRINSIC_DIM);
	// innerproduct between the reference and the current space
	Double_Mat_Multiply (pdTranspose_of_Tangential, pdEigVecs_of_LogHessian, INTRINSIC_DIM, iNum_of_Dims_of_Input_Image, iNum_of_Dims_of_Input_Image, iNum_of_Dims_of_Input_Image, pdTransTang_LogHess_Interim_Output);
	
	for(iTangIter = 0; iTangIter<INTRINSIC_DIM; iTangIter++)
	{
		// get the matched vector for the iDim th tangential basis
		maxInd = 0;
		maxVal = 0;
		for(iColIter = 0; iColIter < iNum_of_Dims_of_Input_Image; iColIter++)
		{
			currEnt = fabs(pdTransTang_LogHess_Interim_Output[ROWCOL(iTangIter, iColIter, INTRINSIC_DIM)]);
			if (currEnt>maxVal)
			{
				maxVal = currEnt;
				maxInd = iColIter;
			}
		}
		processedBasis[maxInd] = 1;
		
		for(iRowIter = 0; iRowIter < iNum_of_Dims_of_Input_Image; iRowIter++)
		{
			eigVec[ROWCOL(iRowIter,iTangIter,iNum_of_Dims_of_Input_Image)]=pdEigVecs_of_LogHessian[ROWCOL(iRowIter,maxInd,iNum_of_Dims_of_Input_Image)];
		}
// 		eigVal[ROWCOL(iTangIter,iTangIter,iNum_of_Dims_of_Input_Image)] = pdEigVals_of_LogHessian[ROWCOL(maxInd,maxInd,iNum_of_Dims_of_Input_Image)];
		eigVal[iTangIter] = pdEigVals_of_LogHessian[maxInd];
	}
	for(Iter = 0,iTangIter = INTRINSIC_DIM; Iter<iNum_of_Dims_of_Input_Image; Iter++)
	{
		if(!processedBasis[Iter])
		{
			for(iRowIter = 0; iRowIter < iNum_of_Dims_of_Input_Image; iRowIter++)
			{
				eigVec[ROWCOL(iRowIter,iTangIter,iNum_of_Dims_of_Input_Image)]=pdEigVecs_of_LogHessian[ROWCOL(iRowIter,Iter,iNum_of_Dims_of_Input_Image)];
			}
// 			eigVal[ROWCOL(iTangIter,iTangIter,iNum_of_Dims_of_Input_Image)] = pdEigVals_of_LogHessian[ROWCOL(Iter,Iter,iNum_of_Dims_of_Input_Image)];
			eigVal[iTangIter] = pdEigVals_of_LogHessian[Iter];
			iTangIter++;
		}
	}	
	// copy the elements
	for(iRowIter = 0; iRowIter < iNum_of_Dims_of_Input_Image; iRowIter++){
		for(iColIter = 0; iColIter < iNum_of_Dims_of_Input_Image; iColIter++){
			pdEigVecs_of_LogHessian[ROWCOL(iRowIter,iColIter,iNum_of_Dims_of_Input_Image)] = eigVec[ROWCOL(iRowIter,iColIter,iNum_of_Dims_of_Input_Image)];
		}
			pdEigVals_of_LogHessian[iRowIter] = eigVal[iRowIter];
	}
	free(pdTranspose_of_Tangential); free(pdTransTang_LogHess_Interim_Output); free(eigVec); free(eigVal);
}
//---------------------------------------------------------------------------------------------------------------------------------------

double Compute_Norm(double *pdInput, int iNum_of_elems_of_input) 
{
	int iIter;
	double dSum_of_Squares = 0;
	double dNorm_of_the_Matrix;

	for (iIter = 0; iIter < iNum_of_elems_of_input ; iIter++)
	{
		//dSum_of_Squares += (double)pow((double)pdInput(iIter),(double)P); 
		dSum_of_Squares += pdInput[iIter]*pdInput[iIter]; 
	}
	//dNorm_of_the_Matrix = (double)pow((double)dSum_of_Squares,(double)(1/P));
	dNorm_of_the_Matrix = sqrt(dSum_of_Squares);
	return dNorm_of_the_Matrix;
}

//--------------------------------------------------------------------------------------------------------------------------------------------
//nm = -(g'*Hperp*g);
double Compute_Numerator(double *pdH_Perp, double *pdGrads, int iNum_of_Dims_of_Input_Image)
{
	double *pdTranspose_of_Grads = (double *) malloc (iNum_of_Dims_of_Input_Image * sizeof(double)); // g' [1 x dData]
	double *pdInterim_Output = (double *) malloc (iNum_of_Dims_of_Input_Image * sizeof (double)); // For storing Hperp*g [dData x 1]
	double dNumerator;

	// Generate g'
	Double_Compute_Transpose (pdGrads, pdTranspose_of_Grads, iNum_of_Dims_of_Input_Image, 1);
	
	// Step 1: Compute Hperp * g [dData x 1]
	Double_Mat_Multiply (pdH_Perp, pdGrads, iNum_of_Dims_of_Input_Image, iNum_of_Dims_of_Input_Image, iNum_of_Dims_of_Input_Image, 1, pdInterim_Output);
		
	// Step 2: Compute g' * interim_output [1 x 1]
	Double_Mat_Multiply (pdTranspose_of_Grads, pdInterim_Output, 1, iNum_of_Dims_of_Input_Image, iNum_of_Dims_of_Input_Image, 1, &dNumerator);
	free (pdTranspose_of_Grads); free (pdInterim_Output);

	dNumerator = -dNumerator;
	return dNumerator;
}

//-------------------------------------------------------------------------------------------------------------------------------
//dm = norm(g)*norm(HH\g);%inverse HH*g           
double Compute_Denominator(double *pdGrads, double *pdLogHessians, int iNum_of_Dims_of_Input_Image)
{
	//Step 1: Compute norm(g)
	double dNorm_of_Grads = Compute_Norm(pdGrads, iNum_of_Dims_of_Input_Image) ;
	double *pdHH_Inverse_by_G = (double *) malloc (iNum_of_Dims_of_Input_Image * sizeof (double)); // For HH/g  [dData x 1]

#if 1	
	// Step 2: Compute (HH/g) - equivalent to (HH inverse)*g

    Compute_ML_Divide(pdLogHessians, pdGrads, iNum_of_Dims_of_Input_Image, iNum_of_Dims_of_Input_Image, iNum_of_Dims_of_Input_Image, 1,
					  pdHH_Inverse_by_G);
	
	//Step 3: Compute norm(Inverse HH*g)
	double dNorm_of_HH_Inverse_by_G = Compute_Norm(pdHH_Inverse_by_G, iNum_of_Dims_of_Input_Image);
#endif
	
	//Step 4: Compute denominator: norm(g)*norm(HH/g)
	double dDenominator = dNorm_of_Grads*dNorm_of_HH_Inverse_by_G;

	free (pdHH_Inverse_by_G );
	return dDenominator;
}



//--------------------------------------------------------------------------------------------------------------------------------------
void Compute_Updated_CurrP(double *pdCurrP, double *pdEigVecs_of_LogHessians, double *pdNormc_of_Grads, int iNum_of_Dims_of_Input_Image)
{
	//currP = currP + muT*Q(:,2:end)*Q(:,2:end)'*normc(g);

	double *pdEigVecs_of_LogHessians_Extracted = &pdEigVecs_of_LogHessians[iNum_of_Dims_of_Input_Image*INTRINSIC_DIM]; // Q(:,2:end) //[dData x dData - 1]
	double *pdTranspose_of_EigVecs_of_LogHessians_Extracted = (double *) malloc ((iNum_of_Dims_of_Input_Image-INTRINSIC_DIM) * (iNum_of_Dims_of_Input_Image) * sizeof(double)); // Q(:,2:end)' [dData-1 x  dData]

	// Generate Q(:,2:end)'
	Double_Compute_Transpose (pdEigVecs_of_LogHessians_Extracted, pdTranspose_of_EigVecs_of_LogHessians_Extracted, iNum_of_Dims_of_Input_Image, iNum_of_Dims_of_Input_Image-INTRINSIC_DIM);
	
	double *pdInterim1_Output = (double *) malloc((iNum_of_Dims_of_Input_Image-INTRINSIC_DIM) * sizeof(double)); //[dData-1 x 1]
	// Compute Q(:,2:end)'*normc(g) [dData-1 x dData] * [dData x 1] = [dData-1 x 1]
	Double_Mat_Multiply (pdTranspose_of_EigVecs_of_LogHessians_Extracted, pdNormc_of_Grads,
		iNum_of_Dims_of_Input_Image-INTRINSIC_DIM, iNum_of_Dims_of_Input_Image, iNum_of_Dims_of_Input_Image, 1, pdInterim1_Output);
	
	double *pdInterim2_Output = (double *) malloc (iNum_of_Dims_of_Input_Image * sizeof(double)); // [dData x 1]
	// Compute Q(:,2:end) * Interim1_Output [dData * dData-1] * [dData-1 x 1] = [dData x 1]
	Double_Mat_Multiply (pdEigVecs_of_LogHessians_Extracted, pdInterim1_Output,
		iNum_of_Dims_of_Input_Image, iNum_of_Dims_of_Input_Image-INTRINSIC_DIM, iNum_of_Dims_of_Input_Image-INTRINSIC_DIM, 1, pdInterim2_Output);
	
	int iIter;
	for (iIter = 0; iIter < iNum_of_Dims_of_Input_Image; iIter++)
	{	
		pdCurrP[iIter] = pdCurrP[iIter] + (pdInterim2_Output[iIter] * muT); 
		//pdCurrP[iIter] = 0 + (pdInterim2_Output[iIter] * muT); 
	}
	free(pdInterim1_Output); free(pdInterim2_Output);
}

//----------------------------------------------------------------------------------------------------------------------------------------------

void Compute_Normc(double *pdInput, int iNum_of_Rows_of_Inp, int iNum_of_Cols_of_Inp, double *pdNormc_of_Input)
{
	double dNorm_of_Cur_Col;
	int iRowIter, iColIter;

	for (iColIter = 0 ; iColIter < iNum_of_Cols_of_Inp; iColIter++)
	{
		dNorm_of_Cur_Col = Compute_Norm (&pdInput[iColIter*iNum_of_Rows_of_Inp], iNum_of_Rows_of_Inp); // Computes norm of current column.

		for(iRowIter = 0; iRowIter < iNum_of_Rows_of_Inp; iRowIter++)
		{
			pdNormc_of_Input[ROWCOL(iRowIter, iColIter, iNum_of_Rows_of_Inp)] = pdInput[ROWCOL(iRowIter, iColIter, iNum_of_Rows_of_Inp)]/dNorm_of_Cur_Col;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------------------------------

/*double Compute_Gamma (double dInput)
{
	mxArray *pmxaInput_to_gamma = mxCreateDoubleMatrix (1,1,mxREAL);
	double *pdInput_to_gamma = mxGetPr(pmxaInput_to_gamma);
	pdInput_to_gamma [0] = dInput;

	mxArray *pmxaOutput_of_gamma;
	double *pdOutput_of_gamma;

	mexCallMATLAB(1, &pmxaOutput_of_gamma, 1, &pmxaInput_to_gamma, "gamma");
	pdOutput_of_gamma = mxGetPr(pmxaOutput_of_gamma);

	return (pdOutput_of_gamma[0]);
}*/

//----------------------------------------------------------------------------------------------------------------------------------------------

void Get_CurrP (int *piData, int iNum_of_Dims_of_Input_Image, int iUpdatedImagePixelIterator, double *pdCurrP)
{
	int iIter;
	for(iIter = 0; iIter < iNum_of_Dims_of_Input_Image; iIter++)
	{
		pdCurrP[iIter] = piData[iUpdatedImagePixelIterator*iNum_of_Dims_of_Input_Image + iIter];
	}
}
