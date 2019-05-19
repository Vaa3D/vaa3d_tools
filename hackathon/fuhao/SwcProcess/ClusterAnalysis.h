 /*
 原作者: DiamonJoy
 修改人：guochanghao20190106
 */
#ifndef __CLUSTERANALYSIS_H__
#define __CLUSTERANALYSIS_H__

#include <iostream>
#include <cmath>
#include <fstream>
#include <iosfwd>
#include <math.h>
#include <vector>
#include "v3d_interface.h"

using namespace std;

//*************************************************define the class DataPoint
#define DIME_NUM 3       //数据维度，全局常量

//数据点类型
class DataPoint
{
private:
    unsigned long dpID;                //数据点ID
    double dimension[DIME_NUM];        //维度数据
    //long clusterId;                    //所属聚类ID
    bool isKey;                        //是否核心对象
    bool visited;                      //是否已访问
    vector<unsigned long> arrivalPoints;    //领域数据点id列表
public:
    long clusterId;
    unsigned long dpID_after_cluster;  //聚类后的每个数据点ID
    DataPoint(){}                                                    //默认构造函数
    DataPoint(unsigned long dpID,double* dimension , bool isKey)
    {
        //传递每维的维度数据
        for(int i=0; i<DIME_NUM;i++)
        {
            this->dimension[i]=dimension[i];
        }
    }
    //构造函数

    unsigned long GetDpId(){
        return this->dpID;
    }
                //GetDpId方法
    unsigned long GetDpId_after_cluster(){
        return this->dpID_after_cluster;
    }   //GetDpId_after_cluster方法
    void SetDpId(unsigned long dpID){
        this->dpID = dpID;
    }        //SetDpId方法
    void SetDpId_after_cluster(unsigned long dpID_after_cluster){
        this->dpID_after_cluster = dpID_after_cluster;
    }           //SetDpId_after_cluster方法
    double* GetDimension()
    {
        return this->dimension;
    }
    //GetDimension方法
    void SetDimension(double* dimension)
    {
        for(int i=0; i<DIME_NUM;i++)
        {
            this->dimension[i]=dimension[i];
        }
    }    //SetDimension方法
    bool IsKey(){
        return this->isKey;
    }                            //GetIsKey方法
    void SetKey(bool isKey){
        this->isKey = isKey;
    }                //SetKey方法
    bool isVisited(){
        return this->visited;
    }                        //GetIsVisited方法
    void SetVisited(bool visited){
        this->visited = visited;
    }            //SetIsVisited方法
    long GetClusterId(){
        return this->clusterId;
    }                    //GetClusterId方法
    void SetClusterId(long classId){
        this->clusterId = clusterId;
    }        //SetClusterId方法
    vector<unsigned long>& GetArrivalPoints(){
        return arrivalPoints;
    }    //GetArrivalPoints方法
};

//GetArrivalPoints方法
//vector<unsigned long>& DataPoint::GetArrivalPoints()

//*************************************************class DataPoint over


//*************************************************define the class ClusterAnalysis
//聚类分析类型
class ClusterAnalysis
{
private:
    vector<DataPoint> dataSets ;        //数据集合
	unsigned int dimNum;			//维度
	double radius;                    //半径
	unsigned int dataNum;            //数据数量
	unsigned int minPTs;            //邻域最小数据个数
	
