// FL_registerAffine.h
// affine transformation between point clouds
// 20090203

#ifndef __FL_REGISTER_AFFINE_H__
#define __FL_REGISTER_AFFINE_H__

void registerAffine(float *subject, long num_pts, float *cpt_subject, float *cpt_target, long num_cpts, unsigned char ndims);

//Matrix pseudoinverse(const Matrix& X); // compute pseudo inverse of a matrix

#endif

