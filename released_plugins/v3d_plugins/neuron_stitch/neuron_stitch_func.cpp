/*
 * 2014.10.07 by: Hanbo Chen cojoc(at)hotmail.com
*/
#include "neuron_stitch_func.h"

#include <QDialog>
#include <QProgressDialog>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include "math.h"

using namespace std;

#define MAX(a,b) (a)>(b)?(a):(b)
#define MIN(a,b) (a)<(b)?(a):(b)

void searchBorderTips(QList<NeuronTree> *ntList, QList<Candidate> &cands, int side, int direction, double scale, double span, double gapThr, double angThr, double segmentThr, int spineLengthThr, double spineAngThr, double spineRadiusThr)
{
    double dir_range=100;
    cands.clear();

    if(side!=0 && side !=1){
        return;
    }
    int orientation = 1;
    if(side==0)
        orientation = -1;

    if(direction!=1 && direction!=2 && direction!=0){
        return;
    }

    double xscale=1, yscale=1, zscale=1;
    double border;
    if(direction==0)
        xscale=scale;
    else if(direction==1)
        yscale=scale;
    else if(direction==2)
        zscale=scale;
    if(side==0)
        border=1e16;
    else if(side==1)
        border=-1e16;
    //construct neuron and rescale it at the same time
    V3DLONG id_accu=0, id_max;
    NeuronTree nt;
    QVector<V3DLONG> nidrec, pidrec;
    for(int nid=0; nid<(*ntList).size(); nid++){
        id_max=0;
        for(int pid=0; pid<(*ntList).at(nid).listNeuron.size(); pid++){
            NeuronSWC tmp;
            tmp.n=(*ntList).at(nid).listNeuron.at(pid).n+id_accu;
            tmp.pn=(*ntList).at(nid).listNeuron.at(pid).pn+id_accu;
            id_max=MAX(tmp.n,id_max);
            tmp.r=(*ntList).at(nid).listNeuron.at(pid).r;
            tmp.x=(*ntList).at(nid).listNeuron.at(pid).x*xscale;
            tmp.y=(*ntList).at(nid).listNeuron.at(pid).y*yscale;
            tmp.z=(*ntList).at(nid).listNeuron.at(pid).z*zscale;
            nt.hashNeuron.insert(tmp.n, nt.listNeuron.size());
            nt.listNeuron.append(tmp);
            nidrec.append(nid);
            pidrec.append(pid);

            if(direction==0){
                if(side==0)
                    border=MIN(border,tmp.x);
                else
                    border=MAX(border,tmp.x);
            }else if(direction==1){
                if(side==0)
                    border=MIN(border,tmp.y);
                else
                    border=MAX(border,tmp.y);
            }else if(direction==2){
                if(side==0)
                    border=MIN(border,tmp.z);
                else
                    border=MAX(border,tmp.z);
            }
        }
        id_accu=id_max+1;
    }
    HBNeuronGraph ng;
    constructNeuronGraph(nt, ng);

    //neuron type
    QList<int> neuronType;
    getNeuronType(nt,ng,neuronType,spineLengthThr,spineAngThr,spineRadiusThr);

    //connected components and size
    QVector<double> sectionLength(nt.listNeuron.size(), 0);
    QList<int> components;
    QList<V3DLONG> pList;
    QVector<V3DLONG> componentSize;
    QVector<V3DLONG> componentLength;
    V3DLONG curid=0;
    for(V3DLONG i=0; i<nt.listNeuron.size(); i++){
        if(nt.listNeuron.at(i).pn<0){
            components.append(curid); curid++;
            pList.append(-1);
        }
        else{
            int pid = nt.hashNeuron.value(nt.listNeuron.at(i).pn);
            sectionLength[i]=sqrt(NTDIS(nt.listNeuron.at(i),nt.listNeuron.at(pid)));
            components.append(-1);
            pList.append(pid);
        }
    }
    //connected component
    for(V3DLONG cid=0; cid<curid; cid++){
        QStack<int> pstack;
        int chid, size = 0;
        if(!components.contains(cid)) //should not happen, just in case
            continue;
        if(components.indexOf(cid)!=components.lastIndexOf(cid)) //should not happen
            qDebug("unexpected multiple tree root, please check the code: neuron_stitch_func.cpp");
        //recursively search for child and mark them as the same component
        pstack.push(components.indexOf(cid));
        size++;
        while(!pstack.isEmpty()){
            int pid=pstack.pop();
            chid = -1;
            chid = pList.indexOf(pid,chid+1);
            while(chid>=0){
                pstack.push(chid);
                components[chid]=cid;
                chid=pList.indexOf(pid,chid+1);
                size++;
            }
        }
        componentSize.append(size);
    }
    //component size
    for(V3DLONG cid=0; cid<curid; cid++){
        double length = 0;
        int idx = -1;
        for(V3DLONG i=0; i<componentSize[cid]; i++){
            idx = components.indexOf(cid,idx+1);
            length+=sectionLength[idx];
        }
        componentLength.append(length);
    }

    //find candidate
    QList<int> tmpcand; tmpcand.clear();
    double min=border-span;
    double max=border+span;
    for(V3DLONG i=0; i<neuronType.size(); i++){
        if((neuronType.at(i)%10)==6 && componentLength[components[i]]>=segmentThr){
            if(direction==0){//x
                if(nt.listNeuron.at(i).x>min && nt.listNeuron.at(i).x<max){
                    tmpcand.append(i);
                }
            }else if(direction==1){//y
                if(nt.listNeuron.at(i).y>min && nt.listNeuron.at(i).y<max){
                    tmpcand.append(i);
                }
            }else if(direction==2){//z
                if(nt.listNeuron.at(i).z>min && nt.listNeuron.at(i).z<max){
                    tmpcand.append(i);
                }
            }else{//all tips
                tmpcand.append(i);
            }
        }
    }

    //for test
    qDebug()<<"first round: "<<tmpcand.size();

    //gap filter
    if(gapThr>1e-6){
        QList<int> gapmask, tmpcandbk;
        for(V3DLONG i=0; i<tmpcand.size(); i++){
            tmpcandbk.append(tmpcand.at(i));
            gapmask.append(0);
        }
        for(V3DLONG i=0; i<tmpcand.size(); i++){
            for(V3DLONG j=i+1; j<tmpcand.size(); j++){
                if(components.at(tmpcand.at(i)) == components.at(tmpcand.at(j)))
                        continue;
                if(NTDIS(nt.listNeuron.at(tmpcand.at(i)),nt.listNeuron.at(tmpcand.at(j)))<gapThr*gapThr){
                    gapmask[i]++;
                    gapmask[j]++;
                }
            }
        }
        tmpcand.clear();
        for(V3DLONG i=0; i<tmpcandbk.size(); i++){
            if(gapmask.at(i)<=0){
                tmpcand.append(tmpcandbk.at(i));
            }
        }
    }

    //for test
    qDebug()<<"second round: "<<tmpcand.size();

    for(V3DLONG i=0; i<tmpcand.size(); i++){
        XYZ tmpdir(0,0,0);
        V3DLONG curid = tmpcand[i];
        V3DLONG nexid, preid=curid;
        if(neuronType.at(tmpcand.at(i))==6){ //normal tips or spine tips
            float lentmp = 0;
            while(lentmp<dir_range && curid>=0){
                lentmp+=sectionLength[curid];
                nexid=nextPointNeuronGraph(ng,curid,preid);
                preid=curid;
                curid=nexid;
            }
            tmpdir.x = nt.listNeuron.at(tmpcand[i]).x - nt.listNeuron.at(preid).x;
            tmpdir.y = nt.listNeuron.at(tmpcand[i]).y - nt.listNeuron.at(preid).y;
            tmpdir.z = nt.listNeuron.at(tmpcand[i]).z - nt.listNeuron.at(preid).z;
        }else if(neuronType.at(tmpcand.at(i))==56){ //spine fork
            float lentmp = 0;
            while(lentmp<dir_range && curid>=0){
                lentmp+=sectionLength[curid];
                nexid=-1;
                for(int j=0; j<ng.at(curid).size(); j++){
                    if(ng.at(curid).at(j)==preid) continue;
                    if(neuronType.at(ng.at(curid).at(j))==2 || neuronType.at(ng.at(curid).at(j))== 52){
                        if(nexid==-1){
                            nexid=ng.at(curid).at(j);
                        }else{
                            nexid=-2;
                            break;
                        }
                    }
                }
                preid=curid;
                curid=nexid;
            }
            tmpdir.x = nt.listNeuron.at(tmpcand[i]).x - nt.listNeuron.at(preid).x;
            tmpdir.y = nt.listNeuron.at(tmpcand[i]).y - nt.listNeuron.at(preid).y;
            tmpdir.z = nt.listNeuron.at(tmpcand[i]).z - nt.listNeuron.at(preid).z;
        }
        //normalize direction
        double tmpNorm = sqrt(tmpdir.x*tmpdir.x+tmpdir.y*tmpdir.y+tmpdir.z*tmpdir.z);
        if(tmpNorm<1e-16) tmpNorm=1e-16;
        tmpdir.x/=tmpNorm; tmpdir.x*=orientation;
        tmpdir.y/=tmpNorm; tmpdir.y*=orientation;
        tmpdir.z/=tmpNorm; tmpdir.z*=orientation;

        //judgement to the direction to avoid the connection that moves from plan to
        double sa = 1;
        if(direction==0){
            sa = tmpdir.x;
        }else if(direction == 1){
            sa = tmpdir.y;
        }else if(direction == 2){
            sa = tmpdir.z;
        }
        //for test:
//        qDebug()<<cand.size()<<":"<<tmpcand[i]<<"; coord"<<nt.listNeuron.at(tmpcand[i]).x<<":"<<nt.listNeuron.at(tmpcand[i]).y<<":"<<nt.listNeuron.at(tmpcand[i]).z<<"; dir"<<tmpdir.x<<":"<<tmpdir.y<<":"<<tmpdir.z<<":"<<tmpNorm;
//        qDebug()<<cand.size()<<":"<<tmpcand[i]<<"; dir"<<tmpdir.x<<":"<<tmpdir.y<<":"<<tmpdir.z<<":"<<tmpNorm;

        if(sa<angThr) continue;

        Candidate ctmp;
        ctmp.nid=nidrec.at(tmpcand[i]);
        ctmp.pid=pidrec.at(tmpcand[i]);
        ctmp.status=0;
        cands.append(ctmp);
    }
}

//normal type: 1:single root; 2:path; 5:fork point; 6:end point;
//spine related: 21:path; 61:end point;
//spine related fork: 51:have 0 none spine path (single root); 56:have only one none spine path (end); 52:have two none spine path (path); 55:have more than two none spine path (fork);
//all types ended with '6' can be used for matching across sections
int getNeuronType(const NeuronTree& nt, const HBNeuronGraph& ng, QList<int>& neuronType, int spineLengthThr, double spineAngThr, double spineRadiusThr)
{
    neuronType.clear();
    int spinecount=0;
    int length=spineLengthThr;
    //basic types
    for(V3DLONG idx=0; idx<nt.listNeuron.size(); idx++){
        if(ng.at(idx).size()<1){
            neuronType.append(1); //single root
        }else if(ng.at(idx).size()==1){
            neuronType.append(6); //end point
        }else if(ng.at(idx).size()==2){
            neuronType.append(2); //path
        }else{
            neuronType.append(5); //fork point
        }
    }

    //spine
    if(length<=1e-10) return 0;

    QMap<V3DLONG, QVector<V3DLONG> > fork_spines_map;
    for(V3DLONG idx=0; idx<nt.listNeuron.size(); idx++){
        if(ng.at(idx).size()==1){ //end point
            int count = 0;
            V3DLONG preid=idx, nexid=idx, curid=idx;
            do{
                count++;
                nexid=nextPointNeuronGraph(ng,curid,preid);
                preid=curid;
                curid=nexid;
            }while(nexid>=0 && count<=length);

            if(count<length){ //is spine
                neuronType[idx]=61; //spine tip
                curid=nextPointNeuronGraph(ng,idx);
                preid=idx;
                while(curid>=0 && ng.at(curid).size()==2){
                    neuronType[curid]=21; //spine path
                    nexid=nextPointNeuronGraph(ng,curid,preid);
                    preid=curid;
                    curid=nexid;
                }
                if(curid>=0){
                    if(fork_spines_map.contains(curid)){
                        fork_spines_map[curid].append(idx);
                    }else{
                        QVector<V3DLONG> tmp;
                        tmp.append(idx);
                        fork_spines_map[curid]=tmp;
                    }
                }
                spinecount++;
            }
        }
    }

    //find the spine that is matchable
    for(QMap<V3DLONG, QVector<V3DLONG> >::Iterator iter = fork_spines_map.begin();
        iter != fork_spines_map.end(); iter++){
        //check the branch type (a.have only one none spine connection (end); b. have two (path); c. have more than two (fork))
        int count=0;
        V3DLONG idx = iter.key();

        //for test
        qDebug()<<"init neuron type: matchable "<<idx;

        for(int i=0; i<ng.at(idx).size(); i++){
            if(neuronType.at(ng.at(idx).at(i))!=21 && neuronType.at(ng.at(idx).at(i))!=61){
                count++;
            }
        }
        switch(count){
        case 0:
            neuronType[idx]=51; //single dendritic root
            break;
        case 1:
            neuronType[idx]=56; //dendritic end
            break;
        case 2:
            neuronType[idx]=52; //dendritic path
            break;
        default:
            neuronType[idx]=55; //dendritic fork
        }


        if(neuronType[idx]==56){
            XYZ rootDir;
            bool spineMatchable=false;
            for(int i=0; i<ng.at(idx).size(); i++){
                if(neuronType.at(ng.at(idx).at(i))!=21 && neuronType.at(ng.at(idx).at(i))!=61){
                    rootDir.x = nt.listNeuron.at(idx).x-nt.listNeuron.at(ng.at(idx).at(i)).x;
                    rootDir.y = nt.listNeuron.at(idx).y-nt.listNeuron.at(ng.at(idx).at(i)).y;
                    rootDir.z = nt.listNeuron.at(idx).z-nt.listNeuron.at(ng.at(idx).at(i)).z;
                }
            }

            for(int i=0; i<iter.value().size(); i++){
                V3DLONG curid = iter.value().at(i);
                //check the angle condition
                XYZ spineDir;
                spineDir.x = nt.listNeuron.at(curid).x-nt.listNeuron.at(idx).x;
                spineDir.y = nt.listNeuron.at(curid).y-nt.listNeuron.at(idx).y;
                spineDir.z = nt.listNeuron.at(curid).z-nt.listNeuron.at(idx).z;

                if(NTDOT(spineDir,rootDir)/(NTNORM(spineDir)*NTNORM(rootDir))<spineAngThr){
                    continue;
                }
                //check radius condition if meet angle condition
                V3DLONG preid=curid, nexid=curid;
                while(nt.listNeuron.at(curid).radius<spineRadiusThr && curid!=idx){
                    nexid=nextPointNeuronGraph(ng,curid,preid);
                    preid=curid;
                    curid=nexid;
                }
                if(curid!=idx){ //this could be a normal branch, not spine
                    curid = iter.value().at(i);
                    neuronType[curid]=6;
                    nexid=nextPointNeuronGraph(ng,curid);
                    preid=curid;
                    curid=nexid;
                    while(curid!=idx){
                        neuronType[curid]=2;
                        nexid=nextPointNeuronGraph(ng,curid,preid);
                        preid=curid;
                        curid=nexid;
                    }
                    spineMatchable=true;
                }
            }

            idx = iter.key();
            if(neuronType[idx]==56){
                if(spineMatchable || nt.listNeuron.at(idx).radius<spineRadiusThr){
                    neuronType[idx]=52; //radius too small or child can match, downgrade it to path of spine
                }
            }
        }else if(neuronType[idx]==55 || neuronType[idx]==51){
            //check radius condition
            for(int i=0; i<iter.value().size(); i++){
                V3DLONG curid = iter.value().at(i);
                V3DLONG preid=curid, nexid=curid;
                while(nt.listNeuron.at(curid).radius<spineRadiusThr && curid!=idx){
                    nexid=nextPointNeuronGraph(ng,curid,preid);
                    preid=curid;
                    curid=nexid;
                }
                if(curid!=idx){ //this could be a normal branch, not spine
                    curid = iter.value().at(i);
                    neuronType[curid]=6;
                    nexid=nextPointNeuronGraph(ng,curid);
                    preid=curid;
                    curid=nexid;
                    while(curid!=idx){
                        neuronType[curid]=2;
                        nexid=nextPointNeuronGraph(ng,curid,preid);
                        preid=curid;
                        curid=nexid;
                    }
                }
            }
        }
    }
    return spinecount;
}


