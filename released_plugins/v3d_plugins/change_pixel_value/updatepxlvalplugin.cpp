/* updatepxlvalplugin.cpp
 * 2009-08-21: create this program by Yang Yu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "updatepxlvalplugin.h"


//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(updatepxlval, UpdatePxlValPlugin)

void processImage(V3DPluginCallback2 &callback, QWidget *parent);

QStringList UpdatePxlValPlugin::menulist() const
{
    return QStringList() << tr("Change Pixel Value")
						<<tr("about this plugin");
}


void UpdatePxlValPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if(menu_name == tr("Change Pixel Value"))
     {
          processImage(callback,parent);
     }
     else if(menu_name == tr("about this plugin"))
	{
          QMessageBox::information(parent, "Version info",
                QString("Change Pixel Value %1 (2009-Aug-21): this plugin is developed by Yang Yu.")
                .arg(getPluginVersion()));
		return;
	}

}


void processImage(V3DPluginCallback2 &callback, QWidget *parent)
{
     v3dhandle curwin = callback.currentImageWindow();
	if (!curwin)
	{
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
		return;
	}

    Image4DSimple* p4DImage = callback.getImage(curwin);

	if (!p4DImage)
	{
		QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
		return;
	}

    unsigned char* data1d = p4DImage->getRawData();
    //V3DLONG totalpxls = p4DImage->getTotalBytes();
    V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    //define datatype here
    //

	//input
	//update the pixel value

    ChangePxlValDialog d(p4DImage, parent);

    if (d.exec()!=QDialog::Accepted)
         return;
    else
    {
         //get the data
         V3DLONG nx = 0, ny = 0, nz = 0;
         int* ncval = new int [sc];
         if(!ncval)
              return;
         else
         {
              for(V3DLONG c=0; c<sc; c++)
              {
                   ncval[c] = d.nval[c]->text().toInt();
                   printf("ncval[%ld] %d \n", c, ncval[c]);
              }
         }

         nx = d.coord_x->text().toLong()-1;
         ny = d.coord_y->text().toLong()-1;
         nz = d.coord_z->text().toLong()-1;

         //change the pixel value
         for(V3DLONG c=0; c<sc; c++)
         {
              V3DLONG offsetc = c*pagesz;
              V3DLONG indLoop = offsetc + nz*M*N + ny*N + nx;
              data1d[indLoop] = ncval[c];

              printf("ncval[%ld] %d \n", c, ncval[c]);
              printf("channel %ld val %d x %ld y %ld z %ld ind %ld \n", c, data1d[indLoop], nx, ny, nz, indLoop);
         }
    }


    //de-alloc
    //		if (coord_x) {delete []coord_x; coord_x=0;}
    //		if (coord_y) {delete []coord_y; coord_y=0;}
    //		if (coord_z) {delete []coord_z; coord_z=0;}
    //
    //		if (ok) {delete []ok; ok=0;}
    //		if (cancel) {delete []cancel; cancel=0;}


}


void ChangePxlValDialog::update()
{
	//get current data
	if(!data1d) printf("No data passing");

	nx = coord_x->text().toLong()-1;
	ny = coord_y->text().toLong()-1;
	nz = coord_z->text().toLong()-1;

	for(V3DLONG c=0; c<sc; c++)
	{
		V3DLONG offsetc = c*pagesz;
		V3DLONG ind = offsetc + nz*M*N + ny*N + nx;
		nval[c]->setValue(data1d[ind]);

		//printf("channel %ld val %d x %ld y %ld z %ld ind %ld \n", c, data1d[ind], nx, ny, nz, ind);
	}

}


