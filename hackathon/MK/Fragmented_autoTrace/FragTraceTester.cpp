#include "FragTraceTester.h"

using namespace std;

FragTraceTester* FragTraceTester::testerInstance = nullptr;

FragTraceTester::FragTraceTester(FragTraceControlPanel* controlPanelPtr)
{
	this->sharedControlPanelPtr = make_shared<FragTraceControlPanel*>(controlPanelPtr);
	this->sharedTraceManagerPtr = nullptr;
}

FragTraceTester* FragTraceTester::instance(FragTraceControlPanel* controlPanelPtr)
{
	FragTraceTester::testerInstance = new FragTraceTester(controlPanelPtr);
	return FragTraceTester::testerInstance;
}

FragTraceTester* FragTraceTester::getInstance()
{
	if (FragTraceTester::testerInstance != nullptr) return FragTraceTester::testerInstance;
	else return nullptr;
}

void FragTraceTester::uninstance()
{
	if (FragTraceTester::testerInstance != nullptr) delete FragTraceTester::testerInstance;
	FragTraceTester::testerInstance = nullptr;
}

void FragTraceTester::imgVolumeCheck(const Image4DSimple* inputImagePtr, const string& saveName)
{
	unsigned char* img1Dptr = new unsigned char[inputImagePtr->getXDim() * inputImagePtr->getYDim() * inputImagePtr->getZDim()];
	memcpy(img1Dptr, inputImagePtr->getRawData(), inputImagePtr->getTotalBytes());
	V3DLONG dims[4] = { inputImagePtr->getXDim(), inputImagePtr->getYDim(), inputImagePtr->getZDim(), 1 };
	const char* saveNameC = saveName.c_str();
	ImgManager::saveimage_wrapper(saveNameC, img1Dptr, dims, 1);
}

void FragTraceTester::printOutVolInfo()
{
	cout << " -- Displaying image local coords: x(" << (*this->sharedControlPanelPtr)->volumeAdjustedCoords[0] << "-" << (*this->sharedControlPanelPtr)->volumeAdjustedCoords[1] << ") "
										    << "y(" << (*this->sharedControlPanelPtr)->volumeAdjustedCoords[2] << "-" << (*this->sharedControlPanelPtr)->volumeAdjustedCoords[3] << ") " 
											<< "z(" << (*this->sharedControlPanelPtr)->volumeAdjustedCoords[4] << "-" << (*this->sharedControlPanelPtr)->volumeAdjustedCoords[5] << ")" << endl;
	cout << " -- Whole image block dimension: " << (*this->sharedControlPanelPtr)->displayingDims[0] << " " << (*this->sharedControlPanelPtr)->displayingDims[1] << " " << (*this->sharedControlPanelPtr)->displayingDims[2] << endl;
	cout << " -- Displaying image global coords: x(" << (*this->sharedControlPanelPtr)->globalCoords[0] << "-" << (*this->sharedControlPanelPtr)->globalCoords[1] << ") "
											 << "y(" << (*this->sharedControlPanelPtr)->globalCoords[2] << "-" << (*this->sharedControlPanelPtr)->globalCoords[3] << ") "
											 << "z(" << (*this->sharedControlPanelPtr)->globalCoords[4] << "-" << (*this->sharedControlPanelPtr)->globalCoords[5] << ")" << endl;
}

void FragTraceTester::printOutImgInfo()
{
	cout << "  -- Scaling back to real world dimension:" << endl;
	cout << "      image dims: " << imgDims[0] << " " << imgDims[1] << " " << imgDims[2] << endl;
	cout << "      image res: " << imgRes[0] << " " << imgRes[1] << " " << imgRes[2] << endl;
	cout << "      image origin: " << imgOrigin[0] << " " << imgOrigin[1] << " " << imgOrigin[2] << endl;
}

void FragTraceTester::saveIntermediateImgSlices(const string& regImgName, const QString& prefixQ, V3DLONG dims[])
{
	if ((*this->sharedTraceManagerPtr)->fragTraceImgManager.imgDatabase.find(regImgName) == (*this->sharedTraceManagerPtr)->fragTraceImgManager.imgDatabase.end())
	{
		cerr << "The specified image doesn't exist. Do nothing and return.";
		return;
	}

	qint64 timeStamp_qint64 = QDateTime::currentMSecsSinceEpoch();
	QString timeStampQ = QString::number(timeStamp_qint64);
	QString fullSaveRootQ = prefixQ + "\\" + timeStampQ;
	if (!QDir(fullSaveRootQ).exists()) QDir().mkpath(fullSaveRootQ);


	for (map<string, myImg1DPtr>::iterator it = (*this->sharedTraceManagerPtr)->fragTraceImgManager.imgDatabase.at(regImgName).slicePtrs.begin();
		it != (*this->sharedTraceManagerPtr)->fragTraceImgManager.imgDatabase.at(regImgName).slicePtrs.end(); ++it)
	{
		string saveFullPath = fullSaveRootQ.toStdString() + "\\" + it->first;
		const char* saveFullPathC = saveFullPath.c_str();
		ImgManager::saveimage_wrapper(saveFullPathC, it->second.get(), dims, 1);
	}
}

map<int, set<vector<float>>> FragTraceTester::clusterSegEndMarkersGen(const set<int>& clusterIDs, const profiledTree& inputProfiledTree)
{
	map<int, set<vector<float>>> outputMap;
	for (set<int>::const_iterator setIDit = clusterIDs.begin(); setIDit != clusterIDs.end(); ++setIDit)
	{
		set<vector<float>> outputMarkerCoords;
		for (auto& segHeadID : inputProfiledTree.segHeadClusters.at(*setIDit))
		{
			vector<float> newMarker(3);
			const NeuronSWC& headNode = inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(inputProfiledTree.segs.at(segHeadID).head));
			newMarker = { headNode.x, headNode.y, headNode.z };
			outputMarkerCoords.insert(newMarker);
		}

		for (auto& segTailID : inputProfiledTree.segTailClusters.at(*setIDit))
		{
			for (vector<int>::const_iterator tailIt = inputProfiledTree.segs.at(segTailID).tails.begin(); tailIt != inputProfiledTree.segs.at(segTailID).tails.end(); ++tailIt)
			{
				vector<float> newMarker(3);
				const NeuronSWC& tailNode = inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(*tailIt));
				newMarker = { tailNode.x, tailNode.y, tailNode.z };
				outputMarkerCoords.insert(newMarker);
			}
		}
		outputMap.insert({ *setIDit, outputMarkerCoords });
	}

	return outputMap;
}

map<int, RGBA8> FragTraceTester::clusterColorGen_RGB(const set<int>& clusterIDs)
{
	map<int, RGBA8> clusterColorMap;
	int colorR = 10, colorG = 10, colorB = 10;
	for (auto& clusterID : clusterIDs)
	{
		RGBA8 newColor;
		if (clusterID % 3 == 0)
		{
			newColor.r = colorR % 255;
			newColor.g = 0;
			newColor.b = 0;
			colorR += 10;
		}
		else if (clusterID % 3 == 1)
		{
			newColor.r = 0;
			newColor.g = colorG % 255;
			newColor.b = 0;
			colorG += 10;
		}
		else if (clusterID % 3 == 2)
		{			
			newColor.r = 0;
			newColor.g = 0;
			newColor.b = colorB % 255;
			colorB += 10;
		}
		clusterColorMap.insert({ clusterID, newColor });
	}

	return clusterColorMap;
}