
/*
 *plugin for find the wrong type in constructed neuron
 and make a maker for wrong point
 developed by Ou Yang 9.11.2018*/



# include "main.h"
# include <vector>
# include "v3d_message.h"
# include <v3d_interface.h>
# include <iostream>
# include <algorithm>
# include <math.h>
# include <cmath>
# include <fstream>
# include "../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h"

using namespace std;

#define VOID 1000000000
#define PI 3.14159265359
#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define min(a,b) (a)<(b)?(a):(b)
#define angle(a,b,c) (acos((((b).x-(a).x)*((c).x-(a).x)+((b).y-(a).y)*((c).y-(a).y)+((b).z-(a).z)*((c).z-(a).z))/(dist(a,b)*dist(a,c)))*180.0/PI)


//calculate the distanceOfPointToLine
double DistanceOfPointToLine(QList<NeuronSWC> neuron,int a,int b,int s)

{
    double s_ab= pow(double((neuron.at(a).x - neuron.at(b).x)), 2.0) + pow(double((neuron.at(a).y - neuron.at(b).y)), 2.0) + pow(double((neuron.at(a).z - neuron.at(b).z)), 2.0);
    double ab=sqrt(s_ab);
    //double ab = sqrt(pow((neuron.at(a).x - neuron.at(b).x), 2.0) + pow((neuron.at(a).y - neuron.at(b).x), 2.0) + pow((neuron.at(a).z - neuron.at(b).z), 2.0));
    //double ab1=  sqrt((neuron.at(a).x - neuron.at(b).x)*(neuron.at(a).x - neuron.at(b).x)+(neuron.at(a).y - neuron.at(b).x)*(neuron.at(a).y - neuron.at(b).x)+(neuron.at(a).z - neuron.at(b).z)*(neuron.at(a).z - neuron.at(b).z));
    double s_as = pow(double((neuron.at(a).x - neuron.at(s).x)), 2.0) + pow(double((neuron.at(a).y - neuron.at(s).y)), 2.0) + pow(double((neuron.at(a).z - neuron.at(s).z)), 2.0);
    double as=sqrt(s_as);
    double s_bs=pow(double((neuron.at(s).x - neuron.at(b).x)), 2.0) + pow(double((neuron.at(s).y - neuron.at(b).y)), 2.0) + pow(double((neuron.at(s).z - neuron.at(b).z)), 2.0);
    double bs=sqrt(s_bs);
    //double bs = sqrt(pow((neuron.at(s).x - neuron.at(b).x), 2.0) + pow((neuron.at(s).y - neuron.at(b).y), 2.0) + pow((neuron.at(s).z - neuron.at(b).z), 2.0));

    double cos_A = (pow(as, 2.0) + pow(ab, 2.0) - pow(bs, 2.0)) / (2 * ab*as);

    double sin_A = sqrt(1 - pow(cos_A, 2.0));

    return as*sin_A;

}
// find the suspecious wrong type of neuron type
/*
bool detect_type_distance(V3DPluginCallback2 &callback, QWidget *parent)
  {
     NeuronTree tree1=callback.getSWCTeraFly();
     LandmarkList Markers = callback.getLandmarkTeraFly();
     QList<NeuronSWC> tree1swc=tree1.listNeuron;
     QList<NeuronSWC> tree2swc=tree1.listNeuron;
//calculate the kinds of different types
     vector<int> kinds;
     for (int i=0;i<tree1swc.size();i++)
     {
       kinds.push_back(tree1swc.at(i).type);
     }
     int max = *max_element(kinds.begin(),kinds.end());
     int ct=0;
     for (int i=0;i<max;i++)
     {
         for (int j=0;j<kinds.size();j++)
         {
             if(kinds.at(j)==i)
             {
                 ct=ct+1;
                 break;
             }
         }
     }
     cout<<"+++++++++++++++++++++++the kinds number of neuron type: "<<ct<<endl;

     vector<int> ids;
     for(V3DLONG i=0;i<tree1swc.size();i++)
      {
         ids.push_back(tree1swc.at(i).pn);
      }

//find the roots
     vector<double> roots;
     for (int i=0;i<tree1swc.size();i++)
      {
         int pt=tree1swc.at(i).pn;
         if(pt ==-1)
         {
             //vector<int>::iterator iter=find(ids.begin(), ids.end(), pt);
             //a=distance(ids.begin(),iter);
             roots.push_back(i);
         }
      }
     int numroot=roots.size();
     cout<<"++++++++++++++++++++++++++++++the roots number: "<<numroot<<endl;

//find the tips
    vector<double> tipslist;
    for (int i=0;i<tree1swc.size();i++)
    {
        int sum=0;
        for (int j=0;j<tree1swc.size();j++)
        {
            if (tree1swc.at(i).n==tree1swc.at(j).pn)
            {
                sum=sum+1;
            }
        }
        if (sum<1)
            {
                tipslist.push_back(i);
            }
     }
    int numtip=tipslist.size();
     cout<<"++++++++++++++++++++++++++++++the tips number: "<<numtip<<endl;
//get the childslist
     NeuronTree n_t;
     QHash <int, int> hash_nt;

     for(V3DLONG j=0; j<tree1swc.size();j++){
         hash_nt.insert(tree1swc[j].n, j);
     }
     n_t.listNeuron=tree1swc;
     n_t.hashNeuron=hash_nt;
     tree1=n_t;
     tree1swc=tree1.listNeuron;
     V3DLONG neuronNum = tree1.listNeuron.size();
     childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
     for (V3DLONG i=0;i<neuronNum;i++)
     {
         V3DLONG par = tree1.listNeuron[i].pn;
         if (par<0) continue;
         childs[tree1.hashNeuron.value(par)].push_back(i);
     }
     cout<<"++++++++++++++++++++++++++++++the childs number: "<<childs.size()<<endl;
//calculate the shortest distance between one root to another segment
     vector<int> suspoint;
     vector<double> edis;
     for (int i=0;i<roots.size();i++)
     {
         for (int j=0;j<roots.size();j++)
         {
           if (roots.at(i) != roots.at(j))
           {
            double angle1,angle2,disresult;
            segsinfo seg=getallchildsofseg(roots.at(j),suspoint);
            //vector<int> segnodes=seg.segpoint;
            int endpoint=seg.endnd;
            angle1=angle(tree1swc.at(roots.at(j)),tree1swc.at(roots.at(i)),tree1swc.at(endpoint));
            angle2=angle(tree1swc.at(endpoint),tree1swc.at(roots.at(j)),tree1swc.at(roots.at(i)));
            if(DistanceOfPointToLine(tree1swc,roots.at(j),endpoint,roots.at(i))==0 || angle1>=90 || angle2>=90)
            //if (angle1>=90 || angle2>=90)
               {
                  double dist1,dist2;
                  dist1=dist(tree1swc.at(roots.at(i)),tree1swc.at(roots.at(j)));
                  dist2=dist(tree1swc.at(roots.at(i)),tree1swc.at(endpoint));
                  disresult=min(dist1,dist2);
                  edis.push_back(disresult);
                  cout<<"++++++++++++++++++++++++++++++the dis of >90: "<<disresult<<endl;
                }
            else
               {
                  disresult=DistanceOfPointToLine(tree1swc,roots.at(j),endpoint,roots.at(i));
                  edis.push_back(disresult);
                  //cout<<"++++++++++++++++++++++++++++++the dis of <90: "<<disresult<<endl;
               }
           }
           else edis.push_back(100000000);
          }

         //cout<<"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<total dis number: "<<edis.size()<<endl;
         double mindis= *min_element(edis.begin(),edis.end());
         cout<<"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<min distance of: "<<i+1<<"---"<<mindis<<endl;
         vector<double>::iterator iter=find(edis.begin(), edis.end(),mindis);
         int a=distance(edis.begin(),iter);
         //cout<<"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<location of min value: "<<a<<endl;
         int type1=tree1swc.at(roots.at(i)).type;
         int type2=tree1swc.at(roots.at(a)).type;
         if(type1 != type2)
         suspoint.push_back(roots.at(i));
         edis.clear();
     }
//find the tip node of every corresponding root node
     vector<int> eend;
     for (int i=0;i<roots.size();i++)
     {
         segsinfo seg=getallchildsofseg(roots.at(i));
         vector<int> segnodes=seg.segpoint;
         int endpoint=segnodes.at((segnodes.size())-1);
         eend.push_back(endpoint);
     }

//calculate the shortest distance between one tip to another segment
     vector<double> edis1;
     for (int i=0;i<eend.size();i++)
     {
         for (int j=0;j<eend.size();j++)
         {        
           if (eend.at(i) != eend.at(j))
           {
            double angle1,angle2,disresult;
            angle1=angle(tree1swc.at(roots.at(j)),tree1swc.at(eend.at(j)),tree1swc.at(eend.at(i)));
            angle2=angle(tree1swc.at(eend.at(j)),tree1swc.at(roots.at(j)),tree1swc.at(eend.at(i)));
            if (DistanceOfPointToLine(tree1swc,roots.at(j),eend.at(j),eend.at(i))==0 || angle1>=90 || angle2>=90)
               {
                  double dist1,dist2;
                  dist1=dist(tree1swc.at(eend.at(i)),tree1swc.at(roots.at(j)));
                  dist2=dist(tree1swc.at(eend.at(i)),tree1swc.at(eend.at(j)));
                  disresult=min(dist1,dist2);
                  edis1.push_back(disresult);
                }
            else
               {
                  disresult=DistanceOfPointToLine(tree1swc,roots.at(j),eend.at(j),eend.at(i));
                  edis1.push_back(disresult);
               }
           }
           else edis1.push_back(100000000);
          }
         double mindis= *min_element(edis1.begin(),edis1.end());
         cout<<"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<min distance of: "<<i+1<<"---"<<mindis<<endl;
         vector<double>::iterator iter=find(edis1.begin(), edis1.end(),mindis);
         int a=distance(edis1.begin(),iter);
         int type1=tree1swc.at(eend.at(i)).type;
         int type2=tree1swc.at(roots.at(a)).type;
         if(type1 != type2)
         suspoint.push_back(eend.at(i));
         edis1.clear();
     }
 cout<<"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<total endpoint number: "<<eend.size()<<endl;
 cout<<"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<+++++++----------------------------------: "<<endl;
//make two vectors have the same length,no necessary!
    if (numroot > numtip){
        while (numtip!=numroot)
        {
           tipslist.push_back(VOID);
        }
    }
    else if (numroot < numtip){
        while (numtip!=numroot)
        {
           roots.push_back(VOID);
        }
    }
    cout<<"++++++++++++++++++++++++++++++the suspoint number: "<<suspoint.size()<<endl;
//set markers back to terafly
     LandmarkList result1;
     LocationSimple m;
     for(int i=0;i<suspoint.size();i++)
     {
             m.x=tree1swc.at(suspoint.at(i)).x;
             m.y=tree1swc.at(suspoint.at(i)).y;
             m.z=tree1swc.at(suspoint.at(i)).z;
             m.color.r = 255;
             m.color.g = 255;
             m.color.b = 255;
             result1.push_back(m);
           }
     for (int i=0;i<Markers.size();i++)
     {
             result1.push_back(Markers.at(i));
     }
     callback.setLandmarkTeraFly(result1);
     return 1;

  }*/

