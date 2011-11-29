
//by Hanchuan Peng
//2009-08-14


#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "canvas_eraser.h"

QStringList CanvasEraserPlugin::menulist() const
{
    return QStringList()
	<< tr("Change canvas size and fill all 0 values")
	<< tr("Change canvas size and fill a specific value")
	<< tr("about this plugin")
	;
}

void CanvasEraserPlugin::processImage(const QString &arg, Image4DSimple *image, QWidget *parent)
{
    qDebug("processImage");
    // CMB 29 Nov 2010
    // Even if there is no image, we should show the "about" dialog.
	if (arg == tr("about this plugin"))
	{
        qDebug("about");
        QString msg = QString(
                "Canvas Eraser version %1 (2009-Aug-14): this demo is developed by Hanchuan Peng to show V3D plugin capability.")
                .arg(getPluginVersion(), 1, 'f', 1);
		QMessageBox::information(parent, "Version info", msg);
        return;
	}

	if (!image) return;

	unsigned char* data1d = image->getRawData();

	V3DLONG N = image->getTotalBytes();
	V3DLONG szx = image->getXDim(), szy = image->getYDim(), szz = image->getZDim(), szc = image->getCDim();

	if (!data1d || szx<=0 || szy<=0 || szz<=0 || szc<=0)
	{
		throw("Your data to the plugin is invalid. Check the program.");
		return;
	}

    if (arg == tr("Change canvas size and fill all 0 values") || arg == tr("Change canvas size and fill a specific value"))
    {
        bool ok;
        V3DLONG xdim = QInputDialog::getInteger(parent, tr("X dim"),
                                                 tr("X dim:"),
                                                 szx, 1, szx*2, 1, &ok);
        if (ok)
		{
			V3DLONG ydim = QInputDialog::getInteger(parent, tr("Y dim"),
                                                 tr("Y dim:"),
                                                 szy, 1, szy*2, 1, &ok);
			if (ok)
			{
				V3DLONG zdim = QInputDialog::getInteger(parent, tr("Z dim"),
													 tr("Z dim:"),
													 szz, 1, szz*2, 1, &ok);
				if (ok)
				{
					int vv = 0;

					if (arg == tr("Change canvas size and fill a specific value"))
					{
						vv = QInputDialog::getInteger(parent, tr("fill in value"),
														  tr("fill in value (between 0 and 255)"),
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

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(canvas_eraser, CanvasEraserPlugin)

