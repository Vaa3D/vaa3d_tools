//change to teh standard c++ sort function, 20130724
  
#ifndef __FL_SORT__
#define __FL_SORT__
 


#include "../../v3d_main/basic_c_fun/v3d_basicdatatype.h"

#include <algorithm>
#include <vector>
template <class T> void sort2(V3DLONG n, T arr[], float brr[])
{
   V3DLONG i;
   std::vector<float> myvec;
   for (V3DLONG i=0; i<n;i++)
     myvec.push_back(arr[i]);

   std::sort(myvec.begin(), myvec.begin()+n);

   for (V3DLONG i=0; i<n;i++)
     brr[i] = myvec.at(i);
}


#endif
