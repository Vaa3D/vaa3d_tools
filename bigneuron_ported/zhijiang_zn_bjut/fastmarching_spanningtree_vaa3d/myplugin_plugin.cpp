/* my_plugin.cpp
 * This is an my plugin perform binary thresholding on an image. You can use it as a demo
 * 2012-02-10 : by Yinan Wan
 */
 
#include "v3d_message.h"

#include "myplugin_plugin.h"
#include "myplugin_func.h"

// 1- Export the plugin class to a target, the first item in the bracket should match the TARGET parameter in the .pro file
Q_EXPORT_PLUGIN2(myplugin, myPlugin);
 

// 2- Set up the items in plugin domenu
QStringList myPlugin::menulist() const
{
        return QStringList() 
                <<tr("myplugin")
                <<tr("about");
}

// 3 - Set up the function list in plugin dofunc
QStringList myPlugin::funclist() const
{
        return QStringList()
                <<tr("myplugin")
                <<tr("help");
}

// 4 - Call the functions corresponding to the domenu items. 
//     The functions may not necessarily be in my_func.cpp, but you are strongly recommended to do so
//     to seperate the Interface from the core functions, and it is consistant with the form in plugin_creator
void myPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
        if (menu_name == tr("myplugin"))
        {
            proc(callback,parent);
        }
        else if (menu_name == tr("about"))
        {
                v3d_msg(tr("This is a demo plugin to perform binary thresholding on the current image.\n"
                        "Developed by Yinan Wan, 2012-02-10"));
        }
        else
        {
                v3d_msg(tr("This is a demo plugin to perform binary thresholding on the current image.\n"
                        "Developed by Yinan Wan, 2012-02-10"));
        }
}

// 5 - Call the functions corresponding to dofunc
//     The functions may not necessarily be in my_func.cpp, but you are strongly recommended to do so
//     to seperate the Interface from the core functions, and it is consistant with the form in plugin_creator
bool myPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
        if (func_name == tr("myplugin"))
        {
        }
        else if (func_name == tr("help"))
        {
        }
		return true;
}
