//q_interpolate.h
//
// by Lei Qu
//2012-07-08

#include "jba_match_landmarks.h"
#include "q_interpolate.h"


Vol3DSimple <MYFLOAT_JBA> * linearinterp_regularmesh_3d(V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, Vol3DSimple <MYFLOAT_JBA> * df_regular_grid)
{
	V3DLONG k, j, i;

	if (!df_regular_grid || !df_regular_grid->valid())
	{
		fprintf(stderr, "The pointer is not correct.\n");
		return 0;
	}
	MYFLOAT_JBA *** df_grid3d = df_regular_grid->getData3dHandle();
//	V3DLONG n0 = df_regular_grid->sz0()-3, n1 = df_regular_grid->sz1()-3, n2 = df_regular_grid->sz2()-3;//-3 for B-spline?
	V3DLONG n0 = df_regular_grid->sz0()-1, n1 = df_regular_grid->sz1()-1, n2 = df_regular_grid->sz2()-1;//modified by qul @ 120710
	if (n0<=0 || n1<=0 || n2<=0)
	{
		fprintf(stderr, "The size  is not correct.\n");
		return 0;
	}
	if (sz0<=0 || sz1<=0 || sz2<=0)
	{
		fprintf(stderr, "The size of the DF to be computed is not correct.\n");
		return 0;
	}

	Vol3DSimple <MYFLOAT_JBA> * df_field  = new Vol3DSimple <MYFLOAT_JBA>(sz0, sz1, sz2);
	if (!df_field ||!df_field->valid())
	{
		fprintf(stderr, "Fail to allocate memory.\n");
		if (df_field) {delete df_field; df_field=0;}
		return 0;
	}

	MYFLOAT_JBA * df_field_ref1d = df_field->getData1dHandle();
	for (i=0;i<df_field->getTotalElementNumber(); i++)
	{
		df_field_ref1d[i] = 0;
	}

	Coord3D_JBA *c = new Coord3D_JBA [df_field->getTotalElementNumber()];
	double nf0=(double)n0/sz0, nf1=(double)n1/sz1, nf2=(double)n2/sz2;
	V3DLONG cnt=0;
	for (k=0;k<sz2;k++)
	{
		double k_tmp=(double)k*nf2;
		for (j=0;j<sz1;j++)
		{
			double j_tmp=(double)j*nf1;
			for (i=0;i<sz0;i++)
			{
				c[cnt].x = i*nf0;
				c[cnt].y = j_tmp;
				c[cnt].z = k_tmp;
				cnt++;
			}
		}
	}

	interpolate_coord_linear(df_field_ref1d, c, df_field->getTotalElementNumber(),
			df_grid3d, df_regular_grid->sz0(), df_regular_grid->sz1(), df_regular_grid->sz2(),
			0, n0-1+1, 0, n1-1+1, 0, n2-1+1);

	if (c) {delete []c; c=0;}
	return df_field;
}

