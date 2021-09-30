#include "NeuronReconTester.h"
#include "ReconOperator.h"

#include "TreeTrimmer.h"

void ReconOperator::downSampleReconFile(const QStringList& fileList, float xFactor, float yFactor, float zFactor)
{
	QString newFolderName = this->rootPath + "\\downscaled";
	QDir newDir(newFolderName);
	if (!newDir.exists()) newDir.mkpath(".");

	for (auto& file : fileList)
	{
		QString fullFileName = this->rootPath + "\\" + file;
		if (file.endsWith("swc"))
		{
			QString outputFullName = newFolderName + "\\" + file.left(file.length() - 4) + "_xy" + QString::number(int(1 / xFactor)) + "z" + QString::number(int(1 / zFactor)) + ".swc";
			NeuronTree outputTree = NeuronStructUtil::swcScale(readSWC_file(fullFileName), xFactor, yFactor, zFactor);
			writeSWC_file(outputFullName, outputTree);
		}
		else if (file.endsWith("eswc"))
		{
			QString outputFullName = newFolderName + "\\" + file.left(file.length() - 5) + "_xy" + QString::number(int(1 / xFactor)) + "z" + QString::number(int(1 / zFactor)) + ".eswc";
			NeuronTree outputTree = NeuronStructUtil::swcScale(readSWC_file(fullFileName), xFactor, yFactor, zFactor);
			writeSWC_file(outputFullName, outputTree);
		}
		else if (file.endsWith("apo"))
		{
			QString outputFullName = newFolderName + "\\" + file.left(file.length() - 4) + "_xy" + QString::number(int(1 / xFactor)) + "z" + QString::number(int(1 / zFactor)) + ".apo";
			QList<CellAPO> outputApo = NeuronStructUtil::apoScale(readAPO_file(fullFileName), xFactor, yFactor, zFactor);
			writeAPO_file(outputFullName, outputApo);
		}
		else if (file.endsWith("ano"))
		{
			QString outputFullName = newFolderName + "\\" + file.left(file.length() - 4) + "_xy" + QString::number(int(1 / xFactor)) + "z" + QString::number(int(1 / zFactor)) + ".ano";
			QFile::copy(fullFileName, outputFullName);
		}
	}
}

void ReconOperator::denAxonSeparate(const QStringList& fileList)
{
	QString denFolderName = this->rootPath + "\\dendrite";
	QDir denDir(denFolderName);
	if (!denDir.exists()) denDir.mkpath(".");
	QString axonFolderName = this->rootPath + "\\axon";
	QDir axonDir(axonFolderName);
	if (!axonDir.exists()) axonDir.mkpath(".");

	for (auto& file : fileList)
	{
		if (file.endsWith("swc") || file.endsWith("eswc"))
		{
			QString fullSWCname = this->rootPath + "\\" + file;
			NeuronTree inputTree = readSWC_file(fullSWCname);
			map<int, QList<NeuronSWC>> swcTypeMap = NeuronStructUtil::swcSplitByType(inputTree);

			NeuronTree denTree;
			if (swcTypeMap.find(1) != swcTypeMap.end()) denTree.listNeuron.append(swcTypeMap.at(1));
			denTree.listNeuron.append(swcTypeMap.at(3));
			if (swcTypeMap.find(4) != swcTypeMap.end()) denTree.listNeuron.append(swcTypeMap.at(4));

			NeuronTree axonTree;
			axonTree.listNeuron.append(swcTypeMap.at(2));

			QString denSWCfullName = denFolderName + "\\" + file;
			writeSWC_file(denSWCfullName, denTree);
			QString axonSWCfullName = axonFolderName + "\\" + file;
			writeSWC_file(axonSWCfullName, axonTree);
		}
	}

	v3d_msg(QString("Neurite separation done."));
}

