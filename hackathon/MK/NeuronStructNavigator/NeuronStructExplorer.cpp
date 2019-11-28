//------------------------------------------------------------------------------
// Copyright (c) 2018 Hsienchi Kuo (Allen Institute, Hanchuan Peng's team)
// All rights reserved.
//------------------------------------------------------------------------------

/*******************************************************************************
*
*  NeuronStructExplorer class intends to manage Neuron data struct by providing the following functionalities:
*
*    a. [profiledTree] data struct management
*    b. Essential segment profiling methods, i.e., NeuronStructExplorer::findSegs, NeuronStructExplorer::segTileMap, and NeuronStructExplorer::getSegHeadTailClusters, etc.
*    c. Inter/intra neuron struct analysis.
*
*  This is the base class of other derived class including TreeGrower, etc.
*  Since segment profiling methods are critical and often are the foundation of higher level algorithms which is mainly included in derived classes,
*  implementing these methods in the base class grants the derived class direct access to them and makes the development cleaner and more convenient.
*
********************************************************************************/

#include <iostream>
#include <algorithm>
#include <cmath>

#include "basic_4dimage.h"
#include "basic_landmark.h"
#include "neuron_format_converter.h"

#include "NeuronStructExplorer.h"
#include "NeuronStructNavigator_Define.h"

/* ================================ Constructors and Basic Profiling Data/Function Members ================================ */
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

void NeuronStructExplorer::segMorphProfile(profiledTree& inputProfiledTree, int range)
{
	int halfRange = (range - 1) / 2;
	if (halfRange < 1)
	{
		cerr << "Invalid search range. Return." << endl;
		return;
	}

	for (map<int, segUnit>::iterator segIt = inputProfiledTree.segs.begin(); segIt != inputProfiledTree.segs.end(); ++segIt)
	{
		//cout << segIt->first << ": " << segIt->second.nodes.size() << endl;
		if (segIt->second.nodes.size() < range) continue;
		boost::container::flat_map<int, map<string, double>> segSmoothness;
		for (QList<NeuronSWC>::iterator nodeIt = segIt->second.nodes.begin() + ptrdiff_t(halfRange); nodeIt != segIt->second.nodes.end() - ptrdiff_t(halfRange); ++nodeIt)
		{
			if (nodeIt->parent == -1 || segIt->second.seg_childLocMap.at(nodeIt->n).size() == 0) continue;
			//cout << "  " << nodeIt->n << ": ";
			
			NeuronSWC currNodePaRoot = *nodeIt;
			NeuronSWC currNodeChildRoot = *nodeIt;
			double length = 0;
			map<string, double> currNodeStatMap;
			for (int i = 1; i <= halfRange; ++i)
			{
				NeuronSWC currPaNode = segIt->second.nodes.at(segIt->second.seg_nodeLocMap.at(currNodePaRoot.parent));
				NeuronSWC currChildNode = segIt->second.nodes.at(*segIt->second.seg_childLocMap.at(currNodeChildRoot.n).begin());
				length = length + sqrt((currPaNode.x - currNodePaRoot.x) * (currPaNode.x - currNodePaRoot.x) +
								       (currPaNode.y - currNodePaRoot.y) * (currPaNode.y - currNodePaRoot.y) +
									   (currPaNode.z - currNodePaRoot.z) * (currPaNode.z - currNodePaRoot.z));
				//cout << length << "(" << currPaNode.x << "," << currPaNode.y << "," << currPaNode.z << ")(" << currNodePaRoot.x << "," << currNodePaRoot.y << "," << currNodePaRoot.z << ")   ";
				length = length + sqrt((currChildNode.x - currNodeChildRoot.x) * (currChildNode.x - currNodeChildRoot.x) +
									   (currChildNode.y - currNodeChildRoot.y) * (currChildNode.y - currNodeChildRoot.y) +
									   (currChildNode.z - currNodeChildRoot.z) * (currChildNode.z - currNodeChildRoot.z));
				//cout << length << "(" << currChildNode.x << "," << currChildNode.y << "," << currChildNode.z << ")(" << currNodeChildRoot.x << "," << currNodeChildRoot.y << "," << currNodeChildRoot.z << ")" << endl;
				currNodePaRoot = currPaNode;
				currNodeChildRoot = currChildNode;
			}
			currNodeStatMap.insert({ "length", length });

			double dist = sqrt((currNodePaRoot.x - currNodeChildRoot.x) * (currNodePaRoot.x - currNodeChildRoot.x) + (currNodePaRoot.y - currNodeChildRoot.y) * (currNodePaRoot.y - currNodeChildRoot.y) + (currNodePaRoot.z - currNodeChildRoot.z) * (currNodePaRoot.z - currNodeChildRoot.z));
			currNodeStatMap.insert({ "distance", dist });

			segSmoothness.insert(pair<int, map<string, double>>(nodeIt->n, currNodeStatMap));
		}
		//cout << endl;
		segIt->second.segSmoothnessMap.insert(pair<int, boost::container::flat_map<int, map<string, double>>>(range, segSmoothness));
	}
}

