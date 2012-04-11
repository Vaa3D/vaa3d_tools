/* movieZCswitch.cpp
 * 2009-09-22: create this program by Yang Yu
 * 2009-09-28. Last edit by Hanchuan Peng. only change the texts in the options
 * 2010-08-06. by hanchuan Peng, to adapt to the patch of the image4dsimple data structure
 * 2011-01-27. by Yang Yu, to support multiple datatype V3D_UINT8, V3D_UINT16, V3D_FLOAT32
 */

// Adapted and upgraded to add dofunc() by Jianlong Zhou, 2012-04-08


#include <QtGui>

#include <string>
#include <exception>
#include <iostream>
#include <algorithm>
#include <iostream>

#include "stackutil.h"
#include "movieZCswitch.h"

using namespace std;
//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(movieZCswitch, MovieZCswitchPlugin)

int changeMS(V3DPluginCallback2 &callback, QWidget *parent);
bool changeMS(const V3DPluginArgList & input, V3DPluginArgList & output);
bool converter(unsigned char* &image1d, Image4DSimple &p4DImage, V3DLONG *in_sz, unsigned int mode, ImagePixelType imgdatatype, int timepoints, void * &pResult);

const QString title = "5D Stack Converter";
QStringList MovieZCswitchPlugin::menulist() const
{
    return QStringList() << tr("5D Stack Converter")
                         << tr("about this plugin");
}

void MovieZCswitchPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("5D Stack Converter"))
     {
          changeMS(callback, parent);
     }
	else if (menu_name == tr("about this plugin"))
	{
		QMessageBox::information(parent, "Version info",
               QString("5D Stack Converter Plugin Demo %1 (2009-Sep-22) developed by Yang Yu. (Hanchuan Peng Lab, Janelia Research Farm Campus, HHMI)"
                       ).arg(getPluginVersion()));
	}
}


QStringList MovieZCswitchPlugin::funclist() const
{
	return QStringList()
		<<tr("5dconvt")
		<<tr("help");
}


bool MovieZCswitchPlugin::dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
{
     if (func_name == tr("5dconvt"))
	{
          return changeMS(input, output);
	}
	else if(func_name == tr("help"))
	{
		cout<<"Usage : v3d -x movieZC -f 5dconvt -i <inimg_file> -o <outimg_file> -p <mode> <timepoints> "<<endl;
		cout<<endl;
		cout<<"mode         1: convert from 5d to 4d, 2: convert from 4d to 5d, default 2 "<<endl;
		cout<<"timepoints   number of time points, default 1"<<endl;
		cout<<endl;
		cout<<"e.g. v3d -x movieZC -f 5dconvt -i input.raw -o output.raw -p 2 1"<<endl;
		cout<<endl;
		return true;
	}

}

bool changeMS(const V3DPluginArgList & input, V3DPluginArgList & output)
{
     cout<<"Welcome to 5D Stack Converter"<<endl;
	if(input.size()<1 || output.size() != 1) return false;

	unsigned int mode = 2, timepoints = 1;
     if (input.size()>=2)
     {
          vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
          if(paras.size() >= 1) mode = atoi(paras.at(0));
          if(paras.size() >= 2) timepoints = atoi(paras.at(1));
     }
	char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
	char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
	cout<<"inimg_file = "<<inimg_file<<endl;
	cout<<"outimg_file = "<<outimg_file<<endl;

	unsigned char * data1d = 0;
	V3DLONG * in_sz = 0;

	int datatype;
	if(!loadImage(inimg_file, data1d, in_sz, datatype)) {cerr<<"load image "<<inimg_file<<" error!"<<endl; return false;}

     Image4DSimple p4DImage;
     if(datatype == 1)
     {
          p4DImage.setData((unsigned char*)data1d, in_sz[0], in_sz[1], in_sz[2], in_sz[3], V3D_UINT8);
     }
     else if(datatype == 2)
     {
          p4DImage.setData((unsigned char*)data1d, in_sz[0], in_sz[1], in_sz[2], in_sz[3], V3D_UINT16);
     }
     else if(datatype == 4)
     {
          p4DImage.setData((unsigned char*)data1d, in_sz[0], in_sz[1], in_sz[2], in_sz[3], V3D_FLOAT32);
     }
     else
     {
          printf("\nError: The program only supports UINT8, UINT16, and FLOAT32 datatype.\n");
          return false;
     }

     // call converter
     void * pResult = NULL;

     ImagePixelType pixeltype;
     switch (datatype)
     {
          case 1:
               pixeltype = V3D_UINT8;
               break;
          case 2:
               pixeltype = V3D_UINT16;
               break;
          case 4:
               pixeltype = V3D_FLOAT32;
               break;
          default:
               break;
     }

     if(!converter(data1d, p4DImage, in_sz, mode, pixeltype, timepoints, pResult))
          return false;

     // save image
     saveImage(outimg_file, (unsigned char*)pResult, in_sz, datatype);

     if(in_sz) {delete []in_sz; in_sz=0;}
     //if(data1d) {delete []data1d; data1d=0;}
     if(pResult) {delete []pResult; pResult =0;}

     return true;
}

