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
* 2014-12-06. Giulio    . @ADDED createDirectoryHiererchy method.
* 2014-09-12. Alessandro. @ADDED [z0, z1] subdata selection along Z in the 'computeDisplacements()' method.
* 2014-09-09. Alessandro. @CHANGED. Black pixels (=0) are ignored in 'sinusoidal_blending()' function (especially useful in 'sparse data' mode).
* 2014-09-02. Alessandro. @FIXED major bug in 'getStripe()' method. Argument 2 ('d_index') has 'short' type, but it should have at least 'int'.
*/

#ifndef STACK_STITCHER_H
#define STACK_STITCHER_H

#include <math.h>
#include "S_config.h"
//#include "../VolumeManager/volumemanager.config.h"
#include "iomanager.config.h"

#ifdef max
#undef max
#endif

class StackRestorer;

// #ifndef _VIRTUAL_VOLUME_H
// class volumemanager::VirtualVolume;
// #endif
// #ifndef STACK_H_
// class VirtualStack;
// #endif

#include "../volumemanager/vmVirtualStack.h"
#include "../volumemanager/vmVirtualVolume.h"

class StackStitcher
{
	private:

		/******OBJECT MEMBERS******/
        //StackedVolume *volume;					//pointer to the <StackedVolume> object to be stitched
		volumemanager::VirtualVolume *volume;		//pointer to the <VirtualVolume> object to be stitched
        int V0, V1, H0, H1, D0, D1;					//voxel intervals that identify the final stitched volume
        int ROW_START, COL_START, ROW_END, COL_END; //stack indexes that identify the stacks involved in stitching

		/******CLASS MEMBERS******/
		static double time_displ_comp;				//time employed for pairwise displacements computation
        static double time_merging;					//time employed to merge stacks
		static double time_stack_desc;				//time employed to compute stacks descriptions
		static double time_stack_restore;			//time employed to restore stacks
		static double time_multiresolution;			//time employed to obtain stitched volume at different resolutions


		/***OBJECT PRIVATE METHODS****/

		//default constructor will not be accessible
		StackStitcher(void){}


		/*************************************************************************************************************
		* Merges all slices of the given row at the given depth index, so obtaining the stripe that is returned.
		* Uses [...]_blending() functions to blend pixels in  overlapping zones.  The appropriate blending function is
		* selected by the [blending_algo] parameter. If a  <StackRestorer>  object has been passed,  each slice is re-
		* stored before it is combined into the final stripe.
		**************************************************************************************************************/
		iom::real_t* getStripe(int row_index, int d_index, int restore_direction=-1, StackRestorer* stk_rst=NULL,
						  int blending_algo=S_SINUSOIDAL_BLENDING)    							   throw (iom::exception);

		/*************************************************************************************************************
		* Returns the (up = true -> TOP, up = false -> BOTTOM) V coordinate of the virtual stripe at <row_index> row. 
		**************************************************************************************************************/
		int getStripeABS_V(int row_index, bool up);



		/***CLASS PRIVATE METHODS****/

		/*************************************************************************************************************
		* Blending functions that returns the value at <angle> blending <pixel1> and <pixel2>.
		* <angle> is in [0, PI] where 0 occurs at the first pixel of the overlapping zone and PI at last pixel of  the 
		* overlapping zone. IMPORTANT: due to efficiency reasons, it is better to handle  different  types of blending 
		* functions with function pointers instead of using polymorphism of OOP.
		**************************************************************************************************************/
		static inline iom::real_t sinusoidal_blending(double& angle, iom::real_t& pixel1, iom::real_t& pixel2){

			// 2014-09-09. Alessandro. @CHANGED. Black pixels (=0) are ignored in 'sinusoidal_blending()' function.
			if(!pixel1 || !pixel2)
				return std::max(pixel1, pixel2);

			return (iom::real_t)(  ((cos(angle)+1.0F)*0.5F)*pixel1 + ( 1.0F - ((cos(angle)+1.0F)*0.5F))*pixel2  );
		}

		static inline iom::real_t no_blending(double& angle, iom::real_t& pixel1, iom::real_t& pixel2){
			return (angle <= S_PI/2 ? pixel1 : pixel2);		
		}

