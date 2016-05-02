// q_convolve.h
// by Lei Qu
// 2010-02-24

#ifndef Q_CONVOLVE_H_
#define Q_CONVOLVE_H_

#include <vector>
using namespace std;


bool q_kernel_gaussian_1D(const long l_radius,const double sigma,
		vector<double> &vec1D_kernel);
bool q_kernel_gaussian(const long l_radius_x,const long l_radius_y,const long l_radius_z,const double sigma,
		vector< vector< vector<double> > > &vec3D_kernel);

bool q_convolve_img64f_3D(double *&p_img64f,const long sz_img[4],
		const vector< vector< vector<double> > > &vec3D_kernel);
bool q_convolve_vec64f_3D(vector< vector< vector< vector<double> > > > &vec4D_grid,
		const vector< vector< vector<double> > > &vec3D_kernel);

bool q_convolve_img64f_3D_fast(double *&p_img64f,const long sz_img[4],
		const vector<double> &vec1D_kernel);
bool q_convolve_vec64f_3D_fast(vector< vector< vector< vector<double> > > > &vec4D_grid,
		const vector<double> &vec1D_kernel);


#endif /* Q_CONVOLVE_H_ */
