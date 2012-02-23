/*
 *  y_imglib.h
 * 
 *  image multiple threaded loading and visualizing header
 *  created Nov. 22, 2010 by Yang Yu
 *  yuy@janelia.hhmi.org
 *
 */

#ifndef __YIMGTHREAD_H__
#define __YIMGTHREAD_H__

#include <QtGui>
#include <QThread>
#include <QString>
#include <QFileInfo>

#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include "tiffio.h"
};

using namespace std;

//bool MUT_imgLoaded;

// Hanchuan raw
#define PHC_RAW_HEADER_SIZE_4BYTE 43
#define PHC_RAW_HEADER_SIZE_2BYTE 35

// following the rules from ITK .lsm reader
// Structure with LSM-specific data ( only in the first image directory).
#define TIF_CZ_LSMINFO 34412
#define TIF_CZ_LSMINFO_SIZE_RESERVED 90+6
#define TIF_CZ_LSMINFO_SIZE 512

typedef int			Int32_t;
typedef unsigned	UInt32_t;

typedef float       Float32_t;
typedef double      Float64_t;
typedef long double Float96_t;

typedef struct {
	UInt32_t    U32MagicNumber;
	Int32_t     S32StructureSize;
	Int32_t     S32DimensionX;
	Int32_t     S32DimensionY;
	Int32_t     S32DimensionZ;
	Int32_t     S32DimensionChannels;
	Int32_t     S32DimensionTime;
	Int32_t     S32DataType;
	Int32_t     S32ThumbnailX;
	Int32_t     S32ThumbnailY;
	Float64_t   F64VoxelSizeX;
	Float64_t   F64VoxelSizeY;
	Float64_t   F64VoxelSizeZ;
	UInt32_t    u32ScanType;
	UInt32_t    u32DataType;
	UInt32_t    u32OffsetVectorOverlay;
	UInt32_t    u32OffsetInputLut;
	UInt32_t    u32OffsetOutputLut;
	UInt32_t    u32OffsetChannelColors;
	Float64_t   F64TimeIntervall;
	UInt32_t    u32OffsetChannelDataTypes;
	UInt32_t    u32OffsetScanInformation;
	UInt32_t    u32OffsetKsData;
	UInt32_t    u32OffsetTimeStamps;
	UInt32_t    u32OffsetEventList;
	UInt32_t    u32OffsetRoi;
	UInt32_t    u32OffsetBleachRoi;
	UInt32_t    u32OffsetNextRecording;
	UInt32_t    u32Reserved [ TIF_CZ_LSMINFO_SIZE_RESERVED ];
} zeiss_info; // itkLSMImageIO.cxx

// define file formats
enum V3DFileFormatType {V3DRAW, V3DTIFF, V3DLSM, V3DSURFACEOBJ, V3DUNKNOWN};
enum V3DRawType {RAW4BYTE, RAW2BYTE, RAWUNKNOWN};

//
#define GREY   1   // 1-byte grey-level image or stack
#define GREY16 2   // 2-byte grey-level image or stack
#define COLOR  3   // 3-byte RGB image or stack

//
#ifndef V3DLONG

#if defined(_MSC_VER) && (_WIN64)
#define V3DLONG long long
#else
#define V3DLONG long
#endif

#endif

// Handling Hanchuan's raw files
// copied from stackutil.cpp
char checkMachineEndian()
{
    char e='N'; //for unknown endianness
	
    V3DLONG int a=0x44332211;
    unsigned char * p = (unsigned char *)&a;
    if ((*p==0x11) && (*(p+1)==0x22) && (*(p+2)==0x33) && (*(p+3)==0x44))
        e = 'L';
    else if ((*p==0x44) && (*(p+1)==0x33) && (*(p+2)==0x22) && (*(p+3)==0x11))
        e = 'B';
    else if ((*p==0x22) && (*(p+1)==0x11) && (*(p+2)==0x44) && (*(p+3)==0x33))
        e = 'M';
    else
        e = 'N';
	
    //printf("[%c] \n", e);
    return e;
}
// copied from stackutil.cpp
void swap2bytes(void *targetp)
{
    unsigned char * tp = (unsigned char *)targetp;
    unsigned char a = *tp;
    *tp = *(tp+1);
    *(tp+1) = a;
}
// copied from stackutil.cpp
void swap4bytes(void *targetp)
{
    unsigned char * tp = (unsigned char *)targetp;
    unsigned char a = *tp;
    *tp = *(tp+3);
    *(tp+3) = a;
    a = *(tp+1);
    *(tp+1) = *(tp+2);
    *(tp+2) = a;
}