void constructNeuronGraph(const NeuronTree& nt, HBNeuronGraph& ng)
{
    ng.clear();
    ng.resize(nt.listNeuron.size());
    for(int i=0; i<nt.listNeuron.size(); i++){
        if(nt.listNeuron.at(i).pn>=0){
            V3DLONG pid=nt.hashNeuron.value(nt.listNeuron.at(i).pn);
            ng[i].append(pid);
            ng[pid].append(i);
        }
    }
}

V3DLONG nextPointNeuronGraph(const HBNeuronGraph& ng, V3DLONG current, V3DLONG previous)
{
    V3DLONG next=-1;
    if(ng.at(current).size()==1 && (previous<0 || current==previous)){ //seek the next of a deadend
        next=ng.at(current).at(0);
    }else if(ng.at(current).size()==2 && previous>=0){ //seek the next on a path
        if(previous==ng.at(current).at(0)){
            next=ng.at(current).at(1);
        }else if(previous==ng.at(current).at(1)){
            next=ng.at(current).at(0);
        }else{
            next=-2;
        }
    }
    return next;
}


V3DLONG findNearestTips(const HBNeuronGraph & ng, const QList<int> & neuronType, V3DLONG startp)
{
    QList<V3DLONG> record;
    V3DLONG tid=-1, id=startp;
    QVector<V3DLONG> prev, next;
    prev.append(id);
    record.append(id);
    while(prev.size()>0){
        QVector<V3DLONG> cur;
        next.clear();
        for(int i=0; i<prev.size(); i++){
            id=prev.at(i);
            if(neuronType.at(id)%10==6){ //eligible border tips
                tid=id;
                break;
            }
            if(neuronType.at(id)%10==5){ //fork, ignore
                continue;
            }
            if(neuronType.at(id)==2){ //path point
                for(int j=0; j<ng.at(id).size(); j++){
                    if(!record.contains(ng.at(id).at(j))){
                        next.append(ng.at(id).at(j)); //add its neighbor to the next round if not reached so far
                        record.append(ng.at(id).at(j));
                    }
                }
            }else if(neuronType.at(id)>10){ //spine point
                for(int j=0; j<ng.at(id).size(); j++){
                    if(!record.contains(ng.at(id).at(j))){ //the point is not touched yet
                        if(neuronType.at(ng.at(id).at(j))==61){ //spine end
                            record.append(ng.at(id).at(j));
                        }else if(neuronType.at(ng.at(id).at(j))==21 || //spine path
                                 neuronType.at(ng.at(id).at(j))==51){ //spine fork
                            record.append(ng.at(id).at(j));
                        }else{ //leave it for next round
                            next.append(ng.at(id).at(j));
                            record.append(ng.at(id).at(j));
                        }
                    }
                }
            }
        }
        if(tid>-1){
            break;
        }
        prev=next;
    }
    return tid;
}

void stitchMatchedPoint(NeuronTree* nt0, NeuronTree* nt1, const HBNeuronGraph & ng0,
                        const HBNeuronGraph & ng1, const QList<int> & neuronType0, const QList<int> & neuronType1, int pid0, int pid1)
{
    //get the list of the list of points to move
    QList<V3DLONG> record;
    V3DLONG id = pid0;
    QVector<QVector<V3DLONG> > idList0;
    if(ng0.at(id).size()<=0){
        QVector<V3DLONG> tmp;
        tmp.append(id);
        idList0.append(tmp);
    }else{
        QVector<V3DLONG> prev, next;
        prev.append(id);
        record.append(id);
        while(prev.size()>0){
            QVector<V3DLONG> cur;
            next.clear();
            for(int i=0; i<prev.size(); i++){
                id=prev.at(i);
                if(neuronType0.at(id)%10==5){ //fork
                    continue;
                }else{
                    //push it to current round point
                    cur.append(id);
                    //check its type to see how to deal with its child
                    if(neuronType0.at(id)==6){ //end point
                        if(!record.contains(ng0.at(id).at(0))){
                            next.append(ng0.at(id).at(0)); //add its only neighbor to the next round
                            record.append(ng0.at(id).at(0));
                        }
                    }else if(neuronType0.at(id)==2){ //path point
                        for(int j=0; j<ng0.at(id).size(); j++){
                            if(!record.contains(ng0.at(id).at(j))){
                                next.append(ng0.at(id).at(j)); //add its neighbor to the next round if not reached so far
                                record.append(ng0.at(id).at(j));
                            }
                        }
                    }else if(neuronType0.at(id)>10){ //spine point
                        QQueue<V3DLONG> spinequeue;
                        for(int j=0; j<ng0.at(id).size(); j++){
                            if(!record.contains(ng0.at(id).at(j))){ //the point is not touched yet
                                if(neuronType0.at(ng0.at(id).at(j))==61){ //spine end
                                    cur.append(ng0.at(id).at(j));
                                    record.append(ng0.at(id).at(j));
                                }else if(neuronType0.at(ng0.at(id).at(j))==21 || //spine path
                                         neuronType0.at(ng0.at(id).at(j))==51){ //spine fork
                                    cur.append(ng0.at(id).at(j));
                                    record.append(ng0.at(id).at(j));
                                    spinequeue.append(ng0.at(id).at(j));
                                }else{ //leave it for next round
                                    next.append(ng0.at(id).at(j));
                                    record.append(ng0.at(id).at(j));
                                }
                            }
                        }
                        //check the queue to obtain all spine branches
                        while(!spinequeue.isEmpty()){
                            V3DLONG tmpid=spinequeue.dequeue();
                            for(int j=0; j<ng0.at(tmpid).size(); j++){
                                if(!record.contains(ng0.at(tmpid).at(j))){
                                    spinequeue.append(ng0.at(tmpid).at(j));
                                    cur.append(ng0.at(tmpid).at(j));
                                    record.append(ng0.at(tmpid).at(j));
                                }
                            }
                        }
                    }
                }
            }
            idList0.append(cur);
            prev=next;
        }
    }

    id = pid1;
    record.clear();
    QVector<QVector<V3DLONG> > idList1;
    if(ng1.at(id).size()<=0){
        QVector<V3DLONG> tmp;
        tmp.append(id);
        idList1.append(tmp);
    }else{
        QVector<V3DLONG> prev, next;
        prev.append(id);
        record.append(id);
        while(prev.size()>0){
            QVector<V3DLONG> cur;
            next.clear();
            for(int i=0; i<prev.size(); i++){
                id=prev.at(i);
                if(neuronType1.at(id)%10==5){ //fork
                    continue;
                }else{
                    //push it to current round point
                    cur.append(id);
                    //check its type to see how to deal with its child
                    if(neuronType1.at(id)==6){ //end point
                        if(!record.contains(ng1.at(id).at(0))){
                            next.append(ng1.at(id).at(0)); //add its only neighbor to the next round
                            record.append(ng1.at(id).at(0));
                        }
                    }else if(neuronType1.at(id)==2){ //path point
                        for(int j=0; j<ng1.at(id).size(); j++){
                            if(!record.contains(ng1.at(id).at(j))){
                                next.append(ng1.at(id).at(j)); //add its neighbor to the next round if not reached so far
                                record.append(ng1.at(id).at(j));
                            }
                        }
                    }else if(neuronType1.at(id)>10){ //spine point
                        QQueue<V3DLONG> spinequeue;
                        for(int j=0; j<ng1.at(id).size(); j++){
                            if(!record.contains(ng1.at(id).at(j))){ //the point is not touched yet
                                if(neuronType0.at(ng1.at(id).at(j))==61){ //spine end
                                    cur.append(ng1.at(id).at(j));
                                    record.append(ng1.at(id).at(j));
                                }else if(neuronType0.at(ng1.at(id).at(j))==21 || //spine path
                                         neuronType0.at(ng1.at(id).at(j))==51){ //spine fork
                                    cur.append(ng1.at(id).at(j));
                                    record.append(ng1.at(id).at(j));
                                    spinequeue.append(ng1.at(id).at(j));
                                }else{ //leave it for next round
                                    next.append(ng1.at(id).at(j));
                                    record.append(ng1.at(id).at(j));
                                }
                            }
                        }
                        //check the queue to obtain all spine branches
                        while(!spinequeue.isEmpty()){
                            V3DLONG tmpid=spinequeue.dequeue();
                            for(int j=0; j<ng1.at(tmpid).size(); j++){
                                if(!record.contains(ng1.at(tmpid).at(j))){
                                    spinequeue.append(ng1.at(tmpid).at(j));
                                    cur.append(ng1.at(tmpid).at(j));
                                    record.append(ng1.at(tmpid).at(j));
                                }
                            }
                        }
                    }
                }
            }
            idList1.append(cur);
            prev=next;
        }
    }


    //calculate the place to go
    XYZ goal;
    goal.x=nt0->listNeuron.at(pid0).x+(nt1->listNeuron.at(pid1).x - nt0->listNeuron.at(pid0).x)*idList0.size()/(idList0.size()+idList1.size());
    goal.y=nt0->listNeuron.at(pid0).y+(nt1->listNeuron.at(pid1).y - nt0->listNeuron.at(pid0).y)*idList0.size()/(idList0.size()+idList1.size());
    goal.z=nt0->listNeuron.at(pid0).z+(nt1->listNeuron.at(pid1).z - nt0->listNeuron.at(pid0).z)*idList0.size()/(idList0.size()+idList1.size());
    XYZ vector0;
    vector0.x=goal.x-nt0->listNeuron.at(pid0).x;
    vector0.y=goal.y-nt0->listNeuron.at(pid0).y;
    vector0.z=goal.z-nt0->listNeuron.at(pid0).z;
    for(int i=0; i<idList0.size(); i++){
        for(int j=0; j<idList0.at(i).size(); j++){
            NeuronSWC* p=(NeuronSWC*)&(nt0->listNeuron.at(idList0.at(i).at(j)));
            p->x += vector0.x*(idList0.size()-i)/(idList0.size()+0);
            p->y += vector0.y*(idList0.size()-i)/(idList0.size()+0);
            p->z += vector0.z*(idList0.size()-i)/(idList0.size()+0);
        }
    }

    vector0.x=goal.x-nt1->listNeuron.at(pid1).x;
    vector0.y=goal.y-nt1->listNeuron.at(pid1).y;
    vector0.z=goal.z-nt1->listNeuron.at(pid1).z;
    for(int i=0; i<idList1.size(); i++){
        for(int j=0; j<idList1.at(i).size(); j++){
            NeuronSWC* p=(NeuronSWC*)&(nt1->listNeuron.at(idList1.at(i).at(j)));
            p->x += vector0.x*(idList1.size()-i)/(idList1.size()+0);
            p->y += vector0.y*(idList1.size()-i)/(idList1.size()+0);
            p->z += vector0.z*(idList1.size()-i)/(idList1.size()+0);
        }
    }

}

