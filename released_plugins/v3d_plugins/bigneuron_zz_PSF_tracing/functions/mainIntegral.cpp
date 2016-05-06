#include <mex.h>
#include <matrix.h>
#include <math.h>
#include <pthread.h>
#include <string.h>

#include "PreProcessDataImage.h"
#include "Create3DLookUpTable.h"
#include "Parse_Input.h"
#include "FunctionsForMainCode.h"
#include "EigenDec_2D.h"
#include "EigenDec_3D.h"
#include "KernelDensityEstimation.h"
#include "MatrixMultiplication.h"
#include "ML_Divide.h"

#define LOOKUP_TABLE_WIDTH DEFAULT_LOOKUP_WIDTH

#define ROUND(x) ((int)(x+0.5))

void *Thread_ProcessImage (void *); // for Pthreads

const mwSize *gpiDims_InputImage;
int giNum_of_pixels, giNum_of_Dims_of_Input_Image, giNum_of_locations;
double *gpdInputImage;
double *gpdInputLocations;
double *gpdInputTangDir;

int *gpiData;
double *gpdWeights_InputImage;
double *gpdEigVec_Cov_InputImage;
double *gpdEigVal_Cov_InputImage;
double *gpdNormP_EigVal;

int *gpiLookUpTable;
double *gpdDistTable;
int gaiWindowDims[3];
int giMinLookupTable, giMaxLookupTable, giLenLookupTable;

bool gbUpdateAllKDEInputs;

mxArray *gpmxaInputImage;
// mxArray *gpmxaProjected_Points;
// mxArray *gpmxaTangential_Space;
mxArray *gpmxaInputLocations;
mxArray *gpmxaInputTangDir;
// mxArray *gpmxaEigenVals;
mxArray *gpmxaScores;

