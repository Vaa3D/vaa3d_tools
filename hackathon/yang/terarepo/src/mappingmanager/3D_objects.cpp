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
*       Bria, A., Iannello, G., "TeraStitcher - A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Microscopy Images", submitted for publication, 2012.
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
 * 3D_objects.cpp
 *
 * created August 13, 2012 by Giulio Iannello
 * changed May 02, 2016 by Pierangelo Afferni
 */

# include "3D_objects.h"

# include <math.h>
# include <stdlib.h>
//# include <stdio.h>


# define GETINDS(ind)                    ( inds+(3*ind) )
# define GET_VALUE_3D(vol,hdimi,hdimj,hdimk,i,j,k) \
	( *(vol + ((j)+(hdimj)) + ((i)+(hdimi))*(2*(hdimj)+1) + ((k)+(hdimk))*(2*(hdimi)+1)*(2*(hdimj)+1)) )
# define GET_UNNORM_VERSOR_COMPONENT()   ( 2.0 * (((double) rand())/((double) RAND_MAX)) - 1.0)
# define ABS_FLOOR(x)                    ( ((x)>=0) ? (floor(x)): (ceil(x)) )
# define ABS_CEIL(x)                     ( ((x)>=0) ? (ceil(x)): (floor(x)) )
# define ABS_ROUND(x)                    ( ((x)>=0) ? (floor((x)+0.5)): (ceil((x)-0.5)) )


void best_sort( double n_[], int org_[], int ind_[] ) {};


void Drawable_object_3D::linearize ( int _stridej, int _stridek ) {
	stridej = _stridej;
	stridek = _stridek;
	linds = new int[n];
	for ( int i=0; i<n; i++ )
		linds[i] = GETINDS(i)[1] + (GETINDS(i)[0] * stridej) + (GETINDS(i)[2] * stridek);
}


void Drawable_object_3D::draw ( BASETYPE *im, int _dimi, int _dimj, int _dimk, int _i, int _j, int _k ) {
	if ( (stridej == _dimj) && (stridek == (_dimi * _dimj)) ) {
		int imInd = _j + (_i * stridej) + (_k * stridek); // linear index in im
		// check if object is totally internal
		if ( (extension[0][0] <= _i) && (_i < (_dimi - extension[0][1])) && 
			 (extension[1][0] <= _j) && (_j < (_dimj - extension[1][1])) && 
			 (extension[2][0] <= _k) && (_k < (_dimk - extension[2][1]))    ) { // is totally internal
			for ( int i=0; i<n; i++ ) {
				im[imInd+linds[i]] = myMAX(im[imInd+linds[i]],vals[i]);
			}
		}
		else { // is partially external
			for ( int i=0; i<n; i++ ) {
				if ( (0 <= (_i + GETINDS(i)[0])) && ((_i + GETINDS(i)[0]) < _dimi) && 
					 (0 <= (_j + GETINDS(i)[1])) && ((_j + GETINDS(i)[1]) < _dimj) && 
					 (0 <= (_k + GETINDS(i)[2])) && ((_k + GETINDS(i)[2]) < _dimk)    )
				im[imInd+linds[i]] = myMAX(im[imInd+linds[i]],vals[i]);
			}
		}
	}
	else
		DISPLAY_ERROR("wrong strides: drowable object cannot be drawn");
}


// SPHERE OBJECT

Sphere_object_3D::Sphere_object_3D ( int radius ) {
	int i, j, k;

	// allocates overdimensioned space
	inds = new int[3*(2*radius+1)*(2*radius+1)*(2*radius+1)]; 
	vals = new BASETYPE[(2*radius+1)*(2*radius+1)*(2*radius+1)];  

	// explore a cube of side (2*radius+1) and adds only offsets corresponding to a sphere
	n = 0;
	for ( k=-radius; k<=radius; k++ ) {
		for ( i=-radius; i<=radius; i++ ) {
			for ( j=-radius; j<=radius; j++ )
				if ( (i*i + j*j + k*k) <= radius*radius ) {
					GETINDS(n)[0] = i;
					GETINDS(n)[1] = j;
					GETINDS(n)[2] = k;
					vals[n++] = (BASETYPE) 1.0;
				}
		}
	}

	extension[0][BEFORE_DIRECTION] = radius;
	extension[1][BEFORE_DIRECTION] = radius;
	extension[2][BEFORE_DIRECTION] = radius;
	extension[0][AFTER_DIRECTION]  = radius;
	extension[1][AFTER_DIRECTION]  = radius;
	extension[2][AFTER_DIRECTION]  = radius;
}


// GAUSSIAN BEAD OBJECT

