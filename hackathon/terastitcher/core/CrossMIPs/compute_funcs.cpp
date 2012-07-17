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
 * compute_funcs.cpp
 *
 *  Created on: September 2010
 *      Author: iannello
 *
 *  Last revision: May, 31 2011
 */


# include <math.h>
# include <stdio.h>
# include <stdlib.h>


# include "compute_funcs.h"

# define LOG2(V)   (log((double)V)/log(2.0))


/**************************** INTERNAL AUXILIARY FUNCTION *******************************/

/* search x in the sorted list (a,n) and returns if x has been found; if x is not found, 
 * returns in pos the index at which x should be inserted (i.e. a[pos] is the first element
 * of the list that is greater than x)
 * specialized for use by enhance: returns the index of the linear transformation to be 
 * applied to value x
 */
static
void binary_search ( real_t *a, int n, real_t x, bool &found, int &pos ) {
	int f = 0;
	int l = n-1;
	int m;

	found = false;
	while ( f <= l ) {
		m = (f+l)/2;
		if ( x == a[m] ) {
			found = true;
			pos = m+1; // if found, the index to be returned is the next one
			return;
		}
		else if ( x < a[m] )
			l = m - 1;
		else   // V > a[m]
			f = m + 1;
	}
	pos = l+1;
	return;
}

/* given general parameters, the NCC map, its second dimension dimj and the index of NCC maximum
 * returns a measure of the horizontal (width1) and vertical (width2) half widths of the NCC peak 
 * (i.e. the largest distance of the pixel equal to a given fraction of the maximum and the 
 * maximum itself)
 */
static
void compute_NCC_width ( NCC_parms_t *NCC_params, real_t *NCC, int dimj, int ind, bool failed, int &width1, int &width2 ) {
	bool found;

	real_t thr = NCC_params->widthThr * NCC[ind];

	if(failed)
	{
		width1 = width2 = NCC_params->INF_W;
	}
	else
	{
		// evaluates first maximum width parallel to second dimension (horizontal)
		found = false;
		width2 = 1;
		while ( width2<=NCC_params->wRangeThr && !found )
			if ( NCC[ind-width2] <= thr )
				found = true;
			else
				width2++;
		found = false;
		while ( width2<=NCC_params->wRangeThr && !found )
			if ( NCC[ind+width2] <= thr )
				found = true;
			else
				width2++;
		if ( !found )
			width2 = NCC_params->INF_W;

		// evaluates maximum width parallel to first dimension (horizontal)
		found = false;
		width1 = 1;
		while ( width1<=NCC_params->wRangeThr && !found )
			if ( NCC[ind-width1*dimj] <= thr )
				found = true;
			else
				width1++;
		found = false;
		while ( width1<=NCC_params->wRangeThr && !found )
			if ( NCC[ind+width1*dimj] <= thr )
				found = true;
			else
				width1++;
		if ( !found )
			width1 = NCC_params->INF_W;
	}
	
}

/*************************** COMPUTE FINAL ALIGNMENT *****************************************/

/* given the general parameters, the index i of a dimension (0 for vertical, 1 for horizontal
 * and 2 for depth), and two alignments d1 and d2 with the corresponding NCC maxima (peak_val1
 * and peak_val2) and half widths (width1 and width2), returns in result the aligment for that 
 * with a measure of its reliability and potential error 
 */
static
void compute_NCC_alignment ( NCC_parms_t *NCC_params, NCC_descr_t *result, int i,
							int d1, real_t peak_val1, int width1, int d2, real_t peak_val2, int width2 ) {

	// check how many values contribute to final alignment
	if ( peak_val1 >= NCC_params->maxThr && width1 < NCC_params->INF_W ) // first value may be considered
		if ( peak_val2 >= NCC_params->maxThr && width2 < NCC_params->INF_W ) // second value may be considered too
			if ( abs(d1 - d2) < MIN(width1,width2) ) { // both values must be considered
				result->coord[i] = (int) floor((peak_val1 * d1 + peak_val2 * d2) / (peak_val1 + peak_val2) + 0.5); // weighted mean of alignments
				result->NCC_maxs[i] = (peak_val1 * peak_val1 + peak_val2 * peak_val2) / (peak_val1 + peak_val2); // weighted mean of reliabilities
				result->NCC_widths[i] = MAX(width1,width2); // maximum width
			}
			else { // only one value should be considered: take into account both peak value and peak width
				if ( peak_val1/width1 > peak_val2/width2 ) { // first value should be considered
					result->coord[i] = d1; 
					result->NCC_maxs[i] = peak_val1; 
					result->NCC_widths[i] = width1; 
				}
				else { // second value should be considered
					result->coord[i] = d2; 
					result->NCC_maxs[i] = peak_val2; 
					result->NCC_widths[i] = width2; 
				}
			}
		else { // only first value should be considered
			result->coord[i] = d1; 
			result->NCC_maxs[i] = peak_val1; 
			result->NCC_widths[i] = width1; 
		}
	else 
		if ( peak_val2 >= NCC_params->maxThr && width2 < NCC_params->INF_W ) { // only second value should be considered 
			result->coord[i] = d2; 
			result->NCC_maxs[i] = peak_val2; 
			result->NCC_widths[i] = width2; 
		}
		else { // none value is reliable
			result->coord[i] = NCC_params->INV_COORD;  // invalid coordinate
			result->NCC_maxs[i] = NCC_params->UNR_NCC; // unreliable NCC
			result->NCC_widths[i] = NCC_params->INF_W; // peak of infinite width
		}
}
/****************************************************************************************/