double *gpdProjected_Points;
double *gpdTangential_Space;
double *gpdEigenVals;
double *gpdScores;

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    mxArray *pmxaInputImage_from_imread;
    mxArray *pmxImage_filename;
    char *strFilename;
    char *strOutname;
    char *strUpdateAllKDEInputs;
    int iVolWidth, iVolHeight;
    int iNumber_Of_Elements_in_ProjectedPoints;
    int iIndex;
    double sigma;
    int prepLookUpTableWidth;
    
    // pthread declarations
    
    pthread_t ptThreads[NUM_OF_THREADS_TO_CREATE];
    int iThreadNumber;
    int iRet_Val_Pthread_Create;
    int iRet_Val_Pthread_Join;
    bool bAll_Threads_Completed_Successfully = TRUE;
    
    gbUpdateAllKDEInputs = FALSE; // default value.
    
    if (mxIsClass(prhs[0], "char"))
    {
        // Extract image filename.
        pmxImage_filename = (mxArray *) prhs[0];
        strFilename = mxArrayToString (prhs[0]);
        mexPrintf ("\nImage File: %s\n", strFilename);
        mexEvalString("pause(0.001);"); // To dump the printf string to the console.
        
        // Get image data from Matlab function: GetImageDataPoints. This will be referred to as Image Data Matrix
        // in the entire project
        mexCallMATLAB(1, &pmxaInputImage_from_imread, 1, &pmxImage_filename, "imread");
        mexCallMATLAB(1, &gpmxaInputImage, 1, &pmxaInputImage_from_imread, "im2double");
        mxDestroyArray (pmxaInputImage_from_imread);
        mexPrintf ("\nSuccessfully read image file: %s\n", strFilename);
        mexEvalString("pause(0.001);"); // To dump the printf string to the console.
    }
    else if (mxIsClass(prhs[0], "double"))
    {
        gpmxaInputImage = (mxArray*) prhs[0];
        mexPrintf ("\nGot Image Matrix.\n");
        mexEvalString("pause(0.001);"); // To dump the printf string to the console.
        
        gpmxaInputLocations = (mxArray*) prhs[1];
        mexPrintf ("\nGot Location Matrix.\n");
        mexEvalString("pause(0.001);"); // To dump the printf string to the console.
        
        gpmxaInputTangDir = (mxArray*) prhs[2];
        mexPrintf ("\nGot Location Matrix.\n");
        mexEvalString("pause(0.001);"); // To dump the printf string to the console.
        
        strOutname = mxArrayToString (prhs[3]);
        mexPrintf ("\nImage File: %s\n", strOutname);
        mexEvalString("pause(0.001);"); // To dump the printf string to the console.
        
        sigma = mxGetScalar(prhs[4]);
        mexPrintf ("\nSIGMA: %1.2f\n", sigma);
        mexEvalString("pause(0.001);"); // To dump the printf string to the console.
    }
    else
    {
        mexPrintf ("\n Input 1 should be either image filename or image matrix of type double\n");
        mexEvalString("pause(0.001);"); // To dump the printf string to the console.
        return;
    }
    
    //-------------------------------------------------------------------------------------------------------
    gpdInputImage = mxGetPr (gpmxaInputImage);
    gpdInputLocations = mxGetPr (gpmxaInputLocations);
    gpdInputTangDir = mxGetPr (gpmxaInputTangDir);
    
	//     mexPrintf ("\nInput Min: %4.2f %4.2f %4.2f\n", gpdInputLocations[0], gpdInputLocations[1], gpdInputLocations[2]);
	//     mexEvalString("pause(0.001);"); // To dump the printf string to the console.
	//     mexPrintf ("\nTang Min: %4.2f %4.2f %4.2f\n", gpdInputTangDir[0], gpdInputTangDir[1], gpdInputTangDir[2]);
	//     mexEvalString("pause(0.001);"); // To dump the printf string to the console.
    
    giNum_of_Dims_of_Input_Image = mxGetNumberOfDimensions(gpmxaInputImage);
    gpiDims_InputImage = mxGetDimensions(gpmxaInputImage);
    giNum_of_pixels = Get_Number_of_Elements(gpmxaInputImage);
    giNum_of_locations = Get_Number_of_Elements(gpmxaInputLocations)/giNum_of_Dims_of_Input_Image;
    
    // Allocate memory for all outputs generated in Pre-processing Step.
    gpdWeights_InputImage = (double*) malloc (1*giNum_of_pixels*sizeof(double)); //weights [row vector, 1 x N]
    gpdEigVec_Cov_InputImage = (double*) malloc (giNum_of_Dims_of_Input_Image*giNum_of_Dims_of_Input_Image*giNum_of_pixels*sizeof(double));  // Vectors [dData x dData * N]
    gpdEigVal_Cov_InputImage = (double*) malloc (giNum_of_Dims_of_Input_Image*giNum_of_pixels*sizeof(double));  // Lambda [dData x N]
    gpdNormP_EigVal = (double*) malloc(1*giNum_of_pixels*sizeof(double));  //NormP [1 x N]
    gpiData = (int*) malloc (giNum_of_Dims_of_Input_Image*giNum_of_pixels*sizeof(int));// data [dData x N]
    
	//	sigma = 4;
#if 1
	prepLookUpTableWidth = 11;
    PreProcessDataImage(gpdInputImage, giNum_of_Dims_of_Input_Image, gpiDims_InputImage, giNum_of_pixels,
						gpdWeights_InputImage, gpdEigVec_Cov_InputImage, gpdEigVal_Cov_InputImage, gpdNormP_EigVal,
						gpiData, strOutname, sigma, prepLookUpTableWidth);
#else
#endif
	
	//-------------------------------------------------------------------------------------------------------
	mexPrintf ("\nBack to maincode.\n");
    mexEvalString("pause(0.001);"); // To dump the printf string to the console.
	//     plhs[0] = gpmxaProjected_Points;
	//     nlhs = 1;
    
