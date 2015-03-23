/* Paint_plugin.cpp
 * This is a paint toolbox
 * 2015-02-04 : by Yujie Li
 */
 
#include "v3d_message.h"
#include <vector>
#include "Paint_plugin.h"
#include "paint_dialog.h"

using namespace std;
Q_EXPORT_PLUGIN2(Paint, paint);
Paint_Dialog *dialog;

QStringList paint::menulist() const
{
	return QStringList() 
        <<tr("paint")
        <<tr("about");

}

QStringList paint::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void paint::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("paint"))
	{
        dialog=new Paint_Dialog(&callback, parent);
        //dialog();
        dialog->setWindowTitle("Paint");
        dialog->show();
	}
    else if (menu_name == tr("about"))
	{
        v3d_msg("This is a paint toolbox. "
                "Developed by Yujie Li, 2015-02-04");
	}
	else
	{
		v3d_msg(tr("This is a paint toolbox. "
			"Developed by Yujie Li, 2015-02-04"));
	}
}

bool paint::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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








