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

#include "SteerableFilter3D.h"
#include "utils.h"

#include <math.h>
#include <omp.h>

SteerableFilter3D::SteerableFilter3D(){
  initialize();
}

SteerableFilter3D::SteerableFilter3D(unsigned char* cubeData, long cubeDepth, long cubeHeight, long cubeWidth,
                                     string vectorName, double s_x, double s_y, double s_z)
{
  sigma_x = s_x;
  sigma_y = s_y;
  sigma_z = s_z;

  vector< double > dcoeffs = readVectorDouble(vectorName);
  inferOrderAndOddEven(dcoeffs.size(), this->M, this->includeOddOrders,
                       this->includeEvenOrders, this->includeOrder0);
  initialize();

  //printf("[SteerableFilter3D] Loading cube (%d,%d,%d)\n", cubeDepth,cubeHeight,cubeWidth);
  cube = new Cube< uchar>(cubeData,cubeDepth,cubeHeight,cubeWidth);
  result = new Cube<float>(0,cubeDepth,cubeHeight,cubeWidth);

  //printf("[SteerableFilter3D] Loading derivatives\n");
  loadDerivatives();

  //printf("[SteerableFilter3D] Loading %d coefficients\n", (int)dcoeffs.size());
  for(int i = 0; i < (int)dcoeffs.size(); i++){
    //printf("[SteerableFilter3D] dcoeffs[i] %f\n", dcoeffs[i]);
    coeffs.push_back(dcoeffs[i]);
    r_coeffs.push_back(dcoeffs[i]);
  }
}

SteerableFilter3D::SteerableFilter3D(unsigned char* cubeData, long cubeDepth, long cubeHeight, long cubeWidth,
                                     vector< double >& dcoeffs, double s_x, double s_y, double s_z)
{
  sigma_x = s_x;
  sigma_y = s_y;
  sigma_z = s_z;

  inferOrderAndOddEven(dcoeffs.size(), this->M, this->includeOddOrders,
                       this->includeEvenOrders, this->includeOrder0);
  initialize();

  //printf("[SteerableFilter3D] Loading cube (%d,%d,%d)\n", cubeDepth,cubeHeight,cubeWidth);
  cube = new Cube< uchar>(cubeData,cubeDepth,cubeHeight,cubeWidth);
  result = new Cube<float>(0,cubeDepth,cubeHeight,cubeWidth);

  //printf("[SteerableFilter3D] Loading derivatives\n");
  loadDerivatives();

  //printf("[SteerableFilter3D] Loading %d coefficients\n", (int)dcoeffs.size());
  for(int i = 0; i < (int)dcoeffs.size(); i++){
    //printf("[SteerableFilter3D] dcoeffs[i] %f\n", dcoeffs[i]);
    coeffs.push_back(dcoeffs[i]);
    r_coeffs.push_back(dcoeffs[i]);
  }
}

SteerableFilter3D::~SteerableFilter3D()
{
   // todo : delete oToIdx

   if(cube)
      delete cube;
   if(result)
      delete result;

   for(vector< Cube< float >* >::iterator itDerivs = cubeDerivs.begin();
      itDerivs != cubeDerivs.end(); itDerivs++)
   {
      delete *itDerivs;
   }
}

