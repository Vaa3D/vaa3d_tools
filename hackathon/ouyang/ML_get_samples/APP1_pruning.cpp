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
#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define MINDIS=2

void pruning_covered_leaf_single_cover(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    cout<<"welcome to tip locationalize"<<endl;
    double trace_z_thickness=5;
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString input_swc=infiles.at(0);
    QString input_image=infiles.at(1);

    bool aver_or_threhold = (inparas.size() >= 2) ? atoi(inparas[1]) : 0;

    double threshold = (inparas.size() >= 3) ? atoi(inparas[2]) : 30;

    int mode=2;
    if (inparas.size()!=0)
    {
        int tmp=atoi(inparas.at(0));
        if (tmp==2)
        {
            qDebug()<<"You are using 2D image to calculate node radius";
        }
        else
        {
            mode=3;
            v3d_msg("You are using 3D image to calculate node radius",0);
        }
    }

    QString output_2d_dir=outfiles.at(0);
    if(!output_2d_dir.endsWith("/")){
        output_2d_dir = output_2d_dir+"/";
    }

    QStringList list=input_swc.split("/");
    QString flag=list.last(); QStringList list1=flag.split(".");// you don't need to add 1 to find the string you want in input_dir
    QString flag1=list1.first();//dont need to match list index

    qDebug("number:%s",qPrintable(flag1));
    NeuronTree mUnittree1=readSWC_file(input_swc);

    Image4DSimple * p4dImage = callback.loadImage((char *)(qPrintable(input_image) ));
    int nChannel = p4dImage->getCDim();

    V3DLONG sz[4];
    sz[0] = p4dImage->getXDim();
    sz[1] = p4dImage->getYDim();
    sz[2] = p4dImage->getZDim();
    sz[3] = nChannel;
    V3DLONG pagesz = sz[0]*sz[1]*sz[2];
    V3DLONG sz1 = sz[0]*sz[1];


    printf("image sizex %d\nsizey %d\nsizez %d\n",sz[0],sz[1],sz[2]);
    vector<MyMarker> allmarkers;
//generate 1D point to 3D point and get all markers
    unsigned char ***imap;
    unsigned char *data1d_crop=p4dImage->getRawDataAtChannel(nChannel);
//get global average threshold
    for (int i=0;i<sz[2];i++){
        for (int j=0;j<sz[1];j++){
            for (int k=0;k<sz[0];k++){
                MyMarker node;
                node.x=k;node.y=j;node.z=i;
                allmarkers.push_back(node);
            }
        }
    }
    printf("pixel num of image:%d \n",allmarkers.size());
    double signal;
    unsigned char *data2d=get_2d_pixel(data1d_crop, sz);//get 2D image signal
    signal=get_aver_signal(allmarkers, data1d_crop, data2d,sz[0], sz[1], sz[2], mode);
    printf("average image signal:%f \n",signal);

//pruning dark tip node
       trer_and_num mUnittree3;
       mUnittree3.tree=mUnittree1;
       int off=1;
       while(off != 0)
       {
           //trer_and_num result_struct;
           mUnittree3=pruning_dark_tip_node(mUnittree3.tree,data1d_crop,signal,sz);//pruning dark nodes still uses average value as threshold,cas we need conservative result
           off=mUnittree3.delete_num;
       }
       NeuronTree mUnittree2=mUnittree3.tree;
       int delete_num=mUnittree1.listNeuron.size()-mUnittree2.listNeuron.size();
       cout<<"-----------------------------pruning dark nodes number:"<<delete_num<<endl;

//get all swc node to marker struct
    QList<NeuronSWC> & mUnit_ori =mUnittree2.listNeuron ;
    vector<MyMarker> allmarkersforradius;
        for (int i=0;i<mUnit_ori.size();i++){
                    MyMarker node;
                    node.x=mUnit_ori.at(i).x;node.y=mUnit_ori.at(i).y;node.z=mUnit_ori.at(i).z;
                     allmarkersforradius.push_back(node);
        }
        printf("swc marker num of image:%d \n", allmarkersforradius.size());

//chose threshold based on average signal
    if (signal>=10 && signal<=20) threshold=30;
    else if (signal<10) threshold=20;
    else if (signal>20 && signal<=30) threshold=40;
    else if (signal>30) threshold=50;

//caculate all radius of everynode
    map<int,float> r_and_index;
    r_and_index=calculate_R(data1d_crop,mUnit_ori,signal,trace_z_thickness,sz,mode,threshold,aver_or_threhold);
    //r_and_index=markerRadius_hanchuan_XY(data1d_crop, sz,  allmarkersforradius, threshold ,aver_or_threhold ,signal);
//revise radius of swc nodes
    NeuronTree mUnittree=revise_radius(mUnittree2,r_and_index);
    QList<NeuronSWC>  mUnit =mUnittree.listNeuron ;
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

            //node in margin should't be calculated
            double x_cur,y_cur,z_cur;
            x_cur=curnode.x;y_cur=curnode.y;z_cur=curnode.z;
            double r_cur=r_and_index[i];
            if(r_cur>=x_cur || r_cur>=y_cur || r_cur>=z_cur){

                printf("this:%d node's radius is out off image block(nodes in margin),will not be calculated",i);
                continue;
            }

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

                if (tmpd + cur_margin <= pi_radius ||  //stop when it is out of the control-range (defined by radius)
                    calculate_overlapping_ratio_n1(curnode, curpnode, data1d_crop, data2d, sz, trace_z_thickness, r_and_index, index_map, mode)>0.9)
                {
                    V3DLONG R =r_and_index[index_map[curnode.n]];
                    V3DLONG R2=r_and_index[index_map[curpnode.n]];
                    childs[i] = -1;
                    childs[index_map[curnode.parent]]--;
                    //printf("tip's parent child size:%d\n",childs[index_map[curnode.parent]]);
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
    int dele_nums,ori_sz,post_sz;
    ori_sz=mUnittree.listNeuron.size();
    rearrange_and_remove_labeled_deletion_nodes_mmUnit(mUnittree,childs,dele_nums);
    post_sz=mUnittree.listNeuron.size();
    dele_nums=ori_sz-post_sz;
    printf("---------------------pruned swc num :%d\n",dele_nums);

    QString swc_name = output_2d_dir+flag1+"."+QString ("purned.swc");
    writeSWC_file(swc_name,mUnittree);


//record result
    FILE * fp=0;
    QString out_result = output_2d_dir+flag1;
    fp = fopen((char *)qPrintable(out_result+QString(".txt")), "wt");
    qDebug("--------------------------------txt output dir:%s",qPrintable(output_2d_dir));

    QString aver_or_thres;
    if(aver_or_threhold)  {aver_or_thres="Yes";}
    else {aver_or_thres="No";}

    double aver_r;
        double cnt=0;
        double total_r=0;
        V3DLONG sz_R=r_and_index.size();if (sz_R<=0) cout<<"NO radius data in your result"<<endl;
        for (V3DLONG i=0;i<sz_R;i++){

            total_r+=r_and_index[i];
            cnt++;
        }
        aver_r=total_r/cnt;


    fprintf(fp, "1.Name 2.Input_thres 3.Average_sig 4.Average_R 5.Nums_dark 6.Nums_overlap 7.Use input_thres or not\n");
    fprintf(fp,"%s %.2f %.2f %.2f %d %d %s",flag1.toStdString().c_str(),threshold,signal,aver_r,delete_num,dele_nums,aver_or_thres.toStdString().c_str());
    fclose(fp);



//    // test.1 image ID
//    fprintf(fp, "Pruned sample name:\t%s\n", input_swc.toStdString().c_str());
//    // test.2 input threshold
//    fprintf(fp, "Input threshold:\t%f\n", threhold);
//    // test.3 average signal value
//    fprintf(fp, "Average signal value of current image:\t%f\n", signal);
//    // test.4 use threshold or not
//    QString aver_or_thres;
//    if(!aver_or_threhold)  {aver_or_thres="No";}
//    else {aver_or_thres="Yes";}
//    fprintf(fp, "Use input threshold/average threshold:\t%s\n", aver_or_thres.toStdString().c_str());// %s is for char not string
//    // test.5 average radius
//    double aver_r;
//    double cnt=0;
//    double total_r=0;
//    V3DLONG sz_R=r_and_index.size();if (sz_R<=0) cout<<"NO radius data in your result"<<endl;
//    for (V3DLONG i=0;i<sz_R;i++){

//        total_r+=r_and_index[i];
//        cnt++;
//    }
//    aver_r=total_r/cnt;
//    fprintf(fp, "Average radius(input threshold/average threshold):\t%f\n", aver_r);
//    // test.6 dark nodes pruned number
//    fprintf(fp, "Dark nodes pruning number:\t%d\n", delete_num);
//    // test.7 single overlapping nodes pruned number
//    fprintf(fp, "Single overlapping nodes pruning number:\t%d\n", dele_nums);
//    fclose(fp);


    printf("done with the pruning_covered_leaf_single_cover() step. \n");

    //return npruned;
}

double distL2square(const NeuronSWC & a, const NeuronSWC & b)
{
    return ((a.x-b.x)*(a.x-b.x) +
            (a.y-b.y)*(a.y-b.y) +
            (a.z-b.z)*(a.z-b.z) );
}

double calculate_overlapping_ratio_n1(const NeuronSWC & n1, const NeuronSWC & n2, unsigned char *data1D, unsigned char *data2D,V3DLONG sz[3], double trace_z_thickness,
                                      map<int,float> r_and_index,map<double, V3DLONG> index_map,int mode)
{
    //in this function I intentionally do not check if n2 has been marked as to be deleted (.nchild<0), as this function should be general

    V3DLONG R =r_and_index[index_map[n1.n]];
    V3DLONG R2=r_and_index[index_map[n2.n]];

    V3DLONG i,j,k;
    //V3DLONG R = ceil(n1.r);
    double r12 = R*R, r22 = R2*R2;
    double curr_k, curr_j, curr_i;
    double n1x = n1.x, n1y = n1.y, n1z = n1.z;
    V3DLONG cx, cy, cz;
    double n1_totalsample = 0, n2_includedsample = 0, n1_totalpixel=0;

    if(mode=3){
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

                  V3DLONG ids=cz*sz[0]*sz[1]+cy*sz[1]+cx;

                  n1_totalpixel += 1;
                  n1_totalsample += data1D[ids];
                  if ((cz-n2.z)*(cz-n2.z)+(cy-n2.y)*(cy-n2.y)+(cx-n2.x)*(cx-n2.x) <= r22)
                      n2_includedsample += data1D[ids];
              }
          }
      }
    }

    if(mode=2){
//      for (k=-R/trace_z_thickness;k<=R/trace_z_thickness;k++)
//      {
//          cz = V3DLONG(n1z+k+0.5); if (cz<0 || cz>=sz[2]) continue;
//          curr_k = double(k)*k;
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

                  V3DLONG ids=cy*sz[1]+cx;

                  n1_totalpixel += 1;
                  n1_totalsample += data2D[ids];
                  if ((cy-n2.y)*(cy-n2.y)+(cx-n2.x)*(cx-n2.x) <= r22)
                      n2_includedsample += data2D[ids];
              }
          }
      //}
    }

    if (n1_totalpixel==0)
    {
        v3d_msg("total # sample is 0. This means radius is wrong. Should never see this!!! Check data and program. \n", 0);
        printf("total pixel=%ld, R=%ld n1.r=%5.3f n1x=%5.3f n1y=%5.3f n1z=%5.3f sz[0]=%ld sz[1]=%ld sz[2]=%ld ", V3DLONG(n1_totalpixel), R, n1.r, n1x, n1y, n1z, sz[0], sz[1], sz[2]);
        return 1; //return 1 because this means this n1 node should be eleminated anyway
    }

    return (n1_totalsample<=0) ? 1.0 : (n2_includedsample/n1_totalsample);
}


