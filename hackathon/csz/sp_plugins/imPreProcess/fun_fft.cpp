#include "fun_fft.h"
#include <iostream>

bool isPower(int n)
{
    if(n<1) return false;
    int m = n&(n-1);
    return m == 0;
}

bool FFT_1d(complex_type* & data, V3DLONG size, const FFT_direction fft_direction)
{
    if(!isPower(size))
    {
        V3DLONG size_new = pow(2, int(log2(size))+1);
        complex_type* data1;
        data1 = new complex_type[size_new];
        for (int i=0; i<size_new; ++i)
            if(i<size) data1[i] = data[i];
            else data1[i] = 0;
        rearrangeData(data1, size_new);

        if(!makeTransform(data1, size_new, fft_direction)) {
            return false;
        }

        if (FFT_BACKWARD == fft_direction) {
            scaleValues(data1, size_new);
        }

        for (int i=0; i<size; ++i)
            data[i] = data1[i];
    }

    else
    {
        rearrangeData(data, size);

        if(!makeTransform(data, size, fft_direction)) {
            return false;
        }

        if (FFT_BACKWARD == fft_direction) {
            scaleValues(data, size);
        }
    }

    return true;
}

void bufferExchangeHelper(complex_type* & data, V3DLONG index_from, V3DLONG index_to, complex_type & buf)
{
    buf = data[index_from];
    data[index_from] = data[index_to];
    data[index_to]= buf;
}

void scaleValues(complex_type* & data, V3DLONG num_elements)
{
   real_type mult = 1.0 / num_elements;
   int num_elements_signed = static_cast<int>(num_elements);

   for(int i = 0; i < num_elements_signed; ++i)
   {
        data[i] *= mult;
   }
}

void rearrangeData(complex_type* & data, V3DLONG num_elements)
{
    complex_type buf;

    V3DLONG target_index = 0;
    V3DLONG bit_mask;

    for (V3DLONG i = 0; i < num_elements; ++i)
    {
        if (target_index > i)
        {
            bufferExchangeHelper(data, target_index, i, buf);
        }

        // Initialize the bit mask
        bit_mask = num_elements;

        // While bit is 1
        while (target_index & (bit_mask >>= 1)) // bit_mask = bit_mask >> 1
        {
            // Drop bit:
            // & is bitwise AND,
            // ~ is bitwise NOT
            target_index &= ~bit_mask; // target_index = target_index & (~bit_mask)
        }

        // | is bitwise OR
        target_index |= bit_mask; // target_index = target_index | bit_mask
    }
}

void fftTransformHelper(complex_type* & data, V3DLONG match,
                        V3DLONG k, complex_type & product,
                        const complex_type factor)
{
    product = data[match] * factor;
    data[match] = data[k] - product;
    data[k] += product;
}

bool FFT_inplace(complex_type* &data, V3DLONG *sz, const FFT_direction fft_direction)
{
    V3DLONG n_cols  = sz[0];
    V3DLONG n_rows  = sz[1];
    V3DLONG n_depth = sz[2];

    complex_type* subarray = 0;
    subarray = new complex_type[n_cols]; // for fft for rows: each column has n_cols elements

    for(V3DLONG k = 0; k < n_depth; ++k) // for all depth layers
    {
        // fft for rows
        for(V3DLONG j = 0; j < n_rows; ++j)
        {
            for(V3DLONG i = 0; i < n_cols; ++i) {
                V3DLONG offset = (k*n_rows+j)*n_cols+i;
                subarray[i] = data[offset];
            }

            if(!FFT_1d(subarray, n_cols, fft_direction))
            {
                continue;
            }

            for(V3DLONG i = 0; i < n_cols; ++i) {
                V3DLONG offset = (k*n_rows+j)*n_cols+i;
                data[offset] = subarray[i];
            }
        }
    }

    subarray = 0;
    subarray = new complex_type[n_rows]; // for fft for columns: each row has n_rows elements

    for(V3DLONG k = 0; k < n_depth; ++k) // for all depth layers
    {
        // fft for cols
        for(V3DLONG i = 0; i < n_cols; ++i)
        {
            for(V3DLONG j = 0; j < n_rows; ++j) {
                V3DLONG offset = (k*n_rows+j)*n_cols+i;

                subarray[j] = data[offset];
            }

            if(!FFT_1d(subarray, n_cols, fft_direction))
            {
                continue;
            }

            for(V3DLONG j = 0; j < n_rows; ++j) {
                V3DLONG offset = (k*n_rows+j)*n_cols+i;
                data[offset] = subarray[j];
            }
        }
    }

    // fft for depth
    subarray = 0;
    subarray = new complex_type[n_depth]; // each depth strip contains n_depth elements

    for(V3DLONG i = 0; i < n_rows; ++i) // for all rows layers
    {
        for(V3DLONG j = 0; j < n_cols; ++j) // for all cols layers
        {
            for(V3DLONG k = 0; k < n_depth; ++k) {
                V3DLONG offset = (k*n_rows+j)*n_cols+i;
                subarray[k] = data[offset];
            }

            if(!FFT_1d(subarray, n_depth, fft_direction))
            {
                continue;
            }

            for(V3DLONG k = 0; k < n_depth; ++k) {
                V3DLONG offset = (k*n_rows+j)*n_cols+i;
                data[offset] = subarray[k];
            }
        }
    }

    delete[] subarray;
    subarray=0;
    return true;
}

bool makeTransform(complex_type* &data, V3DLONG num_elements, const FFT_direction fft_direction)
{
    using std::sin;

    double local_pi;
    switch(fft_direction)
    {
    case(FFT_FORWARD):
        local_pi = -M_PI;
        break;
    case(FFT_BACKWARD):
        local_pi = M_PI;
        break;
    default:
        return false;
    }

    // declare variables to cycle the bits of initial signal
    size_t next, match;
    real_type sine;
    real_type delta;
    complex_type mult, factor, product;

    // cycle for all bit positions of initial signal
    for (V3DLONG i = 1; i < num_elements; i <<= 1)
    {
        next = i << 1;  // getting the next bit
        delta = local_pi / i;    // angle increasing
        sine = sin(0.5 * delta);    // supplementary sin
        // multiplier for trigonometric recurrence
        mult = complex_type(-2.0 * sine * sine, sin(delta));
        factor = 1.0;   // start transform factor

        for (V3DLONG j = 0; j < i; ++j) // iterations through groups
                                       // with different transform factors
        {
            for (V3DLONG k = j; k < num_elements; k += next) // iterations through
                                                            // pairs within group
            {
                match = k + i;
                fftTransformHelper(data, match, k, product, factor);
            }
            factor = mult * factor + factor;
        }
    }

    return true;
}
