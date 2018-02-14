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
* 2017-08-19. Giulio.     @ADDED getter for data member 'enabled'
* 2017-04-18. Alessandro. @ADDED constructor that directly takes vm::VirtualVolume in input, and added 'volume_external' attribute
* 2017-04-01. Giulio.     @ADDED support for multi-layer stitching
* 2016-09-13. Giulio.     @ADDED a cache manager to store stitched subregions
* 2016-06-19. Giulio.     @FIXED bug in the call to input plugin (introduced the information on the plugin type: 2D/3D)
* 2016-05-03. Giulio.     @ADDED field to set the blending algorithm (default: sinousidal blenging) 
* 2016-04-07. Giulio.     @MODIFIED moved default constructor among private methods and declared VirtualVolume a friend class 
* 2016-03-23. Giulio.     @ADDED  offsets of unstitched volume with respect to nominal origin (0,0,0)
* 2015-02-18. Giulio.     @CREATED  
*/

#ifndef _UNSTITCHED_VOLUME_H
#define _UNSTITCHED_VOLUME_H

#include "VirtualVolume.h" 
#include <list>
#include <string>
#include <deque>

#include "../volumemanager/volumemanager.config.h"
#include "../stitcher/StackStitcher.h"


struct coord_2D{int V,H;};
struct stripe_2Dcoords{coord_2D up_left, bottom_right;};
struct stripe_corner{int h,H; bool up;};
struct stripe_2Dcorners{std::list<stripe_corner> ups, bottoms, merged;};
extern bool compareCorners (stripe_corner first, stripe_corner second);


//every object of this class has the default (1,2,3) reference system
class UnstitchedVolume : public iim::VirtualVolume
{
	protected:

		volumemanager::VirtualVolume *volume;
		bool volume_external;				 // whether 'volume' has been provided by an external caller (in which case it must not be deallocated / modified)
		StackStitcher *stitcher;

		iim::ref_sys reference_system;       //reference system of the stored volume

		stripe_2Dcoords  *stripesCoords;
		stripe_2Dcorners *stripesCorners;

		// offests of indices in the unstitched volume
		int V0_offs;
		int H0_offs;
		int D0_offs;

		int blending_algo;

		std::string plugin_type;

		iim::CacheBuffer *cb;
		int current_channel;
		bool internal_buffer_deallocate;

		//***OBJECT PRIVATE METHODS****
		UnstitchedVolume(void);

		//Given the reference system, initializes all object's members using stack's directories hierarchy
        void init();

		//rotate stacks matrix around D axis (accepted values are theta=0,90,180,270)
		void rotate(int theta);

		//mirror stacks matrix along mrr_axis (accepted values are mrr_axis=1,2,3)
        void mirror(iim::axis mrr_axis);

		//extract spatial coordinates (in millimeters) of given Stack object reading directory and filenames as spatial coordinates
        void extractCoordinates(iim::Block* stk, int z, int* crd_1, int* crd_2, int* crd_3);

		// iannello returns the number of channels of images composing the volume
        void initChannels ( ) throw (iim::IOException);

        //loads given subvolume in a 1-D array of iim::real32; since the loaded subvolume can have slightly different vertices, returns the actual 
		//vertices of the subvolume in (VV0, VV1, HH0, HH1, DD0, DD1)
		iim::real32 *internal_loadSubvolume_to_real32(int &VV0, int &VV1, int &HH0, int &HH1, int &DD0, int &DD1, 
													  int V0=-1, int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1)  throw (iim::IOException);

	public:

		// constructor 1 (from source folder)
        UnstitchedVolume(const char* _root_dir, bool cacheEnabled = true, int _blending_algo = S_SINUSOIDAL_BLENDING )  throw (iim::IOException);
		
		// constructor 2 @ADDED by Alessandro on 2014-04-18: takes an external vm::VirtualVolume in input
		UnstitchedVolume(vm::VirtualVolume * _imported_volume, bool cacheEnabled = true, int _blending_algo = S_SINUSOIDAL_BLENDING )  throw (iim::IOException);

		// destructor
		virtual ~UnstitchedVolume(void) throw (iim::IOException);

		//GET methods
        float  getVXL_1(){return VXL_V;}
        float  getVXL_2(){return VXL_H;}
        float  getVXL_3(){return VXL_D;}
        iim::axis   getAXS_1(){return reference_system.first;}
        iim::axis   getAXS_2(){return reference_system.second;}
        iim::axis   getAXS_3(){return reference_system.third;}
		iim::ref_sys getREF_SYS(){return reference_system;}

        // @ADDED by Alessandro on 2016-12-19
        // return true if the given dimension is tiled
        virtual bool isTiled(iim::dimension d) {return false;}
        // return vector of tiles along x-y-z (empty vector if the volume is not tiled)
        virtual std::vector< iim::voi3D<size_t> > tilesXYZ() {return std::vector< iim::voi3D<size_t> >();}

