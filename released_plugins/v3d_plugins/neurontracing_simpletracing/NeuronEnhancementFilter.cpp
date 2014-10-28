/*
 *   NeuronEnhancementFilter.cpp: implementation of the NeuronEnhancementFilter class.
 *
 *  Created by Yang, Jinzhu, on 08/22/11.
 * adjusted by Hanchuan Peng, 03/19/2012
 *
 */
//////////////////////////////////////////////////////////////////////

#include <math.h>
#include "NeuronEnhancementFilter.h"
#include <algorithm>
#include <functional>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


NeuronEnhancementFilter::NeuronEnhancementFilter()
{
}

NeuronEnhancementFilter::~NeuronEnhancementFilter()
{

}


int NeuronEnhancementFilter::MultiScaleFilter3D(short *apsImage, float *apfFiltedImg, 
                                                int iColNmb, int iRowNmb, int iSliceNmb, 
                                                int *apiSigma, int iSigmaLength,
                                                float fA, float fB, float fC)
{
	long iElements = long(iColNmb)*long(iRowNmb)*long(iSliceNmb);
	long iElementsNew = long(iSliceNmb+2)*long(iColNmb+2)*long(iRowNmb+2);	
    
	int iSig;
	int iSize = 5;

    short *apfVesselTmp = 0, *apfVess = 0;
	for( int i=0; i<iSigmaLength; i++)
	{
		iSig = apiSigma[i] ;		
		//float apfGausstemplate[5] = {0.13206726, 0.35899603, 0.35899603, 0.13206726, 0.017873362}; //is this correct?
        float apfGausstemplate[5] = {0.1353,    0.6065,    1.0000,    0.6065,    0.1353}; //maybe should be this? based on exp(-x^2/(2*sigma^2)). by PHC, 20120327
        
		try
        {
            apfVesselTmp = new short[iElements];
            apfVess = new short[iElementsNew];
        }
        catch(...)
        {
            return -1;
        }

		ConvolThreeDimension(apsImage, apfVesselTmp, apfGausstemplate,iRowNmb, iColNmb, iSliceNmb,iSize);
        
		Expand3DBoundary(apfVesselTmp, iSliceNmb, iRowNmb, iColNmb, apfVess);
		if (apfVesselTmp) {delete []apfVesselTmp; apfVesselTmp = NULL;}
        
		memset(apsImage, 0, sizeof(short)*iElements);
		
		int iRet = Calculatelness(apfVess, apsImage, iSliceNmb+2, iColNmb+2, iRowNmb+2, fA, fB, fC);
		if (apfVess) { delete []apfVess; apfVess = NULL;}
		
		if (iRet==-1)
			return -1;
	}
}


void NeuronEnhancementFilter::ConvolThreeDimension(short *apsImage, short *apfFiltedImg, float *apdTemplate, 
						 int iRowNumber, int iColumnNumber, int iSliceNumber, int iConvLen)
{
	int iPad = iConvLen/2;
	double fSum = 0.0;

	int s;
	int r;
	int c;
	int t;
	int i;

	int iCnt = iSliceNumber*iColumnNumber*iRowNumber;
	
	memcpy(apfFiltedImg, apsImage, sizeof(short)*iCnt);


	for (s=0; s<iSliceNumber; s++)
	{
		for(r=iPad; r<iRowNumber-iPad; r++) //row
		{
			for(c=0; c<iColumnNumber; c++) //column	
			{
				fSum = .0;
				for(t=0; t<iConvLen; t++)
				{
					 fSum += double(apsImage[s*iRowNumber*iColumnNumber+(r-iPad+t)*iColumnNumber+c])*apdTemplate[t];
				}
				apfFiltedImg[s*iRowNumber*iColumnNumber + r*iColumnNumber + c] = fSum;
			}
		}
	}


	for (s=0; s<iSliceNumber; s++)
	{
		for(r=0; r<iRowNumber; r++) //row
		{
			for(c=iPad; c<iColumnNumber-iPad; c++) //column	
			{
				fSum = .0;
				for(t=-iPad; t<=iPad; t++)
				{
					 fSum += double(apfFiltedImg[s*iRowNumber*iColumnNumber+r*iColumnNumber+c+t])*apdTemplate[t+iPad];
				}
				apsImage[s*iRowNumber*iColumnNumber + r*iColumnNumber + c] = fSum;
			}
		}
	}

	for (s=iPad; s<iSliceNumber-iPad; s++)
	{
		for(r=0; r<iRowNumber; r++) //row
		{
			for(c=0; c<iColumnNumber; c++) //column	
			{
				fSum = .0;
				for(t=-iPad; t<=iPad; t++)
				{
					 fSum += double(apsImage[(s+t)*iRowNumber*iColumnNumber+r*iColumnNumber+c])*apdTemplate[t+iPad];
				}
				apfFiltedImg[s*iRowNumber*iColumnNumber + r*iColumnNumber + c] = fSum;
			}
		}
	}
    
    return;
}

