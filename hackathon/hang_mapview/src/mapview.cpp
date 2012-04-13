#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>
#include <vector>
#include <string>
#include <cassert>
#include <set>
#include <map>

#include <pthread.h>
#include "stackutil.h"
#include "mapview.h"

using namespace std;

#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif

static bool read_raw_partially(const char * filename, unsigned char * &outimg1d, V3DLONG x0, V3DLONG y0, V3DLONG z0, V3DLONG outsz0, V3DLONG outsz1, V3DLONG outsz2, V3DLONG outsz3 = 0);

static bool is_file_exists(const char * filename)
{
	ifstream ifs(filename);
	return ifs;
}

// thread data and func
struct ThreadData
{
	string filename;
	unsigned char * outimg1d;
	
	V3DLONG bis;
	V3DLONG bie;
	V3DLONG bjs;
	V3DLONG bje;
	V3DLONG bks;
	V3DLONG bke;

	V3DLONG ois;
	V3DLONG ojs;
	V3DLONG oks;

	V3DLONG bs0;
	V3DLONG bs1;
	V3DLONG bs2;
	V3DLONG outsz0;
	V3DLONG outsz1;
	V3DLONG outsz2;
	V3DLONG channel;
};

vector<pthread_t> all_threads;
static map<string, unsigned char*> block_buff_map;
static vector<pthread_t> existing_threads;
static void * loadBlockImage(void* _para)
{
	ThreadData * para = (ThreadData *) _para;
	string filename = para->filename;

	unsigned char * inimg1d = 0; 
	if(block_buff_map.find(filename) == block_buff_map.end())
	{
		V3DLONG * in_sz = 0;
		int datatype;
		if(!loadImage((char*)filename.c_str(), inimg1d, in_sz, datatype))
		{
			block_buff_map[filename] = 0;
			inimg1d = 0;
		}
		else
		{
			block_buff_map[filename] = inimg1d;
			delete [] in_sz; in_sz = 0;
		}
	}
	else
	{
		inimg1d = block_buff_map[filename];
	}
	
	if(inimg1d)
	{
		unsigned char * outimg1d = para->outimg1d;
		assert(outimg1d);
		V3DLONG bis = para->bis;
		V3DLONG bjs = para->bjs;
		V3DLONG bks = para->bks;
		V3DLONG bie = para->bie;
		V3DLONG bje = para->bje;
		V3DLONG bke = para->bke;
		V3DLONG ois = para->ois;
		V3DLONG ojs = para->ojs;
		V3DLONG oks = para->oks;

		V3DLONG bs0 = para->bs0;
		V3DLONG bs1 = para->bs1;
		V3DLONG bs2 = para->bs2;

		V3DLONG outsz0 = para->outsz0;
		V3DLONG outsz1 = para->outsz1;
		V3DLONG outsz2 = para->outsz2;

		V3DLONG channel = para->channel;

		V3DLONG bs01 = bs0 * bs1;
		V3DLONG bs012 = bs01 * bs2;
		V3DLONG outsz01 = outsz0 * outsz1;
		V3DLONG outsz012 = outsz01 * outsz2;

		for(V3DLONG c = 0; c < channel; c++)
		{
			unsigned char * inimg1d_channel = inimg1d + c * bs012;
			unsigned char * outimg1d_channel = outimg1d + c * outsz012;
			for(V3DLONG bk = bks; bk <= bke; bk++)
			{
				V3DLONG ok = oks + bk; //tk * bs2 + bk - z0;
				for(V3DLONG bj = bjs; bj <= bje; bj++)
				{
					V3DLONG oj = ojs + bj;//tj * bs1 + bj - y0;
					for(V3DLONG bi = bis; bi <= bie; bi++)
					{
						V3DLONG oi = ois + bi;// ti * bs0 + bi - x0;
						V3DLONG ind1 = ok * outsz01 + oj * outsz0 + oi;
						V3DLONG ind2 = bk * bs01 + bj * bs0 + bi;
						outimg1d_channel[ind1] = inimg1d_channel[ind2];
					}
				}
			}
		}
	}

	if(para){delete (ThreadData*)para; para = 0;}
	return 0;
}
// thread over

