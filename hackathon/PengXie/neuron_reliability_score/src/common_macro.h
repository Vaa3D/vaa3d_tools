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

#ifndef NTDIS
#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
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
							string outfile = parser.filelist.size() >=2 ? parser.filelist.at(1) : (basename(infile)+ "_out" + file_type(infile)); \
							unsigned char * indata1d = NULL, * outdata1d = NULL; \
                            V3DLONG* in_sz = 0, * out_sz = 0; int datatype = V3D_UINT8; int channel = 0;  
#endif

#ifndef LOAD_FIRST_IMAGE
#define LOAD_FIRST_IMAGE if(infile != "" && is_img_file(infile) && !my_load_image((char*) infile.c_str(), indata1d, in_sz, datatype)) {cout<<"Unable to load image"<<endl; return false;} if(infile != "" &&  indata1d && datatype != V3D_UINT8){cerr<<"Can not deal with 12bit or 16bit data."<<endl; return false;}
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
		if(!my_save_image((char*) outfile.c_str(), outdata1d, out_sz, datatype)) {cout<<"saveImage(\""<<outfile<<"\") error"; return false;} \
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
		if(!my_save_image((char*) outfile.c_str(), outdata1d, out_sz, datatype)) {cout<<"saveImage(\""<<outfile<<"\") error"; return false;} \
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
		if(!my_load_image((char*)infile2.c_str(), inimg2, in2_sz, datatype2)) {cout<<"unable to read image "<<infile2<<endl; return false;}
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

#ifndef SAVE_PHI_IMAGE                 // used in fastmarching related methods
#define SAVE_PHI_IMAGE \
{ \
	double min_val = INF - 1, max_val = 0; \
	for(long i = 1; i < tol_sz; i++)\
	{\
		if(phi[i] == INF) continue; \
		min_val = MIN(min_val, phi[i]);\
		max_val = MAX(max_val, phi[i]);\
	}\
	cout<<" min/max : "<<min_val<<"/"<<max_val<<" ";\
	min_val = 0, max_val = 586; \
	max_val -= min_val;\
	V3DLONG out_sz[4] = {sz0, sz1, sz2, 1};\
	string outfile = "phi" + num2str((int)process1,'0',3) + ".tif";\
	unsigned char * phiimg = new unsigned char[tol_sz];\
	for(long i = 0; i < tol_sz; i++) phiimg[i] = (phi[i] == INF) ? 0 : (phi[i] >= max_val ? 255 : (phi[i] - min_val)/max_val * 254.0 + 1.5);\
	my_save_image(outfile.c_str(), phiimg, out_sz, V3D_UINT8);\
	delete [] phiimg; phiimg = 0;\
}
#endif

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

#endif