void rearrange_and_remove_labeled_deletion_nodes_mmUnit(NeuronTree & mmUnit,QVector<V3DLONG> childs,int delete_nums) //by PHC, 2011-01-15
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
//                if (root_id!=-1)
//                    printf("== [segment %ld] ================== detect a non-unique root!\n", k);
                root_id = V3DLONG(mUnit[j].n);
                printf("== [segment %ld] ================== nchild of root [%ld, id=%ld] = %ld\n", k, j, V3DLONG(mUnit[j].n), V3DLONG(childs[j]));
            }

        }
        printf("----------------not pruned swc size :%d\n",mUnit.size());
        printf("--------------------pruned swc size :%d\n",mUnit_new.size());
        int a=mUnit.size();int b= mUnit_new.size();
        delete_nums=a-b;
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


map<int,float> markerRadius_hanchuan_XY(unsigned char *inimg1d, V3DLONG  sz[4], vector<MyMarker> allmarkers, double thresh,bool aver_or_threhold,double aver)
{
    //printf("markerRadius_hanchuan   XY 2D\n");
    if(!aver_or_threhold) thresh=(float)aver;
    map<int,float> radius;
    for(int i=0;i<allmarkers.size();i++){

        MyMarker marker=allmarkers.at(i);
        long sz0 = sz[0];
        long sz01 = sz[0] * sz[1];
        double max_r = sz[0]/2;
        if (max_r > sz[1]/2) max_r = sz[1]/2;

        double total_num, background_num;
        double ir;
        for (ir=1; ir<=max_r; ir++)
        {
            total_num = background_num = 0;

            double dz, dy, dx;
            double zlower = 0, zupper = 0;
            for (dz= zlower; dz <= zupper; ++dz)
                for (dy= -ir; dy <= +ir; ++dy)
                    for (dx= -ir; dx <= +ir; ++dx)
                    {
                        total_num++;

                        double r = sqrt(dx*dx + dy*dy + dz*dz);
                        if (r>ir-1 && r<=ir)
                        {
                            V3DLONG i = marker.x+dx;   if (i<0 || i>=sz[0]) goto end1;
                            V3DLONG j = marker.y+dy;   if (j<0 || j>=sz[1]) goto end1;
                            V3DLONG k = marker.z+dz;   if (k<0 || k>=sz[2]) goto end1;

                            if (inimg1d[k * sz01 + j * sz0 + i] <= thresh)
                            {
                                background_num++;

                                if ((background_num/total_num) > 0.001) goto end1; //change 0.01 to 0.001 on 100104
                            }
                        }
                    }
        }
    end1:
        radius[i]=ir;
 }
    return radius;

}


