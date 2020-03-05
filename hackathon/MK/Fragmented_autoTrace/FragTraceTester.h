#ifndef FRAGTRACETESTER_H
#define FRAGTRACETESTER_H

#include <memory>

#include "FragTraceControlPanel.h"
#include "FragTraceManager.h"

class FragTraceTester
{
public:
	FragTraceTester() = default;
	FragTraceTester(FragTraceControlPanel* controlPanelPtr) { this->sharedcontrolPanelPtr = make_shared<FragTraceControlPanel*>(controlPanelPtr); }

	// IMPORTANT: Use a shared_ptr here to avoid dangling pointers. 
	shared_ptr<FragTraceControlPanel*> sharedcontrolPanelPtr;

	profiledTree segEndClusterCheck(const profiledTree& inputProfiledTree, QString savePathQ);
	void scale(profiledTree& inputProfiledTree);
};

#endif