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
 * spaceObjects.h
 *
 * created August 10, 2012 by Giulio Iannello
 * changed April 29, 2016 by Pierangelo Afferni
 */

# ifndef SPACE_OBJECTS_H
# define SPACE_OBJECTS_H

# include "lib3Dobjects_defs.h"
//# include "utils.h"

class SpaceObj {

	COORDSTYPE vals[3];  // Define SpaceObj assuming the coordinates system (0,1,2) or (V,H,D) be corresponding to (y,x,z)

public:
	SpaceObj ( ) { vals[0] = vals[1] = vals[2] = 0.0; }
	SpaceObj ( COORDSTYPE y, COORDSTYPE x, COORDSTYPE z ) {
		vals[0] = y;
		vals[1] = x;
		vals[2] = z; 
	}
	SpaceObj ( const SpaceObj& ex_instance ) {
		vals[0] = ex_instance.vals[0];
		vals[1] = ex_instance.vals[1];
		vals[2] = ex_instance.vals[2]; 
	}
	SpaceObj& operator= ( const SpaceObj& ex_instance ) {
		vals[0] = ex_instance.vals[0];
		vals[1] = ex_instance.vals[1];
		vals[2] = ex_instance.vals[2]; 
		return *this;
	}
	bool operator== ( const SpaceObj& ex_instance ) {
		return 
			( vals[0] == ex_instance.vals[0] &&
			  vals[1] == ex_instance.vals[1] &&
			  vals[2] == ex_instance.vals[2]   ); 
	}
	COORDSTYPE y ( ) { return vals[0]; }
	COORDSTYPE x ( ) { return vals[1]; }
	COORDSTYPE z ( ) { return vals[2]; }
	COORDSTYPE val ( int ind ) { return vals[ind]; }
	void sety ( COORDSTYPE val ) { vals[0] = val; }
	void setx ( COORDSTYPE val ) { vals[1] = val; }
	void setz ( COORDSTYPE val ) { vals[2] = val; }
	void setval ( int ind, COORDSTYPE val ) { vals[ind] = val; }
	void setvals ( COORDSTYPE y, COORDSTYPE x, COORDSTYPE z ) {
		vals[0] = y;
		vals[1] = x;
		vals[2] = z; 
	}
};

class SpaceSize : public SpaceObj {
public:
	SpaceSize () : SpaceObj() { }
	SpaceSize ( COORDSTYPE y, COORDSTYPE x, COORDSTYPE z ) : SpaceObj(y,x,z) { }
};

class Point : public SpaceObj {
public:
	Point () : SpaceObj() { }
	Point ( COORDSTYPE y, COORDSTYPE x, COORDSTYPE z ) : SpaceObj(y,x,z) { }

	Point move ( SpaceSize move_info ) {
		Point temp = *this;
		temp.setx(x() + move_info.x());
		temp.sety(y() + move_info.y());
		temp.setz(z() + move_info.z());
		return temp;
	}
};

# endif
