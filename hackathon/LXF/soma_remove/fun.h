
#ifndef FUN_H
#define FUN_H
#include <vector>
using namespace std;
template <class T>
void combine(vector<T> &v, int left, int m, int right, vector<int> &index)
{
    vector<T> tempv(v.begin()+left,v.begin()+right+1);
    vector<int> tempindex(index.begin()+left,index.begin()+right+1);

    int left_size=m-left+1;
    int size=right-left+1;
    int middle=m-left+1;
    int i=0;
    int j=middle;
    int k=left;
    while(i<left_size && j<size)
    {
        if(tempv[i]<=tempv[j])
        {
            v[k]=tempv[i];
            index[k]=tempindex[i];
            k++;
            i++;
        }else{
            v[k]=tempv[j];
            index[k]=tempindex[j];
            k++;
            j++;
        }
    }
    while(i<left_size)
    {
        v[k]=tempv[i];
        index[k]=tempindex[i];
        k++;
        i++;
    }
}


template <class T>
void merge_sort(vector<T> &v, int left, int right, vector<int> &index)
{
    if (left<right)
    {
        int m=(left+right)/2;
        merge_sort(v,left, m,index);
        merge_sort(v,m+1, right, index);
        combine(v,left, m, right,index);
    }
}


template <class T>
void merge_sort(vector<T> v, vector<int> &index)
//index[i] is the original index of i th best element
{
    index.clear();
    index.resize(v.size());
    for(int i=0;i<v.size();i++)
        index[i]=i;
    merge_sort(v,0,v.size()-1,index);
}
#endif
