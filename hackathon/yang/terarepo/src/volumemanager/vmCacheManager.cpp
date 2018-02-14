//------------------------------------------------------------------------------------------------
// Copyright (c) 2012  Alessandro Bria and Giulio Iannello (University Campus Bio-Medico of Rome).  
// All rights reserved.
//------------------------------------------------------------------------------------------------

/*******************************************************************************************************************************************************************************************
*    LICENSE NOTICE
********************************************************************************************************************************************************************************************
*    By downloading/using/running/editing/changing any portion of codes in this package you agree to this license. If you do not agree to this license, do not download/use/run/edit/change
*    this code.
********************************************************************************************************************************************************************************************
*    1. This material is free for non-profit research, but needs a special license for any commercial purpose. Please contact Alessandro Bria at a.bria@unicas.it or Giulio Iannello at 
*       g.iannello@unicampus.it for further details.
*    2. You agree to appropriately cite this work in your related studies and publications.
*
*       Bria, A., Iannello, G., "TeraStitcher - A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Microscopy Images", (2012) BMC Bioinformatics, 13 (1), art. no. 316.
*
*    3. This material is provided by  the copyright holders (Alessandro Bria  and  Giulio Iannello),  University Campus Bio-Medico and contributors "as is" and any express or implied war-
*       ranties, including, but  not limited to,  any implied warranties  of merchantability,  non-infringement, or fitness for a particular purpose are  disclaimed. In no event shall the
*       copyright owners, University Campus Bio-Medico, or contributors be liable for any direct, indirect, incidental, special, exemplary, or  consequential  damages  (including, but not 
*       limited to, procurement of substitute goods or services; loss of use, data, or profits;reasonable royalties; or business interruption) however caused  and on any theory of liabil-
*       ity, whether in contract, strict liability, or tort  (including negligence or otherwise) arising in any way out of the use of this software,  even if advised of the possibility of
*       such damage.
*    4. Neither the name of University  Campus Bio-Medico of Rome, nor Alessandro Bria and Giulio Iannello, may be used to endorse or  promote products  derived from this software without
*       specific prior written permission.
********************************************************************************************************************************************************************************************/

/******************
*    CHANGELOG    *
*******************
* 2016-08-28. Giulio. @CREATED.
*/

#include "vmCacheManager.h"


using namespace vm;

#define _MBYTE_   1048576.0

vm::CacheBuffer::CacheBuffer ( VirtualVolume *_volume, vm::uint64 _bufSizeMB, bool _enable, bool _printstats ) {
	volume = _volume;
	bufSizeMB = _bufSizeMB;

	NROWS      = volume->getN_ROWS();
	NCOLS      = volume->getN_COLS();
	NCHANS     = volume->getDIM_C();
	BYTESxCHAN = volume->getBYTESxCHAN();

	sliceHeight = volume->getStacksHeight();
	sliceWidth  = volume->getStacksWidth();
	sliceSizeMB = sliceHeight * sliceWidth * NCHANS * BYTESxCHAN / _MBYTE_;

	NSLICES = 256;
	while ( (NSLICES * 2) < (int) (bufSizeMB / sliceSizeMB) ) {
		NSLICES <<= 1;
	}
	iSliceMask = NSLICES - 1;

	buffers = new bufEntry **[NROWS];
	for ( int r=0; r<NROWS; r++ ) {
		buffers[r] = new bufEntry *[NCOLS];
		for ( int c=0; c<NCOLS; c++ ) {
			buffers[r][c] = new bufEntry [NSLICES];
			for ( int b=0; b<NSLICES; b++ ) {
				buffers[r][c][b].buf = (unsigned char *) 0;
				buffers[r][c][b].islice = -1;
			}
		}
	}

	cQueue = new SliceID[NSLICES]; // to avoid to maintain the number of IDs in the circular queue
	cQueueIn = cQueueOut = n_cached = 0;

	enabled = _enable; 
	printstats = _printstats;
	chits = cmisses = crplcmnts = 0;
}


vm::CacheBuffer::~CacheBuffer () {
	if ( buffers ) {
		for ( int r=0; r<NROWS; r++ ) {
			if ( buffers[r] ) {
				for ( int c=0; c<NCOLS; c++ ) {
					if ( buffers[r][c] ) {
						for ( int b=0; b<NSLICES; b++ ) {
							if ( buffers[r][c][b].buf )
								delete []buffers[r][c][b].buf;
						}
						delete []buffers[r][c];
					}
				}
				delete []buffers[r];
			}
		}
		delete []buffers;
	}

	if ( cQueue )
		delete []cQueue;

	if ( printstats ) {
		printf("Cache statistics: hits = %llu, misses = %llu, replacements = %llus \n",chits,cmisses,crplcmnts);
	}
}


bool vm::CacheBuffer::cacheSlice ( int row, int col, int islice, unsigned char *slice ) {

	if ( enabled ) {

		int idx;

		if ( n_cached == NSLICES ) { // cache full: free al least one slice
			// Deallocation policy (BEGIN)
			idx = cQueue[cQueueOut].islice & iSliceMask;
			delete buffers[cQueue[cQueueOut].row][cQueue[cQueueOut].col][idx].buf;
			buffers[cQueue[cQueueOut].row][cQueue[cQueueOut].col][idx].buf = (unsigned char *) 0;
			buffers[cQueue[cQueueOut].row][cQueue[cQueueOut].col][idx].islice = -1;
			cQueueOut++;
			if ( cQueueOut == NSLICES )
				cQueueOut = 0;
			n_cached--;
			crplcmnts++;
			// Deallocation policy (END)
		}

		// Deallocation policy (BEGIN)
		cQueue[cQueueIn].row    = row;
		cQueue[cQueueIn].col    = col;
		cQueue[cQueueIn].islice = islice;
		cQueueIn++;
		if ( cQueueIn == NSLICES )
			cQueueIn = 0;
		// Deallocation policy (END)

		idx = islice & iSliceMask;
		buffers[row][col][idx].buf = slice;
		buffers[row][col][idx].islice = islice;
		n_cached++;

		// Deallocation policy (BEGIN)
		return true;
		// Deallocation policy (END)
	}
	else
		return false;
}


bool vm::CacheBuffer::getSlice   ( int row, int col, int islice, unsigned char *&slice ) {

	if ( enabled ) {

		int idx = islice & iSliceMask;

		if ( buffers[row][col][idx].buf && (buffers[row][col][idx].islice == islice) ) {
			slice = buffers[row][col][idx].buf;
			chits++;
			return true;
		}
		else {
			cmisses++;
			return false;
		}
	}
	else {
		cmisses++;
		return false;
	}
}
