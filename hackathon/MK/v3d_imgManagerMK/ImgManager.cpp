#include <iostream>
#include <set>
#include <iterator>
#include <fstream>

#include <boost\filesystem.hpp>

#include "ImgProcessor.h"
#include "ImgManager.h"

using namespace std;
using namespace boost::filesystem;

ImgManager::ImgManager(string inputImgName)
{
	this->wholeImgName = inputImgName;
	const char* imgName = inputImgName.c_str();
	this->wholeImg4DPtr = new Image4DSimple;
	this->wholeImg4DPtr->loadImage(imgName);
	this->imgData1D = new unsigned char;
	ImgManager::img1Ddumpster(this->wholeImg4DPtr, this->imgData1D, this->dims, this->datatype);
}

bool ImgManager::img1Ddumpster(Image4DSimple* inputImgPtr, unsigned char*& data1D, long int dims[4], int datatype)
{
	if (!inputImgPtr || !inputImgPtr->valid())
		return false;

	if (data1D) { delete[] data1D; data1D = 0; }

	V3DLONG totalbytes = inputImgPtr->getTotalBytes();
	try
	{
		data1D = new unsigned char[totalbytes];
		if (!data1D) goto Label_error_simple_loadimage_wrapper;

		memcpy(data1D, inputImgPtr->getRawData(), totalbytes);
		datatype = inputImgPtr->getUnitBytes(); // 1, 2, or 4
		dims[0] = inputImgPtr->getXDim();
		dims[1] = inputImgPtr->getYDim();
		dims[2] = inputImgPtr->getZDim();
		dims[3] = inputImgPtr->getCDim();
	}
	catch (...)
	{
		goto Label_error_simple_loadimage_wrapper;
	}
	return true;

Label_error_simple_loadimage_wrapper:
	if (inputImgPtr) { delete inputImgPtr; inputImgPtr = 0; }
	return false;
}

void ImgManager::MaskMIPfrom2Dseries(string path)
{
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
	long int dims[2];
	dims[0] = tempPtr->getXDim();
	dims[1] = tempPtr->getYDim();
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
		ImgProcessor::imageMax(input1D, maskMIP, totalbytes);
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

void ImgManager::imgSliceDessemble(string imgName, int tileSize)
{
	const char* imgNameC = imgName.c_str();
	Image4DSimple* inputImg4D = new Image4DSimple;
	inputImg4D->loadImage(imgNameC);
	V3DLONG totalbytes = inputImg4D->getTotalBytes();
	unsigned char* img1D = new unsigned char[totalbytes];
	memcpy(img1D, inputImg4D->getRawData(), totalbytes);

	int imgX = inputImg4D->getXDim();
	int imgY = inputImg4D->getYDim();
	int channel = inputImg4D->getCDim();
	cout << "x dimension: " << imgX << "    y dimension: " << imgY << endl;

	int i_tileSteps = imgX / tileSize;
	int j_tileSteps = imgY / tileSize;
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
			ROIxyz[3] = channel;
			imgProcPtr->cropImg2D(img1D, ROIPtr, xlb, xhb, ylb, yhb, imgX, imgY);

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
