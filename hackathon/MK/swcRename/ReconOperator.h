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
	QString rootPath;
	int xFactor, yFactor, zFactor;

	void downSampleReconFile(const QStringList& fileList, float xFactor, float yFactor, float zFactor);

	void denAxonSeparate(const QStringList& fileList);
	void denAxonCombine(bool dupRemove);

	void removeDupedNodes();
};