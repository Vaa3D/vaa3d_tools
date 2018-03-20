/* example_plugin.cpp
 * This is an example plugin perform binary thresholding on an image. You can use it as a demo
 * 2012-02-10 : by Yinan Wan
 */
 
#include "v3d_message.h"

#include "example_plugin.h"
#include "example_func.h"

// 1- Export the plugin class to a target, the first item in the bracket should match the TARGET parameter in the .pro file
Q_EXPORT_PLUGIN2(example, ExamplePlugin);
 

// 2- Set up the items in plugin domenu
QStringList ExamplePlugin::menulist() const
{
	return QStringList() 
		<<tr("tip_detection")
		<<tr("about");
}

// 3 - Set up the function list in plugin dofunc
QStringList ExamplePlugin::funclist() const
{
	return QStringList()
		<<tr("tip_detection")
		<<tr("help");
}

// 4 - Call the functions corresponding to the domenu items. 
//     The functions may not necessarily be in example_func.cpp, but you are strongly recommended to do so
//     to seperate the Interface from the core functions, and it is consistant with the form in plugin_creator
void ExamplePlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("tip_detection"))
	{
		int flag = tipdetection(callback,parent);
		if(flag != -1)
			v3d_msg(tr("tip detection completed!"));
	}
	else if (menu_name == tr("about"))
	{
		v3d_msg(tr("This is a demo plugin to perform tip points detection on the current image.\n"
			"Developed by Keran Liu, 2017-04"));
	}
	else
	{
		v3d_msg(tr("This is a demo plugin to perform tip points detection on the current image.\n"
			"Developed by Keran Liu, 2017-04"));
	}
}

// 5 - Call the functions corresponding to dofunc
//     The functions may not necessarily be in example_func.cpp, but you are strongly recommended to do so
//     to seperate the Interface from the core functions, and it is consistant with the form in plugin_creator


bool ExamplePlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("tip_detection"))
	{
		return true;
	}
	else if (func_name == tr("help"))
	{
		printHelp();
		return true;
	}
}
