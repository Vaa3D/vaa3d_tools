#ifndef NEURONRECONERRORTYPES_H
#define NEURONRECONERRORTYPES_H

#include "integratedDataTypes.h"

using namespace integratedDataTypes;

namespace neuronReconErrorTypes
{
	struct ghostSegUnit : public segUnit
	{
		ghostSegUnit(const QList<NeuronSWC>& inputNodeList) : segUnit(inputNodeList) { this->head = -1; }

		//virtual void retype2highlight();
	};
}

#endif