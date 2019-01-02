//**********using functions from APP1*********developed by OYQ 12.27.2018***********************

#include "APP1_pruning.h"
//#include <vector>
//#include <stdio.h>
//#include <iostream>
//#include <algorithm>
//#include <iterator>

//#define _CHECK_PRUNING_PARAMETERS_() \
//    //V3DLONG npruned = 0; \
//    if (mUnittree.listNeuron.size()<=0 || !imap || !sz || sz[0]<=0 || sz[1]<=0 || sz[2]<=0) \
//    { \
//        v3d_msg("Wrong parameters in a pruning function.\n", 0); \
//        //return npruned; \
//    }


//V3DLONG pruning_covered_leaf_single_cover(vector< NeuronTree >& mmUnit, unsigned char ***imap,  float *dmap, V3DLONG sz[3], double trace_z_thickness)
void pruning_covered_leaf_single_cover(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    printf("welcome to tip locationalize");
    double trace_z_thickness=5;
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString input_swc=infiles.at(0);
    QString input_image=inparas.at(0);
    QString output_2d_dir=outfiles.at(0);
    if(!output_2d_dir.endsWith("/")){
        output_2d_dir = output_2d_dir+"/";
    }

    QStringList list=input_swc.split("/");
    QString flag=list[6]; QStringList list1=flag.split(".");// you don't need to add 1 to find the string you want in input_dir
    QString flag1=list1[0];

    qDebug("number:%s",qPrintable(flag1));
    NeuronTree mUnittree=readSWC_file(input_swc);
    QList<NeuronSWC> & mUnit =mUnittree.listNeuron ;
    Image4DSimple * p4dImage = callback.loadImage((char *)(qPrintable(input_image) ));
    int nChannel = p4dImage->getCDim();

    V3DLONG sz[4];
    sz[0] = p4dImage->getXDim();
    sz[1] = p4dImage->getYDim();
    sz[2] = p4dImage->getZDim();
    sz[3] = nChannel;
    V3DLONG pagesz = sz[0]*sz[1]*sz[2];
    V3DLONG sz1 = sz[0]*sz[1];

    vector<MyMarker> allmarkers;
//generate 1D point to 3D point and get all markers
    unsigned char ***imap;
    unsigned char *data1d_crop=p4dImage->getRawDataAtChannel(nChannel);
    for (int z=0;z<sz[2];z++){
        for (int y=0;y<sz[1];y++){
            for (int x=0;x<sz[0];x++){
                imap[z][y][x]=data1d_crop[z*sz1+y*sz[0]+x];
                MyMarker nodes;nodes.x=x;nodes.y=y;nodes.z=z;
                allmarkers.push_back(nodes);
            }
        }
    }
//get global average threshold
    double signal;
    signal=get_circle_signal(allmarkers,data1d_crop,sz[0], sz[1], sz[2]);
//caculate all radius of everynode
    map<int,double> r_and_index;
    r_and_index=calculate_R(imap,mUnit,signal,trace_z_thickness,sz);
//get child size of every node
    V3DLONG tnodes = mUnit.size();
    V3DLONG neuronNum = mUnittree.listNeuron.size();
    QVector<QVector<V3DLONG> > childs1;
    childs1 = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        V3DLONG par = mUnittree.listNeuron[i].pn;
        if (par<0) continue;
        childs1[mUnittree.hashNeuron.value(par)].push_back(i);
    }
    QVector<V3DLONG> childs;
    for(V3DLONG i=0;i<childs1.size();i++){

        childs.push_back(childs1[i].size());//get child size by index i(0~n)

    }
