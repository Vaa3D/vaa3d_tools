#ifndef RECONOPERATOR_H
#define RECONOPERATOR_H

#define SUBTREE_DEBUG

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
	ReconOperator() : removeSpike(false), branchNodeMin(-1) {}

	QString rootPath;
	int xFactor, yFactor, zFactor;

	bool removeSpike;
	int branchNodeMin;

	void downSampleReconFile(const QStringList& fileList, float xFactor, float yFactor, float zFactor);

	void denAxonSeparate(const QStringList& fileList);
	void denAxonCombine(bool dupRemove);

	void removeDupedNodes();
};

#endif