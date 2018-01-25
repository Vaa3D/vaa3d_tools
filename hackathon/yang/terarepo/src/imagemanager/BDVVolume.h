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
* 2015-11-30. Giulio.     @CREATED
*/

#ifndef _BDV_VOLUME_H
#define _BDV_VOLUME_H

#include "VirtualVolume.h"
#include <list>
#include <string>

//every object of this class has the default (1,2,3) reference system
class BDVVolume : public iim::VirtualVolume
{
	private:	
		//******OBJECT ATTRIBUTES******
		void *HDF5_descr;
		void *BDV_descr;
        float  VXL_1, VXL_2, VXL_3;         //voxel dimensions of the stored volume

		//***OBJECT PRIVATE METHODS****
		BDVVolume(void);

		//Given the reference system, initializes all object's members using stack's directories hierarchy
        //void init() throw (iim::IOException);

		//rotate stacks matrix around D axis (accepted values are theta=0,90,180,270)
		//void rotate(int theta);

		//mirror stacks matrix along mrr_axis (accepted values are mrr_axis=1,2,3)
        //void mirror(iim::axis mrr_axis);

		// iannello returns the number of channels of images composing the volume
		void initChannels ( ) throw (iim::IOException)  { } 

	public:
		//CONSTRUCTORS-DECONSTRUCTOR
        BDVVolume(const char* _root_dir, int res = 0, int tp = 0, void *BDV_descr = 0 )  throw (iim::IOException);

		virtual ~BDVVolume(void) throw (iim::IOException);

		//GET methods
        float  getVXL_1(){return VXL_1;}
        float  getVXL_2(){return VXL_2;}
        float  getVXL_3(){return VXL_3;}		
        iim::axis getAXS_1() {return iim::axis(1);}
        iim::axis getAXS_2() {return iim::axis(2);}
        iim::axis getAXS_3() {return iim::axis(3);}

        // @ADDED by Alessandro on 2016-12-19
        // return true if the given dimension is tiled
        virtual bool isTiled(iim::dimension d) {return false;}
        // return vector of tiles along x-y-z (empty vector if the volume is not tiled)
        virtual std::vector< iim::voi3D<size_t> > tilesXYZ() {return std::vector< iim::voi3D<size_t> >();}

        // returns a unique ID that identifies the volume format
        std::string getPrintableFormat(){return iim::BDV_HDF5_FORMAT;}


		//PRINT method
		//void print(  bool print_stacks = false );

        //loads given subvolume in a 1-D array of iim::real32 while releasing stacks slices memory when they are no longer needed
        inline iim::real32 *loadSubvolume_to_real32(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1)  throw (iim::IOException) {
			return loadSubvolume(V0,V1,H0,H1,D0,D1,0,true);
		}

        //loads given subvolume in a 1-D array and puts used Stacks into 'involved_stacks' iff not null
        iim::real32 *loadSubvolume(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1,
                                                                  std::list<iim::Block*> *involved_blocks = 0, bool release_blocks = false)  throw (iim::IOException);

        //loads given subvolume in a 1-D array of iim::uint8 while releasing stacks slices memory when they are no longer needed
        iim::uint8 *loadSubvolume_to_UINT8(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1,
                                                   int *channels=0, int ret_type=iim::DEF_IMG_DEPTH) throw (iim::IOException, iom::exception);

    	// needed to enable the detection by the factory of volume format through use of the default constructor
        friend class iim::VirtualVolume; 
        friend class VolumeConverter; 
};

#endif //_BDV_VOLUME_H
