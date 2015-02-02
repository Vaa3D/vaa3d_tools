/* resampleimage_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2015-02-02 : by Zhi Zhou 
 */
 
#include "v3d_message.h"
#include <vector>
#include "resampleimage_plugin.h"
#include <iostream>
#include "../../../released_plugins/v3d_plugins/cellseg_gvf/src/FL_upSample3D.h"

using namespace std;
Q_EXPORT_PLUGIN2(resampleimage, resampleimage);

void upsampleImage_domenu(V3DPluginCallback2 &callback, QWidget *parent);
bool upsampleImage_dofunc(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);


QStringList resampleimage::menulist() const
{
	return QStringList() 
        <<tr("up_sample")
        <<tr("down_sample")
		<<tr("about");
}

QStringList resampleimage::funclist() const
{
	return QStringList()
        <<tr("up_sample")
        <<tr("down_sample")
		<<tr("help");
}

void resampleimage::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("up_sample"))
	{
        upsampleImage_domenu(callback,parent);
	}
    else if (menu_name == tr("down_sample"))
	{
		v3d_msg("To be implemented.");
    }
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou , 2015-02-02"));
	}
}

bool resampleimage::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("up_sample"))
	{
        upsampleImage_dofunc(callback, input, output);
	}
    else if (func_name == tr("down_sample"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
        cout<<"Usage : v3d -x image_resample -f up_sample -i <inimg_file> -o <outimg_file> -p <x_factor> <y_factor> <z_factor> <ch>"<<endl;
        cout<<endl;
        cout<<"x_factor          upsample factor (>=1) in x direction, default 1"<<endl;
        cout<<"y_factor          upsample factor (>=1) in y direction, default 1"<<endl;
        cout<<"z_factor          upsample factor (>=1) in z direction, default 1"<<endl;
        cout<<"ch                the input channel value, default 1 and start from 1, default 1"<<endl;
        cout<<endl;
        cout<<endl;
	}
	else return false;

	return true;
}

void upsampleImage_domenu(V3DPluginCallback2 &callback, QWidget *parent)
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

//    unsigned char* data1d = p4DImage->getRawData();
     V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();
     V3DLONG N = p4DImage->getXDim();
     V3DLONG M = p4DImage->getYDim();
     V3DLONG P = p4DImage->getZDim();
     V3DLONG sc = p4DImage->getCDim();

     if(p4DImage->getDatatype()!=V3D_UINT8)
     {
         QMessageBox::information(0, "", "The image format is not supported!");
         return;
     }
     //input
     bool ok1, ok2, ok3, ok4;
     double x_rez=1.0, y_rez=1.0, z_rez=1.0, c=1;

     x_rez = QInputDialog::getDouble(parent, "X factor ",
                                   "Enter X upsample rate (>=1):",
                                   1.0, 1.0, 20.0, 1.0, &ok1);

     if(ok1)
     {
         y_rez = QInputDialog::getDouble(parent, "Y factor",
                                       "Enter Y upsample rate (>=1):",
                                       1.0, 1.0, 20.0, 1.0, &ok2);
     }
     else
         return;

     if(ok2)
     {
         z_rez = QInputDialog::getDouble(parent, "Z factor",
                                       "Enter Z upsample rate (>=1)::",
                                       1.0, 1.0, 20.0, 1.0, &ok3);
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

     unsigned char* data1d = p4DImage->getRawDataAtChannel(c-1);
     V3DLONG in_sz[4];
     in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = 1;

     double dfactor[3];
     dfactor[0] = x_rez; dfactor[1] = y_rez;  dfactor[2] = z_rez;

     V3DLONG pagesz_upsample = (V3DLONG)(ceil(x_rez*y_rez*z_rez*pagesz));

     unsigned char * image_upsampled = 0;
     try {image_upsampled = new unsigned char [pagesz_upsample];}
     catch(...)  {v3d_msg("cannot allocate memory for data_blended."); return;}

     V3DLONG out_sz[4];
     out_sz[0] = (V3DLONG)(ceil(dfactor[0]*N));
     out_sz[1] = (V3DLONG)(ceil(dfactor[1]*M));
     out_sz[2] = (V3DLONG)(ceil(dfactor[2]*P));
     out_sz[3] = 1;
     upsample3dvol((unsigned char *)image_upsampled,(unsigned char *)data1d,out_sz,in_sz,dfactor);
     // display
     Image4DSimple * new4DImage = new Image4DSimple();
     new4DImage->setData((unsigned char *)image_upsampled, out_sz[0], out_sz[1], out_sz[2], 1, V3D_UINT8);
     v3dhandle newwin = callback.newImageWindow();
     callback.setImage(newwin, new4DImage);
     callback.setImageName(newwin, "Upsampled image result");
     callback.updateImageWindow(newwin);
     return;
}

