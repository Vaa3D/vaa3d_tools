#ifndef FRAGTRACEIMGPROCESSOR_H
#define FRAGTRACEIMGPROCESSOR_H

#include "ImgProcessor.h"
#include "ImgAnalyzer.h"
#include "ImgManager.h"

class FragTraceImgProcessor
{
	friend class FragTraceTester;

public:
	void gammaCorrect(const string inputRegImgName, const string outputRegImgName, map<string, registeredImg>& imgDatabase) const;
	void adaThresholding(const string inputRegImgName, const string outputRegImgName, map<string, registeredImg>& imgDatabase, const int stepSize, const int sampleRate) const;
};


#endif