#if 1
	// Calling Create3DLookupTable Function in order to create 3D LookUp Table and Dist Table of Dimensions [15 15 15]
	// for every sample pixel in Image Data Matrix
	/*
	 * width=15;
	 * [LookUpTable DistTable]= create3DLookUpTable([width width width],dim(1),dim(2));
	 *
	 * where dim = size(I)=> size of image data matrix
	 */
	if ( giNum_of_Dims_of_Input_Image == 3)
	{
		gaiWindowDims[0] = LOOKUP_TABLE_WIDTH;
		gaiWindowDims[1] = LOOKUP_TABLE_WIDTH;
		gaiWindowDims[2] = LOOKUP_TABLE_WIDTH;
		iVolHeight = gpiDims_InputImage[0];
		iVolWidth = gpiDims_InputImage[1];
	}
	else if ( giNum_of_Dims_of_Input_Image == 2)
	{
		gaiWindowDims[0] = 1;
		gaiWindowDims[1] = LOOKUP_TABLE_WIDTH;
		gaiWindowDims[2] = LOOKUP_TABLE_WIDTH;
		iVolWidth = gpiDims_InputImage[0];
		iVolHeight = 1;
	}
	
	create3DLookupTable(gaiWindowDims, 3, iVolHeight, iVolWidth, &gpiLookUpTable, &gpdDistTable);
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	// Compute the min, max and length of LookUp Table
	//minL = min(LookUpTable); maxL = max(LookUpTable); lenL = length(LookUpTable);
	giMinLookupTable = Compute_3D_Min(gpiLookUpTable, gaiWindowDims[0], gaiWindowDims[1], gaiWindowDims[2]);
	giMaxLookupTable = Compute_3D_Max(gpiLookUpTable, gaiWindowDims[0], gaiWindowDims[1], gaiWindowDims[2]);
	giLenLookupTable = Compute_3D_Len(gpiLookUpTable, gaiWindowDims[0], gaiWindowDims[1], gaiWindowDims[2]);
	
	mexPrintf ("\nLookup Min: %d \nLookup Max: %d \nLookup Len: %d\n", giMinLookupTable , giMaxLookupTable , giLenLookupTable );
	mexEvalString("pause(0.001);"); // To dump the printf string to the console.
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	// Initialization and Declaration of ProjectedPoints
	//projectedPoints = zeros(dData,N);
	// gpmxaProjected_Points = mxCreateDoubleMatrix(giNum_of_Dims_of_Input_Image, giNum_of_pixels, mxREAL); // projectedPoints [dData x N]
	// gpdProjected_Points = mxGetPr(gpmxaProjected_Points);
	// gpmxaTangential_Space = mxCreateDoubleMatrix(giNum_of_Dims_of_Input_Image, giNum_of_pixels, mxREAL); // projectedPoints [dData x N]
	// gpdTangential_Space = mxGetPr(gpmxaTangential_Space);
	
	// gpmxaEigenVals = mxCreateDoubleMatrix(giNum_of_Dims_of_Input_Image, giNum_of_pixels, mxREAL); // projectedPoints [dData x N]
	// gpdEigenVals = mxGetPr(gpmxaEigenVals);
	
	gpmxaScores = mxCreateDoubleMatrix(1, giNum_of_locations, mxREAL); // projectedPoints [dData x N]
	gpdScores = mxGetPr(gpmxaScores);
	
	// iNumber_Of_Elements_in_ProjectedPoints = giNum_of_Dims_of_Input_Image * giNum_of_pixels;
	for(iIndex = 0; iIndex < giNum_of_locations; iIndex++)
	{
		gpdScores[iIndex] = 0;
		// 		gpdProjected_Points[iIndex] = 0;
		// 		gpdTangential_Space[iIndex] = 0;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	int dummy =NUM_OF_THREADS_TO_CREATE;
	for(iThreadNumber = 0; iThreadNumber < NUM_OF_THREADS_TO_CREATE; iThreadNumber++)
	{
		iRet_Val_Pthread_Create = pthread_create(&ptThreads[iThreadNumber], NULL, Thread_ProcessImage, (void*)iThreadNumber);
	}
	
	for(iThreadNumber = 0; iThreadNumber < NUM_OF_THREADS_TO_CREATE; iThreadNumber++)
	{
		pthread_join(ptThreads[iThreadNumber], NULL);
		mexPrintf("Thread %d returns  %d\n", iThreadNumber, iRet_Val_Pthread_Create);
	}
	
	bAll_Threads_Completed_Successfully = TRUE;
	
#if 1
	if (bAll_Threads_Completed_Successfully == TRUE)
	{
		nlhs = 1;
		plhs[0] = gpmxaScores;
		// 		plhs[1] = gpmxaTangential_Space;
		//         plhs[2] = gpmxaEigenVals;
		//         plhs[2] = gpmxaEigenVals;
		//mexPrintf ("\nPlotting Projected Points...\n");
		//mexEvalString("pause(0.001);"); // To dump the printf string to the console.
		//mexCallMATLAB(0, NULL, 1, &gpmxaProjected_Points, "Plot_Projected_Points");
	}
#endif
	//-------------------------------------------------------------------------------------------------------
	mexPrintf ("\nYOU ARE HERE.\n");
	mexEvalString("pause(0.001);"); // To dump the printf string to the console.
	
	//mxDestroyArray(gpmxaInputImage);
#endif
	free (gpdWeights_InputImage);
	free (gpdEigVec_Cov_InputImage);
	free (gpdEigVal_Cov_InputImage);
	free (gpdNormP_EigVal);
	free (gpiData);
	
}// end of mexFunction