void NeuronStructExplorer::__segMorphProfiled_lengthDistRatio(profiledTree& inputProfiledTree, int range, double lengthDistRatio)
{
	for (map<int, segUnit>::iterator it = inputProfiledTree.segs.begin(); it != inputProfiledTree.segs.end(); ++it)
	{
		if (it->second.nodes.size() < range) continue;
		for (boost::container::flat_map<int, map<string, double>>::iterator nodeIt = it->second.segSmoothnessMap.at(3).begin(); nodeIt != it->second.segSmoothnessMap.at(3).end(); ++nodeIt)
		{
			if (nodeIt->second.at("length") / nodeIt->second.at("distance") >= lengthDistRatio)
				inputProfiledTree.tree.listNeuron[inputProfiledTree.node2LocMap.at(nodeIt->first)].type = 15;
		}
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

#ifdef __SEGEND_CLUSTER_DEBUG__
	QString outputPath = "C:\\Users\\hsienchik\\Desktop\\Work\\FragTrace\\debug\\";
	QDir outputDir(outputPath);
	outputDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
	QStringList segEndTestList = outputDir.entryList();
	int num = 0;
	for (QStringList::iterator it = segEndTestList.begin(); it != segEndTestList.end(); ++it)
		if (it->contains("segEndCluster")) ++num;
	string segEndTestFullPath = "C:\\Users\\hsienchik\\Desktop\\Work\\FragTrace\\debug\\segEndCluster" + to_string(num + 1) + ".swc";
	this->__segEndClusteringExam(inputProfiledTree, segEndTestFullPath);
#endif
}

void NeuronStructExplorer::mergeTileBasedSegClusters(profiledTree& inputProfiledTree, float distThreshold)
{
	// -- This method merges head/tail clusters across tiles and reaches the final form and correctness of [inputProfiledTree]::segHeadClusters and [inputProfiledTree]::segTailClusters.
	// -- Strategy: 1. Copy mergeable head clusters from source clusters to destined clusters. Their tail cluster counterparts are also copied over correspondingly.
	//              2. Merge head and tail clusters that have segments in common.
	//              3. Assign those tails from the tiles where no head exists by searching connectable heads in their neighboring tiles.
	//                 -> Because tails are copied and merged by following head cluster's footstep. Those tails without heads in the tile will be missed out.
	//                 -> Possible issues:
	//                     a. There may be head clusters ought to be merged due to identified tails that join them together however is not recognized. 
	//                        This is because the process handles head clusters first and separately. The reason of this approach is for efficiency purpose.
	//              4. Merge tail clusters that have segments in common.
	//                 -> This step is required because there is a tail cluster step after 1. and 2. Consequently, there could be duplicated clusters.
	//
	// -- This method has been intensively tested. Apart from the mentioned minor issue that could cause insignificant errors, overall it's running correctly. 
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
	// -- This method clusters segment heads and tails tile by tile -> clusters in 1 tile are independent of those in neighboring tiles.
	// NOTE: Nodes are confined within their own tiles. Therefore, cluster merging correctness is irrelevant in this method and doesn't provide any information for that matter.
	// Therefore, the information stored in [inputProfiledTree::segHeadClusters] and [inputProfiledTree::segTailClusters] are not in their final form and not correct.

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
	writeSWC_file("H:\\fMOST_fragment_tracing\\testCase1\\tiled_headEndTest_clusterIndex.swc", testTree.tree);
	testTree = inputProfiledTree;
	for (boost::container::flat_map<int, boost::container::flat_set<int>>::iterator it = testTree.segTailClusters.begin(); it != testTree.segTailClusters.end(); ++it)
	{
		for (boost::container::flat_set<int>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			testTree.tree.listNeuron[testTree.node2LocMap.at(*testTree.segs.at(*it2).tails.begin())].type = it->first % 9;
			testTree.tree.listNeuron[testTree.node2LocMap.at(*testTree.segs.at(*it2).tails.begin())].radius = it->first;
		}
	}
	writeSWC_file("H:\\fMOST_fragment_tracing\\testCase1\\tiled_tailEndTest_clusterIndex.swc", testTree.tree);
	testTree = inputProfiledTree;
	for (boost::container::flat_map<int, int>::iterator it = testTree.headSeg2ClusterMap.begin(); it != testTree.headSeg2ClusterMap.end(); ++it)
	{
		testTree.tree.listNeuron[testTree.node2LocMap.at(testTree.segs.at(it->first).head)].type = it->second % 9;
		testTree.tree.listNeuron[testTree.node2LocMap.at(testTree.segs.at(it->first).head)].radius = it->second;
	}
	writeSWC_file("H:\\fMOST_fragment_tracing\\testCase1\\tiled_headEndTest.swc", testTree.tree);
	testTree = inputProfiledTree;
	for (boost::container::flat_map<int, int>::iterator it = testTree.tailSeg2ClusterMap.begin(); it != testTree.tailSeg2ClusterMap.end(); ++it)
	{
		testTree.tree.listNeuron[testTree.node2LocMap.at(*testTree.segs.at(it->first).tails.begin())].type = it->second % 9;
		testTree.tree.listNeuron[testTree.node2LocMap.at(*testTree.segs.at(it->first).tails.begin())].radius = it->second;
	}
	writeSWC_file("H:\\fMOST_fragment_tracing\\testCase1\\tiled_tailEndTest.swc", testTree.tree);*/
}

void NeuronStructExplorer::getClusterSegPairs(profiledTree& inputProfiledTree)
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
				inputProfiledTree.cluster2segPairMap.insert(pair<int, vector<segPairProfile>>(clusterIt->first, segPairs));
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
				inputProfiledTree.cluster2segPairMap.insert(pair<int, vector<segPairProfile>>(clusterIt->first, segPairs));
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

				inputProfiledTree.cluster2segPairMap.insert(pair<int, vector<segPairProfile>>(clusterIt->first, segPairs));
			}
		}
	}
}

