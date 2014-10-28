/*
 *   NeuronSeg.cpp: implementation of the NeuronSeg class.
 *
 *  Created by Yang, Jinzhu, on 08/22/11.
 *  Adjusted by Hanchuan Peng, 03/19/2012
 *
 */
//////////////////////////////////////////////////////////////////////

#include "NeuronSegmentation.h"
#include <algorithm>
#include <functional>
#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

NeuronSeg::NeuronSeg()
{
}

NeuronSeg::~NeuronSeg()
{
}


void NeuronSeg::Neuron_Segmentation(short *apfImg, int *iSigma, 
								int sy, int sx, int sz,
                                float fA, float fB, float fC, //line-likehood setting
								int iSigmaLen, short* apsVesMark, 
                                int iDelSize,   //volume filter setting
                                int &iVesCnt, 	//the count 
								bool bBinaryPro, //if set as true, then do binarization
								bool bVolFilter //if set to true, then run a volume filter to remove small pieces
                                )
{		
	static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
	static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};
	static int nDz[] = {-1,0,1};
	
	int x,y,z,index;
	
	int imagesize = sy*sx;
	int i,j,k;

	long iElements = long(sx)*long(sy)*long(sz);

	short max = 0;
	for(i=1; i<iElements; i++)
	{
		if(apfImg[i]>max)
		{
			max = apfImg[i];
		}
	}

	printf("sy=%ld sx=%ld sz=%ld fa=%lf fb=%lf fc=%lf\n",sy,sx,sz,fA,fB,fC);

	NeuronEnhancementFilter m_VesEnhanFilter;
	m_VesEnhanFilter.MultiScaleFilter3D(apfImg, NULL, sx, sy, sz, iSigma, iSigmaLen, fA, fB, fC);

	short fMax = apfImg[0];
	short fMin = apfImg[0];
	for(i=1; i<iElements; i++)
	{
		if (apfImg[i]>fMax)
			fMax = apfImg[i];
		else if(apfImg[i]<fMin)
			fMin = apfImg[i];
	}

    if (fMax!=fMin)
    {
        printf("fmin=%5.4f fmax=%5.4f\n", fMin, fMax);
        double fMadMi = 4095.0/(double(fMax) - fMin);
        if (!bBinaryPro)
            double fMadMi = 255.0/(double(fMax) - fMin);

        for(i=0; i<iElements; i++)
        {
            apfImg[i]  = fMadMi * (apfImg[i]-fMin); //rescale between 0 and 4095/255
        }
    }
    else
    {
        for(i=0; i<iElements; i++)
            apfImg[i] = 0;
    }

	printf("enhancement completed\n");
    if (!bBinaryPro)
        return;

    //binarization
    
	if (bBinaryPro) 
	{
		short *apsTmpImg = new short[iElements];
		memset(apsTmpImg, 0, iElements*sizeof(short));
		BinaryProcess(apfImg, sy, sx, sz, apsTmpImg);

		for( i=0; i<iElements; i++)
		{
			apfImg[i] = apsTmpImg[i];
		}

		delete []apsTmpImg;
		apsTmpImg = NULL;

        printf("binarization completed\n");
	}

	if (bVolFilter)
	{
		VoiSelect(apfImg, NULL, sz, sy, sx, apsVesMark, iDelSize, iVesCnt);
	}
}