bool interpolate_coord_linear(MYFLOAT_JBA * interpolatedVal, Coord3D_JBA *c, V3DLONG numCoord,
					   MYFLOAT_JBA *** templateVol3d, V3DLONG tsz0, V3DLONG tsz1, V3DLONG tsz2,
					   V3DLONG tlow0, V3DLONG tup0, V3DLONG tlow1, V3DLONG tup1, V3DLONG tlow2, V3DLONG tup2)
{
	if (!interpolatedVal || !c || numCoord<=0 ||
		!templateVol3d || tsz0<=0 || tsz1<=0 || tsz2<=0 ||
		tlow0<0 || tlow0>=tsz0 || tup0<0 || tup0>=tsz0 || tlow0>tup0 ||
		tlow1<0 || tlow1>=tsz1 || tup1<0 || tup1>=tsz1 || tlow1>tup1 ||
		tlow2<0 || tlow2>=tsz2 || tup2<0 || tup2>=tsz2 || tlow2>tup2)
	{
		fprintf(stderr, "Invalid parameters! [%s][%d]\n", __FILE__, __LINE__);
		return false;
	}

	double curpx, curpy, curpz;
	V3DLONG cpx0, cpx1, cpy0, cpy1, cpz0, cpz1;

	for (V3DLONG ipt=0;ipt<numCoord;ipt++)
	{
		if (c[ipt].x< tlow0 || c[ipt].x> tup0 || c[ipt].y< tlow1 || c[ipt].y> tup1 || c[ipt].z< tlow2 || c[ipt].z> tup2)
		{
			interpolatedVal[ipt] = 0;
			continue;
		}

		curpx = c[ipt].x; curpx=(curpx<tlow0)?tlow0:curpx; curpx=(curpx>tup0)?tup0:curpx;
#ifndef POSITIVE_Y_COORDINATE
		curpy = tsz1-1-c[ipt].y; curpy=(curpy<tlow1)?tlow1:curpy; curpy=(curpy>tup1)?tup1:curpy;
#else
		curpy = c[ipt].y; curpy=(curpy<tlow1)?tlow1:curpy; curpy=(curpy>tup1)?tup1:curpy;
#endif
		curpz = c[ipt].z; curpz=(curpz<tlow2)?tlow2:curpz; curpz=(curpz>tup2)?tup2:curpz;

		cpx0 = V3DLONG(floor(curpx)); cpx1 = V3DLONG(ceil(curpx));
		cpy0 = V3DLONG(floor(curpy)); cpy1 = V3DLONG(ceil(curpy));
		cpz0 = V3DLONG(floor(curpz)); cpz1 = V3DLONG(ceil(curpz));

		if (cpz0==cpz1)
		{
			if (cpy0==cpy1)
			{
				if (cpx0==cpx1)
				{
					interpolatedVal[ipt] = (MYFLOAT_JBA)(templateVol3d[cpz0][cpy0][cpx0]);
				}
				else
				{
					double w0x0y0z = (cpx1-curpx);
					double w1x0y0z = (curpx-cpx0);
					interpolatedVal[ipt] = (MYFLOAT_JBA)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) +
													 w1x0y0z * double(templateVol3d[cpz0][cpy0][cpx1]));
				}
			}
			else
			{
				if (cpx0==cpx1)
				{
					double w0x0y0z = (cpy1-curpy);
					double w0x1y0z = (curpy-cpy0);
					interpolatedVal[ipt] = (MYFLOAT_JBA)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) +
													 w0x1y0z * double(templateVol3d[cpz0][cpy1][cpx0]));
				}
				else
				{
					double w0x0y0z = (cpx1-curpx)*(cpy1-curpy);
					double w0x1y0z = (cpx1-curpx)*(curpy-cpy0);
					double w1x0y0z = (curpx-cpx0)*(cpy1-curpy);
					double w1x1y0z = (curpx-cpx0)*(curpy-cpy0);
					interpolatedVal[ipt] = (MYFLOAT_JBA)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) +
													 w0x1y0z * double(templateVol3d[cpz0][cpy1][cpx0]) +
													 w1x0y0z * double(templateVol3d[cpz0][cpy0][cpx1]) +
													 w1x1y0z * double(templateVol3d[cpz0][cpy1][cpx1]));
				}
			}
		}
		else
		{
			if (cpy0==cpy1)
			{
				if (cpx0==cpx1)
				{
					double w0x0y0z = (cpz1-curpz);
					double w0x0y1z = (curpz-cpz0);

					interpolatedVal[ipt] = (MYFLOAT_JBA)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) + w0x0y1z * double(templateVol3d[cpz1][cpy0][cpx0]));
				}
				else
				{
					double w0x0y0z = (cpx1-curpx)*(cpz1-curpz);
					double w0x0y1z = (cpx1-curpx)*(curpz-cpz0);

					double w1x0y0z = (curpx-cpx0)*(cpz1-curpz);
					double w1x0y1z = (curpx-cpx0)*(curpz-cpz0);

					interpolatedVal[ipt] = (MYFLOAT_JBA)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) + w0x0y1z * double(templateVol3d[cpz1][cpy0][cpx0]) +
													 w1x0y0z * double(templateVol3d[cpz0][cpy0][cpx1]) + w1x0y1z * double(templateVol3d[cpz1][cpy0][cpx1]));
				}
			}
			else
			{
				if (cpx0==cpx1)
				{
					double w0x0y0z = (cpy1-curpy)*(cpz1-curpz);
					double w0x0y1z = (cpy1-curpy)*(curpz-cpz0);

					double w0x1y0z = (curpy-cpy0)*(cpz1-curpz);
					double w0x1y1z = (curpy-cpy0)*(curpz-cpz0);

					interpolatedVal[ipt] = (MYFLOAT_JBA)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) + w0x0y1z * double(templateVol3d[cpz1][cpy0][cpx0]) +
													 w0x1y0z * double(templateVol3d[cpz0][cpy1][cpx0]) + w0x1y1z * double(templateVol3d[cpz1][cpy1][cpx0]));
				}
				else
				{
					double w0x0y0z = (cpx1-curpx)*(cpy1-curpy)*(cpz1-curpz);
					double w0x0y1z = (cpx1-curpx)*(cpy1-curpy)*(curpz-cpz0);

					double w0x1y0z = (cpx1-curpx)*(curpy-cpy0)*(cpz1-curpz);
					double w0x1y1z = (cpx1-curpx)*(curpy-cpy0)*(curpz-cpz0);

					double w1x0y0z = (curpx-cpx0)*(cpy1-curpy)*(cpz1-curpz);
					double w1x0y1z = (curpx-cpx0)*(cpy1-curpy)*(curpz-cpz0);

					double w1x1y0z = (curpx-cpx0)*(curpy-cpy0)*(cpz1-curpz);
					double w1x1y1z = (curpx-cpx0)*(curpy-cpy0)*(curpz-cpz0);

					interpolatedVal[ipt] = (MYFLOAT_JBA)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) + w0x0y1z * double(templateVol3d[cpz1][cpy0][cpx0]) +
													 w0x1y0z * double(templateVol3d[cpz0][cpy1][cpx0]) + w0x1y1z * double(templateVol3d[cpz1][cpy1][cpx0]) +
													 w1x0y0z * double(templateVol3d[cpz0][cpy0][cpx1]) + w1x0y1z * double(templateVol3d[cpz1][cpy0][cpx1]) +
													 w1x1y0z * double(templateVol3d[cpz0][cpy1][cpx1]) + w1x1y1z * double(templateVol3d[cpz1][cpy1][cpx1]) );
				}
			}
		}

	}

	return true;
}
