#include "TreeTrimmer.h"
#include "NeuronStructNavigatingTester.h"

//map<int, profiledTree> TreeTrimmer::trimmedTree_segEndClusterBased(const profiledTree& inputProfiledTree, const map<int, ImageMarker>& axonGrowingPoints)
//{


//}



/* ============================= Minor Tree Trimming / Refining ============================= */
NeuronTree TreeTrimmer::branchBreak(const profiledTree& inputProfiledTree, double spikeThre, bool spikeRemove)
{
	profiledTree outputProfiledTree(inputProfiledTree.tree);

	vector<size_t> spikeLocs;
	if (spikeRemove)
	{
		for (QList<NeuronSWC>::iterator it = outputProfiledTree.tree.listNeuron.begin(); it != outputProfiledTree.tree.listNeuron.end(); ++it)
		{
			if (outputProfiledTree.node2childLocMap.find(it->n) != outputProfiledTree.node2childLocMap.end())
			{
				vector<size_t> childLocs = outputProfiledTree.node2childLocMap.at(it->n);
				if (childLocs.size() >= 2)
				{
					int nodeRemoveCount = 0;
					for (vector<size_t>::iterator locIt = childLocs.begin(); locIt != childLocs.end(); ++locIt)
					{
						if (outputProfiledTree.node2childLocMap.find(outputProfiledTree.tree.listNeuron.at(*locIt).n) == outputProfiledTree.node2childLocMap.end())
						{
							double spikeDist = sqrt((outputProfiledTree.tree.listNeuron.at(*locIt).x - it->x) * (outputProfiledTree.tree.listNeuron.at(*locIt).x - it->x) +
								(outputProfiledTree.tree.listNeuron.at(*locIt).y - it->y) * (outputProfiledTree.tree.listNeuron.at(*locIt).y - it->y) +
								(outputProfiledTree.tree.listNeuron.at(*locIt).z - it->z) * (outputProfiledTree.tree.listNeuron.at(*locIt).z - it->z) * zRATIO * zRATIO);
							if (spikeDist <= spikeThre) // Take out splikes.
							{
								spikeLocs.push_back(*locIt);
								++nodeRemoveCount;
							}
						}
					}

					if (nodeRemoveCount == childLocs.size() - 1) continue; // If there is only 1 child left after taking out all spikes, then this node is supposed to be on the main route.
					else
					{
						for (vector<size_t>::iterator locCheckIt = childLocs.begin(); locCheckIt != childLocs.end(); ++locCheckIt)
						{
							if (find(spikeLocs.begin(), spikeLocs.end(), *locCheckIt) == spikeLocs.end())
								outputProfiledTree.tree.listNeuron[*locCheckIt].parent = -1; // 'at' operator treats the container as const, and cannot assign values. Therefore, use [] instead.
						}
					}
				}
			}
		}
	}
	else
	{
		for (QList<NeuronSWC>::iterator it = outputProfiledTree.tree.listNeuron.begin(); it != outputProfiledTree.tree.listNeuron.end(); ++it)
		{
			if (outputProfiledTree.node2childLocMap.find(it->n) != outputProfiledTree.node2childLocMap.end())
			{
				if (outputProfiledTree.node2childLocMap.at(it->n).size() >= 2)
				{
					for (vector<size_t>::const_iterator locIt = outputProfiledTree.node2childLocMap.at(it->n).begin(); locIt != outputProfiledTree.node2childLocMap.at(it->n).end(); ++locIt)
						outputProfiledTree.tree.listNeuron[*locIt].parent = -1;
				}
			}
		}
	}

	if (spikeRemove) // Erase spike nodes from outputTree.listNeuron.
	{
		sort(spikeLocs.rbegin(), spikeLocs.rend());
		for (vector<size_t>::iterator delIt = spikeLocs.begin(); delIt != spikeLocs.end(); ++delIt)
			outputProfiledTree.tree.listNeuron.erase(outputProfiledTree.tree.listNeuron.begin() + ptrdiff_t(*delIt));
	}

	return outputProfiledTree.tree;
}

