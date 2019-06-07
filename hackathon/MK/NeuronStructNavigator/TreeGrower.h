#ifndef TREEGROWER_H
#define TREEGROWER_H

#include "integratedDataTypes.h"
#include "NeuronGeoGrapher.h"
#include "ImgAnalyzer.h"

class TreeGrower
{
public:
	static boost::container::flat_map<double, NeuronTree> radiusShellNeuronTreeMap(
		const boost::container::flat_map<double, boost::container::flat_set<int>>& inputRadiusMap, const vector<polarNeuronSWC>& inputPolarNodeList);

	static boost::container::flat_map<double, vector<connectedComponent>> shell2radiusConnMap(const boost::container::flat_map<double, NeuronTree> inputRadius2NeuronTreeMap);







};

#endif