
#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "extrafiltersplugin.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(extrafilters, ExtraFilters);

const QString title = "V3DSingleImageInterface demo";
QStringList ExtraFilters::menulist() const
{
    return QStringList()
    << tr("Invert Color")
    << tr("Threshold...")
    << tr("about");
}

void ExtraFilters::processImage(const QString &arg, Image4DSimple *image, QWidget *parent)
{
	if (! image) return;

	unsigned char* data1d = image->getRawData();
	V3DLONG N = image->getTotalBytes();

	//throw("hello intentional error");

    if (arg == tr("Invert Color"))
    {
        for (V3DLONG i = 0; i < N; i++)
        {
        	data1d[i] = 255 - data1d[i];
        }
    }
    else
    if (arg == tr("Threshold..."))
    {
        bool ok;
        int threshold = QInputDialog::getInteger(parent, tr("Threshold"),
                                                 tr("Enter threshold:"),
                                                 100, 0, 255, 1, &ok);
        if (ok)
        {
            for (V3DLONG i = 0; i < N; i++)
            {
            	if (data1d[i] <= threshold)
            		data1d[i] = 0;
            }
        }
    }
    else
    {
    	QMessageBox::information(parent, title, 
                QString("V3DSingleImageInterface Demo version %1"
    			"\ndeveloped by Zongcai Ruan 2009. (Janelia Farm Research Campus, HHMI)")
                .arg(getPluginVersion()));
    }
}

//// can't define 2 interfaces in the same plugin lib
//// redefinition of �const char* qt_plugin_verification_data�
//Q_EXPORT_PLUGIN2(extrafilters, ExtraFilters2);
//
//QStringList ExtraFilters2::menulist() const
//{
//    return QStringList()
//    << tr("ExtraFilters2 ...1")
//    << tr("ExtraFilters2 ...2");
//}
//
//void ExtraFilters2::processImage(const QString &arg, Image4DSimple *image1, Image4DSimple *image2,
//        QWidget *parent)
//{
//
//}