/*********** THREADS PAREMETERS AND CODE ************/

# ifdef _PAR_VERSION

# include <windows.h>

// parallel configuration ------------------------------
int n_procs = 0;
int par_degree = 0;

void init_configuration ( ) {
	fprintf(stdout,"--- PARALLEL VERSION RUNNING ---\n");
	n_procs = atoi(getenv("NUMBER_OF_PROCESSORS"));
	par_degree = n_procs;
}

// compute_3_MIPs --------------------------------------

typedef struct{
	// input parametres
	real_t *A;
	real_t *B;
	int dimi_v;
	int dimj_v;
	int dimk_v;
	int MIP_stridek;
	int stridei;
	int stridek;
	// input/ouput parameters
	// output parametres
	real_t *MIP_xy1;
	real_t *MIP_xz1;
	real_t *MIP_yz1;
	real_t *MIP_xy2;
	real_t *MIP_xz2;
	real_t *MIP_yz2;
} compute_3_MIPs_params_t;

DWORD WINAPI compute_3_MIPs_worker ( LPVOID lpParam ) {
	real_t *A       = ((compute_3_MIPs_params_t *) lpParam)->A;
	real_t *B       = ((compute_3_MIPs_params_t *) lpParam)->B;
	int dimi_v      = ((compute_3_MIPs_params_t *) lpParam)->dimi_v;
	int dimj_v      = ((compute_3_MIPs_params_t *) lpParam)->dimj_v;
	int dimk_v      = ((compute_3_MIPs_params_t *) lpParam)->dimk_v;
	int stridei     = ((compute_3_MIPs_params_t *) lpParam)->stridei;
	int stridek     = ((compute_3_MIPs_params_t *) lpParam)->stridek;
	int MIP_stridek = ((compute_3_MIPs_params_t *) lpParam)->MIP_stridek;
	real_t *MIP_xy1 = ((compute_3_MIPs_params_t *) lpParam)->MIP_xy1;
	real_t *MIP_xz1 = ((compute_3_MIPs_params_t *) lpParam)->MIP_xz1;
	real_t *MIP_yz1 = ((compute_3_MIPs_params_t *) lpParam)->MIP_yz1;
	real_t *MIP_xy2 = ((compute_3_MIPs_params_t *) lpParam)->MIP_xy2;
	real_t *MIP_xz2 = ((compute_3_MIPs_params_t *) lpParam)->MIP_xz2;
	real_t *MIP_yz2 = ((compute_3_MIPs_params_t *) lpParam)->MIP_yz2;

	real_t *vol1, *vol2;
	int i, j, k;

	// calcola MIP su xy, xz, yz scandendo una sola volta i due volumi
	for ( k=0, vol1=A, vol2=B; k<dimk_v; k++, vol1+=stridek, vol2+=stridek )
		for ( i=0; i<dimi_v; i++, vol1+=stridei, vol2+=stridei )
			for ( j=0; j<dimj_v; j++, vol1++, vol2++ ) {
				MIP_xy1[i*dimj_v+j] = MAX(MIP_xy1[i*dimj_v+j],*vol1); 
				MIP_xz1[i*MIP_stridek+k] = MAX(MIP_xz1[i*MIP_stridek+k],*vol1); // MIP stride along k dimension is the original MIP k dimension 
				MIP_yz1[j*MIP_stridek+k] = MAX(MIP_yz1[j*MIP_stridek+k],*vol1); // MIP stride along k dimension is the original MIP k dimension
				MIP_xy2[i*dimj_v+j] = MAX(MIP_xy2[i*dimj_v+j],*vol2);
				MIP_xz2[i*MIP_stridek+k] = MAX(MIP_xz2[i*MIP_stridek+k],*vol2); // MIP stride along k dimension is the original MIP k dimension
				MIP_yz2[j*MIP_stridek+k] = MAX(MIP_yz2[j*MIP_stridek+k],*vol2); // MIP stride along k dimension is the original MIP k dimension
			}

	return 0;
}

