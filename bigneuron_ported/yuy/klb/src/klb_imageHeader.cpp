/*
* Copyright (C) 2014 by  Fernando Amat
* See license.txt for full license and copyright notice.
*
* Authors: Fernando Amat
*  klb_imageHeader.cpp
*
*  Created on: October 2nd, 2014
*      Author: Fernando Amat
*
* \brief Main image heade rutilities for klb format
*
*
*/

#include <limits>
#include "klb_imageHeader.h"



using namespace std;

klb_image_header& klb_image_header::operator=(const klb_image_header& p)
{
	if (this != &p)
	{

		memcpy(optimalBlockSizeInBytes, p.optimalBlockSizeInBytes, sizeof(uint32_t)* KLB_DATA_DIMS);
		setHeader(p.xyzct, p.dataType, p.pixelSize, p.blockSize, p.compressionType, p.metadata, p.headerVersion);
		
		
		resizeBlockOffset(p.Nb);
		memcpy(blockOffset, p.blockOffset, sizeof(uint64_t)* Nb);	
	}
	return *this;
}

//copy constructor
klb_image_header::klb_image_header(const klb_image_header& p)
{
	
	

	memcpy(optimalBlockSizeInBytes, p.optimalBlockSizeInBytes, sizeof(uint32_t)* KLB_DATA_DIMS);

	setHeader(p.xyzct, p.dataType, p.pixelSize, p.blockSize, p.compressionType, p.metadata, p.headerVersion);

	Nb = p.Nb;
	blockOffset = new std::uint64_t[Nb];
	memcpy(blockOffset, p.blockOffset, sizeof(uint64_t)* Nb);

}

klb_image_header::klb_image_header()
{
	uint32_t xyzct_[KLB_DATA_DIMS] = {0,0,0,0,0};

	Nb = 0;
	blockOffset = NULL;

	setHeader(xyzct_, KLB_DATA_TYPE::UINT16_TYPE);// default values

	//you still need to set xyzct, dataType, blockSize and blockOffset
}

klb_image_header::~klb_image_header()
{
	if (Nb != 0)
	{
		delete[] blockOffset;
		blockOffset = NULL;
	}
}

//==========================================
size_t klb_image_header::calculateNumBlocks() const
{
	size_t numBlocks = 1;

	for (int ii = 0; ii < KLB_DATA_DIMS; ii++)
		numBlocks *= (size_t)ceil( (float)(xyzct[ii]) / (float)(blockSize[ii]) );

	return numBlocks;
}

//======================================================
size_t klb_image_header::getBytesPerPixel() const
{
	switch (dataType)
	{
	case 0:
		return sizeof(std::uint8_t);
		break;
	case 1:
		return sizeof(std::uint16_t);
		break;
	case 2:
		return sizeof(std::uint32_t);
		break;
	case 3:
		return sizeof(std::uint64_t);
		break;
	case 4:
		return sizeof(std::int8_t);
		break;
	case 5:
		return sizeof(std::int16_t);
		break;
	case 6:
		return sizeof(std::int32_t);
		break;
	case 7:
		return sizeof(std::int64_t);
		break;
	case 8:
		return sizeof(float32_t);
		break;
	case 9:
		return sizeof(float64_t);
		break;
	default:
		return 0;
		break;
	}
}

//======================================================
uint32_t klb_image_header::getBlockSizeBytes() const
{
	uint32_t blockSizeTotal = 1;
	for (int ii = 0; ii < KLB_DATA_DIMS; ii++)
		blockSizeTotal *= blockSize[ii];

	return blockSizeTotal * getBytesPerPixel();
}

//======================================================
uint64_t klb_image_header::getImageSizeBytes() const
{	
	return getImageSizePixels() * getBytesPerPixel();
}
//========================================
uint64_t klb_image_header::getImageSizePixels() const
{
	uint64_t imgSize = 1;
	for (int ii = 0; ii < KLB_DATA_DIMS; ii++)
		imgSize *= xyzct[ii];

	return imgSize;
}
//==============================================================
void klb_image_header::writeHeader(std::ostream &fid)
{
	fid.write((char*)xyzct, sizeof(uint32_t)* KLB_DATA_DIMS); 
	fid.write((char*)pixelSize, sizeof(float32_t)* KLB_DATA_DIMS);
	fid.write((char*)(&dataType), sizeof(uint8_t));
	fid.write((char*)(&compressionType), sizeof(uint8_t));
	fid.write((char*)blockSize, sizeof(uint32_t)* KLB_DATA_DIMS);
	fid.write((char*)blockOffset, sizeof(uint64_t)* Nb);//this is the only variable size element
};