profiledTree TreeTrimmer::spikeRemoval(const profiledTree& inputProfiledTree, int spikeNodeNum)
{
	boost::container::flat_set<int> tempRootIds = inputProfiledTree.spikeRootIDs;
	profiledTree processingProfiledTree = inputProfiledTree;
	vector<size_t> tipLocs;
	for (QList<NeuronSWC>::iterator it = processingProfiledTree.tree.listNeuron.begin(); it != processingProfiledTree.tree.listNeuron.end(); ++it)
	{
		if (processingProfiledTree.node2childLocMap.find(it->n) == processingProfiledTree.node2childLocMap.end())
			tipLocs.push_back(it - processingProfiledTree.tree.listNeuron.begin());
	}

	vector<size_t> delLocs;
	vector<size_t> tipBranchNodeLocs;
	for (vector<size_t>::iterator tipLocIt = tipLocs.begin(); tipLocIt != tipLocs.end(); ++tipLocIt)
	{
		int nodeCount = 1;
		int currPaID = processingProfiledTree.tree.listNeuron.at(*tipLocIt).parent;
		tipBranchNodeLocs.push_back(*tipLocIt);
		while (processingProfiledTree.node2childLocMap.at(currPaID).size() == 1)
		{
			size_t currPaLoc = processingProfiledTree.node2LocMap.at(currPaID);
			tipBranchNodeLocs.push_back(currPaLoc);
			currPaID = processingProfiledTree.tree.listNeuron.at(currPaLoc).parent;
			++nodeCount;

			if (nodeCount > spikeNodeNum || currPaID == -1)
			{
				tipBranchNodeLocs.clear();
				break;
			}
		}

		if (!tipBranchNodeLocs.empty())
		{
			delLocs.insert(delLocs.end(), tipBranchNodeLocs.begin(), tipBranchNodeLocs.end());
			tipBranchNodeLocs.clear();
			tempRootIds.insert(currPaID);
		}
	}

	sort(delLocs.rbegin(), delLocs.rend());
	for (vector<size_t>::iterator delIt = delLocs.begin(); delIt != delLocs.end(); ++delIt)
		processingProfiledTree.tree.listNeuron.erase(processingProfiledTree.tree.listNeuron.begin() + ptrdiff_t(*delIt));
	profiledTreeReInit(processingProfiledTree);
	processingProfiledTree.spikeRootIDs.insert(tempRootIds.begin(), tempRootIds.end());

	return processingProfiledTree;
}

profiledTree TreeTrimmer::itered_spikeRemoval(const profiledTree& inputProfiledTree, int spikeNodeNum)
{
	cout << "removing spikes.." << endl << "  iteration 1 " << endl;
	int iterCount = 1;
	profiledTree originalProfiledTree = inputProfiledTree;
	profiledTree cleanedTree = TreeTrimmer::spikeRemoval(originalProfiledTree, spikeNodeNum);
	//cout << "    spike number: " << cleanedTree.spikeRootIDs.size() << endl;
	while (cleanedTree.tree.listNeuron.size() != originalProfiledTree.tree.listNeuron.size())
	{
		originalProfiledTree = cleanedTree;

		++iterCount;
		cout << "  iteration " << iterCount << " " << endl;
		cleanedTree = TreeTrimmer::spikeRemoval(originalProfiledTree, spikeNodeNum);
		//cout << "    spike number: " << cleanedTree.spikeRootIDs.size() << endl;
	}
	cout << endl;

	return cleanedTree;
}