// trying to catch exceptions
//bool readTIFFDirectory(TIFF *tif)
//{
//	try
//	{
//		TIFFReadDirectory(tif);
//	}
//	catch (...) 
//	{
//		printf("Invalid file format!\n");
//		return false;
//	}
//	
//	return true;
//}

// class multiple threaded loading
template <class Tidx, class Tdata>
class YY_ImgRead : public QThread
	{	
	public:
		
		YY_ImgRead(string image_name)
		{
			// init
			filename = image_name; nFrame = 0; nChannel = 0; 
			image_loaded = false;
			
			//MUT_imgLoaded = image_loaded;
			if(!QFile(QString(filename.c_str())).exists())
			{
				printf("This file does not exist!\n");
				return;
			}
			
			// 
			QString file_suffix = QFileInfo(QString(filename.c_str())).suffix().toUpper(); 
			
			if(file_suffix.compare("RAW") == 0)
			{
				tag_fileformat = V3DRAW;
			}
			else if(file_suffix.compare("TIF") == 0)
			{
				tag_fileformat = V3DTIFF;
			}
			else if(file_suffix.compare("LSM") == 0)
			{
				tag_fileformat = V3DLSM;
			}
			else if( (file_suffix.compare("ANO") == 0) || (file_suffix.compare("APO") == 0) || (file_suffix.compare("SWC") == 0) || (file_suffix.compare("MARKER") == 0) )
			{
				tag_fileformat = V3DSURFACEOBJ;
				printf("This file is the V3D surface object!\n");
				return;
			}
			else
			{
				tag_fileformat = V3DUNKNOWN;
				printf("This file format is not supported by default! Please try to open it using V3D bio-format plugin!\n");
				return;
			}
		}
		
		~YY_ImgRead(){}
		
	public:
		
		// define atom operation here
		void loadHeader();	// load image header info	
		int loadFrame(Tdata *&p);	// load a frame xy (cz)
		int loadFrameRGB(Tdata *&p); // load a frame xyc (z)
		void allocate(Tdata *&p);	// allocate space for the image pointer 
		void stop();	// stop
		
		void setTriviewControl(TriviewControl *triviewControl)
		{
			if(triviewControl)
			{
				tvControl = triviewControl;
				//tvControl->setPreValidZslice(0L); // init
			}
			else
			{
				printf("NULL pointer to triview window!\n");
				tvControl = NULL;
			}
		}
		
		void init()
		{
			gotXres = false; gotYres = false; gotZres = false;
			res_x = 1.0; res_y = 1.0; res_z = 1.0; 
			
			loadHeader(); // load header
			
			flag_xyc = (V3DLONG(dims_x)*V3DLONG(dims_y)*V3DLONG(dims_c)<(V3DLONG)1024*1024*3) ? true : false;
			
			stopped = false;
		}
		
		void init(Tdata *&pImg)
		{
			gotXres = false; gotYres = false; gotZres = false;
			res_x = 1.0; res_y = 1.0; res_z = 1.0; 
			
			loadHeader();
			allocate(pImg);
			
			p=pImg;
			
			flag_xyc = (V3DLONG(dims_x)*V3DLONG(dims_y)*V3DLONG(dims_c)<(V3DLONG)1024*1024*3) ? true : false;
			
			stopped = false;
		}
		
	public:
		string filename;
		Tidx dims_x, dims_y, dims_z, dims_c, dims_t; // 5D Image (XYZCT)
		Tidx datatype; // 1, 2, 4 for UINT8, UINT16, FLOAT32 respectively
		Tidx b_swap; // Endian
		
		volatile Tidx nFrame, nChannel; // semaphore
		
		Tidx sz_channel, sz_frame, sz_header;
		volatile bool image_loaded;
		
		Tdata *p;
		
		volatile bool stopped; // if true then abort. false by default 
		volatile bool flag_xyc; // is true when frame size is less than (1024*1024*3)
		
		bool gotXres, gotYres, gotZres;
		float res_x, res_y, res_z;
		
		V3DFileFormatType tag_fileformat; // 0 .raw 1 .tif 2 .lsm 3 .ano .apo .swc .marker 4 unknown
		V3DRawType tag_raw; // 0 4-byte 1 2-byte 3 unknown	
		
		TriviewControl *tvControl;
		
	private:
		QMutex mutex;
		
	protected:
		void run()
		{
			//qDebug()<<"load a frame: "<<nFrame<<" of channel:"<<nChannel;
			
			// loading
			mutex.lock();
			
			if(stopped)
			{
				stopped = false;
				mutex.unlock();
			}
			else
			{
				if(tag_fileformat==V3DRAW)
				{
					if(flag_xyc)
					{
						if(loadFrameRGB(p))
						{	
							stopped = true;
							mutex.unlock();
							return;
						}
					}
					else
					{
						if(loadFrame(p))
						{	
							stopped = true;
							mutex.unlock();
							return;
						}
					}
				}
				else
				{
					if(loadFrame(p))
					{	
						stopped = true;
						mutex.unlock();
						return;
					}
				}
				
				// setting z-slice
				if(tvControl)
				{
					tvControl->setValidZslice(nFrame);
					//tvControl->trigger(nFrame); //more than 2 threads
				}			
				
				//qDebug()<<"priority ... ... "<<this->priority();
				
				mutex.unlock();	
			}
			
			//		forever 
			//		{
			//			mutex.lock();
			//			
			//			if(tag_fileformat==V3DRAW)
			//			{
			//				if(flag_xyc)
			//				{
			//					loadFrameRGB(p);
			//				}
			//				else
			//				{
			//					loadFrame(p);
			//				}
			//			}
			//			else
			//			{
			//				loadFrame(p);
			//			}
			//			
			//			// setting z-slice
			//			if(tvControl)
			//			{
			//				qDebug()<<"emit signals ... ..."<<nFrame;
			//				tvControl->setValidZslice(nFrame);
			//				tvControl->trigger(nFrame);
			//			}
			//			else
			//				break;
			//			
			//			if (image_loaded || stopped)
			//				break;
			//			
			//			mutex.unlock();
			//			
			//			//QCoreApplication::processEvents();
			//			
			//		}
			
			//
			return;
		}
		
	};

