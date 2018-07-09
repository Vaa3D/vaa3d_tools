/*
 *  sholl_func.cpp
 *  io functions for domenu and do func 
 *
 *  2018-06-26: by OuYang Qiang
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "sholl_func.h"
#include "openSWCDialog.h"
#include "customary_structs/vaa3d_neurontoolbox_para.h"
#include <vector>
#include <iostream>
#include "sholl_swc.h"
#include "Rinstancing.h"
using namespace std;

const QString title = QObject::tr("Sholl Analysis SWC Plugin");

void sholl_menu(V3DPluginCallback2 &callback, QWidget *parent)
{
	OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
	if (!openDlg->exec())
		return;

    NeuronTree nt = openDlg->nt;
    QList<NeuronSWC> neuron = nt.listNeuron;

    vector<double> radius;
    vector<double> crossings;

    double step;

    bool ok;
    step = QInputDialog::getDouble(0,"Would you like to specify a radius step size?","radius step size (0 for continuous sampling)",0,0,2147483647,1,&ok);
	if (!ok)
        step = VOID;

    crossings = ShollSWC(neuron, step);

    for(int i=0;i<crossings.size();i++)
    {
        radius.push_back(i*step);
    }

    int test1;
    char** test2;
    instantiateR(test1, test2, radius, crossings);

    return;
}

void sholl_toolbox(const V3DPluginArgList & input)
{
	vaa3d_neurontoolbox_paras * paras = (vaa3d_neurontoolbox_paras *)(input.at(0).p);
	NeuronTree nt(paras->nt);
	QList<NeuronSWC> neuron = nt.listNeuron;

    vector<double> crossings;

    double step;

    bool ok;
    step = QInputDialog::getDouble(0,"Would you like to specify a radius step size?","radius step size (0 for continuous sampling)",5000,5000,2147483647,1,&ok);
    if (!ok)
        step = VOID;

    crossings = ShollSWC(neuron, step);

	return;
}

bool sholl_func(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    cout<<"==========Welcome to sholl_swc function============="<<endl;
	vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
	vector<char*>* paralist = NULL;



    double step = VOID;

    QList<NeuronSWC> neuron;
    bool hasPara;
	if (input.size()==1) 
	{
		cout<<"No new parameter specified.\n";
		hasPara = false;
	}
	else {
		hasPara = true;
		paralist = (vector<char*>*)(input.at(1).p);
	}

	if (inlist->size()!=1)
	{
		cout<<"You must specify 1 input file!"<<endl;
		return false;
	}

	if (hasPara)
	{
		if (paralist->size()==0)
		{
            cout<< "radius step size not set, 50 um will be used" << endl;
            step = 50;
		}
		else if (paralist->size() >= 1)
		{
            step = atoi(paralist->at(0));
            cout<<"radius step size (0 for continuous sampling):"<<step<<endl;
		}
	}
 }
#ifndef VOID
	QString fileOpenName = QString(inlist->at(0));

	if (fileOpenName.endsWith(".swc") || fileOpenName.endsWith(".SWC"))
    {
		neuron = readSWC_file(fileOpenName).listNeuron;
    }
    else if (fileOpenName.endsWith(".ano") || fileOpenName.endsWith(".ANO"))
	{
		P_ObjectFileType linker_object;
		if (!loadAnoFile(fileOpenName,linker_object))
		{
			cout<<"Error in reading the linker file."<<endl;
			return false;
		}
		QStringList nameList = linker_object.swc_file_list;
        QStringList aponameList = linker_object.pointcloud_file_list;
		V3DLONG neuronNum = nameList.size();
		vector<QList<NeuronSWC> > nt_list;
		for (V3DLONG i=0;i<neuronNum;i++)
		{
			QList<NeuronSWC> tmp = readSWC_file(nameList.at(i)).listNeuron;
			nt_list.push_back(tmp);
		}
		if (!combine_linker(nt_list, neuron))
		{
			cout<<"Error in combining neurons."<<endl;
			return false;
		}
	}
	else
	{
		cout<<"The file type you specified is not supported."<<endl;
		return false;
	}

    crossings = ShollSWC(neuron, step);
    if (crossings.size()!=0)
	{
        cout<<"Error in doing the sholl analysis of the swc"<<endl;
		return false;
	}
	return true;
}

void printHelp(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3d_msg("(version 2.0) Reallocate id for the points in 1 neuron (.swc) or mutiple neurons (.ano). Set a new root and sort the SWC file into a new order, where the newly set root has the id of 1, and the parent's id is less than its child's. If the link threshold is set, the plugin will only generate nearest link shorter than that threshold. by Yinan Wan 2012-02-01");
}

void printHelp(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"(version 2.0) Reallocate id for the points in 1 neuron (.swc) or mutiple neurons (.ano). Set a new root and sort the SWC file into a new order, where the newly set root has the id of 1, and the parent's id is less than its child's. If the link threshold is set, the plugin will only generate nearest link shorter than that threshold. by Yinan Wan 2012-02-01"<<endl;
	cout<<"usage:\n";
    cout<<"-f<func name>:\t\t sholl_swc\n";
	cout<<"-i<file name>:\t\t input .swc or .ano file\n";
    cout<<"-p<step>:\t (not required) the radius step size you want the plugin to use when doing the sholl analysis. If you set step=0, continuous sampling will be used. DEFAULT: 50.\n";
    cout<<"Demo1:\t ./v3d -x sholl_swc -f sholl_swc -i test.swc -p 0  (continuous sampling) \n";
    cout<<"Demo2:\t ./v3d -x sholl_swc -f sholl_swc -i test.swc (default 50um radius step size will be used) \n";

}

#endif
