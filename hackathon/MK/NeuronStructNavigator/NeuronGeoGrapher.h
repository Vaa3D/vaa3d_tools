#ifndef NEURONGEOGRAPHER_H
#define NEURONGEOGRAPHER_H

#include <vector>
#include <cmath>

#include <qlist.h>

#include "integratedDataTypes.h"

using namespace std;
using namespace integratedDataTypes;

class NeuronGeoGrapher
{
public:
	template<class T>
	inline static vector<T> getVector_NeuronSWC(const NeuronSWC& startNode, const NeuronSWC& endNode);

	template<class T>
	static inline T getRadAngle(const vector<T>& vector1, const vector<T>& vector2);

	template<class T>
	static inline polarNeuronSWC CartesianNode2Polar(const NeuronSWC& inputNode, const vector<T>& origin);

	template<class T>
	static inline void nodeList2polarNodeList(const QList<NeuronSWC>& inputNodeList, vector<polarNeuronSWC>& outputPolarNodeList, const vector<T>& origin);
};

template<class T>
inline vector<T> NeuronGeoGrapher::getVector_NeuronSWC(const NeuronSWC& startNode, const NeuronSWC& endNode)
{
	vector<T> vec(3);
	vec[0] = endNode.x - startNode.x;
	vec[1] = endNode.y - startNode.y;
	vec[2] = endNode.z - startNode.z;
	return vec;
}

template<class T>
inline T NeuronGeoGrapher::getRadAngle(const vector<T>& vector1, const vector<T>& vector2)
{
	T dot = (vector1.at(0) * vector2.at(0) + vector1.at(1) * vector2.at(1) + vector1.at(2) * vector2.at(2));
	T sq1 = (vector1.at(0) * vector1.at(0) + vector1.at(1) * vector1.at(1) + vector1.at(2) * vector1.at(2));
	T sq2 = (vector2.at(0) * vector2.at(0) + vector2.at(1) * vector2.at(1) + vector2.at(2) * vector2.at(2));
	T angle = acos(dot / sqrt(sq1 * sq2));

	if (std::isnan(acos(dot / sqrt(sq1 * sq2)))) return -10;
	else return angle;
}

template<class T>
inline polarNeuronSWC NeuronGeoGrapher::CartesianNode2Polar(const NeuronSWC& inputNode, const vector<T>& origin)
{
	polarNeuronSWC newPolarNode;
	newPolarNode.ID = inputNode.n;
	newPolarNode.type = inputNode.type;
	newPolarNode.parent = inputNode.parent;
	newPolarNode.CartesianX = inputNode.x;
	newPolarNode.CartesianY = inputNode.y;
	newPolarNode.CartesianZ = inputNode.z;
	newPolarNode.polarOriginX = origin.at(0);
	newPolarNode.polarOriginY = origin.at(1);
	newPolarNode.polarOriginZ = origin.at(2);
	
	vector<double> nodeVec(3);
	nodeVec[0] = inputNode.x - origin.at(0);
	nodeVec[1] = inputNode.y - origin.at(1);
	nodeVec[2] = inputNode.z - origin.at(2);

	vector<double> projectedVec(3);	
	projectedVec[0] = inputNode.x;
	projectedVec[1] = inputNode.y;
	projectedVec[2] = origin.at(2);

	vector<double> horizontal_refVec(3);
	horizontal_refVec[0] = origin.at(0) + 10;
	horizontal_refVec[1] = origin.at(1);
	horizontal_refVec[2] = origin.at(2);

	newPolarNode.radius = sqrt((double(inputNode.x) - origin.at(0)) * (double(inputNode.x) - origin.at(0)) + (double(inputNode.y) - origin.at(1)) * (double(inputNode.y) - origin.at(1)) + (double(inputNode.z) - origin.at(2)) * (double(inputNode.z) - origin.at(2)));
	newPolarNode.theta = NeuronGeoGrapher::getRadAngle(projectedVec, horizontal_refVec);
	newPolarNode.phi = NeuronGeoGrapher::getRadAngle(nodeVec, projectedVec);

	return newPolarNode;
}

template<class T>
inline void NeuronGeoGrapher::nodeList2polarNodeList(const QList<NeuronSWC>& inputNodeList, vector<polarNeuronSWC>& outputPolarNodeList, const vector<T>& origin)
{
	outputPolarNodeList.clear();
	for (QList<NeuronSWC>::const_iterator nodeIt = inputNodeList.begin(); nodeIt != inputNodeList.end(); ++nodeIt)
		outputPolarNodeList.push_back(NeuronGeoGrapher::CartesianNode2Polar(*nodeIt, origin));
}

#endif