// compute_NCC_map --------------------------------------

typedef struct{
	// input parametres
	real_t *MIP_1; 
	real_t *MIP_2; 
	int dimu; 
	int dimv; 
	int delayu; 
	int delayv; 
	int u_start; 
	int v_start;	
	int u_end; 
	int v_end;	
	// input/ouput parameters
	// output parametres
	real_t *NCC_map; 
} compute_NCC_map_params_t;

DWORD WINAPI compute_NCC_map_worker ( LPVOID lpParam ) {
	real_t *MIP_1    = ((compute_NCC_map_params_t *) lpParam)->MIP_1;
	real_t *MIP_2    = ((compute_NCC_map_params_t *) lpParam)->MIP_2;
	int dimu         = ((compute_NCC_map_params_t *) lpParam)->dimu;
	int dimv         = ((compute_NCC_map_params_t *) lpParam)->dimv;
	int delayu       = ((compute_NCC_map_params_t *) lpParam)->delayu;
	int delayv       = ((compute_NCC_map_params_t *) lpParam)->delayv;
	int u_start      = ((compute_NCC_map_params_t *) lpParam)->u_start;
	int v_start      = ((compute_NCC_map_params_t *) lpParam)->v_start;
	int u_end        = ((compute_NCC_map_params_t *) lpParam)->u_end;
	int v_end        = ((compute_NCC_map_params_t *) lpParam)->v_end;
	real_t *NCC_map  = ((compute_NCC_map_params_t *) lpParam)->NCC_map;

	real_t *im1, *im2;
	int u, v;

	// nel seguito u=0 rappresenta il massimo scostamento negativo del secondo MIP rispetto al primo
	// con riferimento alla prima coordinata; v=0 ha il medesimo significato con riferimento alla seconda
	// coordinata
	for ( u=u_start; u<=u_end; u++ )
		for ( v=v_start; v<=v_end; v++ ) {
			im1 = MIP_1 + START_IND(u*dimv) + START_IND(v);
			im2 = MIP_2 + START_IND(-u*dimv) + START_IND(-v);
			NCC_map[(u+delayu)*(2*delayv+1)+(v+delayv)] = compute_NCC(im1,im2,dimu-abs(u),dimv-abs(v),abs(v));
		}

	return 0;
}

# endif


/************ OPERATIONS IMPLEMENTATION *************/

