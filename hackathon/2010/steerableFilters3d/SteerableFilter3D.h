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

#ifndef STEERABLEFILTER3D_H_
#define STEERABLEFILTER3D_H_

//Units in radians


#include "Cube.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>

using namespace std;

class SteerableFilter3D
{

private:
  //Used to allocate a 3D array and return a pointer
  int*** malloc3Darray(int sz){
    int*** arr = (int***)malloc(sizeof(int**)*sz);
    for(int i = 0; i < sz; i++){
      arr[i] = (int**)malloc(sizeof(int*)*sz);
      for(int j = 0; j < sz; j++){
        arr[i][j] = (int*)malloc(sizeof(int)*sz);
      }
    }
    return arr;
  }

  // Used to translate from derivative indexes to linear indexes
  int*** o0ToIdx;
  int*** o1ToIdx;
  int*** o2ToIdx;
  int*** o3ToIdx;
  int*** o4ToIdx;
  int**** oToIdx;            //Use oToIdx[order-1][dx][dy][dz]


public:


  vector< float > coeffs;    //Original filter coefficients
  vector< float > r_coeffs;  //Rotated coefficients

  // Orientation of the filter
  double theta;
  double phi;

  // Order of the filter. M to stick with Unser's notation
  int M;

  // Scale of the filter in all the directions
  double sigma_x;
  double sigma_y;
  double sigma_z;

  // Cube in which the filter is being applied
  Cube< uchar>* cube;

  // Result of the filtering schema
  Cube< float>* result;

  // Vector with the derivatives of the cube
  vector< Cube< float >* > cubeDerivs;

  // Output name of the cube
  string outputName;

  string directory;

  // In case we want only odd or only even orders or both
  bool includeOddOrders;
  bool includeEvenOrders;
  bool includeOrder0;

  // Constructors
  SteerableFilter3D();

  SteerableFilter3D(unsigned char* cubeData, long cubeDepth, long cubeHeight, long cubeWidth,
                    string vectorName, double s_x, double s_y, double s_z);

  SteerableFilter3D(unsigned char* cubeData, long cubeDepth, long cubeHeight, long cubeWidth,
                    vector< double >& dcoeffs, double s_x, double s_y, double s_z);

  ~SteerableFilter3D();

  // Gets the structures for oToIdx and the coefficients
  void initialize();

  // Loads the volumes in cubeDerivs
  void loadVolume(string volumeName);

  void loadDerivatives();

  // So everything is in the tables of the constructor
  inline int deriv2idx(int dx, int dy, int dz){
    return oToIdx[dx+dy+dz][dx][dy][dz];
  }

  // Returns the derivatives in xyz of a given index of an order
  static void idx2deriv(int idx, int order,  int& dx, int& dy, int& dz);

  // Returns the number of derivatives for a given order
  static int numDerivsOrder(int order){
    if(order == 0) return 1;
    if(order == 1) return 3;
    if(order == 2) return 6;
    if(order == 3) return 10;
    if(order == 4) return 15;
    return 0;
  }

  // Returns the position in the linear feature vector of the beginning of the derivatives
  //   of a given order. Fixed to a maximum order of 4
  int positionOrder(int order){
    if( includeOddOrders && includeEvenOrders && !includeOrder0){
      if(order == 0) return 0;
      if(order == 1) return 0;
      if(order == 2) return 3;
      if(order == 3) return 9;
      if(order == 4) return 19;
      if(order == 5) return 34;
    }
    if( includeOddOrders && includeEvenOrders && includeOrder0){
      if(order == 0) return 0;
      if(order == 1) return 1;
      if(order == 2) return 4;
      if(order == 3) return 10;
      if(order == 4) return 20;
      if(order == 5) return 35;
    }
    if( includeOddOrders && !includeEvenOrders && !includeOrder0){
      if(order == 0) return 0;
      if(order == 1) return 0;
      if(order == 2) return 3;
      if(order == 3) return 3;
      if(order == 4) return 13;
      if(order == 5) return 13;
    }
    if( includeOddOrders && !includeEvenOrders && includeOrder0){
      if(order == 0) return 0;
      if(order == 1) return 1;
      if(order == 2) return 4;
      if(order == 3) return 4;
      if(order == 4) return 14;
      if(order == 5) return 14;
    }
    if( !includeOddOrders && includeEvenOrders && !includeOrder0){
      if(order == 0) return 0;
      if(order == 1) return 0;
      if(order == 2) return 0;
      if(order == 3) return 6;
      if(order == 4) return 6;
      if(order == 5) return 21;
    }
    if( !includeOddOrders && includeEvenOrders && includeOrder0){
      if(order == 0) return 0;
      if(order == 1) return 1;
      if(order == 2) return 1;
      if(order == 3) return 7;
      if(order == 4) return 7;
      if(order == 5) return 22;
    }
    return 0;
  }