void NeuronStructExplorer::segmentDecompose(NeuronTree* inputTreePtr)
{
	// -- This function used get_link_map and decompose in v_neuronswc.cpp to get segment hierarchical information.

	this->segmentList.clear();
	this->segmentList = NeuronTree__2__V_NeuronSWC_list(*inputTreePtr);
}
/* =============================== END of [Constructors and Basic Data/Function Members] =============================== */



/* ============================ Tree - Subtree Operations ============================= */
void NeuronStructExplorer::downstream_subTreeExtract(const QList<NeuronSWC>& inputList, QList<NeuronSWC>& subTreeList, const NeuronSWC& startingNode, map<int, size_t>& node2locMap, map<int, vector<size_t>>& node2childLocMap)
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

void NeuronStructExplorer::wholeSingleTree_extract(const QList<NeuronSWC>& inputList, QList<NeuronSWC>& tracedList, const NeuronSWC& startingNode)
{
	map<int, size_t> node2locMap;
	map<int, vector<size_t>> node2childLocMap;
	NeuronStructUtil::node2loc_node2childLocMap(inputList, node2locMap, node2childLocMap);

	if (startingNode.parent == -1) NeuronStructExplorer::downstream_subTreeExtract(inputList, tracedList, startingNode, node2locMap, node2childLocMap);
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
		NeuronStructExplorer::downstream_subTreeExtract(inputList, tracedList, rootNode, node2locMap, node2childLocMap);
	}
}
/* ======================== END of [Tree - Subtree Operations] ======================== */



