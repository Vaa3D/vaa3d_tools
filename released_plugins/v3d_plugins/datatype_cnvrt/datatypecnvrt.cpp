/* datatypecnvrt.cpp
 * 2011-01-21: create this program by Yang Yu
 */

// 
#include <QtGui>

#include <cmath>
#include <stdlib.h>
#include <ctime>

#include "datatypecnvrt.h"


//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(datatypeconvert, DTCPlugin);

// func datatype converting main
int datatype_converting(V3DPluginCallback2 &callback, QWidget *parent);

// func convering kernel
template <class Tpre, class Tpost>
int convering(void *subject1d, ImagePixelType v3d_dt, ImagePixelType dt);

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

// func convering kernel
template <class Tpre, class Tpost>
void convering(void *pre1d, Tpost *pPost, V3DLONG imsz, ImagePixelType v3d_dt)
{

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
			{
				pPost[i] = (Tpost) 255*(double)(pPre[i] - min_v)/max_v;
			}	
		}
		else
		{
			for(V3DLONG i=0; i<imsz; i++)
			{
				pPost[i] = (Tpost) pPre[i];
			}
		}

	}
	else if(v3d_dt == V3D_UINT16)
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
			{
				pPost[i] = (Tpost) 65535*(double)(pPre[i] - min_v)/max_v;
			}	
		}
		else
		{
			for(V3DLONG i=0; i<imsz; i++)
			{
				pPost[i] = (Tpost) pPre[i];
			}
		}
		
	}
	else if(v3d_dt == V3D_FLOAT32)
	{
		for(V3DLONG i=0; i<imsz; i++)
		{
			pPost[i] = (Tpost) pPre[i];
		}
		
	}
	
}


// show different datatype data in V3D
int datatype_converting(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandleList win_list = callback.getImageWindowList();
	
	if(win_list.size()<1) 
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
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
			convering<unsigned char, unsigned char>((unsigned char *)subject1d, data1d, sz_sub, cnvrt_dt);
		}
		else if(sub_dt == V3D_UINT16)
		{
			convering<unsigned short, unsigned char>((unsigned short *)subject1d, data1d, sz_sub, cnvrt_dt);
		}
		else if(sub_dt == V3D_FLOAT32)
		{
			convering<float, unsigned char>((float *)subject1d, data1d, sz_sub, cnvrt_dt);
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
			convering<unsigned char, unsigned short>((unsigned char *)subject1d, data1d, sz_sub, cnvrt_dt);
		}
		else if(sub_dt == V3D_UINT16)
		{
			convering<unsigned short, unsigned short>((unsigned short *)subject1d, data1d, sz_sub, cnvrt_dt);
		}
		else if(sub_dt == V3D_FLOAT32)
		{
			convering<float, unsigned short>((float *)subject1d, data1d, sz_sub, cnvrt_dt);
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
			convering<unsigned char, float>((unsigned char *)subject1d, data1d, sz_sub, cnvrt_dt);
		}
		else if(sub_dt == V3D_UINT16)
		{
			convering<unsigned short, float>((unsigned short *)subject1d, data1d, sz_sub, cnvrt_dt);
		}
		else if(sub_dt == V3D_FLOAT32)
		{
			convering<float, float>((float *)subject1d, data1d, sz_sub, cnvrt_dt);
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