void compute_3_MIPs ( real_t *A, real_t *B,
					  real_t *MIP_xy1, real_t *MIP_xz1, real_t *MIP_yz1, 
					  real_t *MIP_xy2, real_t *MIP_xz2, real_t *MIP_yz2, 
					  int dimi_v, int dimj_v, int dimk_v, int stridei, int stridek ) {
# ifndef _PAR_VERSION

	real_t *vol1, *vol2;
	int i, j, k;

	// calcola MIP su xy, xz, yz scandendo una sola volta i due volumi
	for ( k=0, vol1=A, vol2=B; k<dimk_v; k++, vol1+=stridek, vol2+=stridek )
		for ( i=0; i<dimi_v; i++, vol1+=stridei, vol2+=stridei )
			for ( j=0; j<dimj_v; j++, vol1++, vol2++ ) {
				MIP_xy1[i*dimj_v+j] = MAX(MIP_xy1[i*dimj_v+j],*vol1);
				MIP_xz1[i*dimk_v+k] = MAX(MIP_xz1[i*dimk_v+k],*vol1);
				MIP_yz1[j*dimk_v+k] = MAX(MIP_yz1[j*dimk_v+k],*vol1);
				MIP_xy2[i*dimj_v+j] = MAX(MIP_xy2[i*dimj_v+j],*vol2);
				MIP_xz2[i*dimk_v+k] = MAX(MIP_xz2[i*dimk_v+k],*vol2);
				MIP_yz2[j*dimk_v+k] = MAX(MIP_yz2[j*dimk_v+k],*vol2);
			}

# else

	HANDLE *workerHandles = new HANDLE[par_degree];
	compute_3_MIPs_params_t *compute_3_MIPs_params = new compute_3_MIPs_params_t[par_degree];
	int t, i, j;

	/*
	 *  work decomposition is performed by partitioning the volum along the k (i.e. z) direction
	 *  each thread compute a portion of MIPS in xz and yz planes and a partial MIP in xy plane
	 *  partial MIPS are then merged
	 */

	// partition dimk_v
	int n1 = dimk_v / par_degree;
	int n2 = dimk_v % par_degree;
	for ( t=0; t<n2; t++ )
		compute_3_MIPs_params[t].dimk_v = n1 + 1;  
	for ( ; t<par_degree; t++ )
		compute_3_MIPs_params[t].dimk_v = n1;

	// allocate and initialize memory for partial MIP computation
	real_t **MIP_xy1_lst = new real_t *[par_degree];
	real_t **MIP_xy2_lst = new real_t *[par_degree];
	// first partial MIPs are stored in MIP_xy1 and MIP_xy2
	MIP_xy1_lst[0] = MIP_xy1; 
	MIP_xy2_lst[0] = MIP_xy2;
	for ( t=1; t<par_degree; t++ ) {
		MIP_xy1_lst[t] = new real_t[dimi_v*dimj_v];
		MIP_xy2_lst[t] = new real_t[dimi_v*dimj_v];
	}

	int slice_dim = dimi_v*(dimj_v+stridei)+stridek;
	/*
	 * number of pixels of one slice of volumes A and B
	 *
	 * case NORTH_SOUTH:
	 *    dimi_v  = dimi - ni 
	 *    dimj_v  = dimj
	 *    stridei = 0
	 *    stridek = ni*dimj    ====>
	 *      slice_dim = (dimi - ni) * dimj + ni * dimj = dimi * dimj   
	 * 
	 * case WEST_EAST:
	 *    dimi_v  = dimi 
	 *    dimj_v  = dimj - nj
	 *    stridei = nj
	 *    stridek = 0          ====>
	 *      slice_dim = dimi * ( dimj - nj + nj) + 0 = dimi * dimj   
	 */
	int depth = 0;
	for ( t=0; t<par_degree; t++ ) {
		compute_3_MIPs_params[t].A = A + depth*slice_dim;
		compute_3_MIPs_params[t].B = B + depth*slice_dim;
		compute_3_MIPs_params[t].dimi_v = dimi_v;
		compute_3_MIPs_params[t].dimj_v = dimj_v;
		compute_3_MIPs_params[t].stridei = stridei;
		compute_3_MIPs_params[t].stridek = stridek;
		compute_3_MIPs_params[t].MIP_stridek = dimk_v;
		compute_3_MIPs_params[t].MIP_xy1 = MIP_xy1_lst[t];
		compute_3_MIPs_params[t].MIP_xz1 = MIP_xz1 + depth;
		compute_3_MIPs_params[t].MIP_yz1 = MIP_yz1 + depth;
		compute_3_MIPs_params[t].MIP_xy2 = MIP_xy2_lst[t];
		compute_3_MIPs_params[t].MIP_xz2 = MIP_xz2 + depth;
		compute_3_MIPs_params[t].MIP_yz2 = MIP_yz2 + depth;

		workerHandles[t] = CreateThread( NULL, 0, compute_3_MIPs_worker, (compute_3_MIPs_params+t), 0, NULL);

		depth += compute_3_MIPs_params[t].dimk_v;
	}

	WaitForMultipleObjects(par_degree,workerHandles,TRUE,INFINITE);

	for ( t=0; t<par_degree; t++ ) 
		CloseHandle(workerHandles[t]);

	// compute global MIP_xy
	for ( t=1; t<par_degree; t++ )
		for ( i=0; i<dimi_v; i++ )
			for ( j=0; j<dimj_v; j++ ) {
				MIP_xy1[i*dimj_v + j] = MAX(MIP_xy1[i*dimj_v + j],MIP_xy1_lst[t][i*dimj_v + j]);
				MIP_xy2[i*dimj_v + j] = MAX(MIP_xy2[i*dimj_v + j],MIP_xy2_lst[t][i*dimj_v + j]);
			}

	// deallocation
	for ( t=1; t<par_degree; t++ ) {
		delete MIP_xy1_lst[t];
		delete MIP_xy2_lst[t];
	}
	delete MIP_xy1_lst;
	delete MIP_xy2_lst;

	delete compute_3_MIPs_params;
	delete workerHandles;
# endif
}


