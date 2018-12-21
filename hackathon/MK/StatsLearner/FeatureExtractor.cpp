#include "FeatureExtractor.h"

boost::container::flat_map<string, double> FeatureExtractor::objMorphFeature(const connectedComponent& inputSample)
{

}

connectedComponent FeatureExtractor::getSurfaceObj(const connectedComponent& inputObj)
{
	connectedComponent surfaceObj;
	
	vector<int> objSliceLabel;
	for (map<int, set<vector<int>>>::const_iterator sliceIt = inputObj.coordSets.begin(); sliceIt != inputObj.coordSets.end(); ++sliceIt)
		objSliceLabel.push_back(sliceIt->first);
	surfaceObj.coordSets.insert({ objSliceLabel.at(0), inputObj.coordSets.at(objSliceLabel.at(0)) });
	surfaceObj.coordSets.insert({ objSliceLabel.at(*(objSliceLabel.end() - 1)), inputObj.coordSets.at(*(objSliceLabel.end() - 1)) });

	for (vector<int>::iterator it = objSliceLabel.begin() + 1; it != objSliceLabel.end() - 1; ++it)
	{
		for (set<vector<int>>::const_iterator pointIt = inputObj.coordSets.at(*it).begin(); pointIt != inputObj.coordSets.at(*it).end(); ++pointIt)
		{
			vector<int> offsetPoint(3);
			offsetPoint[2] = *it;

			offsetPoint[0] = pointIt->at(0) + 1;
			offsetPoint[1] = pointIt->at(1);
			if (inputObj.coordSets.at(*it).find(offsetPoint) == inputObj.coordSets.at(*it).end()) // check if offsetPoint a surface point
			{
				if (surfaceObj.coordSets.find(*it) != surfaceObj.coordSets.end()) surfaceObj.coordSets.at(*it).insert(*pointIt);
				else
				{
					set<vector<int>> newSet;
					newSet.insert(*pointIt);
					surfaceObj.coordSets.insert({ *it, newSet });
				}
			}
			else
			{
				
			}
		}
	}
}