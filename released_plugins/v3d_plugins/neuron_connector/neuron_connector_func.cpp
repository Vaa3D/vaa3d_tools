/* neuron_connector_plugin.cpp
 * This plugin will connect the segments in swc file that meet the creterian. Only tips will be connected to other segments.
 * 2014-11-03 : by Hanbo Chen
 */

#include "v3d_message.h"
#include <vector>
#include <map>
#include "neuron_connector_func.h"
#include <iostream>

#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
#define NTDOT(a,b) ((a).x*(b).x+(a).y*(b).y+(a).z*(b).z)
#ifndef MAX_DOUBLE
#define MAX_DOUBLE 1.79768e+308        //actual: 1.79769e+308
#endif

using namespace std;

void printHelp()
{
    cout<<"\nUsage: vaa3d -x neuron_connector -f connect_neuron_SWC -i <input.swc> -o <output.swc> "
          <<"-p <angular threshold=60> <distance threshold=10> <zscale=1> <xscale=1> <yscale=1> <matchtype=1> <surfacedis=true> <rootid=-1>"<<endl;
    cout<<"angular threshold: Will first search for connections between tips with angle smaller than angthr degrees within disthr. Then will search for tip to segment connections. Set to 0 for no preference of tip to tip connection."<<endl;
    cout<<"distance threshold: Maximum Euclidean distance to generate new connections. Set to ALL to connect everything."<<endl;
    cout<<"scale: e.g. when zscale=2, the z coordinate will be multiplied by 2 before computing."<<endl;
    cout<<"matchtype: \t0: no constrain;"<<endl;
    cout<<"\t\t1: only branch with the same type will be connected."<<endl;
    cout<<"\t\t2: only branch with the same type will be connected except soma (type 1). And only one connection between axon (2) and soma (1) can be made."<<endl;
    cout<<"\t\t3: only branch with the different type will be connected."<<endl;
    cout<<"surfacedis: when set to true, the radius will be deducted when computing distance."<<endl;
    cout<<"\n";
}

double getswcdiameter(NeuronTree* nt)
{
    if(nt->listNeuron.size()<=0)
        return 0;
    float x_min=nt->listNeuron.at(0).x;
    float x_max=nt->listNeuron.at(0).x;
    float y_min=nt->listNeuron.at(0).y;
    float y_max=nt->listNeuron.at(0).y;
    float z_min=nt->listNeuron.at(0).z;
    float z_max=nt->listNeuron.at(0).z;
    for(V3DLONG i=1; i<nt->listNeuron.size(); i++){
        x_min=MIN(nt->listNeuron.at(i).x, x_min);
        y_min=MIN(nt->listNeuron.at(i).y, y_min);
        z_min=MIN(nt->listNeuron.at(i).z, z_min);
        x_max=MAX(nt->listNeuron.at(i).x, x_max);
        y_max=MAX(nt->listNeuron.at(i).y, y_max);
        z_max=MAX(nt->listNeuron.at(i).z, z_max);
    }
    double tmp=(x_max-x_min)*(x_max-x_min);
    tmp=(y_max-y_min)*(y_max-y_min);
    tmp=(z_max-z_min)*(z_max-z_min);
    tmp=sqrt(tmp);
    return tmp;
}

