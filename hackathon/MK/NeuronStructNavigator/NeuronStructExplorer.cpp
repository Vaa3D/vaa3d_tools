//------------------------------------------------------------------------------
// Copyright (c) 2018 Hsienchi Kuo (Allen Institute, Hanchuan Peng's team)
// All rights reserved.
//------------------------------------------------------------------------------

/*******************************************************************************
*
*  This library intends to provide functionalities for neuron struct analysis, including graph and geometry analysis, etc.
*  Typically NeuronStructExplorer class methods need a profiledTree struct as part of the input arguments. A profiledTree can be assigned when the class is initiated or later.
*
*  profiledTree is the core data type in NeuronStructExplorer class. It profiles the NeuronTree and carries crucial information.
*  Particularly profiledTree provides node-location, child-location, and detailed segment information of a NeuronTree.
*  Each segment of a NeuronTree is represented as a segUnit struct. A segUnit struct carries within-segment node-location, child-location, head, and tails information.
*  All segments are stored and sorted in profiledTree's map<int, segUnit> data member.

*  The class can be initialized with or without a profiledTree being initialized at the same time. A profiledTree can be stored and indexed in NeuronStructExplorer's treeDatabe.
*
********************************************************************************/

#include <iostream>
#include <algorithm>
#include <cmath>

#include "basic_4dimage.h"
#include "basic_landmark.h"
#include "neuron_format_converter.h"

#include "NeuronStructExplorer.h"

/* ================================ Constructors and Basic Profiling Data/Function Members ================================ */
NeuronStructExplorer::NeuronStructExplorer(QString inputFileName)
{
	QStringList fileNameParse1 = inputFileName.split(".");
	if (fileNameParse1.back() == "swc")
	{
		NeuronTree inputSWC = readSWC_file(inputFileName);
		QStringList fileNameParse2 = fileNameParse1.at(0).split("\\");
		string treeName = fileNameParse2.back().toStdString();
		this->treeEntry(inputSWC, treeName);
	}
}

void NeuronStructExplorer::treeEntry(const NeuronTree& inputTree, string treeName, float segTileLength)
{
	if (this->treeDataBase.find(treeName) == this->treeDataBase.end())
	{
		profiledTree registeredTree(inputTree, segTileLength);
		this->treeDataBase.insert(pair<string, profiledTree>(treeName, registeredTree));
	}
	else
	{
		cerr << "This tree name has already existed. The tree will not be registered for further operations." << endl;
		return;
	}
}

void NeuronStructExplorer::profiledTreeReInit(profiledTree& inputProfiledTree)
{
	profiledTree tempTree(inputProfiledTree.tree, inputProfiledTree.segTileSize);
	inputProfiledTree = tempTree;
}

map<int, segUnit> NeuronStructExplorer::findSegs(const QList<NeuronSWC>& inputNodeList, const map<int, vector<size_t>>& node2childLocMap)
{
	// -- This method profiles all segments in a given input tree.

	map<int, segUnit> segs;
	vector<NeuronSWC> inputNodes;
	for (QList<NeuronSWC>::const_iterator nodeIt = inputNodeList.begin(); nodeIt != inputNodeList.end(); ++nodeIt)
		inputNodes.push_back(*nodeIt);

	int segCount = 0;
	for (vector<NeuronSWC>::iterator nodeIt = inputNodes.begin(); nodeIt != inputNodes.end(); ++nodeIt)
	{
		if (nodeIt->parent == -1)
		{
			// ------------ Identify segments ------------
			++segCount;
			//cout << " processing segment " << segCount << "..  head node ID: " << nodeIt->n << endl << "  ";
			segUnit newSeg;
			newSeg.segID = segCount;
			newSeg.head = nodeIt->n;
			newSeg.nodes.push_back(*nodeIt);
			vector<size_t> childLocs = node2childLocMap.find(nodeIt->n)->second;
				// In NeuronStructUtilities::node2loc_node2childLocMap, 
				// if a segment is only with a head, its childLocs will also be assigned as empty set.
			if (childLocs.empty()) 
			{
				newSeg.tails.push_back(nodeIt->n);
				newSeg.seg_nodeLocMap.insert(pair<int, size_t>(nodeIt->n, 0));
				segs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
				continue;
			}

			vector<size_t> grandChildLocs;
			vector<size_t> thisGrandChildLocs;
			bool childNum = true;
			while (childNum)
			{
				grandChildLocs.clear();
				for (vector<size_t>::iterator childIt = childLocs.begin(); childIt != childLocs.end(); ++childIt)
				{
					//cout << inputNodes.at(*childIt).n << " ";
					thisGrandChildLocs.clear();
					for (QList<NeuronSWC>::iterator checkIt = newSeg.nodes.begin(); checkIt != newSeg.nodes.end(); ++checkIt)
						if (checkIt->n == inputNodes.at(*childIt).n) goto LOOP;

					newSeg.nodes.push_back(inputNodes.at(*childIt));
					if (node2childLocMap.find(inputNodes.at(*childIt).n) == node2childLocMap.end())
					{
							newSeg.tails.push_back(inputNodeList[*childIt].n);
							//cout << "<-tail ";
					}
					else
					{
						thisGrandChildLocs = node2childLocMap.at(inputNodes.at(*childIt).n);
						grandChildLocs.insert(grandChildLocs.end(), thisGrandChildLocs.begin(), thisGrandChildLocs.end());
					}

				LOOP:
					continue;
				}
				if (grandChildLocs.size() == 0) childNum = false;

				childLocs = grandChildLocs;
			}
			//cout << endl; 
			// --------- END of [Identify segments] ---------

			// ------------- Identify node/child location in the segment -------------
			for (QList<NeuronSWC>::iterator it = newSeg.nodes.begin(); it != newSeg.nodes.end(); ++it) 
				newSeg.seg_nodeLocMap.insert(pair<int, size_t>(it->n, size_t(it - newSeg.nodes.begin())));

			for (QList<NeuronSWC>::iterator it = newSeg.nodes.begin(); it != newSeg.nodes.end(); ++it)
			{
				vector<size_t> childSegLocs;
				childSegLocs.clear();
				if (newSeg.seg_childLocMap.find(it->n) == newSeg.seg_childLocMap.end())
					newSeg.seg_childLocMap.insert(pair<int, vector<size_t>>(it->n, childSegLocs));
				// This ensures that tail node still has its own pair in seg_chileLocMap, and its childSegLocs is empty.

				if (node2childLocMap.find(it->n) != node2childLocMap.end())
				{
					vector<size_t> childLocs = node2childLocMap.at(it->n);
					for (vector<size_t>::iterator globalChildLocIt = childLocs.begin(); globalChildLocIt != childLocs.end(); ++globalChildLocIt)
					{
						size_t childSegLoc = newSeg.seg_nodeLocMap[inputNodeList.at(*globalChildLocIt).n];
						newSeg.seg_childLocMap[it->n].push_back(childSegLoc);
					}
				}
			}
			segs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
			// --------- END of [Identify node/child location in the segment] ---------
		}
	}

	return segs;
}

map<string, vector<int>> NeuronStructExplorer::segTileMap(const vector<segUnit>& inputSegs, float xyLength, bool head)
{
	if (head)
	{
		map<string, vector<int>> outputSegTileMap;
		for (vector<segUnit>::const_iterator it = inputSegs.begin(); it != inputSegs.end(); ++it)
		{
			string xLabel = to_string(int(it->nodes.begin()->x / xyLength));
			string yLabel = to_string(int(it->nodes.begin()->y / xyLength));
			string zLabel = to_string(int(it->nodes.begin()->z / (xyLength / zRATIO)));
			string keyLabel = xLabel + "_" + yLabel + "_" + zLabel;
			if (outputSegTileMap.find(keyLabel) != outputSegTileMap.end()) outputSegTileMap[keyLabel].push_back(it->segID);
			else
			{
				vector<int> newSet;
				newSet.push_back(it->segID);
				outputSegTileMap.insert(pair<string, vector<int>>(keyLabel, newSet));
			}
		}
		return outputSegTileMap;
	}
	else
	{
		map<string, vector<int>> outputSegTileMap;
		for (vector<segUnit>::const_iterator it = inputSegs.begin(); it != inputSegs.end(); ++it)
		{
			for (vector<int>::const_iterator tailIt = it->tails.begin(); tailIt != it->tails.end(); ++tailIt)
			{
				NeuronSWC tailNode = it->nodes.at(it->seg_nodeLocMap.at(*tailIt));
				string xLabel = to_string(int(tailNode.x / xyLength));
				string yLabel = to_string(int(tailNode.y / xyLength));
				string zLabel = to_string(int(tailNode.z / (xyLength / zRATIO)));
				string keyLabel = xLabel + "_" + yLabel + "_" + zLabel;
				if (outputSegTileMap.find(keyLabel) != outputSegTileMap.end()) outputSegTileMap[keyLabel].push_back(it->segID);
				else
				{
					vector<int> newSet;
					newSet.push_back(it->segID);
					outputSegTileMap.insert(pair<string, vector<int>>(keyLabel, newSet));
				}
			}
		}
		return outputSegTileMap;
	}
}

void NeuronStructExplorer::getSegHeadTailClusters(profiledTree& inputProfiledTree, float distThreshold)
{
	this->getTileBasedSegClusters(inputProfiledTree, distThreshold);
	int headSegNum = 0, tailSegNum = 0;
	for (boost::container::flat_map<int, boost::container::flat_set<int>>::iterator it1 = inputProfiledTree.segHeadClusters.begin(); it1 != inputProfiledTree.segHeadClusters.end(); ++it1)
		headSegNum = headSegNum + it1->second.size();
	for (boost::container::flat_map<int, boost::container::flat_set<int>>::iterator it2 = inputProfiledTree.segTailClusters.begin(); it2 != inputProfiledTree.segTailClusters.end(); ++it2)
		tailSegNum = tailSegNum + it2->second.size();
	//cout << "head segment number: " << headSegNum << " -- tail segment number: " << tailSegNum << endl;
	//cout << "head segment map number: " << inputProfiledTree.headSeg2ClusterMap.size() << " -- tail segment map number: " << inputProfiledTree.tailSeg2ClusterMap.size() << endl;
	this->mergeTileBasedSegClusters(inputProfiledTree, distThreshold);
	//cout << "======= after merging clusters =======" << endl;
	//cout << "head segment number: " << headSegNum << " -- tail segment number: " << tailSegNum << endl;
	//cout << "head segment map number: " << inputProfiledTree.headSeg2ClusterMap.size() << " -- tail segment map number: " << inputProfiledTree.tailSeg2ClusterMap.size() << endl;
}

