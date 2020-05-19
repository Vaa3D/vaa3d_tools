//------------------------------------------------------------------------------
// Copyright (c) 2019 Hsienchi Kuo (Allen Institute, Hanchuan Peng's team)
// All rights reserved.
//------------------------------------------------------------------------------

/*******************************************************************************
*
*  NeuronGeoGrapher provides functionalities handling geometrical/topological analysis, operations for neuron data structures.
*
*  Major functionalities include:
*
*    a. Basic vector geometry
*    b. Neuron segment geometry extended from basic vector geometry methds
*    c. Polar coordinate system operations for NeuronSWC, i.e., NeuronSwc <-> polarNeuronSWC conversion
*    d. SWC-based connected component geometrical analysis
*
*  NeuronGeoGrapher produces basis information that is essential in other class, eg, TreeGrower, for further development in higher level of neuron data structure processing.
*  Most methods in this class are static functions since most of them serve as utilities to the need of other classes.
*
********************************************************************************/

#ifndef NEURONGEOGRAPHER_H
#define NEURONGEOGRAPHER_H

#include <cmath>

#include <qlist.h>

#include "integratedDataTypes.h"
#include "NeuronStructUtilities.h"
#include "integratedDataStructures.h"

using namespace std;
using namespace integratedDataTypes;

class NeuronGeoGrapher
{
public:
	/***************** Constructors and Basic Data/Function Members *****************/
	// Not needed at the moment. Will implement later if necessary.
	/********************************************************************************/



	/*************************** Vector Geometry ***************************/
	template<typename T> // Get the vecotr formed by 2 NeuronSWC nodes.
	static inline vector<T> getVector_NeuronSWC(const NeuronSWC& startNode, const NeuronSWC& endNode);

	template<typename T> // Get the displacement vector of an input [segUnit] with specified orientation.
	static inline vector<T> getSegDispVector(const segUnit& inputSeg, connectOrientation connOrt);

	template<typename T> // Get the unit displacement vector of an input segUnit with specified orientation.
	static inline vector<T> getSegDispUnitVector(const segUnit& inputSeg, connectOrientation connOrt);

	template<typename T> // Get the unit displacement vector between 2 vectors
	static inline vector<T> getDispUnitVector(const vector<T>& endingLoc, const vector<T>& startingLoc);

	// Get the vector between 2 NeuronSWC nodes with the vector's starting location (startNode). 
	// The starting location in each dimension and its components are stored in pairs separately. 
	template<typename T> 
	static inline vector<pair<T, T>> getVectorWithStartingLoc_btwn2nodes(const NeuronSWC& startNode, const NeuronSWC& endNode);

	template<typename T> // Get cosine between 2 given vecotrs.
	static inline T getVectorCosine(const vector<T>& vector1, const vector<T>& vector2);
	
	template<typename T> // Get sine between 2 given vectors.
	static inline T getVectorSine(const vector<T>& vector1, const vector<T>& vector2);

	template<typename T> // Get the included angle of 2 given vectors in pi.
	static inline T getPiAngle(const vector<T>& vector1, const vector<T>& vector2);

	template<typename T> // Get the included angle of 2 given vectors in radian.
	static inline T getRadAngle(const vector<T>& vector1, const vector<T>& vector2);

	template<typename T> // Get the forming angle with 3 input nodes.
	static T get3nodesFormingAngle(const NeuronSWC& angularNode, const NeuronSWC& endNode1, const NeuronSWC& endNode2);

	template<typename T> // Get the forming angle of a given node on a segment. (not implemented yet)
	static T get3nodesFormingAngle(const NeuronSWC& angularNode, const segUnit& inputSeg);

	// Find out the projection vector onto the given axial vector from the projecting vector.
	static vector<pair<float, float>> getProjectionVector(const vector<pair<float, float>>& axialVector, const vector<pair<float, float>>& projectingVector);
	/***********************************************************************/



	/*************************** Segment Geometry **************************/
	template<typename T> // Calculate the sum of turning angles from the segment head to tail.
	static inline T selfTurningRadAngleSum(const vector<vector<T>>& inputSegment);

	// With specified pointing directions for each segments, returns the included angle between the 2 segments. 
	static double segPointingCompare(const segUnit& elongSeg, const segUnit& connSeg, connectOrientation connOrt);

	// This method calculates the "turning angle" from elongating segment to connected segment. 
	// The turning angle is defined as the angle formed by displacement vector of elongating segment and the displacement vector from elongating point to connecting point.
	static double segTurningAngle(const segUnit& elongSeg, const segUnit& connSeg, connectOrientation connOrt);

