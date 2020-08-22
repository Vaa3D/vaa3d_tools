//------------------------------------------------------------------------------
// Copyright (c) 2020 Hsienchi Kuo (Allen Institute)
// All rights reserved.
//------------------------------------------------------------------------------

/*******************************************************************************
*
*  [FragTraceImgProcessor] is responssible for image processing/enhancement in Neuron Assembler app.
*  Depending on the choice of tracing scheme, eg, tracing axon, dendrite, etc., 
*  [FragTraceImgProcessor] provides different solutions to it. 
*
*  [FragTraceImgProcessor] is privately owned by [FragTraceManager] and currently only accepts [integratedDataStructures::registeredImg].
*
********************************************************************************/

#ifndef FRAGTRACEIMGPROCESSOR_H
#define FRAGTRACEIMGPROCESSOR_H

#include "ImgProcessor.h"
#include "ImgAnalyzer.h"
#include "ImgManager.h"

class FragTraceImgProcessor : public ImgAnalyzer
{
	friend class FragTraceTester;

public:
	QString blobTreeSavePathQ;

	/* ----------------- Image Enhancement ----------------- */
	void gammaCorrect(const string inputRegImgName, const string outputRegImgName, map<string, registeredImg>& imgDatabase) const;
	void simpleThre(const string inputRegImgName, const string outputRegImgName, map<string, registeredImg>& imgDatabase, const int cutoffIntensity) const;
	void adaThresholding(const string inputRegImgName, const string outputRegImgName, map<string, registeredImg>& imgDatabase, const int stepSize, const int sampleRate) const;
	void histThreImg3D(const string inputRegImgName, const string outputRegImgName, map<string, registeredImg>& imgDatabase, const float stdFold) const;
	/* ------------- END of [Image Enhancement] ------------ */

	/* ----------------- Image Segmentation ----------------- */
	NeuronTree mask2swc(const string inputImgName, map<string, registeredImg>& imgDatabase, vector<connectedComponent>& signalBlobs);
	/* ------------- END of [Image Segmentation] -------------*/
};


#endif