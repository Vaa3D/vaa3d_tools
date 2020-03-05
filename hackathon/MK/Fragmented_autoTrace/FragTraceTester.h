#ifndef FRAGTRACETESTER_H
#define FRAGTRACETESTER_H

#include <memory>

#include "FragTraceControlPanel.h"
#include "FragTraceManager.h"

class FragTraceTester
{
public:
	FragTraceTester(FragTraceControlPanel* controlPanelPtr) { this->sharedControlPanelPtr = make_shared<FragTraceControlPanel*>(controlPanelPtr); }

	static FragTraceTester* testerInstance;
	static FragTraceTester* instance(FragTraceControlPanel* controlPanelPtr);
	static FragTraceTester* getInstance();
	static void uninstance();
	static bool isInstantiated() { return testerInstance != nullptr; }

	shared_ptr<FragTraceControlPanel*> sharedControlPanelPtr;
	shared_ptr<FragTraceManager*> sharedTraceManagerPtr;

	profiledTree segEndClusterCheck(const profiledTree& inputProfiledTree, QString savePathQ);
	void scale(profiledTree& inputProfiledTree);
};

#endif