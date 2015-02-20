/* shape_extractor_plugin.cpp
 * A tool to extract cell shapes
 * 2015-2-17 : by Yujie Li
 */
 
#include "v3d_message.h"
#include <vector>
#include "shape_extractor_plugin.h"
#include "shape_dialog.h"


using namespace std;
Q_EXPORT_PLUGIN2(shape_extractor, shape_extr_plugin);
static shape_dialog *dialog=0;

QStringList shape_extr_plugin::menulist() const
{
	return QStringList() 
        <<tr("Shape extractor")
		<<tr("about");
}

QStringList shape_extr_plugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void shape_extr_plugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("Shape extractor"))
	{
        dialog=new shape_dialog(&callback);
        dialog->setWindowTitle("Shape extractor");
        dialog->show();
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("A tool to extract cell shapes. "
			"Developed by Yujie Li, 2015-2-17"));
	}
}

bool shape_extr_plugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