void stitchMatchedPoint(NeuronTree* nt0, NeuronTree* nt1, const QList<int>& parent0, const QList<int>& parent1, int pid0, int pid1)
{
    //get the list of points to move
    QVector<int> idList0;
    int id = pid0;
    if(parent0.at(id)<0){ //single child root
        while(id>=0){
            if(parent0.count(id)>1) break; //have two children or more (branches)
            idList0.append(id);
            id = parent0.indexOf(id); //move to child
        }
    }else if(!parent0.contains(id)){ //tips
        while(id>=0){
            if(parent0.count(id)>1) break; //have two children or more (branches)
            idList0.append(id);
            id = parent0.at(id); //move to parent
        }
    }else{ //point in the middle, go both direction
        idList0.append(id);
       int idc=parent0.indexOf(id);
       if(parent0.count(idc)>1){//have two children or more (branches)
           idc=-1;
       }
       int idp=parent0.at(id);
       if(parent0.count(idp)>1){//have two children or more (branches)
           idp=-1;
       }
       while(idp>=0 || idc>=0){
           if(idc>=0){
               if(parent0.count(idc)>1){//have two children or more (branches)
                   idc=-1;
                   continue;
               }else{
                   idList0.append(idc);
                   idc = parent0.indexOf(idc); //move to child
               }
           }
           if(idp>=0){
               if(parent0.count(idp)>1){//have two children or more (branches)
                   idp=-1;
                   continue;
               }else{
                   idList0.append(idp);
                   idp = parent0.at(idp); //move to parent
               }
           }
       }
    }

    QVector<int> idList1;
    id = pid1;
    if(parent1.at(id)<0){ //single child root
        while(id>=0){
            if(parent1.count(id)>1) break; //have two children or more (branches)
            idList1.append(id);
            id = parent1.indexOf(id); //move to child
        }
    }else if(!parent0.contains(id)){ //tips
        while(id>=0){
            if(parent1.count(id)>1) break; //have two children or more (branches)
            idList1.append(id);
            id = parent1.at(id); //move to parent
        }
    }else{ //point in the middle, go both direction
        idList1.append(id);
       int idc=parent1.indexOf(id);
       if(parent1.count(idc)>1){//have two children or more (branches)
           idc=-1;
       }
       int idp=parent1.at(id);
       if(parent1.count(idp)>1){//have two children or more (branches)
           idp=-1;
       }
       while(idp>=0 || idc>=0){
           if(idc>=0){
               if(parent1.count(idc)>1){//have two children or more (branches)
                   idc=-1;
                   continue;
               }else{
                   idList1.append(idc);
                   idc = parent1.indexOf(idc); //move to child
               }
           }
           if(idp>=0){
               if(parent1.count(idp)>1){//have two children or more (branches)
                   idp=-1;
                   continue;
               }else{
                   idList1.append(idp);
                   idp = parent1.at(idp); //move to parent
               }
           }
       }
    }

    //calculate the place to go
    XYZ goal;
    goal.x=nt0->listNeuron.at(idList0.at(0)).x+(nt1->listNeuron.at(idList1.at(0)).x - nt0->listNeuron.at(idList0.at(0)).x)*idList0.size()/(idList0.size()+idList1.size());
    goal.y=nt0->listNeuron.at(idList0.at(0)).y+(nt1->listNeuron.at(idList1.at(0)).y - nt0->listNeuron.at(idList0.at(0)).y)*idList0.size()/(idList0.size()+idList1.size());
    goal.z=nt0->listNeuron.at(idList0.at(0)).z+(nt1->listNeuron.at(idList1.at(0)).z - nt0->listNeuron.at(idList0.at(0)).z)*idList0.size()/(idList0.size()+idList1.size());
    XYZ vector0;
    vector0.x=goal.x-nt0->listNeuron.at(idList0.at(0)).x;
    vector0.y=goal.y-nt0->listNeuron.at(idList0.at(0)).y;
    vector0.z=goal.z-nt0->listNeuron.at(idList0.at(0)).z;
    for(int i=0; i<idList0.size(); i++){
        NeuronSWC* p=(NeuronSWC*)&(nt0->listNeuron.at(idList0.at(i)));
        p->x += vector0.x*(idList0.size()-i)/(idList0.size()+0);
        p->y += vector0.y*(idList0.size()-i)/(idList0.size()+0);
        p->z += vector0.z*(idList0.size()-i)/(idList0.size()+0);
    }

    vector0.x=goal.x-nt1->listNeuron.at(idList1.at(0)).x;
    vector0.y=goal.y-nt1->listNeuron.at(idList1.at(0)).y;
    vector0.z=goal.z-nt1->listNeuron.at(idList1.at(0)).z;
    for(int i=0; i<idList1.size(); i++){
        NeuronSWC* p=(NeuronSWC*)&(nt1->listNeuron.at(idList1.at(i)));
        p->x += vector0.x*(idList1.size()-i)/(idList1.size()+0);
        p->y += vector0.y*(idList1.size()-i)/(idList1.size()+0);
        p->z += vector0.z*(idList1.size()-i)/(idList1.size()+0);
    }
}

bool matchCandidates(QList<NeuronTree> * ntList, QList<int> * cand, double span, int direction, QList<int> MatchMarkers[2]) //from cand[1] to cand[0]
{
    double shift_x = 0, shift_y = 0, shift_z = 0, cent_x = 0, cent_y = 0, cent_z = 0, angle = 0;
    MatchMarkers[0].clear();
    MatchMarkers[1].clear();

    //construct clique
    QList<Clique3> clique0;
    getCliques(ntList->at(0), cand[0], clique0, span);
    if(clique0.size()<0)
        return false;
    QList<Clique3> clique1;
    getCliques(ntList->at(1), cand[1], clique1, span);
    if(clique1.size()<0)
        return false;

    //perform matching based on cliques
    int cmatchcount = 0, changecount = 0;
    int bestMatch = 0;
    double cmatchThr = span;
    QList<int> matchPoint[2];
    matchPoint[0] = QList<int>(); matchPoint[0].clear();
    matchPoint[1] = QList<int>(); matchPoint[1].clear();
    QList<XYZ> candcoord0;
    QList<XYZ> candcoord1;
    QList<XYZ> tmpcoord;
    candcoord0.clear();
    for(int i=0; i<cand[0].size(); i++){
        candcoord0.append(XYZ(ntList->at(0).listNeuron.at(cand[0].at(i))));
        if(direction == 0) candcoord0[i].x = 0;
        if(direction == 1) candcoord0[i].y = 0;
        if(direction == 2) candcoord0[i].z = 0;
    }
    candcoord1.clear();
    for(int i=0; i<cand[1].size(); i++){
        candcoord1.append(XYZ(ntList->at(1).listNeuron.at(cand[1].at(i))));
        if(direction == 0) candcoord1[i].x = 0;
        if(direction == 1) candcoord1[i].y = 0;
        if(direction == 2) candcoord1[i].z = 0;
    }
    QList<int> tmpMatchMarkers[2];
    tmpMatchMarkers[0]=QList<int>();
    tmpMatchMarkers[1]=QList<int>();
    QProgressDialog progressDial("Performing Global Search... Skipping search may generate wrong result.","Skipping Search",0,clique0.size(),0);
    progressDial.setWindowModality(Qt::WindowModal);
    for(int i=0; i<clique0.size(); i++){
        progressDial.setValue(i);
        for(int j=0; j<clique1.size(); j++){
            //find matching clique
            double cdis = fabs(clique0[i].e[0]-clique1[j].e[0]) + fabs(clique0[i].e[1]-clique1[j].e[1]) + fabs(clique0[i].e[2]-clique1[j].e[2]);
            if(cdis>cmatchThr)
                continue;
            cmatchcount++;
            //get affinement between cliques
            shift_x = 0; shift_y = 0; shift_z = 0; cent_x = 0; cent_y = 0; cent_z = 0; angle = 0;
            if(direction == 0) shift_x = -1;
            if(direction == 1) shift_y = -1;
            if(direction == 2) shift_z = -1;

            QList<XYZ> c0,c1;
            c0.clear();
            c0.append(XYZ(candcoord0[clique0[i].idx[0]]));
            c0.append(XYZ(candcoord0[clique0[i].idx[1]]));
            c0.append(XYZ(candcoord0[clique0[i].idx[2]]));
            c1.clear();
            c1.append(XYZ(candcoord1[clique1[j].idx[0]]));
            c1.append(XYZ(candcoord1[clique1[j].idx[1]]));
            c1.append(XYZ(candcoord1[clique1[j].idx[2]]));
            if(compute_affine_4dof(c0,c1,shift_x,shift_y,shift_z,angle,cent_x,cent_y,cent_z,direction)<0)
                continue;
            //find the matched point by clique
            affine_XYZList(candcoord1, tmpcoord, shift_x, shift_y, shift_z, angle, cent_x, cent_y, cent_z, direction);
            getMatchPairs_XYZList(candcoord0, tmpcoord, tmpMatchMarkers, span);
            if(tmpMatchMarkers[0].size()>bestMatch){
                changecount++;
                bestMatch = tmpMatchMarkers[0].size();
                matchPoint[0] = tmpMatchMarkers[0];
                matchPoint[1] = tmpMatchMarkers[1];
                qDebug()<<"affine: "<<shift_x<<":"<<shift_y<<":"<<shift_z<<":"<<angle<<":"<<cent_x<<":"<<cent_y<<":"<<cent_z;
                progressDial.setLabelText("Performing Global Search... Can skip when there are sufficient matched points.\n Identified "+
                                          QString::number(matchPoint[0].size()) + " matched point out of " + QString::number(cand[0].size()) + ":" + QString::number(cand[1].size()));
            }
        }
        if(progressDial.wasCanceled()) break;
    }
    progressDial.setValue(clique0.size());

    qDebug()<<"match and search: "<<matchPoint[0].size()<<" matched point, "<<cmatchcount<<" similar cliques found in the first searching round";

    if(matchPoint[0].size()<0){
        return false;
    }

    //affine by all matched points and try to find more
    QList<XYZ> matchcoord0, matchcoord1;
    do{
        qDebug()<<"new search round, match point number: "<<matchPoint[0].size();
        matchcoord0.clear();
        matchcoord1.clear();
        for(int i=0; i<matchPoint[0].size(); i++){
            matchcoord0.append(XYZ(ntList->at(0).listNeuron.at(cand[0].at(matchPoint[0].at(i)))));
            matchcoord1.append(XYZ(ntList->at(1).listNeuron.at(cand[1].at(matchPoint[1].at(i)))));
        }
        shift_x = 0; shift_y = 0; shift_z = 0; cent_x = 0; cent_y = 0; cent_z = 0; angle = 0;
        if(direction == 0) shift_x = -1;
        if(direction == 1) shift_y = -1;
        if(direction == 2) shift_z = -1;
        if(compute_affine_4dof(matchcoord0,matchcoord1,shift_x,shift_y,shift_z,angle,cent_x,cent_y,cent_z,direction)<0){
            qDebug("section affine process failed, please check the code");
            return false;
        }
        qDebug()<<"affine: "<<shift_x<<":"<<shift_y<<":"<<shift_z<<":"<<angle<<":"<<cent_x<<":"<<cent_y<<":"<<cent_z;
        affine_XYZList(candcoord1, tmpcoord, shift_x,shift_y,shift_z,angle,cent_x,cent_y,cent_z, direction);
        getMatchPairs_XYZList(candcoord0, tmpcoord, tmpMatchMarkers, span);
        if(matchPoint[0].size() == tmpMatchMarkers[0].size()){
            matchPoint[0] = tmpMatchMarkers[0];
            matchPoint[1] = tmpMatchMarkers[1];
            break;
        }else if(matchPoint[0].size() > tmpMatchMarkers[0].size()){
            qDebug("matching point number reduced, quit second round optimization");
            break;
        }else{
            matchPoint[0] = tmpMatchMarkers[0];
            matchPoint[1] = tmpMatchMarkers[1];
        }
    }while(1);
    qDebug()<<"match and search: "<<matchPoint[0].size()<<" matched point found in the second searching round";

    //populate MatchMarkers
    for(int i=0; i<matchPoint[0].size(); i++){
        MatchMarkers[0].append(cand[0].at(matchPoint[0].at(i)));
        MatchMarkers[1].append(cand[1].at(matchPoint[1].at(i)));
    }
    return true;
}

