//------------------------------------------------------------------------------
// Copyright (c) 2018 Hsienchi Kuo (Allen Institute)
// All rights reserved.
//------------------------------------------------------------------------------

/*******************************************************************************
*
*  Most of NeuronStructUtil class methods intend to operate on the whole neuron struct level. 
*  As 'utility' it is called, the functionalities provided in this class include:
*    a. [Basic neuron struct operations]                   -- cropping SWC, scaling SWC, swc registration, etc.
*    b. [Tree - subtree operations]                        -- extracting upstream or downstream of a given tree.
*    c. [SWC - ImgAnalyzer::connectedComponent operations] -- Methods of this category convert SWC into vector<ImgAnalyzer::connectedComponent>
*  
*  Most of NeuronStructUtil class methods are implemented as static functions. The input NeuronTree is always set to be const so that it will not be modified.
*  A typical function call would need at least three input arguments:
*
*		NeuronStructUtil::func(const NeuronTree& inputTree, NeuronTree& outputTree, other input arguments);
*
********************************************************************************/

#ifndef NEURONSTRUCTUTILITIES_H
#define NEURONSTRUCTUTILITIES_H

#include <fstream>

#include <boost/algorithm/string.hpp>

#include <qstring.h>
#include <qstringlist.h>

#include "integratedDataTypes.h"

using namespace std;
using namespace integratedDataTypes;

class NeuronStructUtil
{
public: 
	/***************** Constructors and Basic Data/Function Members *****************/
	// Not needed at the moment. Will implement later if necessary.
	/********************************************************************************/


	/*************************** Segment Operations ***************************/
	static inline connectOrientation getConnOrientation(connectOrientation orit1, connectOrientation orrit2);

	// Generate a new segment that is connected with 2 input segments. Connecting orientation needs to be specified by connOrt.
	// This method is a generalized method and is normally the final step of segment connecting process.
	// NOTE, currently it only supports simple unilateral segment. Forked segment connection will result in throwing error message!!
	static segUnit segUnitConnect_end2end(const segUnit& segUnit1, const segUnit& segUnit2, connectOrientation connOrt);
	
	//static segUnit segUnitConnect_end2body(const segUnit& endSegUnit, const segUnit& bodySegUnit, const NeuronSWC& endSegNode, const NeuronSWC& bodySegNode);
	/**************************************************************************/


	/************************ Neuron Struct Processing ************************/
	// ----------------- Basic Operations ----------------- //
	static inline void swcChangeType(NeuronTree& inputTree, const int type) { for (auto& node : inputTree.listNeuron) node.type = type; }

	static void swcSlicer(const NeuronTree& inputTree, vector<NeuronTree>& outputTrees, int thickness = 1);

	template<typename T>
	static inline void swcCrop(const NeuronTree& inputTree, NeuronTree& outputTree, T xlb, T xhb, T ylb, T yhb, T zlb, T zhb);

	template<typename T> // Scailing by multiplication.
	static inline NeuronTree swcScale(const NeuronTree& inputTree, T xScale, T yScale, T zScale); 

	template<typename T> // Scailing by multiplication.
	static inline QList<CellAPO> apoScale(const QList<CellAPO>& inputApo, T xScale, T yScale, T zScale);

	template<typename T> // Shifting by addition.
	static inline NeuronTree swcShift(const NeuronTree& inputTree, T xShift, T yShift, T zShift);

	template<typename T> // Y points downward, Z points to the right, posiive degree for counter clock-wise.
	static inline NeuronTree swcRotateX_degree(const NeuronTree& inputTree, T degree, T yOrigin = 0, T zOrigin = 0);

	template<typename T> // X points downward, Z points to the right, posiive degree for counter clock-wise.
	static inline NeuronTree swcRotateY_degree(const NeuronTree& inputTree, T degree, T xOrigin = 0, T zOrigin = 0);

	template<typename T> // X points downward, Y points to the right, posiive degree for counter clock-wise.
	static inline NeuronTree swcRotateZ_degree(const NeuronTree& inputTree, T degree, T xOrigin = 0, T yOrigin = 0);
	
	template<typename T> // Get the coordinate boundaries of the inputTree. 6 elements stored in the retruned vector: xMin, xMax, yMin, yNax, zMin, zMax.
	static inline vector<T> getSWCboundary(const NeuronTree& inputTree);

	// Align inputTree with refTree.
	static NeuronTree swcRegister(NeuronTree& inputTree, const NeuronTree& refTree); 