//==============================================================
void klb_image_header::writeHeader(FILE* fid)
{
	fwrite((char*)(&headerVersion), 1, sizeof(uint8_t), fid);
	fwrite((char*)xyzct,1, sizeof(uint32_t)* KLB_DATA_DIMS, fid);
	fwrite((char*)pixelSize, 1, sizeof(float32_t)* KLB_DATA_DIMS, fid);
	fwrite((char*)(&dataType), 1, sizeof(uint8_t), fid);
	fwrite((char*)(&compressionType), 1, sizeof(uint8_t), fid);
	fwrite(metadata, 1, sizeof(char) * KLB_METADATA_SIZE, fid);
	fwrite((char*)blockSize, 1, sizeof(uint32_t)* KLB_DATA_DIMS, fid);
	fwrite((char*)blockOffset, 1, sizeof(uint64_t)* Nb, fid);//this is the only variable size element
};

//=======================================================
void klb_image_header::readHeader(std::istream &fid)
{
	fid.read((char*)(&headerVersion), sizeof(uint8_t));
	fid.read((char*)xyzct, sizeof(uint32_t)* KLB_DATA_DIMS);
	fid.read((char*)pixelSize, sizeof(float32_t)* KLB_DATA_DIMS);
	fid.read((char*)(&dataType), sizeof(uint8_t));
	fid.read((char*)(&compressionType), sizeof(uint8_t));
	fid.read(metadata, sizeof(char) * KLB_METADATA_SIZE);
	fid.read((char*)blockSize, sizeof(uint32_t)* KLB_DATA_DIMS);

	
	//resize if necessary
	resizeBlockOffset(calculateNumBlocks());
	
	fid.read((char*)blockOffset, sizeof(uint64_t)* Nb);//this is the only variable size element
};

//======================================================
void klb_image_header::resizeBlockOffset(size_t Nb_)
{
	//resize if necessary	
	if (Nb != Nb_)
	{
		if (Nb != 0)
			delete[] blockOffset;
		Nb = Nb_;
		blockOffset = new std::uint64_t[Nb];
	}
}

//=======================================================
int klb_image_header::readHeader(const char *filename)
{
	ifstream fid(filename, ios::binary | ios::in);
	if (fid.is_open() == false)
	{
		cout << "ERROR: klb_image_header::readHeader : file " << filename << " could not be opened to read header" << endl;
		return 2;
	}

	readHeader(fid);
	fid.close();
	return 0;
};

//==========================================================
size_t  klb_image_header::getBlockCompressedSizeBytes(size_t blockIdx) const
{
	if (blockIdx >= Nb)
		return 0;
	else if (blockIdx == 0 )//first block
	{
		return blockOffset[blockIdx];
	}
	else{
		return blockOffset[blockIdx] - blockOffset[blockIdx - 1];
	}

}

//======================================================
std::uint64_t klb_image_header::getBlockOffset(size_t blockIdx) const
{
	if (blockIdx >= Nb)
		return numeric_limits<std::uint64_t>::max();
	else if (blockIdx == 0)//first block
	{
		return 0;
	}
	else{
		return blockOffset[blockIdx - 1];
	}
}

//======================================================
std::uint64_t klb_image_header::getCompressedFileSizeInBytes() const
{
	return getSizeInBytes() + blockOffset[Nb-1];
}

void klb_image_header::setDefaultBlockSize()
{
	std::uint32_t bytesPerPixel = getBytesPerPixel();	
	setOptimalBlockSizeInBytes();
	for (int ii = 0; ii < KLB_DATA_DIMS; ii++)
	{
		blockSize[ii] = optimalBlockSizeInBytes[ii] / bytesPerPixel;
		if (blockSize[ii] == 0)
		{
			blockSize[ii] = 1;
		}
	}
}

//======================================================
void klb_image_header::setHeader(const std::uint32_t xyzct_[KLB_DATA_DIMS], const KLB_DATA_TYPE dataType_, const float32_t pixelSize_[KLB_DATA_DIMS], const std::uint32_t blockSize_[KLB_DATA_DIMS], const KLB_COMPRESSION_TYPE compressionType_, const char metadata_[KLB_METADATA_SIZE], const std::uint8_t headerVersion_)
{
	memcpy(xyzct, xyzct_, sizeof(uint32_t)* KLB_DATA_DIMS);
	dataType = dataType_;
	headerVersion = headerVersion_;
	compressionType = compressionType_;

	if (pixelSize_ == NULL)
	{
		for (int ii = 0; ii < KLB_DATA_DIMS; ii++)
			pixelSize[ii] = 1.0f;//default value
	}
	else{
		memcpy(pixelSize, pixelSize_, sizeof(float32_t)* KLB_DATA_DIMS);
	}

	if (metadata_ == NULL)
	{
		memset(metadata, 0, sizeof(char)* KLB_METADATA_SIZE);
	}
	else{
		memcpy(metadata, metadata_, sizeof(char)* KLB_METADATA_SIZE);
	}

	if (blockSize_ == NULL)
	{
		setOptimalBlockSizeInBytes();
		std::uint32_t bytesPerPixel = getBytesPerPixel();
		for (int ii = 9; ii < KLB_DATA_DIMS; ii++)
		{	
			blockSize[ii] = optimalBlockSizeInBytes[ii] / bytesPerPixel;
		}
	}
	else{
		memcpy(blockSize, blockSize_, sizeof(uint32_t)* KLB_DATA_DIMS);
	}

}