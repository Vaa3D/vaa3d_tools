// q_imgaussian3D.h
// by Lei Qu
// 2015-03-07

#ifndef	Q_IMGAUSSIAN3D_H_
#define Q_IMGAUSSIAN3D_H_

void imfilter1D_float(float *I,int lengthI,float *H,int lengthH, float *J);
void imfilter1D_double(double *I, int lengthI, double *H, int lengthH, double *J);

void imfilter2D_float(float *I,int * sizeI,float *H,int lengthH, float *J);
void imfilter2D_double(double *I, int * sizeI, double *H, int lengthH, double *J);

void imfilter3D_float(float *I,int * sizeI,float *H,int lengthH, float *J);
void imfilter3D_double(double *I, int * sizeI, double *H, int lengthH, double *J);

void GaussianFiltering1D_float(float *I, float *J, int lengthI, double sigma, double kernel_size);
void GaussianFiltering1D_double(double *I, double *J, int lengthI, double sigma, double kernel_size);

void GaussianFiltering2D_float(float *I, float *J, int *dimsI, double sigma, double kernel_size);
void GaussianFiltering2D_double(double *I, double *J, int *dimsI, double sigma, double kernel_size);

void GaussianFiltering3D_float(float *I, float *J,int *dimsI,float sigma,float kernel_size);
void GaussianFiltering3D_double(double *I, double *J, int *dimsI, double sigma, double kernel_size);

bool q_imgaussian3D(float *p_img32f,long sz_img[4],
					float sigma,float szkernel,
					float *&p_img32f_output);


#endif /* Q_IMGAUSSIAN3D_H_ */