void SteerableFilter3D::initialize()
{
    oToIdx = (int****)malloc(5*sizeof(int***));

    o0ToIdx = malloc3Darray(1);
    o1ToIdx = malloc3Darray(2);
    o2ToIdx = malloc3Darray(3);
    o3ToIdx = malloc3Darray(4);
    o4ToIdx = malloc3Darray(5);
    oToIdx[0] = o0ToIdx;
    oToIdx[1] = o1ToIdx;
    oToIdx[2] = o2ToIdx;
    oToIdx[3] = o3ToIdx;
    oToIdx[4] = o4ToIdx;

    //Order 0
    o0ToIdx[0][0][0] = 0;

    //Order 1
    o1ToIdx[1][0][0] = 0;
    o1ToIdx[0][1][0] = 1;
    o1ToIdx[0][0][1] = 2;

    //Order 2
    o2ToIdx[2][0][0] = 0;
    o2ToIdx[1][1][0] = 1;
    o2ToIdx[1][0][1] = 2;
    o2ToIdx[0][2][0] = 3;
    o2ToIdx[0][1][1] = 4;
    o2ToIdx[0][0][2] = 5;

    //Order 3
    o3ToIdx[3][0][0] = 0;
    o3ToIdx[2][1][0] = 1;
    o3ToIdx[2][0][1] = 2;
    o3ToIdx[1][2][0] = 3;
    o3ToIdx[0][3][0] = 4;
    o3ToIdx[0][2][1] = 5;
    o3ToIdx[1][0][2] = 6;
    o3ToIdx[0][1][2] = 7;
    o3ToIdx[0][0][3] = 8;
    o3ToIdx[1][1][1] = 9;

    //Order 4
    o4ToIdx[4][0][0] = 0;
    o4ToIdx[3][1][0] = 1;
    o4ToIdx[3][0][1] = 2;
    o4ToIdx[2][2][0] = 3;
    o4ToIdx[2][1][1] = 4;
    o4ToIdx[2][0][2] = 5;
    o4ToIdx[1][3][0] = 6;
    o4ToIdx[0][4][0] = 7;
    o4ToIdx[0][3][1] = 8;
    o4ToIdx[1][2][1] = 9;
    o4ToIdx[0][2][2] = 10;
    o4ToIdx[1][0][3] = 11;
    o4ToIdx[0][1][3] = 12;
    o4ToIdx[0][0][4] = 13;
    o4ToIdx[1][1][2] = 14;

    theta = 0;
    phi = 0;

    directory = "";
}

void SteerableFilter3D::loadDerivatives()
{
  bool tmp_initialized = false;
  Cube< float >* tmp = 0;

  //Now loads the derivatives
  int m,n,p,idx,ord;
  for(ord = 0; ord <=M; ord++){
    if((ord == 0)&&(!includeOrder0))
      continue;
    if((ord%2 == 0)&&(!includeEvenOrders))
      continue;
    if((ord%2 == 1)&&(!includeOddOrders))
      continue;
    for(idx = 0; idx < numDerivsOrder(ord); idx++){
      idx2deriv(idx, ord, m, n, p);
     if(!tmp_initialized){
       tmp_initialized = true;
       tmp = new Cube<float>(0,cube->depth,cube->height,cube->width);
     }
     Cube< float>* deriv = new Cube<float>(0,cube->depth,cube->height,cube->width);
     cube->calculate_derivative(m, n, p, sigma_x, sigma_y, sigma_z,  deriv, tmp);
     cubeDerivs.push_back(deriv);
    }
  }

  if(tmp)
     delete[] tmp;
}

void SteerableFilter3D::idx2deriv
(int idx, int order, int& dx, int& dy, int& dz)
{
  //As the variables are static, they should be initializated only once.
  static int id0ToDevs[1][3] = { {0, 0, 0}  };

  static int id1ToDevs[3][3] = { {1, 0, 0},
                                 {0, 1, 0},
                                 {0, 0, 1} };
  
  static int id2ToDevs[6][3] = { {2, 0, 0},
                                 {1, 1, 0},
                                 {1, 0, 1},
                                 {0, 2, 0},
                                 {0, 1, 1},
                                 {0, 0, 2} };

  static int id3ToDevs[10][3] = { {3, 0, 0},
                                  {2, 1, 0},
                                  {2, 0, 1},
                                  {1, 2, 0},
                                  {0, 3, 0},
                                  {0, 2, 1},
                                  {1, 0, 2},
                                  {0, 1, 2},
                                  {0, 0, 3},
                                  {1, 1, 1}};

  static int id4ToDevs[15][3] = { {4,0,0},
                                  {3,1,0},
                                  {3,0,1},
                                  {2,2,0},
                                  {2,1,1},
                                  {2,0,2},
                                  {1,3,0},
                                  {0,4,0},
                                  {0,3,1},
                                  {1,2,1},
                                  {0,2,2},
                                  {1,0,3},
                                  {0,1,3},
                                  {0,0,4},
                                  {1,1,2} };
  if(order == 0){
    dx = id0ToDevs[idx][0];
    dy = id0ToDevs[idx][1];
    dz = id0ToDevs[idx][2];
  }

  if(order == 1){
    dx = id1ToDevs[idx][0];
    dy = id1ToDevs[idx][1];
    dz = id1ToDevs[idx][2];
  }
  if(order == 2){
    dx = id2ToDevs[idx][0];
    dy = id2ToDevs[idx][1];
    dz = id2ToDevs[idx][2];
  }
  if(order == 3){
    dx = id3ToDevs[idx][0];
    dy = id3ToDevs[idx][1];
    dz = id3ToDevs[idx][2];
  }
  if(order == 4){
    dx = id4ToDevs[idx][0];
    dy = id4ToDevs[idx][1];
    dz = id4ToDevs[idx][2];
  }
}


