/*
 *  compare_feature.cpp
 *  compare_feature 
 *
 *  Created by Wan, Yinan, on 07/21/11.
 *  Last change: Wan, Yinan, on 07/21/11.
 */

#include <QtGlobal>

#include "compare_feature.h"
#include "v3d_message.h" 
#include "../../../v3d_main/basic_c_fun/basic_surf_objs.h"
#include "fcompare.cpp"
#include <unistd.h>
#include <iostream>
using namespace std;




//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(compare_feature, Compare_featurePlugin);


//plugin funcs
const QString title = "compare_feature";

QStringList Compare_featurePlugin::menulist() const
{
    return QStringList();
	//<<tr("compare_feature");
	//<<tr("Help");
}

QStringList Compare_featurePlugin::funclist() const
{
	return QStringList()
	<<tr("compare_feature")
	<<tr("help");
}



void Compare_featurePlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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


bool Compare_featurePlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback, QWidget * parent)
{	
	if (func_name==tr("help"))
	{
		cout<<"(version 0.01) Find certain number of subjects in the input group that are most similar to a query."<<endl;
		cout<<"Usage: specify swc library as input(s)"<<endl;
		cout<<"-i <fileName1 fileName2 ...>:\ta library of swc files to perform comparison"<<endl;
		cout<<"-o <output fileName>:\toutput file name"<<endl;
		cout<<"-p <queryid sbjnum>:\tspecify the query id in the input lib and the number of subjects to pick up"<<endl;
		return true;
	}
	
	else if (func_name==tr("compare_feature"))
	{
		cout<<"\n===============Welcome to compare_feature Function==============="<<endl;
		bool result = compare_feature(input,output); 
		return result;
	}
	return false;
}
