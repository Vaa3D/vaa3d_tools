#ifndef SEGPIPE_CONTROLLER_H
#define SEGPIPE_CONTROLLER_H

#include <string>
#include <vector>
#include <deque>
#include <map>

#include <qstring.h>
#include <qstringlist.h>

#include "basic_surf_objs.h"

using namespace std;

enum folderStruct {singleCase, multipleCase};

class SegPipe_Controller
{
public:
	/***************** Constructors and Basic Data Members *****************/
	SegPipe_Controller(QString inputPath, QString outputPath);

	folderStruct inputContent;
	QString inputCaseRootPath;
	QString outputRootPath;
	QStringList caseList;
	deque<string> inputSingleCaseSliceFullPaths;
	deque<string> outputSingleCaseSliceFullPaths;
	multimap<string, string> inputMultiCasesSliceFullPaths;
	multimap<string, string> outputMultiCasesSliceFullPaths;
	/***********************************************************************/

	unsigned char* currProcessingImgPtr;

	void sliceDownSample2D(int downFactor, string method = "");
	void adaSliceGammaCorrect();
	void sliceThre(float threshold);
	void sliceBkgThre();
	void sliceReversedGammaCorrect();
	
	void histQuickList();

	map<QString, QList<NeuronSWC> > findSoma();
	void findSomaMass();
	void somaNeighborhoodThin();
};

#endif