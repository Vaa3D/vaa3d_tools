/* test_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2015-6-1 : by YourName
 */
 
 
#include "v3d_message.h"
#include <vector>
#include "test_plugin.h"
#include "test_func.h"

using namespace std;
Q_EXPORT_PLUGIN2(test, TestPlugin);

QStringList TestPlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		bool bmenu = true;
		input_PARA PARA;
		reconstruction_func(callback,parent,PARA,bmenu);
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by TestMan, 2015-6-1"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("func1"))
	{
		bool bmenu = false;
		input_PARA PARA;

		vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
		vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
		vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
		vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

		PARA.l = 1; PARA.h = 5; PARA.d = 3;
		if (pparas)
		{
			if(pparas->size() >= 1) PARA.l = atoi(pparas->at(0));
			if(pparas->size() >= 2) PARA.h = atoi(pparas->at(1));
			if(pparas->size() >= 3) PARA.d = atoi(pparas->at(2));
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
		reconstruction_func(callback,parent,PARA,bmenu);
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