map<int,float> calculate_R(unsigned char *data1D,QList<NeuronSWC> mUnit,double avr_thres,double trace_z_thickness,V3DLONG sz[4],int mode,double thresh,bool aver_or_threhold){

switch(mode){
  case 3:
  {
    if(!aver_or_threhold) thresh=avr_thres;
    double tnode=mUnit.size();
    double z_thick=trace_z_thickness;
    V3DLONG pagesz = sz[0]*sz[1]*sz[2];
    std::map<int,float> index_radius;//note the index and radius of everynode
    index_radius.clear();
    for (int i=0;i<tnode;i++)
    {
        double n1x=mUnit.at(i).x;
        double n1y=mUnit.at(i).y;
        double n1z=mUnit.at(i).z;

        V3DLONG g,j,k;
        V3DLONG cx, cy, cz;
        double ratio=0;
        float R=0;

     while(ratio<0.001)
     {

       R+=1;
       double less_num=0;


       trace_z_thickness=(R<5)? R:z_thick;
       float totalsz=8*R*R*R/trace_z_thickness;
       for (k=-R/trace_z_thickness;k<=R/trace_z_thickness;k++)
       {
           cz = int(n1z+k+0.5); if (cz<0 || cz>=sz[2]) continue;
           for (j=-R;j<=R;j++)
           {
               cy = int(n1y+j+0.5); if (cy<0 || cy>=sz[1]) continue;
               for (g=-R;g<=R;g++)
               {
                   cx = int(n1x+g+0.5); if (cx<0 || cx>=sz[0]) continue;
                   if ((cz-n1z)*(cz-n1z)+(cy-n1y)*(cy-n1y)+(cx-n1x)*(cx-n1x) <= R*R){  //make sure that the current node is in the range of R

                         V3DLONG ids=cz*sz[0]*sz[1]+cy*sz[1]+cx;
                         if(data1D[ids]<thresh) less_num += 1;
                   }
               }
           }
       }
          if (less_num<=0) continue;
          ratio=less_num/totalsz;
     }

       if (n1x<R || n1y<R || n1z<R) {

           index_radius[i]=R;//node's R out off image will not be calculated,and will not change it's radius ,this condition is ok for tip node in center
           printf("this node's radius out off image: %d\n",i);
           cout<<"radius:"<<R<<endl;
        goto here;
       }

           index_radius[i]=R;
           printf("this node's radius is in image block:%d\n",i);
           cout<<"radius:"<<R<<endl;
        here:
           if (R>sz[0] || R>sz[1] || R>sz[2]) cout<<"your node's radius bigger than your image size ,plz check your code!"<<endl;
       }
//    for(int i=0;i<index_radius.size();i++){

//        cout<<"id and r:"<<index_radius[i]<<endl;
//    }
     return index_radius;
     break;
    }
  case 2:
  {
    if(!aver_or_threhold) thresh=avr_thres;
    unsigned char *image_mip= get_2d_pixel(data1D,sz);
    double tnode=mUnit.size();
    std::map<int,float> index_radius;//note the index and radius of everynode
    index_radius.clear();
    for (int i=0;i<tnode;i++)
    {
        double n1x=mUnit.at(i).x;
        double n1y=mUnit.at(i).y;

        V3DLONG g,j,k;
        V3DLONG cx, cy, cz;
        double ratio=0;
        float R=0;

     while(ratio<0.01)//what's the ratio should be in 2D image?
     {

         R+=1;
       double less_num=0;

       double totalsz=0;
           for (j=-R;j<=R;j++)
           {
               cy = int(n1y+j+0.5); if (cy<0 || cy>=sz[1]) continue;
               for (g=-R;g<=R;g++)
               {
                   cx = int(n1x+g+0.5); if (cx<0 || cx>=sz[0]) continue;
                   if ((cy-n1y)*(cy-n1y)+(cx-n1x)*(cx-n1x) <= R*R){  //make sure that the current node is in the range of R

                         totalsz+=1;
                         V3DLONG ids=cy*sz[0]+cx;
                         if(image_mip[ids]<thresh) less_num += 1;
                   }
               }
           }
          if (less_num<=0) continue;
          ratio=less_num/totalsz;
     }

     if (n1x<R || n1y<R ) {

           index_radius[i]=R;//node's R out off image will not be calculated,and will not change it's radius ,this condition is ok for tip node in center
           printf("this node's radius out off image: %d\n",i);
           cout<<"radius:"<<R<<endl;
           continue;
     }

        index_radius[i]=R;
        printf("this id in image block:%d\n",i);
        cout<<"radius:"<<R<<endl;
        if (R>sz[0] || R>sz[1] || R>sz[2]) cout<<"your node's radius bigger than your image size ,plz check your code!"<<endl;
     }
        return index_radius;
        break;
    }
  }
}

