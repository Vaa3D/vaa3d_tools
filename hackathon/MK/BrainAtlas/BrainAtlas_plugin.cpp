/* BrainAtlas_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-7-1 : by MK
 */
 
#include <vector>

#include "v3d_message.h"

#include "BrainAtlas_plugin.h"
#include "BrainAtlasManager.h"

using namespace std;

Q_EXPORT_PLUGIN2(BrainAtlas, BrainAtlasApp);
 
QStringList BrainAtlasApp::menulist() const
{
	return QStringList() 
		<<tr("Start_Brain_Atlas")
		<<tr("menu2")
		<<tr("about");
}

QStringList BrainAtlasApp::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void BrainAtlasApp::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("Start_Brain_Atlas"))
	{
		BrainAtlasManaer* managerPtr = new BrainAtlasManaer(parent, &callback);
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by MK, 2019-7-1"));
	}
}

bool BrainAtlasApp::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

