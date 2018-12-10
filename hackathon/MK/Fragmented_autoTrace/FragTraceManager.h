#ifndef _FRAGTRACEMANAGER_H_
#define _FRAGTRACEMANAGER_H_

#include "ImgManager.h"
#include "ImgAnalyzer.h"
#include "ImgProcessor.h"

class FragTraceManager: public QObject
{
	Q_OBJECT

public:
	FragTraceManager() {};
	FragTraceManager(const Image4DSimple* inputImg4DSimplePtr, bool slices = true);

	bool ada;
	string adaImgName;
	int simpleAdaStepsize;
	int simpleAdaRate;
	bool saveAdaResults;
	QString simpleAdaSaveDirQ;

	bool histThre;
	string histThreImgName;
	int stdFold;
	bool saveHistThreResults;
	QString histThreSaveDirQ;

public slots:
	void imgProcPipe_wholeBlock();

private:
	vector<vector<unsigned char>> imgSlices;
	ImgManager fragTraceImgManager;

	void adaThre(const string inputRegImgName, V3DLONG dims[], const string outputRegImgName);
	void histThreImg(const string inputRegImgName, V3DLONG dims[], const string outputRegImgName);

	inline void saveIntermediateResult(string imgName, QString saveRootQ, V3DLONG dims[]);
};

inline void FragTraceManager::saveIntermediateResult(string imgName, QString saveRootQ, V3DLONG dims[])
{
	if (this->fragTraceImgManager.imgDatabase.find(imgName) == this->fragTraceImgManager.imgDatabase.end())
	{
		cerr << "The specified image doesn't exist. Do nothing and return.";
		return;
	}

	qint64 timeStamp_qint64 = QDateTime::currentMSecsSinceEpoch();
	QString timeStampQ = QString::number(timeStamp_qint64);
	saveRootQ = saveRootQ + "\\" + timeStampQ;
	if (!QDir(saveRootQ).exists()) QDir().mkpath(saveRootQ);

	for (map<string, myImg1DPtr>::iterator it = this->fragTraceImgManager.imgDatabase.at(imgName).slicePtrs.begin();
		it != this->fragTraceImgManager.imgDatabase.at(imgName).slicePtrs.end(); ++it)
	{
		string saveFullPath = saveRootQ.toStdString() + "\\" + it->first;
		const char* saveFullPathC = saveFullPath.c_str();
		this->fragTraceImgManager.saveimage_wrapper(saveFullPathC, it->second.get(), dims, 1);
	}
}

#endif