#ifndef NEURONRECONERRORTYPES_H
#define NEURONRECONERRORTYPES_H

#include "integratedDataTypes.h"

using namespace integratedDataTypes;

namespace neuronReconErrorTypes
{
	class errorStructure
	{
		// This is an interface class [segUnit]-based error objects defined in neuronReconErrorTypes.h. 
	public:
		virtual ~errorStructure() {}

		virtual QList<NeuronSWC>& getNodes() = 0;

		virtual void highlightErrorNodes() = 0;
	};

	class ghostSegUnit : public errorStructure
	{
	public:
		ghostSegUnit() = delete;
		ghostSegUnit(const QList<NeuronSWC>& inputNodes) : theSeg(inputNodes) {}
		ghostSegUnit& operator=(const ghostSegUnit&) = delete;
		
		segUnit theSeg;
		virtual QList<NeuronSWC>& getNodes() { return this->theSeg.nodes; }

		virtual void highlightErrorNodes();
	};
}

#endif