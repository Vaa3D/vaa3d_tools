/* readtree_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-8-31 : by ouyang
 */
 
#include "v3d_message.h"
#include <vector>
#include "readtree_plugin.h"
//#include "openSWCDialog.h"
#include "readmain.h"

using namespace std;
Q_EXPORT_PLUGIN2(readtree, readfirsttree);
 
QStringList readfirsttree::menulist() const
{
	return QStringList() 
        <<tr("readtree")
		<<tr("about");
}

QStringList readfirsttree::funclist() const
{
	return QStringList()
            <<tr("readtree")
            <<tr("help");
}

void readfirsttree::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("readtree"))
    {
        readtree(callback,parent);
    }
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by ouyang, 2018-8-31"));
	}
}


bool readfirsttree::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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
