/*
 *  neuron_feature.cpp
 *  neuron_feature 
 *
 *  Created by Wan, Yinan, on 07/07/11.
 *  Last change: Wan, Yinan, on 06/23/11.
 */

#include <QtGlobal>

#include "neuron_feature.h"
#include "v3d_message.h" 
#include "../../../v3d_main/basic_c_fun/basic_surf_objs.h"
#include "Nfmain.h"
#include "Nfmain.cpp"
#include <unistd.h>
#include <iostream>
using namespace std;


//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(neuron_feature, NFPlugin);


//plugin funcs
const QString title = "neuron_feature";

QStringList NFPlugin::menulist() const
{
    return QStringList() 
	//<<tr("neuron_feature");
	<<tr("Help");
}

QStringList NFPlugin::funclist() const
{
	return QStringList()
	<<tr("neuron_feature")
	<<tr("help");
}

void neuron_feature(V3DPluginCallback2 &callback, QWidget *parent, int method_code)
{

}

void NFPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	/*if (menu_name == tr("neuron_feature"))
	{
    		neuron_feature(callback, parent,1 );
    }
	else if (menu_name == tr("Help"))
	{
		v3d_msg("(version 0.01) Compute global features for a certain neuron.");
	}*/
	return;
}


bool NFPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback, QWidget * parent)
{	
	if (func_name==tr("help"))
	{
		cout<<"(version 0.01) Compute global features (topological or geometrical) for sigle or group of neurons."<<endl;
		cout<<"Usage: specify swc file names as input(s)\nDemo: ./v3d -x plugins/neuron_utilities/neuron_feature/libneuron_feature.so -f neuron_feature -i 2189201.CNG.swc \n";
		return true;
	}
	
	else if (func_name==tr("neuron_feature"))
	{
		cout<<"\n===============Welcome to neuron_feature Function==============="<<endl;
		nf_main(input,output); 
		return true;
	}
	return false;
}