NeuronTree revise_radius(NeuronTree inputtree,map<int,float> radius){

    QList<NeuronSWC> result;
    QList<NeuronSWC> swc_line=inputtree.listNeuron;
    for(int i=0;i<swc_line.size();i++){

        NeuronSWC s;
        s.x=swc_line.at(i).x;
        s.y=swc_line.at(i).y;
        s.z=swc_line.at(i).z;
        s.type=swc_line.at(i).type;
        s.radius=radius[i];
        s.pn=swc_line.at(i).pn;
        s.n=swc_line.at(i).n;
        result.push_back(s);
    }
    NeuronTree result1;
    QHash <int, int>  hashNeuron;
    for(V3DLONG j=0; j<result.size();j++)
    {
       hashNeuron.insert(result[j].n, j);
    }
    result1.listNeuron=result;
    result1.hashNeuron=hashNeuron;
    return result1;
}

unsigned char * get_2d_pixel(unsigned char *data1d_crop,V3DLONG mysz[4]){

  V3DLONG stacksz =mysz[0]*mysz[1];
  unsigned char *image_mip=0;
  image_mip = new unsigned char [stacksz];//2D orignal image
  unsigned char *label_mip=0;
  label_mip = new unsigned char [stacksz];//2D annotation
  for(V3DLONG iy = 0; iy < mysz[1]; iy++)
  {
    V3DLONG offsetj = iy*mysz[0];
    for(V3DLONG ix = 0; ix < mysz[0]; ix++)
    {
        int max_mip = 0;
        int max_label = 0;
        for(V3DLONG iz = 0; iz < mysz[2]; iz++)
        {
            V3DLONG offsetk = iz*mysz[1]*mysz[0];
            if(data1d_crop[offsetk + offsetj + ix] >= max_mip)
            {
                image_mip[iy*mysz[0] + ix] = data1d_crop[offsetk + offsetj + ix];
                max_mip = data1d_crop[offsetk + offsetj + ix];
            }
        }
    }
  }
  return image_mip;
}


