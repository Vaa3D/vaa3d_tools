/* standardize_image_plugin.cpp
 * This is a standardize plugin, you can use it as a demo.
 * 2022-5-3 : by zll
 */
 
#include "v3d_message.h"

#include <vector>
#include <iostream>
#include "standardize_image_plugin.h"
using namespace std;
//Q_EXPORT_PLUGIN2(standardize_image, standardizePlugin);

bool standardize(unsigned char* pdata, V3DLONG* sz);

QStringList standardizePlugin::menulist() const
{
	return QStringList() 
		<<tr("standardize")
		<<tr("menu2")
		<<tr("about");
}

QStringList standardizePlugin::funclist() const
{
	return QStringList()
		<<tr("standardize")
		<<tr("func2")
		<<tr("help");
}

void standardizePlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("standardize"))
	{
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a standardize plugin, you can use it as a demo.. "
			"Developed by zll, 2022-5-3"));
	}
}

bool standardizePlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("standardize"))
	{
        QString imagePath = infiles.size() >= 1 ? infiles[0] : "";
        QString outImagePath = outfiles.size()>=1 ? outfiles[0] : "";
        unsigned char* pdata = 0;
        V3DLONG sz[4] = {0,0,0,0};
        int dataType = 1;
        simple_loadimage_wrapper(callback,imagePath.toStdString().c_str(), pdata, sz, dataType);
        standardize(pdata,sz);
        simple_saveimage_wrapper(callback,outImagePath.toStdString().c_str(),pdata,sz,dataType);
	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
        cout<<"Usage : v3d -x standardize_image -f standardize -i <inimg_file> -o <outimg_file> "<<endl;
        cout<<endl;
        return true;
	}
	else return false;

	return true;
}

