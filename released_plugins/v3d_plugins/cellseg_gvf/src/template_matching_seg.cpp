//template_match_seg.cpp
//by Hanchuan Peng
//090426

#include <math.h>
#include <stdio.h>

#include "template_matching_seg.h"
#include "../basic_c_fun/volimg_proc.h"
#include "../basic_c_fun/basic_landmark.h"


bool template_matching_seg(Vol3DSimple <unsigned char> *img3d, Vol3DSimple <unsigned short int> *outimg3d, const para_template_matching_cellseg & mypara)
{
	if (!img3d || !img3d->valid() || !outimg3d || !outimg3d->valid() || !isSameSize(img3d, outimg3d))
	{
		printf("The input sizes are different in template_matching_seg()\n");
		return false;
	}

	V3DLONG rx=mypara.szx,ry=mypara.szy,rz=mypara.szz;
	V3DLONG wx=2*rx+1, wy=2*ry+1, wz=2*rz+1;
	double sigmax=mypara.stdx, sigmay=mypara.stdy, sigmaz=mypara.stdz;

	//create the kernel & flag-img
	Vol3DSimple<double> * g = genGaussianKernal3D(wx, wy, wz, sigmax, sigmay, sigmaz);
	if (!g) return false;
	Vol3DSimple<unsigned char> * d = new Vol3DSimple<unsigned char> (wx, wy, wz);
	if (!d) return false;
	V3DLONG kernel_len = g->getTotalElementNumber();

	Vol3DSimple<unsigned char> * flagimg = new Vol3DSimple<unsigned char> (img3d);
	if (!flagimg) return false;

	//do computation
	double * g_1d = g->getData1dHandle();
	double *** g_3d = g->getData3dHandle();
	unsigned char * d_1d = d->getData1dHandle();
	unsigned char *** d_3d = d->getData3dHandle();

	V3DLONG i,j,k;
	unsigned char *** img3d_p3d = img3d->getData3dHandle();
	unsigned char *** flag_p3d = flagimg->getData3dHandle();
	unsigned short int ***outimg3d_p3d = outimg3d->getData3dHandle();
	V3DLONG cellcnt=0;

	V3DLONG sx=img3d->sz0(),sy=img3d->sz1(),sz=img3d->sz2();
	V3DLONG len = img3d->getTotalElementNumber();

	//set the flag and output img to be 0
	for (k=0;k<sz;k++)
		for (j=0;j<sy;j++)
			for (i=0;i<sx;i++)
			{
				flag_p3d[k][j][i] = 0;
				outimg3d_p3d[k][j][i] = 0;
			}


	//compute
	vector <LocationSimple> detectedPos;

	for (k=0;k<sz;k++)
	{
		if (k<rz || k>(sz-1-rz)) //if at the border, then skip
			continue;

		for (j=0;j<sy;j++)
		{
			if (j<ry || j>(sy-1-ry)) //if at the border, then skip
				continue;

			for (i=0;i<sx;i++)
			{
				if (i<rx || i>(sx-1-rx)) //if at the border, then skip
					continue;

				if (flag_p3d[k][j][i]) //if the location has been masked, then skip
					continue;

				if (img3d_p3d[k][j][i]<=mypara.t_pixval) //do not compute dark pixels
				{
					flag_p3d[k][j][i]=1;
					continue;
				}

				bool b_skip=false;
				//copy data
				V3DLONG i1,j1,k1, i2,j2,k2;
				for (k1=k-rz,k2=0;k1<=k+rz;k1++,k2++)
					for (j1=j-ry,j2=0;j1<=j+ry;j1++,j2++)
						for (i1=i-rx,i2=0;i1<=i+rx;i1++,i2++)
						{
							d_3d[k2][j2][i2] = img3d_p3d[k1][j1][i1];
							if (outimg3d_p3d[k1][j1][i1])
								b_skip=true;
						}

				if (b_skip==true)
					continue;

				//test regional mean
				unsigned char d_mean, d_std;
				mean_and_std(d_1d, kernel_len, d_mean, d_std);
				if (d_mean<mypara.t_rgnval)
				{
					flag_p3d[k][j][i]=1;
					continue;
				}

				//compute correlation
				double score = compute_corrcoef_two_vectors(d_1d, g_1d, kernel_len);
				flag_p3d[k][j][i] = 1; //do not search later
				if (score>=mypara.t_corrcoef)
				{
					//first re-estimate the center
					double ncx=i,ncy=j,ncz=k; //new center position
					double ocx,ocy,ocz; //old center position
					double scx=0,scy=0,scz=0,si=0;
					while (1) //mean shift to estimate the true center
					{
						ocx=ncx; ocy=ncy; ocz=ncz;
						for (k1=ocz-rz;k1<=ocz+rz;k1++)
						{
							if (k1<0 || k1>=sz)
								continue;
							for (j1=ocy-ry;j1<=ocy+ry;j1++)
							{
								if (j1<0 || j1>=sy)
									continue;
								for (i1=ocx-rx;i1<=ocx+rx;i1++)
								{
									if (i1<0 || i1>=sx)
										continue;
									double cv = img3d_p3d[k1][j1][i1];
									scz += k1*cv;
									scy += j1*cv;
									scx += i1*cv;
									si += cv;
								}
							}
						}
						if (si>0)
							{ncx = scx/si; ncy = scy/si; ncz = scz/si;}
						else
							{ncx = ocx; ncy = ocy; ncz = ocz;}

						if (ncx<rx || ncx>=sx-1-rx || ncy<ry || ncy>=sy-1-ry || ncz<rz || ncz>=sz-1-rz) //move out of boundary
						{
							ncx = ocx; ncy = ocy; ncz = ocz; //use the last valid center
							break;
						}

						//printf("%5.3f %5.3f %5.3f\n", ncx,ncy,ncz);
						if (sqrt((ncx-ocx)*(ncx-ocx)+(ncy-ocy)*(ncy-ocy)+(ncz-ocz)*(ncz-ocz))<=1)
							break;
					}
					//printf("\n\n", ocx,ocy,ocz);
					double lncx=ncx,lncy=ncy,lncz=ncz;

					//then estimate the new radius
					scz=0; scy=0;scx=0;si=0;
					for (k1=lncz-rz;k1<=lncz+rz;k1++)
					{
						for (j1=lncy-ry;j1<=lncy+ry;j1++)
						{
							for (i1=lncx-rx;i1<=lncx+rx;i1++)
							{
								double cv = img3d_p3d[k1][j1][i1];
								scz += cv*(k1-lncz)*(k1-lncz);
								scy += cv*(j1-lncy)*(j1-lncy);
								scx += cv*(i1-lncx)*(i1-lncx);
								si += cv;
							}
						}
					}
					double stdx, stdy, stdz;
					if (si>0)
					{
						stdx = sqrt(scx/si); stdy = sqrt(scy/si); stdz = sqrt(scz/si);
						printf("cell=%d, %5.3f (%5.3f) %5.3f (%5.3f) %5.3f (%5.3f)\n", cellcnt+1, lncx, stdx, lncy, stdy, lncz, stdz);
					}
					else
					{
						printf("Error happens in estimating the standard deviation in template_matching_seg(). Force setting std to be 1.\n");
						stdx = 1; stdy = 1; stdz = 1;
					}


					//mask
					bool b_merge=false; int celllabel;
					for (int pii=0;pii<detectedPos.size();pii++)
					{
						register float tmpx = detectedPos.at(pii).x-lncx;
						register float tmpy = detectedPos.at(pii).y-lncy;
						register float tmpz = detectedPos.at(pii).z-lncz;
						if (tmpx*tmpx+tmpy*tmpy+tmpz*tmpz<mypara.merge_radius*mypara.merge_radius) //400=20*20
						{
							b_merge=true;
							celllabel = pii+1; //index start from 1
							break;
						}
					}

					if (b_merge==false)
					{
						LocationSimple pp(lncx, lncy, lncz);
						detectedPos.push_back(pp);
						cellcnt++;
						celllabel = cellcnt;
					}


					double sa2b2c2 = 8*stdx*stdx*stdy*stdy*stdz*stdz;
					double sa2b2 = 4*stdx*stdx*stdy*stdy;
					double sb2c2 = 4*stdy*stdy*stdz*stdz;
					double sa2c2 = 4*stdx*stdx*stdz*stdz;

					for (k1=lncz-2*stdz,k2=0;k1<=lncz+2*stdz;k1++,k2++)
						for (j1=lncy-2*stdy,j2=0;j1<=lncy+2*stdy;j1++,j2++)
							for (i1=lncx-2*stdx,i2=0;i1<=lncx+2*stdx;i1++,i2++)
							{
//								if (g_3d[k2][j2][i2]>0.2)
								if (k1>=0 && k1<sz && j1>=0 && j1<sy && i1>=0 && i1<sx)
								{
									if ((k1-lncz)*(k1-lncz)*sa2b2+(j1-lncy)*(j1-lncy)*sa2c2+(i1-lncx)*(i1-lncx)*sb2c2 <= sa2b2c2)
									{
										flag_p3d[k1][j1][i1] = 1;
										outimg3d_p3d[k1][j1][i1] = cellcnt;
									}
								}
							}


				}
			}
		}
	}

	printf("total cell cnt=%d\n", cellcnt);

	//free space
	if (flagimg) {delete flagimg; flagimg=0;}
	if (d) {delete d; d=0;}
	if (g) {delete g; g=0;}
	return true;
}

