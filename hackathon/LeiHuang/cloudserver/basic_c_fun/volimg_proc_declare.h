/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it. 

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




// basic volume data operations
//by Hanchuan Peng
//separate from the laff project to common sharing directory
// 2008-04-14
//
// Note: This file should NOT be directly included. The actual definitions of the functions are in the
//       file called "volimg_proc.h", which includes this file. Thus always include "volimg_proc.h" directly.
//

#ifndef __BASIC_VOLUME_IMG_PROC_DECLARE__
#define __BASIC_VOLUME_IMG_PROC_DECLARE__

#include "v3d_basicdatatype.h"

template <class T> T hardLimit(T a, T lowerBound, T upperBound);
template <class T> void swapValue(T & a, T & b);

template <class T> bool vol3d_assign(T *** res, T v, V3DLONG d0, V3DLONG d1, V3DLONG d2);
template <class T> bool vol3d_assign(T *** res, T *** sa, V3DLONG d0, V3DLONG d1, V3DLONG d2);
template <class T> bool vol3d_negative(T *** res, T *** sa, V3DLONG d0, V3DLONG d1, V3DLONG d2);
template <class T> bool vol3d_inverse(T *** res, T *** sa, V3DLONG d0, V3DLONG d1, V3DLONG d2);
template <class T> bool vol3d_square(T *** res, T *** sa, V3DLONG d0, V3DLONG d1, V3DLONG d2);
template <class T> bool vol3d_root(T *** res, T *** sa, V3DLONG d0, V3DLONG d1, V3DLONG d2);
template <class T> bool vol3d_exp(T *** res, T *** sa, V3DLONG d0, V3DLONG d1, V3DLONG d2);
template <class T> bool vol3d_log(T *** res, T *** sa, V3DLONG d0, V3DLONG d1, V3DLONG d2);

template <class T> bool vol3d_min(T & res, T *** sa, V3DLONG d0, V3DLONG d1, V3DLONG d2);
template <class T> bool vol3d_max(T & res, T *** sa, V3DLONG d0, V3DLONG d1, V3DLONG d2);
template <class T> bool vol3d_scale_01(T *** res, T *** sa, V3DLONG d0, V3DLONG d1, V3DLONG d2);
template <class T> bool vol3d_sum(T & res, T *** sa, V3DLONG d0, V3DLONG d1, V3DLONG d2);
template <class T> bool vol3d_mean(T & res, T *** sa, V3DLONG d0, V3DLONG d1, V3DLONG d2);
template <class T> bool vol3d_threshold(T *** res, V3DLONG d0, V3DLONG d1, V3DLONG d2, T thres, bool b_set_to_binary);
template <class T> bool vol3d_nnz(V3DLONG &nnz, T *** sa, V3DLONG d0, V3DLONG d1, V3DLONG d2, V3DLONG d0b, V3DLONG d0e, V3DLONG d1b, V3DLONG d1e, V3DLONG d2b, V3DLONG d2e);

template <class T> bool vol3d_plus(T *** res, T *** sa, T *** sb, V3DLONG d0, V3DLONG d1, V3DLONG d2);
template <class T> bool vol3d_plus_constant(T *** res, T *** sa, double c, V3DLONG d0, V3DLONG d1, V3DLONG d2);
template <class T> bool vol3d_minus(T *** res, T *** sa, T *** sb, V3DLONG d0, V3DLONG d1, V3DLONG d2);
template <class T> bool vol3d_time(T *** res, T *** sa, T *** sb, V3DLONG d0, V3DLONG d1, V3DLONG d2);
template <class T> bool vol3d_time_constant(T *** res, T *** sa, double c, V3DLONG d0, V3DLONG d1, V3DLONG d2);
template <class T> bool vol3d_divide(T *** res, T *** sa, T *** sb, V3DLONG d0, V3DLONG d1, V3DLONG d2);

template <class T> bool vol3d_square_root_diff(double & res, T *** sa, T *** sb, V3DLONG d0, V3DLONG d1, V3DLONG d2);

template <class T1, class T2> bool mean_and_std(T1 *data, V3DLONG n, T2 & ave, T2 & sdev);
template <class T> bool mean_and_std(T *data, V3DLONG n, T & ave, T & sdev, T maskval);
template <class T> bool mean_and_std(T *data, V3DLONG n, T & ave, T & sdev, T maskval_lowerbound, T maskval_upperbound);
template <class T> void moment(T *data, V3DLONG n, double & ave, double & adev, double & sdev, double & var, double & skew, double & curt);
template <class T> T minInVector(T * p, V3DLONG len, V3DLONG &pos);
template <class T> T maxInVector(T * p, V3DLONG len, V3DLONG &pos);
template <class T> bool minMaxInVector(T * p, V3DLONG len, V3DLONG &pos_min, T &minv, V3DLONG &pos_max, T &maxv);

template <class T> bool fit_to_cube(T * & img, V3DLONG * sz, V3DLONG * sz_target);
template <class T> bool downsample3dimg_1dpt(T * & img, V3DLONG * sz, double dfactor);
template <class T> bool reslice_Z(T * & invol1d, V3DLONG * sz, double xy_rez, double z_rez);
template <class T> bool resample3dimg_linear_interp(T * & img, V3DLONG * sz, double dfactor_x, double dfactor_y, double dfactor_z);

template <class T> double calCorrelation(T * img1, T * img2, V3DLONG imglen);

template <class T> void twopoints_lineprofile_3dimg(T *** img, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, double x1, double y1, double z1, double x2, double y2, double z2, T * &profile, V3DLONG & profile_len);
template <class T> double twopoints_lineprofile_3dimg_sum(T *** img, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, double x1, double y1, double z1, double x2, double y2, double z2);


#endif
