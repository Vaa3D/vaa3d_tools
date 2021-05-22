#include "until.h"
#include "q_dfblcokinterp.h"
//linear interpolate the SubDFBlock to DFBlock
//use 3d or 4d pointer instead of 1d, since generating 3d or 4d pointer from 1d is time consuming
bool q_dfblcokinterp_linear(DisplaceFieldF3D ***&pppSubDF,
	const V3DLONG szBlock_x, const V3DLONG szBlock_y, const V3DLONG szBlock_z,
	const V3DLONG substart_x, const V3DLONG substart_y, const V3DLONG substart_z,
	DisplaceFieldF3D ***&pppDFBlock)
{
	Vol3DSimple <MYFLOAT_JBA> 		*pDFBlock1C = 0;
	MYFLOAT_JBA			    	***pppDFBlock1C = 0;
	Vol3DSimple<MYFLOAT_JBA> 		*pSubDFBlock1c = new Vol3DSimple<MYFLOAT_JBA>(2, 2, 2);
	MYFLOAT_JBA 		     	***pppSubDFBlock1c = pSubDFBlock1c->getData3dHandle();

	V3DLONG i, j, k, is, js, ks;
	//x
	for (k = 0; k < 2; k++)
		for (j = 0; j < 2; j++)
			for (i = 0; i < 2; i++)
			{
				ks = k + substart_z;
				js = j + substart_y;
				is = i + substart_x;
				pppSubDFBlock1c[k][j][i] = pppSubDF[ks][js][is].sx;
			}
	pDFBlock1C = linearinterp_regularmesh_3d(szBlock_x, szBlock_y, szBlock_z, pSubDFBlock1c);
	pppDFBlock1C = pDFBlock1C->getData3dHandle();
	for (k = 0; k < szBlock_z; k++) for (j = 0; j < szBlock_y; j++) for (i = 0; i < szBlock_x; i++) pppDFBlock[k][j][i].sx = pppDFBlock1C[k][j][i];
	if (pDFBlock1C) { delete pDFBlock1C; pDFBlock1C = 0; }
	//y
	for (k = 0; k < 2; k++)
		for (j = 0; j < 2; j++)
			for (i = 0; i < 2; i++)
			{
				ks = k + substart_z;
				js = j + substart_y;
				is = i + substart_x;
				pppSubDFBlock1c[k][j][i] = pppSubDF[ks][js][is].sy;
			}
	pDFBlock1C = linearinterp_regularmesh_3d(szBlock_x, szBlock_y, szBlock_z, pSubDFBlock1c);
	pppDFBlock1C = pDFBlock1C->getData3dHandle();
	for (k = 0; k < szBlock_z; k++) for (j = 0; j < szBlock_y; j++) for (i = 0; i < szBlock_x; i++) pppDFBlock[k][j][i].sy = pppDFBlock1C[k][j][i];
	if (pDFBlock1C) { delete pDFBlock1C; pDFBlock1C = 0; }
	//z
	for (k = 0; k < 2; k++)
		for (j = 0; j < 2; j++)
			for (i = 0; i < 2; i++)
			{
				ks = k + substart_z;
				js = j + substart_y;
				is = i + substart_x;
				pppSubDFBlock1c[k][j][i] = pppSubDF[ks][js][is].sz;
			}
	pDFBlock1C = linearinterp_regularmesh_3d(szBlock_x, szBlock_y, szBlock_z, pSubDFBlock1c);
	pppDFBlock1C = pDFBlock1C->getData3dHandle();
	for (k = 0; k < szBlock_z; k++) for (j = 0; j < szBlock_y; j++) for (i = 0; i < szBlock_x; i++) pppDFBlock[k][j][i].sz = pppDFBlock1C[k][j][i];
	if (pDFBlock1C) { delete pDFBlock1C; pDFBlock1C = 0; }

	if (pSubDFBlock1c)		{ delete pSubDFBlock1c;		pSubDFBlock1c = 0; }

	return true;
}

//bspline interpolate the DF block
//use 3d or 4d pointer instead of 1d, since generating 3d or 4d pointer from 1d is time consuming
bool q_dfblcokinterp_bspline(DisplaceFieldF3D ***&pppSubDF, const Matrix &x_bsplinebasis,
	const V3DLONG sz_gridwnd, const V3DLONG substart_x, const V3DLONG substart_y, const V3DLONG substart_z,
	DisplaceFieldF3D ***&pppDFBlock)
{
	//vectorize the gridblock's nodes position that use for interpolation
	Matrix x1D_gridblock(4 * 4 * 4, 3);
	long ind = 1;
	for (long dep = substart_z; dep < substart_z + 4; dep++)
		for (long col = substart_x; col < substart_x + 4; col++)
			for (long row = substart_y; row < substart_y + 4; row++)
			{
				x1D_gridblock(ind, 1) = pppSubDF[dep][row][col].sx;
				x1D_gridblock(ind, 2) = pppSubDF[dep][row][col].sy;
				x1D_gridblock(ind, 3) = pppSubDF[dep][row][col].sz;
				ind++;
			}
	//cubic B-spline interpolate the vectorized grid block
	Matrix x1D_gridblock_int = x_bsplinebasis*x1D_gridblock;
	//de-vectorize the interpolated grid block and save back to vec4D_grid_int
	ind = 1;
	for (long zz = 0; zz < sz_gridwnd; zz++)
		for (long xx = 0; xx < sz_gridwnd; xx++)
			for (long yy = 0; yy < sz_gridwnd; yy++)
			{
				pppDFBlock[zz][yy][xx].sx = x1D_gridblock_int(ind, 1);
				pppDFBlock[zz][yy][xx].sy = x1D_gridblock_int(ind, 2);
				pppDFBlock[zz][yy][xx].sz = x1D_gridblock_int(ind, 3);
				ind++;
			}

	return true;
}

