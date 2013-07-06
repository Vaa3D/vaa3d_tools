//last update: 090426, by Hanchuan Peng, change NULL to 0 for infilename/outfilename to fix compiling error


#ifndef __CELL_SEG_PARAMETER_H__
#define __CELL_SEG_PARAMETER_H__

class segParameter // for watershed segmentation
{
public:
	char *infilename, *outfilename; // name of input and output files
	int channelNo; // channel to be segmented
//	int backgndLevel; // background level
	int	medianFiltWid;
	int	gaussinFiltWid;	
	float gaussinFiltSigma;
	int adaptThreWid;
	int adaptThreStp;
	int segMethod; // segmentation method
	int adpatThreMethod; // thresholding method
	
	segParameter() //initialize parameters using default values
	{
		infilename = 0;
		outfilename = 0;
		channelNo = 2;
//		backgndLevel = 15;
		medianFiltWid = 2;
		gaussinFiltWid = 1;
		gaussinFiltSigma = 1;
		adaptThreWid = 10;
		adaptThreStp = 5;
		segMethod = 0;
		adpatThreMethod = 2;
	}
	
	segParameter(const segParameter & segPara) //initialize parameters using input values
	{

		infilename = segPara.infilename;
		outfilename = segPara.outfilename;
		channelNo = segPara.channelNo;
//		backgndLevel = segPara.backgndLevel;
		medianFiltWid = segPara.medianFiltWid;
		gaussinFiltWid = segPara.gaussinFiltWid;
		gaussinFiltSigma= segPara.gaussinFiltSigma;		
		adaptThreWid = segPara.adaptThreWid;
		adaptThreStp = segPara.adaptThreStp;
		segMethod = segPara.segMethod;
		adpatThreMethod = segPara.adpatThreMethod;
	}
	
};

#endif