		//SET methods
		void setBLENDING_ALGO(int _blending_algo){blending_algo = _blending_algo;}

		/* update members controlling the volume size and the mapping from indices of unstitched volume (that may start from a negative value) 
		 * to indices of stitched volume (starting from 0)
		 */
		void updateTilesPositions ( );

        // returns a unique ID that identifies the volume format
        std::string getPrintableFormat(){ return iim::UNST_TIF3D_FORMAT; }
        
		//PRINT method
		void print();

        //loads given subvolume in a 1-D array of iim::real32
        iim::real32 *loadSubvolume_to_real32(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1)  throw (iim::IOException);

        //loads given subvolume in a 1-D array of iim::uint8 while releasing stacks slices memory when they are no longer needed
        iim::uint8 *loadSubvolume_to_UINT8(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1,
                                                   int *channels=0, int ret_type=iim::DEF_IMG_DEPTH) throw (iim::IOException, iom::exception);

    	// needed to enable the detection by the factory of volume format through use of the default constructor
        friend class iim::VirtualVolume; 

		// needed to enable the extraction of the underlying unstitiched volume
		friend class MultiLayersVolume; 
		friend class StackStitcher2; 
		friend class TPAlgo2MST; 
		friend class iim::CacheBuffer; 
};



class iim::CacheBuffer {
private:
	bool enabled;
	bool printstats;
	iim::uint64 caccesses;
	iim::uint64 chits;
	iim::uint64 cmisses;
	iim::uint64 crplcmnts;

	UnstitchedVolume *volume;
	iim::uint64 bufSizeMB;   // maximum size of cache in MB
	double cachedMB;         // currently cached in MB
	double maxCachedMB;      // maximum actually cached in MB

	int N_CHANS;

	struct bufID;
	
	struct bufEntry {
		iom::real_t *buf;
		uint64 ts;
		bool valid;
		bufID *storyEntry;
		int chan;
		int VV0;
		int VV1; 
		int HH0;
		int HH1; 
		int DD0;
		int DD1; 
		double sizeMB;
	};

	std::deque<bufEntry> *dq_buffers;
	std::deque<bufEntry>::iterator dq_it;
	bool it_active;
	uint64 timestamp;

	/************************************************************************************* 
	 * data structure to manage the deallocation policy
	 * simple circular queue to implement a FIFO policy
	 *************************************************************************************/
	// deque of buffer IDs 
	struct bufID {
		uint64 ts;
		bool valid;
		int chan;
		bufEntry *bufs_entry;
	};
	std::deque<bufID> bufStory; // arranged in decreasing order of timestamps (first the most recent ones)
	/*************************************************************************************/ 

	bool scan_sbvID ( int VV0, int VV1, int HH0, int HH1, int DD0, int DD1, bufEntry *e, bool &found );
	/* return true if either buffer e is allocated and it contains the subvolume or it is smaller than the subvolume, false otherwise;
	 * found is true if buffer e is allocated it contains the subvolume, false otherwise 
	 */

	bool wider_sbvID ( int VV0, int VV1, int HH0, int HH1, int DD0, int DD1, bufEntry *e );
	/* return true if the buffer e is contained in the subvolume */

public:

	CacheBuffer ( UnstitchedVolume *_volume,  iim::uint64 _bufSizeMB = 1024, bool _enable = false, bool _printstats = false );

	~CacheBuffer ();

	void enableCache ( )  { enabled = true; }
	void disableCache ( ) { enabled = false; }
	
	// 2017-08-19. Giulio @ADDED
	bool getENABLED ( ) { return enabled; }

	void resetCounts ( ) { caccesses = chits = cmisses = crplcmnts = 0; }
	void getCounts ( vm::uint64 &_caccesses, vm::uint64 &_chits, vm::uint64 &_cmisses, vm::uint64 &_crplcmnts ) { 
		_caccesses = caccesses; _chits = chits; _cmisses = cmisses; _crplcmnts = crplcmnts; 
	}

	bool cacheSubvolume ( int chan, int VV0, int VV1, int HH0, int HH1, int DD0, int DD1, iom::real_t *sbv, iim::sint64 bufsize );
	/* cache the buffer sbv of the subvolume of size voxels (the size in bytes is obtained multiplying by the size of iom::real_t)
	 * defined by vertices (VV0,HH0,DD0) and (VV1,HH1,DD1); the subvolume belongs to channel chan
	 * older buffers may be disposed if the cache is full
	 * return true if caching succeded, false otherwise
	 */

	bool getSubvolume   ( int chan, int &VV0, int &VV1, int &HH0, int &HH1, int &DD0, int &DD1, iom::real_t *&sbv );
	/* returns a buffer containing the subvolume of channel chan and defined by vertices (VV0,HH0,DD0) and (VV1,HH1,DD1) if it
	 * found in the cache
	 * return true if the buffer has been found, false otherwise
	 */
};


#endif //_UNSTITCHED_VOLUME_H
