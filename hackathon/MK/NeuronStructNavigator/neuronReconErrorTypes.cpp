#include "neuronReconErrorTypes.h"

QList<NeuronSWC> neuronReconErrorTypes::ghostSegUnit::selfCorrect()
{
	QList<NeuronSWC> outputNodes;
	cout << "not implemented yet" << endl;

	return outputNodes;
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

neuronReconErrorTypes::conjoinedSegs::conjoinedSegs(const boost::container::flat_set<segUnit>& inputSegUnits)
{
	int maxSegID = 0;
	for (auto& seg : inputSegUnits) if (seg.segID > maxSegID) maxSegID = seg.segID;
	for (auto& seg : inputSegUnits)
	{
		if (seg.segID == 0) this->segMap.insert({ ++maxSegID, seg });
		else this->segMap.insert({ seg.segID, seg });
	}
}

QList<NeuronSWC>& neuronReconErrorTypes::conjoinedSegs::getNodes()
{
	this->totalNodes.clear();
	for (auto& seg : this->segMap) this->totalNodes.append(seg.second.nodes);

	return this->totalNodes;
}

QList<NeuronSWC> neuronReconErrorTypes::conjoinedSegs::selfCorrect()
{

}