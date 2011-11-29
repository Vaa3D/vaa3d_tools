#ifndef FEATURE_H
#define FEATURE_H

#include "Neuron.h"
#include <QList>
#include "Func.h"
#include "Neuron.h"

class Feature {
	QList<double> fValues;
	int valid,ignored;
	double Min,Max,Avg,Std,Sum;
	Func* func;
	
public:
	//Feature();
	//~Feature(){cout<<"feature deleted!\n";};
	Feature(Neuron* n);
	QList<double> getValues() {return fValues;};
	int getValid() {return valid;};
	int getIgnored() {return ignored;};
	double getMin() {return Min;};
	double getMax() {return Max;};
	double getAvg() {return Avg;};
	double getStd() {return Std;};
	double getSum() {return Sum;};
	void computeStep(Segment* t);
};

#endif