void NeuronStructExplorer::mergeTileBasedSegClusters(profiledTree& inputProfiledTree, float distThreshold)
{
	// -- This method merges head/tail clusters across tiles. 
	// -- Strategy: 1. Copy mergeable head clusters from source clusters to destined clusters. Their tail cluster counterparts are also copied over correspondingly.
	//              2. Merge head and tail clusters that have segments in common.
	//              3. Assign those tails from the tiles where no head exists by searching connectable heads in their neighboring tiles.
	//                 -> Because tails are copied and merged by following head cluster's footstep. Those tails without heads in the tile will be missed out.
	//                 -> Possible issues:
	//                     a. There may be head clusters should be merged due to identified tails that join them together however is not recognized. 
	//                        This is because the process handles head clusters first and separately. The reason of this approach is for efficiency purpose.
	//              4. Merge tail clusters that have segments in common.
	//                 -> This step is required because there is a tail cluster step after 1. and 2. Consequently, there could be duplicated clusters.
	//
	// -- This method has been intensively tested. Apart from the menioned minor issue that could cause insignificant errors, overall it's running correctly. 
	// -- Any necessary modification should be contrained in the current framework to avoid complications.

	boost::container::flat_set<string> processedHeadTiles;
	boost::container::flat_set<string> processedTailTiles;
	boost::container::flat_map<string, boost::container::flat_set<int>> headTile2ClusterMap;
		
	boost::container::flat_set<int> allTailClusters, labeledTailClusters, leftTailClusters;
	for (boost::container::flat_map<int, boost::container::flat_set<int>>::iterator it = inputProfiledTree.segTailClusters.begin(); it != inputProfiledTree.segTailClusters.end(); ++it)
		allTailClusters.insert(it->first);

	// ------------------------------------ Merge head clusters ------------------------------------ //
	// head tile key to head clusters
	for (map<string, vector<int>>::iterator headTileIt = inputProfiledTree.segHeadMap.begin(); headTileIt != inputProfiledTree.segHeadMap.end(); ++headTileIt)
	{
		boost::container::flat_set<int> currTileClusters;
		for (vector<int>::iterator headIt = headTileIt->second.begin(); headIt != headTileIt->second.end(); ++headIt)
		{
			if (inputProfiledTree.headSeg2ClusterMap.find(*headIt) != inputProfiledTree.headSeg2ClusterMap.end())
				currTileClusters.insert(inputProfiledTree.headSeg2ClusterMap.at(*headIt));
		}
		headTile2ClusterMap.insert(pair<string, boost::container::flat_set<int>>(headTileIt->first, currTileClusters));
	}
	
	for (map<string, vector<int>>::iterator headTileIt = inputProfiledTree.segHeadMap.begin(); headTileIt != inputProfiledTree.segHeadMap.end(); ++headTileIt)
	{
		vector<string> tileLabelStrings;
		vector<int> tileLabels(3);
		boost::split(tileLabelStrings, headTileIt->first, boost::is_any_of("_"));
		tileLabels[0] = stoi(tileLabelStrings.at(0));
		tileLabels[1] = stoi(tileLabelStrings.at(1));
		tileLabels[2] = stoi(tileLabelStrings.at(2));

		// ------------------ Process clusters by tiles and their adjacent tiles ------------------ //
		for (vector<int>::iterator headSegIt = headTileIt->second.begin(); headSegIt != headTileIt->second.end(); ++headSegIt)
		{
			for (int zi = -1; zi <= 1; ++zi)
			{
				for (int yi = -1; yi <= 1; ++yi)
				{
					for (int xi = -1; xi <= 1; ++xi)
					{
						int checkXlabel = tileLabels[0] + xi;
						int checkYlabel = tileLabels[1] + yi;
						int checkZlabel = tileLabels[2] + zi;
						string checkLabel = to_string(checkXlabel) + "_" + to_string(checkYlabel) + "_" + to_string(checkZlabel);
						
						if (inputProfiledTree.segHeadClusters.at(inputProfiledTree.headSeg2ClusterMap.at(*headSegIt)).empty()) continue; // if the tile to be examined is empty of head segs, skip

						if (inputProfiledTree.segHeadMap.find(checkLabel) != inputProfiledTree.segHeadMap.end())
						{
							if (processedHeadTiles.find(checkLabel) != processedHeadTiles.end()) continue; // check if the tile is already processed

							// all head clusters in the current tile:
							boost::container::flat_set<int> currTileClusters = headTile2ClusterMap.at(checkLabel);
							for (boost::container::flat_set<int>::iterator headClusterIt = currTileClusters.begin(); headClusterIt != currTileClusters.end(); ++headClusterIt)
							{
								// all heads in the current head cluster
								boost::container::flat_set<int> currClusterSegs = inputProfiledTree.segHeadClusters.at(*headClusterIt);
								for (boost::container::flat_set<int>::iterator checkHeadSegIt = currClusterSegs.begin(); checkHeadSegIt != currClusterSegs.end(); ++checkHeadSegIt)
								{
									if (inputProfiledTree.headSeg2ClusterMap.at(*headSegIt) == *headClusterIt) continue;

									NeuronSWC* head1ptr = &(inputProfiledTree.tree.listNeuron[inputProfiledTree.node2LocMap.at(inputProfiledTree.segs.at(*headSegIt).head)]);
									NeuronSWC* head2ptr = &(inputProfiledTree.tree.listNeuron[inputProfiledTree.node2LocMap.at(inputProfiledTree.segs.at(*checkHeadSegIt).head)]);
									float dist = sqrtf((head1ptr->x - head2ptr->x) * (head1ptr->x - head2ptr->x) + (head1ptr->y - head2ptr->y) * (head1ptr->y - head2ptr->y) + (head1ptr->z - head2ptr->z) * (head1ptr->z - head2ptr->z));

									if (dist <= distThreshold)
									{
										// copy all segs from current cluster to targeted cluster (the cluster *headSegIt belongs to)	
										inputProfiledTree.segHeadClusters.at(inputProfiledTree.headSeg2ClusterMap.at(*headSegIt)).insert(currClusterSegs.begin(), currClusterSegs.end());
										//cout << "tile label: " << headTileIt->first << " - " << checkLabel << "   cluster" << inputProfiledTree.headSeg2ClusterMap.at(*headSegIt) << " <= cluster" << *headClusterIt << endl;

										// copy all segs from "current tail cluster" to the targeted corresponding cluster
										if (!inputProfiledTree.segTailClusters.at(*headClusterIt).empty())
											inputProfiledTree.segTailClusters.at(inputProfiledTree.headSeg2ClusterMap.at(*headSegIt)).insert(inputProfiledTree.segTailClusters.at(*headClusterIt).begin(), inputProfiledTree.segTailClusters.at(*headClusterIt).end());

										break;
									}
								}
							}
						}
					}
				}
			}
		}
		// ------------- END of [Process clusters by tiles and their adjacent tiles] ------------- //
		processedHeadTiles.insert(headTileIt->first);
	}
	
	// ----------------- Merge clusters that have segments in common ----------------- //
	int headClusterNum = 0, newHeadClusterNum = 0;
	for (boost::container::flat_map<int, boost::container::flat_set<int>>::iterator it = inputProfiledTree.segHeadClusters.begin(); it != inputProfiledTree.segHeadClusters.end(); ++it)
		if (!it->second.empty()) ++headClusterNum;
	while (1)
	{
		for (boost::container::flat_map<int, boost::container::flat_set<int>>::iterator mergeIt1 = inputProfiledTree.segHeadClusters.begin(); mergeIt1 != inputProfiledTree.segHeadClusters.end() - 1; ++mergeIt1)
		{
			for (boost::container::flat_map<int, boost::container::flat_set<int>>::iterator mergeIt2 = mergeIt1 + 1; mergeIt2 != inputProfiledTree.segHeadClusters.end(); ++mergeIt2)
			{
				for (boost::container::flat_set<int>::iterator segCheck1 = mergeIt1->second.begin(); segCheck1 != mergeIt1->second.end(); ++segCheck1)
				{
					for (boost::container::flat_set<int>::iterator segCheck2 = mergeIt2->second.begin(); segCheck2 != mergeIt2->second.end(); ++segCheck2)
					{
						if (*segCheck1 == *segCheck2)
						{
							mergeIt1->second.insert(mergeIt2->second.begin(), mergeIt2->second.end());
							for (boost::container::flat_set<int>::iterator mapIt = mergeIt1->second.begin(); mapIt != mergeIt1->second.end(); ++mapIt)
								inputProfiledTree.headSeg2ClusterMap[*mapIt] = mergeIt1->first;
							mergeIt2->second.clear();

							if (!inputProfiledTree.segTailClusters.at(mergeIt2->first).empty())
							{
								inputProfiledTree.segTailClusters.at(mergeIt1->first).insert(inputProfiledTree.segTailClusters.at(mergeIt2->first).begin(), inputProfiledTree.segTailClusters.at(mergeIt2->first).end());
								for (boost::container::flat_set<int>::iterator mapIt = inputProfiledTree.segTailClusters.at(mergeIt1->first).begin(); mapIt != inputProfiledTree.segTailClusters.at(mergeIt1->first).end(); ++mapIt)
									inputProfiledTree.tailSeg2ClusterMap[*mapIt] = mergeIt1->first;
								inputProfiledTree.segTailClusters.at(mergeIt2->first).clear();							
								
								labeledTailClusters.insert(mergeIt2->first);
							}

							labeledTailClusters.insert(mergeIt1->first);
							goto HEAD_CLUSTER_MERGED;
						}
					}
				}

			HEAD_CLUSTER_MERGED:
				continue;
			}
		}

		newHeadClusterNum = 0;
		for (boost::container::flat_map<int, boost::container::flat_set<int>>::iterator it = inputProfiledTree.segHeadClusters.begin(); it != inputProfiledTree.segHeadClusters.end(); ++it)
			if (!it->second.empty()) ++newHeadClusterNum;

		if (newHeadClusterNum == headClusterNum) break;
		else headClusterNum = newHeadClusterNum;
	}
	// --------------- END of [Merge clusters that have segments in common] --------------- //
	// -------------------------------- END of [Merge head clusters] -------------------------------- //


	// ------------------------------------ Merge tail clusters ------------------------------------ //
	// --------------------- Assign left-out tail clusters --------------------- //
	headTile2ClusterMap.clear();
	for (map<string, vector<int>>::iterator headTileIt = inputProfiledTree.segHeadMap.begin(); headTileIt != inputProfiledTree.segHeadMap.end(); ++headTileIt)
	{
		boost::container::flat_set<int> currTileClusters;
		for (vector<int>::iterator headIt = headTileIt->second.begin(); headIt != headTileIt->second.end(); ++headIt)
		{
			if (inputProfiledTree.headSeg2ClusterMap.find(*headIt) != inputProfiledTree.headSeg2ClusterMap.end())
				currTileClusters.insert(inputProfiledTree.headSeg2ClusterMap.at(*headIt));
		}
		headTile2ClusterMap.insert(pair<string, boost::container::flat_set<int>>(headTileIt->first, currTileClusters));
	}

	for (map<string, vector<int>>::iterator tailTileIt = inputProfiledTree.segTailMap.begin(); tailTileIt != inputProfiledTree.segTailMap.end(); ++tailTileIt)
	{
		for (vector<int>::iterator tailSegIt = tailTileIt->second.begin(); tailSegIt != tailTileIt->second.end(); ++tailSegIt)
		{
			if (labeledTailClusters.find(inputProfiledTree.tailSeg2ClusterMap.at(*tailSegIt)) != labeledTailClusters.end()) continue;
			if (inputProfiledTree.segHeadClusters.at(inputProfiledTree.tailSeg2ClusterMap.at(*tailSegIt)).empty()) // Check if the tile doesn't have head clusters so that its tail clusters are left out in the previous process.
			{
				//cout << tailTileIt->first << endl;
				vector<string> tileLabelStrings;
				vector<int> tileLabels(3);
				boost::split(tileLabelStrings, tailTileIt->first, boost::is_any_of("_"));
				tileLabels[0] = stoi(tileLabelStrings.at(0));
				tileLabels[1] = stoi(tileLabelStrings.at(1));
				tileLabels[2] = stoi(tileLabelStrings.at(2));

				for (int zi = -1; zi <= 1; ++zi)
				{
					for (int yi = -1; yi <= 1; ++yi)
					{
						for (int xi = -1; xi <= 1; ++xi)
						{
							int checkXlabel = tileLabels[0] + xi;
							int checkYlabel = tileLabels[1] + yi;
							int checkZlabel = tileLabels[2] + zi;
							string checkLabel = to_string(checkXlabel) + "_" + to_string(checkYlabel) + "_" + to_string(checkZlabel);
							if (checkLabel == tailTileIt->first) continue;

							if (headTile2ClusterMap.find(checkLabel) == headTile2ClusterMap.end()) continue; 

							boost::container::flat_set<int> currTileHeadClusters = headTile2ClusterMap.at(checkLabel);
							if (currTileHeadClusters.empty()) continue;

							for (boost::container::flat_set<int>::iterator currTileHeadClusterIt = headTile2ClusterMap.at(checkLabel).begin(); currTileHeadClusterIt != headTile2ClusterMap.at(checkLabel).end(); ++currTileHeadClusterIt)
							{
								for (boost::container::flat_set<int>::iterator headSegIt = inputProfiledTree.segHeadClusters.at(*currTileHeadClusterIt).begin(); headSegIt != inputProfiledTree.segHeadClusters.at(*currTileHeadClusterIt).end(); ++headSegIt)
								{
									NeuronSWC* tail1ptr = &(inputProfiledTree.tree.listNeuron[inputProfiledTree.node2LocMap.at(*inputProfiledTree.segs.at(*tailSegIt).tails.begin())]);
									NeuronSWC* head2ptr = &(inputProfiledTree.tree.listNeuron[inputProfiledTree.node2LocMap.at(inputProfiledTree.segs.at(*headSegIt).head)]);
									float dist = sqrtf((tail1ptr->x - head2ptr->x) * (tail1ptr->x - head2ptr->x) + (tail1ptr->y - head2ptr->y) * (tail1ptr->y - head2ptr->y) + (tail1ptr->z - head2ptr->z) * (tail1ptr->z - head2ptr->z));

									if (dist <= distThreshold)
									{
										int targetedTailCluster = inputProfiledTree.tailSeg2ClusterMap.at(*tailSegIt);
										inputProfiledTree.segTailClusters.at(*currTileHeadClusterIt).insert(inputProfiledTree.segTailClusters.at(targetedTailCluster).begin(), inputProfiledTree.segTailClusters.at(targetedTailCluster).end());
										for (boost::container::flat_set<int>::iterator mapIt = inputProfiledTree.segTailClusters.at(*currTileHeadClusterIt).begin(); mapIt != inputProfiledTree.segTailClusters.at(*currTileHeadClusterIt).end(); ++mapIt)
											inputProfiledTree.tailSeg2ClusterMap[*mapIt] = *currTileHeadClusterIt;
										inputProfiledTree.segTailClusters.at(targetedTailCluster).clear();
										labeledTailClusters.insert(targetedTailCluster);

										goto FLOATING_TAIL_CLUSTER_LABELED;
									}
								}
							}
						}
					}
				}
			}

		FLOATING_TAIL_CLUSTER_LABELED:
			continue;
		}
	}
	// ---------------- END of [Assign left-out tail clusters] ---------------- //

	// ----------------- Merge clusters that have segments in common ----------------- // 
	for (boost::container::flat_set<int>::iterator it = labeledTailClusters.begin(); it != labeledTailClusters.end(); ++it)
		if (leftTailClusters.find(*it) != leftTailClusters.end()) leftTailClusters.erase(leftTailClusters.find(*it));

	for (boost::container::flat_set<int>::iterator leftIt = leftTailClusters.begin(); leftIt != leftTailClusters.end(); ++leftIt)
	{
		for (boost::container::flat_set<int>::iterator leftSegIt = inputProfiledTree.segTailClusters.at(*leftIt).begin(); leftSegIt != inputProfiledTree.segTailClusters.at(*leftIt).end(); ++leftSegIt)
		{
			for (boost::container::flat_set<int>::iterator labeledIt = labeledTailClusters.begin(); labeledIt != labeledTailClusters.end(); ++labeledIt)
			{
				for (boost::container::flat_set<int>::iterator labeledSegIt = inputProfiledTree.segTailClusters.at(*labeledIt).begin(); labeledSegIt != inputProfiledTree.segTailClusters.at(*labeledIt).end(); ++labeledSegIt)
				{
					NeuronSWC* tail1ptr = &(inputProfiledTree.tree.listNeuron[inputProfiledTree.node2LocMap.at(*inputProfiledTree.segs.at(*labeledSegIt).tails.begin())]);
					NeuronSWC* tail2ptr = &(inputProfiledTree.tree.listNeuron[inputProfiledTree.node2LocMap.at(*inputProfiledTree.segs.at(*leftSegIt).tails.begin())]);
					float dist = sqrtf((tail1ptr->x - tail2ptr->x) * (tail1ptr->x - tail2ptr->x) + (tail1ptr->y - tail2ptr->y) * (tail1ptr->y - tail2ptr->y) + (tail1ptr->z - tail2ptr->z) * (tail1ptr->z - tail2ptr->z));

					if (dist <= distThreshold)
					{
						inputProfiledTree.segTailClusters.at(*labeledIt).insert(inputProfiledTree.segTailClusters.at(*leftIt).begin(), inputProfiledTree.segTailClusters.at(*leftIt).end());
						for (boost::container::flat_set<int>::iterator mapIt = inputProfiledTree.segTailClusters.at(*leftIt).begin(); mapIt != inputProfiledTree.segTailClusters.at(*leftIt).end(); ++mapIt)
							inputProfiledTree.tailSeg2ClusterMap[*mapIt] = *labeledIt;
						inputProfiledTree.segTailClusters.at(*leftIt).clear();
						labeledTailClusters.insert(*leftIt);

						goto TAIL_CLUSTERS_MERGED;
					}
				}
			}
		}

	TAIL_CLUSTERS_MERGED:
		continue;
	}
	// ------------ END of [Merge clusters that have segments in common] ------------- //
	// -------------------------------- END of [Merge tail clusters] -------------------------------- //

	// delete clusters that are empty of both heads and tails
	vector<ptrdiff_t> delLocs;
	for (boost::container::flat_map<int, boost::container::flat_set<int>>::iterator delIt = inputProfiledTree.segHeadClusters.begin(); delIt != inputProfiledTree.segHeadClusters.end(); ++delIt)
		if (inputProfiledTree.segHeadClusters.at(delIt->first).empty() && inputProfiledTree.segTailClusters.at(delIt->first).empty()) delLocs.push_back(delIt - inputProfiledTree.segHeadClusters.begin());
	sort(delLocs.rbegin(), delLocs.rend());
	for (vector<ptrdiff_t>::iterator delIt = delLocs.begin(); delIt != delLocs.end(); ++delIt)
	{
		inputProfiledTree.segHeadClusters.erase(inputProfiledTree.segHeadClusters.begin() + *delIt);
		inputProfiledTree.segTailClusters.erase(inputProfiledTree.segTailClusters.begin() + *delIt);
	}

	// ------- For debugging purpose -------
	/*profiledTree testTree = inputProfiledTree;
	for (boost::container::flat_map<int, int>::iterator it = testTree.headSeg2ClusterMap.begin(); it != testTree.headSeg2ClusterMap.end(); ++it)
	{
		testTree.tree.listNeuron[testTree.node2LocMap.at(testTree.segs.at(it->first).head)].type = it->second % 9;
		testTree.tree.listNeuron[testTree.node2LocMap.at(testTree.segs.at(it->first).head)].radius = it->second;
	}
	writeSWC_file("C:\\Users\\hsienchik\\Desktop\\Work\\FragTrace\\headEndTest.swc", testTree.tree);
	testTree = inputProfiledTree;
	for (boost::container::flat_map<int, int>::iterator it = testTree.tailSeg2ClusterMap.begin(); it != testTree.tailSeg2ClusterMap.end(); ++it)
	{
		testTree.tree.listNeuron[testTree.node2LocMap.at(*testTree.segs.at(it->first).tails.begin())].type = it->second % 9;
		testTree.tree.listNeuron[testTree.node2LocMap.at(*testTree.segs.at(it->first).tails.begin())].radius = it->second;
	}
	writeSWC_file("C:\\Users\\hsienchik\\Desktop\\Work\\FragTrace\\tailEndTest.swc", testTree.tree);*/
}

