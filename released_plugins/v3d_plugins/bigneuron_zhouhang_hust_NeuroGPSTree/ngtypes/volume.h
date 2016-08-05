//time : 2013-11-20
//this file provide a structure to save image data and temp image data
//author : zhouhang, WLNO

#ifndef VOLUME_H
#define VOLUME_H
#include "ineurondataobject.h"
#include "volumecreator.h"
typedef unsigned char NGCHAR;
template<typename T> class Volume;
typedef Volume<NGCHAR> CVolume;
typedef Volume<unsigned short> SVolume;
typedef Volume<double> DVolume;
template<typename T>
class Volume : public INeuronDataObject
{
public:
    Volume(INeuronProcessObject* p = nullptr);
    //Volume& operator=(const VolumeBlock&);
    ~Volume();

    bool IsEmpty()const{return dataptr_ == nullptr;}
    T& operator()(int x, int y, int z){return dataptr_[z][x + y * x_];}
    T  operator()(int x, int y, int z)const{return dataptr_[z][x + y * x_];}
    T& GetPixel(int x, int y, int z){return dataptr_[z][x + y * x_];}
    T GetPixel(int x, int y, int z)const{return dataptr_[z][x + y * x_];}
    void SetSize(int,int, int);//original data will lost
    void SetResolution(double, double, double);
    void Clear();
    void QuickCopy(const Volume<T>&);
    void SetZero();
    T** GetPointer(){return dataptr_;}
//    void SetProcessObject(INeuronProcessObject*);
//    void ReleaseProcessObject();

    int x()const{return x_;}
    int y()const{return y_;}
    int z()const{return z_;}
    double XResolution()const{return xRes_;}
    double YResolution()const{return yRes_;}
    double ZResolution()const{return zRes_;}

private:
    //data
    int x_ , y_ , z_ , xy_ ;//size
    double xRes_, yRes_, zRes_;
    T **dataptr_;

    //private function
    Volume(const Volume&);//no copy assign
    Volume& operator=(const Volume&);//no copy
};

template<class T>
Volume<T>::Volume(INeuronProcessObject* p)
{
    m_ProcessObject =  p;
    identifyName =  std::string("Volume");
    x_ = 0;
    y_ = 0;
    z_ = 0;
    xy_ = 0;//size
    xRes_=1.0;
    yRes_=1.0;
    zRes_=1.0;
    dataptr_ = nullptr;
}

template<class T>
Volume<T>::~Volume()
{
    Destroy2DArray<T>(&dataptr_, z_, xy_);
}

template<class T>
void Volume<T>::SetSize(int x, int y, int z)
{
    if(x < 0 || y < 0 || z < 0 || (x_ == x && y_ == y && z_ == z)) return;
    if(0 == (x * y * z) ){
        if(dataptr_)
            Destroy2DArray<T>(&dataptr_, z_, xy_);
        x_ = x; y_ = y; z_ = z; xy_ = x_ * y_;
    }
    else{
        if(dataptr_)
            Destroy2DArray<T>(&dataptr_, z_, xy_);
        x_ = x; y_ = y; z_ = z; xy_ = x_ * y_;
        Create2DArray<T>(&dataptr_, z_, xy_);
    }
}

template<class T>
void Volume<T>::SetResolution(double x, double y, double z)
{
    xRes_ = x;
    yRes_ = y;
    zRes_ = z;
}

template<class T>
void Volume<T>::Clear()
{
    SetSize(0,0,0);
}

template<class T>
void Volume<T>::QuickCopy(const Volume<T> &src)
{
    SetSize(src.x(), src.y(), src.z());
    T** sptr = src.dataptr_;
    T** dptr = dataptr_;
    for (int ij = 0; ij < src.z(); ++ij)
        memcpy(dptr[ij], sptr[ij], sizeof(T) * src.x() * src.y());
}

template<class T>
void Volume<T>::SetZero()
{
    Set2DArray<T>(dataptr_, z_, xy_, 0);
}



#endif // VOLUME_H
