#include <iostream>
#include <sstream>
#include <cmath>
#include <string>

#include "stackutil.h"
#include "mapview.h"

using namespace std;

#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif


ImageMapView::ImageMapView()
{
	L = M = N = 0;
	l = m = n = 0;
	prefix = "";
}

void ImageMapView::setPara(string _prefix, V3DLONG _L, V3DLONG _M, V3DLONG _N, V3DLONG _l, V3DLONG _m, V3DLONG _n)
{
	prefix = _prefix; L = _L; M = _M; N = _N; l = _l; m = _m; n = _n;
}

void ImageMapView::getImageSize(V3DLONG level, V3DLONG & ts0, V3DLONG & ts1, V3DLONG & ts2, V3DLONG &bs0, V3DLONG &bs1, V3DLONG &bs2)
{
	ts0 = L;
	ts1 = M;
	ts2 = N;
	bs0 = l;
	bs1 = m;
	bs2 = n;
	
	for(int i = 1; i < level; i++)
	{
		ts0 = (ts0 + 1)/2;
		ts1 = (ts1 + 1)/2;
		ts2 = (ts2 + 1)/2;

		if(ts0 == 1) bs0 = (bs0 + 1) /2;
		if(ts1 == 1) bs1 = (bs1 + 1) /2;
		if(ts2 == 1) bs2 = (bs2 + 1) /2;
	}
}

// Variables used :
// x0, y0, z0, sz0, sz1, sz2      the crop image origin and size
// oi, oj, ok, ois, oij, oik      the index of outimg1d
// pos0, posk, posj, posi         the seek offset of single block image file
// ti, tj, tk, tis, tjs, tks, tie, tje, tke   the block array index
// bi, bj, bk, bis, bjs, bks, bie, bje, bke   the index in a single block
void ImageMapView::getImage(V3DLONG level, unsigned char * &outimg1d, V3DLONG x0, V3DLONG y0, V3DLONG z0, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2)
{
	V3DLONG tol_sz = sz0 * sz1 * sz2;
	V3DLONG sz01 = sz0 * sz1;
	if(outimg1d == 0) outimg1d = new unsigned char[tol_sz]; for(V3DLONG ind = 0; ind < sz01; ind++) outimg1d[ind] = 0;
	V3DLONG ts0, ts1, ts2, bs0, bs1, bs2;                            // block size and block number
	getImageSize(level,ts0, ts1, ts2, bs0, bs1, bs2);

	cout<<"level = "<<level<<endl;
	cout<<"x0 = "<<x0<<endl;
	cout<<"y0 = "<<y0<<endl;
	cout<<"z0 = "<<z0<<endl;
	cout<<"sz0 = "<<sz0<<endl;
	cout<<"sz1 = "<<sz1<<endl;
	cout<<"sz2 = "<<sz2<<endl;
	cout<<"ts0 = "<<ts0<<endl;
	cout<<"ts1 = "<<ts1<<endl;
	cout<<"ts2 = "<<ts2<<endl;
	cout<<"bs0 = "<<bs0<<endl;
	cout<<"bs1 = "<<bs1<<endl;
	cout<<"bs2 = "<<bs2<<endl;

	V3DLONG tis = x0/bs0, tie = (((x0 + sz0) % bs0) == 0) ? (x0 + sz0)/bs0 - 1 : (x0 + sz0)/bs0; tie = MIN(tie, ts0-1);
	V3DLONG tjs = y0/bs1, tje = (((y0 + sz1) % bs1) == 0) ? (y0 + sz1)/bs1 - 1 : (y0 + sz1)/bs1; tje = MIN(tje, ts1-1);
	V3DLONG tks = z0/bs2, tke = (((z0 + sz2) % bs2) == 0) ? (z0 + sz2)/bs2 - 1 : (z0 + sz2)/bs2; tke = MIN(tje, ts2-1);
	V3DLONG block_size = bs0 * bs1 * bs2;
	V3DLONG bs01 = bs0 * bs1;

	cout<<"tis = "<<tis<<"\ttie = "<<tie<<endl;
	cout<<"tjs = "<<tjs<<"\ttje = "<<tje<<endl;
	cout<<"tks = "<<tks<<"\ttke = "<<tke<<endl;

	// Read related multiple blocks
	for(V3DLONG tk = tks; tk <= tke; tk++)
	{
		for(V3DLONG tj = tjs; tj <= tje; tj++)
		{
			for(V3DLONG ti = tis; ti <= tie; ti++)
			{
				V3DLONG bis = (ti == tis) ? (x0 - tis * bs0) : 0;
				V3DLONG bjs = (tj == tjs) ? (y0 - tjs * bs1) : 0;
				V3DLONG bks = (tk == tks) ? (z0 - tks * bs2) : 0;
				V3DLONG bie = (ti == tie) ? (x0 + sz0 - tie * bs0 - 1) : bs0 - 1;
				V3DLONG bje = (tj == tje) ? (y0 + sz1 - tje * bs1 - 1) : bs1 - 1;
				V3DLONG bke = (tk == tke) ? (z0 + sz2 - tke * bs2 - 1) : bs2 - 1;

				cout<<"bis = "<<bis<<"\tbie = "<<bie<<endl;
				cout<<"bjs = "<<bjs<<"\tbje = "<<bje<<endl;
				cout<<"bks = "<<bks<<"\tbke = "<<bke<<endl;

				V3DLONG ois = ti * bs0 - x0;
				V3DLONG ojs = tj * bs1 - y0;
				V3DLONG oks = tk * bs2 - z0;
				// Read block (ti, tj , tk)
				{
					ostringstream oss;
					oss<<prefix<<"_L"<<level<<"_X"<<ti<<"_Y"<<tj<<"_Z"<<tk<<".raw";
					cout<<"read "<<oss.str()<<endl;

					char * filename = (char*) oss.str().c_str();
					FILE * fid = fopen(filename, "rb");
					if(!fid) continue;

					fseek(fid, 0, SEEK_END);
					V3DLONG fileSize = ftell(fid);
					rewind(fid);

					char formattkey[] = "raw_image_stack_by_hpeng";
					V3DLONG lenkey = strlen(formattkey);

					if(fileSize != block_size+lenkey+2+4*4+1) {cout<<"image size incorrect"<<endl; continue;}
					V3DLONG pos0 = lenkey+2+4*4+1;
					fseek(fid, pos0, SEEK_SET);
					
					V3DLONG line_length = bie - bis + 1;
					cout<<"line_length = "<<line_length<<endl;
					unsigned char * buf = new unsigned char[line_length];
					if(buf == 0) {cerr<<"unable to alloc memory for buf"<<endl; continue;}
					for(V3DLONG bk = bks; bk <= bke; bk++)
					{
						V3DLONG posk = pos0 + bk * bs01;
						V3DLONG ok = oks + bk;
						for(V3DLONG bj = bjs; bj <= bje; bj++)
						{
							V3DLONG posj = posk + bj * bs0;
							V3DLONG oj = ojs + bj;

							V3DLONG posi = posj + bis;
							fseek(fid, posi, SEEK_SET);
							fread(buf, sizeof(unsigned char), line_length, fid);
							for(V3DLONG bi = bis; bi <= bie; bi++)
							{
								V3DLONG oi = ois + bi;
								V3DLONG oind = ok * sz01 + oj * sz0 + oi;
								outimg1d[oind] = buf[bi - bis];
							}
						}
					}
					if(buf){delete [] buf; buf = 0;}
					fclose(fid);
				}
				// Read Over 
			}
		}
	}
}
