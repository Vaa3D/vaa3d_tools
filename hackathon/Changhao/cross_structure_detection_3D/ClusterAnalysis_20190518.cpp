 /*
by guochanghao20190106
 */

#include "ClusterAnalysis_20190518.h"


//默认构造函数
DataPoint::DataPoint(int dim)
{
    DPdim = dim;
    this->dimension=new double[DPdim];
}

//构造函数
DataPoint::DataPoint(int dim, unsigned long dpID,double* dimension , bool isKey):isKey(isKey),dpID(dpID)
{
    //传递每维的维度数据
    for(int i=0; i<dim;i++)
    {
        this->dimension=new double[DPdim];
    }
}

//设置维度数据
void DataPoint::SetDimension(double* dimension)
{
    for(int i=0; i<DPdim;i++)
    {
        this->dimension[i]=dimension[i];
    }
}

//获取维度数据
double* DataPoint::GetDimension()
{
    return this->dimension;
}

//获取是否为核心对象
bool DataPoint::IsKey()
{
    return this->isKey;
}

//设置核心对象标志
void DataPoint::SetKey(bool isKey)
{
    this->isKey = isKey;
}

//获取DpId方法
unsigned long DataPoint::GetDpId()
{
    return this->dpID;
}

//获取DpId_after_cluster方法
long long DataPoint::GetDpId_after_cluster()
{
    return this->dpID_after_cluster;
}

//设置DpId方法
void DataPoint::SetDpId(unsigned long dpID)
{
    this->dpID = dpID;
}
//设置DpId_after_cluster方法
void DataPoint::SetDpId_after_cluster(long long dpID_after_cluster)
{
    this->dpID_after_cluster = dpID_after_cluster;
}

//GetIsVisited方法
bool DataPoint::isVisited()
{
    return this->visited;
}


//SetIsVisited方法
void DataPoint::SetVisited( bool visited )
{
    this->visited = visited;
}

//GetClusterId方法
long DataPoint::GetClusterId()
{
    return this->clusterId;
}

//GetClusterId方法
void DataPoint::SetClusterId( long clusterId )
{
    this->clusterId = clusterId;
}

//GetArrivalPoints方法
vector<unsigned long>& DataPoint::GetArrivalPoints()
{
    return arrivalPoints;
}

/******************************************************************************/
/******************************************************************************/

unsigned long ClusterAnalysis::GetDataNum()
{
    return this->dataNum;
}

long long ClusterAnalysis::GetMaxDpId_after_cluster(vector<DataPoint> dataSets)   //GetMaxDpId_after_cluster方法
{
    long long max=dataSets.at(0).GetDpId_after_cluster();
    for(unsigned long i = 0; i < dataNum; i++)
        if(max<dataSets.at(i).GetDpId_after_cluster())max=dataSets.at(i).GetDpId_after_cluster();

    MaxDpId_after_cluster = max;
    return max;
}


/*
函数：初始化操作
说明：
参数：
返回值： true;    */
bool ClusterAnalysis::Init(int dim)
{
    dimNum = dim;
    dataSets.clear();
    return true;
}

/*
函数：聚类初始化操作
说明：将数据文件名，半径，领域最小数据个数信息写入聚类算法类，读取文件，把数据信息读入写进算法类数据集合中
参数：
char* fileName;    //文件名
double radius;    //半径
int minPTs;        //领域最小数据个数
返回值： true;    */
bool ClusterAnalysis::Init(int dim, char* fileName, double radius, int minPTs)
{
    this->radius = radius;        //设置半径
    this->minPTs = minPTs;        //设置领域最小数据个数
    this->dimNum = dim;    //设置数据维度
    ifstream ifs(fileName);        //打开文件
    if (! ifs.is_open())                //若文件已经被打开，报错误信息
    {
        cout << "Error opening file\n";    //输出错误信息
        exit (-1);                        //程序退出
    }

    unsigned long i=0;            //数据个数统计
    while (! ifs.eof() )                //从文件中读取POI信息，将POI信息写入POI列表中
    {
        DataPoint tempDP(dimNum);                //临时data points对象
        double *tempDimData = new double[dimNum];    //临时data points维度信息
        for(int j=0; j<dimNum; j++)    //读文件，读取每一维数据
        {
            ifs>>tempDimData[j];
        }
        tempDP.SetDimension(tempDimData);    //将维度信息存入data points对象内
        tempDP.SetDpId(i);                    //将data points对象ID设置为i
        tempDP.SetVisited(false);            //data points对象isVisited设置为false
        tempDP.SetClusterId(-1);            //设置默认簇ID为-1
        dataSets.push_back(tempDP);            //将对象压入数据集合容器
        i++;        //计数+1
    }
    ifs.close();        //关闭文件流
    dataNum =i;            //设置数据对象集合大小为i
    for(unsigned long k=0; k<dataNum;k++)
    {
        SetArrivalPoints(dataSets[k]);            //计算data points领域内对象
    }
    return true;    //返回
}


