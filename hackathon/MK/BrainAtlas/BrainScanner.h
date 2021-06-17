#ifndef BRAINSCANNER_H
#define BRAINSCANNER_H

#include "integratedDataStructures.h"

using namespace integratedDataStructures;

class BrainScanner
{
public:
	set<string> involvedRegionScan(const vector<float>& coord, const boost::container::flat_map<string, brainRegion>& regionMap);
	
private:
	bool candidateFilter(const vector<float>& coord, const brainRegion& regionCandidate);
};

#endif