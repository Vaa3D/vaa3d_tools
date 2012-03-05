/****************************************************************
 * file : common_macro.h, Oct 2011 by Hang Xiao
 *
 * Common NUMBER MACRO and common OPERATION MACRO
 *
 * Do not define obscure macro here, like INF (int, float, double?)
 *
 * **************************************************************/
#ifndef __COMMON_MACRO_H__
#define __COMMON_MACRO_H__

#ifndef MAX_LONG_DOUBLE
#define MAX_LONG_DOUBLE 1.18972e+4932  //actual: 1.18973e+4932
#endif

#ifndef MAX_DOUBLE 
#define MAX_DOUBLE 1.79768e+308        //actual: 1.79769e+308
#endif

#ifndef MAX_FLOAT
#define MAX_FLOAT 3.40281e+38          //actual: 3.40282e+38
#endif

#ifndef MAX_LONG_LONG
#define MAX_LONG_LONG 9223372036854775807
#endif

#ifndef MAX_INT
#define MAX_INT  2147483647
#endif


#ifndef ABS
#define ABS(x) ((x) > 0 ? (x) : (-(x)))
#endif

#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif

#ifndef MAXVALUE
#define MAXVALUE(T) (T)((1ll<<(sizeof(T)*8 -1)) - 1)
#endif

#ifndef INIMG3D
#define INIMG3D(z,y,x) (inimg1d[(z)*sz01 + (y)*sz0 + x])
#endif

#ifndef OUTIMG3D
#define OUTIMG3D(z,y,x) (outimg1d[(z)*sz01 + (y)*sz0 + x])
#endif

#ifndef INMASK3D
#define INMASK3D(z,y,x)    (inmask1d[(z)*sz01 + (y)*sz0 + (x)])
#endif

#ifndef OUTMASK3D
#define OUTMASK3D(z,y,x)    (outmask1d[(z)*sz01 + (y)*sz0 + (x)])
#endif

#ifndef INIT_IMAGE_VARIABLE
#define INIT_IMAGE_VARIABLE string infile = parser.filelist.size() >= 1 ? parser.filelist.at(0) : string(""); \
							string outfile = parser.filelist.size() >=2 ? parser.filelist.at(1) : ((infile != "") ? (basename(infile)+ "_out" + file_type(infile)) : ""); \
							unsigned char * indata1d = NULL, * outdata1d = NULL; \
                            V3DLONG* in_sz = 0, * out_sz = 0; int datatype = V3D_UINT8; int channel = 0;  
#endif

#ifndef LOAD_FIRST_IMAGE
#define LOAD_FIRST_IMAGE if(infile != "" && is_img_file(infile) && !loadImage((char*) infile.c_str(), indata1d, in_sz, datatype)) {cout<<"Unable to load image"<<endl; return false;} if(infile != "" &&  indata1d && datatype != V3D_UINT8){cerr<<"Can not deal with 12bit or 16bit data."<<endl; return false;}
#endif

#ifndef CHECK_CHANNEL
#define CHECK_CHANNEL if(parser.is_exist("-channel")){\
						if((channel = parser.get_int_para("-channel")) < 0 || channel > in_sz[3]){cout<<"channel should be  0 or no larger than image channel number"; return false;}\
	                  	if(channel >= 1){channel = channel - 1; indata1d = indata1d + in_sz[0] * in_sz[1] * in_sz[2] * channel; in_sz[3] = 1;} \
						} 
#endif

#ifdef __V3D_CONVERT_PLUGIN__
#ifndef SAVE_OUTPUT_IMAGE
#define SAVE_OUTPUT_IMAGE if(outdata1d) \
	{ \
		if(out_sz == 0) {out_sz = new V3DLONG[4]; out_sz[0] = in_sz[0]; out_sz[1] = in_sz[1]; out_sz[2] = in_sz[2]; out_sz[3] = in_sz[3];} \
		cout<<"save image to "<<outfile<<endl; \
		if(!saveImage((char*) outfile.c_str(), outdata1d, out_sz, datatype)) {cout<<"saveImage(\""<<outfile<<"\") error"; return false;} \
		extern string global_outimg_file; \
		global_outimg_file = outfile; \
	} \
	if(indata1d == outdata1d && indata1d != 0) {delete [] indata1d; indata1d = 0; outdata1d = 0;} \
	else \
	{ \
		if(indata1d) {delete [] indata1d; indata1d = 0;} \
		if(outdata1d) {delete [] outdata1d; outdata1d = 0;} \
	} \
	if(in_sz == out_sz && in_sz != 0) {delete [] in_sz; in_sz = 0; out_sz = 0;} \
	else \
	{ \
		if(in_sz){delete [] in_sz; in_sz = 0;} \
		if(out_sz){delete [] out_sz; out_sz = 0;} \
	}

