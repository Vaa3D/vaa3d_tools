#include <iostream>

#include "NeuronStructUtilities.h"
#include "TreeGrower.h"

using namespace std;
using namespace integratedDataTypes;

boost::container::flat_map<double, NeuronTree> TreeGrower::radiusShellNeuronTreeMap(const boost::container::flat_map<double, boost::container::flat_set<int>>& inputRadiusMap, const vector<polarNeuronSWC>& inputPolarNodeList)
{
	boost::container::flat_map<double, NeuronTree> outputRadius2NeuronTreeMap;
	for (boost::container::flat_map<double, boost::container::flat_set<int>>::const_iterator it = inputRadiusMap.begin(); it != inputRadiusMap.end(); ++it)
	{
		NeuronTree currShellTree;
		for (boost::container::flat_set<int>::const_iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			NeuronSWC newNode = NeuronGeoGrapher::polar2CartesianNode(inputPolarNodeList.at(*it2));
			currShellTree.listNeuron.push_back(newNode);
		}

		outputRadius2NeuronTreeMap.insert(pair<double, NeuronTree>(it->first, currShellTree));
	}

	return outputRadius2NeuronTreeMap;
}

boost::container::flat_map<double, vector<connectedComponent>> TreeGrower::shell2radiusConnMap(const boost::container::flat_map<double, NeuronTree> inputRadius2NeuronTreeMap)
{
	boost::container::flat_map<double, vector<connectedComponent>> outputShell2radiusConnMap;

	for (boost::container::flat_map<double, NeuronTree>::const_iterator it = inputRadius2NeuronTreeMap.begin(); it != inputRadius2NeuronTreeMap.end(); ++it)
	{
		vector<connectedComponent> currConnCompList = NeuronStructUtil::swc2signal3DBlobs(it->second);
		outputShell2radiusConnMap.insert(pair<double, vector<connectedComponent>>(it->first, currConnCompList));
	}

	return outputShell2radiusConnMap;
}

NeuronTree TreeGrower::SWC2MSTtree_boost(const NeuronTree& inputTree)
{
	NeuronTree MSTtrees;
	undirectedGraph graph(inputTree.listNeuron.size());
	//cout << "processing nodes: \n -- " << endl;
	for (int i = 0; i < inputTree.listNeuron.size(); ++i)
	{

		float x1, y1, z1;
		x1 = inputTree.listNeuron.at(i).x;
		y1 = inputTree.listNeuron.at(i).y;
		z1 = inputTree.listNeuron.at(i).z;
		for (int j = 0; j < inputTree.listNeuron.size(); ++j)
		{
			float x2, y2, z2;
			x2 = inputTree.listNeuron.at(j).x;
			y2 = inputTree.listNeuron.at(j).y;
			z2 = inputTree.listNeuron.at(j).z;

			double Vedge = sqrt(double(x1 - x2) * double(x1 - x2) + double(y1 - y2) * double(y1 - y2) + zRATIO * zRATIO * double(z1 - z2) * double(z1 - z2));
			pair<undirectedGraph::edge_descriptor, bool> edgeQuery = boost::edge(i, j, graph);
			if (!edgeQuery.second && i != j) boost::add_edge(i, j, lastVoted(i, weights(Vedge)), graph);
		}

		//if (i % 1000 == 0) cout << i << " ";
	}
	//cout << endl;

	vector <boost::graph_traits<undirectedGraph>::vertex_descriptor > p(num_vertices(graph));
	boost::prim_minimum_spanning_tree(graph, &p[0]);
	NeuronTree MSTtree;
	QList<NeuronSWC> listNeuron;
	QHash<int, int>  hashNeuron;
	listNeuron.clear();
	hashNeuron.clear();

	for (size_t ii = 0; ii != p.size(); ++ii)
	{
		int pn;
		if (p[ii] == ii) pn = -1;
		else pn = p[ii] + 1;

		NeuronSWC S;
		S.n = ii + 1;
		S.type = 7;
		S.x = inputTree.listNeuron.at(ii).x;
		S.y = inputTree.listNeuron.at(ii).y;
		S.z = inputTree.listNeuron.at(ii).z;
		S.r = 1;
		S.pn = pn;
		listNeuron.append(S);
		hashNeuron.insert(S.n, listNeuron.size() - 1);
	}
	MSTtree.listNeuron = listNeuron;
	MSTtree.hashNeuron = hashNeuron;

	return MSTtree;
}

