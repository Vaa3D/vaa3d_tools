#ifndef _FRAGTRACEMANAGER_H_
#define _FRAGTRACEMANAGER_H_

#include "v3d_interface.h"
#include "INeuronAssembler.h"

#include <qprogressbar.h>
#include <qprogressdialog.h>
#include <qtimer.h>

#ifndef Q_MOC_RUN
#include "ImgManager.h"
#include "ImgAnalyzer.h"
#include "ImgProcessor.h"
#include "processManager.h"
#include "TreeGrower.h"
#include "TreeTrimmer.h"
#include "NeuronStructExplorer.h"
#include "NeuronStructUtilities.h"

#include "FragTraceImgProcessor.h"
#include "FragmentPostProcessor.h"
#endif

enum workMode { axon, dendriticTree };

class FragTraceManager : public QWidget
{
	Q_OBJECT

	friend class FragTraceImgProcessor;
	friend class FragmentPostProcessor;
	friend class FragTraceTester;

public:
/* =========================== Parameters from UI =========================== */
	QString finalSaveRootQ;       // Save path for the traced result, acquired from UI.

	vector<string> imgEnhanceSeq; // Image Enhancement steps specified by the user, acquired from UI.
	vector<string> imgThreSeq;    // Image Thresholding steps specified by the user, acquired from UI.
	workMode mode;                // tracing axon or dendrite

	// ------- Partial Volume Tracing ------- //
	bool partialVolumeTracing;
	vector<int> partialVolumeLowerBoundaries;

	// This is an array directly assgigned from FragTraceControlPanel. 
	// There are 6 elements: xlb, xhb, ylb, yhb, zlb, zhb.
	int* volumeAdjustedBounds; 
	// -------------------------------------- //

	// ------- Image Enhancement ------- //
	bool ada;
	string adaImgName;
	int simpleAdaStepsize, simpleAdaRate, cutoffIntensity;

	bool gammaCorrection;
	// --------------------------------- //

	// ------- Image Segmentation ------- //
	bool histThre;
	string histThreImgName;
	float stdFold;
	// ---------------------------------- //

	// ------- Object Classification ------- //
	bool objFilter;
	bool voxelSize, actualSize;
	bool continuousAxon;
	int voxelCount;
	int axonMarkerAllowance;

	vector<NeuronTree> tracedMultipleDendriticTrees;
	vector<int> currDisplayingBlockCenter;
	map<int, ImageMarker> selectedSomaMap;
	map<int, ImageMarker> selectedLocalSomaMap;
	map<int, ImageMarker> localAxonMarkerMap;
	// ------------------------------------- //

	// ------- Fragment Connection ------- //
	bool MST;
	string MSTtreeName;
	int minNodeNum;
	// ----------------------------------- //

	enum imgProcSteps { gamma_Correction,
						adaptiveThresholding, simpleThresholding, histBasedThresholding3D,
						mask2swcBlobs };
/* ====================== END of [Parameters from UI] ======================= */


// ======= Constructors and Basic Member Functions ======= //
	FragTraceManager() = default;
	FragTraceManager(const Image4DSimple* inputImg4DSimplePtr, workMode mode, bool slices = true);
	~FragTraceManager();
	void reinit(const Image4DSimple* inputImg4DSimplePtr, workMode mode, bool slices = true);	
// ======================================================= //


// ======= Crucial Intermediate Result ======= //
	vector<connectedComponent> signalBlobs;   // All segmented blobs are stored here.
	vector<connectedComponent> signalBlobs2D; // not used
// =========================================== //


// **************************************************************************************************** //
	bool imgProcPipe_wholeBlock(); // TRACING PROCESS STARTS HERE; CALLED FROM [FragTraceControlPanel].
// **************************************************************************************************** //


signals:
	void emitTracedTree(NeuronTree tracedTree); // -- Sends traced result back to FragTraceControlPanel
	void getExistingFinalTree(NeuronTree& existingTree);

private:
/* ======= FragTraceManager Fascilities ======= */
	ImgManager fragTraceImgManager;
	ImgAnalyzer fragTraceImgAnalyzer;
	NeuronStructExplorer fragTraceTreeManager;
	TreeGrower* fragTraceTreeGrowerPtr;
	TreeTrimmer* fragTraceTreeTrimmerPtr;  

	FragTraceImgProcessor myImgProcessor;
	FragmentPostProcessor myFragPostProcessor;
/* ============================================ */
	

/* =================== Image Enhancement =================== */
	void adaThre(const string inputRegImgName, V3DLONG dims[], const string outputRegImgName);
	void simpleThre(const string inputRegImgName, V3DLONG dims[], const string outputRegImgName);
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
	

/* =================== Traced Tree Generating and Polishing =================== */
	bool treeAssembly(NeuronTree& PRE_FINALOUTPUT_TREE);
	bool generateTree(workMode mode, profiledTree& objSkeletonProfiledTree);
	profiledTree straightenSpikeRoots(const profiledTree& inputProfiledTree, double angleThre = 0.5);

	map<string, vector<connectedComponent>> peripheralSignalBlobMap;
	NeuronTree getPeripheralSigTree(const profiledTree& inputProfiledTree, int lengthThreshold) const;
	vector<connectedComponent> getPeripheralBlobs(const NeuronTree& inputNeuronTree, const vector<int> origin);
	map<string, profiledTree> generatePeriRawDenTree(const map<string, vector<connectedComponent>>& periSigBlobMap);
	NeuronTree getSmoothedPeriDenTree();

	// This method performs itered-cluster based connecting first. 
	// Then change types if segments are connected to alredy typed existing segments. Duplicated nodes are also removed.
	profiledTree segConnect_withinCurrBlock(const profiledTree& inputProfiledTree, float distThreshold);

	set<int> seedCluster;
	inline set<vector<float>> getAxonMarkerProbes() const;
	NeuronTree axonGrow(const NeuronTree& inputTree, const NeuronTree& scaledExistingTree);
	map<int, segEndClusterUnit*> segEndClusterChains;
/* ============================================================================ */

	
public:
	/* ================= Traced Tree Post Processing ================= */
	float scalingFactor;
	float imgOrigin[3];
	int displayImgDim[3];

	NeuronTree existingTree;
	/* =============================================================== */


private:
	inline void saveIntermediateResult(const string imgName, const QString saveRootQ, V3DLONG dims[]);
	int numProcs;
	QProgressDialog* progressBarDiagPtr;


public slots:
	bool blobProcessMonitor(ProcessManager& blobMonitor); // This mechanism is not completed yet.
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

inline set<vector<float>> FragTraceManager::getAxonMarkerProbes() const
{
	set<vector<float>> probes;
	for (map<int, ImageMarker>::const_iterator it = this->localAxonMarkerMap.begin(); it != this->localAxonMarkerMap.end(); ++it)
	{
		vector<float> probe;
		probe.push_back(it->second.x);
		probe.push_back(it->second.y);
		probe.push_back(it->second.z);
		probes.insert(probe);
	}

	return probes;
}

#endif