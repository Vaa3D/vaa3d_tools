#include "neuron_match_gmhash.h"

neuron_match_gmhash::neuron_match_gmhash(NeuronTree* botNeuron, NeuronTree* topNeuron)
{
    nt0=botNeuron;
    nt1=topNeuron;

    constructNeuronGraph(*nt0, ng0);
    constructNeuronGraph(*nt1, ng1);

    spanCand = 20;
    zscale = 2;
    midplane = 0;
    pmatchThr = 100;
    hashbin = 1;
    direction = 2;
    minmatchrate = 0.5;
    trialNum = 10;
}

void getMatchPairs_XYZList(const QList<XYZ>& c0, const QList<XYZ>& c1, QList<int> * MatchMarkers, double span, const QHash<int, QList<int> >& match1to0)
{
    double thr = span*span;
    QMap<double, QPair<int,int> > MatchPoints;
    MatchMarkers[0].clear();
    MatchMarkers[1].clear();

    for(int i=0; i<c0.size(); i++){
        for(int j=0; j<c1.size(); j++){
            //match by preknowledge
            if(!match1to0.contains(j) || !match1to0[j].contains(i))
                continue;
            //match by distance
            double dis = NTDIS(c0.at(i),c1.at(j));
            if(dis>thr) continue;
            //record and sort
            MatchPoints.insertMulti(dis, QPair<int,int>(i,j));
        }
    }

    QVector<int> mask0(c0.size(),0);
    QVector<int> mask1(c1.size(),0);
    for(QMap<double, QPair<int,int> >::Iterator iter = MatchPoints.begin(); iter!=MatchPoints.end(); iter++){
        int a = iter.value().first;
        int b = iter.value().second;
        if(mask0[a]+mask1[b]>0)
            continue;
        mask0[a]++;
        mask1[b]++;

        MatchMarkers[0].append(a);
        MatchMarkers[1].append(b);
    }
}

void neuron_match_gmhash::init()
{
    qDebug()<<"align in z direction";
    shift_z=quickMoveNeuron(nt0, nt1, direction);
    midplane=getNeuronTreeMidplane((*nt0), (*nt1), direction);

    //find candidates
    qDebug()<<"search candidates";
    initNeuronAndCandidate(*nt0,ng0,candID0,candcoord0);
    initNeuronAndCandidate(*nt1,ng1,candID1,candcoord1);
    qDebug()<<"nt0:"<<candID0.size()<<"; nt1:"<<candID1.size();

    //init hash
    qDebug()<<"init hash";
    initHash0();
    qDebug()<<"hash size:"<<gmhash0.size();
}