	static NeuronTree swcCombine(const vector<NeuronTree>& inputTrees);
	static NeuronTree swcCombine(const map<string, NeuronTree>& inputTreesMap);
	static NeuronTree swcCombine(const map<string, profiledTree>& inputProfiledTreesMap);
	
	static bool checkNodeType(const NeuronTree& inputTree, int nodeType);
	static map<int, QList<NeuronSWC>> swcSplitByType(const NeuronTree& inputTree);

	static void splitNodeList(const QList<NeuronSWC>& inputNodes, const int splittingNodeID, QList<NeuronSWC>& downStreamNodes, QList<NeuronSWC>& upStreamNodes);

	// Subtract refTree from targetTree.
	static NeuronTree swcSubtraction(const NeuronTree& targetTree, const NeuronTree& refTree, int type = 0); 
	// ------------ END of [Basic Operations] ------------- //
	
	// ------------- Higher level processing -------------- //
	static bool isSorted(const NeuronTree& inputNeuronTree);								// ~~ Not implemented yet ~~
	static NeuronTree sortTree(const NeuronTree& inputNeuronTree, const float somaCoord[]); // ~~ Not implemented yet ~~

	static int findSomaNodeID(const NeuronTree& inputTree);
	static void somaCleanUp(NeuronTree& inputTree);	   // ~~ Incomplete ~~ may not needed at all.
	
	static bool multipleSegsCheck(const NeuronTree& inputTree);

	// Remove any segment that is "in the shadow" of other segments, i.e., every node of the segment is overlapped with nodes of other segments.
	// The overlapping situation could be 1. Simple shadow segments: identical segments
	//									  2. Composite shadow segments: a) shadow segments that align through multiple segments
	//																	b) partial shadow segments - embbeded segments
	static NeuronTree removeDupSegs(const NeuronTree& inputTree);

	// Remove duplicated nodes that are linked -> child node and parent node share the same coordinate.
	// Note: Not to be confused with [NeuronStructUtil::removeDupHeads], which is dedicated to linking geometrically connected segments altogether.
	static void removeRedunNodes(profiledTree& inputProfiledTree); 

	// Use [integratedDatatypes::profiledTree::overlappedCoordMap] to search for removable head nodes and determine how links should be made.
	static bool removeDupHeads(NeuronTree& inputTree); 
	
	static void removeDupBranchingNodes(profiledTree& inputProfiledTree); // Faster, but occasionally doesn't do the job right. 
																		  // Probably resulted from segment combing function not thorough enough - [integratedDataTypes::profiledTree::combSegs()] 

	static NeuronTree singleDotRemove(const profiledTree& inputProfiledTree, int shortSegRemove = 0);
	static NeuronTree longConnCut(const profiledTree& inputProfiledTree, double distThre = 50);
	static NeuronTree segTerminalize(const profiledTree& inputProfiledTree);

	// When using SWC root nodes to represent signals, this method can be used to reduce node density.
	// -- NOTE, this method can only be used when all nodes are roots. 
	template<typename T>
	static inline void swcDownSample_allRoots(const NeuronTree& inputTree, NeuronTree& outputTree, T factor, bool shrink);

	// This method creates interpolated nodes in between each pair of 2 adjacent nodes on the input tree. 
	static void treeUpSample(const profiledTree& inputProfiledTree, profiledTree& outputProfiledTree, float intervalLength = 5);

	// -- This method "down samples" the input tree segment by segment. 
	// -- A recursive down sampling method [NeuronStructUtil::rc_segDownSample] is called in this function to deal with all possible braching points in each segment.
	// -- NOTE, this method is essentially used for straightening / smoothing segments when there are too many zigzagging.  
	static profiledTree treeDownSample(const profiledTree& inputProfiledTree, int nodeInterval = 2);
	static void rc_segDownSample(const segUnit& inputSeg, QList<NeuronSWC>& outputNodeList, int branchingNodeID, int interval);
	// --------- END of [Higher level processing] --------- //

	// ------------------- Conversions -------------------- //
	static QList<NeuronSWC> V_NeuronSWC2nodeList(const vector<V_NeuronSWC_unit>& inputV_NeuronSWC);
	// ---------------------------------------------------- //
	/**************************************************************************/