bool matchCandidates_speed(QList<NeuronTree> * ntList, QList<int> * cand, double span, int direction, QList<int> MatchMarkers[2]) //from cand[1] to cand[0]
{
    MatchMarkers[0].clear();
    MatchMarkers[1].clear();

    //construct clique
    QList<Clique3> clique0;
    getCliques(ntList->at(0), cand[0], clique0, span);
    if(clique0.size()<0)
        return false;
    QList<Clique3> clique1;
    getCliques(ntList->at(1), cand[1], clique1, span);
    if(clique1.size()<0)
        return false;

    //perform matching based on cliques
    //initialize
    int cmatchcount = 0, pmatchcount = 0, changecount = 0;
    int bestMatch = 0;
    double cmatchThr = span;
    double shift_x = 0, shift_y = 0, shift_z = 0, cent_x = 0, cent_y = 0, cent_z = 0, angle = 0;
    //cooridnates of candidates
    QList<XYZ> candcoord0;
    QList<XYZ> candcoord1;
    QList<XYZ> tmpcoord;
    candcoord0.clear();
    for(int i=0; i<cand[0].size(); i++){
        candcoord0.append(XYZ(ntList->at(0).listNeuron.at(cand[0].at(i))));
    }
    candcoord1.clear();
    for(int i=0; i<cand[1].size(); i++){
        candcoord1.append(XYZ(ntList->at(1).listNeuron.at(cand[1].at(i))));
    }
    //pairs of points matched
    QList<int> matchPoint[2];
    matchPoint[0] = QList<int>(); matchPoint[0].clear();
    matchPoint[1] = QList<int>(); matchPoint[1].clear();
    QList<int> tmpMatchMarkers[2],tmpMatchMarkers1[2];
    tmpMatchMarkers[0]=QList<int>();
    tmpMatchMarkers[1]=QList<int>();
    tmpMatchMarkers1[0]=QList<int>();
    tmpMatchMarkers1[1]=QList<int>();
    //record the points that has already been matched to current one
    QVector<QList<int> > MatchRecord0;
    MatchRecord0.resize(cand[0].size());
    //progress bar
    QProgressDialog progressDial("Performing Global Search... Skipping now may generate wrong result.","Skipping Search",0,clique0.size(),0);
    progressDial.setWindowModality(Qt::WindowModal);

    //start global search
    for(int i=0; i<clique0.size(); i++){
        progressDial.setValue(i);
        for(int j=0; j<clique1.size(); j++){
            //find matching clique
            double cdis = fabs(clique0[i].e[0]-clique1[j].e[0]) + fabs(clique0[i].e[1]-clique1[j].e[1]) + fabs(clique0[i].e[2]-clique1[j].e[2]);
            if(cdis>cmatchThr)
                continue;
            //to-do check clique direction for matching

            cmatchcount++;

            //continue if the clique has already been matched before
            if(MatchRecord0[clique0[i].idx[0]].indexOf(clique1[j].idx[0])>=0 &&
                    MatchRecord0[clique0[i].idx[1]].indexOf(clique1[j].idx[1])>=0 &&
                    MatchRecord0[clique0[i].idx[2]].indexOf(clique1[j].idx[2])>=0)
                continue;

            pmatchcount++;

            //get affinement between cliques
            shift_x = 0; shift_y = 0; shift_z = 0; cent_x = 0; cent_y = 0; cent_z = 0; angle = 0;
            if(direction == 0) shift_x = -1;
            if(direction == 1) shift_y = -1;
            if(direction == 2) shift_z = -1;

            QList<XYZ> c0,c1;
            c0.clear();
            c0.append(XYZ(candcoord0[clique0[i].idx[0]]));
            c0.append(XYZ(candcoord0[clique0[i].idx[1]]));
            c0.append(XYZ(candcoord0[clique0[i].idx[2]]));
            c1.clear();
            c1.append(XYZ(candcoord1[clique1[j].idx[0]]));
            c1.append(XYZ(candcoord1[clique1[j].idx[1]]));
            c1.append(XYZ(candcoord1[clique1[j].idx[2]]));
            if(compute_affine_4dof(c0,c1,shift_x,shift_y,shift_z,angle,cent_x,cent_y,cent_z,direction)<0)
                continue;
            //find the matched point by clique
            affine_XYZList(candcoord1, tmpcoord, shift_x, shift_y, shift_z, angle, cent_x, cent_y, cent_z, direction);
            getMatchPairs_XYZList(candcoord0, tmpcoord, tmpMatchMarkers, span);

            if(tmpMatchMarkers[0].size()<=matchPoint[0].size()/2)
                continue;
            //iterative optimization for current matched points
            do{
                c0.clear();
                c1.clear();
                for(int i=0; i<tmpMatchMarkers[0].size(); i++){
                    c0.append(XYZ(candcoord0[tmpMatchMarkers[0].at(i)]));
                    c1.append(XYZ(candcoord1[tmpMatchMarkers[1].at(i)]));
                }
                shift_x = 0; shift_y = 0; shift_z = 0; cent_x = 0; cent_y = 0; cent_z = 0; angle = 0;
                if(direction == 0) shift_x = -1;
                if(direction == 1) shift_y = -1;
                if(direction == 2) shift_z = -1;
                if(compute_affine_4dof(c0,c1,shift_x,shift_y,shift_z,angle,cent_x,cent_y,cent_z,direction)<0){
                    qDebug("section affine process failed, please check the code");
                    return false;
                }
                qDebug()<<"new inner search round: "<<tmpMatchMarkers[0].size()<<":"<<cmatchcount<<":"<<pmatchcount<<":"<<changecount<<"; affine: "<<shift_x<<":"<<shift_y<<":"<<shift_z<<":"<<angle<<":"<<cent_x<<":"<<cent_y<<":"<<cent_z;
                affine_XYZList(candcoord1, tmpcoord, shift_x,shift_y,shift_z,angle,cent_x,cent_y,cent_z, direction);
                getMatchPairs_XYZList(candcoord0, tmpcoord, tmpMatchMarkers1, span);
                if(tmpMatchMarkers1[0].size() == tmpMatchMarkers[0].size()){
                    tmpMatchMarkers[0] = tmpMatchMarkers1[0];
                    tmpMatchMarkers[1] = tmpMatchMarkers1[1];
                    qDebug("no further optimization, quit inner round optimization");
                    break;
                }else if(tmpMatchMarkers1[0].size() < tmpMatchMarkers[0].size()){
                    qDebug("matching point number reduced, quit inner round optimization");
                    break;
                }else{
                    tmpMatchMarkers[0] = tmpMatchMarkers1[0];
                    tmpMatchMarkers[1] = tmpMatchMarkers1[1];
                }
            }while(1);
            if(tmpMatchMarkers[0].size()>bestMatch){
                bestMatch = tmpMatchMarkers[0].size();
                matchPoint[0] = tmpMatchMarkers[0];
                matchPoint[1] = tmpMatchMarkers[1];
                qDebug()<<"matched: "<<cmatchcount<<":"<<pmatchcount<<"; affine: "<<shift_x<<":"<<shift_y<<":"<<shift_z<<":"<<angle<<":"<<cent_x<<":"<<cent_y<<":"<<cent_z;
                progressDial.setLabelText("Performing Global Search... Can skip when there are sufficient matched points.\n Identified "+
                                          QString::number(matchPoint[0].size()) + " matched point out of " + QString::number(cand[0].size()) + ":" + QString::number(cand[1].size()));
            }
            //take record for already matched point
            for(int k=0; k<tmpMatchMarkers[0].size(); k++){
                if(MatchRecord0[tmpMatchMarkers[0].at(k)].indexOf(tmpMatchMarkers[1].at(k))<0){
                    MatchRecord0[tmpMatchMarkers[0].at(k)].append(tmpMatchMarkers[1].at(k));
                    changecount++;
                }
            }
        }
        if(progressDial.wasCanceled()) break;
    }
    progressDial.setValue(clique0.size());
    qDebug()<<"match and search: "<<matchPoint[0].size()<<" matched point, "<<cmatchcount<<" similar cliques found in the first searching round";

    if(matchPoint[0].size()<=0){
        return false;
    }

    //populate MatchMarkers
    for(int i=0; i<matchPoint[0].size(); i++){
        MatchMarkers[0].append(cand[0].at(matchPoint[0].at(i)));
        MatchMarkers[1].append(cand[1].at(matchPoint[1].at(i)));
    }
    return true;
}

void getCliques(const NeuronTree& nt, QList<int> list, QList<Clique3> & cqlist, double minDis)
{
    cqlist.clear();
    minDis=minDis*minDis;
    double maxDis=minDis*10000;
    cqlist.clear();
    for(int i=0; i<list.size(); i++){
        for(int j=i+1; j<list.size(); j++){
            double dis_ij = NTDIS(nt.listNeuron.at(list[i]),nt.listNeuron.at(list[j]));
            if(dis_ij<minDis || dis_ij>maxDis)
                continue;
            for(int k=j+1; k<list.size(); k++){
                double dis_ik = NTDIS(nt.listNeuron.at(list[i]),nt.listNeuron.at(list[k]));
                if(dis_ik<minDis || dis_ij>maxDis)
                    continue;
                double dis_jk = NTDIS(nt.listNeuron.at(list[j]),nt.listNeuron.at(list[k]));
                if(dis_jk<minDis || dis_ij>maxDis)
                    continue;
                Clique3 C;
                if(dis_ij <= dis_jk && dis_jk <= dis_ik){
                    C.v[0]=list[i]; C.idx[0]=i;
                    C.v[1]=list[j]; C.idx[1]=j;
                    C.v[2]=list[k]; C.idx[2]=k;
                    C.e[0]=sqrt(dis_ij);
                    C.e[1]=sqrt(dis_jk);
                    C.e[2]=sqrt(dis_ik);
                }else if(dis_ij <= dis_ik && dis_ik <= dis_jk){
                    C.v[0]=list[j]; C.idx[0]=j;
                    C.v[1]=list[i]; C.idx[1]=i;
                    C.v[2]=list[k]; C.idx[2]=k;
                    C.e[0]=sqrt(dis_ij);
                    C.e[1]=sqrt(dis_ik);
                    C.e[2]=sqrt(dis_jk);
                }else if(dis_jk <= dis_ij && dis_ij <= dis_ik){
                    C.v[0]=list[k]; C.idx[0]=k;
                    C.v[1]=list[j]; C.idx[1]=j;
                    C.v[2]=list[i]; C.idx[2]=i;
                    C.e[0]=sqrt(dis_jk);
                    C.e[1]=sqrt(dis_ij);
                    C.e[2]=sqrt(dis_ik);
                }else if(dis_jk <= dis_ik && dis_ik <= dis_ij){
                    C.v[0]=list[j]; C.idx[0]=j;
                    C.v[1]=list[k]; C.idx[1]=k;
                    C.v[2]=list[i]; C.idx[2]=i;
                    C.e[0]=sqrt(dis_jk);
                    C.e[1]=sqrt(dis_ik);
                    C.e[2]=sqrt(dis_ij);
                }else if(dis_ik <= dis_ij && dis_ij <= dis_jk){
                    C.v[0]=list[k]; C.idx[0]=k;
                    C.v[1]=list[i]; C.idx[1]=i;
                    C.v[2]=list[j]; C.idx[2]=j;
                    C.e[0]=sqrt(dis_ik);
                    C.e[1]=sqrt(dis_ij);
                    C.e[2]=sqrt(dis_jk);
                }else if(dis_ik <= dis_jk && dis_jk <= dis_ij){
                    C.v[0]=list[i]; C.idx[0]=i;
                    C.v[1]=list[k]; C.idx[1]=k;
                    C.v[2]=list[j]; C.idx[2]=j;
                    C.e[0]=sqrt(dis_ik);
                    C.e[1]=sqrt(dis_jk);
                    C.e[2]=sqrt(dis_ij);
                }else{
                    printf("error in construction clique, unexpected situation happened! Check the code!\n");
                    continue;
                }
//                C.c.clear();
//                C.c.append(XYZ(nt.listNeuron.at(C.v[0])));
//                C.c.append(XYZ(nt.listNeuron.at(C.v[1])));
//                C.c.append(XYZ(nt.listNeuron.at(C.v[2])));
                cqlist.append(C);
            }
        }
    }
}

void getCliques(const QList<int>& list, const QList<XYZ>& coord, const QList<XYZ>& dir, QVector<Clique3> & cqlist, double minDis,int stackDir)
{
    cqlist.clear();
    minDis=minDis*minDis;
    double maxDis=1e16;
    cqlist.clear();
    for(int i=0; i<coord.size(); i++){
        for(int j=i+1; j<coord.size(); j++){
            double dis_ij = NTDIS(coord.at(i),coord.at(j));
            if(dis_ij<minDis)// || dis_ij>maxDis)
                continue;
            for(int k=j+1; k<coord.size(); k++){
                double dis_ik = NTDIS(coord.at(i),coord.at(k));
                if(dis_ik<minDis)// || dis_ij>maxDis)
                    continue;
                double dis_jk = NTDIS(coord.at(j),coord.at(k));
                if(dis_jk<minDis)// || dis_ij>maxDis)
                    continue;
                Clique3 C;
                if(dis_ij <= dis_jk && dis_jk <= dis_ik){
                    C.v[0]=list[i]; C.idx[0]=i;
                    C.v[1]=list[j]; C.idx[1]=j;
                    C.v[2]=list[k]; C.idx[2]=k;
                    C.e[0]=sqrt(dis_ij);
                    C.e[1]=sqrt(dis_jk);
                    C.e[2]=sqrt(dis_ik);
                }else if(dis_ij <= dis_ik && dis_ik <= dis_jk){
                    C.v[0]=list[j]; C.idx[0]=j;
                    C.v[1]=list[i]; C.idx[1]=i;
                    C.v[2]=list[k]; C.idx[2]=k;
                    C.e[0]=sqrt(dis_ij);
                    C.e[1]=sqrt(dis_ik);
                    C.e[2]=sqrt(dis_jk);
                }else if(dis_jk <= dis_ij && dis_ij <= dis_ik){
                    C.v[0]=list[k]; C.idx[0]=k;
                    C.v[1]=list[j]; C.idx[1]=j;
                    C.v[2]=list[i]; C.idx[2]=i;
                    C.e[0]=sqrt(dis_jk);
                    C.e[1]=sqrt(dis_ij);
                    C.e[2]=sqrt(dis_ik);
                }else if(dis_jk <= dis_ik && dis_ik <= dis_ij){
                    C.v[0]=list[j]; C.idx[0]=j;
                    C.v[1]=list[k]; C.idx[1]=k;
                    C.v[2]=list[i]; C.idx[2]=i;
                    C.e[0]=sqrt(dis_jk);
                    C.e[1]=sqrt(dis_ik);
                    C.e[2]=sqrt(dis_ij);
                }else if(dis_ik <= dis_ij && dis_ij <= dis_jk){
                    C.v[0]=list[k]; C.idx[0]=k;
                    C.v[1]=list[i]; C.idx[1]=i;
                    C.v[2]=list[j]; C.idx[2]=j;
                    C.e[0]=sqrt(dis_ik);
                    C.e[1]=sqrt(dis_ij);
                    C.e[2]=sqrt(dis_jk);
                }else if(dis_ik <= dis_jk && dis_jk <= dis_ij){
                    C.v[0]=list[i]; C.idx[0]=i;
                    C.v[1]=list[k]; C.idx[1]=k;
                    C.v[2]=list[j]; C.idx[2]=j;
                    C.e[0]=sqrt(dis_ik);
                    C.e[1]=sqrt(dis_jk);
                    C.e[2]=sqrt(dis_ij);
                }else{ //this should not happen
                    printf("error in construction clique, unexpected situation happened! Check the code!\n");
                    continue;
                }
                XYZ center;
                center.x+=(coord[i].x+coord[j].x+coord[k].x)/3;
                center.y+=(coord[i].y+coord[j].y+coord[k].y)/3;
                center.z+=(coord[i].z+coord[j].z+coord[k].z)/3;
                for(int p=0; p<3; p++){
                    int m=C.idx[p];
                    if(stackDir == 0){//x plane
                        double py=coord[m].y-center.y;
                        double pz=coord[m].z-center.z;
                        double plen=sqrt(pz*pz+py*py);
                        C.dir[p].y=py/plen*dir[m].y-pz/plen*dir[m].z;
                        C.dir[p].z=pz/plen*dir[m].y+py/plen*dir[m].z;
                        C.dir[p].x=dir[m].x;
                    }else if(stackDir == 1){//y plane
                        double px=coord[m].x-center.x;
                        double pz=coord[m].z-center.z;
                        double plen=sqrt(pz*pz+px*px);
                        C.dir[p].z=pz/plen*dir[m].z-px/plen*dir[m].x;
                        C.dir[p].x=px/plen*dir[m].z+pz/plen*dir[m].x;
                        C.dir[p].y=dir[m].y;
                    }else if(stackDir ==2){//z plane
                        double px=coord[m].x-center.x;
                        double py=coord[m].y-center.y;
                        double plen=sqrt(px*px+py*py);
                        C.dir[p].x=px/plen*dir[m].x-py/plen*dir[m].y;
                        C.dir[p].y=py/plen*dir[m].x+px/plen*dir[m].y;
                        C.dir[p].z=dir[m].z;
                    }else{ //to-do: taken clique plan as reference plan for calculation
                        C.dir[p].x=0;
                        C.dir[p].y=0;
                        C.dir[p].z=0;
                    }
                }
                cqlist.append(C);
            }
        }
    }
}

