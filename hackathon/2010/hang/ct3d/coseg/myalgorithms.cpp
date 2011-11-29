//
//=======================================================================
// Copyright 2010 Institute PICB.
// Authors: Hang Xiao, Axel Mosig
// Data : July 11, 2010
//=======================================================================
//
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include "tiffio.h"
#include "myalgorithms.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifndef WIN32
	#include <sys/time.h>
#else
	#include <time.h>
#endif
using namespace std;

//======================================================================================
//bucketSort : sort the input array and return their positions in value descending order
//======================================================================================

vector<int> bucketSort(unsigned char* img, int _size)
{
	//1. pos store the position
	//   data store the higher order of img
	//   for example 34562, will continuly changes to 3456, 345, 34, 3
	vector<int> pos;
	vector<unsigned char> data;
	pos.resize(_size);
	data.resize(_size);
	for(int i=0;i<_size;i++)
	{
		pos[i] = i;
		data[i] = img[i];
	}
	//2. get the bucks
	// for the 10 bits data, the bucks is 10
	// the maximize of unsigned char is 256, we just use 
	// 3 loop of ordering
	int bucks = 10;        // 0 1 2 3 .. 9
	int n = 3;             // the length maximum num 
	vector< vector<int> > buckets;
	buckets.resize(bucks);
	
	for(int loop =1; loop <= n; loop++)
	{
		for(int i = 0 ; i < _size;i++)
		{
			int whichBuck =data[pos[i]] - data[pos[i]]/bucks *bucks;
			buckets[whichBuck].push_back(pos[i]);
			data[pos[i]] = data[pos[i]]/bucks;
		}
		//put the num in buckets back to pos
		int k=0;
		for(int j = bucks-1; j >= 0; j--)
		{
			vector<int>::iterator it;
			for(it = buckets[j].begin(); it != buckets[j].end();it++)
			{
				pos[k++] = *it;
			}
			buckets[j].clear();
		}
	}
	return pos;
}

vector<int> bucketSort(vector<int> &data)
{
	//1. pos store the position
	//   data store the higher order of img
	//   for example 34562, will continuly changes to 3456, 345, 34, 3
	int _size = data.size();
	vector<int> order;
	order.resize(_size);
	for(int i=0;i<_size;i++)
	{
		order[i] = i;
	}
	//2. get the bucks
	// for the 10 bits data, the bucks is 10
	// the maximize of unsigned char is 256, we just use 
	// 3 loop of ordering
	int bucks = 10;        // 0 1 2 3 .. 9
	int n = 10;             // the length of 4294967296 
	vector< vector<int> > buckets;
	buckets.resize(bucks);
	
	for(int loop =1; loop <= n; loop++)
	{
		for(int i = 0 ; i < _size;i++)
		{
			int whichBuck =data[order[i]] - data[order[i]]/bucks *bucks;
			buckets[whichBuck].push_back(order[i]);
			data[order[i]] = data[order[i]]/bucks;
		}
		//put the num in buckets back to order
		int k=0;
		for(int j = bucks-1; j >= 0; j--)
		{
			vector<int>::iterator it;
			for(it = buckets[j].begin(); it != buckets[j].end();it++)
			{
				order[k++] = *it;
			}
			buckets[j].clear();
		}
	}
	return order;
}


//======================================================================================
// neighbor : get the 6-connected neighbor of current point
//======================================================================================
int createVertex(int x, int y, int z, int width, int height, int depth)
{
    return (z*width*height+y*width+x);
}
vector<int> neighbor(int point, int width, int height, int depth)
{
    vector<int> pointArray;
    int z = point/(width*height);
    int y = (point - z*width*height)/width;
    int x = point - z*width*height - y*width;
	
    if(x >= 1)pointArray.push_back(createVertex(x-1,y,z,width,height,depth));
    if(x + 1 < width)pointArray.push_back(createVertex(x+1,y,z,width,height,depth));
    if(y >= 1)pointArray.push_back(createVertex(x,y-1,z,width,height,depth));
    if(y + 1 < height)pointArray.push_back(createVertex(x,y+1,z,width,height,depth));
    if(z >= 1)pointArray.push_back(createVertex(x,y,z-1,width,height,depth));
    if(z + 1 < depth)pointArray.push_back(createVertex(x,y,z+1,width,height,depth));
    
    return pointArray;
}

//======================================================================================
// split : split the original string to many small string, the return is the num of splits
//======================================================================================
int split(char *original, char ** splits)
{
	int i = 0;
	int j = 0;
	char* str = new char[strlen(original)];
	while (original[i] != '\0')
	{
		if(original[i] == ' ' || original[i] == '\t')
		{
			while (original[i] == ' ' || original[i] == '\t')
			{
				
				str[i] = '\0';
				i++;
				
			}
			if(original[i] != '\0') 
			{
				str[i] = original[i];
				splits[j++] = &(str[i]); 
			}
		}
		else
		{
			if(i == 0) splits[j++] = &(str[0]);
			str[i] = original[i];
		}
		i++;
	}
	str[i] = '\0';
	return j;
}

