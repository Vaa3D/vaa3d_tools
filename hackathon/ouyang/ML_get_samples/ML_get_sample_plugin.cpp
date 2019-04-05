/* ML_get_sample_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-12-6 : by OYQ
 */
 
#include "v3d_message.h"
#include <vector>
#include "ML_get_sample_plugin.h"
#include "APP1_pruning.h"
using namespace std;
Q_EXPORT_PLUGIN2(ML_get_sample, ML_sample);
 
QStringList ML_sample::menulist() const
{
	return QStringList() 
        <<tr("suspicious_tips")
		<<tr("about");
}

QStringList ML_sample::funclist() const
{
	return QStringList()
        <<tr("get_undertraced_sample")
        <<tr("get_overtraced_sample")
        <<tr("get_2D&3D_block")
        <<tr("get_2D_block")
        <<tr("remove_tip_location")
        <<tr("prune_tip_APP1")
        <<tr("find_fake_tip")
        <<tr("help")
        <<tr("help1");
}

void ML_sample::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("suspicious_tips"))
	{
        find_short_branch_tips_menu(callback,parent);
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by OYQ, 2018-12-6"));
	}
}

bool ML_sample::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{

    if (func_name == tr("get_undertraced_sample"))
	{
        get_undertraced_sample(input,output,callback);
	}
    else if (func_name == tr("get_overtraced_sample"))
    {
        get_overtraced_sample(input,output,callback);
    }
    else if (func_name == tr("get_2D&3D_block"))
    {
        get_block(input,output,callback);
    }
    else if (func_name == tr("get_2D_block"))
    {
        get_2d_image(input,output,callback);
    }
    else if (func_name == tr("remove_tip_location"))
    {
        get_tip_image(input,output,callback);
    }
    else if (func_name == tr("prune_tip_APP1"))
    {
        pruning_covered_leaf_single_cover(input,output,callback);
    }
    else if (func_name == tr("find_fake_tip"))
    {
        find_short_branch_tips_func(input,output,callback);
    }
	else if (func_name == tr("help"))
	{
        printHelp(input,output);
	}
    else if (func_name == tr("help1"))
    {
        printHelp1(input,output);
    }
	else return false;

	return true;
}

