/* HistogramEqualization_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2021-5-16 : by YourName
 */
 
#include "v3d_message.h"
#include "v3d_interface.h"
#include <vector>
#include "HistogramEqualization_plugin.h"
#include <iostream>

using namespace std;
//Q_EXPORT_PLUGIN2(HistogramEqualization, TestPlugin);

void equalimage(unsigned char* pdata, V3DLONG* sz);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("HistogramEqualization")
		<<tr("menu2")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("HistogramEqualization"))

	{
        cout<<"welcome"<<endl;

//		v3d_msg("To be implemented.");

	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2021-5-16"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("HistogramEqualization"))
	{
        QString imagePath = infiles.size() >= 1 ? infiles[0] : "";
        QString outImagePath = outfiles.size()>=1 ? outfiles[0] : "";
        unsigned char* pdata = 0;
        V3DLONG sz[4] = {0,0,0,0};
        int dataType = 1;
        simple_loadimage_wrapper(callback,imagePath.toStdString().c_str(), pdata, sz, dataType);
        equalimage (pdata,sz);
        simple_saveimage_wrapper(callback,outImagePath.toStdString().c_str(),pdata,sz,dataType);
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