void compute_NCC_map ( real_t *NCC_map, real_t *MIP_1, real_t *MIP_2, 
					       int dimu, int dimv, int delayu, int delayv ) {
# ifndef _PAR_VERSION

	real_t *im1, *im2;
	int u, v;

	// nel seguito u=0 rappresenta il massimo scostamento negativo del secondo MIP rispetto al primo
	// con riferimento alla prima coordinata; v=0 ha il medesimo significato con riferimento alla seconda
	// coordinata
	for ( u=-delayu; u<=delayu; u++ )
		for ( v=-delayv; v<=delayv; v++ ) {
			im1 = MIP_1 + START_IND(u*dimv) + START_IND(v);
			im2 = MIP_2 + START_IND(-u*dimv) + START_IND(-v);
			NCC_map[(u+delayu)*(2*delayv+1)+(v+delayv)] = compute_NCC(im1,im2,dimu-abs(u),dimv-abs(v),abs(v));
		}

# else

	int npu, npv, nu1, nu2, nv1, nv2;
	int pu, u, pv, v, t, du, dv;

	// assume the processors are a power of 2
	int pow_2 = (int) floor(LOG2(par_degree));
	int par_degree_2 = (int) pow(2.0,pow_2);

	HANDLE *workerHandles = new HANDLE[par_degree_2];
	compute_NCC_map_params_t *compute_NCC_map_params = new compute_NCC_map_params_t[par_degree_2];

	/*
	 *  work decomposition is done by partitioning the NCC maps
	 *  maps are halved repeatedly until floor(log2(#procs)) blocks are generated
	 *  each thread is given the indices of one block and computes the NCCs corresponding to that block
	 */

	npv = pow_2 / 2;          
	npu = npv + (pow_2 % 2);  
	npv = (int) pow(2.0,npv); // partitions along v
	npu = (int) pow(2.0,npu); // partitions along u 

	nu1 = (2*delayu+1) / npu; // minimum number of rows per partition along u
	nu2 = (2*delayu+1) % npu; // number of partitions along u that have one more row
	nv1 = (2*delayv+1) / npv; // minimum number of rows per partition along v
	nv2 = (2*delayu+1) % npv; // number of partitions along v that have one more column

	for ( pu=0, u=-delayu, t=0; pu<npu; pu++, u+=du ) {
		du = (pu<nu2) ? (nu1+1) : nu1;
		for ( pv=0, v=-delayv ; pv<npv; pv++, v+=dv, t++ ) {
			dv = (pv<nv2) ? (nv1+1) : nv1;
			compute_NCC_map_params[t].u_start = u; 
			compute_NCC_map_params[t].v_start = v; 
			compute_NCC_map_params[t].u_end = u + (du-1);	
			compute_NCC_map_params[t].v_end = v + (dv-1);
		}
	}

	for ( t=0; t<par_degree_2; t++ ) {
		compute_NCC_map_params[t].MIP_1 = MIP_1; 
		compute_NCC_map_params[t].MIP_2 = MIP_2; 
		compute_NCC_map_params[t].dimu = dimu; 
		compute_NCC_map_params[t].dimv = dimv; 
		compute_NCC_map_params[t].delayu = delayu; 
		compute_NCC_map_params[t].delayv = delayv; 
		compute_NCC_map_params[t].NCC_map = NCC_map; 

		workerHandles[t] = CreateThread( NULL, 0, compute_NCC_map_worker, (compute_NCC_map_params+t), 0, NULL);
	}

	WaitForMultipleObjects(par_degree_2,workerHandles,TRUE,INFINITE);

	for ( t=0; t<par_degree_2; t++ ) 
		CloseHandle(workerHandles[t]);

	// deallocation
	delete compute_NCC_map_params;
	delete workerHandles;

# endif
}


real_t compute_NCC ( real_t *im1, real_t *im2, int dimi, int dimj, int stride ) {
// parallelization of compute_NCC_map makes parallelization of this operation pointless
	real_t f_mean, t_mean, f_prime, t_prime, numerator, factor1, factor2;
	real_t *pxl1, *pxl2;
	int i, j;

	// computes means
	f_mean = t_mean = 0;
	for ( i=0, pxl1=im1, pxl2=im2; i<dimi; i++, pxl1+=stride, pxl2+=stride )
		for ( j=0; j<dimj; j++, pxl1++, pxl2++ ) {
			f_mean += *pxl1;
			t_mean += *pxl2;
		}
	f_mean /= (dimi*dimj);
	t_mean /= (dimi*dimj);

	// applies the optimization at the beginning of section 5 of Lewis article (t_prime has zero mean)
	numerator = 0;
	factor1 = 0;
	factor2 = 0;
	for ( i=0, pxl1=im1, pxl2=im2; i<dimi; i++, pxl1+=stride, pxl2+=stride )
		for ( j=0; j<dimj; j++, pxl1++, pxl2++ ) {
			f_prime = *pxl1 - f_mean;
			t_prime = *pxl2 - t_mean;
			numerator += *pxl1 * t_prime;
			factor1 += f_prime * f_prime;
			factor2 += t_prime * t_prime;
		}

	return numerator / sqrt(factor1*factor2);
}


