/* test_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2015-6-1 : by YourName
 */
 
 
#include "v3d_message.h"
#include <vector>
#include "binarization_plugin.h"
#include "binarization_func.h"

using namespace std;
Q_EXPORT_PLUGIN2(binarization, BinarizationPlugin);

QStringList BinarizationPlugin::menulist() const
{
	return QStringList() 
		<<tr("Line_segment_kernels")
		<<tr("experimental")
		<<tr("about");
}

QStringList BinarizationPlugin::funclist() const
{
	return QStringList()
		<<tr("LSK")
		<<tr("experimental")
		<<tr("help");
}

void BinarizationPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("Line_segment_kernels") || menu_name == tr("experimental"))
	{
		bool bmenu = true;
		input_PARA PARA;
		int mode = 0;
		if(menu_name == tr("experimental")) mode = 1;
		binarization_func(callback, parent, PARA, bmenu, mode);
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by TestMan, 2015-6-1"));
	}
}

bool BinarizationPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("LSK") || func_name == tr("experimental"))
	{
		bool bmenu = false;
		input_PARA PARA;

		vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
		vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
		vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
		vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

		PARA.l = 1; PARA.d = 3;
		if (pparas)
		{
			if(pparas->size() >= 1) PARA.l = atoi(pparas->at(0));
			if(pparas->size() >= 2) PARA.d = atoi(pparas->at(1));
		}

		if(infiles.empty())
		{
			fprintf (stderr, "Need input image. \n");
			return false;
		}
		else
			PARA.inimg_file = infiles[0];
		int k=0;
		PARA.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
		
		int mode = 0;
		if(func_name == tr("experimental")) mode = 1;
		
		binarization_func(callback, parent, PARA, bmenu, mode);
	}
	else if (func_name == tr("help"))
	{
		
		////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN


		printf("inimg_file       The input image\n");
		printf("channel          Data channel for tracing. Start from 1 (default 1).\n");

		printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

	}
	else return false;

	return true;
}