void ReconOperator::denAxonCombine(bool dupRemove)
{
	QDir inputDenFileFolder(this->rootPath + "\\dendrite");
	QDir inputAxonFileFolder(this->rootPath + "\\axon");
	if (!inputDenFileFolder.exists() || !inputAxonFileFolder.exists())
	{
		v3d_msg(QString("Dendrite folder and axon folder not existing. Do nothing and return."));
		return;
	}
	QString outputFolderName = this->rootPath + "\\denAxonCombined\\";
	QDir outputDir(outputFolderName);
	if (!outputDir.exists()) outputDir.mkpath(".");
	inputAxonFileFolder.setFilter(QDir::Files | QDir::NoDotAndDotDot);
	QStringList axonFileList = inputAxonFileFolder.entryList();

	for (auto& axonFile : axonFileList)
	{
		if (axonFile.endsWith(".ano") || axonFile.endsWith(".apo")) continue;

		vector<NeuronTree> trees;
		QString denFullName;
		QString denFullNameEswcQ = this->rootPath + "\\dendrite\\" + axonFile.left(axonFile.length() - 4) + ".ano.eswc";
		QString denFullNameSwcQ = this->rootPath + "\\dendrite\\" + axonFile;

		if (QFile::exists(denFullNameEswcQ)) denFullName = denFullNameEswcQ;
		else denFullName = denFullNameSwcQ;
		trees.push_back(readSWC_file(denFullName));
		QString axonFullName = this->rootPath + "\\axon\\" + axonFile;
		trees.push_back(readSWC_file(axonFullName));
		NeuronTree outputTree = NeuronStructUtil::swcCombine(trees);
		
		//if (dupRemove) outputTree = NeuronStructUtil::removeDupNodes(outputTree);
		QString outputSWCfullName = outputFolderName + axonFile;
		writeSWC_file(outputSWCfullName, outputTree);
	}
	
	v3d_msg(QString("Dendrite and axon conbination done."));
}

