 /*
 by guochanghao20190106
 */
#ifndef CLUSTERANALYSIS_20190518_H
#define CLUSTERANALYSIS_20190518_H

#include <iostream>
#include <cmath>
#include <fstream>
#include <iosfwd>
#include <math.h>
#include <vector>
#include "v3d_interface.h"

using namespace std;

//*************************************************define the class DataPoint

//*************************************************define the class DataPoint
#define DIME_NUM 3       //数据维度，全局常量


//数据点类型
class DataPoint
{
private:
    int DPdim;			//维度
    unsigned long dpID;                //数据点ID
    double *dimension;        //维度数据
    //long clusterId;                    //所属聚类ID
    bool isKey;                        //是否核心对象
    bool visited;                      //是否已访问
    vector<unsigned long> arrivalPoints;    //领域数据点id列表
public:
    long clusterId;
    long long dpID_after_cluster;  //聚类后的每个数据点ID
    DataPoint(int dim);                                                    //默认构造函数
    DataPoint(int dim,unsigned long dpID,double* dimension , bool isKey);    //构造函数

    unsigned long GetDpId();                //GetDpId方法
    long long GetDpId_after_cluster();   //GetDpId_after_cluster方法
    void SetDpId(unsigned long dpID);        //SetDpId方法
    void SetDpId_after_cluster(long long dpID_after_cluster);           //SetDpId_after_cluster方法
    double* GetDimension();                    //GetDimension方法
    void SetDimension(double* dimension);    //SetDimension方法
    bool IsKey();                            //GetIsKey方法
    void SetKey(bool isKey);                //SetKey方法
    bool isVisited();                        //GetIsVisited方法
    void SetVisited(bool visited);            //SetIsVisited方法
    long GetClusterId();                    //GetClusterId方法
    void SetClusterId(long classId);        //SetClusterId方法
    vector<unsigned long>& GetArrivalPoints();    //GetArrivalPoints方法
};

//*************************************************define the class ClusterAnalysis
//聚类分析类型
class ClusterAnalysis
{
private:
    vector<DataPoint> dataSets ;        //数据集合
	unsigned int dimNum;			//维度
	double radius;                    //半径
    unsigned long dataNum;            //数据数量
	unsigned int minPTs;            //邻域最小数据个数
    unsigned long MaxDpId_after_cluster;

	double GetDistance(DataPoint& dp1, DataPoint& dp2);                    //距离函数
    void SetArrivalPoints(DataPoint& dp);                                //设置data points的领域点列表
    void KeyPointCluster( unsigned long i, unsigned long clusterId );    //对data points领域内的点执行聚类操作
public:
	ClusterAnalysis(){}                    //默认构造函数
    bool Init(int dim);    //初始化操作
    bool Init(int dim, char* fileName, double radius, int minPTs);    //初始化操作
    bool Read_from_coordiante(vector<float> x_coordinate, vector<float> y_coordinate, double radius, int minPTs);
    bool Read_from_coordiante(vector<float> x_coordinate, vector<float> y_coordinate, vector<float> z_coordinate, double radius, int minPTs);
    bool Read_from_pointset(LandmarkList curlist, double radius, int minPTs);
    long long GetMaxDpId_after_cluster(vector<DataPoint> dataSets);//update by Guochanghao in 20190225
    unsigned long GetDataNum();
    int DoDBSCANRecursive();            //DBSCAN递归算法
	bool WriteToFile(char* fileName);    //将聚类结果写入文件
    LandmarkList get_clustered_curlist(LandmarkList &curlist);//update by Guochanghao in 20190225
    void find_CenterPoint_of_MaxPointSet(LocationSimple &p, LandmarkList curlist);//update by Guochanghao in 20190411


};


#endif // CLUSTERANALYSIS_20190518_H
