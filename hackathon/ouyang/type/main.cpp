
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
QVector<QVector<V3DLONG> > childs;
bool detect_type_distance(V3DPluginCallback2 &callback, QWidget *parent)
  {
     NeuronTree tree1=callback.getSWCTeraFly();
     LandmarkList Markers = callback.getLandmarkTeraFly();
     QList<NeuronSWC> tree1swc=tree1.listNeuron;

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
            segsinfo seg=getallchildsofseg(roots.at(j));
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

  }

//collect all childs of one single segment
struct segsinfo getallchildsofseg(int t)
        {
            struct segsinfo result;
            vector<int> rchildlist;
            rchildlist.clear();
            V3DLONG tmp;
            V3DLONG cc=1;
            tmp = childs[t].at(0);
            rchildlist.push_back(tmp);
            while (childs[tmp].size()==1)
            {
              tmp = childs[tmp].at(0);
              cc+=1;
              rchildlist.push_back(tmp);
            }
            int endpoint=rchildlist.at((rchildlist.size())-1);
            result.segpoint=rchildlist;
            result.aa=cc;
            result.endnd=endpoint;
            return result;
        }

bool detect_type(V3DPluginCallback2 &callback, QWidget *parent)
{
    NeuronTree tree1=callback.getSWCTeraFly();
    LandmarkList Markers = callback.getLandmarkTeraFly();
    QList<NeuronSWC> tree1swc=tree1.listNeuron;

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
//find the dumplicate nodes which have diffrent types
    vector<int> suspoint;
    for (int i=0;i<tree1swc.size();i++)
    {
        for (int j=0;j<tree1swc.size();j++)
        {
            if (i != j)
            {
                int x1,x2,y1,y2,z1,z2,type1,type2;
                x1=tree1swc.at(i).x;
                x2=tree1swc.at(j).x;
                y1=tree1swc.at(i).y;
                y2=tree1swc.at(j).y;
                z1=tree1swc.at(i).z;
                z2=tree1swc.at(j).z;
                type1=tree1swc.at(i).type;
                type2=tree1swc.at(j).type;
                if(x1==x2 && y1==y2 && z1==z2 && type1!=type2) suspoint.push_back(i);
            }

        }
    }
    cout<<"++++++++++++++++++++++++++++++the suspoint number: "<<suspoint.size()<<endl;
//delete one suspoint node of two nodes who have short distance
    for (int i=0;i<suspoint.size();i++)
    {
        for (int j=0;j<suspoint.size();j++)
        {
            if (i != j)
            {
                double dist1;
                dist1=dist(tree1swc.at(suspoint.at(i)),tree1swc.at(suspoint.at(j)));
                if (dist1<5)
                {
                    vector<int>::iterator iter=find(suspoint.begin(), suspoint.end(), suspoint.at(j));
                    suspoint.erase(iter);
                }
            }
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

}
     /*vector<vector<int> > boom(numroot,vector<int>());
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
