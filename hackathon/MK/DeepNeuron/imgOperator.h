#ifndef IMGOPERATOR_H
#define IMGOPERATOR_H

#include <vector>

#include "v3d_interface.h"

using namespace std;

class imgOperator
{
public:
	imgOperator(Image4DSimple*);
	unsigned char* ImgPtr;
	V3DLONG imgX;
	V3DLONG imgY;
	V3DLONG imgZ;
	V3DLONG channel;

	void cropStack(unsigned char InputImagePtr[], unsigned char OutputImagePtr[],
		int xlb, int xhb, int ylb, int yhb, int zlb, int zhb, int imgX, int imgY, int imgZ);

	void maxIPStack(unsigned char inputVOIPtr[], unsigned char OutputImage2DPtr[],
		int xlb, int xhb, int ylb, int yhb, int zlb, int zhb);
};

#endif