void getCliques(const NeuronTree& nt, const QList<int>& list, const QList<XYZ>& coord, const QList<XYZ>& dir, QVector<Clique3> & cqlist, double minDis,int stackDir)
{
    cqlist.clear();
    minDis=minDis*minDis;
    cqlist.clear();
    for(int i=0; i<coord.size(); i++){
        if(nt.listNeuron.at(list.at(i)).type==16 || nt.listNeuron.at(list.at(i)).type==17)
            continue;
        for(int j=i+1; j<coord.size(); j++){
            if(nt.listNeuron.at(list.at(j)).type==16 || nt.listNeuron.at(list.at(j)).type==17)
                continue;

            double dis_ij = NTDIS(coord.at(i),coord.at(j));
            if(dis_ij<minDis)// || dis_ij>maxDis)
                continue;
            for(int k=j+1; k<coord.size(); k++){
                if(nt.listNeuron.at(list.at(k)).type==16 || nt.listNeuron.at(list.at(k)).type==17)
                    continue;

                double dis_ik = NTDIS(coord.at(i),coord.at(k));
                if(dis_ik<minDis)// || dis_ij>maxDis)
                    continue;
                double dis_jk = NTDIS(coord.at(j),coord.at(k));
                if(dis_jk<minDis)// || dis_ij>maxDis)
                    continue;
                Clique3 C;
                if(dis_ij <= dis_jk && dis_jk <= dis_ik){
                    C.v[0]=list[i]; C.idx[0]=i;
                    C.v[1]=list[j]; C.idx[1]=j;
                    C.v[2]=list[k]; C.idx[2]=k;
                    C.e[0]=sqrt(dis_ij);
                    C.e[1]=sqrt(dis_jk);
                    C.e[2]=sqrt(dis_ik);
                }else if(dis_ij <= dis_ik && dis_ik <= dis_jk){
                    C.v[0]=list[j]; C.idx[0]=j;
                    C.v[1]=list[i]; C.idx[1]=i;
                    C.v[2]=list[k]; C.idx[2]=k;
                    C.e[0]=sqrt(dis_ij);
                    C.e[1]=sqrt(dis_ik);
                    C.e[2]=sqrt(dis_jk);
                }else if(dis_jk <= dis_ij && dis_ij <= dis_ik){
                    C.v[0]=list[k]; C.idx[0]=k;
                    C.v[1]=list[j]; C.idx[1]=j;
                    C.v[2]=list[i]; C.idx[2]=i;
                    C.e[0]=sqrt(dis_jk);
                    C.e[1]=sqrt(dis_ij);
                    C.e[2]=sqrt(dis_ik);
                }else if(dis_jk <= dis_ik && dis_ik <= dis_ij){
                    C.v[0]=list[j]; C.idx[0]=j;
                    C.v[1]=list[k]; C.idx[1]=k;
                    C.v[2]=list[i]; C.idx[2]=i;
                    C.e[0]=sqrt(dis_jk);
                    C.e[1]=sqrt(dis_ik);
                    C.e[2]=sqrt(dis_ij);
                }else if(dis_ik <= dis_ij && dis_ij <= dis_jk){
                    C.v[0]=list[k]; C.idx[0]=k;
                    C.v[1]=list[i]; C.idx[1]=i;
                    C.v[2]=list[j]; C.idx[2]=j;
                    C.e[0]=sqrt(dis_ik);
                    C.e[1]=sqrt(dis_ij);
                    C.e[2]=sqrt(dis_jk);
                }else if(dis_ik <= dis_jk && dis_jk <= dis_ij){
                    C.v[0]=list[i]; C.idx[0]=i;
                    C.v[1]=list[k]; C.idx[1]=k;
                    C.v[2]=list[j]; C.idx[2]=j;
                    C.e[0]=sqrt(dis_ik);
                    C.e[1]=sqrt(dis_jk);
                    C.e[2]=sqrt(dis_ij);
                }else{ //this should not happen
                    printf("error in construction clique, unexpected situation happened! Check the code!\n");
                    continue;
                }
                XYZ center;
                center.x+=(coord[i].x+coord[j].x+coord[k].x)/3;
                center.y+=(coord[i].y+coord[j].y+coord[k].y)/3;
                center.z+=(coord[i].z+coord[j].z+coord[k].z)/3;
                for(int p=0; p<3; p++){
                    int m=C.idx[p];
                    if(stackDir == 0){//x plane
                        double py=coord[m].y-center.y;
                        double pz=coord[m].z-center.z;
                        double plen=sqrt(pz*pz+py*py);
                        C.dir[p].y=py/plen*dir[m].y-pz/plen*dir[m].z;
                        C.dir[p].z=pz/plen*dir[m].y+py/plen*dir[m].z;
                        C.dir[p].x=dir[m].x;
                    }else if(stackDir == 1){//y plane
                        double px=coord[m].x-center.x;
                        double pz=coord[m].z-center.z;
                        double plen=sqrt(pz*pz+px*px);
                        C.dir[p].z=pz/plen*dir[m].z-px/plen*dir[m].x;
                        C.dir[p].x=px/plen*dir[m].z+pz/plen*dir[m].x;
                        C.dir[p].y=dir[m].y;
                    }else if(stackDir ==2){//z plane
                        double px=coord[m].x-center.x;
                        double py=coord[m].y-center.y;
                        double plen=sqrt(px*px+py*py);
                        C.dir[p].x=px/plen*dir[m].x-py/plen*dir[m].y;
                        C.dir[p].y=py/plen*dir[m].x+px/plen*dir[m].y;
                        C.dir[p].z=dir[m].z;
                    }else{ //to-do: taken clique plan as reference plan for calculation
                        C.dir[p].x=0;
                        C.dir[p].y=0;
                        C.dir[p].z=0;
                    }
                }
                cqlist.append(C);
            }
        }
    }
}

void getTopCliques(const NeuronTree& nt, const QList<int>& list, const QList<XYZ>& coord, const QList<XYZ>& dir, QVector<Clique3> & cqlist, double minDis,int stackDir, int maxCliqueNum)
{
    cqlist.clear();
    minDis=minDis*minDis;
    multimap<double, Clique3, std::greater<double> > cliqueMap;
    for(int i=0; i<coord.size(); i++){
        if(nt.listNeuron.at(list.at(i)).type==16 || nt.listNeuron.at(list.at(i)).type==17)
            continue;
        for(int j=i+1; j<coord.size(); j++){
            if(nt.listNeuron.at(list.at(j)).type==16 || nt.listNeuron.at(list.at(j)).type==17)
                continue;

            double dis_ij = NTDIS(coord.at(i),coord.at(j));
            if(dis_ij<minDis)// || dis_ij>maxDis)
                continue;
            for(int k=j+1; k<coord.size(); k++){
                if(nt.listNeuron.at(list.at(k)).type==16 || nt.listNeuron.at(list.at(k)).type==17)
                    continue;

                double dis_ik = NTDIS(coord.at(i),coord.at(k));
                if(dis_ik<minDis)// || dis_ij>maxDis)
                    continue;
                double dis_jk = NTDIS(coord.at(j),coord.at(k));
                if(dis_jk<minDis)// || dis_ij>maxDis)
                    continue;
                Clique3 C;
                if(dis_ij <= dis_jk && dis_jk <= dis_ik){
                    C.v[0]=list[i]; C.idx[0]=i;
                    C.v[1]=list[j]; C.idx[1]=j;
                    C.v[2]=list[k]; C.idx[2]=k;
                    C.e[0]=sqrt(dis_ij);
                    C.e[1]=sqrt(dis_jk);
                    C.e[2]=sqrt(dis_ik);
                }else if(dis_ij <= dis_ik && dis_ik <= dis_jk){
                    C.v[0]=list[j]; C.idx[0]=j;
                    C.v[1]=list[i]; C.idx[1]=i;
                    C.v[2]=list[k]; C.idx[2]=k;
                    C.e[0]=sqrt(dis_ij);
                    C.e[1]=sqrt(dis_ik);
                    C.e[2]=sqrt(dis_jk);
                }else if(dis_jk <= dis_ij && dis_ij <= dis_ik){
                    C.v[0]=list[k]; C.idx[0]=k;
                    C.v[1]=list[j]; C.idx[1]=j;
                    C.v[2]=list[i]; C.idx[2]=i;
                    C.e[0]=sqrt(dis_jk);
                    C.e[1]=sqrt(dis_ij);
                    C.e[2]=sqrt(dis_ik);
                }else if(dis_jk <= dis_ik && dis_ik <= dis_ij){
                    C.v[0]=list[j]; C.idx[0]=j;
                    C.v[1]=list[k]; C.idx[1]=k;
                    C.v[2]=list[i]; C.idx[2]=i;
                    C.e[0]=sqrt(dis_jk);
                    C.e[1]=sqrt(dis_ik);
                    C.e[2]=sqrt(dis_ij);
                }else if(dis_ik <= dis_ij && dis_ij <= dis_jk){
                    C.v[0]=list[k]; C.idx[0]=k;
                    C.v[1]=list[i]; C.idx[1]=i;
                    C.v[2]=list[j]; C.idx[2]=j;
                    C.e[0]=sqrt(dis_ik);
                    C.e[1]=sqrt(dis_ij);
                    C.e[2]=sqrt(dis_jk);
                }else if(dis_ik <= dis_jk && dis_jk <= dis_ij){
                    C.v[0]=list[i]; C.idx[0]=i;
                    C.v[1]=list[k]; C.idx[1]=k;
                    C.v[2]=list[j]; C.idx[2]=j;
                    C.e[0]=sqrt(dis_ik);
                    C.e[1]=sqrt(dis_jk);
                    C.e[2]=sqrt(dis_ij);
                }else{ //this should not happen
                    printf("error in construction clique, unexpected situation happened! Check the code!\n");
                    continue;
                }
                double score=(C.e[1]-C.e[0])<(C.e[2]-C.e[1])?(C.e[1]-C.e[0]):(C.e[2]-C.e[1]);
                score/=C.e[0]+C.e[1]+C.e[2];
                cliqueMap.insert(pair<double, Clique3>(score, C));
            }
        }
    }

    int count=0;
    for(multimap<double, Clique3, std::greater<double> >::iterator iter=cliqueMap.begin(); iter!=cliqueMap.end(); iter++){
        XYZ center;
        int i=iter->second.idx[0];
        int j=iter->second.idx[1];
        int k=iter->second.idx[2];
        center.x+=(coord[i].x+coord[j].x+coord[k].x)/3;
        center.y+=(coord[i].y+coord[j].y+coord[k].y)/3;
        center.z+=(coord[i].z+coord[j].z+coord[k].z)/3;
        for(int p=0; p<3; p++){
            int m=iter->second.idx[p];
            if(stackDir == 0){//x plane
                double py=coord[m].y-center.y;
                double pz=coord[m].z-center.z;
                double plen=sqrt(pz*pz+py*py);
                iter->second.dir[p].y=py/plen*dir[m].y-pz/plen*dir[m].z;
                iter->second.dir[p].z=pz/plen*dir[m].y+py/plen*dir[m].z;
                iter->second.dir[p].x=dir[m].x;
            }else if(stackDir == 1){//y plane
                double px=coord[m].x-center.x;
                double pz=coord[m].z-center.z;
                double plen=sqrt(pz*pz+px*px);
                iter->second.dir[p].z=pz/plen*dir[m].z-px/plen*dir[m].x;
                iter->second.dir[p].x=px/plen*dir[m].z+pz/plen*dir[m].x;
                iter->second.dir[p].y=dir[m].y;
            }else if(stackDir ==2){//z plane
                double px=coord[m].x-center.x;
                double py=coord[m].y-center.y;
                double plen=sqrt(px*px+py*py);
                iter->second.dir[p].x=px/plen*dir[m].x-py/plen*dir[m].y;
                iter->second.dir[p].y=py/plen*dir[m].x+px/plen*dir[m].y;
                iter->second.dir[p].z=dir[m].z;
            }else{ //to-do: taken clique plan as reference plan for calculation
                iter->second.dir[p].x=0;
                iter->second.dir[p].y=0;
                iter->second.dir[p].z=0;
            }
        }
        cqlist.append(iter->second);
        count++;
        if(count>=maxCliqueNum){
            break;
        }
    }
}

void getMatchingCandidates(const NeuronTree& nt, QList<int>& cand, float min, float max, int direction)
{
    cand.clear();
    QVector<int> childNum(nt.listNeuron.size(), 0);
    for(int i=0; i<nt.listNeuron.size(); i++){
        if(nt.listNeuron.at(i).pn<0){
            childNum[i]--; //root that only have 1 clide will also be a dead end
        }
        else{
            int pid = nt.hashNeuron.value(nt.listNeuron.at(i).pn);
            childNum[pid]++;
        }
    }
    for(int i=0; i<childNum.size(); i++){
        if(childNum[i]<=0){
            if(direction==0){//x
                if(nt.listNeuron.at(i).x>min && nt.listNeuron.at(i).x<max){
                    cand.append(i);
                }
            }else if(direction==1){//y
                if(nt.listNeuron.at(i).y>min && nt.listNeuron.at(i).y<max){
                    cand.append(i);
                }
            }else if(direction==2){//z
                if(nt.listNeuron.at(i).z>min && nt.listNeuron.at(i).z<max){
                    cand.append(i);
                }
            }else{//all tips
                cand.append(i);
            }
        }
    }
}

