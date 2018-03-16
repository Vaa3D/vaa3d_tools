/* wrong_area_search_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-3-13 : by LXF
 */
 
#include "v3d_message.h"
#include <vector>
#include "wrong_area_search_plugin.h"
#include"find_wrong_area.h"
using namespace std;
Q_EXPORT_PLUGIN2(wrong_area_search, wrong_area_searchPlugin);
//bool find_wrong_area(const V3DPluginArgList & input, V3DPluginArgList & output);

 
QStringList wrong_area_searchPlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList wrong_area_searchPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void wrong_area_searchPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by LXF, 2018-3-13"));
	}
}

bool wrong_area_searchPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("wrong_area_search"))
	{
        find_wrong_area(input,output,callback);
    }
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

