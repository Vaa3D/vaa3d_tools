/* neuronrecon_plugin.cpp
 * a plugin to reconstruct neuron from multiple traced neurons
 * 09/11/2017 : by Yang Yu
 */

#include "neuronrecon_plugin.h"
#include "neuronrecon_func.h"

Q_EXPORT_PLUGIN2(neuronrecon, NeuReconPlugin);

QStringList NeuReconPlugin::menulist() const
{
    return QStringList()
            <<tr("NeuronReconstruction")
           <<tr("about");
}

QStringList NeuReconPlugin::funclist() const
{
    return QStringList()
            <<tr("neurecon")
           <<tr("help");
}

void NeuReconPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    //
    if (menu_name == tr("NeuronReconstruction"))
    {
        neuronrecon_menu(callback,parent);
    }
    else if (menu_name == tr("about"))
    {
        v3d_msg(tr("a plugin to reconstruct neuron from multiple traced neurons. "));
    }
    else
    {
        v3d_msg(tr("a plugin to reconstruct neuron from multiple traced neurons. "));
    }
}

bool NeuReconPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    //
    if (func_name == tr("neurecon"))
    {
        return neuronrecon_func(input, output, callback);
    }
    else if (func_name == tr("help"))
    {
        printHelp();
        return true;
    }

    //
    return true;
}

