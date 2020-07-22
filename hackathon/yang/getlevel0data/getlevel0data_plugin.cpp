/* getlevel0data_plugin.cpp
 * a plugin to get level 0 data
 * 10/10/2018 : by Yang Yu
 */

#include "getlevel0data_plugin.h"
#include "getlevel0data_func.h"


Q_EXPORT_PLUGIN2(getlevel0data, GetLevel0DataPlugin);

QStringList GetLevel0DataPlugin::menulist() const
{
    return QStringList()
            <<tr("about");
}

QStringList GetLevel0DataPlugin::funclist() const
{
    return QStringList()
            <<tr("getlevel0data")
              <<tr("getPreimage_fromAno")
           <<tr("help");
}

void GetLevel0DataPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    //
    if (menu_name == tr("about"))
    {
        v3d_msg(tr("a plugin to get level 0 data. developed by Yang Yu, 10/10/2018. "));
    }
    else
    {
        v3d_msg(tr("a plugin to get level 0 data. developed by Yang Yu, 10/10/2018. "));
    }
}

bool GetLevel0DataPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    //
    if (func_name == tr("getlevel0data"))
    {
        return getlevel0data_func(input, output, callback);
    }
    else if (func_name == tr("getPreimage_fromAno"))
    {
        return getPreimage(input, output, callback);
    }
    else if (func_name == tr("help"))
    {
        cout<<"vaa3d -x getlevel0data -f getlevel0data -i inputdir input.swc -o outputdir -p scale<0/1/2/3/4/5/...>"<<endl;
        return true;
    }

    //
    return true;
}

