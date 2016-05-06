#include "IPT.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>

void ReadIPT(const char* filename)
{
	IPTHeader Header;
	std::ifstream inFile(filename, std::ios::binary);

	if( !inFile.is_open() )
		std::cerr<<filename<<": File does not exist\n";

	inFile.read((char*) Header.MagicNum, 4);
	std::cout<<Header.MagicNum[0]<<Header.MagicNum[1]<<Header.MagicNum[2]<<Header.MagicNum[3]<<'\n';

	unsigned char res;
	if( Header.MagicNum[3] == 't' )
	{
		inFile.read((char*) &Header.Version,4);
		std::cout<<"Version: "<<Header.Version<<'\n';

		inFile.read((char*) &Header.width,4);
		std::cout<<"X-Size: "<<Header.width<<'\n';

		inFile.read((char*) &Header.height,4);
		std::cout<<"Y-Size: "<<Header.height<<'\n';

		inFile.read((char*) &Header.zSize,4);
		std::cout<<"Z-Size: "<<Header.zSize<<'\n';

		inFile.read((char*) &Header.tSize,4);
		std::cout<<"T-Size: "<<Header.tSize<<'\n';

		inFile.read((char*) &Header.BPP,4);
		std::cout<<"Byte Per Pixel: "<<Header.BPP<<'\n';

		inFile.read((char*) &Header.gray,4);
		std::cout<<"Number of gray levels: "<<Header.gray<<'\n';

		inFile.read((char*) &Header.Vwidth, 8);
		std::cout<<"Voxel width: "<<Header.Vwidth<<'\n';

		inFile.read((char*) &Header.Vheight, 8);
		std::cout<<"Voxel height: "<<Header.Vheight<<'\n';

		inFile.read((char*) &Header.Vthickness, 8);
		std::cout<<"Voxel thickness: "<<Header.Vthickness<<'\n';

		inFile.read((char*) &Header.VUx, 1);
		std::cout<<"Voxel Unit(X): "<<Header.VUx<<'\n';

		inFile.read((char*) &Header.VUy, 1);
		std::cout<<"Voxel Unit(Y): "<<Header.VUy<<'\n';

		inFile.read((char*) &Header.VUz, 1);
		std::cout<<"Voxel Unit(Z): "<<Header.VUz<<'\n';

		inFile.read((char*) &Header.dummy, 1);
		
		inFile.read((char*) &Header.CenterX, 8);
		std::cout<<"centroid X: "<<Header.CenterX<<'\n';

		inFile.read((char*) &Header.CenterY, 8);
		std::cout<<"centroid Y: "<<Header.CenterY<<'\n';

		inFile.read((char*) &Header.CenterZ, 8);
		std::cout<<"centroid Z: "<<Header.CenterZ<<'\n';

		inFile.read((char*) &Header.CenterT, 8);
		std::cout<<"centroid T: "<<Header.CenterT<<'\n';

		inFile.read((char*) &Header.amBBox_x1, 4);
		std::cout<<"am_BoundingBox_x1: "<<Header.amBBox_x1<<'\n';

		inFile.read((char*) &Header.amBBox_x2, 4);
		std::cout<<"am_BoundingBox_x2: "<<Header.amBBox_x2<<'\n';

		inFile.read((char*) &Header.amBBox_y1, 4);
		std::cout<<"am_BoundingBox_y1: "<<Header.amBBox_y1<<'\n';

		inFile.read((char*) &Header.amBBox_y2, 4);
		std::cout<<"am_BoundingBox_y2: "<<Header.amBBox_y2<<'\n';
		
		inFile.read((char*) &Header.amBBox_z1, 4);
		std::cout<<"am_BoundingBox_z1: "<<Header.amBBox_z1<<'\n';

		inFile.read((char*) &Header.amBBox_z2, 4);
		std::cout<<"am_BoundingBox_z2: "<<Header.amBBox_z2<<'\n';


		for(int i=0; i<4; i++)
		{
			for(int j=0; j<4; j++)
			{
				double tmp;
				inFile.read((char*) &tmp, sizeof(double) );
				Header.Tx[i][j] = tmp;
				std::cout<<Header.Tx[i][j]<<' ';
			}
			std::cout<<'\n';
		}
		for(int i=244; i<4096; i++)
			inFile.read((char*) &res, 1);
	}
	else
	{
		inFile.read((char*) &Header.width,4);
		std::cout<<"X-Size: "<<Header.width<<'\n';

		inFile.read((char*) &Header.height,4);
		std::cout<<"Y-Size: "<<Header.height<<'\n';

		inFile.read((char*) &Header.BPP,4);
		std::cout<<"Byte Per Pixel: "<<Header.BPP<<'\n';

		inFile.read((char*) &Header.gray,4);
		std::cout<<"Number of gray levels: "<<Header.gray<<'\n';

		inFile.read((char*) &Header.zSize,4);
		std::cout<<"Z-Size: "<<Header.zSize<<'\n';		
		
		inFile.read((char*) &Header.tSize,4);
		std::cout<<"T-Size: "<<Header.tSize<<'\n';
	}
	
	if( Header.MagicNum[3] == 'T' )
		for(int i=28; i<40; i++) inFile.read((char*)&res, 1);//read preserve, old ipt version
	if( Header.MagicNum[3] == '2' )
	{
		std::cout<<"Header length : 80\n";
		for(int i=28; i<80; i++) inFile.read((char*)&res, 1); //new ip2 version
	}
	if( Header.MagicNum[3] == '3' )
	{
		std::cout<<"Header length : 2048\n";
		for(int i=28; i<2048; i++) inFile.read((char*)&res, 1); //new ip3 version
	}

}

