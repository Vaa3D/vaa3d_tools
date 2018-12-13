#ifndef _FRAGTRACEMANAGER_H_
#define _FRAGTRACEMANAGER_H_

#include "ImgManager.h"
#include "ImgAnalyzer.h"
#include "ImgProcessor.h"
#include "NeuronStructExplorer.h"
#include "NeuronStructUtilities.h"

class FragTraceManager: public QObject
{
	Q_OBJECT

public:
	FragTraceManager() {};
	FragTraceManager(const Image4DSimple* inputImg4DSimplePtr, bool slices = true);

	QString finalSaveRootQ;
	vector<string> imgEnhanceSeq;
	vector<string> imgThreSeq;

	bool ada;
	string adaImgName;
	int simpleAdaStepsize;
	int simpleAdaRate;
	bool saveAdaResults;
	QString simpleAdaSaveDirQ;

	bool gammaCorrection;

	bool histThre;
	string histThreImgName;
	int stdFold;
	bool saveHistThreResults;
	QString histThreSaveDirQ;

	bool smallBlobRemove;
	string smallBlobRemovalName;
	int smallBlobThreshold;
	bool saveSmallBlobRemovalResults;
	QString smallBlobRemoveDirQ;

	bool MST;
	bool tiledMST;
	string MSTtreeName;
	int segLengthLimit, minNodeNum, tileLength, zSectionNum;
	bool saveMSTresults;
	QString saveMSTDirQ;

	vector<connectedComponent> signalBlobs;
	vector<connectedComponent> signalBlobs2D;

public slots:
	void imgProcPipe_wholeBlock();

signals:
	void emitTracedTree(NeuronTree tracedTree);

private:
	vector<vector<unsigned char>> imgSlices;
	ImgManager fragTraceImgManager;
	NeuronStructExplorer fragTraceTreeManager;
	NeuronStructUtil fragTraceTreeUtil;

	inline void saveIntermediateResult(const string imgName, const QString saveRootQ, V3DLONG dims[]);
	void adaThre(const string inputRegImgName, V3DLONG dims[], const string outputRegImgName);
	void gammaCorrect(const string inputRegImgName, V3DLONG dims[], const string outputRegImgName);
	void histThreImg(const string inputRegImgName, V3DLONG dims[], const string outputRegImgName);

	void mask2swc(const string inputImgName, string outputTreeName);
	void smallBlobRemoval(vector<connectedComponent>& signalBlobs, const int sizeThre);
	inline void get2DcentroidsTree(vector<connectedComponent> signalBlobs);
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
		this->fragTraceImgManager.saveimage_wrapper(saveFullPathC, it->second.get(), dims, 1);
	}
}

inline void FragTraceManager::get2DcentroidsTree(vector<connectedComponent> signalBlobs)
{
	NeuronTree centerTree;
	for (vector<connectedComponent>::iterator it = signalBlobs.begin(); it != signalBlobs.end(); ++it)
	{
		ImgAnalyzer::ChebyshevCenter_connComp(*it);
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