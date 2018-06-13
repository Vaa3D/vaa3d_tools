/* sample2images_plugin.cpp
 * a plugin to sample 2 images
 * 6/13/2018 : by Yang Yu
 */

#include "sample2images_plugin.h"
#include "sample2images_func.h"


Q_EXPORT_PLUGIN2(neurontree_construct, NeuReconPlugin);

ControlPanel* ControlPanel::m_controlpanel = 0;

QStringList NeuReconPlugin::menulist() const
{
    return QStringList()
            <<tr("about");
}

QStringList NeuReconPlugin::funclist() const
{
    return QStringList()
            <<tr("sample2images")
           <<tr("help");
}

void NeuReconPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    //
    if (menu_name == tr("about"))
    {
        v3d_msg(tr("a plugin to sample 2 images. developed by Yang Yu, 6/13/2018. "));
    }
    else
    {
        v3d_msg(tr("a plugin to sample 2 images. "));
    }
}

bool NeuReconPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    //
    if (func_name == tr("sample2images"))
    {
        return sample2images_func(input, output, callback);
    }
    else if (func_name == tr("help"))
    {
        cout<<"vaa3d -x sample2images -f sample2images -i image1.tif image2.tif -o output.tif"<<endl;
        return true;
    }

    //
    return true;
}