QVector<QVector<V3DLONG> > childs;
bool detect_type(V3DPluginCallback2 &callback, QWidget *parent)
{
    NeuronTree tree1=callback.getSWCTeraFly();
    LandmarkList Markers = callback.getLandmarkTeraFly();
    QList<NeuronSWC> ori_tree1swc=tree1.listNeuron;

    vector<int> suspoint_before;
    QList<int>suspoint_after;
    QList<int> plist;
    QList<int> alln;
    int N=ori_tree1swc.size();
    for(int i=0; i<N; i++){
        plist.append(ori_tree1swc.at(i).pn);
        alln.append(ori_tree1swc.at(i).n);
      }

//find the root of wrong type which is not 1,2,3 neither 4
    int numofwrongplace=0;
    int numofwrongtype=0;
    int numoflike_soma_situation=0;
    int numofduplicated=0;
    for (int i=0;i<ori_tree1swc.size();i++)
     {
        if(ori_tree1swc.at(i).type != 1 && ori_tree1swc.at(i).type != 2 && ori_tree1swc.at(i).type != 3 && ori_tree1swc.at(i).type != 4)
        {
            if(ori_tree1swc.at(i).pn == -1) continue;//{suspoint_before.push_back(i);numofwrongtype++;}
            else {
                   int step_index=i;
                   while(ori_tree1swc.at(step_index).pn !=-1){

                       int index_of_pn=alln.indexOf(ori_tree1swc.at(step_index).pn);
                       step_index=index_of_pn;
                   }
                   if (find(suspoint_before.begin(),suspoint_before.end(),step_index)==suspoint_before.end()) {suspoint_before.push_back(step_index);numofwrongtype++;}
                 }
        }
    }
    cout<<"------------------suspoint_before.size:"<<suspoint_before.size()<<endl;//2 wei
//find the dumplicated nodes which have diffrent types before sorting
    LandmarkList markerlist;
    vector<int> all_index;QList<int>allj;
        for (int i=0;i<ori_tree1swc.size();i++)
        {
            if (i!=ori_tree1swc.size()-1 && allj.count(i) == 0 )
            {
            for (int j=i+1;j<ori_tree1swc.size();j++)
                {
                    float x1,x2,y1,y2,z1,z2;
                    int type1,type2,pars1,pars2,n1,n2;
                    x1=ori_tree1swc.at(i).x;
                    x2=ori_tree1swc.at(j).x;
                    y1=ori_tree1swc.at(i).y;
                    y2=ori_tree1swc.at(j).y;
                    z1=ori_tree1swc.at(i).z;
                    z2=ori_tree1swc.at(j).z;
                    type1=ori_tree1swc.at(i).type;
                    type2=ori_tree1swc.at(j).type;
                    pars1=ori_tree1swc.at(i).pn;
                    pars2=ori_tree1swc.at(j).pn;
                    n1=ori_tree1swc.at(i).n;
                    n2=ori_tree1swc.at(j).n;
                    if(x1==x2 && y1==y2 && z1==z2 && type1 != type2)
                    {
                        all_index.push_back(i);
                        allj.push_back(j);
                        numofduplicated++;
                    }
                }
            }
        }
        suspoint_before.insert(suspoint_before.end(),all_index.begin(),all_index.end());
// delete duplicated nodes with the same cordinates
        LocationSimple temp;
        for(int i=0;i<suspoint_before.size();i++)
           {
               if(suspoint_before.at(i)!=VOID && i!=suspoint_before.size()-1){
               temp.x=ori_tree1swc.at(suspoint_before.at(i)).x;
               temp.y=ori_tree1swc.at(suspoint_before.at(i)).y;
               temp.z=ori_tree1swc.at(suspoint_before.at(i)).z;
               for(int j=i+1;j<suspoint_before.size();j++){

                  if(suspoint_before.at(j)!=VOID && temp.x == ori_tree1swc.at(suspoint_before.at(j)).x && temp.y == ori_tree1swc.at(suspoint_before.at(j)).y && temp.z==ori_tree1swc.at(suspoint_before.at(j)).z) {suspoint_before.at(j)=VOID;}

               }
             }
           }

        LocationSimple before_sort;
        for(int i=0;i<suspoint_before.size();i++)
           {
               if(suspoint_before.at(i)!=VOID){
               before_sort.x=ori_tree1swc.at(suspoint_before.at(i)).x;
               before_sort.y=ori_tree1swc.at(suspoint_before.at(i)).y;
               before_sort.z=ori_tree1swc.at(suspoint_before.at(i)).z;
               before_sort.color.r = 255;
               before_sort.color.g = 255;
               before_sort.color.b = 255;
               markerlist.push_back(before_sort);
             }
           }
           cout<<"------------------markerlist.size:"<<markerlist.size()<<endl;//2 wei

//sort swc
    QList<NeuronSWC> tree1swc;
    SortSWC(ori_tree1swc, tree1swc ,VOID, 0);

//get the childslist
         NeuronTree n_t;
         QHash <int, int> hash_nt ;
         for(V3DLONG j=0; j<tree1swc.size();j++){
             hash_nt.insert(tree1swc[j].n, j);
         }
         n_t.listNeuron=tree1swc;
         n_t.hashNeuron=hash_nt;

         V3DLONG neuronNum = n_t.listNeuron.size();
         childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
         for (V3DLONG i=0;i<neuronNum;i++)
         {
             V3DLONG par = n_t.listNeuron[i].pn;
             if (par<0) continue;
             childs[n_t.hashNeuron.value(par)].push_back(i);
         }

//find wrong types based on parent-child connection after sorting
         //bool stop_back=TRUE;
         for (int i=0;i<tree1swc.size();i++)
             {
                 if(childs[i].size()!=0)
                 {
                    for(int j=0;j<childs[i].size();j++)
                    {
                       if(tree1swc.at(i).type != tree1swc.at(childs[i].at(j)).type) {suspoint_after.push_back(i);numofwrongplace++;break;}
                    }
                 }
             }
cout<<"----------------suspoint_after.size:"<<suspoint_after.size()<<endl;//2 wei

//delete duplicated input markers and set all markers back to terafly

         QList<int> plist_sorted;
         QList<int> alln_sorted;
         int N_sorted=tree1swc.size();
         for(int i=0; i<N_sorted; i++){
             plist_sorted.append(tree1swc.at(i).pn);
             alln_sorted.append(tree1swc.at(i).n);
           }
         float x1,y1,z1,x1_pn,y1_pn,z1_pn;int curr_pn;

         if(suspoint_after.size()!=0)
         {

            for(int i=0;i<suspoint_after.size();i++)
            {
                if(suspoint_after.at(i)!=VOID){
                int contain=suspoint_after.at(i);
                x1=tree1swc.at(contain).x;
                y1=tree1swc.at(contain).y;
                z1=tree1swc.at(contain).z;
                bool not_root=FALSE;

                curr_pn=tree1swc.at(contain).pn;
                if (curr_pn!=-1){

                   x1_pn=tree1swc.at(alln_sorted.indexOf(curr_pn)).x;
                   y1_pn=tree1swc.at(alln_sorted.indexOf(curr_pn)).y;
                   z1_pn=tree1swc.at(alln_sorted.indexOf(curr_pn)).z;
                   not_root=TRUE;
                   int pn_index=alln_sorted.indexOf(curr_pn);
                   if(suspoint_after.count(pn_index) != 0) suspoint_after[pn_index]=VOID;
                }

                if(markerlist.size()!=0)
                {
                  for(int j=0;j<markerlist.size();j++){

                       bool check_pn=FALSE;bool check_curr=FALSE;
                       if(x1 == markerlist.at(j).x && y1 == markerlist.at(j).y && z1==markerlist.at(j).z) check_curr=TRUE;
                       if(not_root && x1_pn == markerlist.at(j).x && y1_pn == markerlist.at(j).y && z1_pn==markerlist.at(j).z) check_pn=TRUE;
                       if (check_curr || check_pn) {suspoint_after[i]=VOID;break;}
                   }

                   if(childs[contain].size()!=0){

                       for(int k=0;k<childs[contain].size();k++){

                           for(int j=0;j<markerlist.size();j++){

                               if(tree1swc.at(childs[contain].at(k)).x == markerlist.at(j).x  &&  tree1swc.at(childs[contain].at(k)).y == markerlist.at(j).y
                                  && tree1swc.at(childs[contain].at(k)).z==markerlist.at(j).z) {suspoint_after[i]=VOID;break;}
                    }
                   }
                  }
                }
               }
              }
            }

             LocationSimple m;
             if(suspoint_after.size()!=0){
               for(int i=0;i<suspoint_after.size();i++)
                {
                   if (suspoint_after[i]!=VOID){
                    m.x=tree1swc.at(suspoint_after.at(i)).x;
                    m.y=tree1swc.at(suspoint_after.at(i)).y;
                    m.z=tree1swc.at(suspoint_after.at(i)).z;
                    m.color.r = 255;
                    m.color.g = 255;
                    m.color.b = 255;
                    markerlist.push_back(m);
                 }
               }
             }
             if (markerlist.size()!=0)
             {
                 printf("You got [%d] white markers in your file.[%d] nodes are not type 1,2,3 or 4.[%d] nodes are with wrong type(still 1,2,3 or 4),please check!",markerlist.size(),numofwrongtype,numofwrongplace);
                 v3d_msg(QString("You got [%1] white marker in your file,please check!").arg(markerlist.size()));}
             else v3d_msg(QString("There are not any nodes you need to check!"));

              for (int i=0;i<Markers.size();i++) markerlist.push_back(Markers.at(i));

              callback.setLandmarkTeraFly(markerlist);

              return 1;

}
//calculate the kinds of different types
//    vector<int> kinds;
//    for (int i=0;i<tree1swc.size();i++)
//    {
//      kinds.push_back(tree1swc.at(i).type);
//    }
//    int max = *max_element(kinds.begin(),kinds.end());
//    int ct=0;
//    for (int i=0;i<max+1;i++)
//    {
//        for (int j=0;j<kinds.size();j++)
//        {
//            if(kinds.at(j)==i)
//            {
//                ct=ct+1;
//                break;
//            }
//        }
//    }
//    v3d_msg(QString("The kinds number of neuron type in your file: %1").arg(ct));

