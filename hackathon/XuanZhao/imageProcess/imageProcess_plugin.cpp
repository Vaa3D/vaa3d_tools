/* imageProcess_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2020-1-12 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "imageProcess_plugin.h"
#include "imageprocess.h"
using namespace std;
Q_EXPORT_PLUGIN2(imageProcess, TestPlugin);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
        <<tr("enhanceImage")
		<<tr("func2")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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
			"Developed by YourName, 2020-1-12"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("enhanceImage"))
	{
        QString imgPath = infiles[0];
        QString imgNewPath = imgPath.split('.')[0] + "_result.v3draw";

        unsigned char* data1d = 0;
        V3DLONG* sz = 0;
        int datatype = 0;
        simple_loadimage_wrapper(callback,imgPath.toStdString().c_str(),data1d,sz,datatype);
        enhanceImage(data1d,sz);
        simple_saveimage_wrapper(callback,imgNewPath.toStdString().c_str(),data1d,sz,datatype);
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

