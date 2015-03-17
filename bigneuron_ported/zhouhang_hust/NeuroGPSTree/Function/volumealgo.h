#ifndef VOLUMEALGO_H
#define VOLUMEALGO_H
#include "../ngtypes/volume.h"
#include "../ngtypes/basetypes.h"
#include <algorithm>

template<typename T>
T MedianAfterSort(const std::vector<T>& arg)
{
    if(arg.empty()) return 0;
    if(arg.size()%2 == 0){//not odd
        int size1 = arg.size() /2;
        int size2 = size1 -1;
        return (arg[size1] + arg[size2])/2;
    } else{
        int size = arg.size() / 2;
        return arg[size];
    }
}

template<typename T>
T MaxValueInVector(const std::vector<T>& orig)
{
    if(orig.empty()) return 0;
    T maxVal = orig.front();
    for(size_t i = 1; i < orig.size();++i){
        if(maxVal < orig[i])
            maxVal = orig[i];
    }
    return maxVal;
}

template<typename T>
T MinValueInVector(const std::vector<T>& orig)
{
    if(orig.empty()) return 0;
    T minVal = orig.front();
    for(size_t i = 1; i < orig.size();++i){
        if(minVal > orig[i])
            minVal = orig[i];
    }
    return minVal;
}

template<typename T>
int Round(T val)
{
    //int a = int(val + 0.5 * (val > 0?  1 : -1));
    return int(val + 0.5 * (val > 0?  1 : -1));
}

template<typename T>
T ValueInMinMax(const T orig, T minVal, T maxVal){
    return std::min<T>(maxVal, std::max<T>(orig, minVal));
}

template<typename T>
void FindEqualList(const std::vector<T>& orig, T value, std::vector<int>& list){
    list.clear();
    for(size_t i = 0; i < orig.size();++i){
        if(orig[i] == value){
            list.push_back(i);
        }
    }
}

template<typename T>
void GetAreaSum(const Volume<T> &dst, const int xMin, const int xMax, const int yMin,
                const int yMax, const int zMin, const int zMax,
                int &value)
{
    value = 0;
    if(dst.x() < xMin + 1 || dst.y() < yMax +1 || dst.z() < zMax + 1) return;
    for (int i = xMin; i <= xMax; ++i)
        for (int j = yMin; j <= yMax; ++j)
            for ( int ij = zMin; ij <= zMax; ++ij){
                value += dst(i, j, ij);
            }
}

template<typename T>
void Conv3d(int *dst, const Volume<T> &src, const int nXVoxel, const int nYVoxel, const int nZVoxel)
{
    if(!dst) return;
    ///------------------初始化------------------------------
        int i,j,ij;
        int sum(0);
        int nx	= src.x();
        int ny	= src.y();
        int nz	= src.z();
        int nxy = nx * ny;

        /*根据数据类型来判断*/
        //dst is the same size as src
        for(int ii = 0; ii < src.x(); ++ii)
            for(int jj = 0; jj < src.y(); ++jj)
                for(int iijj = 0; iijj < src.z(); ++iijj)
                    dst[ii + jj * nx + iijj * nxy] = src(ii,jj,iijj);


        for (j = nYVoxel; j <= ny -nYVoxel - 1; ++j)
            for (i = nXVoxel; i <= nx - nXVoxel - 1; ++i)
                for (ij = nZVoxel; ij <= nz - nZVoxel - 1; ++ij)
                {
                    sum = 0;
                    GetAreaSum(src, i-nXVoxel, i+nXVoxel, j -nYVoxel, j +nYVoxel,
                        ij - nZVoxel, ij +nZVoxel, sum);
                    dst[i + j * nx + ij * nxy] = sum;
                }
}

void Get3DRegion(int &xMin, int& xMax, int& yMin, int &yMax, int &zMin, int& zMax,
                 const int xCenter, const int yCenter, const int zCenter,
                 const int xOffset, const int yOffset, const int zOffset);//warning!!

void Get3DRegion(int &xMin, int& xMax, int& yMin, int &yMax, int &zMin, int& zMax,
                 const int xCenter, const int yCenter, const int zCenter,
                 const int xOffset, const int yOffset, const int zOffset,
                 const int xlower, const int xupper,
                 const int ylower, const int yupper,
                 const int zlower, const int zupper );//warning!!

template<typename T>
bool IsAreaMaxValue(const Volume<T> &orig, const int xMin, const int xMax, const int yMin,
                    const int yMax, const int zMin, const int zMax,
                    const T &maxValue)
{
    bool isMax(true);
    //printf("%d\n", orig(45,34,13));
    for (int i = xMin; i <= xMax; ++i){
        for (int j = yMin; j <= yMax; ++j){
            for ( int ij = zMin; ij <= zMax; ++ij){
                int val = orig(i, j, ij);
                if ( val> maxValue){
                    isMax = false;
                    break;
                }
            }
            if (!isMax) break;
        }
        if (!isMax) break;
    }
    return isMax;
}

template<class T>
void SetAreaValue(Volume<T> &dst, const int xMin, const int xMax, const int yMin,
                  const int yMax, const int zMin, const int zMax,
                  const T &value)
{
    for (int i = xMin; i <= xMax; ++i){
        for (int j = yMin; j <= yMax; ++j){
            for ( int ij = zMin; ij <= zMax; ++ij){
                dst(i, j, ij) = value;
            }
        }
    }
}

template<class T>
void ExtractArea(const Volume<T> &orig, const int xMin, const int xMax, const int yMin,
                 const int yMax, const int zMin, const int zMax,
                 Volume<T> &dest)
{
    dest.SetSize(xMax - xMin + 1, yMax - yMin + 1, zMax - zMin + 1);
    for (int i = xMin; i <= xMax; ++i){
        for (int j = yMin; j <= yMax; ++j){
            for ( int ij = zMin; ij <= zMax; ++ij){
                dest(i - xMin, j - yMin, ij - zMin) = orig(i, j, ij);
            }
        }
    }
}

/*区域最大值*/
template<class T>
void GetAreaMaxValue(const Volume<T> &orig, const int xMin, const int xMax, const int yMin,
                     const int yMax, const int zMin, const int zMax,
                     T &maxValue)
{
    maxValue = 0;
    for (int i = xMin; i <= xMax; ++i){
        for (int j = yMin; j <= yMax; ++j){
            for ( int ij = zMin; ij <= zMax; ++ij){
                if (orig(i, j, ij) > maxValue)
                    maxValue = orig(i, j, ij);
            }
        }
    }
}

/*区域最小值*/
template<class T>
void GetAreaMinValue(const Volume<T> &orig, const int xMin, const int xMax, const int yMin,
                     const int yMax, const int zMin, const int zMax,
                     T &minValue)
{
    minValue = sizeof(T) == 1 ? 255 : 10000;

    for (int i = xMin; i <= xMax; ++i){
        for (int j = yMin; j <= yMax; ++j){
            for ( int ij = zMin; ij <= zMax; ++ij){
                if (orig(i, j, ij) < minValue)
                    minValue = orig(i, j, ij);
            }
        }
    }
}

void ExtractLocalDomain(const Vec3d &initPoint, const Volume<unsigned short> &origImg,
                        Volume<unsigned short> &locOrigImg, Vec3d &locPoint);


#endif // VOLUMEALGO_H
