#ifndef REGIONGROW_H
#define REGIONGROW_H

#include <vector>
#include "v3d_interface.h"

using namespace std;

struct point2d
{
    int x,y;
    point2d() {
        x = y = 0;
    }
    point2d(int x,int y){
        this->x = x;
        this->y = y;
    }
};

template<class T>
int regionGrow(T* inimg, unsigned char* &outFlag, long sz0, long sz1, int threshold, int smallObjSize){
    long sz01 = sz0*sz1;

    outFlag = new unsigned char[sz01];
    memset(outFlag,0,sz01*sizeof(unsigned char));

    int DIR[8][2] = {{-1,-1}, {0,-1}, {1,-1}, {1,0}, {1,1}, {0,1}, {-1,1}, {-1,0}};
    T* tmpimg = new T[sz01];
    for(int i=0; i<sz01; i++)
        tmpimg[i] = ((double)inimg[i]>threshold)?1:0;
    for(int i=0; i<sz01; i++){
        if(tmpimg[i]){
            int x = i%sz0;
            int y = i/sz0;
            bool onePoint = true;
            for(int j=0; j<8; j++){
                int tx = x + DIR[j][0];
                int ty = y + DIR[j][1];
                if(tx<0 || tx>sz0-1 || ty<0 || ty>sz1-1)
                    continue;
                int tIndex = ty*sz0 + tx;
                if(tmpimg[tIndex]){
                    onePoint = false;
                    break;
                }
            }
            if(onePoint){
                tmpimg[i] = 0;
            }
        }
    }

    vector<point2d> points;
    int flag = 1;
    for(int i=0; i<sz01; i++){
        if(tmpimg[i]){
            int x = i%sz0;
            int y = i/sz0;

            point2d p = point2d(x,y);
            vector<point2d> growPoints;
            growPoints.push_back(p);

            while (!growPoints.empty()) {
                point2d pt = growPoints.back();
                points.push_back(pt);
                growPoints.pop_back();
                for(int j=0; j<8; j++){
                    point2d pToGrowing;
                    pToGrowing.x = pt.x + DIR[j][0];
                    pToGrowing.y = pt.y + DIR[j][1];
                    if(pToGrowing.x<0 || pToGrowing.x>sz0-1 || pToGrowing.y<0 || pToGrowing.y>sz1-1)
                        continue;
                    int tIndex = pToGrowing.y*sz0 + pToGrowing.x;
                    if(tmpimg[tIndex]){
                        growPoints.push_back(pToGrowing);
                    }
                }
            }

            if(points.size()>=smallObjSize){
                for(int j=0; j<points.size(); j++){
                    point2d pt = points[j];
                    int ptIndex = pt.y*sz0 + pt.x;
                    outFlag[ptIndex] = flag;
                }
                flag++;
            }
            points.clear();

        }
    }

    if(tmpimg){
        delete[] tmpimg;
        tmpimg = 0;
    }

    return flag-1;

}

template <class T>
bool getSignals2d(T* inimg, long sz0, long sz1, vector<double> &c1, vector<double> &c2, int threshold, int smallObjSize){
    unsigned char* outFlag = 0;
    int count = regionGrow(inimg,outFlag,sz0,sz1,threshold,smallObjSize);

    if(count<1){
        if(outFlag){
            delete[] outFlag;
            outFlag = 0;
        }
        return true;
    }

    long sz01 = sz0*sz1;

    vector<double> ss;

    for(int i=1; i<=count; i++){
        c1.push_back(0);
        c2.push_back(0);
        ss.push_back(0);
    }

    for(int i=0; i<sz01; i++){
        int flag = outFlag[i];
        if(flag<=0)
            continue;
        int x = i%sz0;
        int y = i/sz0;
        c1[flag-1] += inimg[i]*x;
        c2[flag-1] += inimg[i]*y;
        ss[flag-1] += inimg[i];
    }

    for(int i=0; i<count; i++){
        if(ss[i]>0){
            c1[i] /= ss[i];
            c2[i] /= ss[i];
        }
    }

    if(outFlag){
        delete[] outFlag;
        outFlag = 0;
    }

    return true;

}

template<class T>
bool getSignals3d(T* inimg, long sz0, long sz1, long sz2, vector<XYZ> &points, int direction, int threshold, int smallObjSize){
    vector<double> c1,c2;
    long SZ0,SZ1;
    if(direction == 1 || direction == 2){
        SZ0 = sz0;
        SZ1 = sz1;
    }else if (direction == 3 || direction == 4) {
        SZ0 = sz0;
        SZ1 = sz2;
    }else if(direction == 5 || direction == 6){
        SZ0 = sz1;
        SZ1 = sz2;
    }

    getSignals2d(inimg,SZ0,SZ1,c1,c2,threshold,smallObjSize);

    switch (direction) {
    case 1:
        for(int i=0; i<c1.size(); i++){
            XYZ p = XYZ((int)(c1[i]+0.5),(int)(c2[i]+0.5),0);
            points.push_back(p);
        }
        break;
    case 2:
        for(int i=0; i<c1.size(); i++){
            XYZ p = XYZ((int)(c1[i]+0.5),(int)(c2[i]+0.5),sz2-1);
            points.push_back(p);
        }
        break;
    case 3:
        for(int i=0; i<c1.size(); i++){
            XYZ p = XYZ((int)(c1[i]+0.5),0,(int)(c2[i]+0.5));
            points.push_back(p);
        }
        break;
    case 4:
        for(int i=0; i<c1.size(); i++){
            XYZ p = XYZ((int)(c1[i]+0.5),sz1-1,(int)(c2[i]+0.5));
            points.push_back(p);
        }
        break;
    case 5:
        for(int i=0; i<c1.size(); i++){
            XYZ p = XYZ(0,(int)(c1[i]+0.5),(int)(c2[i]+0.5));
            points.push_back(p);
        }
        break;
    case 6:
        for(int i=0; i<c1.size(); i++){
            XYZ p = XYZ(sz0-1,(int)(c1[i]+0.5),(int)(c2[i]+0.5));
            points.push_back(p);
        }
        break;
    default:
        return false;
        break;
    }

}

#endif // REGIONGROW_H
