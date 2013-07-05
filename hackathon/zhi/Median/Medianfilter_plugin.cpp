/* Medianfilter_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * Revised from a previous minmaxfiltering plugin
 * 2013-06-10 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "Medianfilter_plugin.h"
#include <QtGui>
#include <iostream>
#include <math.h>
#include <stdlib.h>

#include "stackutil.h"
#define INF 1E9

using namespace std;
Q_EXPORT_PLUGIN2(Medianfilter, MedianFilterPlugin);

void processImage1(V3DPluginCallback2 &callback, QWidget *parent);
bool processImage1(const V3DPluginArgList & input, V3DPluginArgList & output);

void processImage2(V3DPluginCallback2 &callback, QWidget *parent);
bool processImage2(const V3DPluginArgList & input, V3DPluginArgList & output);

template <class T> void median_filter(T* data1d,
                     V3DLONG *in_sz,
                     unsigned int Wx,
                     unsigned int Wy,
                     unsigned int Wz,
                     unsigned int c,
                     T* &outimg);

template <class T> void adp_median_filter(T* data1d,
                     V3DLONG *in_sz,
                     unsigned int c,
                     T* &outimg,T* gsdtdata1d );


 
QStringList MedianFilterPlugin::menulist() const
{
    return QStringList()
      <<tr("Fixed Window")
      <<tr("Adaptive Window")
      ;
}


void MedianFilterPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)

{
	if (menu_name == tr("Fixed Window"))
	{
      processImage1(callback,parent);
	}
	else if (menu_name == tr("Adaptive Window"))
	{
      processImage2(callback,parent);
	}	
}


QStringList MedianFilterPlugin::funclist() const
{
	return QStringList()
      <<tr("fixed_window")
      <<tr("adaptive_window");
}


bool MedianFilterPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{

    if (func_name == tr("fixed_window"))
	{
       return processImage1(input, output);
    }
    else if(func_name == tr("adaptive_window"))
	{
        v3d_msg("To be implemented");
       //return processImage2(input, output);
    }
    else if (func_name == tr("help"))
    {
        cout<<"Usage : v3d -x plugin_dll_name -f function_name -i <inimg_file> -o <outimg_file> -p <wx> <wy> <wz> <ch>"<<endl;
        cout<<endl;
        cout<<"wx          filter window radius size (pixel #) in x direction, window size is 2*wx+1, default 3"<<endl;
        cout<<"wy          filter window radius size (pixel #) in y direction, window size is 2*wy+1, default 3"<<endl;
        cout<<"wz          filter window radius size (pixel #) in z direction, window size is 2*wz+1, default 3"<<endl;
        cout<<"ch           the input channel value, default 1 and start from 1, default 1"<<endl;
        cout<<endl;
        cout<<endl;
        return true;
	}
}

bool processImage1(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to Median filter"<<endl;
	if (output.size() != 1) return false;
	unsigned int Wx=7, Wy=7, Wz=3, ch=1;
     if (input.size()>=2)
     {

	  vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
	  cout<<paras.size()<<endl;
          if(paras.size() >= 1) Wx = atoi(paras.at(0));
          if(paras.size() >= 2) Wy = atoi(paras.at(1));
          if(paras.size() >= 3) Wz = atoi(paras.at(2));
          if(paras.size() >= 4) ch = atoi(paras.at(3));
	}

	char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
	char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);

	cout<<"Wx = "<<Wx<<endl;
    cout<<"Wy = "<<Wy<<endl;
	cout<<"Wz = "<<Wz<<endl;
     cout<<"ch = "<<ch<<endl;
	cout<<"inimg_file = "<<inimg_file<<endl;
	cout<<"outimg_file = "<<outimg_file<<endl;

	unsigned char * data1d = 0,  * outimg1d = 0;
	V3DLONG * in_sz = 0;

     unsigned int c = ch;//-1;

	int datatype;
	if(!loadImage(inimg_file, data1d, in_sz, datatype))
     {
          cerr<<"load image "<<inimg_file<<" error!"<<endl;
          return false;
     }

	//input
     void* outimg = 0;

     switch (datatype)
     {
          case 1: median_filter(data1d, in_sz, Wx, Wy, Wz, c, (unsigned char*&)outimg); break;
          case 2: median_filter((unsigned short int*)data1d, in_sz, Wx, Wy, Wz, c, (unsigned short int*&)outimg); break;
          case 4: median_filter((float *)data1d, in_sz, Wx, Wy, Wz, c, (float*&)outimg); break;
          default:
               v3d_msg("Invalid datatype.");
               if (data1d) {delete []data1d; data1d=0;}
               if (in_sz) {delete []in_sz; in_sz=0;}
               return false;
     }

     // save image
     in_sz[3]=1;
     saveImage(outimg_file, (unsigned char *)outimg, in_sz, datatype);

     if(outimg) {delete []outimg; outimg =0;}
     if (data1d) {delete []data1d; data1d=0;}
     if (in_sz) {delete []in_sz; in_sz=0;}

     return true;
}


void processImage1(V3DPluginCallback2 &callback, QWidget *parent)
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

	//input
	bool ok1, ok2, ok3, ok4;
	unsigned int Wx=1, Wy=1, Wz=1, c=1;

	Wx = QInputDialog::getInteger(parent, "Window X ",
											   "Enter radius (window size is 2*radius+1):",
											   1, 1, N, 1, &ok1);

	if(ok1)
	{
		Wy = QInputDialog::getInteger(parent, "Window Y",
											   "Enter radius (window size is 2*radius+1):",
											   1, 1, M, 1, &ok2);
	}
	else
		return;

	if(ok2)
	{
		Wz = QInputDialog::getInteger(parent, "Window Z",
											   "Enter radius (window size is 2*radius+1):",
											   1, 1, P, 1, &ok3);
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
     in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[4] = sc;

    ImagePixelType pixeltype = p4DImage->getDatatype();
    void* outimg = 0;
    switch (pixeltype)
    {
        case V3D_UINT8: median_filter(data1d, in_sz, Wx, Wy, Wz, c,(unsigned char* &)outimg); break;
        case V3D_UINT16: median_filter((unsigned short int *)data1d, in_sz, Wx, Wy, Wz, c, (unsigned short int* &)outimg); break;
        case V3D_FLOAT32: median_filter((float *)data1d, in_sz, Wx, Wy, Wz, c, (float* &)outimg);break;
        default: v3d_msg("Invalid data type. Do nothing."); return;
    }

     // display
     Image4DSimple * new4DImage = new Image4DSimple();
     new4DImage->setData((unsigned char *)outimg, N, M, P, 1, pixeltype);
     v3dhandle newwin = callback.newImageWindow();
     callback.setImage(newwin, new4DImage);
     callback.setImageName(newwin, "Median filter result");
     callback.updateImageWindow(newwin);

     return;
}



template <class T> void median_filter(T* data1d,
                     V3DLONG *in_sz,
                     unsigned int Wx,
                     unsigned int Wy,
                     unsigned int Wz,
                     unsigned int c,
                     T* &outimg)
{
  
     V3DLONG N = in_sz[0];
     V3DLONG M = in_sz[1];
     V3DLONG P = in_sz[2];
     V3DLONG sc = in_sz[3];
     V3DLONG pagesz = N*M*P;

     T *arr,tmp;
     int ii,jj;
     int size = (2*Wx+1)*(2*Wy+1)*(2*Wz+1); 
     arr = new T[size];	

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

		//Median Filtering
	 	for(V3DLONG iz = 0; iz < P; iz++)
		{
			V3DLONG offsetk = iz*M*N;
			for(V3DLONG iy = 0; iy < M; iy++)
			{
				V3DLONG offsetj = iy*N;
				for(V3DLONG ix = 0; ix < N; ix++)
				{

					V3DLONG xb = ix-Wx; if(xb<0) xb = 0;
					V3DLONG xe = ix+Wx; if(xe>=N-1) xe = N-1;
					V3DLONG yb = iy-Wy; if(yb<0) yb = 0;
					V3DLONG ye = iy+Wy; if(ye>=M-1) ye = M-1;
					V3DLONG zb = iz-Wz; if(zb<0) zb = 0;
					V3DLONG ze = iz+Wz; if(ze>=P-1) ze = P-1;
					ii = 0;

					for(V3DLONG k=zb; k<=ze; k++)
					{
						V3DLONG offsetkl = k*M*N;
						for(V3DLONG j=yb; j<=ye; j++)
						{
							V3DLONG offsetjl = j*N;
							for(V3DLONG i=xb; i<=xe; i++)
							{
								T dataval = data1d[ offsetc + offsetkl + offsetjl + i];
								arr[ii] = dataval;
								if (ii>0) 
								{
								   	jj = ii;
									while(jj > 0 && arr[jj-1]>arr[jj])
									{
										tmp = arr[jj];
										arr[jj] = arr[jj-1];
										arr[jj-1] = tmp;
										jj--;
									}	
								}
								ii++;
							}
						}
					}


					//set value
					V3DLONG index_pim = offsetk + offsetj + ix;
					pImage[index_pim] = arr[int(0.5*ii)+1];
				}
			}
		}
	 			
          outimg = pImage;
	  delete [] arr;

}

void processImage2(V3DPluginCallback2 &callback, QWidget *parent)
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
    
    //invoke gsdt function
    V3DPluginArgItem arg;
    V3DPluginArgList input;
    V3DPluginArgList output;    
    

    //need to change here!!!! The following is wrong
    arg.type = "random";std::vector<char*> args1;
    args1.push_back("../../../../../Desktop/vaa3d/Images/ex_Repo_hb9_eve.tif"); arg.p = (void *) & args1; input<< arg;
    arg.type = "random";std::vector<char*> args;
    args.push_back("0");args.push_back("1");args.push_back("0");args.push_back("1"); arg.p = (void *) & args; input << arg;
    arg.type = "random";std::vector<char*> args2;
    args2.push_back("../../../../../Desktop/vaa3d/Images/gsdt_ex_Repo_hb9_eve2.tif"); arg.p = (void *) & args2; output<< arg;

    QString full_plugin_name = "gsdt";  //for Linux
    QString func_name = "gsdt";

    callback.callPluginFunc(full_plugin_name,func_name, input,output); 
	
    //system("v3d -x gsdt -f gsdt -i /home/zhiz/Desktop/vaa3d/Images/ex_Repo_hb9_eve.tif -o /home/zhiz/Desktop/vaa3d/Images/gsdt_ex_Repo_hb9_eve.tif -p 0 1 0 1.0");

    unsigned char * gsdtdata1d = 0;
    int datatype; 
    V3DLONG * in_zz = 0;
	
    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
    loadImage(outimg_file, gsdtdata1d, in_zz, datatype,1);	
	
    void* outimg = 0;
    switch (pixeltype)
    {
        case V3D_UINT8: adp_median_filter(data1d, in_sz, c,(unsigned char* &)outimg, gsdtdata1d); break;
        case V3D_UINT16: adp_median_filter((unsigned short int *)data1d, in_sz, c, (unsigned short int* &)outimg,(unsigned short int *)gsdtdata1d); break;
        case V3D_FLOAT32: adp_median_filter((float *)data1d, in_sz, c, (float* &)outimg,(float *)gsdtdata1d);break;
        default: v3d_msg("Invalid data type. Do nothing."); return;
    }

     // display
     Image4DSimple * new4DImage = new Image4DSimple();
     new4DImage->setData((unsigned char *)outimg, N, M, P, 1, pixeltype);
     v3dhandle newwin = callback.newImageWindow();
     callback.setImage(newwin, new4DImage);
     callback.setImageName(newwin, "Adaptive Median filter result");
     callback.updateImageWindow(newwin);

     return;
}



template <class T> void adp_median_filter(T* data1d,
                     V3DLONG *in_sz,
                     unsigned int c,
                     T* &outimg, 
		     T* gsdtdatald)
{
  
     V3DLONG N = in_sz[0];
     V3DLONG M = in_sz[1];
     V3DLONG P = in_sz[2];
     V3DLONG sc = in_sz[3];
     V3DLONG pagesz = N*M*P;
     
    int Wx,Wy,Wz;

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

		//Median Filtering
	 	for(V3DLONG iz = 0; iz < P; iz++)
		{
			V3DLONG offsetk = iz*M*N;
			for(V3DLONG iy = 0; iy < M; iy++)
			{
				V3DLONG offsetj = iy*N;
				for(V3DLONG ix = 0; ix < N; ix++)
				{
					
					T PixelValue = data1d[offsetk + offsetj + ix];
					T GsdtValue = gsdtdatald[offsetk + offsetj + ix];
					Wx = (int)round((log(PixelValue)/log(2))/GsdtValue);
					//printf("%d %d\n",PixelValue,Wx);
					 
					if ((Wx<=0)||(PixelValue==0))
					{
						V3DLONG index_pim = offsetk + offsetj + ix;
						pImage[index_pim] = PixelValue;	
					}
					else					
					{
						Wy = Wx;
						Wz = Wx;
					
						V3DLONG xb = ix-Wx; if(xb<0) xb = 0;
						V3DLONG xe = ix+Wx; if(xe>=N-1) xe = N-1;
						V3DLONG yb = iy-Wy; if(yb<0) yb = 0;
						V3DLONG ye = iy+Wy; if(ye>=M-1) ye = M-1;
						V3DLONG zb = iz-Wz; if(zb<0) zb = 0;
						V3DLONG ze = iz+Wz; if(ze>=P-1) ze = P-1;

						T *arr,tmp;
	    					int ii,jj;
	     					int size = (2*Wx+1)*(2*Wy+1)*(2*Wz+1); 
	     					arr = new T[size];	
						//printf("%d %d\n",PixelValue,size);

						ii = 0;

						for(V3DLONG k=zb; k<=ze; k++)
						{
							V3DLONG offsetkl = k*M*N;
							for(V3DLONG j=yb; j<=ye; j++)
							{
								V3DLONG offsetjl = j*N;
								for(V3DLONG i=xb; i<=xe; i++)
								{
									T dataval = data1d[ offsetc + offsetkl + offsetjl + i];
									arr[ii] = dataval;
									if (ii>0) 
									{
									   	jj = ii;
										while(jj > 0 && arr[jj-1]>arr[jj])
										{
											tmp = arr[jj];
											arr[jj] = arr[jj-1];
											arr[jj-1] = tmp;
											jj--;
										}	
									}
									ii++;
								}
							}
						}

						//for (ii = 0;ii<size;ii++)
						//	printf("%d ",arr[ii]);
						//	printf("\n");
						//set value
						V3DLONG index_pim = offsetk + offsetj + ix;
						pImage[index_pim] = arr[int(0.5*size)+1];
	 					delete [] arr;
					}
				}
			}
		}
	 			
         outimg = pImage;
         return;
}