int changeMS(V3DPluginCallback2 &callback, QWidget *parent)
{

	v3dhandle oldwin = callback.currentImageWindow();
	Image4DSimple* image = callback.getImage(oldwin);
	if (! image)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return -1;
	}

	ImagePixelType imgdatatype = image->getDatatype();

	V3DLONG N = image->getTotalBytes();

	Image4DSimple p4DImage;

	unsigned char* image1d = image->getRawData();
	V3DLONG sx=image->getXDim(), sy=image->getYDim(), sz=image->getZDim(), sc=image->getCDim();

	V3DLONG ts_max = (sz>sc)?sz:sc;

	//get Z->C or C->Z command
	bool ok1, ok2;
	int timepoints=0;

	QStringList items;
	items << QObject::tr("4D {XYZ,Color} Stack --> 5D {XYZ,Color,Time} Stack") << QObject::tr("5D {XYZ,Color,Time} Stack --> 4D {XYZ,Color} Stack");

	QString item = QInputDialog::getItem(parent, QObject::tr("Change Movie Stack"),
										QObject::tr("Which direction do you want to change:"), items, 0, false, &ok1);


	if(ok1)
	{
		timepoints = QInputDialog::getInteger(parent, QObject::tr("Set time points"),
									  QObject::tr("Enter the number of time points:"),
									  1, 1, ts_max, 1, &ok2);
	}
	else
	{
		return -1;
	}

     unsigned int mode;
     if(!(QString::compare(item, "4D {XYZ,Color} Stack --> 5D {XYZ,Color,Time} Stack")))
          mode = 2;
     else if(!(QString::compare(item, "5D {XYZ,Color,Time} Stack --> 4D {XYZ,Color} Stack")))
          mode = 1;

     V3DLONG in_sz[4];
     in_sz[0]=sx; in_sz[1]=sy; in_sz[2]=sz; in_sz[3]=sc;
     void * pResult = NULL;

     if(!converter(image1d, p4DImage, in_sz, mode, imgdatatype, timepoints, pResult))
          return false;

	// show in v3d
	p4DImage.setData((unsigned char*)pResult, sx,sy,sz,sc, imgdatatype);

	v3dhandle newwin = callback.newImageWindow();
	callback.setImage(newwin, &p4DImage);
	callback.setImageName(newwin,  callback.getImageName(oldwin)+"_changed");
	callback.updateImageWindow(newwin);

	return true;
}



