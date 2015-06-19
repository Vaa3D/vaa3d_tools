/* N3Dfix_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2015-05-04 : by Eduardo Conde-Sousa
 */
 
#include "v3d_message.h"
#include <vector>
#include <stdio.h>
#include <iostream>
#include "myHeader.h"
#include "Dend_Section.h"
#include "N3Dfix_plugin.h"

using namespace std;


Q_EXPORT_PLUGIN2(N3Dfix, N3DfixPlugin)
 
QStringList N3DfixPlugin::menulist() const
{
	return QStringList() 
		<<tr("Run_menu")
		<<tr("about");
}

QStringList N3DfixPlugin::funclist() const
{
	return QStringList()
		<<tr("Run_cmd")
		<<tr("help");
}

void N3DfixPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("Run_menu"))
    {


        //QString path = QFileDialog::getOpenFileName(0, "Select *.swc file to work with", "", "*.swc");
        _main(callback);

        //v3d_msg("To be implemented.");
    }
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Eduardo Conde-Sousa, 2015-05-04"));
	}
}

bool N3DfixPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("Run_cmd"))
	{
        //QString path = QFileDialog::getOpenFileName(0, "Select *.swc file to work with", "", "*.swc");
        _main(callback);

	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

