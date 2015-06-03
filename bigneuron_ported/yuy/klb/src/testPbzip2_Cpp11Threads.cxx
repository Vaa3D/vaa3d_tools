/*
* Copyright (C) 2014  Fernando Amat
* See license.txt for full license and copyright notice.
*
* Authors: Fernando Amat
*  testPbzip2_Cpp11Threads.xxx
*
*  Created on: October 1st, 2014
*      Author: Fernando Amat
*
* \brief Simple test for std::threads and to try to reproduce pbzip2 basic functionality natively in Windows (without Cygwin for Pthreads)
*
*
*	Modified example from http://www.codeproject.com/Articles/598695/Cplusplus-threads-locks-and-condition-variables
*/



#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <queue>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdint>
#include <list>

#if defined(_WIN32) || defined(_WIN64)
	#include <string>
	#define NOMINMAX
#else
	#include <string.h>
	#include <cmath>
#endif

#include "bzlib.h"

//#define DEBUG_PRINT_THREADS

std::mutex				g_lockblockId;//so each worker reads a unique blockId
std::condition_variable g_queuecheck;//to notify writer that blocks are ready

//global variables for the blocks
const int blockSizeBytes = 2048 * 1024;//set it around 2MB
std::int64_t blockId = 0;//keeps track of which block to read


//======================================================
void workerfunc(char* buffer, std::int64_t fLength, int* g_blockSize)
{
	const int BWTblockSize = 9;//maximum compression
	std::int64_t blockId_t;
	int gcount;//read bytes
	unsigned int sizeCompressed;//size of block in bytes after compression
	char* bufferIn = new char[blockSizeBytes];
	//main loop to keep processing blocks while they are available
	while (1)
	{
		//get the blockId resource
		g_lockblockId.lock();
		blockId_t = blockId;
		blockId++;
		g_lockblockId.unlock();

		
#ifdef DEBUG_PRINT_THREADS
		printf("Thread %d reading block %d\n", (int) (std::this_thread::get_id().hash()), (int)blockId_t);
#endif

		//check if we can access data or we cannot read longer
		if ( blockId_t * blockSizeBytes >= fLength )
			break;

		//read block		
		gcount = std::min(blockSizeBytes, (int)(fLength - blockId_t * blockSizeBytes));
		
		memcpy(bufferIn, &(buffer[blockId_t * blockSizeBytes]), gcount);
		

		// compress the memory buffer (blocksize=9*100k, verbose=0, worklevel=30)
		sizeCompressed = blockSizeBytes;
		int ret = BZ2_bzBuffToBuffCompress(&(buffer[blockId_t * blockSizeBytes]), &sizeCompressed, bufferIn, gcount, BWTblockSize, 0, 30);
		if (ret != BZ_OK)
		{
			std::cout << "ERROR: workerfunc: compressing data at block"<<blockId_t << std::endl;
			sizeCompressed = 0;
		}

		/*
		//for uncompressed copy (for debugging purposes)
		//sizeCompressed = gcount;
		//memcpy(&(buffer[blockId_t * blockSizeBytes]), bufferIn, sizeCompressed);//fid.gcount():Returns the number of characters extracted by the last unformatted input operation performed on the object
		*/

		//signal blockWriter that this block can be writen
		//std::unique_lock<std::mutex> locker(g_lockqueue);//adquires the lock
		g_blockSize[blockId_t] = sizeCompressed;//I don't really need the lock to modify this. I only need to singal the condition variable
		g_queuecheck.notify_one();
	}


	//release memory
	delete[] bufferIn;
	
}

//=========================================================================
//writes compressed blocks sequentially as they become available (in order) from the workers
void blockWriter(char* buffer, std::string filenameOut, int* g_blockSize, std::int64_t numBlocks)
{
	std::int64_t nextBlockId = 0, offset = 0;

	//open output file
	std::ofstream fout(filenameOut.c_str(), std::ios::binary | std::ios::out);
	if (fout.is_open() == false)
	{
		std::cout << "ERROR: file " << filenameOut << " could not be opened" << std::endl;
		nextBlockId = numBlocks;
	}


	// loop until end is signaled	
	std::mutex              g_lockqueue;//mutex for the condition variable (dummy one)
	std::unique_lock<std::mutex> locker(g_lockqueue);//acquires the lock but this is the only thread using it. We cannot have condition_variables without a mutex
	while ( nextBlockId < numBlocks )
	{
				
		g_queuecheck.wait(locker, [&](){return (g_blockSize[nextBlockId] >= 0); });//releases the lock until notify. If condition is not satisfied, it waits again

#ifdef DEBUG_PRINT_THREADS
		printf("Writer appending block %d with %d bytes\n", (int)nextBlockId, g_blockSize[nextBlockId]);
#endif

		//write block
		fout.write(&(buffer[offset]), g_blockSize[nextBlockId]);
		offset += (std::int64_t)(g_blockSize[nextBlockId]);

		//update variables
		nextBlockId++;
	}

	//close file
	fout.close();
}

//========================================================================

int main(int argc, const char** argv)
{
	const int numThreads = 12;//number of threads to process parallel blocks (we use one extra one to write the data into disk)
	
	//std::string filename("E:/compressionFormatData/DrosophilaTM300.raw");
	//std::string filenameOut("E:/compressionFormatData/DrosophilaTM300.klb");

	std::string filename("E:/compressionFormatData/ZebrafishTM200.raw");
	std::string filenameOut("E:/compressionFormatData/ZebrafishTM200.klb");
	

	//open input file
	std::ifstream fid(filename.c_str(), std::ios::binary | std::ios::in);
	if (fid.is_open() == false)
	{
		std::cout << "ERROR: file " << filename << " could not be opened" << std::endl;
		return 2;
	}

	typedef std::chrono::high_resolution_clock Clock;
	auto t0 = Clock::now();

	//read entire file
	//TODO: how to leave this to workers so we do not require big memory allocation
	
	// get length of file:
	fid.seekg(0, fid.end);
	std::int64_t fLength = fid.tellg();
	//read file
	fid.seekg(0, fid.beg);
	char * buffer = new char[fLength];

	fid.read(buffer, fLength);
	fid.close();

	auto t1 = Clock::now();

	//initialization of global communication variables
	blockId = 0;	
	std::int64_t numBlocks = ceil((float)fLength / (float)blockSizeBytes);
	int* g_blockSize = new int[numBlocks];//number of bytes (after compression) to be written. If the block has not been compressed yet, it has a -1 value
	for (std::int64_t ii = 0; ii < numBlocks; ii++)
		g_blockSize[ii] = -1;

	// start the thread to write
	std::thread writerthread(blockWriter, buffer, filenameOut, g_blockSize, numBlocks);

	// start the working threads
	std::vector<std::thread> threads;
	for (int i = 0; i < numThreads; ++i)
	{
		threads.push_back(std::thread(workerfunc, buffer, fLength, g_blockSize ));
	}

	//wait for the workers to finish
	for (auto& t : threads)
		t.join();

	//wait for the writer
	writerthread.join();

	auto t2 = Clock::now();

	std::cout << "Read file = " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() << "ms; compress + write file =" << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " ms; Total time = " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t0).count() << " ms" << std::endl;
	
	//release memory
	delete[] g_blockSize;
	delete[] buffer;

	return 0;
}