IPTHeader ReadIPT(std::ifstream& inFile)
{
	IPTHeader Header;

	//unsigned char MagicNum[4];
	inFile.read((char*) Header.MagicNum, 4);
	std::cout<<Header.MagicNum[0]<<Header.MagicNum[1]<<Header.MagicNum[2]<<Header.MagicNum[3]<<'\n';
	
	unsigned char res;

	if( Header.MagicNum[3] == 't' )
	{
		inFile.read((char*) &Header.Version,4);
		std::cout<<"Version: "<<Header.Version<<'\n';

		inFile.read((char*) &Header.width,4);
		std::cout<<"X-Size: "<<Header.width<<'\n';

		inFile.read((char*) &Header.height,4);
		std::cout<<"Y-Size: "<<Header.height<<'\n';

		inFile.read((char*) &Header.zSize,4);
		std::cout<<"Z-Size: "<<Header.zSize<<'\n';

		inFile.read((char*) &Header.tSize,4);
		std::cout<<"T-Size: "<<Header.tSize<<'\n';

		inFile.read((char*) &Header.BPP,4);
		std::cout<<"Byte Per Pixel: "<<Header.BPP<<'\n';

		inFile.read((char*) &Header.gray,4);
		std::cout<<"Number of gray levels: "<<Header.gray<<'\n';

		inFile.read((char*) &Header.Vwidth, 8);
		std::cout<<"Voxel width: "<<Header.Vwidth<<'\n';

		inFile.read((char*) &Header.Vheight, 8);
		std::cout<<"Voxel height: "<<Header.Vheight<<'\n';

		inFile.read((char*) &Header.Vthickness, 8);
		std::cout<<"Voxel thickness: "<<Header.Vthickness<<'\n';

		inFile.read((char*) &Header.VUx, 1);
		std::cout<<"Voxel Unit(X): "<<Header.VUx<<'\n';

		inFile.read((char*) &Header.VUy, 1);
		std::cout<<"Voxel Unit(Y): "<<Header.VUy<<'\n';

		inFile.read((char*) &Header.VUz, 1);
		std::cout<<"Voxel Unit(Z): "<<Header.VUz<<'\n';

		inFile.read((char*) &Header.dummy, 1);
		
		inFile.read((char*) &Header.CenterX, 8);
		std::cout<<"centroid X: "<<Header.CenterX<<'\n';

		inFile.read((char*) &Header.CenterY, 8);
		std::cout<<"centroid Y: "<<Header.CenterY<<'\n';

		inFile.read((char*) &Header.CenterZ, 8);
		std::cout<<"centroid Z: "<<Header.CenterZ<<'\n';

		inFile.read((char*) &Header.CenterT, 8);
		std::cout<<"centroid T: "<<Header.CenterT<<'\n';

		inFile.read((char*) &Header.amBBox_x1, 4);
		std::cout<<"am_BoundingBox_x1: "<<Header.amBBox_x1<<'\n';

		inFile.read((char*) &Header.amBBox_x2, 4);
		std::cout<<"am_BoundingBox_x2: "<<Header.amBBox_x2<<'\n';

		inFile.read((char*) &Header.amBBox_y1, 4);
		std::cout<<"am_BoundingBox_y1: "<<Header.amBBox_y1<<'\n';

		inFile.read((char*) &Header.amBBox_y2, 4);
		std::cout<<"am_BoundingBox_y2: "<<Header.amBBox_y2<<'\n';
		
		inFile.read((char*) &Header.amBBox_z1, 4);
		std::cout<<"am_BoundingBox_z1: "<<Header.amBBox_z1<<'\n';

		inFile.read((char*) &Header.amBBox_z2, 4);
		std::cout<<"am_BoundingBox_z2: "<<Header.amBBox_z2<<'\n';


		for(int i=0; i<4; i++)
		{
			for(int j=0; j<4; j++)
			{
				double tmp;
				inFile.read((char*) &tmp, sizeof(double) );
				Header.Tx[i][j] = tmp;
				std::cout<<Header.Tx[i][j]<<' ';
			}
			std::cout<<'\n';
		}
		for(int i=244; i<4096; i++)
			inFile.read((char*) &res, 1);
	}
	else
	{
		inFile.read((char*) &Header.width,4);
		std::cout<<"X-Size: "<<Header.width<<'\n';

		inFile.read((char*) &Header.height,4);
		std::cout<<"Y-Size: "<<Header.height<<'\n';

		inFile.read((char*) &Header.BPP,4);
		std::cout<<"Byte Per Pixel: "<<Header.BPP<<'\n';

		inFile.read((char*) &Header.gray,4);
		std::cout<<"Number of gray levels: "<<Header.gray<<'\n';

		inFile.read((char*) &Header.zSize,4);
		std::cout<<"Z-Size: "<<Header.zSize<<'\n';		
		
		inFile.read((char*) &Header.tSize,4);
		std::cout<<"T-Size: "<<Header.tSize<<'\n';
	}
	
	if( Header.MagicNum[3] == 'T' )
		for(int i=28; i<40; i++) inFile.read((char*)&res, 1);//read preserve, old ipt version
	else if( Header.MagicNum[3] == '2' )
	{
		std::cout<<"Header length : 80\n";
		for(int i=28; i<80; i++) inFile.read((char*)&res, 1); //new ip2 version
	}
	else if( Header.MagicNum[3] == '3' )
	{
		std::cout<<"Header length : 2048\n";
		for(int i=28; i<2048; i++) inFile.read((char*)&res, 1); //new ip3 version
	}
	
	return Header;
}
void WriteIPTHeader( std::ofstream& outFile, const IPTHeader& header)
{
	char Magic[5]=".IP3";
	outFile.write((const char*) Magic, 4);
	outFile.write((const char*) &header.width, 4);
	outFile.write((const char*) &header.height, 4);
	outFile.write((const char*) &header.BPP, 4);
	outFile.write((const char*) &header.gray, 4);
	outFile.write((const char*) &header.zSize, 4);
	outFile.write((const char*) &header.tSize, 4);

	char zero = 0;
	for(int i=28; i<2048; i++) outFile.write((const char*) &zero, 1); //old ipt version
	//for(int i=28; i<80; i++) outFile.write((const char*) &zero, 1);	//new ip2 version

}
void WriteIPTHeader( std::ofstream& outFile, const int width, const int height, const int BPP, const int gray, const int zSize, const int tSize)
{
	char Magic[5]=".IP3";
	outFile.write((const char*) Magic, 4);
	outFile.write((const char*) &width, 4);
	outFile.write((const char*) &height, 4);
	outFile.write((const char*) &BPP, 4);
	outFile.write((const char*) &gray, 4);
	outFile.write((const char*) &zSize, 4);
	outFile.write((const char*) &tSize, 4);

	char zero = 0;
	for(int i=28; i<2048; i++) outFile.write((const char*) &zero, 1);
	//for(int i=28; i<80; i++) outFile.write((const char*) &zero, 1);	//new ip2 version

}

