#ifndef SOME_FUNCTION_H
#define SOME_FUNCTION_H

#include<QString>
#include<basic_surf_objs.h>


template<class T,class iterator>
inline T max0(iterator begin,iterator end){
    T temp=*begin;
    T max=*begin;
    while(begin!=end){
      max=max>temp?max:temp;
      begin++;
      temp=*begin;
    }
    return max;
}
template<class T,class iterator>
inline T min0(iterator begin,iterator end){
    T temp=*begin;
    T max=*begin;
    while(begin!=end){
      max=max<temp?max:temp;
      begin++;
      temp=*begin;
    }
    return max;
}

NeuronTree readESWC_file(const QString& filename);
bool writeESWC_file1(const QString& filename, const NeuronTree& nt);









#endif // SOME_FUNCTION_H
