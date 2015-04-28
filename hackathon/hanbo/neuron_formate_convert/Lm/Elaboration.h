#ifndef ELABORATION_H
#define ELABORATION_H
#include "Neuron.h"
#include "Segment.h"
#include "Func.h"
#include <string.h>

// MAx # of Elaborationevaluated at the same time
#define MAX_Elaboration 100

#define MAX_NEURON 80000
#define NR_BINS 30
#define MAX_DATA 1000

class Elaboration;

class ListElaboration {

	//tot number of cell to be analized
	int nrTotCell;
	Elaboration* list[MAX_Elaboration];
	Elaboration* tmp;
	int index;
	void initList();
public:
	ListElaboration();
	void add(Elaboration*f);
	//set all initial values
	void newNeuron();
	//used when Global studies are required
	void newRealNeuron();
	//compute a single value for neuron n and segment t
	void compute(Neuron* n);
	//compute standard deviation
	virtual void computeDev(Segment * s) {
	}
	;
	void computeStep(Segment * s);
	//compute the distribution for Neuron n
	void computeDistr(Neuron* n) {
	}
	;
	// compute the distribution over all neuron
	void computeOverallDistr();

	//print methods: one neuron for line

	void printDistr(std::string str);
	void printDistrDB() {
	}
	;
	void printDistrDB(std::string str);
	void printOverallDistr() {
	}
	;

	//Compute initialization value for following funciotn: ie distribution
	void preCompute(Neuron* n);
	void preComputeStep(Segment * s);
};

class Elaboration {
protected:

	int tab;

	int nrNeuron;

public:

	//print RawData
	int RAW_elab;

	Elaboration();
	Elaboration(ListElaboration* l) {
		(*l).add(this);
		tab = -1;
		nrNeuron = 0;
	}
	;

	//Required for initialization
	void virtual initialize() {
	}
	;
	void virtual preComputeStep(Segment * s) {
	}
	;

	void virtual computeStep(Segment * s) {
	}
	;
	//Compute Standard Deviation
	void virtual computeDev(Segment * s) {
	}
	;

	// compute the distribution over all neuron
	void virtual computeOverallDistr() {
	}
	;

	void virtual newNeuronLocal() {
	}
	;

	void newNeuron() {
		tab++;
		nrNeuron = 1;
		newNeuronLocal();
	}
	;
	void newRealNeuron() {
		nrNeuron++;
	}
	;

	void virtual printHeader() {
	}
	;

	void virtual printDistr(std::string str) {
	}
	;
	void virtual printDistrDB() {
	}
	;

};

class Single: public Elaboration {
	//use to evalua te MEan by stdValue/stdCount
	double mValue, mCount;
	//used to compute standard deviation
	double stdValue, stdCount;
protected:
	Func *a;

	//Elaboration for precompute value
	Func* pre;
	//Nr void values returned
	int nrVoid[MAX_NEURON];

	//SG code added
	//Number of virtual compartemtns.
	int nrVirtual[MAX_NEURON];

	// allow dynamic reconfiguration of the distribution
	//store Min/MAx value of parameter stored
	double dMin[MAX_NEURON], dMax[MAX_NEURON];
	//store min max index in the actual array in.

	int min, max, start;

	//if average=0, then the output print the value, otherwise value/count
	int average;

	//used to rescale the distribution: it is dynamically reassanged
	void rescale(double x);

	double MIN;
	double MAX;
	double data[MAX_NEURON];
	//store nr of good value
	int counter[MAX_NEURON];
	//store stdValue
	double std[MAX_NEURON];

	//print Raw data??
	int RAW;
	//valid minmax: if 0 the values need to be set
	int validminmax;

public:
	Single(ListElaboration * l, Func *aa, int avg = 1, int rawdata = 0) :
		Elaboration(l) {

		RAW = rawdata;

		average = avg;

		stdValue = 0;
		stdCount = 0;
		mValue = 0;
		mCount = 0;
		a = aa;

		pre = aa->getPreFunc();

		start = 1;
		for (int j = 0; j < MAX_NEURON; j++) {
			validminmax = 0;
			dMin[j] = 0;
			dMax[j] = 0;
			counter[j] = 0;
			data[j] = 0;
			nrVoid[j] = 0;
			nrVirtual[j] = 0;
			std[tab] = 0;
		}
	}