void SteerableFilter3D::rotateCoefficientVector(double theta, double phi)
{
  r_coeffs = rotateVector(coeffs, theta, phi);
}


vector< float > SteerableFilter3D::rotateTheta
(vector< float >& orig, double theta)
{
  vector< float > toRet(orig.size());

  double tmp1, tmp2, tmp3, ct, st, sp;
  int m, n, p, i, j, k, l, q, b_v, ord, idx;

  //In Aguet's formula the phi is defined in the opposite sense of the clock (towards x negative),
  // from there the inversion of phi
  phi = 0;
  ct = cos(theta);
  // cp = cos(-phi);
  st = sin(theta);
  sp = sin(-phi);

  //For each order and each component of the order
  for(ord = 0; ord <=M; ord++){
    if( (ord ==0) && !includeOrder0)
      continue;
    if( (ord ==0) && includeOrder0){ //Order 0 is not rotated (as it is already symmetrical)
      toRet[0] = orig[0];
    }
    if( (ord%2==0) && !includeEvenOrders)
      continue;
    if( (ord%2==1) && !includeOddOrders)
      continue;
    b_v = positionOrder(ord);
    for(idx = 0; idx < numDerivsOrder(ord); idx++){
      idx2deriv(idx, ord, m, n, p);
      for(i=0;i<=m;i++)
        for(k=0;k<=n;k++)
          for(q=0;q<=p;q++)
            for(j=0;j<=i;j++)
              for(l=0;l<=k;l++){
                if(j+l+p-q != 0)
                  continue;
                tmp1 = factorial_n(m)*factorial_n(n)*factorial_n(p)*pow(-1.0,i-j+p-q);
                tmp2 = factorial_n(m-i)*factorial_n(i-j)*factorial_n(j)*factorial_n(n-k)*
                  factorial_n(k-l)*factorial_n(l)*factorial_n(p-q)*factorial_n(q);
                tmp3 = ((double)tmp1)/tmp2;
                tmp3 = tmp3*pow(ct,m-i+j+k-l);
                tmp3 = tmp3*pow(st,i-j+n-k+l);
                // tmp3 = tmp3*pow(cp,m-i+n-k+q);  as phi = 0 cp = 1
                tmp3 = tmp3*pow(sp,j+l+p-q);    //only if j+l+p-q is equal to 0
                toRet[b_v + deriv2idx(m-i+n-k+p-q, i-j+k-l, j+l+q)] +=
                  tmp3*orig[b_v + deriv2idx(m,n,p)];
              }
    }
  }
  return toRet;
}


vector< float > SteerableFilter3D::rotatePhi
(vector< float >& orig, double phi)
{
  vector< float > toRet(orig.size());

  double tmp1, tmp2, tmp3, cp, sp;
  int m, n, p, i, j, k, l, q, b_v, ord, idx;

  //In Aguet's formula the phi is defined in the opposite sense of the clock (towards x negative),
  // from there the inversion of phi
  // ct = cos(theta);
  cp = cos(-phi);
  // st = sin(theta);
  sp = sin(-phi);

  //For each order and each component of the order
  for(ord = 0; ord <=M; ord++){
    if( (ord ==0) && !includeOrder0)
      continue;
    if( (ord ==0) && includeOrder0){
      toRet[0] = orig[0];
    }
    if( (ord%2==0) && !includeEvenOrders)
      continue;
    if( (ord%2==1) && !includeOddOrders)
      continue;
    b_v = positionOrder(ord);
    for(idx = 0; idx < numDerivsOrder(ord); idx++){
      idx2deriv(idx, ord, m, n, p);
      for(i=0;i<=m;i++)
        for(k=0;k<=n;k++)
          for(q=0;q<=p;q++)
            for(j=0;j<=i;j++)
              for(l=0;l<=k;l++){
                if(i-j+n-k+l != 0)
                  continue;
                tmp1 = factorial_n(m)*factorial_n(n)*factorial_n(p)*pow(-1.0,i-j+p-q);
                tmp2 = factorial_n(m-i)*factorial_n(i-j)*factorial_n(j)*factorial_n(n-k)*
                  factorial_n(k-l)*factorial_n(l)*factorial_n(p-q)*factorial_n(q);
                tmp3 = ((double)tmp1)/tmp2;
                // tmp3 = tmp3*pow(ct,m-i+j+k-l); //theta = 90 -> ct = 1
                // tmp3 = tmp3*pow(st,i-j+n-k+l); // taken into account in the if above
                tmp3 = tmp3*pow(cp,m-i+n-k+q);
                tmp3 = tmp3*pow(sp,j+l+p-q);
                toRet[b_v + deriv2idx(m-i+n-k+p-q, i-j+k-l, j+l+q)] +=
                  tmp3*orig[b_v + deriv2idx(m,n,p)];
              }
    }
  }
  return toRet;
}