void neuron_match_gmhash::globalmatch()
{
    qDebug()<<"start global match";
    if(candcoord1.size()<3){
        qDebug()<<"Error: quit matching, number of candidate on upper stack is less than 3.";
        return;
    }
    int minmatch=candcoord1.size()*minmatchrate;
    double bestEnergy = 0;
    float norm;
    int x,y;
    float vx,vy,px,py;
    float rxx,rxy,ryx,ryy,sx,sy;
    QPair<int, int> gmval,gmkey;
    rankedPairMatch.clear();
    for(int i = 0; i<candcoord1.size(); i++){
        sx=-candcoord1.at(i).x;
        sy=-candcoord1.at(i).y;
        for(int j=i+1; j<candcoord1.size(); j++){
            //direction of transformed x
            vx = candcoord1.at(j).x-candcoord1.at(i).x;
            vy = candcoord1.at(j).y-candcoord1.at(i).y;
            norm=sqrt(vx*vx+vy*vy);
            rxx=vx/norm;
            rxy=vy/norm;
            ryx=-vy/norm;
            ryy=vx/norm;
            gmval.first=i;
            gmval.second=j;

            QHash<QPair<int, int>, int > candcount;

            //transform the rest point and screen hash
            for(int k=0; k<candcoord1.size(); k++){
                if(k==i || k==j) continue;
                px=candcoord1.at(k).x+sx;
                py=candcoord1.at(k).y+sy;
                x=(px*rxx+py*rxy)/hashbin;
                y=(px*ryx+py*ryy)/hashbin;

                gmkey.first=x;
                gmkey.second=y;
                if(gmhash0.contains(gmkey)){
                    for(QHash<QPair<int,int>, QList<int> >::const_iterator pairiter = gmhash0[gmkey].constBegin();
                        pairiter!=gmhash0[gmkey].constEnd(); pairiter++){
                        if(candcount.contains(pairiter.key())){
                            candcount[pairiter.key()]+=1;
                        }else{
                            candcount.insert(pairiter.key(), 1);
                        }
                    }
                }
            }
            for(QHash<QPair<int, int>, int >::const_iterator countiter = candcount.constBegin();
                countiter != candcount.constEnd(); countiter++){
                if(countiter.value()>minmatch){
                    QPair<QPair<int,int>, QPair<int,int> > pairtmp;
                    pairtmp.first=countiter.key();
                    pairtmp.second=gmval;
                    rankedPairMatch.insert(pair<int, QPair<QPair<int,int>, QPair<int,int> > >(countiter.value(),pairtmp));
                }
            }
        }
    }
    qDebug()<<"Found "<<rankedPairMatch.size()<<" pairs of matching edges";
    if(rankedPairMatch.size()<=0){
        qDebug()<<"Not matched pair identified. Reduce min match rate and try again.";
        return;
    }
    multimap<int, QPair<QPair<int,int>, QPair<int,int> > , std::greater<int> >::iterator matchiter=rankedPairMatch.begin();
    for(int ti=0; ti<trialNum && matchiter!=rankedPairMatch.end(); ti++, matchiter++){
//        qDebug()<<"Trial "<<ti<<", "<<matchiter->first<<" matched candidates";
        QPair<int,int> pairnt0, pairnt1;
        QHash<int, QList<int> > match1to0;
        pairnt0=matchiter->second.first;
        pairnt1=matchiter->second.second;

        //first match the base pairs
        match1to0.insert(pairnt1.first,QList<int>()<<pairnt0.first);
        match1to0.insert(pairnt1.second,QList<int>()<<pairnt0.second);
        //for each combination, match from cand1 to cand0 based on hash
        int i=pairnt1.first, j=pairnt1.second;
        sx=-candcoord1.at(i).x;
        sy=-candcoord1.at(i).y;
        vx = candcoord1.at(j).x-candcoord1.at(i).x;
        vy = candcoord1.at(j).y-candcoord1.at(i).y;
        norm=sqrt(vx*vx+vy*vy);
        rxx=vx/norm;
        rxy=vy/norm;
        ryx=-vy/norm;
        ryy=vx/norm;
        //transform the rest point and screen hash for matches
        for(int k=0; k<candcoord1.size(); k++){
            if(k==i || k==j) continue;
            px=candcoord1.at(k).x+sx;
            py=candcoord1.at(k).y+sy;
            x=(px*rxx+py*rxy)/hashbin;
            y=(px*ryx+py*ryy)/hashbin;

            gmkey.first=x;
            gmkey.second=y;
            if(gmhash0.contains(gmkey) and gmhash0[gmkey].contains(pairnt0)){
                match1to0.insert(k,gmhash0[gmkey][pairnt0]);
            }
        }

        //iteratively optimize result
        double better_shift_x=0,better_shift_y=0,better_shift_z=0,better_angle=0,better_cent_x=0,better_cent_y=0,better_cent_z=0;
        QList<int> tmpmatch0, tmpmatch1;
        for(QHash<int, QList<int> >::iterator matchiter=match1to0.begin(); matchiter!=match1to0.end(); matchiter++){
            if(matchiter.value().size()==1){
                tmpmatch1.append(matchiter.key());
                tmpmatch0.append(matchiter.value().at(0));
            }
        }
        //perform affine transform
        //double tmpEnergy=-1;
        while(1){
//            qDebug()<<":"<<tmpmatch0.size();

            double tmp_shift_x=0,tmp_shift_y=0,tmp_shift_z=0,tmp_angle=0,tmp_cent_x=0,tmp_cent_y=0,tmp_cent_z=0;
            if(direction==0) tmp_shift_x=-1;
            if(direction==1) tmp_shift_y=-1;
            if(direction==2) tmp_shift_z=-1;
            QList<XYZ> c0,c1,tmpcoord;
            for(int i=0; i<tmpmatch0.size(); i++){
                c0.append(XYZ(candcoord0.at(tmpmatch0.at(i))));
                c1.append(XYZ(candcoord1.at(tmpmatch1.at(i))));
            }
            if(compute_affine_4dof(c0,c1,tmp_shift_x,tmp_shift_y,tmp_shift_z,tmp_angle,tmp_cent_x,tmp_cent_y,tmp_cent_z,direction)<0){
                break;
            }
            affine_XYZList(candcoord1, tmpcoord, tmp_shift_x, tmp_shift_y, tmp_shift_z, tmp_angle, tmp_cent_x, tmp_cent_y, tmp_cent_z, direction);
            QList<int> tmpMatchMarkers[2];
            tmpMatchMarkers[0]=QList<int>();
            tmpMatchMarkers[1]=QList<int>();
            getMatchPairs_XYZList(candcoord0, tmpcoord, tmpMatchMarkers, pmatchThr, match1to0);
            if(tmpMatchMarkers[0].size()>tmpmatch0.size()){
                tmpmatch0=tmpMatchMarkers[0];
                tmpmatch1=tmpMatchMarkers[1];
                better_shift_x=tmp_shift_x;
                better_shift_y=tmp_shift_y;
                better_shift_z=tmp_shift_z;
                better_angle=tmp_angle;
                better_cent_x=tmp_cent_x;
                better_cent_y=tmp_cent_y;
                better_cent_z=tmp_cent_z;
            }else{
                break;
            }
        }

        if(tmpmatch0.size()>bestEnergy){
            bestEnergy = tmpmatch0.size();
            candmatch0 = tmpmatch0;
            candmatch1 = tmpmatch1;
            ti=0;
        }
    }
    for(int i=0; i<candmatch0.size(); i++){
        pmatch0.append(candID0.at(candmatch0.at(i)));
        pmatch1.append(candID1.at(candmatch1.at(i)));
    }
    qDebug()<<"global match: "<<pmatch0.size()<<" matched points found; energy: "<<bestEnergy;
    if(pmatch0.size()>0){
        //affine based on matched points
        affine_nt1();
    }
}

