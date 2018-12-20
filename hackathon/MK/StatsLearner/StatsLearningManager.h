#ifndef STATSLEARNINGMANAGER_H
#define STATSLEARNINGMANAGER_H

#include <iostream>
#include <vector>
#include <string>

#include <boost/bimap.hpp>
#include <boost/container/flat_map.hpp>

using namespace std;

struct Observation
{
	int sampleID;
	boost::container::flat_map<string, double> sampleFeatureMap;
};

class StatsLearningManager
{
public:
	/***************** Constructors and Basic Data Members *****************/
	

	boost::bimap<string, int> featureCode_biMap;
	boost::bimap<string, int> sampleCode_biMap;
	vector<Observation> inputData;
	/***********************************************************************/


};

#endif