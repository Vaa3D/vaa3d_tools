#ifndef MATLAB_UTILS_HXX
#define MATLAB_UTILS_HXX

template<typename T>
mxClassID matlabClassID() { mexErrMsgTxt("Class ID not defined"); }

template<>
mxClassID matlabClassID<int>() { return mxINT32_CLASS; }

template<>
mxClassID matlabClassID<unsigned int>() { return mxUINT32_CLASS; }

template<>
mxClassID matlabClassID<unsigned char>() { return mxUINT8_CLASS; }

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
            mexPrintf("Types do not match");
            mexErrMsgTxt(errMsg);
        }

        if ( (mxGetN(mArray) != N) || (mxGetM(mArray) != M)) {
            mexPrintf("Sizes do not match: expected %d x %d matrix", M, N);
            mexErrMsgTxt(errMsg);
        }

        mData = (T *)mxGetData(mArray);
    }

    inline const T* data() const { return mData; }
    inline T data(unsigned int idx) const { return mData[idx]; }
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
