#ifndef DARK_PRUNING_H
#define DARK_PRUNING_H



#include <vector>
#include "basic_surf_objs.h"
#include "basic_4dimage.h"

using namespace std;


bool dark_pruning(NeuronTree input_tree,  QList<NeuronSWC> &output_tree, Image4DSimple * image, int visible_threshold);





#endif // DARK_PRUNING_H