void OutputNonZero3D( const char* out_filename, const IPTHeader& header, std::ifstream inFile)
{
	unsigned char* ImageBuf = new unsigned char [ header.tSize * header.zSize * header.width * header.height ];
	std::ofstream outFile(out_filename);

	std::vector<int> coordinate;
	for(int z=0; z<header.zSize; z++)
	{
		for(int y=0; y<header.height; y++)
		{
			for(int x=0; x<header.width; x++)
			{
				int index  = z * x * y + x * y + x; 
				if(  ImageBuf[ index ] ) 
				{
					coordinate.push_back(x);
					coordinate.push_back(y);
					coordinate.push_back(z);
				}
			}
		}
	}
	outFile<< ( coordinate.size() / 3 )<<std::endl;
	for(int i=0; i < coordinate.size(); i+=3)
		outFile<<coordinate[i]<<' '<<coordinate[i+1]<<' '<<coordinate[i+2]<<std::endl;
}

void PartialVolumeOut( std::ofstream& outFile, unsigned char* inImg, IPTHeader& inHeader, const int beg_z, const int end_z)
{
	IPTHeader header = inHeader;
	header.zSize = end_z - beg_z + 1;

	char Magic[5]=".IPT";
	outFile.write((const char*) Magic, 4);
	outFile.write((const char*) &header.width, 4);
	outFile.write((const char*) &header.height, 4);
	outFile.write((const char*) &header.BPP, 4);
	outFile.write((const char*) &header.gray, 4);
	outFile.write((const char*) &header.zSize, 4);
	outFile.write((const char*) &header.tSize, 4);

	char zero = 0;
	for(int i=28; i<40; i++) outFile.write((const char*) &zero, 1);

	unsigned char* outBuf = new unsigned char [ header.zSize * header.width * header.height ];
	for( int i=0; i<header.zSize * header.width * header.height; i++ ) 
		outBuf[i] = '0';
//	int skip = (beg_z-1) * header.width * header.height;
//	for(int i=0; i<header.zSize * header.width * header.height; i++)
//		outBuf[ i ] = inImg[  i ];
	memcpy( outBuf, inImg + (beg_z-1) * header.width * header.height,  header.zSize * header.width * header.height);

	outFile.write( (char*) outBuf, header.zSize * header.width * header.height);
}