/*
函数：从想XY坐标coordiante获取要聚类的点
说明：将坐标，半径，领域最小数据个数信息写入聚类算法类，读取点，把数据信息读入写进算法类数据集合中
参数：
x_coordinate;    //x坐标
y_coordinate;    //y坐标
double radius;    //半径
int minPTs;       //领域最小数据个数
返回值： true;    */
bool ClusterAnalysis::Read_from_coordiante(vector<float> x_coordinate, vector<float> y_coordinate, double radius, int minPTs)
{
    if(!x_coordinate.size()||!y_coordinate.size())
    {
        v3d_msg(QString("Input is empty!"),0);
        return false;
    }
    if(x_coordinate.size()!=y_coordinate.size())
    {
        v3d_msg(QString("Input location dim is not equal!"));
        return false;
    }
    dataSets.clear();
    this->radius = radius;        //设置半径
    this->minPTs = minPTs;        //设置领域最小数据个数
//    this->dimNum = DIME_NUM;    //设置数据维度

    unsigned long i=0;            //数据个数统计
    for (int num = 0; num < x_coordinate.size(); num++)
    {
        DataPoint tempDP(dimNum);                //临时data points对象
        double *tempDimData= new double[dimNum];    //临时data points维度信息

        if(dimNum==2)
        {
            tempDimData[0] = x_coordinate.at(num);
            tempDimData[1] = y_coordinate.at(num);
        }
        if(dimNum==3)
        {
            v3d_msg(QString("Warning: the input data dim is 2D. Maybe you need input (x,y,z) 3D data to use this function."));
            return false;
        }

        tempDP.SetDimension(tempDimData);    //将维度信息存入data points对象内
        tempDP.SetDpId(i);                    //将data points对象ID设置为i
        tempDP.SetVisited(false);            //data points对象isVisited设置为false
        tempDP.SetClusterId(-1);            //设置默认簇ID为-1
        tempDP.SetDpId_after_cluster(0);
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

/*
函数：从想XY坐标coordiante获取要聚类的点
说明：将坐标，半径，领域最小数据个数信息写入聚类算法类，读取点，把数据信息读入写进算法类数据集合中
参数：
x_coordinate;    //x坐标
y_coordinate;    //y坐标
z_coordinate;    //z坐标
double radius;    //半径
int minPTs;        //领域最小数据个数
返回值： true;    */
bool ClusterAnalysis::Read_from_coordiante(vector<float> x_coordinate, vector<float> y_coordinate, vector<float> z_coordinate, double radius, int minPTs)
{
    if(!x_coordinate.size()||!y_coordinate.size()||!z_coordinate.size())
    {
        v3d_msg(QString("Input is empty!"),0);
        return false;
    }
    if((x_coordinate.size()!=y_coordinate.size())||(x_coordinate.size()!=z_coordinate.size())||(y_coordinate.size()!=z_coordinate.size()))
    {
        v3d_msg(QString("Input location dim is not equal!"));
        return false;
    }
    if(radius<=0||minPTs<0)
    {
        v3d_msg(QString("Input parameters error! Please check input radius or minPTs."));
        return false;
    }
    dataSets.clear();
    this->radius = radius;        //设置半径
    this->minPTs = minPTs;        //设置领域最小数据个数
//    this->dimNum = DIME_NUM;    //设置数据维度

    unsigned long i=0;            //数据个数统计
    for (int num = 0; num < x_coordinate.size(); num++)
    {
        DataPoint tempDP(dimNum);                //临时data points对象
        double *tempDimData= new double[dimNum];    //临时data points维度信息

        if(dimNum==2)
        {
            v3d_msg(QString("Warning: the input data dim is 3D. This function will only deal with the x and y data"));
            tempDimData[0] = x_coordinate.at(num);
            tempDimData[1] = y_coordinate.at(num);
        }
        if(dimNum==3)
        {
            tempDimData[0] = x_coordinate.at(num);
            tempDimData[1] = y_coordinate.at(num);
            tempDimData[2] = z_coordinate.at(num);
        }

        tempDP.SetDimension(tempDimData);    //将维度信息存入data points对象内
        tempDP.SetDpId(i);                    //将data points对象ID设置为i
        tempDP.SetVisited(false);            //data points对象isVisited设置为false
        tempDP.SetClusterId(-1);            //设置默认簇ID为-1
        tempDP.SetDpId_after_cluster(0);
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

/*
函数：从curlist获取要聚类的点
说明：将curlist，半径，领域最小数据个数信息写入聚类算法类，读取点，把数据信息读入写进算法类数据集合中
参数：
LandmarkList curlist;    //点集
double radius;    //半径
int minPTs;        //领域最小数据个数
返回值： true;    */
bool ClusterAnalysis::Read_from_pointset(LandmarkList curlist, double radius, int minPTs)
{
    if(!curlist.size())
    {
        v3d_msg(QString("Input is empty! Please check input curlist."),0);
        return false;
    }
    if(radius<=0||minPTs<0)
    {
        v3d_msg(QString("Input parameters error! Please check input radius or minPTs."));
        return false;
    }
    dataSets.clear();
    this->radius = radius;        //设置半径
    this->minPTs = minPTs;        //设置领域最小数据个数

    unsigned long i=0;            //数据个数统计
    for (int num = 0; num < curlist.size(); num++)
    {
        DataPoint tempDP(dimNum);                //临时data points对象
        double *tempDimData = new double[dimNum];    //临时data points维度信息

        if(dimNum==2)
        {
            tempDimData[0] = curlist.at(num).x;
            tempDimData[1] = curlist.at(num).y;
        }
        if(dimNum==3)
        {
            tempDimData[0] = curlist.at(num).x;
            tempDimData[1] = curlist.at(num).y;
            tempDimData[2] = curlist.at(num).z;
        }

        tempDP.SetDimension(tempDimData);    //将维度信息存入data points对象内
        tempDP.SetDpId(i);                    //将data points对象ID设置为i
        tempDP.SetVisited(false);            //data points对象isVisited设置为false
        tempDP.SetClusterId(-1);            //设置默认簇ID为-1
        tempDP.SetDpId_after_cluster(0);
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

/*
函数：将已经过聚类算法处理的数据集合写回文件
说明：将已经过聚类结果写回文件
参数：
char* fileName;    //要写入的文件名
返回值： true    */
bool ClusterAnalysis::WriteToFile(char* fileName )
{
	ofstream of1(fileName);//初始化文件输出流
    for(int d=1; d<=dimNum ; d++)						//将维度信息写入文件
	{
		of1<<"数据"<<d<<'\t';
        if (d==dimNum) of1<<"所属簇ID"<<'\t'<<endl;
	}
    for(unsigned long i=0; i<dataNum;i++)                //对处理过的每个data points写入文件
	{
        for(int d=0; d<dimNum ; d++)                    //将维度信息写入文件
            of1<<dataSets.at(i).GetDimension()[d]<<'\t';
        if(dataSets.at(i).GetClusterId() != -1) of1 << dataSets.at(i).GetClusterId() <<endl;        //将所属簇ID写入文件
        else of1 <<"noise points"<<endl;
	}
    of1.close();    //关闭输出文件流
    cout<<"resultfile write success!"<< endl;
	return true;    //返回
}

/*
函数：设置data points的领域点列表
说明：设置data points的领域点列表
参数：
返回值： true;    */
void ClusterAnalysis::SetArrivalPoints(DataPoint& dp)
{
    for(unsigned long i=0; i<dataNum; i++)                //对每个data points执行
	{
        double distance =GetDistance(dataSets.at(i), dp);//获取与特定点之间的距离
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


/*
函数：执行聚类操作
说明：执行聚类操作
参数：无
返回值： true;    */
int ClusterAnalysis::DoDBSCANRecursive()
{
	unsigned long clusterId=0, k = 0;                        //聚类id计数，初始化为0

    for(unsigned long i=0; i<dataNum;i++)            //对每一个data points执行
	{
        DataPoint& dp=dataSets.at(i);                    //取到第i个data points对象
		if(!dp.isVisited() && dp.IsKey())            //若对象没被访问过，并且是核心对象执行
		{
            dp.SetDpId_after_cluster(GetMaxDpId_after_cluster(dataSets)+1);
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
}

/*
函数：对data points领域内的点执行聚类操作
说明：采用递归的方法，深度优先聚类数据
参数：
unsigned long dpID;            //data pointsid
unsigned long clusterId;    //data points所属簇id
返回值： void;    */
void ClusterAnalysis::KeyPointCluster(unsigned long dpID, unsigned long clusterId )
{
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
}

//两data points之间距离
/*
函数：获取两data points之间距离
说明：获取两data points之间的欧式距离
参数：
DataPoint& dp1;        //data points1
DataPoint& dp2;        //data points2
返回值： double;    //两点之间的距离        */
double ClusterAnalysis::GetDistance(DataPoint& dp1, DataPoint& dp2)
{
	double distance =0;        //初始化距离为0
    for(int i=0; i<dimNum;i++)    //对数据每一维数据执行
	{
		distance += pow(dp1.GetDimension()[i] - dp2.GetDimension()[i],2);    //距离+每一维差的平方
	}
	return pow(distance,0.5);        //开方并返回距离
}

/*
函数：获得聚类后的新点集
说明：将聚类后的新点集复制给new_curlist
参数：LandmarkList &curlist; 需聚类点集
返回值： true;    */
LandmarkList ClusterAnalysis::get_clustered_curlist(LandmarkList &curlist)
{
    LocationSimple s;
    LandmarkList new_curlist; new_curlist.clear();
    RGBA8 DarkOrange; DarkOrange.r=255; DarkOrange.g=140; DarkOrange.b=0;
    RGBA8 tem_color; tem_color.r=0; tem_color.g=0; tem_color.b=255;

    if(dataNum==0)
    {
        v3d_msg(QString("Warning: count==0 in get_clustered_curlist!"),0);
        s.inputProperty = pxUnknown;//pxUnknown=0,so use 'if(p.inputProperty)' to judge the validation of the point
    }
    else
    {
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
            float sum_x = 0;
            float sum_y = 0;
            float sum_z = 0;

            unsigned long count = 0;
            tem_color.g += 50;
            for(unsigned long j = 0; j < dataNum; j++)
            {
                //聚合方式:同一类的点，聚合为几何中心
                if(dataSets[j].GetDpId_after_cluster()==i)
                {
                    count++;
                    sum_x+=curlist[j].x;
                    sum_y+=curlist[j].y;
                    sum_z+=curlist[j].z;
                    curlist[j].color = tem_color;//这个操作会改变原始点颜色,让不同类的点颜色不一样，同一类的点颜色变为同种颜色
                }
            }

            s.x = sum_x/count;
            s.y = sum_y/count;
            s.z = sum_z/count;
            s.radius = 1;
            s.color = DarkOrange;
            new_curlist<<s;

    //        cout<<"max:"<<MaxDpId_after_cluster<<endl;
        }
    }
//    for(unsigned long i = 0; i < dataNum; i++)
//    {
//    cout<<"DpId_after_cluster:"<<dataSets[i].GetDpId_after_cluster()<<endl;
//    }
    return new_curlist;
}

//*************************************************class ClusterAnalysis over


/*
函数：找到一个点p的周围点集(A、B、C...)点数最大的那个点集M
说明：获取两data points之间的欧式距离
参数：vector<int> pointNumber;//里面存的是每个点集的点数
     int max;//点集中的最大点数
     int max_ind;//最大点数max在pointNumber中的索引，有max==pointNumber.at(max_ind)
     输入一个点的周围采样点集curlist，得到点集中采样点聚类后点数最多的那一类的几何中心点p
     若p.inputProperty == pxLocaUseful 则输出点p有效
返回值： LocationSimple;    //M的几何中心点        */

void ClusterAnalysis::find_CenterPoint_of_MaxPointSet(LocationSimple &p, LandmarkList curlist)
{
    RGBA8 DarkOrange; DarkOrange.r=255; DarkOrange.g=140; DarkOrange.b=0;
    RGBA8 green; green.r=0; green.g=51; green.b=0;

    if(dataNum==0)
    {
        v3d_msg(QString("Warning: count==0 in find_CenterPoint_of_MaxPointSet!"),0);
        p.inputProperty = pxUnknown;//pxUnknown=0,so use 'if(p.inputProperty)' to judge the validation of the point
    }
    else
    {
        vector<int> pointNumber;
        for(long k = 0; k < MaxDpId_after_cluster; k++)//初始化
        {
            pointNumber.push_back(0);
        }
        for(unsigned long i = 0; i < dataNum; i++)
        {
            pointNumber[dataSets.at(i).GetDpId_after_cluster()-1]++;//因为MaxDpId_after_cluster实际值的范围是[1,MaxDpId_after_cluster]，所以要减1
        }
        int max = pointNumber.at(0);
        int max_ind = 0;
        for(long k = 0; k < MaxDpId_after_cluster; k++)
        {
            if(max < pointNumber.at(k))
            {
                max = pointNumber.at(k);
                max_ind = k;
            }
        }
        printf("max_ind:%d   and   max number:%d\n",max_ind,max);

        float sum_x = 0;
        float sum_y = 0;
        float sum_z = 0;

        unsigned long count = 0;

        for(unsigned long i = 0; i < dataNum; i++)
        {
            //聚合方式:同一类的点，聚合为几何中心
            if(dataSets.at(i).GetDpId_after_cluster()==(max_ind+1))
            {
                count++;
                sum_x+=curlist[i].x;
                sum_y+=curlist[i].y;
                sum_z+=curlist[i].z;
            }
        }

        p.inputProperty = pxLocaUseful;//pxLocaUseful=2
        p.x = sum_x/count;
        p.y = sum_y/count;
        p.z = sum_z/count;
        p.radius = 1;
        p.color = green;
    }

}