/* ========================================= Auto-tracing Related Neuron Struct Functions ========================================= */
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
				vector<float> seg1Vector = NeuronGeoGrapher::getVector_NeuronSWC<float>(segUnit1.nodes.at(segUnit1.seg_nodeLocMap.at(segUnit1.head)), segUnit1.nodes.at(segUnit1.seg_nodeLocMap.at(*segUnit1.tails.begin())));
				vector<float> seg2Vector = NeuronGeoGrapher::getVector_NeuronSWC<float>(segUnit2.nodes.at(segUnit2.seg_nodeLocMap.at(*segUnit2.tails.begin())), segUnit2.nodes.at(segUnit2.seg_nodeLocMap.at(segUnit2.head)));
				double turnAngle = NeuronGeoGrapher::getPiAngle(seg1Vector, seg2Vector);
				if (turnAngle > PI / 4)
				{
					distMap.erase(distMap.find(nearestPair.first));
					//cout << nearestPair.first << " " << nearestPair.second << " ";
					//cout << "route turning back, move to the next" << " distMap size: " << distMap.size() << endl;
					continue;
				}

				segUnit connectedSeg = NeuronStructUtil:: segUnitConnect_executer(segUnit1, segUnit2, head_head);
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
				vector<float> seg1Vector = NeuronGeoGrapher::getVector_NeuronSWC<float>(segUnit1.nodes.at(segUnit1.seg_nodeLocMap.at(segUnit1.head)), segUnit1.nodes.at(segUnit1.seg_nodeLocMap.at(*segUnit1.tails.begin())));
				vector<float> seg2Vector = NeuronGeoGrapher::getVector_NeuronSWC<float>(segUnit2.nodes.at(segUnit2.seg_nodeLocMap.at(segUnit2.head)), segUnit2.nodes.at(segUnit2.seg_nodeLocMap.at(*segUnit2.tails.begin())));
				double turnAngle = NeuronGeoGrapher::getPiAngle(seg1Vector, seg2Vector);
				if (turnAngle > PI / 4)
				{
					distMap.erase(distMap.find(nearestPair.first));
					//cout << nearestPair.first << " " << nearestPair.second << " ";
					//cout << "route turning back, move to the next" << " distMap size: " << distMap.size() << endl;
					continue;
				}

				segUnit connectedSeg = NeuronStructUtil:: segUnitConnect_executer(segUnit1, segUnit2, head_tail);
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
				vector<float> seg1Vector = NeuronGeoGrapher::getVector_NeuronSWC<float>(segUnit1.nodes.at(segUnit1.seg_nodeLocMap.at(*segUnit1.tails.begin())), segUnit1.nodes.at(segUnit1.seg_nodeLocMap.at(segUnit1.head)));
				vector<float> seg2Vector = NeuronGeoGrapher::getVector_NeuronSWC<float>(segUnit2.nodes.at(segUnit2.seg_nodeLocMap.at(segUnit2.head)), segUnit2.nodes.at(segUnit2.seg_nodeLocMap.at(*segUnit2.tails.begin())));
				double turnAngle = NeuronGeoGrapher::getPiAngle(seg1Vector, seg2Vector);
				if (turnAngle > PI / 4)
				{
					distMap.erase(distMap.find(nearestPair.first));
					//cout << nearestPair.first << " " << nearestPair.second << " ";
					//cout << "route turning back, move to the next" << " distMap size: " << distMap.size() << endl;
					continue;
				}

				segUnit connectedSeg = NeuronStructUtil:: segUnitConnect_executer(segUnit1, segUnit2, tail_tail);
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

				double pointingRadAngle = NeuronGeoGrapher::segPointingCompare(currProfiledTree.segs.at(*headIt), currProfiledTree.segs.at(*tailIt), head_tail);
				double turningRadAngle = NeuronGeoGrapher::segTurningAngle(currProfiledTree.segs.at(*headIt), currProfiledTree.segs.at(*tailIt), head_tail);
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
		segUnit newSeg = NeuronStructUtil:: segUnitConnect_executer(currProfiledTree.segs[it->first], currProfiledTree.segs[it->second], head_tail);
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

				double pointingRadAngle = NeuronGeoGrapher::segPointingCompare(currProfiledTree.segs.at(*tailIt), currProfiledTree.segs.at(*headIt), tail_head);
				double turningRadAngle = NeuronGeoGrapher::segTurningAngle(currProfiledTree.segs.at(*tailIt), currProfiledTree.segs.at(*headIt), tail_head);
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
		segUnit newSeg = NeuronStructUtil:: segUnitConnect_executer(currProfiledTree.segs[it->first], currProfiledTree.segs[it->second], tail_head);
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

				double pointingRadAngle = NeuronGeoGrapher::segPointingCompare(currProfiledTree.segs.at(*headIt1), currProfiledTree.segs.at(*headIt2), head_head);
				double turningRadAngle = NeuronGeoGrapher::segTurningAngle(currProfiledTree.segs.at(*headIt1), currProfiledTree.segs.at(*headIt2), head_head);
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
		segUnit newSeg = NeuronStructUtil:: segUnitConnect_executer(currProfiledTree.segs[it->first], currProfiledTree.segs[it->second], head_head);
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

				double pointingRadAngle = NeuronGeoGrapher::segPointingCompare(currProfiledTree.segs.at(*tailIt1), currProfiledTree.segs.at(*tailIt2), tail_tail);
				double turningRadAngle = NeuronGeoGrapher::segTurningAngle(currProfiledTree.segs.at(*tailIt1), currProfiledTree.segs.at(*tailIt2), tail_tail);
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
		segUnit newSeg = NeuronStructUtil:: segUnitConnect_executer(currProfiledTree.segs[it->first], currProfiledTree.segs[it->second], tail_tail);
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