	/***************** SWC to ImgAnalyzer::connectedComponent *****************/
	// Identify the connectivity of every node in the input swc and generate connected component data from it.
	static vector<connectedComponent> swc2signal2DBlobs(const NeuronTree& inputTree);
	static vector<connectedComponent> swc2signal3DBlobs(const NeuronTree& inputTree);

	// Merge 2D connected components into 3D ones.
	static vector<connectedComponent> merge2DConnComponent(const vector<connectedComponent>& inputConnCompList);

	// Convert connected component data type into swc format.
	static NeuronTree blobs2tree(const vector<connectedComponent>& inputConnCompList, bool usingRadius2compNum = false);
	/**************************************************************************/


	/***************** Artificial SWC for Developing Purposes *****************/
	// Generates a cubical root node cluster with specified cube length.
	static inline NeuronTree randNodes(float cubeLength, float density);

	// Generates a spherical root node cluster with specified origin, radius, and node density.
	// The total amount of nodes = (4 / 3) * PI * radius^3 * density.
	static inline NeuronTree sphereRandNodes(const float radius, const float centerX, const float centerY, const float centerZ, const float density);

	static inline NeuronTree circleRandNodes(const float radius, const float centerX, const float centerY, const float centerZ, const float density);

	// Generates multiple root node clusters with specified origin, radius, and node density.
	// Each cluster center is separated by stepX, stepY, and stepZ. xRange, yRange, and zRange specify the space range.
	static NeuronTree nodeSpheresGen(float sphereRadius, float density, float stepX, float stepY, float stepZ, float xRange, float yRange, float zRange);
	/**************************************************************************
	
	
	/***************************** Miscellaneous ******************************/ 
	// Generates linker file for swc
	static inline void linkerFileGen_forSWC(string swcFullFileName);	

	// Convert HUST swc
	static NeuronTree convertHUSTswc(QString inputFileNameQ);
	static vector<NeuronTree> convertHUSTswc_old(QString inputQ);
	/**************************************************************************/


	/* ~~~~~~~~~~~~~~~ Sampling Methods for Simulated Volumetric Patch Generation ~~~~~~~~~~~~~~~ */
	static void swcSlicer_DL(const NeuronTree& inputTree, vector<NeuronTree>& outputTrees, int thickness = 0);
	static void sigNode_Gen(const NeuronTree& inputTree, NeuronTree& outputTree, float ratio, float distance);
	static void bkgNode_Gen(const NeuronTree& inputTree, NeuronTree& outputTree, int dims[], float ratio, float distance);
	static void bkgNode_Gen_somaArea(const NeuronTree& inputTree, NeuronTree& outputTree, int xLength, int yLength, int zLength, float ratio, float distance);
	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
};

inline connectOrientation NeuronStructUtil::getConnOrientation(connectOrientation orit1, connectOrientation orit2)
{
	if (orit1 == head && orit2 == head) return head_head;
	else if (orit1 == head && orit2 == tail) return head_tail;
	else if (orit1 == tail && orit2 == head) return tail_head;
	else if (orit1 == tail && orit2 == tail) return tail_tail;
}

template<typename T>
inline vector<T> NeuronStructUtil::getSWCboundary(const NeuronTree& inputTree)
{
	T xMin = 10000, xMax = 0, yMin = 10000, yMax = 0, zMin = 10000, zMax = 0;
	for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		if (it->x > xMax) xMax = it->x;
		else if (it->x < xMin) xMin = it->x;

		if (it->y > yMax) yMax = it->y;
		else if (it->y < yMin) yMin = it->y;

		if (it->z > zMax) zMax = it->z;
		else if (it->z < zMin) zMin = it->z;
	}

	vector<T> boundaries(6);
	boundaries[0] = xMin;
	boundaries[1] = xMax;
	boundaries[2] = yMin;
	boundaries[3] = yMax;
	boundaries[4] = zMin;
	boundaries[5] = zMax;

	return boundaries;
}

template<typename T>
inline NeuronTree NeuronStructUtil::swcScale(const NeuronTree& inputTree, T xScale, T yScale, T zScale)
{
	NeuronTree outputTree;
	for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		NeuronSWC newNode;
		newNode = *it;
		newNode.x = it->x * xScale;
		newNode.y = it->y * yScale;
		newNode.z = it->z * zScale;
		outputTree.listNeuron.push_back(newNode);
	}

	return outputTree;
}

