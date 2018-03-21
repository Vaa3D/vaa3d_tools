#ifndef NEURONSTRUCTEXPLORER_H
#define NEURONSTRUCTEXPLORER_H

#include <vector>
#include <string>

#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>

#include "basic_surf_objs.h"

using namespace std;

class NeuronStructExplorer
{
public:
	/********* Constructors *********/
	NeuronStructExplorer() {};
	NeuronStructExplorer(QString neuronFileName);
	NeuronStructExplorer(string neuronFileName);
	/********************************/

	NeuronTree* singleTreePtr;
	NeuronTree singleTree;
	vector<NeuronTree>* treeVectorPtr;
	QString neuronFileName;

	/********* Neuron structure file basic operations *********/
	static void swcFlipY(NeuronTree const* inputTreePtr, NeuronTree*& outputTreePtr, long int yLength);
	/**********************************************************/
};

#endif