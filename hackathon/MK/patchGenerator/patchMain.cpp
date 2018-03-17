#include <iostream>
#include <string>

#include <qlist.h>

#include "basic_surf_objs.h"

#include "NeuronStructExplorer.h"
#include "NeuronStructUtilities.h"
#include "ImgProcessor.h"
#include "ImgManager.h"

using namespace std;

int main()
{
	/*Image4DSimple* tiffPtr = new Image4DSimple;
	tiffPtr->loadImage("C:\\Users\\hsienchik\\Desktop\\477315958_3D.raw_crop.tif");
	long int totalbyteTiff = tiffPtr->getTotalBytes();
	unsigned char* tiffSlice1D = new unsigned char[totalbyteTiff];
	memcpy(tiffSlice1D, tiffPtr->getRawData(), totalbyteTiff);
	int xDim = tiffPtr->getXDim();
	int yDim = tiffPtr->getYDim();
	int zDim = tiffPtr->getZDim();
	V3DLONG patchsz[4];
	patchsz[0] = xDim;
	patchsz[1] = yDim;
	patchsz[2] = 1;
	patchsz[3] = 1;

	string saveRoot = "C:\\Users\\hsienchik\\Desktop\\testSlices\\";
	for (int slicei = 0; slicei < zDim; ++slicei)
	{
		string prefix = "_00";
		unsigned char* slice1D = new unsigned char[xDim * yDim];
		for (int j = 1; j <= yDim; ++j)
		{
			for (int i = 1; i <= xDim; ++i)
			{
				slice1D[xDim * (j - 1) + (i - 1)] = tiffSlice1D[xDim * yDim * slicei + xDim * (j - 1) + (i - 1)];
			}
		}
		
		if (slicei < 10) prefix = prefix + "00";
		else if (slicei < 100) prefix = prefix + "0";
		string sliceName = saveRoot + prefix + to_string(slicei + 1) + ".tif";

		const char* sliceNameC = sliceName.c_str();
		ImgManager::saveimage_wrapper(sliceNameC, slice1D, patchsz, 1);


		if (slice1D) { delete[] slice1D; slice1D = 0; }
	}*/
	
	string caseRoot, tiffRoot, segRoot, sigPatchRoot, bkgPatchRoot;
	string manualSWCName;
	long int dims[3];
	bool somaBkg = false;
	bool FG = false;
	cout << "path parameters:" << endl;
	cin >> caseRoot >> tiffRoot >> segRoot >> sigPatchRoot >> bkgPatchRoot;
	cout << "manual file: " << endl;
	cin >> manualSWCName;
	cout << "image dimension: " << endl;
	cin >> dims[0] >> dims[1] >> dims[2];
	cout << "soma bkg: ";
	cin >> somaBkg;
	cout << "FG mask: ";
	cin >> FG;

	QString manualSWCNameQ = QString::fromStdString(manualSWCName);
	NeuronTree inputTree = readSWC_file(manualSWCNameQ);
	NeuronTree bkgTree, somaBkgTree, signalTree, combinedBkgTree;

	float secondSoma[3];
	secondSoma[0] = inputTree.listNeuron.at(0).x;
	secondSoma[1] = inputTree.listNeuron.at(0).y;
	secondSoma[2] = inputTree.listNeuron.at(0).z;
	bkgNode_Gen(&inputTree, &bkgTree, dims, 50, 15);
	sigNode_Gen(&inputTree, &signalTree, 15, 2);
	string outbkgSWCName = caseRoot + "\\bkg2.swc";
	string outSWCsigName = caseRoot + "\\signal2.swc";

	const char* outbkgSWCNameC = outbkgSWCName.c_str();
	writeSWC_file(outbkgSWCNameC, bkgTree);

	const char* outSWCsigNameC = outSWCsigName.c_str();
	writeSWC_file(outSWCsigNameC, signalTree);

	if (somaBkg == true)
	{
		bkgNode_Gen_somaArea(&inputTree, &somaBkgTree, 800, 800, 100, 0.001, 30);
		for (QList<NeuronSWC>::iterator bkgIt = somaBkgTree.listNeuron.begin(); bkgIt != somaBkgTree.listNeuron.end(); ++bkgIt)
			bkgTree.listNeuron.push_back(*bkgIt);

		string outSomaBkgSWCName = caseRoot + "\\somaAreaBkg.swc";
		const char* outSomaBkgSWCNameC = outSomaBkgSWCName.c_str();
		writeSWC_file(outSomaBkgSWCNameC, somaBkgTree);
	}
	
	V3DLONG patchsz[4];
	patchsz[0] = 127;
	patchsz[1] = 127;
	patchsz[2] = 1;
	patchsz[3] = 1;
	
	int count = 0;
	for (QList<NeuronSWC>::iterator it = signalTree.listNeuron.begin(); it != signalTree.listNeuron.end(); ++it)
	{
		int xCoord = int(it->x);
		int yCoord = int(it->y);
		int zCoord = int(it->z);

		string prefix;
		if (zCoord < 10) prefix = "_0000";
		else if (zCoord < 100) prefix = "_000";
		else if (zCoord < 1000) prefix = "_00";
		string sliceNo = to_string(zCoord);
		string tiffSliceName = tiffRoot + prefix + sliceNo + ".tif";
		string segSliceName = segRoot + prefix + sliceNo + "_Seg.tif";
		const char* segSliceNameC = segSliceName.c_str();
		const char* tiffSliceNameC = tiffSliceName.c_str();

		if (FG == true)
		{
			Image4DSimple* segPtr = new Image4DSimple;
			segPtr->loadImage(segSliceNameC);
			long int totalbyteSeg = segPtr->getTotalBytes();
			unsigned char* segSlice1D = new unsigned char[totalbyteSeg];
			memcpy(segSlice1D, segPtr->getRawData(), totalbyteSeg);
			int xDimSeg = segPtr->getXDim();
			int yDimSeg = segPtr->getYDim();
			int check = int(segSlice1D[xDimSeg * (yDimSeg - yCoord - 1) + (xCoord - 1)]);
			if (check <= 0)
			{
				cout << xCoord << " " << yCoord << " " << zCoord << endl;
				cout << "seg slice name: " << segSliceName << endl;
				cout << "this node is skipped due to not appearing in the foreground mask" << endl;
				segPtr->~Image4DSimple();
				operator delete(segPtr);

				if (segSlice1D) { delete[] segSlice1D; segSlice1D = 0; }
				continue;
			}

			if (segSlice1D) { delete[] segSlice1D; segSlice1D = 0; }
			segPtr->~Image4DSimple();
			operator delete(segPtr);
		}

		Image4DSimple* tiffPtr = new Image4DSimple;
		tiffPtr->loadImage(tiffSliceNameC);
		long int totalbyteTiff = tiffPtr->getTotalBytes();
		unsigned char* tiffSlice1D = new unsigned char[totalbyteTiff];
		memcpy(tiffSlice1D, tiffPtr->getRawData(), totalbyteTiff);
		int xDim = tiffPtr->getXDim();
		int yDim = tiffPtr->getYDim();
		unsigned char sigPatch[127 * 127];
		int xlb, xhb, ylb, yhb;
		if (xCoord - 63 <= 0) xlb = 1; else xlb = xCoord - 63;
		if (xCoord + 63 >= xDim) xhb = xDim; else xhb = xCoord + 63;
		if (yCoord - 63 <= 0) ylb = 1; else ylb = yCoord - 63;
		if (yCoord + 63 >= yDim) yhb = yDim; else yhb = yCoord + 63;
		ImgProcessor::cropImg2D(tiffSlice1D, sigPatch, xlb, xhb, ylb, yhb, xDim, yDim);
		QString outimg_file = QString::fromStdString(sigPatchRoot) + QString("x%1_y%2_z%3.tif").arg(int(it->x)).arg(int(it->y)).arg(int(it->z));
		string filename = outimg_file.toStdString();
		const char* filenameC = filename.c_str();
		cout << filename << endl;
		ImgManager::saveimage_wrapper(filenameC, sigPatch, patchsz, 1);

		tiffPtr->~Image4DSimple();
		operator delete(tiffPtr);
		if (tiffSlice1D) { delete[] tiffSlice1D; tiffSlice1D = 0; }
	}

	count = 0;
	for (QList<NeuronSWC>::iterator it = bkgTree.listNeuron.begin(); it != bkgTree.listNeuron.end(); ++it)
	{
		int xCoord = int(it->x);
		int yCoord = int(it->y);
		int zCoord = int(it->z);

		string prefix;
		if (zCoord < 10) prefix = "_0000";
		else if (zCoord < 100) prefix = "_000";
		else if (zCoord < 1000) prefix = "_00";
		string sliceNo = to_string(zCoord);
		string tiffSliceName = tiffRoot + prefix + sliceNo + ".tif";
		string segSliceName = segRoot + prefix + sliceNo + "_Seg.tif";
		const char* segSliceNameC = segSliceName.c_str();
		const char* tiffSliceNameC = tiffSliceName.c_str();

		Image4DSimple* bkg4DPtr = new Image4DSimple;
		bkg4DPtr->loadImage(tiffSliceNameC);
		long int totalbytesSWC = bkg4DPtr->getTotalBytes();
		unsigned char* slice1D = new unsigned char[totalbytesSWC];
		memcpy(slice1D, bkg4DPtr->getRawData(), totalbytesSWC);
		int xDim = bkg4DPtr->getXDim();
		int yDim = bkg4DPtr->getYDim();
		unsigned char bkgPatch[127 * 127];
		int xlb, xhb, ylb, yhb;
		if (xCoord - 63 <= 0) xlb = 1; else xlb = xCoord - 63;
		if (xCoord + 63 >= xDim) xhb = xDim; else xhb = xCoord + 63;
		if (yCoord - 63 <= 0) ylb = 1; else ylb = yCoord - 63;
		if (yCoord + 63 >= yDim) yhb = yDim; else yhb = yCoord + 63;
		ImgProcessor::cropImg2D(slice1D, bkgPatch, xlb, xhb, ylb, yhb, xDim, yDim);
		QString outimg_file = QString::fromStdString(bkgPatchRoot) + QString("x%1_y%2_z%3.tif").arg(int(it->x)).arg(int(it->y)).arg(int(it->z));
		string filename = outimg_file.toStdString();
		const char* filenameC = filename.c_str();
		cout << filename << endl;
		ImgManager::saveimage_wrapper(filenameC, bkgPatch, patchsz, 1);

		bkg4DPtr->~Image4DSimple();
		operator delete(bkg4DPtr);
		if (slice1D) { delete[] slice1D; slice1D = 0; }
	}

	return 0;
}