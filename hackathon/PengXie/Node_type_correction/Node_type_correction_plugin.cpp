/* Node_type_correction_plugin.cpp
 * Node_type_correction
 * 2019-6-25 : by Peng Xie
 */
 
#include "v3d_message.h"
#include <vector>
#include "Node_type_correction_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(Node_type_correction, Node_type_correction);
 
QStringList Node_type_correction::menulist() const
{
	return QStringList() 
        <<tr("Node_type_correction")
        <<tr("about");
}

QStringList Node_type_correction::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void Node_type_correction::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("Node_type_correction"))
	{
        // Interface to be implemented
        node_type_correction(QString("D:/Vaa3D_SRC/vaa3d_tools/hackathon/PengXie/Node_type_correction/testing_data/17545_00126_MZ_YLL_LLJ_stamp_2019_06_20_21_57.ano.soma_reassigned.eswc_sorted.eswc"),
                             QString("D:/Vaa3D_SRC/vaa3d_tools/hackathon/PengXie/Node_type_correction/testing_data/output.swc")
                             );
	}
	else
	{
		v3d_msg(tr("Node_type_correction. "
			"Developed by Peng Xie, 2019-6-25"));
	}
}

bool Node_type_correction::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

