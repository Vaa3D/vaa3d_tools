/* PSF_zhi_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2013-03-07 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include<iostream>
#include<fstream>
#include <pthread.h>

#include "PSF_zhi_plugin.h"
#include <cfloat>
#include <math.h>
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


V3DLONG gpiDims_InputImage[3];
int giNum_of_pixels, giNum_of_Dims_of_Input_Image;
double *gpdInputImage;

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

double *gpdProjected_Points;
double *gpdTangential_Space;
double *gpdEigenVals;
double *gpdScores;

double *gpdInputLocations;
double *gpdInputTangDir;
int giNum_of_locations;

using namespace std;
Q_EXPORT_PLUGIN2(PSF_zhi, PSF_zhi);

void *Thread_ProcessImage (void *); // for Pthreads
void *Thread_ProcessImage_score (void *); // for Pthreads

void autotrace_PSF(V3DPluginCallback2 &callback, QWidget *parent);
void PreProcessDataImage(double *, int , V3DLONG *, int , double *,
        double *, double *, double *, int *, char *, double, int);
void ProcessImage(double *, double *, double *,int,int);
void ProcessImage_score(double *, double *, double*, int, int, int);

 
QStringList PSF_zhi::menulist() const
{
	return QStringList() 
		<<tr("trace")
		<<tr("about");
}

QStringList PSF_zhi::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void PSF_zhi::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("trace"))
	{
        autotrace_PSF(callback,parent);
	}
	else if (menu_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2013-03-07"));
	}
}

bool PSF_zhi::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

void autotrace_PSF(V3DPluginCallback2 &callback, QWidget *parent)
{

    char *strFilename;
    char *strOutname;
    char *strUpdateAllKDEInputs;
    int iVolWidth, iVolHeight;
    int iNumber_Of_Elements_in_ProjectedPoints;
    int iIndex;
    double sigma = 3;
    int prepLookUpTableWidth;

    pthread_t ptThreads[NUM_OF_THREADS_TO_CREATE];
    int iThreadNumber;
    int iRet_Val_Pthread_Create;
    bool bAll_Threads_Completed_Successfully = TRUE;

    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
        return;
    }

    Image4DSimple* p4DImage = callback.getImage(curwin);

    if (!p4DImage)
    {
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return;
    }


    unsigned char* data1d = p4DImage->getRawData();
    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();
    V3DLONG pagesz = N*M*P;

    bool ok1;
    int c,p = 0;

    if(sc==1)
    {
        c=1;
        ok1=true;
    }
    else
    {
        c = QInputDialog::getInteger(parent, "Channel",
                                         "Enter channel NO:",
                                         1, 1, sc, 1, &ok1);
    }

    if(!ok1)
        return;

    gpdInputImage = new double[pagesz];
    V3DLONG i = 0;

    for(V3DLONG iz = 0; iz < P; iz++)
    {
        V3DLONG offsetk = iz*M*N;
        for(V3DLONG iy = 0; iy < M; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = 0; ix < N; ix++)
            {
                gpdInputImage[i] = (double)data1d[offsetk + offsetj + ix]/255.0;
                i++;
            }
        }
    }

    giNum_of_Dims_of_Input_Image = 3;
    gpiDims_InputImage[0] = N; gpiDims_InputImage[1] = M; gpiDims_InputImage[2] = P;
    giNum_of_pixels = pagesz;

    // Allocate memory for all outputs generated in Pre-processing Step.

    gpdWeights_InputImage = new double[giNum_of_pixels];//weights [row vector, 1 x N]
    gpdEigVec_Cov_InputImage = new double[giNum_of_Dims_of_Input_Image*giNum_of_Dims_of_Input_Image*giNum_of_pixels];// Vectors [dData x dData * N]
    gpdEigVal_Cov_InputImage = new double[giNum_of_Dims_of_Input_Image*giNum_of_pixels]; // Lambda [dData x N]
    gpdNormP_EigVal = new double[giNum_of_pixels];
    gpiData = new int[giNum_of_Dims_of_Input_Image*giNum_of_pixels];

    prepLookUpTableWidth = 11;
    PreProcessDataImage(gpdInputImage, giNum_of_Dims_of_Input_Image, gpiDims_InputImage, giNum_of_pixels,
             gpdWeights_InputImage, gpdEigVec_Cov_InputImage, gpdEigVal_Cov_InputImage,
            gpdNormP_EigVal,gpiData, "/opt/zhi/Desktop/tmp/", sigma, prepLookUpTableWidth);

    printf ("\nBack to maincode.\n");

    gaiWindowDims[0] = LOOKUP_TABLE_WIDTH;
    gaiWindowDims[1] = LOOKUP_TABLE_WIDTH;
    gaiWindowDims[2] = LOOKUP_TABLE_WIDTH;
    iVolHeight = gpiDims_InputImage[0];
    iVolWidth = gpiDims_InputImage[1];

    create3DLookupTable(gaiWindowDims, 3, iVolHeight, iVolWidth, &gpiLookUpTable, &gpdDistTable);

    giMinLookupTable = Compute_3D_Min(gpiLookUpTable, gaiWindowDims[0], gaiWindowDims[1], gaiWindowDims[2]);
    giMaxLookupTable = Compute_3D_Max(gpiLookUpTable, gaiWindowDims[0], gaiWindowDims[1], gaiWindowDims[2]);
    giLenLookupTable = Compute_3D_Len(gpiLookUpTable, gaiWindowDims[0], gaiWindowDims[1], gaiWindowDims[2]);

    printf ("\nLookup Min: %d \nLookup Max: %d \nLookup Len: %d\n", giMinLookupTable , giMaxLookupTable , giLenLookupTable );

    iNumber_Of_Elements_in_ProjectedPoints = giNum_of_Dims_of_Input_Image * giNum_of_pixels;
    gpdProjected_Points = new double[iNumber_Of_Elements_in_ProjectedPoints];
    gpdTangential_Space = new double[iNumber_Of_Elements_in_ProjectedPoints];
    gpdEigenVals = new double[iNumber_Of_Elements_in_ProjectedPoints];

    for(iIndex = 0; iIndex < iNumber_Of_Elements_in_ProjectedPoints; iIndex++)
    {
        gpdProjected_Points[iIndex] = 0;
        gpdTangential_Space[iIndex] = 0;
    }

    for(iThreadNumber = 0; iThreadNumber < NUM_OF_THREADS_TO_CREATE; iThreadNumber++)
    {
        iRet_Val_Pthread_Create = pthread_create(&ptThreads[iThreadNumber], NULL, Thread_ProcessImage, (void*)iThreadNumber);
    }

    for(iThreadNumber = 0; iThreadNumber < NUM_OF_THREADS_TO_CREATE; iThreadNumber++)
    {
        pthread_join(ptThreads[iThreadNumber], NULL);
    }



    char saveName[80];
    char *outputDir = "/opt/zhi/Desktop/tmp/";
    FILE *fp_final_proj, *fp_final_Tangential, *fp_final_EigenVals;

    strcpy(saveName,outputDir);
    strcat(saveName,"mex_final_Proj.txt");
    fp_final_proj = fopen (saveName, "w");
    if (fp_final_proj == NULL)
    {
        printf ("\nCannot create file: mex_final_Proj.txt\n");
    }
    for (int iColIter = 0; iColIter < giNum_of_pixels ; iColIter ++)
    {
        for (int iRowIter = 0; iRowIter < giNum_of_Dims_of_Input_Image ; iRowIter ++)
        {
            fprintf (fp_final_proj, "%g ", gpdProjected_Points[ROWCOL(iRowIter, iColIter, giNum_of_Dims_of_Input_Image)]);
        }
        fprintf (fp_final_proj, "\n");
    }
    fclose (fp_final_proj);

    strcpy(saveName,outputDir);
    strcat(saveName,"mex_final_Tangential.txt");
    fp_final_Tangential = fopen (saveName, "w");
    if (fp_final_Tangential == NULL)
    {
        printf ("\nCannot create file: mex_final_Tangential.txt\n");
    }
    for (int iColIter = 0; iColIter < giNum_of_pixels ; iColIter ++)
    {
        for (int iRowIter = 0; iRowIter < giNum_of_Dims_of_Input_Image ; iRowIter ++)
        {
            fprintf (fp_final_Tangential, "%g ", gpdTangential_Space[ROWCOL(iRowIter, iColIter, giNum_of_Dims_of_Input_Image)]);
        }
        fprintf (fp_final_Tangential, "\n");
    }
    fclose (fp_final_Tangential);

    strcpy(saveName,outputDir);
    strcat(saveName,"mex_final_EigenVals.txt");
    fp_final_EigenVals = fopen (saveName, "w");
    if (fp_final_EigenVals == NULL)
    {
        printf ("\nCannot create file: mex_final_EigenVals.txt\n");
    }
    for (int iColIter = 0; iColIter < giNum_of_pixels ; iColIter ++)
    {
        for (int iRowIter = 0; iRowIter < giNum_of_Dims_of_Input_Image ; iRowIter ++)
        {
            fprintf (fp_final_EigenVals, "%g ", gpdEigenVals[ROWCOL(iRowIter, iColIter, giNum_of_Dims_of_Input_Image)]);
        }
        fprintf (fp_final_EigenVals, "\n");
    }
    fclose (fp_final_EigenVals);


   /* ifstream xfile;
    xfile.open("/opt/zhi/Desktop/tmp/X.out");
    gpdInputLocations = new double[3*161880];
    if (xfile.is_open())
    {
        for(V3DLONG i = 0; i<3*161880; i++)
        {
           xfile >> gpdInputLocations[i];
        }
    }
    xfile.close();


    ifstream tfile;
    tfile.open("/opt/zhi/Desktop/tmp/T.out");
    gpdInputTangDir = new double[3*161880];
    if (tfile.is_open())
    {
        for(V3DLONG i = 0; i<3*161880; i++)
        {
           tfile >> gpdInputTangDir[i];
        }
    }
    tfile.close();

    giNum_of_locations = 161880;
    gpdScores = new double[giNum_of_locations];
    for(iIndex = 0; iIndex < giNum_of_locations; iIndex++)
    {
        gpdScores[iIndex] = 0;
    }


    int dummy =NUM_OF_THREADS_TO_CREATE;
    for(iThreadNumber = 0; iThreadNumber < NUM_OF_THREADS_TO_CREATE; iThreadNumber++)
    {
        iRet_Val_Pthread_Create = pthread_create(&ptThreads[iThreadNumber], NULL, Thread_ProcessImage_score, (void*)iThreadNumber);
    }

    for(iThreadNumber = 0; iThreadNumber < NUM_OF_THREADS_TO_CREATE; iThreadNumber++)
    {
        pthread_join(ptThreads[iThreadNumber], NULL);
       // mexPrintf("Thread %d returns  %d\n", iThreadNumber, iRet_Val_Pthread_Create);
    }


  //  ProcessImage_score(gpdScores,gpdInputLocations,gpdInputTangDir,giNum_of_locations,giNum_of_Dims_of_Input_Image,giLenLookupTable);

    char saveName[80];
    char *outputDir = "/opt/zhi/Desktop/tmp/";
    FILE *fp_final_score;

    strcpy(saveName,outputDir);
    strcat(saveName,"mex_final_Score.txt");
    fp_final_score = fopen (saveName, "w");
    if (fp_final_score == NULL)
    {
        printf ("\nCannot create file: mex_final_Score.txt\n");
    }
    for (int iColIter = 0; iColIter < giNum_of_locations ; iColIter ++)
    {

        fprintf (fp_final_score, "%g ", gpdScores[iColIter]);
        fprintf (fp_final_score, "\n");
    }
    fclose (fp_final_score);*/

    //process the image
    //ProcessImage(gpdProjected_Points,gpdTangential_Space,gpdEigenVals,giNum_of_Dims_of_Input_Image,giLenLookupTable);

    free (gpdWeights_InputImage);
    free (gpdEigVec_Cov_InputImage);
    free (gpdEigVal_Cov_InputImage);
    free (gpdNormP_EigVal);
    free (gpiData);

}

