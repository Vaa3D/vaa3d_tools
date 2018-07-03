// q_imgaussian3D.h
// by Lei Qu
// 2015-03-07

#ifndef	Q_IMGAUSSIAN3D_H_
#define Q_IMGAUSSIAN3D_H_

void imfilter1D_float(float *I,int lengthI,float *H,int lengthH, float *J);
void imfilter2D_float(float *I,int * sizeI,float *H,int lengthH, float *J);
void imfilter3D_float(float *I,int * sizeI,float *H,int lengthH, float *J);
void GaussianFiltering3D_float(float *I, float *J,int *dimsI,float sigma,float kernel_size);
bool q_imgaussian3D(float *p_img32f,long sz_img[4],
					float sigma,float szkernel,
					float *&p_img32f_output);

#endif /* Q_IMGAUSSIAN3D_H_ */