template<typename T>
inline QList<CellAPO> NeuronStructUtil::apoScale(const QList<CellAPO>& inputApo, T xScale, T yScale, T zScale)
{
	QList<CellAPO> outputApo;
	for (QList<CellAPO>::const_iterator it = inputApo.begin(); it != inputApo.end(); ++it)
	{
		CellAPO newApo = *it;
		newApo.x = it->x * xScale;
		newApo.y = it->y * yScale;
		newApo.z = it->z * zScale;
		outputApo.push_back(newApo);
	}

	return outputApo;
}

template<typename T>
inline NeuronTree NeuronStructUtil::swcShift(const NeuronTree& inputTree, T xShift, T yShift, T zShift)
{
	NeuronTree outputTree;
	for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		NeuronSWC newNode;
		newNode = *it;
		newNode.x = it->x + xShift;
		newNode.y = it->y + yShift;
		newNode.z = it->z + zShift;
		outputTree.listNeuron.push_back(newNode);
	}

	return outputTree;
}

template<typename T>
inline NeuronTree NeuronStructUtil::swcRotateX_degree(const NeuronTree& inputTree, T degree, T yOrigin, T zOrigin)
{
	NeuronTree outputTree;
	float inputRad = (float(degree) / 180) * PI_MK;
	for (auto& node : inputTree.listNeuron)
	{
		NeuronSWC newNode = node;
		newNode.y = newNode.y - yOrigin;
		newNode.z = newNode.z - zOrigin;
		float oldY = newNode.y;
		float oldZ = newNode.z;
		newNode.y = oldY * std::cosf(inputRad) - oldZ * std::sinf(inputRad);
		newNode.z = oldY * std::sinf(inputRad) + oldZ * std::cosf(inputRad);
		newNode.y = newNode.y + yOrigin;
		newNode.z = newNode.z + zOrigin;
		outputTree.listNeuron.append(newNode);
	}

	return outputTree;
}

template<typename T>
inline NeuronTree NeuronStructUtil::swcRotateY_degree(const NeuronTree& inputTree, T degree, T xOrigin, T zOrigin)
{
	NeuronTree outputTree;
	float inputRad = (float(degree) / 180) * PI_MK;
	for (auto& node : inputTree.listNeuron)
	{
		NeuronSWC newNode = node;
		newNode.x = newNode.x - xOrigin;
		newNode.z = newNode.z - zOrigin;
		float oldX = newNode.x;
		float oldZ = newNode.z;
		newNode.x = oldX * std::cosf(inputRad) - oldZ * std::sinf(inputRad);
		newNode.z = oldX * std::sinf(inputRad) + oldZ * std::cosf(inputRad);
		newNode.x = newNode.x + xOrigin;
		newNode.z = newNode.z + zOrigin;
		outputTree.listNeuron.append(newNode);
	}

	return outputTree;
}

template<typename T>
inline NeuronTree NeuronStructUtil::swcRotateZ_degree(const NeuronTree& inputTree, T degree, T xOrigin, T yOrigin)
{
	NeuronTree outputTree;
	float inputRad = (float(degree) / 180) * PI_MK;
	for (auto& node : inputTree.listNeuron)
	{
		NeuronSWC newNode = node;
		newNode.x = newNode.x - xOrigin;
		newNode.y = newNode.y - yOrigin;
		float oldX = newNode.x;
		float oldY = newNode.y;
		newNode.x = oldX * std::cosf(inputRad) - oldY * std::sinf(inputRad);
		newNode.y = oldX * std::cosf(inputRad) + oldY * std::sinf(inputRad);
		newNode.x = newNode.x + xOrigin;
		newNode.y = newNode.y + yOrigin;
		outputTree.listNeuron.append(newNode);
	}

	return outputTree;
}

template<typename T>
inline void NeuronStructUtil::swcCrop(const NeuronTree& inputTree, NeuronTree& outputTree, T xlb, T xhb, T ylb, T yhb, T zlb, T zhb)
{
	for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		if (it->x < xlb || it->x > xhb || it->y < ylb || it->y > yhb || it->z < zlb || it->z > zhb) continue;
		else outputTree.listNeuron.push_back(*it);
	}
}