Vol3DSimple<double> * genGaussianKernal3D(V3DLONG szx, V3DLONG szy, V3DLONG szz, double sigmax, double sigmay, double sigmaz)
{
	if (szx<=0 || szy<=0 || szz<=0) {printf("Invalid sz parameter in genGaussianKernal3D().\n"); return 0;}

	Vol3DSimple<double> * g = 0;
	try
	{
		g = new Vol3DSimple<double> (szx, szy, szz);
	}
	catch (...)
	{
		printf("Fail to create a kernel object.\n");
		return 0;
	}

	double *** d3 = g->getData3dHandle();
	double sx2 = 2.0*sigmax*sigmax, sy2=2.0*sigmay*sigmay, sz2=2.0*sigmaz*sigmaz;
	V3DLONG cx=(szx-1)>>1, cy=(szy-1)>>1, cz=(szz-1)>>1;
	V3DLONG i,j,k;
	for (k=0;k<=cz;k++)
		for (j=0;j<=cy;j++)
			for (i=0;i<=cx;i++)
			{
				d3[szz-1-k][szy-1-j][szx-1-i] = d3[szz-1-k][szy-1-j][i] =
				d3[szz-1-k][j][szx-1-i] = d3[szz-1-k][j][i] =
				d3[k][szy-1-j][szx-1-i] = d3[k][szy-1-j][i] =
				d3[k][j][szx-1-i] = d3[k][j][i] =
					exp(-double(i-cx)*(i-cx)/sx2-double(j-cy)*(j-cy)/sy2-double(k-cz)*(k-cz)/sz2);
			}
	return g;
}

