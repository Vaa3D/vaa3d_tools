#ifndef NEURONSTRUCTUTILITIES_H
#define NEURONSTRUCTUTILITIES_H

#include <vector>

#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>

#include "basic_surf_objs.h"

using namespace std;

// ------- swcSlicer dissembles SWC into "SWC slices" based on z coordinates ------- //
void swcSlicer(NeuronTree* inputTreePtr, vector<NeuronTree>* outputTreesPtr, int thickness = 0);
void swcSliceAssembler(string swcPath);
void swcCrop(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr, float xlb, float xhb, float ylb, float yhb, float zlb, float zhb);

void sigNode_Gen(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr, float ratio, float distance);

void bkgNode_Gen(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr, long int dims[], float ratio, float distance);
void bkgNode_Gen_somaArea(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr, int xLength, int yLength, int zLength, float ratio, float distance);

#endif 