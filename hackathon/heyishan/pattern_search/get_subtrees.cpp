#include"get_subtrees.h"
#include "pattern_analysis.h"
#include <qstack.h>
#include <stdlib.h>
#include "my_sort.h"
#ifndef getParent(n,nt)
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
#endif
#ifndef VOID
#define VOID 1000000000
#endif
bool get_subtrees(const NeuronTree &nt, vector<NeuronTree> &sub_trees, int boundary_length, vector<vector<V3DLONG> >p_to_tree)
{
    if(nt.listNeuron.size()==0)
    {
        cout<<"Neuron Tree is empty"<<endl;
        return false;
    }
    double step_radio= 0.2;
    int search_step = boundary_length * step_radio;
    if(search_step<1)   search_step=1;
    cout<<"search_step="<<search_step<<endl;

    // get each node's children
    V3DLONG nt_size = nt.listNeuron.size();
    QVector<QVector<V3DLONG> > childs;
    childs = QVector< QVector<V3DLONG> >(nt_size, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<nt_size;i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    // get small trees in whole nt
//    boundary_length=8;
    cout<<"length = "<<boundary_length<<endl;
//    for(V3DLONG i=0; i<30-search_step; i+=search_step)
    for(V3DLONG i=0; i<nt.listNeuron.size() - search_step; i+=search_step)
    {
        NeuronTree s_tree;
        vector<V3DLONG> points;
        s_tree.listNeuron.clear();
        s_tree.hashNeuron.clear();
        points.clear();
        NeuronSWC cur_point = nt.listNeuron[i];
        s_tree.listNeuron.push_back(cur_point);
        points.push_back(i);
        // using stack marching put points into s_tree and points(vector)
        QStack<StackElem> TreeStack;
        char * state = new char[nt.listNeuron.size()];
        //memset(state,FAR,sizeof(state));
        memset(state,FAR,nt.listNeuron.size());
        state[i]=ALIVE;
        StackElem first_elem(i,0);
        TreeStack.push(first_elem);
        while(!TreeStack.isEmpty())
        {
            StackElem cur_elem = TreeStack.top();
            TreeStack.pop();
            state[i]=ALIVE;
            V3DLONG cur_elem_pn=getParent(cur_elem.id,nt);
            if(cur_elem.len>=boundary_length) continue;
            else
            {
                //push its children into stack, points and s_tree
                for(V3DLONG j=0; j<childs[cur_elem.id].size(); j++)
                {
                    V3DLONG c=childs[cur_elem.id].at(j);
                    if(state[c]==FAR)
                    {
                        StackElem elem(c,0);
                        elem.len=cur_elem.len +1;
                        TreeStack.push(elem);
                        points.push_back(elem.id);
                        NeuronSWC p=nt.listNeuron[elem.id];
                        s_tree.listNeuron.push_back(p);
                        state[c]=ALIVE;
                    }
                }
                // push its parent into stack,points and s_tree
                if(cur_elem_pn!=VOID && state[cur_elem_pn]==FAR )
                {
                    StackElem elem(cur_elem_pn,0);
                    elem.len=cur_elem.len +1;
                    TreeStack.push(elem);
                    points.push_back(elem.id);
                    NeuronSWC p=nt.listNeuron[elem.id];
                    s_tree.listNeuron.push_back(p);
                    state[cur_elem_pn]=ALIVE;
                }
            }//else
        }//while
        // sort s_tree
        NeuronTree s_tree_sorted;
        s_tree_sorted.listNeuron.clear();
        s_tree_sorted.hashNeuron.clear();
        if(s_tree.listNeuron.size()!=0)
        {
            V3DLONG root_id=s_tree.listNeuron[0].n;
            s_tree_sorted = sort(s_tree, root_id,VOID);
        }

        p_to_tree.push_back(points);
        sub_trees.push_back(s_tree_sorted);
        //cout<<"points = "<<points.size()<<endl;
        TreeStack.clear();
        if(state){delete [] state;state=0;}
    }//end i

    cout<<"nt_size"<<nt.listNeuron.size()<<endl;
    cout<<"sub_trees="<<sub_trees.size()<<endl;
    return true;
}
