/*
 *  neuron_feature.cpp
 *  neuron_feature 
 *
 *  Created by Wan, Yinan, on 07/07/11.
 *  Last change: Wan, Yinan, on 06/23/11.
 *  Last change: add domenu.  Wan, Yinan, on 02/20/12
 */

#include <QtGlobal>

#include "global_neuron_feature.h"
#include "v3d_message.h" 
#include "basic_surf_objs.h"
#include "Nfmain.h"
#include "Nfmain.cpp"
//#include <unistd.h>
#include <iostream>
using namespace std;


//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(global_neuron_feature, GNFPlugin);


//plugin funcs
const QString title = "global_neuron_feature";

QStringList GNFPlugin::menulist() const
{
    return QStringList() 
	<<tr("compute global features")
	<<tr("Help");
}

QStringList GNFPlugin::funclist() const
{
	return QStringList()
	<<tr("compute_feature")
	<<tr("help");
}



void GNFPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("compute global features"))
	{
		nf_main(callback, parent);
	}
	else if (menu_name == tr("Help"))
	{
		v3d_msg("(version 2.0) Compute global features for a certain neuron. Developed by Yinan Wan, 2012-02-20.");
	}
	return;
}


bool GNFPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback, QWidget * parent)
{	
	if (func_name==tr("help"))
	{
		cout<<"\n\n(version 2.0) Compute global features for sigle or group of neurons.Developed by Yinan Wan 12-02-20"<<endl;
		cout<<"Input: a list of swc file names (e.g. 'a.swc b.swc') or a linker file name (.ano)"<<endl;
		cout<<"Usage: v3d -x global_neuron_feature -f compute_feature -i test.swc \n"<<endl;
		return true;
	}

	else if (func_name==tr("compute_feature"))
	{
		cout<<"\n===============Welcome to compute_feature Function==============="<<endl;
		nf_main(input,output); 
		return true;
	}

	else if (func_name==tr("TOOLBOXcompute global features"))
	{
		nf_toolbox(input);
		return true;
	}
	return false;
}
