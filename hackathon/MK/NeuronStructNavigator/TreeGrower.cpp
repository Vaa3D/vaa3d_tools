//------------------------------------------------------------------------------
// Copyright (c) 2019 Hsienchi Kuo (Allen Institute, Hanchuan Peng's team)
// All rights reserved.
//------------------------------------------------------------------------------

/*******************************************************************************
*
*  TreeGrower class intends to handle the functionalities needed to form a NeuronTree from segmented image signals (ImgAnalyzer::connectedComponent).
*  For TreeGrower, NeuronGeoGrapher is an essential class for which many TreeGrower's methods are further development extended from methods in NeuronGeoGrapher class.
*
*  Major functionalities include:
*
*    a. Basic tree operations, i.e., tree trimming, refining
*    b. Segment forming / elongating and other operations
*    c. Dendritic tree and axonal tree forming
*
*  This class is inherited from NeuronStructExplorer class, as it needs NeuronStructExplorer's capability to manage and process neuron tree and neuron segments.
*  TreeGrower is the main interface in NeuronStructNavigator library for "gorwing" trees out of [NeuronSWC]-based signals.
*
********************************************************************************/

#include <iostream>

#include "NeuronStructUtilities.h"
#include "TreeGrower.h"

using namespace std;
using namespace integratedDataTypes;

