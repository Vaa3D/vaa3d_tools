#include <iostream>
#include <iterator>

#include <boost\filesystem.hpp>

#include <qstring.h>
#include <qstringlist.h>

#include "ImgProcessor.h"
#include "NeuronROIProfiler.h"

using namespace std;
using namespace boost::filesystem;

/*NeuronROIProfiler::NeuronROIProfiler(QString neuronFileName, string wholeImgName) : NeuronStructExplorer(neuronFileName), ImgManager(wholeImgName)
{
	this->img1DfromManager = imgData1D;
	this->treeFromNeuStructExplorer_Ptr = &singleTree;
	this->img4DFromManager_Ptr = wholeImg4DPtr;
	imgSz[0] = dims[0]; imgSz[1] = dims[1]; imgSz[2] = dims[2]; imgSz[3] = dims[3];
}*/

bool NeuronROIProfiler::nodeROIMIP()
{
	if (!this->cropped1D)
	{
		cerr << "No valid image cube existing in the class. Return." << endl;
		return false;
	}
	this->MIPSz[0] = croppedSz[0];
	this->MIPSz[1] = croppedSz[1];
	this->MIPSz[2] = 1;
	this->MIPSz[3] = croppedSz[3];

	long int ROISz = this->croppedSz[0] * this->croppedSz[1];
	this->MIP1D = new unsigned char[ROISz];
	ImgProcessor* imgProcPtr = new ImgProcessor;
	imgProcPtr->maxIPStack(this->cropped1D, this->MIP1D, this->croppedSz[0], this->croppedSz[1], this->croppedSz[2]);

	return true;
}

void NeuronROIProfiler::createAugmentedROIList_binMaskBased(string swcMasksPath, string FGpath)
{
	// -- This method generate nodeROIs of every single voxel from tubular SWC binary masks.

	int sliceCount = 0;
	for (directory_iterator itr(swcMasksPath); itr != directory_iterator(); ++itr)
	{
		string maskFileName = itr->path().filename().string();
		string tale = maskFileName.substr(maskFileName.length() - 3, 3);
		deque<nodeROI> newQueu;
		if (tale.compare("tif") == 0)
		{
			++sliceCount;
			string maskFileFullName = swcMasksPath + "\\" + maskFileName;
			Image4DSimple* swcMaskPtr = new Image4DSimple;
			swcMaskPtr->loadImage(maskFileFullName.c_str());
			long int totalbytesSWC = swcMaskPtr->getTotalBytes();
			unsigned char* swcMask1D = new unsigned char[totalbytesSWC];
			memcpy(swcMask1D, swcMaskPtr->getRawData(), totalbytesSWC);
			long int xDim = swcMaskPtr->getXDim();
			long int yDim = swcMaskPtr->getYDim();

			QString maskFileNameQ = QString::fromStdString(maskFileName);
			QStringList nameParse1 = maskFileNameQ.split(".");
			QStringList nameParse2 = nameParse1[0].split("_");
			int zSliceNum = nameParse2[1].toInt();
			cout << "[mask slice No:" << zSliceNum << " count: " << sliceCount << "] ";

			int nodeROIcount = 0;
			if (FGpath.compare("none") != 0) // If FGPath is specified, nodeROI will be selected under foreground masks's constraint.
			{
				string prefix;
				if (zSliceNum < 10) prefix = "_0000";
				else if (zSliceNum < 100) prefix = "_000";
				else if (zSliceNum < 1000) prefix = "_00";
				string FGsliceFullName = FGpath + "\\" + prefix + to_string(zSliceNum) + "_Seg.tif";
				const char* FGsliceFullNameC = FGsliceFullName.c_str();

				Image4DSimple* fgMaskPtr = new Image4DSimple;
				unsigned char* FGMIP1D = nullptr;
				fgMaskPtr->loadImage(FGsliceFullNameC);
				long int totalbytesFG = fgMaskPtr->getTotalBytes();
				FGMIP1D = new unsigned char[totalbytesFG];
				memcpy(FGMIP1D, fgMaskPtr->getRawData(), totalbytesFG);

				for (long int yi = 1; yi <= yDim; ++yi) // Coordinates all start from 1.
				{
					for (long int xi = 1; xi <= xDim; ++xi)
					{
						if (swcMask1D[xDim * (yi - 1) + (xi - 1)] > 0 && FGMIP1D[xDim * (yi - 1) + (xi - 1)] > 0)
						{
							++nodeROIcount;
							nodeROI newDot;
							newDot.x = xi;
							newDot.y = yi;
							newDot.z = zSliceNum;
							newQueu.push_back(newDot);
						}
					}
				}
			}
			else 
			{
				for (long int yi = 1; yi <= yDim; ++yi) // Coordinates all start from 1.
				{
					for (long int xi = 1; xi <= xDim; ++xi)
					{
						if (swcMask1D[xDim * (yi - 1) + (xi - 1)] > 0)
						{
							++nodeROIcount;
							nodeROI newDot;
							newDot.x = xi;
							newDot.y = yi;
							newDot.z = zSliceNum;
							newQueu.push_back(newDot);
						}
					}
				}
			}
			cout << "nodeROIs in the current queu:" << nodeROIcount << endl;
			this->allROIs_binMask.push_back(newQueu);
			newQueu.clear();
			delete swcMaskPtr;
		}
	}	
	
	cout << endl;
}