ImageMapView::ImageMapView()
{
	L = M = N = 0;
	l = m = n = 0;
	channel = 3;
	dir = "";
}
ImageMapView::~ImageMapView()
{
	for(map<string, unsigned char*>::iterator it = block_buff_map.begin(); it != block_buff_map.end(); it++)
	{
		string filename = it->first;
		unsigned char * block_buff = it->second;
		if(block_buff){delete [] block_buff; block_buff = 0;}
	}
}

void ImageMapView::setPara(string _dir, V3DLONG _L, V3DLONG _M, V3DLONG _N, V3DLONG _l, V3DLONG _m, V3DLONG _n, V3DLONG _channel)
{
	dir = _dir; L = _L; M = _M; N = _N; l = _l; m = _m; n = _n; channel = _channel;
}

// level start from 0
void ImageMapView::getBlockTillingSize(V3DLONG level, V3DLONG & ts0, V3DLONG & ts1, V3DLONG & ts2, V3DLONG &bs0, V3DLONG &bs1, V3DLONG &bs2)
{
	ts0 = L;
	ts1 = M;
	ts2 = N;
	bs0 = l;
	bs1 = m;
	bs2 = n;
	
	cout<<"ts0 ts1 ts2 bs0 bs1 bs2"<<endl;
	cout<<ts0<<" "<<ts1<<" "<<ts2<<" "<<bs0<<" "<<bs1<<" "<<bs2<<endl;
	for(int i = 0; i < level; i++)
	{
		ts0 = (ts0 + 1)/2;
		ts1 = (ts1 + 1)/2;
		ts2 = (ts2 + 1)/2;

		if(ts0 == 1) bs0 = (bs0 + 1) /2;
		if(ts1 == 1) bs1 = (bs1 + 1) /2;
		if(ts2 == 1) bs2 = (bs2 + 1) /2;
		cout<<ts0<<" "<<ts1<<" "<<ts2<<" "<<bs0<<" "<<bs1<<" "<<bs2<<endl;
	}
}

