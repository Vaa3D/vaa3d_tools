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
	static void swcCrop(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr, float xlb, float xhb, float ylb, float yhb, float zlb, float zhb);
	static void swcFlipY(NeuronTree const* inputTreePtr, NeuronTree*& outputTreePtr, long int yLength);

	static void swcDownSampleZ(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr, int factor);
	static void detectedSWCprobFilter(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr, float threshold);
	/*******************************************************/

	/********* Simulated volumetric patch generation *********/
	static void sigNode_Gen(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr, float ratio, float distance);
	static void bkgNode_Gen(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr, long int dims[], float ratio, float distance);
	static void bkgNode_Gen_somaArea(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr, int xLength, int yLength, int zLength, float ratio, float distance);
	/*********************************************************/

};

#endif 