double get_aver_signal(vector<MyMarker> allmarkers, unsigned char * data1d,unsigned char * data2d,long sz0, long sz1, long sz2, int mode)
{
    switch(mode){
    case 3:
    {
      double signal = 0;
      double in_block_ct = 0;
      long sz01 = sz0 * sz1;
      long total_sz = sz0 * sz1 * sz2;

      for(int i=0; i<allmarkers.size();i++){
        int x = allmarkers[i].x;
        int y = allmarkers[i].y;
        int z = allmarkers[i].z;
        int id = z*sz01+y*sz0+x;
        if(id<total_sz){
            signal += data1d[id];
            //cout<<"this:"<<id<<" id's signal indensity:"<<data1d[id]<<endl;
            in_block_ct += 1;
        }

      }
      //cout<<"total signal:"<<signal<<endl;
      if(in_block_ct>0)
      {
          signal = signal / in_block_ct;
          return signal;
      }
    }
    case 2:
    {
        double signal = 0;
        double in_block_ct = 0;
        long sz01 = sz0 * sz1;
        long total_sz = sz0 * sz1;

        for(int i=0; i<allmarkers.size();i++){
          int x = allmarkers[i].x;
          int y = allmarkers[i].y;
          //int z = allmarkers[i].z;
          int id = y*sz0+x;
          if(id<total_sz){
              signal += data2d[id];
              //cout<<"this:"<<id<<" id's signal indensity:"<<data1d[id]<<endl;
              in_block_ct += 1;
          }

        }
        //cout<<"total signal:"<<signal<<endl;
        if(in_block_ct>0)
        {
            signal = signal / in_block_ct;
            return signal;
        }
    }
   }
}

