#ifndef __KLB_IMAGE_COMMON_H__
#define __KLB_IMAGE_COMMON_H__

typedef float  float32_t;
typedef double float64_t;

#define KLB_DATA_DIMS (5) //our images at the most have 5 dimensions: x,y,z, c, t
#define KLB_METADATA_SIZE (256) //number of bytes in metadata
#define KLB_DEFAULT_HEADER_VERSION (2) //def

// Following mylib conventions here are the data types
enum KLB_DATA_TYPE
{
	UINT8_TYPE = 0,
	UINT16_TYPE = 1,
	UINT32_TYPE = 2,
	UINT64_TYPE = 3,
	INT8_TYPE = 4,
	INT16_TYPE = 5,
	INT32_TYPE = 6,
	INT64_TYPE = 7,
	FLOAT32_TYPE = 8,
	FLOAT64_TYPE = 9
};

//Compression type look up table (add to the list if you use a different one)
//To add more compression types just add it here and look for 
enum KLB_COMPRESSION_TYPE
{
	NONE = 0,
	BZIP2 = 1,
	ZLIB = 2
};

#endif