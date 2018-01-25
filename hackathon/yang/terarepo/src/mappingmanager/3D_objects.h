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
 * 3D_objects.h
 *
 * created August 13, 2012 by Giulio Iannello
 * changed May 02, 2016 by Pierangelo Afferni
 */

# ifndef OBJECTS_3D_H
# define OBJECTS_3D_H

# include "lib3Dobjects_defs.h"

# include <string.h>

# define BEFORE_DIRECTION   0
# define AFTER_DIRECTION    1

# define GRAYLEVELS   256   // values less than 1/GRAYLEVELS may be set to zero
# define ZERO_PXL     ((double) (1.0 / ((double) GRAYLEVELS)))


class Object_3D {
protected:
	int n;
	int *inds;
	BASETYPE *vals;
	int extension[3][2];

public:
	Object_3D ( ) { inds = (int *) 0; vals = (BASETYPE *) 0; n=0;};
	Object_3D ( const Object_3D& ex_instance ) {
		n = ex_instance.n;
		if ( ex_instance.inds ) {
			inds = new int[3*n];
			memcpy(inds,ex_instance.inds,3*n*sizeof(int));
			vals = new BASETYPE[n];
			memcpy(vals,ex_instance.vals,n*sizeof(BASETYPE));
			memcpy(extension,ex_instance.extension,3*2*sizeof(int));
		}
		else {
			inds = (int *) 0; 
			vals = (BASETYPE *) 0;
		}
	}

	~Object_3D ( ) {
		if ( inds )
			delete inds;
		if ( vals )
			delete vals;
	}

	int getEXTENSION ( int ind, int direction ) {
		return extension[ind][direction];
	}
};

class Drawable_object_3D : public Object_3D {
	int *linds;
	int stridej, stridek;

public:
	Drawable_object_3D ( ) { linds = (int *) 0; stridej=0; stridek=0;}
	Drawable_object_3D ( const Object_3D& ex_instance ) : Object_3D(ex_instance) { linds = (int *) 0; stridej=0; stridek=0;}

	~Drawable_object_3D ( ) {
		if ( linds )
			delete linds;
	}

	void linearize ( int _stridej, int _stridek );

	void draw ( BASETYPE *im, int _dimi, int _dimj, int _dimk, int _i, int _j, int _k );
};


class Sphere_object_3D : public Object_3D {
public:
	Sphere_object_3D ( int radius );
};


class GaussianBead_object_3D : public Object_3D {
public:
	GaussianBead_object_3D ( int sigma );
};

class RayedSphere_object_3D : public Object_3D {
public:
	RayedSphere_object_3D ( int radius, int n_rays, int ray_length, int ray_radius );
};

# endif
