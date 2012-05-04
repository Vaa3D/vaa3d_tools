/* hier_label_func.cpp
 * This plugin heirachically segments the input neuron tree and label the nodes as features in eswc file.
 * 2012-05-04 : by Yinan Wan
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "hier_label_func.h"
#include "hierachical_labeling.h"
#include <vector>
#include <iostream>
using namespace std;

const QString title = QObject::tr("Hierachical Labeling");

int hierachical_labeling_io(V3DPluginCallback2 &callback, QWidget *parent)
{
	QString fileOpenName;
	fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
			"",
			QObject::tr("Supported file (*.swc)"
				";;Neuron structure	(*.swc)"
				));
	if(fileOpenName.isEmpty()) 
		return -1;
		
	NeuronTree nt = readSWC_file(fileOpenName);	

	if (!hierachical_labeling(nt))
	{
		v3d_msg("Error in consensus skeleton!");
		return -1;
	}

	QString fileSaveName;
	QString defaultSaveName = fileOpenName + "_hier.eswc";
	fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save merged neuron to file:"),
			defaultSaveName,
			QObject::tr("Supported file (*.eswc)"
				";;Neuron structure	(*.eswc)"
				));
	if (!writeESWC_file(fileSaveName, nt))
	{
		v3d_msg("Unable to save file");
		return -1;
	}
	return 1;
}

bool hierachical_labeling_io(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to hierachical_labeling"<<endl;
	if(input.size() != 2 || output.size() != 1) return true;
	char * paras = 0;
	if(((vector<char*> *)(input.at(1).p))->empty()){paras = new char[1]; paras[0]='\0';}
	else paras = (*(vector<char*> *)(input.at(1).p)).at(0);
	cout<<"paras : "<<paras<<endl;

	for(int i = 0; i < strlen(paras); i++)
	{
		if(paras[i] == '#') paras[i] = '-';
	}
	cout<<"paras : "<<paras<<endl;

	return true;
}


void printHelp()
{
}