void getMatchingCandidates(const NeuronTree& nt, QList<int>& cand, float min, float max, int direction, float segThr)
{
    cand.clear();
    QVector<int> childNum(nt.listNeuron.size(), 0);
    QVector<int> connNum(nt.listNeuron.size(), 0);
    QVector<double> sectionLength(nt.listNeuron.size(), 0);
    QList<int> components;
    QList<int> pList;
    QVector<V3DLONG> componentSize;
    QVector<V3DLONG> componentLength;
    V3DLONG curid=0;
    for(V3DLONG i=0; i<nt.listNeuron.size(); i++){
        if(nt.listNeuron.at(i).pn<0){
            connNum[i]--; //root that only have 1 clide will also be a dead end
            components.append(curid); curid++;
            pList.append(-1);
        }
        else{
            int pid = nt.hashNeuron.value(nt.listNeuron.at(i).pn);
            childNum[pid]++;
            connNum[pid]++;
            sectionLength[i]=sqrt(NTDIS(nt.listNeuron.at(i),nt.listNeuron.at(pid)));
            components.append(-1);
            pList.append(pid);
        }
    }
    //connected component
    for(V3DLONG cid=0; cid<curid; cid++){
        QStack<int> pstack;
        int chid, size = 0;
        if(!components.contains(cid)) //should not happen, just in case
            continue;
        if(components.indexOf(cid)!=components.lastIndexOf(cid)) //should not happen
            qDebug("unexpected multiple tree root, please check the code: neuron_stitch_func.cpp");
        //recursively search for child and mark them as the same component
        pstack.push(components.indexOf(cid));
        size++;
        while(!pstack.isEmpty()){
            int pid=pstack.pop();
            chid = pList.indexOf(pid);
            while(chid>=0){
                pList[chid]=-2;
                pstack.push(chid);
                components[chid]=cid;
                chid=pList.indexOf(pid);
                size++;
            }
        }
        componentSize.append(size);
    }
    //component size
    for(V3DLONG cid=0; cid<curid; cid++){
        double length = 0;
        int idx = -1;
        for(V3DLONG i=0; i<componentSize[cid]; i++){
            idx = components.indexOf(cid,idx+1);
            length+=sectionLength[idx];
        }
        componentLength.append(length);
    }


    for(V3DLONG i=0; i<childNum.size(); i++){
        if(connNum[i]<=0 && componentLength[components[i]]>=segThr){
            if(direction==0){//x
                if(nt.listNeuron.at(i).x>min && nt.listNeuron.at(i).x<max){
                    cand.append(i);
                }
            }else if(direction==1){//y
                if(nt.listNeuron.at(i).y>min && nt.listNeuron.at(i).y<max){
                    cand.append(i);
                }
            }else if(direction==2){//z
                if(nt.listNeuron.at(i).z>min && nt.listNeuron.at(i).z<max){
                    cand.append(i);
                }
            }else{//all tips
                cand.append(i);
            }
        }
    }
}

bool minus_XYZList(QList<XYZ>& a, QList<XYZ>& b, QList<XYZ>& out)
{
    if(a.size()!=b.size()){
        return false;
    }
    if(out.size()!=a.size()){
        out.clear();
        for(int i=0; i<a.size(); i++){
            out.append(XYZ(a.at(i).x-b.at(i).x,a.at(i).y-b.at(i).y,a.at(i).z-b.at(i).z));
        }
    }else{
        for(int i=0; i<a.size(); i++){
            out[i].x=a.at(i).x-b.at(i).x;
            out[i].y=a.at(i).y-b.at(i).y;
            out[i].z=a.at(i).z-b.at(i).z;
        }
    }
    return true;
}

void getMatchPairs_XYZList(const QList<XYZ>& c0, const QList<XYZ>& c1, const QList<XYZ>& c0_dir, const QList<XYZ>& c1_dir, const QList<int>& c0_conncomponent, const QList<int>& c1_conncomponent, QList<int> * MatchMarkers, double span, double cos_angle)
{
    double thr = span*span;
    QMap<double, QList<int> > MatchPoints;
    MatchMarkers[0].clear();
    MatchMarkers[1].clear();

    for(int i=0; i<c0.size(); i++){
        for(int j=0; j<c1.size(); j++){
            double dis = NTDIS(c0.at(i),c1.at(j));
            if(dis>thr) continue;
            double ang = NTDOT(c0_dir.at(i),c1_dir.at(j));
            if(ang<cos_angle) continue;
            QList<int> tmp = QList<int>()<<i<<j;
            MatchPoints.insertMulti(dis, tmp);
        }
    }

    //this is to avoid loop in matched point
    QList<int> cctmp;
    int ccmax=0;
    int c0_size=c0.size();
    for(int i=0; i<c0_conncomponent.size(); i++){
        ccmax=ccmax>c0_conncomponent.at(i)?ccmax:c0_conncomponent.at(i);
        cctmp.append(c0_conncomponent.at(i));
    }
    ccmax+=10;
    for(int i=0; i<c1_conncomponent.size(); i++){
        cctmp.append(c1_conncomponent.at(i)+ccmax);
    }

    QVector<int> mask0(c0.size(),0);
    QVector<int> mask1(c1.size(),0);
    for(QMap<double, QList<int> >::Iterator iter = MatchPoints.begin(); iter!=MatchPoints.end(); iter++){
        int a = iter.value().at(0);
        int b = iter.value().at(1);
        if(mask0[a]+mask1[b]>0)
            continue;
        if(cctmp[b+c0_size]==cctmp[a])
            continue;

        mask0[a]++;
        mask1[b]++;

        int tmpccid=cctmp.at(b+c0_size);
        int idx=cctmp.indexOf(tmpccid);
        while(idx>=0){
            cctmp[idx]=cctmp.at(a);
            idx=cctmp.indexOf(tmpccid);
        }

        MatchMarkers[0].append(a);
        MatchMarkers[1].append(b);
    }
}

void getMatchPairs_XYZList(const QList<XYZ>& c0, const QList<XYZ>& c1, const QList<XYZ>& c0_dir, const QList<XYZ>& c1_dir, QList<int> * MatchMarkers, double span, double cos_angle)
{
    double thr = span*span;
    QMap<double, QList<int> > MatchPoints;
    MatchMarkers[0].clear();
    MatchMarkers[1].clear();

    for(int i=0; i<c0.size(); i++){
        for(int j=0; j<c1.size(); j++){
            double dis = NTDIS(c0.at(i),c1.at(j));
            if(dis>thr) continue;
            double ang = NTDOT(c0_dir.at(i),c1_dir.at(j));
            if(ang<cos_angle) continue;
            QList<int> tmp = QList<int>()<<i<<j;
            MatchPoints.insertMulti(dis, tmp);
        }
    }
    QVector<int> mask0(c0.size(),0);
    QVector<int> mask1(c1.size(),0);
    for(QMap<double, QList<int> >::Iterator iter = MatchPoints.begin(); iter!=MatchPoints.end(); iter++){
        int a = iter.value().at(0);
        int b = iter.value().at(1);
        if(mask0[a]+mask1[b]>0)
            continue;
        mask0[a]++;
        mask1[b]++;
        MatchMarkers[0].append(a);
        MatchMarkers[1].append(b);
    }
}


void getMatchPairs_XYZList(const QList<XYZ>& c0, const QList<XYZ>& c1, QList<int> * MatchMarkers, double span)
{
    double thr = span*span;
    QMap<double, QList<int> > MatchPoints;
    MatchMarkers[0].clear();
    MatchMarkers[1].clear();

    for(int i=0; i<c0.size(); i++){
        for(int j=0; j<c1.size(); j++){
            double dis = NTDIS(c0[i],c1[j]);
            if(dis>thr) continue;
            while(MatchPoints.find(dis) != MatchPoints.end()){
                dis+=1e-10;
            }
            QList<int> tmp = QList<int>()<<i<<j;
            MatchPoints.insert(dis, tmp);
        }
    }
    QVector<int> mask0(c0.size(),0);
    QVector<int> mask1(c1.size(),0);
    for(QMap<double, QList<int> >::Iterator iter = MatchPoints.begin(); iter!=MatchPoints.end(); iter++){
        int a = iter.value().at(0);
        int b = iter.value().at(1);
        if(mask0[a]+mask1[b]>0)
            continue;
        mask0[a]++;
        mask1[b]++;
        MatchMarkers[0].append(a);
        MatchMarkers[1].append(b);
    }
}

double distance_XYZList(QList<XYZ> c0, QList<XYZ> c1)
{
    if(c0.size()!=c1.size()){
        return -1;
    }
    double dis = 0;
    for(int i=0; i<c0.size(); i++){
        dis+=sqrt(NTDIS(c0[i],c1[i]));
    }
    return dis/c0.size();
}

void rotate_XYZList(QList<XYZ> in, QList<XYZ>& out, double angle, int axis) //angle is 0-360  based
{
    if(in.size()!=out.size()){
        out.clear();
        for(int i=0; i<in.size(); i++){
            out.append(XYZ(in[i]));
        }
    }
    double a = angle/180*M_PI;
    double afmatrix[12] = {1,0,0,0, 0,1,0,0, 0,0,1,0};

    if(axis ==0){
        afmatrix[5] = cos(a); afmatrix[6] = -sin(a);
        afmatrix[9] = -afmatrix[6]; afmatrix[10] = afmatrix[5];
    }
    else if(axis ==1){
        afmatrix[0] = cos(a); afmatrix[2] = sin(a);
        afmatrix[8] = -afmatrix[2]; afmatrix[10] = afmatrix[0];
    }
    else if(axis ==2){
        afmatrix[0] = cos(a); afmatrix[1] = -sin(a);
        afmatrix[4] = -afmatrix[1]; afmatrix[5] = afmatrix[0];
    }

    double x,y,z;
    for(int i=0; i<in.size();i++){
        x=in[i].x; y=in[i].y; z=in[i].z;
        out[i].x = afmatrix[0]*x+afmatrix[1]*y+afmatrix[2]*z;
        out[i].y = afmatrix[4]*x+afmatrix[5]*y+afmatrix[6]*z;
        out[i].z = afmatrix[8]*x+afmatrix[9]*y+afmatrix[10]*z;
    }
}

void affine_XYZList(const QList<XYZ>& in, QList<XYZ>& out, double shift_x, double shift_y, double shift_z, double angle, double cent_x, double cent_y, double cent_z, int axis) //angle is 0-360  based
{
    QList<XYZ> tmp;
    if(in.size()!=out.size()){
        out.clear();
        for(int i=0; i<in.size(); i++){
            out.append(XYZ(in[i]));
        }
    }

    //shift and adjust center
    for(int i=0; i<in.size(); i++){
        tmp.append(XYZ(in[i].x+shift_x-cent_x, in[i].y+shift_y-cent_y, in[i].z+shift_z-cent_z));
    }

    //rotation
    double a = angle/180*M_PI;
    double afmatrix[12] = {1,0,0,0, 0,1,0,0, 0,0,1,0};

    if(axis ==0){
        afmatrix[5] = cos(a); afmatrix[6] = -sin(a);
        afmatrix[9] = -afmatrix[6]; afmatrix[10] = afmatrix[5];
    }
    else if(axis ==1){
        afmatrix[0] = cos(a); afmatrix[2] = sin(a);
        afmatrix[8] = -afmatrix[2]; afmatrix[10] = afmatrix[0];
    }
    else if(axis ==2){
        afmatrix[0] = cos(a); afmatrix[1] = -sin(a);
        afmatrix[4] = -afmatrix[1]; afmatrix[5] = afmatrix[0];
    }

    for(int i=0; i<in.size();i++){
        out[i].x = afmatrix[0]*tmp[i].x+afmatrix[1]*tmp[i].y+afmatrix[2]*tmp[i].z + cent_x;
        out[i].y = afmatrix[4]*tmp[i].x+afmatrix[5]*tmp[i].y+afmatrix[6]*tmp[i].z + cent_y;
        out[i].z = afmatrix[8]*tmp[i].x+afmatrix[9]*tmp[i].y+afmatrix[10]*tmp[i].z + cent_z;
    }
}

double compute_affine_4dof(QList<XYZ> c0, QList<XYZ> c1, double& shift_x, double& shift_y, double & shift_z, double & angle_r, double& cent_x,double& cent_y,double& cent_z, int dir) //angle_r is 180 based
{
    //check
    if(c0.size() != c1.size()){
        qDebug()<<"error: the number of points for affine does not match";
        shift_x = shift_y = shift_z = angle_r = 0;
        cent_x=cent_y=cent_z = 0;
        return -1;
    }
    //get center
    double cent0[3] = {0};
    double cent1[3] = {0};
    for(int i=0; i<c0.size(); i++){
        cent0[0]+=c0[i].x;
        cent0[1]+=c0[i].y;
        cent0[2]+=c0[i].z;
        cent1[0]+=c1[i].x;
        cent1[1]+=c1[i].y;
        cent1[2]+=c1[i].z;
    }
    cent0[0]/=c0.size(); cent0[1]/=c0.size(); cent0[2]/=c0.size();
    cent1[0]/=c0.size(); cent1[1]/=c0.size(); cent1[2]/=c0.size();

    //align by center first, if shift_* is not 0, then shift will not be taken in this direction
    for(int i=0; i<c0.size(); i++){
        if(shift_x==0){
            c0[i].x -= cent0[0];
            c1[i].x -= cent1[0];
        }
        if(shift_y==0){
            c0[i].y -= cent0[1];
            c1[i].y -= cent1[1];
        }
        if(shift_z==0){
            c0[i].z -= cent0[2];
            c1[i].z -= cent1[2];
        }
    }
    if(shift_x==0){
        shift_x=cent0[0]-cent1[0];
        cent_x=cent0[0];
    }else{
        shift_x=0;
        cent_x=(cent0[0]+cent1[0])/2;
    }
    if(shift_y==0){
        shift_y=cent0[1]-cent1[1];
        cent_y=cent0[1];
    }else{
        shift_y=0;
        cent_y=(cent0[1]+cent1[1])/2;
    }
    if(shift_z==0){
        shift_z=cent0[2]-cent1[2];
        cent_z=cent0[2];
    }else{
        shift_z=0;
        cent_z=(cent0[2]+cent1[2])/2;
    }

    //3 steps rotation
    if(c0.size()<2){
        angle_r=0;
        return true;
    }
    QList<XYZ> c1bk; c1bk.clear();
    for(int i=0; i<c0.size(); i++){
        c1bk.append(XYZ(c1[i]));
    }
    //step 1
    double mdis=distance_XYZList(c0,c1);
    double mang=0;
    for(double ang=10; ang<360; ang+=10){
        rotate_XYZList(c1,c1bk,ang,dir);
        double dis = distance_XYZList(c0, c1bk);
        if(dis<mdis){
            mdis=dis;
            mang=ang;
        }
    }
    //qDebug()<<"rotation 0: "<<mang<<":"<<mdis;
    //step 2
    for(double ang=mang-10; ang<mang+10; ang++){
        rotate_XYZList(c1,c1bk,ang,dir);
        double dis = distance_XYZList(c0, c1bk);
        if(dis<mdis){
            mdis=dis;
            mang=ang;
        }
    }
    //qDebug()<<"rotation 1: "<<mang<<":"<<mdis;
    //step 3
    for(double ang=mang-1; ang<mang+1; ang+=0.1){
        rotate_XYZList(c1,c1bk,ang,dir);
        double dis = distance_XYZList(c0, c1bk);
        if(dis<mdis){
            mdis=dis;
            mang=ang;
        }
    }
    //qDebug()<<"rotation 2: "<<mang<<":"<<mdis;
    //step 4
    for(double ang=mang-0.1; ang<mang+0.1; ang+=0.01){
        rotate_XYZList(c1,c1bk,ang,dir);
        double dis = distance_XYZList(c0, c1bk);
        if(dis<mdis){
            mdis=dis;
            mang=ang;
        }
    }
    angle_r=mang;
    //qDebug()<<"calculated affine: "<<shift_x<<":"<<shift_y<<":"<<shift_z<<":"<<angle_r<<":"<<mdis;
    return mdis;
}