// Variables used :
// x0, y0, z0, sz0, sz1, sz2      the crop image origin and size
// oi, oj, ok, ois, oij, oik      the index of outimg1d
// pos0, posk, posj, posi         the seek offset of single block image file
// ti, tj, tk, tis, tjs, tks, tie, tje, tke   the block array index
// bi, bj, bk, bis, bjs, bks, bie, bje, bke   the index in a single block
void ImageMapView::getImage(V3DLONG level, unsigned char * & outimg1d, V3DLONG x0, V3DLONG y0, V3DLONG z0, V3DLONG outsz0, V3DLONG outsz1, V3DLONG outsz2, bool is_use_thread)
{
	V3DLONG outsz012 = outsz0 * outsz1 * outsz2;
	V3DLONG outsz01 = outsz0 * outsz1;

	try
	{
		if(outimg1d == 0) outimg1d = new unsigned char[outsz012 * channel];
		//else outimg1d = (unsigned char*)realloc(outimg1d, outsz012*channel);
	}
	catch(...)
	{
		cerr<<"allocate image memory error."<<endl;
		return;
	}
	memset(outimg1d, 0, outsz012 * channel);

	V3DLONG ts0, ts1, ts2, bs0, bs1, bs2;                            // block number and block size
	getBlockTillingSize(level,ts0, ts1, ts2, bs0, bs1, bs2);

	cout<<"level = "<<level<<endl;

	V3DLONG tis = x0/bs0, tie = (((x0 + outsz0) % bs0) == 0) ? (x0 + outsz0)/bs0 - 1 : (x0 + outsz0)/bs0; tie = MIN(tie, ts0-1);
	V3DLONG tjs = y0/bs1, tje = (((y0 + outsz1) % bs1) == 0) ? (y0 + outsz1)/bs1 - 1 : (y0 + outsz1)/bs1; tje = MIN(tje, ts1-1);
	V3DLONG tks = z0/bs2, tke = (((z0 + outsz2) % bs2) == 0) ? (z0 + outsz2)/bs2 - 1 : (z0 + outsz2)/bs2; tke = MIN(tke, ts2-1);
	V3DLONG block_size = bs0 * bs1 * bs2;
	V3DLONG bs01 = bs0 * bs1;

	// Read related multiple blocks

	//bool is_use_thread = mapview_paras.is_use_thread();
	if(is_use_thread)
	{
		// 1. stop all existing threads
		for(int t = 0; t < existing_threads.size(); t++)
		{
			pthread_t thread = existing_threads[t];
			pthread_cancel(thread);
		}
		existing_threads.clear();

		// 2. the the filename of all related blocks
		set<string> related_blocks;
		for(V3DLONG tk = tks; tk <= tke; tk++)
		{
			for(V3DLONG tj = tjs; tj <= tje; tj++)
			{
				for(V3DLONG ti = tis; ti <= tie; ti++)
				{
					ostringstream oss;
					oss<<dir<<"/L"<<level<<"/L"<<level<<"_X"<<ti<<"_Y"<<tj<<"_Z"<<tk<<".raw";
					string filename = oss.str();
					related_blocks.insert(filename);
				}
			}
		}

		// 3. delete unused blocks
		map<string, unsigned char*>::iterator it = block_buff_map.begin();
		while(it != block_buff_map.end())
		{
			string filename = it->first;
			unsigned char * block_buff = it->second;
			if(related_blocks.find(filename) == related_blocks.end())
			{
				if(block_buff){delete [] block_buff; block_buff = 0;}
				block_buff_map.erase(it);
			}
			it++;
		}

		// 4. create thread to load related block and set to outimg1d
		for(V3DLONG tk = tks; tk <= tke; tk++)
		{
			for(V3DLONG tj = tjs; tj <= tje; tj++)
			{
				for(V3DLONG ti = tis; ti <= tie; ti++)
				{
					ostringstream oss;
					oss<<dir<<"/L"<<level<<"/L"<<level<<"_X"<<ti<<"_Y"<<tj<<"_Z"<<tk<<".raw";
					string filename = oss.str();

					pthread_t thread;
					ThreadData * para = new ThreadData;
					para->filename = filename;
					para->outimg1d = outimg1d;
					para->bis = (ti == tis) ? (x0 - tis * bs0) : 0;                   // the start x index in current block
					para->bjs = (tj == tjs) ? (y0 - tjs * bs1) : 0;                   // the start y index in current block
					para->bks = (tk == tks) ? (z0 - tks * bs2) : 0;                   // the start z index in current block
					para->bie = (ti == tie) ? (x0 + outsz0 - tie * bs0 - 1) : bs0 - 1;   // the end x index in current block
					para->bje = (tj == tje) ? (y0 + outsz1 - tje * bs1 - 1) : bs1 - 1;   // the end y index in current block
					para->bke = (tk == tke) ? (z0 + outsz2 - tke * bs2 - 1) : bs2 - 1;   // the end z index in current block
					para->bie = MIN(para->bie, bs0-1);
					para->bje = MIN(para->bje, bs1-1);
					para->bke = MIN(para->bke, bs2-1);

					para->ois = ti*bs0 - x0;
					para->ojs = tj*bs1 - y0;
					para->oks = tk*bs2 - z0;
					para->bs0 = bs0;
					para->bs1 = bs1;
					para->bs2 = bs2;
					para->outsz0 = outsz0;
					para->outsz1 = outsz1;
					para->outsz2 = outsz2;
					para->channel = channel;

					pthread_create(&thread, NULL, loadBlockImage, (void*)para);
					existing_threads.push_back(thread);
				}
			}
		}
	}
	else
	{
		for(V3DLONG tk = tks; tk <= tke; tk++)
		{
			for(V3DLONG tj = tjs; tj <= tje; tj++)
			{
				for(V3DLONG ti = tis; ti <= tie; ti++)
				{
					V3DLONG bis = (ti == tis) ? (x0 - tis * bs0) : 0;                   // the start x index in current block
					V3DLONG bjs = (tj == tjs) ? (y0 - tjs * bs1) : 0;                   // the start y index in current block
					V3DLONG bks = (tk == tks) ? (z0 - tks * bs2) : 0;                   // the start z index in current block
					V3DLONG bie = (ti == tie) ? (x0 + outsz0 - tie * bs0 - 1) : bs0 - 1;   // the end x index in current block
					V3DLONG bje = (tj == tje) ? (y0 + outsz1 - tje * bs1 - 1) : bs1 - 1;   // the end y index in current block
					V3DLONG bke = (tk == tke) ? (z0 + outsz2 - tke * bs2 - 1) : bs2 - 1;   // the end z index in current block

					cout<<"bis = "<<bis<<"\tbie = "<<bie<<endl;
					cout<<"bjs = "<<bjs<<"\tbje = "<<bje<<endl;
					cout<<"bks = "<<bks<<"\tbke = "<<bke<<endl;

					V3DLONG ois = ti * bs0 - x0;
					V3DLONG ojs = tj * bs1 - y0;
					V3DLONG oks = tk * bs2 - z0;

					unsigned char * tmpimg1d = 0; 
					V3DLONG tmpsz0 = bie - bis + 1;
					V3DLONG tmpsz1 = bje - bjs + 1;
					V3DLONG tmpsz2 = bke - bks + 1;
					V3DLONG tmpsz01 = tmpsz0 * tmpsz1;
					V3DLONG tmpsz012 = tmpsz01 * tmpsz2;

					ostringstream oss;
					oss<<dir<<"/L"<<level<<"/L"<<level<<"_X"<<ti<<"_Y"<<tj<<"_Z"<<tk<<".raw";
					cout<<"read "<<oss.str()<<endl;
					char * filename = (char*) oss.str().c_str();

					read_raw_partially(filename, tmpimg1d, bis, bjs, bks, tmpsz0, tmpsz1, tmpsz2, 0);

					for(V3DLONG c = 0; c < channel; c++)
					{
						unsigned char * tmpimg1d_channel = tmpimg1d + c * tmpsz012;
						unsigned char * outimg1d_channel = outimg1d + c * outsz012;
						for(V3DLONG bk = bks; bk <= bke; bk++)
						{
							V3DLONG ok = oks + bk;
							for(V3DLONG bj = bjs; bj <= bje; bj++)
							{
								V3DLONG oj = ojs + bj;
								for(V3DLONG bi = bis; bi <= bie; bi++)
								{
									V3DLONG oi = ois + bi;
									V3DLONG ind1 = ok * outsz01 + oj * outsz0 + oi;
									V3DLONG ind2 = (bk-bks) * tmpsz01 + (bj-bjs) * tmpsz0 + (bi-bis);
									outimg1d_channel[ind1] = tmpimg1d_channel[ind2];
								}
							}
						}
					}
					if(tmpimg1d){delete [] tmpimg1d; tmpimg1d = 0;}
				}
			}
		}
	}
}

