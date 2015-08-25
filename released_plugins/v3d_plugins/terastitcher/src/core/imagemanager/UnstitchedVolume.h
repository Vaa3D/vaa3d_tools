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
* 2015-02-18. Giulio.     @CREATED  
*/

#ifndef _UNSTITCHED_VOLUME_H
#define _UNSTITCHED_VOLUME_H

#include "VirtualVolume.h" 
#include <list>
#include <string>

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
	private:

		volumemanager::VirtualVolume* volume;
		StackStitcher* stitcher;

		iim::ref_sys reference_system;       //reference system of the stored volume

		stripe_2Dcoords  *stripesCoords;
		stripe_2Dcorners *stripesCorners;


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
		//CONSTRUCTORS-DECONSTRUCTOR
        UnstitchedVolume(const char* _root_dir)  throw (iim::IOException);

		~UnstitchedVolume(void);

		//GET methods
        float  getVXL_1(){return VXL_V;}
        float  getVXL_2(){return VXL_H;}
        float  getVXL_3(){return VXL_D;}
        iim::axis   getAXS_1(){return reference_system.first;}
        iim::axis   getAXS_2(){return reference_system.second;}
        iim::axis   getAXS_3(){return reference_system.third;}

        // returns a unique ID that identifies the volume format
        std::string getPrintableFormat(){ return iim::UNST_TIF3D_FORMAT; }
        
		//PRINT method
		void print();

        //loads given subvolume in a 1-D array of iim::real32
        iim::real32 *loadSubvolume_to_real32(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1)  throw (iim::IOException);

        //loads given subvolume in a 1-D array of iim::uint8 while releasing stacks slices memory when they are no longer needed
        iim::uint8 *loadSubvolume_to_UINT8(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1,
                                                   int *channels=0, int ret_type=iim::DEF_IMG_DEPTH) throw (iim::IOException);
};

#endif //_UNSTITCHED_VOLUME_H
