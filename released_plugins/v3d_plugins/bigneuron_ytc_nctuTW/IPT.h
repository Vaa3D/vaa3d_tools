/*
P.C. Lee cis dept. NCTU
*/
#ifndef _IPT_H
#define _IPT_H
#include <fstream>

typedef struct _IPT
{
	char MagicNum[4];
	int width, height, BPP, gray, zSize, tSize;
	int Version;
	double Vwidth, Vheight, Vthickness;
	unsigned char VUx, VUy, VUz, dummy;
	double CenterX, CenterY, CenterZ, CenterT;
	float amBBox_x1, amBBox_x2, amBBox_y1, amBBox_y2, amBBox_z1, amBBox_z2;
	
	double Tx[4][4];

}IPTHeader;

void ReadIPT(const char* filename);
IPTHeader ReadIPT(std::ifstream& inFile);
void WriteIPTHeader( std::ofstream& outFile, const IPTHeader& header);
void WriteIPTHeader( std::ofstream& outFile, const int width, const int height, const int BPP, const int gray, const int zSize, const int tSize);
void OutputNonZero3D( const char* out_filename, const IPTHeader& header, std::ifstream inFile);
void PartialVolumeOut( std::ofstream& outFile, unsigned char* inImg, IPTHeader& inHeader, const int beg_z, const int end_z);
void PartialVolumeOut( std::ofstream& outFile, unsigned char* inImg, IPTHeader& inHeader, const int beg_x, const int end_x, const int beg_y, const int end_y, const int beg_z, const int end_z);
void MergeIdx3IPT(unsigned char* R, unsigned char* G, unsigned char* B, const IPTHeader header, unsigned char* O);
#endif 