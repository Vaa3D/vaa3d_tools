/* swcFileRename_plugin.cpp
 * This is a plugin which intends to fix messy swc files.
 * 2020-9-14 : by MK
 */
 
#include <vector>

#include "v3d_message.h"
#include "swcFileRename_plugin.h"

using namespace std;

Q_EXPORT_PLUGIN2(swcFileRename, swcFileRename);
 
QStringList swcFileRename::menulist() const
{
	return QStringList() 
		<<tr("start")
		<<tr("about");
}

QStringList swcFileRename::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void swcFileRename::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("start"))
	{
		this->dlgPtr = new SWC_renameDlg(nullptr, &callback);
		this->dlgPtr->exec();
	}
	else
	{
		v3d_msg(tr("This is a plugin which intends to fix messy swc files.. "
			"Developed by MK, 2020-9-14"));
	}
}

bool swcFileRename::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

