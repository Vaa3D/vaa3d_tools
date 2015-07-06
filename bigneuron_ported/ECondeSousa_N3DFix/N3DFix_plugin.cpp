/* Created by:
*
* 				Eduardo Conde-Sousa [econdesousa@gmail.com]
*							and
*				Paulo de Castro Aguiar [pauloaguiar@ineb.up.pt]
*
* on Dez 10, 2014
* to visualize and remove artifacts resulting
* from the 3D reconstruction of dendrites / axons
*
* (last update: July 06, 2015)
*/
 
#include "v3d_message.h"
#include <vector>
#include <stdio.h>
#include <iostream>
#include "myHeader.h"
#include "Dend_Section.h"
#include "N3DFix_plugin.h"

using namespace std;


Q_EXPORT_PLUGIN2(N3Dfix, N3DfixPlugin)

QStringList N3DfixPlugin::menulist() const
{
	return QStringList() 
        <<tr("N3DFix")
		<<tr("about");
}

QStringList N3DfixPlugin::funclist() const
{
	return QStringList()
        <<tr("N3DFix")
		<<tr("help");
}

void N3DfixPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("N3DFix"))
    {
        _main(callback,parent);
    }
	else
	{
        v3d_msg(tr("\n\n========================= N3DFix ============================\n\n\n"
                   "Developed by                                             \n\n"
                   "\t\t    Eduardo Conde-Sousa                    \n"
                   "\t\t[econdesousa@gmail.com]                  \n\n"
                   "to visualize and remove artifacts resulting from the 3D reconstruction of dendrites/axons.\n"
                   "\n\n===========================================================\n\n"));
    }
}

bool N3DfixPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{

    if (func_name == tr("N3DFix"))
	{
        //QString path = QFileDialog::getOpenFileName(0, "Select *.swc file to work with", "", "*.swc");
        N3DFix_func(input, output);
	}
	else if (func_name == tr("help"))
    {
            printHelp(input,output);
	}
	else return false;

	return true;

}