//
void NeuronEnhancementFilter::Expand3DBoundary(short *apsImg, int iLayer, int iRow, int iColumn, short *apsImgNew)
{
	int i;
	int j;
	int k;

	for(i=0; i<iLayer; i++)
	{
		for(j=0; j<iRow; j++)
		{
			for(k=0; k<iColumn;k++)
			{
				apsImgNew[(i+1)*(iRow+2)*(iColumn+2)+(j+1)*(iColumn+2)+k+1] = apsImg[i*iRow*iColumn+j*iColumn+k];
			}
		}
	}
	

	for(i=0; i<iLayer; i++)
	{
		for(j=0; j<iRow; j++)
		{
			apsImgNew[(i+1)*(iRow+2)*(iColumn+2)+(j+1)*(iColumn+2)] = apsImg[i*iRow*iColumn+j*iColumn];
		}
	}


	for(i=0; i<iLayer; i++)
	{
		for(j=0; j<iRow; j++)
		{
			apsImgNew[(i+1)*(iRow+2)*(iColumn+2)+(j+1)*(iColumn+2)+iColumn+1] = 
				apsImg[i*iRow*iColumn+j*iColumn+iColumn-1];
		}
	}

	for(i=1; i<iLayer+1; i++)
	{
		for(k=0; k<iColumn+2;k++)
		{
			apsImgNew[i*(iRow+2)*(iColumn+2)+k] = 
				apsImgNew[i*(iRow+2)*(iColumn+2)+(iColumn+2)+k];
		}
	}
	
	for(i=1; i<iLayer+1; i++)
	{
		for(k=0; k<iColumn+2;k++)
		{
			apsImgNew[i*(iRow+2)*(iColumn+2)+(iRow+1)*(iColumn+2)+k] = 
				apsImgNew[i*(iRow+2)*(iColumn+2)+iRow*(iColumn+2)+k];
		}
	}
	
	for(j=0; j<iRow+2; j++)
	{
		for(k=0; k<iColumn+2; k++)
		{
			apsImgNew[j*(iColumn+2)+k] = 
				apsImgNew[(iColumn+2)*(iRow+2)+j*(iColumn+2)+k];
		}
	}

	for(j=0; j<iRow+2; j++)
	{
		for(k=0; k<iColumn+2; k++)
		{
			apsImgNew[(iLayer+1)*(iRow+2)*(iColumn+2)+j*(iColumn+2)+k] = 
				apsImgNew[iLayer*(iRow+2)*(iColumn+2)+j*(iColumn+2)+k];
		}
	}

}


