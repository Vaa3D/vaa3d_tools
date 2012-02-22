#define FNUM 22

#include "Nfmain.h"
#include "compute.h"

#include <vector>
#include <math.h>
#include <iostream>
using namespace std;

void nf_main(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
	if (inlist->size()!=1)
	{
		cerr<<"Error in input list. You must input .ano file or a list of .swc file name"<<endl;
		return;
	}

	QStringList nameList;
	QString qs_name(inlist->at(0));
	qs_name = qs_name.simplified();
	int neuronNum;
	vector<NeuronTree> nt_list;

	if (qs_name.toUpper().endsWith(".ANO"))
	{
		cout<<"reading a linker file..."<<endl;;
		P_ObjectFileType linker_object;
		if (!loadAnoFile(QString(qs_name),linker_object))
		{
			cerr<<"Error in reading the linker file."<<endl;
			return;
		}
		nameList = linker_object.swc_file_list;
		neuronNum = nameList.size();
		for (V3DLONG i=0;i<neuronNum;i++)
		{
			NeuronTree tmp = readSWC_file(nameList.at(i));
			nt_list.push_back(tmp);
		}
	}
	else if (qs_name.toUpper().endsWith(".SWC") || qs_name.toUpper().endsWith(".ESWC"))
	{
		cout<<"reading a list of swc file names."<<endl;
		nameList = qs_name.split(" ");
		neuronNum = nameList.size();
		for (V3DLONG i=0;i<neuronNum;i++)
		{
			NeuronTree tmp = readSWC_file(nameList.at(i));
			nt_list.push_back(tmp);
		}
	}


	for (int i=0;i<neuronNum;i++)
	{
		NeuronTree nt = nt_list[i];
		cout<<"\n--------------Neuron #"<<(i+1)<<"----------------\n";
		double * features = new double[FNUM];
		computeFeature(nt, features);
		printFeature(features);
		if (features) {delete []features; features = NULL;}
	}
}

void nf_main(V3DPluginCallback2 &callback, QWidget *parent)
{
	QString fileOpenName;
	fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
			"",
			QObject::tr("Supported file (*.swc)"
				";;Neuron structure	(*.swc)"
				));
	if(fileOpenName.isEmpty()) 
	{
		v3d_msg("You don't have any file open.");
		return;
	}
	NeuronTree nt = readSWC_file(fileOpenName);
	double * features = new double[FNUM];
	computeFeature(nt,features);
	QMessageBox infoBox;
	infoBox.setText("Global features of the neuron:");
	infoBox.setInformativeText(QString("<pre><font size='4'>"
				"number of nodes                  : %1<br>"
				"soma surface                     : %2<br>"
				"number of stems                  : %3<br>"
				"number of bifurcations           : %4<br>"
				"number of branches               : %5<br>"
				"number of tips                   : %6<br>"
				"overall width                    : %7<br>"
				"overall height                   : %8<br>"
				"overall depth                    : %9<br>"
				"average diameter                 : %10<br>"
				"total length                     : %11<br>"
				"total surface                    : %12<br>"
				"total volume                     : %13<br>"
				"max euclidean distance           : %14<br>"
				"max path distance                : %15<br>"
				"max branch order                 : %16<br>"
				"average contraction              : %17<br>"
				"average fragmentation            : %18<br>"
				"average parent-daughter ratio    : %19<br>"
				"average bifurcation angle local  : %20<br>"
				"average bifurcation angle remote : %21<br>"
				"Hausdorff dimension              : %22</font></pre>")
				.arg(features[0])
				.arg(features[1])
				.arg(features[2])
				.arg(features[3])
				.arg(features[4])
				.arg(features[5])
				.arg(features[6])
				.arg(features[7])
				.arg(features[8])
				.arg(features[9])
				.arg(features[10])
				.arg(features[11])
				.arg(features[12])
				.arg(features[13])
				.arg(features[14])
				.arg(features[15])
				.arg(features[16])
				.arg(features[17])
				.arg(features[18])
				.arg(features[19])
				.arg(features[20])
				.arg(features[21]));
	infoBox.exec();


	if (features) {delete []features; features = NULL;}

}

void printFeature(double * features)
{

	for (int i=0;i<FNUM;i++)
	{
		switch (i)
		{
			case 0:
				cout<<"Number of Nodes:";
				break;
			case 1:
				cout<<"Soma Surface:\t";
				break;
			case 2:
				cout<<"Number of Stems:";
				break;
			case 3:
				cout<<"Number of Bifurcatons:";
				break;
			case 4:
				cout<<"Number of Branches:";
				break;
			case 5:
				cout<<"Number of Tips:\t";
				break;
			case 6:
				cout<<"Overall Width:\t";
				break;
			case 7:
				cout<<"Overall Height:\t";
				break;
			case 8:
				cout<<"Overall Depth:\t";
				break;
			case 9:
				cout<<"Average Diameter:";
				break;
			case 10:
				cout<<"Total Length:\t";
				break;
			case 11:
				cout<<"Total Surface:\t";
				break;
			case 12:
				cout<<"Total Volume:\t";
				break;
			case 13:
				cout<<"Max Euclidean Distance:";
				break;
			case 14:
				cout<<"Max Path Distance:\t\t";
				break;
			case 15:
				cout<<"Max Branch Order:\t\t";
				break;
			case 16:
				cout<<"Average Contraction:\t\t";
				break;
			case 17:
				cout<<"Average Fragmentation:\t\t";
				break;
			case 18:
				cout<<"Average Parent-daughter Ratio:\t";
				break;
			case 19:
				cout<<"Average Bifurcation Angle Local:";
				break;
			case 20:
				cout<<"Average Bifurcation Angle Remote:";
				break;
			case 21:
				cout<<"Hausdorff Dimension:\t\t";
		}
		cout<<"\t"<<features[i]<<endl;
	}
}
