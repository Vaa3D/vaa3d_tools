/* hier_label_func.cpp
 * This plugin heirachically segments the input neuron tree and label the nodes as features in eswc file.
 * 2012-05-04 : by Yinan Wan
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "hier_label_func.h"
#include "hierachical_labeling.h"
#include "customary_structs/vaa3d_neurontoolbox_para.h"
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
	if(input.size() != 1) 
	{
		cerr<<"please specify only 1 input neuron without parameter"<<endl;
		return true;
	}

	vector<char*> * inlist = (vector<char*> *)(input.at(0).p);
	if (inlist->size()!=1)
	{
		cerr<<"please specify only 1 input neuron"<<endl;
		return false;
	}
	QString fileOpenName(inlist->at(0));

	if (output.size()!=1) return false;
	QString fileSaveName;
	vector<char*> * outlist = (vector<char*> *)(output.at(0).p);
	if (outlist->size()==0)
		fileSaveName = fileOpenName + "_hier.eswc";
	else if (outlist->size()==1)
		fileSaveName = QString(outlist->at(0));
	else
	{
		cerr<<"please specify only 1 input neuron"<<endl;
		return false;
	}

	NeuronTree nt = readSWC_file(fileOpenName);	

	if (!hierachical_labeling(nt))
	{
		cerr<<"Error in consensus skeleton!"<<endl;;
		return false;
	}

	if (!writeESWC_file(fileSaveName, nt))
	{
		cerr<<"Unable to save file"<<endl;
		return false;
	}

	return true;
}

bool hierachical_labeling_toolbox(const V3DPluginArgList & input)
{
	vaa3d_neurontoolbox_paras * paras = (vaa3d_neurontoolbox_paras *)(input.at(0).p);
	NeuronTree neuron = paras->nt;
	QString fileOpenName = neuron.file;

	if (!hierachical_labeling(neuron))
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
	if (!writeESWC_file(fileSaveName, neuron))
	{
		v3d_msg("Unable to save file");
		return -1;
	}
	return 1;
}



void printHelp()
{
	cout<<"\nHierachical Labeling: a plugin that hierachically segments the neuron structure and save it as eswc (Enhanced SWC) format. 12-05-04 by Yinan Wan"<<endl;
	cout<<"Usage: v3d -x hier_label -f hierachical_labeling -i <input_neuron> -o <output_eswc>"<<endl;
	cout<<"Parameters"<<endl;
	cout<<"\t-i <input_neuron>:  input neuron structure, in .swc or .eswc format"<<endl;
	cout<<"\t[-o] <output_eswc> :  output eswc file, with hierachy stored in the fea_val row"<<endl;
	cout<<"\t                      not required. DEFAULT file should be generated under the same folder with input neuron with a name of inputFileName_hier.eswc"<<endl;
	cout<<"Example: v3d-x hier_label -f hierachical_labeling -i input.swc -o input_labeled.swc\n"<<endl;
}
