#ifndef NEURONSTRUCTUTILITIES_H
#define NEURONSTRUCTUTILITIES_H

#include <vector>

#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>

#include "basic_surf_objs.h"

using namespace std;

class NeuronStructUtil
{
public: 
	NeuronStructUtil() {};

	/********* Basic neuron struct file operations *********/
	static void swcSlicer(NeuronTree* inputTreePtr, vector<NeuronTree>* outputTreesPtr, int thickness = 0);
	static void swcSliceAssembler(string swcPath);
	inline static void swcCrop(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr, float xlb, float xhb, float ylb, float yhb, float zlb, float zhb);
	inline static void swcFlipY(NeuronTree const* inputTreePtr, NeuronTree*& outputTreePtr, long int yLength);
	inline static NeuronTree swcScale(NeuronTree const* inputTreePtr, float xScale, float yScale, float zScale);
	static NeuronTree swcRegister(NeuronTree* inputTreePtr, NeuronTree const* refTreePtr, float customFactor = 1);

	static void swcDownSampleZ(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr, int factor);
	static void detectedSWCprobFilter(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr, float threshold);
	/*******************************************************/

	/********* Simulated volumetric patch generation *********/
	static void sigNode_Gen(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr, float ratio, float distance);
	static void bkgNode_Gen(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr, long int dims[], float ratio, float distance);
	static void bkgNode_Gen_somaArea(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr, int xLength, int yLength, int zLength, float ratio, float distance);
	/*********************************************************/
};

inline NeuronTree NeuronStructUtil::swcScale(NeuronTree const* inputTreePtr, float xScale, float yScale, float zScale)
{
	NeuronTree outputTree;
	for (QList<NeuronSWC>::const_iterator it = inputTreePtr->listNeuron.begin(); it != inputTreePtr->listNeuron.end(); ++it)
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

inline void NeuronStructUtil::swcFlipY(NeuronTree const* inputTreePtr, NeuronTree*& outputTreePtr, long int yLength)
{
	float yMiddle = float(yLength + 1) / 2;
	for (QList<NeuronSWC>::const_iterator it = inputTreePtr->listNeuron.begin(); it != inputTreePtr->listNeuron.end(); ++it)
	{
		NeuronSWC flippedNode = *it;
		if (it->y > yMiddle) flippedNode.y = (yMiddle - (it->y - yMiddle));
		else if (it->y < yMiddle) flippedNode.y = (yMiddle + (yMiddle - it->y));
	}
}

inline void NeuronStructUtil::swcCrop(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr, float xlb, float xhb, float ylb, float yhb, float zlb, float zhb)
{
	if (zlb == 0 && zhb == 0)
	{
		for (QList<NeuronSWC>::iterator it = inputTreePtr->listNeuron.begin(); it != inputTreePtr->listNeuron.end(); ++it)
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
				outputTreePtr->listNeuron.push_back(newNode);
			}
		}
	}
}

#endif 