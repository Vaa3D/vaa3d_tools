/* consensus_skeleton_func.cpp
 * a plugin to merge multiple neurons by generating a consensus skeleton
 * 2012-05-02 : by Yinan Wan
 */

#include <v3d_interface.h>
#include "basic_surf_objs.h"
#include "v3d_message.h"
#include "consensus_skeleton_func.h"
#include "consensus_skeleton.h"
#include "median_swc.h"
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

	bool ok;
	V3DLONG n_sampling = QInputDialog::getInt(parent, "sample number", "Please specify the node number of your merged skeleton:", avg_node_num, 1, max_node_num, 1, &ok);
	
	if (!ok)
		return 0;
	

	QList<NeuronSWC> result_lN;
    if (!consensus_skeleton(nt_list, result_lN, 0,callback))
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

bool consensus_skeleton_io(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
	if(input.size()==0 || output.size() != 1) return true;
	char * paras = NULL;

	//parsing input
	vector<char *> * inlist =  (vector<char*> *)(input.at(0).p);
	if (inlist->size()==0)
	{
		cerr<<"You must specify input linker or swc files"<<endl;
		return false;
	}

	//parsing output
	vector<char *> * outlist = (vector<char*> *)(output.at(0).p);
	if (outlist->size()>1)
	{
		cerr<<"You cannot specify more than 1 output files"<<endl;
		return false;
	}

	//parsing parameters
	V3DLONG n_sampling = 0;
	int method_code = 0;
	if (input.size()==2)
	{
		vector<char*> * paras = (vector<char*> *)(input.at(1).p);
		if (paras->size()==1)
		{
			n_sampling = atoi(paras->at(0));
			cout<<"n_sampling = "<<n_sampling<<endl;
		}
		else
		{
			cerr<<"Too many parameters"<<endl;
			return false;
		}
	}

	vector<NeuronTree> nt_list;
	QStringList nameList;
	QString qs_linker;
	char * dfile_result = NULL;
	V3DLONG neuronNum = 0;

	for (int i=0;i<inlist->size();i++)
	{
		qs_linker = QString(inlist->at(i));
		if (qs_linker.toUpper().endsWith(".ANO"))
		{
			cout<<"(0). reading a linker file."<<endl;
			P_ObjectFileType linker_object;
            if (!loadAnoFile(qs_linker,linker_object))
			{
				fprintf(stderr,"Error in reading the linker file.\n");
				return 1;
			}
			nameList = linker_object.swc_file_list;
			neuronNum += nameList.size();
			for (V3DLONG i=0;i<neuronNum;i++)
			{
				NeuronTree tmp = readSWC_file(nameList.at(i));
				nt_list.push_back(tmp);
			}
		}
		else if (qs_linker.toUpper().endsWith(".SWC"))
		{
			cout<<"(0). reading an swc file"<<endl;
			NeuronTree tmp = readSWC_file(qs_linker);
			nt_list.push_back(tmp);
			neuronNum++;
			if (outlist->size()==0)
			{
				cerr<<"You must specify outfile name if you input a list of swcs"<<endl;
				return false;
			}
		}
	}

	QString outfileName;
	if (outlist->size()==0)
		outfileName = qs_linker + "_consensus.swc";
	else
		outfileName = QString(outlist->at(0));

	QList<NeuronSWC> merge_result;
    if (!consensus_skeleton(nt_list, merge_result, method_code, callback))
	{
		cerr<<"error in consensus_skeleton"<<endl;
		return false;
	}

	export_listNeuron_2swc(merge_result,qPrintable(outfileName));
	printf("\t %s has been generated successfully, size:%d.\n",qPrintable(outfileName), merge_result.size());

	return true;
}

