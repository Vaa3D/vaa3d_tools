//------------------------------------------------------------------------------
// Copyright (c) 2021 Hsien-Chi Kuo (Allen Institute)
// All rights reserved.
//------------------------------------------------------------------------------

/*******************************************************************************
*
*  This namespace [neuronReconErrorTypes] is for segment(s) structural error handling purposes.
*  It consists of the base interface class [neuronReconErrorTypes::errorStructure] and a number of derived classes representing different types of structural errors.
*  The polymorphic design here is intended to allow higher level program to highlight erroneous part of the neuron structure in a general way.
*
********************************************************************************/

#ifndef NEURONRECONERRORTYPES_H
#define NEURONRECONERRORTYPES_H

#include "integratedDataTypes.h"

using namespace integratedDataTypes;

namespace neuronReconErrorTypes
{
	enum errorID { ghost, selfLooping, hairpin, conjoined };

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
		selfLoopingSegUnit(const segUnit& inputSegUnit) : theSeg(inputSegUnit), headLoop(false) {}
		selfLoopingSegUnit(const QList<NeuronSWC>& inputNodes) : theSeg(inputNodes), headLoop(false) {}
		selfLoopingSegUnit& operator=(const selfLoopingSegUnit&) = delete;

		bool headLoop;
		virtual errorID getErrorID() { return selfLooping; }

		segUnit theSeg;
		virtual QList<NeuronSWC>& getNodes() { return this->theSeg.nodes; }

		virtual void highlightErrorNodes() { for (auto& node : this->theSeg.nodes) node.type = 5; }
		virtual QList<NeuronSWC> selfCorrect();
	};

	class hairpinSegUnit : public errorStructure
	{
	public:
		hairpinSegUnit() = delete;
		hairpinSegUnit(const segUnit& inputSegUnit) : theSeg(inputSegUnit) {}
		hairpinSegUnit(const QList<NeuronSWC>& inputNodes) : theSeg(inputNodes) {}
		hairpinSegUnit& operator=(const selfLoopingSegUnit&) = delete;

		virtual errorID getErrorID() { return hairpin; }

		segUnit theSeg;
		virtual QList<NeuronSWC>& getNodes() { return this->theSeg.nodes; }

		virtual void highlightErrorNodes() { for (auto& node : this->theSeg.nodes) node.type = 8; }
		virtual QList<NeuronSWC> selfCorrect();
	};

	class conjoinedSegs : public errorStructure
	{
	public:
		conjoinedSegs() = delete;
		conjoinedSegs(const segUnit& errorSegUnit, const set<segUnit>& involvedSegUnits) : theSeg(errorSegUnit), involvedSegUnits(involvedSegUnits) {}
		conjoinedSegs(const QList<NeuronSWC>& inputNodes) {}
		conjoinedSegs& operator=(const conjoinedSegs&) = delete;

		virtual errorID getErrorID() { return conjoined; }

		segUnit theSeg;
		set<segUnit> involvedSegUnits;
		virtual QList<NeuronSWC>& getNodes() { return this->theSeg.nodes; }

		virtual void highlightErrorNodes() { for (auto& node : this->theSeg.nodes) node.type = 6; }

		virtual QList<NeuronSWC> selfCorrect();
	};
}

#endif