#ifndef NEURONROIPROFILER_H
#define NEURONROIPROFILER_H

#include <deque>

#include "ImgManager.h"
#include "NeuronStructExplorer.h"

using namespace std;

enum augmentShift { axesX, axesY, axesZ };
enum rotateParam { MIP, mIP, plain };

struct nodeROI
{
	bool isNode;
	long int ID;
	float nodeX, nodeY, nodeZ;
	int type;
	float radius;
	
	int range, interval;
	int x, y, z;
};

class NeuronROIProfiler : public NeuronStructExplorer, ImgManager
{
public:
	/********* Constructors ********/
	NeuronROIProfiler() {};
	/*NeuronROIProfiler(QString neuronFileName) : NeuronStructExplorer(neuronFileName) 
		{ this->treeFromNeuStructExplorer_Ptr = &singleTree; }

	NeuronROIProfiler(QString neuronFileName, string wholeImgName);*/
	/*******************************/

	// ------- Variables inherited from parent class ImgManager ------- //
	Image4DSimple* img4DFromManager_Ptr;
	unsigned char* img1DfromManager;
	long int imgSz[4];
	// ---------------------------------------------------------------- //

	// ------- Variables inherited from parent class NeuronStructExplorer ------- //
	NeuronTree* treeFromNeuStructExplorer_Ptr;
	// -------------------------------------------------------------------------- //

	// --------- create VOIs/ROIs based on neuron structure file (THESE METHODS ARE DEPRECATED) --------- //
	long long xLength, yLength, zLength; // VOI/ROI size
	unsigned char* cropped1D; // The largest VOI size allowed is 256*256*256.
	long long croppedSz[4];
	template<class T> bool cropNodeVOI(T x, T y, T z);
	
	unsigned char* MIP1D;
	long int MIPSz[4];
	bool nodeROIMIP();
	
	unsigned char* curr1D;
	unsigned char* temp1D;
	long int currSz[4];

	vector<augmentShift> augShiftList;           // decide in which direction to shift for augmenting
	deque<nodeROI> axonROIs;                     // simplified axon node info for generating ROIs
	deque<nodeROI> dendriteROIs;                 // simplified dendrite node info for generating ROIs
	deque<nodeROI> otherROIs;                    // simplified other type of node info for generating ROIs
	deque<nodeROI> allROIs;
	deque<nodeROI> allBkgs;
	void createAugmentedROIList_nodeBased(int rangeAllowance = 0, int sampleInterval = 0);		   // generate ROIs from nodeROI lists (deque<nodeROI>)
	void createAugmentedBkgList(int rangeAllowance = 0, int sampleInterval = 0);				   // generate ROIs from bkgROI lists (deque<nodeBkg>)

	deque<deque<nodeROI>> allROIs_binMask;			
	void createAugmentedROIList_binMaskBased(string swcMasksPath, string foregroundMask = "none"); // generate ROIs from SWC binary masks (deque<nodeROI>)
	// ---------------------------------------------------------------------------------------------------//
};

template<class T> bool NeuronROIProfiler::cropNodeVOI(T x, T y, T z)
{
	if (!img4DFromProfiler_Ptr || !img4DFromProfiler_Ptr->valid())
	{
		cerr << "Invalid Image4DSimple. Return." << endl;
		return false;
	}

	long long xDim = wholeImg4DPtr->getXDim();
	long long yDim = wholeImg4DPtr->getYDim();
	long long zDim = wholeImg4DPtr->getZDim();

	long long xlb = long long(x) - this->xLength / 2;
	long long xhb = long long(x) + this->xLength / 2;
	long long ylb = long long(y) - this->yLength / 2;
	long long yhb = long long(y) + this->yLength / 2;
	long long zlb = long long(z) - this->zLength / 2;
	long long zhb = long long(z) + this->zLength / 2;

	if (xlb < 1) xlb = 1;
	if (xhb > xDim) xhb = xDim;
	if (ylb < 1) ylb = 1;
	if (yhb > yDim) yhb = yDim;
	if (zlb < 1) zlb = 1;
	if (zhb > zDim) zhb = zDim;

	long long VOIx = xhb - xlb + 1;
	long long VOIy = yhb - ylb + 1;
	long long VOIz = zhb - zlb + 1;
	this->croppedSz[0] = VOIx;
	this->croppedSz[1] = VOIy;
	this->croppedSz[2] = VOIz;
	this->croppedSz[3] = wholeImg4DPtr->getCDim();
	long long VOIsz = VOIx * VOIy * VOIz;
	this->cropped1D = new unsigned char[VOIsz];
	cout << "voi size: " << VOIsz << endl;
	ImgProcessor::cropStack(img1DfromProfiler, this->cropped1D, xlb, xhb, ylb, yhb, zlb, zhb, xDim, yDim, zDim);

	return true;
}

#endif