int NeuronEnhancementFilter::Calculatelness(short *apfVess, short *apfFiltedImg, 					
						int iSliceNmb, int iColNmb, int iRowNmb, float fA, float fB, float fC)
{
	int i;
	int j;
	int k;
	int l;
	
	double fGrdXX;
	double fGrdXY;
	double fGrdXZ;
	double fGrdYY;
	double fGrdYZ;
	double fGrdZZ;

	double fAbsLambda1;
	double fAbsLambda2;
	double fAbsLambda3;

	double fRa;
	double fRb;
	double fS;
	double fExpRa;
	double fExpRb;
	double fExpS;
	double fTmpval;

//    float fA = 0.5;
//    float fB = 0.5;
//    float fC = 2.0*20*20;
	
	int output_dims[2]={1, 3};
	float Ma[3][3];
	float Davec[3][3];
	float Daeig[3];	
	
	int iElementsNew = iSliceNmb*iRowNmb*iColNmb;
	
    short *apfDx = 0, *apfDy = 0, *apfDz = 0;
    try 
    {
        apfDx = new short[iElementsNew];
        apfDy = new short[iElementsNew];
        apfDz = new short[iElementsNew];
    }
    catch (...) 
    {
        printf("Fail to allocate memory NeuronEnhancementFilter().\n");
        return -1;
    }
	
	memset(apfDx, 0, sizeof(short)*iElementsNew);
	memset(apfDy, 0, sizeof(short)*iElementsNew);
	memset(apfDz, 0, sizeof(short)*iElementsNew);

	for (i=1; i<iSliceNmb-1; i++)       
	{
		for (j=1; j<iRowNmb-1; j++)
		{
			for (k=1; k<iColNmb-1; k++)
			{
                long curpos = long(i)*long(iRowNmb)*long(iColNmb)+long(j)*long(iColNmb)+long(k);
				apfDx[curpos] = (apfVess[i*iRowNmb*iColNmb+(j+1)*iColNmb+k]-apfVess[i*iRowNmb*iColNmb+(j-1)*iColNmb+k])/2;
				apfDy[curpos] = (apfVess[i*iRowNmb*iColNmb+j*iColNmb+k+1]-apfVess[i*iRowNmb*iColNmb+j*iColNmb+k-1])/2;
				apfDz[curpos] = (apfVess[(i+1)*iRowNmb*iColNmb+j*iColNmb+k]-apfVess[(i-1)*iRowNmb*iColNmb+j*iColNmb+k])/2;
			}
		}
	}
	
	for (i=1; i<iSliceNmb-1; i++)       
	{
		for (j=1; j<iRowNmb-1; j++)
		{
			for (k=1; k<iColNmb-1; k++)
			{
				fGrdXX = (apfDx[i*iRowNmb*iColNmb+(j+1)*iColNmb+k]-apfDx[i*iRowNmb*iColNmb+(j-1)*iColNmb+k])/2;
				fGrdXY = (apfDx[i*iRowNmb*iColNmb+j*iColNmb+k+1]-apfDx[i*iRowNmb*iColNmb+j*iColNmb+k-1])/2;
				fGrdXZ = (apfDx[(i+1)*iRowNmb*iColNmb+j*iColNmb+k]-apfDx[(i-1)*iRowNmb*iColNmb+j*iColNmb+k])/2;
				fGrdYY = (apfDy[i*iRowNmb*iColNmb+j*iColNmb+k+1]-apfDy[i*iRowNmb*iColNmb+j*iColNmb+k-1])/2;
				fGrdYZ = (apfDy[(i+1)*iRowNmb*iColNmb+j*iColNmb+k]-apfDy[(i-1)*iRowNmb*iColNmb+j*iColNmb+k])/2;
				fGrdZZ = (apfDz[(i+1)*iRowNmb*iColNmb+j*iColNmb+k]-apfDz[(i-1)*iRowNmb*iColNmb+j*iColNmb+k])/2;
			
				Ma[0][0] = fGrdXX;
				Ma[0][1] = fGrdXY; 
				Ma[0][2] = fGrdXZ;
				Ma[1][0] = fGrdXY; 
				Ma[1][1] = fGrdYY; 
				Ma[1][2] = fGrdYZ;
				Ma[2][0] = fGrdXZ; 
				Ma[2][1] = fGrdYZ; 
				Ma[2][2] = fGrdZZ;
				
				eigen_decomposition(Ma, Davec, Daeig);
             //   printf("d0=%lf d1=%lf d2=%lf\n",Daeig[0],Daeig[1],Daeig[2]);
/*
		        Dvecx_f[i]=(float)Davec[0][0]; 
		        Dvecy_f[i]=(float)Davec[0][1]; 
		        Dvecz_f[i]=(float)Davec[0][2];*/
		        
				l = (i-1)*(iRowNmb-2)*(iColNmb-2)+(j-1)*(iColNmb-2)+(k-1);     

				if ((Daeig[1]>0) || 
                    (Daeig[2]>0))
				{
					apfFiltedImg[l] = 0;
				}
                else if(((Daeig[1]/Daeig[2])>0.9) &&
                        ((Daeig[1]/Daeig[2])<1.1))
				{
					apfFiltedImg[l] = 0;
				}
				else
				{
					fAbsLambda1 = fabs(Daeig[0]);
					fAbsLambda2 = fabs(Daeig[1]);
					fAbsLambda3 = fabs(Daeig[2]);

					//The lness Features
					if(fAbsLambda3==0)
					{
						fAbsLambda3 = 1.0e-4;
					}
					fRa = fAbsLambda2 / fAbsLambda3;
						
					fTmpval = fAbsLambda2*fAbsLambda3;
					if (fTmpval==0)
					{
						fTmpval = 1.0e-4;
					}
					fRb = fAbsLambda1 / sqrt(fTmpval);
					fS = sqrt(fAbsLambda1*fAbsLambda1 + fAbsLambda2*fAbsLambda2 + fAbsLambda3*fAbsLambda3);
			
					//Compute lness function
					fExpRa = (1-exp(-(fRa*fRa/fA)));
					fExpRb = exp(-(fRb*fRb/fB));
					fExpS  = (1-exp(-fS*fS/fC));
					if (i>0)  //what this means??
					{
						fTmpval = fExpRa * fExpRb * fExpS;
						//printf("fa=%lf fb=%lf fs=%lf ftmpval=%lf\n",fExpRa,fExpRb,fExpS,fTmpval);
						
						if (fTmpval > apfFiltedImg[l])
						{
							apfFiltedImg[l] = fTmpval*4095;//4096;
						}
					}
					else //i==0
					{
						apfFiltedImg[l] = fExpRa * fExpRb * fExpS*4095;//4096;
					}				
				}			

			}//for k
		}//for j
	}// for i

	if (apfDz) {delete []apfDz;	apfDz = NULL;}
	if (apfDx) {delete []apfDx; apfDx = NULL;}
	if (apfDy) {delete []apfDy;	apfDy = NULL;}

	return 1;
}


