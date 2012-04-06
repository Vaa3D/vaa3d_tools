//by Hanchuan Peng
// 090814

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "rotateimg90.h"


//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(rotateimg90, RotateImg90Plugin)


void processImage(V3DPluginCallback2 &callback, QWidget *parent, unsigned int rotateflag);

QStringList RotateImg90Plugin::menulist() const
{
     return QStringList() << tr("Left 90 degrees in XY plane")
                          << tr("Right 90 degrees in XY plane")
                          << tr("180 degrees in XY plane")
                          << tr("about this plugin")
          ;
}


void RotateImg90Plugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
     //choosing method
	unsigned int rotateflag = 0;

	if(menu_name == tr("Left 90 degrees in XY plane"))
     {
		rotateflag  = 1;
          processImage(callback,parent, rotateflag);
     }
	else if(menu_name == tr("Right 90 degrees in XY plane"))
	{
          rotateflag  = 2;
          processImage(callback,parent, rotateflag);
     }

	else if(menu_name == tr("180 degrees in XY plane"))
	{
          rotateflag  = 3;
          processImage(callback,parent, rotateflag);
     }
     else if (menu_name == tr("about this plugin"))
	{
		QMessageBox::information(parent, "Version info",
                QString("XY Plane Image 90-degree Rotation %1 (2009-Aug-14): this demo is developed by Hanchuan Peng to show V3D plugin capability.")
                .arg(getPluginVersion()));
          return;
	}

}


