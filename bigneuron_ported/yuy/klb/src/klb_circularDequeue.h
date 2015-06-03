/*
*
* Copyright(C) 2014 by  Fernando Amat
* See license.txt for full license and copyright notice.
*
* Authors: Fernando Amat
*  klb_circularDequeue.h
*
*  Created on : October 2nd, 2014
* Author : Fernando Amat
*
* \brief Implements a ciruclar queue to store chunks of  data.BlockCompressort writes into it and blockWirter reads from it. It is thread safe as long as there is only one consumer and one producer in two separate threads (not ready for multiple producer/consumers).
*
*
*/

#ifndef __KLB_CIRCULAR_DEQUEUE_H__
#define __KLB_CIRCULAR_DEQUEUE_H__

#include <condition_variable>
#include <mutex>
#include <atomic>

class klb_circular_dequeue
{
public:

	//variables
	std::condition_variable	g_writeWait;//so writer waits until there is a spot until the queue isfull
	std::mutex              g_lockWrite;//mutex to lock read/write resources

	//constructor / destructor
	klb_circular_dequeue(int blockSizeBytes_, int numBlocks_);
	klb_circular_dequeue(const klb_circular_dequeue& p);
	~klb_circular_dequeue();
	klb_circular_dequeue& operator=(const klb_circular_dequeue& p);//only works if blockSizeBytes and numBlocks are the same

	//main functions
	char* getReadBlock();//once you read the block it DOES NOT allow you to overwrite it. you have to call popReadBlock. Returns NULL if nothing is available
	void popReadBlock();//releases the oldest read block if there is any to release
	char* getWriteBlock();//return NULL if it cannot write because queue is full
	void pushWriteBlock();//indiciates block is ready to be written

protected:

private:	
	char* dataBuffer;//stores data. The size is blockSizeBytes*numBlocks
	const int blockSizeBytes;//number of bytes per block
	const int numBlocks;//number of blocks that can be stored
	int readIdx, writeIdx;//index within the dequeu to read / write next element
	int numTaken;//count number of elements taken so we avoid spill over
};


#endif //end of __KLB_CIRCULAR_DEQUEUE_H__