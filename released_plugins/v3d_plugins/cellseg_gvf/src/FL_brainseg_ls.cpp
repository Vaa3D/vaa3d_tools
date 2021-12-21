//cell segmentation
//by Fuhui Long
//2008-03-07

#include <math.h>

//the folowing conditional compilation is added by PHC, 2010-05-20
#if defined (_MSC_VER) && (_WIN64)
#include "../basic_c_fun/vcdiff.h"
#else
#endif


#include "./local_basic_c_fun/volimg_proc.h"
#include "./local_basic_c_fun/img_definition.h"
#include "./local_basic_c_fun/stackutil.h"
#include "./local_basic_c_fun/basic_memory.h"

#include "FL_filter3D.h"
#include "FL_evolve.cpp"
#include "FL_morphology.h"
#include "FL_morphology.cpp"
#include "FL_main_brainseg.h"
#include "FL_regionProps.h"
#include "FL_bwlabel2D3D.h"
#include "FL_bwdist.h"


//rectify shape of segmentated region after evolving, since level set can change topology, sometimes it can generate 
// small residue debris regions that are not needed for this application (for some other applicaations, changing topology
// maybe useful)

// u: 2d evolving result of a region
// sznew: size of 3d image (only i,j dimensions are used in rectifyShape)
// m: index of region 
// labelimgdata3d: 3d data of template label field  
// utpdata2d: a 2d matrix for storing intermediate result

void rectifyShape(unsigned short int **&ulabel, float **u, V3DLONG *sznew, V3DLONG m, unsigned short int ***labelimgdata3d, float **utmpdata2d)
{
	//rectify evolving result, remove very small regions that are the residue of evolving procedure
	// binarize u

//	printf("Rectifying evolving result...\n");
	
	V3DLONG i,j;
	
	for (j=0; j<sznew[1]; j++)
	for (i=0; i<sznew[0]; i++)
	{	
		if (u[j][i]<=0)
		{
			u[j][i] = 1;
		}
		else
		{
			u[j][i] = 0;
		}
	}
	
	//recover utmp to the mask that is before dilation
	for (j=0; j<sznew[1]; j++)
	for (i=0; i<sznew[0]; i++)
	{
		if (labelimgdata3d[m][j][i]>0)
			utmpdata2d[j][i] = 1;
		else
			utmpdata2d[j][i] = 0;
			
	}

	// find labeled regions of u and the label field of the template image
	
	unsigned short int * templatelabel1d=0;
	templatelabel1d = new unsigned short int [sznew[0]*sznew[1]];
	
//	unsigned short int *ulabel1d, **ulabel;
//	ulabel1d = new unsigned short int [sznew[0]*sznew[1]];
//	if (ulabel1d)
//	{
//	   ulabel = 0;
//	   new2dpointer(ulabel, sznew[0], sznew[1], ulabel1d);
//	}
	
	float *u1d = u[0]; //u1d has been deleted in FL_evolve.cpp
	float *utmpdata1d = utmpdata2d[0];
	unsigned short int *ulabel1d = ulabel[0]; // potential problem in deleting???
	
	findConnectedComponent(u1d, sznew, 2, 2, ulabel1d); //u1d is the 1d pointer of u, note that labels are continous under this scenario
	findConnectedComponent(utmpdata1d, sznew, 2, 2, templatelabel1d); // note that labels are continous under this scenario						
	
	// find maximum labels
	unsigned short int maxval = ulabel1d[0];
	V3DLONG tmplen = sznew[0] * sznew[1];
	for (i=0; i<tmplen; i++)
	{
		maxval = (ulabel1d[i] > maxval) ? ulabel1d[i] : maxval;
	}

	unsigned short int maxvalT= templatelabel1d[0];
	tmplen = sznew[0] * sznew[1];
	for (i=0; i<tmplen; i++)
	{
		maxvalT = (templatelabel1d[i] > maxvalT) ? templatelabel1d[i] : maxvalT;
	}
	
	// delete pointer, free memory
	if (templatelabel1d) {delete []templatelabel1d; templatelabel1d=0;}
	
	int num = maxval;
	
	while (num > maxvalT)
	{
		Props rgnprops;
//				Props * rgnprops;
//				rgnprops = regionProps(ulabel, sznew);

		// find the smallest region
		int minval = 999;
		int rgn = 1; 
		
		for (i=0;i<maxval; i++)
		{
			V3DLONG thisarea = rgnprops.getArea(ulabel,i+1,sznew);
//					printf("thisarea = %d\n", thisarea);
			
			if ((thisarea > 0) && (thisarea < minval))
			{
				minval = thisarea;
				rgn = i+1;
				
			}	
		}
		
		// set the smallest region to background
		for (j=0; j<sznew[1]; j++)
		for (i=0; i<sznew[0]; i++)
		{
			ulabel[j][i] = (ulabel[j][i] == rgn) ? 0: ulabel[j][i];
		}
		
		num = num - 1;
	} //while
		
//	printf("Rectifying evolving result done...\n");
}			
			
