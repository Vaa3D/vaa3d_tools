#ifndef SOME_FUNCTION_H
#define SOME_FUNCTION_H

#include<QString>
#include<basic_surf_objs.h>
#include<vector>
#include<string>
#include<cstring>


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

inline bool isin(V3DLONG parent,std::vector<V3DLONG> a)
{
    for(int i=0;i<a.size();++i)
    {
        if(parent==a[i])
            return true;
    }
    return false;
}
inline int findIndex(const char* s)
{
    if(s==NULL)
    {
        return -1;
    }
    for(int i=0;;++i)
    {
        if(s[i]=='\0')
            return -2;
        if(s[i]=='.')
            return i;
    }
}
inline QString path(const QString& s)
{
    std::string tmp=s.toStdString();
    const char* tmp0=tmp.c_str();

    int index=findIndex(tmp0);
    const int num=100;
    char tmp1[num];
    for(int i=0;i<index;++i)
    {
        tmp1[i]=tmp0[i];
    }
    tmp1[index]='\0';
    QString result(tmp1);

    return result;

    /*char* tmp1=new char[100];
    strcpy(tmp1,tmp0);
    char* index=strstr(tmp1,'.');
    char s0[index+1];
    for(int i=0;i<index;++i)
    {
        s0[i]=tmp0[i];
    }
    s0[index]='\0';
    QString result(s0);
    return result;*/
}


NeuronTree readESWC_file(const QString& filename);
bool writeESWC_file1(const QString& filename, const NeuronTree& nt);

#endif // SOME_FUNCTION_H
