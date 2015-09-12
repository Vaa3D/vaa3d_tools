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
	double *result = new double [3]; 
	QString paraStr = QInputDialog::getText(parent, tr("Input trace parameter"),
		tr("Please Input Min Soma Size, Max Neurite Width, Neurite Width Multiplier"),
		QLineEdit::Normal, tr("100 5 2.5"), &ok);
	QStringList listStr = paraStr.split(' ');

	result[0] = listStr[0].toDouble();
	result[1] = listStr[1].toDouble();
        result[2] = listStr[2].toDouble();
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
                string fileDir("Vaa3d_Domenu_pyzh.swc");
		result = shared_lib_func(p4DImage->getRawData(), p4DImage->getTotalBytes(), p4DImage->getUnitBytes(), p4DImage->getXDim(), p4DImage->getYDim(), p4DImage->getZDim(), 1,3,para,fileDir);

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
		//simple_saveimage_wrapper(callback, "trace3D_result.v3draw", (unsigned char *)new_image_data, sz, V3D_UINT8);
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
	double para[3]; 
	bool* result = NULL;
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		printf("start %s,%s,%s\n",inparas[0],inparas[1],inparas[2]);
		//QString paraStr = tr(inparas[0]);
		//QStringList listStr = paraStr.split(' ');
		para[0] = tr(inparas[0]).toDouble();
                para[1] = tr(inparas[1]).toDouble();
                para[2] = tr(inparas[2]).toDouble();
		// 1 - Obtain the current 4D image pointer
		//if (input.size()>=2)
    		//{
        	//	vector<char*> para = (*(vector<char*> *)(input.at(1).p));
        	//	cout<<paras.size()<<endl;
        		//if(paras.size() >= 1) c = atoi(paras.at(0));
    		//}
		printf("para finish\n");
    		//char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
		//Image4DSimple *p4DImage = callback.getImage(curwin);
		printf("file = %d\n",input.size());
   		Image4DSimple *p4DImage = callback.loadImage(infiles[0]);
                string fileDir(infiles[0]);
                //printf("---> infile[0][0] = %s, %d\n",testStr.begin(),testStr.size());
    		if(!p4DImage || !p4DImage->valid())
    		{
         		v3d_msg("Fail to load the input image.",0);
         		if (p4DImage) {delete p4DImage; p4DImage=0;}
         		return false;
    		}
		printf("start tracing\n");
		result = shared_lib_func(p4DImage->getRawData(), p4DImage->getTotalBytes(), p4DImage->getUnitBytes(), p4DImage->getXDim(), p4DImage->getYDim(), p4DImage->getZDim(), 1,3,para,fileDir);
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
		//simple_saveimage_wrapper(callback, "trace3D_result.v3draw", (unsigned char *)new_image_data, sz, V3D_UINT8);

	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		printf("====Usage====");
                printf("./start_vaa3d.sh -x aVaaTrace3D -f func1 -p \"min_soma_size max_neurite_width neurite_width_multiplier\" -i input_image");
                printf("Sample usage: ./start_vaa3d.sh -x aVaaTrace3D -f func1 -p \"100 5 2\" -i input_image");
                printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");
	}
	else return false;

	return true;
}

