// q_imgaussian3D.h
// by Lei Qu
// 2015-03-07

#ifndef	Q_IMGAUSSIAN3D_H_
#define Q_IMGAUSSIAN3D_H_

void imfilter1D_double(double *I,int lengthI,double *H,int lengthH, double *J);
void imfilter2D_double(double *I,int * sizeI,double *H,int lengthH, double *J);
void imfilter3D_double(double *I,int * sizeI,double *H,int lengthH, double *J);
void GaussianFiltering3D_double(double *I, double *J,int *dimsI,double sigma,double kernel_size);
bool q_imgaussian3D(double *p_img64f,long sz_img[4],
					double sigma,double szkernel,
					double *&p_img64f_output);

#endif /* Q_IMGAUSSIAN3D_H_ */