//void ProcessImage_score(double *gpdScores, double *gpdInputLocations, double *gpdInputTangDir,int giNum_of_locations,int giNum_of_Dims_of_Input_Image, int giLenLookupTable)
void *Thread_ProcessImage_score (void *ptriThreadNumber) // for pthreads
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
    double dProbInf = INFINITY;
    double eps =  2.2204e-16;

    double dNumerator, dDenominator, dNorm_of_Grads;
    bool bAllEqual = TRUE;

    //     mexPrintf ("\nInput Min: %4.2f %4.2f %4.2f\n", gpdInputLocations[0], gpdInputLocations[1], gpdInputLocations[2]);
    //     mexEvalString("pause(0.001);"); // To dump the printf string to the console.

#if 1
    // 	for (iLocationIterator = iLocation_Start ; iLocationIterator <= iLocation_End ; iLocationIterator++)
    for (iLocationIterator = iLocation_Start ; iLocationIterator <= iLocation_End ; iLocationIterator+=NUM_OF_THREADS_TO_CREATE)
     {
        printf ("\n thread:%d loc: %d start: %d end: %d numnThread: %d\n", iThreadNumber, iLocationIterator, iLocation_Start, iLocation_End,NUM_OF_THREADS_TO_CREATE);

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
        printf ("\nLoc: %4.2f %4.2f %4.2f\n", pdCurrP[0], pdCurrP[1], pdCurrP[2]);

        printf ("\nTang: %4.2f %4.2f %4.2f\n", pdCurrT[0], pdCurrT[1], pdCurrT[2]);

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

        printf ("\n iLocationIterator: %d\n", iLocationIterator);
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




}

