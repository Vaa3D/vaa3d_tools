/*
* Copyright (C) 2014 by  Fernando Amat
* See license.txt for full license and copyright notice.
*
* Authors: Fernando Amat
*  klb_imageIO.h
*
*  Created on: October 2nd, 2014
*      Author: Fernando Amat
*
* \brief Main class to read/write klb format. THIS CLASS DOES NOT STORE ANY POINTER TO THE IMAGE. IT JUST RETURNS THEM AND KEEPS THE FILE SYSTEM OPEN TO INTERACT WITH THE FILE
*/

#ifndef __KLB_IMAGE_IO_H__
#define __KLB_IMAGE_IO_H__

#include <string>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <atomic>

//#define PROFILE_COMPRESSION //uncomment to check how much is spent in compression

#include "klb_imageHeader.h"
#include "klb_circularDequeue.h"
#include "klb_ROI.h"

#if defined(COMPILE_SHARED_LIBRARY) && defined(_MSC_VER)
#define DECLSPECIFIER __declspec(dllexport)
#define EXPIMP_TEMPLATE

#else

#define DECLSPECIFIER
#define EXPIMP_TEMPLATE

#endif

#if defined(_MSC_VER)
//from http://support2.microsoft.com/default.aspx?scid=KB;EN-US;168958 to export DLL with STL objects within the class
//all the STL containers are private, so it should be safe to ignore the warnings for stl::string http://stackoverflow.com/questions/767579/exporting-classes-containing-std-objects-vector-map-etc-from-a-dll
EXPIMP_TEMPLATE class DECLSPECIFIER std::mutex;
EXPIMP_TEMPLATE class DECLSPECIFIER std::condition_variable;
#endif

class DECLSPECIFIER klb_imageIO
{
public:
	
	klb_image_header header;
	int numThreads;//number of threads to use

	//constructor / destructor
	klb_imageIO();
	klb_imageIO(const std::string &filename_);

	//set/get functions
	std::string getFilename() const{ return filename; };
	void setFilename(const std::string &filename_){ filename = filename_; };	

	//main functions
	int readHeader(){ return header.readHeader(filename.c_str()); };
	int readHeader(const std::string &filename_)
	{
		filename = filename_;
		return readHeader();
	};

	/*
	\brief	Main function to save an image. We assume the correct header has been set prior to calling this function. 
	*/
	int writeImage(const char* BYTE, int numThreads);

	/*
	\brief Special case to write 3D stacks using double pointer (one pointer per 2D XY slice in the image)
	*/
	int writeImageStackSlices(const char** BYTE, int numThreads);

	/*
	\brief	Main function to read an image (or part of an image defined by ROI).We assume the correct header has been set prior to calling this function. 
	*/
	int readImage(char* BYTE, const klb_ROI* ROI, int numThreads);


	/*
	\brief We preload all the file in memory and the threads read from memory (not from disk). Consumes more memory but it is XXX faster. It only makes sense to read the whole image
	*/
	int readImageFull(char* BYTE, int numThreads);

protected:

private:
	std::string filename;

	std::mutex              g_lockqueue;//mutex for the condition variable
	std::condition_variable	g_queuecheck;//to notify writer that blocks are ready
#ifdef PROFILE_COMPRESSION
	static std::atomic<long long>	g_countCompression;
#endif
	
	
	//functions to call for each thread
	void blockWriter(FILE* fout, int* g_blockSize, int* g_blockThreadId, klb_circular_dequeue** cq, int* errFlag);
	void blockCompressor(const char* buffer, int* g_blockSize, std::atomic<uint64_t> *blockId, int* g_blockThreadId, klb_circular_dequeue* cq, int threadId, int* errFlag);
	void blockCompressorStackSlices(const char** buffer, int* g_blockSize, std::atomic<uint64_t> *blockId, int* g_blockThreadId, klb_circular_dequeue* cq, int threadId, int* errFlag);

	void blockUncompressor(char* bufferOut, std::atomic<uint64_t> *blockId, const klb_ROI* ROI, int* errFlag);
	void blockUncompressorImageFull(char* bufferOut, std::atomic<uint64_t> *blockId, int* errFlag);
	void blockUncompressorInMem(char* bufferOut, std::atomic<uint64_t>	*blockId, char* bufferImgFull, int* errFlag);

	std::uint32_t maximumBlockSizeCompressedInBytes();//some formats have overhead so for small blocks of random noise it could be larger than block size
};


#endif //end of __KLB_IMAGE_IO_H__