#include "BrainScanner.h"

#include <fstream>

set<string> BrainScanner::involvedRegionScan(const vector<float>& coord, const boost::container::flat_map<string, brainRegion>& regionMap)
{
	set<string> outputRegionNames;

	vector<boost::container::flat_map<string, brainRegion>::const_iterator> candidateIts;
	for (boost::container::flat_map<string, brainRegion>::const_iterator it = regionMap.begin(); it != regionMap.end(); ++it)
		if (this->candidateFilter(coord, it->second)) candidateIts.push_back(it);

	cout << candidateIts.size() << " candidate brain regions included: ";
	for (auto& iter : candidateIts) cout << iter->first << " ";
	cout << endl;

	vector<int> roundedCoord = { int(coord.at(0)), int(coord.at(1)), int(coord.at(2)) };
	for (auto& candidateIt : candidateIts)
	{
		cout << candidateIt->first << " -- " << endl;
		int bodyCount = 1;
		brainRegion region = candidateIt->second;
		for (auto& body : region.regionBodies)
		{
			cout << "body " << bodyCount << ": ";
			if (body.isEmbedded(roundedCoord))
			{
				cout << "In!" << endl;
				outputRegionNames.insert(candidateIt->first);
				break;
			}
			++bodyCount;
		}
		cout << endl;
	}

	return outputRegionNames;
}

bool BrainScanner::candidateFilter(const vector<float>& coord, const brainRegion& regionCandidate)
{
	bool xMinMax = false, yMinMax = false, zMinMax = false;
	for (auto& body : regionCandidate.regionBodies)
	{
		if ((int(coord.at(0)) >= body.xMin && int(coord.at(0)) <= body.xMax) || (int(coord.at(0) + 1) >= body.xMin && int(coord.at(0) + 1) <= body.xMax)) xMinMax = true;
		if ((int(coord.at(1)) >= body.yMin && int(coord.at(1)) <= body.yMax) || (int(coord.at(1) + 1) >= body.yMin && int(coord.at(1) + 1) <= body.yMax)) yMinMax = true;
		if ((int(coord.at(2)) >= body.zMin && int(coord.at(2)) <= body.zMax) || (int(coord.at(2) + 1) >= body.zMin && int(coord.at(2) + 1) <= body.zMax)) zMinMax = true;
	}
	if (!xMinMax || !yMinMax || !zMinMax)
	{
		//cout << "Not in min max range" << endl;
		return false;
	}

	bool xyP = false, yzP = false, xzP = false;
	vector<vector<int>> xyCombination = { { int(coord.at(0)), int(coord.at(1)) }, { int(coord.at(0) + 1), int(coord.at(1)) }, { int(coord.at(0)), int(coord.at(1) + 1) }, { int(coord.at(0) + 1), int(coord.at(1) + 1) } };
	vector<vector<int>> yzCombination = { { int(coord.at(1)), int(coord.at(2)) }, { int(coord.at(1) + 1), int(coord.at(2)) }, { int(coord.at(1)), int(coord.at(2) + 1) }, { int(coord.at(1) + 1), int(coord.at(2) + 1) } };
	vector<vector<int>> xzCombination = { { int(coord.at(0)), int(coord.at(2)) }, { int(coord.at(0) + 1), int(coord.at(2)) }, { int(coord.at(0)), int(coord.at(2) + 1) }, { int(coord.at(0) + 1), int(coord.at(2) + 1) } };
	for (auto& body : regionCandidate.regionBodies)
	{
		for (auto& xy : xyCombination)
		{
			if (body.xyProjection.find(xy) != body.xyProjection.end())
			{
				xyP = true;
				break;
			}
		}

		for (auto& yz : yzCombination)
		{
			if (body.yzProjection.find(yz) != body.yzProjection.end())
			{
				yzP = true;
				break;
			}
		}

		for (auto& xz : xzCombination)
		{
			if (body.xzProjection.find(xz) != body.xzProjection.end())
			{
				xzP = true;
				break;
			}
		}
	}
	if (!xyP || !yzP || !xzP)
	{
		//cout << "Not in projection range" << endl;
		return false;
	}
	return true;
}

void BrainScanner::scanSomas(const boost::container::flat_map<string, brainRegion>& regionMap)
{
	QDir inputDirQ(this->somaScanRootPathQ);
	inputDirQ.setFilter(QDir::Files | QDir::NoDotAndDotDot);
	QStringList fileList = inputDirQ.entryList();
	
	string outputFileName = this->somaScanRootPathQ.toStdString() + "\\somaRegion.csv";
	ofstream outputFile(outputFileName);
	outputFile << "Neuron Name" << "," << "Region(s)" << endl;
	for (auto& file : fileList)
	{
		if (file.endsWith(".swc"))
		{
			QString swcFullNameQ = this->somaScanRootPathQ + "\\" + file;
			NeuronTree inputTree = readSWC_file(swcFullNameQ);

			NeuronSWC somaNode = NeuronStructUtil::findSomaNode(inputTree);
			if (somaNode.type != 1 || somaNode.parent != -1)
			{
				for (auto& node : inputTree.listNeuron)
				{
					if (node.parent == -1)
					{
						somaNode = node;
						break;
					}
				}

				if (somaNode.parent != -1) continue;
			}

			outputFile << file.left(file.length() - 4).toStdString() << ",";
			vector<float> somaCoord = { somaNode.x, somaNode.y, somaNode.z };
			set<string> locatedRegions = this->involvedRegionScan(somaCoord, regionMap);

			cout << file.toStdString() << ": ";
			for (auto& region : locatedRegions)
			{
				cout << region << " ";
				outputFile << region << " ";
			}
			cout << endl;
			outputFile << endl;
		}
		else if (file.endsWith(".apo"))
		{
			string brainID = file.left(file.length() - 4).toStdString();
			QList<CellAPO> inputAPOlist = readAPO_file(this->somaScanRootPathQ + "\\" + file);
			
			string outFileName = this->somaScanRootPathQ.toStdString() + "\\" + brainID + "somaRegionMap.csv";
			ofstream outFile(outFileName);
			outFile << "Z" << "," << "X" << "," << "Y" << "," << "Marker Name" << "," << "Region(s)" << endl;
			for (auto& apo : inputAPOlist)
			{
				outFile << int(apo.z) << "," << int(apo.x) << "," << int(apo.y) << "," << apo.name.toStdString() << ",";
				vector<float> somaCoord = { apo.x, apo.y, apo.z };
				set<string> locatedRegions = this->involvedRegionScan(somaCoord, regionMap);
				for (auto& region : locatedRegions) outFile << region << " ";
				outFile << endl;
			}

			if (outFile.is_open()) outFile.close();
		}
	}

	if (outputFile.is_open()) outputFile.close();

	QString outputMsgQ = "Scanned soma regions saved to " + QString::fromStdString(outputFileName);
	v3d_msg(outputMsgQ);
}