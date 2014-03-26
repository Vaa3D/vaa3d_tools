#ifndef TUBEMODEL_H
#define TUBEMODEL_H

#include <vector>

#include "tz_neuron_structure.h"
#include "tz_locseg_chain.h"
#include "tz_geo3d_circle.h"
#include "tz_geo3d_ball.h"

namespace TubeModel
{
Swc_Tree_Node* createSwc(Locseg_Chain *chain, int type = 2);

//Neuron structure
void removeRedundantEdge(Neuron_Structure *ns);

Neuron_Structure *createNeuronStructure(
    const std::vector<Locseg_Chain*> &chainArray,
    Stack *signal,
    Connection_Test_Workspace *ctw);
}

#endif // TUBEMODEL_H
