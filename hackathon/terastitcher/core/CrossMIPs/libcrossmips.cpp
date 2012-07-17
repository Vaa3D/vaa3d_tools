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
*       Bria, A., et al., (2012) "Stitching Terabyte-sized 3D Images Acquired in Confocal Ultramicroscopy", Proceedings of the 9th IEEE International Symposium on Biomedical Imaging.
*       Bria, A., Iannello, G., "A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Datasets", submitted on July 2012 to IEEE Transactions on Information Technology in Biomedicine.
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
 * libcrossmips.cpp
 *
 *  Created on: September 2010
 *      Author: iannello
 *
 *  Last revision: May, 31 2011
 *
 * --- revision of May, 31 2011 -------------------------------------------------------------------
 * Parameters can be controlled through a structure defined in CrossMIPs.h which can be 
 * passed as a parameter to norm_cross_corr_mips
 * Default values are set in implementation of norm_cross_corr_mips
 *
 * A structure is returned containing the thre offsets of the second 3d stack with respect to 
 * the first one. The structure is defined in CrossMIPs.h and it contains also information
 * about the reliability and precision of the alignement. Reliability of each offset is measured 
 * by the value of the NCC maxima used to evaluate it. Precision is measured by the distance
 * between NCC maxima and pixels equal to a given fraction of the maxima in the direction of
 * each offset. The algorithm used to determine all these quantities is in function 
 * compute_NCC_alignment, which is internal to the file compute_funcs.cpp
 * ----------------------------------------------------------------------------------------------
 */


# include <math.h>
# include <stdio.h>
# include <stdlib.h>

# include "CrossMIPs.h"

# include "compute_funcs.h"


bool write_3D_stack ( char *fname, real_t *stck, int dimi, int dimj, int dimk );