//get from APP1 pruning part below
    v3d_msg("pruning_covered_leaf_single_cover()\n", 0);
   // _CHECK_PRUNING_PARAMETERS_();

    V3DLONG i,j;


    //first propduce a LUT for swc access
    std::map<double,V3DLONG> index_map;	index_map.clear();
    vector <unsigned char> visited;
    V3DLONG root_id=-1;
    for (j=0; j<tnodes; j++)
    {
        double ndx   = mUnit[j].n;
        V3DLONG new_ndx = index_map.size(); //map the neuron node'd id to row number
        index_map[ndx] = new_ndx;
        visited.push_back(0); //set as non-visited

        if (mUnit[j].parent<0)
        {
            if (root_id!=-1)
                printf("==================== detect a non-unique root!\n");
            root_id = V3DLONG(mUnit[j].n);
            printf("==================== nchild of root [%ld, id=%ld] = %ld\n", j, V3DLONG(mUnit[j].n), V3DLONG(childs[j]));
        }
    }

    V3DLONG nleafdelete;
    V3DLONG nloops = 0;
    while (1)
    {
        nleafdelete = 0;
        nloops++;

        for (i=0; i<tnodes; i++)
        {
            if (childs[i]!=0) //do not start from a non-leaf node
                continue;

            V3DLONG pi = mUnit[i].parent;
            if (pi<0 || pi==mUnit[i].n) //isolated nodes should be removed
            {
                childs[i]= -1; //mark to delete
                nleafdelete++;
                continue;
            }

            V3DLONG tmpcnt=0;
            j = i;
            NeuronSWC & curnode = mUnit[i];

            double cur_margin = r_and_index[i];//curnode.r;
            //if (cur_margin > dmap[V3DLONG(curnode.z)*sz[0]*sz[1] + V3DLONG(curnode.y)*sz[0] + V3DLONG(curnode.x)]) cur_margin = dmap[V3DLONG(curnode.z)*sz[0]*sz[1] + V3DLONG(curnode.y)*sz[0] + V3DLONG(curnode.x)];
            //cur_margin = 0;
            while (tmpcnt<10) //to avoid loops
            {
                tmpcnt++;

                pi = mUnit[j].parent;
                V3DLONG pi_rownum = index_map[pi];
                if (mUnit[j].n == pi) //note should use .n == .parent, but not j==.parent! Also this condition should never be met, anyway.
                {
                    childs[j] = -1;
                    nleafdelete++;
                    printf("**************** exit at loopy node. tmpcnt=%ld\n", tmpcnt);
                    break;
                }

                if (pi<0)
                    break;

                NeuronSWC & curpnode = mUnit[pi_rownum];
                double tmpd = sqrt(distL2square(curnode, curpnode));
                double pi_radius = r_and_index[pi_rownum];//curpnode.r;
                //if (pi_radius < dmap[V3DLONG(curpnode.z)*sz[0]*sz[1] + V3DLONG(curpnode.y)*sz[0] + V3DLONG(curpnode.x)]) pi_radius = dmap[V3DLONG(curpnode.z)*sz[0]*sz[1] + V3DLONG(curpnode.y)*sz[0] + V3DLONG(curpnode.x)];
                if (tmpd + cur_margin <= pi_radius ||  //stop when it is out of the control-range (defined by radius)
                    calculate_overlapping_ratio_n1(curnode, curpnode, imap,  sz, trace_z_thickness,childs,index_map)>0.9)
                {
                    //if (tmpcnt>1) printf("exit at decreasing r (j=%ld) and tmpcnt>=2. tmpcnt=%ld\n", j, tmpcnt);
                    childs[i] = -1;
                    childs[index_map[curnode.parent]]--;
                    nleafdelete++;

                    if (V3DLONG(mUnit[index_map[curnode.parent]].n)==root_id)
                        printf("root nchild after decreasing 1 = %ld cur node id =%ld, cur node parent=%ld\n", V3DLONG(childs[index_map[curnode.parent]]), V3DLONG(curnode.n), V3DLONG(curnode.parent));
                    break;
                }

                j = pi_rownum; //continue
            }
        }

        //npruned += nleafdelete;
        if (nleafdelete==0)
        {
            printf("No more leaf should be deleted. total loops = [%ld].\n", nloops);
            break;
        }
        else
        {
            printf("delete [%ld] leaf-nodes in this [%ld] run.\n", nleafdelete, nloops);
        }
    }

    //delete those deleted labels and rearrange index

    rearrange_and_remove_labeled_deletion_nodes_mmUnit(mUnittree,childs);

    QString swc_name = output_2d_dir+flag1+"."+QString ("tip_APP1_purned.swc");
    writeSWC_file(swc_name,mUnittree);
    qDebug()<<swc_name;


    printf("done with the pruning_covered_leaf_single_cover() step. \n");

    //return npruned;
}