	static segUnit segmentStraighten(const segUnit& inputSeg); // --> May need to be revised later.
	/***********************************************************************/



	/****************** Polar Coordinate System Operations *****************/
	template<typename T> // Converts NeuronSWC to polarNeuronSWC with specified origin.
	static polarNeuronSWC CartesianNode2Polar(const NeuronSWC& inputNode, vector<T> origin = { 0, 0, 0 });

	template<typename T> // Converts input NeuronSWC list to polarNeuronSWC list with specified origin.
	static void nodeList2polarNodeList(const QList<NeuronSWC>& inputNodeList, vector<polarNeuronSWC>& outputPolarNodeList, vector<T> origin = { 0, 0, 0 });

	// Converts polarNeuronSWC to NeuronSWC.
	static inline NeuronSWC polar2CartesianNode(const polarNeuronSWC& inputPolarNode);
	
	// Convert polarNeuronSWC list to NeuronSWC list.
	static inline void polarNodeList2nodeList(const vector<polarNeuronSWC>& inputPolarNodeList, QList<NeuronSWC>& outputNodeList);

	// polarNeuronSWC ID -> location of the node on [inputPolarNodeList].
	static inline boost::container::flat_map<int, int> polarNodeID2locMap(const vector<polarNeuronSWC>& inputPolarNodeList);

	// radius -> IDs of those nodes on inputPolarNodeList that share the same radius.
	static boost::container::flat_map<double, boost::container::flat_set<int>> getShellByRadius_ID(const vector<polarNeuronSWC>& inputPolarNodeList);

	// radius -> locations of those nodes on inputPolarNodeList that share the same radius.
	static boost::container::flat_map<double, boost::container::flat_set<int>> getShellByRadius_loc(const vector<polarNeuronSWC>& inputPolarNodeList, double thickness = 1);
	
	/* -------------------- Polar Coord System Operations --------------------- */
	// Returns a map of radius to shell trees. [radiusShellMap_loc] = map of radius to locations of polarNeuronSWC on [inputPolarNodeList].
	// Note, [radiusShellMap_loc] can be obtained through NeuronGeoGrapher::nodeList2polarNodeList.
	static boost::container::flat_map<double, NeuronTree> radius2NeuronTreeMap(const boost::container::flat_map<double, boost::container::flat_set<int>>& radiusShellMap_loc, const vector<polarNeuronSWC>& inputPolarNodeList);

	static boost::container::flat_map<double, vector<connectedComponent>> radius2connCompsShell(const boost::container::flat_map<double, NeuronTree>& inputRadius2TreeMap);
	/* ------------------------------------------------------------------------ */
	/***********************************************************************/



	/*********** SWC - ImgAnalyzer::ConnectedComponent Analysis ************/
	// Compute and store the ChebyshevCenter for every connected component on the list.
	static inline void findChebyshevCenter_compList(vector<connectedComponent>& inputCompList) { ChebyshevCenter_connCompList(inputCompList); }
	
	// Checks if the bounding boxes of the 2 input connected components overlap. 
	// Note, this is only an APPROXIMATION inplemented for fast dendritic tree tracing.
	static inline bool connCompBoundingRangeCheck(const connectedComponent& comp1, const connectedComponent& comp2, double distThre = 1);
	/***********************************************************************/
};

template<typename T>
inline vector<T> NeuronGeoGrapher::getVector_NeuronSWC(const NeuronSWC& startNode, const NeuronSWC& endNode)
{
	vector<T> vec(3);
	vec[0] = endNode.x - startNode.x;
	vec[1] = endNode.y - startNode.y;
	vec[2] = endNode.z - startNode.z;
	return vec;
}

template<typename T>
inline vector<T> NeuronGeoGrapher::getSegDispVector(const segUnit& inputSeg, connectOrientation connOrt)
{
	NeuronSWC headNode = inputSeg.nodes.at(inputSeg.seg_nodeLocMap.at(inputSeg.head));
	NeuronSWC tailNode = inputSeg.nodes.at(inputSeg.seg_nodeLocMap.at(*inputSeg.tails.begin()));
	
	vector<T> vec;
	if (connOrt == head) vec = NeuronGeoGrapher::getVector_NeuronSWC<T>(headNode, tailNode);
	else if (connOrt == tail) vec = NeuronGeoGrapher::getVector_NeuronSWC<T>(tailNode, headNode);

	return vec;
}

