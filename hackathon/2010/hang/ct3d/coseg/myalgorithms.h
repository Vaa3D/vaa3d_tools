//
//=======================================================================
// Copyright 2010 Institute PICB.
// Authors: Hang Xiao, Axel Mosig
// Data : July 13, 2010
//=======================================================================
//
#ifndef MYALGORITHMS_H_H
#define MYALGORITHMS_H_H

#include <vector>
#include <fstream>
using namespace std;

//#define writeValue(ofs, v) (ofs).write((char*)&(v), sizeof(v))
//#define readValue(ifs, v)  (ifs).read((char*)&(v), sizeof(v))

vector<int> bucketSort(unsigned char* img, int _size);

vector<int> bucketSort(vector<int> &data);

vector<int> neighbor(int point, int width, int height, int depth);

int split(char *original, char ** splits);

double ostu_thresh(vector<double> data);


unsigned char* readtiff(char *myfile, int *width, int *height, int *depth,
						int *channels);
void writetiff( char *myfile, unsigned char *img, int channels,\
			   int width, int height, int depth );

void writeValue(ofstream &ofs, int v);
void writeValue(ofstream &ofs, unsigned short v);
void writeValue(ofstream &ofs, short v);
void writeValue(ofstream &ofs, float v);
void writeValue(ofstream &ofs, double v);

void readValue(ifstream &ifs, int &v);
void readValue(ifstream &ifs, unsigned short &v);
void readValue(ifstream &ifs, short &v);
void readValue(ifstream &ifs, float &v);
void readValue(ifstream &ifs, double &v);
void SetStartTime();
void PrintElapsedTime(const char* message = NULL);
#endif