void  NeuronEnhancementFilter::CalEigenValue( float *Dxx_f, float *Dxy_f, float *Dxz_f, float *Dyy_f, float *Dyz_f, float *Dzz_f,
				   /* float *Dvecx_f, float *Dvecy_f, float *Dvecz_f, */float *Deiga_f, float *Deigb_f, float *Deigc_f, 
				    int npixels) 
{    
    
    int output_dims[2]={1, 3};
    float Ma[3][3];
    float Davec[3][3];
    float Daeig[3];
    
    // Loop variable
    int i;    
    
    for(i=0; i<npixels; i++) {
        Ma[0][0]=(float)Dxx_f[i]; Ma[0][1]=(float)Dxy_f[i]; Ma[0][2]=(float)Dxz_f[i];
        Ma[1][0]=(float)Dxy_f[i]; Ma[1][1]=(float)Dyy_f[i]; Ma[1][2]=(float)Dyz_f[i];
        Ma[2][0]=(float)Dxz_f[i]; Ma[2][1]=(float)Dyz_f[i]; Ma[2][2]=(float)Dzz_f[i];
        eigen_decomposition(Ma, Davec, Daeig);
        Deiga_f[i]=(float)Daeig[0]; 
        Deigb_f[i]=(float)Daeig[1]; 
        Deigc_f[i]=(float)Daeig[2];
	
        /* Main direction (smallest eigenvector) */
//        Dvecx_f[i]=(float)Davec[0][0]; 
//        Dvecy_f[i]=(float)Davec[0][1]; 
//        Dvecz_f[i]=(float)Davec[0][2];

    }
}



#define MAX(a, b) ((a)>(b)?(a):(b))


float  NeuronEnhancementFilter::absd(float val)
{
	if(val>0)
	{ 
		return val;
	} 
	else
	{ 
		return -val;
	}
}

double  NeuronEnhancementFilter::hypot2(double x, double y)
{
	return sqrt(x*x+y*y); 
}