bool upsampleImage_dofunc(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{

    cout<<"Welcome to upsample plugin"<<endl;
    if (output.size() != 1) return false;
    double x_rez=1, y_rez=1, z_rez=1, ch=1;
    if (input.size()>=2)
    {
        vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
        cout<<paras.size()<<endl;
        if(paras.size() >= 1) x_rez = atoi(paras.at(0));
        if(paras.size() >= 2) y_rez = atoi(paras.at(1));
        if(paras.size() >= 3) z_rez = atoi(paras.at(2));
        if(paras.size() >= 4) ch = atoi(paras.at(3));
    }

    if(x_rez<1 || y_rez<1 || z_rez<1)
    {
        cerr<<"Upsample factor cannot be smaller than 1"<<endl;
        return false;
    }
    char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);

    cout<<"x_factor = "<<x_rez<<endl;
    cout<<"y_factor = "<<y_rez<<endl;
    cout<<"z_factor = "<<z_rez<<endl;
    cout<<"ch = "<<ch<<endl;
    cout<<"inimg_file = "<<inimg_file<<endl;
    cout<<"outimg_file = "<<outimg_file<<endl;

    unsigned char * data1d = 0;
    V3DLONG in_sz[4];
    unsigned int c = ch;
    int datatype;
    if(!simple_loadimage_wrapper(callback, inimg_file, data1d, in_sz, datatype))
    {
        cerr<<"load image "<<inimg_file<<" error!"<<endl;
        return false;
    }

    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG sc = in_sz[3];
    V3DLONG pagesz = N*M*P;

    unsigned char* data1d_ch1 = new unsigned char [pagesz];
    if (!data1d_ch1)
    {
        printf("Fail to allocate memory.\n");
        return false;
    }
    else
    {
        for(V3DLONG i=0; i<pagesz; i++)
            data1d_ch1[i] = data1d[i+(c-1)*pagesz];
    }
    if(data1d) {delete []data1d; data1d =0;}

    in_sz[3] = 1;

    double dfactor[3];
    dfactor[0] = x_rez; dfactor[1] = y_rez;  dfactor[2] = z_rez;

    V3DLONG pagesz_upsample = (V3DLONG)(ceil(x_rez*y_rez*z_rez*pagesz));

    unsigned char * image_upsampled = 0;
    try {image_upsampled = new unsigned char [pagesz_upsample];}
    catch(...)  {v3d_msg("cannot allocate memory for data_blended."); return false;}

    V3DLONG out_sz[4];
    out_sz[0] = (V3DLONG)(ceil(dfactor[0]*N));
    out_sz[1] = (V3DLONG)(ceil(dfactor[1]*M));
    out_sz[2] = (V3DLONG)(ceil(dfactor[2]*P));
    out_sz[3] = 1;
    upsample3dvol((unsigned char *)image_upsampled,(unsigned char *)data1d_ch1,out_sz,in_sz,dfactor);
    simple_saveimage_wrapper(callback, outimg_file, (unsigned char *)image_upsampled, out_sz, 1);
    if(data1d_ch1) {delete []data1d_ch1; data1d_ch1 =0;}
    if(image_upsampled) {delete []image_upsampled; image_upsampled =0;}

    return true;
}