void detect_type_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    QStringList list=QString(infiles[0]).split("/");
    QString flag=list.last(); QStringList list1=flag.split(".");// you don't need to add 1 to find the string you want in input_dir
    QString flag1=list1.first();

    NeuronTree tree1=readSWC_file(QString(infiles[0]));
    QList<NeuronSWC> ori_tree1swc=tree1.listNeuron;
    vector<int> suspoint_before;
    QList<int> suspoint_after;
    QList<int> plist;
    QList<int> alln;
    int N=ori_tree1swc.size();
    for(int i=0; i<N; i++){
        plist.append(ori_tree1swc.at(i).pn);
        alln.append(ori_tree1swc.at(i).n);
      }
//find the root of wrong type which is not 1,2,3 neither 4
        int numofwrongplace=0;
        int numofwrongtype=0;
        int numoflike_soma_situation=0;
        int numofduplicated=0;
        for (int i=0;i<ori_tree1swc.size();i++)
         {
            if(ori_tree1swc.at(i).type != 1 && ori_tree1swc.at(i).type != 2 && ori_tree1swc.at(i).type != 3 && ori_tree1swc.at(i).type != 4)
            {
                if(ori_tree1swc.at(i).pn == -1) continue;//{suspoint_before.push_back(i);numofwrongtype++;}
                else {
                       int step_index=i;
                       while(ori_tree1swc.at(step_index).pn !=-1){

                           int index_of_pn=alln.indexOf(ori_tree1swc.at(step_index).pn);
                           step_index=index_of_pn;
                       }
                       if (find(suspoint_before.begin(),suspoint_before.end(),step_index)==suspoint_before.end()) {suspoint_before.push_back(step_index);numofwrongtype++;}
                     }
            }
        }
        cout<<"------------------suspoint_before.size:"<<suspoint_before.size()<<endl;//2 wei
