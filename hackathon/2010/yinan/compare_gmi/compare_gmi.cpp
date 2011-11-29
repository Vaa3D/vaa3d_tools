/*
 *  compare_gmi.cpp
 *  compare_gmi 
 *
 *  Created by Wan, Yinan, on 07/22/11.
 *  
 */

#include <QtGlobal>

#include "compare_gmi.h"
#include "v3d_message.h" 
#include "../../../v3d_main/basic_c_fun/basic_surf_objs.h"
#include "fcompare.h"
#include <unistd.h>
#include <iostream>
using namespace std;


//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(compare_gmi, Compare_gmiPlugin);


//plugin funcs
const QString title = "compare_feature";

QStringList Compare_gmiPlugin::menulist() const
{
    return QStringList();
	//<<tr("compare_feature");
	//<<tr("Help");
}

QStringList Compare_gmiPlugin::funclist() const
{
	return QStringList()
	<<tr("compare_gmi")
	<<tr("help");
}



void Compare_gmiPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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


bool Compare_gmiPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback, QWidget * parent)
{	
	if (func_name==tr("help"))
	{
		cout<<"\n(version 0.03) Find certain number of subjects in the input group that are most similar to a query based on geometric moment invariants."<<endl;
		cout<<"\n-x <plugin_dll_full_path>\ta string indicates the full path of a dll (for a plugin) to be launched."<<endl;
		cout<<"-f <function_name>\t\tcompare_gmi"<<endl;
		cout<<"-i <file>\t\t\tastring indicating the linker file that specifies the library of swc files to perform comparison"<<endl;
		cout<<"-o <file>\t\t\tname for output result file"<<endl;
		cout<<"-p <par1 par2>\t\t\tpar1: a number indicates the query id in the input lib.\tpar2: the number of subjects you want to pick up"<<endl;
		cout<<"\nUsage: ./v3d -x plugins/neuron_comparison/compare_gmi/libcompare_gmi.so -f compare_gmi -i mylinker.ano -o result.txt -p 1 10"<<endl;
		cout<<endl;
		return true;
	}
	
	else if (func_name==tr("compare_gmi"))
	{
		cout<<"\n===============Welcome to compare_gmi Function==============="<<endl;
		bool result = compare_gmi(input,output); 
		return result;
	}
	return false;
}
