/* minMaxfilterplugin.cpp
 * 2009-08-19: create this program by Yang Yu
 */

// Adapted and upgraded to V3DPluginInterface2_1 by Jianlong Zhou, 2012-04-05
// add dofunc() by Jianlong Zhou, 2012-04-11


#include <QtGui>
#include <iostream>
#include <math.h>
#include <stdlib.h>

#include "minMaxfilterplugin.h"

using namespace std;

#define INF 1E9

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(minMaxfilter, minMaxFilterPlugin)

void processImage(V3DPluginCallback2 &callback, QWidget *parent, unsigned int filterflag);
bool processImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);

template <class T> void minmax_filter(T* data1d,
                     V3DLONG *in_sz,
                     unsigned int Wx,
                     unsigned int Wy,
                     unsigned int Wz,
                     unsigned int c,
                     unsigned int filterflag,
                     T* &outimg);


QStringList minMaxFilterPlugin::menulist() const
{
    return QStringList() << tr("Max Filter")
						 << tr("min Filter")
						 << tr("Max-min Filter")
						 << tr("min-Max Filter")
						 << tr("about this plugin");
}

void minMaxFilterPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
     	//choosing filter
	unsigned int filterflag = 0; //Gaussian 0 Max 1 min 2 Max-min 3 min-Max 4

	if(menu_name == tr("Max Filter"))
     {
		filterflag = 1;
          processImage(callback,parent, filterflag);
     }
	else if(menu_name == tr("min Filter"))
	{
          filterflag = 2;
          processImage(callback,parent, filterflag);
     }
	else if(menu_name == tr("Max-min Filter"))
	{
          filterflag = 3;
          processImage(callback,parent, filterflag);
     }
	else if(menu_name == tr("min-Max Filter"))
	{
          filterflag = 4;
          processImage(callback,parent, filterflag);
     }
     else if(menu_name == tr("about this plugin"))
	{
		QMessageBox::information(parent, "",
               QString("min/Max Filter Plugin Demo (2009-Aug-09) developed by Yang Yu, Jianlong Zhou. (Peng Lab, Janelia Research Farm Campus, HHMI)"));
		return;
	}

}


QStringList minMaxFilterPlugin::funclist() const
{
	return QStringList()
		<<tr("mmf")
		<<tr("help");
}


bool minMaxFilterPlugin::dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
{
     if (func_name == tr("mmf"))
	{
        return processImage(callback, input, output);
	}
	else if(func_name == tr("help"))
	{
		cout<<"Usage : v3d -x minMaxfilter -f mmf -i <inimg_file> -o <outimg_file> -p <wx> <wy> <wz> <ch> <filterflag>"<<endl;
		cout<<endl;
		cout<<"wx          filter window radius size (pixel #) in x direction, window size is 2*wx+1, default 3"<<endl;
		cout<<"wy          filter window radius size (pixel #) in y direction, window size is 2*wy+1, default 3"<<endl;
		cout<<"wz          filter window radius size (pixel #) in z direction, window size is 2*wz+1, default 3"<<endl;
		cout<<"ch           the input channel value, default 1 and start from 1, default 1"<<endl;
		cout<<"filterflag  filter method flag, 1: Max Filter, 2: min Filter, 3: Max-min Filter, 4: min-Max Filter, default 1"<<endl;
		cout<<endl;
		cout<<"e.g. v3d -x minMaxfilter -f mmf -i input.raw -o output.raw -p 3 3 3 1 1"<<endl;
		cout<<endl;
		return true;
	}
}