//find the dumplicated nodes which have diffrent types before sorting
        LandmarkList markerlist;
        vector<int> all_index;QList<int>allj;
            for (int i=0;i<ori_tree1swc.size();i++)
            {
                if (i!=ori_tree1swc.size()-1 && allj.count(i) == 0 )
                {
                for (int j=i+1;j<ori_tree1swc.size();j++)
                    {
                        float x1,x2,y1,y2,z1,z2;
                        int type1,type2,pars1,pars2,n1,n2;
                        x1=ori_tree1swc.at(i).x;
                        x2=ori_tree1swc.at(j).x;
                        y1=ori_tree1swc.at(i).y;
                        y2=ori_tree1swc.at(j).y;
                        z1=ori_tree1swc.at(i).z;
                        z2=ori_tree1swc.at(j).z;
                        type1=ori_tree1swc.at(i).type;
                        type2=ori_tree1swc.at(j).type;
                        pars1=ori_tree1swc.at(i).pn;
                        pars2=ori_tree1swc.at(j).pn;
                        n1=ori_tree1swc.at(i).n;
                        n2=ori_tree1swc.at(j).n;
                        if(x1==x2 && y1==y2 && z1==z2 && type1 != type2)
                        {
                            all_index.push_back(i);
                            allj.push_back(j);
                            numofduplicated++;
                        }
                    }
                }
            }
            suspoint_before.insert(suspoint_before.end(),all_index.begin(),all_index.end());
                    //sort(all_index.begin(),all_index.end());
                    //all_index.erase(unique(all_index.begin(),all_index.end()),all_index.end());//delete duplicated index

    //              //for (int i=0;i<all_index.size();i++){sametype.remove(sametype.value(all_index.at(i)));}//delete duplicated lines in hash table

    //                QHash<int,int>::const_iterator iter1=sametype.constBegin();//find all indexs of duplicated nodes in hash table
    //                while(iter1 != sametype.constEnd()){

    //                    all_duplicated_index.push_back(iter1.key());
    //                    ++iter1;
    //                }
    //                sort(all_duplicated_index.begin(),all_duplicated_index.end());
    //                all_duplicated_index.erase(unique(all_duplicated_index.begin(),all_duplicated_index.end()),all_duplicated_index.end());//delete duplicated index
            // delete duplicated nodes with the same cordinates
                    LocationSimple temp;
                    for(int i=0;i<suspoint_before.size();i++)
                       {
                           if(suspoint_before.at(i)!=VOID && i!=suspoint_before.size()-1){
                           temp.x=ori_tree1swc.at(suspoint_before.at(i)).x;
                           temp.y=ori_tree1swc.at(suspoint_before.at(i)).y;
                           temp.z=ori_tree1swc.at(suspoint_before.at(i)).z;
                           for(int j=i+1;j<suspoint_before.size();j++){

                              if(suspoint_before.at(j)!=VOID && temp.x == ori_tree1swc.at(suspoint_before.at(j)).x && temp.y == ori_tree1swc.at(suspoint_before.at(j)).y && temp.z==ori_tree1swc.at(suspoint_before.at(j)).z) {suspoint_before.at(j)=VOID;}

                           }
                         }
                       }

                    LocationSimple before_sort;
                    for(int i=0;i<suspoint_before.size();i++)
                       {
                           if(suspoint_before.at(i)!=VOID){
                           before_sort.x=ori_tree1swc.at(suspoint_before.at(i)).x;
                           before_sort.y=ori_tree1swc.at(suspoint_before.at(i)).y;
                           before_sort.z=ori_tree1swc.at(suspoint_before.at(i)).z;
                           before_sort.color.r = 255;
                           before_sort.color.g = 255;
                           before_sort.color.b = 255;
                           markerlist.push_back(before_sort);
                         }
                       }
                            cout<<"------------------markerlist.size:"<<markerlist.size()<<endl;//2 wei

//sort swc
                QList<NeuronSWC> tree1swc;
                SortSWC(ori_tree1swc, tree1swc ,VOID, 0);

//get the childslist
                     NeuronTree n_t;
                     QHash <int, int> hash_nt ;
                     for(V3DLONG j=0; j<tree1swc.size();j++){
                         hash_nt.insert(tree1swc[j].n, j);
                     }
                     n_t.listNeuron=tree1swc;
                     n_t.hashNeuron=hash_nt;

                     V3DLONG neuronNum = n_t.listNeuron.size();
                     childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
                     for (V3DLONG i=0;i<neuronNum;i++)
                     {
                         V3DLONG par = n_t.listNeuron[i].pn;
                         if (par<0) continue;
                         childs[n_t.hashNeuron.value(par)].push_back(i);
                     }

//find wrong types based on parent-child connection after sorting
                     //bool stop_back=TRUE;
                     for (int i=0;i<tree1swc.size();i++)
                         {
                             if(childs[i].size()!=0)
                             {
                                for(int j=0;j<childs[i].size();j++)
                                {
                                   if(tree1swc.at(i).type != tree1swc.at(childs[i].at(j)).type) {suspoint_after.push_back(i);numofwrongplace++;break;}
                                }
                             }
                         }
            cout<<"----------------suspoint_after.size:"<<suspoint_after.size()<<endl;//2 wei

//delete duplicated input markers and set all markers back to terafly

                     QList<int> plist_sorted;
                     QList<int> alln_sorted;
                     int N_sorted=tree1swc.size();
                     for(int i=0; i<N_sorted; i++){
                         plist_sorted.append(tree1swc.at(i).pn);
                         alln_sorted.append(tree1swc.at(i).n);
                       }
                     float x1,y1,z1,x1_pn,y1_pn,z1_pn;int curr_pn;

                     if(suspoint_after.size()!=0)
                     {

                        for(int i=0;i<suspoint_after.size();i++)
                        {
                            if(suspoint_after.at(i)!=VOID){
                            int contain=suspoint_after.at(i);
                            x1=tree1swc.at(contain).x;
                            y1=tree1swc.at(contain).y;
                            z1=tree1swc.at(contain).z;
                            bool not_root=FALSE;

                            curr_pn=tree1swc.at(contain).pn;
                            if (curr_pn!=-1){

                               x1_pn=tree1swc.at(alln_sorted.indexOf(curr_pn)).x;
                               y1_pn=tree1swc.at(alln_sorted.indexOf(curr_pn)).y;
                               z1_pn=tree1swc.at(alln_sorted.indexOf(curr_pn)).z;
                               not_root=TRUE;
                               int pn_index=alln_sorted.indexOf(curr_pn);
                               if(suspoint_after.count(pn_index) != 0) suspoint_after[pn_index]=VOID;
                            }

                            if(markerlist.size()!=0)
                            {
                              for(int j=0;j<markerlist.size();j++){

                                   bool check_pn=FALSE;bool check_curr=FALSE;
                                   if(x1 == markerlist.at(j).x && y1 == markerlist.at(j).y && z1==markerlist.at(j).z) check_curr=TRUE;
                                   if(not_root && x1_pn == markerlist.at(j).x && y1_pn == markerlist.at(j).y && z1_pn==markerlist.at(j).z) check_pn=TRUE;
                                   if (check_curr || check_pn) {suspoint_after[i]=VOID;break;}
                               }

                               if(childs[contain].size()!=0){

                                   for(int k=0;k<childs[contain].size();k++){

                                       for(int j=0;j<markerlist.size();j++){

                                           if(tree1swc.at(childs[contain].at(k)).x == markerlist.at(j).x  &&  tree1swc.at(childs[contain].at(k)).y == markerlist.at(j).y
                                              && tree1swc.at(childs[contain].at(k)).z==markerlist.at(j).z) {suspoint_after[i]=VOID;break;}
                                }
                               }
                              }
                            }
                           }
                          }
                        }

                         LocationSimple m;
                         if(suspoint_after.size()!=0){
                           for(int i=0;i<suspoint_after.size();i++)
                            {
                               if (suspoint_after[i]!=VOID){
                                m.x=tree1swc.at(suspoint_after.at(i)).x;
                                m.y=tree1swc.at(suspoint_after.at(i)).y;
                                m.z=tree1swc.at(suspoint_after.at(i)).z;
                                m.color.r = 255;
                                m.color.g = 255;
                                m.color.b = 255;
                                markerlist.push_back(m);
                             }
                           }
                         }

