/* checked_mark_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-10-25 : by OY
 */
 
#include "v3d_message.h"
#include <vector>
#include "checked_mark_plugin.h"
#include "change_type.h"
using namespace std;
Q_EXPORT_PLUGIN2(checked_mark, checked_mark_backup);
 
QStringList checked_mark_backup::menulist() const
{
	return QStringList() 
        <<tr("checked_mark")
        <<tr("checked_return")
		<<tr("about");
}

QStringList checked_mark_backup::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void checked_mark_backup::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("checked_mark"))
	{
        checked_mark_type(callback,parent);
	}
    else if (menu_name == tr("checked_return"))
	{
        return_checked(callback,parent);
	}
	else
	{
        v3d_msg(tr("This is a plugin for helping annotators check annotations efficiently "
			"Developed by OY, 2018-10-25"));
	}
}

bool checked_mark_backup::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

