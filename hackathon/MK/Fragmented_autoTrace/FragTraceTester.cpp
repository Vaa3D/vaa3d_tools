#include "FragTraceTester.h"

using namespace std;

profiledTree FragTraceTester::segEndClusterCheck(const profiledTree& inputProfiledTree, QString savePathQ)
{
	profiledTree outputProfiledTree = inputProfiledTree;
	if (outputProfiledTree.segs.empty()) profiledTreeReInit(outputProfiledTree);
	if (outputProfiledTree.segHeadClusters.empty() || outputProfiledTree.segTailClusters.empty())
		(*this->sharedcontrolPanelPtr)->traceManagerPtr->fragTraceTreeManager.getSegHeadTailClusters(outputProfiledTree);

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
	imgDims[0] = (*this->sharedcontrolPanelPtr)->thisCallback->getImageTeraFly()->getXDim();
	imgDims[1] = (*this->sharedcontrolPanelPtr)->thisCallback->getImageTeraFly()->getYDim();
	imgDims[2] = (*this->sharedcontrolPanelPtr)->thisCallback->getImageTeraFly()->getZDim();

	float imgRes[3];
	imgRes[0] = (*this->sharedcontrolPanelPtr)->thisCallback->getImageTeraFly()->getRezX();
	imgRes[1] = (*this->sharedcontrolPanelPtr)->thisCallback->getImageTeraFly()->getRezY();
	imgRes[2] = (*this->sharedcontrolPanelPtr)->thisCallback->getImageTeraFly()->getRezZ();

	float factor = pow(2, abs((*this->sharedcontrolPanelPtr)->CViewerPortal->getTeraflyTotalResLevel() - 1 - (*this->sharedcontrolPanelPtr)->CViewerPortal->getTeraflyResLevel()));
	//cout << "  -- scaling factor = " << factor << endl;
	//cout << "  -- current resolutionl level = " << (*this->sharedcontrolPanelPtr)->CViewerPortal->getTeraflyResLevel() + 1 << endl;
	//cout << "  -- total res levels: " << (*this->sharedcontrolPanelPtr)->CViewerPortal->getTeraflyTotalResLevel() << endl;

	float imgOri[3];
	string currWinTitle = (*this->sharedcontrolPanelPtr)->CViewerPortal->getCviewerWinTitle();
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