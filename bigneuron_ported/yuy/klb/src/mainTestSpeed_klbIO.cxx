/*
* Copyright (C) 2014  Fernando Amat
* See license.txt for full license and copyright notice.
*
* Authors: Fernando Amat
*  mainTestSpeed_klnIO.cxx
*
*  Created on: October 1st, 2014
*      Author: Fernando Amat
*
* \brief Test the main klb I/O library
*
*
*/


#include <string>
#include <cstdint>
#include <chrono>
#include <algorithm>
#include <thread>

#include "klb_imageIO.h"


using namespace std;
typedef std::chrono::high_resolution_clock Clock;

int main(int argc, const char** argv)
{
	int numProg = -1;
	if (argc == 2)
		numProg = atoi(argv[1]);

	int numThreads = std::thread::hardware_concurrency();//<= 0 indicates use as many as possible
	std::uint32_t	blockSize[KLB_DATA_DIMS] = {96, 96, 8, 1, 1};
	KLB_COMPRESSION_TYPE compressionType = KLB_COMPRESSION_TYPE::BZIP2;//1->bzip2; 0->none (look at enum KLB_COMPRESSION_TYPE)


	std::string basename("E:/temp/mouse_TM000000_angle000");
	std::uint32_t	xyzct[KLB_DATA_DIMS] = { 2048, 2048, 335, 1, 1 };	
	
	cout << "Testing KLB speed with file " << basename << ".raw and blockSize =";
	for (int ii = 0; ii < KLB_DATA_DIMS; ii++)
		cout << blockSize[ii] << "x";
	cout <<" with "<<numThreads<< "threads"<< endl;
	
	//================================================
	//common definitions
	string filenameOut(basename + ".klb");
	int err = 0;
	auto t1 = Clock::now();
	auto t2 = Clock::now();
	int dim, planeSize, offsetPlane;
	klb_imageIO imgXYplane(filenameOut);
	long long int totalTime = 0;
	int numPlanes;
	uint16_t* imgB;
	klb_ROI ROIfull;
	//=========================================================================

	
	if (numProg >= 0)
	{
		char buffer[256];
		sprintf(buffer, "D:/temp/testKLB_cpp%.3d.klb", numProg);
		filenameOut = string(buffer);
	}
	
	cout << "Compressing file to " << filenameOut << endl;
	//initialize I/O object
	klb_imageIO imgIO( filenameOut );

	//setup header
	memcpy(imgIO.header.xyzct, xyzct, sizeof(uint32_t)* KLB_DATA_DIMS);
	memcpy(imgIO.header.blockSize, blockSize, sizeof(uint32_t)* KLB_DATA_DIMS);
	imgIO.header.dataType = KLB_DATA_TYPE::UINT16_TYPE;//uint16
	imgIO.header.compressionType = compressionType;
	for (int ii = 0; ii < KLB_DATA_DIMS; ii++)
		imgIO.header.pixelSize[ii] = 1.2f*(ii+1);


	//read image
	uint16_t* img = new uint16_t[imgIO.header.getImageSizePixels()];
	ifstream fim(string(basename + ".raw").c_str(), ios::binary | ios::in);
	fim.read((char*)img, sizeof(uint16_t)* imgIO.header.getImageSizePixels());

	t1 = Clock::now();
	err = imgIO.writeImage((char*)img, numThreads);//all the threads available
	if (err > 0)
		return 2;

	t2 = Clock::now();

	std::cout << "Written test file at "<<filenameOut<<" compress + write file =" << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " ms using "<<numThreads<<" threads"<< std::endl;

	delete[] img;
		

	//===========================================================================================
	//===========================================================================================

	cout << endl << endl << "Reading entire image back" << endl;


	t1 = Clock::now();
	klb_imageIO imgFull(filenameOut);

	err = imgFull.readHeader();
	if (err > 0)
		return err;
	uint16_t* imgA = new uint16_t[imgFull.header.getImageSizePixels()];

	//ROIfull.defineFullImage(imgFull.header.xyzct);
	//err = imgFull.readImage((char*)imgA, &ROIfull, numThreads);
	err = imgFull.readImageFull((char*)imgA, numThreads);
	if (err > 0)
		return err;

	t2 = Clock::now();

	std::cout << "Read full test file at " << filenameOut << " in =" << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " ms using " << numThreads << " threads" << std::endl;
	delete[] imgA;

	return 0;
	
	//===========================================================================================
	//===========================================================================================
	
	cout << endl << endl << "Reading XY planes" << endl;

	//klb_ROI ROIfull;	
	err = imgXYplane.readHeader();
	if (err > 0)
		return err;
	dim = 2;
	planeSize = imgXYplane.header.xyzct[0] * imgXYplane.header.xyzct[1];
	offsetPlane = 1;

	
	
	err = imgXYplane.readHeader();
	if (err > 0)
		return err;
	imgB = new uint16_t[planeSize];

	
	totalTime = 0; 
	numPlanes = std::min(imgXYplane.header.xyzct[dim], 10U);
	t1 = Clock::now();
	for (int ii = 0; ii < numPlanes; ii++)
	{		
		ROIfull.defineSlice(ii, dim, imgXYplane.header.xyzct);
		err = imgXYplane.readImage((char*)imgB, &ROIfull, numThreads);
		if (err > 0)
			return err;		
	}	

	t2 = Clock::now();
	totalTime += std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

	std::cout << "Read all planes test file at " << filenameOut << " in =" <<  ((float)totalTime)/ (float)numPlanes << " ms per plane using " << numThreads << " threads" << std::endl;
	delete[] imgB;
	

	//===========================================================================================
	//===========================================================================================
	
	cout << endl << endl << "Reading XZ planes" << endl;

	err = imgXYplane.readHeader();
	if (err > 0)
		return err;
	dim = 1;
	planeSize = imgXYplane.header.xyzct[0] * imgXYplane.header.xyzct[2];

	imgB = new uint16_t[planeSize];


	totalTime = 0;
	numPlanes = std::min(imgXYplane.header.xyzct[dim], 10U);
	t1 = Clock::now();
	for (int ii = 0; ii < numPlanes; ii++)
	{
		
		ROIfull.defineSlice(ii, dim, imgXYplane.header.xyzct);
		err = imgXYplane.readImage((char*)imgB, &ROIfull, numThreads);
		if (err > 0)
			return err;		
	}

	t2 = Clock::now();
	totalTime += std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

	std::cout << "Read all planes test file at " << filenameOut << " in =" << ((float)totalTime) / (float)numPlanes << " ms per plane using " << numThreads << " threads" << std::endl;
	delete[] imgB;

	

	//===========================================================================================
	//===========================================================================================
	
	cout << endl << endl << "Reading YZ planes" << endl;

	err = imgXYplane.readHeader();
	if (err > 0)
		return err;
	dim = 0;
	planeSize = imgXYplane.header.xyzct[1] * imgXYplane.header.xyzct[2];

	imgB = new uint16_t[planeSize];


	totalTime = 0;
	numPlanes = std::min( imgXYplane.header.xyzct[dim], 10U);
	t1 = Clock::now();
	for (int ii = 0; ii < numPlanes; ii++)
	{
		ROIfull.defineSlice(ii, dim, imgXYplane.header.xyzct);
		err = imgXYplane.readImage((char*)imgB, &ROIfull, numThreads);
		if (err > 0)
			return err;
	}

	t2 = Clock::now();
	totalTime += std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

	std::cout << "Read all planes test file at " << filenameOut << " in =" << ((float)totalTime) / (float)numPlanes << " ms per plane using " << numThreads << " threads" << std::endl;
	delete[] imgB;
	
	//===========================================================================================
	//===========================================================================================
	

	return 0;
}