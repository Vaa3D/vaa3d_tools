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
* 2017-04-01.  Giulio.     @ADDED code for completing the management of multi-layer stitching
* 2017-02-10.  Giulio.     @ADDED in merge methods added a parameter to specify the blending algorithm to be used for layers 
*/


#ifndef STACK_STITCHER2_H
#define STACK_STITCHER2_H

#include <math.h>
#include "S_config.h"
#include "IM_config.h"
#include "MultiLayers.h"
//#include "IOManager_defs.h"
#include "StackRestorer.h" // to be eliminated ?



class StackStitcher2
{
private:

	/******OBJECT MEMBERS******/
	//StackedVolume *volume;					//pointer to the <StackedVolume> object to be stitched
	MultiLayersVolume *volume;					//pointer to the <VirtualVolume> object to be stitched
	int V0, V1, H0, H1, D0, D1;				//voxel intervals that identify the final stitched volume
	int ROW_START, COL_START, ROW_END, COL_END;             //stack indexes that identify the stacks involved in stitching

	/******CLASS MEMBERS******/
	static double time_displ_comp;				//time employed for pairwise displacements computation
	static double time_merging;				//time employed to merge stacks
	static double time_stack_desc;				//time employed to compute stacks descriptions
	static double time_stack_restore;			//time employed to restore stacks
	static double time_multiresolution;			//time employed to obtain stitched volume at different resolutions

	/***OBJECT PRIVATE METHODS****/

	//default constructor will not be accessible
	StackStitcher2(void){}

	/*************************************************************************************************************
	* Merges all slices of the given row at the given depth index, so obtaining the stripe that is returned.
	* Uses [...]_blending() functions to blend pixels in  overlapping zones.  The appropriate blending function is
	* selected by the [blending_algo] parameter. If a  <StackRestorer>  object has been passed,  each slice is re-
	* stored before it is combined into the final stripe.
	**************************************************************************************************************/
	iim::real32 *getStripe(short row_index, short d_index, int restore_direction=-1, StackRestorer* stk_rst=NULL, int blending_algo=S_SINUSOIDAL_BLENDING) throw (iim::IOException);

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
	static inline iim::real32 sinusoidal_blending(double& angle, iim::real32& pixel1, iim::real32& pixel2){
		return (iim::real32)(  ((cos(angle)+1.0F)*0.5F)*pixel1 + ( 1.0F - ((cos(angle)+1.0F)*0.5F))*pixel2  );
	}

	static inline iim::real32 no_blending(double& angle, iim::real32& pixel1, iim::real32& pixel2){
		return (angle <= S_PI/2 ? pixel1 : pixel2);		
	}

    /*************************************************************************************************************
    * These are special blending functions (together with the necessary static variables) which draws blank lines
    * along stacks borders without performing any blending. This enables easy checking of motorized stages coordi-
    * nates precision.
    **************************************************************************************************************/
    static double stack_marging_old_val;
    static bool blank_line_drawn;
    static inline iim::real32 stack_margin(double& angle, iim::real32& pixel1, iim::real32& pixel2)
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
		
	static inline iim::real32 enhanced_no_blending(double& angle, iim::real32& pixel1, iim::real32& pixel2){
		return (iim::real32)(  ((cos(angle)+1.0F)*0.5F)*pixel2 + ( 1.0F - ((cos(angle)+1.0F)*0.5F))*pixel1  );
	}

	/*************************************************************************************************************
	* Performs downsampling at a halved frequency on the given 3D image.  The given image is overwritten in order
	* to store its halvesampled version without allocating any additional resources.
	**************************************************************************************************************/
	static void halveSample(iim::real32* img, int height, int width, int depth);

	/*************************************************************************************************************
	* copies a block from buffer src to a sub-buffer of dst
	*
	* src:        pointer to the source buffer
	* dst:        pointer to the destination buffer
	* dimi:       dimension in pixels along V
	* dimj:       dimension in pixels along H
	* dimk:       dimension in pixels along D
	* s_stridej:  stride between rows in source buffer
	* s_strideij: stride between slices in source buffer
	* d_stridej:  stride between rows in destination buffer
	* d_strideij: stride abetween slices in destination buffer
	**************************************************************************************************************/
	void copyBlock2SubBuf ( iim::real32 *src, iim::real32 *dst, int dimi, int dimj, int dimk, 
	 						iim::sint64 s_stridej, iim::sint64 s_strideij, iim::sint64 d_stridej, iim::sint64 d_strideij );

