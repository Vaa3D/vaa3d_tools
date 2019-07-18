/* length_demo_plugin.cpp
 * A demo plugin for calculate the length of a nerou
 * by Longfei Li
 */

#include "v3d_message.h"

#include "length_demo_plugin.h"

// Export the plugin class to a target; 
Q_EXPORT_PLUGIN2(length_demo, LengthDemoPlugin);

// Set up items in plugin domenu
QStringList LengthDemoPlugin::menulist() const
{
    return QStringList() 
        <<tr("import_swc_file")
		<<tr("about");
}

// Set up funcs in plugin dofunc
QStringList LengthDemoPlugin::funclist() const
{
    return QStringList()
        <<tr("calculate")
        <<tr("help");
}

// Call the functions 
void LengthDemoPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("import_swc_file"))
    {
        calculate_length(callback, parent);
    }
    else if (menu_name == tr("about"))
    {
		v3d_msg(tr("Developed by Longfei Li, All rights reserved"));
	}
    else
    {
        v3d_msg(tr("Wooo!"));
    }

}

// Call the functions
// The funcs should in length_demo_func.cpp
bool LengthDemoPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("calculate"))
    {
        //caculate_length(callback, input, output);
    }
	else if (func_name == tr("help"))
	{
		printHelp();
	}

    return true;
}