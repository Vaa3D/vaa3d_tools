#ifndef SEGPIPE_CONTROLLER_H
#define SEGPIPE_CONTROLLER_H

#include <string>
#include <vector>
#include <deque>
#include <map>
#include <set>

#include <qstring.h>
#include <qstringlist.h>

#include "basic_surf_objs.h"

#include "ImgManager.h"
#include "ImgAnalyzer.h"
#include "ImgProcessor.h"
#include "NeuronStructUtilities.h"
#include "NeuronStructExplorer.h"

using namespace std;

enum folderStruct {singleCase, multipleCase};
enum task {downsample2D, threshold2D, bkgThreshold2D, gammaCorrect2D};

class SegPipe_Controller
{
public:
	/***************** Constructors and Basic Data Members *****************/
	SegPipe_Controller(QString inputPath, QString outputPath);

	folderStruct inputContent;
	QString inputCaseRootPath;
	QString inputSWCRootPath;
	QString refSWCRootPath;
	QString outputRootPath;
	QString outputSWCRootPath;
	QStringList caseList;
	QStringList swcList;

	deque<string> inputSingleCaseSliceFullPaths;
	deque<string> outputSingleCaseSliceFullPaths;
	multimap<string, string> inputMultiCasesSliceFullPaths;
	multimap<string, string> outputMultiCasesSliceFullPaths;
	/***********************************************************************/

	deque<task> taskList;

	unsigned char* currProcessingImgPtr;

	void sliceDownSample2D(int downFactor, string method = "");
	void sliceGammaCorrect();
	void sliceThre(float threshold);
	void sliceBkgThre();
	void sliceReversedGammaCorrect();
	
	void histQuickList();

	vector<connectedComponent> signalBlobs;
	QList<NeuronSWC> centers;
	void findSignalBlobs2D();
	void swc2DsignalBlobsCenter();
	void swcSignalBlob3Dcenter();
	void findSomaMass();
	void getChebyshevCenters(QString caseNum);

	void somaNeighborhoodThin();

	void swc_imgCrop();
	
	void getMST();
	void getMST_2Dslices();
	void swcScale(float xScale, float yScale, float zScale);
	void swcRegister();
	void getTiledMST();
	void cutMST();
	void MSTtrim();

private:
	ImgManager* myImgManagerPtr;
	ImgAnalyzer* myImgAnalyzerPtr;
	NeuronStructUtil* myNeuronUtilPtr;
	NeuronStructExplorer* myNeuronStructExpPtr;

	void singleTaskDispatcher(deque<task> taskList);
};

#endif