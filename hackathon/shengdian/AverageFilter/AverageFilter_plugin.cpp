/* AverageFilter_plugin.cpp
 * This is a average filter plugin, you can use it as a demo.
 * 2018-3-22 : by shengdian jiang
 */
 
#include "v3d_message.h"
#include <vector>
#include "AverageFilter_plugin.h"
#include <math.h>
#include<iostream>


using namespace std;

Q_EXPORT_PLUGIN2(AverageFilter, AverageFilterPlugin);
void processImage(V3DPluginCallback2 &callback,QWidget *parent);
bool processImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output, QWidget *parent);
template <class T> bool AverageFilter(T* data1d, V3DLONG *in_sz, V3DLONG c,int wd, float* &outimg);
const QString title = QObject::tr("Average Filter Plugin");

QStringList AverageFilterPlugin::menulist() const
{
	return QStringList() 
		<<tr("AverageFilter")
		<<tr("about");
}

QStringList AverageFilterPlugin::funclist() const
{
	return QStringList()
		<<tr("Averagefilterfun")
		<<tr("help");
}

void AverageFilterPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("AverageFilter"))
	{
        processImage(callback,parent);

        //JsdGUITest(callback,p);
	}
	else
	{
        v3d_msg(tr("This is a average filter plugin, you can use it to filter data in a average way. "
			"Developed by shengdian jiang, 2018-3-22"));
	}
}

bool AverageFilterPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{  
    if (func_name == tr("Averagefilterfun"))
    {
        return processImage(callback,input,output,parent);
    }
    else if (func_name == tr("help"))
    {

        cout<<"Usage : v3d -x gaussian -f gf -i <inimg_file> -o <outimg_file> -p <wd> "<<endl;
        cout<<endl;
        cout<<"wd          filter window size (pixel #) in every direction, default 1 and maximum 4"<<endl;
        cout<<endl;
        cout<<"e.g. v3d -x average -f af -i input.raw -o output.raw -p 2"<<endl;
        cout<<endl;
        return true;
    }
    else return false;

    return true;
}
bool processImage(V3DPluginCallback2 &callback,const V3DPluginArgList & input, V3DPluginArgList & output,QWidget *parent)
{
    cout<<"Welcome to Average filter"<<endl;
    if(output.size()!=1)
        return false;
    unsigned int wd=1;
    if(input.size()>=2)
    {
        vector<char*> paras=(*(vector<char*> *)(input.at(1).p));
        if(paras.size()>=1)
            wd=atoi(paras.at(0));
    }

    char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
    cout<<"window size = "<<wd<<endl;
    cout<<"inimg_file = "<<inimg_file<<endl;
    cout<<"outimg_file = "<<outimg_file<<endl;

    Image4DSimple *inimg = callback.loadImage(inimg_file);
    if (!inimg || !inimg->valid())
    {
        v3d_msg("Fail to open the image file.", 0);
        return false;
    }
    float* outimg = 0; //no need to delete it later as the Image4DSimple variable "outimg" will do the job

    bool ok2;
    V3DLONG c=1;
    V3DLONG in_sz[4];
    in_sz[0] = inimg->getXDim();
    in_sz[1] = inimg->getYDim();
    in_sz[2] = inimg->getZDim();
    in_sz[3] = inimg->getCDim();
    if (in_sz[3]>1) //only need to ask if more than one channel
    {
        c = QInputDialog::getInteger(parent, "Channel",
                                         "Enter channel # (starts from 1):",
                                         1, 1, in_sz[3], 1, &ok2);
        if (!ok2)
            return false;
    }

    switch (inimg->getDatatype())
    {
         case V3D_UINT8: AverageFilter(inimg->getRawData(), in_sz, c-1, wd, outimg); break;
         case V3D_UINT16: AverageFilter((unsigned short int*)(inimg->getRawData()), in_sz, c-1, wd, outimg); break;
         case V3D_FLOAT32: AverageFilter((float *)(inimg->getRawData()), in_sz, c-1, wd, outimg); break;
         default:
              v3d_msg("Invalid datatype in Gaussian fileter.", 0);
              if (inimg) {delete inimg; inimg=0;}
              return false;
    }
    // save image
    Image4DSimple outimg1;
    outimg1.setData((unsigned char *)outimg, in_sz[0], in_sz[1], in_sz[2], 1, V3D_FLOAT32);

    callback.saveImage(&outimg1, outimg_file);

    if(inimg) {delete inimg; inimg =0;}

    return true;

}

