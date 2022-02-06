#ifndef BRANCHTREE_H
#define BRANCHTREE_H

#include "v3d_interface.h"
#include "op_neurontree.h"
#include "swc2mask.h"
#include "swc_convert.h"

#include "../jba/newmat11/newmatap.h"
#include "../jba/newmat11/newmatio.h"

#define zx_dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define PI 3.14157292653589793238

using namespace std;

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

struct PointFeature
{
    float x, y, z;
    int intensity;
    float linearity_3, linearity_5, linearity_8;
//    float pc1, pc2, pc3;
//    XYZ v1, v2, v3;
    int nearEdge;
    /**
     * @brief getFeature 获取点的特征
     * @param pdata 图像的一维数据
     * @param sz 图像的维度大小，x，y，z，channel
     * @param x 点的x坐标
     * @param y 点的y坐标
     * @param z 点的z坐标
     */
    void getFeature(unsigned char* pdata, V3DLONG* sz, float x, float y, float z);
};

struct LineFeature
{
    vector<PointFeature> pointsFeature;
    float intensity_mean, intensity_std;
    float intensity_mean_r5, intensity_std_r5;
    float linearity_3_mean, linearity_5_mean, linearity_8_mean;
    vector<XYZ> directions;

    /**
     * @brief intial 初始化
     */
    void intial();
};

struct Branch
{
    Branch* parent;
    vector<Branch*> children;
    int level;
    float length, distance;
    LineFeature line_feature;

    vector<NeuronSWC> points;


    Branch() : line_feature(), level(-1), length(0), distance(0) {
        parent = nullptr;
        children.clear();
        points.clear();
    }

    bool operator <(const Branch &other) const
    {
        if(level!=other.level)
            return (level>other.level);
        return (length<other.length);
    }

    /**
     * @brief getFeature 计算当前线的所有feature
     * @param pdata 图像的一维数据
     * @param sz 图像的维度大小，x，y，z，channel
     */
    void getFeature(unsigned char* pdata, V3DLONG* sz);

    /**
     * @brief splitByInflectionPoint 将branch按拐点分段
     * @param d 有效方向的最小长度
     * @param cosAngleThres 拐点处的角度阈值
     * @return 分的段，如果为分段，则返回本身
     */
    vector<Branch*> splitByInflectionPoint(float d, float cosAngleThres);

    /**
     * @brief splitByLength 将长的branch分成几段
     * @param l_thres branch的最长长度
     * @return
     */
    vector<Branch*> splitByLength(float l_thres);

    /**
     * @brief removePointsNearSoma 移除离soma近的点，小于soma半径乘以ratio的点则视为离soma近
     * @param soma soma点
     * @param ratio 比例
     */
    void removePointsNearSoma(const NeuronSWC& soma, float ratio);

    /**
     * @brief removeTerminalPoints 移除两端的点
     * @param d 移除的长度
     */
    void removeTerminalPoints(float d);

    /**
     * @brief calLength 计算branch的路径长度总合
     * @return ranch的路径长度总合
     */
    float calLength();

    /**
     * @brief calDistance 计算两个端点的直线距离
     * @return 两个端点的直线距离
     */
    float calDistance();

};

void adjustRalationForSplitBranches(Branch* origin, vector<Branch*> target);

void adjustRalationForRemoveBranch(Branch* origin);

struct BranchTree
{
    vector<Branch*> branches;
    bool hasSoma;
    NeuronSWC soma;

    /**
     * @brief initialize 初始化BranchTree 输入需要保证只有一个根节点
     * @param nt 神经元neuronTree
     * @return
     */
    bool initialize(NeuronTree& nt);

    /**
     * @brief splitByInflectionPoint 将branch按拐点分段
     * @param d 有效方向的最小长度
     * @param cosAngleThres 拐点处的角度阈值
     */
    void splitByInflectionPoint(float d, float cosAngleThres);

    /**
     * @brief splitByLength 将长的branch分成几段
     * @param l_thres branch的最长长度
     */
    vector<Branch*> splitByLength(float l_thres);

    void preProcess(float inflection_d, float cosAngleThres,
                    float l_thres_max, float l_thres_min,
                    float t_length, float soma_ratio);

    void getFeature(unsigned char* pdata, V3DLONG* sz);
};

#endif // BRANCHTREE_H
