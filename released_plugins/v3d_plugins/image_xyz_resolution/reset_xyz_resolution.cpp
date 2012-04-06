
//by Hanchuan Peng
//2010-06-26


#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "reset_xyz_resolution.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(example_reset_xyz_resolution, example_reset_xyz_resolutionPlugin)


void processImage(V3DPluginCallback2 &callback, QWidget *parent, unsigned int flag);

QStringList example_reset_xyz_resolutionPlugin::menulist() const
{
     return QStringList()
          << tr("check the xyz voxel size")
          << tr("set the xyz voxel size")
          << tr("about this plugin")
          ;
}


void example_reset_xyz_resolutionPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
     //choosing filter
	unsigned int flag = 0; //Gaussian 0 Max 1 min 2 Max-min 3 min-Max 4

	if(menu_name == tr("check the xyz voxel size"))
     {
		flag = 1;
          processImage(callback,parent, flag);
     }
	else if(menu_name == tr("set the xyz voxel size"))
	{
          flag = 2;
          processImage(callback,parent, flag);
     }
     else if(menu_name == tr("about this plugin"))
	{
          QMessageBox::information(parent, "Version info",
                QString("Check and set image voxel xyz resolution %1 (2010-Jun-26): this demo is developed by Hanchuan Peng to show V3D plugin capability.")
                .arg(getPluginVersion()));
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

	if (image)
	{
		QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
		return;
	}

	//if (!image) return;

     if(flag ==1)//"check the xyz voxel size"
    {
		QMessageBox::information(parent, "Resolution", QString("X resolution = %1; Y resolution = %2; Z resolution = %3").arg(image->getRezX()).arg(image->getRezY()).arg(image->getRezZ()));
	}
     else if (flag == 2) //"set the xyz voxel size"
    {
		bool ok;
        double xr = QInputDialog::getDouble(parent, "X rez",
                                                 "new X resolution:",
                                                 image->getRezX(), image->getRezX()*0.01, image->getRezX()*100, 1, &ok);
        if (ok)
		{
			double yr = QInputDialog::getDouble(parent, "Y rez",
												"new Y resolution:",
												image->getRezY(), image->getRezY()*0.01, image->getRezY()*100, 1, &ok);
			if (ok)
			{
				double zr = QInputDialog::getDouble(parent, "Z rez",
													"new Z resolution:",
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