//record result
    FILE * fp=0;
    QString out_result =QString(outfiles.at(0))+"/"+flag1;
    fp = fopen((char *)qPrintable(out_result+QString(".txt")), "wt");
    qDebug("--------------------------------txt output dir:%s",qPrintable(out_result));
    //int numofdup=floor(numofduplicated/2);
    fprintf(fp, "1.Name of swc 2.Number of wrong type 3.Number of wrong type based on P-C(still 1,2,3 or 4) 4.Total number\n") ;
    fprintf(fp,"%s %d %d %d %d",flag1.toStdString().c_str(),numofwrongtype,numofwrongplace,markerlist.size());
    fclose(fp);

}


//vector<int> ids;
//for(V3DLONG i=0;i<tree1swc.size();i++)
 //{
  //  ids.push_back(tree1swc.at(i).n);
 //}
//find the roots
     /*vector<double> roots;
     for (int i=0;i<tree1swc.size();i++)
      {
         int pt=tree1swc.at(i).pn;
         if(pt ==-1)
         {
             //vector<int>::iterator iter=find(ids.begin(), ids.end(), pt);
             //a=distance(ids.begin(),iter);
             roots.push_back(i);
         }
      }
     int numroot=roots.size();
     cout<<"++++++++++++++++++++++++++++++the roots number: "<<numroot<<endl;*/
//find the tips
//                 vector<int> tipslist;
//                 for (int i=0;i<tree1swc.size();i++)
//                 {
//                     int sum=0;
//                     for (int j=0;j<tree1swc.size();j++)
//                     {
//                         if (tree1swc.at(i).n==tree1swc.at(j).pn)
//                         {
//                             sum=sum+1;
//                         }
//                     }
//                     if (sum<1)
//                         {
//                             tipslist.push_back(i);
//                         }
//                  }
             //cout<<"++++++++++++++++++++++++++++++the roots number: "<<tipslist.size()<<endl;
//                cout<<"++++++++++++++++++++++++++++++the suspoint number: "<<suspoint.size()<<endl;
//                cout<<"++++++++++++++++++++++++++++++the suspointroot1 number: "<<suspointroot.size()<<endl;
//                cout<<"++++++++++++++++++++++++++++++the suspointnotroot number: "<<suspointnotroot.size()<<endl;
//                cout<<"++++++++++++++++++++++++++++++the coornotroot number: "<<coornotroot.size()<<endl;
//                cout<<"++++++++++++++++++++++++++++++the tworoot number: "<<tworoot.size()<<endl;
//                cout<<"++++++++++++++++++++++++++++++the allnotroot number: "<<allnotroot1.size()<<endl;
//                cout<<"++++++++++++++++++++++++++++++the sametyperoot number: "<<sametyperoot.size()<<endl;
//                cout<<"++++++++++++++++++++++++++++++the samecoornotroot number: "<<samecoornotroot.size()<<endl;

//calculate if the type of root node is 2 or 3,if not,change all the child types of this root to 2 or 3
      //QList<NeuronTree> alltypetree;
//    for (int i=0;i<allnotroot1.size();i++)
//    {
//        if(find(tipslist.begin(), tipslist.end(), allnotroot1.at(i)) == tipslist.end()) suspointroot.push_back(allnotroot1.at(i));
//    }
//    cout<<"++++++++++++++++++++++++++++++the suspointroot2 number: "<<suspointroot.size()<<endl;
//    for (int i=0;i<suspointroot.size();i++)
//    {
//        QList<NeuronTree> result;
//        int root=suspointroot.at(i);
//        int resulttype=tree1swc.at(coornotroot.at(i)).type;
//        result=find_wrong_type_ending(tree1swc,root,resulttype,sametyperoot,samecoornotroot,suspoint,allduplnodes,totalduplnodes);
//        for (int j=0;j<result.size();j++)
//        {
//          alltypetree.push_back(result.at(j));
//        }
//        cout<<"++++++++++++++++++++++++++++++TTTTTTTTTTTTTTTTTTTTTTTTTTTT++++++++++++++++++++++++++++"<<endl;
//    }

//case 0:wrong type isn't 2 or 3,but the others,and this two dumplicated nodes are all root nodes
//    for(int i=0;i<tworoot.size();i++)
//    {
//        if (tree1swc.at(tworoot.at(i)).type!=2 && tree1swc.at(tworoot.at(i)).type!=3)
//          {
//            QList<NeuronSWC> result;
//            result.clear();
//            int rootpoint=tworoot.at(i);
//            int resulttype=tree1swc.at(twocorroot.at(i)).type;
//            vector<int> allchilds;
//            segsinfo seg=getallchildsofseg(rootpoint);
//            allchilds=seg.segpoint;
//            result=change_type_of_seg(tree1swc,allchilds,resulttype);
//                            NeuronTree typetree;
//                            typetree.listNeuron=result;
//                            QHash <int, int> hash1_nt;
//                            for(V3DLONG j=0; j<result.size();j++)
//                            {
//                                hash1_nt.insert(result[j].n, j);
//                            }
//                            typetree.hashNeuron=hash1_nt;
//                            alltypetree.push_back(typetree);
//                            cout<<"++++++++++++++++++++++++++++++pppppppppppppppppppppppp+++++++++++++++++++++++++++"<<endl;
//          }
//    }
    //for (int i=0;i<suspointroot.size();i++)
    //{
//case 1:wrong type isn't 2 or 3,but the others
//        if (tree1swc.at(suspointroot.at(i)).type!=2 && tree1swc.at(suspointroot.at(i)).type!=3)
//          {
//            QList<NeuronSWC> result;
//            result.clear();
//            int rootpoint=suspointroot.at(i);
//            int resulttype=tree1swc.at(coornotroot.at(i)).type;
//            vector<int> allchilds;
//            segsinfo seg=getallchildsofseg(rootpoint);
//            allchilds=seg.segpoint;
//            result=change_type_of_seg(tree1swc,allchilds,resulttype);
//                            NeuronTree typetree;
//                            typetree.listNeuron=result;
//                            QHash <int, int> hash1_nt;
//                            for(V3DLONG j=0; j<result.size();j++)
//                            {
//                                hash1_nt.insert(result[j].n, j);
//                            }
//                            typetree.hashNeuron=hash1_nt;
//                            alltypetree.push_back(typetree);
//                            cout<<"++++++++++++++++++++++++++++++pppppppppppppppppppppppp+++++++++++++++++++++++++++"<<endl;
//          }
//case 2:wrong type is 2 or 3 and this wrong segment is in the middle of two correct segments,this is a more tricky situation.
            //QList<NeuronSWC> result;
            //vector<int> allchilds;
            //result.clear();
            //allchilds.clear();
            //int rootpoint=suspointroot.at(i);
            //int endpoint;
            //segsinfo seg=getallchildsofseg(rootpoint);
            //endpoint=seg.endnd;
            //allchilds=seg.segpoint;
