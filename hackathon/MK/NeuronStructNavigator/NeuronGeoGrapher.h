#ifndef NEURONGEOGRAPHER_H
#define NEURONGEOGRAPHER_H

#include <vector>
#include <cmath>

#include "integratedDataTypes.h"

using namespace std;
using namespace integratedDataTypes;

class NeuronGeoGrapher
{
public:
	static inline double getRadAngle(const vector<float>& vector1, const vector<float>& vector2);


	static inline NeuronSWC_polar CartesianNode2Polar(const NeuronSWC& inputNode, const vector<double>& origin);




};

inline double NeuronGeoGrapher::getRadAngle(const vector<float>& vector1, const vector<float>& vector2)
{
	double dot = (vector1.at(0) * vector2.at(0) + vector1.at(1) * vector2.at(1) + vector1.at(2) * vector2.at(2));
	double sq1 = (vector1.at(0) * vector1.at(0) + vector1.at(1) * vector1.at(1) + vector1.at(2) * vector1.at(2));
	double sq2 = (vector2.at(0) * vector2.at(0) + vector2.at(1) * vector2.at(1) + vector2.at(2) * vector2.at(2));
	double angle = acos(dot / sqrt(sq1 * sq2));

	if (std::isnan(acos(dot / sqrt(sq1 * sq2)))) return -10;
	else return angle;
}

inline NeuronSWC_polar NeuronGeoGrapher::CartesianNode2Polar(const NeuronSWC& inputNode, const vector<double>& origin)
{
	NeuronSWC_polar newNode_polar;
	newNode_polar.ID = inputNode.n;
	newNode_polar.type = inputNode.type;
	newNode_polar.parent = inputNode.parent;
	newNode_polar.CartesianX = inputNode.x;
	newNode_polar.CartesianY = inputNode.y;
	newNode_polar.CartesianZ = inputNode.z;
	newNode_polar.polarOriginX = origin.at(0);
	newNode_polar.polarOriginY = origin.at(1);
	newNode_polar.polarOriginZ = origin.at(2);
	
	vector<double> horizontal_refVec(3);
	vector<double> vertical_refVec(3);
	horizontal_refVec[0] = origin.at(0) + 10;
	horizontal_refVec[1] = origin.at(1);
	horizontal_refVec[2] = origin.at(2);
	vertical_refVec[0] = origin.at(0);
	vertical_refVec[1] = origin.at(1);
	vertical_refVec[2] = origin.at(2) + 10;


	return newNode_polar;
}

#endif