NeuronTree TreeGrower::SWC2MSTtreeTiled_boost(NeuronTree const& inputTree, float tileLength, float zDivideNum)
{
	map<string, QList<NeuronSWC>> tiledSWCmap;

	QList<NeuronSWC> tileSWCList;
	tileSWCList.clear();
	for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		int tileXlabel = int(floor(it->x / tileLength));
		int tileYlabel = int(floor(it->y / tileLength));
		int tileZlabel = int(floor(it->z / ((tileLength / zRATIO) / zDivideNum)));
		string swcTileKey = to_string(tileXlabel) + "_" + to_string(tileYlabel) + "_" + to_string(tileZlabel);
		tiledSWCmap.insert(pair<string, QList<NeuronSWC>>(swcTileKey, tileSWCList));
		tiledSWCmap[swcTileKey].push_back(*it);
	}
	cout << "tiledSWCmap size = " << tiledSWCmap.size() << endl;

	NeuronTree assembledTree;
	for (map<string, QList<NeuronSWC>>::iterator it = tiledSWCmap.begin(); it != tiledSWCmap.end(); ++it)
	{
		NeuronTree tileTree;
		tileTree.listNeuron = it->second;
		NeuronTree tileMSTtree = TreeGrower::SWC2MSTtree_boost(tileTree);

		int currnodeNum = assembledTree.listNeuron.size();
		for (QList<NeuronSWC>::iterator nodeIt = tileMSTtree.listNeuron.begin(); nodeIt != tileMSTtree.listNeuron.end(); ++nodeIt)
		{
			nodeIt->n = nodeIt->n + currnodeNum;
			if (nodeIt->parent != -1)
			{
				nodeIt->parent = nodeIt->parent + currnodeNum;
				//cout << "  " << nodeIt->parent << " " << currnodeNum << endl;
			}

			//cout << nodeIt->n << " " << nodeIt->parent << endl;
			assembledTree.listNeuron.push_back(*nodeIt);
		}
	}

	return assembledTree;
}

