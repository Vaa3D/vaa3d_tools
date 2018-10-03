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
	void sliceThre(float threPercentile);
	void threshold3D(float threPercentile);
	void sliceBkgThre();
	void sliceReversedGammaCorrect();
	
	void histQuickList();

	vector<connectedComponent> signalBlobs;
	QList<NeuronSWC> centers;
	void findSignalBlobs2D();
	void swc2DsignalBlobsCenter();
	void swcSignalBlob3Dcenter();
	void findSomaMass(int somaSizeThre = 27);
	void getChebyshevCenters(QString caseNum);

	void somaNeighborhoodThin();

	void swc_imgCrop();
	
	void getMST();
	void getMST_2Dslices();
	void swcScale(float xScale, float yScale, float zScale);
	void swcRegister();
	void swcMapBack();
	void correctSWC();
	void getTiledMST();
	void cutMST();
	void MSTtrim();
	void breakMSTbranch();
	void segElongation();
	void segTerminalize();
	void dotRemove();
	void longConnCut();
	void treeUnion();
	void treeWithinDist();

	void nodeIdentify();
	void swcSeparate(QString outputRoot2);
	void cleanUpzFor2Dcentroids();

private:
	ImgManager* myImgManagerPtr;
	ImgAnalyzer* myImgAnalyzerPtr;
	NeuronStructUtil* myNeuronUtilPtr;
	NeuronStructExplorer* myNeuronStructExpPtr;

	void singleTaskDispatcher(deque<task> taskList);

	inline NeuronTree swcBackTrack(int xFactor, int yFactor, int zFactor, float xShift, float yShift, float zShift, NeuronTree& intputTree);
};

NeuronTree SegPipe_Controller::swcBackTrack(int xFactor, int yFactor, int zFactor, float xlb, float ylb, float zlb, NeuronTree& inputTree)
{
	NeuronTree outputTree;
	for (QList<NeuronSWC>::iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		NeuronSWC newNode;
		newNode = *it;
		newNode.x = it->x / xFactor - xlb;
		newNode.y = it->y / yFactor - ylb;
		newNode.z = it->z / zFactor - zlb;
		outputTree.listNeuron.push_back(newNode);
	}

	return outputTree;
}

#endif