void NeuronSeg::VoiSelect(short* apsInputData, short*apsImg, int iImageLayer, int iImageHeight,
												int iImageWidth, short* apsVesMark, int iDelSize, int &iVesCnt)
{
	long i,j,k;

	long iTotal = long(iImageLayer)*long(iImageHeight)*long(iImageWidth);
	short* InputCopy = new short[iTotal];
	memcpy(InputCopy, apsInputData, sizeof(short)*iTotal);

	short *apsMask = new short[iTotal];
    for (i=0;i<iTotal; i++)
        apsMask[i] = -1;

	deque<Point3D_t> pointSetVOI;
	deque<Point3D_t> pointSetTemp;
	deque<Point3D_t> pointSetTemp2;
	Point3D_t myPoint;

	vector<int> vx;
	vector<int> vy;
	vector<int> vz;

	int counter = 0;
	int counter1 = 0;

	int a=0, b=0, c=0, e=0, f=0, g=0;	
	int n=0;
	bool flag1 = 1;

	float fMinVess = 9999;
	int fMinVessIdx = 0;

	VolumeFilterInitial(apsInputData, InputCopy, iImageLayer, iImageHeight, iImageWidth);

	for (i=0; i<iImageLayer; i++)
	{
		for (j=0; j<iImageHeight; j++)
		{
			for (k=0; k<iImageWidth; k++)
			{
				if (InputCopy[i*iImageHeight*iImageWidth + j*iImageWidth + k] !=0)
				{
					    myPoint.x = k;
					    myPoint.y = j;
					    myPoint.z = i;
					    pointSetVOI.push_back(myPoint);
					    InputCopy[i*iImageHeight*iImageWidth + j*iImageWidth + k] = 9999;
						Calculate26Adjacent(InputCopy, pointSetVOI,  pointSetTemp, i, j, k, iImageHeight, iImageWidth, counter);
					
						if (counter==0)
						{
							if(pointSetVOI.size() ==1)
							{
								pointSetVOI.clear();
								apsInputData[i*iImageHeight*iImageWidth + j*iImageWidth + k] = -1;
								InputCopy[i*iImageHeight*iImageWidth + j*iImageWidth + k] = 0;
							}
						}
						else if (counter>0)
						{	
							flag1 = 1;											
							while(flag1)
							{
								for (n=0; n<counter; n++)  
								{
									myPoint = pointSetTemp.at(n);
									a = myPoint.x;
									b = myPoint.y;
									c = myPoint.z;

									CalculateRegionGrowAdjacent(InputCopy, pointSetVOI,  pointSetTemp2,
															c, b, a, iImageHeight, iImageWidth, counter1);
									
								}
								if (counter1==0)
								{
									flag1 = 0;
									if (pointSetVOI.size()>0 && pointSetVOI.size()<iDelSize)
									{
										for (int v=0; v<pointSetVOI.size(); v++)
										{										
											myPoint = pointSetVOI.at(v);
											e = myPoint.x;
											f = myPoint.y;
											g = myPoint.z;
											apsInputData[g*iImageHeight*iImageWidth + f*iImageWidth + e] = -1;
											InputCopy[g*iImageHeight*iImageWidth + f*iImageWidth + e] = 0;
										}
									}
                                    else
									{
										float fSum = 0.0;
									    for (int u=0; u<pointSetVOI.size(); u++)
										{
											myPoint = pointSetVOI.at(u);
											e = myPoint.x;
											f = myPoint.y;
											g = myPoint.z;

                                            InputCopy[g*iImageHeight*iImageWidth + f*iImageWidth + e] = 0;
											apsMask[g*iImageHeight*iImageWidth + f*iImageWidth + e] = iVesCnt;											
										}
										iVesCnt++;
									}

									pointSetVOI.clear();
									pointSetTemp.clear();
									pointSetTemp2.clear();
									counter = 0;
									counter1 = 0;
								}
								else
								{
									counter = counter1;
									pointSetTemp.clear();
									for (int t=0; t<counter1; t++)
									{
                                       pointSetTemp.push_back(pointSetTemp2.at(t));
									}
									counter1 = 0;
									pointSetTemp2.clear();
									
								}
							}//while
						}//else if
				}
			}				
		}
	}
    
	memcpy(apsInputData, apsMask, sizeof(short)*iTotal);
	delete []apsMask;
    delete []InputCopy;
	return;
}

