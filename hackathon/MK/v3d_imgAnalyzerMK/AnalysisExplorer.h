#ifndef ANALYSISEXPLORER_H
#define ANALYSISEXPLORER_H

#include <vector>
#include <unordered_map>
#include <cmath>

#include "basic_4dimage.h"
#include "v3d_interface.h"

using namespace std;

class AnalysisExplorer
{
public:

	AnalysisExplorer() {};
	
	/********* Histogram analysis *********/
	float mean;
	float variance;
	float std;
	unordered_map<int, float> stackMeanMap;

	inline vector<int>* getHist2D(vector<int>* inputImgVecPtr, int binNum);
	/**************************************/

	static inline void correlation2D(unsigned char input1D_1[], unsigned char input1D_2[], long int dims[2], double& corrCoeff);
	static inline void autoCorr2D(unsigned char input1D_1[], unsigned char input1D_2[], long int dims[2], vector<double>*& corrCoeffs);
};

inline void AnalysisExplorer::correlation2D(unsigned char input1D_1[], unsigned char input1D_2[], long int dims[2], double& corrCoeff)
{
	float* selfCorr = new float[dims[0] * dims[1]];
	float* crossCorr = new float[dims[0] * dims[1]];
	float selfCorrSum = 0;
	float crossCorrSum = 0;
	for (size_t i = 0; i < dims[0] * dims[1]; ++i)
	{
		selfCorr[i] = (float(input1D_1[i]) / 255) * (float(input1D_1[i]) / 255);
		selfCorrSum = selfCorrSum + selfCorr[i];

		crossCorr[i] = (float(input1D_1[i]) / 255) * (float(input1D_2[i]) / 255);
		crossCorrSum = crossCorrSum + crossCorr[i];
	}
	cout << selfCorrSum << " " << crossCorrSum << endl;

	if (selfCorrSum == 0)
	{
		cerr << "No signal. Return." << endl;
		return;
	}
	corrCoeff = crossCorrSum / selfCorrSum;
}

inline void AnalysisExplorer::autoCorr2D(unsigned char input1D_1[], unsigned char input1D_2[], long int dims[2], vector<double>*& corrCoeffs)
{

}

inline vector<int>* AnalysisExplorer::getHist2D(vector<int>* inputImgVecPtr, int binNum)
{
	vector<int> outputHistVec(binNum, 0);
	float sum = 0;
	
	for (vector<int>::iterator it = inputImgVecPtr->begin(); it != inputImgVecPtr->end(); ++it)
	{
		sum = sum + *it;
		int value = int(*it);
		++outputHistVec.at(value);
	}
	this->mean = sum / float(inputImgVecPtr->size());
	cout << "mean value: " << this->mean;

	float varSum = 0;
	for (vector<int>::iterator it = inputImgVecPtr->begin(); it != inputImgVecPtr->end(); ++it)
		varSum = varSum + (*it - this->mean) * (*it - this->mean);

	this->variance = varSum / float(inputImgVecPtr->size());
	this->std = sqrt(this->variance);
	cout << "   std: " << this->std << endl;

	return &outputHistVec;
}


#endif