void PartialVolumeOut( std::ofstream& outFile, unsigned char* inImg, IPTHeader& inHeader, const int beg_x, const int end_x, const int beg_y, const int end_y, const int beg_z, const int end_z)
{
	IPTHeader header = inHeader;
	header.zSize = end_z - beg_z + 1;
	header.height = end_y - beg_y + 1;
	header.width = end_x - beg_x + 1;

	char Magic[5]=".IPT";
	outFile.write((const char*) Magic, 4);
	outFile.write((const char*) &header.width, 4);
	outFile.write((const char*) &header.height, 4);
	outFile.write((const char*) &header.BPP, 4);
	outFile.write((const char*) &header.gray, 4);
	outFile.write((const char*) &header.zSize, 4);
	outFile.write((const char*) &header.tSize, 4);
	
	char zero = 0;
	for(int i=28; i<40; i++) outFile.write((const char*) &zero, 1);

	unsigned char* outBuf = new unsigned char [ header.zSize * header.width * header.height ];
	for( int i=0; i<header.zSize * header.width * header.height; i++ ) 
		outBuf[i] = 0;

	int pos = 0;
	for(int z=beg_z; z<=end_z; z++)
	{
		unsigned char* slice = new unsigned char [ inHeader.width * inHeader.height ];
		memcpy( slice, inImg + (z-1) * inHeader.width * inHeader.height, inHeader.width * inHeader.height );
		
		for(int i=0; i<inHeader.width * inHeader.height; i++)
		{
			int y = i / inHeader.width;
			int x = i % inHeader.width;

			if( y >= beg_y && y <= end_y && x >= beg_x && x <= end_x)
			{
				outBuf[pos] = slice[i];
				pos++;
			}
		}
	}

	outFile.write( (char*) outBuf, header.zSize * header.height * header.width );
}

void MergeIdx3IPT(unsigned char* R, unsigned char* G, unsigned char* B, const IPTHeader header, unsigned char* O)
{

	for(int i=0; i<header.width* header.height * header.zSize; i++)
	{
		if(B[i] > 0)
			O[i] = 255;
		else if(G[i] > 0)
			O[i] = 127;
		else if(R[i] > 0)
			O[i] = 63;
	}

}