vector< float > SteerableFilter3D::rotateVector
(vector< float >& orig, double theta, double phi)
{
  vector< float > tmp = rotatePhi(orig, phi);
  return rotateTheta(tmp, theta);
}

void SteerableFilter3D::filter(double theta, double phi)
{
  // rotateCoefficientVector(theta, phi);
  vector< float > tmp = rotateVector(coeffs, 0, phi);
  r_coeffs = rotateVector(tmp, theta,0);

  /*
  printf("Coefficients\n");
  for(int i = 0; i < (int)r_coeffs.size(); i++){
    std::cout << coeffs[i] << " ";
  }
  std::cout << std::endl;
  printf("Rotated coefficients\n");
  for(int i = 0; i < (int)r_coeffs.size(); i++){
    std::cout << r_coeffs[i] << " ";
  }
  std::cout << std::endl;
  */

#ifdef WITH_OPENMP
#pragma omp parallel for
#endif
  for(int z = 0; z < cube->depth; z++){
    float res = 0;
    for(int y = 0; y < cube->height; y++)
      for(int x = 0; x < cube->width; x++){
        res = 0;
        for(int i = 0; i < (int)coeffs.size(); i++)
          res += r_coeffs[i]*cubeDerivs[i]->at(x,y,z);
        result->put(x,y,z,res);
      }
    }
}


void SteerableFilter3D::filter(double theta, double phi, double alpha)
{
  r_coeffs = rotateTheta(coeffs, alpha);
  vector< float > tmp = rotatePhi(r_coeffs, phi);
  r_coeffs = rotateTheta(tmp, theta);

  std::cout << "coeffs\n";
  for(int i = 0; i < (int)coeffs.size(); i++){
     std::cout << i << ":" << coeffs[i] << " ";
  }
  std::cout << std::endl;
  for(int i = 0; i < (int)r_coeffs.size(); i++){
    std::cout << i << ":" << r_coeffs[i] << " ";
  }
  std::cout << std::endl;

  std::cout << "includeOddOrders: " << includeOddOrders << std::endl;
  std::cout << "includeEvenOrders: " << includeEvenOrders << std::endl;
  std::cout << "includeOrder0: " << includeOrder0 << std::endl;

#ifdef WITH_OPENMP
#pragma omp parallel for
#endif
  for(int z = 0; z < cube->depth; z++){
    float res = 0;
    for(int y = 0; y < cube->height; y++)
      for(int x = 0; x < cube->width; x++){
        res = 0;
        for(int i = 0; i < (int)coeffs.size(); i++)
          res += r_coeffs[i]*cubeDerivs[i]->at(x,y,z);
        result->put(x,y,z,res);
      }
    }
}


void SteerableFilter3D::filter
(Cube<float>* theta,
 Cube<float>* phi)
{
  int nthreads = omp_get_max_threads();

  vector< vector< float > > coefficientsRotated(nthreads);

#ifdef WITH_OPENMP
#pragma omp parallel for
#endif
  for(int z = 0; z < cube->depth; z++){
    int nt =  omp_get_thread_num(); //number of the current thread
    float res = 0;
    for(int y = 0; y < cube->height; y++)
      for(int x = 0; x < cube->width; x++){
        coefficientsRotated[nt] = rotateVector(coeffs, theta->at(x,y,z), phi->at(x,y,z));
        res = 0;
        for(int i = 0; i < (int)coeffs.size(); i++)
          res += coefficientsRotated[nt][i]*cubeDerivs[i]->at(x,y,z);
        result->put(x,y,z,res);
      }
    }
}
