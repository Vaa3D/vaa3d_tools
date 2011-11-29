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

#ifndef CUBE_H_
#define CUBE_H_

#include <vector>
#include <string>

using namespace std;

template <class T>
class Cube
{
 public:

  Cube();
  Cube(T* data, long _depth, long _height, long _width);

  ~Cube();

  /** Calculates the derivative in the orders defined.*/
  void calculate_derivative ( int nx, int ny, int nz,
                              float sigma_x, float sigma_y, float sigma_z,
                              Cube<float>* output, Cube<float>* tmp);

  void convolve_depth(vector< float >& mask, Cube<float>* output, bool use_borders);

  void convolve_horizontally(vector< float >& mask, Cube< float >* output, bool use_borders);

  void convolve_vertically(vector< float >& mask, Cube<float>* output, bool use_borders);

  /** Returns the value of the voxel at indexes x,y,z.*/
  inline T at(int x, int y, int z);

  /** Changes the value of the voxel at indexes x,y,z.*/
  inline void put(int x, int y, int z, T value);

  static std::vector<float> gaussian_mask(int order, float sigma, bool limit_value);

  //Keeps pointers to the ordered (for fast indexing). it goes as voxels[z][y][x]
  T*** voxels;

  //Pointer to the volume data
  T* voxels_origin;

  string type;

  string directory;

  long width;
  long height;
  long depth;
};

#include "Cube.tpp"

#endif
