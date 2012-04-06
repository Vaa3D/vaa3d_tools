#include <cmath>
#include <fstream>
#include <iostream>
#include "mapview.h"

using namespace std;

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif

static string basename(string para)
{
    int pos = para.find_last_of(".");
    if(pos == string::npos) return para;
    else return para.substr(0, pos);
}

string dirname(string para)
{
	int pos = para.find_last_of("/");
	if(pos == string::npos) return ".";
	else if(pos == 0) return "/";
	string dir = para.substr(0, pos);
}

string absolute_path(string dir)
{
	if(dir[0] == '.')
	{
		string pwd = getenv("PWD");
		if(dir.size() == 1) return pwd;
		else if(dir[1] == '/') return pwd + dir.substr(2, dir.size() - 2);
		else return dir;
	}
	else if(dir[0] == '~')
	{
		string home = getenv("HOME");
		if(dir.size() == 1) return home;
		else if(dir[1] == '/') return home + dir.substr(2, dir.size() - 2);
		else return dir;
	}
	return dir;
}

int main(int argc, char ** argv)
{
	if(argc < 6) 
	{
		cerr<<"Usage : "<<argv[0]<<"<raw_img_file> <L0_block_size0> <L0_block_size1> <L0_block_size2> <hraw_file>"<<endl;
		cerr<<endl;
		cerr<<"Step 1: split a raw image into many blocks with fixed block size"<<endl;
		cerr<<"Step 2: create hierarchy level data"<<endl;
		cerr<<"Step 3: create hraw_file"<<endl;
		return 0;
	}

	string infile = argv[1];
	V3DLONG insz0 = 0, insz1 = 0, insz2 = 0, channel = 0;
	getRawImageSize(infile, insz0, insz1, insz2, channel);

	V3DLONG bs0 = atoi(argv[2]);
	V3DLONG bs1 = atoi(argv[3]);
	V3DLONG bs2 = atoi(argv[4]);

	V3DLONG ts0 = (insz0 % bs0 == 0) ? insz0/bs0 : insz0/bs0 + 1;
	V3DLONG ts1 = (insz1 % bs1 == 0) ? insz1/bs1 : insz1/bs1 + 1;
	V3DLONG ts2 = (insz2 % bs2 == 0) ? insz2/bs2 : insz2/bs2 + 1;

	V3DLONG level_num = log(MIN(MIN(insz0, insz1), insz2))/log(2.0);

	string hraw_file = argv[5];
	string dir = dirname(infile);
	
	raw_split((char*)(infile.c_str()), (char*)dir.c_str(), bs0, bs1, bs2);
	createMapViewFiles((char*)dir.c_str(), ts0, ts1, ts2);

	ofstream ofs(hraw_file.c_str());
	if(ofs.fail()){cerr<<"Unable to open "<<hraw_file<<endl; return false;}
	
	ofs<<"PATH "<<absolute_path(dir)<<endl;
	ofs<<"L0_X_BLOCKS "<<ts0<<endl;
	ofs<<"L0_Y_BLOCKS "<<ts1<<endl;
	ofs<<"L0_Z_BLOCKS "<<ts2<<endl;
	ofs<<"CHANNEL "<<channel<<endl;
	ofs<<"LEVEL_NUM "<<level_num<<endl;
	ofs.close();
}
