/*
* Copyright (C) 2014  Fernando Amat
* See license.txt for full license and copyright notice.
*
* Authors: Fernando Amat
*  mainTest_klnIO.cxx
*
*  Created on: October 1st, 2014
*      Author: Fernando Amat
*
* \brief Test the main klb I/O library using the C-style code
*
*
*/


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>  
#include <time.h>
#include <string.h>
#include "klb_Cwrapper.h"



int main(int argc, const char** argv)
{
	int numThreads = 10;//<= 0 indicates use as many as possible
	enum KLB_COMPRESSION_TYPE compressionType = BZIP2;//2->zlib;1->bzip2; 0->none
	
	char filenameOut[256] = "../../testData/img";
	uint32_t	xyzct[KLB_DATA_DIMS] = { 101, 151, 29, 1, 1 };//137 total Z planes
	uint32_t	blockSize[KLB_DATA_DIMS] = { 96, 96, 8, 1, 1 };
	char metadata_[KLB_METADATA_SIZE];
	enum KLB_DATA_TYPE dataType = UINT16_TYPE;
	char filenameAux[256];

	int64_t ii;
	FILE* fin;
	void* im;

	if (argc > 1)//do not use default arguments
	{		
		if (argc != 2 + KLB_DATA_DIMS)
		{
			printf("ERROR: wrong number of arguments. Call the programs with <filename> x y z c t\n");
			return 2;
		}

		sprintf(filenameOut, "%s", argv[1]);
	
		for (ii = 0; ii < KLB_DATA_DIMS; ii++)
			xyzct[ii] = atoi(argv[2 + ii]);
	}
		
	sprintf(metadata_, "Testing metadata");


	//read image	
	size_t imSize = xyzct[0];
	for (ii = 1; ii < KLB_DATA_DIMS; ii++)
		imSize *= xyzct[ii];

	im = malloc(sizeof(uint16_t)* imSize);
		
	sprintf(filenameAux, "%s.raw", filenameOut);
	printf("Reading file %s\n", filenameAux);

	fin = fopen(filenameAux, "rb");

	if (fin == NULL)
	{
		printf("ERROR: opening file %s\n", filenameOut);
		return 2;
	}
	fread((char*)im, sizeof(uint16_t), imSize, fin);
	fclose(fin);


	//compress file
	sprintf(filenameAux, "%s.klb", filenameOut);	

	printf("Compressing file to %s\n", filenameAux );

	
	//write image
	clock_t start = clock() / (CLOCKS_PER_SEC / 1000);
	int err = writeKLBstack(im, filenameAux, xyzct, dataType, numThreads, NULL, blockSize, compressionType, metadata_);
	clock_t end = clock() / (CLOCKS_PER_SEC / 1000);
	printf("Took %ld ms\n", end-start);

	//----------------------------------------------------------
	//write image using double pointer
	const size_t numSlices = xyzct[2];
	const size_t sliceSizeBytes = xyzct[0] * xyzct[1] * sizeof(uint16_t);
	size_t offsetSlice = 0;
	char** imSlice = (char**)malloc(numSlices * sizeof(char*));
	const char *imAux = (char*)im;
	for (ii = 0; ii < numSlices; ii++)
	{
		imSlice[ii] = (char*)malloc(sliceSizeBytes * sizeof(char));
		memcpy(imSlice[ii], &(imAux[offsetSlice]), sliceSizeBytes);
		offsetSlice += sliceSizeBytes;
	}
	//compress file
	char filenameSlices[512];
	sprintf(filenameSlices, "%s_stackSlice.klb", filenameOut);
	printf("Compressing file to %s using double pointer\n", filenameSlices);
	
	start = clock() / (CLOCKS_PER_SEC / 1000);	
	err = writeKLBstackSlices((void**)imSlice, filenameSlices, xyzct, dataType, numThreads, NULL, blockSize, compressionType, metadata_);

	end = clock() / (CLOCKS_PER_SEC / 1000);
	printf("Took %ld ms\n", end-start);
	//release memory
	for (ii = 0; ii < numSlices; ii++)
		free(imSlice[ii]);
	free(imSlice);

	
	//=====================================================================
	//read header
	printf("reading header...\n");

	uint32_t	xyzctR[KLB_DATA_DIMS];
	uint32_t	blockSizeR[KLB_DATA_DIMS];
	char metadataR[KLB_METADATA_SIZE];
	enum KLB_DATA_TYPE dataTypeR;
	enum KLB_COMPRESSION_TYPE compressionTypeR;
	float32_t pixelSizeR[KLB_METADATA_SIZE];


	err = readKLBheader(filenameAux, xyzctR, &dataTypeR, pixelSizeR, blockSizeR, &compressionTypeR, metadataR);

	printf("Metadata: %s\n", metadataR);
	printf("Compression type = %d, data type = %d\n", (int)compressionTypeR, (int)dataTypeR);
	for (ii = 0; ii < KLB_DATA_DIMS; ii++)
	{
		printf("%d\t%d\t%f\n", (int)(xyzctR[ii]), (int)(blockSizeR[ii]), pixelSizeR[ii]);
	}


	//=======================================================================
	//read full image
	printf("Reading full image\n");
	uint16_t* imRead;

	start = clock() / (CLOCKS_PER_SEC / 1000);
	imRead = (uint16_t*)readKLBstack(filenameAux, xyzctR, &dataTypeR, -1, NULL, NULL, NULL, NULL);
	end = clock() / (CLOCKS_PER_SEC / 1000);	
	if (imRead == NULL || dataTypeR != 1)
	{
		printf("ERROR: reading file\n");
	}

	//compare elements
	uint64_t N = 1;
	for (ii = 0; ii < KLB_DATA_DIMS; ii++)
		N *= xyzctR[ii];
	int isEqual = 1;
	uint16_t* imPtr = (uint16_t*)im;
	for (ii = 0; ii < N; ii++)
	{
		if (imRead[ii] != imPtr[ii])
		{
			isEqual = 0;
			break;
		}
	}
	if (!isEqual)
		printf("ERROR!!!: images are different\n");
	else
		printf("Reading completed successfully in %ld ms\n", end-start);
	free(imRead);


	//=======================================================================
	//release memory
	free(im);

	return 0;
}