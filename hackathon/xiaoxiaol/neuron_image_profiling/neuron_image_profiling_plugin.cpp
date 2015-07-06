/* neuron_image_profiling_plugin.cpp
 * This is a plugin for profiling image qualities for neuron tracing, via SNR and tubuliarty measures.
 * 2015-7-6 : by Xiaoxiao Liu and Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "neuron_image_profiling_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(neuron_image_profiling, image_profiling);
 
QStringList image_profiling::menulist() const
{
	return QStringList() 
		<<tr("profile_swc")
		<<tr("about");
}

QStringList image_profiling::funclist() const
{
	return QStringList()
		<<tr("profile_swc")
		<<tr("help");
}

void image_profiling::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("profile_swc"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a plugin for profiling image qualities for neuron tracing, via SNR and tubuliarty measures.. "
			"Developed by Xiaoxiao Liu and Zhi Zhou, 2015-7-6"));
	}
}

bool image_profiling::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("profile_swc"))
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