GaussianBead_object_3D::GaussianBead_object_3D ( int sigma ) {

	int radius = (int) ceil(sqrt(((double) sigma) * -log(ZERO_PXL)));
	//double radius = log(ZERO_PXL);
	int i, j, k;

	// allocates overdimensioned space
	inds = new int[3*(2*radius+1)*(2*radius+1)*(2*radius+1)]; 
	vals = new BASETYPE[(2*radius+1)*(2*radius+1)*(2*radius+1)];  

	// explore a cube of side (2*radius+1) and adds only offsets corresponding to a sphere
	n = 0;
	for ( k=-radius; k<=radius; k++ ) {
		for ( i=-radius; i<=radius; i++ ) {
			for ( j=-radius; j<=radius; j++ ) {
				vals[n] = (BASETYPE) exp( -(i*i + j*j + k*k)/((double) sigma) );
				if ( vals[n] >= ZERO_PXL ) {
					GETINDS(n)[0] = i;
					GETINDS(n)[1] = j;
					GETINDS(n)[2] = k;
					n++;
				}
			}
		}
	}

	extension[0][0] = radius;
	extension[1][0] = radius;
	extension[2][0] = radius;
	extension[0][1] = radius;
	extension[1][1] = radius;
	extension[2][1] = radius;
}


// SPHERE WITH RAYS OBJECT

RayedSphere_object_3D::RayedSphere_object_3D ( int radius, int n_rays, int ray_length, int ray_radius ) {
	int i, j, k;
	static bool initialized = false;

	double n_i, n_j, n_k; // ray direction components
	double norm;          // normalization factor
	double ray_length_r = (double) ray_length;
	double u;    // linear coordinate over rays
	double step_l; // increment in drawing rays	
	//double n2_jk, n2_ijk; // sums of squares of versors components
	//double np_i, np_j; // versor components of normal plane to ray
	BASETYPE *cube;
	int ii, jj, kk;
	Sphere_object_3D ray_gen(ray_radius);
	Drawable_object_3D ray_gen_d(ray_gen);

	ray_gen_d.linearize((2*ray_length+1),(2*ray_length+1)*(2*ray_length+1));

	cube = new BASETYPE[(2*ray_length+1)*(2*ray_length+1)*(2*ray_length+1)];
	memset(cube,0,sizeof(BASETYPE)*(2*ray_length+1)*(2*ray_length+1)*(2*ray_length+1));

	// explore a cube of side (2*radius+1) and adds only offsets corresponding to a sphere
	for ( k=-radius; k<=radius; k++ ) {
		for ( i=-radius; i<=radius; i++ ) {
			for ( j=-radius; j<=radius; j++ )
				if ( (i*i + j*j + k*k) <= radius*radius ) {
					GET_VALUE_3D(cube,ray_length,ray_length,ray_length,i,j,k) = (BASETYPE) 1.0;
				}
		}
	}

	// draw rays
	if ( !initialized ) {
		srand(55);
		initialized = true;
	}


	for ( i=0; i<n_rays; i++ ) {
		// calculate random direction
		n_i = GET_UNNORM_VERSOR_COMPONENT();
		n_j = GET_UNNORM_VERSOR_COMPONENT();
		n_k = GET_UNNORM_VERSOR_COMPONENT();
		norm = sqrt(n_i*n_i + n_j*n_j + n_k*n_k);
		n_i /= norm;
		n_j /= norm;
		n_k /= norm;
		//printf("(%f,%f,%f)\n",n_i,n_j,n_k);

		// compute step along direction (n_i,n_j,n_k)
		step_l = 1.0 / myMAX(myMAX(myABS(n_i),myABS(n_j)),myABS(n_k)); // step used for length

		// draw ray axis
		for ( u=0; u<ray_length_r; u+=step_l ) {
			if ( ceil(u) >= (radius-ray_radius) ) {
				ii = (int) ABS_ROUND(u*n_i) + ray_length;
				jj = (int) ABS_ROUND(u*n_j) + ray_length;
				kk = (int) ABS_ROUND(u*n_k) + ray_length;
				ray_gen_d.draw(cube,(2*ray_length+1),(2*ray_length+1),(2*ray_length+1),ii,jj,kk);
				//GET_VALUE_3D(cube,ray_length,ray_length,ray_length,(int)ABS_ROUND(u*n_i),(int)ABS_ROUND(u*n_j),(int)ABS_ROUND(u*n_k)) = (BASETYPE) 1.0;
			}
		}
	}

	// allocates overdimensioned space
	inds = new int[3*(2*ray_length+1)*(2*ray_length+1)*(2*ray_length+1)]; 
	vals = new BASETYPE[(2*ray_length+1)*(2*ray_length+1)*(2*ray_length+1)];  

	n = 0;
	for ( k=-ray_length; k<=ray_length; k++ ) {
		for ( i=-ray_length; i<=ray_length; i++ ) {
			for ( j=-ray_length; j<=ray_length; j++ )
				if ( GET_VALUE_3D(cube,ray_length,ray_length,ray_length,i,j,k) ) {
					GETINDS(n)[0] = i;
					GETINDS(n)[1] = j;
					GETINDS(n)[2] = k;
					vals[n++] = (BASETYPE) 1.0;
				}
		}
	}

	extension[0][BEFORE_DIRECTION] = ray_length;
	extension[1][BEFORE_DIRECTION] = ray_length;
	extension[2][BEFORE_DIRECTION] = ray_length;
	extension[0][AFTER_DIRECTION]  = ray_length;
	extension[1][AFTER_DIRECTION]  = ray_length;
	extension[2][AFTER_DIRECTION]  = ray_length;

	delete[] cube;
}

