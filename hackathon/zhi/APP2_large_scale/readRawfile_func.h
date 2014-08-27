/* gsdt_func.h
 * Perform distance transformation on grayscale image.
 * 2012-03-02 : by Hang Xiao
 */
 
#ifndef __READRAWFILE_FUNC_H__
#define __READRAWFILE_FUNC_H__

#include "basic_surf_objs.h"


int loadRawRegion(char * filename, unsigned char * & img, V3DLONG * & sz,V3DLONG * & region_sz, int & datatype,
                  V3DLONG startx, V3DLONG starty, V3DLONG startz,
                  V3DLONG endx, V3DLONG endy, V3DLONG endz);


#endif

