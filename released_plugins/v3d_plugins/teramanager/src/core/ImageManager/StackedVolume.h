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

#ifndef _STACKED_VOLUME_H
#define _STACKED_VOLUME_H

//#include "IM_defs.h"
//#include "MyException.h"
#include "VirtualVolume.h" // ADDED
#include <list>
#include <string>

#define STACKED_FORMAT "Stacked" // ADDED

//FORWARD-DECLARATIONS
class  Stack;

//******* ABSTRACT TYPES DEFINITIONS *******
struct VHD_triple{int V, H, D;};
struct interval_t
{
	int start, end;
	interval_t(void) :				   start(-1),	  end(-1)  {};
	interval_t(int _start, int _end) : start(_start), end(_end){};
};
enum axis {vertical=1, inv_vertical=-1, horizontal=2, inv_horizontal=-2, depth=3, inv_depth=-3, axis_invalid=0};
const char* axis_to_str(axis ax);
struct ref_sys 
{
	axis first, second, third; 
	ref_sys(axis _first, axis _second, axis _third) : first(_first), second(_second), third(_third){}
	ref_sys(const ref_sys &_rvalue) : first(_rvalue.first), second(_rvalue.second), third(_rvalue.third){}
	ref_sys(): first(axis_invalid), second(axis_invalid), third(axis_invalid){}
};

//every object of this class has the default (1,2,3) reference system
class StackedVolume : public VirtualVolume
{
	private:
		
/* 
        iannello ATTRBUTES OF BASE ABSTRACT CLASS

        char* stacks_dir;			    //C-string that contains the directory path of stacks matrix
		float  VXL_V, VXL_H, VXL_D;		//voxel dimensions (in microns) along V(Vertical), H(horizontal) and D(Depth) axes
		float  ORG_V, ORG_H, ORG_D;		//origin spatial coordinates (in millimeters) along VHD axes
		uint32 DIM_V, DIM_H, DIM_D;		//volume dimensions (in voxels) along VHD axes
*/

		//******OBJECT ATTRIBUTES******
		uint16 N_ROWS, N_COLS;			//dimensions (in stacks) of stacks matrix along VH axes
        Stack ***STACKS;			    //2-D array of <Stack*>
        ref_sys reference_system;       //reference system of the stored volume
        float  VXL_1, VXL_2, VXL_3;     //voxel dimensions of the stored volume

		//***OBJECT PRIVATE METHODS****
		StackedVolume(void);

		//Given the reference system, initializes all object's members using stack's directories hierarchy
        void init();

		//rotate stacks matrix around D axis (accepted values are theta=0,90,180,270)
		void rotate(int theta);

		//mirror stacks matrix along mrr_axis (accepted values are mrr_axis=1,2,3)
		void mirror(axis mrr_axis);

		//extract spatial coordinates (in millimeters) of given Stack object reading directory and filenames as spatial coordinates
		void extractCoordinates(Stack* stk, int z, int* crd_1, int* crd_2, int* crd_3);

		// iannello returns the number of channels of images composing the volume
		void initChannels ( ) throw (MyException);

	public:
		//CONSTRUCTORS-DECONSTRUCTOR
		StackedVolume(const char* _root_dir)  throw (MyException);
        StackedVolume(const char* _root_dir, ref_sys _reference_system,
					  float _VXL_1, float _VXL_2, float _VXL_3, 
					  bool overwrite_mdata = false, bool save_mdata=true)  throw (MyException);
		~StackedVolume(void);

