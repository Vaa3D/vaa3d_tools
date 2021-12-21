//FL_cellSegmentation3D.h
//by Fuhui Long
//20080310
//20080825: add a FL_SEG_LABEL_TYPE

#ifndef __CELL_SEGMENTATION3D__
#define __CELL_SEGMENTATION3D__

#include "../basic_c_fun/v3d_basicdatatype.h"

//typedef unsigned short int CellSegLabelType;
//class Vol3DSimple <unsigned char>;

//class segParameter // for watershed segmentation
//{
//public:
//	char *infilename, *outfilename; // name of input and output files
//	int channelNo; // channel to be segmented
////	int backgndLevel; // background level
//	int	medianFiltWid;
//	int	gaussinFiltWid;	
//	float gaussinFiltSigma;
//	int adaptThreWid;
//	int adaptThreStp;
//	int segMethod; // segmentation method
//	int adpatThreMethod; // thresholding method
//	
//	segParameter() //initialize parameters using default values
//	{
//		infilename = NULL;
//		outfilename = NULL;
//		channelNo = 2;
////		backgndLevel = 15;
//		medianFiltWid = 2;
//		gaussinFiltWid = 1;
//		gaussinFiltSigma = 1;
//		adaptThreWid = 10;
//		adaptThreStp = 5;
//		segMethod = 0;
//		adpatThreMethod = 0;
//	}
//	
//	segParameter(const segParameter & segPara) //initialize parameters using input values
//	{
//
//		infilename = segPara.infilename;
//		outfilename = segPara.outfilename;
//		channelNo = segPara.channelNo;
////		backgndLevel = segPara.backgndLevel;
//		medianFiltWid = segPara.medianFiltWid;
//		gaussinFiltWid = segPara.gaussinFiltWid;
//		gaussinFiltSigma= segPara.gaussinFiltSigma;		
//		adaptThreWid = segPara.adaptThreWid;
//		adaptThreStp = segPara.adaptThreStp;
//		segMethod = segPara.segMethod;
//		adpatThreMethod = segPara.adpatThreMethod;
//	}
//	
//};

#include "seg_parameter.h"

bool FL_cellseg(Vol3DSimple <unsigned char> *img3d, Vol3DSimple <unsigned short int> *outimg3d, const segParameter & mypara);

#endif

