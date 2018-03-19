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
	tiffPtr->loadImage("C:\\Users\\King Mars\\Desktop\\477315958\\477315958_crop1.tif");
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

	string saveRoot = "C:\\Users\\King Mars\\Desktop\\477315958\\testSlices_300MB";
	ImgManager::imgStackSlicer(tiffSlice1D, xDim, yDim, zDim, saveRoot);*/
	

	bool useDefault = false;
	string caseRoot, tiffRoot, segRoot, sigPatchRoot, bkgPatchRoot;
	string manualSWCName;
	long int dims[3];
	bool somaBkg = false;
	bool FG = false;
	int halfSideLength;

	cout << "using default path parameters: ";
	cin >> useDefault;

	if (useDefault == true)
	{
		caseRoot = "C:\\Users\\King Mars\\Desktop\\477315958";
		tiffRoot = "C:\\Users\\King Mars\\Desktop\\477315958\\testSlices_down2";
		segRoot = "";
		sigPatchRoot = "C:\\Users\\King Mars\\Desktop\\477315958\\sigPatches_down2";
		bkgPatchRoot = "C:\\Users\\King Mars\\Desktop\\477315958\\bkgPatches_down2";
		manualSWCName = "C:\\Users\\King Mars\\Desktop\\477315958\\test_down2.swc";
		dims[0] = 258;
		dims[1] = 219;
		dims[2] = 366;
		somaBkg = false;
		FG = false;

		cout << "half side length: ";
		cin >> halfSideLength;
	}
	else
	{
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
	}

	QString manualSWCNameQ = QString::fromStdString(manualSWCName);
	NeuronTree inputTree = readSWC_file(manualSWCNameQ);
	NeuronTree bkgTree, somaBkgTree, signalTree, combinedBkgTree;

	float secondSoma[3];
	secondSoma[0] = inputTree.listNeuron.at(0).x;
	secondSoma[1] = inputTree.listNeuron.at(0).y;
	secondSoma[2] = inputTree.listNeuron.at(0).z;
	bkgNode_Gen(&inputTree, &bkgTree, dims, 60, 10);
	sigNode_Gen(&inputTree, &signalTree, 20, 2);
	string outbkgSWCName = caseRoot + "\\bkg_test_down2.swc";
	string outSWCsigName = caseRoot + "\\signal_test_down2.swc";

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
	patchsz[0] = halfSideLength * 2 + 1;
	patchsz[1] = halfSideLength * 2 + 1;
	patchsz[2] = 1;
	patchsz[3] = 1;
	
	int count = 0;
	for (QList<NeuronSWC>::iterator it = signalTree.listNeuron.begin(); it != signalTree.listNeuron.end(); ++it)
	{
		int xCoord = int(it->x);
		int yCoord = int(it->y);
		int zCoord = int(it->z);

		string prefix;
		if (zCoord < 10) prefix = "\\0000";
		else if (zCoord < 100) prefix = "\\000";
		else if (zCoord < 1000) prefix = "\\00";
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
		unsigned char* sigPatch = new unsigned char[(halfSideLength * 2 + 1) * (halfSideLength * 2 + 1)];
		int xlb, xhb, ylb, yhb;
		if (xCoord - halfSideLength <= 0) xlb = 1; else xlb = xCoord - halfSideLength;
		if (xCoord + halfSideLength >= xDim) xhb = xDim; else xhb = xCoord + halfSideLength;
		if (yCoord - halfSideLength <= 0) ylb = 1; else ylb = yCoord - halfSideLength;
		if (yCoord + halfSideLength >= yDim) yhb = yDim; else yhb = yCoord + halfSideLength;
		ImgProcessor::cropImg2D(tiffSlice1D, sigPatch, xlb, xhb, ylb, yhb, xDim, yDim);
		QString outimg_file = QString::fromStdString(sigPatchRoot) + QString("\\x%1_y%2_z%3.tif").arg(int(it->x)).arg(int(it->y)).arg(int(it->z));
		string filename = outimg_file.toStdString();
		const char* filenameC = filename.c_str();
		cout << filename << endl;
		ImgManager::saveimage_wrapper(filenameC, sigPatch, patchsz, 1);

		tiffPtr->~Image4DSimple();
		operator delete(tiffPtr);
		if (tiffSlice1D) { delete[] tiffSlice1D; tiffSlice1D = 0; }
		if (sigPatch) { delete[] sigPatch; sigPatch = 0; }
	}

	count = 0;
	for (QList<NeuronSWC>::iterator it = bkgTree.listNeuron.begin(); it != bkgTree.listNeuron.end(); ++it)
	{
		int xCoord = int(it->x);
		int yCoord = int(it->y);
		int zCoord = int(it->z);

		string prefix;
		if (zCoord < 10) prefix = "\\0000";
		else if (zCoord < 100) prefix = "\\000";
		else if (zCoord < 1000) prefix = "\\00";
		string sliceNo = to_string(zCoord);
		string tiffSliceName = tiffRoot + prefix + sliceNo + ".tif";
		const char* tiffSliceNameC = tiffSliceName.c_str();

		Image4DSimple* bkg4DPtr = new Image4DSimple;
		bkg4DPtr->loadImage(tiffSliceNameC);
		long int totalbytesSWC = bkg4DPtr->getTotalBytes();
		unsigned char* slice1D = new unsigned char[totalbytesSWC];
		memcpy(slice1D, bkg4DPtr->getRawData(), totalbytesSWC);
		int xDim = bkg4DPtr->getXDim();
		int yDim = bkg4DPtr->getYDim();
		unsigned char* bkgPatch = new unsigned char[(halfSideLength * 2 + 1) * (halfSideLength * 2 + 1)];
		int xlb, xhb, ylb, yhb;
		if (xCoord - halfSideLength <= 0) xlb = 1; else xlb = xCoord - halfSideLength;
		if (xCoord + halfSideLength >= xDim) xhb = xDim; else xhb = xCoord + halfSideLength;
		if (yCoord - halfSideLength <= 0) ylb = 1; else ylb = yCoord - halfSideLength;
		if (yCoord + halfSideLength >= yDim) yhb = yDim; else yhb = yCoord + halfSideLength;
		ImgProcessor::cropImg2D(slice1D, bkgPatch, xlb, xhb, ylb, yhb, xDim, yDim);
		QString outimg_file = QString::fromStdString(bkgPatchRoot) + QString("\\x%1_y%2_z%3.tif").arg(int(it->x)).arg(int(it->y)).arg(int(it->z));
		string filename = outimg_file.toStdString();
		const char* filenameC = filename.c_str();
		cout << filename << endl;
		ImgManager::saveimage_wrapper(filenameC, bkgPatch, patchsz, 1);

		bkg4DPtr->~Image4DSimple();
		operator delete(bkg4DPtr);
		if (slice1D) { delete[] slice1D; slice1D = 0; }
		if (bkgPatch) { delete[] bkgPatch; bkgPatch = 0; }
	}

	return 0;
}