/* datatypecnvrt.cpp
 * 2011-01-21: create this program by Yang Yu
 */

// Adapted and upgraded to add dofunc() by Jianlong Zhou, 2012-04-08
// last edit. by Hanchuan Peng, 2013-08-09 to add the pbd file saving

#include <QtGui>

#include <cmath>
#include <stdlib.h>
#include <ctime>

#include <iostream>

#include "stackutil.h"
#include "datatypecnvrt.h"

#define _ALLOW_WORKMODE_MENU_


using namespace std;

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(datatypeconvert, DTCPlugin);

// func datatype converting main
int  datatype_converting(V3DPluginCallback2 &callback, QWidget *parent);
bool datatype_converting(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);

// func converting
template <class Tpre, class Tpost>
void converting(void *pre1d, Tpost *pPost, V3DLONG imsz, ImagePixelType v3d_dt);


//plugin funcs
const QString title = "Datatype Converting";
QStringList DTCPlugin::menulist() const
{
    return QStringList() << tr("Data type converter")
			<< tr("About");
}

void DTCPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("Data type converter"))
    {
    	datatype_converting(callback, parent);
    }
	else if (menu_name == tr("About"))
	{
		QMessageBox::information(parent, "Version info", QString("Datatype Converting Plugin %1 (Jan 25, 2011) developed by Yang Yu. (Peng Lab, Janelia Research Farm Campus, HHMI)").arg(getPluginVersion()));
		return;
	}
}

QStringList DTCPlugin::funclist() const
{
	return QStringList()
		<<tr("dtc")
		<<tr("help");
}


bool DTCPlugin::dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
{
     if (func_name == tr("dtc"))
	{
        return datatype_converting(callback, input, output);
	}
	else if(func_name == tr("help"))
	{
        cout<<"Convert the data type of an image file and save to another."<<endl;
        cout<<"Usage : v3d -x datatypeconvert -f dtc -i <inimg_file> -o <outimg_file> -p <tar_dt>"<<endl;
		cout<<endl;
        cout<<"tar_dt   target datatype to be converted to: 1 for V3D_UINT8, 2 for V3D_UINT16, 4 for V3D_FLOAT32 "<<endl;
		cout<<endl;
		cout<<"e.g. v3d -x datatypeconvert -f dtc -i input.raw -o output.raw -p 1"<<endl;
		cout<<endl;
		return true;
	}
}