/*
RayedSphere_object_3D::RayedSphere_object_3D ( int radius, int n_rays, int ray_length, int ray_radius ) {
	int i, j, k;
	static bool initialized = false;

	double n_i, n_j, n_k; // ray direction components
	double n_[3];
	int org_[3];
	int ind_[3];
	double norm;          // normalization factor
	double ray_length_r = (double) ray_length;
	double u;    // linear coordinate over rays
	double step_s; // increment in drawing sections
	double step_l; // increment in drawing rays	
	double n2_jk, n2_ijk; // sums of squares of versors components
	double np_i, np_j; // versor components of normal plane to ray
	double x_start, x_end, y_start, y_end; // ranges for x and y in drawing ray initial section
	double x_cap, y, z; // coordinates of points in the ray initial section

	int *section_inds = new int[30*(2*ray_radius+1)*(2*ray_radius+1)]; // a conservative factor 10 is used  
	int n_s_inds;

	// allocates overdimensioned space
	inds = new int[3*(2*ray_length+1)*(2*ray_length+1)*(2*ray_length+1)]; 
	vals = new BASETYPE[(2*ray_length+1)*(2*ray_length+1)*(2*ray_length+1)];  

	// explore a cube of side (2*radius+1) and adds only offsets corresponding to a sphere
	n = 0;
	for ( k=-radius; k<=radius; k++ ) {
		for ( i=-radius; i<=radius; i++ ) {
			for ( j=-radius; j<=radius; j++ )
				if ( (i*i + j*j + k*k) <= radius*radius ) {
					GETINDS(n)[0] = i;
					GETINDS(n)[1] = j;
					GETINDS(n)[2] = k;
					vals[n++] = (BASETYPE) 1.0;
				}
		}
	}

	// draw rays
	if ( !initialized ) {
		srand(10);
		initialized = true;
	}
	for ( i=0; i<n_rays; i++ ) {
		// calculate random direction
		n_[0] = GET_UNNORM_VERSOR_COMPONENT();
		n_[1] = GET_UNNORM_VERSOR_COMPONENT();
		n_[2] = GET_UNNORM_VERSOR_COMPONENT();

		//n_[0] = 1/sqrt(3.0);
		//n_[1] = 1/sqrt(3.0);
		//n_[2] = 1/sqrt(3.0);

		// sort in decreasing order (to avoid anomalies in disc drawing
		best_sort(n_,org_,ind_);
		n_i = n_[0];
		n_j = n_[1];
		n_k = n_[2];

		norm = sqrt(n_i*n_i + n_j*n_j + n_k*n_k);
		n_i /= norm;
		n_j /= norm;
		n_k /= norm;
		n2_jk = n_j*n_j + n_k*n_k;
		n2_ijk = n_i*n_i + n_j*n_j + n_k*n_k;
		step_s = myMIN(abs(n_i),abs(n_j)); // step used for section
		step_s = 0.67 / sqrt(1 - step_s*step_s);
		np_i = sqrt(1-n_i*n_i);
		np_j = sqrt(1-n_j*n_j);

		// calculate initial section of ray
		x_start = -sqrt(n2_jk / n2_ijk) * ray_radius;
		x_end   =  sqrt(n2_jk / n2_ijk) * ray_radius;

		n_s_inds = 0;
		for ( x_cap=x_start; x_cap<=x_end; x_cap+=(step_s*np_i) ) {
			// myMAX is used to prevent round off errors in (n2_jk*ray_radius*ray_radius - x_cap*x_cap)
			y_start = myMIN( -n_i*n_k*x_cap + n_k*sqrt(myMAX(0,n2_jk*ray_radius*ray_radius - x_cap*x_cap)),
							 -n_i*n_k*x_cap - n_k*sqrt(myMAX(0,n2_jk*ray_radius*ray_radius - x_cap*x_cap)) );
			y_end   = myMAX( -n_i*n_k*x_cap + n_k*sqrt(myMAX(0,n2_jk*ray_radius*ray_radius - x_cap*x_cap)),
							 -n_i*n_k*x_cap - n_k*sqrt(myMAX(0,n2_jk*ray_radius*ray_radius - x_cap*x_cap)) );
			for ( y=y_start; y<=y_end; y+=(step_s*np_j) ) {
				z = -(n_i*x_cap + n_j*y) / n_k;
				(section_inds+(3*n_s_inds))[ind_[0]] = (int) ABS_ROUND(x_cap);
				(section_inds+(3*n_s_inds))[ind_[1]] = (int) ABS_ROUND(y);
				(section_inds+(3*n_s_inds))[ind_[2]] = (int) ABS_ROUND(z);
				n_s_inds++;
				//GETINDS(n)[ind_[0]] = (int) ABS_ROUND(x_cap);
				//GETINDS(n)[ind_[1]] = (int) ABS_ROUND(y);
				//GETINDS(n)[ind_[2]] = (int) ABS_ROUND(z);
				//vals[n++] = (BASETYPE) 1.0;
			}
		}

		// reset versor components and compute step of the linear coordinate 
		n_i = n_[org_[0]];
		n_j = n_[org_[1]];
		n_k = n_[org_[2]];
		norm = sqrt(n_i*n_i + n_j*n_j + n_k*n_k);
		n_i /= norm;
		n_j /= norm;
		n_k /= norm;
		step_l = 0.67 / myMAX(myMAX(abs(n_i),abs(n_j)),abs(n_k)); // step used for length

		for ( j=0; j<n_s_inds; j++ ) {
			// draw ray axis
			for ( u=0; u<ray_length_r; u+=step_l ) {
				if ( ceil(u) >= radius ) {
					GETINDS(n)[0] = (int) ABS_ROUND(u*n_i) + (section_inds+(3*j))[0];
					GETINDS(n)[1] = (int) ABS_ROUND(u*n_j) + (section_inds+(3*j))[1];
					GETINDS(n)[2] = (int) ABS_ROUND(u*n_k) + (section_inds+(3*j))[2];
					vals[n++] = (BASETYPE) 1.0;
				}
			}
		}
	}

	extension[0][BEFORE_DIRECTION] = ray_length;
	extension[1][BEFORE_DIRECTION] = ray_length;
	extension[2][BEFORE_DIRECTION] = ray_length;
	extension[0][AFTER_DIRECTION]  = ray_length;
	extension[1][AFTER_DIRECTION]  = ray_length;
	extension[2][AFTER_DIRECTION]  = ray_length;

	delete[] section_inds; 
}


void best_sort( double n_[], int org_[], int ind_[] ) {
	double temp;

	if ( (abs(n_[0]) <= abs(n_[1])) && (abs(n_[1]) <= abs(n_[2])) ) {
		org_[0] = 0; org_[1] = 1, org_[2] = 2;
		ind_[0] = 0; ind_[1] = 1, ind_[2] = 2;
	}
	else if ( (abs(n_[0]) <= abs(n_[2])) && (abs(n_[2]) <= abs(n_[1])) ) {
		temp = n_[1]; n_[1] = n_[2]; n_[2] = temp;
		org_[0] = 0; org_[1] = 2, org_[2] = 1;
		ind_[0] = 0; ind_[1] = 2, ind_[2] = 1;
	}
	else if ( (abs(n_[1]) <= abs(n_[0])) && (abs(n_[0]) <= abs(n_[2])) ) {
		temp = n_[0]; n_[0] = n_[1]; n_[1] = temp;
		org_[0] = 1; org_[1] = 0, org_[2] = 2;
		ind_[0] = 1; ind_[1] = 0, ind_[2] = 2;
	}
	else if ( (abs(n_[1]) <= abs(n_[2])) && (abs(n_[2]) <= abs(n_[0])) ) {
		temp = n_[0]; n_[0] = n_[1]; n_[1] = n_[2]; n_[2] = temp;
		org_[0] = 2; org_[1] = 0, org_[2] = 1;
		ind_[0] = 1; ind_[1] = 2, ind_[2] = 0;
	}
	else if ( (abs(n_[2]) <= abs(n_[0])) && (abs(n_[0]) <= abs(n_[1])) ) {
		temp = n_[0]; n_[0] = n_[2]; n_[2] = n_[1]; n_[1] = temp;
		org_[0] = 1; org_[1] = 2, org_[2] = 0;
		ind_[0] = 2; ind_[1] = 0, ind_[2] = 1;
	}
	else if ( (abs(n_[2]) <= abs(n_[1])) && (abs(n_[1]) <= abs(n_[0])) ) {
		temp = n_[0]; n_[0] = n_[2]; n_[2] = temp;
		org_[0] = 2; org_[1] = 1, org_[2] = 0;
		ind_[0] = 2; ind_[1] = 1, ind_[2] = 0;
	}
}
*/