//===========================================
double ostu_thresh(vector<double> data)
//===========================================
{
	sort(data.begin(),data.end());
	
	double w1,  w2;
	double mu1, mu2;
	map<double,int> theta_b;
	int t_size = data.size() - 1;

	for(int t = 0; t < t_size; t++)
	{
		w1 = (data[t]+data[t+1])/2 - data[0];
		w2 = data[t_size] - (data[t]+data[t+1])/2;
		mu1 = 0.0;
		mu2 = 0.0;
		for(int i = 0; i <= t; i++)
		{
				mu1 += data[i];
		}
		mu1 = mu1/(t+1);
		for(int j = t+1; j <= t_size; j++)
		{
				mu2 += data[j];
		}	
		mu2 = mu2/(t_size -t);

		double b = w1*w2*(mu1-mu2)*(mu1-mu2);

		theta_b[b] = t ;
	}
	int max_id = (*(theta_b.rbegin())).second;
	return (data[max_id]+data[max_id+1])/2;
}

//================================================================================
unsigned char *readtiff(char *myfile, int *width, int *height, int *depth, int *channels )
//================================================================================

{
    TIFF* tif = TIFFOpen(myfile, "r");
    if (tif)
    {
		*depth = 0;
		do
		{
			(*depth)++;
			uint16 compression, cc, bpp         ;
			
			//check if we are reading a compressed image. If so
			//return with an error
			TIFFGetField(tif, TIFFTAG_COMPRESSION, &compression);
			
			if ( compression != COMPRESSION_NONE )
			{
				cerr<<"File is not an Uncompressed TIFF image !\n";
			}
			
			//check if the image has 8 bits-per-pixel in each channel
			TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bpp);
			
			if ( bpp != 8 )
			{
				cerr<<"File does not have 8 bits per channel !\n";
			}
			
			
			//read the dimensions of the TIFF image
			TIFFGetField(tif, TIFFTAG_IMAGEWIDTH , width);//width
			TIFFGetField(tif, TIFFTAG_IMAGELENGTH, height);//height
			//determine what type of image is it (color or grayscale)
			TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &cc);
			
			//get the photometric if a grayscale image is
			//being read
			if (cc == 1)
			{
				*channels = 1;
			}
			else
			{
				*channels = 3;
			}
		}while(TIFFReadDirectory(tif));
		
		TIFFClose(tif);
	}
	else
    {
        cerr<<"Unable to open file for reading !\n";
    }
	
	
	tif = TIFFOpen(myfile,"r");
	if(tif)
	{
		unsigned char *img = new unsigned char[(*channels)*(*width)*(*height)*(*depth)];
		int layer = 0;
		do
		{			
			uint16 p, compression, cc, bpp;
			
			//check if we are reading a compressed image. If so
			//return with an error
			TIFFGetField(tif, TIFFTAG_COMPRESSION, &compression);
			
			if ( compression != COMPRESSION_NONE )
			{
				cerr<<"File is not an Uncompressed TIFF image !\n";
			}
			
			//check if the image has 8 bits-per-pixel in each channel
			TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bpp);
			
			if ( bpp != 8 )
			{
				cerr<<"File does not have 8 bits per channel !\n";
			}
			
			
			//read the dimensions of the TIFF image
			TIFFGetField(tif, TIFFTAG_IMAGEWIDTH , width);//width
			TIFFGetField(tif, TIFFTAG_IMAGELENGTH, height);//height
			//determine what type of image is it (color or grayscale)
			TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &cc);
			
			//get the photometric if a grayscale image is
			//being read
			if (cc == 1)
			{
				*channels = 1;
				TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &p);
			}
			else
			{
				*channels = 3;
			}
			
			//create the matrix to hold the image data
			
			//create a dummy array
			int linebytes = (*channels)*(*width);
			unsigned char *buf = new unsigned char[linebytes];
			
			//start reading the data from the TIFF file in scanlines
			for ( int row = 0; row < *height; row++ )
			{
				if(TIFFReadScanline(tif, buf, row, 0) != 1)
				{
					cerr<<"Error reading TIFF scanline !\n";
				}
				memcpy(&img[layer*(*channels)*(*width)*(*height)+row*linebytes], buf, linebytes);
				
			}
			
			delete(buf);
		
			layer++;
		} while(TIFFReadDirectory(tif));
			
        TIFFClose(tif);
		
		return img;
    }
	
	return NULL;
}

