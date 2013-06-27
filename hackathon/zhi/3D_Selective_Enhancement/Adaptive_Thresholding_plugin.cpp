/* Adaptive_Thresholding_plugin.cpp
 * Detect fiber
 * 2013-06-17 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "Adaptive_Thresholding_plugin.h"
#include <QtGui>
#include <fstream>
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <vector>
#include "stackutil.h"
#include <Eigen/Dense>

/*#define WANT_STREAM       // include iostream and iomanipulators
#include "/home/zhiz/work/v3d_external/v3d_main/jba/newmat11/newmatap.h"
#include "/home/zhiz/work/v3d_external/v3d_main/jba/newmat11/newmatio.h"
#ifdef use_namespace
using namespace RBD_LIBRARIES;
#endif
*/

using namespace Eigen;
using namespace std;
Q_EXPORT_PLUGIN2(Adaptive_Thresholding, adpThresholding);

void processImage(V3DPluginCallback2 &callback, QWidget *parent);
bool processImage(const V3DPluginArgList & input, V3DPluginArgList & output);

template <class T> void AdpThresholding(T* data1d,
                     V3DLONG *in_sz,
                     unsigned int c,
                     T* &outimg1,T* &outimg2);
int swapthree(float& dummya, float& dummyb, float& dummyc);
 
QStringList adpThresholding::menulist() const
{
	return QStringList() 
		<<tr("AdpTheresholding")
		<<tr("about");
}

QStringList adpThresholding::funclist() const
{
	return QStringList()
		<<tr("AdpTheresholding")
		<<tr("help");
}

void adpThresholding::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("AdpTheresholding"))
	{
		processImage(callback,parent);
	}
	else
	{
		v3d_msg(tr("Detect fiber. "
			"Developed by Zhi Zhou, 2013-06-17"));
	}
}

bool adpThresholding::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("AdpTheresholding"))
	{
	}
	else if (func_name == tr("help"))
	{
	}
	else return false;
}

void processImage(V3DPluginCallback2 &callback, QWidget *parent)
{
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
    //V3DLONG totalpxls = p4DImage->getTotalBytes();
    V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

	//input
	bool ok4;
	unsigned int c=1;

	c = QInputDialog::getInteger(parent, "Channel",
												  "Enter channel NO:",
												  1, 1, sc, 1, &ok4);


     // filter
     V3DLONG in_sz[4];
     in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[4] = sc;

    ImagePixelType pixeltype = p4DImage->getDatatype();
    void* outimg1 = 0; 
    void* outimg2 = 0;	
    switch (pixeltype)
    {
        case V3D_UINT8: AdpThresholding(data1d, in_sz, c,(unsigned char* &)outimg1,(unsigned char* &)outimg2); break;
        case V3D_UINT16: AdpThresholding((unsigned short int *)data1d, in_sz, c, (unsigned short int* &)outimg1,(unsigned short int* &)outimg2); break;
        case V3D_FLOAT32: AdpThresholding((float *)data1d, in_sz, c, (float* &)outimg1,(float* &)outimg2);break;
        default: v3d_msg("Invalid data type. Do nothing."); return;
    }
	
  
    
     // display
     V3DLONG DS = 1;
     Image4DSimple * new4DImage = new Image4DSimple();
     new4DImage->setData((unsigned char *)outimg1, V3DLONG(N/DS), V3DLONG(M/DS), V3DLONG(P/DS), 1, pixeltype);
     v3dhandle newwin = callback.newImageWindow();
     callback.setImage(newwin, new4DImage);
     callback.setImageName(newwin, "Downsampled result");
     callback.updateImageWindow(newwin);


     Image4DSimple * new4DImage2 = new Image4DSimple();	
     new4DImage2->setData((unsigned char *)outimg2, V3DLONG(N/DS), V3DLONG(M/DS), V3DLONG(P/DS), 1, pixeltype);
     v3dhandle newwin2 = callback.newImageWindow();
     callback.setImage(newwin2, new4DImage2);
     callback.setImageName(newwin2, "Adaptive Thresholding result");
     callback.updateImageWindow(newwin2);

}