	void newNeuronLocal() {
		std[tab] = getStd();
		reset();
	}
	void reset() {

		mValue = 0;
		mCount = 0;
		stdValue = 0;
		stdCount = 0;
		validminmax = 0;
		//value exceeding these values will be rejected
		MIN = -1000000;
		MAX = 1000000;

		//local var for storing absolute min/max
		min = 10000000;
		max = -1000000;

	}
	int minMax(double i);

	virtual void storeValue(double i) {
		if (i >= VOID || i <= -VOID) {
			nrVoid[tab]++;

			if(i == -VOID)
				nrVirtual[tab]++;

			return;
		}
		data[tab] += i;
		counter[tab]++;
		//std
		stdValue += (i - getMean()) * (i - getMean());
		stdCount++;
		if (RAW == 1)
			cout << i << "\n";

	}
	void virtual preComputeStep(Segment * t) {
		double d = pre->compute(t);
		minMax(d);
		preMean(d);
	}
	;

	virtual void preMean(double i) {
		if (i >= VOID || i <= -VOID)
			return;
		mValue += i;
		mCount++;
	}
	double getMean() {

		if (mCount != 0) {
			return mValue / mCount;
		} else if (counter[tab] != 0) {
			return data[tab] / counter[tab];
		} else {
			return 0;
		}
	}

	void initialize() {
		a->initialize(getMean());
		reset();
	}

	virtual void computeStep(Segment* t) {

		double d = a->compute(t);
		minMax(d);

		if (t->getType() == -1) {
			storeValue(-VOID);
			return;
		}
		storeValue(d);
	}

	double getStd() {

		if (stdCount <= 1)
			return 0;
		return sqrt(stdValue / (stdCount - 1.0));
	}

	virtual void printHeader() {
		if (strcmp(a->getName(), "N_Class") != 0) {
			cout << a->getName() << "TotSum ";

			cout << a->getName() << "Min ";
			cout << a->getName() << "Avg ";
			cout << a->getName() << "Max ";

		} else {
			cout << "class8";
		}

	}

	virtual void printDistrDB() {
		if (strcmp(a->getName(), "N_Class") == 0) {
			int i = getMax();
			if (i > 10000 || i < -10000)
				i = 0;
			cout << i;

		} else {

			double div = nrNeuron - 1;
			if (div < 1)
				div = 1;
			if (data[tab] > 1000000000)
				data[tab] = 999999;

			cout << data[tab] / div << ","; //tot

			double min, max, std;
			min = getMin();
			max = getMax();
			std = getStd();
			double m1, m2;
			m1 = data[tab] / counter[tab];
			m2 = getMean();
			if (fabs(m1 - m2) > 1)
				cerr << "error computing mean!\n";
			if (fabs(min) < 0.000001)
				min = 0;
			if (fabs(max) < 0.000001)
				max = 0;
			if (fabs(std) < 0.000001)
				std = 0;
			if ((min) > 1000000)
				min = 999999;
			if ((max) > 1000000)
				max = 999999;
			if ((std) > 1000000)
				std = 999999;

			cout << min << ",";
			cout << getMean() << "," << max;

		}

		cerr << "";
	}
	;

	virtual void printDistr(std::string str) {
		cout << str.c_str() << "\t";
		cout << a->getName() << " " << a->getLimit() << "\t";

		int div = nrNeuron - 1;
		if (div < 1)
			div = 1;

		//		if(a->getName().find("Width") != -1 || a->getName().find("Height") != -1 || a->getName().find("Depth") != -1 )
		//		{
		//			cout<<getMax()<<"\t"<<counter[tab];
		//		}else{

		//SG Code fix 07/28/2011
		//This piece of the code was put to make the total sum same a max/min/avg
		if (strcmp(a->getName(), "Width") == 0
				|| strcmp(a->getName(), "Height") == 0 || strcmp(a->getName(),
				"Depth") == 0) {
			cout << getMax() << "\t" << counter[tab];
		} else {
			cout << data[tab] << "\t" << counter[tab];
		}

		cout << "\t(" << (nrVoid[tab]-nrVirtual[tab]) << ")";
		cout << "\t" << getMin();
		cout << "\t" << getMean() << "\t" << getMax() << "\t" << getStd()
				<< " \n";

	}
	;
	void setAverage(int i) {
		average = i;
	}
	double getMin() {
		if (counter[tab] == 0)
			return 0;
		return dMin[tab];
	}
	double getMax() {
		if (counter[tab] == 0)
			return 0;
		return dMax[tab];
	}
	void setMin(double j) {
		dMin[tab] = j;
	}
	void setMax(double j) {
		dMax[tab] = j;
	}
	virtual double getValue(Segment* t) {
		return 0.0;
	}
};

