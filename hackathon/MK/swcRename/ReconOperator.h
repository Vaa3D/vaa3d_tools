#ifndef RECONOPERATOR_H
#define RECONOPERATOR_H

#define __ACTIVATE_TESTER__

#include <iostream>

#include <qstring.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qfile.h>

#include "NeuronStructUtilities.h"
#include "NeuronStructExplorer.h"

const bool DUPSEG_REMOVE = false;
const bool TYPE1_REMOVE = false;
const bool GROUPED_TREES = false;
const bool GROUPED_ASSEMBLED_TREES = false;

class ReconOperator
{
public:
	ReconOperator() : autoConnect(false), removeSpike(false), branchNodeMin(-1), connectThreshold(-1) {}

	QString rootPath;
	int xFactor, yFactor, zFactor;

	bool autoConnect;
	bool removeSpike;
	int branchNodeMin;
	int connectThreshold;

	void downSampleReconFile(const QStringList& fileList, float xFactor, float yFactor, float zFactor);

	void denAxonSeparate(const QStringList& fileList);
	void denAxonCombine(bool dupRemove);

	void assembleSegs2tree();

	void markerApo2swc();

private:
	NeuronStructExplorer myNeuronStructExplorer;

	vector<shared_ptr<neuronReconErrorTypes::errorStructure>> errorList;

	void removeAdditionalType1Nodes(profiledTree& inputProfiledTree);
	void errorCheckRepair(profiledTree& inputProfiledTree);
	void assembleGroupedSegs(boost::container::flat_map<int, profiledTree>& connectedTrees, map<int, int>& tree2HeadNodeMap, const CellAPO& somaAPO);
	void connectType7trees2otherTree(boost::container::flat_map<int, profiledTree>& connectedTrees);
};

#endif