//void ProcessImage(double *gpdProjected_Points, double *gpdTangential_Space, double *gpdEigenVals, int giNum_of_Dims_of_Input_Image, int giLenLookupTable)
void *Thread_ProcessImage (void *ptriThreadNumber) // for pthreads
{

        long iThreadNumber;
        iThreadNumber =	(long) ptriThreadNumber;
        int iImagePixel_Start, iImagePixel_End;

        iImagePixel_Start = iThreadNumber;
        iImagePixel_End = giNum_of_pixels-1 ;

        int iImagePixelIterator;
        int iUpdatedImagePixelIterator;
        double dImagePixelWeight;
        double dImagePixel_EigVal_NormP;
        int iNum_of_Valid_Neighbors;

        double *alpha_ij;
        double *pdKernels, *pdProbs, *pdGrads, *pdHessians, *pdLogHessians;
        double *pdWeights_for_KDE, *pdEig_Vecs_for_KDE, *pdEig_Vals_for_KDE, *pdNormP_for_KDE;
        double *pdCurrP;
        double *pdEigVals_of_LogHessians, *pdAbsEigVals_of_LogHessians, *pdEigVecs_of_LogHessians, *pdEigVals_of_LogHessians_Extracted, *pdH_Perp, *pdNormc_of_Grads;

                            // CurrP will have (X,Y,Z) co-ords of updatedimagepixeliterator (3-d) [dData x 1]
        int *piNeighbors_ImagePixel, *piData_of_Neighbors;
        int iCur_Neig ;
        double score;
        int iIter, iWhileLoopIter, iNeigIter, iRowIter, iColIter;
        int iFlag;
        int iWhileLoopThreshold;

        piNeighbors_ImagePixel = (int *) malloc (giLenLookupTable * sizeof(int)); // indNeig [1 x length(LookupTable)]
        piData_of_Neighbors = (int *) malloc (giNum_of_Dims_of_Input_Image * giLenLookupTable * sizeof(int));  // data(:, indNeig) [dData x length(LookupTable)]

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
        double dProbInf = INFINITY;
        double eps =  2.2204e-16;

        double dNumerator, dDenominator, dNorm_of_Grads;


        bool bAllEqual = TRUE;

        #if 1
        for (iImagePixelIterator = iImagePixel_Start ; iImagePixelIterator <= iImagePixel_End ; iImagePixelIterator+=NUM_OF_THREADS_TO_CREATE)
        {

            dImagePixelWeight = gpdWeights_InputImage[iImagePixelIterator]; // weights(j)
            dImagePixel_EigVal_NormP = gpdNormP_EigVal[iImagePixelIterator]; // normP(j)
            if((iImagePixelIterator < (-giMinLookupTable)) ||
                (iImagePixelIterator > (giNum_of_pixels - giMaxLookupTable - 1)) ||
                (dImagePixel_EigVal_NormP == INFINITY) ||
                (gpdNormP_EigVal[iImagePixelIterator] < eps) ||
                (dImagePixelWeight < W_THRESHOLD))
            {
                continue;
            }
             printf("current wieght is %f\n",dImagePixelWeight);
            iUpdatedImagePixelIterator = iImagePixelIterator;
            //-------------------------------------------------------------------------------------------------------------------------------------------------
            dImagePixelWeight = gpdWeights_InputImage[iImagePixelIterator]; // weights(jj)

            Get_ImagePixel_Neighbors(gpiLookUpTable, giLenLookupTable, iImagePixelIterator, gpdNormP_EigVal, piNeighbors_ImagePixel, &iNum_of_Valid_Neighbors);

            Generate_Weights_for_KDE (dImagePixelWeight, piNeighbors_ImagePixel, iNum_of_Valid_Neighbors, gpdWeights_InputImage, pdWeights_for_KDE);

            //------------------------------------------------------------------------------------------------------------------------------------------------
            // Generate eigen vectors for KDE

            Generate_Eig_Vecs_For_KDE(gpdEigVec_Cov_InputImage, piNeighbors_ImagePixel, iNum_of_Valid_Neighbors, giNum_of_Dims_of_Input_Image,
                                      giNum_of_pixels, pdEig_Vecs_for_KDE );
            //-------------------------------------------------------------------------------------------------------------------------------------------------

            // Inputs for KDE, while loop

            Get_CurrP (gpiData, giNum_of_Dims_of_Input_Image, iUpdatedImagePixelIterator, pdCurrP);

            iWhileLoopIter = 1;
            iFlag = DEFAULT_FLAG;
            iWhileLoopThreshold = DEFAULT_WHILE_LOOP_THR;
            bAllEqual = TRUE;
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
            // while abs(score)>sqrt(eps) & t<numIterThr
            while ( (fabs(score) > sqrt(eps)) && (iWhileLoopIter < iWhileLoopThreshold) )
            {

                KernelDensityEstimation(pdCurrP, giNum_of_Dims_of_Input_Image, piData_of_Neighbors, pdWeights_for_KDE, pdEig_Vecs_for_KDE,
                                        pdEig_Vals_for_KDE, pdNormP_for_KDE, 1, iNum_of_Valid_Neighbors, beta, alpha_ij,
                                        pdKernels, pdProbs, pdGrads, pdHessians, pdLogHessians, 5);


        #if 1 // just after KDE
                dProbs = pdProbs[0];
                if((dProbs*dProbs == 0) || (dProbs == dProbInf))
                {
                    iFlag = 0;
                    //mexPrintf ("\nProbs = 0, iWhileLoopIter = %d", iWhileLoopIter);
                    break;
                }
                //---------------------------------------------------------------------------------------------------------------------------------------------
                // [Q,L]=svd(HH);
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

                for (iIter = 0 ; iIter < giNum_of_Dims_of_Input_Image ; iIter++)
                {
                    if (pdEigVals_of_LogHessians[iIter] < 0)
                    {
        // 					iFlag = 0;
        //                     goto outer;
                        pdAbsEigVals_of_LogHessians[iIter] = -pdEigVals_of_LogHessians[iIter];
                    }
                    else
                    {
                        pdAbsEigVals_of_LogHessians[iIter] = pdEigVals_of_LogHessians[iIter];

                    }
                }

               // Populate L(d+1:end, d+1:end)
                for (iColIter = 0 ; iColIter < (giNum_of_Dims_of_Input_Image-INTRINSIC_DIM) ; iColIter++)
                {
                   for (iRowIter = 0 ; iRowIter < (giNum_of_Dims_of_Input_Image-INTRINSIC_DIM) ; iRowIter ++)
                   {
                       if (iRowIter == iColIter)
                       {
                           pdEigVals_of_LogHessians_Extracted[ROWCOL(iRowIter, iColIter, (giNum_of_Dims_of_Input_Image-INTRINSIC_DIM))] =
                                       1/pdAbsEigVals_of_LogHessians[iColIter+INTRINSIC_DIM ];
                       }
                       else
                       {
                           pdEigVals_of_LogHessians_Extracted[ROWCOL(iRowIter, iColIter, (giNum_of_Dims_of_Input_Image-INTRINSIC_DIM ))] = 0;
                       }
                   }
               }

                //Hperp = Q(:,2:end)*L(2:end,2:end)*Q(:,2:end)';%nEigVal + 1*/
                Compute_H_perp (pdEigVecs_of_LogHessians, pdEigVals_of_LogHessians_Extracted, giNum_of_Dims_of_Input_Image, pdH_Perp);

                //---------------------------------------------------------------------------------------------------------------------------------------------
                //nm = -(g'*Hperp*g);
                dNumerator = Compute_Numerator(pdH_Perp, pdGrads, giNum_of_Dims_of_Input_Image);
                //----  -----------------------------------------------------------------------------------------------------------------------------------------
                //dm = norm(g)*norm(HH\g);%inverse HH*g
                dDenominator = Compute_Denominator(pdGrads, pdLogHessians, giNum_of_Dims_of_Input_Image);


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

                //---------------------------------------------------------------------------------------------------------------------------------------------
                // Compute normc(g)
                Compute_Normc(pdGrads, giNum_of_Dims_of_Input_Image, 1, pdNormc_of_Grads);

                if(iWhileLoopIter==306)
                    int stop=1;


                if (fabs(score) > sqrt(eps))
                {
                    //currP = currP + muT*Q(:,2:end)*Q(:,2:end)'*normc(g);
                Compute_Updated_CurrP(pdCurrP, pdEigVecs_of_LogHessians, pdNormc_of_Grads, giNum_of_Dims_of_Input_Image);

                }
                else
                {
                    int stop=1;
                }
                //---------------------------------------------------------------------------------------------------------------------------------------------
                //if ~all(ROUND(currP)==data(:,jj)) %| all(diag(L)>0)
                // any(ROUND(currP)!=data(:,jj))
                for(iIter = 0; iIter < giNum_of_Dims_of_Input_Image; iIter++)
                {
                    if(ROUND(pdCurrP[iIter]) != gpiData[ROWCOL(iIter, iUpdatedImagePixelIterator, giNum_of_Dims_of_Input_Image)])
                    {
                        bAllEqual = FALSE;
                    }
                }
                if (bAllEqual == FALSE) // means [ ~all(ROUND(currP)==data(:,jj)) ] is TRUE
                {
                    if (gbUpdateAllKDEInputs == TRUE)
                    {
                        //jj = sub2ind(dim,ROUND(currP(1)),ROUND(currP(2)),ROUND(currP(3)));
                        if(giNum_of_Dims_of_Input_Image == 2)
                        {
                            iUpdatedImagePixelIterator = ROUND(pdCurrP[0]) + ROUND(pdCurrP[1])*gpiDims_InputImage[0];
                        }
                        else if(giNum_of_Dims_of_Input_Image == 3)
                        {
                            iUpdatedImagePixelIterator = ROUND(pdCurrP[0]) + ROUND(pdCurrP[1])*gpiDims_InputImage[0] +
                                                          ROUND(pdCurrP[2])*gpiDims_InputImage[0]*gpiDims_InputImage[1];
                        }

                        //indNeig = jj + LookUpTable';
                        //validInd = (normP(indNeig)>0 & isfinite(normP(indNeig)));
                        //indNeig = indNeig(validInd);
                        Get_ImagePixel_Neighbors(gpiLookUpTable, giLenLookupTable, iUpdatedImagePixelIterator, gpdNormP_EigVal,
                                                 piNeighbors_ImagePixel, &iNum_of_Valid_Neighbors);

                        //wjmx = (repmat(weights(:,jj),1,lenL)-weights(:,indNeig)).^2/scaleInt^2;%mexCallMATLAB
                        //W = weights(:,indNeig).*exp(-wjmx);
                        //W = W/sum(W);
                        dImagePixelWeight = gpdWeights_InputImage[iUpdatedImagePixelIterator]; // weights(jj)
                        Generate_Weights_for_KDE (dImagePixelWeight, piNeighbors_ImagePixel, iNum_of_Valid_Neighbors, gpdWeights_InputImage, pdWeights_for_KDE);

                        //Vecs = Vectors(:,:,indNeig);
                        //Vecs = reshape(Vecs,dData,numel(Vecs)/dData);
                        Generate_Eig_Vecs_For_KDE(gpdEigVec_Cov_InputImage, piNeighbors_ImagePixel, iNum_of_Valid_Neighbors, giNum_of_Dims_of_Input_Image,
                                          giNum_of_pixels, pdEig_Vecs_for_KDE );
                    }
                    else
                    {
                        iFlag = 0;
                        break;
                    }
                }
                else // means [ ~all(ROUND(currP)==data(:,jj)) ] is FALSE
                {

                }
        #endif // #if 0 // just after KDE
            //------------------------------------------------------------------------------------------------------------------------------------------------
                //t = t+1;
                iWhileLoopIter = iWhileLoopIter + 1;
              //  printf("\n Incremented while loop iter %d", iWhileLoopIter);
            }//end of while loop
            //------------------------------------------------------------------------------------------------------------------------------------------------
            /*
            if flag & t<numIterThr
                projectedPoints(:,j) = currP;
            end
            */
        // outer:

            // check if the projection is on the PC or minor curve
            for (iIter = INTRINSIC_DIM ; iIter < giNum_of_Dims_of_Input_Image ; iIter++)
            {
                if (pdEigVals_of_LogHessians[iIter]>0)
                {
                    iFlag=0;
                    break;
                }
            }

            if((iFlag) && (iWhileLoopIter < iWhileLoopThreshold))
            {
                // projectedPoints(:,j) = currP;
                #if 1
                //mexPrintf ("\nProjection at: ");
                for (iIter = 0; iIter < giNum_of_Dims_of_Input_Image ; iIter++)
                {
                    gpdProjected_Points[ROWCOL(iIter, iImagePixelIterator, giNum_of_Dims_of_Input_Image)] = pdCurrP[iIter]+1;
        //				get the last column of the eigenvector matrix
                    gpdTangential_Space[ROWCOL(iIter, iImagePixelIterator, giNum_of_Dims_of_Input_Image)] =  pdEigVecs_of_LogHessians[iIter];
        //				get the eigenValues
                    gpdEigenVals[ROWCOL(iIter, iImagePixelIterator, giNum_of_Dims_of_Input_Image)] =  pdEigVals_of_LogHessians[iIter];
                    //mexPrintf ("%g  ", pdCurrP[iIter]);
                }
                #endif //#if 0
            }
        } // end of for (iImagePixelIterator ...)

        #endif // #if 0

        free (pdWeights_for_KDE); free (pdEig_Vecs_for_KDE); free (pdEig_Vals_for_KDE); free (pdNormP_for_KDE);
        free (pdCurrP); free (piNeighbors_ImagePixel); free (piData_of_Neighbors);
        free(alpha_ij);
        free(pdKernels); free(pdProbs); free(pdGrads); free (pdHessians); free(pdLogHessians);
        free (pdEigVals_of_LogHessians); free (pdAbsEigVals_of_LogHessians);
        free (pdEigVecs_of_LogHessians); free (pdEigVals_of_LogHessians_Extracted); free (pdH_Perp);
        free (pdNormc_of_Grads);
}
