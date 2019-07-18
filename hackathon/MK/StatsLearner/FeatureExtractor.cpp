#include "FeatureExtractor.h"

boost::container::flat_map<string, double> FeatureExtractor::objMorphFeature(const connectedComponent& inputSample)
{

}

connectedComponent FeatureExtractor::getObjSurface(const connectedComponent& inputObj)
{
	cout << "input component label: " << inputObj.islandNum << " " << endl;

	connectedComponent surfaceObj;

	vector<int> objSliceLabel;
	objSliceLabel.clear();
	for (map<int, set<vector<int>>>::const_iterator sliceIt = inputObj.coordSets.begin(); sliceIt != inputObj.coordSets.end(); ++sliceIt)
		objSliceLabel.push_back(sliceIt->first);

	if (objSliceLabel.size() == 1)
	{
		surfaceObj.coordSets.insert({ objSliceLabel.at(0), inputObj.coordSets.at(objSliceLabel.at(0)) });
		return surfaceObj;
	}
	else if (objSliceLabel.size() == 2)
	{
		surfaceObj.coordSets.insert({ objSliceLabel.at(0), inputObj.coordSets.at(objSliceLabel.at(0)) });
		surfaceObj.coordSets.insert({ objSliceLabel.at(1), inputObj.coordSets.at(objSliceLabel.at(1)) });
		return surfaceObj;
	}
	surfaceObj.coordSets.insert({ objSliceLabel.at(0), inputObj.coordSets.at(objSliceLabel.at(0)) });
	surfaceObj.coordSets.insert({ objSliceLabel.at(*(objSliceLabel.end() - 1)), inputObj.coordSets.at(*(objSliceLabel.end() - 1)) });

	for (vector<int>::iterator it = objSliceLabel.begin() + 1; it != objSliceLabel.end() - 1; ++it) // iterate through slices
	{
		for (set<vector<int>>::const_iterator pointIt = inputObj.coordSets.at(*it).begin(); pointIt != inputObj.coordSets.at(*it).end(); ++pointIt)
		{
			vector<int> offsetPoint(3);
			offsetPoint[0] = pointIt->at(0);
			offsetPoint[1] = pointIt->at(1);
			offsetPoint[2] = *it;

			offsetPoint[0] = offsetPoint[0] + 1;
			if (inputObj.coordSets.at(*it).find(offsetPoint) == inputObj.coordSets.at(*it).end()) // check if offsetPoint is at the margin of surface.
			{
				if (surfaceObj.coordSets.find(*it) != surfaceObj.coordSets.end()) surfaceObj.coordSets.at(*it).insert(*pointIt);
				else
				{
					set<vector<int>> newSet;
					newSet.insert(*pointIt);
					surfaceObj.coordSets.insert({ *it, newSet });
				}
			}
			
			offsetPoint[0] = offsetPoint[0] - 2;
			if (inputObj.coordSets.at(*it).find(offsetPoint) == inputObj.coordSets.at(*it).end()) // check if offsetPoint is at the margin of surface.
			{
				if (surfaceObj.coordSets.find(*it) != surfaceObj.coordSets.end()) surfaceObj.coordSets.at(*it).insert(*pointIt);
				else
				{
					set<vector<int>> newSet;
					newSet.insert(*pointIt);
					surfaceObj.coordSets.insert({ *it, newSet });
				}
			}

			offsetPoint[0] = offsetPoint[0] + 1;
			offsetPoint[1] = offsetPoint[1] + 1;
			if (inputObj.coordSets.at(*it).find(offsetPoint) == inputObj.coordSets.at(*it).end()) // check if offsetPoint is at the margin of surface.
			{
				if (surfaceObj.coordSets.find(*it) != surfaceObj.coordSets.end()) surfaceObj.coordSets.at(*it).insert(*pointIt);
				else
				{
					set<vector<int>> newSet;
					newSet.insert(*pointIt);
					surfaceObj.coordSets.insert({ *it, newSet });
				}
			}

			offsetPoint[1] = offsetPoint[1] - 2;
			if (inputObj.coordSets.at(*it).find(offsetPoint) == inputObj.coordSets.at(*it).end()) // check if offsetPoint is at the margin of surface.
			{
				if (surfaceObj.coordSets.find(*it) != surfaceObj.coordSets.end()) surfaceObj.coordSets.at(*it).insert(*pointIt);
				else
				{
					set<vector<int>> newSet;
					newSet.insert(*pointIt);
					surfaceObj.coordSets.insert({ *it, newSet });
				}
			}
		}
	}

	return surfaceObj;
}

double FeatureExtractor::irregularity_voxBased(const connectedComponent& inputObj)
{
	connectedComponent surfaceObj = FeatureExtractor::getObjSurface(inputObj);
	
	double inputObjSize = 0, surfaceObjSize = 0;
	for (map<int, set<vector<int>>>::const_iterator inputIt = inputObj.coordSets.begin(); inputIt != inputObj.coordSets.end(); ++inputIt)
		inputObjSize = inputObjSize + inputIt->second.size();
	for (map<int, set<vector<int>>>::const_iterator surIt = surfaceObj.coordSets.begin(); surIt != surfaceObj.coordSets.end(); ++surIt)
		surfaceObjSize = surfaceObjSize + surIt->second.size();

	double irregularity = surfaceObjSize / inputObjSize;

	return irregularity;
}