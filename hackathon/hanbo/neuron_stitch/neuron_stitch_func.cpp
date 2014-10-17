/*
 * 2014.10.07 by: Hanbo Chen cojoc(at)hotmail.com
*/
#include "neuron_stitch_func.h"

#include <QDialog>
#include <fstream>
#include <iostream>
#include <vector>
#include "math.h"

using namespace std;

double distance_XYZList(QList<XYZ> c0, QList<XYZ> c1)
{
    if(c0.size()!=c1.size()){
        return -1;
    }
    double dis = 0;
    for(int i=0; i<c0.size(); i++){
        dis+=sqrt(NTDIS(c0[i],c1[i]));
    }
    return dis;
}

void rotation_XYZList(QList<XYZ> in, QList<XYZ>& out, double angle, int axis) //angle is 0-360  based
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

    for(int i=0; i<in.size();i++){
        out[i].x = afmatrix[0]*in[i].x+afmatrix[1]*in[i].y+afmatrix[2]*in[i].z;
        out[i].y = afmatrix[4]*in[i].x+afmatrix[5]*in[i].y+afmatrix[6]*in[i].z;
        out[i].z = afmatrix[8]*in[i].x+afmatrix[9]*in[i].y+afmatrix[10]*in[i].z;
    }
}

bool compute_affine_4dof(QList<XYZ> c0, QList<XYZ> c1, double& shift_x, double& shift_y, double & shift_z, double & angle_r, double& cent_x,double& cent_y,double& cent_z, int dir)
{
    //check
    if(c0.size() != c1.size()){
        qDebug()<<"error: the number of points for affine does not match";
        shift_x = shift_y = shift_z = angle_r = 0;
        cent_x=cent_y=cent_z = 0;
        return false;
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

    //align by center first
    shift_x=cent0[0]-cent1[0];
    shift_y=cent0[1]-cent1[1];
    shift_z=cent0[2]-cent1[2];
    for(int i=0; i<c0.size(); i++){
        c0[i].x -= cent0[0];
        c0[i].y -= cent0[1];
        c0[i].z -= cent0[2];
        c1[i].x -= cent1[0];
        c1[i].y -= cent1[1];
        c1[i].z -= cent1[2];
    }

    //3 steps rotation
    cent_x=cent0[0];cent_y=cent0[1];cent_z=cent0[2];
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
        rotation_XYZList(c1,c1bk,ang,dir);
        double dis = distance_XYZList(c0, c1bk);
        if(dis<mdis){
            mdis=dis;
            mang=ang;
        }
    }
    qDebug()<<"rotation 0: "<<mang<<":"<<mdis;
    //step 2
    for(double ang=mang-10; ang<mang+10; ang++){
        rotation_XYZList(c1,c1bk,ang,dir);
        double dis = distance_XYZList(c0, c1bk);
        if(dis<mdis){
            mdis=dis;
            mang=ang;
        }
    }
    qDebug()<<"rotation 1: "<<mang<<":"<<mdis;
    //step 3
    for(double ang=mang-1; ang<mang+1; ang+=0.1){
        rotation_XYZList(c1,c1bk,ang,dir);
        double dis = distance_XYZList(c0, c1bk);
        if(dis<mdis){
            mdis=dis;
            mang=ang;
        }
    }
    qDebug()<<"rotation 2: "<<mang<<":"<<mdis;
    angle_r=mang;
    qDebug()<<"calculated affine: "<<shift_x<<":"<<shift_y<<":"<<shift_z<<":"<<angle_r<<":"<<mdis;
    return true;
}

void update_marker_info(const ImageMarker& mk, int* info) //info[0]=neuron id, info[1]=point id, info[2]=matching marker
{
    ImageMarker *p;
    p = (ImageMarker *)&mk;
    p->comment=QString::number(info[0]) + " " + QString::number(info[1]) + " " + QString::number(info[2]);
}

bool get_marker_info(const ImageMarker& mk, int* info) //info[0]=neuron id, info[1]=point id, info[2]=matching marker
{
    info[0]=info[1]=info[2]=-1;
    QStringList items = mk.comment.split(" ", QString::SkipEmptyParts);
    if(items.size()!=3)
        return false;
    for(int i=0; i<3; i++){
        bool check=false;
        int val=items[i].toInt(&check, 10);
        if(!check)
            return false;
        else
            info[i]=val;
    }
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
                tp->type=7;
                tp = (NeuronSWC *)(&(nt2.listNeuron[j]));
                tp->type=7;
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
