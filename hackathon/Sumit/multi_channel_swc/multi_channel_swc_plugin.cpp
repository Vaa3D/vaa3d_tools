/* multi_channel_swc_plugin.cpp
 * This plugin will creat swc files based on multiple channel information in the neuron image.
 * 2015-6-18 : by Sumit and Hanbo
 */
 
#include "v3d_message.h"
#include <vector>
#include "multi_channel_swc_plugin.h"
#include "multi_channel_swc_dialog.h"
using namespace std;
Q_EXPORT_PLUGIN2(multi_channel_swc, MultiChannelSWC);
 
QStringList MultiChannelSWC::menulist() const
{
	return QStringList() 
        <<tr("multi_channel_compute")
       <<tr("multi_channel_render")
		<<tr("about");
}

QStringList MultiChannelSWC::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("help");
}

void MultiChannelSWC::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("multi_channel_compute"))
    {
        multi_channel_swc_dialog dialog(&callback);
        dialog.exec();
	}
    else if (menu_name == tr("multi_channel_render"))
    {
    }
	else
	{
        v3d_msg(tr("This plugin will create swc files based on multiple channel information in the neuron image.."
			"Developed by Sumit and Hanbo, 2015-6-18"));
	}
}

bool MultiChannelSWC::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
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