void NeuronSeg::VolumeFilterInitial(short* apsInputData, short *apsInputCopy, 
													 int  iSliceNmb, int  iRowNmb, int iColNmb)
{
	int i;
	int j;
	int k;
	
	for (i=0; i<1; i++)
	{
		for (j=0; j<iRowNmb; j++)
		{
			for (k=0; k<iColNmb; k++)
			{
				if (apsInputData[j*iColNmb + k] != 0)
				{
					apsInputData[j*iColNmb + k] = 0;
					apsInputCopy[j*iColNmb + k] = 0;
				}
			}
		}
	}

	for (i=iSliceNmb-1; i<iSliceNmb; i++)
	{
		for (j=0; j<iRowNmb; j++)
		{
			for (k=0; k<iColNmb; k++)
			{
				if (apsInputData[i*iRowNmb*iColNmb + j*iColNmb + k] != 0)
				{
					apsInputData[i*iRowNmb*iColNmb + j*iColNmb + k] = 0;
					apsInputCopy[i*iRowNmb*iColNmb + j*iColNmb + k] = 0;
				}
			}
		}

	}

	for (i=0; i<iSliceNmb; i++)
	{
		for (j=0; j<iRowNmb; j++)
		{
			for (k=0; k<1; k++)
			{
				if (apsInputData[i*iRowNmb*iColNmb + j*iColNmb + k] != 0)
				{
					apsInputData[i*iRowNmb*iColNmb + j*iColNmb + k] = 0;
					apsInputCopy[i*iRowNmb*iColNmb + j*iColNmb + k] = 0;
				}
			}
		}

	}
	for (i=0; i<iSliceNmb; i++)
	{
		for (j=0; j<iRowNmb; j++)
		{
			for (k=iColNmb-1; k<iColNmb; k++)
			{
				if (apsInputData[i*iRowNmb*iColNmb + j*iColNmb + k] != 0)
				{
					apsInputData[i*iRowNmb*iColNmb + j*iColNmb + k] = 0;
					apsInputCopy[i*iRowNmb*iColNmb + j*iColNmb + k] = 0;
				}
			}
		}

	}

	for (i=0; i<iSliceNmb; i++)
	{
		for (j=iRowNmb-1; j<iRowNmb; j++)
		{
			for (k=0; k<iColNmb; k++)
			{
				if (apsInputData[i*iRowNmb*iColNmb + j*iColNmb + k] != 0)
				{
					apsInputData[i*iRowNmb*iColNmb + j*iColNmb + k] = 0;
					apsInputCopy[i*iRowNmb*iColNmb + j*iColNmb + k] = 0;
				}
			}
		}

	}

	for (i=0; i<iSliceNmb; i++)
	{
		for (j=0; j<1; j++)
		{
			for (k=0; k<iColNmb; k++)
			{
				if (apsInputData[i*iRowNmb*iColNmb + j*iColNmb + k] != 0)
				{
					apsInputData[i*iRowNmb*iColNmb + j*iColNmb + k] = 0;
					apsInputCopy[i*iRowNmb*iColNmb + j*iColNmb + k] = 0;
				}
			}
		}

	}
}

