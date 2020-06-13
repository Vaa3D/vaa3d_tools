#include "function.h"

void getNodeLength(NeuronTree& nt,int maxR,double dendritR, double otherR){

    cout<<"---------------in getNodeLength-----------------"<<endl;

    QList<NeuronSWC>& listNeuron =  nt.listNeuron;
    QHash<int,int>& hN = nt.hashNeuron;
    int pointNum = listNeuron.size();

    SwcTree t;
    t.initialize(nt);

    vector<double> nodeLenth = vector<double>(pointNum,0);
    vector<int> nodeLevel = vector<int>(pointNum,0);

    for(int i=0; i<t.branchs.size(); i++){
        vector<int> indexs = vector<int>();
        t.branchs.at(i).get_points_of_branch(indexs,nt);
        cout<<"i: "<<i<<" level: "<<t.branchs.at(i).level<<endl;
        for(int j=1; j<indexs.size(); j++){
            nodeLevel.at(indexs.at(j)) = t.branchs.at(i).level;
        }
    }




    vector<vector<int> > child = vector<vector<int> >(pointNum,vector<int>());
    vector<int> roots = vector<int>();
    for(int i=0; i<pointNum; i++){
        const NeuronSWC& p = listNeuron.at(i);
        V3DLONG prt = p.parent;
        if(prt != -1 && hN.contains(prt)){
            int prtIndex = hN.value(prt);
            child.at(prtIndex).push_back(i);
        }else {
            roots.push_back(i);
        }
    }

    vector<int> tips = vector<int>();

    nodeLevel.at(roots.at(0)) = -1;

    int maxLevel = t.get_max_level();
    cout<<"maxLevel: "<<maxLevel<<endl;
    double ratio = 1;

    while (maxLevel>=0) {
        cout<<"level: "<<maxLevel<<endl;
        if(maxLevel<0)
            break;
        for(int i=0; i<pointNum; i++){
            if(child.at(i).size() != 1 && nodeLevel.at(i) == maxLevel){
                tips.push_back(i);
            }
        }


        cout<<"tips size:"<<tips.size()<<endl;


        for(int i=0; i<tips.size(); i++){
            int temp = tips.at(i);
            if(child.at(temp).size()>1){
                if(listNeuron.at(temp).type == 2){
                    ratio = dendritR;
                }else if(listNeuron.at(temp).type == 3){
                    ratio = otherR;
                }else {
                    ratio = 1;
                }
                for(int j=0; j<child.at(temp).size(); j++){
                    int cIndex = child.at(temp).at(j);
                    nodeLenth.at(temp) += (dis(listNeuron.at(cIndex),listNeuron.at(temp))*ratio+nodeLenth.at(cIndex));
                }
            }
            int prtIndex = hN.value(listNeuron.at(temp).parent);

            while (child.at(prtIndex).size() == 1 && listNeuron.at(prtIndex).parent != -1) {
                if(listNeuron.at(prtIndex).type == 2){
                    ratio = dendritR;
                }else if(listNeuron.at(prtIndex).type == 3){
                    ratio = otherR;
                }else {
                    ratio = 1;
                }
                nodeLenth.at(prtIndex) += (dis(listNeuron.at(temp),listNeuron.at(prtIndex))*ratio + nodeLenth.at(temp));
                temp = prtIndex;
                prtIndex = hN.value(listNeuron.at(temp).parent);
            }
        }
        cout<<"level end one-----------"<<endl;
        tips.clear();
        maxLevel--;
    }

    cout<<"start cal root length"<<endl;

    double max = 0;


    for(int i=0; i<child.at(roots.at(0)).size(); i++){
        int cIndex = child.at(roots.at(0)).at(i);
        if(max<nodeLenth.at(cIndex)){
            max = nodeLenth.at(cIndex);
        }
//        nodeLenth.at(roots.at(0)) += (dis(listNeuron.at(roots.at(0)),listNeuron.at(cIndex)) + nodeLenth.at(cIndex)*2/child.at(roots.at(0)).size());
    }
    nodeLenth.at(roots.at(0)) = max + 1;

    cout<<"--------------length cal end---------------"<<endl;

    double maxLength = nodeLenth.at(roots.at(0));

    for(int i=0; i<pointNum; i++){
//        cout<<"i: "<<i<<endl;
        nodeLenth.at(i) = (nodeLenth.at(i)/maxLength)*maxR;
        NeuronSWC tmp;
        tmp.x = listNeuron.at(i).x;
        tmp.y = listNeuron.at(i).y;
        tmp.z = listNeuron.at(i).z;
        tmp.n = listNeuron.at(i).n;
        tmp.parent = listNeuron.at(i).parent;
        tmp.type = listNeuron.at(i).type;
//        if(nodeLenth.at(i) > 0.2*maxR){
//            tmp.radius = 5;
//        }else {
//            tmp.radius = 1;
//        }
        tmp.radius = nodeLenth.at(i);

        listNeuron.replace(i,tmp);
    }

}


