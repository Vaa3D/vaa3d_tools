/* missing_branch_detection_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-01-13 : by LMG
 */
 
#include "v3d_message.h"
#include <vector>
#include "missing_branch_detection_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(missing_branch_detection, missing_branch);
 
QStringList missing_branch::menulist() const
{
	return QStringList() 
        <<tr("missing_branch_detection")
		<<tr("about");
}

QStringList missing_branch::funclist() const
{
	return QStringList()
        <<tr("missing_branch_detection")
        <<tr("help");
}

void missing_branch::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("missing_branch_detection"))
	{
        if(callback.currentImageWindow())
        {
            v3dhandle curwin = callback.currentImageWindow();
            Image4DSimple* p4DImage = callback.getImage(curwin);
            get_missing_branches_menu(callback,parent,p4DImage);
        }
        else v3d_msg(tr("An open 3D image is needed."));
	}
	else
	{
        v3d_msg(tr("This is a plugin for missing branch detection."
            "Developed by LMG, 2019-01-13"));
	}
}

bool missing_branch::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{

    if (func_name == tr("missing_branch_detection"))
	{
        get_missing_branches_func(input,output,callback);
	}
	else if (func_name == tr("help"))
	{
        printHelp(input,output);
    }
	else return false;

	return true;
}

