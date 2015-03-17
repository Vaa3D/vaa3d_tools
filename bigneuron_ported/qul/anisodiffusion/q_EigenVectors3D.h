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
bool q_eigenvectors3D(double *p_Ixx,double *p_Ixy,double *p_Ixz,double *p_Iyy,double *p_Iyz,double *p_Izz,
					  long sz_img[4],
					  double *&p_eigval3,double *&p_eigval2,double *&p_eigval1,
					  double *&p_eigvec3x,double *&p_eigvec3y,double *&p_eigvec3z,
					  double *&p_eigvec2x,double *&p_eigvec2y,double *&p_eigvec2z,
					  double *&p_eigvec1x,double *&p_eigvec1y,double *&p_eigvec1z);


//consider to add mask image to save time - todo

#endif /* Q_EIGENVECTORS3D_H_ */
