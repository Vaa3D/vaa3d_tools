/* Deconvolution_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2013-07-05 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "Deconvolution_plugin.h"
#include <QtGui>
#include <fstream>
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <vector>
#include "stackutil.h"


using namespace std;
Q_EXPORT_PLUGIN2(Deconvolution, TestPlugin);

void processImage(V3DPluginCallback2 &callback, QWidget *parent);
bool processImage(const V3DPluginArgList & input, V3DPluginArgList & output);

 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
		<<tr("Deconv")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
		<<tr("Deconv")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("Deconv"))
	{
		processImage(callback,parent);
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2013-07-05"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("Deconv"))
	{
	}
	else if (func_name == tr("help"))
	{
	}
	else return false;
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

	//input
	bool ok1;
	unsigned int c=1;

	if(ok1)
	{
		c = QInputDialog::getInteger(parent, "Channel",
									 "Enter channel NO (starts from 1):",
									 1, 1, sc, 1, &ok4);
	}
	else
		return;


     // filter
     V3DLONG in_sz[4];
     in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[4] = sc;

    ImagePixelType pixeltype = p4DImage->getDatatype();
    void* outimg = 0; 
    switch (pixeltype)
    {
        case V3D_UINT8: AdpThresholding(data1d, in_sz,c,(unsigned char* &)outimg); break;
        case V3D_UINT16: AdpThresholding((unsigned short int *)data1d, in_sz,c,(unsigned short int* &)outimg); break;
        case V3D_FLOAT32: AdpThresholding((float *)data1d, in_sz,c,(float* &)outimg);break;
        default: v3d_msg("Invalid data type. Do nothing."); return;
    }
	
  
    
     // display
     Image4DSimple * new4DImage = new Image4DSimple();
     new4DImage->setData((unsigned char *)outimg, V3DLONG(N), V3DLONG(M), V3DLONG(P), 1, pixeltype);
     v3dhandle newwin = callback.newImageWindow();
     callback.setImage(newwin, new4DImage);
     callback.setImageName(newwin, "3D enhancement result");
     callback.updateImageWindow(newwin);

}


