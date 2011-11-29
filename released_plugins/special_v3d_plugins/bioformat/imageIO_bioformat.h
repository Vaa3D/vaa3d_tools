/* imageIO_bioformat.h
 * 2010-06-01: create this program by Yang Yu
 */

#ifndef __IMAGEIO_BIOFORMAT_H__
#define __IMAGEIO_BIOFORMAT_H__

#include "v3d_basicdatatype.h"

//EXAMPLES USING BIOFORMAT
// for Bio-Formats C++ bindings
#include "bio-formats.h"
#include "loci-common.h"
//exception
using jace::JNIException;
using jace::proxy::java::lang::Exception;
using jace::proxy::java::io::IOException;
using jace::proxy::loci::common::Log;
//read & write
using jace::proxy::java::lang::Object;
using jace::proxy::java::lang::String;
using jace::proxy::java::util::Hashtable;
using jace::proxy::loci::common::XMLTools;
using jace::proxy::loci::formats::ChannelFiller;
using jace::proxy::loci::formats::ChannelMerger;
using jace::proxy::loci::formats::ChannelSeparator;
using jace::proxy::loci::formats::DimensionSwapper;
using jace::proxy::loci::formats::FilePattern;
using jace::proxy::loci::formats::FileStitcher;
using jace::proxy::loci::formats::FormatException;
using jace::proxy::loci::formats::FormatTools;
using jace::proxy::loci::formats::IFormatHandler;
using jace::proxy::loci::formats::IFormatReader;
using jace::proxy::loci::formats::ImageReader;
using jace::proxy::loci::formats::MetadataTools;
using jace::proxy::loci::formats::meta::IMetadata;
using jace::proxy::loci::formats::meta::MetadataRetrieve;
using jace::proxy::loci::formats::meta::MetadataStore;
using jace::proxy::loci::formats::tools::StatusEchoer;
using jace::proxy::java::lang::Boolean;
using jace::proxy::java::lang::Integer;
using jace::proxy::loci::formats::ImageWriter;

#include <iostream>
using std::cout;
using std::endl;

#include <exception>
using std::exception;

#include <string>
using std::string;

// for atoi on some platforms
#include <stdio.h>
#include <stdlib.h>

#include <vector>
using std::vector;

#if defined (_MSC_VER)
#define PATHSEP string(";")
#else
#define PATHSEP string(":")
#endif

// Initializes the Java virtual machine.
void createJVM() {
  cout << "Creating JVM... ";
  jace::StaticVmLoader loader(JNI_VERSION_1_4);
  jace::OptionList list;
  list.push_back(jace::ClassPath("jace-runtime.jar" + PATHSEP + "bio-formats.jar" + PATHSEP + "loci_tools.jar"));
  list.push_back(jace::CustomOption("-Xcheck:jni"));
  list.push_back(jace::CustomOption("-Xmx512m"));
  list.push_back(jace::CustomOption("-Djava.awt.headless=true"));
  //list.push_back(jace::CustomOption("-verbose:jni"));
  jace::helper::createVm(loader, list, false);
  cout << "JVM created." << endl;
}

// agent image class
class IMAGE 
{
public:
	IMAGE(V3DLONG sizeX, V3DLONG sizeY, V3DLONG sizeZ, V3DLONG sizeC, V3DLONG sizeT, ImagePixelType v3dComponentType, string SeriesName)
	{
		//init
		data1d_8 = 0;
		data1d_16 = 0;
		data1d_32 = 0;
		
		//
		sx = sizeX;
		sy = sizeY;
		sz = sizeZ;
		sc = sizeC;
		st = sizeT;
		v3dDataType = v3dComponentType;
		seriesname = SeriesName;
		
		V3DLONG N = sizeX*sizeY*sizeZ*sizeC*sizeT; //
		
		if(v3dComponentType == V3D_UINT8)
		{
			try
			{
				data1d_8 = new unsigned char[N];
			}
			catch (...) 
			{
				printf("Fail to allocate memory.\n");
				return;
			}
		}
		else if (v3dComponentType == V3D_UINT16)
		{
			try
			{
				data1d_16 = new short int [N];
			}
			catch (...) 
			{
				printf("Fail to allocate memory.\n");
				return;
			}
		}
		else if (v3dComponentType == V3D_FLOAT32)
		{
			try
			{
				data1d_32 = new float [N];
			}
			catch (...) 
			{
				printf("Fail to allocate memory.\n");
				return;
			}
		}
		
		
	}
	~IMAGE(){}
	
public:
	void clear()
	{
		if(data1d_8) {delete []data1d_8; data1d_8=0;}
		if(data1d_16) {delete []data1d_16; data1d_16=0;}
		if(data1d_32) {delete []data1d_32; data1d_32=0;}
	}
	
public:
	string seriesname;
	
	ImagePixelType v3dDataType;
	unsigned char* data1d_8;
	short int* data1d_16;
	float* data1d_32;
	
	V3DLONG sx, sy, sz, sc, st;
};

#endif

