#ifndef _FRAGTRACEMANAGER_H_
#define _FRAGTRACEMANAGER_H_

#include <qprogressbar.h>
#include <qprogressdialog.h>
#include <qtimer.h>

#ifndef Q_MOC_RUN
#include "ImgManager.h"
#include "ImgAnalyzer.h"
#include "ImgProcessor.h"
#include "processManager.h"
#include "TreeGrower.h"
#include "NeuronStructExplorer.h"
#include "NeuronStructUtilities.h"
#endif

enum workMode { axon, dendriticTree };

class FragTraceManager: public QWidget
{
	Q_OBJECT

public:
	FragTraceManager() = default;
	FragTraceManager(const Image4DSimple* inputImg4DSimplePtr, workMode mode, bool slices = true);
	void reinit(const Image4DSimple* inputImg4DSimplePtr, workMode mode, bool slices = true);

	QString finalSaveRootQ;       // Save path for the traced result, acquired from UI.
	
	vector<string> imgEnhanceSeq; // Image Enhancement steps specified by the user, acquired from UI.
	vector<string> imgThreSeq;    // Image Thresholding steps specified by the user, acquired from UI.
	vector<int> partialVolumeLowerBoundaries;

	workMode mode;                // tracing axon or dendrite



/* =========================== Parameters =========================== */
	// ------- Image Enhancement ------- //
	bool ada;
	string adaImgName;
	int simpleAdaStepsize, simpleAdaRate, cutoffIntensity;
	bool saveAdaResults;
	QString simpleAdaSaveDirQ;

	bool gammaCorrection;
	// --------------------------------- //

	// ------- Image Segmentation ------- //
	bool histThre;
	string histThreImgName;
	float stdFold;
	bool saveHistThreResults;
	QString histThreSaveDirQ;
	// ---------------------------------- //

	// ------- Object Classification ------- //
	bool objFilter;
	bool voxelSize, actualSize;
	int voxelCount;

	vector<NeuronTree> tracedMultipleDendriticTrees;
	vector<int> currDisplayingBlockCenter;
	map<int, ImageMarker> selectedSomaMap;
	map<int, ImageMarker> selectedLocalSomaMap;
	// ------------------------------------- //

	// ------- Fragment Connection ------- //
	bool MST;
	string MSTtreeName;
	int minNodeNum;
	// ----------------------------------- //

	// ------- Blank Area Specification (to be deprecated) ------- //
	bool blankArea;
	vector<int> blankXs;
	vector<int> blankYs;
	vector<int> blankZs;
	vector<int> blankRadius;
	// ----------------------------------------------------------- //
/* ====================== END of [Parameters] ======================= */



	// ======= Crucial Intermediate Result ======= //
	vector<connectedComponent> signalBlobs;   // All segmented blobs are stored here.
	vector<connectedComponent> signalBlobs2D; // not used
	// =========================================== //



	// *********************************************************************************************** //
	bool imgProcPipe_wholeBlock(); // TRACING PROCESS STARTS HERE; CALLED FROM [FragTraceControlPanel].
	// *********************************************************************************************** //


/* ================= Result Finalization ================= */
	// -- Connects existing trees.
	profiledTree segConnectAmongTrees(const profiledTree& inputProfiledTree, float distThreshold);

signals:
	// -- Sends traced result back to FragTraceControlPanel
	void emitTracedTree(NeuronTree tracedTree);
/* ======================================================= */



public slots:
	bool blobProcessMonitor(ProcessManager& blobMonitor); // This mechanism is not completed yet.



private:
/* ======= FragTraceManager Fascilities ======= */
	ImgManager fragTraceImgManager;
	ImgAnalyzer fragTraceImgAnalyzer;
	TreeGrower fragTraceTreeGrower;
	NeuronStructExplorer fragTraceTreeManager;
/* ============================================ */

	int numProcs;
	QProgressDialog* progressBarDiagPtr;

	inline void saveIntermediateResult(const string imgName, const QString saveRootQ, V3DLONG dims[]);
	


/* =================== Image Enhancement =================== */
	void adaThre(const string inputRegImgName, V3DLONG dims[], const string outputRegImgName);
	void simpleThre(const string inputRegImgName, V3DLONG dims[], const string outputRegImgName);
	void gammaCorrect(const string inputRegImgName, V3DLONG dims[], const string outputRegImgName);
/* ========================================================= */



/* =================== Image Segmentation =================== */
	void histThreImg(const string inputRegImgName, V3DLONG dims[], const string outputRegImgName);
	void histThreImg3D(const string inputRegImgName, V3DLONG dims[], const string outputRegImgName);
	
	// ------- Object Classification ------- //
	void smallBlobRemoval(vector<connectedComponent>& signalBlobs, const int sizeThre);
	// ------------------------------------- //

	bool mask2swc(const string inputImgName, string outputTreeName);
	
	// -- Each signal blob is represented by its centroid
	inline void get2DcentroidsTree(vector<connectedComponent> signalBlobs);
/* ========================================================== */

	

/* =================== Final Traced Tree Generation =================== */
	profiledTree straightenSpikeRoots(const profiledTree& inputProfiledTree, double angleThre = 0.5);
	bool generateTree(workMode mode, profiledTree& objSkeletonProfiledTree);

	vector<connectedComponent> peripheralSignalBlobs;
	NeuronTree getPeripheralSigTree(const profiledTree& inputProfiledTree, int lengthThreshold);
	vector<connectedComponent> getPeripheralBlobs(const NeuronTree& inputNeuronTree);
/* ==================================================================== */
};

inline void FragTraceManager::saveIntermediateResult(const string imgName, const QString saveRootQ, V3DLONG dims[])
{
	if (this->fragTraceImgManager.imgDatabase.find(imgName) == this->fragTraceImgManager.imgDatabase.end())
	{
		cerr << "The specified image doesn't exist. Do nothing and return.";
		return;
	}

	qint64 timeStamp_qint64 = QDateTime::currentMSecsSinceEpoch();
	QString timeStampQ = QString::number(timeStamp_qint64);
	QString fullSaveRootQ = saveRootQ + "\\" + timeStampQ;
	if (!QDir(fullSaveRootQ).exists()) QDir().mkpath(fullSaveRootQ);

	for (map<string, myImg1DPtr>::iterator it = this->fragTraceImgManager.imgDatabase.at(imgName).slicePtrs.begin();
		it != this->fragTraceImgManager.imgDatabase.at(imgName).slicePtrs.end(); ++it)
	{
		string saveFullPath = fullSaveRootQ.toStdString() + "\\" + it->first;
		const char* saveFullPathC = saveFullPath.c_str();
		ImgManager::saveimage_wrapper(saveFullPathC, it->second.get(), dims, 1);
	}
}

inline void FragTraceManager::get2DcentroidsTree(vector<connectedComponent> signalBlobs)
{
	NeuronTree centerTree;
	for (vector<connectedComponent>::iterator it = signalBlobs.begin(); it != signalBlobs.end(); ++it)
	{
		ChebyshevCenter_connComp(*it);
		NeuronSWC centerNode;
		centerNode.n = it->islandNum;
		centerNode.x = it->ChebyshevCenter[0];
		centerNode.y = it->ChebyshevCenter[1];
		centerNode.z = it->ChebyshevCenter[2];
		centerNode.type = 2;
		centerNode.parent = -1;
		centerTree.listNeuron.push_back(centerNode);
	}

	profiledTree profiledCenterTree(centerTree);
	this->fragTraceTreeManager.treeDataBase.insert({ "centerTree", profiledCenterTree });
}

#endif