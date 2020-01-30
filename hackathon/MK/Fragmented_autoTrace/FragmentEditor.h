#ifndef FRAGMENTEDITOR_H
#define FRAGMENTEDITOR_H

#include <iostream>

#include <v3d_interface.h>
#include "INeuronAssembler.h"

#ifndef Q_MOC_RUN
#include "NeuronStructUtilities.h"
#endif

using namespace std;

class FragmentEditor : public QWidget
{
	Q_OBJECT;

public:
	FragmentEditor(QWidget* parent, V3DPluginCallback2* callback) : thisCallback(callback), CViewerPortal(callback->castCViewer) {};

	void updateCViewerPortal() { this->CViewerPortal = thisCallback->castCViewer; }

	vector<V_NeuronSWC> inputSegList;
	map<int, segUnit> segMap;
	boost::container::flat_multimap<int, int> node2segMap;
	void erasingProcess(V_NeuronSWC_list& displayingSegs, const float nodeCoords[], const int mouseX, const int mouseY);

private:
	V3DPluginCallback2* thisCallback;
	INeuronAssembler* CViewerPortal;

	// This method determines the cutting range for eraser function. 
	// NOTE, currently the range is empirically determined, due to the inaccuracy of both [Renderer_gl1::findNearestNeuronNode_WinXY] and [terafly::myRengerer_gl1::get3Dpoint].
	// This method must be optimized later by fitting a function of 2 variables: [zooomLevel] and [eraserSize].
	float getErasingRange(const int teraflyResPow, const int zoomLevel, const int eraserSize);
	void erasingProcess_cuttingSeg(V_NeuronSWC_list& displayingSegs, const map<int, set<int>>& seg2BeditedInfo);
};



#endif