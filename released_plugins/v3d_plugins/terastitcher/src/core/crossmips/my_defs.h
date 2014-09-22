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

/*
 * my_defs.h
 *
 *  Created on: September 2010
 *      Author: iannello
 *
 *  Last revision: May, 31 2011
 */

# ifndef MY_DEFS_H
# define MY_DEFS_H

#define CM_VERBOSE 1
//#define _PAR_VERSION
#define _ERROR_TO_STDERR

#define CM_DEF_MAX_THR 0.50	  //default threshold for NCC maximum (below this threshold the NCC is considered unreliable)
#define CM_DEF_WIDTH_THR 0.75	  //default fraction of maximum used to evaluate the maximum width (belongs to [0,1])
#define CM_DEF_W_RANGE_THR 29     //default range used to evaluate maximum width (when maximum width is greater or equal to this value, width is set to INF_W
#define CM_DEF_UNR_NCC 0	  //default unreliable NCC peak value
#define CM_DEF_INF_W 30    //default infinite NCC peak width


typedef float real_t;



# define MIN(a,b)       (((a)<(b)) ? (a) : (b))
# define MAX(a,b)       (((a)>(b)) ? (a) : (b))
# define START_IND(i)   (((i)>0) ? (i) : 0)


# ifdef _ERROR_TO_STDERR
# define DISPLAY_ERROR(msg) \
	fprintf(stderr,"*** libcrossmips ERROR: %s\n", msg)
# else
# define DISPLAY_ERROR(msg)
# endif


# endif
