// q_derivatives3D.h
// by Lei Qu
// 2015-03-07

#ifndef	Q_DERIVATIVES3D_H_
#define Q_DERIVATIVES3D_H_

void gradient3Dx_float(const float *I,const int *sizeI,float *&Ix);
void gradient3Dy_float(const float *I,const int *sizeI,float *&Iy);
void gradient3Dz_float(const float *I,const int *sizeI,float *&Iz);
bool q_derivatives_3D(const float *p_img32f,const long sz_img[4],const char xyz,float *&p_img32f_d);

#endif /* Q_DERIVATIVES3D_H_ */