#define VOID 1000000000
#define min(a,b) (a)<(b)?(a):(b)
#define max(a,b) (a)>(b)?(a):(b)

#include "Func.h"
#include <math.h>
#include "Feature.h"
#include <iostream>
using namespace std;

Feature::Feature(Neuron *n)
{
	fValues = QList<double>();
	valid=ignored=0;
	Min = VOID;
	Max = Sum = Std = 0;
	func = n->getFunc();
	//cout<<"function successfully set: "<<func->getName()<<"\n";
	computeStep(n->getSoma());
	Avg = Sum/valid;
	double tmp=0;
	for (int i=0;i<valid;i++){
		tmp +=(fValues.at(i)-Avg)*(fValues.at(i)-Avg);
	}
	Std = sqrt(tmp)/valid;	
}

void Feature::computeStep(Segment* t){
	//cout<<"compute step: ";
	double d;
	d = func->computeStep(t);
	//cout<<d<<"\n";
	if (d==VOID) ignored++;
	else{
		fValues.append(d);
		valid++;
		Min = min(Min,d);
		Max = max(Max,d);
		Sum += d;
	}
	if (t->getNext1()!=NULL){
		computeStep(t->getNext1());}
	if (t->getNext2()!=NULL){
		computeStep(t->getNext2());}
}
