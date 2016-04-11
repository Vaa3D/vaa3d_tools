/*
* N3DFix - automatic removal of swelling artifacts in neuronal 2D/3D reconstructions
* last update: Mar 2016
* VERSION 2.0
*
* Authors: Eduardo Conde-Sousa <econdesousa@gmail.com>
*          Paulo de Castro Aguiar <pauloaguiar@ineb.up.pt>
* Date:    Mar 2016
*
* N3DFix v2.0 is described in the following publication (!UNDER REVIEW!)
* Conde-Sousa E, Szucs P, Peng H, Aguiar P - Neuroinformatics, 2016
*
*    Disclaimer
*    ----------
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You can view a copy of the GNU General Public License at
*    <http://www.gnu.org/licenses/>.
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
        v3d_msg(tr( "\n\n ==================== N3DFix ===================\n\n"
					" Automatic removal of swelling artifacts in neuronal reconstructions\n"
					" last update: Mar 2016\n"
					" VERSION 2.0\n"
					" \n"
					" Authors: Eduardo Conde-Sousa <econdesousa@gmail.com>\n"
					"          Paulo de Castro Aguiar <pauloaguiar@ineb.up.pt>\n"
					" Date:    Mar 2016\n"
					" \n"
					" N3DFix v2.0 is described in the following publication (!UNDER REVIEW!)\n"
					" Conde-Sousa E, Szucs P, Peng H, Aguiar P - Neuroinformatics, 2016\n"
					" \n"
					"    Disclaimer\n"
					"    ----------\n"
					"    This program is free software: you can redistribute it and/or modify\n"
					"    it under the terms of the GNU General Public License as published by\n"
					"    the Free Software Foundation, either version 3 of the License, or\n"
					"    any later version.\n"
					" \n"
					"    This program is distributed in the hope that it will be useful,\n"
					"    but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
					"    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
					"    GNU General Public License for more details.\n"
					" \n"
					"    You can view a copy of the GNU General Public License at\n"
					"    <http://www.gnu.org/licenses/>."
                   ));
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

