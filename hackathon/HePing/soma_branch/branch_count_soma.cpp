#include<math.h>
#include<algorithm>
#include"branch_count_soma.h"
#include<compute_win_pca.h>
#include<eigen_3_3_4/Eigen/Dense>
using namespace Eigen;
#define VAL_INVALID 0


bool compute_marker_pca_hp(vector<location> markers,V3DLONG r,double &pc1,double &pc2,double &pc3){
   //frist get the center of mass:soma location
    double xm,ym,zm;//均值
    int size=markers.size();

    for(int i=0;i<size;i++){
        xm+=markers[i].x;
        ym+=markers[i].y;
        zm+=markers[i].z;
    }
    xm/=size;
    ym/=size;
    zm/=size;

    //计算协方差
    double cc11=0,cc12=0,cc13=0,cc22=0,cc23=0,cc33=0;
    double dfx,dfy,dfz;
    for(int j=0;j<size;j++){
        dfx=markers[j].x-xm;
        dfy=markers[j].y-ym;
        dfz=markers[j].z-zm;
        cc11+=dfx*dfx;
        cc12+=dfx*dfy;
        cc13+=dfx*dfz;
        cc22+=dfy*dfy;
        cc23+=dfy*dfz;
        cc33+=dfz*dfz;
    }
    cc11/=(size-1);cc12/=(size-1);cc13=(size-1);cc22/=(size-1);cc23/=(size-1);cc33/=(size-1);

    try{

        Matrix3d cov_matrix;
        cov_matrix<<cc11,cc12,cc13,cc12,cc22,cc23,cc13,cc23,cc33;

        EigenSolver<Matrix3d> es(cov_matrix);
        MatrixXcd val=es.eigenvalues();
        MatrixXd evalsReal=val.real();
        //std::cout<<evalsReal;
        pc1=evalsReal(0);
        pc2=evalsReal(1);
        pc3=evalsReal(2);
        //std::cout<<es.eigenvalues();

        //MatrixXd evalsReal;
//        evalsReal=evals.real();
//        cout<<evalsReal(0,0);
        //count<<es.eigenvalues();
//        qDebug()<<evals.rows()<<evals.cols();
//        for(int k=0;k<evalsReal.rows();k++){
//        }
        //获取特征向量
//        SymmetricMatrix Cov_Matrix(3);
//        Cov_Matrix.Row(0)<<cc11;
//        Cov_Matrix.Row(1)<<cc12<<cc22;
//        Cov_Matrix.Row(2)<<cc13<<cc23<<cc33;
//        DiagonalMatrix DD;
//        Matrix vv;
//        EigenValues(Cov_Matrix,DD,vv);

//        //得到特征值
//        pc1=DD(3);
//        pc2=DD(2);
//        pc3=DD(1);

    }
    catch(...){
        pc1 = VAL_INVALID;
        pc2 = VAL_INVALID;
        pc3 = VAL_INVALID;
    }

    return true;
}



//找到所有与soma估计半径表面相交的分支上的点
void SWCTree::count_branch_location(NeuronTree nt,QList<ImageMarker> &markers,vector<location> &points,double &max_radius){

    V3DLONG size=nt.listNeuron.size();
    NeuronSWC soma;
    double radius_threshold;//soma 半径
    //double max_radius;
    vector<vector<V3DLONG> > children=vector<vector<V3DLONG> >(size,vector<V3DLONG>());
    for(V3DLONG i=0;i<size;i++){
        V3DLONG par=nt.listNeuron[i].parent;
        if(par<0){
            soma=nt.listNeuron[i];
            radius_threshold=soma.r;
            qDebug()<<"radius:"<<radius_threshold;
            continue;
        }
        children[nt.hashNeuron.value(par)].push_back(i);//所有node的children
    }

    max_radius=radius_threshold;
    vector<NeuronSWC> candidate;
    candidate.push_back(soma);
    bool flag=false;
    //从soma开始遍历所有的分支
    while(!candidate.empty()){
        NeuronSWC tmp;
        tmp=candidate.front();
        candidate.erase(candidate.begin());
        for(int i=0;i<children[nt.hashNeuron.value(tmp.n)].size();i++){
            Branch seg;
            seg.head_point=tmp;
            NeuronSWC child=nt.listNeuron[children[nt.hashNeuron.value(seg.head_point.n)][i]];
            seg.distance_to_soma=distance_two_point(soma,child);//该分支上的点与soma的距离
            if(seg.distance_to_soma>=radius_threshold){//到达soma边界,采样点不在位置上暂时不考虑
                if(seg.distance_to_soma>max_radius)max_radius=seg.distance_to_soma;
                locations.push_back(child);
                qDebug()<<child.x<<child.y<<child.z;
                continue;
            }
            while(children[nt.hashNeuron.value(child.n)].size()==1){

                    child=nt.listNeuron[children[nt.hashNeuron.value(child.n)][0]];
                    seg.distance_to_soma=distance_two_point(soma,child);
                    if(seg.distance_to_soma>=radius_threshold){//到达soma边界,采样点不在位置上暂时不考虑
                        locations.push_back(child);
                        qDebug()<<child.x<<child.y<<child.z;
                        flag=true;
                        break;
                    }

            }
            if(flag==true){
                flag=false;
                continue;
            }
            if(children[nt.hashNeuron.value(child.n)].size()>=2){
                candidate.push_back(child);
            }
    }

    }
    //将soma放入点集中
    location point0=location(soma.x,soma.y,soma.z);
    points.push_back(point0);

    //将与soma表面相交的分支上的点放入点集中
    for(int j=0;j<locations.size();j++){
        NeuronSWC node;
        node=locations[j];
        location point=location(node.x,node.y,node.z);
        points.push_back(point);
        ImageMarker *marker;
        marker=new ImageMarker();
        marker->x=node.x+1;
        marker->y=node.y+1;
        marker->z=node.z+1;
        marker->color.r=255;
        marker->color.g=0;
        marker->color.b=0;
        markers.push_back(*marker);
    }






}

//计算三维空间中任意四个点是否共面
bool four_point(vector<location> &points){
    int size=points.size();
    if(size<=3)return true;
    location s1,s2,s3;
    s1.x=points[1].x-points[0].x;
    s1.y=points[1].y-points[0].y;
    s1.z=points[1].z-points[0].z;
    s2.x=points[2].x-points[0].x;
    s2.y=points[2].y-points[0].y;
    s2.z=points[2].z-points[0].z;
    s3.x=points[3].x-points[0].x;
    s3.y=points[3].y-points[0].y;
    s3.z=points[3].z-points[0].z;
    int ans;
    ans=s1.x*s2.y*s3.z + s1.y*s2.z*s3.x + s1.z*s2.x*s3.y - s1.z*s2.y*s3.x - s1.x*s2.z*s3.y - s1.y*s2.x*s3.z;
    if(ans!=0)
         return false;

    for(int i=4;i<size;i++){
        s3.x=points[i].x-points[0].x;
        s3.y=points[i].y-points[0].y;
        s3.z=points[i].z-points[0].z;
        ans=s1.x*s2.y*s3.z + s1.y*s2.z*s3.x + s1.z*s2.x*s3.y - s1.z*s2.y*s3.x - s1.x*s2.z*s3.y - s1.y*s2.x*s3.z;
        if(ans!=0)
             return false;
    }
    return true;
}




