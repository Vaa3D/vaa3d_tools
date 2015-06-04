/* VaaPortTest_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2012-01-01 : by ZZ
 */
 
#include "v3d_message.h"
#include <vector>
#include "VaaPortTest_plugin.h"
#include "func.h"
using namespace std;
Q_EXPORT_PLUGIN2(VaaPortTest, VaaPortTest);
 
QStringList VaaPortTest::menulist() const
{
	return QStringList() 
		<<tr("3DTrace")
		<<tr("menu2")
		<<tr("about");
}

QStringList VaaPortTest::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

double* VaaPortTest::getPara(QWidget *parent){
	bool ok = 0;
	double *result = new double [2]; 
	QString paraStr = QInputDialog::getText(parent, tr("Input trace parameter"),
		tr("Please Input Min Soma Size, Max Neurite Width"),
		QLineEdit::Normal, tr("400 8"), &ok);
	QStringList listStr = paraStr.split(' ');

	result[0] = listStr[0].toDouble();
	result[1] = listStr[1].toDouble();
	return result;
}

void VaaPortTest::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	bool* result = NULL;
	double* para = NULL;
	if (menu_name == tr("3DTrace"))
	{
		// 1 - Obtain the current 4D image pointer
		v3dhandle curwin = callback.currentImageWindow();
		if (!curwin)
		{
			v3d_msg("no image open");
			return;
		}
		para = VaaPortTest::getPara(parent);
		Image4DSimple *p4DImage = callback.getImage(curwin);

		result = shared_lib_func(p4DImage->getRawData(), p4DImage->getTotalBytes(), p4DImage->getUnitBytes(), p4DImage->getXDim(), p4DImage->getYDim(), p4DImage->getZDim(), 1,2,para);

		// 4 - Set and show the thresholded image in a new window
		//v3dhandle newwin = callback.newImageWindow();
		//char msg_buffer[20];
		//sprintf(msg_buffer, "Finish tracing, %d\n", sizeof(bool));
		//v3d_msg(msg_buffer);
		V3DLONG sz[4];
		sz[0] = p4DImage->getXDim();
		sz[1] = p4DImage->getYDim();
		sz[2] = p4DImage->getZDim();
		sz[3] = 1;
		unsigned char * new_image_data = new unsigned char[sz[0] * sz[1] * sz[2]];
		V3DLONG i;
		for (i = 0; i < sz[0] * sz[1] * sz[2]; i++){
			if (result[i]){
				new_image_data[i] = 255;
			}
			else{
				new_image_data[i] = 0;
			}
			//new_image_data[i] = 0;
		}
		//printf("%d\nimage size %d,%d,%d\n", new_image_data, p4DImage->getRawData()[1], sz[1], sz[2]);
		Image4DSimple *new_img = new Image4DSimple;
		new_img->setData(new_image_data, sz[0], sz[1], sz[2], 1, V3D_UINT8);//setData() will free the original memory automatically
		printf("Finish SetData\n");
		//callback.setImage(newwin, new_img);
		//callback.setImageName(newwin, QObject::tr("Trace3D"));
		//callback.updateImageWindow(newwin);
		simple_saveimage_wrapper(callback, "trace3D_result.v3draw", (unsigned char *)new_image_data, sz, V3D_UINT8);
		delete []para;
		v3d_msg("Tracing Finished.");
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by ZZ, 2012-01-01"));
	}
}

bool VaaPortTest::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
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

