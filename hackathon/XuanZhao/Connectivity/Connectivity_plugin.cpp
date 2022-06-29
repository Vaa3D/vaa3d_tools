/* Connectivity_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2022-6-15 : by zx
 */
 
#include "v3d_message.h"
#include <vector>
#include "Connectivity_plugin.h"
#include "connectivity_func.h"

using namespace std;
Q_EXPORT_PLUGIN2(Connectivity, ConnectivityPlugin);
 
QStringList ConnectivityPlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList ConnectivityPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void ConnectivityPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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
			"Developed by zx, 2022-6-15"));
	}
}

bool ConnectivityPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
        QString imagePath = infiles.size() >= 1 ? infiles[0] : "";
        int flag = inparas.size() >= 1 ? atoi(inparas[0]) : 1;
        float ratio = inparas.size() >= 2 ? atof(inparas[1]) : 0.5;
        int thres = inparas.size() >= 3 ? atoi(inparas[2]) : 30;

        getBrainRegions(imagePath, flag, ratio, thres, callback);

	}
    else if (func_name == tr("getConnectivity"))
	{
        QString indexImagePath = infiles.size() >= 1 ? infiles[0] : "";
        QString imagePath = infiles.size() >= 2 ? infiles[1] : "";
        int flag = inparas.size() >= 1 ? atoi(inparas[0]) : 1;
        float ratio = inparas.size() >= 2 ? atof(inparas[1]) : 0.5;
        int thres = inparas.size() >= 3 ? atoi(inparas[2]) : 30;

        getIndexImageMarkers(indexImagePath, imagePath, flag, ratio, thres, callback);
	}
    else if (func_name == tr("resampleImage"))
    {
        QString imagePath = infiles.size() >= 1 ? infiles[0] : "";
        int sz_x = inparas.size() >= 1 ? atoi(inparas[0]) : 512;
        int sz_y = inparas.size() >= 2 ? atoi(inparas[1]) : 512;
        int sz_z = inparas.size() >= 3 ? atoi(inparas[2]) : 256;
        int flag = inparas.size() >= 4 ? atoi(inparas[3]) : 2;


        QString outImagePath = imagePath + "_out.v3draw";
        resample3DImage(imagePath, outImagePath, sz_x, sz_y, sz_z, flag, callback);
    }
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