template <class T> void AdpThresholding(T* data1d,
                     V3DLONG *in_sz,
                     unsigned int c,
                     T* &outimg1, T* &outimg2)
{
    
  
     V3DLONG N = in_sz[0];
     V3DLONG M = in_sz[1];
     V3DLONG P = in_sz[2];
     V3DLONG sc = in_sz[3];
     V3DLONG DS = 1;
     V3DLONG WS = 1;
    // V3DLONG pagesz = N*M*P;
     V3DLONG pagesz = (N/DS)*(M/DS)*(P/DS);
     
     V3DLONG DSN = in_sz[0]/DS;
     V3DLONG DSM = in_sz[1]/DS;
     V3DLONG DSP = in_sz[2]/DS;	
     int SUM = 0;
             
     //filtering
	V3DLONG offsetc = (c-1)*pagesz;

		//declare temporary pointer
		T *pImage = new T [pagesz];
		if (!pImage)
		{
			printf("Fail to allocate memory.\n");
			return;
		 }
		 else
		 {
			for(V3DLONG i=0; i<pagesz; i++)
				pImage[i] = 0;
		  }

		//Thresholding
		V3DLONG dsiz=0;
	 	for(V3DLONG iz = 0; iz < P; iz = iz+DS)
		{
			V3DLONG offsetk = iz*M*N;
			V3DLONG dsoffsetk = dsiz*DSM*DSN;
			V3DLONG dsiy=0;
			for(V3DLONG iy = 0; iy < M; iy = iy+DS)
			{
				V3DLONG offsetj = iy*N;
				V3DLONG dsoffsetj = dsiy*DSN;
				V3DLONG dsix=0;
				for(V3DLONG ix = 0; ix < N; ix = ix+DS)
				{
					V3DLONG index_pim = offsetk + offsetj + ix;
					V3DLONG DSindex_pim = dsoffsetk+dsoffsetj+dsix;
 					SUM = 0;
				        if (DSindex_pim <= pagesz)
					{
						for(V3DLONG k=iz; k< iz+DS; k++)
						{
							V3DLONG offsetkl = k*M*N;
							for(V3DLONG j=iy; j< iy+DS; j++)
							{
								V3DLONG offsetjl = j*N;
								for(V3DLONG i=ix; i< ix+DS; i++)
								{
									T dataval = data1d[ offsetc + offsetkl + offsetjl + i];
									SUM += dataval;
									//if(SUM<dataval) SUM = dataval;
									//SUM++;

								}
							}
						}
						//printf("sum is %d\n",SUM);	
						pImage[DSindex_pim] = SUM/(DS*DS*DS);
						dsix++;
						
					}
				      //  else
					//printf("%d %d %d\n", iz,iy,ix);	
					
				}
				dsiy++;
			}
			dsiz++;
			
		}
	   	
		T *pImage2 = new T [pagesz];
		if (!pImage2)
		{
			printf("Fail to allocate memory.\n");
			return;
		 }
		 else
		 {
			for(V3DLONG i=0; i<pagesz; i++)
				pImage2[i] = 0;
		  }

		 
		 //outimg = pImage;

		for(V3DLONG dsiz = WS; dsiz < DSP-WS; dsiz++)
		{
			V3DLONG dsoffsetk = dsiz*DSM*DSN;
			for(V3DLONG dsiy = WS; dsiy < DSM-WS; dsiy++)
			{
				V3DLONG dsoffsetj = dsiy*DSN;
			 	for(V3DLONG dsix = WS; dsix < DSN-WS; dsix++)
				{
					//double fx = 0.5*(pImage[dsoffsetk+dsoffsetj+dsix+WS]-pImage[dsoffsetk+dsoffsetj+dsix-WS]);
					//double fy = 0.5*(pImage[dsoffsetk+(dsiy+WS)*DSN+dsix]-pImage[dsoffsetk+(dsiy-WS)*DSN+dsix]);
					//double fz = 0.5*(pImage[(dsiz+WS)*DSM*DSN+dsoffsetj+dsix]-pImage[(dsiz-WS)*DSM*DSN+dsoffsetj+dsix]);
					//Seletive approach
					float  fxx = pImage[dsoffsetk+dsoffsetj+dsix+WS]+ pImage[dsoffsetk+dsoffsetj+dsix-WS]- 2*pImage[dsoffsetk+dsoffsetj+dsix];
					float fyy = pImage[dsoffsetk+(dsiy+WS)*DSN+dsix]+pImage[dsoffsetk+(dsiy-WS)*DSN+dsix]-2*pImage[dsoffsetk+dsoffsetj+dsix];
					float fzz = pImage[(dsiz+WS)*DSM*DSN+dsoffsetj+dsix]+pImage[(dsiz-WS)*DSM*DSN+dsoffsetj+dsix]- 2*pImage[dsoffsetk+dsoffsetj+dsix];

					float fxy = 0.25*(pImage[dsoffsetk+(dsiy+WS)*DSN+dsix+WS]+pImage[dsoffsetk+(dsiy-WS)*DSN+dsix-WS]-pImage[dsoffsetk+(dsiy+WS)*DSN+dsix-WS]-pImage[dsoffsetk+(dsiy-WS)*DSN+dsix+WS]);
					float fxz = 0.25*(pImage[(dsiz+WS)*DSM*DSN+dsoffsetj+dsix+WS]+pImage[(dsiz-WS)*DSM*DSN+dsoffsetj+dsix-WS]-pImage[(dsiz+WS)*DSM*DSN+dsoffsetj+dsix-WS]-pImage[(dsiz-WS)*DSM*DSN+dsoffsetj+dsix+WS]);
					float fyz = 0.25*(pImage[(dsiz+WS)*DSM*DSN+(dsiy+WS)*DSN+dsix]+pImage[(dsiz-WS)*DSM*DSN+(dsiy-WS)*DSN+dsix]-pImage[(dsiz+WS)*DSM*DSN+(dsiy-WS)*DSN+dsix]-pImage[(dsiz-WS)*DSM*DSN+(dsiy+WS)*DSN+dsix]);
					 
 				 	Matrix3f A;
					A << fxx,fxy,fxz,fxy,fyy,fyz,fxz,fyz,fzz;	
  					SelfAdjointEigenSolver<Matrix3f> eigensolver(A, false);
       				//	if (eigensolver.info() != Success) abort();
					/*SymmetricMatrix Cov_Matrix(3);
					Cov_Matrix.Row(1) << fxx;
					Cov_Matrix.Row(2) << fxy << fyy; 
					Cov_Matrix.Row(3) << fxz << fyz <<fzz; 

					DiagonalMatrix DD;
					//cout << "Matrix" << endl;
					//cout << Cov_Matrix << endl <<endl;

					EigenValues(Cov_Matrix,DD);
					//cout << "EigenValues" << endl;
					//cout <<  DD << endl <<endl;*/


					{
						float a1 = eigensolver.eigenvalues()(0);
	   					float a2 = eigensolver.eigenvalues()(1);
						float a3 = eigensolver.eigenvalues()(2);
	   					swapthree(a1, a2, a3);
						//printf("%f,%f,%f,%f,%f,%f,%f,%f,%f\n\n\n\n",fxx,fyy,fzz,fxy,fxz,fyz,a1,a2,a3);
						float output1 = 0;
						float output2 = 0;	
						if(a1<0 && a2 < 0)
						{	
							//pImage2[dsoffsetk+dsoffsetj+dsix] = abs(a2)*(abs(a2)-abs(a3))/abs(a1);
							output1 =  abs(a2)*(abs(a2)-abs(a3))/abs(a1);
							//T output = abs(a1)/abs(a2);	
							
						//printf("%f %f %f %d\n", a1,a2,a3,output);
							//if(dsix ==96 && dsiy == 37 && dsiz == 6)
							if(a3 < 0) output2 = pow(abs(a3),2)/abs(a1);
							
						}

						pImage2[dsoffsetk+dsoffsetj+dsix]=sqrt(pow(output1,2)+pow(output2,2));

					}
					
				}
			}
					
		}
		outimg1 = pImage;
		outimg2 = pImage2;
}