int compute_MAX_ind ( real_t *vect, int len ) {
// actual len values are too small to deserve parallelization 
	int i;
	real_t val_max = vect[0];
	int ind_max = 0;
	for ( i=0; i<len; i++ )
		if ( vect[i] > val_max ) {
			val_max = vect[i];
			ind_max = i;
		}
	return ind_max;
}


void compute_Neighborhood ( NCC_parms_t *NCC_params, real_t *NCC, int delayu, int delayv, int ind_max, 
						   real_t *MIP_1, real_t *MIP_2, int dimu, int dimv, real_t *NCCnew, int &du, int &dv, bool &failed) {

	// suffixes u and v denote the vertical and the horizontal dimensions, respectively
	// suffix i denotes linear indices

	int u, v, i, d; // for variables

	int newu = NCC_params->wRangeThr; // vertical half dimension of NCCnew 
	int newv = NCC_params->wRangeThr; // horizontal half dimension of NCCnew

	int ind_ref; // index of the center of NCCnew

	int initu; // vertical index of first pixel of subregion of NCC to be used to initially fill NCCnew
	int initv; // horizontal index of first pixel of subregion of NCC to be used to initially fill NCCnew
	int initi; // linear index of first pixel of subregion of NCC to be used to initially fill NCCnew

	int srcStartu; // vertical index of first pixel of the subregion of NCCnew to be reused when current maximum is moved to the center of NCCnew
	int srcStartv; // horizontal index of first pixel of the subregion of NCCnew to be reused when current maximum is moved to the center of NCCnew
	int srcStarti; // linear index of first pixel of the subregion of NCCnew to be reused when current maximum is moved to the center of NCCnew

	int dstStartu; // vertical index of first pixel of the subregion of NCCnew where te subregion to be used has to be copied
	int dstStartv; // horizontal index of first pixel of the subregion of NCCnew where te subregion to be used has to be copied
	int dstStarti; // linear index of first pixel of the subregion of NCCnew where te subregion to be used has to be copied

	int deltau; // vertical displacement of current maximum from the center of NCCnew
	int deltav; // horizontal displacement of current maximum from the center of NCCnew

	int firstv; // first horizontal index for copying elements of NCCnew to be reused
	int lastv;  // last horizontal index for copying elements of NCCnew to be reused

	int n_miss; // number of NCC to be computed to fill NCCnew
	int *missu = new int[(2*newu+1)*(2*newv+1)]; // list of vertical indices of NCC to be computed to fill NCCnew
	int *missv = new int[(2*newu+1)*(2*newv+1)]; // list of vertical indices of NCC to be computed to fill NCCnew

	real_t *im1, *im2;

	// INITIALIZATION

	// fill NCCnew copying useful NCCs that have been already computed from NCC to NCCnew
	initu = MIN(MAX(0,ind_max/(2*delayv+1) - newu),2*(delayu - newu));
	initv = MIN(MAX(0,ind_max%(2*delayv+1) - newv),2*(delayv - newv));
	initi = initu * (2*delayv+1) + initv;
	for ( u=0, i=0, d=0; u<(2*newu+1); u++, d+=2*(delayv-newv) ) // when row changes 2*(delayv-newv) values have to be skipped
		for ( v=0; v<(2*newv+1); v++ , i++)
			NCCnew[i] = NCC[i + initi + d];
	// compute displacement of the center of NCCnew with respect to the initial alignment (center of NCC)
	du = initu - delayu + newu; // displacement of first row + half dimension of NCCnew
	dv = initv - delayv + newv; // displacement of first column + half dimension of NCCnew
	// update ind_max with respect to NCCnew
	//      contribution due to rows (integer division is not distributive)               contribution due to columns
	ind_max = (2*newv+1) * (ind_max/(2*delayv+1) - initi/(2*delayv+1))   +   (ind_max%(2*delayv+1)) - (initi%(2*delayv+1)); 
	// index of the center of the new NCC
	ind_ref = (2*newv+1) * newu + newv; 

	// UPDATE NEIGHBORHOOD AND SEARCH MAXIMUM 

	int c=1;
	while ( c < NCC_params->maxIter && ind_max != ind_ref ) {
		// update NCCnew 
		srcStartu = MAX(0,ind_max/(2*newv+1) - newu);
		srcStartv = MAX(0,ind_max%(2*newv+1) - newv);
		srcStarti = srcStartu * (2*newv+1) + srcStartv;
		deltau = ind_max/(2*newv+1) - ind_ref/(2*newv+1);
		deltav = ind_max%(2*newv+1) - ind_ref%(2*newv+1);
		dstStartu = srcStartu - deltau;
		dstStartv = srcStartv - deltav;
		dstStarti = dstStartu * (2*newv+1) + dstStartv;
		if ( srcStartu > 0 ) { // forward copy is safe
			i = 0;
			for ( u=0; u<((2*newu+1)-abs(deltau)); u++, i+=abs(deltav) ) // when row changes |deltav| values have to be skipped
				for ( v=0; v<((2*newv+1)-abs(deltav)); v++ , i++)
					NCCnew[i + dstStarti] = NCCnew[i + srcStarti];
		}
		else { // srcStartu == 0 : beckward copy is safe
			i=(((2*newu+1)-abs(deltau))*((2*newv+1)-abs(deltav)) - 1);
			for ( u=0; u<((2*newu+1)-abs(deltau)); u++, i+=abs(deltav) ) // when row changes |deltav| values have to be skipped
				for ( v=0; v<((2*newv+1)-abs(deltav)); v++ , i--)
					NCCnew[i + dstStarti] = NCCnew[i + srcStarti];
		}

		// update position of the new center (current maximum)
		du += deltau;
		dv += deltav;

		// generate list of missing NCCs
		n_miss = 0;
		for ( u=0; u<(2*newu+1); u++ ) { 
			if ( u+deltau < 0 || u+deltau >= (2*newu+1) ) { // all this row has to be computed
				firstv = 0;
				lastv  = 2*newv+1;
			}
			else { // only a fraction of the row has to be computed
				if ( deltav < 0 ) { // the initial part of the row has to be computed
					firstv = 0;
					lastv  = -deltav;
				}
				else if ( deltav > 0 ) { // the final part of the row has to be computed
					firstv = (2*newv+1) - deltav; 
					lastv  = 2*newv+1;
				}
				else { // deltav == 0: no NCCs has to be computed
					firstv = 0;
					lastv  = 0;
				}
			}
			for ( v=firstv; v<lastv; v++ ) {
				missu[n_miss] = u;
				missv[n_miss] = v;
				n_miss++;
			}
		}
		// CHECK, MUST BE: n_miss == ((2*newu+1)*(2*newv+1) - ((2*newu+1)-abs(deltau))*((2*newv+1)-abs(deltav)))
		if ( n_miss != ((2*newu+1)*(2*newv+1) - ((2*newu+1)-abs(deltau))*((2*newv+1)-abs(deltav))) )
			DISPLAY_ERROR("incomplete NCC map in compute_Neighborhood");

		// compute missing NCCs
		for ( i=0; i<n_miss; i++ ) {
			// indices over MIPs have to be shifted to take into account their relative position with respecto to the center of NCCnew
			// and the relative position of the center with respect to the initial initial alignment (center of NCC)
			u = missu[i] - newu + du;
			v = missv[i] - newv + dv;
			im1 = MIP_1 + START_IND(u*dimv) + START_IND(v);
			im2 = MIP_2 + START_IND(-u*dimv) + START_IND(-v);
			NCCnew[missu[i]*(2*newv+1)+missv[i]] = compute_NCC(im1,im2,dimu-abs(u),dimv-abs(v),abs(v));
		}

		// find maximum 
		ind_max = compute_MAX_ind(NCCnew,(2*newu+1)*(2*newv+1));

		c++;
	}

	if(ind_ref != ind_max)
	{
		deltau = ind_max/(2*newv+1) - ind_ref/(2*newv+1);
		deltav = ind_max%(2*newv+1) - ind_ref%(2*newv+1);

		// update position of the new center (current maximum)
		du += deltau;
		dv += deltav;

		failed=true;
	}

	delete missu;
	delete missv;
}


