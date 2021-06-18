#include "BrainScanner.h"

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