void NeuronStructExplorer::getTileBasedSegClusters(profiledTree& inputProfiledTree, float distThreshold)
{
	// -- This method clusters segment heads and tails tile by tile.
	// NOTE: Nodes are confined within their own tiles. Therefore, cluster merging correctness is irrelevant to it.

	inputProfiledTree.segHeadClusters.clear(); 
	inputProfiledTree.segTailClusters.clear(); 
	inputProfiledTree.headSeg2ClusterMap.clear();
	inputProfiledTree.tailSeg2ClusterMap.clear();

	vector<string> tailSegTiles;
	for (map<string, vector<int>>::iterator segTailTileIt = inputProfiledTree.segTailMap.begin(); segTailTileIt != inputProfiledTree.segTailMap.end(); ++segTailTileIt)
		tailSegTiles.push_back(segTailTileIt->first);

	boost::container::flat_set<int> newHeadClusters, newTailClusters; // cluster labels 
	newHeadClusters.clear();
	newTailClusters.clear();
	
	boost::container::flat_set<int> newClusterHeads, newClusterTails; // Head or tail segments in a cluster, these flat_sets are used only when a new cluster is being formed.

	// for all head seg tiles
	for (map<string, vector<int>>::iterator headSegTileIt = inputProfiledTree.segHeadMap.begin(); headSegTileIt != inputProfiledTree.segHeadMap.end(); ++headSegTileIt)
	{
		// ---------------------------------------- processing segment heads ----------------------------------------- //
		newClusterHeads.clear();
		newClusterHeads.insert(*headSegTileIt->second.begin()); // newClusterHeads: a set of head segments in the current new head cluster
		inputProfiledTree.segHeadClusters.insert(pair<int, boost::container::flat_set<int>>(inputProfiledTree.segHeadClusters.size() + 1, newClusterHeads));	
		newHeadClusters.insert(inputProfiledTree.segHeadClusters.size()); // newHeadClusters: a set of new head clusters formed in each tile
		inputProfiledTree.headSeg2ClusterMap.insert(pair<int, int>(*headSegTileIt->second.begin(), inputProfiledTree.segHeadClusters.size()));

		newClusterTails.clear(); // matching the head cluster with its tail cluster counterpart
		inputProfiledTree.segTailClusters.insert(pair<int, boost::container::flat_set<int>>(inputProfiledTree.segHeadClusters.size(), newClusterTails));
		newTailClusters.insert(inputProfiledTree.segHeadClusters.size()); // new empty tail cluster to match head clusters
			
		if (headSegTileIt->second.size() > 1)
		{
			// for all heads in the current tile
			for (vector<int>::iterator headSegIt = headSegTileIt->second.begin() + 1; headSegIt != headSegTileIt->second.end(); ++headSegIt)
			{
				// for all newly formed clusters in the current tile
				for (boost::container::flat_set<int>::iterator newClusterIt = newHeadClusters.begin(); newClusterIt != newHeadClusters.end(); ++newClusterIt)
				{
					// for all heads that are recorded in the current new head cluster
					for (boost::container::flat_set<int>::iterator clusterHeadIt = inputProfiledTree.segHeadClusters.at(*newClusterIt).begin(); clusterHeadIt != inputProfiledTree.segHeadClusters.at(*newClusterIt).end(); ++clusterHeadIt)
					{
						NeuronSWC* node1Ptr = &inputProfiledTree.tree.listNeuron[inputProfiledTree.node2LocMap.at(inputProfiledTree.segs.at(*headSegIt).head)];
						NeuronSWC* node2Ptr = &inputProfiledTree.tree.listNeuron[inputProfiledTree.node2LocMap.at(inputProfiledTree.segs.at(*clusterHeadIt).head)];
						float dist = sqrtf((node1Ptr->x - node2Ptr->x) * (node1Ptr->x - node2Ptr->x) + (node1Ptr->y - node2Ptr->y) * (node1Ptr->y - node2Ptr->y) + (node1Ptr->z - node2Ptr->z) * (node1Ptr->z - node2Ptr->z));

						if (dist <= distThreshold)
						{
							inputProfiledTree.segHeadClusters.at(*newClusterIt).insert(*headSegIt);
							inputProfiledTree.headSeg2ClusterMap.insert(pair<int, int>(*headSegIt, *newClusterIt));
							goto FOUND_HEAD_CLUSTER;
						}
					}
				}

				newClusterHeads.clear();
				newClusterHeads.insert(*headSegIt);
				inputProfiledTree.segHeadClusters.insert(pair<int, boost::container::flat_set<int>>(inputProfiledTree.segHeadClusters.size() + 1, newClusterHeads));
				newHeadClusters.insert(inputProfiledTree.segHeadClusters.size());
				inputProfiledTree.headSeg2ClusterMap.insert(pair<int, int>(*headSegIt, inputProfiledTree.segHeadClusters.size()));

				inputProfiledTree.segTailClusters.insert(pair<int, boost::container::flat_set<int>>(inputProfiledTree.segHeadClusters.size(), newClusterTails));
				newTailClusters.insert(inputProfiledTree.segHeadClusters.size());

			FOUND_HEAD_CLUSTER:
				continue;
			}
		}
		// ------------------------------------ END of [processing segment heads] ------------------------------------ //
		  
		// ---------------------------------------- processing segment tails ----------------------------------------- // 
		// ----------------- tail - head ----------------- //
		newClusterHeads.clear(); // -> This is for tail cluster's counterpart if it doesn't exist.
		if (find(tailSegTiles.begin(), tailSegTiles.end(), headSegTileIt->first) != tailSegTiles.end()) // check tail tiles that share with the same keys with head tiles first
		{
			// for all tails in the current tile
			vector<int> unlabeledTailSegs = inputProfiledTree.segTailMap.at(headSegTileIt->first);
			set<int> labeledTailSegsSet;
			vector<int> labeledTailSegs;
			for (vector<int>::iterator tailSegIt = inputProfiledTree.segTailMap.at(headSegTileIt->first).begin(); tailSegIt != inputProfiledTree.segTailMap.at(headSegTileIt->first).end(); ++tailSegIt)
			{
				// for all newly formed head clusters in the current tile
				for (boost::container::flat_set<int>::iterator currHeadClusterIt = newHeadClusters.begin(); currHeadClusterIt != newHeadClusters.end(); ++currHeadClusterIt)
				{
					// for all heads in the current new head cluster
					for (boost::container::flat_set<int>::iterator headSegIt = inputProfiledTree.segHeadClusters.at(*currHeadClusterIt).begin(); headSegIt != inputProfiledTree.segHeadClusters.at(*currHeadClusterIt).end(); ++headSegIt)
					{	
						NeuronSWC* node1Ptr = &inputProfiledTree.tree.listNeuron[inputProfiledTree.node2LocMap.at(inputProfiledTree.segs.at(*headSegIt).head)];
						NeuronSWC* node2Ptr = &inputProfiledTree.tree.listNeuron[inputProfiledTree.node2LocMap.at(*inputProfiledTree.segs.at(*tailSegIt).tails.begin())];
						float dist = sqrtf((node1Ptr->x - node2Ptr->x) * (node1Ptr->x - node2Ptr->x) + (node1Ptr->y - node2Ptr->y) * (node1Ptr->y - node2Ptr->y) + (node1Ptr->z - node2Ptr->z) * (node1Ptr->z - node2Ptr->z));

						if (dist <= distThreshold)
						{
							inputProfiledTree.segTailClusters.at(*currHeadClusterIt).insert(*tailSegIt); // There must be a tail cluster correspondging to its head cluster counterpart. This part is done in [processing segment heads] section.
							labeledTailSegsSet.insert(*tailSegIt);
							inputProfiledTree.tailSeg2ClusterMap.insert(pair<int, int>(*tailSegIt, *currHeadClusterIt));
							
							goto TAIL_SEG_LABELED;
						}
					}
				}

			TAIL_SEG_LABELED:
				continue;
			}
			for (set<int>::iterator delIt = labeledTailSegsSet.begin(); delIt != labeledTailSegsSet.end(); ++delIt)
				unlabeledTailSegs.erase(find(unlabeledTailSegs.begin(), unlabeledTailSegs.end(), *delIt));
			// ---------- END of [tail - head] ---------- //

			// --------------- tail - tail -------------- //
			while (1)
			{
				for (vector<int>::iterator leftTailIt = unlabeledTailSegs.begin(); leftTailIt != unlabeledTailSegs.end(); ++leftTailIt)
				{
					for (set<int>::iterator labeledTailIt = labeledTailSegsSet.begin(); labeledTailIt != labeledTailSegsSet.end(); ++labeledTailIt)
					{
						NeuronSWC* node1Ptr = &inputProfiledTree.tree.listNeuron[inputProfiledTree.node2LocMap.at(*inputProfiledTree.segs.at(*leftTailIt).tails.begin())];
						NeuronSWC* node2Ptr = &inputProfiledTree.tree.listNeuron[inputProfiledTree.node2LocMap.at(*inputProfiledTree.segs.at(*labeledTailIt).tails.begin())];
						float dist = sqrtf((node1Ptr->x - node2Ptr->x) * (node1Ptr->x - node2Ptr->x) + (node1Ptr->y - node2Ptr->y) * (node1Ptr->y - node2Ptr->y) + (node1Ptr->z - node2Ptr->z) * (node1Ptr->z - node2Ptr->z));

						if (dist <= distThreshold)
						{
							inputProfiledTree.segTailClusters.at(inputProfiledTree.tailSeg2ClusterMap.at(*labeledTailIt)).insert(*leftTailIt); // There must be a tail cluster correspondging to its head cluster counterpart. This part is done in [processing segment heads] section.
							inputProfiledTree.tailSeg2ClusterMap.insert(pair<int, int>(*leftTailIt, inputProfiledTree.tailSeg2ClusterMap.at(*labeledTailIt)));
							unlabeledTailSegs.erase(find(unlabeledTailSegs.begin(), unlabeledTailSegs.end(), *leftTailIt));
							goto TAIL_SEG_ERASED;
						}
					}
				}
				break;

			TAIL_SEG_ERASED:
				continue;
			}
			// --------- END of [tail - tail] ---------- //

			// -------------- tails that need to have new tail clusters -------------- //
			if (unlabeledTailSegs.size() != 0)
			{
				newClusterTails.clear();
				newClusterTails.insert(unlabeledTailSegs.at(0));
				inputProfiledTree.segTailClusters.insert(pair<int, boost::container::flat_set<int>>(inputProfiledTree.segTailClusters.size() + 1, newClusterTails));
				newTailClusters.insert(inputProfiledTree.segTailClusters.size());
				inputProfiledTree.tailSeg2ClusterMap.insert(pair<int, int>(unlabeledTailSegs.at(0), inputProfiledTree.segTailClusters.size()));
				inputProfiledTree.segHeadClusters.insert(pair<int, boost::container::flat_set<int>>(inputProfiledTree.segTailClusters.size(), newClusterHeads));

				if (unlabeledTailSegs.size() > 1)
				{
					for (vector<int>::iterator it = unlabeledTailSegs.begin() + 1; it != unlabeledTailSegs.end(); ++it)
					{
						for (boost::container::flat_set<int>::iterator newTailClusterIt = newTailClusters.begin(); newTailClusterIt != newTailClusters.end(); ++newTailClusterIt)
						{
							for (boost::container::flat_set<int>::iterator existingTailIt = inputProfiledTree.segTailClusters.at(*newTailClusterIt).begin(); existingTailIt != inputProfiledTree.segTailClusters.at(*newTailClusterIt).end(); ++existingTailIt)
							{
								NeuronSWC* node1Ptr = &inputProfiledTree.tree.listNeuron[inputProfiledTree.node2LocMap.at(*inputProfiledTree.segs.at(*it).tails.begin())];
								NeuronSWC* node2Ptr = &inputProfiledTree.tree.listNeuron[inputProfiledTree.node2LocMap.at(*inputProfiledTree.segs.at(*existingTailIt).tails.begin())];
								float dist = sqrtf((node1Ptr->x - node2Ptr->x) * (node1Ptr->x - node2Ptr->x) + (node1Ptr->y - node2Ptr->y) * (node1Ptr->y - node2Ptr->y) + (node1Ptr->z - node2Ptr->z) * (node1Ptr->z - node2Ptr->z));

								if (dist <= distThreshold)
								{
									inputProfiledTree.segTailClusters.at(*newTailClusterIt).insert(*it); // There must be a tail cluster correspondging to its head cluster counterpart. This part is done in [processing segment heads] section.
									inputProfiledTree.tailSeg2ClusterMap.insert(pair<int, int>(*it, *newTailClusterIt));
									goto FOUND_TAIL_CLUSTER;
								}
							}
						}

						newClusterTails.clear();
						newClusterTails.insert(*it);
						inputProfiledTree.segTailClusters.insert(pair<int, boost::container::flat_set<int>>(inputProfiledTree.segTailClusters.size() + 1, newClusterTails));
						newTailClusters.insert(inputProfiledTree.segTailClusters.size());
						inputProfiledTree.tailSeg2ClusterMap.insert(pair<int, int>(*it, inputProfiledTree.segTailClusters.size()));

						inputProfiledTree.segHeadClusters.insert(pair<int, boost::container::flat_set<int>>(inputProfiledTree.segTailClusters.size(), newClusterHeads));
						newHeadClusters.insert(inputProfiledTree.segTailClusters.size());


					FOUND_TAIL_CLUSTER:
						continue;
					}
				}
			}
			// ------------ END of [tails that need to have new tail clusters] -------------- //

			tailSegTiles.erase(find(tailSegTiles.begin(), tailSegTiles.end(), headSegTileIt->first)); // eliminate tail tiles that are processed
		}

		newHeadClusters.clear();
		newTailClusters.clear();
	}
	// ------------------------------------ END of [processing segment tails] ------------------------------------ //

	// For those tail tiles that do not have their head tile counter parts:
	newClusterHeads.clear();
	while (!tailSegTiles.empty())
	{
		for (vector<string>::iterator tailTileCheckIt = tailSegTiles.begin(); tailTileCheckIt != tailSegTiles.end(); ++tailTileCheckIt)
		{
			newHeadClusters.clear(); 
			newTailClusters.clear();

			vector<int> currTileTailSegs = inputProfiledTree.segTailMap.at(*tailTileCheckIt);
			if (currTileTailSegs.size() == 1)
			{
				newClusterTails.clear();
				newClusterTails.insert(*currTileTailSegs.begin());
				inputProfiledTree.segTailClusters.insert(pair<int, boost::container::flat_set<int>>(inputProfiledTree.segTailClusters.size() + 1, newClusterTails));
				newTailClusters.insert(inputProfiledTree.segTailClusters.size());
				inputProfiledTree.tailSeg2ClusterMap.insert(pair<int, int>(*currTileTailSegs.begin(), inputProfiledTree.segTailClusters.size()));

				inputProfiledTree.segHeadClusters.insert(pair<int, boost::container::flat_set<int>>(inputProfiledTree.segTailClusters.size(), newClusterHeads));
				newHeadClusters.insert(inputProfiledTree.segTailClusters.size());
			}
			else
			{
				newClusterTails.clear();
				newClusterTails.insert(*currTileTailSegs.begin());
				inputProfiledTree.segTailClusters.insert(pair<int, boost::container::flat_set<int>>(inputProfiledTree.segTailClusters.size() + 1, newClusterTails));
				newTailClusters.insert(inputProfiledTree.segTailClusters.size());
				inputProfiledTree.tailSeg2ClusterMap.insert(pair<int, int>(*currTileTailSegs.begin(), inputProfiledTree.segTailClusters.size()));

				inputProfiledTree.segHeadClusters.insert(pair<int, boost::container::flat_set<int>>(inputProfiledTree.segTailClusters.size(), newClusterHeads));
				newHeadClusters.insert(inputProfiledTree.segTailClusters.size());

				// for all tails in the current tile
				for (vector<int>::iterator tailIt = currTileTailSegs.begin() + 1; tailIt != currTileTailSegs.end(); ++tailIt)
				{
					// for all new tail clusters
					for (boost::container::flat_set<int>::iterator clusterIt = newTailClusters.begin(); clusterIt != newTailClusters.end(); ++clusterIt)
					{
						// for all tails in the current new tail cluster
						for (boost::container::flat_set<int>::iterator tailIt2 = inputProfiledTree.segTailClusters.at(*clusterIt).begin(); tailIt2 != inputProfiledTree.segTailClusters.at(*clusterIt).end(); ++tailIt2)
						{
							NeuronSWC* node1Ptr = &inputProfiledTree.tree.listNeuron[inputProfiledTree.node2LocMap.at(*inputProfiledTree.segs.at(*tailIt).tails.begin())];
							NeuronSWC* node2Ptr = &inputProfiledTree.tree.listNeuron[inputProfiledTree.node2LocMap.at(*inputProfiledTree.segs.at(*tailIt2).tails.begin())];
							float dist = sqrtf((node1Ptr->x - node2Ptr->x) * (node1Ptr->x - node2Ptr->x) + (node1Ptr->y - node2Ptr->y) * (node1Ptr->y - node2Ptr->y) + (node1Ptr->z - node2Ptr->z) * (node1Ptr->z - node2Ptr->z));

							if (dist <= distThreshold)
							{
								inputProfiledTree.segTailClusters.at(*clusterIt).insert(*tailIt);
								inputProfiledTree.tailSeg2ClusterMap.insert(pair<int, int>(*tailIt, *clusterIt));
								goto FOUND_CLUSTER_TAIL2;
							}
						}
					}
					newClusterTails.clear();
					newClusterTails.insert(*tailIt);
					inputProfiledTree.segTailClusters.insert(pair<int, boost::container::flat_set<int>>(inputProfiledTree.segTailClusters.size() + 1, newClusterTails));
					newTailClusters.insert(inputProfiledTree.segTailClusters.size());
					inputProfiledTree.tailSeg2ClusterMap.insert(pair<int, int>(*tailIt, inputProfiledTree.segTailClusters.size()));

					inputProfiledTree.segHeadClusters.insert(pair<int, boost::container::flat_set<int>>(inputProfiledTree.segTailClusters.size(), newClusterHeads));
					newHeadClusters.insert(inputProfiledTree.segTailClusters.size());

				FOUND_CLUSTER_TAIL2:
					continue;
				}
			}

			tailSegTiles.erase(find(tailSegTiles.begin(), tailSegTiles.end(), *tailTileCheckIt));
			break; // This break is needed as tailSegTiles' size is changing.
		}
	}

	// ------- For debugging purpose -------
	/*profiledTree testTree = inputProfiledTree;
	for (boost::container::flat_map<int, boost::container::flat_set<int>>::iterator it = testTree.segHeadClusters.begin(); it != testTree.segHeadClusters.end(); ++it)
	{
		for (boost::container::flat_set<int>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			testTree.tree.listNeuron[testTree.node2LocMap.at(testTree.segs.at(*it2).head)].type = it->first % 9;
			testTree.tree.listNeuron[testTree.node2LocMap.at(testTree.segs.at(*it2).head)].radius = it->first;
		}
	}
	writeSWC_file("C:\\Users\\hsienchik\\Desktop\\Work\\FragTrace\\tiled_headEndTest_clusterIndex.swc", testTree.tree);
	testTree = inputProfiledTree;
	for (boost::container::flat_map<int, boost::container::flat_set<int>>::iterator it = testTree.segTailClusters.begin(); it != testTree.segTailClusters.end(); ++it)
	{
		for (boost::container::flat_set<int>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			testTree.tree.listNeuron[testTree.node2LocMap.at(*testTree.segs.at(*it2).tails.begin())].type = it->first % 9;
			testTree.tree.listNeuron[testTree.node2LocMap.at(*testTree.segs.at(*it2).tails.begin())].radius = it->first;
		}
	}
	writeSWC_file("C:\\Users\\hsienchik\\Desktop\\Work\\FragTrace\\tiled_tailEndTest_clusterIndex.swc", testTree.tree);
	testTree = inputProfiledTree;
	for (boost::container::flat_map<int, int>::iterator it = testTree.headSeg2ClusterMap.begin(); it != testTree.headSeg2ClusterMap.end(); ++it)
	{
		testTree.tree.listNeuron[testTree.node2LocMap.at(testTree.segs.at(it->first).head)].type = it->second % 9;
		testTree.tree.listNeuron[testTree.node2LocMap.at(testTree.segs.at(it->first).head)].radius = it->second;
	}
	writeSWC_file("C:\\Users\\hsienchik\\Desktop\\Work\\FragTrace\\tiled_headEndTest.swc", testTree.tree);
	testTree = inputProfiledTree;
	for (boost::container::flat_map<int, int>::iterator it = testTree.tailSeg2ClusterMap.begin(); it != testTree.tailSeg2ClusterMap.end(); ++it)
	{
		testTree.tree.listNeuron[testTree.node2LocMap.at(*testTree.segs.at(it->first).tails.begin())].type = it->second % 9;
		testTree.tree.listNeuron[testTree.node2LocMap.at(*testTree.segs.at(it->first).tails.begin())].radius = it->second;
	}
	writeSWC_file("C:\\Users\\hsienchik\\Desktop\\Work\\FragTrace\\tiled_tailEndTest.swc", testTree.tree);*/
}

void NeuronStructExplorer::segmentDecompose(NeuronTree* inputTreePtr)
{
	// -- This function used get_link_map and decompose in v_neuronswc.cpp to get segment hierarchical information.

	this->segmentList.clear();
	this->segmentList = NeuronTree__2__V_NeuronSWC_list(*inputTreePtr);
}
/* =============================== END of [Constructors and Basic Data/Function Members] =============================== */



/* ================================== Neuron Struct Processing Functions ================================== */
void NeuronStructExplorer::treeUpSample(const profiledTree& inputProfiledTree, profiledTree& outputProfiledTree, float intervalLength)
{
	// -- This method creates interpolated nodes in between each pair of 2 adjacent nodes on the input tree. 

	size_t maxNodeID = 0;
	for (QList<NeuronSWC>::const_iterator it = inputProfiledTree.tree.listNeuron.begin(); it != inputProfiledTree.tree.listNeuron.end(); ++it)
		if (it->n > maxNodeID) maxNodeID = it->n;
	
	for (map<int, segUnit>::const_iterator segIt = inputProfiledTree.segs.begin(); segIt != inputProfiledTree.segs.end(); ++segIt)
	{
		QList<NeuronSWC> newSegNodes;
		map<int, vector<QList<NeuronSWC>>> interpolatedNodeMap;
		for (QList<NeuronSWC>::const_iterator nodeIt = segIt->second.nodes.begin(); nodeIt != segIt->second.nodes.end() - 1; ++nodeIt)
		{
			vector<size_t> childLocs = segIt->second.seg_childLocMap.at(nodeIt->n);
			for (vector<size_t>::iterator childLocIt = childLocs.begin(); childLocIt != childLocs.end(); ++childLocIt)
			{
				float dist = sqrt((segIt->second.nodes.at(*childLocIt).x - nodeIt->x) * (segIt->second.nodes.at(*childLocIt).x - nodeIt->x) +
								  (segIt->second.nodes.at(*childLocIt).y - nodeIt->y) * (segIt->second.nodes.at(*childLocIt).y - nodeIt->y) +
								  (segIt->second.nodes.at(*childLocIt).z - nodeIt->z) * (segIt->second.nodes.at(*childLocIt).z - nodeIt->z));
				int intervals = int(dist / intervalLength);
				float intervalX = (segIt->second.nodes.at(*childLocIt).x - nodeIt->x) / float(intervals);
				float intervalY = (segIt->second.nodes.at(*childLocIt).y - nodeIt->y) / float(intervals);
				float intervalZ = (segIt->second.nodes.at(*childLocIt).z - nodeIt->z) / float(intervals);

				QList<NeuronSWC> interpolatedNodes;
				for (int i = 1; i < intervals; ++i)
				{
					NeuronSWC newNode;
					newNode.x = nodeIt->x + intervalX * float(i);
					newNode.y = nodeIt->y + intervalY * float(i);
					newNode.z = nodeIt->z + intervalZ * float(i);
					newNode.type = nodeIt->type;
					++maxNodeID;
					newNode.n = maxNodeID;
					newNode.parent = maxNodeID - 1;
					interpolatedNodes.push_back(newNode);
				}
				interpolatedNodes.push_back(segIt->second.nodes.at(*childLocIt));
				if (interpolatedNodes.size() >= 2)
				{
					interpolatedNodes.back().parent = (interpolatedNodes.end() - 2)->n;
					interpolatedNodes.begin()->parent = nodeIt->n;
				}

				interpolatedNodeMap[nodeIt->n].push_back(interpolatedNodes);
			}	
		}
		for (map<int, vector<QList<NeuronSWC>>>::iterator mapIt = interpolatedNodeMap.begin(); mapIt != interpolatedNodeMap.end(); ++mapIt)
			for (vector<QList<NeuronSWC>>::iterator qlistIt = mapIt->second.begin(); qlistIt != mapIt->second.end(); ++qlistIt) newSegNodes.append(*qlistIt);
		newSegNodes.push_front(segIt->second.nodes.at(segIt->second.seg_nodeLocMap.at(segIt->second.head)));

		segUnit newSegUnit;
		newSegUnit.nodes = newSegNodes;
		outputProfiledTree.segs.insert(pair<int, segUnit>(segIt->first, newSegUnit));
		outputProfiledTree.tree.listNeuron.append(newSegNodes);
	}
}