// load header info
template <class Tidx, class Tdata> void YY_ImgRead<Tidx, Tdata> :: loadHeader()
{
	//
	if(tag_fileformat == V3DRAW) // .raw
	{
		// reference stackutil.cpp
		FILE * fid = fopen(filename.c_str(), "rb");
		if (!fid)
		{
			printf("Fail to open file for reading.\n");
			return;
		}
		
		fseek (fid, 0, SEEK_END);
		Tidx fileSize = ftell(fid);
		rewind(fid);
		
		// Read header
		char formatkey[] = "raw_image_stack_by_hpeng";
		int lenkey = strlen(formatkey);
		
		//#ifndef _MSC_VER
		if (fileSize<lenkey+2+4*4+1) // datatype has 2 bytes, and sz has 4*4 bytes and endian flag has 1 byte. 
		{
			printf("The size of your input file is too small and is not correct, -- it is too small to contain the legal header.\n");
			printf("The fseek-ftell produces a file size = %ld.", fileSize);
			return;
		}
		//		else if (fileSize<lenkey+2+4*2+1) // datatype has 2 bytes, and sz has 4*4 bytes and endian flag has 1 byte. 
		//		{
		//			printf("The size of your input file is too small and is not correct, -- it is too small to contain the legal header.\n");
		//			printf("The fseek-ftell produces a file size = %ld.", fileSize);
		//			return;
		//		}
		
		//#endif
		
		char * keyread = new char [lenkey+1];
		if (!keyread)
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		Tidx nread = fread(keyread, 1, lenkey, fid);
		if (nread!=lenkey)
		{
			printf("File unrecognized or corrupted file.\n");
			return;
		}
		keyread[lenkey] = '\0';
		
		if (strcmp(formatkey, keyread)) // is non-zero then the two strings are different
		{
			printf("Unrecognized file format.\n");
			if (keyread) {delete []keyread; keyread=0;}
			return;
		}
		
		char endianCodeData;
		fread(&endianCodeData, 1, 1, fid);
		if (endianCodeData!='B' && endianCodeData!='L')
		{
			printf("This program only supports big- or little- endian but not other format. Check your data endian.\n");
			if (keyread) {delete []keyread; keyread=0;}
			return;
		}
		
		char endianCodeMachine = checkMachineEndian();
		if (endianCodeMachine!='B' && endianCodeMachine!='L')
		{
			printf("This program only supports big- or little- endian but not other format. Check your data endian.\n");
			if (keyread) {delete []keyread; keyread=0;}
			return;
		}
		
		b_swap = (endianCodeMachine==endianCodeData)?0:1; // Endian
		
		short int dcode = 0;
		fread(&dcode, 2, 1, fid); 
		if (b_swap)
			swap2bytes((void *)&dcode);
		
		switch (dcode)
		{
			case 1:
				datatype = 1;
				break;
				
			case 2:
				datatype = 2;
				break;
				
			case 4:
				datatype = 4;
				break;
				
			default:
				printf("Unrecognized data type code [%d]. The file type is incorrect or this code is not supported in this version.\n", dcode);
				if (keyread) {delete []keyread; keyread=0;}
				return;
		}
		
		// try loading .raw as 4-byte data
		int mysz[4];
		int tmpn=fread(mysz, 4, 4, fid);
		if (tmpn!=4)
		{
			printf("This program only reads [%d] units.\n", tmpn);
			return;
		}
		if (b_swap)
		{
			for (Tidx i=0;i<4;i++)
				swap4bytes((void *)(mysz+i));
		}
		
		dims_x = mysz[0]; dims_y = mysz[1]; dims_z = mysz[2]; dims_c = mysz[3]; 
		
		Tidx totalUnit = dims_x * dims_y * dims_z * dims_c;
		
		tag_raw = RAW4BYTE;
		sz_header = PHC_RAW_HEADER_SIZE_4BYTE; // for Hanchuan's .raw 
		
		//#ifndef _MSC_VER
		if ((totalUnit*datatype+4*4+2+1+lenkey) != fileSize)
		{
			printf("The input file has a size [%ld bytes], different from what specified in the header [%ld bytes]. Now try to open .raw as 2-byte data.\n", fileSize, totalUnit*datatype+4*4+2+1+lenkey);
			
			// try loading .raw as 2-byte data
			fseek(fid, -16, SEEK_CUR);
			
			short int mysz[4];
			fread(mysz, 2, 4, fid); /* because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. */
			if (b_swap)
			{
				for (Tidx i=0;i<4;i++)
					swap2bytes((void *)(mysz+i));
			}
			
			dims_x = mysz[0]; dims_y = mysz[1]; dims_z = mysz[2]; dims_c = mysz[3]; 
			
			Tidx totalUnit = dims_x * dims_y * dims_z * dims_c;
			
			if (totalUnit*datatype+4*2+2+1+lenkey != fileSize)
			{
				tag_raw = RAWUNKNOWN;
				printf("The input file has a size [%ld bytes], different from what specified in the header [%ld bytes]. Exit.\n", fileSize, totalUnit*datatype+4*2+2+1+lenkey);
				if (keyread) {delete []keyread; keyread=0;}
				return;				
			}
			
			tag_raw = RAW2BYTE;
			sz_header = PHC_RAW_HEADER_SIZE_2BYTE; // for Hanchuan's .raw
			
		}
		//#endif
		//
		sz_frame = dims_x*dims_y;
		sz_channel = dims_x*dims_y*dims_z;
		
		//
		if (keyread) {delete []keyread; keyread=0;}
		fclose(fid);
	}
	else if(tag_fileformat == V3DTIFF) // .tif
	{
		TIFF  *tif=NULL;
		int   depth, width, height, kind;
		
		// dimensions
		TIFFSetWarningHandler(NULL);
		if( (tif = TIFFOpen(const_cast<char *>(filename.c_str()),"r")) == NULL )
		{
			printf("Could not open incoming image\n");
			return;
		}
		depth = 1;
		while (TIFFReadDirectory(tif))
			depth += 1;
		TIFFClose(tif);
		
		TIFFSetWarningHandler(NULL);
		tif = TIFFOpen(const_cast<char *>(filename.c_str()),"r");
		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
		
		short bits, channels, photo;
		
		TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bits);
		TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &channels);
		TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photo);
		
		if (photo <= 1)
		{ if (channels > 1)
			printf("Black and white tiff has more than 1 channel!\n");
			if (bits == 16)
			{
				kind = (GREY16);
				dims_c = 1;
				datatype = 2;
			}
			else
			{
				kind = (GREY);
				dims_c = 1;
				datatype = 1;
			}
			
		}
		else
		{
			kind = (COLOR);
			dims_c = 3;
			datatype = 1;
		}
		
		// resolutions
		float xres=1.0, yres=1.0;
		gotXres = TIFFGetField(tif, TIFFTAG_XRESOLUTION, &xres);
		gotYres = TIFFGetField(tif, TIFFTAG_YRESOLUTION, &yres);
		//TIFFGetField(tif, TIFFTAG_ZRESOLUTION, &res_z);
		
		uint16 res_unit;
		TIFFGetFieldDefaulted(tif, TIFFTAG_RESOLUTIONUNIT, &res_unit);
		
		if(res_unit == RESUNIT_CENTIMETER)
		{
			xres *= 25.4;
			yres *= 25.4;
		}
		
		if(gotXres || gotYres)
		{
			if(gotXres)
			{
				res_x = xres;
			}
			else
			{
				res_x = yres;
			}
			
			if(gotYres)
			{
				res_y = yres;
			}
			else 
			{
				res_y = xres;
			}
			
		}
		
		TIFFClose(tif);
		
		// assign
		dims_x = width; dims_y = height; dims_z = depth;
		
		//Tidx totalUnit = dims_x * dims_y * dims_z * dims_c;
		
		//
		sz_frame = dims_x*dims_y;
		sz_channel = dims_x*dims_y*dims_z;
		
	}
	else if(tag_fileformat == V3DLSM) // .lsm
	{
		TIFF  *tif=NULL;
		int   depth, width, height, kind;
		
		// 
		TIFFSetWarningHandler(NULL);
		if( (tif = TIFFOpen(const_cast<char *>(filename.c_str()),"r")) == NULL )
		{
			printf("Could not open incoming image\n");
			return;
		}
		
		depth = 1;
		while(TIFFReadDirectory(tif))
			depth += 1;
		TIFFClose(tif);
		depth = depth / 2;		/* half the dirs are thumbnails */
		
		TIFFSetWarningHandler(NULL);
		tif = TIFFOpen(const_cast<char *>(filename.c_str()),"r");
		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
		
		short bits, channels;
		
		TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bits);
		TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &channels);
		
		if (bits<=8 && bits>0) datatype=1;
		else if (bits<=16 && bits>0) datatype=2;
		else
		{
			printf("LSM file should not support more than 16 bits data.\n");
			return;
		}
		
		// resolutions
		ttag_t tag = TIF_CZ_LSMINFO;
		void *praw = NULL;
		short value_count = 512; // TIF_CZ_LSMINFO_SIZE
		const TIFFFieldInfo *fld = NULL;
		
		if( (fld = TIFFFieldWithTag( tif, tag )) == NULL )
		{
			printf("Fail in reading .lsm header info!\n");
			return;
		}
		else
		{
			if( fld->field_passcount )
			{
				if( TIFFGetField( tif, tag, &value_count, &praw ) != 1 )
				{
					printf("Tag of .lsm cannot be found!\n");
					return;
				}
				else
				{
					if( fld->field_type != TIFF_BYTE )
					{
						printf("Tag of .lsm is not of type TIFF_BYTE!\n");
						return;
					}
				}
			}
		}
		
		const zeiss_info *zi = reinterpret_cast<zeiss_info*>(praw);
		
		if( sizeof(*zi) != TIF_CZ_LSMINFO_SIZE)
		{
			printf("Problem of alignement of reading header of .lsm file on your platform.\n");
			return;
		}
		res_x = zi->F64VoxelSizeX;
		res_y = zi->F64VoxelSizeY;
		// TIFF only support 2 or 3 dimension:
		if ( dims_z > 1 )
		{
			res_z = zi->F64VoxelSizeZ;
		}
		
		//
		TIFFClose(tif);
		
		// assign
		dims_x = width; dims_y = height; dims_z = depth; dims_c = channels;
		
		//		dims_x = zi->S32DimensionX;
		//		dims_y = zi->S32DimensionY;
		//		dims_z = zi->S32DimensionZ;
		//		dims_c = zi->S32DimensionChannels;
		//		dims_t = zi->S32DimensionTime;
		//		datatype = zi->S32DataType;
		
		//Tidx totalUnit = dims_x * dims_y * dims_z * dims_c;
		
		//
		sz_frame = dims_x*dims_y;
		sz_channel = dims_x*dims_y*dims_z;
	}
	else
	{
		printf("This file format is not supported by default! Please try to open it using V3D bio-format plugin!\n");
		return;
	}
	
}