void *Thread_ProcessImage (void *ptriThreadNumber) // for pthreads
{
    long iThreadNumber;
    iThreadNumber =	(long) ptriThreadNumber;
    int iLocation_Start, iLocation_End;
    
    iLocation_Start = iThreadNumber;
    iLocation_End = giNum_of_locations-1 ;
    
    int iLocationIterator;
    int iRoundedLocationIterator;
    double dImagePixelWeight;
    double dImagePixel_EigVal_NormP;
    double dInfinity = mxGetInf();
    int iNum_of_Valid_Neighbors;
    
    int *piNeighbors_ImagePixel, *piData_of_Neighbors;
    
    double *pdCurrP, *pdCurrT;
    double *pdWeights_for_KDE, *pdEig_Vecs_for_KDE, *pdEig_Vals_for_KDE, *pdNormP_for_KDE;
	
    double *alpha_ij;
    double *pdKernels, *pdProbs, *pdGrads, *pdHessians, *pdLogHessians;
	
    double *pdEigVals_of_LogHessians, *pdAbsEigVals_of_LogHessians, *pdEigVecs_of_LogHessians, *pdEigVals_of_LogHessians_Extracted, *pdH_Perp, *pdNormc_of_Grads;
    
    // CurrP will have (X,Y,Z) co-ords of updatedimagepixeliterator (3-d) [dData x 1]
    int iCur_Neig ;
    double score;
    int iIter, iWhileLoopIter, iNeigIter, iRowIter, iColIter, iDims;
    int minInd;
    double currEnt, minVal;
    int iFlag;
    int iWhileLoopThreshold;
    
    piNeighbors_ImagePixel = (int *) malloc (giLenLookupTable * sizeof(int)); // indNeig [1 x length(LookupTable)]
    piData_of_Neighbors = (int *) malloc (giNum_of_Dims_of_Input_Image * giLenLookupTable * sizeof(int));  // data(:, indNeig) [dData x length(LookupTable)]
    
    pdCurrT = (double *) malloc (giNum_of_Dims_of_Input_Image *INTRINSIC_DIM* sizeof(double));// CurrT will have (X,Y) vec of tangential space [dData x d_int]
    pdCurrP = (double *) malloc (giNum_of_Dims_of_Input_Image * sizeof(double));// CurrP will have (X,Y) co-ords of updatedimagepixeliterator (2-d) [dData x 1]
    pdWeights_for_KDE = (double *) malloc (giLenLookupTable*sizeof(double)); //  W [1 x length(LookupTable)]
    pdEig_Vecs_for_KDE = (double *) malloc (giNum_of_Dims_of_Input_Image*giNum_of_Dims_of_Input_Image*giLenLookupTable*sizeof(double)); // Vecs [1 x dData.dData.length(LookupTable)]
    pdEig_Vals_for_KDE = (double *) malloc (giNum_of_Dims_of_Input_Image*giLenLookupTable*sizeof(double)); // Lamba(:,indNeig) [1 x dData.length(LookupTable)]
    pdNormP_for_KDE = (double *) malloc (giLenLookupTable*sizeof(double)); // NormP(:,indNeig) [1 x length(LookupTable)]
    
    alpha_ij = (double *)malloc(giNum_of_Dims_of_Input_Image*giLenLookupTable * sizeof(double));
    pdKernels= (double *) malloc(1*giLenLookupTable *sizeof(double));
    pdProbs = (double *) malloc(1*1*sizeof(double));
    pdGrads = (double *) malloc(giNum_of_Dims_of_Input_Image*1*sizeof(double));
    pdHessians = (double *) malloc(giNum_of_Dims_of_Input_Image*giNum_of_Dims_of_Input_Image*1*sizeof(double));
    pdLogHessians = (double *) malloc(giNum_of_Dims_of_Input_Image*giNum_of_Dims_of_Input_Image*1*sizeof(double));
    
    pdEigVals_of_LogHessians = (double*) malloc ( giNum_of_Dims_of_Input_Image * sizeof(double)); // L [1 x dData]
    pdAbsEigVals_of_LogHessians = (double*) malloc ( giNum_of_Dims_of_Input_Image * sizeof(double)); // L [1 x dData]
    pdEigVecs_of_LogHessians = (double*) malloc ( giNum_of_Dims_of_Input_Image * giNum_of_Dims_of_Input_Image * sizeof(double)); // Q [dData x dData]
    pdEigVals_of_LogHessians_Extracted = (double*) malloc ((giNum_of_Dims_of_Input_Image-INTRINSIC_DIM)*(giNum_of_Dims_of_Input_Image-INTRINSIC_DIM) * sizeof (double)); // L(2:end,2:end) [dData-1  x  dData-1]
    pdH_Perp = (double*) malloc (giNum_of_Dims_of_Input_Image * giNum_of_Dims_of_Input_Image * sizeof(double)); // H_Perp [dData x dData]
    pdNormc_of_Grads = (double *) malloc (giNum_of_Dims_of_Input_Image * sizeof(double)); // normc(g) [dData x 1] SAME AS dims of grads (g)

    double dProbs;
    double dProbInf = mxGetInf();
    double eps = mxGetEps();
    
    double dNumerator, dDenominator, dNorm_of_Grads;
    bool bAllEqual = TRUE;
    
	//     mexPrintf ("\nInput Min: %4.2f %4.2f %4.2f\n", gpdInputLocations[0], gpdInputLocations[1], gpdInputLocations[2]);
	//     mexEvalString("pause(0.001);"); // To dump the printf string to the console.
    
#if 1
	// 	for (iLocationIterator = iLocation_Start ; iLocationIterator <= iLocation_End ; iLocationIterator++)
	for (iLocationIterator = iLocation_Start ; iLocationIterator <= iLocation_End ; iLocationIterator+=NUM_OF_THREADS_TO_CREATE)
	{
		mexPrintf ("\n thread:%d loc: %d start: %d end: %d numnThread: %d\n", iThreadNumber, iLocationIterator, iLocation_Start, iLocation_End,NUM_OF_THREADS_TO_CREATE);
		mexEvalString("pause(0.001);"); // To dump the printf string to the console.
		
#if 1 //
		// get the location
		for(iIter = 0; iIter < giNum_of_Dims_of_Input_Image; iIter++)
		{
			pdCurrP[iIter] = gpdInputLocations[iLocationIterator*giNum_of_Dims_of_Input_Image + iIter];
			
			for(iDims = 0; iDims<INTRINSIC_DIM; iDims++)
			{
				//gpdInputTangDir
				pdCurrT[ROWCOL(iIter,iDims,giNum_of_Dims_of_Input_Image)] =
				gpdInputTangDir[iLocationIterator*giNum_of_Dims_of_Input_Image*INTRINSIC_DIM + giNum_of_Dims_of_Input_Image*iDims+ iIter];
			}
			
		}
		mexPrintf ("\nLoc: %4.2f %4.2f %4.2f\n", pdCurrP[0], pdCurrP[1], pdCurrP[2]);
		mexEvalString("pause(0.1);"); // To dump the printf string to the console.
		
		mexPrintf ("\nTang: %4.2f %4.2f %4.2f\n", pdCurrT[0], pdCurrT[1], pdCurrT[2]);
		mexEvalString("pause(0.1);"); // To dump the printf string to the console.
		
		// Get_CurrP(gpdInputLocations, giNum_of_Dims_of_Input_Image, iLocationIterator, pdCurrP);
        
        
        // First get the closest voxel and use the neighborhood of that voxel
		if(giNum_of_Dims_of_Input_Image == 2)
		{
			iRoundedLocationIterator = ROUND(pdCurrP[0]) + ROUND(pdCurrP[1])*gpiDims_InputImage[0];
		}
		else if(giNum_of_Dims_of_Input_Image == 3)
		{
			iRoundedLocationIterator = ROUND(pdCurrP[0]) + ROUND(pdCurrP[1])*gpiDims_InputImage[0] +
			ROUND(pdCurrP[2])*gpiDims_InputImage[0]*gpiDims_InputImage[1];
		}
		
		//indNeig = jj + LookUpTable';
		//validInd = (normP(indNeig)>0 & isfinite(normP(indNeig)));
		//indNeig = indNeig(validInd);
		Get_ImagePixel_Neighbors(gpiLookUpTable, giLenLookupTable, iRoundedLocationIterator, gpdNormP_EigVal,
								 piNeighbors_ImagePixel, &iNum_of_Valid_Neighbors);
		
		//wjmx = (repmat(weights(:,jj),1,lenL)-weights(:,indNeig)).^2/scaleInt^2;%mexCallMATLAB
		//W = weights(:,indNeig).*exp(-wjmx);
		//W = W/sum(W);
		dImagePixelWeight = gpdWeights_InputImage[iRoundedLocationIterator]; // weights(jj)
		
		dImagePixel_EigVal_NormP = gpdNormP_EigVal[iRoundedLocationIterator]; // normP(j)
		Generate_Weights_for_KDE (dImagePixelWeight, piNeighbors_ImagePixel, iNum_of_Valid_Neighbors, gpdWeights_InputImage, pdWeights_for_KDE);
		
		//Vecs = Vectors(:,:,indNeig);
		//Vecs = reshape(Vecs,dData,numel(Vecs)/dData);
		Generate_Eig_Vecs_For_KDE(gpdEigVec_Cov_InputImage, piNeighbors_ImagePixel, iNum_of_Valid_Neighbors, giNum_of_Dims_of_Input_Image,
								  giNum_of_pixels, pdEig_Vecs_for_KDE );
		//-------------------------------------------------------------------------------------------------------------------------------------------------
		
		score = DEFAULT_SCORE;
		
		//------------------------------------------------------------------------------------------------------------------------------------------------
		// Inputs for KDE
		
		for (iNeigIter = 0 ; iNeigIter < iNum_of_Valid_Neighbors ; iNeigIter ++)
		{
			iCur_Neig = piNeighbors_ImagePixel[iNeigIter];
			for (iIter = 0 ; iIter < giNum_of_Dims_of_Input_Image ; iIter++)
			{
				piData_of_Neighbors[iIter + iNeigIter*giNum_of_Dims_of_Input_Image] =
				gpiData[iIter + iCur_Neig*giNum_of_Dims_of_Input_Image]; // data(:,indNeig)
				pdEig_Vals_for_KDE[iIter + iNeigIter*giNum_of_Dims_of_Input_Image] =
				gpdEigVal_Cov_InputImage[iIter + iCur_Neig*giNum_of_Dims_of_Input_Image]; // Lambda(:,indNeig)
			}
			pdNormP_for_KDE[iNeigIter] =
			gpdNormP_EigVal[iCur_Neig]*iNum_of_Valid_Neighbors/giLenLookupTable; 	// normP(:,indNeig)*sum(validInd)/lenL
		}
		//-----------------------------------------------------------------------------------------------------------------------------------------------
		KernelDensityEstimation(pdCurrP, giNum_of_Dims_of_Input_Image, piData_of_Neighbors, pdWeights_for_KDE, pdEig_Vecs_for_KDE,
								pdEig_Vals_for_KDE, pdNormP_for_KDE, 1, iNum_of_Valid_Neighbors, beta, alpha_ij,
								pdKernels, pdProbs, pdGrads, pdHessians, pdLogHessians, 5);
		
#if 1 // just after KDE
		
		dProbs = pdProbs[0];
		if((dProbs*dProbs == 0) || (dProbs == dProbInf))
		{
			iFlag = 0;
			score = 0;
		}
		else
		{
			if (giNum_of_Dims_of_Input_Image == 2)
			{
				Compute_Eig_Dec_2D(1, &pdLogHessians[0], &pdLogHessians[2], &pdLogHessians[3],
								   &pdEigVals_of_LogHessians[0], &pdEigVals_of_LogHessians[1],
								   &pdEigVecs_of_LogHessians[0], &pdEigVecs_of_LogHessians[1], &pdEigVecs_of_LogHessians[2], &pdEigVecs_of_LogHessians[3]);
			}
			else if (giNum_of_Dims_of_Input_Image == 3)
			{
				Compute_Eig_Dec_3D(1, &pdLogHessians[0], &pdLogHessians[3], &pdLogHessians[6], &pdLogHessians[4], &pdLogHessians[7], &pdLogHessians[8],
								   &pdEigVecs_of_LogHessians[0], &pdEigVecs_of_LogHessians[1], &pdEigVecs_of_LogHessians[2],
								   &pdEigVecs_of_LogHessians[3], &pdEigVecs_of_LogHessians[4], &pdEigVecs_of_LogHessians[5],
								   &pdEigVecs_of_LogHessians[6], &pdEigVecs_of_LogHessians[7], &pdEigVecs_of_LogHessians[8],
								   &pdEigVals_of_LogHessians[0], &pdEigVals_of_LogHessians[1], &pdEigVals_of_LogHessians[2]);
			}

            SortTangentialSpace(pdCurrT, pdEigVecs_of_LogHessians, pdEigVals_of_LogHessians, giNum_of_Dims_of_Input_Image);
			
//             // check if the projection is on the PC or minor curve
//             for (iIter = INTRINSIC_DIM ; iIter < giNum_of_Dims_of_Input_Image ; iIter++)
//             {
//                 if (pdEigVals_of_LogHessians[iIter]>0)
//                 {
//                     score = DEFAULT_SCORE;
//                     break;
//                 }
//             }

            #if 0
            // take the absolute value
			for (iIter = 0 ; iIter < giNum_of_Dims_of_Input_Image ; iIter++)
			{
				if (pdEigVals_of_LogHessians[iIter] < 0)
				{
					pdAbsEigVals_of_LogHessians[iIter] = -pdEigVals_of_LogHessians[iIter];
				}
				else
				{
					pdAbsEigVals_of_LogHessians[iIter] = pdEigVals_of_LogHessians[iIter];
				}
			}
            #else
                                // take the absolute value
			for (iIter = 0 ; iIter < giNum_of_Dims_of_Input_Image ; iIter++)
			{
					pdAbsEigVals_of_LogHessians[iIter] = pdEigVals_of_LogHessians[iIter];
			}

            #endif

            // Populate L(2:end, 2:end)
			for (iColIter = 0 ; iColIter < (giNum_of_Dims_of_Input_Image-INTRINSIC_DIM) ; iColIter++)
			{
				for (iRowIter = 0 ; iRowIter < (giNum_of_Dims_of_Input_Image-INTRINSIC_DIM) ; iRowIter ++)
				{
					if (iRowIter == iColIter)
					{
						pdEigVals_of_LogHessians_Extracted[ROWCOL(iRowIter, iColIter, (giNum_of_Dims_of_Input_Image-INTRINSIC_DIM))] =
                        1/pdAbsEigVals_of_LogHessians[iColIter+1];
					}
					else
					{
						pdEigVals_of_LogHessians_Extracted[ROWCOL(iRowIter, iColIter, (giNum_of_Dims_of_Input_Image-INTRINSIC_DIM))] = 0;
					}
				}
			}
            
			//---------------------------------------------------------------------------------------------------------------------------------------------
			//Hperp = Q(:,2:end)*L(2:end,2:end)*Q(:,2:end)';%nEigVal + 1*/
			Compute_H_perp (pdEigVecs_of_LogHessians, pdEigVals_of_LogHessians_Extracted, giNum_of_Dims_of_Input_Image, pdH_Perp);
			
			//---------------------------------------------------------------------------------------------------------------------------------------------
			//nm = -(g'*Hperp*g);
			dNumerator = Compute_Numerator(pdH_Perp, pdGrads, giNum_of_Dims_of_Input_Image);
			
			//---------------------------------------------------------------------------------------------------------------------------------------------
			//dm = norm(g)*norm(HH\g);%inverse HH*g
			dDenominator = Compute_Denominator(pdGrads, pdLogHessians, giNum_of_Dims_of_Input_Image);
			
			//---------------------------------------------------------------------------------------------------------------------------------------------
			dNorm_of_Grads = Compute_Norm(pdGrads, giNum_of_Dims_of_Input_Image); // norm(g)
			
			if(dNorm_of_Grads < eps)
			{
				score = 0;
				//mexPrintf ("\nScore = 0, for CurrP(%g, %g)", pdCurrP[0], pdCurrP[1]);
			}
			else
			{
				score = dNumerator/dDenominator;
			}
			
            // check if the projection is on the PC or minor curve
            for (iIter = INTRINSIC_DIM ; iIter < giNum_of_Dims_of_Input_Image ; iIter++)
            {
                if (pdEigVals_of_LogHessians[iIter]>0)
                {
                    score = DEFAULT_SCORE;
                    break;
                }
            }

        }
		
#endif // #if 0 // just after KDE
		
		mexPrintf ("\n iLocationIterator: %d\n", iLocationIterator);
		mexEvalString("pause(0.001);"); // To dump the printf string to the console.
		
		gpdScores[iLocationIterator] =  score;
		
#endif // #if 0
		
	} // end of for (iLocationIterator ...)
	
#endif // #if 0
	
	free (piNeighbors_ImagePixel); 
	free (piData_of_Neighbors);
	
	free (pdCurrT); 
	free (pdCurrP); 
	free (pdWeights_for_KDE); 
	free (pdEig_Vecs_for_KDE); 
	free (pdEig_Vals_for_KDE); 
	free (pdNormP_for_KDE);
	
	free(alpha_ij);
	free(pdKernels); 
	free(pdProbs); 
	free(pdGrads); 
	free (pdHessians); 
	free(pdLogHessians);
	
	free (pdEigVals_of_LogHessians); 
	free (pdAbsEigVals_of_LogHessians);
	free (pdEigVecs_of_LogHessians); 
	free (pdEigVals_of_LogHessians_Extracted); 
	free (pdH_Perp);
	free (pdNormc_of_Grads);
// 	free (pdTranspose_of_Tangential);
	//------------------------------------------------------------------------------------------------------------------------------------------------
	
}