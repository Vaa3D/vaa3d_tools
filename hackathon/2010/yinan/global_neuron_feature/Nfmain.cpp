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
	//char * out = (*(vector<char*>*)(output.at(0).p)).at(0);
	//QString outfileName = QString(out);
	//QFile file(outfileName);
	//file.open(QIODevice::WriteOnly|QIODevice::Text);
	//QTextStream myfile(&file);

	//myfile <<"id\tFile Name\tN_Node\tN_stem\tN_bifs\tN_branch\tN_tips\tWidth\tHeight\tDepth\tLength\tVolume\tSurface\tContraction\tFragmentation\tPd_ratio\tHausdorff\tFractal_Dim"<<endl;

	int neuronNum = (int)inlist->size();

	for (int i=0;i<neuronNum;i++)
	{
		QString name = QString(inlist->at(i));
		NeuronTree nt = readSWC_file(name);
		
		cout<<"\n--------------Neuron #"<<(i+1)<<"----------------\n";
		//myfile<<(i+1)<<"\t"<<name<<"\t";
		double * features = new double[FNUM];
		computeFeature(nt, features);
		printFeature(features);
		//for (int jj=0;jj<FNUM;jj++)
		//	myfile<<features[jj]<<"\t";
		//myfile<<endl;
	}
	//file.close();
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
