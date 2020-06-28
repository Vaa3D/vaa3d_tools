#include "branchtree.h"

#include <iostream>

using namespace std;

bool Branch::get_r_pointsIndex_of_branch(vector<V3DLONG> &r_points, NeuronTree &nt)
{
//    qDebug()<<"headPointIndex: "<<headPointIndex<<" endPointIndex: "<<endPointIndex;
    int tmp = endPointIndex;
    r_points.push_back(tmp);
    while(tmp != headPointIndex)
    {
//        qDebug()<<"tmp: "<<tmp;
        tmp = nt.hashNeuron.value(nt.listNeuron[tmp].parent);
        r_points.push_back(tmp);
    }
    return true;
}

bool Branch::get_pointsIndex_of_branch(vector<V3DLONG> &points, NeuronTree &nt)
{
    vector<V3DLONG> r_points;
    this->get_r_pointsIndex_of_branch(r_points,nt);
    while(!r_points.empty())
    {
        int tmp=r_points.back();
        r_points.pop_back();
        points.push_back(tmp);
    }

    return true;
}

bool BranchTree::initialize(NeuronTree t)
{
    nt.deepCopy(t);

    vector<V3DLONG> ori = vector<V3DLONG>();

    V3DLONG num_p=nt.listNeuron.size();
    vector<vector<V3DLONG> > children=vector<vector<V3DLONG> >(num_p,vector<V3DLONG>());
    for(V3DLONG i=0;i<num_p;++i)
    {
        V3DLONG prt=nt.listNeuron[i].parent;
        if(prt != -1){
            V3DLONG prtIndex = nt.hashNeuron.value(prt);
            children.at(prtIndex).push_back(i);
        }else {
            ori.push_back(i);
        }
    }

    vector<V3DLONG> queue = vector<V3DLONG>();
    queue.insert(queue.begin(),ori.begin(),ori.end());
    cout<<"queue size: "<<queue.size()<<endl;


    cout<<"initial head_point,end_point"<<endl;
    while(!queue.empty())
    {
        V3DLONG tmp=queue.front();
        queue.erase(queue.begin());
        vector<V3DLONG>& child = children.at(tmp);
        cout<<"child size: "<<child.size()<<endl;
        for(int i=0;i<child.size();++i)
        {
            Branch branch;
            branch.headPointIndex = tmp;
            int cIndex = child.at(i);

            while(children.at(cIndex).size() == 1)
            {
                cIndex = children.at(cIndex).at(0);
            }
            if(children.at(cIndex).size() >= 1)
            {
                queue.push_back(cIndex);
            }
            branch.endPointIndex = cIndex;
            branchs.push_back(branch);
        }
    }

    //initial parent
    cout<<"initial parent"<<endl;
    cout<<"branch size: "<<branchs.size()<<endl;
    for(int i=0;i<branchs.size();++i)
    {
        cout<<i<<endl;
        if(nt.listNeuron.at(branchs[i].headPointIndex).parent < 0)
        {
            branchs[i].parent = 0;
        }
        else
        {
            for(int j=0; j<branchs.size(); ++j)
            {
                if(branchs[i].headPointIndex == branchs[j].endPointIndex)
                {
                    branchs[i].parent = &branchs[j];
                }
            }
        }
    }

    //initial level
    for(int i=0; i<branchs.size(); ++i)
    {
        Branch* tmp;
        tmp = &branchs[i];
        int level=0;
        while(tmp->parent != 0)
        {
            level++;
            tmp = tmp->parent;
        }
        branchs[i].level=level;
    }

    cout<<"--------------------initialize end--------------";

    return true;
}

bool BranchTree::get_level_index(vector<int> &level_index, int level){
    for(int i=0; i<branchs.size(); i++){
        if(branchs[i].level == level){
            level_index.push_back(i);
        }
    }
    return true;
}

int BranchTree::get_max_level(){
    int max = 0;
    for(int i=0; i<branchs.size(); i++){
        if(branchs[i].level > max){
            max = branchs[i].level;
        }
    }
    return max;
}
