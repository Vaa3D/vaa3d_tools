
#ifndef __q_warp_h__
#define __q_warp_h__


//#include "until.h"

using namespace std;

#define WANT_STREAM
//linear interpolate the SubDFBlock to DFBlock
//use 3d or 4d pointer instead of 1d, since generating 3d or 4d pointer from 1d is time consuming
bool q_dfblcokinterp_linear(DisplaceFieldF3D ***&pppSubDF,
	const V3DLONG szBlock_x, const V3DLONG szBlock_y, const V3DLONG szBlock_z,
	const V3DLONG substart_x, const V3DLONG substart_y, const V3DLONG substart_z,
	DisplaceFieldF3D ***&pppDFBlock);

//bspline interpolate the DF block
//use 3d or 4d pointer instead of 1d, since generating 3d or 4d pointer from 1d is time consuming
bool q_dfblcokinterp_bspline(DisplaceFieldF3D ***&pppSubDF, const Matrix &x_bsplinebasis,
	const V3DLONG sz_gridwnd, const V3DLONG substart_x, const V3DLONG substart_y, const V3DLONG substart_z,
	DisplaceFieldF3D ***&pppDFBlock);

#endif