  // Given the size of the feature vector, it returns the order of the filter and 
  //if there are only odd or even terms. It implements the following table:
  /***********************************************************************************

     Order     Odd&Even     Odd    Even
     1         3            3      X
     2         9            X      6
     3         19           13     X
     4         34           X      21

   **********************************************************************************/
  // and add one if g0 is included
  static void inferOrderAndOddEven
  (int nCoeffs, int& order,
   bool& _includeOddTerms, bool& _includeEvenTerms,
   bool& _includeOrder0)
  {
    switch (nCoeffs){
    case 3:
      _includeOddTerms  = true;
      _includeEvenTerms = false;
      _includeOrder0    = false;
      order = 1;
      break;
    case 4:
      _includeOddTerms  = true;
      _includeEvenTerms = false;
      _includeOrder0    = true;
      order = 1;
      break;
    case 6:
      _includeOddTerms  = false;
      _includeEvenTerms = true;
      _includeOrder0    = false;
      order = 2;
      break;
    case 7:
      _includeOddTerms  = false;
      _includeEvenTerms = true;
      _includeOrder0    = true;
      order = 2;
      break;
    case 9:
      _includeOddTerms  = true;
      _includeEvenTerms = true;
      _includeOrder0    = false;
      order = 2;
      break;
    case 10:
      _includeOddTerms  = true;
      _includeEvenTerms = true;
      _includeOrder0    = true;
      order = 2;
      break;
    case 13:
      _includeOddTerms  = true;
      _includeEvenTerms = false;
      _includeOrder0    = false;
      order = 3;
      break;
    case 14:
      _includeOddTerms  = true;
      _includeEvenTerms = false;
      _includeOrder0    = true;
      order = 3;
      break;
    case 19:
      _includeOddTerms  = true;
      _includeEvenTerms = true;
      _includeOrder0    = false;
      order = 3;
      break;
    case 20:
      _includeOddTerms  = true;
      _includeEvenTerms = true;
      _includeOrder0    = true;
      order = 3;
      break;
    case 21:
      _includeOddTerms  = false;
      _includeEvenTerms = true;
      _includeOrder0    = false;
      order = 4;
      break;
    case 22:
      _includeOddTerms  = false;
      _includeEvenTerms = true;
      _includeOrder0    = true;
      order = 4;
      break;
    case 34:
      _includeOddTerms  = true;
      _includeEvenTerms = true;
      _includeOrder0    = false;
      order = 4;
      break;
    case 35:
      _includeOddTerms  = true;
      _includeEvenTerms = true;
      _includeOrder0    = true;
      order = 4;
      break;
    default:
      printf("SteerableFilters3D has no idea of what is a feature vector of size %i, quiting ... \n", nCoeffs);
      exit(0);
    }
  }

  /*
  //Compatibility hack
  static void inferOrderAndOddEven
  (int nCoeffs, int& order,
   bool& _includeOddTerms, bool& _includeEvenTerms)
  {
    bool includeOrder0;
    inferOrderAndOddEven(nCoeffs, order, _includeOddTerms, _includeEvenTerms,
                         includeOrder0);

  }
*/

  // Puts in the result cube the filtered cube at orientation theta phi
  void filter(double theta, double phi);

  // Puts in the result cube the filtered cube at orientation theta phi
  void filter(double theta, double phi, double alpha);

  // Filters the cube with the orientation estimated at each voxel
  void filter(Cube< float >* theta, Cube<float >* phi);

  // Rotates the vector of coefficients of the filter
  void rotateCoefficientVector(double theta, double phi);

  // Roataes any coefficient vector
  vector< float > rotateVector(vector< float >& orig, double theta, double phi);

  // Applies a rotation in theta
  vector< float > rotateTheta(vector< float >& orig, double theta);

  // Applies a rotation in phi
  vector< float > rotatePhi(vector< float >& orig, double phi);

  // From a cloud, outputs the coordinates of it in Torch format
  void outputCoordinates(string trainingPointsCloud, string filename, bool rotated);

  // Returns the vector of coordinates at point [x,y,z]
  vector< float > getDerivativeCoordinates(int x, int y, int z);
};
#endif