void connectall(NeuronTree* nt, QList<NeuronSWC>& newNeuron, double xscale, double yscale, double zscale, double angThr, double disThr, int matchType, bool b_minusradius, int rootID=-1)
{
    newNeuron.clear();
    bool b_connectall = false;
    bool b_somaaxon = false;
    if(disThr<0){
        disThr=getswcdiameter(nt);
        b_connectall=true;
    }

    //rescale neurons
    QList<XYZ> scaledXYZ;
    for(V3DLONG i=0; i<nt->listNeuron.size(); i++){
        XYZ S;
        S.x = nt->listNeuron.at(i).x*xscale;
        S.y = nt->listNeuron.at(i).y*yscale;
        S.z = nt->listNeuron.at(i).z*zscale;
        scaledXYZ.append(S);
    }

    qDebug()<<"search for components and tips";
    //initialize tree components and get all tips
    QList<V3DLONG> cand;
    QList<XYZ> canddir;
    QVector<int> childNum(nt->listNeuron.size(), 0);
    QVector<int> connNum(nt->listNeuron.size(), 0);
    QList<V3DLONG> components;
    QList<V3DLONG> pList;
    V3DLONG curid=0;
    for(V3DLONG i=0; i<nt->listNeuron.size(); i++){
        if(nt->listNeuron.at(i).pn<0){
            connNum[i]--; //root that only have 1 child will also be a dead end
            components.append(curid); curid++;
            pList.append(-1);
        }else{
            V3DLONG pid = nt->hashNeuron.value(nt->listNeuron.at(i).pn);
            childNum[pid]++;
            connNum[pid]++;
            components.append(-1);
            pList.append(pid);

            //check if there is connection between soma and axon already
            if(!b_somaaxon){
                if(nt->listNeuron.at(i).type==2 && nt->listNeuron.at(pid).type==1)
                    b_somaaxon = true;
                if(nt->listNeuron.at(i).type==1 && nt->listNeuron.at(pid).type==2)
                    b_somaaxon = true;
            }
        }
    }
    qDebug()<<"components searching";
    //connected component
    for(V3DLONG cid=0; cid<curid; cid++){
        QStack<V3DLONG> pstack;
        V3DLONG chid;
        if(!components.contains(cid)) //should not happen, just in case
            continue;
        if(components.indexOf(cid)!=components.lastIndexOf(cid)) //should not happen
            qDebug("unexpected multiple tree root, please check the code: neuron_stitch_func.cpp");
        //recursively search for child and mark them as the same component
        pstack.push(components.indexOf(cid));
        while(!pstack.isEmpty()){
            V3DLONG pid=pstack.pop();
            chid = -1;
            chid = pList.indexOf(pid,chid+1);
            while(chid>=0){
                pstack.push(chid);
                components[chid]=cid;
                chid=pList.indexOf(pid,chid+1);
            }
        }
    }
    qDebug()<<"tips searching";
    //get tips
    for(V3DLONG i=0; i<childNum.size(); i++){
        if(connNum.at(i)<1){
            cand.append(i);
            //get direction
            V3DLONG id=i;
            XYZ tmpdir(0,0,0);
            if(childNum[id]==1){ //single child root
                V3DLONG sid = pList.indexOf(id);
                if(sid>=0){
                    tmpdir.x += scaledXYZ.at(id).x-scaledXYZ.at(sid).x;
                    tmpdir.y += scaledXYZ.at(id).y-scaledXYZ.at(sid).y;
                    tmpdir.z += scaledXYZ.at(id).z-scaledXYZ.at(sid).z;
                }
            }else{ //tips
                V3DLONG sid = pList[id];
                if(sid>=0){
                    tmpdir.x += scaledXYZ.at(id).x-scaledXYZ.at(sid).x;
                    tmpdir.y += scaledXYZ.at(id).y-scaledXYZ.at(sid).y;
                    tmpdir.z += scaledXYZ.at(id).z-scaledXYZ.at(sid).z;
                }
            }
            double tmpNorm = sqrt(tmpdir.x*tmpdir.x+tmpdir.y*tmpdir.y+tmpdir.z*tmpdir.z);
            if(tmpNorm>1e-16){
                tmpdir.x/=tmpNorm;
                tmpdir.y/=tmpNorm;
                tmpdir.z/=tmpNorm;
            }
            canddir.append(tmpdir);
        }
    }

    qDebug()<<connNum.size()<<":"<<childNum.size()<<":"<<cand.size();

    qDebug()<<"match tips";
    //match tips
    multimap<double, QVector<V3DLONG> > connMap;
    for(V3DLONG tid=0; tid<cand.size(); tid++){
        V3DLONG tidx=cand.at(tid);
        for(V3DLONG cid=0; cid<curid; cid++){
            if(cid==components.at(cand[tid])) continue;
            double mvdis=disThr, mtdis=disThr;
            V3DLONG mvid=-1, mtid=-1;
            V3DLONG id=components.indexOf(cid);
            while(id>=0){
                if(matchType==1){ //must be the same type to connect
                    if(nt->listNeuron.at(id).type!=nt->listNeuron.at(tidx).type){
                        id=components.indexOf(cid, id+1);
                        continue;
                    }
                }
                if(matchType==2){ //must be the same type except soma
                    if(nt->listNeuron.at(id).type!=nt->listNeuron.at(tidx).type &&
                            nt->listNeuron.at(id).type!=1 && nt->listNeuron.at(tidx).type!=1){
                        id=components.indexOf(cid, id+1);
                        continue;
                    }
                }
                if(matchType==3){ //must be the different type to connect
                    if(nt->listNeuron.at(id).type==nt->listNeuron.at(tidx).type){
                        id=components.indexOf(cid, id+1);
                        continue;
                    }
                }
                double dis=NTDIS(scaledXYZ.at(tidx),scaledXYZ.at(id));
                if(b_minusradius){
                    dis-=nt->listNeuron.at(id).radius;
                    dis-=nt->listNeuron.at(tidx).radius;
                }
                if(dis<mvdis){
                    mvdis=dis;
                    mvid=id;
                }
                if(dis<mtdis){
                    if(connNum.at(id)<1){//tips
                        V3DLONG tmpid=cand.indexOf(id);
                        if(tmpid<0){//should not happen, just in case
                            qDebug()<<"unexpected error: cannot locate dead end in candidate list, please check code."<<tid<<":"<<cid<<":"<<id;
                            id=components.indexOf(cid, id+1);
                            continue;
                        }
                        if(NTDOT(canddir.at(tid),canddir.at(tmpid))<angThr){
                            mtdis=dis;
                            mtid=id;
                        }
                    }
                }
                id=components.indexOf(cid, id+1);
            }
//            if(b_connectall){
//                if(mvid>=0){
//                    QVector<V3DLONG> tmp;
//                    tmp.append(tidx); tmp.append(mvid);
//                    connMap.insert(pair<double, QVector<V3DLONG> >(mvdis,tmp));
//                }
//            }else{
                if(mvid>=0){
                    QVector<V3DLONG> tmp;
                    tmp.append(tidx); tmp.append(mvid);
                    connMap.insert(pair<double, QVector<V3DLONG> >(mvdis+disThr,tmp));
                }
                if(mtid>=0){
                    QVector<V3DLONG> tmp;
                    tmp.append(tidx); tmp.append(mtid);
                    connMap.insert(pair<double, QVector<V3DLONG> >(mtdis,tmp));
                }
//            }
        }
    }

    qDebug()<<"connecting tips";
    //find the best solution for connecting tips
    QMap<V3DLONG, QVector<V3DLONG> > connectPairs;
    for(multimap<double, QVector<V3DLONG> >::iterator iter=connMap.begin(); iter!=connMap.end(); iter++){
        if(components.at(iter->second.at(0))==components.at(iter->second.at(1))) //already connected
            continue;
        if(matchType==2){ //check soma axon connection
            if(nt->listNeuron.at(iter->second.at(0)).type * nt->listNeuron.at(iter->second.at(1)).type == 2){ //is soma axon connection
                if(b_somaaxon) //already has soma axon connection
                    continue;
                else
                    b_somaaxon=true;
            }
        }
        if(connectPairs.contains(iter->second.at(0))){
            connectPairs[iter->second.at(0)].append(iter->second.at(1));
        }else{
            QVector<V3DLONG> tmp; tmp.append(iter->second.at(1));
            connectPairs.insert(iter->second.at(0),tmp);
        }
        if(connectPairs.contains(iter->second.at(1))){
            connectPairs[iter->second.at(1)].append(iter->second.at(0));
        }else{
            QVector<V3DLONG> tmp; tmp.append(iter->second.at(0));
            connectPairs.insert(iter->second.at(1),tmp);
        }
        V3DLONG cid_0=components.at(iter->second.at(0));
        V3DLONG cid_1=components.at(iter->second.at(1));
        V3DLONG tmpid=components.indexOf(cid_1);
        while(tmpid>=0){
            components[tmpid]=cid_0;
            tmpid=components.indexOf(cid_1,tmpid+1);
        }
    }

    qDebug()<<"reconstruct neuron tree";
    //reconstruct tree
    QVector<V3DLONG> newid(nt->listNeuron.size(), -1);
    QVector<V3DLONG> newpn(nt->listNeuron.size(), -1); //id starts from 1, -1: not touched, 0: touched but overlap with parent
    curid=1;
    int rootidx=nt->hashNeuron.value(rootID);
    if(nt->listNeuron[rootidx].n != rootID)
        rootidx=-1;
    QVector<V3DLONG> prinode;
    if(rootidx!=-1){
        prinode.push_back(rootidx);
    }
    for(V3DLONG i=0; i<nt->listNeuron.size(); i++){
        if(nt->listNeuron[i].parent==-1){
            prinode.push_back(i);
        }
    }
    V3DLONG i=0;
    V3DLONG priIdx=0;
    while(1){
        if(priIdx<prinode.size()){
            i=prinode[priIdx];
            priIdx++;
        }else if(priIdx==prinode.size()){
            i=0;
            priIdx++;
        }else{
            i++;
            if(i>=nt->listNeuron.size())
                break;
        }
        if(newid[i]>0) continue;
        QQueue<V3DLONG> pqueue; pqueue.clear();
        pqueue.enqueue(i);
        newid[i]=curid++;
        while(!pqueue.isEmpty()){
            //add current node to the listNeuron
            V3DLONG oid=pqueue.dequeue();

            if(newid[oid]>0){
                NeuronSWC tmpNeuron;
                tmpNeuron.n = newid[oid];
                tmpNeuron.x = nt->listNeuron.at(oid).x;
                tmpNeuron.y = nt->listNeuron.at(oid).y;
                tmpNeuron.z = nt->listNeuron.at(oid).z;
                tmpNeuron.type = nt->listNeuron.at(oid).type;
                tmpNeuron.r = nt->listNeuron.at(oid).r;
                tmpNeuron.fea_val = nt->listNeuron.at(oid).fea_val;
                tmpNeuron.pn = newpn.at(oid);
                newNeuron.append(tmpNeuron);
            }

            //add current node's children/parent/new-neighbor to the stack
            //parent
            if(nt->listNeuron.at(oid).pn>=0){
                V3DLONG opid = nt->hashNeuron.value(nt->listNeuron.at(oid).pn);
                if(newid.at(opid)<0){
                    pqueue.enqueue(opid);
                    newpn[opid]=newid[oid];
                    newid[opid]=curid++;
                }
            }
            //child
            V3DLONG tmpid=pList.indexOf(oid);
            while(tmpid>=0){
                if(newid.at(tmpid)<0){
                    pqueue.enqueue(tmpid);
                    newpn[tmpid]=newid[oid];
                    newid[tmpid]=curid++;
                }
                tmpid=pList.indexOf(oid,tmpid+1);
            }
            //new-neighbor
            if(connectPairs.contains(oid)){
                for(V3DLONG j=0; j<connectPairs[oid].size(); j++){
                    V3DLONG onid=connectPairs[oid].at(j);
                    if(newid.at(onid)<0){
                        pqueue.enqueue(onid);
                        newpn[onid]=newid[oid];
                        newid[onid]=curid++;
                    }
                }
            }
        }
    }
}

bool export_list2file(const QList<NeuronSWC>& lN, QString fileSaveName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    bool eswc_flag=false;
    if(fileSaveName.section('.',-1).toUpper()=="ESWC")
        eswc_flag=true;
    QTextStream myfile(&file);
    myfile<<"# generated by Vaa3D Plugin neuron_connector"<<endl;
    if(eswc_flag)
        myfile<<"##n,type,x,y,z,radius,parent,segment_id,segment_layer,feature_value"<<endl;
    else
        myfile<<"##n,type,x,y,z,radius,parent"<<endl;
    for (V3DLONG i=0;i<lN.size();i++){
        myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn;
        if(eswc_flag){
            myfile<<" "<<lN.at(i).seg_id<<" "<<lN.at(i).level;
            for(int j=0; j<lN.at(i).fea_val.size(); j++)
                myfile <<" "<< lN.at(i).fea_val.at(j);
        }
        myfile << endl;
    }
    file.close();
    return true;
}
