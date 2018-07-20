/* neuronfeature_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-5-28 : by binWW
 */

#include "v3d_message.h" 
#include "basic_surf_objs.h"
#include "Nfmain.h"

#include <vector>
#include "neuronfeature_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(neuronfeature, NeuronFeature);
 
QStringList NeuronFeature::menulist() const
{
	return QStringList() 
		<<tr("compute_feature")
		<<tr("about")
		<<tr("about");
}

QStringList NeuronFeature::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void NeuronFeature::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("compute_feature"))
	{
		nf__main(callback, parent);
	}
	else if (menu_name == tr("about"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by binWW, 2018-5-28"));
	}
}

bool NeuronFeature::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		nf__main(callback, parent);
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

