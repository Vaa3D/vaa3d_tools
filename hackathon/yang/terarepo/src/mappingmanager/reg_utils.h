/*
 * reg_utils.h
 *
 * created August 29, 2012 by Giulio Iannello
 */

# ifndef REG_UTILS_H
# define REG_UTILS_H

# include "spaceObjects.h"

# define myPI   (3.141592653589793)

// minimum extension along one axis because a volume is not considered empty
# define MIN_VOLUME_DIMENSION   25

enum View {V000=0, V090=1, V180=2, V270=3};

SpaceSize getMOVE_INFO ( Point fromPoint, Point toPoint );
/* returns the components of the oriented segment connecting fromPoint to toPoint */

void reorder_VERTICES ( Point *p0, Point *p1, SpaceSize vxlsz );
/* given two opposite vertices p0 and p1 defining a rectangular subvolume returns in 
 * p0 and p1 the opposite vertices defining the same rectangular subvolume that in each 
 * dimension k (k=x,y,z) satisfies the propoerty ((p1.k - p0.k) * vxlsz.k) >= 0 
 */

bool intersect_VOLUMES ( Point p00, Point p01, Point p10, Point p11, SpaceSize vxlsz, Point *p20, Point *p21 );
/* computes the intersection of two rectangular subvolumes 
 * p00, p01: opposite vertices defining the first rectangular subvolume
 * p10, p11: opposite vertices defining the second rectangular subvolume
 * vxlsz: voxel size (signed versors) 
 * p20, p21: pointers to the opposit vertices defining the intersection subvolume (defined only if true is returned)
 * if the intersection is empty, p20 and p21 are set to (0,0,0) and the function returns false
 */

bool isempty_VOLUME ( Point p0, Point p1, SpaceSize vxlsz );

# endif
