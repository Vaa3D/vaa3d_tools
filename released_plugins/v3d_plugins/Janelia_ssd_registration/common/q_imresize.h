// q_imresize.h
// by Lei Qu
// 2010-02-24

#ifndef __Q_IMRESIZE_H__
#define __Q_IMRESIZE_H__


//resize 3D image stack (linear interpolate)
bool q_imresize8u_3D(const unsigned char *p_img8u_input,const long sz_img_input[4],
		const long sz_img_output[4],unsigned char *&p_img8u_output);

bool q_imresize64f_3D(const double *p_img64f_input,const long sz_img_input[4],
		const long sz_img_output[4],double *&p_img64f_output);


#endif