profiledTree NeuronStructExplorer::treeDownSample(const profiledTree& inputProfiledTree, int nodeInterval)
{
	// -- This method "down samples" the input tree segment by segment. 
	// -- A recursive down sampling method [NeuronStructExplorer::rc_segDownSample] is called in this function to deal with all possible braching points in each segment.
	// -- NOTE, this method is essentially used for straightening / smoothing segments when there are too many zigzagging.  

	NeuronTree outputTree;
	QList<NeuronSWC> currSegOutputList;
	for (map<int, segUnit>::const_iterator it = inputProfiledTree.segs.begin(); it != inputProfiledTree.segs.end(); ++it)
	{
		//if (it->second.seg_childLocMap.empty()) continue; => Using this line is not safe. Can occasionally result in program crash.
															// The safety of seg_childLocMap needs to be investigated later.
		if (it->second.nodes.size() <= 3) continue;

		currSegOutputList.clear();
		currSegOutputList.push_back(*(it->second.nodes.begin()));
		this->rc_segDownSample(it->second, currSegOutputList, it->second.head, nodeInterval);
		outputTree.listNeuron.append(currSegOutputList);
	}

	vector<size_t> delLocs;
	for (QList<NeuronSWC>::iterator nodeIt = outputTree.listNeuron.begin(); nodeIt != outputTree.listNeuron.end(); ++nodeIt)
		if (nodeIt->n == nodeIt->parent) delLocs.push_back(size_t(nodeIt - outputTree.listNeuron.begin()));
	sort(delLocs.rbegin(), delLocs.rend());
	for (vector<size_t>::iterator delIt = delLocs.begin(); delIt != delLocs.end(); ++delIt) outputTree.listNeuron.erase(outputTree.listNeuron.begin() + ptrdiff_t(*delIt));
	profiledTree outputProfiledTree(outputTree);
	
	return outputProfiledTree;
}

void NeuronStructExplorer::rc_segDownSample(const segUnit& inputSeg, QList<NeuronSWC>& outputNodeList, int branchigNodeID, int interval)
{
	int currNodeID = 0, count = 0;
	for (vector<size_t>::const_iterator childIt = inputSeg.seg_childLocMap.at(branchigNodeID).begin(); childIt != inputSeg.seg_childLocMap.at(branchigNodeID).end(); ++childIt)
	{
		outputNodeList.push_back(inputSeg.nodes.at(*childIt));
		outputNodeList.last().parent = branchigNodeID;
		currNodeID = inputSeg.nodes.at(*childIt).n;
		count = 0;
		while (inputSeg.seg_childLocMap.at(currNodeID).size() > 0)
		{
			if (inputSeg.seg_childLocMap.at(currNodeID).size() >= 2) // branching point found, function recursively called
			{
				outputNodeList.push_back(inputSeg.nodes.at(inputSeg.seg_nodeLocMap.at(currNodeID)));
				outputNodeList.last().parent = (outputNodeList.end() - 2)->n;
				rc_segDownSample(inputSeg, outputNodeList, currNodeID, interval);
				break;
			}
			
			++count;
			currNodeID = inputSeg.nodes.at(*(inputSeg.seg_childLocMap.at(currNodeID).begin())).n;
			if (count % interval == 0 || inputSeg.seg_childLocMap.at(currNodeID).size() == 0) // The tail(s) of the segment needs to stay.
			{
				outputNodeList.push_back(inputSeg.nodes.at(inputSeg.seg_nodeLocMap.at(currNodeID)));
				outputNodeList.last().parent = (outputNodeList.end() - 2)->n;
			}
		}
	}
}
/* ================================== END of [Neuron Struct Processing Functions] ================================== */



/* ========================================= Auto-tracing Related Neuron Struct Functions ========================================= */
NeuronTree NeuronStructExplorer::SWC2MSTtree(NeuronTree const& inputTree)
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

NeuronTree NeuronStructExplorer::SWC2MSTtree_tiled(NeuronTree const& inputTree, float tileLength, float zDivideNum)
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
		NeuronTree tileMSTtree = this->SWC2MSTtree(tileTree);

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

NeuronTree NeuronStructExplorer::MSTbranchBreak(const profiledTree& inputProfiledTree, double spikeThre, bool spikeRemove)
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

boost::container::flat_map<int, vector<segPairProfile>> NeuronStructExplorer::getSegConnPairs_cluster(const profiledTree& inputProfiledTree)
{
	boost::container::flat_map<int, vector<segPairProfile>> outputSegPairsMap;

	for (boost::container::flat_map<int, boost::container::flat_set<int>>::const_iterator clusterIt = inputProfiledTree.segHeadClusters.begin(); clusterIt != inputProfiledTree.segHeadClusters.end(); ++clusterIt)
	{
		boost::container::flat_set<int> segHeads = clusterIt->second;
		boost::container::flat_set<int> segTails = inputProfiledTree.segTailClusters.at(clusterIt->first);

		if (segHeads.size() + segTails.size() >= 2)
		{
			vector<segPairProfile> segPairs;

			if (segTails.size() == 0)
			{
				for (boost::container::flat_set<int>::const_iterator headSegIt1 = clusterIt->second.begin(); headSegIt1 != clusterIt->second.end() - 1; ++headSegIt1)
				{
					for (boost::container::flat_set<int>::const_iterator headSegIt2 = headSegIt1 + 1; headSegIt2 != clusterIt->second.end(); ++headSegIt2)
					{
						segPairProfile newSegPair(inputProfiledTree.segs.at(*headSegIt1), inputProfiledTree.segs.at(*headSegIt2), head_head);
						segPairs.push_back(newSegPair);
					}
				}
				outputSegPairsMap.insert(pair<int, vector<segPairProfile>>(clusterIt->first, segPairs));
			}
			else if (segHeads.size() == 0)
			{
				for (boost::container::flat_set<int>::const_iterator tailSegIt1 = inputProfiledTree.segTailClusters.at(clusterIt->first).begin(); tailSegIt1 != inputProfiledTree.segTailClusters.at(clusterIt->first).end() - 1; ++tailSegIt1)
				{
					for (boost::container::flat_set<int>::const_iterator tailSegIt2 = tailSegIt1 + 1; tailSegIt2 != inputProfiledTree.segTailClusters.at(clusterIt->first).end(); ++tailSegIt2)
					{
						segPairProfile newSegPair(inputProfiledTree.segs.at(*tailSegIt1), inputProfiledTree.segs.at(*tailSegIt2), tail_tail);
						segPairs.push_back(newSegPair);
					}
				}
				outputSegPairsMap.insert(pair<int, vector<segPairProfile>>(clusterIt->first, segPairs));
			}
			else
			{
				for (boost::container::flat_set<int>::const_iterator headSegIt1 = clusterIt->second.begin(); headSegIt1 != clusterIt->second.end() - 1; ++headSegIt1)
				{
					for (boost::container::flat_set<int>::const_iterator headSegIt2 = headSegIt1 + 1; headSegIt2 != clusterIt->second.end(); ++headSegIt2)
					{
						segPairProfile newSegPair(inputProfiledTree.segs.at(*headSegIt1), inputProfiledTree.segs.at(*headSegIt2), head_head);
						segPairs.push_back(newSegPair);
					}
				}

				for (boost::container::flat_set<int>::const_iterator tailSegIt1 = inputProfiledTree.segTailClusters.at(clusterIt->first).begin(); tailSegIt1 != inputProfiledTree.segTailClusters.at(clusterIt->first).end() - 1; ++tailSegIt1)
				{
					for (boost::container::flat_set<int>::const_iterator tailSegIt2 = tailSegIt1 + 1; tailSegIt2 != inputProfiledTree.segTailClusters.at(clusterIt->first).end(); ++tailSegIt2)
					{
						segPairProfile newSegPair(inputProfiledTree.segs.at(*tailSegIt1), inputProfiledTree.segs.at(*tailSegIt2), tail_tail);
						segPairs.push_back(newSegPair);
					}
				}

				for (boost::container::flat_set<int>::const_iterator headSegIt = clusterIt->second.begin(); headSegIt != clusterIt->second.end(); ++headSegIt)
				{
					for (boost::container::flat_set<int>::const_iterator tailSegIt = inputProfiledTree.segTailClusters.at(clusterIt->first).begin(); tailSegIt != inputProfiledTree.segTailClusters.at(clusterIt->first).end(); ++tailSegIt)
					{
						segPairProfile newSegPair(inputProfiledTree.segs.at(*headSegIt), inputProfiledTree.segs.at(*tailSegIt), head_tail);
						segPairs.push_back(newSegPair);
					}
				}

				outputSegPairsMap.insert(pair<int, vector<segPairProfile>>(clusterIt->first, segPairs));
			}
		}
	}

	return outputSegPairsMap;
}

profiledTree NeuronStructExplorer::connectLongNeurite(const profiledTree& inputProfiledTree, float distThreshold)
{
	profiledTree outputProfiledTree = inputProfiledTree;
	this->getSegHeadTailClusters(outputProfiledTree, distThreshold);
	boost::container::flat_map<int, vector<segPairProfile>> segPairsMap = this->getSegConnPairs_cluster(outputProfiledTree);
	
	vector<int> newSegIDs;
	set<int> connectedSegIDs;
	for (boost::container::flat_map<int, vector<segPairProfile>>::iterator it = segPairsMap.begin(); it != segPairsMap.end(); ++it)
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

				vector<pair<float, float>> axialVecPair = NeuronStructExplorer::getVectorLocPair(seg1Tail, seg1Head);
				vector<pair<float, float>> projectingVecPair = NeuronStructExplorer::getVectorLocPair(seg2Head, seg2Tail);
				vector<pair<float, float>> projectedVecPair = NeuronStructExplorer::getProjectedVector(axialVecPair, projectingVecPair);
				float overlapCheck = (projectingVecPair.begin()->first - seg1Head.x) * (seg1Head.x - seg1Tail.x) + 
									 ((projectingVecPair.begin() + 1)->first - seg1Head.y) * (seg1Head.y - seg1Tail.y) + ((projectingVecPair.begin() + 2)->first - seg1Head.z) * (seg1Head.z - seg1Tail.z);
				if (overlapCheck < 0) continue;

				segUnit newSeg = this->segUnitConnect_executer(*it->second.begin()->seg1Ptr, *it->second.begin()->seg2Ptr, head_head);
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

				vector<pair<float, float>> axialVecPair = NeuronStructExplorer::getVectorLocPair(seg1Tail, seg1Head);
				vector<pair<float, float>> projectingVecPair = NeuronStructExplorer::getVectorLocPair(seg2Tail, seg2Head);
				vector<pair<float, float>> projectedVecPair = NeuronStructExplorer::getProjectedVector(axialVecPair, projectingVecPair);
				float overlapCheck = (projectingVecPair.begin()->first - seg1Head.x) * (seg1Head.x - seg1Tail.x) +
									 ((projectingVecPair.begin() + 1)->first - seg1Head.y) * (seg1Head.y - seg1Tail.y) + ((projectingVecPair.begin() + 2)->first - seg1Head.z) * (seg1Head.z - seg1Tail.z);
				if (overlapCheck < 0) continue;

				segUnit newSeg = this->segUnitConnect_executer(*it->second.begin()->seg1Ptr, *it->second.begin()->seg2Ptr, head_tail);
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

				vector<pair<float, float>> axialVecPair = NeuronStructExplorer::getVectorLocPair(seg1Head, seg1Tail);
				vector<pair<float, float>> projectingVecPair = NeuronStructExplorer::getVectorLocPair(seg2Tail, seg2Head);
				vector<pair<float, float>> projectedVecPair = NeuronStructExplorer::getProjectedVector(axialVecPair, projectingVecPair);
				float overlapCheck = (projectingVecPair.begin()->first - seg1Tail.x) * (seg1Tail.x - seg1Head.x) +
									 ((projectingVecPair.begin() + 1)->first - seg1Tail.y) * (seg1Tail.y - seg1Head.y) + ((projectingVecPair.begin() + 2)->first - seg1Tail.z) * (seg1Tail.z - seg1Head.z);
				if (overlapCheck < 0) continue;

				segUnit newSeg = this->segUnitConnect_executer(*it->second.begin()->seg1Ptr, *it->second.begin()->seg2Ptr, tail_tail);
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

	this->profiledTreeReInit(outputProfiledTree);
	return outputProfiledTree;
}

profiledTree NeuronStructExplorer::itered_connectLongNeurite(profiledTree& inputProfiledTree, float distThreshold)
{
	cout << "iteration 1 " << endl;
	int iterCount = 1;
	profiledTree elongatedTree = this->connectLongNeurite(inputProfiledTree, distThreshold);
	while (elongatedTree.segs.size() != inputProfiledTree.segs.size())
	{
		inputProfiledTree = elongatedTree;

		++iterCount;
		cout << "iteration " << iterCount << " " << endl;
		elongatedTree = this->connectLongNeurite(inputProfiledTree);
	}
	cout << endl;

	return elongatedTree;
}

