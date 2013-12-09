/* ting1_plugin.cpp
 * a collaboration with Hanchuan
 * 2012-12-9 : by Ting Zhao and HP
 */
 
#include "v3d_message.h"
#include <vector>
#include "ting1_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(ting1, neutube);
 
QStringList neutube::menulist() const
{
	return QStringList() 
		<<tr("neutube")
		<<tr("about");
}

QStringList neutube::funclist() const
{
	return QStringList()
		<<tr("neutube")
		<<tr("help");
}

void neutube::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("neutube"))
	{
//        system()

        neurotube_main()
	}
	else
	{
		v3d_msg(tr("a collaboration with Hanchuan. "
			"Developed by Ting Zhao and HP, 2012-12-9"));
	}
}

bool neutube::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("neutube"))
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

