// q_bspline.h
// by Lei Qu
// 2012-07-23

#ifndef __Q_BSPLINE_H__
#define __Q_BSPLINE_H__

#include "jba_mainfunc.h"

#include <vector>
#include <string>
using namespace std;

#define WANT_STREAM
#include "../../jba/newmat11/newmatap.h"
#include "../../jba/newmat11/newmatio.h"

Vol3DSimple<DisplaceFieldF3D> * compute_df_tps_subsampled_volume_4bspline(const vector <Coord3D_JBA> & matchTargetPos, const vector <Coord3D_JBA> & matchSubjectPos, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2,
																 V3DLONG gfactor_x, V3DLONG gfactor_y, V3DLONG gfactor_z);

bool q_nonrigid_ini_bsplinebasis_3D(const long n,Matrix &BxBxB);

#endif