void NeuronEnhancementFilter::tred2(float V[3][3], float d[3], float e[3]) 
{
    int n = 3;
//  This is derived from the Algol procedures tred2 by
//  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for
//  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding
//  Fortran subroutine in EISPACK.
    int i, j, k;
    float scale;
    float f, g, h;
    float hh;
    for (j = 0; j < n; j++) {d[j] = V[n-1][j]; }
    
    // Householder reduction to tridiagonal form.
    
    for (i = n-1; i > 0; i--) {
        // Scale to avoid under/overflow.
        scale = 0.0;
        h = 0.0;
        for (k = 0; k < i; k++) { scale = scale + fabs(d[k]); }
        if (scale == 0.0) {
            e[i] = d[i-1];
            for (j = 0; j < i; j++) { d[j] = V[i-1][j]; V[i][j] = 0.0;  V[j][i] = 0.0; }
        } else {
            
            // Generate Householder vector.
            
            for (k = 0; k < i; k++) { d[k] /= scale; h += d[k] * d[k]; }
            f = d[i-1];
            g = sqrt(h);
            if (f > 0) { g = -g; }
            e[i] = scale * g;
            h = h - f * g;
            d[i-1] = f - g;
            for (j = 0; j < i; j++) { e[j] = 0.0; }
            
            // Apply similarity transformation to remaining columns.
            
            for (j = 0; j < i; j++) {
                f = d[j];
                V[j][i] = f;
                g = e[j] + V[j][j] * f;
                for (k = j+1; k <= i-1; k++) { g += V[k][j] * d[k]; e[k] += V[k][j] * f; }
                e[j] = g;
            }
            f = 0.0;
            for (j = 0; j < i; j++) { e[j] /= h; f += e[j] * d[j]; }
            hh = f / (h + h);
            for (j = 0; j < i; j++) { e[j] -= hh * d[j]; }
            for (j = 0; j < i; j++) {
                f = d[j]; g = e[j];
                for (k = j; k <= i-1; k++) { V[k][j] -= (f * e[k] + g * d[k]); }
                d[j] = V[i-1][j];
                V[i][j] = 0.0;
            }
        }
        d[i] = h;
    }
    
    // Accumulate transformations.
    
    for (i = 0; i < n-1; i++) {
        V[n-1][i] = V[i][i];
        V[i][i] = 1.0;
        h = d[i+1];
        if (h != 0.0) {
            for (k = 0; k <= i; k++) { d[k] = V[k][i+1] / h;}
            for (j = 0; j <= i; j++) {
                g = 0.0;
                for (k = 0; k <= i; k++) { g += V[k][i+1] * V[k][j]; }
                for (k = 0; k <= i; k++) { V[k][j] -= g * d[k]; }
            }
        }
        for (k = 0; k <= i; k++) { V[k][i+1] = 0.0;}
    }
    for (j = 0; j < n; j++) { d[j] = V[n-1][j]; V[n-1][j] = 0.0; }
    V[n-1][n-1] = 1.0;
    e[0] = 0.0;
}

