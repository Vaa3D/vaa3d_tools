#ifndef SHRINK_THRESHOLD
#define SHRINK_THRESHOLD
/*  function shrink_im2bw is adopted from ada_threshold.cpp
 *  which is created by Yang, Jinzhu and Hanchuan Peng, on 11/22/10.
 *  Add dofunc() interface by Jianlong Zhou, 2012-04-18.
 *
 *  modified by Gao Shan
 *  2015/3/19
*/
template <class T>
void shrink_im2bw(T *apsInput, T *aspOutput, long iImageWidth, long iImageHeight, long iImageLayer, long h, long d)
{
    long mCount = iImageHeight * iImageWidth;
    for (long i=0; i<iImageLayer; i++)
    {
        for (long j=0; j<iImageHeight; j++)
        {
            for (long k=0; k<iImageWidth; k++)
            {
                double temp = 0;
                long count = 0;
                for(long n=1; n<=d; n++)
                {
                    if (k>h*n)                {temp += apsInput[i*mCount + j*iImageWidth + (k-h*n)]; count++;}
                    if (k+h*n<iImageWidth)    {temp += apsInput[i*mCount + j*iImageWidth + (k+h*n)]; count++;}
                    if (j>h*n)                {temp += apsInput[i*mCount + (j-h*n)*iImageWidth + k]; count++;}
                    if (j+h*n<iImageHeight)   {temp += apsInput[i*mCount + (j+h*n)*iImageWidth + k]; count++;}
                    if (i>h*n)                {temp += apsInput[(i-h*n)*mCount + j*iImageWidth + k]; count++;}
                    if (i+h*n<iImageLayer)    {temp += apsInput[(i+h*n)*mCount + j*iImageWidth + k]; count++;}
                }
                long idx = i*mCount + j*iImageWidth + k;
                aspOutput[idx]= apsInput[idx]>temp/count?255:0;
            }
        }
    }
}

#endif // SHRINK_THRESHOLD

