/* neuronrecon_plugin.cpp
 * a plugin to construct neuron tree(s) from detected signals
 * 09/11/2017 : by Yang Yu
 */

#include "neuronrecon_plugin.h"
#include "neuronrecon_func.h"
#include "neuronrecon.h"


Q_EXPORT_PLUGIN2(neurontree_construct, NeuReconPlugin);

ControlPanel* ControlPanel::m_controlpanel = 0;

QStringList NeuReconPlugin::menulist() const
{
    return QStringList()
            <<tr("line_construct")
           <<tr("about");
}

QStringList NeuReconPlugin::funclist() const
{
    return QStringList()
            <<tr("trees2pointcloud")
           <<tr("samplingtree")
          <<tr("finetunepoints")
         <<tr("getbranchpoints")
        <<tr("getstatistics")
       <<tr("lineconstruction")
      <<tr("anisotropicfilter")
     <<tr("sort")
    <<tr("bnprocess")
    <<tr("dlprocess")
    <<tr("lmprocess")
    <<tr("test")
    <<tr("help");
}

void NeuReconPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    //
    if (menu_name == tr("line_construct"))
    {
        lineconstruct_menu(callback,parent);
    }
    else if (menu_name == tr("about"))
    {
        v3d_msg(tr("a plugin to construct neuron tree(s) from detected signals. developed by Yang Yu, 2017. "));
    }
    else
    {
        v3d_msg(tr("a plugin to construct neuron tree(s) from detected signals. "));
    }
}

bool NeuReconPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    //
    if (func_name == tr("trees2pointcloud"))
    {
        return convertTrees2Pointcloud_func(input, output, callback);
    }
    else if (func_name == tr("samplingtree"))
    {
        return samplingtree_func(input, output, callback);
    }
    else if (func_name == tr("finetunepoints"))
    {
        return finetunepoints_func(input, output, callback);
    }
    else if (func_name == tr("getbranchpoints"))
    {
        return getbranchpoints_func(input, output, callback);
    }
    else if (func_name == tr("getstatistics"))
    {
        return getStatisticsTracedNeurons_func(input, output, callback);
    }
    else if (func_name == tr("lineconstruction"))
    {
        return connectpointstolines_func(input, output, callback);
    }
    else if (func_name == tr("anisotropicfilter"))
    {
        return anisotropicimagefilter_func(input, output, callback);
    }
    else if (func_name == tr("runpipeline"))
    {
        return processpipeline_func(input, output, callback);
    }
    else if (func_name == tr("sort"))
    {
        return sort_func(input, output, callback);
    }
    else if (func_name == tr("lmprocess"))
    {
        return lmpipeline_func(input, output, callback);
    }
    else if (func_name == tr("bnprocess"))
    {
        return bnpipeline_func(input, output, callback);
    }
    else if (func_name == tr("dlprocess"))
    {
        return dlpipeline_func(input, output, callback);
    }
    else if (func_name == tr("test"))
    {
        return test_func(input, output, callback);
    }
    else if (func_name == tr("help"))
    {
        printHelp();
        return true;
    }

    //
    return true;
}