bool compute_rotation(QList<XYZ> c0, QList<XYZ> c1, double & angle_r, int dir)
{
    //check
    if(c0.size() != c1.size()){
        qDebug()<<"error: the number of points for affine does not match";
        angle_r = 0;
        return false;
    }

    //3 steps rotation
    if(c0.size()<2){
        angle_r=0;
        return true;
    }
    QList<XYZ> c1bk; c1bk.clear();
    for(int i=0; i<c0.size(); i++){
        c1bk.append(XYZ(c1[i]));
    }
    //step 1
    double mdis=distance_XYZList(c0,c1);
    double mang=0;
    for(double ang=10; ang<360; ang+=10){
        rotate_XYZList(c1,c1bk,ang,dir);
        double dis = distance_XYZList(c0, c1bk);
        if(dis<mdis){
            mdis=dis;
            mang=ang;
        }
    }
    qDebug()<<"rotation 0: "<<mang<<":"<<mdis;
    //step 2
    for(double ang=mang-10; ang<mang+10; ang++){
        rotate_XYZList(c1,c1bk,ang,dir);
        double dis = distance_XYZList(c0, c1bk);
        if(dis<mdis){
            mdis=dis;
            mang=ang;
        }
    }
    qDebug()<<"rotation 1: "<<mang<<":"<<mdis;
    //step 3
    for(double ang=mang-1; ang<mang+1; ang+=0.1){
        rotate_XYZList(c1,c1bk,ang,dir);
        double dis = distance_XYZList(c0, c1bk);
        if(dis<mdis){
            mdis=dis;
            mang=ang;
        }
    }
    qDebug()<<"rotation 2: "<<mang<<":"<<mdis;
    angle_r=mang;
    return true;
}

void update_marker_info(const LocationSimple& mk, int* info) //info[0]=neuron id, info[1]=point id, info[2]=matching marker, info[3]=marker name/id
{
    LocationSimple *p;
    p = (LocationSimple *)&mk;
    QString tmp;
    tmp=QString::number(info[0]) + " " + QString::number(info[1]) + " " + QString::number(info[2]);
    p->comments=tmp.toStdString();
    p->name=QString::number(info[3]).toStdString();
}

void update_marker_info(const LocationSimple& mk, int* info, int* color) //info[0]=neuron id, info[1]=point id, info[2]=matching marker, info[3]=marker name/id
{
    LocationSimple *p;
    p = (LocationSimple *)&mk;
    QString tmp;
    tmp=QString::number(info[0]) + " " + QString::number(info[1]) + " " + QString::number(info[2]);
    p->comments=tmp.toStdString();
    p->name=QString::number(info[3]).toStdString();
    p->color.r = color[0];
    p->color.g = color[1];
    p->color.b = color[2];
}

bool get_marker_info(const LocationSimple& mk, int* info) //info[0]=neuron id, info[1]=point id, info[2]=matching marker, info[3]=marker name
{
    info[0]=info[1]=info[2]=info[3]=-1;
    QString tmp;
    tmp=QString::fromStdString(mk.comments);
    QStringList items = tmp.split(" ", QString::SkipEmptyParts);
    int val;
    bool check;
    if(items.size()<3)
        return false;
    for(int i=0; i<3; i++){
        check=false;
        val=items[i].toInt(&check, 10);
        if(!check)
            return false;
        else
            info[i]=val;
    }
    tmp=QString::fromStdString(mk.name);
    val=tmp.toInt(&check, 10);
    if(!check)
        return false;
    else
        info[3]=val;
    return true;
}

bool export_list2file(QList<NeuronSWC> & lN, QString fileSaveName, QString fileOpenName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);
    myfile<<"# generated by Vaa3D Plugin resample_swc"<<endl;
    myfile<<"# source file(s): "<<fileOpenName<<endl;
    myfile<<"# id,type,x,y,z,r,pid"<<endl;
    for (V3DLONG i=0;i<lN.size();i++)
        myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";

    file.close();
    cout<<"swc file "<<fileSaveName.toStdString()<<" has been generated, size: "<<lN.size()<<endl;
    return true;
}

bool writeAmat(const char* fname, double* amat)
{
    ofstream file(fname);
    if(!file.is_open()){
        return false;
    }

    for(int i=0; i<4; i++){
        for(int j=0; j<4; j++){
            file<<amat[i*4+j]<<"\t";
        }
        file<<endl;
    }
    file.close();

    return true;
}

bool readAmat(const char* fname, double* amat)
{
    ifstream file(fname);
    if(!file.is_open()){
        return false;
    }

    for(int i=0; i<16; i++){
        file>>amat[i];
    }
    file.close();

    return true;
}

