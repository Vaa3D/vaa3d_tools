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

profiledTree FragTraceTester::segEndClusterCheck(const profiledTree& inputProfiledTree, QString savePathQ)
{
	profiledTree outputProfiledTree = inputProfiledTree;
	if (outputProfiledTree.segs.empty()) profiledTreeReInit(outputProfiledTree);
	if (outputProfiledTree.segHeadClusters.empty() || outputProfiledTree.segTailClusters.empty())
		(*this->sharedControlPanelPtr)->traceManagerPtr->fragTraceTreeManager.getSegHeadTailClusters(outputProfiledTree);

	for (auto& headSegIt : outputProfiledTree.headSeg2ClusterMap)
	{
		size_t headLoc = outputProfiledTree.node2LocMap.at(outputProfiledTree.segs.at(headSegIt.first).head);
		outputProfiledTree.tree.listNeuron[headLoc].type = headSegIt.second % 9;
		outputProfiledTree.tree.listNeuron[headLoc].radius = headSegIt.second;
	}

	for (auto& tailSegIt : outputProfiledTree.tailSeg2ClusterMap)
	{
		for (auto& tailIt : outputProfiledTree.segs.at(tailSegIt.first).tails)
		{
			size_t tailLoc = outputProfiledTree.node2LocMap.at(tailIt);
			outputProfiledTree.tree.listNeuron[tailLoc].type = tailSegIt.second % 9;
			outputProfiledTree.tree.listNeuron[tailLoc].radius = tailSegIt.second;
		}
	}

	return outputProfiledTree;
}

void FragTraceTester::scale(profiledTree& inputProfiledTree)
{
	float imgDims[3];
	imgDims[0] = (*this->sharedControlPanelPtr)->thisCallback->getImageTeraFly()->getXDim();
	imgDims[1] = (*this->sharedControlPanelPtr)->thisCallback->getImageTeraFly()->getYDim();
	imgDims[2] = (*this->sharedControlPanelPtr)->thisCallback->getImageTeraFly()->getZDim();

	float imgRes[3];
	imgRes[0] = (*this->sharedControlPanelPtr)->thisCallback->getImageTeraFly()->getRezX();
	imgRes[1] = (*this->sharedControlPanelPtr)->thisCallback->getImageTeraFly()->getRezY();
	imgRes[2] = (*this->sharedControlPanelPtr)->thisCallback->getImageTeraFly()->getRezZ();

	float factor = pow(2, abs((*this->sharedControlPanelPtr)->CViewerPortal->getTeraflyTotalResLevel() - 1 - (*this->sharedControlPanelPtr)->CViewerPortal->getTeraflyResLevel()));
	//cout << "  -- scaling factor = " << factor << endl;
	//cout << "  -- current resolutionl level = " << (*this->sharedcontrolPanelPtr)->CViewerPortal->getTeraflyResLevel() + 1 << endl;
	//cout << "  -- total res levels: " << (*this->sharedcontrolPanelPtr)->CViewerPortal->getTeraflyTotalResLevel() << endl;

	float imgOri[3];
	string currWinTitle = (*this->sharedControlPanelPtr)->CViewerPortal->getCviewerWinTitle();
	vector<string> splitWhole;
	boost::split(splitWhole, currWinTitle, boost::is_any_of("["));
	vector<string> xSplit;
	boost::split(xSplit, splitWhole[1], boost::is_any_of(","));
	imgOri[0] = stof(xSplit[0]) * factor - 1;
	vector<string> ySplit;
	boost::split(ySplit, splitWhole[2], boost::is_any_of(","));
	imgOri[1] = stof(ySplit[0]) * factor - 1;
	vector<string> zSplit;
	boost::split(zSplit, splitWhole[3], boost::is_any_of(","));
	imgOri[2] = stof(zSplit[0]) * factor - 1;

	//NeuronTree scaledTree = NeuronStructUtil::swcScale(this->tracedTree, imgRes[0] / imgDims[0], imgRes[1] / imgDims[1], imgRes[2] / imgDims[2]);
	inputProfiledTree.tree = NeuronStructUtil::swcScale(inputProfiledTree.tree, factor, factor, factor);
	inputProfiledTree.tree = NeuronStructUtil::swcShift(inputProfiledTree.tree, imgOri[0], imgOri[1], imgOri[2]);
}