class Distr: public Elaboration {
protected:
	//store the two Elaboration : distribution of A VS B
	Func *a, *b;

	// allow dynamic reconfiguration of the distribution
	//store Min/MAx value of parameter stored
	double dMin[MAX_NEURON], dMax[MAX_NEURON];
	//store min max index in the actual array in.

	int min, max, start;

	//if average=0, then the output print the value, otherwise value/count
	int average;

	//if direct=1 the plot is directly in the number of bin (used for integer distribution
	int direct;
	//used to rescale the distribution: it is dynamically reassanged
	void rescale(double x);

	//1:then binPar contains the number of bins
	//0: binPar contains the witdh of the bins( max # of Bin set by MAX_DATA
	int binMode;
	double binPar;
	//the resulting width of bins 
	double widthBins;

	//store the max number of bin;
	int index[MAX_DATA];

	//print RawData
	int RAW;

	double MIN;
	double MAX;
	double data[MAX_NEURON][MAX_DATA];
	double dataSquare[MAX_NEURON][MAX_DATA];
	//store counter for computing averages
	int counter[MAX_NEURON][MAX_DATA];
public:

	Distr(ListElaboration * l, Func *aa, Func *bb = 0, int i = 0,
			int directV = 0, int binMode1 = 0, double binPar1 = NR_BINS,
			int rawdata = 0) :
		Elaboration(l) {
		
		//Setting the raw data flag
		RAW_elab = rawdata;
		RAW = rawdata;
		average = i;
		direct = directV;
		a = aa;
		b = bb;
		binMode = binMode1;
		binPar = binPar1;
		widthBins = 0;
		if (b == 0)
			b = a;
		start = 1;
		//reset nrBins : to be computed after prePhase
		widthBins = -1;
		for (int j = 0; j < MAX_NEURON; j++) {
			dMin[j] = -1;
			dMax[j] = -1;
			index[j] = 0;
			for (int i = 0; i < MAX_DATA; i++) {
				counter[j][i] = 0;
				data[j][i] = 0;
				dataSquare[j][i] = 0;
			}
		}

	}

	void newNeuronLocal() {
		widthBins = 0;

	}

	int minMax(double i);

	virtual void storeValue(double i) {
		storeValue(i, i);
	}
	virtual void storeValue(double i, double j);
	void virtual preComputeStep(Segment * t) {
		minMax(b->compute(t));
	}
	;
	//compute the number of bins to be used( up to MAX_DATA
	double computeWidthBins() {
		double width = 0;
		if (binMode == 0) {
			//to set starting value to zero

			double m = getMin();
			double M = getMax();
			//add 0.01% more on MAx value to include the Max in the last bin!
			width = ((M * 0.0001 + M - m) / (binPar));

		} else {
			width = binPar;
		}
		return width;
	}

	void computeStep(Segment* t) {
		if (widthBins == 0) {
			widthBins = computeWidthBins();

			index[tab] = (int) ((getMax() - getMin()) / widthBins);
		}
		double a1 = a->compute(t);
		double b2 = b->compute(t);
		if (strcmp(a->getName(), b->getName()) == 0 && b2 < VOID) {
			a1 = 1;
		}
		storeValue(a1, b2);
	}

	//the same as getMEna, but with a different way of computation
	double getAverage();

	virtual void printDistr(std::string str);
	virtual void printDistrDB();

	void setAverage(int i) {
		average = i;
	}
	double getMin() {

		return dMin[tab];
	}
	double getMax() {
		return dMax[tab];
	}
	void setMin(double j) {
		dMin[tab] = j;
	}
	void setMax(double j) {
		dMax[tab] = j;
	}
	virtual double getValue(Segment* t) {
		return 0.0;
	}
};

#endif
