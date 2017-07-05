#include "pattern_analysis.h"
#include "sort_swc.h"
#include <qstack.h>
#ifndef VOID
#define VOID 1000000000
#endif
#ifndef getParent(n,nt)
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
#endif
enum{ALIVE=-1, TRIAL=0, FAR=1};

struct StackElem
{
    int id;
    int len;
    TreeNdoe(int _id)
    {
        id=_id;
        len=0;
    }
};

struct Boundary
{
    float minx;
    float miny;
    float minz;
    float maxx;
    float maxy;
    float maxz;
};

//QVector<V3DLONG> getRemoteChild(int t)
//{
//    QVector<V3DLONG> rchildlist;
//    rchildlist.clear();
//    int tmp;
//    for (int i=0;i<childs[t].size();i++)
//    {
//        tmp = childs[t].at(i);
//        while (childs[tmp].size()==1)
//            tmp = childs[tmp].at(0);
//        rchildlist.append(tmp);
//    }
//    return rchildlist;
//}

bool pattern_analysis(const NeuronTree &nt, const NeuronTree &boundary,NeuronTree & consensus, int & boundary_length)
{
    V3DLONG boundary_size = boundary.listNeuron.size();
    V3DLONG nt_size= nt.listNeuron.size();
    if(boundary_size==0 || nt_size==0)
    {
        v3d_msg("Input is empty, please retry");
        return false;
    }

    // find min max boundary of each area of interest
    Boundary temp;
    vector<Boundary> v_boundary;
    for(V3DLONG i=0;i<boundary_size;i++)
    {

       if(boundary.listNeuron[i].pn<0)
       {
            temp.minx=VOID; temp.miny=VOID;temp.minz=VOID;
            temp.maxx=0;temp.maxy=0;temp.maxz=0;
        }
       temp.minx=min(boundary.listNeuron[i].x,temp.minx);
       temp.miny=min(boundary.listNeuron[i].y,temp.miny);
       temp.minz=min(boundary.listNeuron[i].z,temp.minz);
       temp.maxx=max(boundary.listNeuron[i].x,temp.maxx);
       temp.maxy=max(boundary.listNeuron[i].y,temp.maxy);
       temp.maxz=max(boundary.listNeuron[i].z,temp.maxz);

       if(i==boundary_size-2)
       {
           temp.minx=min(boundary.listNeuron[i+1].x,temp.minx);
           temp.miny=min(boundary.listNeuron[i+1].y,temp.miny);
           temp.minz=min(boundary.listNeuron[i+1].z,temp.minz);
           temp.maxx=max(boundary.listNeuron[i+1].x,temp.maxx);
           temp.maxy=max(boundary.listNeuron[i+1].y,temp.maxy);
           temp.maxz=max(boundary.listNeuron[i+1].z,temp.maxz);
           v_boundary.push_back(temp);
           i++;
       }
       else if(boundary.listNeuron[i+1].pn==-1)
       {
           v_boundary.push_back(temp);
       }
    }

    // push points in v_boundary into v_area;
    vector<NeuronTree> v_area(v_boundary.size());

   for(V3DLONG i=0;i<nt.listNeuron.size();i++)
   {
       NeuronSWC curr = nt.listNeuron[i];
       for(int j=0; j<v_boundary.size();j++)
       {           Boundary b=v_boundary[j];
           if(curr.x>b.minx && curr.y>b.miny && curr.z>b.minz &&curr.x<b.maxx && curr.y<b.maxy&&curr.z<b.maxz)
           {
               v_area[j].listNeuron.push_back(curr);
               break;
           }
       }
    }

   // calculate boundary_length of each of v_boundary
   vector<double> length(v_boundary.size(),0);
   int area_num = 0;
   double v_tol_dist = 0;
   for(V3DLONG i=0;i<v_boundary.size();i++)
   {
       int endPointNum = 0;
       int tol_len = 0;
       if(v_area[i].listNeuron.size()<=1) {cout<<"the number of points within this boundary isn't more than 1"<<endl; continue;}
       NeuronTree area_sorted;
        if(!SortSWC(v_area[i].listNeuron,area_sorted.listNeuron,VOID,VOID))
        {
            cout<<"sort failed"<<endl;
            return false;
        }
        area_sorted.hashNeuron.clear();
        for(V3DLONG i=0; i<area_sorted.listNeuron.size();i++){area_sorted.hashNeuron.insert(area_sorted.listNeuron[i].n, i);}

        // get each node's children
        V3DLONG area_size = area_sorted.listNeuron.size();
        QVector<QVector<V3DLONG> > childs;
        childs = QVector< QVector<V3DLONG> >(area_size, QVector<V3DLONG>() );
        for (V3DLONG i=0;i<area_size;i++)
        {
            V3DLONG par = area_sorted.listNeuron[i].pn;
            if (par<0) continue;
            childs[area_sorted.hashNeuron.value(par)].push_back(i);
        }

        // using stack marching caculate lenth from each point k to end point;
        for(V3DLONG j=0; j<area_size;j++)
        {
            QStack<StackElem> TreeStack;
            char * state = new char[area_size];
            for(V3DLONG k=0;k<area_size;k++)
            {
                if(k=j) state[k]=ALIVE;
                else    state[k]=FAR;
            }
            StackElem * first_elem = new StackElem(j);
            TreeStack.push(first_elem);
            while (!TreeStack.isEmpty())
            {
                StackElem cur_elem = TreeStack.top();   // get top element in the stack (this way might be wrong)
                state[cur_elem.id] = ALIVE;
                V3DLONG cur_elem_pn=getParent(cur_elem.id,area_sorted);
                // if cur_elem is end point add its len into tol_len and endPointNum plus one
                if(childs[cur_elem.id].size()==0 || cur_elem_pn==VOID)
                {
                    if(cur_elem.len != 0)   //in case cur_elem is end point
                    {
                        endPointNum+=1;
                        tol_len+=cur_elem.len;
                    }
                }
                // push its children into stack
                for(int k=0; k<childs[cur_elem.id].size(); k++)
                {
                    if(state[k]==FAR)
                    {
                        StackElem * elem = new StackElem(k);
                        elem->len=cur_elem.len + 1;
                        TreeStack.push(elem);
                        state[k]=ALIVE;
                    }
                }
                // push its parent into stack
                if(cur_elem_pn!=VOID && state[cur_elem_pn]==FAR )
                {
                    StackElem *elem = new StackElem(cur_elem_pn);
                    elem->len=cur_elem.len +1;
                    TreeStack.push(elem);
                    state[cur_elem_pn]=ALIVE;
                }
                TreeStack.pop();
            }//while
            TreeStack.clear();
        }//end j
        v_tol_dist = v_tol_dist + tol_len/endPointNum;
        area_num += 1;
        childs.clear();
   }//end i

   boundary_length= int(v_tol_dist/area_num);

}
