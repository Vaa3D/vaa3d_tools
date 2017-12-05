#ifndef NEURONSTRUCTNAVIGATOR_H
#define NEURONSTRUCTNAVIGATOR_H

#include <vector>
#include <queue>

#include <qstring.h>

#include <v3d_interface.h>
#include "basic_surf_objs.h"

using namespace std;

struct nodeInfo
{
	// ------- information from original neuron structure file -------
	float x_coord, y_coord, z_coord;
	int type;
	// ---------------------------------------------------------------

	int xlb, xhb, ylb, yhb, zlb, zhb; // information needed by image cropping functions

	int nameX, nameY, nameZ; // for patch file name
};

class NeuronStructNavigator
{
public:
	NeuronStructNavigator(int xRadius, int yRadius, int zRadius, int imgX, int imgY, int imgZ);

	// ------- original image stack dimensions and cube side length sent from UI -------
	int uiXradius, uiYradius, uiZradius;
	int stackX, stackY, stackZ;
	// ---------------------------------------------------------------------------------
	
	QString neuronStructFileName;
	queue<nodeInfo> nodeQueue;
	void generateNodeQueue();
};

#endif