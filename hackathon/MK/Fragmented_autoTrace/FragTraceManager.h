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

public slots:
	void imgProcPipe_wholeBlock();

private:
	vector<vector<unsigned char>> imgSlices;
	ImgManager fragTraceImgManager;
};


#endif