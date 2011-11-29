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

#include <algorithm>
#include <errno.h>
#include <limits.h>
#include <values.h>
#include <string.h>

#include "utils.h"

//---------------------------------------------------------------------FUNCTIONS

bool isDirectory(string path)
{
  bool test = false;
  DIR* dir;
  dir = opendir(path.c_str());
  if (dir) {
    test = true;
  }
  closedir(dir);
  return test;
}

bool fileExists(const char* filename)
{
  ifstream inp;
  inp.open(filename, ifstream::in);
  if(inp.fail()){
    inp.close();
    return false;
  }
  inp.close();
  return true;
}

bool fileExists(string filename)
{
  return fileExists(filename.c_str());
}

string getDirectoryFromPath(string path){
  size_t pos = path.find_last_of("/\\");
  if(pos == string::npos)
    return "./";
  else
    return path.substr(0,pos+1);
}

string getNameFromPath(string path){
  return path.substr(path.find_last_of("/\\")+1);
}

string getNameFromPathWithoutExtension(string path){
  string nameWith =  path.substr(path.find_last_of("/\\")+1);
  string nameWithout = nameWith.substr(0,nameWith.find_last_of("."));
  return nameWithout;
}

string getExtension(string path){
  return path.substr(path.find_last_of(".")+1);
}

int factorial_n(int n){
  int ret = 1;
  for(int i = 1; i <= n; i++)
    ret = ret*i;
  return ret;
}

vector< double > readVectorDouble(string filename){
  //assert(fileExists(filename));
  std::ifstream in(filename.c_str());
  if(!in.good())
    {
      printf("readVectorDouble::The file %s can not be opened\n",filename.c_str());
      exit(0);
    }
  vector< double > toReturn;
  string s;
  while(getline(in,s))
    {
      stringstream ss(s);
      double d;
      while(!ss.fail()){
        ss >> d;
        if(!ss.fail()){
          toReturn.push_back(d);
        }
      }
    }

  in.close();
  return toReturn;
}

Cube<uchar>* createDiracCube(int nx, int ny, int nz)
{
   Cube<uchar>* cube = new Cube<uchar>(0,nx,ny,nz);
   memset(cube->voxels_origin,0,nx*ny*nz*sizeof(uchar));
   cube->put(nx/2,ny/2,nz/2,255);
   return cube;
}

string getDerivativeName(int order_x, int order_y, int order_z)
{
  string ret = "";
  for(int i = 0; i < order_x; i++)
    ret = ret + "x";
  for(int i = 0; i < order_y; i++)
    ret = ret + "y";
  for(int i = 0; i < order_z; i++)
    ret = ret + "z";
  return ret;
}