        /*************************************************************************************************************
        * This is a special blending function (together with the necessary static variables) which draws blank lines
        * along stacks borders without performing any blending. This enables easy checking of motorized stages coordi-
        * nates precision.
        **************************************************************************************************************/
        static double stack_marging_old_val;
        static bool blank_line_drawn;
        static inline iom::real_t stack_margin(double& angle, iom::real_t& pixel1, iom::real_t& pixel2)
        {
            if(angle == stack_marging_old_val && blank_line_drawn){
                stack_marging_old_val = angle;
                return 1;
            }
            else if(angle > S_PI/2.0 && stack_marging_old_val < S_PI/2.0){
                stack_marging_old_val = angle;
                blank_line_drawn = true;
                return 1;
            }
            else{
                stack_marging_old_val = angle;
                blank_line_drawn = false;
                return ( angle <= S_PI/2 ? pixel1 : pixel2);
            }
		}
		
		/*************************************************************************************************************
		* Performs downsampling at a halved frequency on the given 3D image.  The given image is overwritten in order
		* to store its halvesampled version without allocating any additional resources.
		**************************************************************************************************************/
		static void halveSample(iom::real_t* img, int height, int width, int depth);

	public:

        StackStitcher(volumemanager::VirtualVolume* _volume);

		// compute pairwise displacements
		// 2014-09-12. Alessandro. @ADDED [z0, z1] subdata selection along Z in the 'computeDisplacements()' method.
		void computeDisplacements(
			int algorithm_type,							// ID of the pairwise displacement algorithm to be used.
			int row0 = -1,								// subdata selection along X: [row0, row1] rows will be processed only
			int col0 = -1,								// subdata selection along Y: [col0, col1] cols will be processed only
			int row1 = -1,								// subdata selection along X: [row0, row1] rows will be processed only
			int col1 = -1,								// subdata selection along Y: [col0, col1] cols will be processed only
			int overlap_V = -1,							// overlaps along V and H directions. If not given, default values are ...
			int overlap_H =-1,							// ... computed using the <MEC_...> members of the <StackedVolume> object.			
			int displ_max_V=S_DISPL_SEARCH_RADIUS_DEF,  // maximum displacements along VHD between two  adjacent stacks. ...
			int displ_max_H=S_DISPL_SEARCH_RADIUS_DEF,  // ... If not given, value S_DISPL_SEARCH_RADIUS_DEF is assigned.
			int displ_max_D=S_DISPL_SEARCH_RADIUS_DEF, 
			int subvol_DIM_D = S_SUBVOL_DIM_D_DEFAULT,	// dimension of layers obtained by dividing the volume along D.
			bool restoreSPIM=false,						// enable SPIM artifacts removal (zebrated patterns) ...
			int restore_direction=-1,					// ... along the given direction.
			bool show_progress_bar=true,				// enable/disable progress bar with estimated time remaining
			int z0=-1,									// subdata selection along Z: [z0, z1] slices will be processed only
			int z1=-1)									// subdata selection along Z: [z0, z1] slices will be processed only
		throw (iom::exception);


		/*************************************************************************************************************
		* For each stack, the vector of redundant displacements along D is projected into the displacement which embe-
		* ds the most reliable parameters. After this operation, such vector will contain only the projected displace-
                * ment. Where for a pair of adjacent stacks no displacement is available,  a displacement  is generated using
                * nominal stage coordinates.
		**************************************************************************************************************/
		void projectDisplacements()																  throw (iom::exception);

		/*************************************************************************************************************
		* Assuming that for each pair of adjacent stacks  exists one  and only one displacement,  this displacement is 
		* thresholded according to the given <reliability_threshold>. When a displacement is not reliable enough,  its
		* parameters are set to default values (i.e. nominal motorized stage coordinates).
		* Moreover, stacks which do not have any reliable single-direction displacements with all 4 neighbors are mar-
		* ked as NON STITCHABLE.
		**************************************************************************************************************/
		void thresholdDisplacements(float reliability_threshold)								  throw (iom::exception);


		/*************************************************************************************************************
		* Executes the compute tiles placement algorithm associated to the given ID <algorithm_type>
		**************************************************************************************************************/
		void computeTilesPlacement(int algorithm_type)											  throw (iom::exception);


        /*************************************************************************************************************
        * Computes final stitched volume dimensions assuming that current <StackedVolume> object contains  the correct
        * stack coordinates. The given parameters identify the possible VOI (Volume Of Interest). If these are not us-
        * ed, the whole volume is  considered and the parameter <exclude_nonstitchable_stacks> is used to discard rows
        * or columns with no stitchable stacks
        **************************************************************************************************************/
        void computeVolumeDims(bool exclude_nonstitchable_stacks = true, int _ROW_START = -1,	   int _ROW_END = -1,
                                                    int _COL_START = -1, int _COL_END = -1, int _D0 = -1, int _D1 = -1) throw (iom::exception);

