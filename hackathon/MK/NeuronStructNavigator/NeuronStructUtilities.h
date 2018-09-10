#ifndef NEURONSTRUCTUTILITIES_H
#define NEURONSTRUCTUTILITIES_H

#include <vector>

#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>

#include "basic_surf_objs.h"
#include "ImgAnalyzer.h"

using namespace std;

#ifndef zRATIO // This is the ratio of z resolution to x and y in IVSCC images.
#define zRATIO (0.28 / 0.1144) 
#endif

class NeuronStructUtil
{
public: 
	NeuronStructUtil() {};

	/********* Basic Neuron Struct File Operations *********/
	static void swcSlicer(const NeuronTree& inputTree, vector<NeuronTree>& outputTrees, int thickness = 0);
	static void swcSliceAssembler(string swcPath);
	inline static void swcCrop(const NeuronTree& inputTree, NeuronTree& outputTree, float xlb, float xhb, float ylb, float yhb, float zlb, float zhb);
	inline static NeuronTree swcScale(const NeuronTree& inputTree, float xScale, float yScale, float zScale);
	inline static NeuronTree swcShift(const NeuronTree& inputTree, float xShift, float yShift, float zShift);
	static NeuronTree swcRegister(NeuronTree& inputTree, const NeuronTree& refTree);
	/*******************************************************/
	
	/********* SWC to ImgAnalyzer::connectedComponent *********/
	vector<connectedComponent> swc2signal2DBlobs(const NeuronTree& inputTree);
	vector<connectedComponent> swc2signal3DBlobs(const NeuronTree& inputTree);
	vector<connectedComponent> merge2DConnComponent(const vector<connectedComponent>& inputConnCompList);
	/**********************************************************/

	static NeuronTree swcZclenUP(const NeuronTree& inputTree, float zThre = 10);
	static NeuronTree swcIdentityCompare(const NeuronTree& subjectTree, const NeuronTree& refTree, float radius, float distThre);

	static void swcDownSampleZ(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr, int factor);
	static void detectedSWCprobFilter(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr, float threshold);
	/*******************************************************/

	/********* Sampling Methods for Simulated Volumetric Patch Generation *********/
	static void sigNode_Gen(const NeuronTree& inputTree, NeuronTree& outputTree, float ratio, float distance);
	static void bkgNode_Gen(const NeuronTree& inputTree, NeuronTree& outputTree, int dims[], float ratio, float distance);
	static void bkgNode_Gen_somaArea(const NeuronTree& inputTree, NeuronTree& outputTree, int xLength, int yLength, int zLength, float ratio, float distance);
	/******************************************************************************/
};

inline NeuronTree NeuronStructUtil::swcScale(const NeuronTree& inputTree, float xScale, float yScale, float zScale)
{
	NeuronTree outputTree;
	for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		NeuronSWC newNode;
		newNode = *it;
		newNode.x = it->x / xScale;
		newNode.y = it->y / yScale;
		newNode.z = it->z / zScale;
		outputTree.listNeuron.push_back(newNode);
	}

	return outputTree;
}

inline NeuronTree NeuronStructUtil::swcShift(const NeuronTree& inputTree, float xShift, float yShift, float zShift)
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

inline void NeuronStructUtil::swcCrop(const NeuronTree& inputTree, NeuronTree& outputTree, float xlb, float xhb, float ylb, float yhb, float zlb, float zhb)
{
	if (zlb == 0 && zhb == 0)
	{
		for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
		{
			if (it->x < xlb || it->x > xhb || it->y < ylb || it->y > yhb) continue;
			else
			{
				NeuronSWC newNode;
				newNode.x = it->x - (xlb - 1);
				newNode.y = it->y - (ylb - 1);
				newNode.z = it->z;
				newNode.type = it->type;
				newNode.n = it->n;
				newNode.parent = it->parent;
				outputTree.listNeuron.push_back(newNode);
			}
		}
	}
}

#endif 