template<typename T>
inline vector<T> NeuronGeoGrapher::getSegDispUnitVector(const segUnit& inputSeg, connectOrientation connOrt)
{
	vector<T> segDispVec = NeuronGeoGrapher::getSegDispVector(inputSeg, connOrt);
	T segLength = sqrt((segDispVec.at(0) * segDispVec.at(0)) + (segDispVec.at(1) * segDispVec.at(1)) + (segDispVec.at(2) * segDispVec.at(2)));
	
	vector<T> outputVec;
	outputVec[0] = segDispVec.at(0) / segLength;
	outputVec[1] = segDispVec.at(1) / segLength;
	outputVec[2] = segDispVec.at(2) / segLength;

	return outputVec;
}

template<typename T>
inline vector<T> NeuronGeoGrapher::getDispUnitVector(const vector<T>& endingLoc, const vector<T>& startingLoc)
{
	T disp = sqrt((endingLoc.at(0) - startingLoc.at(0)) * (endingLoc.at(0) - startingLoc.at(0)) +
				  (endingLoc.at(1) - startingLoc.at(1)) * (endingLoc.at(1) - startingLoc.at(1)) +
				  (endingLoc.at(2) - startingLoc.at(2)) * (endingLoc.at(2) - startingLoc.at(2)));
	vector<T> dispUnitVector;
	dispUnitVector.push_back((endingLoc.at(0) - startingLoc.at(0)) / disp);
	dispUnitVector.push_back((endingLoc.at(1) - startingLoc.at(1)) / disp);
	dispUnitVector.push_back((endingLoc.at(2) - startingLoc.at(2)) / disp);

	return dispUnitVector;
}

template<typename T>
inline vector<pair<T, T>> NeuronGeoGrapher::getVectorWithStartingLoc_btwn2nodes(const NeuronSWC& startNode, const NeuronSWC& endNode)
{
	vector<T> thisVector = NeuronGeoGrapher::getVector_NeuronSWC<T>(startNode, endNode);

	vector<pair<T, T>> outputVectorPairs(3);
	outputVectorPairs[0] = pair<T, T>(startNode.x, thisVector.at(0));
	outputVectorPairs[1] = pair<T, T>(startNode.y, thisVector.at(1));
	outputVectorPairs[2] = pair<T, T>(startNode.z, thisVector.at(2));

	return outputVectorPairs;
}

template<typename T>
inline T NeuronGeoGrapher::getVectorCosine(const vector<T>& vector1, const vector<T>& vector2)
{
	T dot = (vector1.at(0) * vector2.at(0) + vector1.at(1) * vector2.at(1) + vector1.at(2) * vector2.at(2));
	T sq1 = (vector1.at(0) * vector1.at(0) + vector1.at(1) * vector1.at(1) + vector1.at(2) * vector1.at(2));
	T sq2 = (vector2.at(0) * vector2.at(0) + vector2.at(1) * vector2.at(1) + vector2.at(2) * vector2.at(2));

	return dot / sqrt(sq1 * sq2);
}

template<typename T>
inline T NeuronGeoGrapher::getVectorSine(const vector<T>& vector1, const vector<T>& vector2)
{
	T thisVectorCos = NeuronGeoGrapher::getVectorCosine<T>(vector1, vector2);
	T thisVectorSin = sqrt(1 - thisVectorCos * thisVectorCos);

	return thisVectorSin;
}

template<typename T>
inline T NeuronGeoGrapher::getPiAngle(const vector<T>& vector1, const vector<T>& vector2)
{
	T dot = (vector1.at(0) * vector2.at(0) + vector1.at(1) * vector2.at(1) + vector1.at(2) * vector2.at(2));
	T sq1 = (vector1.at(0) * vector1.at(0) + vector1.at(1) * vector1.at(1) + vector1.at(2) * vector1.at(2));
	T sq2 = (vector2.at(0) * vector2.at(0) + vector2.at(1) * vector2.at(1) + vector2.at(2) * vector2.at(2));
	T angle = acos(dot / sqrt(sq1 * sq2));

	if (isnan(acos(dot / sqrt(sq1 * sq2)))) return -10;
	else return angle / PI_MK;
}

template<typename T>
inline T NeuronGeoGrapher::getRadAngle(const vector<T>& vector1, const vector<T>& vector2)
{
	T dot = (vector1.at(0) * vector2.at(0) + vector1.at(1) * vector2.at(1) + vector1.at(2) * vector2.at(2));
	T sq1 = (vector1.at(0) * vector1.at(0) + vector1.at(1) * vector1.at(1) + vector1.at(2) * vector1.at(2));
	T sq2 = (vector2.at(0) * vector2.at(0) + vector2.at(1) * vector2.at(1) + vector2.at(2) * vector2.at(2));
	T angle = acos(dot / sqrt(sq1 * sq2));

	if (isnan(acos(dot / sqrt(sq1 * sq2)))) return -10;
	else return angle;
}