#endif

#else
#ifndef SAVE_OUTPUT_IMAGE
#define SAVE_OUTPUT_IMAGE if(outdata1d) \
	{ \
		if(out_sz == 0) {out_sz = new V3DLONG[4]; out_sz[0] = in_sz[0]; out_sz[1] = in_sz[1]; out_sz[2] = in_sz[2]; out_sz[3] = in_sz[3];} \
		cout<<"save image to "<<outfile<<endl; \
		if(!saveImage((char*) outfile.c_str(), outdata1d, out_sz, datatype)) {cout<<"saveImage(\""<<outfile<<"\") error"; return false;} \
	} \
	if(indata1d == outdata1d && indata1d != 0) {delete [] indata1d; indata1d = 0; outdata1d = 0;} \
	else \
	{ \
		if(indata1d) {delete [] indata1d; indata1d = 0;} \
		if(outdata1d) {delete [] outdata1d; outdata1d = 0;} \
	} \
	if(in_sz == out_sz && in_sz != 0) {delete [] in_sz; in_sz = 0; out_sz = 0;} \
	else \
	{ \
		if(in_sz){delete [] in_sz; in_sz = 0;} \
		if(out_sz){delete [] out_sz; out_sz = 0;} \
	}
#endif
#endif

#ifndef LOAD_SECOND_IMAGE
#define LOAD_SECOND_IMAGE string infile1 = infile; \
		unsigned char * inimg1 = indata1d; \
		V3DLONG * in1_sz = in_sz;\
		int datatype1 = datatype;\
		string infile2 = parser.filelist.at(1);\
		unsigned char * inimg2 = 0; \
		V3DLONG * in2_sz = 0;\
		int datatype2;\
		if(!loadImage((char*)infile2.c_str(), inimg2, in2_sz, datatype2)) {cout<<"unable to read image "<<infile2<<endl; return false;}
#endif 

#ifndef GET_MEM
#define GET_MEM(T,sz,key) (T*)shmat(shmget((key), (sz), IPC_CREAT | 0666),0,0)
#endif

#ifndef REFRESH_XYZ
#define REFRESH_XYZ(x,y,z) {x++; if(x==sz0){x=0; y++; if(y==sz1){y=0;z++;}}}
#endif

#ifndef REFRESH_XY
#define REFRESH_XY(x,y) {x++; if(x==sz0){x=0; y++;}}
#endif

#ifndef IS_FGD3D
#define IS_FGD3D(z,y,x)    ((z)>=0 && (z) < sz2 && (y)>=0 && (y)<sz1 && (x)>=0 && (x)<sz0 && INMASK3D(z,y,x) > 0)
#endif

#ifndef IS_BKG3D
#define IS_BKG3D(z,y,x)    ((z)>=0 && (z) < sz2 && (y)>=0 && (y)<sz1 && (x)>=0 && (x)<sz0 && INMASK3D(z,y,x) == 0)
#endif

#ifndef IS_BOUNDARY3D
#define IS_BOUNDARY3D(z,y,x) (IS_FGD3D(z,y,x) && (IS_BKG3D(z,y,x-1) || IS_BKG3D(z,y,x+1) || \
			                                      IS_BKG3D(z,y-1,x) || IS_BKG3D(z,y+1,x) || \
			                                      IS_BKG3D(z-1,y,x) || IS_BKG3D(z+1,y,x)))
#endif

//#ifndef INF
//#define INF 3.4e38                         // please define in your own file to avoid overide by other INF definition
//#endif

#ifndef HTTP_DOWNLOAD
#define HTTP_DOWNLOAD(url, outfile) \
{\
    cout<<"download "<<url<<" to "<<outfile<<endl; \
	CURL *curl; \
	CURLcode res; \
	curl = curl_easy_init(); \
	if(curl) { \
		FILE * fp = fopen(outfile.c_str(), "wb"); \
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); \
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite); \
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp); \
		res = curl_easy_perform(curl); \
		curl_easy_cleanup(curl); \
		fclose(fp);\
	}\
}
#endif