bool median_swc_func(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    if(input.size()==0) return false;

    //parsing input
    vector<char *> * inlist =  (vector<char*> *)(input.at(0).p);
    if (inlist->size()==0)
    {
        cerr<<"You must specify input linker or swc files"<<endl;
        return false;
    }

    V3DLONG neuronNum = 0;
    vector<NeuronTree> nt_list;
    QString qs_linker;
    QStringList nameList;
    for (int i=0;i<inlist->size();i++)
    {
        qs_linker = QString(inlist->at(i));
        if (qs_linker.toUpper().endsWith(".ANO"))
        {
            cout<<"(0). reading a linker file."<<endl;
            P_ObjectFileType linker_object;
            if (!loadAnoFile(qs_linker,linker_object))
            {
                fprintf(stderr,"Error in reading the linker file.\n");
                return 1;
            }
            nameList = linker_object.swc_file_list;
            neuronNum += nameList.size();
            for (V3DLONG i=0;i<neuronNum;i++)
            {
                NeuronTree tmp = readSWC_file(nameList.at(i));
                nt_list.push_back(tmp);
            }
        }
        else if (qs_linker.toUpper().endsWith(".SWC"))
        {
            cout<<"(0). reading an swc file"<<endl;
            NeuronTree tmp = readSWC_file(qs_linker);
            nt_list.push_back(tmp);
            neuronNum++;
        }
    }


    cout << "There are "<<nt_list.size() <<" input neurons."<<endl;
    int idx = median_swc(nt_list);
    if (idx <0){
        cerr << "error in median_swc()" << endl;
        return false;
    }

    cout<<"Median swc is " << idx << endl;
    return true;
}


bool average_node_position_func(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    //parsing input
    vector<char *> * inlist =  (vector<char*> *)(input.at(0).p);
    cout<<"\n\n  inlist.size = "<<inlist->size()<<endl;
    if ( inlist->size() <2 )
    {
        cerr<<"You must specify inputs: median swc file and the linker file"<<endl;
        return false;
    }

    //parsing output
    vector<char *> * outlist = (vector<char*> *)(output.at(0).p);
    if (outlist->size()>1)
    {
        cerr<<"You cannot specify more than 1 output files"<<endl;
        return false;
    }

    //parsing parameters
    V3DLONG distance_threshold= 0;
    vector<char*> * paras = (vector<char*> *)(input.at(1).p);
    if (paras->size()==1)
    {
        distance_threshold = atoi(paras->at(0));
        cout<<"distance_threshold = "<<distance_threshold<<endl;
    }
    else
    {
        cerr<<"Too many parameters"<<endl;
        return false;
    }

    vector<NeuronTree> nt_list;
    QStringList nameList;
    QString qs_linker;
    char * dfile_result = NULL;
    V3DLONG neuronNum = 0;
    NeuronTree median_neuron;

    qs_linker = QString(inlist->at(0));
    if (qs_linker.toUpper().endsWith(".SWC"))
    {
        cout<<"(0). reading an swc file"<<endl;
        median_neuron= readSWC_file(qs_linker);
    }
    else{
        cout <<" The first input should be a swc file ( median neuron)." <<endl;
        return false;
    }


    qs_linker = QString(inlist->at(1));
    if (qs_linker.toUpper().endsWith(".ANO"))
    {
        cout<<"(0). reading a linker file."<<endl;
        P_ObjectFileType linker_object;
        if (!loadAnoFile(qs_linker,linker_object))
        {
            fprintf(stderr,"Error in reading the linker file.\n");
            return 1;
        }
        nameList = linker_object.swc_file_list;
        neuronNum += nameList.size();
        for (V3DLONG i=0;i<neuronNum;i++)
        {
            NeuronTree tmp = readSWC_file(nameList.at(i));
            nt_list.push_back(tmp);
        }
    }
    else
    {
        cout <<" The second input should be a ANO file ( the group of neurons)." <<endl;
    }

    QString outfileName;
    if (outlist->size()==0)
        outfileName = qs_linker + "_median_adjusted.swc";
    else
        outfileName = QString(outlist->at(0));

    NeuronTree median_adjusted = average_node_position(median_neuron, nt_list, distance_threshold );
    if (median_adjusted.listNeuron.size() == 0 ){

        cerr<<"error in average_node_position()"<<endl;
        return false;
    }

    writeSWC_file(outfileName, median_adjusted);
    printf("\t %s has been generated successfully\n",qPrintable(outfileName));

    return true;
}

int average_node_position_menu(V3DPluginCallback2 &callback, QWidget *parent)
{
    QString fileOpenName1;
    fileOpenName1 = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
            "",
            QObject::tr("Supported file (*.swc)"
                ));
    if(fileOpenName1.isEmpty())
        return -1;
    NeuronTree median_neuron = readSWC_file(fileOpenName1);

     QString fileOpenName2 = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
            "",
            QObject::tr("Supported file (*.ano)"
                ";;Neuron structure	(*.ano)"
                ));
    if(fileOpenName2.isEmpty())
        return -1;

    P_ObjectFileType linker_object;
    if (!loadAnoFile(fileOpenName2,linker_object))
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

    bool ok;
    double distance_threshold= QInputDialog::getDouble(parent, "distance threshold",
              "Please specify the maximum distance allowed to search for maching nodes in all input neurons: ",8, 1, 100, 1, &ok );

    if (!ok)
        return 0;

    NeuronTree median_adjusted =  average_node_position(median_neuron, nt_list, distance_threshold);

    QString fileSaveName;
    QString defaultSaveName = fileOpenName1 + "_adjusted.swc";
    fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save merged neuron to file:"),
            defaultSaveName,
            QObject::tr("Supported file (*.swc)"
                ";;Neuron structure	(*.swc)"
                ));
    if (!writeSWC_file(qPrintable(fileSaveName),median_adjusted))
    {
        v3d_msg("Unable to save file");
        return -1;
    }

    return 1;
}


