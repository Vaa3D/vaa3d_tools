// q_derivatives3D.h
// by Lei Qu
// 2015-03-07

#ifndef	Q_DERIVATIVES3D_H_
#define Q_DERIVATIVES3D_H_

void gradient3Dx_double(const double *I,const int *sizeI,double *&Ix);
void gradient3Dy_double(const double *I,const int *sizeI,double *&Iy);
void gradient3Dz_double(const double *I,const int *sizeI,double *&Iz);
bool q_derivatives_3D(const double *p_img64f,const long sz_img[4],const char xyz,double *&p_img64f_d);

#endif /* Q_DERIVATIVES3D_H_ */