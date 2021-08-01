#include "Utility_function.h"
void resample_path(Segment * seg, double step)
{
    char c;
    Segment seg_r;
    double path_length = 0;
    Point* start = seg->at(0);
    Point* seg_par = seg->back()->p;
    V3DLONG iter_old = 0;
    seg_r.push_back(start);
    while (iter_old < seg->size() && start && start->p)
    {
        path_length += DISTP(start,start->p);
        if (path_length<=seg_r.size()*step)
        {
            start = start->p;
            iter_old++;
        }
        else//a new point should be created
        {
            path_length -= DISTP(start,start->p);
            Point* pt = new Point;
                        double rate = (seg_r.size()*step-path_length)/(DISTP(start,start->p));
            pt->x = start->x + rate*(start->p->x-start->x);
            pt->y = start->y + rate*(start->p->y-start->y);
            pt->z = start->z + rate*(start->p->z-start->z);
            pt->r = start->r*(1-rate) + start->p->r*rate;//intepolate the radius
            pt->p = start->p;
                        // Peng Xie 20190507
                        // Node type set as the child's
                        // This is better for sorted trees.
                        pt->type = start->type;

                        if (rate<0.5)
                        {
//                            pt->type = start->p->type;
                            pt->seg_id = start->seg_id;
                            pt->level = start->level;
                            pt->fea_val = start->fea_val;
                        }
                        else
                        {
//                            pt->type = start->p->type;
                            pt->seg_id = start->p->seg_id;
                            pt->level = start->p->level;
                            pt->fea_val = start->p->fea_val;

                        }
            seg_r.back()->p = pt;
            seg_r.push_back(pt);
            path_length += DISTP(start,pt);
            start = pt;
        }
    }
    seg_r.back()->p = seg_par;
    for (V3DLONG i=0;i<seg->size();i++)
        if (!seg->at(i)) {delete seg->at(i); seg->at(i) = NULL;}
    *seg = seg_r;
}
NeuronTree resample(NeuronTree input, double step)
{
    NeuronTree result;
    V3DLONG siz = input.listNeuron.size();
    Tree tree;
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = input.listNeuron[i];
        Point* pt = new Point;
        pt->x = s.x;
        pt->y = s.y;
        pt->z = s.z;
        pt->r = s.r;
        pt ->type = s.type;
        pt->seg_id = s.seg_id;
        pt->level = s.level;
        pt->fea_val = s.fea_val;
        pt->p = NULL;
        pt->childNum = 0;
        tree.push_back(pt);
    }
    for (V3DLONG i=0;i<siz;i++)
    {
        if (input.listNeuron[i].pn<0) continue;
        V3DLONG pid = input.hashNeuron.value(input.listNeuron[i].pn);
        tree[i]->p = tree[pid];
        tree[pid]->childNum++;
    }
//	printf("tree constructed.\n");
    vector<Segment*> seg_list;
    for (V3DLONG i=0;i<siz;i++)
    {
        if (tree[i]->childNum!=1)//tip or branch point
        {
            Segment* seg = new Segment;
            Point* cur = tree[i];
            do
            {
                seg->push_back(cur);
                cur = cur->p;
            }
            while(cur && cur->childNum==1);
            seg_list.push_back(seg);
        }
    }
//	printf("segment list constructed.\n");
    for (V3DLONG i=0;i<seg_list.size();i++)
    {
        resample_path(seg_list[i], step);
    }