profiledTree NeuronStructExplorer::treeHollow(const profiledTree& inputProfiledTree, const float hollowCenterX, const float hollowCenterY, const float hollowCenterZ, const float radius)
{
	QList<NeuronSWC> outputList;
	for (map<int, segUnit>::const_iterator inputSegIt = inputProfiledTree.segs.begin(); inputSegIt != inputProfiledTree.segs.end(); ++inputSegIt)
	{
		bool out, in;
		for (QList<NeuronSWC>::const_iterator inputNodeIt = inputSegIt->second.nodes.begin(); inputNodeIt != inputSegIt->second.nodes.end(); ++inputNodeIt)
		{
			float dist2center = sqrtf((inputNodeIt->x - hollowCenterX) * (inputNodeIt->x - hollowCenterX) + (inputNodeIt->y - hollowCenterY) * (inputNodeIt->y - hollowCenterY) + (inputNodeIt->z - hollowCenterZ) * (inputNodeIt->z - hollowCenterZ) / 4);
			if (dist2center >= radius)
			{
				if (inputNodeIt == inputSegIt->second.nodes.begin())
				{
					out = true;
					in = false;
					NeuronSWC newNode;
					newNode = *inputNodeIt;
					outputList.push_back(newNode);
					continue;
				}
				else
				{
					if (out && !in)
					{
						NeuronSWC newNode;
						newNode = *inputNodeIt;
						outputList.push_back(newNode);
						continue;
					}
					else if (!out && in)
					{
						NeuronSWC newNode;
						newNode = *inputNodeIt;
						newNode.parent = -1;
						outputList.push_back(newNode);
						out = true;
						in = false;
						continue;
					}
				}
			}
			else
			{
				if (inputNodeIt == inputSegIt->second.nodes.begin())
				{
					out = false;
					in = true;				
				}
				else
				{
					if (out && !in)
					{
						out = false;
						in = true;
					}
				}
			}
		}
	}

	NeuronTree outputTree;
	outputTree.listNeuron = outputList;
	profiledTree outputProfiledTree(outputTree);

	return outputProfiledTree;
}
/* ===================================== END of [Auto-tracing Related Neuron Struct Functions] ===================================== */



