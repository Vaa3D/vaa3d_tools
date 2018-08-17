#ifndef IMGANALYZER_H
#define IMGANALYZER_H

#include <iostream>
#include <vector>
#include <map>

#include "ImgProcessor.h"

using namespace std;

struct connectedComponent
{
	map<string, int[3]> coords;
	long int size;
	vector<float> centroid;
};

class ImgAnalyzer
{
public:
	static vector<connectedComponent> findConnectedComponent(vector<unsigned char**> inputSlicesVector, int imgDims[]);
};

#endif