void NeuronROIProfiler::createAugmentedROIList_nodeBased(int rangeAllowance, int sampleInterval)
{
	if (treeFromNeuStructExplorer_Ptr->listNeuron.size() <= 0)
	{
		cerr << "No existing neuron tree found. Return." << endl;
		return;
	}

	for (QList<NeuronSWC>::iterator nodeIt = treeFromNeuStructExplorer_Ptr->listNeuron.begin();
		nodeIt != treeFromNeuStructExplorer_Ptr->listNeuron.end(); ++nodeIt)
	{
		if (nodeIt->type == 2)
		{
			nodeROI center;
			center.isNode = true;
			center.ID = nodeIt->n;
			center.nodeX = nodeIt->x; center.nodeY = nodeIt->y; center.nodeZ = nodeIt->z;
			center.x = int(nodeIt->x); center.y = int(nodeIt->y); center.z = int(nodeIt->z);
			center.type = 2;
			center.radius = nodeIt->radius;
			center.interval = sampleInterval;
			this->axonROIs.push_back(center);
			this->allROIs.push_back(center);
			if (center.interval == 0) continue;

			nodeROI centerxUp, centerxDn, centeryUp, centeryDn, centerzUp, centerzDn;
			centerxUp.isNode = false; centerxDn.isNode = false; centeryUp.isNode = false; centeryDn.isNode = false; centerzUp.isNode = false; centerzDn.isNode = false;
			centerxUp.ID = nodeIt->n; centerxDn.ID = nodeIt->n; centeryUp.ID = nodeIt->n; centeryDn.ID = nodeIt->n; centerzUp.ID = nodeIt->n; centerzDn.ID = nodeIt->n;
			centerxUp.x = int(center.nodeX + sampleInterval); centerxUp.y = center.y; centerxUp.z = center.z;
			centerxDn.x = int(center.nodeX - sampleInterval); centerxDn.y = center.y; centerxDn.z = center.z;
			centeryUp.x = center.x; centeryUp.y = int(center.nodeY + sampleInterval); centeryUp.z = center.z;
			centeryDn.x = center.x; centeryDn.y = int(center.nodeY - sampleInterval); centeryDn.z = center.z;
			centerzUp.x = center.x; centerzUp.y = center.y; centerzUp.z = int(center.nodeZ + sampleInterval);
			centerzDn.x = center.x; centerzDn.y = center.y; centerzDn.z = int(center.nodeZ - sampleInterval);
			centerxUp.type = 2; centerxDn.type = 2; centeryUp.type = 2; centeryDn.type = 2; centerzUp.type = 2; centerzDn.type = 2;

			this->axonROIs.push_back(centerxUp); this->axonROIs.push_back(centerxDn);
			this->axonROIs.push_back(centeryUp); this->axonROIs.push_back(centeryDn);
			this->axonROIs.push_back(centerzUp); this->axonROIs.push_back(centerzDn);
			this->allROIs.push_back(centerxUp); this->allROIs.push_back(centerxDn);
			this->allROIs.push_back(centeryUp); this->allROIs.push_back(centeryDn);
			this->allROIs.push_back(centerzUp); this->allROIs.push_back(centerzDn);
		}
		else if (nodeIt->type == 3)
		{
			nodeROI center;
			center.isNode = true;
			center.ID = nodeIt->n;
			center.nodeX = nodeIt->x; center.nodeY = nodeIt->y; center.nodeZ = nodeIt->z;
			center.x = nodeIt->x; center.y = nodeIt->y; center.z = nodeIt->z;
			center.type = 3;
			center.radius = nodeIt->radius;
			center.interval = sampleInterval;
			this->dendriteROIs.push_back(center);
			this->allROIs.push_back(center);
			if (center.interval == 0) continue;

			nodeROI centerxUp, centerxDn, centeryUp, centeryDn, centerzUp, centerzDn;
			centerxUp.isNode = false; centerxDn.isNode = false; centeryUp.isNode = false; centeryDn.isNode = false; centerzUp.isNode = false; centerzDn.isNode = false;
			centerxUp.ID = nodeIt->n; centerxDn.ID = nodeIt->n; centeryUp.ID = nodeIt->n; centeryDn.ID = nodeIt->n; centerzUp.ID = nodeIt->n; centerzDn.ID = nodeIt->n;
			centerxUp.x = int(center.nodeX + sampleInterval); centerxUp.y = center.y; centerxUp.z = center.z;
			centerxDn.x = int(center.nodeX - sampleInterval); centerxDn.y = center.y; centerxDn.z = center.z;
			centeryUp.x = center.x; centeryUp.y = int(center.nodeY + sampleInterval); centeryUp.z = center.z;
			centeryDn.x = center.x; centeryDn.y = int(center.nodeY - sampleInterval); centeryDn.z = center.z;
			centerzUp.x = center.x; centerzUp.y = center.y; centerzUp.z = int(center.nodeZ + sampleInterval);
			centerzDn.x = center.x; centerzDn.y = center.y; centerzDn.z = int(center.nodeZ - sampleInterval);
			centerxUp.type = 3; centerxDn.type = 3; centeryUp.type = 3; centeryDn.type = 3; centerzUp.type = 3; centerzDn.type = 3;
			
			this->dendriteROIs.push_back(centerxUp); this->dendriteROIs.push_back(centerxDn);
			this->dendriteROIs.push_back(centeryUp); this->dendriteROIs.push_back(centeryDn);
			this->dendriteROIs.push_back(centerzUp); this->dendriteROIs.push_back(centerzDn);
			this->allROIs.push_back(centerxUp); this->allROIs.push_back(centerxDn);
			this->allROIs.push_back(centeryUp); this->allROIs.push_back(centeryDn);
			this->allROIs.push_back(centerzUp); this->allROIs.push_back(centerzDn);
		}
		else
		{
			nodeROI center;
			center.isNode = true;
			center.ID = nodeIt->n;
			center.nodeX = nodeIt->x; center.nodeY = nodeIt->y; center.nodeZ = nodeIt->z;
			center.x = nodeIt->x; center.y = nodeIt->y; center.z = nodeIt->z;
			center.type = nodeIt->type;
			center.radius = nodeIt->radius;
			center.interval = sampleInterval;
			this->otherROIs.push_back(center);
			this->allROIs.push_back(center);
			if (center.interval == 0) continue;

			nodeROI centerxUp, centerxDn, centeryUp, centeryDn, centerzUp, centerzDn;
			centerxUp.isNode = false; centerxDn.isNode = false; centeryUp.isNode = false; centeryDn.isNode = false; centerzUp.isNode = false; centerzDn.isNode = false;
			centerxUp.ID = nodeIt->n; centerxDn.ID = nodeIt->n; centeryUp.ID = nodeIt->n; centeryDn.ID = nodeIt->n; centerzUp.ID = nodeIt->n; centerzDn.ID = nodeIt->n;
			centerxUp.x = int(center.nodeX + sampleInterval); centerxUp.y = center.y; centerxUp.z = center.z;
			centerxDn.x = int(center.nodeX - sampleInterval); centerxDn.y = center.y; centerxDn.z = center.z;
			centeryUp.x = center.x; centeryUp.y = int(center.nodeY + sampleInterval); centeryUp.z = center.z;
			centeryDn.x = center.x; centeryDn.y = int(center.nodeY - sampleInterval); centeryDn.z = center.z;
			centerzUp.x = center.x; centerzUp.y = center.y; centerzUp.z = int(center.nodeZ + sampleInterval);
			centerzDn.x = center.x; centerzDn.y = center.y; centerzDn.z = int(center.nodeZ - sampleInterval);
			centerxUp.type = nodeIt->type; centerxDn.type = nodeIt->type; centeryUp.type = nodeIt->type; centeryDn.type = nodeIt->type; centerzUp.type = nodeIt->type; centerzDn.type = nodeIt->type;

			this->otherROIs.push_back(centerxUp); this->otherROIs.push_back(centerxDn);
			this->otherROIs.push_back(centeryUp); this->otherROIs.push_back(centeryDn);
			this->otherROIs.push_back(centerzUp); this->otherROIs.push_back(centerzDn);
			this->allROIs.push_back(centerxUp); this->allROIs.push_back(centerxDn);
			this->allROIs.push_back(centeryUp); this->allROIs.push_back(centeryDn);
			this->allROIs.push_back(centerzUp); this->allROIs.push_back(centerzDn);
		}
	}
}