profiledTree NeuronStructExplorer::segElongate_cluster(const profiledTree& inputProfiledTree)
{
	// -- This method looks segments to connect within each segment end cluster. 
	// -- NOTE, any connected segment will be excluded from the rest of the process in order to avoid erroneous repeated connection.
	// -- To finish connecting fragments in the whole tree, NeuronStrucExplorer::itered_segElongate_cluster is called to run through this method iteratively.

	profiledTree outputProfiledTree = inputProfiledTree;
	this->getSegHeadTailClusters(outputProfiledTree);

	// -- The following block is for debugging purpose: segment ends clustering correctness
	for (boost::container::flat_map<int, boost::container::flat_set<int>>::iterator it = outputProfiledTree.segHeadClusters.begin(); it != outputProfiledTree.segHeadClusters.end(); ++it)
	{
		for (boost::container::flat_set<int>::iterator segIt = it->second.begin(); segIt != it->second.end(); ++segIt)
			outputProfiledTree.tree.listNeuron[outputProfiledTree.node2LocMap.at(outputProfiledTree.segs.at(*segIt).head)].type = it->first % 9;
			
		for (boost::container::flat_set<int>::iterator tailSegIt = outputProfiledTree.segTailClusters.at(it->first).begin(); tailSegIt != outputProfiledTree.segTailClusters.at(it->first).end(); ++tailSegIt)
			outputProfiledTree.tree.listNeuron[outputProfiledTree.node2LocMap.at(*outputProfiledTree.segs.at(*tailSegIt).tails.begin())].type = it->first % 9;
	}
	//writeSWC_file("C:\\Users\\hsienchik\\Desktop\\Work\\FragTrace\\newConnect_cluster.swc", outputProfiledTree.tree);
	
	map<int, segUnit> allNewSegs;
	int maxInputSegID = outputProfiledTree.segs.rbegin()->first;
	boost::container::flat_set<int> headSegs;
	boost::container::flat_set<int> tailSegs;
	vector<segPairProfile> segPairs;

	// --------- For every cluster, identify and connect the pair of segments that is qualified to be connected --------- //
	for (boost::container::flat_map<int, boost::container::flat_set<int>>::iterator clusterIt = outputProfiledTree.segHeadClusters.begin(); clusterIt != outputProfiledTree.segHeadClusters.end(); ++clusterIt)
	{
		headSegs.clear();
		tailSegs.clear();
		segPairs.clear();

		headSegs = clusterIt->second;
		tailSegs = outputProfiledTree.segTailClusters.at(clusterIt->first);

		/*cout << endl << clusterIt->first << ": ";
		for (boost::container::flat_set<int>::iterator it1 = headSegs.begin(); it1 != headSegs.end(); ++it1) cout << *it1 << " ";
		cout << "|| ";
		for (boost::container::flat_set<int>::iterator it1 = tailSegs.begin(); it1 != tailSegs.end(); ++it1) cout << *it1 << " ";
		cout << endl;*/

		for (boost::container::flat_set<int>::iterator headIt = headSegs.begin(); headIt != headSegs.end(); ++headIt)
		{
			for (boost::container::flat_set<int>::iterator tailIt = tailSegs.begin(); tailIt != tailSegs.end(); ++tailIt)
			{
				if (*tailIt == *headIt) continue;

				segPairProfile newSegPair(outputProfiledTree.segs.at(*headIt), outputProfiledTree.segs.at(*tailIt), head_tail);
				if (newSegPair.turningAngle == -1) continue;
				segPairs.push_back(newSegPair);
			}
		}
		if (headSegs.size() > 1)
		{
			for (boost::container::flat_set<int>::iterator headIt1 = headSegs.begin(); headIt1 != headSegs.end() - 1; ++headIt1)
			{
				for (boost::container::flat_set<int>::iterator headIt2 = headSegs.begin() + 1; headIt2 != headSegs.end(); ++headIt2)
				{
					segPairProfile newSegPair(outputProfiledTree.segs.at(*headIt1), outputProfiledTree.segs.at(*headIt2), head_head);
					if (newSegPair.turningAngle == -1) continue;
					segPairs.push_back(newSegPair);
				}
			}
		}
		if (tailSegs.size() > 1)
		{
			for (boost::container::flat_set<int>::iterator tailIt1 = tailSegs.begin(); tailIt1 != tailSegs.end() - 1; ++tailIt1)
			{
				for (boost::container::flat_set<int>::iterator tailIt2 = tailSegs.begin() + 1; tailIt2 != tailSegs.end(); ++tailIt2)
				{
					segPairProfile newSegPair(outputProfiledTree.segs.at(*tailIt1), outputProfiledTree.segs.at(*tailIt2), tail_tail);
					if (newSegPair.turningAngle == -1) continue;
					segPairs.push_back(newSegPair);
				}
			}
		}

		double minTurningAngle = 10000;
		segPairProfile chosenPair;
		//cout << "segPairs num: " << segPairs.size() << endl;
		if (segPairs.size() > 1)
		{
			for (vector<segPairProfile>::iterator pairIt = segPairs.begin(); pairIt != segPairs.end(); ++pairIt)
			{
				//cout << pairIt->seg1Ptr->segID << " " << pairIt->seg2Ptr->segID << " ";
				if (pairIt->turningAngle < minTurningAngle)
				{
					minTurningAngle = pairIt->turningAngle;
					chosenPair = *pairIt;
					//cout << minTurningAngle;
				}
				//cout << endl;
			}
			//cout << " ==> " << chosenPair.seg1Ptr->segID << " " << chosenPair.seg2Ptr->segID << " " << minTurningAngle << endl;
		}
		else continue;

		if (minTurningAngle > PI * 0.75) continue;
		else
		{
			outputProfiledTree.segs.at(chosenPair.seg1Ptr->segID).to_be_deleted = true;
			outputProfiledTree.segs.at(chosenPair.seg2Ptr->segID).to_be_deleted = true;
			if (chosenPair.currConnOrt == head_head)
			{
				outputProfiledTree.tree.listNeuron[outputProfiledTree.node2LocMap.at(chosenPair.seg1Ptr->head)].type = 7;
				outputProfiledTree.tree.listNeuron[outputProfiledTree.node2LocMap.at(chosenPair.seg2Ptr->head)].type = 7;
			}
			else if (chosenPair.currConnOrt == head_tail)
			{
				outputProfiledTree.tree.listNeuron[outputProfiledTree.node2LocMap.at(chosenPair.seg1Ptr->head)].type = 7;
				outputProfiledTree.tree.listNeuron[outputProfiledTree.node2LocMap.at(*chosenPair.seg2Ptr->tails.begin())].type = 7;
			}
			else if (chosenPair.currConnOrt == tail_tail)
			{
				outputProfiledTree.tree.listNeuron[outputProfiledTree.node2LocMap.at(*chosenPair.seg1Ptr->tails.begin())].type = 7;
				outputProfiledTree.tree.listNeuron[outputProfiledTree.node2LocMap.at(*chosenPair.seg2Ptr->tails.begin())].type = 7;
			}
			segUnit newSeg = this->segUnitConnect_executer(*chosenPair.seg1Ptr, *chosenPair.seg2Ptr, chosenPair.currConnOrt);
			allNewSegs.insert({ ++maxInputSegID, newSeg });

			if (chosenPair.currConnOrt == head_head)
			{
				int head1TailCluster = outputProfiledTree.tailSeg2ClusterMap.at(chosenPair.seg1Ptr->segID);
				int head2TailCluster = outputProfiledTree.tailSeg2ClusterMap.at(chosenPair.seg2Ptr->segID);
				
				if (chosenPair.seg1Ptr->segID > chosenPair.seg2Ptr->segID)
				{
					clusterIt->second.erase(clusterIt->second.find(chosenPair.seg1Ptr->segID));
					clusterIt->second.erase(clusterIt->second.find(chosenPair.seg2Ptr->segID));
					if (head1TailCluster != clusterIt->first) outputProfiledTree.segTailClusters.at(head1TailCluster).erase(outputProfiledTree.segTailClusters.at(head1TailCluster).find(chosenPair.seg1Ptr->segID));
					if (head2TailCluster != clusterIt->first) outputProfiledTree.segTailClusters.at(head2TailCluster).erase(outputProfiledTree.segTailClusters.at(head2TailCluster).find(chosenPair.seg2Ptr->segID));
				}
				else if (chosenPair.seg2Ptr->segID > chosenPair.seg1Ptr->segID)
				{
					clusterIt->second.erase(clusterIt->second.find(chosenPair.seg2Ptr->segID));
					clusterIt->second.erase(clusterIt->second.find(chosenPair.seg1Ptr->segID));
					if (head2TailCluster != clusterIt->first) outputProfiledTree.segTailClusters.at(head2TailCluster).erase(outputProfiledTree.segTailClusters.at(head2TailCluster).find(chosenPair.seg2Ptr->segID));
					if (head1TailCluster != clusterIt->first) outputProfiledTree.segTailClusters.at(head1TailCluster).erase(outputProfiledTree.segTailClusters.at(head1TailCluster).find(chosenPair.seg1Ptr->segID));
				}
			}
			else if (chosenPair.currConnOrt == head_tail)
			{
				int head1TailCluster = outputProfiledTree.tailSeg2ClusterMap.at(chosenPair.seg1Ptr->segID);
				int tail2HeadCluster = outputProfiledTree.headSeg2ClusterMap.at(chosenPair.seg2Ptr->segID);

				if (chosenPair.seg1Ptr->segID > chosenPair.seg2Ptr->segID)
				{
					clusterIt->second.erase(clusterIt->second.find(chosenPair.seg1Ptr->segID));
					outputProfiledTree.segTailClusters.at(clusterIt->first).erase(chosenPair.seg2Ptr->segID);
					if (head1TailCluster != clusterIt->first) outputProfiledTree.segTailClusters.at(head1TailCluster).erase(outputProfiledTree.segTailClusters.at(head1TailCluster).find(chosenPair.seg1Ptr->segID));
					if (tail2HeadCluster != clusterIt->first) outputProfiledTree.segHeadClusters.at(tail2HeadCluster).erase(outputProfiledTree.segHeadClusters.at(tail2HeadCluster).find(chosenPair.seg2Ptr->segID));
				}
				else if (chosenPair.seg2Ptr->segID > chosenPair.seg1Ptr->segID)
				{
					outputProfiledTree.segTailClusters.at(clusterIt->first).erase(chosenPair.seg2Ptr->segID);
					clusterIt->second.erase(clusterIt->second.find(chosenPair.seg1Ptr->segID));
					if (tail2HeadCluster != clusterIt->first) outputProfiledTree.segHeadClusters.at(tail2HeadCluster).erase(outputProfiledTree.segHeadClusters.at(tail2HeadCluster).find(chosenPair.seg2Ptr->segID));
					if (head1TailCluster != clusterIt->first) outputProfiledTree.segTailClusters.at(head1TailCluster).erase(outputProfiledTree.segTailClusters.at(head1TailCluster).find(chosenPair.seg1Ptr->segID));
				}
			}
			else if (chosenPair.currConnOrt == tail_tail)
			{
				int tail1HeadCluster = outputProfiledTree.headSeg2ClusterMap.at(chosenPair.seg1Ptr->segID);
				int tail2HeadCluster = outputProfiledTree.headSeg2ClusterMap.at(chosenPair.seg2Ptr->segID);

				if (chosenPair.seg1Ptr->segID > chosenPair.seg2Ptr->segID)
				{
					outputProfiledTree.segTailClusters.at(clusterIt->first).erase(chosenPair.seg1Ptr->segID);
					outputProfiledTree.segTailClusters.at(clusterIt->first).erase(chosenPair.seg2Ptr->segID);
					if (tail1HeadCluster != clusterIt->first) outputProfiledTree.segHeadClusters.at(tail1HeadCluster).erase(outputProfiledTree.segHeadClusters.at(tail1HeadCluster).find(chosenPair.seg1Ptr->segID));
					if (tail2HeadCluster != clusterIt->first) outputProfiledTree.segHeadClusters.at(tail2HeadCluster).erase(outputProfiledTree.segHeadClusters.at(tail2HeadCluster).find(chosenPair.seg2Ptr->segID));
				}
				else if (chosenPair.seg2Ptr->segID > chosenPair.seg1Ptr->segID)
				{
					outputProfiledTree.segTailClusters.at(clusterIt->first).erase(chosenPair.seg2Ptr->segID);
					outputProfiledTree.segTailClusters.at(clusterIt->first).erase(chosenPair.seg1Ptr->segID);
					
					if (tail2HeadCluster != clusterIt->first) outputProfiledTree.segHeadClusters.at(tail2HeadCluster).erase(outputProfiledTree.segHeadClusters.at(tail2HeadCluster).find(chosenPair.seg2Ptr->segID));
					if (tail1HeadCluster != clusterIt->first) outputProfiledTree.segHeadClusters.at(tail1HeadCluster).erase(outputProfiledTree.segHeadClusters.at(tail1HeadCluster).find(chosenPair.seg1Ptr->segID));
				}
			}
		}
	}
	// ----- END of [For every cluster, identify and connect the pair of segments that is qualified to be connected] ----- //

	// ------- Remove nodes from segments that have been connected and append nodes from new segments ------- //
	vector<size_t> nodeDeleteLocs;
	bool segDeleted = true;
	while (segDeleted)
	{
		for (map<int, segUnit>::iterator it = outputProfiledTree.segs.begin(); it != outputProfiledTree.segs.end(); ++it)
		{
			if (it->second.to_be_deleted)
			{
				//cout << it->first << ": " << it->second.nodes.size() << endl;
				for (QList<NeuronSWC>::iterator nodeIt = it->second.nodes.begin(); nodeIt != it->second.nodes.end(); ++nodeIt)
					nodeDeleteLocs.push_back(outputProfiledTree.node2LocMap.at(nodeIt->n));
				outputProfiledTree.segs.erase(it); // Here it avoids the same segment being connected multiple times.

				goto SEG_DELETED;
			}
		}
		segDeleted = false;

	SEG_DELETED:
		continue;
	}

	sort(nodeDeleteLocs.rbegin(), nodeDeleteLocs.rend());
	for (vector<size_t>::iterator it = nodeDeleteLocs.begin(); it != nodeDeleteLocs.end(); ++it)
		outputProfiledTree.tree.listNeuron.erase(outputProfiledTree.tree.listNeuron.begin() + ptrdiff_t(*it));

	//cout << endl << "NEW SEGMENTS ------------- " << endl;
	for (map<int, segUnit>::iterator it = allNewSegs.begin(); it != allNewSegs.end(); ++it)
	{
		//cout << it->first << ": " << it->second.nodes.size() << endl;
		outputProfiledTree.tree.listNeuron.append(it->second.nodes);
	}
	// --- END of [Remove nodes from segments that have been connected and append nodes from new segments] --- //

	this->profiledTreeReInit(outputProfiledTree);
	return outputProfiledTree;
}

profiledTree NeuronStructExplorer::itered_segElongate_cluster(profiledTree& inputProfiledTree, float distThreshold)
{
	cout << "iteration 1 " << endl;
	int iterCount = 1;
	profiledTree elongatedTree = this->segElongate_cluster(inputProfiledTree);
	while (elongatedTree.segs.size() != inputProfiledTree.segs.size())
	{
		inputProfiledTree = elongatedTree;

		++iterCount;
		cout << "iteration " << iterCount << " " << endl;
		this->getSegHeadTailClusters(inputProfiledTree, distThreshold);
		elongatedTree = this->segElongate_cluster(inputProfiledTree);
	}
	cout << endl;

	return elongatedTree;
}

map<int, segUnit> NeuronStructExplorer::segUnitConnPicker_dist(const vector<int>& currTileHeadSegIDs, const vector<int>& currTileTailSegIDs, profiledTree& currProfiledTree, float distThreshold)
{
	int maxInputSegID = currProfiledTree.segs.rbegin()->first;
	map<string, float> distMap;
	distMap.clear();
	map<int, segUnit> newSegs;
	newSegs.clear();

	if (!currTileHeadSegIDs.empty())
	{
		for (vector<int>::const_iterator headIt1 = currTileHeadSegIDs.begin(); headIt1 != currTileHeadSegIDs.end() - 1; ++headIt1)
		{
			if (currProfiledTree.segs.at(*headIt1).to_be_deleted) continue;
			for (vector<int>::const_iterator headIt2 = headIt1 + 1; headIt2 != currTileHeadSegIDs.end(); ++headIt2)
			{
				if (currProfiledTree.segs.at(*headIt2).to_be_deleted) continue;

				NeuronSWC* headPtr1 = &currProfiledTree.tree.listNeuron[currProfiledTree.node2LocMap.at(currProfiledTree.segs.at(*headIt1).head)];
				NeuronSWC* headPtr2 = &currProfiledTree.tree.listNeuron[currProfiledTree.node2LocMap.at(currProfiledTree.segs.at(*headIt2).head)];
				float dist = sqrtf(float((headPtr1->x - headPtr2->x) * (headPtr1->x - headPtr2->x)) + float((headPtr1->y - headPtr2->y) * (headPtr1->y - headPtr2->y)) + float((headPtr1->z - headPtr2->z) * (headPtr1->z - headPtr2->z)));
				if (dist > distThreshold) continue;

				string label = "hh_" + to_string(*headIt1) + "_" + to_string(*headIt2);
				distMap.insert({ label, dist });
			}
		}
	}

	if (!currTileTailSegIDs.empty())
	{
		for (vector<int>::const_iterator tailIt1 = currTileTailSegIDs.begin(); tailIt1 != currTileTailSegIDs.end() - 1; ++tailIt1)
		{
			if (currProfiledTree.segs.at(*tailIt1).to_be_deleted) continue;
			for (vector<int>::const_iterator tailIDit1 = currProfiledTree.segs.at(*tailIt1).tails.begin(); tailIDit1 != currProfiledTree.segs.at(*tailIt1).tails.end(); ++tailIDit1)
			{
				for (vector<int>::const_iterator tailIt2 = tailIt1 + 1; tailIt2 != currTileTailSegIDs.end(); ++tailIt2)
				{
					if (currProfiledTree.segs.at(*tailIt2).to_be_deleted) continue;
					for (vector<int>::const_iterator tailIDit2 = currProfiledTree.segs.at(*tailIt2).tails.begin(); tailIDit2 != currProfiledTree.segs.at(*tailIt2).tails.end(); ++tailIDit2)
					{
						NeuronSWC* tailPtr1 = &currProfiledTree.tree.listNeuron[currProfiledTree.node2LocMap.at(*tailIDit1)];
						NeuronSWC* tailPtr2 = &currProfiledTree.tree.listNeuron[currProfiledTree.node2LocMap.at(*tailIDit2)];
						float dist = sqrtf(float((tailPtr1->x - tailPtr2->x) * (tailPtr1->x - tailPtr2->x)) + float((tailPtr1->y - tailPtr2->y) * (tailPtr1->y - tailPtr2->y)) + float((tailPtr1->z - tailPtr2->z) * (tailPtr1->z - tailPtr2->z)));
						if (dist > distThreshold) continue;

						string label = "tt_" + to_string(*tailIt1) + "_" + to_string(*tailIDit1) + "_" + to_string(*tailIt2) + "_" + to_string(*tailIDit2);
						distMap.insert({ label, dist });
					}
				}
			}
		}
	}

	if (!currTileHeadSegIDs.empty() && !currTileTailSegIDs.empty())
	{
		for (vector<int>::const_iterator headIt = currTileHeadSegIDs.begin(); headIt != currTileHeadSegIDs.end(); ++headIt)
		{
			if (currProfiledTree.segs.at(*headIt).to_be_deleted) continue;
			for (vector<int>::const_iterator tailIt = currTileTailSegIDs.begin(); tailIt != currTileTailSegIDs.end(); ++tailIt)
			{
				if (currProfiledTree.segs.at(*tailIt).to_be_deleted) continue;
				else if (*headIt == *tailIt) continue;

				for (vector<int>::const_iterator tailIDit = currProfiledTree.segs.at(*tailIt).tails.begin(); tailIDit != currProfiledTree.segs.at(*tailIt).tails.end(); ++tailIDit) // tailIt point to tail node ID
				{
					NeuronSWC* headPtr = &currProfiledTree.tree.listNeuron[currProfiledTree.node2LocMap.at(currProfiledTree.segs.at(*headIt).head)];
					NeuronSWC* tailPtr = &currProfiledTree.tree.listNeuron[currProfiledTree.node2LocMap.at(*tailIDit)];
					float dist = sqrtf(float((headPtr->x - tailPtr->x) * (headPtr->x - tailPtr->x)) + float((headPtr->y - tailPtr->y) * (headPtr->y - tailPtr->y)) + float((headPtr->z - tailPtr->z) * (headPtr->z - tailPtr->z)));
					if (dist > distThreshold) continue;

					string label = "ht_" + to_string(*headIt) + "_" + to_string(*tailIt) + "_" + to_string(*tailIDit);
					distMap.insert({ label, dist });
				}
			}
		}
	}

	pair<string, float> nearestPair;
	if (!distMap.empty())
	{
		while (1)
		{
			float minDist = 10000;
			if (distMap.empty()) return newSegs;

			// Connect segment 1 at a time to avoid forming branches. Therefore, using the shortest distace as the seletion criterion.
			for (map<string, float>::iterator it = distMap.begin(); it != distMap.end(); ++it)
			{
				if (it->second < minDist)
				{
					minDist = it->second;
					nearestPair.first = it->first;
					nearestPair.second = minDist;
				}
			}

			++maxInputSegID;
			vector<string> labelSplits;
			boost::split(labelSplits, nearestPair.first, boost::is_any_of("_"));
			if (!labelSplits.at(0).compare("hh"))
			{
				segUnit segUnit1 = currProfiledTree.segs.at(stoi(labelSplits.at(1)));
				segUnit segUnit2 = currProfiledTree.segs.at(stoi(labelSplits.at(2)));
				vector<float> seg1Vector = NeuronStructExplorer::getVector_NeuronSWC(segUnit1.nodes.at(segUnit1.seg_nodeLocMap.at(segUnit1.head)), segUnit1.nodes.at(segUnit1.seg_nodeLocMap.at(*segUnit1.tails.begin())));
				vector<float> seg2Vector = NeuronStructExplorer::getVector_NeuronSWC(segUnit2.nodes.at(segUnit2.seg_nodeLocMap.at(*segUnit2.tails.begin())), segUnit2.nodes.at(segUnit2.seg_nodeLocMap.at(segUnit2.head)));
				double turnAngle = NeuronStructExplorer::getPiAngle(seg1Vector, seg2Vector);
				if (turnAngle > PI / 4)
				{
					distMap.erase(distMap.find(nearestPair.first));
					//cout << nearestPair.first << " " << nearestPair.second << " ";
					//cout << "route turning back, move to the next" << " distMap size: " << distMap.size() << endl;
					continue;
				}

				segUnit connectedSeg = this->segUnitConnect_executer(segUnit1, segUnit2, head_head);
				newSegs.insert({ maxInputSegID, connectedSeg });
				currProfiledTree.segs.at(stoi(labelSplits.at(1))).to_be_deleted = true;
				currProfiledTree.segs.at(stoi(labelSplits.at(2))).to_be_deleted = true;
				//cout << "segments picked" << endl;

				break;
			}
			else if (!labelSplits.at(0).compare("ht"))
			{
				segUnit segUnit1 = currProfiledTree.segs.at(stoi(labelSplits.at(1)));
				segUnit segUnit2 = currProfiledTree.segs.at(stoi(labelSplits.at(2)));
				vector<float> seg1Vector = NeuronStructExplorer::getVector_NeuronSWC(segUnit1.nodes.at(segUnit1.seg_nodeLocMap.at(segUnit1.head)), segUnit1.nodes.at(segUnit1.seg_nodeLocMap.at(*segUnit1.tails.begin())));
				vector<float> seg2Vector = NeuronStructExplorer::getVector_NeuronSWC(segUnit2.nodes.at(segUnit2.seg_nodeLocMap.at(segUnit2.head)), segUnit2.nodes.at(segUnit2.seg_nodeLocMap.at(*segUnit2.tails.begin())));
				double turnAngle = NeuronStructExplorer::getPiAngle(seg1Vector, seg2Vector);
				if (turnAngle > PI / 4)
				{
					distMap.erase(distMap.find(nearestPair.first));
					//cout << nearestPair.first << " " << nearestPair.second << " ";
					//cout << "route turning back, move to the next" << " distMap size: " << distMap.size() << endl;
					continue;
				}

				segUnit connectedSeg = this->segUnitConnect_executer(segUnit1, segUnit2, head_tail);
				newSegs.insert({ maxInputSegID, connectedSeg });
				currProfiledTree.segs.at(stoi(labelSplits.at(1))).to_be_deleted = true;
				currProfiledTree.segs.at(stoi(labelSplits.at(2))).to_be_deleted = true;
				//cout << "segments picked" << endl;

				break;
			}
			else if (!labelSplits.at(0).compare("tt"))
			{
				segUnit segUnit1 = currProfiledTree.segs.at(stoi(labelSplits.at(1)));
				segUnit segUnit2 = currProfiledTree.segs.at(stoi(labelSplits.at(3)));
				vector<float> seg1Vector = NeuronStructExplorer::getVector_NeuronSWC(segUnit1.nodes.at(segUnit1.seg_nodeLocMap.at(*segUnit1.tails.begin())), segUnit1.nodes.at(segUnit1.seg_nodeLocMap.at(segUnit1.head)));
				vector<float> seg2Vector = NeuronStructExplorer::getVector_NeuronSWC(segUnit2.nodes.at(segUnit2.seg_nodeLocMap.at(segUnit2.head)), segUnit2.nodes.at(segUnit2.seg_nodeLocMap.at(*segUnit2.tails.begin())));
				double turnAngle = NeuronStructExplorer::getPiAngle(seg1Vector, seg2Vector);
				if (turnAngle > PI / 4)
				{
					distMap.erase(distMap.find(nearestPair.first));
					//cout << nearestPair.first << " " << nearestPair.second << " ";
					//cout << "route turning back, move to the next" << " distMap size: " << distMap.size() << endl;
					continue;
				}

				segUnit connectedSeg = this->segUnitConnect_executer(segUnit1, segUnit2, tail_tail);
				newSegs.insert({ maxInputSegID, connectedSeg });
				currProfiledTree.segs.at(stoi(labelSplits.at(1))).to_be_deleted = true;
				currProfiledTree.segs.at(stoi(labelSplits.at(3))).to_be_deleted = true;
				//cout << "segments picked" << endl;

				break;
			}
		}
	}

	return newSegs;
}

