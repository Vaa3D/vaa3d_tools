#ifndef MATLAB_UTILS_HXX
#define MATLAB_UTILS_HXX

// This file is part of SQBlib, a gradient boosting/boosted trees implementation.
//
// Copyright (C) 2012 Carlos Becker, http://sites.google.com/site/carlosbecker
//
// SQBlib is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// SQBlib is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with SQBlib.  If not, see <http://www.gnu.org/licenses/>.

template<typename T>
mxClassID matlabClassID() { mexErrMsgTxt("Class ID not defined"); }

template<>
mxClassID matlabClassID<int>() { return mxINT32_CLASS; }

template<>
mxClassID matlabClassID<unsigned int>() { return mxUINT32_CLASS; }

template<>
mxClassID matlabClassID<unsigned char>() { return mxUINT8_CLASS; }

template<>
mxClassID matlabClassID<char>() { return mxCHAR_CLASS; }


template<>
mxClassID matlabClassID<double>() { return mxDOUBLE_CLASS; }

template<>
mxClassID matlabClassID<float>() { return mxSINGLE_CLASS; }

template<typename T>
class MatlabInputMatrix
{
private:
    const T *mData;
    const mxArray *mArray;

public:
    typedef T DataType;

    // if M and/or N is zero => means that it won't check on that dimension
    MatlabInputMatrix( const mxArray *_mArray, unsigned int M, unsigned int N, const char *errMsg ) {
        mArray = _mArray;

        if ( mxGetClassID(mArray) != matlabClassID<T>() ) {
            mexPrintf("Types do not match, got %d", mxGetClassID(mArray));
            mexErrMsgTxt(errMsg);
        }

        if (N > 0)
        {
            if (mxGetN(mArray) != N)
            {
                mexPrintf("Sizes do not match: expected %d x %d matrix", M, N);
                mexErrMsgTxt(errMsg);
            }
        }

        if (M > 0)
        {
            if (mxGetM(mArray) != M)
            {
                mexPrintf("Sizes do not match: expected %d x %d matrix", M, N);
                mexErrMsgTxt(errMsg);
            }
        }

        mData = (T *)mxGetData(mArray);
    }

    inline const T* data() const { return mData; }
    inline T data(unsigned int idx) const { return mData[idx]; }
    inline unsigned int rows() const { return mxGetM(mArray); }
    inline unsigned int cols() const { return mxGetN(mArray); }
};

template<typename T>
class MatlabOutputMatrix
{
private:
    T *mData;
    mxArray **mArray;

public:
    typedef T DataType;

    MatlabOutputMatrix( mxArray **_mArray, unsigned int M, unsigned int N ) {
        mArray = _mArray;

        // allocate, real type

        *mArray = mxCreateNumericMatrix( M, N, matlabClassID<T>(), mxREAL );

        mData = (T *)mxGetData(*mArray);
    }

    inline const T* data() const { return mData; }
    inline T* data() { return mData; }

    inline T data(unsigned int idx) const { return mData[idx]; }
};



#endif // MATLAB_UTILS_HXX
