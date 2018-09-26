#include <iostream>
#include <fstream>
#include <set>

#include "ImgProcessor.h"
#include "ImgManager.h"

using namespace std;
using namespace boost::filesystem;

void ImgManager::imgEntry(QString caseID, imgFormat format)
{
	if (format == slices)
	{
		registeredImg currImgCase;
		currImgCase.imgAlias = caseID;
		pair<multimap<string, string>::iterator, multimap<string, string>::iterator> range = this->inputMultiCasesSliceFullPaths.equal_range(caseID.toStdString());
		for (multimap<string, string>::iterator it = range.first; it != range.second; ++it)
		{
			string sliceFullName = it->second;
			string fileName = it->second.substr(it->second.length() - 9, 9);
			const char* sliceFullNameC = sliceFullName.c_str();
			Image4DSimple* slicePtr = new Image4DSimple;
			slicePtr->loadImage(sliceFullNameC);
			currImgCase.dims[0] = int(slicePtr->getXDim());
			currImgCase.dims[1] = int(slicePtr->getYDim());
			currImgCase.dims[2] = 1;
			long int totalbyteSlice = slicePtr->getTotalBytes();
			myImg1DPtr slice1D(new unsigned char[totalbyteSlice]);
			memcpy(slice1D.get(), slicePtr->getRawData(), totalbyteSlice);
			currImgCase.slicePtrs.insert(pair<string, myImg1DPtr>(fileName, slice1D));

			slicePtr->~Image4DSimple();
			operator delete(slicePtr);
		}

		//for (map<string, myImg1DPtr>::iterator sliceIt = currImgCase.slicePtrs.begin(); sliceIt != currImgCase.slicePtrs.end(); ++sliceIt)
		//	qDebug() << QString::fromStdString(sliceIt->first) << " " << sliceIt->second[805985];

		this->imgDatabase.insert(pair<string, registeredImg>(caseID.toStdString(), currImgCase));
	}
	else if (format == single2D)
	{
		registeredImg currImgCase;
		currImgCase.imgAlias = caseID;
		
		string sliceFullName = *(this->inputSingleCaseSliceFullPaths.begin());
		string fileName = (*(this->inputSingleCaseSliceFullPaths.begin())).substr((*(this->inputSingleCaseSliceFullPaths.begin())).length() - 9, 9);
		const char* sliceFullNameC = sliceFullName.c_str();
		Image4DSimple* slicePtr = new Image4DSimple;
		slicePtr->loadImage(sliceFullNameC);
		currImgCase.dims[0] = int(slicePtr->getXDim());
		currImgCase.dims[1] = int(slicePtr->getYDim());
		currImgCase.dims[2] = 1;
		long int totalbyteSlice = slicePtr->getTotalBytes();
		myImg1DPtr slice1D(new unsigned char[totalbyteSlice]);
		memcpy(slice1D.get(), slicePtr->getRawData(), totalbyteSlice);
		currImgCase.slicePtrs.insert(pair<string, myImg1DPtr>(fileName, slice1D));

		slicePtr->~Image4DSimple();
		operator delete(slicePtr);
		
		this->imgDatabase.insert(pair<string, registeredImg>(caseID.toStdString(), currImgCase));
	}
}

// ================= Methods for generating binary masks from SWC files ================= //
void ImgManager::detectedNodes2mask_2D(QList<NeuronSWC>* nodeListPtr, long int dims[2], unsigned char*& mask1D)
{
	// -- Generate 2D masks based on each detected "SWC signal slice". 
	// -- Note, since the detected signal does not contain topological information (all parents = -1), no cross z section node appearace is allowed as oppose to NeuronStructUtil::swcSlicer.

	mask1D = new unsigned char[dims[0] * dims[1]];
	for (size_t i = 0; i < (dims[0] * dims[1]); ++i) mask1D[i] = 0;
	for (QList<NeuronSWC>::iterator it = nodeListPtr->begin(); it != nodeListPtr->end(); ++it)
	{
		int xCoord = int(it->x);
		int yCoord = int(it->y);

		mask1D[dims[0] * (yCoord - 1) + (xCoord - 1)] = 255;
	}
}

