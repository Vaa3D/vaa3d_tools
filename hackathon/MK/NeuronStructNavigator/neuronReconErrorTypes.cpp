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
	// -- a) End node overlapping other segment node
	//    b) Extended segment end overlapping segment body (the last several nodes overlapping other segment nodes)
	//    c) Body nodes overlapping other segment nodes
	//
	// Currently this method is only dedicated to condition a). A generalized self correcting method will be implemented in the future.

	QList<NeuronSWC> outputNodes = this->theSeg.nodes;
	
	vector<ptrdiff_t> delLocs;
	if (this->headLoop)
	{
		delLocs.push_back(this->theSeg.seg_nodeLocMap.at(this->theSeg.head));
		for (auto& childLoc : this->theSeg.seg_childLocMap.at(this->theSeg.head)) this->theSeg.nodes[childLoc].parent = -1;
	}
	else
		for (auto& tailID : this->theSeg.tails) delLocs.push_back(this->theSeg.seg_nodeLocMap.at(tailID));
	
	sort(delLocs.rbegin(), delLocs.rend());
	for (auto& delLoc : delLocs) outputNodes.erase(outputNodes.begin() + delLoc);

	return outputNodes;
}

QList<NeuronSWC> neuronReconErrorTypes::hairpinSegUnit::selfCorrect()
{
	QList<NeuronSWC> outputNodes;
	int actualSize;
	if (this->theSeg.nodes.size() % 2 == 0) actualSize = this->theSeg.nodes.size() / 2;
	else actualSize = this->theSeg.nodes.size() / 2 + 1;

	int count = 1;
	int currentNodeID = this->theSeg.head;
	outputNodes.append(this->theSeg.nodes.at(this->theSeg.seg_nodeLocMap.at(currentNodeID)));
	while (count <= actualSize)
	{
		const NeuronSWC& childNode = this->theSeg.nodes.at(*this->theSeg.seg_childLocMap.at(currentNodeID).begin());
		outputNodes.append(childNode);
		currentNodeID = childNode.n;
		++count;
	}

	return outputNodes;
}

neuronReconErrorTypes::compositeShadowSegs::compositeShadowSegs(const boost::container::flat_set<segUnit>& inputSegUnits) : effectiveHeadCoordPtr(nullptr)
{
	int maxSegID = 0;
	for (auto& seg : inputSegUnits) if (seg.segID > maxSegID) maxSegID = seg.segID;
	for (auto& seg : inputSegUnits)
	{
		if (seg.segID == 0) this->segMap.insert({ ++maxSegID, seg });
		else this->segMap.insert({ seg.segID, seg });
	}

	for (auto& seg : inputSegUnits)
	{
		const NeuronSWC& headNode = seg.nodes.at(seg.seg_nodeLocMap.at(seg.head));
		coordUnit* segHeadCoordPtr = new coordUnit(headNode);
		coordUnit* currCoordPtr = segHeadCoordPtr;
		int currNodeID = headNode.n;
		while (seg.seg_childLocMap.find(currNodeID) != seg.seg_childLocMap.end())
		{
			const NeuronSWC& childNode = seg.nodes.at(*seg.seg_childLocMap.at(currNodeID).begin());
			if (*currCoordPtr == childNode)
			{
				currNodeID = childNode.n;
				continue;
			}

			coordUnit* childCoordPtr = new coordUnit(seg.nodes.at(*seg.seg_childLocMap.at(currNodeID).begin()));
			currCoordPtr->childCoordPtrs.insert(childCoordPtr);
			childCoordPtr->parentCoordPtr = currCoordPtr;
			currCoordPtr = childCoordPtr;
			currNodeID = childCoordPtr->nodeID;
		}
		this->segCoordListPtrs.push_back(segHeadCoordPtr);
	}


}

neuronReconErrorTypes::compositeShadowSegs::~compositeShadowSegs()
{
	//if ()
}

coordUnit* neuronReconErrorTypes::compositeShadowSegs::getLastCoordPtr()
{
	if (this->effectiveHeadCoordPtr == nullptr) return nullptr;

	coordUnit* currCoordUnitPtr = this->effectiveHeadCoordPtr;
	while (!currCoordUnitPtr->childCoordPtrs.empty()) currCoordUnitPtr = *currCoordUnitPtr->childCoordPtrs.begin();
	
	return currCoordUnitPtr;
}

int neuronReconErrorTypes::compositeShadowSegs::getCoordListLength()
{
	if (this->effectiveHeadCoordPtr == nullptr) return 0;
	
	coordUnit* currCoordUnitPtr = this->effectiveHeadCoordPtr;
	int count = 1;
	while (!currCoordUnitPtr->childCoordPtrs.empty())
	{
		currCoordUnitPtr = *currCoordUnitPtr->childCoordPtrs.begin();
		++count;
	}

	return count;
}

QList<NeuronSWC>& neuronReconErrorTypes::compositeShadowSegs::getNodes()
{
	this->totalNodes.clear();
	for (auto& seg : this->segMap) this->totalNodes.append(seg.second.nodes);

	return this->totalNodes;
}

QList<NeuronSWC> neuronReconErrorTypes::compositeShadowSegs::selfCorrect()
{
	QList<NeuronSWC> outputNodes;

	return outputNodes;
}

void neuronReconErrorTypes::compositeShadowSegs::getEffectiveCoordList()
{
	if (this->segCoordListPtrs.empty()) return;

	coordUnit* effectiveHead = *this->segCoordListPtrs.begin();
	for (vector<coordUnit*>::const_iterator it = this->segCoordListPtrs.begin() + 1; it != this->segCoordListPtrs.end(); ++it)
	{
		bool headIn = false, tailIn = false;
		coordUnit* nextCoordPtr = effectiveHead;
		
		do
		{
			if (*it == nextCoordPtr)
			{
				headIn = true;
				break;
			}
		} while (!headIn);


	}
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
	QList<NeuronSWC> outputNodes;

	return outputNodes;
}