// segment brain compartment in 2D
// rawimg3d: raw intensity image needs to be segmentated
// segimg3d: segmented label field of template image
// outimg3d: segmentation result
// segPara: paramters used in segmentation

bool brainSeg2D(Vol3DSimple <unsigned char> *rawimg3d, Vol3DSimple <unsigned short int> *segimg3d, 
				Vol3DSimple <unsigned short int> *outimg3d, const levelset_segParameter & segPara)
{
	
	// ------------------------------------------------
	// initialization and allocate memory
	// ------------------------------------------------
	
	unsigned char marginx = 0, c0 = 4; // expand the image a little bit, so that the evolving of optical lobes can form closed boundary		
	int i,j,k,t; //note that Fuhui origionally ahs double-defined j here and in another inner loop, which can cause confusion. But I don't know how to revise at this moment, run out of time! by PHC, 2010-05-20

	float para[6];
	para[0] = segPara.lamda; 
	para[1] = segPara.mu;
	para[2] = segPara.alf;
	para[3] = segPara.epsilon;
	para[4] = segPara.delt;
	para[5] = segPara.gama;
	V3DLONG sz[3];

	Strel *se1 = new Strel;
	Strel *se2 = new Strel; 
	se1 -> createDiskStrel(3);
	se2 -> createDiskStrel(5);
	
	// size of original image
	sz[0] = rawimg3d->sz0(); sz[1] = rawimg3d->sz1(); sz[2] = rawimg3d->sz2();
	
	// size of expanded image
	V3DLONG sznew[3]; 
	sznew[0] = sz[0]+2*marginx;	sznew[1] = sz[1]; sznew[2] = sz[2];
	
	V3DLONG sznew2d[2];
	sznew2d[0] = sznew[0]; sznew2d[1] = sznew[1];
	
	V3DLONG tmplen = sznew[0] * sznew[1];
	
	unsigned short int ***outimgdata3d = outimg3d -> getData3dHandle();
	unsigned char ***rawimgdata3d = rawimg3d->getData3dHandle();	
	unsigned short int ***segimg3ddata3d = segimg3d -> getData3dHandle();
	
	Vol3DSimple <unsigned short int> *labelimg = new Vol3DSimple <unsigned short int> (sznew[0], sznew[1], segPara.regionnum); //  expand y dimension to help optical lobe segmentation
	unsigned short int ***labelimgdata3d = labelimg->getData3dHandle();
	
	Image2DSimple <float> *utmp = new Image2DSimple <float> (sznew[0], sznew[1]);
	float **utmpdata2d = utmp -> getData2dHandle();
	float *utmpdata1d = utmp -> getData1dHandle();

	float *imgdata1d = new float [tmplen]; //data array used to save intermediate results	
	float *imgdata1dm = new float [sznew[0]*sznew[1]*segPara.regionnum];	
	float **imgdata2d = 0, ***imgdata3d = 0;
	
	unsigned char *smoothedimgdata1d = new unsigned char [sz[0]*sz[1]*sz[2]]; // smoothed image data, need to keep it throughout the program and delete it at the very end
	unsigned char ***smoothedimgdata3d = 0;
	
	float *gradientimgdata1d = new float [tmplen];
	float **gradientimgdata2d = 0;

	unsigned short int *resultimgdata1d = new unsigned short int [sznew[0]*sznew[1]*segPara.regionnum];
	unsigned short int ***resultimgdata3d = 0; // record the segmenation result of each region on each slice that evolve independently
	
	float *uneighbor1d = new float [tmplen];
	float **uneighbor2d = 0; //for repelling force

    float * u1d = new float [tmplen]; //u is the level set function, 
	float **u = 0;

	float * uold1d = new float [tmplen];
	float **uold = 0; //uold is used to test if the evolution converges and should stop	
	
	float *uall1d = new float [sznew[0]*sznew[1]*segPara.regionnum]; // for evolving after adding repelling force
	float *** uall =0;
	
	unsigned short int *ulabel1d = new unsigned short int [tmplen];
	unsigned short int **ulabel = 0; // for rectifying shape

			
	unsigned short int *tag = new unsigned short int [segPara.maxregionnum]; // tag indicates which region need to be segmented	
	unsigned int *labelimgdataArea = new unsigned int [segPara.regionnum]; // size of an labeled area, used to determined 1) how much should a region be dilated as initial segmentation; 2) evolving stopping criteria
	V3DLONG *pixindex = new V3DLONG [sznew[0]*sznew[1]]; //used in distance transform
	unsigned char *tagModel = new unsigned char [segPara.regionnum];
	

	//unsigned char overlaprgn[segPara.regionnum][segPara.regionnum];// matrix indicating which region and which has overlap after independent evolving
    unsigned char **overlaprgn = new unsigned char * [(V3DLONG)(segPara.regionnum)]; //added by PHC, 2010-05-20
	unsigned char * overlaprgn1d = new unsigned char [(V3DLONG)(segPara.regionnum) * segPara.regionnum]; //added by PHC, 2010-05-20
	for (i=0;i<(V3DLONG)(segPara.regionnum);i++) //added by PHC, 2010-05-20
		overlaprgn[i] = overlaprgn1d + i*(V3DLONG)(segPara.regionnum);

	// allocate memory for smoothedimgdata3d	
   if (sz[0]>0 && sz[1]>0 && sz[2]>0)
   {
		
	   if (smoothedimgdata1d)
	   {	if (!new3dpointer(smoothedimgdata3d, sz[0], sz[1], sz[2], smoothedimgdata1d))
			{
				fprintf(stderr, "fail to allocate memory for smoothedimgdata3d\n");
				return false;			
			}
		}
	}
	
	//allocate memory for gradientimgdata2d, uneighbor2d, imgdata2d, u, uold, ulabel	
   if (sznew[0]>0 && sznew[1]>0)
   {
		
	   if (gradientimgdata1d)
	   {   if (!new2dpointer(gradientimgdata2d, sznew[0], sznew[1], gradientimgdata1d))
		   {
				fprintf(stderr, "fail to allocate memory for gradientimgdata2d\n");
				return false;
		   }
		}
		
	   if (uneighbor1d)
	   {
		   if (!new2dpointer(uneighbor2d, sznew[0], sznew[1], uneighbor1d))
		   {
				fprintf(stderr, "fail to allocate memory for uneighbor2d\n");
				return false;		   
		   }
		}
		
	   if (imgdata1d)
	   {
		   if (!new2dpointer(imgdata2d, sznew[0], sznew[1], imgdata1d))
		   {
				fprintf(stderr, "fail to allocate memory for imgdata2d\n");
				return false;		   		   
		   }
		}

	   if (u1d)
	   {
		   if (!new2dpointer(u, sznew[0], sznew[1], u1d))
		   {
				fprintf(stderr, "fail to allocate memory for u\n");
				return false;		   		   		   
		   }
		}
		
	   if (uold1d)
	   {
		   if (!new2dpointer(uold, sznew[0], sznew[1], uold1d))
		   {
				fprintf(stderr, "fail to allocate memory for uold\n");
				return false;		   		   		   
		   }
		}

		if (ulabel1d)
		{		
		   if (!new2dpointer(ulabel, sznew[0], sznew[1], ulabel1d))
		   {
				fprintf(stderr, "fail to allocate memory for ulabel\n");
				return false;		   		   		   		   
		   }
		}
	}

	// allocate memory for resultimgdata3d, imgdata3d, uall				
   if (sznew[0]>0 && sznew[1]>0 && segPara.regionnum>0)
   {		
   
   	   if (resultimgdata1d)
	   {
		   if (!new3dpointer(resultimgdata3d, sznew[0], sznew[1], segPara.regionnum, resultimgdata1d))
		   {
				fprintf(stderr, "fail to allocate memory for resultimgdata3d\n");
				return false;		   		   		   		   		   
		   }
		}

	   if (imgdata1dm)
	   {
		   if (!new3dpointer(imgdata3d, sznew[0], sznew[1], segPara.regionnum, imgdata1dm))
		   {
				fprintf(stderr, "fail to allocate memory for imgdata3d\n");
				return false;		   		   		   		   		   		   
		   }
		}
		
		if (uall1d)
		{
		   if (!new3dpointer(uall, sznew[0], sznew[1], segPara.regionnum, uall1d))
		   {
				fprintf(stderr, "fail to allocate memory for imgdata3d\n");
				return false;		   		   		   		   		   		   
		   }
		}
		
	}

	// assign values to tag
	for (i=0;i<segPara.maxregionnum; i++)
		tag[i] = 0;
		
	for (i=0;i<segPara.regionnum; i++)
	{
		tag[segPara.regions[i]-1] = i+1;
		int n=0;
		
		tagModel[i] = 1;
		while (n< segPara.regionNoModelNum)
		{
			if (segPara.regionNoModel[n]!=segPara.regions[i])
				n++;
			else
			{  tagModel[i] = 0; // does not use model
				break;
			}	
		}
	}

	// ------------------------------------------------
	// smooth intensity image using gaussian filtering
	// ------------------------------------------------
	
	printf("Gaussian smoothing ...\n");
	
//	printf("%d, %d, %d\n", rawimg3d->sz0(), rawimg3d->sz1(), rawimg3d->sz2());
//	printf("%d, %d, %d\n", sz[0], sz[1], sz[2]);
	
	gaussfilt3d(rawimgdata3d, smoothedimgdata3d, sz, 2, 1); //use outimgdata3d as output temporarily
				
	printf("Gaussian smoothing done ...\n");

	//-----------------------
	// process each 2d slide
	//-----------------------
	
//	for (k=0;k<sz[2]; k++) // process each 2d slide
	for (k=40;k<41; k++) // process each 2d slide for region 12, and 4, which are not adjacent, no need to add repelling force
//	for (k=52;k<53; k++) // process each 2d slide for region 12, 38, and 7, which are adjacent	
	{
		// generate a 2d slide image, expand size to help optical lobe segmentation
		// assign values to labelimgdata3d, imgdata3d
	
		printf("Segmenting %d th slice\n", k);
		
		for (int m=0; m<segPara.regionnum; m++)
			labelimgdataArea[m] = 0;

		for (int m=0; m<segPara.regionnum; m++)
		for (j=0; j<sznew[1]; j++)
		for (i=0; i<sznew[0]; i++)
		{
			labelimgdata3d[m][j][i] = 0;
			resultimgdata3d[m][j][i] = 0;
		}
				
//		for (j=0; j<sznew[1]; j++)
//		for (i=0; i<sznew[0]; i++)
//		{
//			imgdata2d[j][i] = 0;
//		}

		for (j=0; j<sznew[1]; j++)
		for (i=marginx; i<sz[0]+marginx; i++)
		{
		
			t = segimg3ddata3d[k][j][i-marginx];
			
			if (t>0)
			{
				if (tag[t-1]>0) // the region needs to be segmented
				{
					labelimgdata3d[tag[t-1]-1][j][i] = t;
					labelimgdataArea[tag[t-1]-1] = labelimgdataArea[tag[t-1]-1] + 1;
				}	
			}

			gradientimgdata2d[j][i] = smoothedimgdata3d[k][j][i-marginx];
			
		}
		
//		// test if labelimgdata3d is correct for multiple regions
//		for (int m=0; m<segPara.regionnum; m++)
//			for (j=0; j<sz[1]; j++)
//				for (i=0; i<sz[0]; i++)
//				{
//					outimgdata3d[m][j][i] = labelimgdata3d[m][j][i];
//				}


		// compute gradient
		float **Ix, **Iy;
		float *Ix1d, *Iy1d;
		
		tmplen = sznew[0] * sznew[1];
	   if (tmplen>0 && sznew[0]>0 && sznew[1]>0)
	   {
		   Ix1d = new float [tmplen];
		   if (Ix1d)
		   {
			   Ix = 0;
			   new2dpointer(Ix, sznew[0], sznew[1], Ix1d);
			}
			
		   Iy1d = new float [tmplen];
		   if (Iy1d)
		   {
			   Iy = 0;
			   new2dpointer(Iy, sznew[0], sznew[1], Iy1d);
			}
		}

		gradient(gradientimgdata2d,Ix, Iy,sznew);
		
		
		for (j=0;j<sznew[1];j++)
		for (i=0;i<sznew[0]; i++)			
		{
			gradientimgdata2d[j][i] = Ix[j][i]*Ix[j][i] + Iy[j][i]*Iy[j][i];
		}
		
		//delete pointers
		if (Ix) delete2dpointer(Ix, sznew[0], sznew[1]);		
		if (Ix1d) {delete Ix1d; Ix1d=0;}		
		if (Iy) delete2dpointer(Iy, sznew[0], sznew[1]);				
		if (Iy1d) {delete Iy1d; Iy1d=0;}		
		
		// segment each region
//		printf("number of regions to be segmented %d\n", segPara.regionnum);

		// compute imgdata3d, the purpose of keep imgdata3d is to accelerate simultanoeus evoloving of multiple regions later on
		for (int m=0; m<segPara.regionnum;m++)
		{
			for (j=0; j<sznew[1]; j++)
			for (i=0; i<sznew[0]; i++)
				imgdata3d[m][j][i] = 0;
					
			// compute g
			if (tagModel[m] ==1) // use model 
			{
				for (j=0; j<sznew[1]; j++)
				for (i=0; i<sznew[0]; i++)
				{
					if (labelimgdata3d[m][j][i]>0)
					{
						imgdata3d[m][j][i] = 1/(1+gradientimgdata2d[j][i])*(1-0.9999);
					}
					else
					{
						imgdata3d[m][j][i] = 1/(1+gradientimgdata2d[j][i]);
					}
				}				
			}
			else // do not use model, only use intensity
			{
				for (j=0; j<sznew[1]; j++)
				for (i=0; i<sznew[0]; i++)
				{
					imgdata3d[m][j][i] = 1/(1+gradientimgdata2d[j][i]);
				}
			}
		}

		
		for (int m=0; m<segPara.regionnum;m++)
		{
			printf("\n Segmenting region %d ...\n", segPara.regions[m]);
			
			// reinitialize imgdata2d for each region
			for (j=0; j<sznew[1]; j++)
				for (i=0; i<sznew[0]; i++)
					imgdata2d[j][i] = imgdata3d[m][j][i];
//					
//			// compute g
//			if (tagModel[m] ==1) // use model 
//			{
//				for (j=0; j<sznew[1]; j++)
//				for (i=0; i<sznew[0]; i++)
//				{
//					if (labelimgdata3d[m][j][i]>0)
//					{
//						imgdata2d[j][i] = 1/(1+gradientimgdata2d[j][i])*(1-0.9999);
//					}
//					else
//					{
//						imgdata2d[j][i] = 1/(1+gradientimgdata2d[j][i]);
//					}
//				}				
//			}
//			else // do not use model, only use intensity
//			{
//				for (j=0; j<sznew[1]; j++)
//				for (i=0; i<sznew[0]; i++)
//				{
//					imgdata2d[j][i] = 1/(1+gradientimgdata2d[j][i]);
//				}
//			}
										
			
			//generate initial contour

			for (j=0;j<sznew[1]; j++)
			for (i=0; i<sznew[0]; i++)
			{
				u[j][i] = 0;
			}
				
			printf("Initialize boundary ...\n");
			
			switch (segPara.regions[m])
			{
				case 34: 
						{//left optical lobe
							
							float x1 = sznew[0]/10;
							float x2 = sznew[0]*2/5;
							float y1 = 0;
							float y2 = sznew[1];
							
							float slope = (y2-y1)/(x2-x1);

							for (int yy=0+20; yy<sznew[1]-5; yy++)
							{
								int xx = round((yy-y1)/slope+x1);
								for (i=2; i<xx; i++)
	//								u[m][yy][i] = 1;
									u[yy][i] = 1;
							}
								
						   break;
					   }
				case 1:{ //right optical lobe
						//note that if every slice has the same initial boundary, then should be calculated only once
								
							float x1 = sznew[0]*9/10;
							float x2 = sznew[0]*3/5;
							float y1 = 0;
							float y2 = sznew[1];
							
							float slope = (y2-y1)/(x2-x1);

							for (int yy=0+20; yy<sznew[1]-5; yy++)
							{
								int xx = round((yy-y1)/slope+x1);
								for (i=xx; i<sznew[0]-2; i++)
									u[yy][i] = 1;

							}
							
//							for (j=0; j<sz[1]; j++)
//							for (i=0; i<sz[0]; i++)
//							{
//								outimgdata3d[k][j][i] = u[j][i+marginx];
//							}		
												
							break;
						}
				default:
						{
							for (j=0; j<sznew[1]; j++)
							for (i=0; i<sznew[0]; i++)
							{
								if (labelimgdata3d[m][j][i] > 0)
								{
									utmpdata2d[j][i] = 1;
								}
								else
								{
									utmpdata2d[j][i] = 0;
								}

							}
																					
							if (labelimgdataArea[m]<600)
							{
								bwDilate(utmp,  se1);
							}
							else
							{
								bwDilate(utmp,  se2);
							}	
							
							//assign values
//							int ccc = 0;
							
							for (j=0; j<sznew[1]; j++)
							for (i=0; i<sznew[0]; i++)
							{
								u[j][i]= utmpdata2d[j][i];
//								if (u[j][i]>0)
//									ccc++;
							}
							
//							printf("%d\n", ccc);											
//							break;
						}
			} //switch
			
//			for (j=0; j<sz[1]; j++)
//			for (i=0; i<sz[0]; i++)
//			{
//				outimgdata3d[k][j][i] = u[j][i+marginx];					
//			}
			
			
			printf("Initialize boundary done \n");
			
			printf("Evolving ...\n");
			
			// initialize level set function
			for (j=0; j<sznew[1]; j++)
			for (i=0; i<sznew[0]; i++)
			{
				u[j][i]= c0*2*(0.5-u[j][i]);
				uold[j][i] = u[j][i];
			}

			
			//evolve
			int cnt = 0;
			
			for (int n=0; n<2000; n++)
			{
//				u = evloveLevelSet(u, imgdata2d, para, 1, sz2);	
				
				evloveLevelSet(u, imgdata2d, para, 1, sznew);
				
				// test if the evluation should stop or not
				int differnum = 0;
				int area = 0;
								
				for (j=0;j<sznew[1]; j++)
				for (i=0; i<sznew[0]; i++)
				{

					if (((uold[j][i]>0) && (u[j][i]<0)) || ((uold[j][i]<0) && (u[j][i]>0)))
						differnum++;
					
					if (u[j][i]<=0)
						area++; //pixels inside the segmented region
				}
				
				if (differnum < 3) // consider as no change
				{
					cnt = cnt + 1;
				}
				else
				{
					cnt = 0;
				}
				
				if ((area<labelimgdataArea[m]*0.8)||(cnt >10)) // stop evolvin criteria
				{
//					printf("%d, %f, %d, %d\n", area, labelimgdataArea[m]*0.8, n, cnt);
					break;
				}
			
				// let uold = u
				for (j=0; j<sznew[1]; j++)
				for (i=0; i<sznew[0]; i++)
				{
					uold[j][i] = u[j][i];
				}				

//				int nn = n/10;
//				
//				for (j=0; j<sz[1]; j++)
//				for (i=0; i<sz[0]; i++)
//				{
//					if (u[j][i+marginx]<=0)
//						outimgdata3d[nn][j][i] = 1;
//				}		
				
			} // for n
			
			printf("Evolving done...\n");

//			//rectify evolving result, remove very small regions that are the residue of evolving procedure
//			printf("Rectifying evolving result...\n");
			rectifyShape(ulabel, u, sznew, m, labelimgdata3d, utmpdata2d);
//			printf("Rectifying evolving result done...\n");


			// assign values to the tempoary result image for adding repelling force
			for (j=0; j<sznew[1]; j++)
			for (i=0; i<sznew[0]; i++)
			{
				if (ulabel[j][i]>0)
				{
					resultimgdata3d[m][j][i] =  segPara.regions[m];
				}	
			}
			
		} // for (int m=0; m<segPara.regionnum;m++)
				
		// up to now, the independent evolution of each region is done, 
		// next we need to simultaneously evolve multiple regions that have overlapping with each other

		// initialize uall
		 for (int m=0; m<segPara.regionnum;m++)
		 {
			for (int j=0; j<sznew[1]; j++)
			for (int i=0; i<sznew[0]; i++)
				uall[m][j][i] = c0*2*(0.5-(float)(resultimgdata3d[m][j][i]>0));
		 }
		
		
		//test which regions are overlapping

		 for (int m=0; m<segPara.regionnum;m++)
			for (int pp =0; pp<segPara.regionnum;pp++)
				overlaprgn[m][pp] = 0;
		
		 for (int m=0; m<segPara.regionnum;m++)
		 {
			for (int pp=0; pp<segPara.regionnum; pp++)
			{
				if (pp!=m)
				{
					unsigned char stoptag = 0;
					
					int j=0;
					while ((j<sznew[1])&&(stoptag ==0))
					{
						int i=0;
						while ((i<sznew[0])&&(stoptag ==0))
						{
							if ((resultimgdata3d[m][j][i] >0) && (resultimgdata3d[pp][j][i] >0)) // region pp and m are overlapping
							{
								overlaprgn[m][pp] = 1;
								stoptag = 1;
							}
							else
							{
								i++;
							}
						}
						j++;
					}
				}
//				printf("%d ", overlaprgn[m][pp]);
			}
//			printf("\n");
		}
		
		// simultanously evolve multiple regions
		for (int n=0; n<2000; n++) // number of iterations
		{
		
			// test if the evoluation of this region should stop or not
			// if it does not have overlap with other regions any more, it stops
			
			printf("n = %d\n", n);
			
			//unsigned char evolvetag[segPara.regionnum]; // indicate whether the region should keep on evolving
			unsigned char *evolvetag = new unsigned char [segPara.regionnum]; // indicate whether the region should keep on evolving
			unsigned short int endevolveflag = 0; // indicate whether the entire evolving should end
			
			// update evolvetag

			for (int m=0; m<segPara.regionnum;m++) 
			{
				evolvetag[m] = 0;
			}
			
			for (int m=0; m<segPara.regionnum;m++) 
			{
				for (int pp=0; pp<segPara.regionnum; pp++)
				{
					if (overlaprgn[m][pp] == 1)
					{
						evolvetag[m] = 1;
						break;
					}
					
				}
				
//				printf("%d ", evolvetag[m]);
				endevolveflag += evolvetag[m];
			}
//			printf("\n");
			
		
			// test if the entire evolving should end or not
			if (endevolveflag == 0) // stop evolving all regions, final result
				break;
			
			 for (int m=0; m<segPara.regionnum;m++) //add repelling force for regions that have overlap with others
			 {

//				unsigned char flag = 0;
				if (evolvetag[m]==1) // the mth region should evolve
				{
				
					// generate uneighbor for each region		
					
					for (int j=0; j<sznew[1]; j++)
					for (int i=0; i<sznew[0]; i++)
						uneighbor2d[j][i] = 0;
					
					for (int pp=0; pp<segPara.regionnum; pp++)
					{
						if (overlaprgn[m][pp]==1) // m and pp region have overlapping
						{
//							printf("%d ", overlaprgn[m][pp]);
							//distance transform
							dt2d_binary(resultimgdata1d+pp*sznew[1]*sznew[0], utmpdata1d, pixindex, sznew2d, 1); //utmpdata1d and utmpdata2d point to the same location					
							
							for (int j=0; j<sznew[1]; j++)
							for (int i=0; i<sznew[0]; i++)
								uneighbor2d[j][i] -= sqrt(utmpdata2d[j][i]);
//								uneighbor2d[j][i] += sqrt(utmpdata2d[j][i]);
							
//							flag = 1;
						}
					}
//					printf("\n");
								
					// initialize level set function and gradient image
					for (j=0; j<sznew[1]; j++)
					for (i=0; i<sznew[0]; i++)
					{
						u[j][i]= uall[m][j][i];
						uold[j][i] = u[j][i];
						imgdata2d[j][i] = imgdata3d[m][j][i];
					}
					
					//evolve
					evloveLevelSet(u, uneighbor2d, imgdata2d, para, 1, sznew);

					//update uall for next evoluation
					for (j=0; j<sznew[1]; j++)
					for (i=0; i<sznew[0]; i++)
						uall[m][j][i] = u[j][i];
					
					// rectify shape, remove residual of small isolated regions during evolving
					
					rectifyShape(ulabel, u, sznew, m, labelimgdata3d, utmpdata2d);
					
					//update  resultimgdata3d
											
					for (j=0; j<sznew[1]; j++)
					for (i=0; i<sznew[0]; i++)
					{
						if (ulabel[j][i]>0)
							resultimgdata3d[m][j][i]  = segPara.regions[m]; // this is fine since segmenation masks of different regions sit at different 'depth' of resultimgdata3d 
						else
							resultimgdata3d[m][j][i]  = 0;
							
					}

					//update repelling force tag overlaprgn[m][pp]						
					for (int pp=0; pp<segPara.regionnum; pp++)
					{
						if (overlaprgn[m][pp]==1)
						{

							unsigned char nonoverlapflg = 1;
							unsigned char stoptag = 0;
							
							int j=0;
							
							while ((j<sznew[1])&&(stoptag ==0))
							{
								int i=0;
								while ((i<sznew[0])&&(stoptag ==0))
								{
									if ((resultimgdata3d[m][j][i] > 0) && (resultimgdata3d[pp][j][i] > 0)) // region pp and m are overlapping
									{
										overlaprgn[m][pp] = 1;
										stoptag = 1;
										nonoverlapflg = 0;
									}
									else
										i++;
								}
								j++;
							} //while
							
							if (nonoverlapflg ==1)
								overlaprgn[m][pp] = 0;
						}
					} //for (int pp=0; pp<segPara.regionnum; pp++)
					
				} // if (evolvetag[m]==1)
			
				if (evolvetag) {delete []evolvetag; evolvetag=0;} //added by PHC, 2010-05-20	
			} //for (int m=0; m<segPara.regionnum;m++)
		} //for (int n=0; n<2000; n++)

		// assign output
		for (int m=0; m<segPara.regionnum; m++)
		for (j=0; j<sz[1]; j++)
		for (i=0; i<sz[0]; i++)
		{
			outimgdata3d[k][j][i] = outimgdata3d[k][j][i] + resultimgdata3d[m][j][i+marginx];
//			outimgdata3d[m][j][i] = resultimgdata3d[m][j][i+marginx];

		}
		 
	} // for (k=0;k<sz[2]; k++)
	
									
	// delete pointers

	if (overlaprgn) {delete  []overlaprgn; overlaprgn=0;}  //added by PHC, 2010-05-20
	if (overlaprgn1d) {delete []overlaprgn1d; overlaprgn1d=0;} //added by PHC, 2010-05-20


//	if (labelimgdata3d) {delete labelimgdata3d; labelimgdata3d=0;}
	if (labelimg) {delete labelimg; labelimg=0;}
//	if (imgdata3d) {delete [] imgdata3d; imgdata3d=0;}
//	if (imgdata2d) {delete [] imgdata2d; imgdata2d=0;}
	if (imgdata3d) {delete3dpointer(imgdata3d, sznew[0], sznew[1], segPara.regionnum);}
	if (imgdata2d) {delete2dpointer(imgdata3d, sznew[0], sznew[1]);}
	if (imgdata1d) {delete [] imgdata1d; imgdata1d=0;}
	if (imgdata1dm) {delete [] imgdata1dm; imgdata1dm=0;}	
	if (tag) {delete [] tag; tag=0;}
	if (tagModel) {delete [] tagModel; tagModel=0;}
	if (labelimgdataArea) {delete [] labelimgdataArea; labelimgdataArea=0;}	
	if (se1) {delete se1; se1=0;}		
	if (se2) {delete se2; se2=0;}	
	if (u) delete2dpointer(u, sznew[0], sznew[1]);
	if (u1d) {delete []u1d; u1d=0;} //has been deleted
	
//	if (uold) {delete uold; uold=0;}
	if (uold) delete2dpointer(uold, sznew[0], sznew[1]);
	
	if (uold1d) {delete []uold1d; uold1d=0;}	
//	if (utmpdata2d) {delete utmpdata2d; utmpdata2d=0;} //will be deleted by deleting utmp
//	if (utmpdata1d) {delete utmpdata1d; utmpdata1d=0;} //will be deleted by deleting utmp
	if (utmp) {delete utmp; utmp=0;}
	if (smoothedimgdata3d) {delete3dpointer(smoothedimgdata3d,sz[0],sz[1],sz[2]);}
	if (smoothedimgdata1d) {delete [] smoothedimgdata1d; smoothedimgdata1d=0;}
	if (gradientimgdata2d) {delete2dpointer(gradientimgdata2d, sznew[0], sznew[1]);}
	if (gradientimgdata1d) {delete [] gradientimgdata1d; gradientimgdata1d=0;}
	if (resultimgdata1d) {delete [] resultimgdata1d; resultimgdata1d=0;}
//	if (resultimgdata3d) {delete [] resultimgdata3d; resultimgdata3d=0;}
	if (resultimgdata3d) {delete3dpointer(resultimgdata3d, sznew[0], sznew[1], segPara.regionnum);}

	if (uneighbor1d) {delete [] uneighbor1d; uneighbor1d=0;}
//	if (uneighbor2d) {delete [] uneighbor2d; uneighbor2d=0;}
	if (uneighbor2d) {delete2dpointer(uneighbor2d, sznew[0], sznew[1]);}

	if (pixindex) {delete [] pixindex; pixindex=0;}
	if (ulabel) {delete2dpointer(ulabel, sznew[0], sznew[1]);}
	if (ulabel1d) {delete [] ulabel1d; ulabel1d=0;}
	if (uall) {delete3dpointer(uall, sznew[0], sznew[1], segPara.regionnum);}
	if (uall1d) {delete [] uall1d; uall1d=0;}
	
	return true;
}

