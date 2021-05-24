#ifndef NEURONRECONERRORTYPES_H
#define NEURONRECONERRORTYPES_H

#include "integratedDataTypes.h"

using namespace integratedDataTypes;

namespace neuronReconErrorTypes
{
	enum errorID { ghost, selfLooping };

	class errorStructure
	{
		// This is an interface class [segUnit]-based error objects defined in neuronReconErrorTypes.h. 
	public:
		virtual ~errorStructure() {}

		virtual errorID getErrorID() = 0;

		virtual QList<NeuronSWC>& getNodes() = 0;

		virtual void highlightErrorNodes() = 0;
		virtual void selfCorrect() = 0;
	};

	class ghostSegUnit : public errorStructure
	{
	public:
		ghostSegUnit() = delete;
		ghostSegUnit(const QList<NeuronSWC>& inputNodes) : theSeg(inputNodes) {}
		ghostSegUnit& operator=(const ghostSegUnit&) = delete;
		
		virtual errorID getErrorID() { return ghost; }

		segUnit theSeg;
		virtual QList<NeuronSWC>& getNodes() { return this->theSeg.nodes; }

		virtual void highlightErrorNodes();
		virtual void selfCorrect();
	};

	class selfLoopingSegUnit : public errorStructure
	{
	public:
		selfLoopingSegUnit() = delete;
		selfLoopingSegUnit(const QList<NeuronSWC>& inputNodes) : theSeg(inputNodes) {}
		selfLoopingSegUnit& operator=(const selfLoopingSegUnit&) = delete;

		virtual errorID getErrorID() { return selfLooping; }

		segUnit theSeg;
		virtual QList<NeuronSWC>& getNodes() { return this->theSeg.nodes; }

		virtual void highlightErrorNodes();
		virtual void selfCorrect();
	};
}

#endif