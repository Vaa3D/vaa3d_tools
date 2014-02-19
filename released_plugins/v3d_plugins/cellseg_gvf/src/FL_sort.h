//change to teh standard c++ sort function, 20130724
  
#ifndef __FL_SORT__
#define __FL_SORT__
 


#include "../../v3d_main/basic_c_fun/v3d_basicdatatype.h"

#include <algorithm>
#include <vector>

template <class T1, class T2> bool comparator ( const std::pair<T1, T2> & l, const std::pair<T1, T2> & r)
   { return l.first < r.first; }


template <class T> void sort2(V3DLONG n, T arr[])
{
   V3DLONG i;
   std::vector<float> myvec;
   for (V3DLONG i=0; i<n;i++)
     myvec.push_back(arr[i]);

   std::sort(myvec.begin(), myvec.begin()+n);

   for (V3DLONG i=0; i<n;i++)
     arr[i] = myvec.at(i);
}

template <class T1, class T2> void sort2_withindex(V3DLONG n, T1 arr[], T2 brr[])
{
   V3DLONG i;

   typedef std::pair<T1, T2> mypair;
   std::vector<mypair> myvec;
   for (V3DLONG i=0; i<n;i++)
   {
     mypair p(arr[i], brr[i]);
     myvec.push_back(p);
   }

   std::sort(myvec.begin(), myvec.end());

   for (V3DLONG i=0; i<n;i++)
   {
     arr[i] = myvec.at(i).first;
     brr[i] = myvec.at(i).second;
   }
}



#endif