trer_and_num pruning_dark_tip_node(NeuronTree mmUnit,unsigned char *image3d,double threshold,V3DLONG sz[4]){

    //threshold=30;
    trer_and_num result_struct;
    QVector<QVector<V3DLONG> > childs;
    V3DLONG neuronNum = mmUnit.listNeuron.size();
    MyMarker center;
    center.x=sz[0]/2;center.y=sz[1]/2;center.z=sz[2]/2;
    double dis1=0;

    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        V3DLONG par = mmUnit.listNeuron[i].pn;
        if (par<0) continue;
        childs[mmUnit.hashNeuron.value(par)].push_back(i);
    }

    QList<NeuronSWC> & mUnit = mmUnit.listNeuron;
    int count=0;
    int delete_count;
    int total_count=mUnit.size();


    V3DLONG i,j,k;
    QList<NeuronSWC> mUnit_new;
    long sz0 = sz[0];
    long sz01 = sz[0] * sz[1];
    for (j=0; j<total_count; j++)
        {
            V3DLONG cx=V3DLONG(mUnit.at(j).x+0.5);
            V3DLONG cy=V3DLONG(mUnit.at(j).y+0.5);
            V3DLONG cz=V3DLONG(mUnit.at(j).z+0.5);
            V3DLONG ids=cz*sz01+cy*sz[1]+cx;
            dis1=dist(mUnit.at(j),center);
            if ((image3d[ids] <= threshold) && (dis1 <= 2) && (childs[j].size() == 0)) continue;
            //if ((image3d[ids] <= threshold) && (childs[j].size() == 0)) continue;
            else
            {
                mUnit_new.push_back(mUnit[j]);
                count++;
            }
        }
     delete_count=total_count-count;
     NeuronTree result;
     QHash <int, int>  hashNeuron;
     for(V3DLONG j=0; j<mUnit_new.size();j++)
        {
           hashNeuron.insert(mUnit_new[j].n, j);
        }
     result.listNeuron=mUnit_new;
     result.hashNeuron=hashNeuron;
     result_struct.tree=result;
     result_struct.delete_num=delete_count;
     return result_struct;
}

