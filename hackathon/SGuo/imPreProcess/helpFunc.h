#ifndef IMPREPROCESS_MAIN_H
#define IMPREPROCESS_MAIN_H

#include "v3d_interface.h"
#include "bilateral_filter.h"
#include "fun_fft.h"
#include "morphology.h"
#include "algorithm"
#include <iostream>

double cal_percentile(int* src, V3DLONG sz, double per);
bool sigma_correction(unsigned char* src, V3DLONG * mysz, double cutoff, double gain, unsigned char* &dst);
bool intensity_rescale(unsigned char* src, V3DLONG * mysz, unsigned char* &dst);
bool subtract_min(unsigned char* src, V3DLONG * mysz, unsigned char* & dst);

//template <typename T> bool cal_percentile(T* src, V3DLONG sz, int num_pers, int* per, T* &dst);
bool fft_filter(unsigned char* & data, V3DLONG * mysz);
bool intensity_projection(unsigned char* src, V3DLONG * mysz, unsigned char* & dst, int flag);
#endif // IMPREPROCESS_MAIN_H