bool processImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to minmax filter"<<endl;
	if (output.size() != 1) return false;

	unsigned int Wx=7, Wy=7, Wz=3, ch=1, filterflag=1;
     if (input.size()>=2)
     {
          vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
          if(paras.size() >= 1) Wx = atoi(paras.at(0));
          if(paras.size() >= 2) Wy = atoi(paras.at(1));
          if(paras.size() >= 3) Wz = atoi(paras.at(2));
          if(paras.size() >= 4) ch = atoi(paras.at(3));
          if(paras.size() >= 5) filterflag = atoi(paras.at(4));
	}

	char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
	char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
	cout<<"Wx = "<<Wx<<endl;
     cout<<"Wy = "<<Wy<<endl;
	cout<<"Wz = "<<Wz<<endl;
     cout<<"ch = "<<ch<<endl;
     cout<<"filterflag = "<<filterflag<<endl;
	cout<<"inimg_file = "<<inimg_file<<endl;
	cout<<"outimg_file = "<<outimg_file<<endl;

	unsigned char * data1d = 0,  * outimg1d = 0;
    V3DLONG in_sz[4];

     unsigned int c = ch;//-1;

	int datatype;
    if(!simple_loadimage_wrapper(callback, inimg_file, data1d, in_sz, datatype))
     {
          cerr<<"load image "<<inimg_file<<" error!"<<endl;
          return false;
     }

	//input
     void* outimg = 0;

     switch (datatype)
     {
          case 1: minmax_filter(data1d, in_sz, Wx, Wy, Wz, c, filterflag, (unsigned char*&)outimg); break;
          case 2: minmax_filter((unsigned short int*)data1d, in_sz, Wx, Wy, Wz, c, filterflag, (unsigned short int*&)outimg); break;
          case 4: minmax_filter((float *)data1d, in_sz, Wx, Wy, Wz, c, filterflag, (float*&)outimg); break;
          default:
               v3d_msg("Invalid datatype.");
               if (data1d) {delete []data1d; data1d=0;}
               return false;
     }

     // save image
     in_sz[3]=1;
     simple_saveimage_wrapper(callback, outimg_file, (unsigned char *)outimg, in_sz, datatype);

     if(outimg) {delete []outimg; outimg =0;}
     if (data1d) {delete []data1d; data1d=0;}

     return true;
}





void processImage(V3DPluginCallback2 &callback, QWidget *parent, unsigned int filterflag)
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



    //define datatype here
    //


	//input
	bool ok1, ok2, ok3, ok4;
	unsigned int Wx=1, Wy=1, Wz=1, c=1;

	Wx = QInputDialog::getInteger(parent, "Window X ",
											   "Enter radius (window size is 2*radius+1):",
											   3, 1, N, 1, &ok1);

	if(ok1)
	{
		Wy = QInputDialog::getInteger(parent, "Window Y",
											   "Enter radius (window size is 2*radius+1):",
											   3, 1, M, 1, &ok2);
	}
	else
		return;

	if(ok2)
	{
		Wz = QInputDialog::getInteger(parent, "Window Z",
											   "Enter radius (window size is 2*radius+1):",
											   3, 1, P, 1, &ok3);
	}
	else
		return;

	if(sc==1)
	{
		c=1;
		ok4=true;
	}
	else
	{
		if(ok3)
		{
			c = QInputDialog::getInteger(parent, "Channel",
												  "Enter channel NO:",
												  1, 1, sc, 1, &ok4);
		}
		else
			return;
	}


     // filter
     V3DLONG in_sz[4];
     in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = sc;

    ImagePixelType pixeltype = p4DImage->getDatatype();
    void* outimg = 0;
    switch (pixeltype)
    {
        case V3D_UINT8: minmax_filter(data1d, in_sz, Wx, Wy, Wz, c, filterflag, (unsigned char* &)outimg); break;
        case V3D_UINT16: minmax_filter((unsigned short int *)data1d, in_sz, Wx, Wy, Wz, c, filterflag, (unsigned short int* &)outimg); break;
        case V3D_FLOAT32: minmax_filter((float *)data1d, in_sz, Wx, Wy, Wz, c, filterflag, (float* &)outimg);break;
        default: v3d_msg("Invalid data type. Do nothing."); return;
    }

     // display
     Image4DSimple * new4DImage = new Image4DSimple();
     new4DImage->setData((unsigned char *)outimg, N, M, P, 1, pixeltype);
     v3dhandle newwin = callback.newImageWindow();
     callback.setImage(newwin, new4DImage);
     callback.setImageName(newwin, "minmax filter result");
     callback.updateImageWindow(newwin);

}



