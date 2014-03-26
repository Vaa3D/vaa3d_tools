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

#include "extractTree.h"

#include <stdio.h>
#include <stdlib.h>
#include <queue>
#include <vector>


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
double eps =  2.2204e-16;
int hmirror;


Q_EXPORT_PLUGIN2(PSF_zhi, PSF_zhi);

void *Thread_ProcessImage (void *); // for Pthreads
void *Thread_ProcessImage_score (void *); // for Pthreads

void autotrace_PSF(V3DPluginCallback2 &callback, QWidget *parent);
void PreProcessDataImage(double *, int , V3DLONG *, int , double *,
        double *, double *, double *, int *, char *, double, int);
void ProcessImage(double *, double *, double *,int,int);
void ProcessImage_score(double *, double *, double*, int, int, int);
void preIntegral(double *, double *, int, int, int,int, double *, double *,double *, int *);
void graph_all_shortest_paths(double *, double*, int);
int minDistance(double dist[], bool sptSet[], int V);
void dijkstra(double *graph, int num, int src, double* dist);
 
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

    int iVolWidth, iVolHeight;
    int iNumber_Of_Elements_in_ProjectedPoints;
    int iIndex;
    double sigma = 3;
    int prepLookUpTableWidth;

    pthread_t ptThreads[NUM_OF_THREADS_TO_CREATE];
    int iThreadNumber;
    int iRet_Val_Pthread_Create;

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

    hmirror = 7;
    V3DLONG N_new = N+2*hmirror;
    V3DLONG M_new = M+2*hmirror;
    V3DLONG P_new = P+2*hmirror;
    V3DLONG pagesz_new = N_new*M_new*P_new;

    V3DLONG i = 0;

    gpdInputImage = new double[pagesz_new];
    for(int i = 0; i <pagesz_new ;i++ )
        gpdInputImage[i] = 0;

    for(V3DLONG iz = hmirror; iz < P_new - hmirror; iz++)
    {
        V3DLONG offsetk = iz*M_new*N_new;
        for(V3DLONG iy = hmirror; iy < M_new - hmirror; iy++)
        {
            V3DLONG offsetj = iy*N_new;
            for(V3DLONG ix = hmirror; ix < N_new-hmirror; ix++)
            {
                gpdInputImage[offsetk + offsetj + ix] = (double)data1d[i]/255.0;
                i++;
            }
        }
    }

    for(V3DLONG iz = 0; iz < hmirror; iz++)
    {
        V3DLONG offsetk = iz*M_new*N_new;
        V3DLONG offsetk2 = (2*hmirror - iz)*M_new*N_new;
        for(V3DLONG iy = hmirror; iy < M_new - hmirror; iy++)
        {
            V3DLONG offsetj = iy*N_new;
            for(V3DLONG ix = hmirror; ix < N_new-hmirror; ix++)
            {
                gpdInputImage[offsetk + offsetj + ix] = gpdInputImage[offsetk2 + offsetj + ix];
                i++;
            }
        }
    }

    for(V3DLONG iz = P_new-hmirror; iz < P_new; iz++)
    {
        V3DLONG offsetk = iz*M_new*N_new;
        V3DLONG offsetk2 = (2*P_new-2*hmirror-2-iz)*M_new*N_new;
        for(V3DLONG iy = hmirror; iy < M_new - hmirror; iy++)
        {
            V3DLONG offsetj = iy*N_new;
            for(V3DLONG ix = hmirror; ix < N_new-hmirror; ix++)
            {
                gpdInputImage[offsetk + offsetj + ix] = gpdInputImage[offsetk2 + offsetj + ix];
                i++;
            }
        }
    }



    for(V3DLONG iz = hmirror; iz < P_new - hmirror; iz++)
    {
        V3DLONG offsetk = iz*M_new*N_new;
        for(V3DLONG iy = 0; iy < hmirror; iy++)
        {
            V3DLONG offsetj = iy*N_new;
            V3DLONG offsetj2 = (2*hmirror - iy)*N_new;
            for(V3DLONG ix = hmirror; ix < N_new-hmirror; ix++)
            {
                gpdInputImage[offsetk + offsetj + ix] = gpdInputImage[offsetk + offsetj2 + ix];
                i++;
            }
        }
    }


    for(V3DLONG iz = hmirror; iz < P_new - hmirror; iz++)
    {
        V3DLONG offsetk = iz*M_new*N_new;
        for(V3DLONG iy = M_new-hmirror; iy < M_new; iy++)
        {
            V3DLONG offsetj = iy*N_new;
            V3DLONG offsetj2 = (2*M_new-2*hmirror-2-iy)*N_new;
            for(V3DLONG ix = hmirror; ix < N_new-hmirror; ix++)
            {
                gpdInputImage[offsetk + offsetj + ix] = gpdInputImage[offsetk + offsetj2 + ix];
                i++;
            }
        }
    }


    for(V3DLONG iz = hmirror; iz < P_new - hmirror; iz++)
    {
        V3DLONG offsetk = iz*M_new*N_new;
        for(V3DLONG iy = hmirror; iy < M_new - hmirror; iy++)
        {
            V3DLONG offsetj = iy*N_new;
            for(V3DLONG ix = 0; ix < hmirror; ix++)
            {
                gpdInputImage[offsetk + offsetj + ix] = gpdInputImage[offsetk + offsetj + 2*hmirror - ix];
                i++;
            }
        }
    }

    for(V3DLONG iz = hmirror; iz < P_new - hmirror; iz++)
    {
        V3DLONG offsetk = iz*M_new*N_new;
        for(V3DLONG iy = hmirror; iy < M_new - hmirror; iy++)
        {
            V3DLONG offsetj = iy*N_new;
            for(V3DLONG ix = N_new - hmirror; ix < N_new; ix++)
            {
                gpdInputImage[offsetk + offsetj + ix] = gpdInputImage[offsetk + offsetj + 2*N_new-2*hmirror-2-ix];
                i++;
            }
        }
    }


    giNum_of_Dims_of_Input_Image = 3;
    gpiDims_InputImage[0] = N_new; gpiDims_InputImage[1] = M_new; gpiDims_InputImage[2] = P_new;
    giNum_of_pixels = pagesz_new;

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

    if(gpdInputImage) {delete []gpdInputImage; gpdInputImage = 0;}

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

    if(gpdEigenVals) {delete []gpdEigenVals; gpdEigenVals = 0;}
    int imsize[3] = {N,M,P};

    //tidyvaribales
    double prjs;
    int number_points = 0;
    int index_points[giNum_of_pixels];
    int flag;
    for (int iColIter = 0; iColIter < giNum_of_pixels ; iColIter ++)
    {
        flag = 0;
        for (int iRowIter = 0; iRowIter < giNum_of_Dims_of_Input_Image ; iRowIter ++)
        {
           prjs = gpdProjected_Points[ROWCOL(iRowIter, iColIter, giNum_of_Dims_of_Input_Image)]-hmirror;
           if(prjs > 0.5 && (prjs + 0.5) < imsize[iRowIter])
           {
               flag ++;
           }
        }
        if(flag ==3)
        {
            index_points[number_points] = iColIter;
            number_points++;
        }
    }

    double *gpdProjected_Points_updated = new double[number_points * giNum_of_Dims_of_Input_Image];
    double *tangentialSpace_updated = new double[number_points * giNum_of_Dims_of_Input_Image];
    i = 0;
    for (int index = 0; index < number_points ; index ++)
    {
        int iColIter = index_points[index];
        for (int iRowIter = 0; iRowIter < giNum_of_Dims_of_Input_Image ; iRowIter ++)
        {
            gpdProjected_Points_updated[i] = gpdProjected_Points[ROWCOL(iRowIter, iColIter, giNum_of_Dims_of_Input_Image)]-hmirror;
            tangentialSpace_updated[i] = gpdTangential_Space[ROWCOL(iRowIter, iColIter, giNum_of_Dims_of_Input_Image)];
            i++;
        }
    }
    printf("total number of useful points is %d, %d\n",number_points,iNumber_Of_Elements_in_ProjectedPoints);

    if(gpdTangential_Space) {delete []gpdTangential_Space; gpdTangential_Space=0;}
    if(gpdProjected_Points) {delete []gpdProjected_Points; gpdProjected_Points=0;}

    int k1 = 30;
    int N1 = 19;
    gpdInputTangDir = new double[giNum_of_Dims_of_Input_Image*number_points*k1*N1];
    gpdInputLocations = new double[giNum_of_Dims_of_Input_Image*number_points*k1*N1];
    double *W = new double[1*number_points*k1*N1];
    int *idx = new int[number_points*k1];

    preIntegral(gpdProjected_Points_updated,tangentialSpace_updated,N1,k1,number_points,giNum_of_Dims_of_Input_Image,gpdInputTangDir,gpdInputLocations,W,idx);

    giNum_of_locations = number_points*k1*N1;
    gpdScores = new double[giNum_of_locations];
    for(iIndex = 0; iIndex < giNum_of_locations; iIndex++)
    {
        gpdScores[iIndex] = 0;
    }

    for(iThreadNumber = 0; iThreadNumber < NUM_OF_THREADS_TO_CREATE; iThreadNumber++)
    {
        iRet_Val_Pthread_Create = pthread_create(&ptThreads[iThreadNumber], NULL, Thread_ProcessImage_score, (void*)iThreadNumber);
    }

    for(iThreadNumber = 0; iThreadNumber < NUM_OF_THREADS_TO_CREATE; iThreadNumber++)
    {
        pthread_join(ptThreads[iThreadNumber], NULL);
    }

    if(gpdInputTangDir) {delete []gpdInputTangDir; gpdInputTangDir = 0;}
    if(gpdInputLocations) {delete []gpdInputLocations; gpdInputLocations = 0;}

    double *gpdScores_w = new double[giNum_of_locations];
    for(V3DLONG i = 0; i<number_points*k1*N1; i++)
    {
          gpdScores_w[i] = gpdScores[i]*W[i];
     }

    double *D = new double[number_points*k1];
    double sum_w;
    for(V3DLONG i = 0; i < number_points*k1; i++)
    {
        sum_w = 0;
        for(int j = 0; j < N1; j++)
        {
            sum_w = sum_w + gpdScores_w[i*N1+j];
        }

        D[i] = sum_w;
    }


    double *DScore = new double[number_points*number_points];
    for(V3DLONG i = 0; i <number_points*number_points; i++ )
        DScore[i] = INFINITY;

    for(int ix = 0 ; ix < number_points; ix++ )
    {
        for(int iy = 0; iy < k1; iy ++)
        {
            int knn_index = idx[ix*k1 + iy];
            DScore[ix + knn_index*number_points] = D[ix*k1 + iy];
        }
    }

    if(idx) {delete []idx; idx = 0;}


    int indmin = 59; //root index
    double x1,y1,z1,x2,y2,z2;

    double *Xnew = new double[(number_points+1) * giNum_of_Dims_of_Input_Image];
    for(int i = 0; i < number_points+1;i++)
    {
        if(i == number_points)
        {
             x1 = gpdProjected_Points_updated[(ROWCOL(0, indmin, giNum_of_Dims_of_Input_Image))];
             y1 = gpdProjected_Points_updated[(ROWCOL(1, indmin, giNum_of_Dims_of_Input_Image))];
             z1 = gpdProjected_Points_updated[(ROWCOL(2, indmin, giNum_of_Dims_of_Input_Image))];

        }
        else
        {
             x1 = gpdProjected_Points_updated[(ROWCOL(0, i, giNum_of_Dims_of_Input_Image))];
             y1 = gpdProjected_Points_updated[(ROWCOL(1, i, giNum_of_Dims_of_Input_Image))];
             z1 = gpdProjected_Points_updated[(ROWCOL(2, i, giNum_of_Dims_of_Input_Image))];
        }

        Xnew[i*giNum_of_Dims_of_Input_Image+0] = x1;
        Xnew[i*giNum_of_Dims_of_Input_Image+1] = y1;
        Xnew[i*giNum_of_Dims_of_Input_Image+2] = z1;

    }

    double *DScore_wRoot = new double[(number_points+1)*(number_points+1)];
     for(V3DLONG i = 0; i < (number_points+1)*(number_points+1); i++)
     {
         DScore_wRoot[i] = INFINITY;
     }

    for(V3DLONG i = 0; i < number_points; i++)
    {
        for(V3DLONG j = 0; j < number_points; j++)
        {
            DScore_wRoot[i*(number_points+1) + j] = DScore[i*number_points + j];
        }
    }


    DScore_wRoot[number_points*(number_points+1) + number_points] = eps;
    DScore_wRoot[number_points*(number_points+1) + indmin] = eps;
    DScore_wRoot[indmin*(number_points+1) + number_points] = eps;

    double *Dsp_wRoot = new double[(number_points+1)*(number_points+1)];

    for(V3DLONG i = 0; i < number_points+1; i++)
    {
        for(V3DLONG j = i; j < number_points+1; j++)
        {
            double score1 = DScore_wRoot[i*(number_points+1) + j];
            double score2 = DScore_wRoot[j*(number_points+1) + i];
            if(score1 > score2)
            {
                Dsp_wRoot[i*(number_points+1) + j] = score1;
                Dsp_wRoot[j*(number_points+1) + i] = score1;
            }
            else
            {
                Dsp_wRoot[i*(number_points+1) + j] = score2;
                Dsp_wRoot[j*(number_points+1) + i] = score2;
            }

        }
    }

    double *DX = new double[(number_points+1)*(number_points+1)];
    for(int i = 0; i < number_points+1;i++)
    {
        if(i == number_points)
        {
             x1 = gpdProjected_Points_updated[(ROWCOL(0, indmin, giNum_of_Dims_of_Input_Image))];
             y1 = gpdProjected_Points_updated[(ROWCOL(1, indmin, giNum_of_Dims_of_Input_Image))];
             z1 = gpdProjected_Points_updated[(ROWCOL(2, indmin, giNum_of_Dims_of_Input_Image))];

        }
        else
        {
             x1 = gpdProjected_Points_updated[(ROWCOL(0, i, giNum_of_Dims_of_Input_Image))];
             y1 = gpdProjected_Points_updated[(ROWCOL(1, i, giNum_of_Dims_of_Input_Image))];
             z1 = gpdProjected_Points_updated[(ROWCOL(2, i, giNum_of_Dims_of_Input_Image))];
        }
        for(int j = 0; j < number_points+1; j++)
        {
            if(j == number_points)
            {
                 x2 = gpdProjected_Points_updated[(ROWCOL(0, indmin, giNum_of_Dims_of_Input_Image))];
                 y2 = gpdProjected_Points_updated[(ROWCOL(1, indmin, giNum_of_Dims_of_Input_Image))];
                 z2 = gpdProjected_Points_updated[(ROWCOL(2, indmin, giNum_of_Dims_of_Input_Image))];
            }
            else
            {
                 x2 = gpdProjected_Points_updated[(ROWCOL(0, j, giNum_of_Dims_of_Input_Image))];
                 y2 = gpdProjected_Points_updated[(ROWCOL(1, j, giNum_of_Dims_of_Input_Image))];
                 z2 = gpdProjected_Points_updated[(ROWCOL(2, j, giNum_of_Dims_of_Input_Image))];
            }
            double dist_1D = sqrt(pow(x1-x2,2.0)+pow(y1-y2,2.0) + pow (z1-z2,2.0));
            DX[i*(number_points+1) + j] = dist_1D;
            DX[j*(number_points+1) + i] = dist_1D;
        }
    }

    double *Dsp_w = new double[(number_points+1)*(number_points+1)];
    double *DX_T_man = new double[(number_points+1)*(number_points+1)];
    for(V3DLONG i = 0; i<(number_points+1)*(number_points+1) ;i++)
    {
        if(Dsp_wRoot[i] < 1.0)
        {
            Dsp_w[i] = DX[i];
            DX_T_man[i] = Dsp_wRoot[i];
        }
        else
        {
           Dsp_w[i] = 0;
           DX_T_man[i] = INFINITY;
        }
    }


    double *D_euc = new double[(number_points+1)*(number_points+1)];

    graph_all_shortest_paths(Dsp_w, D_euc,number_points+1);

    if(gpdScores) {delete []gpdScores; gpdScores = 0;}
    if(W) {delete []W; W = 0;}
    if(Dsp_w) {delete []Dsp_w; Dsp_w = 0;}
    if(D) {delete []D; D = 0;}
    if(DX) {delete []DX; DX = 0;}
    if(Dsp_wRoot) {delete []Dsp_wRoot; Dsp_wRoot = 0;}
    if(DScore_wRoot) {delete []DScore_wRoot; DScore_wRoot = 0;}
    if(DScore) {delete []DScore; DScore = 0;}
    if(gpdScores_w) {delete []gpdScores_w; gpdScores_w = 0;}
    if(gpdProjected_Points_updated) {delete []gpdProjected_Points_updated; gpdProjected_Points_updated=0;}

    QString outswc_file = QString(p4DImage->getFileName()) + "_PSF.swc";
    extractTree(D_euc,DX_T_man,Xnew,number_points+1,outswc_file);


    if(D_euc) {delete []D_euc; D_euc = 0;}
    if(DX_T_man) {delete []DX_T_man; DX_T_man = 0;}
    if(Xnew) {delete []Xnew; Xnew = 0;}


    free (gpdWeights_InputImage);
    free (gpdEigVec_Cov_InputImage);
    free (gpdEigVal_Cov_InputImage);
    free (gpdNormP_EigVal);
    free (gpiData);

    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(outswc_file));
    return;

}

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

