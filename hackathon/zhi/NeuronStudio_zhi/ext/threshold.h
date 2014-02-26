/*******************************************************************
 threshold.h : Routine for computing adaptive threshold of an image
               based on the isodata algorithm by Ridler and Calvard.
               With improvements to measure confidence in CLSM data.

 Copyright (c) Alfredo Rodriguez 2005
 CNIC, Mount Sinai School of Medicine

********************************************************************/
#ifndef THRESHOLD_H
#define THRESHOLD_H

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsmath.h>

NS_DECLS_BEGIN

typedef struct
	{
	nsfloat threshold; // computed threshold value
	nsfloat average;   // sample mean
	nsfloat contrast;  // separation of threshold from background and object
	nsfloat density;   // fraction of data values above threshold
	nsfloat confidence;// low values indicate non-bimodal sample
	nsfloat dual_threshold;
	}
	ThresholdStruct;


NS_IMPEXP nsfloat compute_confidence( const nsfloat *value, nssize width, nssize height, nssize length, nsfloat threshold );


NS_IMPEXP void ComputeThreshold
	(
	nsfloat          *values,
	nsint             count,
	ThresholdStruct  *s,
	nsdouble          dynamic_range,
	nsboolean         flatten
	);

NS_IMPEXP void ComputeDualThreshold
	(
	nsfloat          *values,
	nsint             count,
	ThresholdStruct  *s,
	nsdouble          dynamic_range
	);

NS_DECLS_END

#endif
