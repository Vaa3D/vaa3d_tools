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
* 2016-07-25. Giulio. @CREATED 
*/

#ifndef _MEX_OPERATIONS_H
#define _MEX_OPERATIONS_H

struct info_t {
    char*         root_dir;				// C-string that contains the directory path of stacks matrix
    float         VXL_V, VXL_H, VXL_D;		// [microns]: voxel dimensions (in microns) along V(Vertical), H(horizontal) and D(Depth) axes
    float         ORG_V, ORG_H, ORG_D;		// [millimeters]: origin spatial coordinates (in millimeters) along VHD axes
    unsigned int  DIM_V, DIM_H, DIM_D;// volume dimensions (in voxels) along VHD axes
    int           DIM_C;					// number of channels        (@ADDED by Iannello   on ..........)
    int           BYTESxCHAN;              // number of bytes per channel
    unsigned int *active;          // array  of active channels (@MOVED from "TiledMCVolume" by Alessandro on 2014-02-20)
    int           n_active;         // number of active channels (@MOVED from "TiledMCVolume" by Alessandro on 2014-02-20)

    int           DIM_T;					// number of time frames         (@ADDED by Alessandro on 2014-02-20)
    int           t0, t1;                     // active frames are in [t0, t1] (@ADDED by Alessandro on 2014-02-20)
};

int createObject ( void **objectList, int listLen, const char *initString );

void disposeObject ( void *obj );

void getInfo ( void *obj, info_t *info_ );

unsigned char *loadSubvolume ( void *obj, int V0 = -1, int V1 = -1, int H0 = -1, int H1 = -1, int D0 = -1, int D1= -1 );

void disposeObjects ( void **objectList, int listLen );

#endif //_MEX_OPERATIONS_H