double distL2square(const NeuronSWC & a, const NeuronSWC & b)
{
    return ((a.x-b.x)*(a.x-b.x) +
            (a.y-b.y)*(a.y-b.y) +
            (a.z-b.z)*(a.z-b.z) );
}

double calculate_overlapping_ratio_n1(const NeuronSWC & n1, const NeuronSWC & n2, unsigned char ***imap,  V3DLONG sz[3], double trace_z_thickness, QVector<V3DLONG> childs,map<double, V3DLONG> index_map)
{
    //in this function I intentionally do not check if n2 has been marked as to be deleted (.nchild<0), as this function should be general

    V3DLONG R =childs[index_map[n1.n]];
    V3DLONG R2=childs[index_map[n2.n]];
    V3DLONG i,j,k;
    //V3DLONG R = ceil(n1.r);
    double r12 = R*R, r22 = R2*R2;
    double curr_k, curr_j, curr_i;
    double n1x = n1.x, n1y = n1.y, n1z = n1.z;
    V3DLONG cx, cy, cz;
    double n1_totalsample = 0, n2_includedsample = 0, n1_totalpixel=0;
    for (k=-R/trace_z_thickness;k<=R/trace_z_thickness;k++)
    {
        cz = V3DLONG(n1z+k+0.5); if (cz<0 || cz>=sz[2]) continue;
        curr_k = double(k)*k;
        for (j=-R;j<=R;j++)
        {
            cy = V3DLONG(n1y+j+0.5); if (cy<0 || cy>=sz[1]) continue;
            if ((curr_j = double(j)*j+curr_k) > r12)
                continue;

            for (i=-R;i<=R;i++)
            {
                cx = V3DLONG(n1x+i+0.5); if (cx<0 || cx>=sz[0]) continue;
                if ((curr_i = double(i)*i+curr_j) > r12)
                    continue;

                n1_totalpixel += 1;
                n1_totalsample += imap[cz][cy][cx];
                if ((cz-n2.z)*(cz-n2.z)+(cy-n2.y)*(cy-n2.y)+(cx-n2.x)*(cx-n2.x) <= r22)
                    n2_includedsample += imap[cz][cy][cx];
            }
        }
    }

    if (n1_totalpixel==0)
    {
        v3d_msg("total # sample is 0. This means radius is wrong. Should never see this!!! Check data and program. \n", 0);
        printf("total pixel=%ld, R=%ld n1.r=%5.3f n1x=%5.3f n1y=%5.3f n1z=%5.3f sz[0]=%ld sz[1]=%ld sz[2]=%ld ", V3DLONG(n1_totalpixel), R, n1.r, n1x, n1y, n1z, sz[0], sz[1], sz[2]);
        return 1; //return 1 because this means this n1 node should be eleminated anyway
    }

    return (n1_totalsample<=0) ? 1.0 : (n2_includedsample/n1_totalsample);
}


