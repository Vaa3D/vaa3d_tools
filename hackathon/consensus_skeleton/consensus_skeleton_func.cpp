/* consensus_skeleton_func.cpp
 * a plugin to merge multiple neurons by generating a consensus skeleton
 * 2012-05-02 : by Yinan Wan
 */

#include <v3d_interface.h>
#include "basic_surf_objs.h"
#include "v3d_message.h"
#include "consensus_skeleton_func.h"
#include "consensus_skeleton.h"
#include <vector>
#include <iostream>
using namespace std;

const QString title = QObject::tr("Consensus Skeleton");

int consensus_skeleton_io(V3DPluginCallback2 &callback, QWidget *parent)
{
	QString fileOpenName;
	fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
			"",
			QObject::tr("Supported file (*.ano)"
				";;Neuron structure	(*.ano)"
				));
	if(fileOpenName.isEmpty()) 
		return -1;
		
	P_ObjectFileType linker_object;
	if (!loadAnoFile(fileOpenName,linker_object))
	{
		fprintf(stderr,"Error in reading the linker file.\n");
		return -1;
	}
	
	QStringList nameList = linker_object.swc_file_list;
	V3DLONG neuronNum = nameList.size();
	V3DLONG avg_node_num = 0;
	V3DLONG max_node_num = -1;
	vector<NeuronTree> nt_list;
	
	for (V3DLONG i=0;i<neuronNum;i++)
	{
		NeuronTree tmp = readSWC_file(nameList.at(i));
		nt_list.push_back(tmp);
		avg_node_num += tmp.listNeuron.size();
		if (tmp.listNeuron.size()>max_node_num) max_node_num = tmp.listNeuron.size();
	}
	avg_node_num /= neuronNum;


	V3DLONG n_sampling = QInputDialog::getInt(parent, "sample number", "Please specify the node number of your merged skeleton:", avg_node_num, max_node_num, 1, 0);

	QList<NeuronSWC> result_lN;
	if (!consensus_skeleton(nt_list, result_lN, n_sampling, 0))
	{
		v3d_msg("Error in consensus skeleton!");
		return -1;
	}

	QString fileSaveName;
	QString defaultSaveName = fileOpenName + "_consensus.swc";
	fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save merged neuron to file:"),
			defaultSaveName,
			QObject::tr("Supported file (*.swc)"
				";;Neuron structure	(*.swc)"
				));
	if (!export_listNeuron_2swc(result_lN,qPrintable(fileSaveName)))
	{
		v3d_msg("Unable to save file");
		return -1;
	}

	return 1;
}

bool consensus_skeleton_io(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to consensus_skeleton"<<endl;
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