int median_swc_menu(V3DPluginCallback2 &callback, QWidget *parent)
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
    vector<NeuronTree> nt_list;

    for (V3DLONG i=0;i<neuronNum;i++)
    {
        NeuronTree tmp = readSWC_file(nameList.at(i));
        nt_list.push_back(tmp);
    }

    int id = median_swc( nt_list);
    if (id >=0 )
    {
        v3d_msg( QString("The median swc is : %1").arg( nameList.at(id)) );
    }

    /*
    QString fileSaveName;
    QString defaultSaveName = fileOpenName + "_consensus.swc";
    fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save merged neuron to file:"),
            defaultSaveName,
            QObject::tr("Supported file (*.swc)"
                ";;Neuron structure	(*.swc)"
                ));
    if ( !writeSWC_file(fiveSaveName, median_neuron))
    {
        v3d_msg("Unable to save file");
        return -1;
    }

    */
    return 1;
}

void printHelp()
{
    cout<<"\nConsensus Skeleton: This plugin has the following three functions:"<<endl;
    cout<<"\n  1) Pick the median neuron tree from a group of input neuron tress."<<endl;
    cout<<"\nUsage: v3d -x consensus_skeleton_2 -f median_swc -i <input ANO linker file> [-o <output swc file>] "<<endl;
    cout<<"Parameters:"<<endl;
    cout<<"\t-f <function_name>:  median_swc"<<endl;
    cout<<"\t-i <input_file(s)>:  input linker file (.ano) "<<endl;
    cout<<"\t-o <output_file>:  output median swc file name, this parameter is optional. "
          "When not specified, no median swc file will be generated (duplicate of the corresponding median case)."
          " The index number of the median swc in the ano file will be reported in standard output. "<<endl;
    cout<<"Example: v3d -x consensus_skeleton_2 -f median_swc -i mylinker.ano  [-o median.swc] \n"<<endl;

    cout<<"\n  2) Adjust input neuron node locations by averaging over the matching nodes from the input group of neurons tree."<<endl;
    cout<<"\nUsage: v3d -x consensus_skeleton_2 -f average_node_position -i <median swc> <linker ANO file> -o <output_file> -p <distance_threshold>"<<endl;
    cout<<"Parameters:"<<endl;
    cout<<"\t-f <function_name>:  average_node_position"<<endl;
    cout<<"\t-i    <median swc>:  input median swc file (generated from median_swc function)"<<endl;
    cout<<"\t <linker ANO file>:  input linker file (.ano)"<<endl;
    cout<<"\t -p <distance_threshold>:  nodes that have distances larger than this threshold will "
          "not be considered matching for averaging." <<endl;
    cout<<"\t        -o <output_file>:  output file name." <<endl;
    cout<<"Example: v3d -x consensus_skeleton_2 -f average_node_position -i median.swc mylinker.ano -p 8 -o median_adjusted.swc\n"<<endl;

    cout<<"\n  3) Generate a consensus neuron skeleton (swc file) from a group of neurons ( radii are ignored)."<<endl;
    cout<<"\nUsage: v3d -x consensus_skeleton_2 -f consensus_skeleton -i <input> -o <output_file> "<<endl;
    cout<<"Parameters:"<<endl;
	cout<<"\t-f <function_name>:  consensus_skeleton"<<endl;
    cout<<"\t-i <input>:  input linker file (.ano) or folder path"<<endl;
    cout<<"\t-o   <output_file>:  output file name. If -i is followd by a linker file name, this parameter can be omitted"<<endl;
	cout<<"\t                     default result will be generated under the same directory of the ref linkerfile and has a name of 'linkerFileName_consensus.swc'"<<endl;
    cout<<"Example: v3d -x consensus_skeleton -f consensus_skeleton -i mylinker.ano -p 200 -o consensus_skeleton.swc\n"<<endl;
}