map<int, segUnit> NeuronStructExplorer::segRegionConnector_angle(const vector<int>& currTileHeadSegIDs, const vector<int>& currTileTailSegIDs, profiledTree& currProfiledTree, double angleThre, bool length)
{
	set<int> connectedSegs;
	map<int, int> elongConnMap;
	map<string, double> segAngleMap;
	map<int, segUnit> newSegs;

	//__________________________________________________________________________________________________________________________
	
	//cout << "------- head-tail:" << endl;
	for (vector<int>::const_iterator headIt = currTileHeadSegIDs.begin(); headIt != currTileHeadSegIDs.end(); ++headIt)
	{
		for (vector<int>::const_iterator tailIt = currTileTailSegIDs.begin(); tailIt != currTileTailSegIDs.end(); ++tailIt)
		{
			if (*headIt == *tailIt) continue;
			else
			{
				if (currProfiledTree.segs.at(*headIt).to_be_deleted || currProfiledTree.segs.at(*tailIt).to_be_deleted) continue;

				double pointingRadAngle = this->segPointingCompare(currProfiledTree.segs.at(*headIt), currProfiledTree.segs.at(*tailIt), head_tail);
				double turningRadAngle = this->segTurningAngle(currProfiledTree.segs.at(*headIt), currProfiledTree.segs.at(*tailIt), head_tail);
				if (pointingRadAngle == -1 || turningRadAngle == -1) continue;

				if (pointingRadAngle < angleThre && turningRadAngle < angleThre)
				{
					//cout << *headIt << "_" << *tailIt << "->(" << pointingRadAngle << "," << turningRadAngle << ") ";
					string segAngleKey = to_string(*headIt) + "_" + to_string(*tailIt);
					segAngleMap.insert(pair<string, double>(segAngleKey, (pointingRadAngle + turningRadAngle)));
				}
			}
		}
	}
	this->tileSegConnOrganizer_angle(segAngleMap, connectedSegs, elongConnMap);
	for (map<int, int>::iterator it = elongConnMap.begin(); it != elongConnMap.end(); ++it)
	{
		currProfiledTree.segs[it->first].to_be_deleted = true;
		currProfiledTree.segs[it->second].to_be_deleted = true;
		segUnit newSeg = this->segUnitConnect_executer(currProfiledTree.segs[it->first], currProfiledTree.segs[it->second], head_tail);
		newSeg.segID = currProfiledTree.segs.size() + 1;
		while (currProfiledTree.segs.find(newSeg.segID) != currProfiledTree.segs.end()) ++newSeg.segID;
		currProfiledTree.segs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
		newSegs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
	}
	segAngleMap.clear();
	elongConnMap.clear();
	//cout << endl;
	//__________________________________________________________________________________________________________________________

	//cout << "------- tail-head:" << endl;
	for (vector<int>::const_iterator tailIt = currTileHeadSegIDs.begin(); tailIt != currTileHeadSegIDs.end(); ++tailIt)
	{
		for (vector<int>::const_iterator headIt = currTileTailSegIDs.begin(); headIt != currTileTailSegIDs.end(); ++headIt)
		{
			if (*headIt == *tailIt) continue;
			else
			{
				if (currProfiledTree.segs.at(*tailIt).to_be_deleted || currProfiledTree.segs.at(*headIt).to_be_deleted) continue;

				double pointingRadAngle = this->segPointingCompare(currProfiledTree.segs.at(*tailIt), currProfiledTree.segs.at(*headIt), tail_head);
				double turningRadAngle = this->segTurningAngle(currProfiledTree.segs.at(*tailIt), currProfiledTree.segs.at(*headIt), tail_head);
				if (pointingRadAngle == -1 || turningRadAngle == -1) continue;

				if (pointingRadAngle < angleThre && turningRadAngle < angleThre)
				{
					//cout << *tailIt << "_" << *headIt << "->(" << pointingRadAngle << "," << turningRadAngle << ") ";
					string segAngleKey = to_string(*tailIt) + "_" + to_string(*headIt);
					segAngleMap.insert(pair<string, double>(segAngleKey, (pointingRadAngle + turningRadAngle)));
				}
			}
		}
	}
	this->tileSegConnOrganizer_angle(segAngleMap, connectedSegs, elongConnMap);
	for (map<int, int>::iterator it = elongConnMap.begin(); it != elongConnMap.end(); ++it)
	{
		currProfiledTree.segs[it->first].to_be_deleted = true;
		currProfiledTree.segs[it->second].to_be_deleted = true;
		segUnit newSeg = this->segUnitConnect_executer(currProfiledTree.segs[it->first], currProfiledTree.segs[it->second], tail_head);
		newSeg.segID = currProfiledTree.segs.size() + 1;
		while (currProfiledTree.segs.find(newSeg.segID) != currProfiledTree.segs.end()) ++newSeg.segID;
		currProfiledTree.segs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
		newSegs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
	}
	segAngleMap.clear();
	elongConnMap.clear();
	//cout << endl;
	//__________________________________________________________________________________________________________________________

	//cout << "------- head-head:" << endl;
	for (vector<int>::const_iterator headIt1 = currTileHeadSegIDs.begin(); headIt1 != currTileHeadSegIDs.end(); ++headIt1)
	{
		for (vector<int>::const_iterator headIt2 = currTileHeadSegIDs.begin(); headIt2 != currTileHeadSegIDs.end(); ++headIt2)
		{
			if (*headIt1 == *headIt2) continue;
			else
			{
				if (currProfiledTree.segs.at(*headIt1).to_be_deleted || currProfiledTree.segs.at(*headIt2).to_be_deleted) continue;

				double pointingRadAngle = this->segPointingCompare(currProfiledTree.segs.at(*headIt1), currProfiledTree.segs.at(*headIt2), head_head);
				double turningRadAngle = this->segTurningAngle(currProfiledTree.segs.at(*headIt1), currProfiledTree.segs.at(*headIt2), head_head);
				if (pointingRadAngle == -1 || turningRadAngle == -1) continue;

				if (pointingRadAngle < angleThre && turningRadAngle < angleThre)
				{
					//cout << *headIt1 << "_" << *headIt2 << "->(" << pointingRadAngle << "," << turningRadAngle << ") ";
					string segAngleKey = to_string(*headIt1) + "_" + to_string(*headIt2);
					segAngleMap.insert(pair<string, double>(segAngleKey, (pointingRadAngle + turningRadAngle)));
				}
			}
		}
	}
	this->tileSegConnOrganizer_angle(segAngleMap, connectedSegs, elongConnMap);
	for (map<int, int>::iterator it = elongConnMap.begin(); it != elongConnMap.end(); ++it)
	{
		currProfiledTree.segs[it->first].to_be_deleted = true;
		currProfiledTree.segs[it->second].to_be_deleted = true;
		segUnit newSeg = this->segUnitConnect_executer(currProfiledTree.segs[it->first], currProfiledTree.segs[it->second], head_head);
		newSeg.segID = currProfiledTree.segs.size() + 1;
		while (currProfiledTree.segs.find(newSeg.segID) != currProfiledTree.segs.end()) ++newSeg.segID;
		currProfiledTree.segs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
		newSegs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
	}
	segAngleMap.clear();
	elongConnMap.clear();
	//cout << endl;
	//__________________________________________________________________________________________________________________________

	//cout << "------- tail-tail:" << endl;
	for (vector<int>::const_iterator tailIt1 = currTileHeadSegIDs.begin(); tailIt1 != currTileHeadSegIDs.end(); ++tailIt1)
	{
		for (vector<int>::const_iterator tailIt2 = currTileTailSegIDs.begin(); tailIt2 != currTileTailSegIDs.end(); ++tailIt2)
		{
			if (*tailIt1 == *tailIt2) continue;
			else
			{
				if (currProfiledTree.segs.at(*tailIt1).to_be_deleted || currProfiledTree.segs.at(*tailIt2).to_be_deleted) continue;

				double pointingRadAngle = this->segPointingCompare(currProfiledTree.segs.at(*tailIt1), currProfiledTree.segs.at(*tailIt2), tail_tail);
				double turningRadAngle = this->segTurningAngle(currProfiledTree.segs.at(*tailIt1), currProfiledTree.segs.at(*tailIt2), tail_tail);
				if (pointingRadAngle == -1 || turningRadAngle == -1) continue;

				if (pointingRadAngle < angleThre && turningRadAngle < angleThre)
				{
					//cout << *tailIt1 << "_" << *tailIt2 << "->(" << pointingRadAngle << "," << turningRadAngle << ") ";
					string segAngleKey = to_string(*tailIt1) + "_" + to_string(*tailIt2);
					segAngleMap.insert(pair<string, double>(segAngleKey, (pointingRadAngle + turningRadAngle)));
				}
			}
		}
	}
	this->tileSegConnOrganizer_angle(segAngleMap, connectedSegs, elongConnMap);
	for (map<int, int>::iterator it = elongConnMap.begin(); it != elongConnMap.end(); ++it)
	{
		currProfiledTree.segs[it->first].to_be_deleted = true;
		currProfiledTree.segs[it->second].to_be_deleted = true;
		segUnit newSeg = this->segUnitConnect_executer(currProfiledTree.segs[it->first], currProfiledTree.segs[it->second], tail_tail);
		newSeg.segID = currProfiledTree.segs.size() + 1;
		while (currProfiledTree.segs.find(newSeg.segID) != currProfiledTree.segs.end()) ++newSeg.segID;
		currProfiledTree.segs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
		newSegs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
	}
	segAngleMap.clear();
	elongConnMap.clear();
	//cout << endl << endl;
	//__________________________________________________________________________________________________________________________

	return newSegs;
}

profiledTree NeuronStructExplorer::segElongate(const profiledTree& inputProfiledTree, double angleThre)
{
	// -- This method connects segments tile by tile based on distance and angle threshold. 
	// -- It is usually called by NeuronStructExplorer::itered_segElongate to iteratively complete the process process for the whole tree.
	// ---- This method is currently used in IVSCC auto-recon.

	//cout << inputProfiledTree.segHeadMap.size() << " " << inputProfiledTree.segTailMap.size() << endl;

	profiledTree outputProfiledTree = inputProfiledTree;
	QList<NeuronSWC> newNodeList = inputProfiledTree.tree.listNeuron;
	map<int, segUnit> allNewSegs;
	
	set<string> allTileKeys;
	for (map<string, vector<int>>::const_iterator headTileIt = inputProfiledTree.segHeadMap.begin(); headTileIt != inputProfiledTree.segHeadMap.end(); ++headTileIt)
		allTileKeys.insert(headTileIt->first);
	for (map<string, vector<int>>::const_iterator tailTileIt = inputProfiledTree.segTailMap.begin(); tailTileIt != inputProfiledTree.segTailMap.end(); ++tailTileIt)
		allTileKeys.insert(tailTileIt->first);
	
	vector<int> currTileHeadSegIDs;
	vector<int> currTileTailSegIDs;
	set<int> connectedSegs;
	for (set<string>::iterator keyIt = allTileKeys.begin(); keyIt != allTileKeys.end(); ++keyIt)
	{
		//cout << "TILE " << *keyIt << " =======" << endl;
		//cout << " - Heads: ";
		if (inputProfiledTree.segHeadMap.find(*keyIt) != inputProfiledTree.segHeadMap.end())
		{
			for (vector<int>::const_iterator headIt = inputProfiledTree.segHeadMap.at(*keyIt).begin(); headIt != inputProfiledTree.segHeadMap.at(*keyIt).end(); ++headIt)
			{
				//cout << *headIt << " ";
				currTileHeadSegIDs.push_back(*headIt);
			}
		}
		//cout << endl;
		//cout << " - Tails: ";
		if (inputProfiledTree.segTailMap.find(*keyIt) != inputProfiledTree.segTailMap.end())
		{
			for (vector<int>::const_iterator tailIt = inputProfiledTree.segTailMap.at(*keyIt).begin(); tailIt != inputProfiledTree.segTailMap.at(*keyIt).end(); ++tailIt)
			{
				//cout << *tailIt << " ";
				currTileTailSegIDs.push_back(*tailIt);
			}
		}
		//cout << endl;
		if (currTileHeadSegIDs.size() + currTileTailSegIDs.size() <= 1) // If there is <= 1 head or tail (only 1 terminal) in the tile, no further process is needed.
		{
			currTileHeadSegIDs.clear();
			currTileTailSegIDs.clear();
			//cout << endl;
			continue;
		}

		map<int, segUnit> newSegs = this->segRegionConnector_angle(currTileHeadSegIDs, currTileTailSegIDs, outputProfiledTree, angleThre);
		for (map<int, segUnit>::iterator newSegIt = newSegs.begin(); newSegIt != newSegs.end(); ++newSegIt) 
			allNewSegs.insert(pair<int, segUnit>(newSegIt->first, newSegIt->second));

		currTileHeadSegIDs.clear();
		currTileTailSegIDs.clear();
	}

	vector<size_t> nodeDeleteLocs;
	bool segDeleted = true;
	while (segDeleted)
	{
		for (map<int, segUnit>::iterator it = outputProfiledTree.segs.begin(); it != outputProfiledTree.segs.end(); ++it)
		{
			if (it->second.to_be_deleted)
			{
				//cout << it->first << ": " << it->second.nodes.size() << endl;
				for (QList<NeuronSWC>::iterator nodeIt = it->second.nodes.begin(); nodeIt != it->second.nodes.end(); ++nodeIt)
					nodeDeleteLocs.push_back(outputProfiledTree.node2LocMap.at(nodeIt->n));
				outputProfiledTree.segs.erase(it);

				goto SEG_DELETED;
			}
		}
		segDeleted = false;

	SEG_DELETED:
		continue;
	}
	//cout << endl;

	sort(nodeDeleteLocs.rbegin(), nodeDeleteLocs.rend());
	for (vector<size_t>::iterator it = nodeDeleteLocs.begin(); it != nodeDeleteLocs.end(); ++it) 
		outputProfiledTree.tree.listNeuron.erase(outputProfiledTree.tree.listNeuron.begin() + ptrdiff_t(*it));

	//cout << endl << "NEW SEGMENTS ------------- " << endl;
	for (map<int, segUnit>::iterator it = allNewSegs.begin(); it != allNewSegs.end(); ++it)
	{
		//cout << it->first << ": " << it->second.nodes.size() << endl;
		outputProfiledTree.tree.listNeuron.append(it->second.nodes);
	}

	this->profiledTreeReInit(outputProfiledTree);
	return outputProfiledTree;
}