void ReconOperator::assembleSegs2tree()
{
#ifdef __ACTIVATE_TESTER__
	NeuronReconTester::instance(this);
	NeuronReconTester::getInstance()->rootPathQ = this->rootPath;
	if (GROUPED_TREES) NeuronReconTester::getInstance()->makeConnectedFolder();
	if (GROUPED_ASSEMBLED_TREES) NeuronReconTester::getInstance()->makeConnectedAssembledFolder();
#endif

	QDir inputFolder(this->rootPath);
	inputFolder.setFilter(QDir::Files | QDir::NoDotAndDotDot);
	QStringList fileList = inputFolder.entryList();

	QString outputFolderQ = this->rootPath + "\\allCleanedUp\\";
	QDir outputDir(outputFolderQ);
	if (!outputDir.exists()) outputDir.mkpath(".");

	QString errorFolderQ = this->rootPath + "\\notAssembled\\";
	QDir errorDir(errorFolderQ);

	QString tooFarType7folderQ = this->rootPath + "\\subtreeNotConnected\\";
	QDir type7dir(tooFarType7folderQ);

	for (auto& file : fileList)
	{
		QString baseName;
		if (file.endsWith(".swc")) baseName = file.left(file.length() - 4);
		else if (file.endsWith(".eswc")) baseName = file.left(file.length() - 5);

		bool apoFound = false;
		CellAPO somaAPO;
		if (!baseName.isEmpty())
		{
			QString targetAPOfileName = baseName + ".apo";
			for (auto& file2 : fileList)
			{
				if (file2 == targetAPOfileName)
				{
					somaAPO = *readAPO_file(this->rootPath + "\\" + file2).begin();
					apoFound = true;
					break;
				}
			}
		}
		else continue;
		
		qDebug() << "Processing Cell: " << baseName;
		if (apoFound)
		{
			cout << "Input soma coordinate: " << somaAPO.x << " " << somaAPO.y << " " << somaAPO.z << endl;
			QString inputSWCFullName = this->rootPath + "\\" + file;
			NeuronTree inputTree = readSWC_file(inputSWCFullName);


			/***************** Remove duplicated segments *****************/
			NeuronTree noDupSegTree = NeuronStructUtil::removeDupStructures(inputTree);
			if (NeuronReconTester::getInstance() != nullptr && DUPSEG_REMOVE) NeuronReconTester::getInstance()->saveIntermediateResult(noDupSegTree, "noDupSegs", baseName);
			/**************************************************************/


			/***************** Remove redundant nodes *****************/
			profiledTree inputProfiledTree(noDupSegTree);
			NeuronStructUtil::removeRedunNodes(inputProfiledTree);
			if (NeuronReconTester::getInstance() != nullptr && REDUN_REMOVE) NeuronReconTester::getInstance()->saveIntermediateResult(noDupSegTree, "noRedunNodes", baseName);
			/**********************************************************/


			if (NeuronStructUtil::multipleSegsCheck(inputProfiledTree.tree))
			{		
				/***************** Remove additional type-1 nodes *****************/
				this->removeAdditionalType1Nodes(inputProfiledTree);
				if (NeuronReconTester::getInstance() != nullptr && TYPE1_REMOVE) NeuronReconTester::getInstance()->saveIntermediateResult(inputProfiledTree.tree, "type1Removed", baseName);
				/******************************************************************/


				/******************* ERROR STRUCTURE CHECK *******************/
				this->errorCheckRepair(inputProfiledTree);

				// The tree is said to be "preprocessed" after this step. It's supposed not to have any duplicated segments, nodes, and error segments. 
				if (NeuronReconTester::getInstance() != nullptr && PREPROCESS) NeuronReconTester::getInstance()->saveIntermediateResult(inputProfiledTree.tree, "preprocessed", baseName);
				/*************************************************************/


				/***************** Group Connected Segmets *****************/
				clock_t start = clock();
				boost::container::flat_map<int, profiledTree> connectedTrees = myNeuronStructExplorer.groupGeoConnectedTrees(inputProfiledTree.tree);	
				if (NeuronReconTester::getInstance() != nullptr && GROUPED_TREES) 
					NeuronReconTester::getInstance()->saveIntermediateResult(connectedTrees, NeuronReconTester::getInstance()->connectedTreePathQ, baseName);

				cout << endl << "-- " << connectedTrees.size() << " separate trees identified." << endl;
				clock_t end = clock();
				float duration = float(end - start) / CLOCKS_PER_SEC;
				cout << "--> Goupring connected segments done. " << duration << " seconds elapsed." << endl << endl;
				/***********************************************************/


				// ******* ================= LOOP DETECTION FOR EACH CONNECTED TREE ================= ******* //
				if (LOOP_CHECK)
				{
					cout << "-- Scanning for loops: " << endl;

					bool loopFound = false;
					for (boost::container::flat_map<int, profiledTree>::iterator treeIt = connectedTrees.begin(); treeIt != connectedTrees.end(); ++treeIt)
					{
						cout << "Tree " << int(treeIt - connectedTrees.begin()) + 1 << ":" << endl;
						if (treeIt->second.loopCheck()) loopFound = true;
					}

					if (loopFound)
					{
						NeuronTree loopTree;
						for (boost::container::flat_map<int, profiledTree>::iterator treeIt = connectedTrees.begin(); treeIt != connectedTrees.end(); ++treeIt)
						{
							map<int, segUnit> segsCopy = treeIt->second.segs;
							if (!treeIt->second.loopingSegs.empty())
							{
								for (set<set<int>>::iterator loopIt = treeIt->second.loopingSegs.begin(); loopIt != treeIt->second.loopingSegs.end(); ++loopIt)
								{
									for (auto& segID : *loopIt)
									{
										for (auto& node : segsCopy[segID].nodes) node.type = 6;
									}
								}
							}
							for (auto& seg : segsCopy) loopTree.listNeuron.append(seg.second.nodes);
						}

						QString outputFolderQ = this->rootPath + "\\loopCases\\";
						QDir outputDir(outputFolderQ);
						if (!outputDir.exists()) outputDir.mkpath(".");

						QString outputLoopTreeNameQ = outputFolderQ + baseName + ".swc";
						writeSWC_file(outputLoopTreeNameQ, loopTree);

						continue;
					}
				}
				// ******* ========================================================================== ******* //
				

				/*********************************** START ASSEMBLING EACH SUBTREE ***********************************/
				map<int, int> tree2HeadNodeMap;
				this->assembleGroupedSegs(connectedTrees, tree2HeadNodeMap, somaAPO);
				if (NeuronReconTester::getInstance() != nullptr && GROUPED_ASSEMBLED_TREES)
					NeuronReconTester::getInstance()->saveIntermediateResult(connectedTrees, NeuronReconTester::getInstance()->connectedAssembledTreePathQ, baseName);
				/****************************** END of [START ASSEMBLING EACH SUBTREE] *******************************/


				// ******* Remove Splikes ******* //
				if (this->removeSpike)
					for (auto& tree : connectedTrees) tree.second = TreeTrimmer::spikeRemoval(tree.second, this->branchNodeMin);
				// ****************************** //


				/********* Reassign Soma And Connect Subtrees To It *********/
				int minNodeID = 10000000, maxNodeID = 0;
				for (auto& connectedTree : connectedTrees)
				{
					if (connectedTree.second.node2LocMap.begin()->first < minNodeID) minNodeID = connectedTree.second.node2LocMap.begin()->first;
					if (connectedTree.second.node2LocMap.rbegin()->first > maxNodeID) maxNodeID = connectedTree.second.node2LocMap.rbegin()->first;
				}

				NeuronSWC somaNode;
				if (minNodeID > 1) somaNode.n = minNodeID - 1;
				else somaNode.n = maxNodeID + 1;
				somaNode.x = somaAPO.x;
				somaNode.y = somaAPO.y;
				somaNode.z = somaAPO.z;
				somaNode.parent = -1;
				somaNode.type = 1;

				for (auto& treeID : tree2HeadNodeMap)
				{
					profiledTree& currTree = connectedTrees[treeID.first];
					currTree.tree.listNeuron[currTree.node2LocMap.at(treeID.second)].parent = somaNode.n;
				}
				/************************************************************/


				/***************** LOOK FOR OTHER POSSIBLE CONNECTING PLACE OTHER THAN SOMA FOR TYPE 7 TREES *****************/
				if (this->autoConnect) this->connectType7trees2otherTree(connectedTrees);
				/************* END of [LOOK FOR OTHER POSSIBLE CONNECTING PLACE OTHER THAN SOMA FOR TYPE 7 TREES] ************/


				NeuronTree outputTree;
				bool type7exist = false;
				for (auto& connectedTree : connectedTrees)
				{
					if (connectedTree.second.tree.listNeuron.size() > 1)
					{
						if (connectedTree.second.tree.listNeuron.at(0).type == 7) type7exist = true;
						outputTree.listNeuron.append(connectedTree.second.tree.listNeuron);
					}
				}
				outputTree.listNeuron.push_front(somaNode); // Finally, add soma node to complete the whole tree structure.

				/*if (!errorList.empty())
				{
					for (auto& ghostSeg : errorList)
					{
						ghostSeg.get()->highlightErrorNodes();
						outputTree.listNeuron.append(ghostSeg.get()->getNodes());
					}

					if (!errorDir.exists()) errorDir.mkpath(".");
					writeSWC_file(errorFolderQ + file, outputTree);
					QString inputAPOfullNameQ = this->rootPath + "\\" + baseName + ".apo";
					QString copyAPOfullNameQ = errorFolderQ + "\\" + baseName + ".apo";
					QString inputANOfullNameQ = this->rootPath + "\\" + baseName;
					QString copyANOfullNameQ = errorFolderQ + "\\" + baseName;
					QFile::copy(inputAPOfullNameQ, copyAPOfullNameQ);
					QFile::copy(inputANOfullNameQ, copyANOfullNameQ);

					continue;
				}*/

				if (type7exist)
				{
					if (!type7dir.exists()) type7dir.mkpath(".");
					writeSWC_file(tooFarType7folderQ + baseName + ".swc", outputTree);
					QString inputAPOfullNameQ = this->rootPath + "\\" + baseName + ".apo";
					QString copyAPOfullNameQ = tooFarType7folderQ + "\\" + baseName + ".apo";
					QString inputANOfullNameQ = this->rootPath + "\\" + baseName;
					QString copyANOfullNameQ = tooFarType7folderQ + "\\" + baseName;
					QFile::copy(inputAPOfullNameQ, copyAPOfullNameQ);
					QFile::copy(inputANOfullNameQ, copyANOfullNameQ);
				}
				else 
				{
					writeSWC_file(outputFolderQ + baseName + ".swc", outputTree);
					//QString inputAPOfullNameQ = this->rootPath + "\\" + baseName + ".apo";
					//QString inputANOfullNameQ = this->rootPath + "\\" + baseName;
					cout << endl;
				}
			}
			else
			{
				if (NeuronStructUtil::findSomaNodeID(inputProfiledTree.tree) == -1)
				{
					QString outputMsgQ = file + " is assembled, but contains multiple soma-type nodes. Please correct it.";
					v3d_msg(outputMsgQ);
					continue;
				}
				else writeSWC_file(outputFolderQ + baseName + ".swc", inputProfiledTree.tree);
			}
		}
		else
		{
			QString outputMsgQ = "The apo file of " + file + " not found. The process has been canceled and skip to the next cell.";
			v3d_msg(outputMsgQ);
			continue;
		}
	}
}

