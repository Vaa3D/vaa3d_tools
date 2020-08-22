//------------------------------------------------------------------------------
// Copyright (c) 2020 Hsienchi Kuo (Allen Institute)
// All rights reserved.
//------------------------------------------------------------------------------

/*******************************************************************************
*
*  [FragmentEditor] is a UI level segment editing class.
*  This class receives calls from [FragTraceControlPanel] to perform editing actions on those segments in display.
*  [FragmentEditor] doesn't invovle in any auto-traced segments generation. It serves as an enhancement to basic Vaa3D editing functions.
*  Currently the enhanced editing functions include connecting, erasing, show/hide, etc.
*
*  A typical working scheme looks like this:
*
*  [terafly::CViewer]/[terafly::CViewer.view3DWidget] -> [Fragmented_Auto-traced_plugin] 
*	   																				     \
*																						  -> [FragTraceControlPanel] -> [FragmentEditor] -> back to [terafy::CViewer]
*																						 /
*												Neuron Assembler segment editing buttons
*
********************************************************************************/

#ifndef FRAGMENTEDITOR_H
#define FRAGMENTEDITOR_H

#include <iostream>

#include <v3d_interface.h>
#include "INeuronAssembler.h"

#ifndef Q_MOC_RUN
#include "NeuronStructUtilities.h"
#include "NeuronStructExplorer.h"
#include "TreeGrower.h"
#include "TreeTrimmer.h"
#endif

using namespace std;

class FragmentEditor : public QWidget
{
	Q_OBJECT;

public:
	// The access to v3d interface and terafly::CViewer interface is captured in constructor.
	FragmentEditor(V3DPluginCallback2* callback) : thisCallback(callback), CViewerPortal(callback->castCViewer) {};

	// [CViewerPortal] needs to be updated whenever a change of image volume happens.
	void updateCViewerPortal() { this->CViewerPortal = thisCallback->castCViewer; }

	vector<V_NeuronSWC> inputSegList; // Normally the [My4DImage::tracedNeuron] sent over by [terafly::CViewer]
	map<int, segUnit> segMap;
	boost::container::flat_multimap<int, int> node2segMap;
	
	// Erasing and connecting functions
	void erasingProcess(V_NeuronSWC_list& displayingSegs, const map<int, vector<NeuronSWC>>& seg2includedNodeMap);
	void connectingProcess(V_NeuronSWC_list& displayingSegs, const map<int, vector<NeuronSWC>>& seg2includedNodeMap);

	// Sequential segment typing
	bool sequentialTypeToggled;
	void sequencialTypeChanging(V_NeuronSWC_list& displayingSegs, const int seedSegID, const int type);
	void sequencialTypeChanging(V_NeuronSWC_list& displayingSegs, const set<int>& startingSegs, const int type);

private:
	V3DPluginCallback2* thisCallback;
	INeuronAssembler* CViewerPortal;

	//NeuronStructExplorer editorExplorer;
	//TreeGrower* editorGrowerPtr;

	void erasingProcess_cuttingSeg(V_NeuronSWC_list& displayingSegs, const map<int, set<int>>& seg2BeditedInfo);
	vector<float> getSegEndPointingVec(const segUnit& inputSeg, const int endNodeID, int nodeNum = 3);
	NeuronSWC mostProbableBodyNode(const vector<float>& segEndPointingVec, const vector<NeuronSWC>& bodyNodes, const NeuronSWC& endNode);
	set<int> getNodeTypesInSeg(const V_NeuronSWC& inputVneuronSWC);
	void rc_findConnectedSegs(const profiledTree& inputProfiledTree, const set<int>& seedSegs, set<int>& connectedSegs);
};



#endif