profiledTree NeuronStructExplorer::itered_segElongate(profiledTree& inputProfiledTree, double angleThre)
{
	cout << "iteration 1 ";
	int iterCount = 1;
	profiledTree elongatedTree = this->segElongate(inputProfiledTree, angleThre);
	while (elongatedTree.segs.size() != inputProfiledTree.segs.size())
	{
		++iterCount;
		cout << "iterator " << iterCount << " " << endl;
		inputProfiledTree = elongatedTree;
		elongatedTree = this->segElongate(inputProfiledTree, angleThre);
	}
	cout << endl;

	return elongatedTree;
}

segUnit NeuronStructExplorer::segUnitConnect_executer(const segUnit& segUnit1, const segUnit& segUnit2, connectOrientation connOrt)
{
	if (segUnit1.tails.size() > 1 || segUnit2.tails.size() > 1)
		throw invalid_argument("Currently forked segment connection is not supported. Do nothing and return");

	segUnit newSeg;
	QList<NeuronSWC> newSegNodes;
	QList<NeuronSWC> endEditedNodes;

	switch (connOrt)
	{
	case head_tail:
	{
		int connTailID = *segUnit2.tails.cbegin();
		endEditedNodes = segUnit1.nodes;
		endEditedNodes.begin()->parent = connTailID; // In current implementation, the 1st element of a seg must be a root.
		newSegNodes.append(segUnit2.nodes);
		newSegNodes.append(endEditedNodes);
		newSeg.nodes = newSegNodes;
		break;
	}
	case tail_head:
	{
		int connTailID = *segUnit1.tails.cbegin();
		endEditedNodes = segUnit2.nodes;
		endEditedNodes.begin()->parent = connTailID; // In current implementation, the 1st element of a seg must be a root. 
		newSegNodes.append(segUnit1.nodes);
		newSegNodes.append(endEditedNodes);
		newSeg.nodes = newSegNodes;
		break;
	}
	case head_head:
	{
		int connTailID = segUnit2.head;
		for (map<int, vector<size_t>>::const_iterator it = segUnit2.seg_childLocMap.cbegin(); it != segUnit2.seg_childLocMap.cend(); ++it)
		{
			size_t nodeLoc = segUnit2.seg_nodeLocMap.at(it->first);
			NeuronSWC newNode = segUnit2.nodes.at(nodeLoc);
			if (it->second.empty())
			{
				//cout << newNode.x << " " << newNode.y << " " << newNode.z << endl;
				newNode.parent = -1;
			}
			else newNode.parent = segUnit2.nodes.at(*(it->second.cbegin())).n;
			endEditedNodes.push_back(newNode);
		}
		newSegNodes.append(segUnit1.nodes);
		newSegNodes.begin()->parent = connTailID;
		newSegNodes.append(endEditedNodes);
		newSeg.nodes = newSegNodes;
		break;
	}
	case tail_tail:
	{
		int connTailID = *segUnit2.tails.cbegin();
		for (map<int, vector<size_t>>::const_iterator it = segUnit1.seg_childLocMap.cbegin(); it != segUnit1.seg_childLocMap.cend(); ++it)
		{
			size_t nodeLoc = segUnit1.seg_nodeLocMap.at(it->first);
			NeuronSWC newNode = segUnit1.nodes.at(nodeLoc);
			if (it->second.empty()) newNode.parent = connTailID;
			else newNode.parent = segUnit1.nodes.at(*(it->second.cbegin())).n;
			endEditedNodes.push_back(newNode);
		}
		newSegNodes.append(segUnit2.nodes);
		newSegNodes.append(endEditedNodes);
		newSeg.nodes = newSegNodes;
		break;
	}
	default:
		break;
	}

	return newSeg;
}

profiledTree NeuronStructExplorer::treeUnion_MSTbased(const profiledTree& expandingPart, const profiledTree& baseTree)
{
	set<int> connectedSegs;
	profiledTree outputProfiledTree = baseTree;

	for (map<int, segUnit>::const_iterator segIt = expandingPart.segs.begin(); segIt != expandingPart.segs.end(); ++segIt)
	{
		NeuronSWC headNode = *segIt->second.nodes.begin();
		string xLabel = to_string(int(headNode.x / SEGtileXY_LENGTH));
		string yLabel = to_string(int(headNode.y / SEGtileXY_LENGTH));
		string zLabel = to_string(int(headNode.z / (SEGtileXY_LENGTH / zRATIO)));
		string keyLabel = xLabel + "_" + yLabel + "_" + zLabel;
		//cout << keyLabel << ": ";

		vector<int> baseSegHeads;
		vector<int> baseSegTails;
		vector<int> baseSegs;
		if (baseTree.segHeadMap.find(keyLabel) != baseTree.segHeadMap.end()) baseSegHeads = baseTree.segHeadMap.at(keyLabel);
		if (baseTree.segTailMap.find(keyLabel) != baseTree.segTailMap.end()) baseSegTails = baseTree.segTailMap.at(keyLabel);
		baseSegs = baseSegHeads;
		for (vector<int>::iterator tileSegIt = baseSegTails.begin(); tileSegIt != baseSegTails.end(); ++tileSegIt)
		{
			if (find(baseSegHeads.begin(), baseSegHeads.end(), *tileSegIt) == baseSegs.end())
				baseSegs.push_back(*tileSegIt);
		}
		//for (vector<int>::iterator checkSegIt = baseSegs.begin(); checkSegIt != baseSegs.end(); ++checkSegIt) cout << *checkSegIt << " ";
		//cout << endl;

		boost::container::flat_map<int, connectOrientation> connRoutingMap;
		for (QList<NeuronSWC>::const_iterator nodeIt = segIt->second.nodes.begin(); nodeIt != segIt->second.nodes.end(); ++nodeIt)
		{
			for (vector<int>::iterator baseSegIt = baseSegs.begin(); baseSegIt != baseSegs.end(); ++baseSegIt)
			{
				NeuronSWC baseSegHeadNode = *baseTree.segs.at(*baseSegIt).nodes.begin();
				if (nodeIt->x == baseSegHeadNode.x && nodeIt->y == baseSegHeadNode.y && nodeIt->z == baseSegHeadNode.z)
				{
					connRoutingMap.insert(pair<int, connectOrientation>(*baseSegIt, head));
					goto SEGMENT_CONFIRMED;
				}

				for (vector<int>::const_iterator tailIt = baseTree.segs.at(*baseSegIt).tails.begin(); tailIt != baseTree.segs.at(*baseSegIt).tails.end(); ++tailIt)
				{
					NeuronSWC baseSegTailNode = baseTree.segs.at(*baseSegIt).nodes.at(baseTree.segs.at(*baseSegIt).seg_nodeLocMap.at(*tailIt));
					if (nodeIt->x == baseSegTailNode.x && nodeIt->y == baseSegTailNode.y && nodeIt->z == baseSegTailNode.z)
					{
						connRoutingMap.insert(pair<int, connectOrientation>(*baseSegIt, tail));
						goto SEGMENT_CONFIRMED;
					}
				}
			}

		SEGMENT_CONFIRMED:
			continue;
		}

		if (connRoutingMap.size() == 0) continue;

		int segID1 = baseTree.segs.at(connRoutingMap.begin()->first).segID;
		connectOrientation ort1 = connRoutingMap.begin()->second;
		for (boost::container::flat_map<int, connectOrientation>::iterator connIt = connRoutingMap.begin() + 1; connIt != connRoutingMap.end(); ++connIt)
		{
			int segID2 = connIt->first;
			connectOrientation ort2 = connIt->second;
			if (segID1 == segID2)
			{
				segID1 = segID2;
				ort1 = ort2;
				continue;
			}

			if (ort1 == head && ort2 == head)
			{
				int nodeID1 = outputProfiledTree.segs.at(segID1).head;
				int nodeID2 = outputProfiledTree.segs.at(segID2).head;
				size_t nodeLoc1 = outputProfiledTree.node2LocMap.at(nodeID1);
				outputProfiledTree.tree.listNeuron[nodeLoc1].parent = nodeID2;
			}
			else if (ort1 == head && ort2 == tail)
			{
				int nodeID1 = outputProfiledTree.segs.at(segID1).head;
				int nodeID2 = *outputProfiledTree.segs.at(segID2).tails.begin();
				size_t nodeLoc1 = outputProfiledTree.node2LocMap.at(nodeID1);
				outputProfiledTree.tree.listNeuron[nodeLoc1].parent = nodeID2;
			}
			else if (ort1 == tail && ort2 == head)
			{
				int nodeID1 = *outputProfiledTree.segs.at(segID1).tails.begin();
				int nodeID2 = outputProfiledTree.segs.at(segID2).head;
				size_t nodeLoc2 = outputProfiledTree.node2LocMap.at(nodeID2);
				outputProfiledTree.tree.listNeuron[nodeLoc2].parent = nodeID1;
			}
		}
	}

	return outputProfiledTree;
}
/* ===================================== END of [Auto-tracing Related Neuron Struct Functions] ===================================== */



/* ================================================== Geometry ================================================= */
vector<pair<float, float>> NeuronStructExplorer::getProjectedVector(const vector<pair<float, float>>& axialVector, const vector<pair<float, float>>& projectingVector)
{
	vector<pair<float, float>> projectedVector;

	double axialVectorLength = sqrt(axialVector.begin()->second * axialVector.begin()->second + (axialVector.begin() + 1)->second * (axialVector.begin() + 1)->second + (axialVector.begin() + 2)->second * (axialVector.begin() + 2)->second);

	vector<float> projectingStartVec(3);
	projectingStartVec[0] = projectingVector.begin()->first - axialVector.begin()->first;
	projectingStartVec[1] = (projectingVector.begin() + 1)->first - (axialVector.begin() + 1)->first;
	projectingStartVec[2] = (projectingVector.begin() + 2)->first - (axialVector.begin() + 2)->first;
	vector<float> axialVector_vec(3);
	axialVector_vec[0] = axialVector.begin()->second;
	axialVector_vec[1] = (axialVector.begin() + 1)->second;
	axialVector_vec[2] = (axialVector.begin() + 2)->second;
	double projectingStartVecSine = NeuronStructExplorer::getVectorSine(axialVector_vec, projectingStartVec);
	double projectingStartVecLength = sqrt(projectingStartVec.at(0) * projectingStartVec.at(0) + projectingStartVec.at(1) * projectingStartVec.at(1) + projectingStartVec.at(2) * projectingStartVec.at(2));
	double ProjectedStartLength = projectingStartVecLength * projectingStartVecSine;
	double projectedStartLengthRatio = ProjectedStartLength / axialVectorLength;
	vector<float> projectedStart(3);
	projectedStart[0] = axialVector.begin()->second * projectedStartLengthRatio + axialVector.begin()->first;
	projectedStart[1] = (axialVector.begin() + 1)->second * projectedStartLengthRatio + (axialVector.begin() + 1)->first;
	projectedStart[2] = (axialVector.begin() + 2)->second * projectedStartLengthRatio + (axialVector.begin() + 2)->first;

	vector<float> projectingVector_vec(3);
	projectingVector_vec[0] = projectingVector.begin()->second;
	projectingVector_vec[1] = (projectingVector.begin() + 1)->second;
	projectingVector_vec[2] = (projectingVector.begin() + 2)->second;
	double projectingVecSine = NeuronStructExplorer::getVectorSine(axialVector_vec, projectingVector_vec);
	double projectingVecLength = sqrt(projectingVector.begin()->second * projectingVector.begin()->second + (projectingVector.begin() + 1)->second * (projectingVector.begin() + 1)->second + (projectingVector.begin()->second + 2) * (projectingVector.begin()->second + 2));
	double projectedVecLength = projectingVecLength * projectingVecSine;
	double projectedLengthRatio = projectedVecLength / projectingVecLength;
	vector<float> projectedVector_vec(3);
	projectedVector_vec[0] = axialVector.begin()->second * projectedLengthRatio;
	projectedVector_vec[1] = (axialVector.begin() + 1)->second * projectedLengthRatio;
	projectedVector_vec[2] = (axialVector.begin() + 2)->second * projectedLengthRatio;

	projectedVector.push_back(pair<float, float>(projectedStart[0], projectedVector_vec[0]));
	projectedVector.push_back(pair<float, float>(projectedStart[1], projectedVector_vec[1]));
	projectedVector.push_back(pair<float, float>(projectedStart[2], projectedVector_vec[2]));

	return projectedVector;
}

double NeuronStructExplorer::segPointingCompare(const segUnit& elongSeg, const segUnit& connSeg, connectOrientation connOrt)
{
	if (elongSeg.tails.size() > 1 || connSeg.tails.size() > 1)
	{
		cerr << "Invalid input: Currently segment elongation only allows to happen between 2 non-branching segments." << endl;
		return -1;
	}

	NeuronSWC elongHeadNode = elongSeg.nodes[elongSeg.seg_nodeLocMap.at(elongSeg.head)];
	NeuronSWC elongTailNode;
	if (elongSeg.tails.size() == 0)
	{
		//cerr << "Elongating segment only has head. Do nothinig and return.";
		return -1;
	}
	else elongTailNode = elongSeg.nodes[elongSeg.seg_nodeLocMap.at(*elongSeg.tails.begin())];

	NeuronSWC connHeadNode = connSeg.nodes[connSeg.seg_nodeLocMap.at(connSeg.head)];
	NeuronSWC connTailNode;
	if (connSeg.tails.size() == 0)
	{
		if (connOrt == head_tail || connOrt == tail_tail)
		{
			//cerr << "Connecting segment only has head. Do nothing and return";
			return -1;
		}
	}
	else connTailNode = connSeg.nodes[connSeg.seg_nodeLocMap.at(*connSeg.tails.begin())];

	vector<float> elongHeadLoc;
	elongHeadLoc.push_back(elongHeadNode.x);
	elongHeadLoc.push_back(elongHeadNode.y);
	elongHeadLoc.push_back(elongHeadNode.z * zRATIO);

	vector<float> elongTailLoc;
	elongTailLoc.push_back(elongTailNode.x);
	elongTailLoc.push_back(elongTailNode.y);
	elongTailLoc.push_back(elongTailNode.z * zRATIO);

	vector<float> connHeadLoc;
	connHeadLoc.push_back(connHeadNode.x);
	connHeadLoc.push_back(connHeadNode.y);
	connHeadLoc.push_back(connHeadNode.z * zRATIO);

	vector<float> connTailLoc;
	connTailLoc.push_back(connTailNode.x);
	connTailLoc.push_back(connTailNode.y);
	connTailLoc.push_back(connTailNode.z * zRATIO);

	vector<float> elongDispUnitVec;
	vector<float> connDispUnitVec;
	if (connOrt == head_head)
	{
		elongDispUnitVec = this->getDispUnitVector(elongHeadLoc, elongTailLoc);
		connDispUnitVec = this->getDispUnitVector(connTailLoc, connHeadLoc);

		double radAngle = this->getPiAngle(elongDispUnitVec, connDispUnitVec);
		return radAngle;
	}
	else if (connOrt == head_tail)
	{
		elongDispUnitVec = this->getDispUnitVector(elongHeadLoc, elongTailLoc);
		connDispUnitVec = this->getDispUnitVector(connHeadLoc, connTailLoc);

		double radAngle = this->getPiAngle(elongDispUnitVec, connDispUnitVec);
		return radAngle;
	}
	else if (connOrt == tail_head)
	{
		elongDispUnitVec = this->getDispUnitVector(elongHeadLoc, elongTailLoc);
		connDispUnitVec = this->getDispUnitVector(connHeadLoc, connTailLoc);

		double radAngle = this->getPiAngle(elongDispUnitVec, connDispUnitVec);
		return radAngle;
	}
	else if (connOrt == tail_tail)
	{
		elongDispUnitVec = this->getDispUnitVector(elongTailLoc, elongHeadLoc);
		connDispUnitVec = this->getDispUnitVector(connHeadLoc, connTailLoc);

		double radAngle = this->getPiAngle(elongDispUnitVec, connDispUnitVec);
		return radAngle;
	}
}