void NeuronSeg::Calculate26Adjacent(short * apsInputCopy, deque<Point3D_t> &dePointSetVessel, 
										deque<Point3D_t> &dePointSetTmp, int iCurSlice,int iCurRowNmb,int iCurColNmb,
										int iRowNmb, int iColNmb, int &iCnt)
{
	Point3D_t ptPoint;

	
	if (apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb-1]!=0)
	{
		ptPoint.x = iCurColNmb-1;
		ptPoint.y = iCurRowNmb-1;
		ptPoint.z = iCurSlice-1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb-1] = 9999;
		iCnt++;
	}

	
	if (apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb]!=0)
	{
		ptPoint.x = iCurColNmb;
		ptPoint.y = iCurRowNmb-1;
		ptPoint.z = iCurSlice-1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb] = 9999;
		iCnt++;
	}


	if (apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb+1]!=0)
	{
		ptPoint.x = iCurColNmb+1;
		ptPoint.y = iCurRowNmb-1;
		ptPoint.z = iCurSlice-1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb+1] = 9999;
		iCnt++;
	}


	if (apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb-1]!=0)
	{
		ptPoint.x = iCurColNmb-1;
		ptPoint.y = iCurRowNmb;
		ptPoint.z = iCurSlice-1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb-1] = 9999;
		iCnt++;
	}

	
	if (apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb]!=0)
	{
		ptPoint.x = iCurColNmb;
		ptPoint.y = iCurRowNmb;
		ptPoint.z = iCurSlice-1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb] = 9999;
		iCnt++;
	}


	if (apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb+1]!=0)
	{
		ptPoint.x = iCurColNmb+1;
		ptPoint.y = iCurRowNmb;
		ptPoint.z = iCurSlice-1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb+1] = 9999;
		iCnt++;
	}

	if (apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb-1]!=0)
	{
		ptPoint.x = iCurColNmb-1;
		ptPoint.y = iCurRowNmb+1;
		ptPoint.z = iCurSlice-1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb-1] = 9999;
		iCnt++;
	}

	if (apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb]!=0)
	{
		ptPoint.x = iCurColNmb;
		ptPoint.y = iCurRowNmb+1;
		ptPoint.z = iCurSlice-1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb] = 9999;
		iCnt++;
	}

	if (apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb+1]!=0)
	{
		ptPoint.x = iCurColNmb+1;
		ptPoint.y = iCurRowNmb+1;
		ptPoint.z = iCurSlice-1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb+1] = 9999;
		iCnt++;
	}

	if (apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb-1]!=0)
	{
		ptPoint.x = iCurColNmb-1;
		ptPoint.y = iCurRowNmb-1;
		ptPoint.z = iCurSlice;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb-1] = 9999;
		iCnt++;
	}

	if (apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb]!=0)
	{
		ptPoint.x = iCurColNmb;
		ptPoint.y = iCurRowNmb-1;
		ptPoint.z = iCurSlice;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb] = 9999;
		iCnt++;
	}
	if (apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb+1]!=0)
	{
		ptPoint.x = iCurColNmb+1;
		ptPoint.y = iCurRowNmb-1;
		ptPoint.z = iCurSlice;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb+1] = 9999;
		iCnt++;
	}

	if (apsInputCopy[iCurSlice*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb-1]!=0)
	{
		ptPoint.x = iCurColNmb-1;
		ptPoint.y = iCurRowNmb;
		ptPoint.z = iCurSlice;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[iCurSlice*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb-1] = 9999;
		iCnt++;
	}

	if (apsInputCopy[iCurSlice*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb+1]!=0)
	{
		ptPoint.x = iCurColNmb+1;
		ptPoint.y = iCurRowNmb;
		ptPoint.z = iCurSlice;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[iCurSlice*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb+1] = 9999;
		iCnt++;
	}

	if (apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb-1]!=0)
	{
		ptPoint.x = iCurColNmb-1;
		ptPoint.y = iCurRowNmb+1;
		ptPoint.z = iCurSlice;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb-1] = 9999;
		iCnt++;
	}



	if (apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb]!=0)
	{
		ptPoint.x = iCurColNmb;
		ptPoint.y = iCurRowNmb+1;
		ptPoint.z = iCurSlice;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb] = 9999;
		iCnt++;
	}

	if (apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb+1]!=0)
	{
		ptPoint.x = iCurColNmb+1;
		ptPoint.y = iCurRowNmb+1;
		ptPoint.z = iCurSlice;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb+1] = 9999;
		iCnt++;
	}

	//19
	if (apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb-1]!=0)
	{
		ptPoint.x = iCurColNmb-1;
		ptPoint.y = iCurRowNmb-1;
		ptPoint.z = iCurSlice+1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb-1] = 9999;
		iCnt++;
	}

	//20
	if (apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb]!=0)
	{
		ptPoint.x = iCurColNmb;
		ptPoint.y = iCurRowNmb-1;
		ptPoint.z = iCurSlice+1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb] = 9999;
		iCnt++;
	}

	//21
	if (apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb+1]!=0)
	{
		ptPoint.x = iCurColNmb+1;
		ptPoint.y = iCurRowNmb-1;
		ptPoint.z = iCurSlice+1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb+1] = 9999;
		iCnt++;
	}

	//22
	if (apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb-1]!=0)
	{
		ptPoint.x = iCurColNmb-1;
		ptPoint.y = iCurRowNmb;
		ptPoint.z = iCurSlice+1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb-1] = 9999;
		iCnt++;
	}

	//23
	if (apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb]!=0)
	{
		ptPoint.x = iCurColNmb;
		ptPoint.y = iCurRowNmb;
		ptPoint.z = iCurSlice+1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb] = 9999;
		iCnt++;
	}

	//24
	if (apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb+1]!=0)
	{
		ptPoint.x = iCurColNmb+1;
		ptPoint.y = iCurRowNmb;
		ptPoint.z = iCurSlice+1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb+1] = 9999;
		iCnt++;
	}

	//25
	if (apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb-1]!=0)
	{
		ptPoint.x = iCurColNmb-1;
		ptPoint.y = iCurRowNmb+1;
		ptPoint.z = iCurSlice+1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb-1] = 9999;
		iCnt++;
	}


	//26
	if (apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb]!=0)
	{
		ptPoint.x = iCurColNmb;
		ptPoint.y = iCurRowNmb+1;
		ptPoint.z = iCurSlice+1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb] = 9999;
		iCnt++;
	}


	//27
	if (apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb+1]!=0)
	{
		ptPoint.x = iCurColNmb+1;
		ptPoint.y = iCurRowNmb+1;
		ptPoint.z = iCurSlice+1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb+1] = 9999;
		iCnt++;
	}

}



