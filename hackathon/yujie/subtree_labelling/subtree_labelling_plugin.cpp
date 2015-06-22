/* subtree_labelling_plugin.cpp
 * This tree labels dendrite id, segment id and branch order
 * 2015-6-19 : by Yujie Li
 */
 
#include "v3d_message.h"
#include <vector>
#include "subtree_labelling_plugin.h"
#include "subtree_dialog.h"

using namespace std;
Q_EXPORT_PLUGIN2(subtree_labelling, subtree_label);
 
QStringList subtree_label::menulist() const
{
	return QStringList() 
		<<tr("subtree_labelling")
		<<tr("about");
}

QStringList subtree_label::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void subtree_label::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("subtree_labelling"))
	{
        subtree_dialog *dialog=new subtree_dialog(&callback);
        dialog->get_swc_marker();
        dialog->assign_marker_type();
        dialog->subtree_extract();
	}
	else
	{
		v3d_msg(tr("This tree labels dendrite id, segment id and branch order. "
			"Developed by Yujie Li, 2015-6-19"));
	}
}

bool subtree_label::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
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

