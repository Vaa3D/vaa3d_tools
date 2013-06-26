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
//include <Eigen/Dense>

#define WANT_STREAM       // include iostream and iomanipulators
#include "../../../../v3d_external/v3d_main/jba/newmat11/newmatap.h"
#include "../../../../v3d_external/v3d_main/jba/newmat11/newmatio.h"
#ifdef use_namespace
using namespace RBD_LIBRARIES;
#endif


//using namespace Eigen;
using namespace std;
Q_EXPORT_PLUGIN2(Adaptive_Thresholding, adpThresholding);

void processImage(V3DPluginCallback2 &callback, QWidget *parent);
bool processImage(const V3DPluginArgList & input, V3DPluginArgList & output);

template <class T> void AdpThresholding(T* data1d,
                     V3DLONG *in_sz,
                     unsigned int c,
                     T* &outimg1,T* &outimg2);

 
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
     V3DLONG WS = 2;
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

		T *pImage3 = new T[pagesz];
		if (!pImage3)
		{
			printf("Fail to allocate memory.\n");
			return;
		 }
		 else
		 {
			for(int i=0; i<pagesz; i++)
				pImage3[i] = 0;
		  }
		 //outimg = pImage;



		double maxNum = 0;
		DiagonalMatrix DDMax;
		for(V3DLONG dsiz = WS; dsiz < DSP-WS; dsiz++)
		{
			V3DLONG dsoffsetk = dsiz*DSM*DSN;
			for(V3DLONG dsiy = WS; dsiy < DSM-WS; dsiy++)
			{
				V3DLONG dsoffsetj = dsiy*DSN;
			 	for(V3DLONG dsix = WS; dsix < DSN-WS; dsix++)
				{
					
					V3DLONG xb=dsix-WS;
					V3DLONG xe=dsix+WS; 
					V3DLONG yb=dsiy-WS; 
					V3DLONG ye=dsiy+WS; 
					V3DLONG zb=dsiz-WS; 
					V3DLONG ze=dsiz+WS; 

					V3DLONG i,j,k;
					double MEAN = 0, NonZero = 0;	
					for (k=zb;k<=ze;k++)
					{
						V3DLONG offsetkl = k*DSM*DSN;
						for (j=yb;j<=ye;j++)
						{
							V3DLONG offsetjl = j*DSN;
							for (i=xb;i<=xe;i++)
							{
								MEAN = double(pImage[offsetkl + offsetjl + i])+MEAN;
								if(pImage[offsetkl + offsetjl + i]>0)
									NonZero++;
							}
						}
					}
					if(NonZero>0)
					MEAN = MEAN/NonZero;
					//MEAN = 0;
					//printf("MEAN is %f\n",MEAN);
					//MEAN = 60;
					double w;
					
					double xm=0,ym=0,zm=0, s=0;
					for (k=zb;k<=ze;k++)
					{
						V3DLONG offsetkl = k*DSM*DSN;
						for (j=yb;j<=ye;j++)
						{
							V3DLONG offsetjl = j*DSN;
							for (i=xb;i<=xe;i++)
							{
								w = double(pImage[offsetkl + offsetjl + i])-MEAN;
								if(w<0) w=0;
								xm += w*i;
								ym += w*j;
								zm += w*k;
								s += w;
							}
						}
					}
					if(s>0)
					{
						xm /= s; ym /=s; zm /=s;	

						double cc11=0, cc12=0, cc13=0, cc22=0, cc23=0, cc33=0;
						double dfx, dfy, dfz;
						for (k=zb;k<=ze;k++)
						{
							dfz = double(k)-zm;
							V3DLONG offsetkl = k*DSM*DSN;
							for (j=yb;j<=ye;j++)
							{
								dfy = double(j)-ym;
								V3DLONG offsetjl = j*DSN;
								for (i=xb;i<=xe;i++)
								{
									dfx = double(i)-xm;
									w = double(pImage[offsetkl + offsetjl + i]-MEAN);
									if(w<0) w = 0;
									cc11 += w*dfx*dfx;
									cc12 += w*dfx*dfy;
									cc13 += w*dfx*dfz;
									cc22 += w*dfy*dfy;
									cc23 += w*dfy*dfz;
									cc33 += w*dfz*dfz;
								}
							}
						}

						cc11 /= s; 	cc12 /= s; 	cc13 /= s; 	cc22 /= s; 	cc23 /= s; 	cc33 /= s;
						try 
						{	
							//then find the eigen vector
							SymmetricMatrix Cov_Matrix(3);
							Cov_Matrix.Row(1) << cc11;
							Cov_Matrix.Row(2) << cc12 << cc22;
							Cov_Matrix.Row(3) << cc13 << cc23 << cc33;

							DiagonalMatrix DD;
							EigenValues(Cov_Matrix,DD);

							//output the result
							double pc1 = DD(3);
							double pc2 = DD(2);
							double pc3 = DD(1);
							pImage2[dsoffsetk+dsoffsetj+dsix] = pc1/pc2;
								
											

						}
						catch (...)
						{
							
						}
					}
					if (pImage[dsoffsetk+dsoffsetj+dsix]-MEAN>0)
					pImage3[dsoffsetk+dsoffsetj+dsix] = pImage[dsoffsetk+dsoffsetj+dsix]-MEAN;




				}
			}
					
		}

		/*for(V3DLONG dsiz = WS; dsiz < DSP-WS; dsiz++)
		{
			V3DLONG dsoffsetk = dsiz*DSM*DSN;
			for(V3DLONG dsiy = WS; dsiy < DSM-WS; dsiy++)
			{
				V3DLONG dsoffsetj = dsiy*DSN;
			 	for(V3DLONG dsix = WS; dsix < DSN-WS; dsix++)
				{
					
					pImage3[dsoffsetk+dsoffsetj+dsix] = 255*(pImage3[dsoffsetk+dsoffsetj+dsix]/maxNum);
				}
			}
		}*/
		outimg1 = pImage3;
		outimg2 = pImage2;

}


