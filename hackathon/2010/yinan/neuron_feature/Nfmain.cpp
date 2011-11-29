#include "Nfmain.h"
#include "Neuron.h"
#include "Feature.h"
#include <vector>
#include <iostream>
using namespace std;

class Feature;

//stem number
void computeStems(Neuron * n)
{
	n->setFunc(1);
	Feature* features = new Feature(n);
	//print values
	cout<<"Number of Stems: "<<(int)features->getSum()<<"\n";	
	//myfile<<"Number of Stems: "<<(int)features->getSum()<<"\n";
	features->~Feature();
}

//bifurcation number
void computeBifs(Neuron *n)
{
	n->setFunc(2);
	Feature* features = new Feature(n);
	cout<<"Number of Bifurcations: "<<(int)features->getSum()<<"\n";
	//myfile<<"Number of Bifurcations: "<<(int)featues->getSum()<<"\n";
	features->~Feature();
}

//branch number
void computeBranches(Neuron *n)
{
	n->setFunc(3);
	Feature* features = new Feature(n);
	cout<<"Number of Branches: "<<(int)features->getSum()<<"\n";
	//myfile<<"Number of Branches: "<<(int)featues->getSum()<<"\n";
	features->~Feature();
}

//tip number
void computeTips(Neuron *n)
{
	n->setFunc(4);
	Feature* features = new Feature(n);
	cout<<"Number of Tips: "<<(int)features->getSum()<<"\n";
	//myfile<<"Number of Tips: "<<(int)featues->getSum()<<"\n";
	features->~Feature();
}


//Width
void computeWidth(Neuron *n)
{
	n->setFunc(5);
	Feature* features = new Feature(n);
	cout<<"Width: "<<features->getMin()<<"\n";
	//myfile<<"Width: "<<featues->getMin()<<"\n";
		features->~Feature();
}

//Height
void computeHeight(Neuron *n)
{
	n->setFunc(6);
	Feature* features = new Feature(n);
	cout<<"Height: "<<features->getMin()<<"\n";
	//myfile<<"Height: "<<featues->getMin()<<"\n";
	features->~Feature();

}

//Depth
void computeDepth(Neuron *n)
{
	n->setFunc(7);
	Feature* features = new Feature(n);
	cout<<"Depth: "<<features->getMin()<<"\n";
	//myfile<<"Depth: "<<featues->getMin()<<"\n";
	features->~Feature();
}

//average type
void avgType(Neuron *n)
{
	n->setFunc(8);
	Feature* features = new Feature(n);
	cout<<"Type: "<<"\t"<<"Average: "<<features->getAvg()<<"\t"<<"Std :"<<features->getStd()<<"\n";
	//myfile<<"Type: "<<"\n"<<"Average: "<<featues->getAvg()<<"\t"<<"Std :"<<featues->getStd()<<"\n";
	features->~Feature();
}

//average diameter
void avgDia(Neuron *n)
{
	n->setFunc(9);
	Feature* features = new Feature(n);
	cout<<"Diameter: "<<"\t"<<"Average: "<<features->getAvg()<<"\t"<<"Std :"<<features->getStd()<<"\n";
	//myfile<<"Diameter: "<<"\n"<<"Average: "<<featues->getAvg()<<"\t"<<"Std :"<<featues->getStd()<<"\n";
	features->~Feature();
}

//total length
void computeLength(Neuron *n)
{
	n->setFunc(11);
	Feature* features = new Feature(n);
	cout<<"Total Length: "<<features->getSum()<<"\n";
	//myfile<<"Total length: "<<featues->getSum()<<"\n";
	features->~Feature();
}

//surface area
void computeSurface(Neuron *n)
{
	n->setFunc(12);
	Feature* features = new Feature(n);
	cout<<"Total Surface Area: "<<features->getSum()<<"\n";
	//myfile<<"Total Surface Area: "<<featues->getSum()<<"\n";
	features->~Feature();
}

//total volume
void computeVol(Neuron *n)
{
	n->setFunc(14);
	Feature* features = new Feature(n);
	cout<<"Total Volume: "<<features->getSum()<<"\n";
	//myfile<<"Total Volume: "<<featues->getSum()<<"\n";
	features->~Feature();
}

//Hausdorff demension
void computeHausdorff(Neuron *n)
{	
	n->setFunc(42);
	Feature* features = new Feature(n);
	cout<<"Hausdorff: "<<features->getAvg()<<"\n";
	//myfile<<"Hausdorff: "<<"\n"<<"Average: "<<featues->getAvg()<<"\t"<<"Std :"<<featues->getStd()<<"\n";
	features->~Feature();
}

//fractal dimension
void computeFractalDim(Neuron *n)
{
	n->setFunc(44);
	Feature* features = new Feature(n);
	cout<<"Fractal Dim: "<<"\t"<<"Average: "<<features->getAvg()<<"\t"<<"Std :"<<features->getStd()<<"\n";
	//myfile<<"Fractal Dim: "<<"\n"<<"Average: "<<featues->getAvg()<<"\t"<<"Std :"<<featues->getStd()<<"\n";
	features->~Feature();
}

void nf_main(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	//cout<<"Welcome to nf_main"<<"\n";
	vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
	//char * out = (*(vector<char*>*)(input.at(0).p)).at(0);
	//QString outfileName = QString(out);
	//QFile file(outfileName);
	//file.open(QIODevice::WriteOnly|QIODevice::Text);
	//QTextStream myfile(&file);

	int neuronNum = (int)inlist->size();
	for (int i=0;i<neuronNum;i++)
	{
		Neuron * n = new Neuron((char*)inlist->at(i));
		cout<<"\n--------------Neuron #"<<(i+1)<<"----------------\n";
		//myfile<<"Neuron #"<<(i+1)<<"\n";
		
		//Node #
		cout<<"Number of Nodes: "<<n->read<<"\n";
		//myfile<<"Neuron #"<<n->read<<"\n";

		computeStems(n);
		computeBifs(n);
		computeBranches(n);
		computeTips(n);
		computeWidth(n);
		computeHeight(n);
		computeDepth(n);
		avgType(n);
		avgDia(n);
		computeLength(n);
		computeSurface(n);
		computeVol(n);
		computeHausdorff(n);
		//computeFractalDim(n);

	}
//	file.close();
}
