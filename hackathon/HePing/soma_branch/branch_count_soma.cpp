
#include<math.h>
#include<algorithm>

#include"branch_count_soma.h"
void SWCTree::count_branch_location(NeuronTree nt,QList<ImageMarker> &markers,vector<location> &points){

    V3DLONG size=nt.listNeuron.size();
    NeuronSWC soma;
    double radius_threshold;//soma 半径
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


