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
	FragmentEditor(V3DPluginCallback2* callback) : thisCallback(callback), CViewerPortal(callback->castCViewer) { this->editorGrowerPtr = new TreeGrower(&this->editorExplorer); }

	void updateCViewerPortal() { this->CViewerPortal = thisCallback->castCViewer; }

	vector<V_NeuronSWC> inputSegList;
	map<int, segUnit> segMap;
	boost::container::flat_multimap<int, int> node2segMap;
	void erasingProcess(V_NeuronSWC_list& displayingSegs, const float nodeCoords[]); // DEPRECATED
	void erasingProcess(V_NeuronSWC_list& displayingSegs, const map<int, vector<NeuronSWC>>& seg2includedNodeMap);
	void connectingProcess(V_NeuronSWC_list& displayingSegs, const map<int, vector<NeuronSWC>>& seg2includedNodeMap);

private:
	V3DPluginCallback2* thisCallback;
	INeuronAssembler* CViewerPortal;

	NeuronStructExplorer editorExplorer;
	TreeGrower* editorGrowerPtr;

	// This method determines the cutting range for eraser function. 
	// NOTE, currently the range is empirically determined, due to the inaccuracy of both [Renderer_gl1::findNearestNeuronNode_WinXY] and [terafly::myRengerer_gl1::get3Dpoint].
	// This method must be optimized later by fitting a function of 2 variables: [zooomLevel] and [eraserSize].
	float getErasingRange(const int teraflyResPow, const int zoomLevel, const int eraserSize); // DEPRECATED

	void erasingProcess_cuttingSeg(V_NeuronSWC_list& displayingSegs, const map<int, set<int>>& seg2BeditedInfo);
	vector<float> getSegEndPointingVec(const segUnit& inputSeg, const int endNodeID, int nodeNum = 3);
	NeuronSWC mostProbableBodyNode(const vector<float>& segEndPointingVec, const vector<NeuronSWC>& bodyNodes, const NeuronSWC& endNode);
};



#endif