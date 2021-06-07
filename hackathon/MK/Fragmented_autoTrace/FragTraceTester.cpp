#include "FragTraceTester.h"

#include "NeuronStructNavigatingTester.h"

using namespace std;
using NSlibTester = NeuronStructNavigator::Tester;

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

void FragTraceTester::printOutTerminalSegInfo(const segUnit& seg)
{
	if (!seg.seg_childLocMap.empty())
	{
		cout << endl << "node-child info:" << endl;
		for (auto& node : seg.seg_childLocMap)
		{
			cout << node.first << ": ";
			for (auto& child : node.second) 
				cout << seg.nodes.at(child).n << " ";
			cout << endl;
		}
	}
}

void FragTraceTester::printOutEditSegInfo(const vector<segUnit>& segs)
{
	for (auto& seg : segs)
	{
		cout << "profiled segment" << seg.segID << ":" << endl;
		for (auto& node : seg.nodes)
			cout << node.n << " " << node.x << " " << node.y << " " << node.z << " " << node.parent << endl;
		cout << endl;
	}
}

void FragTraceTester::printOutEditSegInfo(const V_NeuronSWC_list& displayingSegs, const set<int>& segIDs)
{
	for (auto& segID : segIDs)
	{
		cout << "displaying segmet " << segID << ":" << endl;
		for (auto& node : displayingSegs.seg.at(segID).row)
			cout << node.data[0] << " " << node.data[2] << " " << node.data[3] << " " << node.data[4] << " " << node.data[6] << endl;
		cout << endl;
	}
}

void FragTraceTester::printOutEditSegInfo(const V_NeuronSWC_list& displayingSegs, const map<int, set<int>>& segs2Bedited)
{
	for (auto& seg : segs2Bedited)
	{
		cout << "segment " << seg.first << ":" << endl;
		for (auto& node : displayingSegs.seg.at(seg.first).row)
		{
			cout << node.data[0] << " " << node.data[2] << " " << node.data[3] << " " << node.data[4] << " " << node.data[6];
			if (seg.second.find(node.data[0]) != seg.second.end()) cout << " -> to be deleted" << endl;
			else cout << endl;
		}
	}
}

void FragTraceTester::clusterSegEndMarkersGen(profiledTree& inputProfiledTree, const float segClusterRange)
{
	if (!NSlibTester::isInstantiated()) NSlibTester::instance(&(*this->sharedTraceManagerPtr)->fragTraceTreeManager);
	(*this->sharedTraceManagerPtr)->fragTraceTreeManager.getSegHeadTailClusters(inputProfiledTree, segClusterRange);
	map<int, set<vector<float>>> clusterSegEndMap = NSlibTester::getInstance()->getSegEndClusterNodeMap(inputProfiledTree);
	this->clusterColorGen_RGB(clusterSegEndMap);
	NSlibTester::uninstance();
}

void FragTraceTester::clusterSegEndMarkersGen_withSeed(const set<int>& seedCluster, profiledTree& inputProfiledTree, const float segClusterRange)
{
	if (!NSlibTester::isInstantiated()) NSlibTester::instance(&(*this->sharedTraceManagerPtr)->fragTraceTreeManager);
	(*this->sharedTraceManagerPtr)->fragTraceTreeManager.getSegHeadTailClusters(inputProfiledTree, segClusterRange);
	map<int, set<vector<float>>> clusterSegEndMap = NSlibTester::getInstance()->getSegEndClusterNodeMap(inputProfiledTree);
	map<int, set<vector<float>>> seedClusterSegEndMap;
	for (set<int>::const_iterator it = seedCluster.begin(); it != seedCluster.end(); ++it)
		seedClusterSegEndMap.insert({ *it, clusterSegEndMap.at(*it) });
	this->clusterColorGen_RGB(seedClusterSegEndMap);
	NSlibTester::uninstance();

	RGBA8 seedClusterColor;
	seedClusterColor.r = 255;
	seedClusterColor.g = 255;
	seedClusterColor.b = 255;
	for (map<int, set<vector<float>>>::iterator it = seedClusterSegEndMap.begin(); it != seedClusterSegEndMap.end(); ++it)
		this->clusterSegEndNodeMaps.back()[it->first].second = seedClusterColor;
}

