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
* 2017-04-01.  Giulio.     @ADDED code for completing multi-layer management
* 2017-02-10.  Giulio.     @CREATED
*/


#ifndef _MULTILAYERS_MANAGER_H
#define _MULTILAYERS_MANAGER_H

//#include "MyException.h"

#include "../utils/terastitcher2/GUI_config.h"
#include "IM_config.h"
#include "tinyxml.h"
#include "Displacement.h"
#include "../volumemanager/vmVirtualStack.h"

#include <string>
#include <vector>

class MultiLayersVolume {

protected:

	typedef int VHD_coords[3];

	char* layers_dir;			 //C-string that contains the directory path of multi-layers volume
	float normal_factor_D;

	float  VXL_V, VXL_H, VXL_D;		 // [microns]: voxel dimensions (in microns) along V(Vertical), H(horizontal) and D(Depth) axes
    float  ORG_V, ORG_H, ORG_D;		 // [millimeters]: origin spatial coordinates (in millimeters) along VHD axes
    iim::uint32 DIM_V, DIM_H, DIM_D; // nominal volume dimensions (in voxels) along VHD axes
    int    DIM_C;					 // number of channels        (@ADDED by Iannello   on ..........)
    int    BYTESxCHAN;               // number of bytes per channel
	iim::ref_sys reference_system;

	int N_LAYERS;
	iim::VirtualVolume **LAYERS;
	VHD_coords *layers_coords;      // elements are triplets (V,H,D)
	int *nominal_D_overlap;
	float cut_depth;				// depth in um affected by both fluorescence decay (in the top layer) and the cut between layers (in the bottom layer)

 	// disps[i] is the displacements of layer i+1 with respect to layer i
	std::vector< std::vector<Displacement *> > **disps;

	// list of xml files associated to layers to be used for saving the xml file describing the multilayer volume (not active if null)
	std::string *layers_new_xml_fnames;

	// the default constructor should not be used
	MultiLayersVolume ( ) { }

	void init ( );

	iim::VirtualVolume *getLAYER ( int i ) { return LAYERS[i]; }

public:
	MultiLayersVolume ( std::string _layers_dir, float _cut_depth = CLI_DEF_CUT_DEPTH, float _norm_factor_D = CLI_DEF_NORM3 );

	MultiLayersVolume ( const char *_layers_dir );

	~MultiLayersVolume ( ); 


	const char* getLAYERS_DIR(){return this->layers_dir;}
	int			getN_LAYERS() {return N_LAYERS;}
	int			getOVERLAP_D(int i) {return nominal_D_overlap[i];}
	float		getCUT_DEPTH() {return cut_depth;}
	int			getCUT_DEPTH_PXL() {return (int)floor(cut_depth / VXL_D + 0.5);}

	int			getLAYER_DIM(int i, int j);
	iim::real32* getSUBVOL(int i, int V0 = -1, int V1 = -1, int H0 = -1, int H1 = -1, int D0 = -1, int D1 = -1);
	int			getLAYER_COORDS(int i, int j) {return layers_coords[i][j];}

	float		getVXL_V() {return VXL_V;}
	float		getVXL_H() {return VXL_H;}
	float		getVXL_D() {return VXL_D;}
	float		getORG_V() {return ORG_V;}
	float		getORG_H() {return ORG_H;}
	float		getORG_D() {return ORG_D;}
	iim::uint32	getDIM_V() {return DIM_V;}
	iim::uint32	getDIM_H() {return DIM_H;}
	iim::uint32	getDIM_D() {return DIM_D;}
	int			getDIM_C() {return DIM_C;}
	int			getBYTESxCHAN() {return BYTESxCHAN;}

	float		getABS_V(int ABS_PIXEL_V) {return ORG_V * 1000 + ABS_PIXEL_V*VXL_V;}
	float		getABS_H(int ABS_PIXEL_H) {return ORG_H * 1000 + ABS_PIXEL_H*VXL_H;}
	float		getABS_D(int ABS_PIXEL_D) {return (ORG_D * 1000 + ABS_PIXEL_D*VXL_D) * normal_factor_D;}

	std::vector< std::vector< Displacement *> >  *getDISPS(int i) {return disps[i];}

	void		setORG_V(float _ORG_V) {ORG_V = _ORG_V;}
	void		setORG_H(float _ORG_H) {ORG_H = _ORG_H;}
	void		setORG_D(float _ORG_D) {ORG_D = _ORG_D;}

	void		initDISPS(int i, int _DIM_V, int _DIM_H);

	void		addVHD_COORDS(int i, int j, int c) {layers_coords[i][j] += c;}

	void		setDIMS(int _DIM_V, int _DIM_H, int _DIM_D) {
		DIM_V = _DIM_V; DIM_H = _DIM_H; DIM_D = _DIM_D;
	}
	void		setDISPS(int i, std::vector< std::vector<Displacement *> > *disp) {
		if ( disps[i] )
			delete disps[i];
		disps[i] = disp;
	};

	void updateLayerCoords ( ) ;

	//int			getDEFAULT_DISPLACEMENT_V(int i) {return 0;}
	//int			getDEFAULT_DISPLACEMENT_H(int i) {return 0;}
	//int			getDEFAULT_DISPLACEMENT_D(int i) {return -nominal_D_overlap[i];}

	iim::ref_sys getREF_SYS() {return reference_system;}  // WARNING: assume that layers are TiledVolumes

	// methods to be checked
	int			getStacksHeight() {return 0;}
	int			getStacksWidth() {return 0;}
	//int			getOVERLAP_V() {return 0;}
	//int			getOVERLAP_H() {return 0;}
	int			getN_ROWS() {return 0;}
	int			getN_COLS() {return 0;}
	//int			getN_SLICES() {return 0;}
	vm::VirtualStack*** getSTACKS() {return 0;}

	//XML methods
	void			initFromXML(const char *xml_filename) throw (iim::IOException);
	void			saveXML(const char *xml_filename=0, const char *xml_filepath=0) throw (iim::IOException);
	void            saveLayersXML(const char *xml_filename=0, const char *xml_filepath=0) throw (iim::IOException);

	void insertDisplacement(int i, int j, int k, Displacement *displacement) throw (iim::IOException);

	// needed to extract layer volumes
	friend class TPAlgo2MST; 
	friend class StackStitcher2;
};

#endif /* _MULTILAYERS_MANAGER_H */


