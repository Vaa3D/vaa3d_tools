//FL_main_brainseg.h
//by Fuhui Long
//20081024

#ifndef __FL_MAIN_BRAINSEG_H__
#define __FL_MAIN_BRAINSEG_H__

//typedef unsigned short int CellSegLabelType;

class levelset_segParameter // for watershed segmentation
{
public:
	char *infilename, *segfilename, *outfilename; // name of input and output files
	int channelNo; // channel to be segmented
	int dimension;
	int regionnum; // number of regions to be segmented
	int *regions;
	int maxregionnum; //maximum number of regions in the template
	int *regionNoModel; // regions that should not apply model, such as optical lobes
	int regionNoModelNum; // number of regions that should not apply model
	float lamda; //coefficient of the weighted length term Lg(\phi)
	float alf; //coefficient of the weighted area term Ag(\phi);
	float epsilon; // the papramater in the definition of smoothed Dirac function
	float delt; // time step
	float mu; //coefficient of the internal (penalizing) energy term P(\phi)	
	float gama ;	
//	int method;

	levelset_segParameter() //initialize parameters using default values
	{
		infilename = NULL;
		outfilename = NULL;
		segfilename = NULL;
		channelNo = 2;
		dimension = 2;
		regionnum = 62;
		maxregionnum = 62;
	
		regions = new int [regionnum];
		for (int i=0; i<regionnum; i++)
			regions[i] = i+1;
		
		regionNoModel = new int [2]; // should revise
		regionNoModel[0] = 1;
		regionNoModel[1] = 34;
		
		regionNoModelNum = 2;
		
		lamda = 0.5; //coefficient of the weighted length term Lg(\phi)
		alf =1; //coefficient of the weighted area term Ag(\phi);
		epsilon = 1.5; // the papramater in the definition of smoothed Dirac function
		delt = 5; // time step
		mu = 0.1/delt; //coefficient of the internal (penalizing) energy term P(\phi)	
		gama = 0.001;	
//		method = 1;

	}
	
	levelset_segParameter(const levelset_segParameter & segPara) //initialize parameters using input values
	{

		infilename = segPara.infilename;
		segfilename = segPara.segfilename;
		outfilename = segPara.outfilename;
		channelNo = segPara.channelNo;
		dimension = segPara.dimension;
		regionnum = segPara.regionnum;
		maxregionnum = 62;
		
		regions = new int [regionnum];				
		for (int i=0; i<regionnum; i++)
			regions[i] = segPara.regions[i];
			
		regionNoModel = new int [2]; // should revise
		regionNoModel[0] = 1;
		regionNoModel[1] = 34;			
			
		regionNoModelNum = 2;
		
		lamda = segPara.lamda; //coefficient of the weighted length term Lg(\phi)
		alf = segPara.alf; //coefficient of the weighted area term Ag(\phi);
		epsilon = segPara.epsilon; // the papramater in the definition of smoothed Dirac function
		delt = segPara.delt; // time step
		mu = 0.1/segPara.delt; //coefficient of the internal (penalizing) energy term P(\phi)	
		gama = segPara.gama;	
//		method = segPara.method;
		
	}
	
};

bool brainSeg2D(Vol3DSimple <unsigned char> *rawimg3d, Vol3DSimple <unsigned short int> *segimg3d, Vol3DSimple <unsigned short int> *outimg3d, const levelset_segParameter & mypara);

#endif