bool converter(unsigned char* &image1d, Image4DSimple &p4DImage, V3DLONG *in_sz, unsigned int mode, ImagePixelType imgdatatype, int timepoints, void * &pResult)
{
     if (!image1d || !in_sz || in_sz[0]<=0 || in_sz[1]<=0 || in_sz[2]<=0 || in_sz[3]<=0 || pResult)
     {
          v3d_msg("Invalid parameters to converter().", 0);
          return false;
     }

	pResult = NULL;

     V3DLONG sx=in_sz[0], sy=in_sz[1], sz=in_sz[2], sc=in_sz[3];

	V3DLONG ts_max = (sz>sc)?sz:sc;

     V3DLONG un, N;
     switch (imgdatatype)
     {
          case V3D_UINT8:
               un=1;
               break;
          case V3D_UINT16:
               un=2;
               break;
          case V3D_FLOAT32:
               un=4;
               break;
          default:
               un=1;
               break;
     }

     N = un*sx*sy*sz*sc;

	//Get the old image info
	if(mode == 2)// (!(QString::compare(item, "4D {XYZ,Color} Stack --> 5D {XYZ,Color,Time} Stack")))
	{
		p4DImage.setTimePackType(TIME_PACK_C);

//		V3DLONG imagecount = image->getTDim();

		V3DLONG pagesz=sx*sy;
		V3DLONG channelsz=sx*sy*sz;

		p4DImage.setTDim(timepoints);
		V3DLONG imagecount = timepoints;

		sz /= imagecount;

		if(imgdatatype == V3D_UINT8)
		{
			// init
			unsigned char *data1d = NULL;

			try
			{
				data1d = new unsigned char [N];

				memset(data1d, 0, sizeof(unsigned char)*N);
			}
			catch (...)
			{
				printf("Fail to allocate memory.\n");
				return -1;
			}

			// assign
			unsigned char *pImg = (unsigned char *)image1d;
			for(V3DLONG no=0; no<imagecount; no++)
			{
				for(V3DLONG c=0; c<sc; c++)
				{
					for(V3DLONG k =0; k<sz; k++)
					{
						V3DLONG offsetc = k*pagesz + c*pagesz*sz + no*pagesz*sz*sc;
						V3DLONG offsets = k*pagesz + c*channelsz + no*pagesz*sz;
						for(V3DLONG i=0; i<pagesz; i++)
						{
							data1d[i+offsetc] = pImg[i + offsets];
						}
					}
				}
			}

			sc = sc*imagecount;

			pResult = data1d; //
		}
		else if(imgdatatype == V3D_UINT16)
		{
			// init
			unsigned short *data1d = NULL;

			try
			{
				data1d = new unsigned short [N];

				memset(data1d, 0, sizeof(unsigned short)*N);
			}
			catch (...)
			{
				printf("Fail to allocate memory.\n");
				return -1;
			}

			// assign
			unsigned short *pImg = (unsigned short *)image1d;
			for(V3DLONG no=0; no<imagecount; no++)
			{
				for(V3DLONG c=0; c<sc; c++)
				{
					for(V3DLONG k =0; k<sz; k++)
					{
						V3DLONG offsetc = k*pagesz + c*pagesz*sz + no*pagesz*sz*sc;
						V3DLONG offsets = k*pagesz + c*channelsz + no*pagesz*sz;
						for(V3DLONG i=0; i<pagesz; i++)
						{
							data1d[i+offsetc] = pImg[i + offsets];
						}
					}
				}
			}

			sc = sc*imagecount;

			pResult = data1d; //
		}
		else if(imgdatatype == V3D_FLOAT32)
		{
			// init
			float *data1d = NULL;

			try
			{
				data1d = new float [N];

				memset(data1d, 0, sizeof(float)*N);
			}
			catch (...)
			{
				printf("Fail to allocate memory.\n");
				return -1;
			}

			// assign
			float *pImg = (float *)image1d;
			for(V3DLONG no=0; no<imagecount; no++)
			{
				for(V3DLONG c=0; c<sc; c++)
				{
					for(V3DLONG k =0; k<sz; k++)
					{
						V3DLONG offsetc = k*pagesz + c*pagesz*sz + no*pagesz*sz*sc;
						V3DLONG offsets = k*pagesz + c*channelsz + no*pagesz*sz;
						for(V3DLONG i=0; i<pagesz; i++)
						{
							data1d[i+offsetc] = pImg[i + offsets];
						}
					}
				}
			}

			sc = sc*imagecount;

			pResult = data1d; //
		}
		else
		{
			printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
			return -1;
		}
	}
	else if(mode == 1)// (!(QString::compare(item, "5D {XYZ,Color,Time} Stack --> 4D {XYZ,Color} Stack")))
	{
		p4DImage.setTimePackType(TIME_PACK_Z);

		V3DLONG pagesz=sx*sy;
//		V3DLONG channelsz=sx*sy*sz;
//		V3DLONG imagecount = image->getTDim();

		V3DLONG imagecount = timepoints;

		if(imagecount>sc)
		{
			QMessageBox::information(0, title, QObject::tr("# time points should not be greater than # color channel."));
			return -1;
		}

		sc /= imagecount;

		if(imgdatatype == V3D_UINT8)
		{
			// init
			unsigned char *data1d = NULL;

			try
			{
				data1d = new unsigned char [N];

				memset(data1d, 0, sizeof(unsigned char)*N);
			}
			catch (...)
			{
				printf("Fail to allocate memory.\n");
				return -1;
			}

			// assign
			unsigned char *pImg = (unsigned char *)image1d;
			for(V3DLONG no=0; no<imagecount; no++)
			{
				for(V3DLONG c=0; c<sc; c++)
				{
					for(V3DLONG k =0; k<sz; k++)
					{
						V3DLONG offsetc = k*pagesz + c*pagesz*sz + no*pagesz*sz*sc;
						V3DLONG offsets = k*pagesz + c*pagesz*sz*imagecount + no*pagesz*sz;
						for(V3DLONG i=0; i<pagesz; i++)
						{
							data1d[i+offsets] = pImg[i + offsetc];
						}
					}
				}
			}

			sz = sz*imagecount;

			pResult = data1d; //
		}
		else if(imgdatatype == V3D_UINT16)
		{
			// init
			unsigned short *data1d = NULL;

			try
			{
				data1d = new unsigned short [N];

				memset(data1d, 0, sizeof(unsigned short)*N);
			}
			catch (...)
			{
				printf("Fail to allocate memory.\n");
				return -1;
			}

			// assign
			unsigned short *pImg = (unsigned short *)image1d;
			for(V3DLONG no=0; no<imagecount; no++)
			{
				for(V3DLONG c=0; c<sc; c++)
				{
					for(V3DLONG k =0; k<sz; k++)
					{
						V3DLONG offsetc = k*pagesz + c*pagesz*sz + no*pagesz*sz*sc;
						V3DLONG offsets = k*pagesz + c*pagesz*sz*imagecount + no*pagesz*sz;
						for(V3DLONG i=0; i<pagesz; i++)
						{
							data1d[i+offsets] = pImg[i + offsetc];
						}
					}
				}
			}

			sz = sz*imagecount;

			pResult = data1d; //
		}
		else if(imgdatatype == V3D_FLOAT32)
		{
			// init
			float *data1d = NULL;

			try
			{
				data1d = new float [N];

				memset(data1d, 0, sizeof(float)*N);
			}
			catch (...)
			{
				printf("Fail to allocate memory.\n");
				return -1;
			}

			// assign
			float *pImg = (float *)image1d;
			for(V3DLONG no=0; no<imagecount; no++)
			{
				for(V3DLONG c=0; c<sc; c++)
				{
					for(V3DLONG k =0; k<sz; k++)
					{
						V3DLONG offsetc = k*pagesz + c*pagesz*sz + no*pagesz*sz*sc;
						V3DLONG offsets = k*pagesz + c*pagesz*sz*imagecount + no*pagesz*sz;
						for(V3DLONG i=0; i<pagesz; i++)
						{
							data1d[i+offsets] = pImg[i + offsetc];
						}
					}
				}
			}

			sz = sz*imagecount;

			pResult = data1d; //
		}
		else
		{
			printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
			return -1;
		}

	}
	else
	{
		QMessageBox::information(0, title, QObject::tr("This program only supports time series data. Your current image data type is not supported."));
		return -3;
	}

     return true;
}
