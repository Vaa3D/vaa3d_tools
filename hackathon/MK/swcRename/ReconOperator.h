#ifndef RECONOPERATOR_H
#define RECONOPERATOR_H

//#define SUBTREE_DEBUG
<<<<<<< HEAD
//#define DUPSEG_REMOVE
=======
>>>>>>> 0ffe00a584d35588e86f5063ca57565605c6b052

#include <iostream>

#include <qstring.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qfile.h>

#include "NeuronStructUtilities.h"
#include "NeuronStructExplorer.h"

class ReconOperator
{
public:
	ReconOperator() : autoConnect(false), removeSpike(false), branchNodeMin(-1), connectThreshold(-1) {}

	QString rootPath;
	int xFactor, yFactor, zFactor;

	bool autoConnect;
	bool removeSpike;
	int branchNodeMin;
	int connectThreshold;

	void downSampleReconFile(const QStringList& fileList, float xFactor, float yFactor, float zFactor);

	void denAxonSeparate(const QStringList& fileList);
	void denAxonCombine(bool dupRemove);

	void removeDupedNodes();
};

#endif