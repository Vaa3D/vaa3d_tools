#ifndef IMGMANAGER_H
#define IMGMANAGER_H

#include <string>
#include <vector>
#include <deque>
#include <map>
#include <unordered_map>

#include <boost\filesystem.hpp>
#include <boost\shared_array.hpp>

#include <qstring.h>
#include <qstringlist.h>

#include "basic_surf_objs.h"
#include "my_surf_objs.h"
#include "basic_4dimage.h"
#include "v3d_interface.h"

enum imgFormat { cube, slices, single2D };
typedef boost::shared_array<unsigned char> myImg1DPtr; // --> Since GNU 4.8 hasn't adopted C++11 standard (Linux Vaa3D), 
													   //     I decided to use boost's shared pointer instead of C++11's std::shared_ptr.

struct registeredImg
{
	QString imgAlias;
	QString imgCaseRootQ;

	//shared_ptr<unsigned char*> imgData1D;
	//shared_ptr<unsigned char**> imgData2D;
	//shared_ptr<unsigned char***> imgData3D;

	map<string, myImg1DPtr> slicePtrs;
	
	int dims[3];
};

class ImgManager
{
public: 
	/********* Constructors and Basic Data Members *********/
	ImgManager() {};
	ImgManager(string wholeImgName);

	QString inputCaseRootPath;
	QString inputSWCPath;
	QString outputRootPath;
	QStringList caseList;
	deque<string> inputSingleCaseSliceFullPaths;
	deque<string> outputSingleCaseSliceFullPaths;
	multimap<string, string> inputMultiCasesSliceFullPaths;
	multimap<string, string> outputMultiCasesSliceFullPaths;
	/*******************************************************/

	/********* IO *********/
	static inline bool saveimage_wrapper(const char* filename, unsigned char* pdata, V3DLONG sz[], int datatype);
	
	map<string, registeredImg> imgDatabase;
	void imgManager_regisImg(QString caseID, imgFormat format);
	/**********************/

	/********* Methods for generating binary masks from SWC files *********/
	void swc2Mask_2D(string swcFileName, long int dims[2], unsigned char*& mask1D); // Generate a 2D mask based on the corresponding "SWC slice."
	bool getMarkersBetween(vector<MyMarker>& allmarkers, MyMarker m1, MyMarker m2);

	void detectedNodes2mask_2D(QList<NeuronSWC>* nodeListPtr, long int dims[2], unsigned char*& mask1D);
	/**********************************************************************/

	/********* Assemble all SWC masks together as an "SWC mip mask." *********/
	void MaskMIPfrom2Dseries(string path);                       
	/*************************************************************************/

	/********* Dessemble image/stack into tiles. This is for Caffe's memory leak issue *********/
	static void imgSliceDessemble(string imgName, int tileSize);
	/*******************************************************************************************/
};

inline bool ImgManager::saveimage_wrapper(const char* filename, unsigned char pdata[], V3DLONG sz[], int datatype)
{
	if (!pdata)
	{
		cerr << "input array not valid" << endl;
		return false;
	}

	if (!filename || !sz)
	{
		v3d_msg("some of the parameters for simple_saveimage_wrapper() are not valid.", 0);
		return false;
	}

	ImagePixelType dt;
	if (datatype == 1) dt = V3D_UINT8;
	else if (datatype == 2) dt = V3D_UINT16;
	else if (datatype == 4) dt = V3D_FLOAT32;
	else
	{
		v3d_msg(QString("the specified save data type in simple_saveimage_wrapper() is not valid, dt=[%1].").arg(datatype), 0);
		return false;
	}

	Image4DSimple* outimg = new Image4DSimple;
	if (outimg) outimg->setData(pdata, sz[0], sz[1], sz[2], sz[3], dt);
	else
	{
		v3d_msg("Fail to new Image4DSimple for outimg.");
		return false;
	}
	outimg->saveImage(filename);
	
	return true;
}

#endif