profiledTree TreeTrimmer::removeHookingHeadTail(const profiledTree& inputProfiledTree, float radAngleThre)
{
	profiledTree outputProfiledTree = inputProfiledTree;
	vector<size_t> delLocs;

	for (map<int, segUnit>::iterator segIt = outputProfiledTree.segs.begin(); segIt != outputProfiledTree.segs.end(); ++segIt)
	{
		if (segIt->second.nodes.size() <= 3) continue;	// skip short segments
		if (segIt->second.seg_childLocMap.at(segIt->second.head).size() > 1) continue;
		int headChildID = segIt->second.nodes.at(*(segIt->second.seg_childLocMap.at(segIt->second.head).begin())).n;
		if (segIt->second.seg_childLocMap.at(headChildID).size() > 1) continue;

		NeuronSWC headPivotNode = segIt->second.nodes.at(segIt->second.seg_nodeLocMap.at(headChildID));
		NeuronSWC headNode = segIt->second.nodes.at(segIt->second.seg_nodeLocMap.at(segIt->second.head));
		int headPivotChildID = segIt->second.nodes.at(*(segIt->second.seg_childLocMap.at(headPivotNode.n).begin())).n;
		NeuronSWC headPivotChildNode = segIt->second.nodes.at(segIt->second.seg_nodeLocMap.at(headPivotChildID));
		vector<float> vec1(3);
		vector<float> vec2(3);
		vec1 = NeuronGeoGrapher::getVector_NeuronSWC<float>(headNode, headPivotNode);
		vec2 = NeuronGeoGrapher::getVector_NeuronSWC<float>(headPivotChildNode, headPivotNode);
		float headTurnAngle = NeuronGeoGrapher::getRadAngle(vec1, vec2);
		if (headTurnAngle < radAngleThre)
		{
			outputProfiledTree.tree.listNeuron[outputProfiledTree.node2LocMap.at(headPivotNode.n)].parent = -1;
			delLocs.push_back(outputProfiledTree.node2LocMap.at(headNode.n));
		}

		for (vector<int>::iterator tailIt = segIt->second.tails.begin(); tailIt != segIt->second.tails.end(); ++tailIt)
		{
			int tailID = *tailIt;
			int tailPivotID = segIt->second.nodes.at(segIt->second.seg_nodeLocMap.at(tailID)).parent;
			int tailPivotPaID = segIt->second.nodes.at(segIt->second.seg_nodeLocMap.at(tailPivotID)).parent;
			NeuronSWC tailNode = segIt->second.nodes.at(segIt->second.seg_nodeLocMap.at(tailID));
			NeuronSWC tailPivotNode = segIt->second.nodes.at(segIt->second.seg_nodeLocMap.at(tailPivotID));
			NeuronSWC tailPivotPaNode = segIt->second.nodes.at(segIt->second.seg_nodeLocMap.at(tailPivotPaID));
			vector<float> tailVec1(3);
			vector<float> tailVec2(3);
			tailVec1 = NeuronGeoGrapher::getVector_NeuronSWC<float>(tailNode, tailPivotNode);
			tailVec2 = NeuronGeoGrapher::getVector_NeuronSWC<float>(tailPivotPaNode, tailPivotNode);
			float tailTurnAngle = NeuronGeoGrapher::getRadAngle(tailVec1, tailVec2);
			if (tailTurnAngle < radAngleThre) delLocs.push_back(outputProfiledTree.node2LocMap.at(tailNode.n));
		}
	}

	sort(delLocs.rbegin(), delLocs.rend());
	for (vector<size_t>::iterator delIt = delLocs.begin(); delIt != delLocs.end(); ++delIt)
		outputProfiledTree.tree.listNeuron.erase(outputProfiledTree.tree.listNeuron.begin() + ptrdiff_t(*delIt));
	profiledTreeReInit(outputProfiledTree);

	return outputProfiledTree;
}

profiledTree TreeTrimmer::itered_removeHookingHeadTail(const profiledTree& inputProfiledTree, float radAngleThre)
{
	cout << "removing hooks.." << endl << "  iteration 1 " << endl;
	int iterCount = 1;
	profiledTree originalProfiledTree = inputProfiledTree;
	profiledTree hookRemovedTree = TreeTrimmer::removeHookingHeadTail(originalProfiledTree, radAngleThre);
	while (hookRemovedTree.tree.listNeuron.size() != originalProfiledTree.tree.listNeuron.size())
	{
		originalProfiledTree = hookRemovedTree;

		++iterCount;
		cout << "  iteration " << iterCount << " " << endl;
		hookRemovedTree = TreeTrimmer::removeHookingHeadTail(originalProfiledTree, radAngleThre);
	}
	cout << endl;

	return hookRemovedTree;
}