template <class T> void minmax_filter(T* data1d,
                     V3DLONG *in_sz,
                     unsigned int Wx,
                     unsigned int Wy,
                     unsigned int Wz,
                     unsigned int c,
                     unsigned int filterflag,
                     T* &outimg)
{
    if (!data1d || !in_sz || in_sz[0]<=0 || in_sz[1]<=0 || in_sz[2]<=0 || in_sz[3]<=0 || outimg || filterflag<1 || filterflag>4)
    {
        v3d_msg("Invalid parameters to gaussian_filter().", 0);
        return;
    }

     V3DLONG N = in_sz[0];
     V3DLONG M = in_sz[1];
     V3DLONG P = in_sz[2];
     V3DLONG sc = in_sz[3];
     V3DLONG pagesz = N*M*P;

     T maxfl = 0, minfl = INF;

    //filtering
	V3DLONG offsetc = (c-1)*pagesz;

	if (filterflag == 1 || filterflag == 2 || filterflag == 3 ||filterflag == 4) // delete &&ok4
	{
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

		//Filtering
		//
		//min Max filtering
		for(V3DLONG iz = 0; iz < P; iz++)
		{
			V3DLONG offsetk = iz*M*N;
			for(V3DLONG iy = 0; iy < M; iy++)
			{
				V3DLONG offsetj = iy*N;
				for(V3DLONG ix = 0; ix < N; ix++)
				{
					maxfl = 0; minfl = INF;

					V3DLONG xb = ix-Wx; if(xb<0) xb = 0;
					V3DLONG xe = ix+Wx; if(xe>=N-1) xe = N-1;
					V3DLONG yb = iy-Wy; if(yb<0) yb = 0;
					V3DLONG ye = iy+Wy; if(ye>=M-1) ye = M-1;
					V3DLONG zb = iz-Wz; if(zb<0) zb = 0;
					V3DLONG ze = iz+Wz; if(ze>=P-1) ze = P-1;

					for(V3DLONG k=zb; k<=ze; k++)
					{
						V3DLONG offsetkl = k*M*N;
						for(V3DLONG j=yb; j<=ye; j++)
						{
							V3DLONG offsetjl = j*N;
							for(V3DLONG i=xb; i<=xe; i++)
							{
								T dataval = data1d[ offsetc + offsetkl + offsetjl + i];

								if(maxfl<dataval) maxfl = dataval;
								if(minfl>dataval) minfl = dataval;
							}
						}
					}

					//set value
					V3DLONG index_pim = offsetk + offsetj + ix;

					if(filterflag == 1 || filterflag == 3)
					{
						pImage[index_pim] = maxfl;
					}
					else if(filterflag == 2 || filterflag == 4)
					{
						pImage[index_pim] = minfl;
					}

				}
			}
		}

		//Max-min || min-Max
		if(filterflag == 3 || filterflag ==4) //ok4 &&
		{

			//   Filtering
			for(V3DLONG iz = 0; iz < P; iz++)
			{
				V3DLONG offsetk = iz*M*N;
				for(V3DLONG iy = 0; iy < M; iy++)
				{
					V3DLONG offsetj = iy*N;
					for(V3DLONG ix = 0; ix < N; ix++)
					{
						maxfl = 0; minfl = INF;

						V3DLONG xb = ix-Wx; if(xb<0) xb = 0;
						V3DLONG xe = ix+Wx; if(xe>=N-1) xe = N-1;
						V3DLONG yb = iy-Wy; if(yb<0) yb = 0;
						V3DLONG ye = iy+Wy; if(ye>=M-1) ye = M-1;
						V3DLONG zb = iz-Wz; if(zb<0) zb = 0;
						V3DLONG ze = iz+Wz; if(ze>=P-1) ze = P-1;

						for(V3DLONG k=zb; k<=ze; k++)
						{
							V3DLONG offsetkl = k*M*N;
							for(V3DLONG j=yb; j<=ye; j++)
							{
								V3DLONG offsetjl = j*N;
								for(V3DLONG i=xb; i<=xe; i++)
								{
									T dataval = data1d[ offsetc + offsetkl + offsetjl + i];

									if(maxfl<dataval) maxfl = dataval;
									if(minfl>dataval) minfl = dataval;
								}
							}
						}

						//set value
						V3DLONG index_pim = offsetk + offsetj + ix;

						if(filterflag == 4)
						{
							pImage[index_pim] = maxfl;
						}
						else if(filterflag == 3)
						{
							pImage[index_pim] = minfl;
						}

					}
				}
			}
		}

          outimg = pImage;

		// //rescaling for display
		// for(V3DLONG k=0; k<P; k++)
		// {
		// 	V3DLONG offsetk = k*M*N;
		// 	for(V3DLONG j=0; j<M; j++)
		// 	{
		// 		V3DLONG offsetj = j*N;
		// 		for(V3DLONG i=0; i<N; i++)
		// 		{
		// 			V3DLONG indLoop = offsetk + offsetj + i;

		// 			data1d[offsetc + indLoop] = pImage[indLoop];
		// 		}
		// 	}
		// }

		// //de-alloc
		// if (pImage) {delete []pImage; pImage=0;}
	}


}

