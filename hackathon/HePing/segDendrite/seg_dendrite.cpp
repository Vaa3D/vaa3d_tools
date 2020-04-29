
#include<vector>
#include<v3d_interface.h>
#include "seg_dendrite.h"
using namespace std;


bool seg_dendrite(QString outpath,NeuronTree &nt){
    vector<segment> segs;
    NeuronTree n;
    V3DLONG size=nt.listNeuron.size();
    NeuronSWC soma;
    vector<vector<V3DLONG> > children = vector<vector<V3DLONG> >(size,vector<V3DLONG>());//segment的所有孩子节点index
    for(V3DLONG i=0;i<size;i++){
        V3DLONG par=nt.listNeuron[i].parent;
        if(par<0){
            soma=nt.listNeuron[i];
            continue;
        }
        children[nt.hashNeuron.value(par)].push_back(i);
    }
    qDebug()<<"find all child!";

    vector<Point> queue;
    Point p0(soma.n,soma.n,soma.x,soma.y,soma.z,0);
    queue.push_back(p0);
    NeuronSWC axon;
    bool flag=false;
    while(!queue.empty()){//从soma开始遍历所有的segment，存放到branchs
        Point tmp=queue.front();
        queue.erase(queue.begin());

        for(int i=0;i<children[nt.hashNeuron.value(tmp.n)].size();i++){//从每一个点的孩子节点往下广度遍历
            segment seg;
            seg.head=tmp;//分支的起点
            NeuronSWC node;
            node.x=tmp.x;
            node.y=tmp.y;
            node.z=tmp.z;
            NeuronSWC par;
            NeuronSWC child=nt.listNeuron[children[nt.hashNeuron.value(tmp.n)][i]];
            seg.distance=tmp.dis_to_soma+distance_two_point(node,child);//父段的距离加新的点的距离

            while(children[nt.hashNeuron.value(child.n)].size()==1){//没有分叉点就不断往下，
                par=child;
                child=nt.listNeuron[children[nt.hashNeuron.value(par.n)][0]];
                if(seg.distance>2000){//使用了阈值确定
                    axon=child;
                    flag=true;
                    qDebug()<<"find********";
                    break;
                }
                seg.distance+=distance_two_point(par,child);
            }
            if(flag==true){
                break;
            }
            if(children[nt.hashNeuron.value(child.n)].size()==0){//终端点
                Point p1(child.n,child.parent,child.x,child.y,child.z,seg.distance);
                seg.end=p1;
                segs.push_back(seg);//终端则该分支结束
                continue;
            }
            else{//分叉

                Point p1(child.n,child.parent,child.x,child.y,child.z,seg.distance);
                seg.end=p1;
                queue.push_back(p1);

            }

        }
        if(flag==true)
            break;

    }
    qDebug()<<"finish find all segment";


//    for(int i=0;i<segs.size();i++){//找到每个段的父段

//        if(segs[i].head.parent<0){
//            segs[i].parent=0;
//        }
//        else{
//            for(int j=0;j<segs.size();j++){
//                if(sges[i].head.n==segs[j].end.n){
//                    segs[i].parent=&segs[j];
//                    break;
//                }
//            }

//        }
//    }







    vector<NeuronSWC> axon_node;
    NeuronSWC axon_ori;
    //找到long projection
    qDebug()<<"start to find axon!";
    if(flag==true){

        NeuronSWC tmp=axon;
        //找到所有祖先节点，逆序
        qDebug()<<soma.n;
        while(tmp.n!=soma.n){
            axon_ori=tmp;
            tmp=nt.listNeuron[nt.hashNeuron.value(tmp.parent)];
            axon_node.push_back(tmp);

        }
        qDebug()<<axon_ori.n;
        qDebug()<<"------";
        //找到最原始的节点进行递归向下删除
        for(int k=0;k<size;k++){
            if(nt.listNeuron[k].n=axon_ori.n){
                //nt.listNeuron[k].type=8;
                vector<NeuronSWC> que;
                que.push_back(nt.listNeuron[k]);
                while(!que.empty()){//找到axon上的所有节点
                    NeuronSWC tmp=que.front();
                    que.erase(que.begin());
                    nt.listNeuron[nt.hashNeuron.value(tmp.n)].type=8;
                    for(int i=0;i<children[nt.hashNeuron.value(tmp.n)].size();i++){
                        nt.listNeuron[children[nt.hashNeuron.value(tmp.n)][i]].type=8;

                        NeuronSWC child=nt.listNeuron[children[nt.hashNeuron.value(tmp.n)][i]];
                        while(children[nt.hashNeuron.value(child.n)].size()==1){
                            nt.listNeuron[children[nt.hashNeuron.value(child.n)][0]].type=8;
                            child=nt.listNeuron[children[nt.hashNeuron.value(child.n)][0]];

                        }
                        if(children[nt.hashNeuron.value(child.n)].size()==0)
                            continue;
                        else if(children[nt.hashNeuron.value(child.n)].size()>=2){
                            que.push_back(child);
                        }
                    }

                }
                break;
            }
            else continue;
        }

        NeuronTree axon_swc;
        axon_swc.listNeuron.push_back(soma);
        for(int jj=0;jj<size;jj++){
            if(nt.listNeuron[jj].type!=8){//非axon上的点
                NeuronSWC tmp=nt.listNeuron[jj];
                n.listNeuron.push_back(tmp);
            }
            else{//axon上的点,没有soma点不行？？？
                NeuronSWC temp=nt.listNeuron[jj];
                axon_swc.listNeuron.push_back(temp);
            }
        }

        const QString out=outpath;
        const NeuronTree tree=n;
        qDebug()<<out;
        writeESWC_file(out,tree);
        writeESWC_file(out+"_axon.swc",axon_swc);



    }
    else{
        writeESWC_file(outpath,nt);
    }




    return true;
}