// load xy frame
template <class Tidx, class Tdata> int YY_ImgRead<Tidx, Tdata> :: loadFrame(Tdata *&p)
{
	
	// 
	if(tag_fileformat == V3DRAW) // .raw
	{
		// FILE 
		FILE * fid = fopen(filename.c_str(), "rb");
		if (!fid)
		{
			printf("Fail to open file for reading.\n");
			return -1 ;
		}
		
		// load frame
		Tidx offsets = 0;
		
		offsets += nChannel*sz_channel;
		offsets += nFrame*sz_frame;
		
		// overlook
		fseek (fid, sz_header+offsets*datatype, SEEK_SET);
		
		Tidx nread = fread(p+offsets, datatype, sz_frame, fid);
		if (nread!=sz_frame)
		{
			printf("Something wrong in file reading. The program reads [%ld data points] but the file says there should be [%ld data points].\n", nread, sz_frame);
			if (p) {delete []p; p=0;}
			return -1 ;
		}
		
		// swap if necessary
		if (b_swap==1)
		{
			if (datatype==2)
			{
				for (Tidx i=offsets;i<offsets+sz_frame; i++)
				{
					swap2bytes((void *)(p+i*datatype));
				}
			}
			else if (datatype==4)
			{
				for (Tidx i=offsets;i<offsets+sz_frame; i++)
				{
					swap4bytes((void *)(p+i*datatype));
				}
			}
		}
		
		//
		nChannel++;
		
		// load streaming order in XYZC
		//		if(nFrame>=dims_z)
		//		{
		//			nFrame = 0;
		//			nChannel++; 
		//			if(nChannel>=dims_c)
		//			{
		//				image_loaded = true;
		//				//MUT_imgLoaded = image_loaded;
		//				return -1 ;
		//			}
		//		}
		
		// load streaming order in XYCZ
		if(nChannel>=dims_c)
		{
			nChannel = 0;
			nFrame++; 
			if(nFrame>=dims_z)
			{
				image_loaded = true;
				//MUT_imgLoaded = image_loaded;
				return -1 ;
			}
		}
		
		// close
		fclose(fid);
		
	}
	else if(tag_fileformat == V3DTIFF) // .tif
	{
		if(!(datatype==1 || datatype==2))
		{
			printf("Invalid datatype from reading .tif file.\n");
			return -1 ;
		}
		else
		{
			//
			TIFF  *tif=NULL;
			
			TIFFSetWarningHandler(NULL);
			if( (tif = TIFFOpen(const_cast<char *>(filename.c_str()),"r")) == NULL )
			{
				printf("Could not open incoming image\n");
				return -1 ;
			}
			
			// skip already read part
			for(Tidx i=1; i<=nFrame; i++)
			{
				if (!TIFFReadDirectory(tif)) 
					return -1 ;
			}
			
			//TIFFSetDirectory(tif, nFrame);
			
			// load frame
			Tidx offsets = 0;
			
			Tidx sz_channel2 = 2*sz_channel;
			
			offsets += nChannel*sz_channel;
			offsets += nFrame*sz_frame;
			
			// read directory
			uint32 *raster;
			uint8  *row;
			
			try
			{
				raster = new uint32 [sz_frame];
			}
			catch(...)
			{
				printf("Error allocate memory for image!\n");
				return -1 ;
			}
			
			row = (uint8 *)( (Tdata *)(p+offsets));
			
			if (dims_c == 1 && datatype == 2)
			{ 
				int tile_width, tile_height;
				
				if (TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tile_width))    // File is tiled  
				{ 
					qDebug()<<"File is tiled";
					
					//
					Tidx x, y;
					Tidx i, j;
					Tidx m, n;
					uint16 *buffer = (uint16 *) raster;
					uint16 *out, *in, *rous;
					
					TIFFGetField(tif, TIFFTAG_TILELENGTH, &tile_height);
					
					for (y = 0; y < dims_y; y += tile_height)
					{ 
						if (y + tile_height > dims_y)
							n = dims_y - y;
						else
							n = tile_height;
						
						for (x = 0; x < dims_x; x += tile_width)
						{ 
							TIFFReadTile(tif, buffer, x, y, 0, 0);
							if (x + tile_width > dims_x)
								m = dims_x - x;
							else
								m = tile_width;
							
							for (j = 0; j < n; j++)
							{ 
								out = (uint16 *) (row + 2*(j*dims_x + x));
								in  = buffer + j*tile_width;
								for (i = 0; i < m; i++)
									*out++ = *in++; 
							}
						}
						row += n*dims_x*2;
					}
				}
				else    // File is striped
				{ 
					qDebug()<<"File is striped";
					
					//
					for (Tidx y = 0; y < dims_y; y++)
					{ 
						TIFFReadScanline(tif, row, y, 0);
						row += dims_x*2;
					}
				}
			}
			else
			{ 
				Tidx i, j;
				uint32 *in;
				uint8  *out;
				
				if (TIFFReadRGBAImage(tif,dims_x,dims_y,raster,0) == 0)
				{
					printf("Failed in reading .tif file.\n");
					return -1 ;
				}
				
				in = raster;
				if (dims_c == 1 && datatype == 1)
				{ 
					for (j = dims_y-1; j >= 0; j--)
					{ 
						out = row;
						for (i = 0; i < dims_x; i++)
						{ 
							uint32 pixel = *in++;
							*out++ = TIFFGetR(pixel);
						}
						row += dims_x;
					}
				}
				else if (dims_c == 3 && datatype == 1)
				{ 
					for (j = dims_y-1; j >= 0; j--)
					{ 
						out = row;
						for (i = 0; i < dims_x; i++)
						{ 
							uint32 pixel = *in++;
							*out = TIFFGetR(pixel);
							*(out+sz_channel) = TIFFGetG(pixel);
							*(out+sz_channel2) = TIFFGetB(pixel);
							
							out++;
						}
						row += dims_x;
					}
				}
				else
				{
					printf("This kind of tif we donot support!\n");
					return -1 ;
				}
			}
			
			// de-alloc
			if(raster) {delete []raster; raster=NULL;}
			
			//
			TIFFClose(tif);
			
			//
			nFrame++; 
			if(nFrame>=dims_z)
			{
				image_loaded = true;
				//MUT_imgLoaded = image_loaded;
				return -1 ;
			}
		}
		
	}
	else if(tag_fileformat == V3DLSM) // .lsm
	{
		TIFF  *tif=NULL;
		
		TIFFSetWarningHandler(NULL);
		if( (tif = TIFFOpen(const_cast<char *>(filename.c_str()),"r")) == NULL )
		{
			printf("Could not open incoming image\n");
			return -1 ;
		}
		
		//
		for(Tidx i=1; i<=nFrame; i++)
		{
			if (!TIFFReadDirectory(tif)) break;	  // skip the one we just read, it's a thumbnail 
			if (!TIFFReadDirectory(tif)) break;	  // get the next slice
		}
		
		// compute current offsets
		Tidx offsets = 0;
		
		offsets += nChannel*sz_channel;
		offsets += nFrame*sz_frame;
		
		// load frame
		if (TIFFIsTiled(tif))
		{
			// File is tiled
			//qDebug()<<"File is tiled";
			
			uint32 *bytecounts;
			TIFFGetField(tif, TIFFTAG_TILEBYTECOUNTS, &bytecounts);
			
			ttile_t t, nt = TIFFNumberOfTiles(tif);
			for (t = 0; t < nt; t++)
			{
				if(TIFFReadEncodedTile(tif, t, (unsigned char *)p + offsets*datatype + (Tidx)t*sz_channel, bytecounts[t])<0)
					return -1 ;
			}
		}
		else
		{
			// File is striped
			//qDebug()<<"File is striped";
			
			tstrip_t s, ns = TIFFNumberOfStrips(tif);
			for (s = 0; s < ns; s++)
			{
				if(TIFFReadEncodedStrip(tif, s, (unsigned char *)p + offsets*datatype + (Tidx)s*sz_channel*datatype, sz_frame*datatype)<0)
					continue;
			}
		}
		
		//
		TIFFClose(tif);
		
		//
		nFrame++; 
		if(nFrame>=dims_z)
		{
			image_loaded = true;
			//MUT_imgLoaded = image_loaded;
			return -1 ;
		}
		
	}
	else
	{
		printf("This file format is not supported by default! Please try to open it using V3D bio-format plugin!\n");
		return -1 ;
	}
	
	return 0;
	
}