void processImage(V3DPluginCallback2 &callback,QWidget *parent)
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
         //V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

        //input
        bool ok1;
        V3DLONG c=1;

        V3DLONG in_sz[4];
        in_sz[0] = p4DImage->getXDim();
        in_sz[1] = p4DImage->getYDim();
        in_sz[2] = p4DImage->getZDim();
        in_sz[3] = p4DImage->getCDim();

        AverageFilterDialog dialog(callback,parent);
        if(!dialog.image)
            return;
        if(dialog.exec()!=QDialog::Accepted)
            return;
        dialog.update();
        Image4DSimple *subject=dialog.image;
        if(!subject)
            return;
        int wd=dialog.Wd;
        cout<<"Window width = "<<wd<<endl;

        if (in_sz[3]>1) //only need to ask if more than one channel
        {
            c = QInputDialog::getInteger(parent, "Channel",
                                             "Enter channel # (starts from 1):",
                                             1, 1, in_sz[3], 1, &ok1);
            if (!ok1)
                return;
        }

         // filtering

        float * outimg = 0;
        bool b_res;

        v3d_msg(QString("datatype=%1").arg(int(p4DImage->getDatatype())), 0);

        switch(p4DImage->getDatatype())
        {
            case V3D_UINT8:
                b_res = AverageFilter(data1d, in_sz, c-1,wd, outimg);
                break;
            case V3D_UINT16:
                b_res = AverageFilter((unsigned short int *)data1d, in_sz, c-1, wd,outimg);
                break;
            case V3D_FLOAT32:
                b_res = AverageFilter((float *)data1d, in_sz, c-1,wd, outimg);
                break;
            default:
                b_res = false;
                v3d_msg("Unsupported data type. Do nothing.");
                return;
        }

         if (!b_res)
         {
             v3d_msg("Fail to invoke the filter. Do nothing.");
             return;
         }

         // display
         Image4DSimple * new4DImage = new Image4DSimple();
         new4DImage->setData((unsigned char *)outimg, in_sz[0], in_sz[1], in_sz[2], 1, V3D_FLOAT32);
         v3dhandle newwin = callback.newImageWindow();
         callback.setImage(newwin, new4DImage);
         callback.setImageName(newwin, title);
         callback.updateImageWindow(newwin);
}
template <class T> bool AverageFilter(T* data1d, V3DLONG *in_sz, V3DLONG c,int wd, float* &outimg)
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

     T *curdata1d = data1d + offset_init;

     for(V3DLONG i=0; i<pagesz; i++)
         outimg[i] = curdata1d[i];

    for(V3DLONG iz = wd; iz < P-wd; iz++)
    {
        for(V3DLONG iy = wd; iy < M-wd; iy++)
        {
            for(V3DLONG ix = wd; ix < N-wd; ix++)
            {
                //average filter 3*3*3
                double temp=0.0;
                //V3DLONG temp_p=(iz-1)*pagesz+(iy-1)*N+ix-1;
                for(V3DLONG izz=iz-wd;izz<iz+wd+1;izz++)
                {
                    for(V3DLONG iyy=iy-wd;iyy<iy+wd+1;iyy++)
                    {
                        for(V3DLONG ixx=ix-wd;ixx<ix+wd+1;ixx++)
                        {
                            temp+=double(curdata1d[izz*pagesz+iyy*N+ixx]);
                        }
                    }
                }
                outimg[iz*pagesz+iy*N+ix]=float(temp/pow(wd,3));
                //outimg[iz*pagesz + iy*N + ix] = t - cnt*float(curdata1d[iz*pagesz + iy*N + ix]);
            }
        }
    }
    return true;
}
