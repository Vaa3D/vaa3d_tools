// q_convolve_kernel.h
// by Lei Qu
// 2019-05-07

#ifndef Q_CONVOLVE_KERNEL_H_
#define Q_CONVOLVE_KERNEL_H_

#include <vector>
using namespace std;


//Gaussian kernel
bool q_kernel_gaussian_1D(const long l_radius,const double sigma,
		vector<double> &vec1D_kernel);
bool q_kernel_gaussian_3D(const long l_radius_x,const long l_radius_y,const long l_radius_z,const double sigma,
		vector< vector< vector<double> > > &vec3D_kernel);

//sobel kernel (for edge and gradient)
bool q_kernel_sobel_3D(const char xyz, vector< vector< vector<double> > > &vec3D_kernel);

//laplacian kernel (for edge)
bool q_kernel_laplacian_3D(vector< vector< vector<double> > > &vec3D_kernel);

#endif