profiledTree TreeTrimmer::segSharpAngleSmooth_lengthDistRatio(const profiledTree& inputProfiledTree, const double ratio) const
{
	profiledTree outputProfiledTree = inputProfiledTree;
	boost::container::flat_set<int> smoothedNodeIDs;
	outputProfiledTree.smoothedNodeIDs.clear();
	for (map<int, segUnit>::iterator segIt = outputProfiledTree.segs.begin(); segIt != outputProfiledTree.segs.end(); ++segIt)
	{
		if (segIt->second.segSmoothnessMap.find(3) == segIt->second.segSmoothnessMap.end()) continue;

		for (boost::container::flat_map<int, map<string, double>>::iterator nodeIt = segIt->second.segSmoothnessMap.at(3).begin(); nodeIt != segIt->second.segSmoothnessMap.at(3).end(); ++nodeIt)
		{
			if (nodeIt->second.at("length") / nodeIt->second.at("distance") >= ratio)
			{
				NeuronSWC currPaNode = segIt->second.nodes.at(segIt->second.seg_nodeLocMap.at(segIt->second.nodes.at(segIt->second.seg_nodeLocMap.at(nodeIt->first)).parent));
				NeuronSWC currChildNode = segIt->second.nodes.at(*segIt->second.seg_childLocMap.at(nodeIt->first).begin());
				outputProfiledTree.tree.listNeuron[outputProfiledTree.node2LocMap.at(nodeIt->first)].x = (currPaNode.x + currChildNode.x) / 2;
				outputProfiledTree.tree.listNeuron[outputProfiledTree.node2LocMap.at(nodeIt->first)].y = (currPaNode.y + currChildNode.y) / 2;
				outputProfiledTree.tree.listNeuron[outputProfiledTree.node2LocMap.at(nodeIt->first)].z = (currPaNode.z + currChildNode.z) / 2;
				smoothedNodeIDs.insert(nodeIt->first);
			}
		}
	}

	profiledTreeReInit(outputProfiledTree);
	outputProfiledTree.smoothedNodeIDs = smoothedNodeIDs;
	return outputProfiledTree;
}

profiledTree TreeTrimmer::itered_segSharpAngleSmooth_lengthDistRatio(const profiledTree& inputProfiledTree, double ratio) const
{
	cout << "smoothing angles.." << endl << "  iteration 1 " << endl;
	int iterCount = 1;

	profiledTree originalProfiledTree = inputProfiledTree;
	profiledTree angleSmoothedTree = TreeTrimmer::segSharpAngleSmooth_lengthDistRatio(originalProfiledTree, ratio);
	while (angleSmoothedTree.smoothedNodeIDs.size() > 0)
	{
		NeuronStructExplorer::segMorphProfile(angleSmoothedTree);
		originalProfiledTree = angleSmoothedTree;

		++iterCount;
		cout << "  iteration " << iterCount << " " << endl;
		angleSmoothedTree = TreeTrimmer::segSharpAngleSmooth_lengthDistRatio(originalProfiledTree, ratio);
	}
	cout << endl;

	return angleSmoothedTree;
}

profiledTree TreeTrimmer::segSharpAngleSmooth_distThre_3nodes(const profiledTree& inputProfiledTree, const double distThre) const
{
	profiledTree outputProfiledTree = inputProfiledTree;
	for (map<int, segUnit>::iterator segIt = outputProfiledTree.segs.begin(); segIt != outputProfiledTree.segs.end(); ++segIt)
	{
		if (segIt->second.segSmoothnessMap.find(3) == segIt->second.segSmoothnessMap.end()) continue;

		for (boost::container::flat_map<int, map<string, double>>::iterator nodeIt = segIt->second.segSmoothnessMap.at(3).begin(); nodeIt != segIt->second.segSmoothnessMap.at(3).end(); ++nodeIt)
		{
			if (nodeIt->second.at("distance") > distThre)
			{
				NeuronSWC currPaNode = segIt->second.nodes.at(segIt->second.seg_nodeLocMap.at(segIt->second.nodes.at(segIt->second.seg_nodeLocMap.at(nodeIt->first)).parent));
				NeuronSWC currChildNode = segIt->second.nodes.at(int(*segIt->second.seg_childLocMap.at(nodeIt->first).begin()));
				NeuronSWC centerNode = segIt->second.nodes.at(int(segIt->second.seg_nodeLocMap.at(nodeIt->first)));
				if (abs(currPaNode.x - centerNode.x) + abs(currPaNode.y - centerNode.y) + abs(currPaNode.z - centerNode.z) >
					abs(currChildNode.x - centerNode.x) + abs(currChildNode.y - centerNode.y) + abs(currChildNode.z - centerNode.z))
					outputProfiledTree.tree.listNeuron[int(outputProfiledTree.node2LocMap.at(nodeIt->first))].parent = -1;
				else outputProfiledTree.tree.listNeuron[int(outputProfiledTree.node2LocMap.at(currChildNode.n))].parent = -1;
			}
		}
	}

	profiledTreeReInit(outputProfiledTree);
	return outputProfiledTree;
}
/* ======================== END of [Minor Tree Trimming / Refining] ========================= */