void NeuronROIProfiler::createAugmentedBkgList(int rangeAllowance, int sampleInterval)
{
	if (treeFromNeuStructExplorer_Ptr->listNeuron.size() <= 0)
	{
		cerr << "No existing neuron tree found. Return." << endl;
		return;
	}

	for (QList<NeuronSWC>::iterator nodeIt = treeFromNeuStructExplorer_Ptr->listNeuron.begin();
		nodeIt != treeFromNeuStructExplorer_Ptr->listNeuron.end(); ++nodeIt)
	{
		nodeROI center;
		center.isNode = true;
		center.ID = nodeIt->n;
		center.nodeX = nodeIt->x; center.nodeY = nodeIt->y; center.nodeZ = nodeIt->z;
		center.x = int(nodeIt->x); center.y = int(nodeIt->y); center.z = int(nodeIt->z);
		center.interval = sampleInterval;
		this->allBkgs.push_back(center);
		if (sampleInterval == 0) continue;

		nodeROI centerxUp, centerxDn, centeryUp, centeryDn, centerzUp, centerzDn;
		centerxUp.isNode = false; centerxDn.isNode = false; centeryUp.isNode = false; centeryDn.isNode = false; centerzUp.isNode = false; centerzDn.isNode = false;
		centerxUp.ID = nodeIt->n; centerxDn.ID = nodeIt->n; centeryUp.ID = nodeIt->n; centeryDn.ID = nodeIt->n; centerzUp.ID = nodeIt->n; centerzDn.ID = nodeIt->n;
		centerxUp.x = int(center.nodeX + sampleInterval); centerxUp.y = center.y; centerxUp.z = center.z;
		centerxDn.x = int(center.nodeX - sampleInterval); centerxDn.y = center.y; centerxDn.z = center.z;
		centeryUp.x = center.x; centeryUp.y = int(center.nodeY + sampleInterval); centeryUp.z = center.z;
		centeryDn.x = center.x; centeryDn.y = int(center.nodeY - sampleInterval); centeryDn.z = center.z;
		centerzUp.x = center.x; centerzUp.y = center.y; centerzUp.z = int(center.nodeZ + sampleInterval);
		centerzDn.x = center.x; centerzDn.y = center.y; centerzDn.z = int(center.nodeZ - sampleInterval);

		this->allBkgs.push_back(centerxUp); this->allBkgs.push_back(centerxDn);
		this->allBkgs.push_back(centeryUp); this->allBkgs.push_back(centeryDn);
		this->allBkgs.push_back(centerzUp); this->allBkgs.push_back(centerzDn);
	}
}