	/*************************************************************************************************************
	* adds a block from buffer src to a sub-buffer of dst; values in the block are weighted with a weight alpha 
	* (0<=alpha<=1), values in the sub-buffer are weighted with a weight 1-alpha
	*
	* src:        pointer to the source buffer
	* dst:        pointer to the destination buffer
	* dimi:       dimension in pixels along V
	* dimj:       dimension in pixels along H
	* dimk:       dimension in pixels along D
	* s_stridej:  stride between rows in source buffer
	* s_strideij: stride between slices in source buffer
	* d_stridej:  stride between rows in destination buffer
	* d_strideij: stride abetween slices in destination buffer
	* alpha:      wheight to be used for the sum
	**************************************************************************************************************/
	void addBlock2SubBuf ( iim::real32 *src, iim::real32 *dst, int dimi, int dimj, int dimk, 
	 						iim::sint64 s_stridej, iim::sint64 s_strideij, iim::sint64 d_stridej, iim::sint64 d_strideij, 
							iim::real32 (*blending)(double&, iim::real32&, iim::real32&), double alpha );


public:

    StackStitcher2(MultiLayersVolume* _volume);

	/*************************************************************************************************************
	* Method to be called for displacement computation. <> parameters are mandatory, while [] are optional.
	* <algorithm_type>		: ID of the pairwise displacement algorithm to be used.
	* [start/end_...]		: layers interval that possible identify the portion of volume to be processed.
	*						  If not given, all stacks will be processed.
	* [displ_max_...]		: maximum displacements along VHD between two  adjacent stacks  taking the given over-
	*						  lap as reference. These parameters, together with <overlap_...>,can be used to iden-
	*						  tify the region of interest where the correspondence between the given stacks has to
	*						  be found. When used, these parameters have to be tuned with respect to the precision 
	*						  of the motorized stages. 
	*						  If not given, value S_DISPL_MAX_VHD is assigned.
	* [tile_idx_V ...]	    : indices of tiles to be aligned
	* [show_progress_bar]	: enables/disables progress bar with estimated time remaining.
	*
	* WARNING: this method should be used when layers are still unstitched matrices of tiles.
	**************************************************************************************************************/
	void computeTileDisplacements(int algorithm_type, int start_layer, int end_layer, 
												  int displ_max_V, int displ_max_H, int displ_max_D, bool show_progress_bar) throw (iim::IOException);


	/*************************************************************************************************************
	* For each pair of layers projects the best interlayer displacement and leaves one displacements that  has  to 
	* be applied to the layer as a whole. 
	* WARNING: this mathod it has to be used if layers are already stitched 3D images.
	**************************************************************************************************************/
	void projectDisplacements()																  throw (iim::IOException);

	/*************************************************************************************************************
	* Compute the tiles placement with a global optimization algorithm taking into account the alignment of the 
	* whole 3D matrix of tiles.
	* Update the internal representation of each layer.
	**************************************************************************************************************/
	void computeTilesPlacement(int algorithm_type)																  throw (iim::IOException);

	/*************************************************************************************************************
	* Assuming that for each pair of adjacent stacks  exists one  and only one displacement,  this displacement is 
	* thresholded according to the given <reliability_threshold>. When a displacement is not reliable enough,  its
	* parameters are set to default values (i.e. nominal motorized stage coordinates).
	* Moreover, stacks which do not have any reliable single-direction displacements with all 4 neighbors are mar-
	* ked as NON STITCHABLE.
	**************************************************************************************************************/
	void thresholdDisplacements(float reliability_threshold)								  throw (iim::IOException);


	/*************************************************************************************************************
	* Executes the compute tiles placement algorithm associated to the given ID <algorithm_type>
	**************************************************************************************************************/
	void computeLayersPlacement()											  throw (iim::IOException);


