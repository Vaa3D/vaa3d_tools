#ifndef BRAINSCANNER_H
#define BRAINSCANNER_H

#include "integratedDataStructures.h"
#include "NeuronStructUtilities.h"

using namespace integratedDataStructures;

class BrainScanner
{
public:
	BrainScanner() : somaScan(false) {}

	QString somaScanRootPathQ;
	bool somaScan;
	int voxRange;

	set<string> involvedRegionScan(const vector<float>& coord, const boost::container::flat_map<string, brainRegion>& regionMap);
	void scanSomas(const boost::container::flat_map<string, brainRegion>& regionMap);
	
private:
	bool candidateFilter(const vector<float>& coord, const brainRegion& regionCandidate);
	set<string> sphericalSearch(const vector<float>& coord, const boost::container::flat_map<string, brainRegion>& regionMap, bool whiteMatter = false);

	map<string, int> whiteMatterSearchDistMap;
	inline void printOutWhiteMatterSearchDistMap();
};

inline void BrainScanner::printOutWhiteMatterSearchDistMap()
{
	for (auto& region : this->whiteMatterSearchDistMap) cout << region.first << ": " << region.second << endl;
	cout << endl;
}

#endif