void ReconOperator::removeAdditionalType1Nodes(profiledTree& inputProfiledTree)
{
	// This method exists because of the segment decomposition - [NeuronTree__2__V_NeuronSWC_list] in neuron_format_converter.cpp.
	// The decomposition algorithm seems to create a more complicated situation at soma location where many segments come to connect to the same, and that a lot more (more than the number of connected segments) repeated nodes at soma are created. 
	// [NeuronStructUtil::removeRedunNodes] not only cleans up redundant nodes but also transform soma into a normal branching node, making it misrecongnized in line 212: [profiledTree::findNearestSegEndNodeID].
	// As a result, the whole neuron tree becomes green after the 2nd run and so on.
	
	vector<ptrdiff_t> somaRemoveLocs;
	for (auto& node : inputProfiledTree.tree.listNeuron)
	{
		if (node.type == 1)
		{
			if (node.parent == -1) // -- soma node (type 1, root node) --
			{
				somaRemoveLocs.push_back(inputProfiledTree.node2LocMap.at(node.n));
				if (inputProfiledTree.node2childLocMap.find(node.n) != inputProfiledTree.node2childLocMap.end())
					for (auto& childLoc : inputProfiledTree.node2childLocMap.at(node.n)) inputProfiledTree.tree.listNeuron[childLoc].parent = -1;
			}
			else // -- type 1, but not a root --
			{
				if (inputProfiledTree.node2childLocMap.find(node.n) != inputProfiledTree.node2childLocMap.end())
				{
					// The child node type = 1.
					if (inputProfiledTree.tree.listNeuron.at(*inputProfiledTree.node2childLocMap.at(node.n).begin()).type == 1)
						somaRemoveLocs.push_back(inputProfiledTree.node2LocMap.at(node.n));
					else
					{
						// The child node type is not 1, taking out type 1 nodes stops here. Changing that child node to root.
						for (auto& childNodeLoc : inputProfiledTree.node2childLocMap.at(node.n))
						{
							inputProfiledTree.tree.listNeuron[childNodeLoc].parent = -1;
							//node.parent = -1;
							//node.type = inputProfiledTree.tree.listNeuron.at(*inputProfiledTree.node2childLocMap.at(node.n).begin()).type;
						}
						somaRemoveLocs.push_back(inputProfiledTree.node2LocMap.at(node.n));
					}
				}
				else somaRemoveLocs.push_back(inputProfiledTree.node2LocMap.at(node.n)); // single dangling type 1 non-root node
			}
		}
	}
	if (somaRemoveLocs.size() > 0)
	{
		sort(somaRemoveLocs.rbegin(), somaRemoveLocs.rend());
		for (vector<ptrdiff_t>::iterator locIt = somaRemoveLocs.begin(); locIt != somaRemoveLocs.end(); ++locIt) inputProfiledTree.tree.listNeuron.erase(inputProfiledTree.tree.listNeuron.begin() + *locIt);
	}

	profiledTreeReInit(inputProfiledTree);
}