bool datatype_converting(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{
    cout<<"DataType/file_format Converter"<<endl;
	if (output.size() != 1) return false;

     int tar_dt = 0;
     if (input.size()>=2)
     {
          vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
          if(paras.size() >= 1) tar_dt = atoi(paras.at(0));
	}

	char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
	char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
    cout<<"target data type = "<<tar_dt<<endl;
	cout<<"inimg_file = "<<inimg_file<<endl;
	cout<<"outimg_file = "<<outimg_file<<endl;

     if(tar_dt!=1 && tar_dt!=2 && tar_dt!=4
             && tar_dt!=0)
     {
          v3d_msg("Invalide target data type. Only support V3D_UINT8, V3D_UINT16, and V3D_FLOAT32 data.");
          return false;
     }

	unsigned char * subject1d = 0;
    V3DLONG in_sz[4];

	int sub_dt;
    if(!simple_loadimage_wrapper(callback, inimg_file, subject1d, in_sz, sub_dt, -1))
     {
          cerr<<"load image ["<<inimg_file<<"] error!"<<endl;
          return false;
     }

    if (tar_dt==0) //in this case, set the target data type to be the same as the input image file. In this case, the file format can be converted without changing of the data. by PHC, 20130809
        tar_dt = sub_dt;

     V3DLONG	sz_sub = in_sz[0]*in_sz[1]*in_sz[2]*in_sz[3];

     //Converting
	if(tar_dt == 1) //V3D_UINT8
	{
		unsigned char * data1d = NULL;

		try
		{
			data1d = new unsigned char [sz_sub];
		}
		catch(...)
		{
			printf("Error allocating memory. \n");
			return -1;
		}

		if(sub_dt == 1)
		{
            converting<unsigned char, unsigned char>((unsigned char *)subject1d, data1d, sz_sub, V3D_UINT8);
		}
		else if(sub_dt == 2)
		{
            converting<unsigned short, unsigned char>((unsigned short *)subject1d, data1d, sz_sub, V3D_UINT8);
		}
		else if(sub_dt == 4)
		{
            converting<float, unsigned char>((float *)subject1d, data1d, sz_sub, V3D_UINT8);
		}

          // save image
        v3d_msg("****** \n Finished conversion operation in memory. Now try to save file. \n****** \n\n", 0);
          if(!simple_saveimage_wrapper(callback, outimg_file, (unsigned char *)data1d, in_sz, 1))
              v3d_msg("Fail to save the output image.\n",0);

          if (data1d) {delete []data1d; data1d=0;}
	}
	else if(tar_dt == 2) //V3D_UINT16
	{
		unsigned short * data1d = NULL;

		try
		{
			data1d = new unsigned short [sz_sub];
		}
		catch(...)
		{
			v3d_msg("Error allocating memory.", 0);
			return false;
		}

		//
		if(sub_dt == 1)
		{
            converting<unsigned char, unsigned short>((unsigned char *)subject1d, data1d, sz_sub, V3D_UINT16);
		}
		else if(sub_dt == 2)
		{
            converting<unsigned short, unsigned short>((unsigned short *)subject1d, data1d, sz_sub, V3D_UINT16);
		}
		else if(sub_dt == 4)
		{
            converting<float, unsigned short>((float *)subject1d, data1d, sz_sub, V3D_UINT16);
		}

          // save image
        v3d_msg("****** \n Finished conversion operation in memory. Now try to save file. \n****** \n\n", 0);
          if(!simple_saveimage_wrapper(callback, outimg_file, (unsigned char *)data1d, in_sz, 2))
              v3d_msg("Fail to save the output image.\n",0);
          if (data1d) {delete []data1d; data1d=0;}
	}
	else if(tar_dt == 4) //V3D_FLOAT32
	{
		float * data1d = NULL;

		try
		{
			data1d = new float [sz_sub];
		}
		catch(...)
		{
			v3d_msg("Error allocating memory.", 0);
			return false;
		}

		//
		if(sub_dt == 1)
		{
            converting<unsigned char, float>((unsigned char *)subject1d, data1d, sz_sub, V3D_FLOAT32);
		}
		else if(sub_dt == 2)
		{
            converting<unsigned short, float>((unsigned short *)subject1d, data1d, sz_sub, V3D_FLOAT32);
		}
		else if(sub_dt == 4)
		{
            converting<float, float>((float *)subject1d, data1d, sz_sub, V3D_FLOAT32);
		}

          // save image
        v3d_msg("****** \n Finished conversion operation in memory. Now try to save file. \n****** \n\n", 0);
        if(simple_saveimage_wrapper(callback, outimg_file, (unsigned char *)data1d, in_sz, 4))
            v3d_msg("Fail to save the output image.\n",0);

        if (data1d) {delete []data1d; data1d=0;}
	}
	else
	{
        v3d_msg("Currently this program only supports UINT8, UINT16, and FLOAT32 data type.", 0);
		return false;
	}

     if(subject1d) {delete []subject1d; subject1d=0;}

     return true;
}

// func converting
template <class Tpre, class Tpost>
void converting(void *pre1d, Tpost *pPost, V3DLONG imsz, ImagePixelType v3d_dt)
{
     if (!pre1d ||!pPost || imsz<=0 )
     {
          v3d_msg("Invalid parameters to converting().", 0);
          return;
     }

	Tpre *pPre = (Tpre *)pre1d;

	if(v3d_dt == V3D_UINT8)
	{
		Tpre max_v=0, min_v = 255;

		for(V3DLONG i=0; i<imsz; i++)
		{
			if(max_v<pPre[i]) max_v = pPre[i];
			if(min_v>pPre[i]) min_v = pPre[i];
		}
		max_v -= min_v;

		if(max_v>0)
		{
			for(V3DLONG i=0; i<imsz; i++)
				pPost[i] = (Tpost) 255*(double)(pPre[i] - min_v)/max_v;
		}
		else
		{
			for(V3DLONG i=0; i<imsz; i++)
				pPost[i] = (Tpost) pPre[i];
		}
	}
	else if(v3d_dt == V3D_UINT16)
	{
        Tpre max_v=0, min_v = 65535;

		for(V3DLONG i=0; i<imsz; i++)
		{
			if(max_v<pPre[i]) max_v = pPre[i];
			if(min_v>pPre[i]) min_v = pPre[i];
		}
		max_v -= min_v;

		if(max_v>0)
		{
			for(V3DLONG i=0; i<imsz; i++)
				pPost[i] = (Tpost) 65535*(double)(pPre[i] - min_v)/max_v;
		}
		else
		{
			for(V3DLONG i=0; i<imsz; i++)
				pPost[i] = (Tpost) pPre[i];
		}

	}
	else if(v3d_dt == V3D_FLOAT32)
	{
		for(V3DLONG i=0; i<imsz; i++)
			pPost[i] = (Tpost) pPre[i];
	}

}


// show different datatype data in V3D
int datatype_converting(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandleList win_list = callback.getImageWindowList();

	if(win_list.size()<1)
	{
        v3d_msg("No image is open.");
		return -1;
	}

	//
	DTCDialog dialog(callback,parent);
	if (dialog.exec()!=QDialog::Accepted)	return -1;

	dialog.update();

	V3DLONG isub = dialog.isub;
	V3DLONG dt = dialog.dt;

	//
	ImagePixelType cnvrt_dt;

	if(dt==0)
	{
		cnvrt_dt = V3D_UINT8;
	}
	else if(dt==1)
	{
		cnvrt_dt = V3D_UINT16;
	}
	else if(dt==2)
	{
		cnvrt_dt = V3D_FLOAT32;
	}

	Image4DSimple* subject = callback.getImage(win_list[isub]);

	if (!subject)
	{
		QMessageBox::information(0, title, QObject::tr("Image does not exist."));
		return -1;
	}

    unsigned char* subject1d = subject->getRawData();

	V3DLONG sx = subject->getXDim();
    V3DLONG sy = subject->getYDim();
    V3DLONG sz = subject->getZDim();
	V3DLONG sc = subject->getCDim();

	V3DLONG	sz_sub = sx*sy*sz*sc;
	ImagePixelType sub_dt = subject->getDatatype();


	//Converting
	if(cnvrt_dt == V3D_UINT8)
	{
		unsigned char * data1d = NULL;

		try
		{
			data1d = new unsigned char [sz_sub];
		}
		catch(...)
		{
			printf("Error allocating memory. \n");
			return -1;
		}

		//
		if(sub_dt == V3D_UINT8)
		{
            converting<unsigned char, unsigned char>((unsigned char *)subject1d, data1d, sz_sub, cnvrt_dt);
		}
		else if(sub_dt == V3D_UINT16)
		{
            converting<unsigned short, unsigned char>((unsigned short *)subject1d, data1d, sz_sub, cnvrt_dt);
		}
		else if(sub_dt == V3D_FLOAT32)
		{
            converting<float, unsigned char>((float *)subject1d, data1d, sz_sub, cnvrt_dt);
		}

		//display
		Image4DSimple p4DImage;
		p4DImage.setData((unsigned char*)data1d, sx, sy, sz, sc, V3D_UINT8); //

		v3dhandle newwin = callback.newImageWindow();
		callback.setImage(newwin, &p4DImage);
		callback.setImageName(newwin, "Converted Image");
		callback.updateImageWindow(newwin);
	}
	else if(cnvrt_dt == V3D_UINT16)
	{
		unsigned short * data1d = NULL;

		try
		{
			data1d = new unsigned short [sz_sub];
		}
		catch(...)
		{
			printf("Error allocating memory. \n");
			return -1;
		}

		//
		if(sub_dt == V3D_UINT8)
		{
            converting<unsigned char, unsigned short>((unsigned char *)subject1d, data1d, sz_sub, cnvrt_dt);
		}
		else if(sub_dt == V3D_UINT16)
		{
            converting<unsigned short, unsigned short>((unsigned short *)subject1d, data1d, sz_sub, cnvrt_dt);
		}
		else if(sub_dt == V3D_FLOAT32)
		{
            converting<float, unsigned short>((float *)subject1d, data1d, sz_sub, cnvrt_dt);
		}

		//display
		Image4DSimple p4DImage;
		p4DImage.setData((unsigned char*)data1d, sx, sy, sz, sc, V3D_UINT16); //

		v3dhandle newwin = callback.newImageWindow();
		callback.setImage(newwin, &p4DImage);
		callback.setImageName(newwin, "Converted Image");
		callback.updateImageWindow(newwin);
	}
	else if(cnvrt_dt == V3D_FLOAT32)
	{
		float * data1d = NULL;

		try
		{
			data1d = new float [sz_sub];
		}
		catch(...)
		{
			printf("Error allocating memory. \n");
			return -1;
		}

		//
		if(sub_dt == V3D_UINT8)
		{
            converting<unsigned char, float>((unsigned char *)subject1d, data1d, sz_sub, cnvrt_dt);
		}
		else if(sub_dt == V3D_UINT16)
		{
            converting<unsigned short, float>((unsigned short *)subject1d, data1d, sz_sub, cnvrt_dt);
		}
		else if(sub_dt == V3D_FLOAT32)
		{
            converting<float, float>((float *)subject1d, data1d, sz_sub, cnvrt_dt);
		}

		//display
		Image4DSimple p4DImage;
		p4DImage.setData((unsigned char*)data1d, sx, sy, sz, sc, V3D_FLOAT32); //

		v3dhandle newwin = callback.newImageWindow();
		callback.setImage(newwin, &p4DImage);
		callback.setImageName(newwin, "Converted Image");
		callback.updateImageWindow(newwin);
	}
	else
	{
		printf("Currently this program only support UINT8, UINT16, and FLOAT32 data type.\n");
		return -1;
	}

	//
	return 0;

}