void processImage(V3DPluginCallback2 &callback, QWidget *parent, unsigned int rotateflag)
{
    v3dhandle curwin = callback.currentImageWindow();
	if (!curwin)
	{
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
		return;
	}

    Image4DSimple* image = callback.getImage(curwin);

	if (!image)
	{
		QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
		return;
	}

	unsigned char* data1d = image->getRawData();

	V3DLONG N = image->getTotalBytes();
	V3DLONG szx = image->getXDim(), szy = image->getYDim(), szz = image->getZDim(), szc = image->getCDim();
	V3DLONG szchan = image->getTotalUnitNumberPerChannel(), szplane = image->getTotalUnitNumberPerPlane();
	V3DLONG i,j,k,c;

	if (!data1d || szx<=0 || szy<=0 || szz<=0 || szc<=0)
	{
		throw("Your data to the plugin is invalid. Check the program.");
		return;
	}

    if (rotateflag == 1)
    {
		unsigned char *nm = new unsigned char [N];
		switch (image->getDatatype())
		{
			case V3D_UINT8:
				{
					for (c=0;c<szc;c++)
						for (k=0;k<szz;k++)
						{
							V3DLONG offset = c*szchan + k*szplane;
							for (j=0;j<szy;j++)
								for (i=0;i<szx;i++)
									nm[offset + (szx-1-i)*szy + (j)] = data1d[offset + j*szx + i];
						}
				}
				break;
			case V3D_UINT16:
				{
					unsigned short int * nm_uint16 = (unsigned short int *)nm;
					unsigned short int * old_uint16 = (unsigned short int *)data1d;
					for (c=0;c<szc;c++)
						for (k=0;k<szz;k++)
						{
							V3DLONG offset = c*szchan + k*szplane;
							for (j=0;j<szy;j++)
								for (i=0;i<szx;i++)
									nm_uint16[offset + (szx-1-i)*szy + (j)] = old_uint16[offset + j*szx + i];
						}
				}
				break;
			case V3D_FLOAT32:
				{
					float * nm_float32 = (float *)nm;
					float * old_float32 = (float *)data1d;
					for (c=0;c<szc;c++)
						for (k=0;k<szz;k++)
						{
							V3DLONG offset = c*szchan + k*szplane;
							for (j=0;j<szy;j++)
								for (i=0;i<szx;i++)
									nm_float32[offset + (szx-1-i)*szy + (j)] = old_float32[offset + j*szx + i];
						}
				}
				break;
			default:
				return;
		}
		image->setData(nm, szy, szx, szz, szc, image->getDatatype()); //setData() will free the original memory automatically
    }
    else if (rotateflag == 2) // "Right 90 degrees in XY plane"
	{
		unsigned char *nm = new unsigned char [N];
		switch (image->getDatatype())
		{
			case V3D_UINT8:
				{
					for (c=0;c<szc;c++)
						for (k=0;k<szz;k++)
						{
							V3DLONG offset = c*szchan + k*szplane;
							for (j=0;j<szy;j++)
								for (i=0;i<szx;i++)
									nm[offset + (i)*szy + (szy-1-j)] = data1d[offset + j*szx + i];
						}
				}
				break;
			case V3D_UINT16:
				{
					unsigned short int * nm_uint16 = (unsigned short int *)nm;
					unsigned short int * old_uint16 = (unsigned short int *)data1d;
					for (c=0;c<szc;c++)
						for (k=0;k<szz;k++)
						{
							V3DLONG offset = c*szchan + k*szplane;
							for (j=0;j<szy;j++)
								for (i=0;i<szx;i++)
									nm_uint16[offset + (i)*szy + (szy-1-j)] = old_uint16[offset + j*szx + i];
						}
				}
				break;
			case V3D_FLOAT32:
				{
					float * nm_float32 = (float *)nm;
					float * old_float32 = (float *)data1d;
					for (c=0;c<szc;c++)
						for (k=0;k<szz;k++)
						{
							V3DLONG offset = c*szchan + k*szplane;
							for (j=0;j<szy;j++)
								for (i=0;i<szx;i++)
									nm_float32[offset + (i)*szy + (szy-1-j)] = old_float32[offset + j*szx + i];
						}
				}
				break;
			default:
				return;
		}
		image->setData(nm, szy, szx, szz, szc, image->getDatatype()); //setData() will free the original memory automatically
	}
    else if (rotateflag == 3) //"180 degrees in XY plane"
    {
		switch (image->getDatatype())
		{
			case V3D_UINT8:
				{
					for (c=0;c<szc;c++)
						for (k=0;k<szz;k++)
							for (j=0;j<szy/2;j++)
								for (i=0;i<szx;i++)
								{
									unsigned char tmp = data1d[c*szchan + k*szplane + j*szx + i];
									data1d[c*szchan + k*szplane + j*szx + i] = data1d[c*szchan + k*szplane + (szy-1-j)*szx + (szx-1-i)];
									data1d[c*szchan + k*szplane + (szy-1-j)*szx + (szx-1-i)] = tmp;
								}
				}
				break;
			case V3D_UINT16:
				{
					unsigned short int * old_uint16 = (unsigned short int *)data1d;
					for (c=0;c<szc;c++)
						for (k=0;k<szz;k++)
							for (j=0;j<szy/2;j++)
								for (i=0;i<szx;i++)
								{
									unsigned short int tmp = old_uint16[c*szchan + k*szplane + j*szx + i];
									old_uint16[c*szchan + k*szplane + j*szx + i] = old_uint16[c*szchan + k*szplane + (szy-1-j)*szx + (szx-1-i)];
									old_uint16[c*szchan + k*szplane + (szy-1-j)*szx + (szx-1-i)] = tmp;
								}
				}
				break;
			case V3D_FLOAT32:
				{
					float * old_float32 = (float *)data1d;
					for (c=0;c<szc;c++)
						for (k=0;k<szz;k++)
							for (j=0;j<szy/2;j++)
								for (i=0;i<szx;i++)
								{
									float tmp = old_float32[c*szchan + k*szplane + j*szx + i];
									old_float32[c*szchan + k*szplane + j*szx + i] = old_float32[c*szchan + k*szplane + (szy-1-j)*szx + (szx-1-i)];
									old_float32[c*szchan + k*szplane + (szy-1-j)*szx + (szx-1-i)] = tmp;
								}
				}
				break;
			default:
				return;
		}
    }
	else
		return;
}


