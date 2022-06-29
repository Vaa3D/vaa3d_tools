/* FFT-3D
 * adopted by S Guo 16.Dec, 2020
 * source: https://github.com/d1vanov/Simple-FFT
*/

#ifndef FUN_FFT_H
#define FUN_FFT_H

#include <cstddef>
#include <math.h>
#include <vector>
#include <complex>
#include "v3d_basicdatatype.h"

using namespace std;

typedef double real_type;
typedef complex<real_type> complex_type;

#ifndef M_PI
#define M_PI 3.1415926535897932
#endif

enum FFT_direction
{
    FFT_FORWARD = 0,
    FFT_BACKWARD
};

bool isPower(int n);
bool FFT_inplace(complex_type* &data, V3DLONG *sz, const FFT_direction fft_direction);

bool FFT_1d(complex_type* &data, V3DLONG size, const FFT_direction fft_direction);

bool makeTransform(complex_type* &data, V3DLONG num_elements, const FFT_direction fft_direction);

void bufferExchangeHelper(complex_type* & data, V3DLONG index_from, V3DLONG index_to, complex_type & buf);
void rearrangeData(complex_type* & data, V3DLONG num_elements);
void scaleValues(complex_type* & data, V3DLONG num_elements);
#endif // FUN_FFT_H
