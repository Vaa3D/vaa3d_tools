// q_histogram_matching.h
// by Lei Qu
// 2010-06-09

#ifndef __Q_HISTOGRAM_MATCHING_H__
#define __Q_HISTOGRAM_MATCHING_H__

// match the histogram of subject image to that of target
// note:
//		both target and subject image should be 8bits and 1 channel
//		target and subject image may have difference size
bool q_histogram_matching_1c(const unsigned char *p_img8u1c_tar,const long *sz_img8u1c_tar,
						     const unsigned char *p_img8u1c_sub,const long *sz_img8u1c_sub,
						     unsigned char *&p_img8u1c_sub2tar);

#endif