    /*************************************************************************************************************
    * Computes final stitched volume dimensions assuming that current <StackedVolume> object contains  the correct
    * stack coordinates. The given parameters identify the possible VOI (Volume Of Interest). If these are not us-
    * ed, the whole volume is  considered and the parameter <exclude_nonstitchable_stacks> is used to discard rows
    * or columns with no stitchable stacks
	*
	* _ROW_START: fist coordinate of the up/left/top corner
	* _ROW_END:   fist coordinate of the down/right/bottm corner
	* _COL_START: second coordinate of the up/left/top corner
	* _COL_END:   second coordinate of the down/right/bottm corner
	* _D0:        third coordinate of the up/left/top corner
	* _D1:        third coordinate of the down/right/bottm corner
    **************************************************************************************************************/
    void computeVolumeDims(int _ROW_START = -1,	   int _ROW_END = -1,
                           int _COL_START = -1, int _COL_END = -1, int _D0 = -1, int _D1 = -1) throw (iim::IOException);

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
	* [blending_algo]		: ID of the blending algorithm to be used in the overlapping regions of layers.		 
	* [intralayer_blending_algo]: ID of the blending algorithm to be used in for merging tiles of each layer.		 
	* [test_mode]			: if enabled, the middle slice of the whole volume will be stitched and and  saved lo-
	*						  cally. Stage coordinates will be used, s o this can be used to test  their precision
	*						  as well as the selected reference system.
	* [show_progress_bar]	: enables/disables progress bar with estimated time remaining.
	* [saved_img_format]	: determines saved images format ("png","tif","jpeg", etc.).
	* [saved_img_depth]		: determines saved images bitdepth (16 or 8).
	**************************************************************************************************************/
	void mergeTiles(std::string output_path, int slice_height = -1, int slice_width = -1, bool* resolutions = NULL, 
					int _ROW_START=-1, int _ROW_END=-1, int _COL_START=-1,
					int _COL_END=-1, int _D0=-1, int _D1=-1, int blending_algo=S_SINUSOIDAL_BLENDING, int intralayer_blending_algo=S_SINUSOIDAL_BLENDING, 
					bool test_mode=false, bool show_progress_bar= true, const char* saved_img_format=iom::DEF_IMG_FORMAT.c_str(), 
					int saved_img_depth=iom::DEF_BPP) throw (iim::IOException);
		

	/*************************************************************************************************************
	* Functions used to save single phase time performances
	**************************************************************************************************************/
	static void saveComputationTimes(const char *filename, MultiLayersVolume &stk_org, double total_time=-1);
	static void resetComputationTimes();

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
	* [blending_algo]		: ID of the blending algorithm to be used in the overlapping regions of layers.		 
	* [intralayer_blending_algo]: ID of the blending algorithm to be used in for merging tiles of each layer.		 
	* [test_mode]			: if enabled, the middle slice of the whole volume will be stitched and and  saved lo-
	*						  cally. Stage coordinates will be used, s o this can be used to test  their precision
	*						  as well as the selected reference system.
	* [show_progress_bar]	: enables/disables progress bar with estimated time remaining.
	* [saved_img_format]	: determines saved images format ("png","tif","jpeg", etc.).
	* [saved_img_depth]		: determines saved images bitdepth (16 or 8).
	**************************************************************************************************************/

	void mergeTilesVaa3DRaw(std::string output_path, int block_height = -1, int block_width = -1, int block_depth = -1, bool* resolutions = NULL, 
							int _ROW_START=-1, int _ROW_END=-1, int _COL_START=-1,
							int _COL_END=-1, int _D0=-1, int _D1=-1, int blending_algo=S_SINUSOIDAL_BLENDING, int intralayer_blending_algo=S_SINUSOIDAL_BLENDING, 
							bool test_mode=false, bool show_progress_bar= true, const char* saved_img_format=iom::DEF_IMG_FORMAT.c_str(), 
							int saved_img_depth=iom::DEF_BPP) throw (iim::IOException);
};

#endif


