//------------------------------------------------------------------------------
// Copyright (c) 2020 Hsienchi Kuo (Allen Institute)
// All rights reserved.
//------------------------------------------------------------------------------

/*******************************************************************************
*
*  [FragTraceManager] controls the flow of auto-traced segment generation. This is the controlling center of the entire Neuron Assembler app.
*  
*  The mission of [FragTraceManager] is as follow:
*    1. Use [FragTraceImgProcessor] to process the input image.
*    2. Generate signal blobs.
*	 3. Extract segments/skeletons out of signal blobs.
*	 4. Refine extracted trees with [FragTracePostProcessor].
*	 5. Perform higher level tree forming process with [FragTracePostProcessor].
*
********************************************************************************/

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
// =========================================== //


// **************************************************************************************************** //
	// TRACING PROCESS STARTS HERE; called from [FragTraceControlPanel];
	bool imgProcPipe_wholeBlock();						 // PRIMARY SEQUENCE CONTROL
	
	// Connect segments, refine and smoothe, then clean up junks
	bool treeAssembly(NeuronTree& PRE_FINALOUTPUT_TREE); // SECONDARY SEQUENCE CONTROL 
// **************************************************************************************************** //


signals:
	void emitTracedTree(NeuronTree tracedTree); // -- Sends traced result back to FragTraceControlPanel
	void getExistingFinalTree(NeuronTree& existingTree);

private:
/* ======= FragTraceManager Fascilities ======= */
	ImgManager fragTraceImgManager;
	NeuronStructExplorer fragTraceTreeManager;
	TreeGrower* fragTraceTreeGrowerPtr;
	TreeTrimmer* fragTraceTreeTrimmerPtr;  

	FragTraceImgProcessor myImgProcessor;
	FragmentPostProcessor myFragPostProcessor;
/* ============================================ */
	

/* =================== Traced Tree Generating and Polishing =================== */
	// Generate auto-traced segments
	bool generateTree(workMode mode, profiledTree& objSkeletonProfiledTree);
	
	

	map<string, vector<connectedComponent>> peripheralSignalBlobMap;
	NeuronTree getPeripheralSigTree(const profiledTree& inputProfiledTree, int lengthThreshold) const;
	vector<connectedComponent> getPeripheralBlobs(const NeuronTree& inputNeuronTree, const vector<int> origin);
	map<string, profiledTree> generatePeriRawDenTree(const map<string, vector<connectedComponent>>& periSigBlobMap);
	NeuronTree getSmoothedPeriDenTree();

	// Using segment end cluster approach to connect obvious segments
	profiledTree segConnect_withinCurrBlock(const profiledTree& inputProfiledTree, float distThreshold);

	// Extended axon tracing methods.
	set<int> seedCluster;
	map<int, segEndClusterUnit*> segEndClusterChains;
	inline set<vector<float>> getAxonMarkerProbes() const;
	NeuronTree axonGrow(const NeuronTree& inputTree, const NeuronTree& scaledExistingTree);
/* ============================================================================ */

	
public:
	/* ================= Traced Tree Post Processing ================= */
	float scalingFactor;
	float imgOrigin[3];
	int displayImgDim[3];

	NeuronTree existingTree;

	vector<V_NeuronSWC> getCurrentVolumeV_NeuronSWCs() { return this->totalV_NeuronSWCs; }
	void addV_NeuronSWCs(const profiledTree& inputProfiledTree);
	/* =============================================================== */


private:
	inline void saveIntermediateResult(const string imgName, const QString saveRootQ, V3DLONG dims[]);

	vector<V_NeuronSWC> totalV_NeuronSWCs; // Current V_NeuronSWC in display, acquired from My4DImage::tracedNeuron.

	int numProcs;
	QProgressDialog* progressBarDiagPtr;
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