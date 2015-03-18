// q_EigenVectors3D.h
// by Lei Qu
// 2015-03-08

#ifndef	Q_EIGENVECTORS3D_H_
#define Q_EIGENVECTORS3D_H_

//calculates the eigen values and vectors of the 3D hessian images
//        |Ixx Ixy Ixz|
//hessian=|Ixy Iyy Iyz|
//        |Ixz Iyz Izz|
//eigenvalue: eigval3>eigval2>eigval1
bool q_eigenvectors3D(float *p_Ixx,float *p_Ixy,float *p_Ixz,float *p_Iyy,float *p_Iyz,float *p_Izz,
					  long sz_img[4],
					  float *&p_eigval3,float *&p_eigval2,float *&p_eigval1,
					  float *&p_eigvec3x,float *&p_eigvec3y,float *&p_eigvec3z,
					  float *&p_eigvec2x,float *&p_eigvec2y,float *&p_eigvec2z,
					  float *&p_eigvec1x,float *&p_eigvec1y,float *&p_eigvec1z);


//consider to add mask image to save time - todo

#endif /* Q_EIGENVECTORS3D_H_ */