NCC_descr_t *norm_cross_corr_mips ( real_t *A, real_t *B,
						    int dimk, int dimi, int dimj, int nk, int ni, int nj,
							int delayk, int delayi, int delayj, int side, NCC_parms_t *NCC_params ) {
#if CM_VERBOSE > 1
	printf("\nin libcrossmips::norm_cross_corr_mips(A[%.6f-%.6f], B[%.6f-%.6f], dimk[%d], dimi[%d], dimj[%d], nk[%d], ni[%d], nj[%d], delayk[%d], delayi[%d], delayj[%d], side[%d]\n",
		A[0], A[(dimk-1)*dimj*dimi + (dimi-1)*dimj + dimj -1], B[0], B[(dimk-1)*dimj*dimi + (dimi-1)*dimj + dimj -1], dimk, dimi, dimj, nk, ni, nj, delayk, delayi, delayj, side );
#endif

	NCC_descr_t *result = new NCC_descr_t;
	int dimk_v, dimi_v, dimj_v;
	int stridei, stridek; // arrays are stored along j dimension, hence it does not need a stride
	real_t *A1; // starting pixels of volume A to be scanned

	real_t *MIP_xy1, *MIP_xz1, *MIP_yz1;
	real_t *MIP_xy2, *MIP_xz2, *MIP_yz2;
	int i;

	real_t *NCC_xy, *NCC_xz, *NCC_yz;

	real_t *temp;

	int ind_xy, ind_xz, ind_yz;
	int dx1, dx2, dy1, dy2, dz1, dz2;

	bool failed_xy=false, failed_xz=false, failed_yz=false;

	bool allocated = !NCC_params;
	if ( !NCC_params ) {
		// ************************** SET DEFAULT PARAMETERS ****************
		NCC_params = new NCC_parms_t;
		// to change default behavior change the following parameters
		NCC_params->enhance      = false;
		NCC_params->maxIter		 = 2;
		NCC_params->maxThr       = CM_DEF_MAX_THR;
		NCC_params->widthThr     = CM_DEF_WIDTH_THR;
		NCC_params->wRangeThr    = CM_DEF_W_RANGE_THR;
		NCC_params->UNR_NCC      = CM_DEF_UNR_NCC;
		NCC_params->INF_W        = CM_DEF_INF_W;
		NCC_params->INV_COORD    = 0;

		/* Example of settings for fields 'percents' and 'c'
		 * percents = < 0.1, 0.9, 1.0 > and c = < 0.0, 0,0, 1.0, 1.0 >
		 * means that:
		 * - 10% of pixels with lower values are set to 0 (first transformation maps pixels from c[0]=0.0 to c[1]=0.0
		 * - 10% of pixels with higher values are set to 1 (third transformation maps pixels from c[2]=1.0 to c[3]=1.0
		 * - 80% of pixels with intemetiate values are mapped to the interval [0,1] (second transformation maps pixels
		 *   from c[1]=0.0 to c[2]01.0
		 */

		if ( enhance ) {
			//// two transformations defined by pairs (percentiles,value): {(0.0,0.0),(0.8,0.2),(1.0,1.0)}
			//NCC_params->n_transforms = 2;
			//NCC_params->percents     = new REAL_T[NCC_params->n_transforms];
			//NCC_params->c            = new REAL_T[NCC_params->n_transforms+1];
			//// percents must have n_transforms elements and percents[n_transforms-1] must be 1.00
			//NCC_params->percents[0]  = (REAL_T) 0.80;
			//NCC_params->percents[1]  = (REAL_T) 1.00;
			//// c must have (n_transforms+1) elements, c[0] must be 0.00 and c[n_transforms] must be 1.00
			//NCC_params->c[0]         = (REAL_T) 0.00;
			//NCC_params->c[1]         = (REAL_T) 0.20;
			//NCC_params->c[2]         = (REAL_T) 1.00;

			// three transformations defined by pairs (percentiles,value): {(0.0,0.0),(0.1,0.0),(0.95,1.0),(1.0,1.0)}
			NCC_params->n_transforms = 3;
			NCC_params->percents     = new real_t[NCC_params->n_transforms];
			NCC_params->c            = new real_t[NCC_params->n_transforms+1];
			// percents must have n_transforms elements and percents[n_transforms-1] must be 1.00
			NCC_params->percents[0]  = (real_t) 0.10;
			NCC_params->percents[1]  = (real_t) 0.99;
			NCC_params->percents[2]  = (real_t) 1.00;
			// c must have (n_transforms+1) elements, c[0] must be 0.00 and c[n_transforms] must be 1.00
			NCC_params->c[0]         = (real_t) 0.00;
			NCC_params->c[1]         = (real_t) 0.00;
			NCC_params->c[2]         = (real_t) 1.00;
			NCC_params->c[3]         = (real_t) 1.00;
		}
	}

	if ( NCC_params->wRangeThr > 2*MAX(delayi,MAX(delayj,delayk))+1 )
	{
		char err_msg[500];
		sprintf(err_msg, "parameter wRangeThr[=%d] is too large with respect to 2*delayi/j/k +1 [=%d]", NCC_params->wRangeThr, 2*MAX(delayi,MAX(delayj,delayk))+1);
		DISPLAY_ERROR(err_msg);
	}

	/*
	 *  il seguente codice assume che:
	 *  - le matrici bidimensionali siano memorizzate secondo l'ultima dimensione
	 *  ad esempio il MIP proiettato lungo y, che include le dimensioni x e z, e' memorizzato per
	 *  valori di z consecutivi
	 *  - gli stack 3D siano memorizzati per piani orizzontali e ciascun piano sia memorizzato
	 *  come una metrice bidimensionale (secondo l'ultima dimensione); questa ipotesi ha effetto
	 *  solo nel calcolo delle tre proiezioni MIP
	 */

# ifdef _PAR_VERSION

	init_configuration();

# endif
			
	// calcola parametri per scandire i volumi
	if ( side == NORTH_SOUTH ) {
		dimk_v = dimk;
		dimi_v = dimi - ni;
		dimj_v = dimj;
		stridei = 0; // rows are entirely scanned
		stridek = ni*dimj; // pixels to be skipped when changing slice are all contiguous
		A1 = A + stridek; // a block of contiguous pixels of volume A have to be skipped
	}
	else if ( side == WEST_EAST ) {
		dimk_v = dimk;
		dimi_v = dimi;
		dimj_v = dimj - nj;
		stridei = nj; // rows are partially scanned and nj pixels have to be skipped when changing row
		stridek = 0; // no more pixels have to be skipped when changing slice 
		A1 = A + stridei; // a partial row of pixels of volume A have to be skipped
	}
	else
		DISPLAY_ERROR("unexpected alignmnt configuration");

	// alloca le 6 immagini per i MIP
	MIP_xy1 = new real_t[dimi_v*dimj_v];
	for ( i=0; i<(dimi_v*dimj_v); i++ )
		MIP_xy1[i] = 0;
	MIP_xz1 = new real_t[dimi_v*dimk_v];
	for ( i=0; i<(dimi_v*dimk_v); i++ )
		MIP_xz1[i] = 0;
	MIP_yz1 = new real_t[dimj_v*dimk_v];
	for ( i=0; i<(dimj_v*dimk_v); i++ )
		MIP_yz1[i] = 0;

	MIP_xy2 = new real_t[dimi_v*dimj_v];
	for ( i=0; i<(dimi_v*dimj_v); i++ )
		MIP_xy2[i] = 0;
	MIP_xz2 = new real_t[dimi_v*dimk_v];
	for ( i=0; i<(dimi_v*dimk_v); i++ )
		MIP_xz2[i] = 0;
	MIP_yz2 = new real_t[dimj_v*dimk_v];
	for ( i=0; i<(dimj_v*dimk_v); i++ )
		MIP_yz2[i] = 0;

	compute_3_MIPs(A1,B,MIP_xy1,MIP_xz1,MIP_yz1,MIP_xy2,MIP_xz2,MIP_yz2,
									dimi_v,dimj_v,dimk_v,stridei,stridek);

#ifdef _WRITE_IMGS
	write_3D_stack("MIP_xy1.dat",MIP_xy1,dimi_v,dimj_v,1);
	write_3D_stack("MIP_xy2.dat",MIP_xy2,dimi_v,dimj_v,1);
	write_3D_stack("MIP_xz1.dat",MIP_xz1,dimi_v,dimk_v,1);
	write_3D_stack("MIP_xz2.dat",MIP_xz2,dimi_v,dimk_v,1);
	write_3D_stack("MIP_yz1.dat",MIP_yz1,dimj_v,dimk_v,1);
	write_3D_stack("MIP_yz2.dat",MIP_yz2,dimj_v,dimk_v,1);
#endif

	// calcola NCC su xy
	NCC_xy = new real_t[(2*delayi+1)*(2*delayj+1)];
	for ( i=0; i<((2*delayi+1)*(2*delayj+1)); i++ )
		NCC_xy[i] = 0;

	if ( NCC_params->enhance ) {
		enhance(MIP_xy1,(dimi_v*dimj_v),GRAY_LEVELS,NCC_params);
		enhance(MIP_xy2,(dimi_v*dimj_v),GRAY_LEVELS,NCC_params);
	}

	compute_NCC_map(NCC_xy,MIP_xy1,MIP_xy2,dimi_v,dimj_v,delayi,delayj);

	// calcola NCC su xz
	NCC_xz = new real_t[(2*delayi+1)*(2*delayk+1)];
	for ( i=0; i<((2*delayi+1)*(2*delayk+1)); i++ )
		NCC_xz[i] = 0;

	if ( NCC_params->enhance ) {
		enhance(MIP_xz1,(dimi_v*dimk_v),GRAY_LEVELS,NCC_params);
		enhance(MIP_xz2,(dimi_v*dimk_v),GRAY_LEVELS,NCC_params);
	}

	compute_NCC_map(NCC_xz,MIP_xz1,MIP_xz2,dimi_v,dimk_v,delayi,delayk);

	// calcola NCC su yz
	NCC_yz = new real_t[(2*delayj+1)*(2*delayk+1)];
	for ( i=0; i<((2*delayj+1)*(2*delayk+1)); i++ )
		NCC_yz[i] = 0;

	if ( NCC_params->enhance ) {
		enhance(MIP_yz1,(dimj_v*dimk_v),GRAY_LEVELS,NCC_params);
		enhance(MIP_yz2,(dimj_v*dimk_v),GRAY_LEVELS,NCC_params);
	}

	compute_NCC_map(NCC_yz,MIP_yz1,MIP_yz2,dimj_v,dimk_v,delayj,delayk);

#ifdef _WRITE_IMGS
	if ( NCC_params->enhance ) {
		write_3D_stack("MIP_xy1_en.dat",MIP_xy1,dimi_v,dimj_v,1);
		write_3D_stack("MIP_xy2_en.dat",MIP_xy2,dimi_v,dimj_v,1);
		write_3D_stack("MIP_xz1_en.dat",MIP_xz1,dimi_v,dimk_v,1);
		write_3D_stack("MIP_xz2_en.dat",MIP_xz2,dimi_v,dimk_v,1);
		write_3D_stack("MIP_yz1_en.dat",MIP_yz1,dimj_v,dimk_v,1);
		write_3D_stack("MIP_yz2_en.dat",MIP_yz2,dimj_v,dimk_v,1);
	}
#endif

#ifdef _WRITE_IMGS
	write_3D_stack("NCC_xy.dat",NCC_xy,(2*delayi+1),(2*delayj+1),1);
	write_3D_stack("NCC_xz.dat",NCC_xz,(2*delayi+1),(2*delayk+1),1);
	write_3D_stack("NCC_yz.dat",NCC_yz,(2*delayj+1),(2*delayk+1),1);
#endif

	// max su xy, xz, yz
	ind_xy = compute_MAX_ind(NCC_xy,((2*delayi+1)*(2*delayj+1)));
	ind_xz = compute_MAX_ind(NCC_xz,((2*delayi+1)*(2*delayk+1)));
	ind_yz = compute_MAX_ind(NCC_yz,((2*delayj+1)*(2*delayk+1)));

	// NCC_xy: check neighborhood of maxima and search for better maxima if any
	temp = new real_t[(2*NCC_params->wRangeThr+1)*(2*NCC_params->wRangeThr+1)];;
	for ( i=0; i<((2*NCC_params->wRangeThr+1)*(2*NCC_params->wRangeThr+1)); i++ )
		temp[i] = 0;
	compute_Neighborhood(NCC_params,NCC_xy,delayi,delayj,ind_xy,MIP_xy1,MIP_xy2,dimi_v,dimj_v,temp,dx1,dy1, failed_xy);
	// substitute NCC map and delete the old one
	delete NCC_xy;
	NCC_xy = temp;
	temp = (real_t *)0;

	// NCC_xz: check neighborhood of maxima and search for better maxima if any
	temp = new real_t[(2*NCC_params->wRangeThr+1)*(2*NCC_params->wRangeThr+1)];;
	for ( i=0; i<((2*NCC_params->wRangeThr+1)*(2*NCC_params->wRangeThr+1)); i++ )
		temp[i] = 0;
	compute_Neighborhood(NCC_params,NCC_xz,delayi,delayk,ind_xz,MIP_xz1,MIP_xz2,dimi_v,dimk_v,temp,dx2,dz1, failed_xz);
	// substitute NCC map and delete the old one
	delete NCC_xz;
	NCC_xz = temp;
	temp = (real_t *)0;

	// NCC_yz: check neighborhood of maxima and search for better maxima if any
	temp = new real_t[(2*NCC_params->wRangeThr+1)*(2*NCC_params->wRangeThr+1)];;
	for ( i=0; i<((2*NCC_params->wRangeThr+1)*(2*NCC_params->wRangeThr+1)); i++ )
		temp[i] = 0;
	compute_Neighborhood(NCC_params,NCC_yz,delayj,delayk,ind_yz,MIP_yz1,MIP_yz2,dimj_v,dimk_v,temp,dy2,dz2, failed_yz);
	// substitute NCC map and delete the old one
	delete NCC_yz;
	NCC_yz = temp;
	temp = (real_t *)0;

	//compute_Alignment(NCC_params,NCC_xy,NCC_xz,NCC_yz,delayi,delayj,delayk,ind_xy,ind_xz,ind_yz,result);
	
	compute_Alignment(NCC_params,NCC_xy,NCC_xz,NCC_yz,
		NCC_params->wRangeThr,NCC_params->wRangeThr,NCC_params->wRangeThr,dx1,dx2,dy1,dy2,dz1,dz2,failed_xy, failed_xz, failed_yz, result);

	if ( side == NORTH_SOUTH ) 
		result->coord[0] += ni;
	else if ( side == WEST_EAST ) 
		result->coord[1] += nj;
	else
		DISPLAY_ERROR("unexpected alignmnt configuration");

	if ( allocated ) {
		delete NCC_params->percents;
		delete NCC_params->c;
		delete NCC_params;
	}

	delete MIP_xy1;
	delete MIP_xz1;
	delete MIP_yz1;
	delete MIP_xy2;
	delete MIP_xz2;
	delete MIP_yz2;

	delete NCC_xy;
	delete NCC_xz;
	delete NCC_yz;

	// temp must not be deleted
#if CM_VERBOSE > 1
	printf("\tReturning\n\t\tV[%d, %.6f, %d]\n\t\tH[%d, %.6f, %d]\n\t\tD[%d, %.6f, %d]\n\n", 
		result->coord[0], result->NCC_maxs[0], result->NCC_widths[0],
		result->coord[1], result->NCC_maxs[1], result->NCC_widths[1],
		result->coord[2], result->NCC_maxs[2], result->NCC_widths[2]);
#endif
	return result;
}


bool write_3D_stack ( char *fname, real_t *stck, int dimi, int dimj, int dimk ) {
	FILE *fout;
	int i, j, k;

	if ( (fout = fopen(fname,"wb")) == NULL ) return false;
	
	fwrite(&dimi,sizeof(int),1,fout);
	fwrite(&dimj,sizeof(int),1,fout);
	fwrite(&dimk,sizeof(int),1,fout);

	for ( k=0; k<dimk; k++ )
		for ( j=0; j<dimj; j++ )
			for ( i=0; i<dimi; i++ )
				fwrite((stck+j+i*dimj+k*dimj*dimi),sizeof(real_t),1,fout);

	fclose(fout);

	return true;
}

