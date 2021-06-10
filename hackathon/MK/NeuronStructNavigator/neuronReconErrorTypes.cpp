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
		for (auto& childLoc : this->theSeg.seg_childLocMap.at(this->theSeg.head)) outputNodes[childLoc].parent = -1;
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

QList<NeuronSWC> neuronReconErrorTypes::conjoinedSegs::selfCorrect()
{
	QList<NeuronSWC> outputNodes = this->theSeg.nodes;

	vector<int> conjoinedHeadNodeIDs, conjoinedTailNodeIDs;
	NeuronSWC currPaNode = this->theSeg.nodes.at(this->theSeg.seg_nodeLocMap.at(this->theSeg.head));
	while (1)
	{
		for (auto& involvedSeg : this->involvedSegUnits)
		{
			for (auto& node : involvedSeg.nodes)
			{
				if (currPaNode.x == node.x && currPaNode.y == node.y && currPaNode.z == node.z)
				{
					conjoinedHeadNodeIDs.push_back(currPaNode.n);
					currPaNode = this->theSeg.nodes.at(*this->theSeg.seg_childLocMap.at(currPaNode.n).begin());
					goto TO_THE_NEXT_ROUND_HEAD;
				}
			}
		}
		break;

	TO_THE_NEXT_ROUND_HEAD:
		continue;
	}

	NeuronSWC currChildNode = this->theSeg.nodes.at(this->theSeg.seg_nodeLocMap.at(*this->theSeg.tails.begin()));
	while (1)
	{
		for (auto& involvedSeg : this->involvedSegUnits)
		{
			for (auto& node : involvedSeg.nodes)
			{
				if (currChildNode.x == node.x && currChildNode.y == node.y && currChildNode.z == node.z)
				{
					conjoinedTailNodeIDs.push_back(currChildNode.n);
					currChildNode = this->theSeg.nodes.at(this->theSeg.seg_nodeLocMap.at(currChildNode.parent));
					goto TO_THE_NEXT_ROUND_TAIL;
				}
			}
		}
		break;

	TO_THE_NEXT_ROUND_TAIL:
		continue;
	}

	vector<ptrdiff_t> delLocs;
	if (!conjoinedHeadNodeIDs.empty())
	{
		outputNodes[this->theSeg.seg_nodeLocMap.at(*(conjoinedHeadNodeIDs.end() - 1))].parent = -1;
		for (vector<int>::iterator it = conjoinedHeadNodeIDs.begin(); it != conjoinedHeadNodeIDs.end() - 1; ++it) delLocs.push_back(this->theSeg.seg_nodeLocMap.at(*it));
	}
	
	if (!conjoinedTailNodeIDs.empty())
		for (vector<int>::iterator it = conjoinedTailNodeIDs.begin(); it != conjoinedTailNodeIDs.end() - 1; ++it) delLocs.push_back(this->theSeg.seg_nodeLocMap.at(*it));
	
	sort(delLocs.rbegin(), delLocs.rend());
	for (auto& loc : delLocs) outputNodes.erase(outputNodes.begin() + loc);

	return outputNodes;
}