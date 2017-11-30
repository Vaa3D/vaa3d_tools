/* Deep_Learning_Handler_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2017-11-29 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "Deep_Learning_Handler_plugin.h"
#include <iostream>
#include <qfileinfo.h>
#include <qdir.h>>

using namespace std;
Q_EXPORT_PLUGIN2(Deep_Learning_Handler, DL_Handler);
 
QStringList DL_Handler::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList DL_Handler::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void DL_Handler::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2017-11-29"));
	}
}

bool DL_Handler::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("imageCrop4CaffePredict"))
	{
		string outputDir = infiles.at(1);
        QString patchPath = QString::fromStdString(outputDir) + "/patches";
        if (!QDir(patchPath).exists()) QDir().mkpath(patchPath);
        QString swcPath = QString::fromStdString(outputDir) + "/SWCs";
        if (!QDir(swcPath).exists()) QDir().mkpath(swcPath);
		unsigned char* ImgPtr = 0;
		V3DLONG in_sz[4];
		int datatype;
		if (!simple_loadimage_wrapper(callback, infiles.at(0), ImgPtr, in_sz, datatype))
		{
			cerr << "Error reading image file [" << infiles.at(0) << "]. Exit." << endl;
			return true;
		}
		int imgX = in_sz[0];
		int imgY = in_sz[1];
		int imgZ = in_sz[2];
		int channel = in_sz[3];

		int i_incres = imgX / 256;
		int j_incres = imgY / 256;
		int zlb = 1;
		int zhb = imgZ;
		for (int j = 0; j < j_incres; ++j)
		{
			for (int i = 0; i < i_incres; ++i)
			{
                V3DLONG VOIxyz[4];
                VOIxyz[0] = 256;
                VOIxyz[1] = 256;
                VOIxyz[2] = imgZ;
                VOIxyz[3] = channel;
                V3DLONG VOIsz = VOIxyz[0] * VOIxyz[1] * VOIxyz[2];
                unsigned char* VOIPtr = new unsigned char[VOIsz];
				int xlb = 256 * i + 1;
				int xhb = 256 * (i + 1);
				int ylb = 256 * j + 1;
				int yhb = 256 * (j + 1);
				this->cropStack(ImgPtr, VOIPtr, xlb, xhb, ylb, yhb, 1, zhb, imgX, imgY, imgZ);
                QString outimg_file = patchPath + QString("/x%1_y%2.v3draw").arg(xlb-1).arg(ylb-1);
				string filename = outimg_file.toStdString();
				const char* filenameptr = filename.c_str();
                simple_saveimage_wrapper(callback, filenameptr, VOIPtr, VOIxyz, 1);
                delete [] VOIPtr;
			}
		}

        QString m_InputfolderName = patchPath;
        QStringList imgList = importSeriesFileList_addnumbersort(m_InputfolderName);
        for (QStringList::iterator imgIt=imgList.begin(); imgIt!=imgList.end(); ++imgIt)
        {
            QStringList imgNameparse = (*imgIt).split("/");
            QString name1 = *(imgNameparse.end()-1);
            QStringList name1parse = name1.split(".");
            QString name = name1parse[0];
            QString indiSWCPath = swcPath + "/" + name;
            if (!QDir(indiSWCPath).exists()) QDir().mkpath(indiSWCPath);
            string fileFullName = (*imgIt).toStdString();
            string command = "vaa3d -x prediction_caffe -f 3D_Axon_detection_raw -i ";
            command = command + fileFullName + " -p /local2/MK/DL_work/IVSCC/ModelComparison/AlexNet_6layers_all/deploy.prototxt " +
                    "/local2/MK/DL_work/IVSCC/ModelComparison/AlexNet_6layers_all/caffenet_train_iter_240000.caffemodel " +
                    "/local2/MK/DL_work/IVSCC/ModelComparison/AlexNet_6layers_all/imagenet_mean.binaryproto 4 128 -o " +
                    indiSWCPath.toStdString() + "/traced.swc";
            const char* command_cStr = command.c_str();

            system(command_cStr);
        }

		return true;
	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

