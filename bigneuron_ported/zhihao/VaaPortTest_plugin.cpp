/* VaaPortTest_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2012-01-01 : by ZZ
 */
 
#include "v3d_message.h"
#include <vector>
#include "VaaPortTest_plugin.h"
//#include "func.h"
using namespace std;
Q_EXPORT_PLUGIN2(VaaPortTest, VaaPortTest);
 
QStringList VaaPortTest::menulist() const
{
	return QStringList() 
		<<tr("menu1")
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

void VaaPortTest::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	bool* result = NULL;
	if (menu_name == tr("menu1"))
	{
		// 1 - Obtain the current 4D image pointer
		v3dhandle curwin = callback.currentImageWindow();
		if (!curwin)
		{
			v3d_msg("no image open");
			return;
		}
		Image4DSimple *p4DImage = callback.getImage(curwin);

		//result = shared_lib_func(p4DImage->getRawData(), p4DImage->getTotalBytes(), p4DImage->getUnitBytes(), p4DImage->getXDim(), p4DImage->getYDim(), p4DImage->getZDim(), 1);

		// 4 - Set and show the thresholded image in a new window
		v3dhandle newwin = callback.newImageWindow();
		char msg_buffer[20];
		sprintf(msg_buffer, "Finish tracing, %d\n", sizeof(bool));
		v3d_msg(msg_buffer);
		V3DLONG sz[3];
		sz[0] = p4DImage->getXDim();
		sz[1] = p4DImage->getYDim();
		sz[2] = p4DImage->getZDim();
		unsigned char * new_image_data = new unsigned char[sz[0] * sz[1] * sz[2]];
		V3DLONG i;
		for ( i = 0; i < sz[0] * sz[1] * sz[2]; i++){
			/*if (result[i]){
				new_image_data[i] = 255;
			}
			else{
				new_image_data[i] = 0;
			}*/
			new_image_data[i] = 255;
		}

		printf("%d\nimage size %d,%d,%d\n", i,sz[0],sz[1],sz[2]);
		p4DImage->setData(new_image_data, sz[0], sz[1], sz[2], 1, V3D_UINT8);//setData() will free the original memory automatically
		sprintf(msg_buffer, "Finish SetData\n");
		v3d_msg(msg_buffer);
		callback.setImage(newwin, p4DImage);
		callback.setImageName(newwin, QObject::tr("Image Thresholding"));
		callback.updateImageWindow(newwin);
		v3d_msg("To be implemented.");
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

