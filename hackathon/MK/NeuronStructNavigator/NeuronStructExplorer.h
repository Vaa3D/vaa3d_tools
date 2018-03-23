#ifndef NEURONSTRUCTEXPLORER_H
#define NEURONSTRUCTEXPLORER_H

#include <vector>
#include <unordered_map>
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

	unordered_map<string, unordered_map<int, float>> zProfileMap;
	void swcXYprofile(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr);
	void swcZcleanup(unordered_map<string, unordered_map<int, float>> zProfileMap, NeuronTree* outputTreePtr, int minSecNum, bool max, int threshold = 0);

	void swcDetectedDist(NeuronTree* inputTreePtr1, NeuronTree* inputTreePtr2);
};

#endif