double NeuronStructExplorer::segTurningAngle(const segUnit& elongSeg, const segUnit& connSeg, connectOrientation connOrt)
{
	// -- This method calculates the "turning angle" from elongating segment to connected segment. 
	// -- The turning angle is defined as the angle formed by displacement vector of elongating segment and the displacement vector from elongating point to connecting point.

	if (elongSeg.tails.size() > 1 || connSeg.tails.size() > 1)
	{
		cerr << "Invalid input: Currently segment elongation only allows to happen between 2 non-branching segments." << endl;
		return -1;
	}

	NeuronSWC elongHeadNode = elongSeg.nodes[elongSeg.seg_nodeLocMap.at(elongSeg.head)];
	NeuronSWC elongTailNode;
	if (elongSeg.tails.size() == 0)
	{
		//cerr << "Elongating segment only has head. Do nothinig and return.";
		return -1;
	}
	else elongTailNode = elongSeg.nodes[elongSeg.seg_nodeLocMap.at(*elongSeg.tails.begin())];

	NeuronSWC connHeadNode = connSeg.nodes[connSeg.seg_nodeLocMap.at(connSeg.head)];
	NeuronSWC connTailNode;
	if (connSeg.tails.size() == 0)
	{
		if (connOrt == head_tail || connOrt == tail_tail)
		{
			//cerr << "Connecting segment only has head. Do nothing and return";
			return -1;
		}
	}
	else connTailNode = connSeg.nodes[connSeg.seg_nodeLocMap.at(*connSeg.tails.begin())];

	vector<float> elongHeadLoc;
	elongHeadLoc.push_back(elongHeadNode.x);
	elongHeadLoc.push_back(elongHeadNode.y);
	elongHeadLoc.push_back(elongHeadNode.z * zRATIO);

	vector<float> elongTailLoc;
	elongTailLoc.push_back(elongTailNode.x);
	elongTailLoc.push_back(elongTailNode.y);
	elongTailLoc.push_back(elongTailNode.z * zRATIO);

	vector<float> elongDispUnitVec;
	vector<float> connPointUnitVec;
	if (connOrt == head_head)
	{
		elongDispUnitVec = NeuronStructExplorer::getDispUnitVector(elongHeadLoc, elongTailLoc);
		vector<float> connPointLoc;
		connPointLoc.push_back(connHeadNode.x);
		connPointLoc.push_back(connHeadNode.y);
		connPointLoc.push_back(connHeadNode.z * zRATIO);
		connPointUnitVec = NeuronStructExplorer::getDispUnitVector(connPointLoc, elongHeadLoc);

		double radAngle = NeuronStructExplorer::getPiAngle(elongDispUnitVec, connPointUnitVec);
		return radAngle;
	}
	else if (connOrt == head_tail)
	{
		elongDispUnitVec = NeuronStructExplorer::getDispUnitVector(elongHeadLoc, elongTailLoc);
		vector<float> connPointLoc;
		connPointLoc.push_back(connTailNode.x);
		connPointLoc.push_back(connTailNode.y);
		connPointLoc.push_back(connTailNode.z * zRATIO);
		connPointUnitVec = NeuronStructExplorer::getDispUnitVector(connPointLoc, elongHeadLoc);

		double radAngle = NeuronStructExplorer::getPiAngle(elongDispUnitVec, connPointUnitVec);
		return radAngle;
	}
	else if (connOrt == tail_head)
	{
		elongDispUnitVec = NeuronStructExplorer::getDispUnitVector(elongTailLoc, elongHeadLoc);
		vector<float> connPointLoc;
		connPointLoc.push_back(connHeadNode.x);
		connPointLoc.push_back(connHeadNode.y);
		connPointLoc.push_back(connHeadNode.z * zRATIO);
		connPointUnitVec = NeuronStructExplorer::getDispUnitVector(connPointLoc, elongTailLoc);

		double radAngle = NeuronStructExplorer::getPiAngle(elongDispUnitVec, connPointUnitVec);
		return radAngle;
	}
	else if (connOrt == tail_tail)
	{
		elongDispUnitVec = NeuronStructExplorer::getDispUnitVector(elongTailLoc, elongHeadLoc);
		vector<float> connPointLoc;
		connPointLoc.push_back(connTailNode.x);
		connPointLoc.push_back(connTailNode.y);
		connPointLoc.push_back(connTailNode.z * zRATIO);
		connPointUnitVec = NeuronStructExplorer::getDispUnitVector(connPointLoc, elongTailLoc);

		double radAngle = NeuronStructExplorer::getPiAngle(elongDispUnitVec, connPointUnitVec);
		return radAngle;
	}
}

segUnit NeuronStructExplorer::segmentStraighten(const segUnit& inputSeg)
{
	segUnit outputSeg = inputSeg;

	if (inputSeg.nodes.size() <= 3) return outputSeg;

	cout << " seg ID: " << inputSeg.segID << "   number of nodes contained in this segment: " << inputSeg.nodes.size() << endl;

	double segDistSqr = ((inputSeg.nodes.end() - 1)->x - inputSeg.nodes.begin()->x) * ((inputSeg.nodes.end() - 1)->x - inputSeg.nodes.begin()->x) + 
		((inputSeg.nodes.end() - 1)->y - inputSeg.nodes.begin()->y) * ((inputSeg.nodes.end() - 1)->y - inputSeg.nodes.begin()->y) +
		((inputSeg.nodes.end() - 1)->z - inputSeg.nodes.begin()->z) * ((inputSeg.nodes.end() - 1)->z - inputSeg.nodes.begin()->z);
	double segDist = sqrt(segDistSqr);
	cout << "segment displacement: " << segDist << endl << endl;
	
	for (QList<NeuronSWC>::const_iterator check = inputSeg.nodes.begin(); check != inputSeg.nodes.end(); ++check) cout << "[" << check->x << " " << check->y << " " << check->z << "] ";
	cout << endl;

	vector<profiledNode> pickedNode;
	double dot, sq1, sq2, dist, turnCost, radAngle, turnCostSum = 0, turnCostMean;
	double nodeDeviation, nodeHeadDistSqr, nodeHeadRadAngle, nodeToMainDist, nodeToMainDistSum = 0, nodeToMainDistMean;
	for (QList<NeuronSWC>::const_iterator it = inputSeg.nodes.begin() + 1; it != inputSeg.nodes.end() - 1; ++it)
	{
		dot = ((it - 1)->x - it->x) * ((it + 1)->x - it->x) + ((it - 1)->y - it->y) * ((it + 1)->y - it->y) + ((it - 1)->z - it->z) * ((it + 1)->z - it->z);
		sq1 = ((it - 1)->x - it->x) * ((it - 1)->x - it->x) + ((it - 1)->y - it->y) * ((it - 1)->y - it->y) + ((it - 1)->z - it->z) * ((it - 1)->z - it->z);
		sq2 = ((it + 1)->x - it->x) * ((it + 1)->x - it->x) + ((it + 1)->y - it->y) * ((it + 1)->y - it->y) + ((it + 1)->z - it->z) * ((it + 1)->z - it->z);
		if (isnan(acos(dot / sqrt(sq1 * sq2)))) return outputSeg;
		radAngle = acos(dot / sqrt(sq1 * sq2));
		
		nodeDeviation = (it->x - inputSeg.nodes.begin()->x) * ((inputSeg.nodes.end() - 1)->x - inputSeg.nodes.begin()->x) +
			(it->y - inputSeg.nodes.begin()->y) * ((inputSeg.nodes.end() - 1)->y - inputSeg.nodes.begin()->y) +
			(it->z - inputSeg.nodes.begin()->z) * ((inputSeg.nodes.end() - 1)->z - inputSeg.nodes.begin()->z);
		nodeHeadDistSqr = (it->x - inputSeg.nodes.begin()->x) * (it->x - inputSeg.nodes.begin()->x) + 
			(it->y - inputSeg.nodes.begin()->y) * (it->y - inputSeg.nodes.begin()->y) + 
			(it->z - inputSeg.nodes.begin()->z) * (it->z - inputSeg.nodes.begin()->z);
		nodeHeadRadAngle = PI - acos(nodeDeviation / sqrt(segDistSqr * nodeHeadDistSqr));
		nodeToMainDist = sqrt(nodeHeadDistSqr) * sin(nodeHeadRadAngle);
		nodeToMainDistSum = nodeToMainDistSum + nodeToMainDist;
		cout << "       d(node-main):" << nodeToMainDist << " radian/pi:" << (radAngle / PI) << " turning cost:" << (sqrt(sq1) + sqrt(sq2)) / (radAngle / PI) << " " << it->x << " " << it->y << " " << it->z << endl;

		if ((radAngle / PI) < 0.6) // Detecting sharp turns and distance outliers => a) obviously errorneous depth situation
		{
			profiledNode sharp;
			sharp.x = it->x; sharp.y = it->y; sharp.z = it->z;
			cout << "this node is picked" << endl;

			sharp.segID = inputSeg.segID;
			sharp.distToMainRoute = nodeToMainDist;
			sharp.previousSqr = sq1; sharp.nextSqr = sq2; sharp.innerProduct = dot;
			sharp.radAngle = radAngle;
			sharp.index = int(it - inputSeg.nodes.begin());
			sharp.turnCost = (sqrt(sq1) + sqrt(sq2)) / (radAngle / PI);

			pickedNode.push_back(sharp);
			turnCostSum = turnCostSum + sharp.turnCost;
		}
	}
	if (pickedNode.empty()) return outputSeg;

	nodeToMainDistMean = nodeToMainDistSum / (inputSeg.nodes.size() - 2);
	turnCostMean = turnCostSum / pickedNode.size();

	cout << endl << endl << "  ==== start deleting nodes... " << endl;
	int delete_count = 0;
	vector<int> delLocs;
	for (vector<profiledNode>::iterator it = pickedNode.begin(); it != pickedNode.end(); ++it)
	{
		cout << "  Avg(d(node_main)):" << nodeToMainDistMean << " d(node-main):" << it->distToMainRoute << " Avg(turning cost):" << turnCostMean << " turning cost:" << it->turnCost;
		cout << " [" << it->x << " " << it->y << " " << it->z << "] " << endl;
		if (it->distToMainRoute >= nodeToMainDistMean || it->turnCost >= turnCostMean || it->distToMainRoute >= segDist)
		{
			outputSeg.nodes[it->index + 1].parent = -1;
			delLocs.push_back(it->index);
		}
	}
	sort(delLocs.rbegin(), delLocs.rend());
	for (vector<int>::iterator it = delLocs.begin(); it != delLocs.end(); ++it) outputSeg.nodes.erase(outputSeg.nodes.begin() + ptrdiff_t(*it));
	delLocs.clear();

	cout << endl << "  ==== cheking angles... " << endl;
	int deleteCount2;
	do
	{
		deleteCount2 = 0;
		for (QList<NeuronSWC>::iterator it = outputSeg.nodes.begin() + 1; it != outputSeg.nodes.end() - 1; ++it)
		{
			double dot2 = (it->x - (it - 1)->x) * ((it + 1)->x - it->x) + (it->y - (it - 1)->y) * ((it + 1)->y - it->y) + (it->z - (it - 1)->z) * ((it + 1)->z - it->z);
			double sq1_2 = ((it - 1)->x - it->x) * ((it - 1)->x - it->x) + ((it - 1)->y - it->y) * ((it - 1)->y - it->y) + ((it - 1)->z - it->z) * ((it - 1)->z - it->z);
			double sq2_2 = ((it + 1)->x - it->x) * ((it + 1)->x - it->x) + ((it + 1)->y - it->y) * ((it + 1)->y - it->y) + ((it + 1)->z - it->z) * ((it + 1)->z - it->z);
			if (isnan(acos(dot2 / sqrt(sq1_2 * sq2_2)))) break;
			double radAngle_2 = acos(dot2 / sqrt(sq1_2 * sq2_2));
			cout << "2nd rad Angle:" << radAngle_2 << " [" << it->x << " " << it->y << " " << it->z << "]" << endl;

			if ((radAngle_2 / PI) * 180 > 75)
			{
				if (sqrt(sq1_2) > (1 / 10) * sqrt(sq2_2))
				{
					++deleteCount2;
					cout << "delete " << " [" << it->x << " " << it->y << " " << it->z << "] " << deleteCount2 << endl;
					if ((outputSeg.nodes.size() - deleteCount2) <= 2)
					{
						--deleteCount2;
						break;
					}

					(it + 1)->parent = -1;
					delLocs.push_back(int(it - outputSeg.nodes.begin()));
				}
			}
		}
		sort(delLocs.rbegin(), delLocs.rend());
		for (vector<int>::iterator it = delLocs.begin(); it != delLocs.end(); ++it) outputSeg.nodes.erase(outputSeg.nodes.begin() + ptrdiff_t(*it));
		delLocs.clear();
		cout << "deleted nodes: " << deleteCount2 << "\n=================" << endl;
	} while (deleteCount2 > 0);

	size_t label = 1;
	cout << "number of nodes after straightening process: " << outputSeg.nodes.size() << " ( segID = " << outputSeg.segID << " )" << endl;
	//cout << "seg num: " << curImgPtr->tracedNeuron.seg.size() << endl;
	for (QList<NeuronSWC>::iterator it = outputSeg.nodes.begin() + 1; it != outputSeg.nodes.end() - 1; ++it)
	{
		if (it->parent == -1) it->parent = (it - 1)->n;
	}

	return outputSeg;
}
/* ============================================= End of [Geometry] ============================================= */



/* ====================================== Neuron Struct Refining Method ====================================== */
profiledTree NeuronStructExplorer::spikeRemove(const profiledTree& inputProfiledTree, int spikeNodeNum)
{
	profiledTree processTree = inputProfiledTree;
	NeuronStructExplorer myExplorer;
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
			myExplorer.profiledTreeReInit(processTree);
			++currNodeNum;
		}
	}

	return processTree;
}
/* ================================== END of [Neuron Struct Refining Method] ================================== */










void NeuronStructExplorer::falsePositiveList(NeuronTree* detectedTreePtr, NeuronTree* manualTreePtr, float distThreshold)
{
	// -- Determing false positives out of the detected structure by the threshold of specified distance to the manual structure.
	// -- if (the shortest distance of node i from detectedTree to manualTree > distThreshold) node i is deemed false positive.

	long int nodeCount = 1;
	for (QList<NeuronSWC>::iterator it = detectedTreePtr->listNeuron.begin(); it != detectedTreePtr->listNeuron.end(); ++it)
	{
		cout << "Scanning detected node " << nodeCount << ".. " << endl;
		++nodeCount;

		QList<NeuronSWC>::iterator checkIt = manualTreePtr->listNeuron.begin();
		while (checkIt != manualTreePtr->listNeuron.end())
		{
			float xSquare = (it->x - checkIt->x) * (it->x - checkIt->x);
			float ySquare = (it->y - checkIt->y) * (it->y - checkIt->y);
			float zSquare = (it->z - checkIt->z) * (it->z - checkIt->z);
			float thisNodeDist = sqrtf(xSquare + ySquare + zSquare);

			if (thisNodeDist < distThreshold)
			{
				this->processedTree.listNeuron.push_back(*it);
				break;
			}

			++checkIt;
			if (checkIt == manualTreePtr->listNeuron.end())
			{
				vector<float> FPcoords;
				FPcoords.push_back(it->x);
				FPcoords.push_back(it->y);
				FPcoords.push_back(it->z);
				this->FPsList.push_back(FPcoords);
				FPcoords.clear();
			}
		}
	}
}

void NeuronStructExplorer::falseNegativeList(NeuronTree* detectedTreePtr, NeuronTree* manualTreePtr, float distThreshold)
{
	// -- False negavies are selected out of the manual structure if none of detected nodes are close enough to them.
	// -- if (the shortest distance of node i from manualTree to detectedTree > distThreshold) node i is deemed false negative.

	long int nodeCount = 1;
	for (QList<NeuronSWC>::iterator it = manualTreePtr->listNeuron.begin(); it != manualTreePtr->listNeuron.end(); ++it)
	{
		cout << "Scanning detected node " << nodeCount << ".. " << endl;
		++nodeCount;

		QList<NeuronSWC>::iterator checkIt = detectedTreePtr->listNeuron.begin();
		while (checkIt != detectedTreePtr->listNeuron.end())
		{
			float xSquare = (it->x - checkIt->x) * (it->x - checkIt->x);
			float ySquare = (it->y - checkIt->y) * (it->y - checkIt->y);
			float zSquare = (it->z - checkIt->z) * (it->z - checkIt->z);
			float thisNodeDist = sqrtf(xSquare + ySquare + zSquare);

			if (thisNodeDist > distThreshold)
			{
				vector<float> FNcoords;
				FNcoords.push_back(it->x);
				FNcoords.push_back(it->y);
				FNcoords.push_back(it->z);
				this->FNsList.push_back(FNcoords);
				FNcoords.clear();

				break;
			}

			++checkIt;
		}
	}
}

void NeuronStructExplorer::detectedDist(NeuronTree* inputTreePtr1, NeuronTree* inputTreePtr2)
{
	// -- Compute the shortest distance of each node from input structure (inputTreePtr1) to refernce structure (inputTreePtr2) 
	// -- The order of inputTreePtr1 and inputTreePtr2 represent the distances from opposite comparing directions.

	vector<float> minDists;
	long int count = 1;
	for (QList<NeuronSWC>::iterator it1 = inputTreePtr1->listNeuron.begin(); it1 != inputTreePtr1->listNeuron.end(); ++it1)
	{
		++count;
		//cout << "node No. " << count << ".." << endl;
		float minDist = 10000;
		for (QList<NeuronSWC>::iterator it2 = inputTreePtr2->listNeuron.begin(); it2 != inputTreePtr2->listNeuron.end(); ++it2)
		{
			float xSquare = ((it2->x) - (it1->x)) * ((it2->x) - (it1->x));
			float ySquare = ((it2->y) - (it1->y)) * ((it2->y) - (it1->y));
			float zSquare = ((it2->z) - (it1->z)) * ((it2->z) - (it1->z));
			float currDist = sqrtf(xSquare + ySquare + zSquare);

			if (currDist <= minDist) minDist = currDist;
		}
		minDists.push_back(minDist);
	}

	float distSum = 0;
	for (vector<float>::iterator it = minDists.begin(); it != minDists.end(); ++it) distSum = distSum + *it;
	float distMean = distSum / minDists.size();
	float varSum = 0;
	for (int i = 0; i < minDists.size(); ++i) varSum = varSum + (minDists[i] - distMean) * (minDists[i] - distMean);
	float distVar = varSum / minDists.size();
	float distStd = sqrtf(distVar);
	
	cout << distMean << " " << distStd << endl;
}

void NeuronStructExplorer::shortestDistCDF(NeuronTree* inputTreePtr1, NeuronTree* inputTreePtr2, int upperBound, int binNum)
{
	// -- Compute the PDF and CDF of shortest distance of detected nodes to manual nodes.

	float interval = float(upperBound) / float(binNum);
	vector<float> shortestDists;
	for (QList<NeuronSWC>::iterator it1 = inputTreePtr1->listNeuron.begin(); it1 != inputTreePtr1->listNeuron.end(); ++it1)
	{
		float minDist = 10000;
		for (QList<NeuronSWC>::iterator it2 = inputTreePtr2->listNeuron.begin(); it2 != inputTreePtr2->listNeuron.end(); ++it2)
		{
			float xSquare = ((it2->x) - (it1->x)) * ((it2->x) - (it1->x));
			float ySquare = ((it2->y) - (it1->y)) * ((it2->y) - (it1->y));
			float zSquare = ((it2->z) - (it1->z)) * ((it2->z) - (it1->z));
			float currDist = sqrtf(xSquare + ySquare + zSquare);

			if (currDist <= minDist) minDist = currDist;
		}
		shortestDists.push_back(minDist);
	}

	for (int i = 0; i < binNum; ++i)
	{
		this->nodeDistPDF[i] = 0;
		this->nodeDistPDF[i] = 0;
	}
	for (vector<float>::iterator distIt = shortestDists.begin(); distIt != shortestDists.end(); ++distIt)
	{
		int binNo = (*distIt) / interval;
		++(this->nodeDistPDF[binNo]);
	}
	this->nodeDistCDF[0] = this->nodeDistPDF[0];
	for (int i = 1; i < binNum; ++i) this->nodeDistCDF[i] = this->nodeDistCDF[i - 1] + nodeDistPDF[i];
}