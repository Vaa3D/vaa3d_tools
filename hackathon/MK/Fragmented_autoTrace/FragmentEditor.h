#ifndef FRAGMENTEDITOR_H
#define FRAGMENTEDITOR_H

#include <iostream>

#include <v3d_interface.h>

#include "integratedDataTypes.h"

using namespace std;

class FragmentEditor : public QWidget
{
	Q_OBJECT;

public:
	FragmentEditor(QWidget* parent, V3DPluginCallback2* callback);

	vector<V_NeuronSWC> inputV_NeuronSWCs;
	vector<V_NeuronSWC> erasingProcess(const float nodeCoords[]);

private:
	V3DPluginCallback2* thisCallback;
};



#endif