//void compute_Alignment( NCC_parms_t *NCC_params, REAL_T *NCC_xy, REAL_T *NCC_xz, REAL_T *NCC_yz,
//					    int dimi, int dimj, int dimk, int ind_xy, int ind_xz, int ind_yz, NCC_descr_t *result) {
void compute_Alignment( NCC_parms_t *NCC_params, real_t *NCC_xy, real_t *NCC_xz, real_t *NCC_yz,
					    int dimi, int dimj, int dimk, int dx1, int dx2, int dy1, int dy2, int dz1, int dz2, bool failed_xy, bool failed_xz, bool failed_yz, NCC_descr_t *result) {

	int w1x, w2x, w1y, w2y, w1z, w2z;

	compute_NCC_width(NCC_params,NCC_xy,(2*dimj+1),(dimi*(2*dimj+1)+dimj),failed_xy, w1y,w1x);
	compute_NCC_width(NCC_params,NCC_xz,(2*dimk+1),(dimi*(2*dimk+1)+dimk),failed_xz, w1z,w2x);
	compute_NCC_width(NCC_params,NCC_yz,(2*dimk+1),(dimj*(2*dimk+1)+dimk),failed_yz, w2z,w2y);

	compute_NCC_alignment(NCC_params,result,0,dx1,NCC_xy[(dimi*(2*dimj+1)+dimj)],w1x,dx2,NCC_xz[(dimi*(2*dimk+1)+dimk)],w2x);
	compute_NCC_alignment(NCC_params,result,1,dy1,NCC_xy[(dimi*(2*dimj+1)+dimj)],w1y,dy2,NCC_yz[(dimj*(2*dimk+1)+dimk)],w2y);
	compute_NCC_alignment(NCC_params,result,2,dz1,NCC_xz[(dimi*(2*dimk+1)+dimk)],w1z,dz2,NCC_yz[(dimj*(2*dimk+1)+dimk)],w2z);
}


