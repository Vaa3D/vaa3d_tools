#include <cassert>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

#include "stackutil.h"
using namespace std;

#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif

static void getImageSize(string filename, V3DLONG &sz0, V3DLONG &sz1, V3DLONG &sz2)
{
	FILE * fid = fopen((char*) filename.c_str(), "rb");
	if(!fid) return;

	int lenkey = strlen("raw_image_stack_by_hpeng");
	fseek(fid, lenkey + 1 + 2, SEEK_SET);
	int mysz[4]; mysz[0] = mysz[1] = mysz[2] = mysz[3] = 0;
	int n = fread(mysz, 4, 4, fid);
	if(n!= 4) return;

	sz0 = mysz[0];
	sz1 = mysz[1];
	sz2 = mysz[2];
	fclose(fid);
}
bool createMapViewFiles(string prefix, V3DLONG ts0, V3DLONG ts1, V3DLONG ts2)
{
	assert(ts0 == pow(2, log(ts0)/log(2.0)));
	assert(ts1 == pow(2, log(ts1)/log(2.0)));
	assert(ts2 == pow(2, log(ts2)/log(2.0)));
	// check block size
	V3DLONG bs0 = 0, bs1 = 0, bs2 = 0;
	for(V3DLONG tk = 0; tk < ts2; tk++)
	{
		for(V3DLONG tj = 0; tj < ts1; tj++)
		{
			for(V3DLONG ti = 0; ti < ts0; ti++)
			{
				ostringstream oss;
				oss<<prefix<<"_"<<0<<"_"<<ti<<"_"<<tj<<"_"<<tk<<".raw";
				string filename = oss.str();
				V3DLONG sz0, sz1, sz2;
				getImageSize(filename, sz0, sz1, sz2);
				if(bs0 == 0 && bs1 ==0 && bs2 == 0)
				{
					bs0 = sz0; bs1 = sz1; bs2 = sz2;
				}
				else
				{
					if(bs0 != sz0 || bs1 != sz1 || bs2 != sz2) return false;
				}
			}
		}
	}
	//V3DLONG min_level = 0, max_level = 10;// log(MIN(MIN(ts0, ts1), ts2))/log(2.0);
	V3DLONG pts0 = ts0, pts1 = ts1, pts2 = ts2;
	V3DLONG pbs0 = bs0, pbs1 = bs1, pbs2 = bs2;
	V3DLONG cts0 = ts0, cts1 = ts1, cts2 = ts2;
	V3DLONG cbs0 = bs0, cbs1 = bs1, cbs2 = bs2;
	V3DLONG level = 0;
	while(true)
	{
		cout<<"================= level "<<level<<" ======================="<<endl;
		if((pts0 == 1 && pbs0 == 1) ||
		   (pts1 == 1 && pbs1 == 1) ||
		   (pts2 == 1 && pbs2 == 1)) break;
		(pts0 == 1) ? (cbs0 = MAX(pbs0/2, 1)) : (cts0 = pts0/2);
		(pts1 == 1) ? (cbs1 = MAX(pbs1/2, 1)) : (cts1 = pts1/2);
		(pts2 == 1) ? (cbs2 = MAX(pbs2/2, 1)) : (cts2 = pts2/2);

		V3DLONG pbs01 = pbs0 * pbs1;
		V3DLONG cbs01 = cbs0 * cbs1;

		for(V3DLONG tk = 0; tk < cts2; tk++)
		{
			for(V3DLONG tj = 0; tj < cts1; tj++)
			{
				for(V3DLONG ti = 0; ti < cts0; ti++)
				{
					V3DLONG gimax = (pts0 > 1) ? 1 : 0;
					V3DLONG gjmax = (pts1 > 1) ? 1 : 0;
					V3DLONG gkmax = (pts2 > 1) ? 1 : 0;

					V3DLONG tol_csz = cbs0 * cbs1 * cbs2;
					unsigned char * curimg1d = new unsigned char[tol_csz];
					// grid i, j k
					for(V3DLONG gk = 0; gk <= gkmax; gk++)
					{
						for(V3DLONG gj = 0; gj <= gjmax; gj++)
						{
							for(V3DLONG gi = 0; gi <= gimax; gi++)
							{
								ostringstream oss;
								oss<<prefix<<"_"<<level<<"_"<<2*ti + gi<<"_"<<2*tj + gj<<"_"<<2*tk + gk<<".raw";
								string parimg_file = oss.str();
								unsigned char * parimg1d = 0; V3DLONG * parsz = 0; int datatype;
								if(!loadImage((char *) parimg_file.c_str(), parimg1d, parsz, datatype)){cerr<<"load "<<parimg_file<<" error."<<endl; return false;}
								if(parsz[0] != pbs0 || parsz[1] != pbs1 || parsz[2] != pbs2)
								{
									cerr<<"Incrorect size for "<<parimg_file<<endl;
									cerr<<"It should be "<<pbs0<<"x"<<pbs1<<"x"<<pbs2<<" , the actual size is "<<parsz[0]<<"x"<<parsz[1]<<"x"<<parsz[2]<<endl;
									return false;
								}
								V3DLONG cbis = gi*pbs0/2 , cbie = (gi+1) * pbs0/2;
								V3DLONG cbjs = gj*pbs1/2 , cbje = (gj+1) * pbs1/2;
								V3DLONG cbks = gk*pbs2/2 , cbke = (gk+1) * pbs2/2;
								for(V3DLONG cbk = cbks; cbk < cbke; cbk++)
								{
									for(V3DLONG cbj = cbjs; cbj < cbje; cbj++)
									{
										for(V3DLONG cbi = cbis; cbi < cbie; cbi++)
										{
											V3DLONG pbi = (cbi - cbis)*2;
											V3DLONG pbj = (cbj - cbjs)*2;
											V3DLONG pbk = (cbk - cbks)*2;
											V3DLONG pind = pbk * pbs01 + pbj * pbs0 + pbi;
											V3DLONG cind = cbk * cbs01 + cbj * cbs0 + cbi;
											curimg1d[cind] = parimg1d[pind];
										}
									}
								}
								if(parimg1d){delete [] parimg1d; parimg1d = 0;}
								if(parsz){delete [] parsz; parsz = 0;}
							}
						}
					}
					ostringstream oss; oss<<prefix<<"_"<<level+1<<"_"<<ti<<"_"<<tj<<"_"<<tk<<".raw";
					string curimg_file = oss.str();
					V3DLONG cursz[4] = {cbs0, cbs1, cbs2, 1};
					cout<<"======== save "<<curimg_file<<" ========="<<endl;
					saveImage((char*) curimg_file.c_str(), curimg1d, cursz, V3D_UINT8);
					if(curimg1d){delete [] curimg1d; curimg1d = 0;}
				}
			}
		}

		pts0 = cts0; pts1 = cts1; pts2 = cts2;
		pbs0 = cbs0; pbs1 = cbs1; pbs2 = cbs2;
		level++;
	}
}

int main(int argc, char ** argv)
{
	createMapViewFiles("test", 8, 8, 8);
}
