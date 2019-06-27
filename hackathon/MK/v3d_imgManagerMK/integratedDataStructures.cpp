#include <iostream>

#include "ImgProcessor.h"
#include "integratedDataStructures.h"

void integratedDataStructures::connectedComponent::getConnCompSurface()
{
	boost::container::flat_set<vector<int>> topSliceSet;
	for (set<vector<int>>::iterator it = this->coordSets.begin()->second.begin(); it != this->coordSets.begin()->second.end(); ++it)
	{
		vector<int> newVec = { it->at(0), it->at(1), it->at(2) };
		topSliceSet.insert(newVec);
	}
	this->surfaceCoordSets.insert(pair<int, boost::container::flat_set<vector<int>>>(this->coordSets.begin()->first, topSliceSet));

	boost::container::flat_set<vector<int>> bottomSliceSet;
	for (set<vector<int>>::iterator it = this->coordSets.rbegin()->second.begin(); it != this->coordSets.rbegin()->second.end(); ++it)
	{
		vector<int> newVec = { it->at(0), it->at(1), it->at(2) };
		topSliceSet.insert(newVec);
	}
	this->surfaceCoordSets.insert(pair<int, boost::container::flat_set<vector<int>>>(this->coordSets.rbegin()->first, topSliceSet));

	if (this->coordSets.size() > 2)
	{
		for (map<int, set<vector<int>>>::iterator it = ++this->coordSets.begin(); it != --this->coordSets.end(); ++it)
		{
			boost::container::flat_set<vector<int>> currSliceSurf;
			for (set<vector<int>>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
			{
				vector<int> vec1 = { it2->at(0) - 1, it2->at(1), it2->at(2) };
				vector<int> vec2 = { it2->at(0) + 1, it2->at(1), it2->at(2) };
				vector<int> vec3 = { it2->at(0), it2->at(1) - 1, it2->at(2) };
				vector<int> vec4 = { it2->at(0), it2->at(1) + 1, it2->at(2) };
				vector<int> vec5 = { it2->at(0), it2->at(1), it2->at(2) - 1 };
				vector<int> vec6 = { it2->at(0), it2->at(1), it2->at(2) + 1 };
				if (this->coordSets.at(it->first - 1).find(vec5) == this->coordSets.at(it->first - 1).end() ||
					this->coordSets.at(it->first + 1).find(vec6) == this->coordSets.at(it->first + 1).end() ||
					this->coordSets.at(it->first).find(vec1) == this->coordSets.at(it->first).end() ||
					this->coordSets.at(it->first).find(vec2) == this->coordSets.at(it->first).end() ||
					this->coordSets.at(it->first).find(vec3) == this->coordSets.at(it->first).end() ||
					this->coordSets.at(it->first).find(vec4) == this->coordSets.at(it->first).end()) currSliceSurf.insert(*it2);
			}
			this->surfaceCoordSets.insert(pair<int, boost::container::flat_set<vector<int>>>(it->first, currSliceSurf));
		}
	}
}

void integratedDataStructures::registeredImg::createBlankImg(const int imgDims[])
{
	myImg1DPtr blank1D(new unsigned char[imgDims[0] * imgDims[2] * imgDims[2]]);
	for (size_t i = 0; i < imgDims[0] * imgDims[2] * imgDims[2]; ++i) blank1D.get()[i] = 0;

	for (map<string, myImg1DPtr>::iterator it = this->slicePtrs.begin(); it != this->slicePtrs.end(); ++it)
	{
		if (it->first.length() >= 5)
		{
			if (!it->first.substr(0, 5).compare("blank"))
			{
				char blankNumChar = *(it->first.end() - 1);
				int blankNum = atoi(&blankNumChar);
				int newBlankNum = blankNum + 1;
				string newBlankNumString = "blank" + to_string(newBlankNum);

				this->slicePtrs.insert({ newBlankNumString, blank1D });
				return;
			}
		}
	}

	this->slicePtrs.insert({ "blank1", blank1D });
}

void integratedDataStructures::registeredImg::getHistMap_no0()
{
	if (slicePtrs.empty())
	{
		cerr << "No existing images. Do nothing and return." << endl;
		return;
	}

	int sliceDims[3];
	sliceDims[0] = this->dims[0];
	sliceDims[1] = this->dims[1];
	sliceDims[2] = 1;

	for (map<string, myImg1DPtr>::iterator it = this->slicePtrs.begin(); it != this->slicePtrs.end(); ++it)
	{
		map<int, size_t> currSliceHistMap = ImgProcessor::histQuickList(it->second.get(), sliceDims);
		for (map<int, size_t>::iterator cummIt = currSliceHistMap.begin(); cummIt != currSliceHistMap.end(); ++cummIt)
		{
			if (!this->histMap.insert({ cummIt->first, cummIt->second }).second)
				this->histMap[cummIt->first] = this->histMap[cummIt->first] = cummIt->second;
		}
	}
}

void integratedDataStructures::registeredImg::getHistMap_no0_log10()
{
	if (slicePtrs.empty())
	{
		cerr << "No existing images. Do nothing and return." << endl;
		return;
	}

	if (!this->histMap.empty())
	{
		for (map<int, size_t>::iterator it = this->histMap.begin(); it != this->histMap.end(); ++it)
		{
			double countLog10 = log10(float(it->second));
			this->histMap_log10.insert({ it->first, countLog10 });
		}
	}
	else
	{
		this->getHistMap_no0();
		this->getHistMap_no0_log10();
	}
}