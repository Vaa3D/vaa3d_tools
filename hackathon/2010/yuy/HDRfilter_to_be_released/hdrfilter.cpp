/* hdrfilter.cpp
 * 2011-03-21: create this program by Yang Yu
 */

//
#include <cmath>
#include <stdlib.h>
#include <ctime>

#include <iostream>

#include "hdrfilter.h"

#include "stackutil.h"
#include "mg_utilities.h"
#include "mg_image_lib.h"


// redefine loadimage function copy from stackutil.h remove size constrains
bool loadImageWSC(char imgSrcFile[], unsigned char *& data1d, V3DLONG * &sz, int & datatype)
{
	if (data1d)
	{
		printf("Warning: The pointer for 1d data storage is not empty. This pointer will be freed first and the  reallocated. \n");
		delete []data1d; data1d=0;
	}
	if (sz)
	{
		printf("Warning: The pointer for size is not empty. This pointer will be freed first and the  reallocated. \n");
		delete []sz; sz=0;
	}
	else
	{
		sz = new V3DLONG [4];
		if (!sz)
		{
			printf("Fail to alocate memory for the size variable.\n");
			return false;
		}
	}
	
	unsigned char *tmp_data1d =0;
	V3DLONG * tmp_sz = 0; /* note that this variable must be initialized as NULL. */
	int tmp_datatype = 0;
	bool b_5d=false;
	
	char * curFileSuffix = getSurfix(imgSrcFile);
	if (strcasecmp(curFileSuffix, "tif")==0 || strcasecmp(curFileSuffix, "tiff")==0) //read tiff stacks
	{
		if (loadTif2Stack(imgSrcFile, tmp_data1d, tmp_sz, tmp_datatype))
		{
			printf("Error happens in TIF file reading. Stop. \n");
			return false;
		}
	}
	else if ( strcasecmp(curFileSuffix, "lsm")==0 ) //read lsm stacks
	{
		if (loadLsm2Stack(imgSrcFile, tmp_data1d, tmp_sz, tmp_datatype))
		{
			printf("Error happens in LSM file reading. Stop. \n");
			return false;
		}
	}
	else if ( strcasecmp(curFileSuffix, "raw5")==0 ) //read lsm stacks
	{
		if (loadRaw5d2Stack(imgSrcFile, tmp_data1d, tmp_sz, tmp_datatype))
		{
			printf("Error happens in V3D .raw5 (5D) file reading. Stop. \n");
			return false;
		}
		b_5d = true;
	}
	else //then assume it is Hanchuan's RAW format
	{
		if (loadRaw2Stack(imgSrcFile, tmp_data1d, tmp_sz, tmp_datatype))
		{
			if (loadRaw2Stack_2byte(imgSrcFile, tmp_data1d, tmp_sz, tmp_datatype))
			{
				printf("Error happens in reading 2-byte-size RAW file. Stop. \n");
				return false;
			}
		}
	}
	
	
	//copy output data
	switch (tmp_datatype)
	{
		case 1:
			datatype = 1;
			break;
			
		case 2:
			datatype = 2;
			break;
			
		case 4:
			datatype = 4;
			break;
			
		default:
			printf("Something wrong with the program, -- should NOT display this message at all. Check your program. \n");
			if (data1d) {delete []data1d; data1d=0;}
			if (tmp_sz) {delete []tmp_sz; tmp_sz=0;}
			if (sz) {delete []sz; sz=0;}
			return false;
	}
	
	sz = new V3DLONG [5];
	sz[0] = tmp_sz[0];
	sz[1] = tmp_sz[1];
	sz[2] = tmp_sz[2];
	sz[3] = tmp_sz[3]; //no longer merge the 3rd and 4th dimensions
	sz[4] = (b_5d) ? tmp_sz[4] : 1; //090802
	
	data1d = tmp_data1d;
	
	/* clean all workspace variables */
	if (tmp_sz) {delete []tmp_sz; tmp_sz=0;}
	
	return true;
}


//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(hdrfilter, HDRFilterPlugin);

// func datatype converting main
int hdr_filtering(V3DPluginCallback2 &callback, QWidget *parent);

// func convering kernel
template <class Tpre, class Tpost>
int convering(void *subject1d, ImagePixelType v3d_dt, ImagePixelType dt);

//plugin funcs
const QString title = "HDR Filtering";
QStringList HDRFilterPlugin::menulist() const
{
    return QStringList() << tr("HDR Filtering")
						 << tr("About");
}

void HDRFilterPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("HDR Filtering"))
    {
    	hdr_filtering(callback, parent);
    }
	else if (menu_name == tr("About"))
	{
		QMessageBox::information(parent, "Version info", QString("HDR Filtering Plugin %1 (March 21, 2011) developed by Yang Yu. (Janelia Research Farm Campus, HHMI)").arg(getPluginVersion()));
		return;
	}
}

// show different datatype data in V3D
int hdr_filtering(V3DPluginCallback2 &callback, QWidget *parent)
{
	qDebug()<<"run hdr filtering ...";
	
	// get image
	HDRFilerDialog dialog(callback, parent);
	if (dialog.exec()!=QDialog::Accepted)	return -1;
	
	QString fn_image = dialog.fn_image;
	
	qDebug()<<"input is ... "<<dialog.fn_image;
	
	// load image
	unsigned char *data1d=NULL;
	V3DLONG *imgsz=NULL, sx, sy, sz, sc;
	int datatype;
	
	if(QFile::exists(fn_image))
	{
		if (loadImageWSC(const_cast<char *>( fn_image.toStdString().c_str() ), data1d, imgsz, datatype)!=true)
		{
			std::cout << "The image loading fails."<<endl;
			return false;
		}
		
		sx= imgsz[0], sy = imgsz[1], sz = imgsz[2]; sc = imgsz[3];
		
	}
	else
	{
		std::cout << "The image does not exist."<<endl;
		return false;
	}
	
	Image4DSimple p4DImage;
	
	if(datatype==1)
	{
		p4DImage.setData((unsigned char *)data1d, sx,sy,sz,sc, V3D_UINT8);
	}
	else if(datatype==2)
	{
		p4DImage.setData((unsigned char *)data1d, sx,sy,sz,sc, V3D_UINT16);
	}
	else if(datatype==4)
	{
		p4DImage.setData((unsigned char *)data1d, sx,sy,sz,sc, V3D_FLOAT32);
	}
	
	
	// HDR filtering
	HDRwidget *pHDRwidget = new HDRwidget(parent, p4DImage);
	pHDRwidget->show();

	//
	return 0;
	
}

