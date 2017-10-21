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
#include <fstream>
#include <string>
#include <sstream>

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
    <<tr("compute global features (only the first connected tree)")
	<<tr("Help");
}

QStringList GNFPlugin::funclist() const
{
	return QStringList()
	<<tr("compute_feature_batch")
	<<tr("compute_feature")
	<<tr("help");
}



void GNFPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("compute global features"))
	{
		nf_main(callback, parent);
	}
    else if (menu_name == tr("compute global features (only the first connected tree)"))
    {
        nf_first_main(callback, parent);
    }
	else if (menu_name == tr("Help"))
	{
		v3d_msg("(version 2.0) Compute global features for a certain neuron. Developed by Yinan Wan, 2012-02-20.");
	}
	return;
}


bool GNFPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback, QWidget * parent)
{	
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name==tr("help"))
	{
		cout<<"\n\n(version 2.0) Compute global features for sigle or group of neurons.Developed by Yinan Wan 12-02-20"<<endl;
		cout<<"Input: a list of swc file names (e.g. 'a.swc b.swc') or a linker file name (.ano)"<<endl;
		cout<<"Usage: v3d -x global_neuron_feature -f compute_feature -i test.swc \n"<<endl;
		return true;
	}

	// ----- As per annotation team's request, a dofunc allowing batch operations is added here. MK, Oct, 2017
	else if (func_name==tr("compute_feature_batch"))
	{
		const char* swcDIR;
		QString inputSWCPath = infiles.at(0);
		string str = inputSWCPath.toStdString();
		swcDIR = str.c_str();

		QDir swcDir(inputSWCPath);
		QFileInfoList swcList = swcDir.entryInfoList(QDir::AllEntries);
		for (int i=0; i<swcList.size(); ++i)
		{
			if (swcList.at(i).fileName() == "."  ||  swcList.at(i).fileName() == "..") continue;
			//qDebug() << swcList.at(i).fileName();

			QString fullSWCPath = inputSWCPath + "\\" + swcList.at(i).fileName();
			//qDebug() << fullSWCPath;

			string fullSWCstring = fullSWCPath.toStdString() + ".txt";
			const char* outputSWCName = fullSWCstring.c_str();
			
			
			ofstream outfile(outputSWCName);
			NeuronTree nt = readSWC_file(fullSWCPath);
			cout<<"\n--------------Neuron #"<<(i+1)<<"----------------\n";
			double * features = new double[FNUM];
			computeFeature(nt, features);
			printFeature(features);

			string input;
			for (int i=0;i<FNUM;i++)
			{
				switch (i)
				{
					case 0:
						input = "Number of Nodes:";
						break;
					case 1:
						input = "Soma Surface:\t";
						break;
					case 2:
						input = "Number of Stems:";
						break;
					case 3:
						input = "Number of Bifurcatons:";
						break;
					case 4:
						input = "Number of Branches:";
						break;
					case 5:
						input = "Number of Tips:\t";
						break;
					case 6:
						input = "Overall Width:\t";
						break;
					case 7:
						input = "Overall Height:\t";
						break;
					case 8:
						input = "Overall Depth:\t";
						break;
					case 9:
						input = "Average Diameter:";
						break;
					case 10:
						input = "Total Length:\t";
						break;
					case 11:
						input = "Total Surface:\t";
						break;
					case 12:
						input = "Total Volume:\t";
						break;
					case 13:
						input = "Max Euclidean Distance:";
						break;
					case 14:
						input = "Max Path Distance:\t\t";
						break;
					case 15:
						input = "Max Branch Order:\t\t";
						break;
					case 16:
						input = "Average Contraction:\t\t";
						break;
					case 17:
						input = "Average Fragmentation:\t\t";
						break;
					case 18:
						input = "Average Parent-daughter Ratio:\t";
						break;
					case 19:
						input = "Average Bifurcation Angle Local:";
						break;
					case 20:
						input = "Average Bifurcation Angle Remote:";
						break;
					case 21:
						input = "Hausdorff Dimension:\t\t";
				}
				ostringstream convert;
				convert << features[i];
				string result = convert.str();
				outfile << input << "\t" << result << "\n";
				cout<<"\t"<<features[i]<<endl;
			}

			outfile.close();
		}
	} 
	// ------------------------------------------------------------------------------------------------------

	else if (func_name == tr("compute_feature"))
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
