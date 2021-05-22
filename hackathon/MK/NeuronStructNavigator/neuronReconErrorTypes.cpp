#include "neuronReconErrorTypes.h"

void neuronReconErrorTypes::ghostSegUnit::highlightErrorNodes()
{
	for (auto& node : this->theSeg.nodes) node.type = 0;
}