void getRawImageSize(string filename, V3DLONG &sz0, V3DLONG &sz1, V3DLONG &sz2, V3DLONG &sz3)
{
	//cout<<"filename = "<<filename<<endl;
    FILE * fid = fopen((char*) filename.c_str(), "rb");
    if(!fid) return;

    int lenkey = strlen("raw_image_stack_by_hpeng");
    fseek(fid, lenkey, SEEK_SET);
	
	char endianCodeData;
	fread(&endianCodeData, 1, 1, fid);
	int b_swap = (endianCodeData == checkMachineEndian())?0:1;

	short int dcode;
	fread(&dcode, 2, 1, fid); if (b_swap) swap2bytes((void *)&dcode);

    int mysz[4]; mysz[0] = mysz[1] = mysz[2] = mysz[3] = 0;
    int n = fread(mysz, 4, 4, fid);
    if(n!= 4) return;
	if(b_swap)
	{
		swap4bytes((void*)(mysz+0));
		swap4bytes((void*)(mysz+1));
		swap4bytes((void*)(mysz+2));
		swap4bytes((void*)(mysz+3));
	}

    sz0 = mysz[0];
    sz1 = mysz[1];
    sz2 = mysz[2];
	sz3 = mysz[3];
    fclose(fid);
}


// outsz3 == 0 for all channels
static bool read_raw_partially(const char * filename, unsigned char * &outimg1d, V3DLONG x0, V3DLONG y0, V3DLONG z0, V3DLONG outsz0, V3DLONG outsz1, V3DLONG outsz2, V3DLONG outsz3)
{
	//cout<<"read_raw_partially x0 = "<<x0<<" y0 = "<<y0<<" z0 = "<<z0<<" outsz0 = "<<outsz0<<" outsz1 = "<<outsz1<<" outsz2 = "<<outsz2<<" outsz3 = "<<outsz3<<endl;

	V3DLONG insz0 = 0, insz1 = 0, insz2 = 0, insz3 = 0;
	getRawImageSize(filename, insz0, insz1, insz2, insz3);
	V3DLONG insz01 = insz0 * insz1;
	V3DLONG insz012 = insz01 * insz2;
	//cout<<"insz0 = "<<insz0<<" insz1 = "<<insz1<<" insz2 = "<<insz2<<" insz3 = "<<insz3<<endl;

	if(outsz3 == 0) outsz3 = insz3;

	V3DLONG outsz01 = outsz0 * outsz1;
	V3DLONG outsz012 = outsz01 * outsz2;
	if(outimg1d){cerr<<"outimg1d is not empty"<<endl;}
	outimg1d = new unsigned char[outsz012 * outsz3]; memset(outimg1d, 0, outsz012 * outsz3);

	if(x0 < 0 || x0 >= insz0 || insz0 <= 0 ||  
			y0 < 0 || y0 >= insz1 || insz1 <= 0 || 
			z0 < 0 || z0 >= insz2 || insz2 <= 0 ||
			insz3 <= 0) return 0;


	FILE * fid = fopen(filename, "rb");
	if(!fid) return false;

	fseek(fid, 0, SEEK_END);
	V3DLONG fileSize = ftell(fid);
	rewind(fid);

	char formattkey[] = "raw_image_stack_by_hpeng";
	V3DLONG lenkey = strlen(formattkey);

	//if(fileSize != block_size+lenkey+2+4*4+1) {cout<<"image size incorrect"<<endl; continue;}
	V3DLONG pos0 = lenkey+2+4*4+1;
	fseek(fid, pos0, SEEK_SET);

	V3DLONG xs = x0, xe = MIN(x0 + outsz0, insz0), dx = xe - xs;
	V3DLONG ys = y0, ye = MIN(y0 + outsz1, insz1), dy = ye - ys;
	V3DLONG zs = z0, ze = MIN(z0 + outsz2, insz2), dz = ze - zs;

	cout<<"xs = "<<xs<<" xe = "<<xe<<" dx = "<<dx<<endl;
	cout<<"ys = "<<ys<<" ye = "<<ye<<" dy = "<<dy<<endl;
	cout<<"zs = "<<zs<<" ze = "<<ze<<" dz = "<<dz<<endl;

	V3DLONG line_length = xe - xs;
	cout<<"line_length = "<<line_length<<endl;
	unsigned char * buf = new unsigned char[line_length];
	if(buf == 0) {cerr<<"unable to alloc memory for buf"<<endl; return false;}
	
	for(V3DLONG c = 0; c < outsz3; c++)
	{
		V3DLONG pos1 = pos0 + c * insz012;
		unsigned char * outimg1d_channel = outimg1d + c * outsz012;
		for(V3DLONG z = 0; z < dz; z++)
		{
			for(V3DLONG y = 0; y < dy; y++)
			{
				V3DLONG pos2 = pos1 + (z+zs) * insz01 + (y+ys) * insz0 + xs;
				fseek(fid, pos2, SEEK_SET);
				fread(buf, sizeof(unsigned char), line_length, fid);
				V3DLONG ind0 = z*outsz01 + y*outsz0;
				for(V3DLONG x = 0; x < dx; x++)
				{
					outimg1d_channel[ind0 + x] = buf[x];
				}
			}
		}
	}
	if(buf){delete [] buf; buf = 0;}
	fclose(fid);
	return true;
}

