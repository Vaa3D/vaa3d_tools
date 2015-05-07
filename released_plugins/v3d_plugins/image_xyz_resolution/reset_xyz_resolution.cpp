
//by Hanchuan Peng
//2010-06-26

// Adapted and upgraded to V3DPluginInterface2_1 by Jianlong Zhou, 2012-04-05
// add dofunc() by Jianlong Zhou, 2012-04-17


#include <QtGui>
#include <iostream>
#include <math.h>
#include <stdlib.h>

#include "reset_xyz_resolution.h"

using namespace std;

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(example_reset_xyz_resolution, example_reset_xyz_resolutionPlugin)


void processImage(V3DPluginCallback2 &callback, QWidget *parent, unsigned int flag);
bool processImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output, unsigned int flag);

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
     //choosing flag
	unsigned int flag = 0;

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



QStringList example_reset_xyz_resolutionPlugin::funclist() const
{
	return QStringList()
		// <<tr("checksz")
          // <<tr("setsz")
		<<tr("help");
}


bool example_reset_xyz_resolutionPlugin::dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
{
     if(func_name == tr("help"))
     {
          cout<<"Usage : v3d -x resolution -f help"<<endl;
          cout<<"The resolution is not saved in VAA3D image. Please use domenu() function to check and set it."<<endl;
          return true;
     }
     // if (func_name == tr("checksz"))
	// {
	// 	return processImage(input, output, 1);
	// }
     // else if (func_name == tr("setsz"))
	// {
	// 	return processImage(input, output, 2);
	// }
	// else if(func_name == tr("help"))
	// {
	// 	cout<<"Usage : v3d -x resolution -f setsz -i <inimg_file> -o <outimg_file> -p <rezx> <rezy> <rezz> "<<endl;
	// 	cout<<endl;
	// 	cout<<"rezx          voxel size in x direction, default 1"<<endl;
	// 	cout<<"rezy          voxel size in y direction, default 1"<<endl;
	// 	cout<<"rezz          voxel size in z direction, default 1"<<endl;
     //      cout<<endl;
	// 	cout<<"e.g. v3d -x resolution -f setsz -i input.raw -o output.raw -p 1 1 1"<<endl;
	// 	cout<<endl;
     //      cout<<"Usage : v3d -x resolution -f checksz -i <inimg_file> "<<endl;
	// 	cout<<endl;
	// 	return true;
	// }
}

bool processImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output, unsigned int flag)
{
	// cout<<"Welcome to image resolution plugin"<<endl;

	// if (input.size() < 1) return false;

     // unsigned int rezx=1, rezy=1, rezz=1;

     // if(flag == 2)
     // {
     //      if (output.size() != 1) return false;
     //      if (input.size()>=2)
     //      {
     //           vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
     //           if(paras.size() >= 1) rezx = atoi(paras.at(0));
     //           if(paras.size() >= 2) rezy = atoi(paras.at(1));
     //           if(paras.size() >= 3) rezz = atoi(paras.at(2));

     //           cout<<"rezx = "<<rezx<<endl;
     //           cout<<"rezy = "<<rezy<<endl;
     //           cout<<"rezz = "<<rezz<<endl;
     //      }
     // }

	// char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
	// char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);

	// cout<<"inimg_file = "<<inimg_file<<endl;
	// if(flag == 2) cout<<"outimg_file = "<<outimg_file<<endl;

	// unsigned char * data1d = 0,  * outimg1d = 0;
	// V3DLONG * in_sz = 0;

	// int datatype;
	// if(!loadImage(inimg_file, data1d, in_sz, datatype))
     // {
     //      cerr<<"load image "<<inimg_file<<" error!"<<endl;
     //      return false;
     // }

	// //input
     // float* outimg = 0;

     // // if(flag ==1)//"check the xyz voxel size"
     // // {
     // //      QString str = QString("X resolution = %1; Y resolution = %2; Z resolution = %3").arg(image->getRezX()).arg(image->getRezY()).arg(image->getRezZ());
	// // 	v3d_msg(str, 0);
	// // }
     // // else if (flag == 2) //"set the xyz voxel size"
     // // {
     // //      image->setRezX(xr);
     // //      image->setRezY(yr);
     // //      image->setRezZ(zr);
     // // }
	// // else
     // // {
     // //      v3d_msg("Invalid flag.");
	// // 	return false;
     // // }

     // // saveImage(outimg_file, (unsigned char *)outimg, in_sz, datatype);

     // if (outimg) {delete []outimg; outimg =0;}
     // if (data1d) {delete []data1d; data1d=0;}
     // if (in_sz) {delete []in_sz; in_sz=0;}

     return true;
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

