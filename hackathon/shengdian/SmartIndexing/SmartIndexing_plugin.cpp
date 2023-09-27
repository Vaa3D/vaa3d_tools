/* SmartIndexing_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2023-9-3 : by SD-Jiang
 */
 
#include "v3d_message.h"
#include <vector>
#include "SmartIndexing_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(SmartIndexing, SmartIndexing);
 
QStringList SmartIndexing::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("about");
}

QStringList SmartIndexing::funclist() const
{
	return QStringList()
        <<tr("vol_filter")
       <<tr("vol_signals")
		<<tr("help");
}

void SmartIndexing::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by SD-Jiang, 2023-9-3"));
	}
}

bool SmartIndexing::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("vol_signals"))
	{
        return volume_signals(callback,input,output);
	}
    else if (func_name == tr("vol_filter"))
    {
        return volume_filter(callback,input,output);
    }
    else if (func_name == tr("terafly_vol_index"))
    {
        return terafly_vol_index(callback,input,output);
    }
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

