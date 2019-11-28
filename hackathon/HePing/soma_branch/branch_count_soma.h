#ifndef BRANCH_COUNT_SOMA_H
#define BRANCH_COUNT_SOMA_H
#include<v3d_interface.h>
#include<vector>
#include<math.h>
using namespace std;

//template<class T> bool compute_marker_pca_hp(vector<T> markers,V3DLONG r,double &pc1,double &pc2,double &pc3);


template<class T>
inline double distance_two_point(T &point1,T &point2){
    return sqrt((point1.x-point2.x)*(point1.x-point2.x)+(point1.y-point2.y)*(point1.y-point2.y)+(point1.z-point2.z)*(point1.z-point2.z));
}


//double getBlockAveValue(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2,
//                            V3DLONG x0, V3DLONG y0, V3DLONG z0,
//                            int xstep, int ystep, int zstep)
//{
//    if (!img3d || dim0<=0 || dim1<=0 || dim2<=0 ||
//        x0<0 || x0>=dim0 || y0<0 || y0>=dim1 || z0<0 || z0>=dim2)
//        return 0;

//    double xsteph=fabs(xstep)/2, ysteph=fabs(ystep)/2, zsteph=fabs(zstep)/2;
//    V3DLONG xs=x0-xsteph, xe=x0+xsteph,
//        ys=y0-ysteph, ye=y0+ysteph,
//        zs=z0-zsteph, ze=z0+zsteph;

//    if (xs<0) xs=0; if (xe>=dim0) xe=dim0-1;
//    if (ys<0) ys=0; if (ye>=dim1) ye=dim1-1;
//    if (zs<0) zs=0; if (ze>=dim2) ze=dim2-1;

//    V3DLONG i,j,k,n;
//    double v=0;
//    n=0;
//    for (k=zs;k<=ze; k++)
//        for (j=ys;j<=ye; j++)
//            for (i=xs;i<=xe; i++)
//            {
//                v += double(img3d[k][j][i]);
//                n++;
//            }
//    return (n==0)?0:v/n;
//}



//将原图的1维数据格式转换为三维
bool p1data_to_image3d(unsigned char* p1data,V3DLONG* sz,unsigned char*** &image3d);

struct Branch{
    NeuronSWC head_point,end_point;
    double distance_to_soma=0;
    inline double get_distance()
    {
        return distance_two_point(head_point,end_point);

    }
};

struct location{

    double x;
    double y;
    double z;
    location(double x,double y,double z){
        this->x=x;
        this->y=y;
        this->z=z;
    }
    location(){
        x=y=z=0;
    }
};




struct SWCTree{

    vector<Branch> branchs;//存放从soma出来的所有分支
    vector<NeuronSWC> locations;
    void count_branch_location(NeuronTree t,QList<ImageMarker> &markers,vector<location> &points,NeuronSWC &soma,double &max_radius);

};


bool four_point(vector<location> &points);

double compute_marker_pca_hp(vector<location> markers,V3DLONG r,double &pc1,double &pc2,double &pc3);

bool mean_shift_soma_location_hp(unsigned char*** image3d,NeuronSWC &soma,V3DLONG dim0,V3DLONG dim1,V3DLONG dim2,double radius,int max_loops);
bool find_soma(NeuronTree n,NeuronSWC &soma,double &radius);
//找到输入参数目录下的所有swc文件和v3draw文件
void find_files(QString path,vector<QString> &swcfiles,vector<QString> &v3drawfiles);

bool find_corresponding_file(QString dir,QString &swcfile,QString &v3drawfile);

#endif // BRANCH_COUNT_SOMA_H
