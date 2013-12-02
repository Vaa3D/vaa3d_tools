#include <iostream> 
#include "v3d_message.h"

#include "dataiom_plugin.h"
#include "dataiom_func.h"

using namespace std;

Q_EXPORT_PLUGIN2(data_io_manager, DataIOMPlugin);
 
QStringList DataIOMPlugin::menulist() const
{
	return QStringList() 
		<<tr("Neuron Toolbox")
		<<tr("about");
}

QStringList DataIOMPlugin::funclist() const
{
	return QStringList()
		<<tr("help");
}

void DataIOMPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("Neuron Toolbox"))
	{
        dataiom_toolbox_func(callback, parent);
	}
	else if (menu_name == tr("help"))
		help(true);
	else
        help(true);
}

bool DataIOMPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
//	cout<<"callback_dofunc="<<(void *)&callback<<endl;
//	callback.getImageWindowList();
	
	if (parent==NULL)
		cout<<"parent is NULL"<<endl;

	if (func_name == tr("neuron_toolbox"))
	{
        dataiom_toolbox_func(callback, parent, input, output);
	}
	else if (func_name == tr("help"))
	{
		help(false);
	}

    return true;
}

