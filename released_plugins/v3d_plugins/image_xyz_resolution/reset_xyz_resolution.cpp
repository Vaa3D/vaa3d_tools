
//by Hanchuan Peng
//2010-06-26


#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "reset_xyz_resolution.h"

QStringList example_reset_xyz_resolutionPlugin::menulist() const
{
    return QStringList()
	<< tr("check the xyz voxel size")
	<< tr("set the xyz voxel size")
	<< tr("about this plugin")
	;
}

void example_reset_xyz_resolutionPlugin::processImage(const QString &arg, Image4DSimple *image, QWidget *parent)
{
	if (!image) return;

	if (arg == tr("about this plugin"))
	{
		QMessageBox::information(parent, "Version info", 
                QString("Check and set image voxel xyz resolution %1 (2010-Jun-26): this demo is developed by Hanchuan Peng to show V3D plugin capability.")
                .arg(getPluginVersion()));
	}
    else if (arg == tr("check the xyz voxel size"))
    {
		QMessageBox::information(parent, "Resolution", QString("X resolution = %1; Y resolution = %2; Z resolution = %3").arg(image->getRezX()).arg(image->getRezY()).arg(image->getRezZ()));
	}
    else if (arg == tr("set the xyz voxel size"))
    {
		bool ok;
        double xr = QInputDialog::getDouble(parent, tr("X rez"),
                                                 tr("new X resolution:"),
                                                 image->getRezX(), image->getRezX()*0.01, image->getRezX()*100, 1, &ok);
        if (ok)
		{
			double yr = QInputDialog::getDouble(parent, tr("Y rez"),
												tr("new Y resolution:"),
												image->getRezY(), image->getRezY()*0.01, image->getRezY()*100, 1, &ok);
			if (ok)
			{
				double zr = QInputDialog::getDouble(parent, tr("Z rez"),
													tr("new Z resolution:"),
													image->getRezZ(), image->getRezZ()*0.01, image->getRezZ()*100, 1, &ok);
				if (ok)
				{
					image->setRezX(xr);
					image->setRezY(yr);
					image->setRezZ(zr);
				}
			}
		}
    }
	else
		return;
}

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(example_reset_xyz_resolution, example_reset_xyz_resolutionPlugin)

