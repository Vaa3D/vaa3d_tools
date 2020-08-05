/* app2WithPreinfo_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2020-8-4 : by ZX
 */
 
#include "v3d_message.h"
#include <vector>
#include "app2WithPreinfo_plugin.h"

#include "somefunction.h"

using namespace std;
Q_EXPORT_PLUGIN2(app2WithPreinfo, app2WithPreinfoPlugin);
 
QStringList app2WithPreinfoPlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList app2WithPreinfoPlugin::funclist() const
{
	return QStringList()
        <<tr("app2WithPreinfo")
        <<tr("app2WithPreinfoForBatch")
		<<tr("help");
}

void app2WithPreinfoPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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
			"Developed by ZX, 2020-8-4"));
	}
}

bool app2WithPreinfoPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("app2WithPreinfo"))
	{
        QString dir = (infiles.size()>=1) ? infiles[0] : "";
        QString brainPath = (infiles.size()>=2) ? infiles[1] : "";
        QString outdir = (outfiles.size()>=1) ? outfiles[0] : "";
        double ratio = (inparas.size()>=1) ? atof(inparas[0]) : 0;
        app2WithPreinfo(dir,brainPath,outdir,ratio,callback);
	}
    else if (func_name == tr("app2WithPreinfoForBatch"))
	{
        QString dir = (infiles.size()>=1) ? infiles[0] : "";
        QString brainPath = (infiles.size()>=2) ? infiles[1] : "";
        double ratio = (inparas.size()>=1) ? atof(inparas[0]) : 0;
        app2WithPreinfoForBatch(dir,brainPath,ratio,callback);
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