NeuronTree TreeGrower::MSTbranchBreak(const profiledTree& inputProfiledTree, double spikeThre, bool spikeRemove)
{
	// -- This method breaks all branching points of a MST-connected tree.
	// -- When the value of spikeRemove is true, it eliminates spikes on a main route without breaking it. The default value is true. 

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

void TreeGrower::downstream_subTreeExtract(const QList<NeuronSWC>& inputList, QList<NeuronSWC>& subTreeList, const NeuronSWC& startingNode, map<int, size_t>& node2locMap, map<int, vector<size_t>>& node2childLocMap)
{
	NeuronStructUtil::node2loc_node2childLocMap(inputList, node2locMap, node2childLocMap);

	QList<NeuronSWC> parents;
	QList<NeuronSWC> children;
	parents.push_back(startingNode);
	vector<size_t> childLocs;
	do
	{
		children.clear();
		childLocs.clear();
		for (QList<NeuronSWC>::iterator pasIt = parents.begin(); pasIt != parents.end(); ++pasIt)
		{
			if (node2childLocMap.find(pasIt->n) != node2childLocMap.end()) childLocs = node2childLocMap.at(pasIt->n);
			else continue;

			for (vector<size_t>::iterator childLocIt = childLocs.begin(); childLocIt != childLocs.end(); ++childLocIt)
			{
				subTreeList.append(inputList.at(int(*childLocIt)));
				children.push_back(inputList.at(int(*childLocIt)));
			}
		}
		parents = children;
	} while (childLocs.size() > 0);

	subTreeList.push_front(startingNode);

	return;
}

void TreeGrower::wholeSingleTree_extract(const QList<NeuronSWC>& inputList, QList<NeuronSWC>& tracedList, const NeuronSWC& startingNode)
{
	map<int, size_t> node2locMap;
	map<int, vector<size_t>> node2childLocMap;
	NeuronStructUtil::node2loc_node2childLocMap(inputList, node2locMap, node2childLocMap);

	if (startingNode.parent == -1) TreeGrower::downstream_subTreeExtract(inputList, tracedList, startingNode, node2locMap, node2childLocMap);
	else
	{
		int parentID = startingNode.parent;
		int somaNodeID = inputList.at(int(node2locMap.at(parentID))).n;

		while (1)
		{
			parentID = inputList.at(int(node2locMap.at(parentID))).parent;
			if (parentID != -1) somaNodeID = inputList.at(int(node2locMap.at(parentID))).n;
			else break;
		}

		NeuronSWC rootNode = inputList.at(int(node2locMap.at(somaNodeID)));
		TreeGrower::downstream_subTreeExtract(inputList, tracedList, rootNode, node2locMap, node2childLocMap);
	}
}

profiledTree TreeGrower::connectSegsWithinClusters(const profiledTree& inputProfiledTree, float distThreshold)
{
	profiledTree outputProfiledTree = inputProfiledTree;
	this->getSegHeadTailClusters(outputProfiledTree, distThreshold);
	this->getClusterSegPairs(outputProfiledTree);

	vector<int> newSegIDs;
	set<int> connectedSegIDs;
	for (boost::container::flat_map<int, vector<segPairProfile>>::iterator it = outputProfiledTree.cluster2segPairMap.begin(); it != outputProfiledTree.cluster2segPairMap.end(); ++it)
	{
		if (it->second.size() == 1)
		{
			if (connectedSegIDs.find(it->second.begin()->seg1Ptr->segID) != connectedSegIDs.end() || connectedSegIDs.find(it->second.begin()->seg2Ptr->segID) != connectedSegIDs.end()) continue;

			if (it->second.begin()->currConnOrt == head_head)
			{
				NeuronSWC seg1Head, seg1Tail, seg2Head, seg2Tail;
				int seg1HeadID, seg1TailID, seg2HeadID, seg2TailID;
				seg1HeadID = it->second.begin()->seg1Ptr->head;
				seg1TailID = *it->second.begin()->seg1Ptr->tails.begin();
				seg2HeadID = it->second.begin()->seg2Ptr->head;
				seg2TailID = *it->second.begin()->seg2Ptr->tails.begin();
				seg1Head = it->second.begin()->seg1Ptr->nodes[it->second.begin()->seg1Ptr->seg_nodeLocMap.at(seg1HeadID)];
				seg1Tail = it->second.begin()->seg1Ptr->nodes[it->second.begin()->seg1Ptr->seg_nodeLocMap.at(seg1TailID)];
				seg2Head = it->second.begin()->seg2Ptr->nodes[it->second.begin()->seg2Ptr->seg_nodeLocMap.at(seg2HeadID)];
				seg2Tail = it->second.begin()->seg2Ptr->nodes[it->second.begin()->seg2Ptr->seg_nodeLocMap.at(seg2TailID)];

				vector<pair<float, float>> axialVecPair = NeuronGeoGrapher::getVectorWithStartingLoc_btwn2nodes<float>(seg1Tail, seg1Head);
				vector<pair<float, float>> projectingVecPair = NeuronGeoGrapher::getVectorWithStartingLoc_btwn2nodes<float>(seg2Head, seg2Tail);
				vector<pair<float, float>> projectedVecPair = NeuronGeoGrapher::getProjectionVector(axialVecPair, projectingVecPair);
				float overlapCheck = (projectingVecPair.begin()->first - seg1Head.x) * (seg1Head.x - seg1Tail.x) +
					((projectingVecPair.begin() + 1)->first - seg1Head.y) * (seg1Head.y - seg1Tail.y) + ((projectingVecPair.begin() + 2)->first - seg1Head.z) * (seg1Head.z - seg1Tail.z);
				if (overlapCheck < 0) continue;

				segUnit newSeg = NeuronStructUtil::segUnitConnect_executer(*it->second.begin()->seg1Ptr, *it->second.begin()->seg2Ptr, head_head);
				outputProfiledTree.segs.at(it->second.begin()->seg1Ptr->segID).to_be_deleted = true;
				outputProfiledTree.segs.at(it->second.begin()->seg2Ptr->segID).to_be_deleted = true;
				outputProfiledTree.segs.insert({ outputProfiledTree.segs.size() + 1, newSeg });
				newSegIDs.push_back(outputProfiledTree.segs.size());
				connectedSegIDs.insert(it->second.begin()->seg1Ptr->segID);
				connectedSegIDs.insert(it->second.begin()->seg2Ptr->segID);
				//cout << "connecting segs " << it->second.begin()->seg1Ptr->segID << " and " << it->second.begin()->seg2Ptr->segID << endl;
			}
			else if (it->second.begin()->currConnOrt == head_tail)
			{
				NeuronSWC seg1Head, seg1Tail, seg2Head, seg2Tail;
				int seg1HeadID, seg1TailID, seg2HeadID, seg2TailID;
				seg1HeadID = it->second.begin()->seg1Ptr->head;
				seg1TailID = *it->second.begin()->seg1Ptr->tails.begin();
				seg2HeadID = it->second.begin()->seg2Ptr->head;
				seg2TailID = *it->second.begin()->seg2Ptr->tails.begin();
				seg1Head = it->second.begin()->seg1Ptr->nodes[it->second.begin()->seg1Ptr->seg_nodeLocMap.at(seg1HeadID)];
				seg1Tail = it->second.begin()->seg1Ptr->nodes[it->second.begin()->seg1Ptr->seg_nodeLocMap.at(seg1TailID)];
				seg2Head = it->second.begin()->seg2Ptr->nodes[it->second.begin()->seg2Ptr->seg_nodeLocMap.at(seg2HeadID)];
				seg2Tail = it->second.begin()->seg2Ptr->nodes[it->second.begin()->seg2Ptr->seg_nodeLocMap.at(seg2TailID)];

				vector<pair<float, float>> axialVecPair = NeuronGeoGrapher::getVectorWithStartingLoc_btwn2nodes<float>(seg1Tail, seg1Head);
				vector<pair<float, float>> projectingVecPair = NeuronGeoGrapher::getVectorWithStartingLoc_btwn2nodes<float>(seg2Tail, seg2Head);
				vector<pair<float, float>> projectedVecPair = NeuronGeoGrapher::getProjectionVector(axialVecPair, projectingVecPair);
				float overlapCheck = (projectingVecPair.begin()->first - seg1Head.x) * (seg1Head.x - seg1Tail.x) +
					((projectingVecPair.begin() + 1)->first - seg1Head.y) * (seg1Head.y - seg1Tail.y) + ((projectingVecPair.begin() + 2)->first - seg1Head.z) * (seg1Head.z - seg1Tail.z);
				if (overlapCheck < 0) continue;

				segUnit newSeg = NeuronStructUtil::segUnitConnect_executer(*it->second.begin()->seg1Ptr, *it->second.begin()->seg2Ptr, head_tail);
				outputProfiledTree.segs.at(it->second.begin()->seg1Ptr->segID).to_be_deleted = true;
				outputProfiledTree.segs.at(it->second.begin()->seg2Ptr->segID).to_be_deleted = true;
				outputProfiledTree.segs.insert({ outputProfiledTree.segs.size() + 1, newSeg });
				newSegIDs.push_back(outputProfiledTree.segs.size());
				connectedSegIDs.insert(it->second.begin()->seg1Ptr->segID);
				connectedSegIDs.insert(it->second.begin()->seg2Ptr->segID);
				//cout << "connecting segs " << it->second.begin()->seg1Ptr->segID << " and " << it->second.begin()->seg2Ptr->segID << endl;
			}
			else if (it->second.begin()->currConnOrt == tail_tail)
			{
				NeuronSWC seg1Head, seg1Tail, seg2Head, seg2Tail;
				int seg1HeadID, seg1TailID, seg2HeadID, seg2TailID;
				seg1HeadID = it->second.begin()->seg1Ptr->head;
				seg1TailID = *it->second.begin()->seg1Ptr->tails.begin();
				seg2HeadID = it->second.begin()->seg2Ptr->head;
				seg2TailID = *it->second.begin()->seg2Ptr->tails.begin();
				seg1Head = it->second.begin()->seg1Ptr->nodes[it->second.begin()->seg1Ptr->seg_nodeLocMap.at(seg1HeadID)];
				seg1Tail = it->second.begin()->seg1Ptr->nodes[it->second.begin()->seg1Ptr->seg_nodeLocMap.at(seg1TailID)];
				seg2Head = it->second.begin()->seg2Ptr->nodes[it->second.begin()->seg2Ptr->seg_nodeLocMap.at(seg2HeadID)];
				seg2Tail = it->second.begin()->seg2Ptr->nodes[it->second.begin()->seg2Ptr->seg_nodeLocMap.at(seg2TailID)];

				vector<pair<float, float>> axialVecPair = NeuronGeoGrapher::getVectorWithStartingLoc_btwn2nodes<float>(seg1Head, seg1Tail);
				vector<pair<float, float>> projectingVecPair = NeuronGeoGrapher::getVectorWithStartingLoc_btwn2nodes<float>(seg2Tail, seg2Head);
				vector<pair<float, float>> projectedVecPair = NeuronGeoGrapher::getProjectionVector(axialVecPair, projectingVecPair);
				float overlapCheck = (projectingVecPair.begin()->first - seg1Tail.x) * (seg1Tail.x - seg1Head.x) +
					((projectingVecPair.begin() + 1)->first - seg1Tail.y) * (seg1Tail.y - seg1Head.y) + ((projectingVecPair.begin() + 2)->first - seg1Tail.z) * (seg1Tail.z - seg1Head.z);
				if (overlapCheck < 0) continue;

				segUnit newSeg = NeuronStructUtil::segUnitConnect_executer(*it->second.begin()->seg1Ptr, *it->second.begin()->seg2Ptr, tail_tail);
				outputProfiledTree.segs.at(it->second.begin()->seg1Ptr->segID).to_be_deleted = true;
				outputProfiledTree.segs.at(it->second.begin()->seg2Ptr->segID).to_be_deleted = true;
				outputProfiledTree.segs.insert({ outputProfiledTree.segs.size() + 1, newSeg });
				newSegIDs.push_back(outputProfiledTree.segs.size());
				connectedSegIDs.insert(it->second.begin()->seg1Ptr->segID);
				connectedSegIDs.insert(it->second.begin()->seg2Ptr->segID);
				//cout << "connecting segs " << it->second.begin()->seg1Ptr->segID << " and " << it->second.begin()->seg2Ptr->segID << endl;
			}
		}
	}

	vector<ptrdiff_t> nodeDelLocs;
	for (set<int>::iterator connectedIt = connectedSegIDs.begin(); connectedIt != connectedSegIDs.end(); ++connectedIt)
	{
		for (QList<NeuronSWC>::iterator nodeIt = outputProfiledTree.segs.at(*connectedIt).nodes.begin(); nodeIt != outputProfiledTree.segs.at(*connectedIt).nodes.end(); ++nodeIt)
			nodeDelLocs.push_back(outputProfiledTree.node2LocMap.at(nodeIt->n));
		outputProfiledTree.segs.erase(outputProfiledTree.segs.find(*connectedIt));
	}

	sort(nodeDelLocs.rbegin(), nodeDelLocs.rend());
	for (vector<ptrdiff_t>::iterator delNodeIt = nodeDelLocs.begin(); delNodeIt != nodeDelLocs.end(); ++delNodeIt)
		outputProfiledTree.tree.listNeuron.erase(outputProfiledTree.tree.listNeuron.begin() + *delNodeIt);

	for (vector<int>::iterator newSegIt = newSegIDs.begin(); newSegIt != newSegIDs.end(); ++newSegIt)
		outputProfiledTree.tree.listNeuron.append(outputProfiledTree.segs.at(*newSegIt).nodes);

	profiledTreeReInit(outputProfiledTree);
	return outputProfiledTree;
}

profiledTree TreeGrower::itered_connectSegsWithinClusters(profiledTree& inputProfiledTree, float distThreshold)
{
	cout << "iteration 1 " << endl;
	int iterCount = 1;
	profiledTree elongatedTree = this->connectSegsWithinClusters(inputProfiledTree, distThreshold);
	while (elongatedTree.segs.size() != inputProfiledTree.segs.size())
	{
		inputProfiledTree = elongatedTree;

		++iterCount;
		cout << "iteration " << iterCount << " " << endl;
		elongatedTree = this->connectSegsWithinClusters(inputProfiledTree);
	}
	cout << endl;

	return elongatedTree;
}

NeuronTree TreeGrower::swcSamePartExclusion(const NeuronTree& subjectTree, const NeuronTree& refTree, float distThreshold, float nodeTileLength)
{
	map<string, vector<NeuronSWC>> refGridSWCmap, suGridSWCmap;
	NeuronStructUtil::nodeTileMapGen(refTree, refGridSWCmap, nodeTileLength);
	NeuronStructUtil::nodeTileMapGen(subjectTree, suGridSWCmap, nodeTileLength);

	NeuronTree outputTree;
	for (map<string, vector<NeuronSWC>>::iterator suTileIt = suGridSWCmap.begin(); suTileIt != suGridSWCmap.end(); ++suTileIt)
	{
		if (refGridSWCmap.find(suTileIt->first) == refGridSWCmap.end())
		{
			for (vector<NeuronSWC>::iterator it = suTileIt->second.begin(); it != suTileIt->second.end(); ++it)
				outputTree.listNeuron.push_back(*it);
		}
		else
		{
			float minDist = 10000;
			for (vector<NeuronSWC>::iterator it1 = suTileIt->second.begin(); it1 != suTileIt->second.end(); ++it1)
			{
				for (vector<NeuronSWC>::iterator it2 = refGridSWCmap.at(suTileIt->first).begin(); it2 != refGridSWCmap.at(suTileIt->first).end(); ++it2)
				{
					float dist = sqrt((it1->x - it2->x) * (it1->x - it2->x) + (it1->y - it2->y) * (it1->y - it2->y) + (it1->z - it2->z) * (it1->z - it2->z));
					if (dist <= minDist) minDist = dist;
				}

				if (minDist <= distThreshold) continue;
				else outputTree.listNeuron.push_back(*it1);
			}
		}
	}

	map<string, vector<NeuronSWC>> outputGridSWCmap;
	NeuronStructUtil::nodeTileMapGen(outputTree, outputGridSWCmap, nodeTileLength);
	boost::container::flat_set<int> nodeIDs;
	for (map<string, vector<NeuronSWC>>::iterator mapIt = outputGridSWCmap.begin(); mapIt != outputGridSWCmap.end(); ++mapIt)
	{
		for (vector<NeuronSWC>::iterator nodeIt = mapIt->second.begin(); nodeIt != mapIt->second.end(); ++nodeIt)
			nodeIDs.insert(nodeIt->n);
	}

	for (QList<NeuronSWC>::iterator nodeIt = outputTree.listNeuron.begin(); nodeIt != outputTree.listNeuron.end(); ++nodeIt)
	{
		if (nodeIt->parent == -1) continue;
		else
			if (nodeIDs.find(nodeIt->parent) == nodeIDs.end()) nodeIt->parent = -1;
	}

	return outputTree;
}

profiledTree TreeGrower::spikeRemove(const profiledTree& inputProfiledTree, int spikeNodeNum)
{
	profiledTree processTree = inputProfiledTree;
	for (int currNodeNumThre = 1; currNodeNumThre <= spikeNodeNum; ++currNodeNumThre)
	{
		int currNodeNum = 1;
		while (currNodeNum <= currNodeNumThre)
		{
			vector<size_t> delLocs;
			vector<size_t> delLocsCandidates;
			for (QList<NeuronSWC>::iterator it = processTree.tree.listNeuron.begin(); it != processTree.tree.listNeuron.end(); ++it)
			{
				if (processTree.node2childLocMap.find(it->n) == processTree.node2childLocMap.end()) // tip point
				{
					int currID = it->n;
					delLocsCandidates.clear();
					while (1)
					{
						int currPaID = processTree.tree.listNeuron.at(processTree.node2LocMap.at(currID)).parent;
						if (processTree.node2childLocMap.at(currPaID).size() >= 2 && delLocsCandidates.size() <= currNodeNum)
						{
							delLocs.push_back(processTree.node2LocMap.at(currID));
							delLocs.insert(delLocs.end(), delLocsCandidates.begin(), delLocsCandidates.end());
							break;
						}
						else if (processTree.node2childLocMap.at(currPaID).size() == 1 && delLocsCandidates.size() <= currNodeNum)
						{
							delLocsCandidates.push_back(processTree.node2LocMap.at(currID));
							currID = currPaID;
						}
						else if (delLocsCandidates.size() > currNodeNum) break;
					}
				}
			}

			sort(delLocs.rbegin(), delLocs.rend());
			for (vector<size_t>::iterator it = delLocs.begin(); it != delLocs.end(); ++it) processTree.tree.listNeuron.erase(processTree.tree.listNeuron.begin() + ptrdiff_t(*it));
			profiledTreeReInit(processTree);
			++currNodeNum;
		}
	}

	return processTree;
}