template <class T1, class T2> double compute_corrcoef_two_vectors(T1 *v1, T2 *v2, V3DLONG len)
{
	if (!v1 || !v2 || len<=1) return 0;

	//first compute mean
	double m1=0,m2=0;
	V3DLONG i;
	for (i=0;i<len;i++)
	{
		m1+=v1[i];
		m2+=v2[i];
	}
	m1/=len;
	m2/=len;

	//now compute corrcoef
	double tmp_s=0, tmp_s1=0, tmp_s2=0, tmp1, tmp2;
	for (i=0;i<len;i++)
	{
		tmp1 = v1[i]-m1;
		tmp2 = v2[i]-m2;
		tmp_s += tmp1*tmp2;
		tmp_s1 += tmp1*tmp1;
		tmp_s2 += tmp2*tmp2;
	}

	//the final score
	double s;
	s = (tmp_s / sqrt(tmp_s1) / sqrt(tmp_s2) + 1 )/2;
	return s;
}

template <class T1, class T2> double compute_normalized_L1dist_two_vectors(T1 *v1, T2 *v2, V3DLONG len, bool b_remove_mean)
{
	if (!v1 || !v2 || len<=0) return 0;
	double m1=0,m2=0;
	V3DLONG i;

	//first compute mean
	if (b_remove_mean)
	{
		for (i=0;i<len;i++)
		{
			m1+=v1[i];
			m2+=v2[i];
		}
		m1/=len;
		m2/=len;
	}

	//now compute dist
	double tmp_s=0, tmp1, mdiff=m2-m1;
	if (b_remove_mean)
	{
		for (i=0;i<len;i++)
		{
			tmp1 = (v1[i]-v2[i] + mdiff);
			if (tmp1<0) tmp1 = -tmp1;
			tmp_s += tmp1;
		}
	}
	else
	{
		for (i=0;i<len;i++)
		{
			tmp1 = fabs(v1[i]-v2[i]);
			if (tmp1<0) tmp1 = -tmp1;
			tmp_s += tmp1;
		}
	}

	//the final score
	double s;
	s = tmp_s/len;
	return s;
}

template <class T1, class T2> double compute_normalized_L2dist_two_vectors(T1 *v1, T2 *v2, V3DLONG len, bool b_remove_mean)
{
	if (!v1 || !v2 || len<=0) return 0;
	double m1=0,m2=0;
	V3DLONG i;

	//first compute mean
	if (b_remove_mean)
	{
		for (i=0;i<len;i++)
		{
			m1+=v1[i];
			m2+=v2[i];
		}
		m1/=len;
		m2/=len;
	}

	//now compute dist
	double tmp_s=0, tmp1, tmp2, mdiff=m2-m1;
	if (b_remove_mean)
	{
		for (i=0;i<len;i++)
		{
			tmp1 = (v1[i]-m1);
			tmp2 = (v2[i]-m2);
			tmp_s += tmp1*tmp1+tmp2*tmp2;
		}
	}
	else
	{
		for (i=0;i<len;i++)
		{
			tmp1 = (v1[i]);
			tmp2 = (v2[i]);
			tmp_s += tmp1*tmp1+tmp2*tmp2;
		}
	}

	//the final score
	double s;
	s = sqrt(tmp_s)/len;
	return s;
}