void neuron_match_gmhash::initNeuronAndCandidate(NeuronTree& nt, const HBNeuronGraph& ng, QList<int>& cand, QList<XYZ>& candcoord)
{
    float min=midplane-spanCand;
    float max=midplane+spanCand;

    cand.clear();
    candcoord.clear();

    //find candidate
    cand.clear();
    candcoord.clear();
    for(int i=0; i<ng.size(); i++){
        if(ng[i].size()==1 && nt.listNeuron.at(i).z>min && nt.listNeuron.at(i).z<max){
            cand.append(i);
            candcoord.append(XYZ(nt.listNeuron.at(i)));
        }
    }
}

void neuron_match_gmhash::initHash0()
{
    gmhash0.clear();
    if(candcoord0.size()<3){
        qDebug()<<"Error: quit gmhash, number of candidate on lower stack is less than 3.";
        return;
    }
    float norm;
    int x,y;
    float vx,vy,px,py;
    float rxx,rxy,ryx,ryy,sx,sy;
    QPair<int, int> gmval,gmkey;
    for(int i = 0; i<candcoord0.size(); i++){
        sx=-candcoord0.at(i).x;
        sy=-candcoord0.at(i).y;
        for(int j=i+1; j<candcoord0.size(); j++){
            //direction of transformed x
            vx = candcoord0.at(j).x-candcoord0.at(i).x;
            vy = candcoord0.at(j).y-candcoord0.at(i).y;
            norm=sqrt(vx*vx+vy*vy);
            rxx=vx/norm;
            rxy=vy/norm;
            ryx=-vy/norm;
            ryy=vx/norm;
            gmval.first=i;
            gmval.second=j;

            //transform the rest point and hash
            for(int k=0; k<candcoord0.size(); k++){
                if(k==i || k==j) continue;
                px=candcoord0.at(k).x+sx;
                py=candcoord0.at(k).y+sy;
                x=(px*rxx+py*rxy)/hashbin;
                y=(px*ryx+py*ryy)/hashbin;

                gmkey.first=x;
                gmkey.second=y;
                if(gmhash0.contains(gmkey)){
                    if(gmhash0[gmkey].contains(gmval))
                        gmhash0[gmkey][(gmval)].append(k);
                    else
                        gmhash0[gmkey].insert(gmval,QList<int>()<<k);
                }else{
                    QHash<QPair<int,int>,QList<int> > tmphash;
                    tmphash.insert(gmval,QList<int>()<<k);
                    gmhash0.insert(gmkey,tmphash);
                }
            }
        }
    }
}

