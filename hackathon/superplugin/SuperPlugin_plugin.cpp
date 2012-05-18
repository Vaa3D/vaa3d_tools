/* SuperPlugin_plugin.cpp
 * This is SuperPlugin which uses multiple other plugins in a pipeline.
 * 2012-05-11 : by J Zhou
 */

#include <iostream>
#include "v3d_message.h"

#include "SuperPlugin_plugin.h"
#include "SuperPlugin_func.h"

using namespace std;

Q_EXPORT_PLUGIN2(SuperPlugin, PluginSuperPlugin);

QStringList PluginSuperPlugin::menulist() const
{
	return QStringList()
		<<tr("Multi-Plugin Calling Pipeline")
		<<tr("about");
}

QStringList PluginSuperPlugin::funclist() const
{
	return QStringList()
		<<tr("superplugin")
		<<tr("help");
}

void PluginSuperPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("Multi-Plugin Calling Pipeline"))
	{
		parse_multi_plugin(callback,parent);
	}
	else
	{
		v3d_msg(tr("This is SuperPlugin which uses multiple other plugins in a pipeline. "
			"Developed by Jianlong Zhou, 2012-05-11"));
	}
}

bool PluginSuperPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("superplugin"))
	{
		return parse_multi_plugin(input, output);
	}
	else if (func_name == tr("help"))
	{
          cout<<"Usage : v3d -x superplugin -f superplugin -i <inimg_file> -o <outimg_file> -p + <plugin1_name> <plugin1_func> <plugin1_p_paras> + <plugin2_name> <plugin2_func> <plugin2_p_paras> [+ ...]"<<endl;
		cout<<endl;
          cout<<"   +               used to separate different plugins in -p parameter"<<endl;
          cout<<"plugin1_name       name of plugin1"<<endl;
		cout<<"plugin1_func       func name of plugin1"<<endl;
		cout<<"plugin1_p_paras    -p paras for plugin1"<<endl;
		cout<<endl;
          cout<<"e.g. v3d -x superplugin -f superplugin -i ex_Repo_hb9_eve.tif -o mysuperout.raw -p + gaussian gf 3 3 3 1 1.0 + gsdt gsdt 60 1 0 1.0 + gaussian gf 3 3 3 1 1.0 "<<endl;
          cout<<endl;
		return true;
	}
}