void ReconOperator::errorCheckRepair(profiledTree& inputProfiledTree)
{
	clock_t start = clock();
	
	this->errorList = myNeuronStructExplorer.structErrorCheck(inputProfiledTree);
	cout << endl << "-- Error segment count (self-looping): " << errorList.size() << endl;
	if (errorList.empty()) return;

	vector<ptrdiff_t> selfLoopingDelLocs;
	vector<QList<NeuronSWC>> correctedSegNodes;
	for (auto& error : errorList)
	{
		selfLoopingSegUnit* selfLoopingSegUnitPtr = dynamic_cast<neuronReconErrorTypes::selfLoopingSegUnit*>(error.get());
		cout << selfLoopingSegUnitPtr->theSeg.segID << ": ";
		for (auto& node : selfLoopingSegUnitPtr->theSeg.nodes) cout << node.n << " ";
		cout << endl << endl;

		for (auto& node : error.get()->getNodes()) selfLoopingDelLocs.push_back(inputProfiledTree.node2LocMap.at(node.n));
		correctedSegNodes.push_back(error.get()->selfCorrect());
	}

	sort(selfLoopingDelLocs.rbegin(), selfLoopingDelLocs.rend());
	for (auto& loc : selfLoopingDelLocs) inputProfiledTree.tree.listNeuron.erase(inputProfiledTree.tree.listNeuron.begin() + loc);
	for (auto& nodes : correctedSegNodes) inputProfiledTree.tree.listNeuron.append(nodes);
	profiledTreeReInit(inputProfiledTree);
	
	clock_t end = clock();
	float duration = float(end - start) / CLOCKS_PER_SEC;
	cout << "--> Finished checking for structural error. " << duration << " seconds elapsed." << endl << endl;
}