        double GetDistance(DataPoint& dp1, DataPoint& dp2){
            double distance =0;        //初始化距离为0
            for(int i=0; i<DIME_NUM;i++)    //对数据每一维数据执行
            {
                    distance += pow(dp1.GetDimension()[i] - dp2.GetDimension()[i],2);    //距离+每一维差的平方
            }
            return pow(distance,0.5);        //开方并返回距离
    }                    //距离函数
    void SetArrivalPoints(DataPoint& dp){
        for(unsigned long i=0; i<dataNum; i++)                //对每个data points执行
            {
            double distance =GetDistance(dataSets [i], dp);//获取与特定点之间的距离
                    //cout << dp.GetDpId()<<"to"<<i<<"is"<<distance<< endl;
                    if(distance <= radius && i!=dp.GetDpId())        //若距离小于半径，并且特定点的id与dp的id不同执行
                dp.GetArrivalPoints().push_back(i);            //将特定点id压入dp的领域列表中
        }
    if(dp.GetArrivalPoints().size() >= minPTs)            //若dp领域内data points数据量> minPTs执行
            {
                    dp.SetKey(true);    //将dp核心对象标志位设为true
                    return;                //返回
            }
            dp.SetKey(false);    //若非核心对象，则将dp核心对象标志位设为false
    }
                                //设置data points的领域点列表
    void KeyPointCluster( unsigned long dpID, unsigned long clusterId  ){
        DataPoint& srcDp = dataSets [dpID];        //获取data points对象
            if(!srcDp.IsKey())    return;
            vector<unsigned long>& arrvalPoints = srcDp.GetArrivalPoints();        //获取对象领域内点ID列表
            for(unsigned long i=0; i<arrvalPoints.size(); i++)
            {
            DataPoint& desDp = dataSets [arrvalPoints[i]];    //获取领域内点data points
                    if(!desDp.isVisited())                            //若该对象没有被访问过执行
                    {
                //cout << "data points"<< desDp.GetDpId()<<" cluster ID is " <<clusterId << endl;
                            desDp.SetClusterId(clusterId);        //设置该对象所属簇的ID为clusterId，即将该对象吸入簇中
                desDp.SetVisited(true);               //设置该对象已被访问
                            if(desDp.IsKey())                    //若该对象是核心对象
                            {
                    desDp.dpID_after_cluster=GetMaxDpId_after_cluster(dataSets);
                                    KeyPointCluster(desDp.GetDpId(),clusterId);    //递归地对该领域点数据的领域内的点执行聚类操作，采用深度优先方法
                            }
                    }
            }
    }    //对data points领域内的点执行聚类操作
public:
    unsigned long MaxDpId_after_cluster;
    ClusterAnalysis(){}                    //默认构造函数
    bool Init(char* fileName, double radius, int minPTs)

    {
            this->radius = radius;        //设置半径
            this->minPTs = minPTs;        //设置领域最小数据个数
            this->dimNum = DIME_NUM;    //设置数据维度
            ifstream ifs(fileName);        //打开文件
            if (! ifs.is_open())                //若文件已经被打开，报错误信息
            {
                    cout << "Error opening file\n";    //输出错误信息
                    exit (-1);                        //程序退出
            }

            unsigned long i=0;            //数据个数统计
            while (! ifs.eof() )                //从文件中读取POI信息，将POI信息写入POI列表中
            {
            DataPoint tempDP;                //临时data points对象
            double tempDimData[DIME_NUM];    //临时data points维度信息
                    for(int j=0; j<DIME_NUM; j++)    //读文件，读取每一维数据
                    {
                            ifs>>tempDimData[j];
                    }
            tempDP.SetDimension(tempDimData);    //将维度信息存入data points对象内
            tempDP.SetDpId(i);                    //将data points对象ID设置为i
            tempDP.SetVisited(false);            //data points对象isVisited设置为false
                    tempDP.SetClusterId(-1);            //设置默认簇ID为-1
            tempDP.SetDpId_after_cluster(-1);   //设置默认聚类后的点的ID为-1
            dataSets.push_back(tempDP);            //将对象压入数据集合容器
                    i++;        //计数+1
            }
            ifs.close();        //关闭文件流
            dataNum =i;            //设置数据对象集合大小为i
            for(unsigned long k=0; k<dataNum;k++)
            {
            SetArrivalPoints(dataSets [k]);            //计算data points领域内对象
            }
            return true;    //返回
    }
        //初始化操作
//    bool Read_from_coordiante(vector<float> x_coordinate, vector<float> y_coordinate,double radius, int minPTs)
//    {
//        dataSets.clear();
//        this->radius = radius;        //设置半径
//        this->minPTs = minPTs;        //设置领域最小数据个数
//        this->dimNum = DIME_NUM;    //设置数据维度

//        unsigned long i=0;            //数据个数统计
//        for (int num = 0; num < x_coordinate.size(); num++)
//        {
//            DataPoint tempDP;                //临时data points对象
//            double tempDimData[DIME_NUM];    //临时data points维度信息

//            if(DIME_NUM==2)
//            {
//                tempDimData[0] = x_coordinate.at(num);
//                tempDimData[1] = y_coordinate.at(num);
//            }
//            if(DIME_NUM==3)
//            {
//                //Temporarily not needed


////                tempDimData[0] = curlist[num].x;
////                tempDimData[1] = curlist[num].y;
////                tempDimData[2] = curlist[num].z;
//            }

//            tempDP.SetDimension(tempDimData);    //将维度信息存入data points对象内
//            tempDP.SetDpId(i);                    //将data points对象ID设置为i
//            tempDP.SetVisited(false);            //data points对象isVisited设置为false
//            tempDP.SetClusterId(-1);            //设置默认簇ID为-1
//            dataSets.push_back(tempDP);            //将对象压入数据集合容器
//            i++;        //计数+1

//        }
//        dataNum =i;            //设置数据对象集合大小为i
//        for(unsigned long k=0; k<dataNum;k++)
//        {
//            SetArrivalPoints(dataSets[k]);            //计算data points领域内对象
//        }
//        return true;    //返回
//    }

