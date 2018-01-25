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
* 2016-04-27 Pierangelo.      @CREATED
*/

#ifndef _MAPPED_VOLUME_H
#define _MAPPED_VOLUME_H

#include "VirtualVolume.h" // ADDED
#include "MappingObjects.h"
#include <list>
#include <string>
#include "imBlock.h"

//FORWARD-DECLARATIONS
//class  Block;

//every object of this class has the default (1,2,3) reference system
class MappedVolume : public iim::VirtualVolume
{
	private:	
		//******OBJECT ATTRIBUTES******
        VirtualVolume * volume;  // 'feature' volume
        bool releaseVol;
        MappingViews  * map;   // Mapping transformation

        iim::ref_sys reference_system;       //reference system of the stored volume
        float  VXL_1, VXL_2, VXL_3;         //voxel dimensions of the stored volume


		//***OBJECT PRIVATE METHODS****
		MappedVolume(void);

		//Given the reference system, initializes all object's members using stack's directories hierarchy
        void init() throw (iim::IOException);

		//rotate stacks matrix around D axis (accepted values are theta=0,90,180,270)
		void rotate(int theta);

		//mirror stacks matrix along mrr_axis (accepted values are mrr_axis=1,2,3)
        void mirror(iim::axis mrr_axis);

		//extract spatial coordinates (in millimeters) of given Stack object reading directory and filenames as spatial coordinates
        void extractCoordinates(iim::Block* stk, int z, int* crd_1, int* crd_2, int* crd_3);

	public:
		//CONSTRUCTORS-DESTRUCTOR
        // objects passed to constructors will not be deallocated by the destructor
        MappedVolume(const char* _root_dir, MappingViews * _map, bool overwrite_mdata = false, bool save_mdata=true)  throw (iim::IOException);

        MappedVolume(VirtualVolume * _volume, MappingViews * _map, bool overwrite_mdata = false, bool save_mdata=true)  throw (iim::IOException);

		virtual ~MappedVolume(void) throw (iim::IOException);

		//GET methods
        float  getVXL_1(){return VXL_1;}
        float  getVXL_2(){return VXL_2;}
        float  getVXL_3(){return VXL_3;}
        iim::axis   getAXS_1(){return volume->getAXS_1();}
        iim::axis   getAXS_2(){return volume->getAXS_2();}
        iim::axis   getAXS_3(){return volume->getAXS_3();}
		iim::ref_sys getREF_SYS(){return reference_system;}

        // returns a unique ID that identifies the volume format
        std::string getPrintableFormat(){return iim::MAPPED_FORMAT;}
        
		//PRINT method
		void print( bool print_stacks = false );

 		// iannello returns the number of channels of images composing the volume
        void initChannels ( ) throw (iim::IOException);

       //loads given subvolume in a 1-D array of iim::real32 while releasing stacks slices memory when they are no longer needed
        iim::real32 *loadSubvolume_to_real32(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1)  throw (iim::IOException);

        //loads given subvolume in a 1-D array of iim::uint8 while releasing stacks slices memory when they are no longer needed
        iim::uint8 *loadSubvolume_to_UINT8(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1,
                                                   int *channels=0, int ret_type=iim::DEF_IMG_DEPTH) throw (iim::IOException, iom::exception);

		//releases allocated memory of stacks
		void releaseStacks(int first_file=-1, int last_file=-1);

    
    	// needed to enable the detection by the factory of volume format through use of the default constructor
        friend class iim::VirtualVolume; 

};

#endif //_MAPPED_VOLUME_H