void rearrange_and_remove_labeled_deletion_nodes_mmUnit(NeuronTree & mmUnit,QVector<V3DLONG> childs) //by PHC, 2011-01-15
{


    printf("....... rearranging index number \n");
//    std::map<double,V3DLONG> index_map;

    V3DLONG i,j,k;

//    for (k=0; k<mmUnit.size(); k++)
//    {
//        printf("....... removing the [%ld] segment children that have been labeled to delete (i.e. nchild < 0).\n", k);
        QList<NeuronSWC> & mUnit = mmUnit.listNeuron;
//        index_map.clear();

//        for (j=0; j<mUnit.size(); j++)
//        {
//            if (mUnit[j].nchild >=0)
//            {
//                double ndx   = mUnit[j].n;
//                V3DLONG new_ndx = index_map.size()+1;
//                index_map[ndx] = new_ndx;
//            }
//        }
//        for (j=0; j<mUnit.size(); j++)
//        {
//            if (mUnit[j].nchild >=0)
//            {
//                double ndx    = mUnit[j].n;
//                double parent = mUnit[j].parent;
//                mUnit[j].n = index_map[ndx];
//                if (parent>=0)	mUnit[j].parent = index_map[parent];
//            }
//        }

        QList<NeuronSWC> mUnit_new;
        V3DLONG root_id=-1;
        for (j=0; j<mUnit.size(); j++)
        {
            if (childs[j] >= 0)
            {
                mUnit_new.push_back(mUnit[j]);
            }

            if (mUnit[j].parent<0)
            {
                if (root_id!=-1)
                    printf("== [segment %ld] ================== detect a non-unique root!\n", k);
                root_id = V3DLONG(mUnit[j].n);
                printf("== [segment %ld] ================== nchild of root [%ld, id=%ld] = %ld\n", k, j, V3DLONG(mUnit[j].n), V3DLONG(childs[j]));
            }

        }
        NeuronTree result;
        QHash <int, int>  hashNeuron;
        for(V3DLONG j=0; j<mUnit_new.size();j++)
        {
           hashNeuron.insert(mUnit_new[j].n, j);
        }
        result.listNeuron=mUnit_new;
        result.hashNeuron=hashNeuron;

        mmUnit = result;
//    }
}

map<int,double> calculate_R(unsigned char ***imap,QList<NeuronSWC> mUnit,double avr_thres,double trace_z_thickness,V3DLONG sz[4]){

    double tnode=mUnit.size();
    double z_thick=trace_z_thickness;
    std::map<int,double> index_radius;//note the index and radius of everynode
    index_radius.clear();
    for (int i=0;i<tnode;i++)
    {
        double n1x=mUnit.at(i).x;
        double n1y=mUnit.at(i).y;
        double n1z=mUnit.at(i).z;
        V3DLONG g,j,k;
        V3DLONG cx, cy, cz;
        double ratio=0;
        double R=0;

       while(ratio<0.001){

       R+=1;
       double less_num=0;
       if (n1x<R || n1y<R || n1z<R) {index_radius[i]=1;break;}//node's R out off image will not be calculated,this condition is ok for tip node in center
       trace_z_thickness=(R<5)? R:z_thick;
       double totalsz=8*R*R*R/trace_z_thickness;
       for (k=-R/trace_z_thickness;k<=R/trace_z_thickness;k++)
       {
           cz = int(n1z+k+0.5); if (cz<0 || cz>=sz[2]) continue;
           for (j=-R;j<=R;j++)
           {
               cy = int(n1y+j+0.5); if (cy<0 || cy>=sz[1]) continue;
               for (g=-R;g<=R;g++)
               {
                   cx = int(n1x+g+0.5); if (cx<0 || cx>=sz[0]) continue;
                   if(imap[cz][cy][cx]<avr_thres) less_num += 1;
               }
           }
       }
          if (less_num<=0) continue;
          ratio=less_num/totalsz;
       }
    index_radius[i]=R;
    printf("this id:%d \n radius:%d \n",i,R);
    if (R>sz[0] || R>sz[1] || R>sz[2]) cout<<"your node's radius bigger than your image size ,plz check your code!"<<endl;
    }
    return index_radius;
}

