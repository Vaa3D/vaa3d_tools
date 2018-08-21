#ifndef IMGANALYZER_H
#define IMGANALYZER_H

#include <iostream>
#include <vector>
#include <map>
#include <set>

#include "ImgManager.h"
#include "ImgProcessor.h"

using namespace std;

struct connectedComponent
{
	int islandNum;
	multimap<int, vector<int> > coords;
	map<int, vector<int> > finalCoords;
	long int size;
	vector<float> centroid;
};

class ImgAnalyzer
{
public:
	static vector<connectedComponent> findConnectedComponent(vector<unsigned char**> inputSlicesVector, int imgDims[]);
};

#endif