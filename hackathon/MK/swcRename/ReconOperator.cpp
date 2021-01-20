#include "ReconOperator.h"

void ReconOperator::downSampleReconFile(const QStringList& fileList, float xFactor, float yFactor, float zFactor)
{
	QString newFolderName = this->rootPath + "\\downsampled";
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

void ReconOperator::removeDupedNodes()
{
	QDir inputFolder(this->rootPath);
	inputFolder.setFilter(QDir::Files | QDir::NoDotAndDotDot);
	QStringList fileList = inputFolder.entryList();

	QString outputFolderQ = this->rootPath + "\\allCleanedUp\\";
	QDir outputDir(outputFolderQ);
	if (!outputDir.exists()) outputDir.mkpath(".");

	/*QString subTreeFolderQ = this->rootPath + "\\subTrees\\";
	QDir subTreeDir(subTreeFolderQ);
	if (!subTreeDir.exists()) subTreeDir.mkpath(".");*/

	for (auto& file : fileList)
	{
		QString baseName;
		if (file.endsWith(".swc")) baseName = file.left(file.length() - 4);
		else if (file.endsWith(".eswc")) baseName = file.left(file.length() - 5);

		bool apoFound = false;
		CellAPO somaAPO;
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
		
		if (apoFound)
		{
			cout << "Input soma coordinate: " << somaAPO.x << " " << somaAPO.y << " " << somaAPO.z << endl;
			QString inputSWCFullName = this->rootPath + "\\" + file;
			NeuronTree inputTree = readSWC_file(inputSWCFullName);
			NeuronTree noDupSegTree = NeuronStructUtil::removeDupSegs(inputTree);
			profiledTree inputProfiledTree(noDupSegTree);
			NeuronStructUtil::removeRedunNodes(inputProfiledTree);
			if (NeuronStructUtil::multipleSegsCheck(inputTree))
			{
				/*QString treesFolderQ = subTreeFolderQ + baseName + "\\";
				QDir treesFolderDir(treesFolderQ);
				if (!treesFolderDir.exists()) treesFolderDir.mkpath(".");*/

				clock_t start = clock();
				boost::container::flat_map<int, profiledTree> connectedTrees = NeuronStructExplorer::groupGeoConnectedTrees(inputProfiledTree.tree);
				cout << endl << "-- " << connectedTrees.size() << " separate trees identified." << endl << endl;
				map<int, int> tree2HeadNodeMap;
				int minNodeID, maxNodeID;
				for (auto& connectedTree : connectedTrees)
				{
					//writeSWC_file(treesFolderQ + QString::number(int(connectedTrees.find(connectedTree.first) - connectedTrees.begin()) + 1) + ".swc", connectedTree.second.tree);

					NeuronStructUtil::removeRedunNodes(inputProfiledTree);
					cout << "Processing tree " << int(connectedTrees.find(connectedTree.first) - connectedTrees.begin()) + 1 << "..." << endl;
					minNodeID = 10000000;
					maxNodeID = 0;
					if (connectedTree.second.node2LocMap.begin()->first < minNodeID) minNodeID = connectedTree.second.node2LocMap.begin()->first;
					if (connectedTree.second.node2LocMap.rbegin()->first > maxNodeID) maxNodeID = connectedTree.second.node2LocMap.rbegin()->first;

					int nearestNodeID = connectedTree.second.findNearestSegEndNodeID(somaAPO);
					cout << "ID of the nearest node to soma coordinates " << nearestNodeID << " -> ";
					const NeuronSWC& nearestNode = connectedTree.second.tree.listNeuron.at(connectedTree.second.node2LocMap.at(nearestNodeID));
					float dist = sqrtf((nearestNode.x - somaAPO.x) * (nearestNode.x - somaAPO.x) + (nearestNode.y - somaAPO.y) * (nearestNode.y - somaAPO.y) + (nearestNode.z - somaAPO.z) * (nearestNode.z - somaAPO.z));
					if (dist <= 35)
					{
						connectedTree.second.assembleSegs2singleTree(nearestNodeID);
						tree2HeadNodeMap.insert({ connectedTree.first, nearestNodeID });
						connectedTree.second.tree.listNeuron[connectedTree.second.node2LocMap.at(nearestNodeID)].type = 1;
						cout << "-----> Finish with tree " << int(connectedTrees.find(connectedTree.first) - connectedTrees.begin()) + 1 << endl << endl;
					}
					else
					{
						cout << "Tree " << int(connectedTrees.find(connectedTree.first) - connectedTrees.begin()) + 1 << " Not in the soma range." << endl;
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

				NeuronTree outputTree;
				for (auto& connectedTree : connectedTrees) outputTree.listNeuron.append(connectedTree.second.tree.listNeuron);
				outputTree.listNeuron.push_front(somaNode);				
				writeSWC_file(outputFolderQ + baseName + ".swc", outputTree);
			}	
		}
	}
}