// load xyc frame when frame size is less than (1024*1024*3)
template <class Tidx, class Tdata> int YY_ImgRead<Tidx, Tdata> :: loadFrameRGB(Tdata *&p)
{	
	// 
	if(tag_fileformat == V3DRAW) // .raw
	{
		// FILE 
		FILE * fid = fopen(filename.c_str(), "rb");
		if (!fid)
		{
			printf("Fail to open file for reading.\n");
			return -1 ;
		}
		
		// overlook the header
		// fseek (fid, sz_header, SEEK_SET);
		
		// load frame xyc
		nChannel = 0;
		while (nChannel<dims_c) 
		{
			Tidx offsets = 0;
			
			offsets += nChannel*sz_channel;
			offsets += nFrame*sz_frame;
			
			fseek (fid, sz_header+offsets*datatype, SEEK_SET);
			
			//
			Tidx nread = fread(p+offsets, datatype, sz_frame, fid);
			if (nread!=sz_frame)
			{
				printf("Something wrong in file reading. The program reads [%ld data points] but the file says there should be [%ld data points].\n", nread, sz_frame);
				if (p) {delete []p; p=0;}
				return -1 ;
			}
			
			// swap if necessary
			if (b_swap==1)
			{
				if (datatype==2)
				{
					for (Tidx i=offsets;i<offsets+sz_frame; i++)
					{
						swap2bytes((void *)(p+i*datatype));
					}
				}
				else if (datatype==4)
				{
					for (Tidx i=offsets;i<offsets+sz_frame; i++)
					{
						swap4bytes((void *)(p+i*datatype));
					}
				}
			}
			
			//
			nChannel++;
		}
		nChannel = 0;
		
		// load streaming order in XYCZ
		nFrame++; 
		if(nFrame>=dims_z)
		{
			image_loaded = true;
			//MUT_imgLoaded = image_loaded;
			return -1 ;
		}
		
		// close
		fclose(fid);
		
	}
	else if(tag_fileformat == V3DTIFF) // .tif
	{
		
	}
	else if(tag_fileformat == V3DLSM) // .lsm
	{
		
	}
	else
	{
		
	}
    
    return 0;
	
}

// allocate
template <class Tidx, class Tdata> void YY_ImgRead<Tidx, Tdata> :: allocate(Tdata *&p)
{
	// datatype UINT8 UINT16 FLOAT32
	
	Tidx nplxs = dims_x * dims_y * dims_z * dims_c; // dims_t = 1
	
	if(p) {delete []p; p=NULL;}
	else
	{
		try
		{
			p = new Tdata [nplxs];
			
			if(p)
			{
				memset(p, 0, sizeof(Tdata)*nplxs);
			}
		}
		catch(...)
		{
			printf("Error allocate memory for image!\n");
			return;
		}
	}
	
}

// stop func
template <class Tidx, class Tdata> void YY_ImgRead<Tidx, Tdata> :: stop()
{
	mutex.lock();
	stopped = true;
	mutex.unlock();
}


#endif

