/**@file tz_<1t>_lib.h
 * @brief routines for <3t> image
 * @author Ting Zhao
 */

#ifndef _TZ_<1T>_LIB_H_
#define _TZ_<1T>_LIB_H_

/* <T1 DIMAGE> <T2 DMatrix> <T3 DOUBLE> <T4 D> 
   <t5 fftw> <T6 Double> <t7 darray> */

#include "tz_cdefs.h"
<3t=double,float>
#include "tz_fftw_header.h"
</t>
#include "image_lib.h"
#include "tz_<2t>.h"
#include "tz_geo3d_scalar_field.h"
#include "tz_object_3d.h"

__BEGIN_DECLS

<3t=double,float>
 /**@brief Cumulative sum of an image.
 * Cumsum_Image_<4T>() calculates cumulative sum of an image. It returns a
 * <3t> array whose ith element is the sum of the values of 0 - i pixels in
 * the image.
 */
<3t>* Cumsum_Image_<4T>(const Image *image);

<3t=double>
#if defined(HAVE_LIBFFTW)
</t>

<3t=float>
#if defined(HAVE_LIBFFTWF)
</t>

/**@brief Fast fourier transform.
 * Image_FFT_<4T>() calculates the fourier transform of an image. It returns an
 * array of complex numbers. Image_IFFT_<4T> calculates the inverse fourier
 * transform of a complex array in the 2D space. The function assumes the array
 * has size <width> x <height>. 
 */
<5t>_complex* Image_FFT_<4T>(Image *image);
<3t> *Image_IFFT_<4T>(<5t>_complex* fimage,int width,int height);

#endif

/**@brief Convolve an image.
 * 
 */
<3t>* Convolve_Image_<4T>(Image *image1, Image *image2,int reflect);
<3t>* Correlate_Image_<4T>(Image* image1,Image* image2);
<3t>* Normcorr_Image_<4T>(Image *image1, Image *image2);

<2T=DMatrix,FMatrix>
<2T>* Mexihat_2D_<4T>(double sigma, <2T> *filter);
<2T>* Filter_Image_<4T>(Image *image, const <2T> *filter, <2T> *out);
<2T>* Filter_Image_Fast_<4T>(Image *image, <2T> *filter, <2T> *out, int pad);
</t>


<3t>* Get_<6T>_Array_Pad(Stack *stack);

<3t=double>
#if defined(HAVE_LIBFFTW)
</t>

<3t=float>
#if defined(HAVE_LIBFFTWF)
</t>

<5t>_complex* Stack_FFT_<4T>(Stack *stack);
<3t>* Stack_IFFT_<4T>(<5t>_complex* fstack,int width,int height,int depth);

#endif

<3t>* Convolve_Stack_<4T>(Stack* stack1,Stack* stack2,int reflect);
<3t>* Correlate_Padstack_<4T>(Stack* stack1,Stack* stack2);
<3t>* Correlate_Stack_Part_<4T>(Stack* stack1,Stack* stack2,int start[],int end[]);
<2T>* Normcorr_Stack_<4T>(Stack *stack1, Stack *stack2,int std,<3t> *max_corr);
<2T>* Normcorr_Stack_Part_<4T>(Stack *stack1,Stack *stack2,int std,int start[],int end[]);
<3t> Align_Stack_<4T>(Stack* stack1,Stack* stack2,int offset[],<3t> *unnorm_maxcorr);
<3t> Align_Stack_C_<4T>(Stack* stack1, Stack* stack2, const int config[],
			int offset[], <3t> *unnorm_maxcorr);
<3t> Align_Stack_MR_<4T>(Stack* stack1, Stack* stack2, int intv[], int fine,
			 const int *config, int offset[], <3t> *unnorm_maxcorr);
</t>

<2T=DMatrix,FMatrix>
<2T>* Ring_Filter_<4T>(double r1, double r2, <2T> *filter);
<2T>* Gaussian_2D_Filter_<4T>(const double *sigma, <2T> *filter);
<2T>* Mexihat_3D1_<4T>(double sigma, <2T> *filter, ndim_t dt);
<2T>* Mexihat_2D_<4T>(double sigma, <2T> *filter);
<2T>* Mexihat_3D_<4T>(double sigma, <2T> *filter);
<2T>* Gaussian_3D_Filter_<4T>(const double *sigma, <2T> *filter);
<2T>* Gaussian_3D_Filter_2x_<4T>(const double *sigma, <2T> *filter);
<2T>* Gaussian_Deriv_3D_Filter_<4T>(const double *sigma, double theta, 
				    double psi, <2T> *filter);
<2T>* Gaussian_3D_D2_Filter_<4T>(const double *sigma, int dim[2], <2T> *filter);
<2T>* Geo3d_Scalar_Field_To_Filter_<4T>(const Geo3d_Scalar_Field *field);

<2T>* Filter_Stack_<4T>(const Stack *stack, const <2T> *filter, <2T> *out);
<2T>* Filter_Stack_Fast_<4T>(const Stack *stack, const <2T> *filter, <2T> *out, int pad);
<2T>* Filter_Stack_Block_<4T>(const Stack *stack, const <2T> *filter, 
			      <2T> *out);
<2T>* Filter_Stack_Slice_<4T>(const Stack *stack, const <2T> *filter, <2T> *out);

<2T>* Smooth_Stack_Fast_<4T>(const Stack *stack, int wx, int wy, int wz, 
    <2T> *out);

void Correct_Filter_Stack_<4T>(<2T> *filter, <2T> *stack);

<2T>* El_Stack_<4T>(const Stack *stack, const double *scale, <2T> *out);
<2T>* El_Stack_L_<4T>(const Stack *stack, const double *scale, <2T> *out);

Stack *Stack_Line_Paint_<4T>(const Stack *stack, double *scale, int option);
<2T>* Stack_Pixel_Feature_<4T>(const Stack *stack, const double *scale, 
			       const Object_3d *pts, <2T> *result);
</t>

<3t=double,float,int>
<2T>* Get_<6T>_Matrix3(const Stack *stack);
</t>

__END_DECLS
#endif