/* ============================================== Inter/Intra-SWC Comparison/Analysis ============================================== */
map<string, float> NeuronStructExplorer::selfNodeDist(const QList<NeuronSWC>& inputNodeList)
{
	boost::container::flat_map<string, vector<NeuronSWC>> labeledNodeMap;
	NeuronStructUtil::nodeTileMapGen(inputNodeList, labeledNodeMap, 30);

	float distSum = 0;
	vector<float> distVec;
	for (boost::container::flat_map<string, vector<NeuronSWC>>::iterator it = labeledNodeMap.begin(); it != labeledNodeMap.end(); ++it)
	{
		if (it->second.size() == 1)
		{
			float dist = 10000;
			for (QList<NeuronSWC>::const_iterator nodeIt = inputNodeList.begin(); nodeIt != inputNodeList.end(); ++nodeIt)
			{
				if (it->second.begin()->x == nodeIt->x && it->second.begin()->y == nodeIt->y && it->second.begin()->z == nodeIt->z)
					continue;

				float tempDist = sqrtf((it->second.begin()->x - nodeIt->x) * (it->second.begin()->x - nodeIt->x) +
					(it->second.begin()->y - nodeIt->y) * (it->second.begin()->y - nodeIt->y) +
					(it->second.begin()->z - nodeIt->z) * (it->second.begin()->z - nodeIt->z) * zRATIO * zRATIO);
				if (tempDist < dist) dist = tempDist;
			}
			distVec.push_back(dist);
			distSum = distSum + dist;
			continue;
		}

		for (vector<NeuronSWC>::iterator nodeIt1 = it->second.begin(); nodeIt1 != it->second.end(); ++nodeIt1)
		{
			float dist = 10000;
			for (vector<NeuronSWC>::iterator nodeIt2 = it->second.begin(); nodeIt2 != it->second.end(); ++nodeIt2)
			{
				if (nodeIt1 == nodeIt2) continue;

				float tempDist = sqrtf((nodeIt1->x - nodeIt2->x) * (nodeIt1->x - nodeIt2->x) +
					(nodeIt1->y - nodeIt2->y) * (nodeIt1->y - nodeIt2->y) +
					(nodeIt1->z - nodeIt2->z) * (nodeIt1->z - nodeIt2->z) * zRATIO * zRATIO);
				if (tempDist < dist) dist = tempDist;
			}
			distVec.push_back(dist);
			distSum = distSum + dist;
		}
	}

	float distMean = distSum / float(distVec.size());
	float distVarSum = 0;
	for (vector<float>::iterator it = distVec.begin(); it != distVec.end(); ++it) distVarSum = distVarSum + (*it - distMean) * (*it - distMean);
	float distVar = distVarSum / float(distVec.size());
	float distStd = sqrtf(distVar);
	sort(distVec.begin(), distVec.end());
	float distMedian = float(distVec.at(floor(distVec.size() / 2)));

	map<string, float> outputMap;
	outputMap.insert(pair<string, float>("mean", distMean));
	outputMap.insert(pair<string, float>("std", distStd));
	outputMap.insert(pair<string, float>("var", distVar));
	outputMap.insert(pair<string, float>("median", distMedian));

	return outputMap;
}

NeuronTree NeuronStructExplorer::swcIdentityCompare(const NeuronTree& subjectTree, const NeuronTree& refTree, float distThre, float nodeTileLength)
{
	map<string, vector<NeuronSWC>> gridSWCmap; // Better use vector instead of set here, as set by default sorts the elements.
	// This can cause complication if the element is a data struct.

	NeuronStructUtil::nodeTileMapGen(refTree, gridSWCmap, nodeTileLength);

	NeuronTree outputTree;
	NeuronTree refConfinedFilteredTree;
	for (QList<NeuronSWC>::const_iterator suIt = subjectTree.listNeuron.begin(); suIt != subjectTree.listNeuron.end(); ++suIt)
	{
		string xLabel = to_string(int((suIt->x) / (nodeTileLength)));
		string yLabel = to_string(int((suIt->y) / (nodeTileLength)));
		string zLabel = to_string(int((suIt->z) / (nodeTileLength / zRATIO)));
		string keyLabel = xLabel + "_" + yLabel + "_" + zLabel;

		if (gridSWCmap.find(keyLabel) != gridSWCmap.end())
		{
			bool identified = false;
			for (vector<NeuronSWC>::iterator nodeIt = gridSWCmap[keyLabel].begin(); nodeIt != gridSWCmap[keyLabel].end(); ++nodeIt)
			{
				float dist = sqrt((nodeIt->x - suIt->x) * (nodeIt->x - suIt->x) + (nodeIt->y - suIt->y) * (nodeIt->y - suIt->y) +
					zRATIO * zRATIO * (nodeIt->z - suIt->z) * (nodeIt->z - suIt->z));

				if (dist <= distThre)
				{
					outputTree.listNeuron.push_back(*suIt);
					(outputTree.listNeuron.end() - 1)->type = 2;
					identified = true;
					break;
				}
			}

			if (!identified)
			{
				outputTree.listNeuron.push_back(*suIt);
				(outputTree.listNeuron.end() - 1)->type = 3;
			}
		}
		else refConfinedFilteredTree.listNeuron.push_back(*suIt);
	}

	return outputTree;
}
/* ================================================================================================================================= */










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
				//this->processedTree.listNeuron.push_back(*it);
				//break;
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