    bool Read_from_curlist(LandmarkList curlist,double radius, int minPTs){
        dataSets.clear();
        this->radius = radius;        //设置半径
        this->minPTs = minPTs;        //设置领域最小数据个数
        this->dimNum = DIME_NUM;    //设置数据维度

        unsigned long i=0;            //数据个数统计
        for (int num = 0; num < curlist.size(); num++)
        {
            DataPoint tempDP;                //临时data points对象
            double tempDimData[DIME_NUM];    //临时data points维度信息

            if(DIME_NUM==2)
            {
                tempDimData[0] = curlist.at(num).x;
                tempDimData[1] = curlist.at(num).y;
            }
            if(DIME_NUM==3)
            {
                //Temporarily not needed

                tempDimData[0] = curlist[num].x;
                tempDimData[1] = curlist[num].y;
                tempDimData[2] = curlist[num].z;
            }

            tempDP.SetDimension(tempDimData);    //将维度信息存入data points对象内
            tempDP.SetDpId(i);                    //将data points对象ID设置为i
            tempDP.SetVisited(false);            //data points对象isVisited设置为false
            tempDP.SetClusterId(-1);            //设置默认簇ID为-1
            dataSets.push_back(tempDP);            //将对象压入数据集合容器
            i++;        //计数+1

        }
        dataNum =i;            //设置数据对象集合大小为i
        for(unsigned long k=0; k<dataNum;k++)
        {
            SetArrivalPoints(dataSets[k]);            //计算data points领域内对象
        }
        return true;    //返回
    }
    unsigned long GetMaxDpId_after_cluster(vector<DataPoint> dataSets){
        unsigned long max=dataSets[0].dpID_after_cluster;
        for(unsigned long i = 0; i < dataNum; i++)
            if(max<dataSets[i].GetDpId_after_cluster())max=dataSets[i].GetDpId_after_cluster();

        MaxDpId_after_cluster = max;
        return max;
    }
    unsigned long GetDataNum(){
        return this->dataNum;
    }
    int DoDBSCANRecursive(){
        unsigned long clusterId=0, k = 0;                        //聚类id计数，初始化为0

    for(unsigned long i=0; i<dataNum;i++)            //对每一个data points执行
        {
        DataPoint& dp=dataSets[i];                    //取到第i个data points对象
                if(!dp.isVisited() && dp.IsKey())            //若对象没被访问过，并且是核心对象执行
                {
            dp.dpID_after_cluster=GetMaxDpId_after_cluster(dataSets)+1;
                        dp.SetClusterId(clusterId);                //设置该对象所属簇ID为clusterId
                        dp.SetVisited(true);                    //设置该对象已被访问过
                        KeyPointCluster(i,clusterId);            //对该对象领域内点进行聚类
                        clusterId++;                            //clusterId自增1
                }
        if(!dp.IsKey())
        {
//            cout <<"noise points"<<i<<endl;
            k++;
        }

        }
//    for(unsigned long j=0; j<dataNum;j++)
//    {
//        cout<<"DpId:"<<dataSets[j].GetDpId()<<"\t";
//        cout<<"clusterId:"<<dataSets[j].clusterId<<"\t";
//        cout<<"DpId_after_cluster"<<dataSets[j].GetDpId_after_cluster()<<endl;
//    }

//    cout <<"cluster number:" <<clusterId<<endl;       //算法完成后，输出聚类个数
//    cout <<"noise points:" <<k<<endl;
    return clusterId;    //返回
}            //DBSCAN递归算法
    bool WriteToFile(char* fileName){
        ofstream of1(fileName);//初始化文件输出流
        for(int d=1; d<=DIME_NUM ; d++)						//将维度信息写入文件
        {
                of1<<"数据"<<d<<'\t';
                if (d==DIME_NUM) of1<<"所属簇ID"<<'\t'<<endl;
        }
    for(unsigned long i=0; i<dataNum;i++)                //对处理过的每个data points写入文件
        {
                for(int d=0; d<DIME_NUM ; d++)                    //将维度信息写入文件
            of1<<dataSets [i].GetDimension()[d]<<'\t';
        if(dataSets [i].GetClusterId() != -1) of1 << dataSets [i].GetClusterId() <<endl;        //将所属簇ID写入文件
        else of1 <<"noise points"<<endl;
        }
    of1.close();    //关闭输出文件流
    cout<<"resultfile write success!"<< endl;
        return true;    //返回
}    //将聚类结果写入文件
    LandmarkList get_clustered_curlist(LandmarkList curlist){
        LocationSimple s;
        LandmarkList new_curlist;
        RGBA8 DarkOrange; DarkOrange.r=255; DarkOrange.g=140; DarkOrange.b=0;
        for(unsigned long i = 0; i < dataNum; i++)
        {
            if(dataSets[i].GetDpId_after_cluster()==0)
            {
                //所有的噪点不聚类
                s.x = curlist[i].x;
                s.y = curlist[i].y;
                s.z = curlist[i].z;
                s.radius = 1;
                s.color = DarkOrange;
                new_curlist<<s;
            }
        }

        for(unsigned long i = 1; i <= MaxDpId_after_cluster; i++)
        {
            int sum_x = 0;
            int sum_y = 0;
            int sum_z = 0;

            unsigned long count = 0;
            for(unsigned long j = 0; j < dataNum; j++)
            {

                //聚合方式:同一类的点，聚合为几何中心
                if(dataSets[j].GetDpId_after_cluster()==i)
                {
                    count++;
                    sum_x+=curlist[j].x;
                    sum_y+=curlist[j].y;
                    sum_z+=curlist[j].z;
                }
            }
            if(count==0)
            {
    //            cout<<"it is error"<<endl;
                return curlist;
            }
            s.x = sum_x/count;
            s.y = sum_y/count;
            s.z = sum_z/count;
            s.radius = 1;
            s.color = DarkOrange;
            new_curlist<<s;
    //        cout<<"max:"<<MaxDpId_after_cluster<<endl;

        }
        for(unsigned long i = 0; i < dataNum; i++)
        {
    //    cout<<"DpId_after_cluster:"<<dataSets[i].GetDpId_after_cluster()<<endl;
        }
        return new_curlist;
    }
    LandmarkList return_labeled_curlist(LandmarkList curlist){
        LocationSimple s;
        LandmarkList new_curlist;
//        RGBA8 DarkOrange; DarkOrange.r=255; DarkOrange.g=140; DarkOrange.b=0;
        RGBA8 temp_point_color=RGBA8::random_rgb8();
        int MaxDpId;
        MaxDpId=MaxDpId_after_cluster+1;

        for(unsigned long i = 0; i < dataNum; i++)
        {
            if(dataSets[i].GetDpId_after_cluster()==0)
            {
                //所有的噪点不聚类
                s.x = curlist[i].x;
                s.y = curlist[i].y;
                s.z = curlist[i].z;
                s.radius = 1;
                s.color = temp_point_color;
                s.comments=QString::number(MaxDpId).toStdString();
                s.name=QString::number(i).toStdString();
                MaxDpId++;

                new_curlist<<s;
            }
            else
            {
                s.x = curlist[i].x;
                s.y = curlist[i].y;
                s.z = curlist[i].z;
                s.radius = 1;
                s.color = temp_point_color;
                s.comments=QString::number( dataSets[i].GetDpId_after_cluster()).toStdString();
                s.name=QString::number(i).toStdString();
                new_curlist<<s;

            }
        }
        return new_curlist;
    }
};

#endif
