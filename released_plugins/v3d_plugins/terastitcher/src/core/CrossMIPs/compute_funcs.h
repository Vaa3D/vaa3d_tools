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
 * compute_funcs.h
 *
 *  Created on: September 2010
 *      Author: iannello
 *
 *  Last revision: May, 31 2011
 */


#ifndef COMPUTE_FUNCS_H
#define COMPUTE_FUNCS_H


# include "my_defs.h"

# include "CrossMIPs.h"


# define GRAY_LEVELS    65536


void compute_3_MIPs ( real_t *A, real_t *B,
					  real_t *MIP_xy1, real_t *MIP_xz1, real_t *MIP_yz1, 
					  real_t *MIP_xy2, real_t *MIP_xz2, real_t *MIP_yz2, 
					  int dimi_v, int dimj_v, int dimk_v, int stridei, int stridek );

void compute_NCC_map ( real_t *NCC_map, real_t *MIP_1, real_t *MIP_2, 
					       int dimu, int dimv, int delayu, int delayv );

real_t compute_NCC ( real_t *im1, real_t *im2, int dimi, int dimj, int stride );

int compute_MAX_ind ( real_t *vect, int len );

void compute_Neighborhood ( NCC_parms_t *NCC_params, real_t *NCC, int delayu, int delayv, int ind, 
						    real_t *MIP_1, real_t *MIP_2, int dimu, int dimv, real_t *NCCnew, int &du, int &dv, bool &failed)  throw (MyException);
/* given an NCC map with dimensions delayu x delayv around the initial alignment, the index ind of its maximum
 * and an empty NCC map NCCnew with dimensions NCC_params->wRangeThr x NCC_params->wRangeThr,
 * returns in NCCnew the NCC map with center in the NCC maximum of MIPs MIP_1 and MIP_2, with dimensions dimu x dimv;
 * also returns the position (du,dv) of this maximum, relative to the initial alignment
 */

//void compute_Alignment( NCC_parms_t *NCC_params, REAL_T *NCC_xy, REAL_T *NCC_xz, REAL_T *NCC_yz, 
//					    int dimi, int dimj, int dimk, int ind_xy, int ind_xz, int ind_yz, NCC_descr_t *result);
void compute_Alignment( NCC_parms_t *NCC_params, real_t *NCC_xy, real_t *NCC_xz, real_t *NCC_yz,
					    int dimi, int dimj, int dimk, int dx1, int dx2, int dy1, int dy2, int dz1, int dz2,  bool failed_xy, bool failed_xz, bool failed_yz, NCC_descr_t *result);
/* given the three NCCs, the corresponding indicex of NCC maxima and their displacements with respect to the 
 * initial alignment, returns in the parameter result the three alignments and the corresponding reliability indices;
 * alignments represent offsets of the second stack with respect to the first one
 */

void enhance ( real_t *im, int imLen, int graylevels, NCC_parms_t *NCC_params );
/* enhance contrast of an image
 * INPUT parameters:
 *    im is a pointer to actual pixels, represented as a linear sequence of length imLen
 *    graylevels is the number of discrete gray levels to be used to discriminate pixels for
 *    enhancement
 *    NCC_params contains data describing the transformation used for enhancement (see 
 *    CrossMIPs.h for details)
 */

void stack_percentiles ( real_t *im, int imLen, int graylevels, 
						 real_t *percentiles, real_t *thresholds, int n_percentiles );
/* returns thresholds on pixel values to implement an arbitrary grayscale transformation
 * INPUT parameters:
 *    im is a pointer to actual pixels represented as a linear sequence of length imLen
 *    percentiles is a pointer to an array of n_percentiles real values representing
 *    the percentiles of pixels to which the i-th rescaled linear transformation has
 *    to be applied (i=1,...,n_percentiles)
 * INPUT/OUTPUT parmeter:
 *    thresholds is a pointer to an array of (n_percentiles+1) real values representing the
 *    pixel values interval to which the i-th linear transformation has to be applied
 *    (i=1,...,n_percentiles)
 * PRE: the value of percentiles[n_percentiles-1] must be (REAL_T)1.0
 *      the value of threshold[0] must be 0
 */

# ifdef _PAR_VERSION
void init_configuration ( );
# endif

# endif
