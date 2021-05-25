#include "neuronReconErrorTypes.h"

void neuronReconErrorTypes::ghostSegUnit::highlightErrorNodes()
{
	for (auto& node : this->theSeg.nodes) node.type = 0;
}

QList<NeuronSWC> neuronReconErrorTypes::ghostSegUnit::selfCorrect()
{
	QList<NeuronSWC> outputNodes;
	cout << "not implemented yet" << endl;

	return outputNodes;
}

void neuronReconErrorTypes::selfLoopingSegUnit::highlightErrorNodes()
{
	for (auto& node : this->theSeg.nodes) node.type = 5;
}

QList<NeuronSWC> neuronReconErrorTypes::selfLoopingSegUnit::selfCorrect()
{
	// There are 3 possible self-looping conditions:
	// -- a) Tail node overlapping other segment node
	//    b) Extended tail overlapping segment body (the last several nodes overlapping other segment nodes)
	//    c) Body nodes overlapping other segment nodes
	//
	// Currently this method is only dedicated to condition a). A generalized self correcting method will be implemented in the future.

	QList<NeuronSWC> outputNodes = this->theSeg.nodes;
	
	vector<ptrdiff_t> delLocs;
	for (auto& tailID : this->theSeg.tails) delLocs.push_back(this->theSeg.seg_nodeLocMap.at(tailID));
	sort(delLocs.rbegin(), delLocs.rend());
	for (auto& delLoc : delLocs) outputNodes.erase(outputNodes.begin() + delLoc);

	return outputNodes;
}