		/*************************************************************************************************************
		* Method to be called for tile merging. <> parameters are mandatory, while [] are optional.
		* <output_path>			: absolute directory path where merged tiles have to be stored.
		* [slice_height/width]	: desired dimensions of tiles  slices after merging.  It is actually an upper-bound of
		*						  the actual slice dimensions, which will be computed in such a way that all tiles di-
		*						  mensions can differ by 1 pixel only along both directions. If not given, the maximum
		*						  allowed dimensions will be set, which will result in a volume composed by  one large 
		*						  tile only.
		* [resolutions]			: pointer to an array of S_MAX_MULTIRES  size which boolean entries identify the acti-
		*						  vaction/deactivation of the i-th resolution.  If not given, all resolutions will  be
		*						  activated.
		* [exclude_nonstitc...] 
		* [_...START/END]		
		* [_D0/_D1]				: identify the possible VOI (Volume Of Interest). If these are not used, the whole vo-
		*						  lume is  considered and the parameter <exclude_nonstitchable_stacks> is used to dis-
		*						  card rows or columns with no stitchable stacks.
		* [restoreSPIM]			: enables SPIM artifacts removal (zebrated patterns) along the given direction.
		* [restore_direction]	: direction of SPIM zebrated patterns to be removed.
		* [blending_algo]		: ID of the blending algorithm to be used in the overlapping regions.		 
		* [test_mode]			: if enabled, the middle slice of the whole volume will be stitched and and  saved lo-
		*						  cally. Stage coordinates will be used, s o this can be used to test  their precision
		*						  as well as the selected reference system.
		* [show_progress_bar]	: enables/disables progress bar with estimated time remaining.
		* [saved_img_format]	: determines saved images format ("png","tif","jpeg", etc.).
		* [saved_img_depth]		: determines saved images bitdepth (16 or 8).
		**************************************************************************************************************/
		void mergeTiles(std::string output_path, int slice_height = -1, int slice_width = -1, bool* resolutions = NULL, 
						bool exclude_nonstitchable_stacks =true, int _ROW_START=-1, int _ROW_END=-1, int _COL_START=-1,
						int _COL_END=-1, int _D0=-1, int _D1=-1,	bool restoreSPIM=false,	  int restore_direction=-1,
						int blending_algo=S_SINUSOIDAL_BLENDING,	bool test_mode=false, bool show_progress_bar= true,
						const char* saved_img_format=iom::DEF_IMG_FORMAT.c_str(), int saved_img_depth=iom::DEF_BPP) throw (iom::exception);
		

		/*************************************************************************************************************
		* Functions used to save single phase time performances
		**************************************************************************************************************/
		static void saveComputationTimes(const char *filename, volumemanager::VirtualVolume &stk_org, double total_time=-1);

        /*************************************************************************************************************
        * Get methods
        **************************************************************************************************************/
        int getV0(){return V0;}
        int getV1(){return V1;}
        int getH0(){return H0;}
        int getH1(){return H1;}
        int getD0(){return D0;}
        int getD1(){return D1;}
        int getROW0(){return ROW_START;}
        int getROW1(){return ROW_END;}
        int getCOL0(){return COL_START;}
        int getCOL1(){return COL_END;}

        /*************************************************************************************************************
        * Functions used to obtain absolute coordinates at different resolutions from relative coordinates
        **************************************************************************************************************/
        int getMultiresABS_V(int res, int REL_V);
        std::string getMultiresABS_V_string(int res, int REL_V);
        int getMultiresABS_H(int res, int REL_H);
        std::string getMultiresABS_H_string(int res, int REL_H);
        int getMultiresABS_D(int res, int REL_D);
        std::string getMultiresABS_D_string(int res, int REL_D);