void FragTraceTester::clusterSegEndMarkersGen_axonChain(const map<int, segEndClusterUnit*>& chains, const profiledTree& inputProfiledTree)
{
	if (!NSlibTester::isInstantiated()) NSlibTester::instance(&(*this->sharedTraceManagerPtr)->fragTraceTreeManager);
	map<int, set<vector<float>>> clusterSegEndMap = NSlibTester::getInstance()->getSegEndClusterNodeMap(inputProfiledTree);
	NSlibTester::uninstance();

	map<int, set<vector<float>>> chainClusterSegEndMap;
	for (map<int, segEndClusterUnit*>::const_iterator it = chains.begin(); it != chains.end(); ++it)
	{
		chainClusterSegEndMap.insert({ it->first, clusterSegEndMap.at(it->second->ID) });
		this->rc_markersGen_axonChain(it->second->childClusterMap, clusterSegEndMap, chainClusterSegEndMap);
	}
	this->clusterColorGen_RGB(chainClusterSegEndMap);

	RGBA8 seedClusterColor;
	seedClusterColor.r = 255;
	seedClusterColor.g = 255;
	seedClusterColor.b = 255;
	for (map<int, set<vector<float>>>::iterator it = chainClusterSegEndMap.begin(); it != chainClusterSegEndMap.end(); ++it)
		this->clusterSegEndNodeMaps.back()[it->first].second = seedClusterColor;
}

void FragTraceTester::rc_markersGen_axonChain(const map<int, segEndClusterUnit*>& childClusters, const map<int, set<vector<float>>>& clusterSegEndMap, map<int, set<vector<float>>>& chainClusterSegEndMap)
{
	for (map<int, segEndClusterUnit*>::const_iterator it = childClusters.begin(); it != childClusters.end(); ++it)
	{
		chainClusterSegEndMap.insert({ it->first, clusterSegEndMap.at(it->first) });
		if (it->second->childClusterMap.empty()) continue;
		else this->rc_markersGen_axonChain(it->second->childClusterMap, clusterSegEndMap, chainClusterSegEndMap);
	}
}

void FragTraceTester::clusterColorGen_RGB(const map<int, set<vector<float>>>& clusterSegEndMap)
{
	map<int, pair<set<vector<float>>, RGBA8>> clusterSegEndNodeMap;
	int colorR = 10, colorG = 10, colorB = 10;
	for (auto& cluster : clusterSegEndMap)
	{
		RGBA8 newColor;
		if (cluster.first % 3 == 0)
		{
			newColor.r = colorR % 255;
			newColor.g = 0;
			newColor.b = 0;
			colorR += 10;
		}
		else if (cluster.first % 3 == 1)
		{
			newColor.r = 0;
			newColor.g = colorG % 255;
			newColor.b = 0;
			colorG += 10;
		}
		else if (cluster.first % 3 == 2)
		{			
			newColor.r = 0;
			newColor.g = 0;
			newColor.b = colorB % 255;
			colorB += 10;
		}
		clusterSegEndNodeMap.insert({ cluster.first, pair<set<vector<float>>, RGBA8>(cluster.second, newColor) });
	}
	this->clusterSegEndNodeMaps.push_back(clusterSegEndNodeMap);
}

void FragTraceTester::printOutHiddenType16Info()
{
	int count = 0;
	for (auto& hidden16 : *(*(this->sharedControlPanelPtr))->CViewerPortal->getDisplayingSegs())
		if (hidden16.row.begin()->data[1] == 16 && hidden16.to_be_deleted) ++count;
	cout << "CViewer (" << (*(this->sharedControlPanelPtr))->CViewerPortal->getViewerID() << "):            hidden type16 segs = " << count << endl;
	cout << "FragTraceManager:      hidden type16 segs = " << (*(this->sharedTraceManagerPtr))->totalV_NeuronSWCs.size() << endl;
	cout << "FragTraceControlPanel: hidden type16 segs = " << (*(this->sharedControlPanelPtr))->tracedType16Vsegs.size() << endl;
}