void NeuronSeg::CalculateRegionGrowAdjacent(short * apsInputCopy, deque<Point3D_t> &dePointSetVessel,  deque<Point3D_t> &dePointSetTmp,
							 int iCurSlice,int iCurRowNmb,int iCurColNmb, int iRowNmb, int iColNmb, int &iCnt)
{
	Point3D_t ptPoint;

	if ((apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb-1] !=9999) && 
		(apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb-1] !=0))
	{
		ptPoint.x = iCurColNmb-1;
		ptPoint.y = iCurRowNmb-1;
		ptPoint.z = iCurSlice-1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb-1] = 9999;
		iCnt++;
	}


	if ((apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb] !=9999) && 
		(apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb] !=0))
	{
		ptPoint.x = iCurColNmb;
		ptPoint.y = iCurRowNmb-1;
		ptPoint.z = iCurSlice-1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb] = 9999;
		iCnt++;
	}


	if ((apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb+1] !=9999) && 
		(apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb+1] !=0))
	{
		ptPoint.x = iCurColNmb+1;
		ptPoint.y = iCurRowNmb-1;
		ptPoint.z = iCurSlice-1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb+1] = 9999;
		iCnt++;
	}


	if ((apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb-1] !=9999) && 
		(apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb-1] !=0))
	{
		ptPoint.x = iCurColNmb-1;
		ptPoint.y = iCurRowNmb;
		ptPoint.z = iCurSlice-1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb-1] = 9999;
		iCnt++;
	}


	if ((apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb] !=9999) && 
		(apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb] !=0))
	{
		ptPoint.x = iCurColNmb;
		ptPoint.y = iCurRowNmb;
		ptPoint.z = iCurSlice-1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb] = 9999;
		iCnt++;
	}


	if ((apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb+1] !=9999) && 
		(apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb+1] !=0))
	{
		ptPoint.x = iCurColNmb+1;
		ptPoint.y = iCurRowNmb;
		ptPoint.z = iCurSlice-1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb+1] = 9999;
		iCnt++;
	}


	if ((apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb-1] !=9999) && 
		(apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb-1] !=0))
	{
		ptPoint.x = iCurColNmb-1;
		ptPoint.y = iCurRowNmb+1;
		ptPoint.z = iCurSlice-1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb-1] = 9999;
		iCnt++;
	}


	if ((apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb] !=9999) && 
		(apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb] !=0))
	{
		ptPoint.x = iCurColNmb;
		ptPoint.y = iCurRowNmb+1;
		ptPoint.z = iCurSlice-1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb] = 9999;
		iCnt++;
	}


	if ((apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb+1] !=9999) && 
		(apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb+1] !=0))
	{
		ptPoint.x = iCurColNmb+1;
		ptPoint.y = iCurRowNmb+1;
		ptPoint.z = iCurSlice-1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice-1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb+1] = 9999;
		iCnt++;
	}


	if ((apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb-1] !=9999) && 
		(apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb-1] !=0))
	{
		ptPoint.x = iCurColNmb-1;
		ptPoint.y = iCurRowNmb-1;
		ptPoint.z = iCurSlice;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb-1] = 9999;
		iCnt++;
	}


	if ((apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb] !=9999) && 
		(apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb] !=0))
	{
		ptPoint.x = iCurColNmb;
		ptPoint.y = iCurRowNmb-1;
		ptPoint.z = iCurSlice;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb] = 9999;
		iCnt++;
	}


	if ((apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb+1] !=9999) && 
		(apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb+1] !=0))
	{
		ptPoint.x = iCurColNmb+1;
		ptPoint.y = iCurRowNmb-1;
		ptPoint.z = iCurSlice;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb+1] = 9999;
		iCnt++;
	}


	if ((apsInputCopy[iCurSlice*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb-1] !=9999) && 
		(apsInputCopy[iCurSlice*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb-1] !=0))
	{
		ptPoint.x = iCurColNmb-1;
		ptPoint.y = iCurRowNmb;
		ptPoint.z = iCurSlice;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[iCurSlice*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb-1] = 9999;
		iCnt++;
	}


	if ((apsInputCopy[iCurSlice*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb+1] !=9999) && 
		(apsInputCopy[iCurSlice*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb+1] !=0))
	{
		ptPoint.x = iCurColNmb+1;
		ptPoint.y = iCurRowNmb;
		ptPoint.z = iCurSlice;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[iCurSlice*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb+1] = 9999;
		iCnt++;
	}


	if ((apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb-1] !=9999) && 
		(apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb-1] !=0))
	{
		ptPoint.x = iCurColNmb-1;
		ptPoint.y = iCurRowNmb+1;
		ptPoint.z = iCurSlice;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb-1] = 9999;
		iCnt++;
	}


	if ((apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb] !=9999) && 
		(apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb] !=0))
	{
		ptPoint.x = iCurColNmb;
		ptPoint.y = iCurRowNmb+1;
		ptPoint.z = iCurSlice;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb] = 9999;
		iCnt++;
	}


	if ((apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb+1] !=9999) && 
		(apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb+1] !=0))
	{
		ptPoint.x = iCurColNmb+1;
		ptPoint.y = iCurRowNmb+1;
		ptPoint.z = iCurSlice;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[iCurSlice*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb+1] = 9999;
		iCnt++;
	}


	if ((apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb-1] !=9999) && 
		(apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb-1] !=0))
	{
		ptPoint.x = iCurColNmb-1;
		ptPoint.y = iCurRowNmb-1;
		ptPoint.z = iCurSlice+1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb-1] = 9999;
		iCnt++;
	}


	if ((apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb] !=9999) && 
		(apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb] !=0))
	{
		ptPoint.x = iCurColNmb;
		ptPoint.y = iCurRowNmb-1;
		ptPoint.z = iCurSlice+1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb] = 9999;
		iCnt++;
	}


	if ((apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb+1] !=9999) && 
		(apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb+1] !=0))
	{
		ptPoint.x = iCurColNmb+1;
		ptPoint.y = iCurRowNmb-1;
		ptPoint.z = iCurSlice+1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb-1)*iColNmb + iCurColNmb+1] = 9999;
		iCnt++;
	}


	if ((apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb-1] !=9999) && 
		(apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb-1] !=0))
	{
		ptPoint.x = iCurColNmb-1;
		ptPoint.y = iCurRowNmb;
		ptPoint.z = iCurSlice+1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb-1] = 9999;
		iCnt++;
	}


	if ((apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb] !=9999) && 
		(apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb] !=0))
	{
		ptPoint.x = iCurColNmb;
		ptPoint.y = iCurRowNmb;
		ptPoint.z = iCurSlice+1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb] = 9999;
		iCnt++;
	}


	if ((apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb+1] !=9999) && 
		(apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb+1] !=0))
	{
		ptPoint.x = iCurColNmb+1;
		ptPoint.y = iCurRowNmb;
		ptPoint.z = iCurSlice+1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + iCurRowNmb*iColNmb + iCurColNmb+1] = 9999;
		iCnt++;
	}


	if ((apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb-1] !=9999) && 
		(apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb-1] !=0))
	{
		ptPoint.x = iCurColNmb-1;
		ptPoint.y = iCurRowNmb+1;
		ptPoint.z = iCurSlice+1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb-1] = 9999;
		iCnt++;
	}


	if ((apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb] !=9999) && 
		(apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb] !=0))
	{
		ptPoint.x = iCurColNmb;
		ptPoint.y = iCurRowNmb+1;
		ptPoint.z = iCurSlice+1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb] = 9999;
		iCnt++;
	}


	if ((apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb+1] !=9999) && 
		(apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb+1] !=0))
	{
		ptPoint.x = iCurColNmb+1;
		ptPoint.y = iCurRowNmb+1;
		ptPoint.z = iCurSlice+1;
		dePointSetVessel.push_back(ptPoint);
		dePointSetTmp.push_back(ptPoint);
		apsInputCopy[(iCurSlice+1)*iRowNmb*iColNmb + (iCurRowNmb+1)*iColNmb + iCurColNmb+1] = 9999;
		iCnt++;
	}
}