		/*************************************************************************************************************
		* Method to be called for tile merging. <> parameters are mandatory, while [] are optional.
		* <output_path>			: absolute directory path where merged tiles have to be stored.
		* [block_height/width/depth]: desired dimensions of tiles  slices after merging.  It is actually an upper-bound of
		*						  the actual slice dimensions, which will be computed in such a way that all tiles di-
		*						  mensions can differ by 1 pixel only along both directions. If not given, the maximum
		*						  allowed dimensions will be set, which will result in a volume composed by  one large 
		*						  tile only.
		* [resolutions]			: pointer to an array of S_MAX_MULTIRES  size which boolean entries identify the acti-
		*						  vaction/deactivation of the i-th resolution.  If not given, all resolutions will  be
		*						  activated.
		* [exclude_nonstitc...] 
		* [_...START/END]		
		* [_D0/_D1]				: identify the possible VOI (Volume Of Interest). If these are not used, the whole vo-
		*						  lume is  considered and the parameter <exclude_nonstitchable_stacks> is used to dis-
		*						  card rows or columns with no stitchable stacks.
		* [restoreSPIM]			: enables SPIM artifacts removal (zebrated patterns) along the given direction.
		* [restore_direction]	: direction of SPIM zebrated patterns to be removed.
		* [blending_algo]		: ID of the blending algorithm to be used in the overlapping regions.
		* [test_mode]			: if enabled, the middle slice of the whole volume will be stitched and and  saved lo-
		*						  cally. Stage coordinates will be used, s o this can be used to test  their precision
		*						  as well as the selected reference system.
		* [show_progress_bar]	: enables/disables progress bar with estimated time remaining.
		* [saved_img_format]	: determines saved images format ("png","tif","jpeg", etc.).
		* [saved_img_depth]		: determines saved images bitdepth (16 or 8).
		* [par_mode]            : if enabled, do not take into account parameters _D0/_D1 is determining the mane of 
		*                         the directories and do not generates the file mdata.bin in the output volume
		**************************************************************************************************************/

		void mergeTilesVaa3DRaw(std::string output_path, int block_height = -1, int block_width = -1, int block_depth = -1, bool* resolutions = NULL, 
								bool exclude_nonstitchable_stacks =true, int _ROW_START=-1, int _ROW_END=-1, int _COL_START=-1,
								int _COL_END=-1, int _D0=-1, int _D1=-1,	bool restoreSPIM=false,	  int restore_direction=-1,
								int blending_algo=S_SINUSOIDAL_BLENDING,	bool test_mode=false, bool show_progress_bar= true,
								const char* saved_img_format=iom::DEF_IMG_FORMAT.c_str(), int saved_img_depth=iom::DEF_BPP, bool par_mode=false) throw (iom::exception);


		/*************************************************************************************************************
		* Method to be called for creating the directory hierarchy without actually saving image tiles.
		* It can be used to perform the mergeTiles operation on different volume portions in parallel
		* Has must be called with the same parameters as mergeTilesVaa3DRaw
		**************************************************************************************************************/
		void createDirectoryHierarchy (std::string output_path, int block_height = -1, int block_width = -1, int block_depth = -1, bool* resolutions = NULL, 
								bool exclude_nonstitchable_stacks =true, int _ROW_START=-1, int _ROW_END=-1, int _COL_START=-1,
								int _COL_END=-1, int _D0=-1, int _D1=-1,	bool restoreSPIM=false,	  int restore_direction=-1,
								int blending_algo=S_SINUSOIDAL_BLENDING,	bool test_mode=false, bool show_progress_bar= true,
								const char* saved_img_format=iom::DEF_IMG_FORMAT.c_str(), int saved_img_depth=iom::DEF_BPP, bool par_mode=false) throw (iom::exception);

		/*************************************************************************************************************
		* Method to be called for creating the metadata file of the output volume starting from the importad unstitched
		* volume. It can be used to perform the mergeTiles operation on different volume portions in parallel
		* Has must be called with the same parameters as mergeTilesVaa3DRaw
		**************************************************************************************************************/
		void mdataGenerator (std::string output_path, int block_height = -1, int block_width = -1, int block_depth = -1, bool* resolutions = NULL, 
								bool exclude_nonstitchable_stacks =true, int _ROW_START=-1, int _ROW_END=-1, int _COL_START=-1,
								int _COL_END=-1, int _D0=-1, int _D1=-1,	bool restoreSPIM=false,	  int restore_direction=-1,
								int blending_algo=S_SINUSOIDAL_BLENDING,	bool test_mode=false, bool show_progress_bar= true,
								const char* saved_img_format=iom::DEF_IMG_FORMAT.c_str(), int saved_img_depth=iom::DEF_BPP, bool par_mode=false) throw (iom::exception);
};

#endif


