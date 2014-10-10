/*
 * 2014.10.07 by: Hanbo Chen cojoc(at)hotmail.com
*/
#include "neuron_stitch_func.h"

#include <QDialog>

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
    np->n=source.n; np->color=source.color; np->on=source.on; np->selected=source.selected; np->name=source.name; np->comment=source.comment;
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
        pt_tmp->color = ps_tmp->color;
    }
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
