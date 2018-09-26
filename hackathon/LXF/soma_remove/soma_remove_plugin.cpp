/* soma_remove_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-7-11 : by LXF
 */
 
#include "v3d_message.h"
#include <vector>
#include "soma_remove_plugin.h"
#include "soma_remove_main.h"
#include "data_training.h"
using namespace std;
Q_EXPORT_PLUGIN2(soma_remove, soma_removePlugin);
 
QStringList soma_removePlugin::menulist() const
{
	return QStringList() 
        <<tr("soma_remove")
        <<tr("data_training")
		<<tr("about");
}

QStringList soma_removePlugin::funclist() const
{
	return QStringList()
        <<tr("soma_remove")
        <<tr("data_training")
		<<tr("help");
}

void soma_removePlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("soma_remove"))
	{
        v3dhandle win = callback.currentImageWindow();
        Image4DSimple * img = callback.getImage(win);
        V3DLONG in_sz[4];
        in_sz[0] = img->getXDim();
        in_sz[1] = img->getYDim();
        in_sz[2] = img->getZDim();
        in_sz[3] = img->getCDim();

        //soma_remove_main(img->getRawData(),in_sz,callback);
        soma_remove_main_2(img->getRawData(),in_sz,callback);
        //v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by LXF, 2018-7-11"));
	}
}

bool soma_removePlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("soma_remove"))
	{
		v3d_msg("To be implemented.");
	}
    else if (func_name == tr("data_training"))
	{
        //QString filelistOpenName = QString(inlist->at(0));
        //QString filelistfileOpenName2 = QString(inlist->at(1));
        //QString raw_img = QString(inlist->at(2));
        data_training(input,output,callback);
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}