void ImgManager::MaskMIPfrom2Dseries(string path)
{
	// -- Generate MIP of a series of SWC binary masks.
	// path: the directory in which SWC binary masks are stored.

	vector<string> maskName;
	for (directory_iterator itr(path); itr != directory_iterator(); ++itr)
	{
		string tale = itr->path().filename().string();
		//cout << tale << endl;
		tale = tale.substr(tale.length() - 3, 3);
		if (tale.compare("tif") != 0) continue;
		else maskName.push_back(itr->path().filename().string());
	}
	cout << "number of images to be processed: " << maskName.size() << endl;
	
	string firstImg = path + "\\" + maskName.at(0);
	const char* firstImgC = firstImg.c_str();
	Image4DSimple* tempPtr = new Image4DSimple;
	tempPtr->loadImage(firstImgC);
	int dims[2];
	dims[0] = tempPtr->getXDim();
	dims[1] = tempPtr->getYDim();
	dims[2] = tempPtr->getZDim();
	dims[3] = tempPtr->getCDim();
	cout << "image dimension: " << dims[0] << " " << dims[1] << endl;
	
	unsigned char* maskMIP = new unsigned char[dims[0] * dims[1]];
	for (int i = 0; i < (dims[0] * dims[1]); ++i) maskMIP[i] = 0;
	string outputFileName = path + "\\maskMIP.tif";
	const char* outputFileNameC = outputFileName.c_str();
	for (vector<string>::iterator it = maskName.begin(); it != maskName.end(); ++it)
	{
		string inputName = path + "\\" + *it;
		cout << inputName << endl;
		const char* inputNameC = inputName.c_str();
		Image4DSimple* inputMask4D = new Image4DSimple;
		inputMask4D->loadImage(inputNameC);
		V3DLONG totalbytes = inputMask4D->getTotalBytes();
		unsigned char* input1D = new unsigned char[totalbytes];
		memcpy(input1D, inputMask4D->getRawData(), totalbytes);
		ImgProcessor::imageMax(input1D, maskMIP, maskMIP, dims);
		delete inputMask4D;
	}

	V3DLONG sz[4];
	sz[0] = dims[0];
	sz[1] = dims[1];
	sz[2] = 1;
	sz[3] = 1;
	ImgManager::saveimage_wrapper(outputFileNameC, maskMIP, sz, V3D_UINT8);
}

void ImgManager::swc2Mask_2D(string swcFileName, long int dims[2], unsigned char*& mask1D)
{
	// -- This is function is borrowed and modified from Hanchuan's swc2mask functionalities. 
	//    It create a corresponding binary mask of a given SWC file with image size specified.
	// dims: image dimension

	long int tol_sz = dims[0] * dims[1];
	if (mask1D == 0) mask1D = new unsigned char[tol_sz]; 
	for (long i = 0; i < tol_sz; ++i) mask1D[i] = 0;

	vector<MyMarker*> inswc = readSWC_file(swcFileName);
	vector<MyMarker*> leaf_markers = getLeaf_markers(inswc);
	set<MyMarker*> visited_markers;
	for (int i = 0; i < leaf_markers.size(); ++i)
	{
		MyMarker* leaf = leaf_markers[i];
		MyMarker* p = leaf;
		while (visited_markers.find(p) == visited_markers.end() && p->parent != 0)
		{
			MyMarker* par = p->parent;
			vector<MyMarker> tmp_markers;
			this->getMarkersBetween(tmp_markers, *p, *par);
			for (int j = 0; j < tmp_markers.size(); ++j)
			{
				int x = tmp_markers[j].x;
				int y = tmp_markers[j].y;
				int z = 0;
				if (x < 0 || x >= dims[0] || y < 0 || y >= dims[1]) continue;
				mask1D[y * dims[0] + x] = 255;
			}
			visited_markers.insert(p);
			p = par;
		}
	}
	return;
}