void preIntegral(double *gpdProjected_Points_updated,double *tangentialSpace_updated,int N,int k,int number_points,int giNum_of_Dims_of_Input_Image, double *T, double *Y, double *W, int *idx)
{
    double w[19] = {0.0139,0.0694,0.0694,0.0278,0.0694,0.0694,0.0278,0.0694,0.0694,0.0278,0.0694,0.0694,0.0278,0.0694,0.0694,0.0278,0.0694,0.0694,0.0139};
    double x[19] = {0,0.0461,0.1206,0.1667,0.2127,0.2873,0.3333,0.3794,0.4539,0.5000,0.5461,0.6206,0.6667,0.7127,0.7873,0.8333,0.8794,0.9539,1.0000};

    double dist_1D[number_points];
    int index_ID[number_points];
    int hmirror = 7;
    for(int i = 0; i < number_points;i++)
    {
        double x1 = gpdProjected_Points_updated[(ROWCOL(0, i, giNum_of_Dims_of_Input_Image))];
        double y1 = gpdProjected_Points_updated[(ROWCOL(1, i, giNum_of_Dims_of_Input_Image))];
        double z1 = gpdProjected_Points_updated[(ROWCOL(2, i, giNum_of_Dims_of_Input_Image))];
        for(int j = 0; j < number_points; j++)
        {
            double x2 = gpdProjected_Points_updated[(ROWCOL(0, j, giNum_of_Dims_of_Input_Image))];
            double y2 = gpdProjected_Points_updated[(ROWCOL(1, j, giNum_of_Dims_of_Input_Image))];
            double z2 = gpdProjected_Points_updated[(ROWCOL(2, j, giNum_of_Dims_of_Input_Image))];
            dist_1D[j] = sqrt(pow(x1-x2,2.0)+pow(y1-y2,2.0) + pow (z1-z2,2.0));
            index_ID[j] = j;
        }

        for (int c = 0 ; c < ( number_points - 1 ); c++)
        {
            for (int d = 0 ; d < number_points - c - 1; d++)
            {
              if (dist_1D[d] > dist_1D[d+1])
              {
                double swap       = dist_1D[d];
                dist_1D[d]   = dist_1D[d+1];
                dist_1D[d+1] = swap;

                int tmp = index_ID[d];
                index_ID[d] = index_ID[d+1];
                index_ID[d+1] = tmp;
              }
            }
        }

         for (int ii = 0; ii < k; ii++)
             idx[i*k + ii] = index_ID[ii];
     }

    for(int ix = 0; ix < number_points; ix++)
    {
        V3DLONG offsetx = ix*N*k;
        for(int iy = 0; iy < k; iy ++ )
        {
            V3DLONG offsety = iy*N;
            for(int iz = 0; iz < N; iz++)
            {
                W[offsetx+offsety+iz] = w[iz];
                for(int id = 0; id < giNum_of_Dims_of_Input_Image;id++)
                {
                    T[(offsetx+offsety+iz)*giNum_of_Dims_of_Input_Image + id] = tangentialSpace_updated[ROWCOL(id, ix, giNum_of_Dims_of_Input_Image)];
                    double ref_x = gpdProjected_Points_updated[(ROWCOL(id, ix, giNum_of_Dims_of_Input_Image))]+ hmirror - 1;
                    double neig_x = gpdProjected_Points_updated[(ROWCOL(id,idx[ix*k + iy], giNum_of_Dims_of_Input_Image))]+ hmirror - 1;
                    Y[(offsetx+offsety+iz)*giNum_of_Dims_of_Input_Image + id] = (neig_x-ref_x)*x[iz] + ref_x;

                }
            }
        }

    }
   return;

}

