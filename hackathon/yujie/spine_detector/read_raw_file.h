#ifndef READ_RAW_FILE_H
#define READ_RAW_FILE_H

#include "basic_surf_objs.h"
int loadRawRegion(char * filename, unsigned char * & img, V3DLONG * & sz,V3DLONG * & region_sz, int & datatype,
                  V3DLONG startx, V3DLONG starty, V3DLONG startz,
                  V3DLONG endx, V3DLONG endy, V3DLONG endz);

#endif // READ_RAW_FILE_H