int swapthree(float& dummya, float& dummyb, float& dummyc)
{
 
    if ( (abs(dummya) >= abs(dummyb)) && (abs(dummyb) >= abs(dummyc)) )
    {
        return 1;
    }
 
 
    else if ( (abs(dummya) >= abs(dummyc)) && (abs(dummyc) >= abs(dummyb)) )
 
    {
        float temp = dummyb;
        dummyb = dummyc;
        dummyc = temp;
 
        return 1;
    }
 
    else if ( (abs(dummyb) >= abs(dummya)) && (abs(dummya) >= abs(dummyc)) )
    {
        float temp = dummya;
        dummya = dummyb;
        dummyb = temp;
 
        return 1;
    }
 
 
    else if ( (abs(dummyb) >= abs(dummyc)) && (abs(dummyc) >= abs(dummya)) )
    {
        float temp = dummya;
        dummya = dummyb;
        dummyb = dummyc;
        dummyc = temp;
 
        return 1;
    }
 
 
    else if ( (abs(dummyc) >= abs(dummya)) && (abs(dummya) >= abs(dummyb)) )
    {
        float temp = dummya;
        dummya = dummyc;
        dummyc = dummyb;
        dummyb = temp;
 
        return 1;
    }
 
 
    else if ( (abs(dummyc) >= abs(dummyb)) && (abs(dummyb) >= abs(dummya)) )
    {
        float temp = dummyc;
        dummyc = dummya;
        dummya = temp;
 
        return 1;
    }
 
 
    else
       {
           return 0;
       }
 
}