void ReconOperator::assembleGroupedSegs(boost::container::flat_map<int, profiledTree>& connectedTrees, map<int, int>& tree2HeadNodeMap, const CellAPO& somaAPO)
{
	clock_t start = clock();

	for (auto& connectedTree : connectedTrees)
	{
		connectedTree.second.treeName = to_string(connectedTree.first);
		if (connectedTree.second.tree.listNeuron.size() <= 1) continue;

		cout << "Processing tree " << int(connectedTrees.find(connectedTree.first) - connectedTrees.begin()) + 1 << "..." << endl;
		int nearestNodeID = connectedTree.second.findNearestSegEndNodeID(somaAPO);
		cout << "ID of the nearest node to soma coordinates: " << nearestNodeID << " -> ";
		const NeuronSWC& nearestNode = connectedTree.second.tree.listNeuron.at(connectedTree.second.node2LocMap.at(nearestNodeID));
		float dist = sqrtf((nearestNode.x - somaAPO.x) * (nearestNode.x - somaAPO.x) + (nearestNode.y - somaAPO.y) * (nearestNode.y - somaAPO.y) + (nearestNode.z - somaAPO.z) * (nearestNode.z - somaAPO.z));
		if (dist <= 35)
		{
			cout << "IN SOMA RANGE" << endl;
			//for (auto& node : connectedTree.second.segs.begin()->second.nodes) cout << node.n << " " << node.parent << endl;
			connectedTree.second.assembleSegs2singleTree(nearestNodeID);
			//for (auto& node : connectedTree.second.segs.begin()->second.nodes) cout << node.n << " " << node.parent << endl;
			tree2HeadNodeMap.insert({ connectedTree.first, nearestNodeID });
			connectedTree.second.tree.listNeuron[connectedTree.second.node2LocMap.at(nearestNodeID)].type = 1;
			cout << "-----> Finish with tree " << int(connectedTrees.find(connectedTree.first) - connectedTrees.begin()) + 1 << endl << endl;
		}
		else
		{
			cout << "TREE " << int(connectedTrees.find(connectedTree.first) - connectedTrees.begin()) + 1 << " NOT IN SOMA RANGE" << endl;
			if (connectedTree.second.segs.size() > 1)
			{
				cout << "  More than 1 segments are identified. Still needs to assemble segments." << endl;
				connectedTree.second.assembleSegs2singleTree(nearestNodeID);
			}

			cout << "    Change node type to 7." << endl;
			for (auto& node : connectedTree.second.tree.listNeuron) node.type = 7;
			cout << "-----> Finish with tree " << int(connectedTrees.find(connectedTree.first) - connectedTrees.begin()) + 1 << endl << endl;
		}
	}

	clock_t end = clock();
	float duration = float(end - start) / CLOCKS_PER_SEC;
	cout << "--> All trees processed. " << duration << " seconds elapsed." << endl;
}