// Symmetric tridiagonal QL algorithm.
void  NeuronEnhancementFilter::tql2(float V[3][3], float d[3], float e[3])
{
    
//  This is derived from the Algol procedures tql2, by
//  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for
//  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding
//  Fortran subroutine in EISPACK.
    int n = 3;
    int i, j, k, l, m;
    float f;
    float tst1;
    float eps;
    int iter;
    float g, p, r;
    float dl1, h, c, c2, c3, el1, s, s2;
    
    for (i = 1; i < n; i++) { e[i-1] = e[i]; }
    e[n-1] = 0.0;
    
    f = 0.0;
    tst1 = 0.0;
    eps = pow(2.0, -52.0);
    for (l = 0; l < n; l++) {
        
        // Find small subdiagonal element
        
        tst1 = MAX(tst1, fabs(d[l]) + fabs(e[l]));
        m = l;
        while (m < n) {
            if (fabs(e[m]) <= eps*tst1) { break; }
            m++;
        }
        
        // If m == l, d[l] is an eigenvalue,
        // otherwise, iterate.
        
        if (m > l) {
            iter = 0;
            do {
                iter = iter + 1;  // (Could check iteration count here.)
                // Compute implicit shift
                g = d[l];
                p = (d[l+1] - g) / (2.0 * e[l]);
                r = hypot2(p, 1.0);
                if (p < 0) { r = -r; }
                d[l] = e[l] / (p + r);
                d[l+1] = e[l] * (p + r);
                dl1 = d[l+1];
                h = g - d[l];
                for (i = l+2; i < n; i++) { d[i] -= h; }
                f = f + h;
                // Implicit QL transformation.
                p = d[m]; c = 1.0; c2 = c; c3 = c;
                el1 = e[l+1]; s = 0.0; s2 = 0.0;
                for (i = m-1; i >= l; i--) {
                    c3 = c2;
                    c2 = c;
                    s2 = s;
                    g = c * e[i];
                    h = c * p;
                    r = hypot2(p, e[i]);
                    e[i+1] = s * r;
                    s = e[i] / r;
                    c = p / r;
                    p = c * d[i] - s * g;
                    d[i+1] = h + s * (c * g + s * d[i]);
                    // Accumulate transformation.
                    for (k = 0; k < n; k++) {
                        h = V[k][i+1];
                        V[k][i+1] = s * V[k][i] + c * h;
                        V[k][i] = c * V[k][i] - s * h;
                    }
                }
                p = -s * s2 * c3 * el1 * e[l] / dl1;
                e[l] = s * p;
                d[l] = c * p;
                
                // Check for convergence.
            } while (fabs(e[l]) > eps*tst1);
        }
        d[l] = d[l] + f;
        e[l] = 0.0;
    }
    
    // Sort eigenvalues and corresponding vectors.
    for (i = 0; i < n-1; i++) {
        k = i;
        p = d[i];
        for (j = i+1; j < n; j++) {
            if (d[j] < p) {
                k = j;
                p = d[j];
            }
        }
        if (k != i) {
            d[k] = d[i];
            d[i] = p;
            for (j = 0; j < n; j++) {
                p = V[j][i];
                V[j][i] = V[j][k];
                V[j][k] = p;
            }
        }
    }
}

void  NeuronEnhancementFilter::eigen_decomposition(float A[3][3], float V[3][3], float d[3])
{
	int n = 3;
    float e[3];
    float da[3];
    float dt, dat;
    float vet[3];
    int i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            V[i][j] = A[i][j];
        }
    }
    tred2(V, d, e);
    tql2(V, d, e);
    
    /* Sort the eigen values and vectors by abs eigen value */
    da[0]=absd(d[0]); 
	da[1]=absd(d[1]);
	da[2]=absd(d[2]);
    if((da[0]>da[1])&&(da[0]>da[2]))
    {
        dt=d[2];   dat=da[2];   vet[0]=V[2][0];    vet[1]=V[2][1];    vet[2]=V[2][2];
        d[2]=d[0]; da[2]=da[0]; V[2][0] = V[0][0]; V[2][1] = V[0][1]; V[2][2] = V[0][2];
        d[0]=dt;   da[0]=dat;   V[0][0] = vet[0];  V[0][1] = vet[1];  V[0][2] = vet[2]; 
    }
    else if((da[1]>da[0])&&(da[1]>da[2]))  
    {
        dt=d[2];   dat=da[2];   vet[0]=V[2][0];    vet[1]=V[2][1];    vet[2]=V[2][2];
        d[2]=d[1]; da[2]=da[1]; V[2][0] = V[1][0]; V[2][1] = V[1][1]; V[2][2] = V[1][2];
        d[1]=dt;   da[1]=dat;   V[1][0] = vet[0];  V[1][1] = vet[1];  V[1][2] = vet[2]; 
    }
    if(da[0]>da[1])
    {
        dt=d[1];   dat=da[1];   vet[0]=V[1][0];    vet[1]=V[1][1];    vet[2]=V[1][2];
        d[1]=d[0]; da[1]=da[0]; V[1][0] = V[0][0]; V[1][1] = V[0][1]; V[1][2] = V[0][2];
        d[0]=dt;   da[0]=dat;   V[0][0] = vet[0];  V[0][1] = vet[1];  V[0][2] = vet[2]; 
    }

    
}



