/////////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or       //
// modify it under the terms of the GNU General Public License         //
// version 2 as published by the Free Software Foundation.             //
//                                                                     //
// This program is distributed in the hope that it will be useful, but //
// WITHOUT ANY WARRANTY; without even the implied warranty of          //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   //
// General Public License for more details.                            //
//                                                                     //
// Written and (C) by German Gonzale and Aurelien Lucchi               //
// Contact <german.gonzale@epfl.ch> or <aurelien.lucchi@gmail.com>     //
// for comments & bug reports                                          //
/////////////////////////////////////////////////////////////////////////


#include <string>
#include <stdio.h>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <dirent.h>
#include <string>

#include "Cube.h"

using namespace std;

#ifndef UTILS_H
#define UTILS_H

//------------------------------------------------------------------------MACROS

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

//------------------------------------------------------------------------TYPES

typedef unsigned char uchar;

//---------------------------------------------------------------------FUNCTIONS

Cube<uchar>* createDiracCube(int nx, int ny, int nz);

/**
 * @param outputData memory is allocated inside the function
 */
void cubeFloat2Uchar(float* inputData, uchar*& outputData,
                     int nx, int ny, int nz);

bool isDirectory(string path);

int factorial_n(int n);

bool fileExists(const char* filename);
bool fileExists(string filename);

string getDerivativeName(int order_x, int order_y, int order_z);

string getDirectoryFromPath(string path);

string getNameFromPath(string path);

string getNameFromPathWithoutExtension(string path);

string getExtension(string path);

vector< double > readVectorDouble(string filename);

#endif //UTILS_H