bool brainSeg3D(Vol3DSimple <unsigned char> *rawimg3d, Vol3DSimple <unsigned short int> *segimg3d, Vol3DSimple <unsigned short int> *outimg3d, const levelset_segParameter & segPara)
{
	return false; //since originally here is emopty, and it has not been implemented yet by FUhui, so I input false here to
	              // make the MSVC compiler happy. by PHC, 2010-05-20
}

bool braingSegLevelSet(Vol3DSimple <unsigned char> *rawimg3d, Vol3DSimple <unsigned short int> *segimg3d, Vol3DSimple <unsigned short int> *outimg3d, const levelset_segParameter & segPara)
{
	if (!rawimg3d ||!segimg3d || !outimg3d) 
	{
		printf("Invalid parameters to FL_cellseg();\n");
		return false;
	}
	
	//downsample

	if (segPara.dimension == 2) // 2d segmentation
	{
		brainSeg2D(rawimg3d, segimg3d, outimg3d, segPara);
	}
	else
	{ 
		if (segPara.dimension == 3) // 3d segmentation
		{
			brainSeg3D(rawimg3d, segimg3d, outimg3d, segPara);
		}
		else
		{
			printf("Must be either 2D or 3D segmentation!\n");
			return false;
		}
	}
				
	return true;
}