bool raw_split(char * infile, char * dir, V3DLONG outsz0, V3DLONG outsz1, V3DLONG outsz2)
{
	cout<<"outsz0 = "<<outsz0<<" outsz1 = "<<outsz1<<" outsz2 = "<<outsz2<<endl;
	V3DLONG insz0 = 0, insz1 = 0, insz2 = 0, channel = 0;
	getRawImageSize(infile, insz0, insz1, insz2, channel);
	cout<<"insz0 = "<<insz0<<" insz1 = "<<insz1<<" insz2 = "<<insz2<<" channel = "<<channel<<endl;

	system(string(string("mkdir ") + dir + "/L0").c_str());

	int ts0 = (insz0 % outsz0 == 0) ? insz0/outsz0 : insz0/outsz0 + 1;
	int ts1 = (insz1 % outsz1 == 0) ? insz1/outsz1 : insz1/outsz1 + 1;
	int ts2 = (insz2 % outsz2 == 0) ? insz2/outsz2 : insz2/outsz2 + 1;
	for(int tz = 0; tz < ts2; tz++)
	{
		V3DLONG z0 = tz * outsz2;
		for(int ty = 0; ty < ts1; ty++)
		{
			V3DLONG y0 = ty * outsz1;
			for(int tx = 0; tx < ts0; tx++)
			{
				V3DLONG x0 = tx * outsz0;
				unsigned char * outimg1d = 0;
				ostringstream oss;
				oss<<dir<<"/L0/L0_X"<<tx<<"_Y"<<ty<<"_Z"<<tz<<".raw";
				V3DLONG tmpsz0, tmpsz1, tmpsz2, tmpsz3;
				if(is_file_exists(oss.str().c_str())) getRawImageSize(oss.str(), tmpsz0, tmpsz1, tmpsz2, tmpsz3);
				if(!is_file_exists(oss.str().c_str()) || ! (outsz0 == tmpsz0 && outsz1 == tmpsz1 && outsz2 == tmpsz2 && channel == tmpsz3))
				{
					if(outsz0 == tmpsz0 && outsz1 == tmpsz1 && outsz2 == tmpsz2 && channel == tmpsz3){cout<<"overide "<<oss.str()<<endl;}
					read_raw_partially(infile, outimg1d, x0, y0, z0, outsz0, outsz1, outsz2, channel);
					V3DLONG outsz[4] = {outsz0, outsz1, outsz2, channel};
					cout<<"save "<<oss.str()<<endl;
					saveImage(oss.str().c_str(), outimg1d, outsz, V3D_UINT8);
				}
				else
				{
					cout<<oss.str()<<" exists"<<endl;
				}
			}
		}
	}
	return true;
}

