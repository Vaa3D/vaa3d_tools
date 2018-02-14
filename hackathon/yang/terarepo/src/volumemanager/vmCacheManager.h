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
* 2016-08-28. Giulio. @CREATED
*/

#ifndef _VM_CACHEMANAGER_H
#define _VM_CACHEMANAGER_H

#include "vmVirtualVolume.h"


class vm::CacheBuffer {
private:
	bool enabled;
	bool printstats;
	vm::uint64 chits;
	vm::uint64 cmisses;
	vm::uint64 crplcmnts;

	vm::VirtualVolume *volume;
	vm::uint64 bufSizeMB;   // maximum size of cache in MB
	double sliceSizeMB; // size of one slice of any stack in MB
	
	struct bufEntry {
		unsigned char *buf;
		int   islice;
	};
	bufEntry ***buffers;       // matrix of list of pointer to buffers
	int NROWS;              // number of row of the matrix
	int NCOLS;              // number of colums of the matrix
	int NSLICES;            // maximum number of slices per stack that can be cached (must be a power of 2)
	int iSliceMask;         // mask to hash the slice index (must be NSLICE-1)

	vm::uint64 sliceHeight; // height of any slice
	vm::uint64 sliceWidth;  // width of any slice
	int NCHANS;             // number of channels
	int BYTESxCHAN;         // number of bytes per channel

	/************************************************************************************* 
	 * data structure to manage the deallocation policy
	 * simple circular queue to implement a FIFO policy
	 *************************************************************************************/
	// circular queue of slices IDs 
	struct SliceID {
		int row;
		int col;
		int islice;
	};
	SliceID *cQueue;
	int cQueueIn;
	int cQueueOut;
	int n_cached;           // number of slice cached

	bool cqueueEmpty ( ) { return n_cached == 0; }
	bool cqueueFull  ( ) { return n_cached == NSLICES; }
	/*************************************************************************************/ 

public:

	CacheBuffer ( vm::VirtualVolume *_volume, vm::uint64 _bufSizeMB = 1024, bool _enable = false, bool _printstats = false );

	~CacheBuffer ();

	void enableCache ( )  { enabled = true; }
	void disableCache ( ) { enabled = false; }

	void resetCounts ( ) { chits = cmisses = crplcmnts = 0; }
	void getCounts ( vm::uint64 &_chits, vm::uint64 &_cmisses, vm::uint64 &_crplcmnts ) { 
		_chits = chits; _cmisses = cmisses; _crplcmnts = crplcmnts; 
	}

	bool cacheSlice ( int row, int col, int islice, unsigned char *slice );
	bool getSlice   ( int row, int col, int islice, unsigned char *&slice );
};

#endif
