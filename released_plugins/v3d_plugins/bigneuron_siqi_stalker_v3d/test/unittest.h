#pragma once
#ifndef __PRESSUNITTEST__
#define __PRESSUNITTEST__

// Test "utils/matmath.h"
#include <limits> 
#include "lib/ImageOperation.h"
#include "stackutil.h"
#define EPSILON 5E-2; //Since we use floats, do not be too harsh
using namespace std;
typedef vector<float> vectype;

void TestMatMath();

void TestPressureSampler(ImagePointer OriginalImage, GradientImagePointer GVF, LabelImagePointer wallimg,
 PointList3D seeds, vectype * xpfinal, vectype * ypfinal, vectype * zpfinal, vectype * pn, vectype * rfinal, vectype * sn);

void Trace(ImagePointer OriginalImage, GradientImagePointer GVF, LabelImagePointer wallimg,
 PointList3D seeds, vectype * xpfinal, vectype * ypfinal, vectype * zpfinal, vectype * pn, vectype * rfinal, vectype * sn);
//void TestRadius(unsigned char * inimg1d, V3DLONG * sz);

#endif