//            if(find(suspoint.begin(), suspoint.end(), endpoint) != suspoint.end())
//            {
//                int roottype=tree1swc.at(rootpoint).type;
//                int rootcoortype=tree1swc.at(coornotroot.at(i)).type;
//                int endtype=tree1swc.at(endpoint).type;
//                result=change_type_of_seg(tree1swc,allchilds,rootcoortype);
//                            NeuronTree typetree;
//                            typetree.listNeuron=result;
//                            QHash <int, int> hash1_nt;
//                            for(V3DLONG j=0; j<result.size();j++)
//                            {
//                                hash1_nt.insert(result[j].n, j);
//                            }
//                            typetree.hashNeuron=hash1_nt;
//                            alltypetree.push_back(typetree);
//                            cout<<"++++++++++++++++++++++++++++++oooooooooooooooooooo+++++++++++++++++++++++++++"<<endl;
//            }
//case 3:wrong type is 2 or 3 and this wrong segment is in the end of one correct segment.
            //double dis_root2tip;
            //dis_root2tip=dist(tree1swc.at(rootpoint),tree1swc.at(endpoint));
            //if(dis_root2tip<20)
            //if(find(suspoint.begin(), suspoint.end(), endpoint) == suspoint.end())
            //{
                //int rootcoortype=tree1swc.at(coornotroot.at(i)).type;
                //result=change_type_of_seg(tree1swc,allchilds,rootcoortype);
                            //NeuronTree typetree;
                            //typetree.listNeuron=result;
                            //QHash <int, int> hash1_nt;
                            //for(V3DLONG j=0; j<result.size();j++)
                            //{
                             //   hash1_nt.insert(result[j].n, j);
                            //}
                            //typetree.hashNeuron=hash1_nt;
                            //alltypetree.push_back(typetree);
            //}
   //}
    //cout<<"++++++++++++++++++++++++++++++--------------------------------222---------------------------------"<<endl;
//cout<<"++++++++++++++++++++++++++++++alltypetree size---------------------------------"<<alltypetree.size()<<endl;

//set swc back to terafly
//            vector<int> worngids;
//            QList <NeuronSWC> listNeuron;
//            for (int i=0;i<alltypetree.size();i++)
//            {
//                for (int j=0;j<alltypetree.at(i).listNeuron.size();j++)
//                   {
//                     listNeuron.append(alltypetree.at(i).listNeuron.at(j));
//                     worngids.push_back(alltypetree.at(i).listNeuron.at(j).n);
//                   }
//            }

//            for (int i=0;i<tree1swc.size();i++)
//            {
//                if (find(worngids.begin(), worngids.end(), tree1swc.at(i).n) == worngids.end())
//                    {
//                      listNeuron.append(tree1swc.at(i));
//                    }
//            }
//            cout<<"++++++++++++++++++++++++++++++--------------------------------333---------------------------------"<<endl;
           /* for (int i=0;i<worngids.size();i++)
            {
                vector<int>::iterator iter=find(ids.begin(), ids.end(), worngids.at(i));
                int a=distance(ids.begin(),iter);
                tree1swc.removeAt(a);
                //tree1swc.append(tree2swc.at(worngids.at(i)));
            }*/

//            cout<<"++++++++++++++++++++++++++++++the nodes number of new swc----------------------------------"<<listNeuron.size()<<endl;
//            QHash <int, int>  hashNeuron;
//            for(V3DLONG j=0; j<listNeuron.size();j++)
//            {
//               hashNeuron.insert(listNeuron[j].n, j);
//            }
//            NeuronTree resultTree;
//            resultTree.listNeuron=listNeuron;
//            resultTree.hashNeuron=hashNeuron;
//            cout<<"++++++++++++++++++++++++++++++--------------------------------666---------------------------------"<<endl;
//            callback.setSWCTeraFly(resultTree);
//            cout<<"++++++++++++++++++++++++++++++--------------------------------444--------------------------------"<<endl;
//set markers back to terafly
//            vector<int> sus_after;
//            int cc=0;
//            for (int i=0;i<listNeuron.size();i++)
//            {
//                if(listNeuron.at(i).type != 2 && listNeuron.at(i).type != 3) sus_after.push_back(i);
//                for (int j=0;j<listNeuron.size();j++)
//                {
//                    if (i != j)
//                    {
//                        int x11,x12,y11,y12,z11,z12,type11,type12,pars11,pars12,n11,n12;
//                        x11=listNeuron.at(i).x;
//                        x12=listNeuron.at(j).x;
//                        y11=listNeuron.at(i).y;
//                        y12=listNeuron.at(j).y;
//                        z11=listNeuron.at(i).z;
//                        z12=listNeuron.at(j).z;
//                        type11=listNeuron.at(i).type;
//                        type12=listNeuron.at(j).type;
//                        pars11=listNeuron.at(i).pn;
//                        pars12=listNeuron.at(j).pn;
//                        n11=listNeuron.at(i).n;
//                        n12=listNeuron.at(j).n;

//                        if(x11==x12 && y11==y12 && z11==z12 && type11!=type12)
//                        {
//                            sus_after.push_back(i);
//                            cc++;
//                        }
//                    }
//                }
//            }


//calculate the kinds of different types
//            vector<int> kinds1;
//            for (int i=0;i<listNeuron.size();i++)
//            {
//                kinds1.push_back(listNeuron.at(i).type);
//            }
//            int max1 = *max_element(kinds1.begin(),kinds1.end());
//            int ct1=0;
//            for (int i=0;i<max1+1;i++)
//            {
//               for (int j=0;j<kinds1.size();j++)
//               {
//                  if(kinds1.at(j)==i)
//                  {
//                     ct1=ct1+1;
//                     break;
//                  }
//               }
//             }
//           // v3d_msg("The kinds number of neuron type in your file: "+ct1);
//            //cout<<"+++++++++++++++++++++++the kinds number of neuron type: "<<ct1<<endl;

//QList<NeuronSWC> change_type_of_seg(QList<NeuronSWC> &neurons,vector<int> &allchilds,int resulttype)
//{
//    QList<NeuronSWC> result;
//    for (int i=0;i<allchilds.size();i++)
//    {
//        NeuronSWC s;
//        s.x=neurons.at(allchilds.at(i)).x;
//        s.y=neurons.at(allchilds.at(i)).y;
//        s.z=neurons.at(allchilds.at(i)).z;
//        s.type=resulttype;
//        s.radius=neurons.at(allchilds.at(i)).radius;
//        s.pn=neurons.at(allchilds.at(i)).pn;
//        s.n=neurons.at(allchilds.at(i)).n;
//        result.append(s);
//    }
//    return result;
//}
//collect all childs of one single segment
/*struct segsinfo getallchildsofseg(int t,vector<int> & allduplnodes)
        {
            struct segsinfo result;
            vector<int> rchildlist;
            rchildlist.clear();
            V3DLONG tmp;
            V3DLONG cc=1;
            vector<int> endpoints;
            if(childs[t].size()>0)
            {
              tmp = childs[t].at(0);
              if ((find(allduplnodes.begin(), allduplnodes.end(), tmp) != allduplnodes.end())) endpoints.push_back(tmp);
              rchildlist.push_back(tmp);
              while (childs[tmp].size()==1)
               {
                 tmp = childs[tmp].at(0);
                 cc+=1;
                 rchildlist.push_back(tmp);
                 if ((find(allduplnodes.begin(), allduplnodes.end(), tmp) != allduplnodes.end())) endpoints.push_back(tmp);//allduplnodes are dumplicated nodes who have the same types
               }
               endpoints.push_back(rchildlist.at(rchildlist.size()-1));
               result.segpoint=rchildlist;
               result.aa=cc;
               result.endnd=endpoints;//including both duplicated nodes and real end point
               return result;
            }
        }*/