// make sure L0 folder is exist
bool createMapViewFiles(char * dir, V3DLONG ts0, V3DLONG ts1, V3DLONG ts2)
{
	//make sure all blocks have the same size
	V3DLONG bs0 = 0, bs1 = 0, bs2 = 0, channel = 0; //block size
	// get size of first block
	{
		ostringstream oss;
		oss<<dir<<"/L0/L0_"<<"X"<<0<<"_Y"<<0<<"_Z"<<0<<".raw";
		string filename = oss.str();
		V3DLONG tmp_sz0, tmp_sz1, tmp_sz2, tmp_channel;
		getRawImageSize(filename, tmp_sz0, tmp_sz1, tmp_sz2, tmp_channel);
		bs0 = tmp_sz0; bs1 = tmp_sz1; bs2 = tmp_sz2; channel = tmp_channel;
	}

	if(0)
	{
		for(V3DLONG tk = 0; tk < ts2; tk++)
		{
			for(V3DLONG tj = 0; tj < ts1; tj++)
			{
				for(V3DLONG ti = 0; ti < ts0; ti++)
				{
					ostringstream oss;
					oss<<dir<<"/L0/L0_"<<"X"<<ti<<"_Y"<<tj<<"_Z"<<tk<<".raw";
					string filename = oss.str();
					V3DLONG tmp_sz0, tmp_sz1, tmp_sz2, tmp_channel;
					getRawImageSize(filename, tmp_sz0, tmp_sz1, tmp_sz2, tmp_channel);
					if(bs0 != tmp_sz0 || bs1 != tmp_sz1 || bs2 != tmp_sz2 || channel != tmp_channel)
					{
						cerr<<"file "<<filename<<" have different size"<<endl;
						return false;
					}
				}
			}
		}
	}
	assert(channel <= 3);

	//V3DLONG min_level = 0, max_level = 10;// log(MIN(MIN(ts0, ts1), ts2))/log(2.0);
	V3DLONG pts0 = ts0, pts1 = ts1, pts2 = ts2;       // parent tiling size
	V3DLONG pbs0 = bs0, pbs1 = bs1, pbs2 = bs2;       // parent block size
	V3DLONG cts0 = ts0, cts1 = ts1, cts2 = ts2;       // child tiling size
	V3DLONG cbs0 = bs0, cbs1 = bs1, cbs2 = bs2;       // child block size
	V3DLONG level = 0;
	while(true)
	{
		cout<<"================= level "<<level<<" ======================="<<endl;
		{
			ostringstream oss; oss<<"mkdir "<<dir<<"/L"<<level+1;
			system(oss.str().c_str());
		}

		if((pts0 == 1 && pbs0 == 1) ||
		   (pts1 == 1 && pbs1 == 1) ||
		   (pts2 == 1 && pbs2 == 1)) break;
		(pts0 == 1) ? (cbs0 = (pbs0 + 1)/2) : (cts0 = (pts0 + 1)/2);
		(pts1 == 1) ? (cbs1 = (pbs1 + 1)/2) : (cts1 = (pts1 + 1)/2);
		(pts2 == 1) ? (cbs2 = (pbs2 + 1)/2) : (cts2 = (pts2 + 1)/2);

		V3DLONG pbs01 = pbs0 * pbs1;
		V3DLONG pbs012 = pbs01 * pbs2;

		V3DLONG cbs01 = cbs0 * cbs1;
		V3DLONG cbs012 = cbs01 * cbs2;

		for(V3DLONG tk = 0; tk < cts2; tk++)
		{
			for(V3DLONG tj = 0; tj < cts1; tj++)
			{
				for(V3DLONG ti = 0; ti < cts0; ti++)
				{
					string curimg_file;
					{
						ostringstream oss; oss<<dir<<"/L"<<level+1<<"/L"<<level+1<<"_X"<<ti<<"_Y"<<tj<<"_Z"<<tk<<".raw";
						curimg_file = oss.str();
						if(is_file_exists(curimg_file.c_str()))
						{
							cout<<curimg_file<<" exists"<<endl;
							continue;
						}
					}

					V3DLONG gimax = (ti == cts0 - 1 && 2*cts0 - 1 == pts0) ? 0 : 1;
					V3DLONG gjmax = (tj == cts1 - 1 && 2*cts1 - 1 == pts1) ? 0 : 1;
					V3DLONG gkmax = (tk == cts2 - 1 && 2*cts2 - 1 == pts2) ? 0 : 1;

					unsigned char * curimg1d = new unsigned char[cbs012 * channel]; memset(curimg1d, 0, cbs012 * channel);
					// grid i, j k
					for(V3DLONG gk = 0; gk <= gkmax; gk++)
					{
						for(V3DLONG gj = 0; gj <= gjmax; gj++)
						{
							for(V3DLONG gi = 0; gi <= gimax; gi++)
							{
								ostringstream oss;
								oss<<dir<<"/L"<<level<<"/L"<<level<<"_X"<<2*ti + gi<<"_Y"<<2*tj + gj<<"_Z"<<2*tk + gk<<".raw";
								string parimg_file = oss.str();
								assert(is_file_exists(parimg_file.c_str()));

								unsigned char * parimg1d = 0; V3DLONG * parsz = 0; int datatype;
								if(!loadImage((char *) parimg_file.c_str(), parimg1d, parsz, datatype)){cerr<<"load "<<parimg_file<<" error."<<endl; return false;}
								if(parsz[0] != pbs0 || parsz[1] != pbs1 || parsz[2] != pbs2 || parsz[3] != channel)
								{
									cerr<<"Incrorect size for "<<parimg_file<<endl;
									cerr<<"It should be "<<pbs0<<"x"<<pbs1<<"x"<<pbs2<<"x"<<channel<<" , the actual size is "<<parsz[0]<<"x"<<parsz[1]<<"x"<<parsz[2]<<"x"<<parsz[3]<<endl;
									return false;
								}

								V3DLONG cbis = gi*pbs0/2 , cbie = (gi+1) * pbs0/2;    // child block index along x
								V3DLONG cbjs = gj*pbs1/2 , cbje = (gj+1) * pbs1/2;
								V3DLONG cbks = gk*pbs2/2 , cbke = (gk+1) * pbs2/2;
								for(int chn = 0; chn < channel; chn++)
								{
									unsigned char * curimg1d_channel = curimg1d + chn * cbs012;
									unsigned char * parimg1d_channel = parimg1d + chn * pbs012;

									for(V3DLONG cbk = cbks; cbk < cbke; cbk++)
									{
										for(V3DLONG cbj = cbjs; cbj < cbje; cbj++)
										{
											for(V3DLONG cbi = cbis; cbi < cbie; cbi++)
											{
												if(0) //is_blur
												{
													V3DLONG pbi = (cbi - cbis)*2;
													V3DLONG pbj = (cbj - cbjs)*2;
													V3DLONG pbk = (cbk - cbks)*2;
													V3DLONG pind = pbk * pbs01 + pbj * pbs0 + pbi;
													V3DLONG cind = cbk * cbs01 + cbj * cbs0 + cbi;
													curimg1d_channel[cind] = parimg1d_channel[pind];
												}
												else
												{
													V3DLONG pbi = (cbi - cbis)*2;
													V3DLONG pbj = (cbj - cbjs)*2;
													V3DLONG pbk = (cbk - cbks)*2;
													V3DLONG pind = pbk * pbs01 + pbj * pbs0 + pbi;
													V3DLONG cind = cbk * cbs01 + cbj * cbs0 + cbi;
													double sum_int = parimg1d_channel[pind];
													int count = 1;
													if(pbi - 1 >= 0) {sum_int += parimg1d_channel[pind - 1]; count++;}
													if(pbj - 1 >= 0) {sum_int += parimg1d_channel[pind - pbs0]; count++;}
													if(pbk - 1 >= 0) {sum_int += parimg1d_channel[pind - pbs01]; count++;}
													if(pbi + 1 < pbs0) {sum_int += parimg1d_channel[pind + 1]; count++;}
													if(pbj + 1 < pbs1) {sum_int += parimg1d_channel[pind + pbs0]; count++;}
													if(pbk + 1 < pbs2) {sum_int += parimg1d_channel[pind + pbs01]; count++;}
													
													curimg1d_channel[cind] = sum_int/count + 0.4999;
												}
											}
										}
									}
								}
								if(parimg1d){delete [] parimg1d; parimg1d = 0;}
								if(parsz){delete [] parsz; parsz = 0;}
							}
						}
					}	
					cout<<"======== save "<<curimg_file<<" ========="<<endl;
					V3DLONG cursz[4] = {cbs0, cbs1, cbs2, channel};
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

void ImageMapView::getDownSamplingSize(V3DLONG level, V3DLONG & out_sz0, V3DLONG & out_sz1, V3DLONG & out_sz2)
{
	out_sz0 = L * l;
	out_sz1 = M * m;
	out_sz2 = N * n;
	
	for(int i = 0; i < level; i++)
	{
		out_sz0 = (out_sz0 + 1)/2;
		out_sz1 = (out_sz1 + 1)/2;
		out_sz2 = (out_sz2 + 1)/2;
	}
}