#ifndef GET_LOCAL_TIME_STR
#define GET_LOCAL_TIME_STR(str) \
	{\
		time_t rawtime;\
		struct tm * timeinfo;\
		time(&rawtime);\
		timeinfo = localtime(&rawtime);\
		str=asctime(timeinfo);\
	}
#endif

#ifndef SAVE_PHI_IMAGE
#define SAVE_PHI_IMAGE(phi, sz0, sz1, sz2, filename) \
{\
    float min_val = phi[0], max_val = phi[0];\
	long tol_sz = sz0 * sz1 * sz2; \
    unsigned char * phi1d = new unsigned char[tol_sz];\
    for(long i = 0; i < tol_sz; i++) {if(phi[i] == INF) continue; min_val = MIN(min_val, phi[i]); max_val = MAX(max_val, phi[i]);}\
	cout<<"min_val = "<<min_val<<" max_val = "<<max_val<<endl;\
    max_val -= min_val; if(max_val == 0.0) max_val = 0.00001;\
    for(long i = 0; i < tol_sz; i++) phi1d[i] = phi[i] == INF ? 0.0 : (phi[i] - min_val)/max_val * 255 + 0.5;\
    V3DLONG phi_sz[4] = {sz0, sz1, sz2, 1};\
    saveImage((char*)((filename).c_str()), phi1d, phi_sz, V3D_UINT8);\
    delete [] phi1d; phi1d = 0;\
}
#endif

#ifndef SAVE_MASK_IMAGE
#define SAVE_MASK_IMAGE(mask, filename) \
{\
    unsigned char * outimg1d = 0;\
    V3DLONG voro_sz[4] = {sz0, sz1, sz2, 1}; V3DLONG * out_sz = 0;\
    convert_single_to_multi(voro_mask, voro_sz, outimg1d, out_sz);\
    saveImage((char*)((filename).c_str()), outimg1d, out_sz, V3D_UINT8);\
    delete [] outimg1d; outimg1d = 0;\ 
}
#endif

#ifndef RELEASE_MARKERS
#define RELEASE_MARKERS(inmarkers) \
{\
	for(int i = 0; i < inmarkers.size(); i++)\
	{\
		delete inmarkers[i];\
	}\
	inmarkers.clear();\
}
#endif

#ifndef DISPLAY_HISTGRAM
#define DISPLAY_HISTGRAM(indata, hist_num) \
{\
	double min_val = indata[0], max_val = indata[0];\
	for(int i = 0; i < indata.size(); i++)\
	{\
		min_val = MIN(indata[i], min_val);\
		max_val = MAX(indata[i], max_val);\
	}\
	\
	double step = (max_val-min_val)/hist_num;\
	vector<double> hist(hist_num, 0.0);\
	for(int i = 0; i < indata.size(); i++)\
	{\
		double val = indata[i];\
		int hist_id = (val - min_val)/step;\
		hist_id = MIN(hist_id, hist_num-1);\
		hist[hist_id]++;\
	}\
	for(int h = 0; h < hist_num; h++)\
	{\
		double hist_min = min_val + h * step;\
		double hist_max = hist_min + step;\
		int percent = hist[h] / (double)(indata.size()) * 100000 + 0.5;\
		cout<<hist_min<<" ~ "<<hist_max<<" "<<percent/1000.0<<"% ("<<hist[h]<<")\t"; cout.flush();\
		int n = percent/1000.0 + 0.5;\
		for(int i = 0; i < n; i++) {cout<<"=";cout.flush();}\
		cout<<endl;\
	}\
	hist.clear();\
}
#endif

#ifndef INIT_CLOCK
#define INIT_CLOCK  \
    static clock_t start, finish;
#endif

#ifndef START_CLOCK
#define START_CLOCK  \
{\
    start = clock();\
}
#endif

#ifndef END_CLOCK
#define END_CLOCK \
{\
    finish = clock(); \
    double totaltime = (double)(finish-start)/CLOCKS_PER_SEC;\
    cout<<"total time : "<<totaltime<<endl;\
}
#endif

#endif