//===================================================================================================
void writetiff( char *myfile, unsigned char *img, int channels, int width, int height, int depth )
//===================================================================================================
{
    TIFF* tif = TIFFOpen(myfile, "w");
    if (tif)
    {
		for(int layerIndex = 0; layerIndex < depth ; layerIndex++)
		{
			//set tiff tags to generate an uncompressed TIFF image
			//SETTAGS( tif, width, height, channels );
			//dimensions of the image
			TIFFSetField(tif, TIFFTAG_IMAGEWIDTH,  width);
			TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
			
			//starting from top-left corner
			TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
			
			//grayscale or color image
			TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, channels);
			TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
			TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
			if (channels == 1)
				TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
			else
				if (channels == 3)
					TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
			
			//Miscalleneous Tags
			TIFFSetField( tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
			TIFFSetField( tif, TIFFTAG_COMPRESSION , COMPRESSION_NONE );
			TIFFSetField( tif, TIFFTAG_ROWSPERSTRIP,
						 TIFFDefaultStripSize(tif , (uint32) -1) );
			TIFFSetField(tif, TIFFTAG_XRESOLUTION, 72);
			TIFFSetField(tif, TIFFTAG_YRESOLUTION, 72);
			TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
			
			//create a dummy array
			int linebytes = channels*width;
			unsigned char *buf = new unsigned char[linebytes];
			TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, linebytes));
			
			//start writing the data into the TIFF file in scanlines
			for ( int row = 0; row < height; row++)
			{
				//check if a grayscale or color image is being written
				memcpy(buf, &img[layerIndex*width*height*channels+row*linebytes], linebytes);
				
				if (TIFFWriteScanline(tif, buf, row, 0) < 0)
				{
					cerr<<"Error in scanline while writing !\n";
				}
			}
			delete(buf);
			TIFFWriteDirectory(tif);
		}
        TIFFClose(tif);
    }
    else
    {
        cerr<<"Unable to open file for writing !\n";
    }
}

void writeValue(ofstream &ofs, int v)
{
    if(ofs.good())
        ofs.write((char*)&v, sizeof(int));
    else
    {
        cerr<<"write int value error."<<endl;
        exit(-1);
    }
}

void writeValue(ofstream &ofs, unsigned short v)
{
    if(ofs.good())
        ofs.write((char*)&v, sizeof(unsigned short));
    else
    {
        cerr<<"write unsigned short value error."<<endl;
        exit(-1);
    }
}

void writeValue(ofstream &ofs, short v)
{
    if(ofs.good())
        ofs.write((char*)&v, sizeof(short));
    else
    {
        cerr<<"write short value error."<<endl;
        exit(-1);
    }
}

void writeValue(ofstream &ofs, double v)
{
    if(ofs.good())
        ofs.write((char*)&v, sizeof(double));
    else
    {
        cerr<<"write double error."<<endl;
        exit(-1);
    }
}

void writeValue(ofstream &ofs, float v)
{
    if(ofs.good())
        ofs.write((char*)&v, sizeof(float));
    else
    {
        cerr<<"write float error."<<endl;
        exit(-1);
    }
}

void readValue(ifstream &ifs, int& v)
{
    if(ifs.good())
    {
        ifs.read((char*)&v, sizeof(int));
        return;
    }
    else
    {
        cerr<<"read int value error"<<endl;
        exit(-1);
    }
}

void readValue(ifstream &ifs, unsigned short& v)
{
    if(ifs.good())
    {
        ifs.read((char*)&v, sizeof(unsigned short));
        return;
    }
    else
    {
        cerr<<"read unsigned short value error"<<endl;
        exit(-1);
    }
}

void readValue(ifstream &ifs, short& v)
{
    if(ifs.good())
    {
        ifs.read((char*)&v, sizeof(short));
        return;
    }
    else
    {
        cerr<<"read short value error"<<endl;
        exit(-1);
    }
}

void readValue(ifstream &ifs, double& v)
{
    if(ifs.good())
    {
        ifs.read((char*)&v, sizeof(double));
        return;
    }
    else
    {
        cerr<<"read double value error"<<endl;
        exit(-1);
    }
}

void readValue(ifstream &ifs, float& v)
{
    if(ifs.good())
    {
        ifs.read((char*)&v, sizeof(float));
        return;
    }
    else
    {
        cerr<<"read float value error"<<endl;
        exit(-1);
    }
}

#ifndef WIN32
	struct timeval tv;
	unsigned long startTime, crtTime, prevTime;
	unsigned long phaseCompletionTime, elapsedTime;
#else
	DWORD   startTime, crtTime, prevTime;
	DWORD   phaseCompletionTime, elapsedTime;
#endif
void SetStartTime()
{
#ifdef WIN32
	startTime = GetTickCount();
#else
	gettimeofday(&tv, NULL);
	startTime = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
#endif
	prevTime = startTime;
	return;
}

void PrintElapsedTime(const char* message)
{
#ifdef WIN32
	crtTime = GetTickCount();
#else
	gettimeofday(&tv, NULL);
	crtTime = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
#endif
	phaseCompletionTime = crtTime - prevTime;
	elapsedTime = crtTime - startTime;
	prevTime = crtTime;

	if(message == NULL || strlen(message) == 0) {
		printf("Total time: %d ms.\n", elapsedTime);
	}
	else {
		if(strcmp(message, " ") == 0) {
			printf("completed in: %d ms. Total time: %d ms.\n", phaseCompletionTime, elapsedTime);
		}
		else {
			printf("%s\n"
					"\tcompleted in: %d ms. Total time: %d ms.\n", message, phaseCompletionTime, elapsedTime);
		}
	}
	return;
}
