#ifndef NEURONRECONERRORTYPES_H
#define NEURONRECONERRORTYPES_H

#include "integratedDataTypes.h"

using namespace integratedDataTypes;

namespace neuronReconErrorTypes
{
	enum errorID { ghost, selfLooping, conjoinedSeg };

	class errorStructure
	{
		// This is an interface class [segUnit]-based error objects defined in neuronReconErrorTypes.h. 
	public:
		virtual ~errorStructure() {}

		virtual errorID getErrorID() = 0;

		virtual QList<NeuronSWC>& getNodes() = 0;

		virtual void highlightErrorNodes() = 0;
		virtual QList<NeuronSWC> selfCorrect() = 0;
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

		virtual void highlightErrorNodes() { for (auto& node : this->theSeg.nodes) node.type = 0; }
		virtual QList<NeuronSWC> selfCorrect();
	};

	class selfLoopingSegUnit : public errorStructure
	{
	public:
		selfLoopingSegUnit() = delete;
		selfLoopingSegUnit(const segUnit& inputSegUnit) : theSeg(inputSegUnit) {}
		selfLoopingSegUnit(const QList<NeuronSWC>& inputNodes) : theSeg(inputNodes) {}
		selfLoopingSegUnit& operator=(const selfLoopingSegUnit&) = delete;

		virtual errorID getErrorID() { return selfLooping; }

		segUnit theSeg;
		virtual QList<NeuronSWC>& getNodes() { return this->theSeg.nodes; }

		virtual void highlightErrorNodes() { for (auto& node : this->theSeg.nodes) node.type = 5; }
		virtual QList<NeuronSWC> selfCorrect();
	};

	class conjoinedSegs : public errorStructure
	{
	public:
		conjoinedSegs() = default;
		conjoinedSegs(const boost::container::flat_set<segUnit>& inputSegUnits);
		conjoinedSegs(const QList<NeuronSWC>& inputNodes) {}
		conjoinedSegs& operator=(const conjoinedSegs&) = delete;

		virtual errorID getErrorID() { return conjoinedSeg; }

		map<int, segUnit> segMap;
		QList<NeuronSWC> totalNodes;
		virtual QList<NeuronSWC>& getNodes();

		virtual void highlightErrorNodes()
		{
			for (auto& seg : this->segMap)
				for (auto& node : seg.second.nodes) node.type = 6;
		}

		virtual QList<NeuronSWC> selfCorrect();
	};
}

#endif