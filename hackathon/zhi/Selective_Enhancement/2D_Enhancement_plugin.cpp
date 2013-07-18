/* 2D_Enhancement_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2013-06-26 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "2D_Enhancement_plugin.h"
#include <QtGui>
#include <fstream>
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <vector>
#include "stackutil.h"
#include <Eigen/Dense>

using namespace Eigen;
using namespace std;

Q_EXPORT_PLUGIN2(2D_Enhancement, Enhancement);
void processImage(V3DPluginCallback2 &callback, QWidget *parent);
bool processImage(const V3DPluginArgList & input, V3DPluginArgList & output);
template <class T> void selectiveEnhance(T* data1d,
                     V3DLONG *in_sz,
                     unsigned int c,
                     unsigned int WS,
                     T* &outimg);
 
QStringList Enhancement::menulist() const
{
	return QStringList() 
		<<tr("selectiveEnhance")
		<<tr("about");
}

QStringList Enhancement::funclist() const
{
	return QStringList()
		<<tr("selectiveEnhance")
		<<tr("help");
}

void Enhancement::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("selectiveEnhance"))
	{
		processImage(callback,parent);
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2013-06-26"));
	}
}

bool Enhancement::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("selectiveEnhance"))
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
    bool ok4,ok1;
    unsigned int c = 1, w = 1;


	c = QInputDialog::getInteger(parent, "Channel",
												  "Enter channel NO:",
												  1, 1, sc, 1, &ok4);
    if(ok4)
    {

        w = QInputDialog::getInteger(parent, "Window size",
                                          "Enter window size  (# voxels):",
                                            1, 1, M, 1, &ok1);
    }
    else
        return;

     // filter
     V3DLONG in_sz[4];
     in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[4] = sc;

    ImagePixelType pixeltype = p4DImage->getDatatype();
    void* outimg = 0; 
    switch (pixeltype)
    {
        case V3D_UINT8: selectiveEnhance(data1d, in_sz, c,w,(unsigned char* &)outimg); break;
        case V3D_UINT16: selectiveEnhance((unsigned short int *)data1d, in_sz, c,w, (unsigned short int* &)outimg); break;
        case V3D_FLOAT32: selectiveEnhance((float *)data1d, in_sz, c,w, (float* &)outimg);break;
        default: v3d_msg("Invalid data type. Do nothing."); return;
    }
	
  
    
     // display
     Image4DSimple * new4DImage = new Image4DSimple();
     new4DImage->setData((unsigned char *)outimg, V3DLONG(N), V3DLONG(M), V3DLONG(P), 1, pixeltype);
     v3dhandle newwin = callback.newImageWindow();
     callback.setImage(newwin, new4DImage);
     callback.setImageName(newwin, "Enhancement result");
     callback.updateImageWindow(newwin);

}



template <class T> void selectiveEnhance(T* data1d,
                     V3DLONG *in_sz,
                     unsigned int c,
                     unsigned int WS,
                     T* &outimg)
{
    
  
     V3DLONG N = in_sz[0];
     V3DLONG M = in_sz[1];
     V3DLONG P = in_sz[2];
     V3DLONG sc = in_sz[3];
     V3DLONG pagesz = N*M*P;
    // V3DLONG WS = 1;


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
		for(V3DLONG iy = WS; iy < M-WS; iy++)
		{
			V3DLONG offsetj = iy*N;
			for(V3DLONG ix = WS; ix < N-WS; ix++)
			{

				float  fxx = data1d[offsetj+ix+WS]+ data1d[offsetj+ix-WS]- 2*data1d[offsetj+ix];
				float  fyy = data1d[(iy+WS)*N+ix]+data1d[(iy-WS)*N+ix]-2*data1d[offsetj+ix];
				float  fxy = 0.25*(data1d[(iy+WS)*N+ix+WS]+data1d[(iy-WS)*N+ix-WS]-data1d[(iy+WS)*N+ix-WS]-data1d[(iy-WS)*N+ix+WS]);
				/*float	K = 0.5*(fxx+fyy);
				float 	Q = sqrt(fxx*fyy-fxy*fxy); 	
		   		float 	a1 = K + sqrt(K*K-Q*Q);
				float 	a2 = K - sqrt(K*K-Q*Q);	
*/
 				Matrix2f A;
				A << fxx,fxy,fxy,fyy;	
  				SelfAdjointEigenSolver<Matrix2f> eigensolver(A, false);
                                float a1 = eigensolver.eigenvalues()(0);
	   			float a2 = eigensolver.eigenvalues()(1);

			 	if(abs(a1)<abs(a2)) 
				{	
						float temp = a2;
					        a2 = a1;
						a1 = temp;
				}
				float output1 = 0;
				float output2 = 0;			
				if(a1<0)
				{	
				 	output1 = abs(a1)-abs(a2);
		 			//pImage[offsetj+ix]=output;
				//	if(ix == 71 && iy == 46)
				//   	 printf("fxx = %f, fyy= %f,fxy = %f, a1 = %f,a2 = %f, value = %f\n",fxx,fyy,fxy,a1,a2,output);	
				}
				if(a1<0 && a2<0) 
				{	
				 	output2 = 0;// abs(a2)*abs(a2)/abs(a1);
		 			

				}
					pImage[offsetj+ix]=sqrt(pow(output1,2)+pow(output2,2));
			}
		}
					
	
		outimg = pImage;
		
}


