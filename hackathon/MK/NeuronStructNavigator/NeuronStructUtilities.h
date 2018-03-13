#ifndef NEURONSTRUCTUTILITIES_H
#define NEURONSTRUCTUTILITIES_H

#include <vector>

#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>

#include "basic_surf_objs.h"

using namespace std;

// ------- swcSlicer disembles SWC into "SWC slices" based on z coordinates ------- //
void swcSlicer(NeuronTree* inputTreePtr, vector<NeuronTree>* outputTreesPtr, int thickness = 0);



#endif 