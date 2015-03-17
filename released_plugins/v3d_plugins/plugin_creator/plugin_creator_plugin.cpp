/* plugin_creator_plugin.cpp
 * This plugin is used to produce v3d plugin project from a template file
 * 2012-01-27 : by Hang Xiao
 */
 
#include "v3d_message.h"

#include "plugin_creator_plugin.h"
#include "plugin_creator_func.h"
#include "produce_simplest_plugin.h"
 
Q_EXPORT_PLUGIN2(plugin_creator, PluginCreatorPlugin);

QStringList PluginCreatorPlugin::menulist() const
{
	return QStringList()
		<<tr("create plugin")
        <<tr("create plugin for neuron reconstruction")
//		<<tr("produce simplest plugin")
//		<<tr("domenu usage demo")
//		<<tr("load image and swc demo")
		<<tr("about");
}

void PluginCreatorPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("create plugin"))
	{
		create_plugin(callback,parent);
	}
    else if(menu_name == tr("create plugin for neuron reconstruction"))
    {
        create_plugin_neuronrec(callback,parent);
    }
	else if(menu_name == tr("produce simplest plugin"))
	{
		QString dir = QFileDialog::getExistingDirectory(0, tr("Save to directory"),
				"/home",
				QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);	
		produce_simplest_plugin(dir.toStdString());
		QMessageBox::information(0, "Success", tr("Plugin codes are produced in %1\n"
					">> cd %1\n"
					">> qmake && make").arg(dir));
	}
	else if(menu_name == tr("domenu usage demo"))
	{
		create_demo1(callback,parent);
	}
	else if(menu_name == tr("load image and swc demo"))
	{
		create_demo2(callback,parent);
	}
	else
	{
		v3d_msg(tr("This plugin is used to produce v3d plugin project from a template file. "
			"Developed by Hang Xiao, 2012-01-27"));
	}
}