void getAffineAmat(double amat[16], double& shift_x, double& shift_y, double & shift_z, double & angle_r, double & cent_x,double & cent_y,double & cent_z,int dir)
{
    for(int i=0; i<16; i++){
        amat[i]=0;
    }
    amat[0]=amat[5]=amat[10]=amat[15]=1;
    amat[3]=shift_x;
    amat[7]=shift_y;
    amat[11]=shift_z;

    double afmatrix[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    double a = angle_r / 180.0 * M_PI;
    if(dir==0){
        afmatrix[5] = cos(a); afmatrix[6] = -sin(a);
        afmatrix[9] = -afmatrix[6]; afmatrix[10] = afmatrix[5];
    }else if(dir==1){
        afmatrix[0] = cos(a); afmatrix[2] = sin(a);
        afmatrix[8] = -afmatrix[2]; afmatrix[10] = afmatrix[0];
    }else if(dir==2){
        afmatrix[0] = cos(a); afmatrix[1] = -sin(a);
        afmatrix[4] = -afmatrix[1]; afmatrix[5] = afmatrix[0];
    }

    multiplyAmat_centerRotate(afmatrix,amat, cent_x, cent_y, cent_z);

}

void multiplyAmat_centerRotate(double* rotate, double* tmat, double cx, double cy, double cz)
{
    double tmp[16]={1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};

    tmp[3]=-cx;    tmp[7]=-cy;    tmp[11]=-cz;
    multiplyAmat(tmp, tmat);

    multiplyAmat(rotate, tmat);

    tmp[3]=cx;    tmp[7]=cy;    tmp[11]=cz;
    multiplyAmat(tmp, tmat);
}

void multiplyAmat(double* front, double* back)
{
    float tmp[16]={0};
    for(int i=0; i<3; i++){
        for(int j=0; j<4; j++){
            for(int k=0; k<4; k++){
                tmp[i*4+j]+=front[i*4+k]*back[j+k*4];
            }
        }
    }
    for(int i=0; i<12; i++){
        back[i]=tmp[i];
    }
}

double getNeuronTreeMidplane(const NeuronTree& nt0, const NeuronTree& nt1, int direction)
{
    float bound0[2], bound1[2];
    getNeuronTreeBound(nt0, bound0, direction);
    getNeuronTreeBound(nt1, bound1, direction);

    if(bound0[0]+bound0[1]>bound1[0]+bound1[1]){
        return (bound0[0]+bound1[1])/2;
    }else{
        return (bound0[1]+bound1[0])/2;
    }
}

void getNeuronTreeBound(const NeuronTree& nt, float * bound, int direction)
{
    bound[0]=1e10;
    bound[1]=-1e10;
    for(int i=0; i < nt.listNeuron.size(); i++){
        switch(direction)
        {
        case 0:
            bound[0]=nt.listNeuron[i].x < bound[0]?nt.listNeuron[i].x:bound[0];
            bound[1]=nt.listNeuron[i].x > bound[1]?nt.listNeuron[i].x:bound[1];
            break;
        case 1:
            bound[0]=nt.listNeuron[i].y < bound[0]?nt.listNeuron[i].y:bound[0];
            bound[1]=nt.listNeuron[i].y > bound[1]?nt.listNeuron[i].y:bound[1];
            break;
        case 2:
            bound[0]=nt.listNeuron[i].z < bound[0]?nt.listNeuron[i].z:bound[0];
            bound[1]=nt.listNeuron[i].z > bound[1]?nt.listNeuron[i].z:bound[1];
            break;
        default:
            bound[0]=0; bound[1]=0;
        }
    }
}

void getNeuronTreeBound(const NeuronTree& nt, double &minx, double &miny, double &minz,
                        double &maxx, double &maxy, double &maxz,
                        double &mmx, double &mmy, double &mmz)
{
    minx=maxx=nt.listNeuron[0].x;
    miny=maxy=nt.listNeuron[0].y;
    minz=maxz=nt.listNeuron[0].z;
    for(int i=0; i < nt.listNeuron.size(); i++){
        minx=nt.listNeuron[i].x < minx?nt.listNeuron[i].x:minx;
        maxx=nt.listNeuron[i].x > maxx?nt.listNeuron[i].x:maxx;
        miny=nt.listNeuron[i].y < miny?nt.listNeuron[i].y:miny;
        maxy=nt.listNeuron[i].y > maxy?nt.listNeuron[i].y:maxy;
        minz=nt.listNeuron[i].z < minz?nt.listNeuron[i].z:minz;
        maxz=nt.listNeuron[i].z > maxz?nt.listNeuron[i].z:maxz;
    }
    mmx=(minx+maxx)/2.0;
    mmy=(miny+maxy)/2.0;
    mmz=(minz+maxz)/2.0;
}


void highlight_planepoint(const NeuronTree& nt0, const NeuronTree& nt1, float dis, int direction)
{
    if(direction < 0 || direction > 2)
        return;

    int count=0, type1=8, type2=9;
    float val;
    float bound0[2], bound1[2];
    NeuronSWC * tp;
    getNeuronTreeBound(nt0,bound0,direction);
    getNeuronTreeBound(nt1,bound1,direction);
    if(bound0[0]+bound0[1]>bound1[0]+bound1[1]){
        for(int j=0; j<nt0.listNeuron.size(); j++){
            switch(direction)
            {
            case 0:
                val=nt0.listNeuron[j].x;
                break;
            case 1:
                val=nt0.listNeuron[j].y;
                break;
            case 2:
                val=nt0.listNeuron[j].z;
            }
            tp = (NeuronSWC *)(& nt0.listNeuron[j]);
            if(val>bound0[0]-dis && val<bound0[0]+dis)
                tp->type=4;
        }
        for(int j=0; j<nt1.listNeuron.size(); j++){
            switch(direction)
            {
            case 0:
                val=nt1.listNeuron[j].x;
                break;
            case 1:
                val=nt1.listNeuron[j].y;
                break;
            case 2:
                val=nt1.listNeuron[j].z;
            }
            tp = (NeuronSWC *)(& nt1.listNeuron[j]);
            if(val>bound1[1]-dis && val<bound1[1]+dis)
                tp->type=5;
        }
    }else{
        for(int j=0; j<nt0.listNeuron.size(); j++){
            switch(direction)
            {
            case 0:
                val=nt0.listNeuron[j].x;
                break;
            case 1:
                val=nt0.listNeuron[j].y;
                break;
            case 2:
                val=nt0.listNeuron[j].z;
            }
            tp = (NeuronSWC *)(& nt0.listNeuron[j]);
            if(val>bound0[1]-dis && val<bound0[1]+dis)
                tp->type=4;
        }
        for(int j=0; j<nt1.listNeuron.size(); j++){
            switch(direction)
            {
            case 0:
                val=nt1.listNeuron[j].x;
                break;
            case 1:
                val=nt1.listNeuron[j].y;
                break;
            case 2:
                val=nt1.listNeuron[j].z;
            }
            tp = (NeuronSWC *)(& nt1.listNeuron[j]);
            if(val>bound1[0]-dis && val<bound1[0]+dis)
                tp->type=5;
        }
    }
}

int highlight_edgepoint(const QList<NeuronTree> *ntList, float dis, int direction)
{
    if(direction < 0 || direction > 2)
        return 0;

    int count=0, type1=8, type2=9;
    float val;
    float bound[2];
    NeuronSWC * tp;
    for(int i=0; i<ntList->size(); i++){
        getNeuronTreeBound(ntList->at(i),bound,direction);
        for(int j=0; j<ntList->at(i).listNeuron.size(); j++){
            switch(direction)
            {
            case 0:
                val=ntList->at(i).listNeuron[j].x;
                break;
            case 1:
                val=ntList->at(i).listNeuron[j].y;
                break;
            case 2:
                val=ntList->at(i).listNeuron[j].z;
            }
            tp = (NeuronSWC *)(& ntList->at(i).listNeuron[j]);
            if(val>bound[0]-dis && val<bound[0]+dis)
                tp->type=type1;
            else if(val>bound[1]-dis && val<bound[1]+dis)
                tp->type=type2;
        }
        type1+=2; type2+=2;
    }

    return count;
}

int highlight_adjpoint(const NeuronTree& nt1, const NeuronTree& nt2, float dis)
{
    int count=0;
    NeuronSWC * tp;
    dis*=dis;
    for(int i=0; i < nt1.listNeuron.size(); i++){
        for(int j=0; j<nt2.listNeuron.size(); j++){
            if(NTDIS(nt1.listNeuron[i],nt2.listNeuron[j])<dis){
                count++;
                tp = (NeuronSWC *)(&(nt1.listNeuron[i]));
                tp->type=12;
                tp = (NeuronSWC *)(&(nt2.listNeuron[j]));
                tp->type=12;
            }
        }
    }

    return count;
}

void change_neuron_type(const NeuronTree& nt, int type)
{
    NeuronSWC * tp;
    for(int j=0; j<nt.listNeuron.size(); j++){
        tp=(NeuronSWC *)(&nt.listNeuron[j]);
        tp->type = type;
    }
}

void backupNeuron(const NeuronTree & source, const NeuronTree & backup)
{
    NeuronTree *np = (NeuronTree *)(&backup);
    np->n=source.n; np->on=source.on; np->selected=source.selected; np->name=source.name; np->comment=source.comment;
    np->color.r=source.color.r; np->color.g=source.color.g; np->color.b=source.color.b; np->color.a=source.color.a;
    np->listNeuron.clear();
    for(V3DLONG i=0; i<source.listNeuron.size(); i++)
    {
        NeuronSWC S;
        S.n = source.listNeuron[i].n;
        S.type = source.listNeuron[i].type;
        S.x = source.listNeuron[i].x;
        S.y = source.listNeuron[i].y;
        S.z = source.listNeuron[i].z;
        S.r = source.listNeuron[i].r;
        S.pn = source.listNeuron[i].pn;
        S.seg_id = source.listNeuron[i].seg_id;
        S.level = source.listNeuron[i].level;
        S.fea_val = source.listNeuron[i].fea_val;
        np->listNeuron.append(S);
    }
    np->hashNeuron = source.hashNeuron;
    np->file     = source.file;
    np->editable = source.editable;
    np->linemode = source.linemode;
}

void copyCoordinate(const NeuronTree & source, const NeuronTree & target)
{
    if (source.listNeuron.size()!=target.listNeuron.size()) return;

    NeuronSWC *ps_tmp;
    NeuronSWC *pt_tmp;
    for (V3DLONG i=0;i<source.listNeuron.size();i++)
    {
        ps_tmp = (NeuronSWC *)(&(source.listNeuron.at(i)));
        pt_tmp = (NeuronSWC *)(&(target.listNeuron.at(i)));
        pt_tmp->x = ps_tmp->x;
        pt_tmp->y = ps_tmp->y;
        pt_tmp->z = ps_tmp->z;
    }
    NeuronTree *np = (NeuronTree *)(&target);
    np->color.r = source.color.r;
    np->color.g = source.color.g;
    np->color.b = source.color.b;
    np->color.a = source.color.a;
}

void copyProperty(const NeuronTree & source, const NeuronTree & target)
{
    if (source.listNeuron.size()!=target.listNeuron.size()) return;

    NeuronSWC *ps_tmp;
    NeuronSWC *pt_tmp;
    for (V3DLONG i=0;i<source.listNeuron.size();i++)
    {
        ps_tmp = (NeuronSWC *)(&(source.listNeuron.at(i)));
        pt_tmp = (NeuronSWC *)(&(target.listNeuron.at(i)));
        pt_tmp->type = ps_tmp->type;
    }
    NeuronTree *np = (NeuronTree *)(&target);
    np->color.r = source.color.r;
    np->color.g = source.color.g;
    np->color.b = source.color.b;
    np->color.a = source.color.a;
}

void copyType(QList<int> source, const NeuronTree & target)
{
    if (source.size()!=target.listNeuron.size()) return;

    NeuronSWC *pt_tmp;
    for (V3DLONG i=0;i<source.size();i++)
    {
        pt_tmp = (NeuronSWC *)(&(target.listNeuron.at(i)));
        pt_tmp->type = source[i];
    }
}

void copyType(const NeuronTree & source, QList<int> & target)
{
    if (target.size()!=source.listNeuron.size()) return;

    for (V3DLONG i=0;i<target.size();i++)
    {
        target[i]=source.listNeuron[i].type;
    }
}

void affineNeuron(NeuronTree &nt_in, NeuronTree &nt_out, double shift_x, double shift_y, double shift_z, double angle, double cent_x, double cent_y, double cent_z, int axis) //angle is 0-360  based
{
    QList<XYZ> tmpcoord;
    for(int i=0; i<nt_in.listNeuron.size(); i++){
        XYZ tmp;
        tmp.x = nt_in.listNeuron.at(i).x +shift_x-cent_x;
        tmp.y = nt_in.listNeuron.at(i).y +shift_y-cent_y;
        tmp.z = nt_in.listNeuron.at(i).z +shift_z-cent_z;
        tmpcoord.append(tmp);
    }
    rotate_XYZList(tmpcoord, tmpcoord, angle, axis);
    for(int i=0; i<nt_in.listNeuron.size(); i++){
        tmpcoord[i].x += cent_x;
        tmpcoord[i].y += cent_y;
        tmpcoord[i].z += cent_z;
    }


    if(nt_out.listNeuron.size() != nt_in.listNeuron.size()){
        backupNeuron(nt_in, nt_out);
    }
    for(int i=0; i<nt_out.listNeuron.size(); i++){
        NeuronSWC * S = (NeuronSWC *)(&(nt_out.listNeuron.at(i)));
        S->x = tmpcoord[i].x;
        S->y = tmpcoord[i].y;
        S->z = tmpcoord[i].z;
    }
}

float quickMoveNeuron(QList<NeuronTree> * ntTreeList, int ant, int stackdir, int idx_firstnt)
{
    if(stackdir<0 || stackdir>2)
        return 0;
    int idx_secondnt=0; // the index of neuron on bottom/left/back (smaller z/x/y)
    if(idx_firstnt==0){
        idx_secondnt=1;
    }else if(idx_firstnt==1){
        idx_secondnt=0;
    }else{
        return 0;
    }
    float gap=0; // the gap between two stacks
    float delta=0;

    //adjust the neuron tree
    if(stackdir==0) //x direction move
    {
        NeuronSWC * tp;
        float first_min=1e10, second_max=-1e10;
        for(V3DLONG nid = 0; nid < ntTreeList->at(idx_firstnt).listNeuron.size(); nid++){
            first_min=first_min>ntTreeList->at(idx_firstnt).listNeuron.at(nid).x?ntTreeList->at(idx_firstnt).listNeuron.at(nid).x:first_min;
        }
        for(V3DLONG nid = 0; nid < ntTreeList->at(idx_secondnt).listNeuron.size(); nid++){
            second_max=second_max<ntTreeList->at(idx_secondnt).listNeuron.at(nid).x?ntTreeList->at(idx_secondnt).listNeuron.at(nid).x:second_max;
        }
        if(idx_firstnt==ant){
            delta=second_max-first_min+gap;
        }else{
            delta=first_min-second_max-gap;
        }
        for(V3DLONG nid = 0; nid < ntTreeList->at(ant).listNeuron.size(); nid++){
            tp = (NeuronSWC *)(&(ntTreeList->at(ant).listNeuron.at(nid)));
            tp->x+=delta;
        }
    }
    else if(stackdir==1) //y direction move
    {
        NeuronSWC * tp;
        float first_min=1e10, second_max=-1e10;
        for(V3DLONG nid = 0; nid < ntTreeList->at(idx_firstnt).listNeuron.size(); nid++){
            first_min=first_min>ntTreeList->at(idx_firstnt).listNeuron.at(nid).y?ntTreeList->at(idx_firstnt).listNeuron.at(nid).y:first_min;
        }
        for(V3DLONG nid = 0; nid < ntTreeList->at(idx_secondnt).listNeuron.size(); nid++){
            second_max=second_max<ntTreeList->at(idx_secondnt).listNeuron.at(nid).y?ntTreeList->at(idx_secondnt).listNeuron.at(nid).y:second_max;
        }
        delta=second_max-first_min+gap;
        if(idx_firstnt==ant){
            delta=second_max-first_min+gap;
        }else{
            delta=first_min-second_max-gap;
        }
        for(V3DLONG nid = 0; nid < ntTreeList->at(ant).listNeuron.size(); nid++){
            tp = (NeuronSWC *)(&(ntTreeList->at(ant).listNeuron.at(nid)));
            tp->y+=delta;
        }
    }
    else if(stackdir==2) //z direction move
    {
        NeuronSWC * tp;
        float first_min=1e10, second_max=-1e10;
        for(V3DLONG nid = 0; nid < ntTreeList->at(idx_firstnt).listNeuron.size(); nid++){
            first_min=first_min>ntTreeList->at(idx_firstnt).listNeuron.at(nid).z?ntTreeList->at(idx_firstnt).listNeuron.at(nid).z:first_min;
        }
        for(V3DLONG nid = 0; nid < ntTreeList->at(idx_secondnt).listNeuron.size(); nid++){
            second_max=second_max<ntTreeList->at(idx_secondnt).listNeuron.at(nid).z?ntTreeList->at(idx_secondnt).listNeuron.at(nid).z:second_max;
        }
        delta=second_max-first_min+gap;
        if(idx_firstnt==ant){
            delta=second_max-first_min+gap;
        }else{
            delta=first_min-second_max-gap;
        }
        for(V3DLONG nid = 0; nid < ntTreeList->at(ant).listNeuron.size(); nid++){
            tp = (NeuronSWC *)(&(ntTreeList->at(ant).listNeuron.at(nid)));
            tp->z+=delta;
        }
    }

    printf("%d %d %d %f\n",ant,idx_firstnt,idx_secondnt,delta);

    return delta;
}

float quickMoveNeuron(NeuronTree * nt0, const QList<int>& cand0, NeuronTree * nt1, const QList<int>& cand1, int stackdir)
{
    if(stackdir<0 || stackdir>2)
        return 0;
    float gap=0; // the gap between two stacks
    float delta=0;

    //adjust the neuron tree
    if(stackdir==0) //x direction move
    {
        NeuronSWC * tp;
        float first_min=1e10, second_max=-1e10;
        for(V3DLONG cid = 0; cid < cand1.size(); cid++){
            V3DLONG nid=cand1.at(cid);
            first_min=first_min>nt1->listNeuron.at(nid).x?nt1->listNeuron.at(nid).x:first_min;
        }
        for(V3DLONG cid = 0; cid < cand0.size(); cid++){
            V3DLONG nid=cand0.at(cid);
            second_max=second_max<nt0->listNeuron.at(nid).x?nt0->listNeuron.at(nid).x:second_max;
        }
        delta=second_max-first_min+gap;
        for(V3DLONG nid = 0; nid < nt1->listNeuron.size(); nid++){
            tp = (NeuronSWC *)(&(nt1->listNeuron.at(nid)));
            tp->x+=delta;
        }
    }
    else if(stackdir==1) //y direction move
    {
        NeuronSWC * tp;
        float first_min=1e10, second_max=-1e10;
        for(V3DLONG cid = 0; cid < cand1.size(); cid++){
            V3DLONG nid=cand1.at(cid);
            first_min=first_min>nt1->listNeuron.at(nid).y?nt1->listNeuron.at(nid).y:first_min;
        }
        for(V3DLONG cid = 0; cid < cand0.size(); cid++){
            V3DLONG nid=cand0.at(cid);
            second_max=second_max<nt0->listNeuron.at(nid).y?nt0->listNeuron.at(nid).y:second_max;
        }
        delta=second_max-first_min+gap;
        for(V3DLONG nid = 0; nid < nt1->listNeuron.size(); nid++){
            tp = (NeuronSWC *)(&(nt1->listNeuron.at(nid)));
            tp->y+=delta;
        }
    }
    else if(stackdir==2) //z direction move
    {
        NeuronSWC * tp;
        float first_min=1e10, second_max=-1e10;
        for(V3DLONG cid = 0; cid < cand1.size(); cid++){
            V3DLONG nid=cand1.at(cid);
            first_min=first_min>nt1->listNeuron.at(nid).z?nt1->listNeuron.at(nid).z:first_min;
        }
        for(V3DLONG cid = 0; cid < cand0.size(); cid++){
            V3DLONG nid=cand0.at(cid);
            second_max=second_max<nt0->listNeuron.at(nid).z?nt0->listNeuron.at(nid).z:second_max;
        }
        delta=second_max-first_min+gap;
        for(V3DLONG nid = 0; nid < nt1->listNeuron.size(); nid++){
            tp = (NeuronSWC *)(&(nt1->listNeuron.at(nid)));
            tp->z+=delta;
        }
    }

    return delta;
}

float quickMoveNeuron(NeuronTree * nt0, NeuronTree * nt1, int stackdir)
{
    if(stackdir<0 || stackdir>2)
        return 0;
    float gap=0; // the gap between two stacks
    float delta=0;

    //adjust the neuron tree
    if(stackdir==0) //x direction move
    {
        NeuronSWC * tp;
        float first_min=1e10, second_max=-1e10;
        for(V3DLONG nid = 0; nid < nt1->listNeuron.size(); nid++){
            first_min=first_min>nt1->listNeuron.at(nid).x?nt1->listNeuron.at(nid).x:first_min;
        }
        for(V3DLONG nid = 0; nid < nt0->listNeuron.size(); nid++){
            second_max=second_max<nt0->listNeuron.at(nid).x?nt0->listNeuron.at(nid).x:second_max;
        }
        delta=second_max-first_min+gap;
        for(V3DLONG nid = 0; nid < nt1->listNeuron.size(); nid++){
            tp = (NeuronSWC *)(&(nt1->listNeuron.at(nid)));
            tp->x+=delta;
        }
    }
    else if(stackdir==1) //y direction move
    {
        NeuronSWC * tp;
        float first_min=1e10, second_max=-1e10;
        for(V3DLONG nid = 0; nid < nt1->listNeuron.size(); nid++){
            first_min=first_min>nt1->listNeuron.at(nid).y?nt1->listNeuron.at(nid).y:first_min;
        }
        for(V3DLONG nid = 0; nid < nt0->listNeuron.size(); nid++){
            second_max=second_max<nt0->listNeuron.at(nid).y?nt0->listNeuron.at(nid).y:second_max;
        }
        delta=second_max-first_min+gap;
        for(V3DLONG nid = 0; nid < nt1->listNeuron.size(); nid++){
            tp = (NeuronSWC *)(&(nt1->listNeuron.at(nid)));
            tp->y+=delta;
        }
    }
    else if(stackdir==2) //z direction move
    {
        NeuronSWC * tp;
        float first_min=1e10, second_max=-1e10;
        for(V3DLONG nid = 0; nid < nt1->listNeuron.size(); nid++){
            first_min=first_min>nt1->listNeuron.at(nid).z?nt1->listNeuron.at(nid).z:first_min;
        }
        for(V3DLONG nid = 0; nid < nt0->listNeuron.size(); nid++){
            second_max=second_max<nt0->listNeuron.at(nid).z?nt0->listNeuron.at(nid).z:second_max;
        }
        delta=second_max-first_min+gap;
        for(V3DLONG nid = 0; nid < nt1->listNeuron.size(); nid++){
            tp = (NeuronSWC *)(&(nt1->listNeuron.at(nid)));
            tp->z+=delta;
        }
    }

    return delta;
}
