#include "make_consensus.h"

#define VOID 1000000000
#define min(a,b) (a)<(b)?(a):(b)
#define max(a,b) (a)>(b)?(a):(b)

struct Boundary
{
    double minx;
    double miny;
    double minz;
    double maxx;
    double maxy;
    double maxz;
};

void make_consensus(const NeuronTree & nt, NeuronTree & pattern, NeuronTree & mk)
{
    V3DLONG pattern_size = pattern.listNeuron.size();
    V3DLONG nt_size = nt.listNeuron.size();
    vector<Boundary> v_boundary;
    //vector<NeuronTree> trees();
    if(pattern_size==0 || nt_size==0)
    {
        cout<<"no input"<<endl;
        return;
    }
    cout<<"size="<<pattern_size<<endl;
    Boundary temp;
    for(V3DLONG i=0;i<pattern_size;i++)
    {

       if(pattern.listNeuron[i].pn<0)
       {
            temp.minx=VOID; temp.miny=VOID;temp.minz=VOID;
            temp.maxx=0;temp.maxy=0;temp.maxz=0;
        }
       temp.minx=min(pattern.listNeuron[i].x,temp.minx);
       temp.miny=min(pattern.listNeuron[i].y,temp.miny);
       temp.minz=min(pattern.listNeuron[i].z,temp.minz);
       temp.maxx=max(pattern.listNeuron[i].x,temp.maxx);
       temp.maxy=max(pattern.listNeuron[i].y,temp.maxy);
       temp.maxz=max(pattern.listNeuron[i].z,temp.maxz);

       if(i==pattern_size-2)
       {
           temp.minx=min(pattern.listNeuron[i+1].x,temp.minx);
           temp.miny=min(pattern.listNeuron[i+1].y,temp.miny);
           temp.minz=min(pattern.listNeuron[i+1].z,temp.minz);
           temp.maxx=max(pattern.listNeuron[i+1].x,temp.maxx);
           temp.maxy=max(pattern.listNeuron[i+1].y,temp.maxy);
           temp.maxz=max(pattern.listNeuron[i+1].z,temp.maxz);
           v_boundary.push_back(temp);
           i++;
       }
       else if(pattern.listNeuron[i+1].pn==-1)
       {
           v_boundary.push_back(temp);
       }
    }
    vector<NeuronTree> trees(v_boundary.size());

   for(V3DLONG i=0;i<nt.listNeuron.size();i++)
   {
       NeuronSWC curr = nt.listNeuron[i];
       for(int j=0; j<v_boundary.size();j++)
       {
           Boundary b=v_boundary[j];
           if(curr.x>b.minx && curr.y>b.miny && curr.z>b.minz &&curr.x<b.maxx && curr.y<b.maxy&&curr.z<b.maxz)
           {
               trees[j].listNeuron.push_back(curr);
               cout<<j<<endl;
               break;
           }
       }
   }

}
