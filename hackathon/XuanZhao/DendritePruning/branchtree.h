#ifndef BRANCHTREE_H
#define BRANCHTREE_H

#include "v3d_interface.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include <vector>
#include <queue>
#include <algorithm>

#include "../jba/newmat11/newmatap.h"
#include "../jba/newmat11/newmatio.h"

#define zx_dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define PI 3.14157292653589793238

#define ToDeleteType 11
#define InFlectionType 12
#define AdjacentSomaType 13
#define BreakType 14


template <class T>
bool computeCubePcaEigVec(T* data1d, V3DLONG* sz,
                          V3DLONG x0, V3DLONG y0, V3DLONG z0,
                          V3DLONG wx, V3DLONG wy, V3DLONG wz,
                          double &pc1, double &pc2, double &pc3,
                          double *vec1, double *vec2, double *vec3){
    V3DLONG xb = x0 - wx; if(xb<0) xb = 0; if(xb >= sz[0]) xb = sz[0] - 1;
    V3DLONG xe = x0 + wx; if(xe<0) xe = 0; if(xe >= sz[0]) xe = sz[0] - 1;
    V3DLONG yb = y0 - wy; if(yb<0) yb = 0; if(yb >= sz[1]) yb = sz[1] - 1;
    V3DLONG ye = y0 + wy; if(ye<0) ye = 0; if(ye >= sz[1]) ye = sz[1] - 1;
    V3DLONG zb = z0 - wz; if(zb<0) zb = 0; if(zb >= sz[2]) zb = sz[2] - 1;
    V3DLONG ze = z0 + wz; if(ze<0) ze = 0; if(ze >= sz[2]) ze = sz[2] - 1;

    V3DLONG i,j,k;
    V3DLONG index;

    double w;

    //first get the center of mass
    double xm = 0, ym = 0, zm = 0, s = 0, mv = 0;
    for (k = zb; k <= ze; k++)
    {
        for (j = yb; j <= ye; j++)
        {
            for (i = xb; i <= xe; i++)
            {
                index = k*sz[0]*sz[1] + j*sz[0] + i;
                w = (double) data1d[index];
                xm += w*i;
                ym += w*j;
                zm += w*k;
                s += w;
            }
        }
    }

    if (s>0)
    {
        xm /= s; ym /= s; zm /= s;
        mv = s / ((double)((ze - zb + 1)*(ye - yb + 1)*(xe - xb + 1)));
    }
    else
    {
        printf("Sum of window pixels equals or is smaller than 0. The window is not valid or some other problems in the data. Do nothing.\n");
        return false;
    }

    double cc11 = 0, cc12 = 0, cc13 = 0, cc22 = 0, cc23 = 0, cc33 = 0;
    double dfx, dfy, dfz;
    for (k = zb; k <= ze; k++)
    {
        dfz = double(k) - zm;
        for (j = yb; j <= ye; j++)
        {
            dfy = double(j) - ym;
            for (i = xb; i <= xe; i++)
            {
                dfx = double(i) - xm;

                //                w = img3d[k][j][i]; //140128
                index = k*sz[0]*sz[1] + j*sz[0] + i;
                w = data1d[index] - mv;  if (w<0) w = 0; //140128 try the new formula

                cc11 += w*dfx*dfx;
                cc12 += w*dfx*dfy;
                cc13 += w*dfx*dfz;
                cc22 += w*dfy*dfy;
                cc23 += w*dfy*dfz;
                cc33 += w*dfz*dfz;
            }
        }
    }

    cc11 /= s; 	cc12 /= s; 	cc13 /= s; 	cc22 /= s; 	cc23 /= s; 	cc33 /= s;
//            if(count%1000 == 0){
//                qDebug()<<" cc11:"<<cc11<<" cc12:"<<cc12<<" cc13:"<<cc13<<" cc22:"<<cc22<<" cc23:"<<cc23<<" cc33:"<<cc33;
//            }



    try
    {
        //then find the eigen vector
        SymmetricMatrix Cov_Matrix(3);
        Cov_Matrix.Row(1) << cc11;
        Cov_Matrix.Row(2) << cc12 << cc22;
        Cov_Matrix.Row(3) << cc13 << cc23 << cc33;

        DiagonalMatrix DD;
        Matrix VV;
        EigenValues(Cov_Matrix, DD, VV);

        //output the result
        pc1 = DD(3);
        pc2 = DD(2);
        pc3 = DD(1);
//                qDebug()<<"pc1:"<<pc1<<"pc2:"<<pc2<<"pc3:"<<pc3;

        //output the vector
        for (int i = 1; i <= 3; i++){
            vec1[i-1] = VV(i, 3);
            vec2[i-1] = VV(i, 2);
            vec3[i-1] = VV(i, 1);
//                    qDebug()<<"vec1i:"<<vec1[i]<<"vec2i:"<<vec2[i]<<"vec3i:"<<vec3[i];
        }
    }catch (...)
    {
        pc1 = VAL_INVALID;
        pc2 = VAL_INVALID;
        pc3 = VAL_INVALID;
        return false;
    }

    return true;
}


struct BranchTree;

struct Branch
{
    BranchTree* bt;
    V3DLONG headPointIndex,endPointIndex;
    int parentIndex;
    vector<int> childrenIndex;

    int level, rLevel;
    float length, distance;
    float localAngle1,localAngle2,localAngle3;
    float globalAngle1,globalAngle2,globalAngle3;

    float intensityMean, intensityStd;

    Branch() : level(-1),rLevel(-1),length(0),distance(0),
        localAngle1(-1),localAngle2(-1),localAngle3(-1),intensityMean(0),intensityStd(0){
        childrenIndex.clear();
        bt = 0;
    }

    bool get_pointsIndex_of_branch(vector<V3DLONG> &points);
    bool get_r_pointsIndex_of_branch(vector<V3DLONG> &r_points);

    void setBranchType(int type, bool isBreak = false);

    float getBranchIntensity(unsigned char* pdata, V3DLONG* sz);
    float getBranchPcaValue(unsigned char* pdata, V3DLONG* sz, int r = 5);

    void findBranchInflectionPoint(double d, double cosAngleThres);
    bool checkBranchInflectionPoint(unsigned char *inimg1d, V3DLONG *sz);

    bool calAngle();

    double distToNeuronTree(NeuronTree* otherTree);

};

class BranchTree
{
public:

    NeuronTree nt;
    vector<Branch> branches;

    V3DLONG somaIndex;

    BranchTree(){
        branches.clear();
    }

    bool initialize(NeuronTree t);


    bool pruningByLength(unsigned char *pdata, long long *sz, int length);
    bool pruningSoma(double times);
    bool pruningAdjacentSoma(double somaRTh);
    bool pruningCross(double angleTh, double lengthTh);
    bool pruningInflectionPoints(unsigned char *inimg1d, V3DLONG *sz, double d, double cosAngleThres);


    bool getLevelIndex(vector<int> &levelIndex, int level);

    bool setChildenBranchType(int branchIndex, int type);
    bool setChildenBranchDeleteType(int branchIndex);
    bool setParentBranchRLevel(int branchIndex);

    XYZ getBranchLocalVector(vector<V3DLONG> pointsIndex, double d);

    void show(unsigned char* pdata, V3DLONG* sz);

    void calRlevel0Branches(unsigned char* pdata, V3DLONG* sz, ofstream &csvFile);


    bool update();
    bool saveNeuronTree(QString path);
    bool savePrunedNeuronTree(QString path);

};

#endif // BRANCHTREE_H
