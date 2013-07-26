/* laplacianfilterplugin.cpp
 * 2012-04-09 Hanchuan Peng
 */


#include <QtGui>

#include <math.h>
#include <stdlib.h>
#include <iostream>

#include "v3d_message.h"
#include "stackutil.h"

#include "laplacianfilterplugin.h"

using namespace std;

#define INF 1E9

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(laplacianfilter, LaplacianFilterPlugin)


void processImage(V3DPluginCallback2 &callback, QWidget *parent);
bool processImage(const V3DPluginArgList & input, V3DPluginArgList & output);
template <class T> bool laplacian_filter(T* data1d, V3DLONG *in_sz, V3DLONG c, float* &outimg);

const QString title = QObject::tr("Laplacian Filter Plugin");
QStringList LaplacianFilterPlugin::menulist() const
{
    return QStringList() << tr("Laplacian Filter") << tr("About");
}

void LaplacianFilterPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("Laplacian Filter"))
	{
		processImage(callback,parent);
	}
    else if (menu_name == tr("About"))
     {
         v3d_msg("This plugin is developed by Hanchuan Peng as a simple example for 3D Laplacian transform of an image.");
     }
}

QStringList LaplacianFilterPlugin::funclist() const
{
	return QStringList()
		<<tr("laplacian")
		<<tr("help");
}


bool LaplacianFilterPlugin::dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
{
     if (func_name == tr("laplacian"))
	{
		return processImage(input, output);
	}
	else if(func_name == tr("help"))
	{
		cout<<"Usage : v3d -x laplacian -f laplacian -i <inimg_file> -o <outimg_file> -p <channel>"<<endl;
		cout<<endl;
		cout<<"channel                  the input channel value, default 1 and start from 1"<<endl;
		cout<<"Developed by Hanchuan Peng as a simple example for 3D Laplacian transform of an image."<<endl;
		cout<<endl;
		return true;
	}
}

bool processImage(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to Laplacian filter"<<endl;
	if(input.size() < 1 || output.size() != 1) 
    {
        v3d_msg("You have not specified enough arguments. Do nothing.\n", 0);
        return false;
    }

	int c=1;
    float sigma = 1.0;
    if (input.size()>=2)
    {
        vector<char*> paras = *(vector<char*> *)(input.at(1).p);
        if(paras.size() >= 1) c = atoi(paras.at(0));
    }
    if (c<=0)
    {
        v3d_msg("Invalid channel parameter (it should starts from 1). \n",0);
        return false;
    }
        
	char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
	char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
    cout<<"channel = "<<c<<endl;
	cout<<"inimg_file = "<<inimg_file<<endl;
	cout<<"outimg_file = "<<outimg_file<<endl;

	unsigned char * data1d = 0;
	V3DLONG * in_sz = 0;
    float* outimg = 0;
    bool b_res;
    
	int datatype;
	if(!loadImage(inimg_file, data1d, in_sz, datatype)) 
    {
        cerr<<"load image "<<inimg_file<<" error!"<<endl; return false;
    }
    
    if (c<=0 || c>in_sz[3])
        goto Label_exit;

    switch(datatype)
    {
        case 1: b_res = laplacian_filter(data1d, in_sz, c-1, outimg); break;
        case 2: b_res = laplacian_filter((unsigned short int *)data1d, in_sz, c-1, outimg); break;
        case 4: b_res = laplacian_filter((float *)data1d, in_sz, c-1, outimg); break;
        default: b_res = false; v3d_msg("Right now this plugin supports only UINT8 data. Do nothing."); goto Label_exit;
    }
    
     if (!b_res)
     {
         v3d_msg("Fail to invoke the filter. Do nothing.\n", 0);
         goto Label_exit;
     }

     // save image
     in_sz[3]=1;
     b_res = saveImage(outimg_file, (unsigned char *)outimg, in_sz, 4); //since b_res must be true right before this, it is safe to assign a value to it

Label_exit:
    if(outimg) {delete []outimg; outimg =0;}
     if (data1d) {delete []data1d; data1d=0;}
     if (in_sz) {delete []in_sz; in_sz=0;}
     return b_res;
}