void ReconOperator::connectType7trees2otherTree(boost::container::flat_map<int, profiledTree>& connectedTrees)
{
	bool autoConnect;
	map<int, map<float, pair<int, int>>> nearTreeMap;
	do
	{
		autoConnect = false;
		for (boost::container::flat_map<int, profiledTree>::iterator type7it = connectedTrees.begin(); type7it != connectedTrees.end(); ++type7it)
		{
			if (type7it->second.tree.listNeuron.at(0).type == 7)
			{
				for (boost::container::flat_map<int, profiledTree>::iterator treeIt = connectedTrees.begin(); treeIt != connectedTrees.end(); ++treeIt)
				{
					if (treeIt->second.tree.listNeuron.at(0).type == 7) continue;

					// Why only the 1st segment?? Need to review later.
					map<float, pair<int, int>> distMap = NeuronStructExplorer::nearestSegEnd2targetTree(type7it->second.segs.begin()->second, treeIt->second, this->connectThreshold);
					if (!distMap.empty()) nearTreeMap.insert({ int(treeIt - connectedTrees.begin()) + 1, distMap });
				}

				if (nearTreeMap.empty()) continue;

				autoConnect = true;
				float minDist = 1000000;
				int targetTreeNum = 0;
				for (auto& nearTree : nearTreeMap)
				{
					if (nearTree.second.begin()->first <= minDist)
					{
						minDist = nearTree.second.begin()->first;
						targetTreeNum = nearTree.first;
					}
				}
				cout << endl << "  Subject Tree No.: " << int(type7it - connectedTrees.begin()) + 1 << endl;
				cout << "  Target Tree No.: " << targetTreeNum << endl;

				type7it->second.assembleSegs2singleTree(nearTreeMap.at(targetTreeNum).begin()->second.first); // This step changes the head node to the node closest to the target tree.
				profiledTree& targetProfiledTree = (connectedTrees.begin() + targetTreeNum - 1)->second;
				if (int(type7it - connectedTrees.begin()) + 1 == connectedTrees.size()) 
				{
					// If it's the largest tree, it's most likely the main axon branch. There's no need to change type.
					for (auto& node : type7it->second.tree.listNeuron) node.type = 2;
				}
				else
				{
					int targetType = targetProfiledTree.tree.listNeuron.at(targetProfiledTree.node2LocMap.at(nearTreeMap.at(targetTreeNum).begin()->second.second)).type;
					for (auto& node : type7it->second.tree.listNeuron) node.type = targetType;
				}
				type7it->second.tree.listNeuron[type7it->second.node2LocMap.at(nearTreeMap.at(targetTreeNum).begin()->second.first)].parent = nearTreeMap.at(targetTreeNum).begin()->second.second;
			}

			nearTreeMap.clear();
		}
	} while (autoConnect == true);
}

void ReconOperator::markerApo2swc()
{
	QDir inputFolder(this->rootPath);
	inputFolder.setFilter(QDir::Files | QDir::NoDotAndDotDot);
	QStringList fileList = inputFolder.entryList();

	QString outputFolderQ = this->rootPath + "\\apoMarker2swc\\";
	QDir outputDir(outputFolderQ);
	if (!outputDir.exists()) outputDir.mkpath(".");

	for (auto& fileName : fileList)
	{
		QString baseName;
		if (fileName.endsWith(".apo")) 
		{
			baseName = fileName.left(fileName.length() - 4);
			QString fullFileName = this->rootPath + "\\" + fileName;
			QList<CellAPO> inputApoList = readAPO_file(fullFileName);
			
			NeuronTree outputSWC;
			outputSWC.listNeuron.clear();
			for (auto& apo : inputApoList)
			{
				NeuronSWC newNode;
				newNode.n = apo.name.toInt();
				newNode.x = apo.x;
				newNode.y = apo.y;
				newNode.z = apo.z;
				newNode.parent = -1;
				newNode.type = 0;
				newNode.radius = 10;
				outputSWC.listNeuron.append(newNode); 
			}

			QString outputFileName = outputFolderQ + baseName + ".swc";
			writeSWC_file(outputFileName, outputSWC);
		}
		else if (fileName.endsWith(".marker")) baseName = fileName.left(fileName.length() - 7);
	}
}