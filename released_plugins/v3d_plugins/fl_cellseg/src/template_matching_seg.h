//template_match_seg.h
//by Hanchuan Peng
//090426

#ifndef __TEMPLATE_MATCHING_SEG_H__
#define __TEMPLATE_MATCHING_SEG_H__

#include "../basic_c_fun/img_definition.h"

#include "../v3d/template_matching_cellseg_dialog.h"

bool template_matching_seg(Vol3DSimple <unsigned char> *img3d, Vol3DSimple <unsigned short int> *outimg3d, const para_template_matching_cellseg & mypara);

Vol3DSimple<double> * genGaussianKernal3D(V3DLONG szx, V3DLONG szy, V3DLONG szz, double sigmax, double sigmay, double sigmaz);

template <class T1, class T2> double compute_corrcoef_two_vectors(T1 *v1, T2 *v2, V3DLONG len);
template <class T1, class T2> double compute_normalized_L1dist_two_vectors(T1 *v1, T2 *v2, V3DLONG len, bool b_remove_mean);
template <class T1, class T2> double compute_normalized_L2dist_two_vectors(T1 *v1, T2 *v2, V3DLONG len, bool b_remove_mean);


#endif