template<typename T>
inline void NeuronStructUtil::swcDownSample_allRoots(const NeuronTree& inputTree, NeuronTree& outputTree, T factor, bool shrink)
{
	QList<NeuronSWC> inputList = inputTree.listNeuron;
	outputTree.listNeuron.clear();
	for (QList<NeuronSWC>::iterator it = inputList.begin(); it != inputList.end(); ++it)
	{
		if (T(it->z) % factor == 0 && T(it->x) % factor == 0 && T(it->y) % factor == 0) 
			outputTree.listNeuron.push_back(*it);
	}

	if (shrink)
	{
		for (QList<NeuronSWC>::iterator it = outputTree.listNeuron.begin(); it != outputTree.listNeuron.end(); ++it)
		{
			it->x = it->x / 2;
			it->y = it->y / 2;
			it->z = it->z / 2;
		}
	}
}

inline NeuronTree NeuronStructUtil::randNodes(float cubeLength, float density)
{
	NeuronTree outputTree;
	int targetNodeCount = int(cubeLength * cubeLength * cubeLength * density);
	int producedNodeCount = 0;
	while (producedNodeCount <= targetNodeCount)
	{
		int randNumX = rand() % int(cubeLength) + 1;
		int randNumY = rand() % int(cubeLength) + 1;
		int randNumZ = rand() % int(cubeLength) + 1;

		NeuronSWC newNode;
		newNode.n = producedNodeCount + 1;
		newNode.x = randNumX;
		newNode.y = randNumY;
		newNode.z = randNumZ;
		newNode.type = 2;
		newNode.parent = -1;
		outputTree.listNeuron.push_back(newNode);

		++producedNodeCount;
	}

	return outputTree;
}

inline NeuronTree NeuronStructUtil::sphereRandNodes(float radius, float centerX, float centerY, float centerZ, float density)
{
	int targetNum = int((4 / 3) * PI_MK * radius * radius * radius * density);
	int nodeCount = 1;
	NeuronTree outputTree;
	while (nodeCount <= targetNum)
	{
		float randomX = float(rand()) / float(RAND_MAX) * (2 * radius) + (centerX - radius);
		float randomY = float(rand()) / float(RAND_MAX) * (2 * radius) + (centerY - radius);
		float randomZ = float(rand()) / float(RAND_MAX) * (2 * radius) + (centerZ - radius);

		float dist = sqrtf((randomX - centerX) * (randomX - centerX) + (randomY - centerY) * (randomY - centerY) + (randomZ - centerZ) * (randomZ - centerZ));
		if (dist > radius) continue;

		NeuronSWC newNode;
		newNode.n = nodeCount;
		newNode.x = randomX;
		newNode.y = randomY;
		newNode.z = randomZ / zRATIO;
		newNode.type = 2;
		newNode.parent = -1;
		outputTree.listNeuron.push_back(newNode);

		++nodeCount;
	}
	
	return outputTree;
}

inline NeuronTree NeuronStructUtil::circleRandNodes(const float radius, const float centerX, const float centerY, const float centerZ, const float density)
{
	int targetNum = int(PI_MK * radius * radius * density);
	int nodeCount = 1;
	NeuronTree outputTree;
	while (nodeCount <= targetNum)
	{
		float randomX = float(rand()) / float(RAND_MAX) * (2 * radius) + (centerX - radius);
		float randomY = float(rand()) / float(RAND_MAX) * (2 * radius) + (centerY - radius);
		float randomZ = centerZ;

		float dist = sqrtf((randomX - centerX) * (randomX - centerX) + (randomY - centerY) * (randomY - centerY));
		if (dist > radius) continue;

		NeuronSWC newNode;
		newNode.n = nodeCount;
		newNode.x = randomX;
		newNode.y = randomY;
		newNode.z = centerZ;
		newNode.type = 2;
		newNode.parent = -1;
		outputTree.listNeuron.push_back(newNode);

		++nodeCount;
	}

	return outputTree;
}

inline void NeuronStructUtil::linkerFileGen_forSWC(string swcFullFileName)
{
	vector<string> swcFullNameParse;
	boost::split(swcFullNameParse, swcFullFileName, boost::is_any_of("\\"));
	string swcName = swcFullNameParse.back();
	vector<string> swcBaseNameParse;
	boost::split(swcBaseNameParse, swcName, boost::is_any_of("."));
	string swcBaseName = swcBaseNameParse.front();

	string rootPath;
	for (vector<string>::iterator it = swcFullNameParse.begin(); it != swcFullNameParse.end() - 1; ++it)
		rootPath = rootPath + *it + "/";
	string anoFullName = rootPath + swcBaseName + ".ano";

	ofstream outputFile(anoFullName.c_str());
	outputFile << "SWCFILE=" << swcName << endl;
	outputFile.close();
}

#endif 