bool Comp(const int &a,const int &b)
{
    return a>b;
}

bool Branch::get_r_points_of_branch(vector<int> &r_points, NeuronTree &nt)
{
    NeuronSWC tmp=end_point;
    r_points.push_back(nt.hashNeuron.value(end_point.n));
    while(tmp.n!=head_point.n)
    {
        tmp=nt.listNeuron[nt.hashNeuron.value(tmp.parent)];
        r_points.push_back(nt.hashNeuron.value(tmp.n));
    }
    return true;
}

bool Branch::get_points_of_branch(vector<int> &points, NeuronTree &nt)
{
    vector<int> r_points;
    this->get_r_points_of_branch(r_points,nt);
    while(!r_points.empty())
    {
        int tmp=r_points.back();
        r_points.pop_back();
        points.push_back(tmp);
    }

    return true;
}

bool SwcTree::initialize(NeuronTree t)
{
    nt.deepCopy(t);

    V3DLONG ori;

    V3DLONG num_p=nt.listNeuron.size();
    vector<vector<V3DLONG> > children=vector<vector<V3DLONG> >(num_p,vector<V3DLONG>());
    for(V3DLONG i=0;i<num_p;++i)
    {
        V3DLONG prt=nt.listNeuron[i].parent;
        if(prt!= -1){
            V3DLONG prtIndex = nt.hashNeuron.value(prt);
            children.at(prtIndex).push_back(i);
        }else {
            ori = i;
        }
    }

    vector<V3DLONG> queue = vector<V3DLONG>();
    queue.push_back(ori);


    cout<<"initial head_point,end_point"<<endl;
    while(!queue.empty())
    {
        int tmp=queue.front();
        queue.erase(queue.begin());
        vector<V3DLONG>& child = children.at(tmp);
        cout<<"child size: "<<child.size()<<endl;
        for(int i=0;i<child.size();++i)
        {
            Branch branch;
            branch.head_point=nt.listNeuron.at(tmp);
            int cIndex = child.at(i);

            while(children.at(cIndex).size()==1)
            {
                cIndex = children.at(cIndex).at(0);
            }
            if(children.at(cIndex).size()>=1)
            {
                queue.push_back(cIndex);
            }
            branch.end_point=nt.listNeuron.at(cIndex);
            branchs.push_back(branch);
        }
    }

    //initial parent
    cout<<"initial parent"<<endl;
    cout<<"branch size: "<<branchs.size()<<endl;
    for(int i=0;i<branchs.size();++i)
    {
        cout<<i<<endl;
        if(branchs[i].head_point.parent<0)
        {
            branchs[i].parent=0;
        }
        else
        {
            for(int j=0;j<branchs.size();++j)
            {
                if(branchs[i].head_point==branchs[j].end_point)
                {
                    branchs[i].parent=&branchs[j];
                }
            }
        }
    }

    //initial level
    for(int i=0;i<branchs.size();++i)
    {
        Branch* tmp;
        tmp=&branchs[i];
        int level=0;
        while(tmp->parent!=0)
        {
            level++;
            tmp=tmp->parent;
        }
        branchs[i].level=level;
    }

    cout<<"--------------------initialize end--------------";

    return true;
}

bool SwcTree::get_level_index(vector<int> &level_index,int level)
{
    for(int i=0;i<branchs.size();++i)
    {
        if(branchs[i].level==level)
        {
            level_index.push_back(i);
        }
    }
    return true;
}




int SwcTree::get_max_level()
{
    int max_level = -1;
    for(int i=0;i<branchs.size();++i)
    {
        max_level=(max_level>branchs[i].level)?max_level:branchs[i].level;
    }
    return max_level;
}