//struct segsinfo getallchildsofseg(int t)
//        {
//            struct segsinfo result;
//            vector<int> rchildlist;
//            rchildlist.clear();
//           //rchildlist.push_back(t);
//            int tmp;
//            int cc=1;
//            int endpoints;
//            if(childs[t].size()>0)
//            {
//              //tmp = childs[t].at(0);
//              tmp=t;
//              rchildlist.push_back(tmp);
//              while (childs[tmp].size()==1)
//               {
//                 tmp = childs[tmp].at(0);
//                 cc+=1;
//                 rchildlist.push_back(tmp);

//               }
//               //endpoints=rchildlist.at(rchildlist.size()-1);
//               endpoints=rchildlist.at(cc-1);
//               result.segpoint=rchildlist;
//               result.aa=cc;
//               result.endnd=endpoints;

//            }
//            else {

//                rchildlist.push_back(t);
//                result.segpoint=rchildlist;
//                result.aa=0;
//                result.endnd=t;
//            }
//            return result;
//        }
/*QList<NeuronTree> find_wrong_type_ending(QList<NeuronSWC> &neurons,int root,int resulttype,vector<int> & sametyperoot,
                                         vector<int> & samecoornotroot,vector<int> & suspoint,vector<int> & allduplnodes,vector<int> & totalduplnodes)
{
      vector<int> allwrongchilds,allwrongroots,endpoint;
      QList<NeuronTree>typetreetotal;
      NeuronTree typetree;
      segsinfo seg1=getallchildsofseg(root);
      cout<<"+++++++++++++++++++++++++++++ffffffffffffffffffffffffffffff+++++++++++++++++++++++++++"<<endl;
      allwrongroots.push_back(root);
      endpoint=seg1.endnd;
      cout<<"++++++++++++++++++++++++++++++ endpoint++++++++++++++++++++++++++++"<<endpoint.size()<<endl;
      if((find(totalduplnodes.begin(), totalduplnodes.end(), endpoint.at(0)) == totalduplnodes.end()) || (find(suspoint.begin(), suspoint.end(), endpoint.at(0)) != suspoint.end()))
          //if this first endpoint is a tip node or a suspicious node(two nodes which have different types )
          {
             cout<<"++++++++++++++++++++++++++++kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk++++++++++++++++++++++++++++"<<endl;
             segsinfo seg=getallchildsofseg(allwrongroots.at(0));
             vector<int> allchilds;
             allchilds=seg.segpoint;
             allwrongchilds.insert(allwrongchilds.end(), allchilds.begin(), allchilds.end());
             //allwrongchilds.insert(allwrongchilds.end(), endpoints.begin(), endpoints.end());
             QList<NeuronSWC> result;
             result=change_type_of_seg(neurons,allwrongchilds,resulttype);
             typetree.listNeuron=result;
             QHash <int, int> hash1_nt;
             for(V3DLONG j=0; j<result.size();j++)
                {
                   hash1_nt.insert(result[j].n, j);
                }
             typetree.hashNeuron=hash1_nt;
             typetreetotal.push_back(typetree);
             //cc+=1;
          }

      else if((find(allduplnodes.begin(), allduplnodes.end(), endpoint.at(0)) != allduplnodes.end())) //all complicated nodes who have the same type
     {
       for (int cc1=0;cc1<endpoint.size();cc1++)
        {
          int root1;
          vector<int>::iterator iter=find(samecoornotroot.begin(),samecoornotroot.end(), endpoint.at(cc1));
          int a=distance(samecoornotroot.begin(),iter);
          root1=sametyperoot.at(a);
          //allwrongroots.push_back(root1);
          for (int c1=0;c1<5;c1++)
         {
           vector<int> endpoints;
           cout<<"++++++++++++++++++++++++++++++llllllllllllllllllllllllllllllllllllllllll++++++++++++++++++++++++++++"<<endl;
           segsinfo seg=getallchildsofseg(root1);
           cout<<"+++++++++++++++++++++++++++++ffffffffffffffffffffffffffffff+++++++++++++++++++++++++++"<<endl;
           allwrongroots.push_back(root1);
           allwrongchilds.push_back(root1);
           endpoints=seg.endnd;
           cout<<"++++++++++++++++++++++++++++++qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq++++++++++++++++++++++++++++"<<endl;
            if ((find(suspoint.begin(), suspoint.end(), endpoints.at(0)) == suspoint.end()) && (find(samecoornotroot.begin(), samecoornotroot.end(), endpoints.at(0)) != samecoornotroot.end()))
                // judge if this endpoint is the same type duplicated node
              {
                   cout<<"++++++++++++++++++++++++++++LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL++++++++++++++++++++++++++++"<<endl;
                   vector<int>::iterator iter=find(samecoornotroot.begin(),samecoornotroot.end(), endpoints.at(0));
                   int a=distance(samecoornotroot.begin(),iter);
                   root1=sametyperoot.at(a);
                   //cc+=1;
                   continue;
              }
            if ((find(suspoint.begin(), suspoint.end(), endpoints.at(0)) != suspoint.end()) || (find(totalduplnodes.begin(), totalduplnodes.end(), endpoints.at(0)) == totalduplnodes.end()))
              {
                 cout<<"+++++++++++++++++++++++++++vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv++++++++++++++++++++++++++++"<<endl;
                 for (int i=0;i<allwrongroots.size();i++)
                    {
                       segsinfo seg=getallchildsofseg(allwrongroots.at(i));
                       vector<int> allchilds;
                       allchilds=seg.segpoint;
                       allwrongchilds.insert(allwrongchilds.end(), allchilds.begin(), allchilds.end());
                    }
                 QList<NeuronSWC> result;
                 result=change_type_of_seg(neurons,allwrongchilds,resulttype);
                 typetree.listNeuron=result;
                 QHash <int, int> hash1_nt;
                 for(V3DLONG j=0; j<result.size();j++)
                    {
                       hash1_nt.insert(result[j].n, j);
                    }
                 typetree.hashNeuron=hash1_nt;
                 typetreetotal.push_back(typetree);
                 break;
              }
            }
          }
      }
      return typetreetotal;
}



//delete one suspoint node of two nodes who have very short distance
/*vector<int> deletesus(QList<NeuronSWC> &neurons,vector<int> &points,int ofone)
{
    vector<int> result=points;
    int num=result.size();
    if (num<=0 || ofone<0)
    {
        v3d_msg("current neuron have no points needed to be deleted.");
    }
    else
    {
        for (int j=0;j<num;j++)
        {
            if (ofone != j)
            {
                double dist1;
                dist1=dist(neurons.at(result.at(ofone)),neurons.at(result.at(j)));
                if (dist1<3)
                {
                    vector<int>::iterator iter=find(result.begin(), result.end(), result.at(j));
                    result.erase(iter);
                    cout<<"++++++++++++++++++++++++++++++test++++++++++++++++++++++ "<<endl;
                }
            }
        }
    }
    return result;
}


//find the tips
        /*vector<double> tipslist;
        for (int i=0;i<tree1swc.size();i++)
        {
            int sum=0;
            for (int j=0;j<tree1swc.size();j++)
            {
                if (tree1swc.at(i).n==tree1swc.at(j).pn)
                {
                    sum=sum+1;
                }
            }
            if (sum<1)
                {
                    tipslist.push_back(i);
                }
         }
        int numtip=tipslist.size();*/
       // cout<<"++++++++++++++++++++++++++++++the tips number: "<<numroot<<endl;
    /* vector<vector<int> > boom(numroot,vector<int>());
     for (int i=0;i<numroot;i++)
     {
         segsinfo seg=getallchildsofseg(roots.at(i));
         vector<int> segnodes=seg.segpoint;
         //int number=seg.aa;
         for(int k=0;k<segnodes.size();k++)
             {
                 for(int j=0;j<tree1swc.size();j++)
                 {
                   if (segnodes.at(k) != j)
                   {
                    dist(tree1swc.at(roots.at(i)),tree1swc.at(j))
                   }

                 }
             }
     }

//calculate the shortest distance between one root to another root or tip
     vector<double> pro;
     vector<int> suspoint;
     //vector<vector<double> > pro(num,vector<double>());
     for(int j=0;j<numroot;j++)
     {
         vector<double> eroot,eroot2;
         for(int i=0;i<numroot;i++)
           {
             //if (roots.at(j) != i)
             double dis1,dis2;
             dis1=dist(tree1swc.at(roots.at(j)),tree1swc.at(roots.at(i)));
             dis2=dist(tree1swc.at(roots.at(j)),tree1swc.at(tipslist.at(i)));
             eroot.push_back(dis1);
             eroot2.push_back(dis2);
           }
         vector<double> eroot1=eroot;
         vector<double>::iterator iter=eroot.begin();
         while(iter != eroot.end()){
             if (*iter == 0)
             {
               iter=eroot.erase(iter);
             }
             else  {++iter;}
         }
         double min1= *min_element(eroot.begin(),eroot.end());
         double min2 = *min_element(eroot2.begin(),eroot2.end());
         double minre=min(min1,min2);

         if (minre==min1)
         {
            vector<double>::iterator iter1=find(eroot1.begin(), eroot1.end(), min1);
            int a=distance(eroot1.begin(),iter1);
            if(tree1swc.at(roots.at(j)).type != tree1swc.at(roots.at(a)).type){
              suspoint.push_back(roots.at(j));
            }
         }
         else if (minre==min2)
         {
            vector<double>::iterator iter1=find(eroot2.begin(), eroot2.end(), min2);
            int a=distance(eroot2.begin(),iter1);
            if(tree1swc.at(tipslist.at(j)).type != tree1swc.at(tipslist.at(a)).type){
              suspoint.push_back(tipslist.at(j));
            }
         }
     }
     cout<<"================the suspoints number: "<<suspoint.size()<<endl;*/


     /*for(int i;i<roots.size();i++)
     {
         int type1=tree1swc.at(roots.at(i)).type;
         int type2=tree1swc.at(pro.at(i)).type;
         if (type1 != type2) suspoint.push_back(roots.at(i));
     }
     V3DLONG rootid=VOID;
     V3DLONG thres=100;
     QList<NeuronSWC> result;
     QList<NeuronSWC> neuron_nt;
     neuron_nt.clear();
     if (SortSWC(tree1swc, result ,rootid, thres))
     {
         neuron_nt=result;
     }
     tree1swc=neuron_nt;
     NeuronTree n_t;
     QHash <int, int> hash_nt;

     for(V3DLONG j=0; j<tree1swc.size();j++){
         hash_nt.insert(tree1swc[j].n, j);
     }
     n_t.listNeuron=tree1swc;
     n_t.hashNeuron=hash_nt;
     tree1=n_t;

     vector<int> ids;
     // Reorder tree ids so that neuron.at(i).n=i+1
     for(V3DLONG i=0;i<tree1swc.size();i++)
     {
         ids.push_back(tree1swc.at(i).n);
     }
     for(V3DLONG i=0;i<tree1swc.size();i++)
     {
         tree1swc[i].n=i+1;
         if(tree1swc.at(i).pn !=-1)
         {
             tree1swc[i].pn=find(ids.begin(), ids.end(),tree1swc.at(i).pn) - ids.begin()+1;
         }
     }
     /*ids.clear();
     for (int i=0;i<tree1swc.size();i++){
         int cc=tree1swc.at(i).n;
         ids.push_back(cc);
     }
     NeuronTree n_t;
     QHash <int, int> hash_nt;

     for(V3DLONG j=0; j<tree1swc.size();j++){
         hash_nt.insert(tree1swc[j].n, j);
     }
     n_t.listNeuron=tree1swc;
     n_t.hashNeuron=hash_nt;
     tree1=n_t;
     //for(int i=100;i<150;i++){
     //cout<<"-------------================-"<<tree1swc.at(i).n<<endl;}
     /*int temp;
     int j=0;
     for(int i=j+1;i<kinds.size();i++)
     {
         if (kinds.at(j)>=kinds.at(i)){
             temp=kinds.at(j);
             kinds.at(j)=kinds.at(i);
             kinds.at(i)=temp;
         }
     }*/

     //get the childslist
     /*V3DLONG neuronNum = tree1.listNeuron.size();
     childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
     for (V3DLONG i=0;i<neuronNum;i++)
     {
         V3DLONG par = tree1.listNeuron[i].pn;
         if (par<0) continue;
         childs[tree1.hashNeuron.value(par)].push_back(i);
     }
     vector<int> suspoint;
     //for (int i=0;i<tree1swc.size();i++)
     //{
         /*if (childs[i].size()==2)
         {
             int child1=childs[i][0];
             int child2=childs[i][1];
             if(tree1swc.at(i).type != tree1swc.at(child1).type) suspoint.push_back(child1);
             else if(tree1swc.at(i).type != tree1swc.at(child2).type) suspoint.push_back(child2);
         }
         if(childs[i].size()==1)
         {
             int child1=childs[i][0];
             int type1=tree1swc.at(i).type; int type2=tree1swc.at(child1).type;
             if (type1 != type2)
             suspoint.push_back(child1);
             if(tree1swc.at(i).type != tree1swc.at(child1).type) {suspoint.push_back(child1);}
         }
       }*/
     /*vector<int> suspoint;
     for (int i=0;i<tree1swc.size();i++)
      {
         int child1=childs[i][0];
         int type1=tree1swc.at(i).type; int type2=tree1swc.at(child1).type;
         if (type1 == type2)
         suspoint.push_back(child1);

       }

     // use iterator to find the points having different types

     for (int i=0;i<tree1swc.size();i++){
         int type1=tree1swc.at(i).type;
         int pt=tree1swc.at(i).pn;
         if(pt!=-1)
         {
             vector<int>::iterator iter=find(ids.begin(), ids.end(), pt);
             int a=distance(ids.begin(),iter);
             int type2=tree1swc.at(a-1).type;
             if(type1 != type2)
                 suspoint.push_back(i);}
     }*/

     /*vector<int> suspoint;
     int j=0;
     for (int i=j+1;i<kinds.size();i++){
         if (kinds.at(i)!=kinds.at(j))
             suspoint.push_back(i);
     }*/
