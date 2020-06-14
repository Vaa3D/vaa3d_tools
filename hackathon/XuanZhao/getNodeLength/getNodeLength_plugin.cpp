/* getNodeLength_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2020-6-13 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "getNodeLength_plugin.h"
#include "function.h"
using namespace std;
Q_EXPORT_PLUGIN2(getNodeLength, getNodeLengthPlugin);
 
QStringList getNodeLengthPlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList getNodeLengthPlugin::funclist() const
{
	return QStringList()
        <<tr("getNodeLength")
		<<tr("func2")
		<<tr("help");
}

void getNodeLengthPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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
			"Developed by YourName, 2020-6-13"));
	}
}

bool getNodeLengthPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("getNodeLength"))
	{
        QString swcfile = infiles[0];
        NeuronTree nt = readSWC_file(swcfile);
        int maxR = (inparas.size() >= 1) ? atoi(inparas[0]) : 100;
        double dendritR = (inparas.size()>=2) ? atof(inparas[1]) : 1;
        double otherR = (inparas.size()>=3) ? atof(inparas[2]) : 1;
        double thre = (inparas.size()>=4) ? atof(inparas[3]) : 1;
        getNodeLength(nt,maxR,dendritR,otherR,thre);
        writeSWC_file(swcfile.split(".").at(0)+"_result.swc",nt);
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