/* =========================== Segment Forming / Elongation =========================== */
profiledTree TreeGrower::connectSegsWithinClusters(const profiledTree& inputProfiledTree, float distThreshold)
{
	profiledTree outputProfiledTree = inputProfiledTree;
	this->getSegHeadTailClusters(outputProfiledTree, distThreshold);
	this->getClusterSegPairs(outputProfiledTree);

	vector<int> newSegIDs;
	set<int> connectedSegIDs;
	cout << " -- sharp angle: " << endl << "      ";
	for (boost::container::flat_map<int, vector<segPairProfile>>::iterator it = outputProfiledTree.cluster2segPairMap.begin(); it != outputProfiledTree.cluster2segPairMap.end(); ++it)
	{
		// Currently it ONLY processes clusters with ONLY 1 segPair.
		// Clusters with multiple segPairs are more complicated since it involves optimizing the segPair arrangement.
		if (it->second.size() == 1) 
		{
			// Skip segments if they have been connected.
			if (connectedSegIDs.find(it->second.begin()->seg1Ptr->segID) != connectedSegIDs.end() || connectedSegIDs.find(it->second.begin()->seg2Ptr->segID) != connectedSegIDs.end()) continue;
			
			// Skip segments if they have multiple tails.
			if (it->second.begin()->seg1Ptr->tails.size() > 1 || it->second.begin()->seg2Ptr->tails.size() > 1) continue;

			if (it->second.begin()->currConnOrt == head_head)
			{
				// Skip segments if the angle formed by their displacement vector is greater than 90 degrees.
				if (it->second.begin()->segsAngleDiff > 0.5)
				{
					cout << it->second.begin()->seg1Ptr->segID << "-" << it->second.begin()->seg2Ptr->segID << "(head-head), ";
					continue;
				}

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

				/*NeuronTree segTree1, segTree2;
				vector<NeuronTree> trees;
				segTree1.listNeuron.append(it->second.begin()->seg1Ptr->nodes);
				segTree2.listNeuron.append(it->second.begin()->seg2Ptr->nodes);
				trees.push_back(segTree1);
				trees.push_back(segTree2);
				NeuronTree segPairTree = NeuronStructUtil::swcCombine(trees);
				QString saveFullName = "C:\\Users\\hsienchik\\Desktop\\segPairDebug\\" + QString::number(it->second.begin()->seg1Ptr->segID) + "_" + QString::number(it->second.begin()->seg2Ptr->segID) + ".swc";
				writeSWC_file(saveFullName, segPairTree);*/

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
				// Skip segments if the angle formed by their displacement vector is greater than 90 degrees.
				if (it->second.begin()->segsAngleDiff > 0.5)
				{
					cout << it->second.begin()->seg1Ptr->segID << "-" << it->second.begin()->seg2Ptr->segID << "(head-tail), ";
					continue;
				}

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

				/*NeuronTree segTree1, segTree2;
				vector<NeuronTree> trees;
				segTree1.listNeuron.append(it->second.begin()->seg1Ptr->nodes);
				segTree2.listNeuron.append(it->second.begin()->seg2Ptr->nodes);
				trees.push_back(segTree1);
				trees.push_back(segTree2);
				NeuronTree segPairTree = NeuronStructUtil::swcCombine(trees);
				QString saveFullName = "C:\\Users\\hsienchik\\Desktop\\segPairDebug\\" + QString::number(it->second.begin()->seg1Ptr->segID) + "_" + QString::number(it->second.begin()->seg2Ptr->segID) + ".swc";
				writeSWC_file(saveFullName, segPairTree);*/

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
				// Skip segments if the angle formed by their displacement vector is greater than 90 degrees.
				if (it->second.begin()->segsAngleDiff > 0.5)
				{
					cout << it->second.begin()->seg1Ptr->segID << "-" << it->second.begin()->seg2Ptr->segID << "(tail-tail), ";
					continue;
				}

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

				/*NeuronTree segTree1, segTree2;
				vector<NeuronTree> trees;
				segTree1.listNeuron.append(it->second.begin()->seg1Ptr->nodes);
				segTree2.listNeuron.append(it->second.begin()->seg2Ptr->nodes);
				trees.push_back(segTree1);
				trees.push_back(segTree2);
				NeuronTree segPairTree = NeuronStructUtil::swcCombine(trees);
				QString saveFullName = "C:\\Users\\hsienchik\\Desktop\\segPairDebug\\" + QString::number(it->second.begin()->seg1Ptr->segID) + "_" + QString::number(it->second.begin()->seg2Ptr->segID) + ".swc";
				writeSWC_file(saveFullName, segPairTree);*/

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
	cout << endl;

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
	cout << "analyzing seg end clusters..." << endl;
	cout << " -- iteration 1 " << endl;
	int iterCount = 1;
	profiledTree elongatedTree = this->connectSegsWithinClusters(inputProfiledTree, distThreshold);
	while (elongatedTree.segs.size() != inputProfiledTree.segs.size())
	{
		inputProfiledTree = elongatedTree;

		++iterCount;
		cout << " -- iteration " << iterCount << " " << endl;
		elongatedTree = this->connectSegsWithinClusters(inputProfiledTree);
	}
	cout << endl;

	return elongatedTree;
}
/* ====================== END of [Segment Forming / Elongation] ======================= */



/* =================================== Tree Forming =================================== */
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

NeuronTree TreeGrower::branchBreak(const profiledTree& inputProfiledTree, double spikeThre, bool spikeRemove)
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

NeuronTree TreeGrower::dendriticTree_shellCentroid(double distThre)
{
	NeuronTree outputTree;

	if (this->radiusShellMap_loc.empty() || this->polarNodeList.empty())
	{
		cout << "Either polarNodeList or the map of radius to polarNodeList location is empty. Do nothing and return." << endl;
		return outputTree;
	}

	this->radius2shellTreeMap = NeuronGeoGrapher::radius2NeuronTreeMap(this->radiusShellMap_loc, this->polarNodeList);
	this->radius2shellConnCompMap = NeuronGeoGrapher::radius2connCompsShell(this->radius2shellTreeMap);	
	
	map<double, boost::container::flat_map<int, vector<int>>> shell2shellConnMap; // The map of inner shell's connected component to outer shell's connected components.
																				  // shell2shellConnMap: the location of the conn. component on the previous shell to the locaions of the conn. components on the current shell.
																				  //   -> 'location' means the location on the vector<connectedComponent> of radius2connCompsShell.second.
	// Pre-allocate shell2shellConnMap to speed up the process later on.
	for (boost::container::flat_map<double, vector<connectedComponent>>::iterator shellIt = this->radius2shellConnCompMap.begin(); shellIt != this->radius2shellConnCompMap.end(); ++shellIt)
	{
		boost::container::flat_map<int, vector<int>> innerLoc2outerLocMap;
		shell2shellConnMap.insert(pair<double, boost::container::flat_map<int, vector<int>>>(shellIt->first, innerLoc2outerLocMap));
	}
	
	// ------------------- Profiling spacially adjacent connected components between inner and outer shells -------------------
	for (boost::container::flat_map<double, vector<connectedComponent>>::iterator shellIt = this->radius2shellConnCompMap.begin() + 1; shellIt != this->radius2shellConnCompMap.end(); ++shellIt)
	{
		for (vector<connectedComponent>::iterator innerIt = (shellIt - 1)->second.begin(); innerIt != (shellIt - 1)->second.end(); ++innerIt)
		{
			vector<int> outerConnLocs;
			outerConnLocs.clear();
			for (vector<connectedComponent>::iterator outerIt = shellIt->second.begin(); outerIt != shellIt->second.end(); ++outerIt)
			{
				if (NeuronGeoGrapher::connCompBoundingRangeCheck(*innerIt, *outerIt))
				{
					float dist = sqrtf((innerIt->ChebyshevCenter[0] - outerIt->ChebyshevCenter[0]) * (innerIt->ChebyshevCenter[0] - outerIt->ChebyshevCenter[0]) +
									   (innerIt->ChebyshevCenter[1] - outerIt->ChebyshevCenter[1]) * (innerIt->ChebyshevCenter[1] - outerIt->ChebyshevCenter[1]) +
									   (innerIt->ChebyshevCenter[2] - outerIt->ChebyshevCenter[2]) * (innerIt->ChebyshevCenter[2] - outerIt->ChebyshevCenter[2]));
					if (dist < 3) outerConnLocs.push_back(int(outerIt - shellIt->second.begin()));  // This is the criterion determining if 2 conn. components from 2 consecutive layers are adjacent to each other.
				}
			}
			if (outerConnLocs.empty()) continue;

			shell2shellConnMap.at(shellIt->first).insert(pair<int, vector<int>>(int(innerIt - (shellIt - 1)->second.begin()), outerConnLocs));
		}
	}


	// --------------- END of [Profiling spacially adjacent connected components between inner and outer shells] ---------------

	// ------------------------------------- Building up dendritic tree -------------------------------------
	int nodeID = 1;
	boost::container::flat_map<int, int> innerLoc2nodeIDmap;
	boost::container::flat_map<int, int> outerLoc2nodeIDmap;
	boost::container::flat_set<int> outerLocs;
	bool emptyShell = true;
	for (map<double, boost::container::flat_map<int, vector<int>>>::iterator shellIt = shell2shellConnMap.begin(); shellIt != shell2shellConnMap.end(); ++shellIt)
	{
		//cout << "shell " << shellIt->first << ":" << endl;
		if (shellIt->second.empty()) emptyShell = true;
		else emptyShell = false;

		if (emptyShell) // If previous shell is empty, then all centroids from the connected components on the current shell will be roots.
		{		
			for (vector<connectedComponent>::iterator rootCompIt = this->radius2shellConnCompMap.at(shellIt->first).begin(); rootCompIt != this->radius2shellConnCompMap.at(shellIt->first).end(); ++rootCompIt)
			{
				NeuronSWC newRootNode;
				newRootNode.n = nodeID;
				newRootNode.x = rootCompIt->ChebyshevCenter[0];
				newRootNode.y = rootCompIt->ChebyshevCenter[1];
				newRootNode.z = rootCompIt->ChebyshevCenter[2];
				newRootNode.parent = -1;
				newRootNode.type = 3;
				outputTree.listNeuron.push_back(newRootNode);
				innerLoc2nodeIDmap.insert(pair<int, int>(int(rootCompIt - this->radius2shellConnCompMap.at(shellIt->first).begin()), nodeID));
				++nodeID;
			}	
			//cout << endl;
			continue;
		}
		else
		{
			outerLocs.clear();
			for (int i = 0; i < this->radius2shellConnCompMap.at(shellIt->first).size(); ++i) outerLocs.insert(i);

			for (boost::container::flat_map<int, vector<int>>::iterator loc2locIt = shellIt->second.begin(); loc2locIt != shellIt->second.end(); ++loc2locIt)
			{
				//cout << loc2locIt->first << "-> ";			
				for (vector<int>::iterator outerIt = loc2locIt->second.begin(); outerIt != loc2locIt->second.end(); ++outerIt)
				{
					//cout << *outerIt << " ";
					NeuronSWC newNode;
					newNode.n = nodeID;
					newNode.x = this->radius2shellConnCompMap.at(shellIt->first).at(*outerIt).ChebyshevCenter[0];
					newNode.y = this->radius2shellConnCompMap.at(shellIt->first).at(*outerIt).ChebyshevCenter[1];
					newNode.z = this->radius2shellConnCompMap.at(shellIt->first).at(*outerIt).ChebyshevCenter[2];
					newNode.parent = innerLoc2nodeIDmap.at(loc2locIt->first);
					newNode.type = 3;
					outputTree.listNeuron.push_back(newNode);
					outerLoc2nodeIDmap.insert(pair<int, int>(*outerIt, nodeID)); // boost's associate containers haven't adopted C++11's expression yet.
					
					if (outerLocs.find(*outerIt) != outerLocs.end()) outerLocs.erase(outerLocs.find(*outerIt));
					++nodeID;
				}
				//cout << endl;
			}

			//cout << "new root: ";
			for (boost::container::flat_set<int>::iterator remainIt = outerLocs.begin(); remainIt != outerLocs.end(); ++remainIt)
			{
				//cout << *remainIt << " ";
				NeuronSWC newRootNode;
				newRootNode.n = nodeID;
				newRootNode.x = this->radius2shellConnCompMap.at(shellIt->first).at(*remainIt).ChebyshevCenter[0];
				newRootNode.y = this->radius2shellConnCompMap.at(shellIt->first).at(*remainIt).ChebyshevCenter[1];
				newRootNode.z = this->radius2shellConnCompMap.at(shellIt->first).at(*remainIt).ChebyshevCenter[2];
				newRootNode.parent = -1;
				newRootNode.type = 3;
				outputTree.listNeuron.push_back(newRootNode);
				outerLoc2nodeIDmap.insert(pair<int, int>(*remainIt, nodeID));
				++nodeID;
			}
			//cout << endl;
		}
		//cout << endl;

		innerLoc2nodeIDmap.clear();
		innerLoc2nodeIDmap = outerLoc2nodeIDmap;
		outerLoc2nodeIDmap.clear();
	}
	// -------------------------------- END of [Building up dendritic tree] --------------------------------

	return outputTree;
}

NeuronTree TreeGrower::swcSamePartExclusion(const NeuronTree& subjectTree, const NeuronTree& refTree, float distThreshold, float nodeTileLength)
{
	map<string, vector<NeuronSWC>> refGridSWCmap, sbjGridSWCmap;
	NeuronStructUtil::nodeTileMapGen(refTree, refGridSWCmap, nodeTileLength);
	NeuronStructUtil::nodeTileMapGen(subjectTree, sbjGridSWCmap, nodeTileLength);

	NeuronTree outputTree;
	for (map<string, vector<NeuronSWC>>::iterator sbjTileIt = sbjGridSWCmap.begin(); sbjTileIt != sbjGridSWCmap.end(); ++sbjTileIt)
	{
		if (refGridSWCmap.find(sbjTileIt->first) == refGridSWCmap.end())
		{
			for (vector<NeuronSWC>::iterator it = sbjTileIt->second.begin(); it != sbjTileIt->second.end(); ++it)
				outputTree.listNeuron.push_back(*it);
		}
		else
		{
			float minDist = 10000;
			for (vector<NeuronSWC>::iterator it1 = sbjTileIt->second.begin(); it1 != sbjTileIt->second.end(); ++it1)
			{
				for (vector<NeuronSWC>::iterator it2 = refGridSWCmap.at(sbjTileIt->first).begin(); it2 != refGridSWCmap.at(sbjTileIt->first).end(); ++it2)
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
/* ============================== END of [Tree Forming] =============================== */



/* ============================= Tree Trimming / Refining ============================= */
profiledTree TreeGrower::spikeRemoval(const profiledTree& inputProfiledTree, int spikeNodeNum)
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

profiledTree TreeGrower::itered_spikeRemoval(profiledTree& inputProfiledTree, int spikeNodeNum)
{
	cout << "removing spikes.." << endl << "  iteration 1 " << endl;
	int iterCount = 1;
	profiledTree cleanedTree = TreeGrower::spikeRemoval(inputProfiledTree, spikeNodeNum);
	//cout << "    spike number: " << cleanedTree.spikeRootIDs.size() << endl;
	while (cleanedTree.tree.listNeuron.size() != inputProfiledTree.tree.listNeuron.size())
	{
		inputProfiledTree = cleanedTree;

		++iterCount;
		cout << "  iteration " << iterCount << " " << endl;
		cleanedTree = TreeGrower::spikeRemoval(inputProfiledTree, spikeNodeNum);
		//cout << "    spike number: " << cleanedTree.spikeRootIDs.size() << endl;
	}
	cout << endl;

	return cleanedTree;
}

profiledTree TreeGrower::removeHookingHeadTail(const profiledTree& inputProfiledTree, float radAngleThre)
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

profiledTree TreeGrower::itered_removeHookingHeadTail(profiledTree& inputProfiledTree, float radAngleThre)
{
	cout << "removing hooks.." << endl << "  iteration 1 " << endl;
	int iterCount = 1;
	profiledTree hookRemovedTree = TreeGrower::removeHookingHeadTail(inputProfiledTree, radAngleThre);
	while (hookRemovedTree.tree.listNeuron.size() != inputProfiledTree.tree.listNeuron.size())
	{
		inputProfiledTree = hookRemovedTree;

		++iterCount;
		cout << "  iteration " << iterCount << " " << endl;
		hookRemovedTree = TreeGrower::removeHookingHeadTail(inputProfiledTree, radAngleThre);
	}
	cout << endl;

	return hookRemovedTree;
}

profiledTree TreeGrower::segSharpAngleSmooth_lengthDistRatio(const profiledTree& inputProfiledTree, const double ratio)
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

profiledTree TreeGrower::itered_segSharpAngleSmooth_lengthDistRatio(profiledTree& inputProfiledTree, double ratio)
{
	cout << "smoothing angles.." << endl << "  iteration 1 " << endl;
	int iterCount = 1;
	
	profiledTree angleSmoothedTree = TreeGrower::segSharpAngleSmooth_lengthDistRatio(inputProfiledTree, ratio);
	while (angleSmoothedTree.smoothedNodeIDs.size() > 0)
	{
		NeuronStructExplorer::segMorphProfile(angleSmoothedTree);
		inputProfiledTree = angleSmoothedTree;

		++iterCount;
		cout << "  iteration " << iterCount << " " << endl;
		angleSmoothedTree = TreeGrower::segSharpAngleSmooth_lengthDistRatio(inputProfiledTree, ratio);
	}
	cout << endl;

	return angleSmoothedTree;
}

profiledTree TreeGrower::segSharpAngleSmooth_distThre_3nodes(const profiledTree& inputProfiledTree, const double distThre)
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
				NeuronSWC currChildNode = segIt->second.nodes.at(*segIt->second.seg_childLocMap.at(nodeIt->first).begin());
				NeuronSWC centerNode = segIt->second.nodes.at(segIt->second.seg_nodeLocMap.at(nodeIt->first));
				if (abs(currPaNode.x - centerNode.x) + abs(currPaNode.y - centerNode.y) + abs(currPaNode.z - centerNode.z) >
					abs(currChildNode.x - centerNode.x) + abs(currChildNode.y - centerNode.y) + abs(currChildNode.z - centerNode.z))
					outputProfiledTree.tree.listNeuron[outputProfiledTree.node2LocMap.at(nodeIt->first)].parent = -1;
				else outputProfiledTree.tree.listNeuron[outputProfiledTree.node2LocMap.at(currChildNode.n)].parent = -1;			
			}
		}
	}

	profiledTreeReInit(outputProfiledTree);
	return outputProfiledTree;
}
/* ======================== END of [Tree Trimming / Refining] ========================= */
