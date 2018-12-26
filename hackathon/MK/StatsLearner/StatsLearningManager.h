#ifndef STATSLEARNINGMANAGER_H
#define STATSLEARNINGMANAGER_H

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>

#include <boost/bimap.hpp>
#include <boost/container/flat_map.hpp>

#include "FeatureExtractor.h"

using namespace std;

struct Observation
{
	int sampleID;
	string sampleName;
	map<string, boost::container::flat_map<string, double>> featureMaps;
};

class StatsLearningManager
{
public:
	/***************** Constructors and Basic Data Members *****************/
	boost::bimap<string, int> featureCode_biMap;
	boost::bimap<string, int> sampleCode_biMap;
	list<Observation> inputData;
	/***********************************************************************/


};

#endif