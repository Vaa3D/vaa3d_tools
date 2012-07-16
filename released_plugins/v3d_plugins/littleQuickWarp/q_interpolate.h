//q_interpolate.h
//
// by Lei Qu
//2012-07-08

#ifndef __Q_INTERPOLATE_H__
#define __Q_INTERPOLATE_H__

#include "../../basic_c_fun/v3d_basicdatatype.h"
#include "../../basic_c_fun/basic_surf_objs.h"

Vol3DSimple <MYFLOAT_JBA> * linearinterp_regularmesh_3d(V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, Vol3DSimple <MYFLOAT_JBA> * df_regular_grid);

bool interpolate_coord_linear(MYFLOAT_JBA * interpolatedVal, Coord3D_JBA *c, V3DLONG numCoord,
                       MYFLOAT_JBA *** templateVol3d, V3DLONG tsz0, V3DLONG tsz1, V3DLONG tsz2,
  		               V3DLONG tlow0, V3DLONG tup0, V3DLONG tlow1, V3DLONG tup1, V3DLONG tlow2, V3DLONG tup2);

#endif