void processImage(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
	if (!curwin)
	{
        v3d_msg("You don't have any image open in the main window.");
		return;
	}

    Image4DSimple* p4DImage = callback.getImage(curwin);
	if (!p4DImage)
	{
		v3d_msg("The image pointer is invalid. Ensure your data is valid and try again!");
		return;
	}
    
     unsigned char* data1d = p4DImage->getRawData();
     V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

     V3DLONG N = p4DImage->getXDim();
     V3DLONG M = p4DImage->getYDim();
     V3DLONG P = p4DImage->getZDim();
     V3DLONG sc = p4DImage->getCDim();

	//input
	bool ok1;
	V3DLONG c=1;

    if (sc>1) //only need to ask if more than one channel
    {
        c = QInputDialog::getInteger(parent, "Channel",
                                         "Enter channel # (starts from 1):",
                                         1, 1, sc, 1, &ok1);
        if (!ok1)
            return;
    }
    
     // filtering
     V3DLONG in_sz[4];
     in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = sc;

     float * outimg = 0;
    bool b_res;
    
    switch(p4DImage->getDatatype())
    {
        case V3D_UINT8: b_res = laplacian_filter(data1d, in_sz, c-1, outimg); break;
        case V3D_UINT16: b_res = laplacian_filter((unsigned short int *)data1d, in_sz, c-1, outimg); break;
        case V3D_FLOAT32: b_res = laplacian_filter((float *)data1d, in_sz, c-1, outimg); break;
        default: b_res = false; v3d_msg("Right now this plugin supports only UINT8 data. Do nothing."); return;
    }
    
     if (!b_res)
     {
         v3d_msg("Fail to invoke the filter. Do nothing.");
         return;
     }
    
     // display
     Image4DSimple * new4DImage = new Image4DSimple();
     new4DImage->setData((unsigned char *)outimg, N, M, P, 1, V3D_FLOAT32);
     v3dhandle newwin = callback.newImageWindow();
     callback.setImage(newwin, new4DImage);
     callback.setImageName(newwin, title);
     callback.updateImageWindow(newwin);
}



template <class T> bool laplacian_filter(T* data1d, V3DLONG *in_sz, V3DLONG c, float* &outimg)
{
    if (!data1d || !in_sz || c<0 || outimg)
        return false;
    
     V3DLONG N = in_sz[0];
     V3DLONG M = in_sz[1];
     V3DLONG P = in_sz[2];
     V3DLONG sc = in_sz[3];
    V3DLONG pagesz = N*M;
    V3DLONG channelsz = pagesz*P;

     //filtering
     V3DLONG offset_init = c*channelsz;

    try
    {
        outimg = new float [channelsz];
    }
    catch (...)
     {
          printf("Fail to allocate memory.\n");
          return false;
     }

     for(V3DLONG i=0; i<pagesz; i++)
         outimg[i] = data1d[i + offset_init]; 

 
    for(V3DLONG iz = 0; iz < P; iz++)
    {
        for(V3DLONG iy = 0; iy < M; iy++)
        {
            for(V3DLONG ix = 0; ix < N; ix++)
            {
                double t=0.0;
                int cnt = 0;
                
                if (iz>=1)
                { t += float(data1d[(iz-1)*pagesz + iy*N + ix]);cnt++;}
                if (iz<=P-2)
                {    t += float(data1d[(iz+1)*pagesz + iy*N + ix]);cnt++;}               
                if (iy>=1)
                {    t += float(data1d[iz*pagesz + (iy-1)*M + ix]);cnt++;}
                if (iy<=M-2)
                {    t += float(data1d[iz*pagesz + (iy+1)*M + ix]);cnt++;}         
                if (ix>=1)
                {    t += float(data1d[iz*pagesz + iy*N + ix-1]);cnt++;}
                if (ix<=N-2)
                {    t += float(data1d[iz*pagesz + iy*N + ix+1]);cnt++;}
                    
                outimg[iz*pagesz + iy*N + ix] = t - cnt*float(data1d[iz*pagesz + iy*N + ix]);
            }
        }
    }
    
    return true;
}