template<typename T>
inline T NeuronGeoGrapher::selfTurningRadAngleSum(const vector<vector<T>>& inputSegment)
{
	T radAngleSum = 0;
	for (vector<vector<T>>::const_iterator it = inputSegment.begin() + 1; it != inputSegment.end() - 1; ++it)
	{
		vector<T> vector1(3), vector2(3);
		vector1[0] = it->at(0) - (it - 1)->at(0);
		vector1[1] = it->at(1) - (it - 1)->at(1);
		vector1[2] = it->at(2) - (it - 1)->at(2);
		vector2[0] = (it + 1)->at(0) - it->at(0);
		vector2[1] = (it + 1)->at(1) - it->at(1);
		vector2[2] = (it + 1)->at(2) - it->at(2);
		//cout << "(" << vector1[0] << ", " << vector1[1] << ", " << vector1[2] << ") (" << vector2[0] << ", " << vector2[1] << ", " << vector2[2] << ")" << endl;
		T radAngle = NeuronGeoGrapher::getRadAngle(vector1, vector2);

		if (radAngle == -1) continue;
		else radAngleSum = radAngleSum + radAngle;
	}

	return radAngleSum;
}

template<typename T>
T NeuronGeoGrapher::get3nodesFormingAngle(const NeuronSWC& angularNode, const NeuronSWC& endNode1, const NeuronSWC& endNode2)
{
	vector<T> vector1 = NeuronGeoGrapher::getVector_NeuronSWC<T>(endNode1, angularNode);
	vector<T> vector2 = NeuronGeoGrapher::getVector_NeuronSWC<T>(endNode2, angularNode);
	T piAngle = NeuronGeoGrapher::getPiAngle(vector1, vector2);
	
	return piAngle;
}

template<typename T>
polarNeuronSWC NeuronGeoGrapher::CartesianNode2Polar(const NeuronSWC& inputNode, vector<T> origin)
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

template<typename T>
void NeuronGeoGrapher::nodeList2polarNodeList(const QList<NeuronSWC>& inputNodeList, vector<polarNeuronSWC>& outputPolarNodeList, vector<T> origin)
{
	outputPolarNodeList.clear();
	for (QList<NeuronSWC>::const_iterator nodeIt = inputNodeList.begin(); nodeIt != inputNodeList.end(); ++nodeIt)
		outputPolarNodeList.push_back(NeuronGeoGrapher::CartesianNode2Polar(*nodeIt, origin));
}

inline NeuronSWC NeuronGeoGrapher::polar2CartesianNode(const polarNeuronSWC& inputPolarNode)
{
	NeuronSWC newNode;
	newNode.n = inputPolarNode.ID;
	newNode.type = inputPolarNode.type;
	newNode.parent = inputPolarNode.parent;
	newNode.x = inputPolarNode.CartesianX;
	newNode.y = inputPolarNode.CartesianY;
	newNode.z = inputPolarNode.CartesianZ;

	return newNode;
}

inline void NeuronGeoGrapher::polarNodeList2nodeList(const vector<polarNeuronSWC>& inputPolarNodeList, QList<NeuronSWC>& outputNodeList)
{
	outputNodeList.clear();
	for (vector<polarNeuronSWC>::const_iterator polarNodeIt = inputPolarNodeList.begin(); polarNodeIt != inputPolarNodeList.end(); ++polarNodeIt)
		outputNodeList.push_back(NeuronGeoGrapher::polar2CartesianNode(*polarNodeIt));
}

inline boost::container::flat_map<int, int> NeuronGeoGrapher::polarNodeID2locMap(const vector<polarNeuronSWC>& inputPolarNodeList)
{
	boost::container::flat_map<int, int> outputMap;
	for (vector<polarNeuronSWC>::const_iterator it = inputPolarNodeList.begin(); it != inputPolarNodeList.end(); ++it)
		outputMap.insert(pair<int, int>(it->ID, int(it - inputPolarNodeList.begin())));

	return outputMap;
}

inline bool NeuronGeoGrapher::connCompBoundingRangeCheck(const connectedComponent& comp1, const connectedComponent& comp2, double distThre)
{	
	if (comp1.xMin > comp2.xMax + distThre || comp1.xMax < comp2.xMin - distThre ||
		comp1.yMin > comp2.yMax + distThre || comp1.yMax < comp2.yMin - distThre ||
		comp1.zMin > comp2.zMax + distThre || comp1.zMax < comp2.zMin - distThre) return false;
	else return true;
}

#endif