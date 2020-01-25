#ifndef FRAGMENTEDITOR_H
#define FRAGMENTEDITOR_H

#include <iostream>

#include <v3d_interface.h>

#ifndef Q_MOC_RUN
#include "NeuronStructUtilities.h"
#endif

using namespace std;

class FragmentEditor : public QWidget
{
	Q_OBJECT;

public:
	FragmentEditor(QWidget* parent, V3DPluginCallback2* callback) : thisCallback(callback) {};

	vector<V_NeuronSWC> inputSegList;
	map<int, segUnit> segMap;
	boost::container::flat_multimap<int, int> node2segMap;
	void erasingProcess(V_NeuronSWC_list& displayingSegs, const float nodeCoords[]);

private:
	V3DPluginCallback2* thisCallback;

	void erasingProcess_cuttingSeg(V_NeuronSWC_list& displayingSegs, const map<int, set<int>>& seg2BeditedInfo);
};



#endif