
//by Hanchuan Peng
//2009-08-14


#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "canvas_eraser.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(canvas_eraser, CanvasEraserPlugin)

void processImage(V3DPluginCallback2 &callback, QWidget *parent, unsigned int flag);


QStringList CanvasEraserPlugin::menulist() const
{
     return QStringList()
          << tr("Change canvas size and fill all 0 values")
          << tr("Change canvas size and fill a specific value")
          << tr("about this plugin")
          ;
}


void CanvasEraserPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
     	//choosing filter
	unsigned int flag = 0;

	if(menu_name == tr("Change canvas size and fill all 0 values"))
     {
		flag = 1;
          processImage(callback,parent, flag);
     }
	else if(menu_name == tr("Change canvas size and fill a specific value"))
	{
          flag = 2;
          processImage(callback,parent, flag);
     }
     else if(menu_name == tr("about this plugin"))
	{
          QString msg = QString(
               "Canvas Eraser version %1 (2009-Aug-14): this demo is developed by Hanchuan Peng to show V3D plugin capability.")
               .arg(getPluginVersion());
		QMessageBox::information(parent, "Version info", msg);
		return;
	}

}



void processImage(V3DPluginCallback2 &callback, QWidget *parent, unsigned int flag)
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

	if (!data1d || szx<=0 || szy<=0 || szz<=0 || szc<=0)
	{
		throw("Your data to the plugin is invalid. Check the program.");
		return;
	}

     if(flag == 1 || flag== 2) // (arg == tr("Change canvas size and fill all 0 values") || arg == tr("Change canvas size and fill a specific value"))
    {
        bool ok;
        V3DLONG xdim = QInputDialog::getInteger(parent, QObject::tr("X dim"),
                                                 QObject::tr("X dim:"),
                                                 szx, 1, szx*2, 1, &ok);
        if (ok)
		{
			V3DLONG ydim = QInputDialog::getInteger(parent, QObject::tr("Y dim"),
                                                 QObject::tr("Y dim:"),
                                                 szy, 1, szy*2, 1, &ok);
			if (ok)
			{
				V3DLONG zdim = QInputDialog::getInteger(parent, QObject::tr("Z dim"),
													 QObject::tr("Z dim:"),
													 szz, 1, szz*2, 1, &ok);
				if (ok)
				{
					int vv = 0;

					if (flag == 2) //(arg == tr("Change canvas size and fill a specific value"))
					{
						vv = QInputDialog::getInteger(parent, QObject::tr("fill in value"),
														  QObject::tr("fill in value (between 0 and 255)"),
														  0, 0, 255, 1, &ok);
						if (!ok)
							return;
					}

					if (szx!=xdim || szy!=ydim || szz!=zdim)
					{
						V3DLONG tb = xdim*ydim*zdim*szc*image->getUnitBytes();
						unsigned char * nm = 0;
						try {nm = new unsigned char [tb];} catch (...) {throw("Fail to allocate memory in Canvas Eraser plugin.");}


						for (V3DLONG i=0;i<tb;i++) nm[i]=vv;

						image->setData(nm, xdim, ydim, zdim, szc, image->getDatatype()); //setData() will free the original memory automatically
					}
					else
					{
						for (V3DLONG i=0;i<N;i++)
							data1d[i] = vv;
					}
				}
			}
		}
    }
	else
		return;
}