bool ImgManager::getMarkersBetween(vector<MyMarker>& allmarkers, MyMarker m1, MyMarker m2)
{
	// This is function is borrowed from Hanchuan's swc2mask functionalities. It create binary area between nodes by interpolating the radius.

	double A = m2.x - m1.x;
	double B = m2.y - m1.y;
	double C = m2.z - m1.z;
	double R = m2.radius - m1.radius;
	double D = sqrt(A*A + B*B + C*C);
	A = A / D; B = B / D; C = C / D; R = R / D;

	double ctz = A / sqrt(A*A + B*B);
	double stz = B / sqrt(A*A + B*B);

	double cty = C / sqrt(A*A + B*B + C*C);
	double sty = sqrt(A*A + B*B) / sqrt(A*A + B*B + C*C);

	double x0 = m1.x;
	double y0 = m1.y;
	double z0 = m1.z;
	double r0 = m1.radius;

	set<MyMarker> marker_set;

	for (double t = 0.0; t <= dist(m1, m2); t += 1.0)
	{
		MyMarker marker;
		int cx = x0 + A*t + 0.5;
		int cy = y0 + B*t + 0.5;
		int cz = z0 + C*t + 0.5;
		int radius = r0 + R*t + 0.5;
		int radius2 = radius * radius;

		for (int k = -radius; k <= radius; k++)
		{
			for (int j = -radius; j <= radius; j++)
			{
				for (int i = -radius; i <= radius; i++)
				{
					if (i * i + j * j + k * k > radius2) continue;
					double x = i, y = j, z = k;
					double x1, y1, z1;

					//rotate_coordinate_z_clockwise(ctz, stz, x, y, z);
					//rotate_along_y_clockwise     (cty, sty, x, y, z);
					//rotate_coordinate_x_anticlock(ctz, stz, x, y, z);
					//translate_to(cx, cy, cz, x, y, z);
					y1 = y * ctz - x * stz; x1 = x * ctz + y * stz; y = y1; x = x1;
					x1 = x * cty + z * sty; z1 = z * cty - x * sty; x = x1; z = z1;
					z1 = z * ctz + y * stz; y1 = y * ctz - z * stz; z = z1; y = y1;
					x += cx; y += cy; z += cz;
					x = (int)(x + 0.5);
					y = (int)(y + 0.5);
					z = (int)(z + 0.5);
					marker_set.insert(MyMarker(x, y, z));
				}
			}
		}
	}

	allmarkers.insert(allmarkers.end(), marker_set.begin(), marker_set.end());
	return true;
}
// ================ END of [Methods for generating binary masks from SWC files] ================ //

void ImgManager::imgSliceDessemble(string imgName, int tileSize)
{
	const char* imgNameC = imgName.c_str();
	Image4DSimple* inputImg4D = new Image4DSimple;
	inputImg4D->loadImage(imgNameC);
	V3DLONG totalbytes = inputImg4D->getTotalBytes();
	unsigned char* img1D = new unsigned char[totalbytes];
	memcpy(img1D, inputImg4D->getRawData(), totalbytes);

	int dims[3];
	dims[0] = inputImg4D->getXDim();
	dims[1] = inputImg4D->getYDim();
	dims[2] = inputImg4D->getCDim();
	cout << "x dimension: " << dims[0] << "    y dimension: " << dims[1] << endl;

	int i_tileSteps = dims[0] / tileSize;
	int j_tileSteps = dims[1] / tileSize;
	cout << "tiles in x: " << i_tileSteps << "    tiles in y: " << j_tileSteps << endl;
	ImgProcessor* imgProcPtr = new ImgProcessor;
	for (int j = 0; j < j_tileSteps; ++j)
	{
		for (int i = 0; i < i_tileSteps; ++i)
		{
			int xlb = tileSize * i + 1;
			int xhb = tileSize * (i + 1);
			int ylb = tileSize * j + 1;
			int yhb = tileSize * (j + 1);
			long int ROIsz = tileSize * tileSize;
			unsigned char* ROIPtr = new unsigned char[ROIsz];
			V3DLONG ROIxyz[4];
			ROIxyz[0] = tileSize;
			ROIxyz[1] = tileSize;
			ROIxyz[2] = 1;
			ROIxyz[3] = dims[2];
			imgProcPtr->cropImg2D(img1D, ROIPtr, xlb, xhb, ylb, yhb, dims);

			QString patchPath = QString::fromStdString(imgName) + "_patches";
			if (!QDir(patchPath).exists()) QDir().mkpath(patchPath);
			QString outimg_file = patchPath + QString("/x%1_y%2.tif").arg(xlb - 1).arg(ylb - 1);
			string filename = outimg_file.toStdString();
			const char* filenameC = filename.c_str();
			ImgManager::saveimage_wrapper(filenameC, ROIPtr, ROIxyz, 1);

			ROIPtr = nullptr;
		}
	}
	delete imgProcPtr;
}