		//GET methods
		// iannello char* getSTACKS_DIR(){return stacks_dir;}
		Stack*** getSTACKS(){return STACKS;}
		// iannello uint32 getDIM_V(){return DIM_V;}
		// iannello uint32 getDIM_H(){return DIM_H;}
		// iannello uint32 getDIM_D(){return DIM_D;}
        uint16 getN_ROWS(){return N_ROWS;}
        uint16 getN_COLS(){return N_COLS;}
		// iannello float  getVXL_V(){return VXL_V;}
		// iannello float  getVXL_H(){return VXL_H;}
		// iannello float  getVXL_D(){return VXL_D;}
		// iannello float  getORG_V(){return ORG_V;}
		// iannello float  getORG_H(){return ORG_H;}
		// iannello float  getORG_D(){return ORG_D;}
        int    getStacksHeight();
        int    getStacksWidth();
        float  getVXL_1(){return VXL_1;}
        float  getVXL_2(){return VXL_2;}
        float  getVXL_3(){return VXL_3;}
        axis   getAXS_1(){return reference_system.first;}
        axis   getAXS_2(){return reference_system.second;}
        axis   getAXS_3(){return reference_system.third;}


		//PRINT method
		void print();

		//saving-loading methods to/from metadata binary file
		void save(char* metadata_filepath) throw (MyException);
		void load(char* metadata_filepath) throw (MyException);

		//loads given subvolume in a 1-D array of REAL_T while releasing stacks slices memory when they are no longer needed
		inline REAL_T *loadSubvolume_to_REAL_T(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1)  throw (MyException) {
			return loadSubvolume(V0,V1,H0,H1,D0,D1,0,true);
		}

        //loads given subvolume in a 1-D array and puts used Stacks into 'involved_stacks' iff not null
        REAL_T *loadSubvolume(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1,
                                                                  std::list<Stack*> *involved_stacks = 0, bool release_stacks = false)  throw (MyException);

        //loads given subvolume in a 1-D array of uint8 while releasing stacks slices memory when they are no longer needed
        uint8 *loadSubvolume_to_UINT8(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1, int *channels=0)
																										throw (MyException);

		//saves given subvolume as a stack of 8-bit grayscale images in a directory created in the default path
		static void saveSubVolume(REAL_T* subvol, int V0, int V1, int H0, int H1, int D0, int D1, int V_idx, int H_idx, int D_idx);

		//saves in 'dir_path' the selected subvolume in stacked format, with the given stacks dimensions
		void saveVolume(const char* dir_path, uint32 max_slice_height=0, uint32 max_slice_width=0, 
			            uint32 V0=0, uint32 V1=0, uint32 H0=0, uint32 H1=0, uint32 D0=0, uint32 D1=0, 
						const char* img_format=IM_DEF_IMG_FORMAT, int img_depth=IM_DEF_IMG_DEPTH) throw (MyException);

		//saves in 'dir_path' the selected subvolume in multi-stack format, with the exact given stacks dimensions and the given overlap between adjacent stacks
		void saveOverlappingStacks(char* dir_path, uint32 slice_height, uint32 slice_width,	uint32 overlap_size,
						uint32 V0=0, uint32 V1=0, uint32 H0=0, uint32 H1=0, uint32 D0=0, uint32 D1=0) throw (MyException);

		//saves the Maximum Intensity Projections (MIP) of the selected subvolume along the selected direction into the given paths
		void saveMIPs(bool direction_V = true, bool direction_H = true, bool direction_D = true, 
			          char* MIP_V_path = NULL, char* MIP_H_path = NULL, char* MIP_D_path = NULL,
					  uint32 V0=0, uint32 V1=0, uint32 H0=0, uint32 H1=0, uint32 D0=0, uint32 D1=0) throw (MyException);


		//shows the selected slice with a simple GUI
		void show(REAL_T *vol, int vol_DIM_V, int vol_DIM_H, int D_index, int window_HEIGHT=0, int window_WIDTH=0);

		//deletes slices from disk from first_file to last_file, extremes included
		void deleteSlices(int first_file, int last_file);

		//releases allocated memory of stacks
		void releaseStacks(int first_file=-1, int last_file=-1);

		//returns true if file exists at the given filepath
		static bool fileExists(const char *filepath);

};

#endif //_STACKED_VOLUME_H
