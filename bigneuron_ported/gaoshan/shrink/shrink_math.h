#ifndef SHRINK_MATH
#define SHRINK_MATH

template <class T>
double shrink_sum(T *Input,long n)
{
    double Output = 0;
    for (int i = 0;i < n;i++)
    {
        Output+=Input[i];
    }
    return Output;
}

#endif // SHRINK_MATH