//	printf("resample done.\n");
    tree.clear();
    map<Point*, V3DLONG> index_map;
    for (V3DLONG i=0;i<seg_list.size();i++)
        for (V3DLONG j=0;j<seg_list[i]->size();j++)
        {
            tree.push_back(seg_list[i]->at(j));
            index_map.insert(pair<Point*, V3DLONG>(seg_list[i]->at(j), tree.size()-1));
        }
    for (V3DLONG i=0;i<tree.size();i++)
    {
        NeuronSWC S;
        Point* p = tree[i];
        S.n = i+1;
        if (p->p==NULL) S.pn = -1;
        else
            S.pn = index_map[p->p]+1;
        if (p->p==p) printf("There is loop in the tree!\n");
        S.x = p->x;
        S.y = p->y;
        S.z = p->z;
        S.r = p->r;
        S.type = p->type;
        S.seg_id = p->seg_id;
        S.level = p->level;
        S.fea_val = p->fea_val;
        result.listNeuron.push_back(S);
    }
    for (V3DLONG i=0;i<tree.size();i++)
    {
        if (tree[i]) {delete tree[i]; tree[i]=NULL;}
    }
    for (V3DLONG j=0;j<seg_list.size();j++)
        if (seg_list[j]) {delete seg_list[j]; seg_list[j] = NULL;}
    for (V3DLONG i=0;i<result.listNeuron.size();i++)
        result.hashNeuron.insert(result.listNeuron[i].n, i);
    return result;
}
vector<int> getNodeType(NeuronTree nt)
{
    /*1. get tip, branch and soma nodes;
    */
    V3DLONG siz=nt.listNeuron.size();
    vector<int> ntype(siz,0);
    if(!siz)
        return ntype;
    /*1. get the index of nt:
                                        * swc_n -> index */
    QHash <int, int>  hashNeuron;hashNeuron.clear();
    V3DLONG somaid=1;
    for (V3DLONG i=0;i<siz;i++)
    {
        hashNeuron.insert(nt.listNeuron[i].n,i);
        if(nt.listNeuron[i].type==1&&nt.listNeuron[i].pn<0)
            somaid=i;
    }
    // 2. get node type: index -> node_type
    /*soma: ntype>=3, branch: ntype=2; tip: ntype=0; internodes: ntype=1*/

    ntype[somaid]=2;
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = nt.listNeuron[i];
        if(s.pn&&hashNeuron.contains(s.pn))
        {
            V3DLONG spn_id=hashNeuron.value(s.pn);
            ntype[spn_id]+=1;
        }
    }
    return ntype;
}
vector<int> getNodeTips(NeuronTree nt)
{
    /*for each nodes, get its subtree, get the node type of subtree, count the type*/
    V3DLONG siz=nt.listNeuron.size();
    vector<int> ntype(siz,0);
    vector<int> ntips(siz,0);
    ntype=getNodeType(nt);
    if(!siz)
        return ntips;
    V3DLONG somaid=1;
    for (V3DLONG i=0;i<siz;i++)
    {
        if(nt.listNeuron[i].type==1&&nt.listNeuron[i].pn<0)
            somaid=i;
    }
    vector< vector<long> > child_index_list(siz,vector<long>(ntype[somaid]-2,-1));
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = nt.listNeuron[i];
        if(s.pn<0)
            continue;//child_index_list[somaid].push_back(i);
        V3DLONG p_index=nt.hashNeuron.value(s.pn);
        child_index_list[p_index].push_back(i);
    }
    get_node_subtree_tips_iter(nt,child_index_list,ntips,somaid);
    return ntips;
}
NeuronTree pruning_subtree(NeuronTree nt, int pruning_thre)
{
    NeuronTree nt_out;
    V3DLONG siz=nt.listNeuron.size();
    vector<double> n_subtree_len(siz,0.0);
    n_subtree_len=get_node_subtree_len_v1(nt);
    for(V3DLONG i=0;i<nt.listNeuron.size();i++)
    {
        NeuronSWC s = nt.listNeuron[i];
        if(n_subtree_len[i]<pruning_thre)
            continue;
        nt_out.listNeuron.append(s);
        nt_out.hashNeuron.insert(s.n,nt_out.listNeuron.size()-1);
    }
    return nt_out;
}
vector<int> getNodeOrder(NeuronTree nt)
{
    /*1. get node type;
     * 2. from one node to soma,count how many branch nodes will be scanned.
     * 3.out
    */
    V3DLONG siz=nt.listNeuron.size();
    vector<int> ntype(siz,0);
    vector<int> norder(siz,0);
    ntype=getNodeType(nt);
    if(!siz)
        return norder;
    /*1. get the index of nt:
                                        * swc_n -> index */
    QHash <int, int>  hashNeuron;hashNeuron.clear();
    V3DLONG somaid=1;
    for (V3DLONG i=0;i<siz;i++)
    {
        hashNeuron.insert(nt.listNeuron[i].n,i);
        if(nt.listNeuron[i].type==1&&nt.listNeuron[i].pn<0)
            somaid=i;
    }
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = nt.listNeuron[i];
        if(somaid==i)
            continue;
        NeuronSWC s_iter=s;
        long pIndex=hashNeuron.value(s_iter.pn);
        int ptype=ntype[pIndex];
        /*for all the nodes except soma_node*/
        while(ptype<3)
        {
            if(ptype==2)
                norder[i]+=1;
            s_iter=nt.listNeuron[pIndex];
            pIndex=hashNeuron.value(s_iter.pn);
            ptype=ntype[pIndex];
        }
        norder[i]+=1;
    }
    return norder;
}
NeuronTree getSubtree(NeuronTree nt,V3DLONG nodeid)
{
    /*nodeid: swc n id*/
    NeuronTree nt_out;nt_out.listNeuron.clear();nt_out.hashNeuron.clear();
    V3DLONG siz=nt.listNeuron.size();
    if(!siz)
        return nt_out;
    /*
     *from all the tips and end at target_node
    */
    vector<int> ntype(siz,0);  vector<int> nkept(siz,0);
    ntype=getNodeType(nt);
    QHash <int, int>  hashNeuron;hashNeuron.clear();
    V3DLONG somaid=1;
    for (V3DLONG i=0;i<siz;i++)
    {
        hashNeuron.insert(nt.listNeuron[i].n,i);
        if(nt.listNeuron[i].type==1&&nt.listNeuron[i].pn<0)
            somaid=i;
    }
    //soma node
    if(somaid==nodeid)
        return nt;
    if(!hashNeuron.contains(nodeid))
        return nt_out;
    long nodeIndex=hashNeuron.value(nodeid);
    nkept[nodeIndex]=1;
    if(ntype[nodeIndex]!=0) //not tip node
    {
        for (V3DLONG i=0;i<siz;i++)
        {
            NeuronSWC s = nt.listNeuron[i];
            if(somaid==i||nodeIndex==i||nkept[i]==1)
                continue;
            NeuronSWC s_iter=s;
            long pIndex=hashNeuron.value(s_iter.pn);
            while(pIndex!=somaid)
            {
                if(pIndex==nodeIndex)
                {
                    nkept[i]=1;break;
                }
                s_iter=nt.listNeuron[pIndex];
                pIndex=hashNeuron.value(s_iter.pn);
            }
        }
    }
    for (V3DLONG i=0;i<siz;i++)
    {
        if(nkept[i]!=0)
        {
            NeuronSWC s = nt.listNeuron[i];
            if(i==nodeIndex)
                s.pn=-1;
            nt_out.listNeuron.append(s);
            nt_out.hashNeuron.insert(s.n,nt_out.listNeuron.size()-1);
        }
    }
   return nt_out;
}
double get_nt_len(NeuronTree nt)
{
    double out_len=0.0;
    V3DLONG siz=nt.listNeuron.size();
    if(!siz)
        return out_len;
     for (V3DLONG i=0;i<siz;i++)
     {
          NeuronSWC s = nt.listNeuron[i];
          if(s.pn>0)
          {
              V3DLONG spid=nt.hashNeuron.value(s.pn);
              NeuronSWC sp=nt.listNeuron[spid];
              out_len+=sqrt((s.x-sp.x)*(s.x-sp.x)+
                          (s.y-sp.y)*(s.y-sp.y)+
                            (s.z-sp.z)*(s.z-sp.z));
          }
     }
    return out_len;
}
vector<double> get_node_subtree_len(NeuronTree nt,int normalized_size)
{

    V3DLONG siz=nt.listNeuron.size();
    vector<double> n_subtree_len(siz,0.0);
    if(!siz)
        return n_subtree_len;
    vector<int> ntype(siz,0);
    ntype=getNodeType(nt);
    QHash <int, int>  hashNeuron;hashNeuron.clear();
    V3DLONG somaid=1;
    for (V3DLONG i=0;i<siz;i++)
    {
        hashNeuron.insert(nt.listNeuron[i].n,i);
        if(nt.listNeuron[i].type==1&&nt.listNeuron[i].pn<0)
            somaid=i;
    }
    vector< vector<long> > child_index_list(siz,vector<long>(ntype[somaid]-2,-1));
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = nt.listNeuron[i];
        if(s.pn<0)
            continue;//child_index_list[somaid].push_back(i);
        V3DLONG p_index=hashNeuron.value(s.pn);
        child_index_list[p_index].push_back(i);
    }
    cout<<"size of child max "<<child_index_list[somaid].size()<<endl;
    get_node_subtree_len_iter(nt,child_index_list,n_subtree_len,somaid);
    double nt_len=0.0;
    for(V3DLONG i=0;i<siz;i++)
        nt_len=(nt_len<n_subtree_len[i])?n_subtree_len[i]:nt_len;
    if(normalized_size&&nt_len)
    {
        for(V3DLONG i=0;i<siz;i++)
        {
            n_subtree_len[i]=normalized_size*n_subtree_len[i]/nt_len;
        }
    }
    return n_subtree_len;
}
void get_node_subtree_tips_iter(NeuronTree nt,vector< vector<long> > child_index_list,vector<int> & n_subtree_tips,V3DLONG iter_id)
{
    for(int i=0;i<child_index_list[iter_id].size();i++)
    {
        if(child_index_list[iter_id][i]<0)
            continue;
        if(n_subtree_tips[child_index_list[iter_id][i]]==0)
            get_node_subtree_tips_iter(nt,child_index_list,n_subtree_tips,child_index_list[iter_id][i]);
        n_subtree_tips[iter_id]+=n_subtree_tips[child_index_list[iter_id][i]];
    }
    if(n_subtree_tips[iter_id]==0)
        n_subtree_tips[iter_id]=1;
}
void get_node_subtree_len_iter(NeuronTree nt,vector< vector<long> > child_index_list,vector<double> & n_subtree_len,V3DLONG iter_id)
{
    NeuronSWC s=nt.listNeuron[iter_id];
    for(int i=0;i<child_index_list[iter_id].size();i++)
    {
        if(child_index_list[iter_id][i]<0)
            continue;
        NeuronSWC sc=nt.listNeuron[child_index_list[iter_id][i]];
        if(n_subtree_len[child_index_list[iter_id][i]]==0)
            get_node_subtree_len_iter(nt,child_index_list,n_subtree_len,child_index_list[iter_id][i]);
        n_subtree_len[iter_id]+=(n_subtree_len[child_index_list[iter_id][i]]+sqrt((s.x-sc.x)*(s.x-sc.x)+
                                                                                  (s.y-sc.y)*(s.y-sc.y)+
                                                                                  (s.z-sc.z)*(s.z-sc.z)
                                                                                  ));
    }
}
vector<double> get_node_subtree_len_v1(NeuronTree nt,int normalized_size)
{
    cout<<"---------------in getNodeLength-----------------"<<endl;
    double axonRatio=1.0; double otherR=1.0;

    QList<NeuronSWC>& listNeuron =  nt.listNeuron;
    QHash<int,int>& hN = nt.hashNeuron;
    int pointNum = listNeuron.size();

    SwcTree t;    t.initialize(nt);
    vector<double> nodeLenth = vector<double>(pointNum,0);
    vector<int> nodeLevel = vector<int>(pointNum,0);

    for(int i=0; i<t.branchs.size(); i++){
        vector<int> indexs = vector<int>();
        t.branchs.at(i).get_points_of_branch(indexs,nt);
//        cout<<"i: "<<i<<" level: "<<t.branchs.at(i).level<<endl;
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
//        cout<<"level: "<<maxLevel<<endl;
        if(maxLevel<0)
            break;
        for(int i=0; i<pointNum; i++){
            if(child.at(i).size() != 1 && nodeLevel.at(i) == maxLevel){
                tips.push_back(i);
            }
        }
//        cout<<"tips size:"<<tips.size()<<endl;
        for(int i=0; i<tips.size(); i++){
            int temp = tips.at(i);
            if(child.at(temp).size()>1){
                if(listNeuron.at(temp).type == 2){
                    ratio = axonRatio;
                }else if(listNeuron.at(temp).type == 3){
                    ratio = otherR;
                }else {
                    ratio = 1;
                }
                for(int j=0; j<child.at(temp).size(); j++){
                    int cIndex = child.at(temp).at(j);
                    nodeLenth.at(temp) += (dis(listNeuron.at(cIndex),listNeuron.at(temp))*ratio+nodeLenth.at(cIndex));
                }
            }else if(child.at(temp).size() == 0){
                nodeLenth.at(temp) = 1;
            }
            int prtIndex = hN.value(listNeuron.at(temp).parent);

            while (child.at(prtIndex).size() == 1 && listNeuron.at(prtIndex).parent != -1) {
                if(listNeuron.at(prtIndex).type == 2){
                    ratio = axonRatio;
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
//        cout<<"level end one-----------"<<endl;
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
    if(normalized_size>0)
        for(int i=0; i<pointNum; i++)
            nodeLenth.at(i) = (nodeLenth.at(i)/maxLength)*normalized_size;
    return nodeLenth;
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
//        cout<<"child size: "<<child.size()<<endl;
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
//        cout<<i<<endl;
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