void  graph_all_shortest_paths(double *inputMatrix, double *disMatrix, int n)
{

    double dist[n];

    for(int i = 0; i < n; i++)
    {

       dijkstra(inputMatrix,n,i,dist);
       for(int j = 0; j < n; j++)
           disMatrix[j + n*i] = dist[j];
    }

    return;

}

void dijkstra(double *graph, int V, int src, double *dist)
{
//     double dist[V];     // The output array.  dist[i] will hold the shortest
                      // distance from src to i

     bool sptSet[V]; // sptSet[i] will true if vertex i is included in shortest
                     // path tree or shortest distance from src to i is finalized

     // Initialize all distances as INFINITE and stpSet[] as false
     for (int i = 0; i < V; i++)
        dist[i] = INT_MAX, sptSet[i] = false;

     // Distance of source vertex from itself is always 0
     dist[src] = 0;

     // Find shortest path for all vertices
     for (int count = 0; count < V-1; count++)
     {
       // Pick the minimum distance vertex from the set of vertices not
       // yet processed. u is always equal to src in first iteration.
       int u = minDistance(dist, sptSet,V);

       // Mark the picked vertex as processed
       sptSet[u] = true;

       // Update dist value of the adjacent vertices of the picked vertex.
       for (int v = 0; v < V; v++)
       {

         // Update dist[v] only if is not in sptSet, there is an edge from
         // u to v, and total weight of path from src to  v through u is
         // smaller than current value of dist[v]
         if (!sptSet[v] && graph[v*V+u] && dist[u] != INT_MAX
                                       && dist[u]+graph[v*V+u] < dist[v])
            dist[v] = dist[u] + graph[v*V+u];
       }
     }

     // print the constructed distance array
}
int minDistance(double dist[], bool sptSet[],int V)
{
   // Initialize min value
   double min = INT_MAX;
   int min_index;

   for (int v = 0; v < V; v++)
     if (sptSet[v] == false && dist[v] <= min)
         min = dist[v], min_index = v;

   return min_index;
}
