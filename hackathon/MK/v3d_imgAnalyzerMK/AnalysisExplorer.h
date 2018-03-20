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
	
	float mean;
	float variance;
	float std;
	unordered_map<int, float> stackMeanMap;

	vector<int>* getHist2D(vector<int>* inputImgVecPtr, int binNum);
	unordered_map<int, float> sliceMeanMapper(string slicePath);
};

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