//gradient diffusion field 
// ref:
//revised by Fuhui Long
//20080902

#ifndef __GVF_SEGMENTATION3D__
#define __GVF_SEGMENTATION3D__

#include "../basic_c_fun/v3d_basicdatatype.h"

class gvfsegPara // for gradient diffusion field
{
public:
	char *infilename, *outfilename; // name of input and output files
	int channelNo; // channel to be segmented
	int diffusionIteration;
	int fusionThreshold;
	int minRegion;
	float sigma;
	
	gvfsegPara() //initialize parameters using default values
	{
		infilename = NULL;
		outfilename = NULL;
		channelNo = 0;
		diffusionIteration = 15;
		fusionThreshold = 3;
		minRegion = 50;		
		sigma = 1.0;

	}
	
	gvfsegPara(const gvfsegPara & segPara) //initialize parameters using input values
	{

		infilename = segPara.infilename;
		outfilename = segPara.outfilename;
		channelNo = segPara.channelNo;
		diffusionIteration = segPara.diffusionIteration;
		fusionThreshold = segPara.fusionThreshold;
		minRegion = segPara.minRegion;		
		sigma = segPara.sigma;

	}
	
};

bool gvfCellSeg(Vol3DSimple <unsigned char> *img3d, Vol3DSimple <unsigned short int> *outimg3d, const gvfsegPara & mypara);

#endif