void neuron_match_gmhash::affine_nt1()
{
    if(pmatch0.size()<=0){
        qDebug()<<"not match point identified, quit align";
        return;
    }

    QList<XYZ> c0, c1;
    for(int i=0; i<pmatch0.size(); i++){
        c0.append(XYZ(nt0->listNeuron.at(pmatch0.at(i))));
        c1.append(XYZ(nt1->listNeuron.at(pmatch1.at(i))));
    }

    double tmp_shift_x=0,tmp_shift_y=0,tmp_shift_z=0,tmp_angle=0,tmp_cent_x=0,tmp_cent_y=0,tmp_cent_z=0;
    if(direction==0) tmp_shift_x=-1;
    if(direction==1) tmp_shift_y=-1;
    if(direction==2) tmp_shift_z=-1;
    if(compute_affine_4dof(c0,c1,tmp_shift_x,tmp_shift_y,tmp_shift_z,tmp_angle,tmp_cent_x,tmp_cent_y,tmp_cent_z,direction)<0){
        return;
    }

    affineNeuron(*nt1,*nt1,tmp_shift_x,tmp_shift_y,tmp_shift_z,tmp_angle,tmp_cent_x,tmp_cent_y,tmp_cent_z,direction);
    if(direction==0) tmp_shift_x=shift_x;
    if(direction==1) tmp_shift_y=shift_y;
    if(direction==2) tmp_shift_z=shift_z;
    shift_x = tmp_shift_x;
    shift_y = tmp_shift_y;
    shift_z = tmp_shift_z;
    rotation_ang = tmp_angle;
    rotation_cx = tmp_cent_x;
    rotation_cy = tmp_cent_y;
    rotation_cz = tmp_cent_z;
}

void neuron_match_gmhash::output(QString fname)
{
    QString fname_neuron;
    if(nt1->file.section('.',-1).toUpper() == "ESWC")
        fname_neuron = fname+"_affine.eswc";
    else
        fname_neuron = fname+"_affine.swc";

    //output neuron
    if (!export_list2file(nt1->listNeuron,fname_neuron,nt1->file))
    {
        v3d_msg("fail to write the output swc file:\n" + fname_neuron);
        return;
    }
}

void neuron_match_gmhash::examineMatchingResult(double num[11], NeuronTree* nt_truth)
{
    num[4]=candID0.size();
    num[5]=candID1.size();
    num[6]=gmhash0.size();
    num[7]=rankedPairMatch.size();
    num[8]=rankedPairMatch.begin()->first;

    //find all ground truth matches
    QSet<int> connSet0, connSet1;
    QMap<V3DLONG, int> VtxConnMap0,VtxConnMap1;
    for(V3DLONG pid=0; pid<nt0->listNeuron.size(); pid++){
        if(nt0->listNeuron.at(pid).type>0){
            connSet0.insert(nt0->listNeuron.at(pid).type);
            VtxConnMap0[pid]=nt0->listNeuron.at(pid).type;
        }
    }
    for(V3DLONG pid=0; pid<nt1->listNeuron.size(); pid++){
        if(nt1->listNeuron.at(pid).type>0){
            connSet1.insert(nt1->listNeuron.at(pid).type);
            VtxConnMap1[pid]=nt1->listNeuron.at(pid).type;
        }
    }
    connSet0.intersect(connSet1);
    num[3]=connSet0.size();

    //check all matches found
    num[0]=num[1]=num[2]=0;
    int maxcomp=0;
    for(QSet<int>::iterator iter=connSet0.begin(); iter!=connSet0.end(); iter++){
        maxcomp=MAX(maxcomp, *iter+1);
    }
    vector<int> record(maxcomp,-1);
    for(QSet<int>::iterator iter=connSet0.begin(); iter!=connSet0.end(); iter++){
        record[*iter]=0;
    }

    for(int i=0; i<pmatch0.size(); i++){
        if((!VtxConnMap0.contains(pmatch0.at(i)))||(!VtxConnMap1.contains(pmatch1.at(i)))){
            num[1]++;
            continue;
        }
        if(VtxConnMap0[pmatch0.at(i)] != VtxConnMap1[pmatch1.at(i)]){
            num[1]++;
            continue;
        }
        record[VtxConnMap0[pmatch0.at(i)]]++;
    }
    for(int i=0; i<record.size(); i++){
        if(record[i]==0){
            num[2]++;
        }
        if(record[i]>0){
            num[0]++;
        }
    }


    //calculate distance to truth
    if(nt_truth->listNeuron.size() == nt1->listNeuron.size()){
        num[9]=0;
        num[10]=0;
        for(int i=0; i<nt_truth->listNeuron.size(); i++){
            double tmpd=0;
            double tmps=(nt_truth->listNeuron.at(i).x-nt1->listNeuron.at(i).x);
            tmpd+=tmps*tmps;
            tmps=(nt_truth->listNeuron.at(i).y-nt1->listNeuron.at(i).y);
            tmpd+=tmps*tmps;
            num[10]+=sqrt(tmpd);
            tmps=(nt_truth->listNeuron.at(i).z-nt1->listNeuron.at(i).z)*zscale;
            tmpd+=tmps*tmps;
            num[9]+=sqrt(tmpd);
        }
        num[9]/=nt_truth->listNeuron.size();
        num[10]/=nt_truth->listNeuron.size();
    }else{
        num[9]=-1;
        num[10]=-1;
    }
}