void NeuronSeg::BinaryProcess(short *apsInput, int iImageWidth, int iImageHeight, int iImageLayer, short *apsOutput)
{
	int pMax, pMin;
	int i, j;
	double T ;
	double TT;
	int S0 , n0;
	int S1, n1 ;
	double allow; 
	double d ;

	int mCount = iImageHeight * iImageWidth;


	for(int m=0; m<iImageLayer; m++)
	{		
		pMax = pMin =apsInput[m*mCount];		
		for(i=0; i<mCount; i++)
		{
			for(i=0; i<mCount; i++)
			{
				pMax = (pMax > apsInput[m*mCount+i]) ? pMax : apsInput[m*mCount+i];
				
				pMin = (pMin < apsInput[m*mCount+i]) ? pMin : apsInput[m*mCount+i];
				
			}

			T = (pMax + pMin) / 2.0;
			TT = 0;
			S0 = 0;
			n0 = 0;
			S1 = 0;
			n1 = 0;
			allow = 6.0f; 
			d = fabs(T - TT);


			while(d > allow) 
			{	
				for(j=0; j<mCount; j++)
				{
					if(apsInput[m*mCount+j] > T) 
					{
						S0 += apsInput[m*mCount+j];
						n0++;
					}
					else
					{
						S1 += apsInput[m*mCount+j];
						n1++;
					}
				}

				if(n0 ==0 || n1 == 0)
					return  ;
				else
				{   
					TT = (S0 / n0 + S1 / n1) / 2;
				}

				d = fabs (T - TT);
				T = TT;
			}

			for(i=0; i<mCount; i++)
			{
				if(apsInput[m*mCount+i] > T)
				{
					apsOutput[m*mCount+i] = 4096;				
				}
				else
				{
					apsOutput[m*mCount+i] = 0;				
				}
			}				
		}			
	}

}