void enhance ( real_t *im, int imLen, int graylevels, NCC_parms_t *NCC_params ) {
/*
 * the enhancement transformation is a multi-linear curve with n_transforms rescaled linear
 * tranformations
 * for i=0, ..., (n_transforms-1), percentiles[i] contains the fraction of pixels to which the 
 * (i+1)-th rescaled linear transformation has to be applied; percentiles[n_transforms-1] must be 1.00
 * for i=1, ..., n_transforms, c[i-1] amd c[i] contain the lowest and highest value corresponding to 
 * the the i-th rescaled linear transformation; c[0] must be 0.00 and c[n_transforms] must be 1.00
 */

	int n_transforms = NCC_params->n_transforms;
	real_t *percentiles = NCC_params->percents;
	real_t *c = NCC_params->c; // tranformed values of thresholds
	real_t *thresholds = new real_t[n_transforms+1];
	real_t *a = new real_t[n_transforms+1];
	real_t *b = new real_t[n_transforms+1];
	int i, j;
	bool found;

	thresholds[0] = (real_t)0.00;

	stack_percentiles(im,imLen,graylevels,percentiles,thresholds,n_transforms);

	for ( i=1; i<=n_transforms; i++ ) {
		a[i] = (c[i] - c[i-1]) / (thresholds[i] - thresholds[i-1]);
		b[i] = c[i] - a[i]*thresholds[i];
	}

	for ( i=0; i<imLen; i++ ) {
		binary_search(thresholds,n_transforms,im[i],found,j);
		im[i] = a[j]*im[i] + b[j];
	}

	delete thresholds;
	delete a;
	delete b;
}


void stack_percentiles ( real_t *im, int imLen, int graylevels, 
						 real_t *percentiles, real_t *thresholds, int n_percentiles ) {

	real_t d = (real_t)1.0 / (real_t)graylevels;
	real_t *cumsum = new real_t[graylevels];
	int i, j;

	for ( i=0; i<graylevels; i++ )
		cumsum[i] = 0;

	for ( i=0; i<imLen; i++ ) {
		j = MIN((int)floor(im[i]/d),graylevels-1); // guarantees that index is within limits
		cumsum[j]++;
	}

	cumsum[0] /= imLen;
	for ( i=1; i<graylevels; i++ ) 
		cumsum[i] = cumsum[i]/imLen + cumsum[i-1];

	// guarantees that last cumulative fraction is 1.0
	cumsum[graylevels-1] = 1.0;

	for ( i=0, j=1; j<n_percentiles; ) {
		if ( i == graylevels ) {
			DISPLAY_ERROR("i out of limits");
			exit(1);
		}
		if ( cumsum[i] >= percentiles[j-1] ) {
			thresholds[j] = d * i;
			j++;
		}
		else
			i++;
	}
	thresholds[n_percentiles] = (real_t)1; // to avoid round off errors

	delete cumsum;
}
