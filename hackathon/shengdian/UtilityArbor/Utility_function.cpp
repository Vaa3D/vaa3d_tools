#include "Utility_function.h"
V3DLONG get_soma(NeuronTree & nt,bool connect){
    V3DLONG niz=nt.listNeuron.size();
    V3DLONG somaid=-1;
    if(connect){
        for(V3DLONG i=0;i<niz;i++){
            NeuronSWC s=nt.listNeuron.at(i);
            if(s.pn<0&&s.type!=1){
                cout<<"---------------Attempt to process multiple -1 nodes-----------------------"<<endl;
                //find the node with same coordinates
                for(V3DLONG j=0;j<niz;j++){
                    NeuronSWC sj=nt.listNeuron.at(j);
                    if(i!=j&&s.x==sj.x&&s.y==sj.y&&s.z==sj.z)
                    {
                        nt.listNeuron[i].pn=sj.n;
                    }
                }
            }
        }
    }
    for(V3DLONG i=0;i<niz;i++){
        NeuronSWC s=nt.listNeuron.at(i);
        if(s.pn<0){
            if(s.type==1){
                if(somaid>0)
                {
                    cout<<"---------------Error: multiple soma nodes!!!-----------------------"<<endl;
                    return -1;
                }else
                    somaid=i;
            }
            else{
                cout<<"-------------- multiple -1 nodes!!!-----------------------"<<endl;
                return -1;
            }
        }
    }
    return somaid;
}
bool getNodeType(NeuronTree nt,vector<int> & ntype)
{
    /*soma: ntype>=3, branch: ntype=2; tip: ntype=0; internodes: ntype=1
    PS: not have to be a single tree */
    V3DLONG siz=nt.listNeuron.size(); if(!siz) {return false;}
    V3DLONG somaid=get_soma(nt,false);
    //
     for (V3DLONG i=0;i<siz;i++)
         ntype[i]=0;
    if(somaid>=0&&somaid<siz)
        ntype[somaid]=2;
    else
        cout<<"no soma node"<<endl;
    /*1. get the index of nt:     * swc_n -> index */
    QHash <V3DLONG, V3DLONG>  hashNeuron;
    for (V3DLONG i=0;i<siz;i++)
        hashNeuron.insert(nt.listNeuron[i].n,i);
    // 2. get node type: index -> node_type
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = nt.listNeuron.at(i);
        if(s.pn>=0&&hashNeuron.contains(s.pn))
        {
            V3DLONG spn_id=hashNeuron.value(s.pn);
            ntype[spn_id]+=1;
        }
    }
    for (V3DLONG i=0;i<siz;i++)
    {
        if(ntype.at(i)>2&&somaid!=i)
        {
            cout<<"Node "<<nt.listNeuron.at(i).n<<" has "<<ntype.at(i)<<" child"<<endl;
//            return false;
        }
    }
    return true;
}
bool getNodeTips(NeuronTree nt,int &qtips,V3DLONG qindex)
{
    /*for each nodes, get its subtree, get the node type of subtree, count the type*/
    V3DLONG siz=nt.listNeuron.size();
    if(!siz||qindex>=siz||qindex<0) {return false;}
    V3DLONG soma_index=get_soma(nt);
    qtips=0;

    if(soma_index==qindex)
    {
        vector<int> ntype(siz,0);
        if(!getNodeType(nt,ntype))
            return false;
        else
            for (V3DLONG i=0;i<ntype.size();i++)
                if(ntype.at(i)==0)
                    qtips+=1;
    }
    else{
        V3DLONG qid=nt.listNeuron.at(qindex).n;
        NeuronTree qnt=getSubtree(nt,qid);
        vector<int> ntype(qnt.listNeuron.size(),0);
        if(!getNodeType(qnt,ntype))
            return false;
        else
            for (V3DLONG i=0;i<ntype.size();i++)
                if(ntype.at(i)==0)
                    qtips+=1;
    }
    return true;
}
bool get_axonarbor(NeuronTree nt, NeuronTree& nt_out,int arbor_type,float r_thre){
    V3DLONG siz=nt.listNeuron.size();
    if(siz==0) {return false;}
    vector<double> n_subtree_len=get_node_subtree_len_v1(nt,100,1,0);
    vector<int> norder(siz,0);
    if(!getNodeOrder(nt,norder)) {cout<<"Fail to get tree-order"<<endl;return false;}
    cout<<"finish node tree-order generation"<<endl;
    QHash <V3DLONG, V3DLONG>  hashNeuron;
    for(V3DLONG i=0;i<siz;i++)
    {
        hashNeuron.insert(nt.listNeuron[i].n,i);
        nt.listNeuron[i].radius=n_subtree_len[i];
        nt.listNeuron[i].level=norder.at(i);
    }
    float max_r=0; float max_level=0;
    for(V3DLONG i=0;i<nt.listNeuron.size();i++){
        max_r=(nt.listNeuron.at(i).r>max_r)?nt.listNeuron.at(i).r:max_r;
        max_level=(nt.listNeuron.at(i).level>max_level)?nt.listNeuron.at(i).level:max_level;
    }
    cout<<"max level="<<max_level<<endl;
    vector<int> ntype(siz,0);
    if(!getNodeType(nt,ntype))
        return false;
    cout<<"finish node type generation"<<endl;
    //all the candicate nodes
    float order_thre=0;
    V3DLONG nid_index=0;
    QList<V3DLONG> rnids;
    float loop_times=0.0;
    while(rnids.size()==0){
        float order_step_up=0.02;
        for(V3DLONG i=0;i<nt.listNeuron.size();i++)
        {
            NeuronSWC s=nt.listNeuron.at(i);
            if(s.type==2
                    &&s.r>=(r_thre-loop_times*0.05)*max_r
                    &&s.level>=(order_thre-loop_times*order_step_up)*max_level
                   &&ntype.at(i)>=2)
                rnids.append(i);
        }
        loop_times++;
    }
    cout<<"candicated nodes="<<rnids.size()<<endl;
    nid_index=rnids.at(0);
    //find the minimum radius
    for(V3DLONG i=0;i<rnids.size();i++)
        if(nt.listNeuron.at(rnids.at(i)).r<nt.listNeuron.at(nid_index).r)
            nid_index=rnids.at(i);
    NeuronTree nt_mid=getSubtree(nt,nt.listNeuron.at(nid_index).n);
    if(arbor_type==6){
        nt_out=reindexNT(nt_mid);
        //rescale
        for(V3DLONG i=0;i<nt_out.listNeuron.size();i++)
            nt_out.listNeuron[i].r=1;
        return true;
    }
    else if(arbor_type==7){
        //remove long branch
        QList<V3DLONG> rm_long_nodes;
        double tmp_len=0.0;
        {
            //nid_index to parent-branch node
            NeuronSWC s=nt.listNeuron.at(nid_index);
            V3DLONG sid,pid;
            sid=nid_index;
            if(s.pn>0&&hashNeuron.contains(s.pn)){
                pid=hashNeuron.value(s.pn);

                NeuronSWC sp=nt.listNeuron.at(pid);
                while(ntype.at(pid)==1){
                    rm_long_nodes.append(pid);
                    s=sp;
                    if(s.pn<0||!hashNeuron.contains(s.pn))
                        break;
                    pid=hashNeuron.value(s.pn);
                    sp=nt.listNeuron.at(pid);
                }
            }
            tmp_len=n_subtree_len.at(pid)-n_subtree_len.at(sid);
            if(tmp_len<max_r*0.05)
                rm_long_nodes.clear();
        }
        nt_out.listNeuron.clear();
        nt_out.hashNeuron.clear();
        for(V3DLONG i=0;i<siz;i++){
            NeuronSWC s=nt.listNeuron.at(i);
            if(nt_mid.hashNeuron.contains(s.n)||rm_long_nodes.contains(i))
                continue;
            nt_out.listNeuron.append(s);
            nt_out.hashNeuron.insert(s.n,nt_out.listNeuron.size()-1);
        }
        nt_out=reindexNT(nt_out);
        //rescale
        for(V3DLONG i=0;i<nt_out.listNeuron.size();i++)
            nt_out.listNeuron[i].r=1;
        return true;
    }
    else
        return false;
}
bool max_arbor_single_tree(NeuronTree nt,NeuronTree& nt_out){
    nt_out.listNeuron.clear();
    nt_out.hashNeuron.clear();
    V3DLONG siz=nt.listNeuron.size();
    if(!siz){return false;}
    QHash <V3DLONG, V3DLONG>  hashNeuron;
    QList<V3DLONG> roots;
    V3DLONG somaid=-1;
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s=nt.listNeuron.at(i);
        hashNeuron.insert(s.n,i);
        if(s.pn<0){
            if(s.type==1){
                if(somaid>=0)
                {
                    cout<<"---------------Error: multiple soma nodes!!!-----------------------"<<endl;
                    return false;
                }else{
                    somaid=i;
                    roots.append(i);
                }
            }
            else
                roots.append(i);
        }
    }
    //from roots get connected trees
    if(roots.size()==0){cout<<"No roots in this arbor"<<endl;return false;}

    for(V3DLONG r=0;r<roots.size();r++)
    {
        V3DLONG rid=roots.at(r);
        NeuronTree this_nt;
        for (V3DLONG i=0;i<siz;i++){
            NeuronSWC s=nt.listNeuron.at(i);
            if(this_nt.hashNeuron.contains(s.n))
                continue;
            bool to_root=false;
            QList<V3DLONG> clist; clist.append(i);
            if(roots.contains(i)){
                if(i==rid){
                    to_root=true;
                }
                continue;
            }

            //from s to root
            if(hashNeuron.contains(s.pn)){
                V3DLONG spid=hashNeuron.value(s.parent);
                NeuronSWC sp=nt.listNeuron.at(spid);
                while(true){
                    clist.append(spid);
                    if(spid==rid){
                        to_root=true;
                        break;
                    }
                    s=sp;
                    if(!hashNeuron.contains(s.pn)||s.pn<0){
                        break;
                    }
                    spid=hashNeuron.value(s.pn);
                    sp=nt.listNeuron.at(spid);
                }
            }
            //to nt_list
            if(to_root&&clist.size()>0){
                for (V3DLONG c=0;c<clist.size();c++){
                    V3DLONG cindex=clist.at(c);
                    NeuronSWC sc=nt.listNeuron.at(cindex);
                    if(cindex==rid){
                        sc.type=1;sc.pn=-1;
                    }
                    if(this_nt.hashNeuron.contains(sc.n))
                        continue;
                    this_nt.listNeuron.append(sc);
                    this_nt.hashNeuron.insert(sc.n,this_nt.listNeuron.size()-1);
                }
            }
        }
        cout<<r+1<<": tree-size="<<this_nt.listNeuron.size()<<endl;
        if(nt_out.listNeuron.size()<this_nt.listNeuron.size()){
            nt_out.listNeuron.clear();
            nt_out.hashNeuron.clear();
            nt_out.deepCopy(this_nt);
        }
    }
    return true;
}
bool arbor_topo(NeuronTree in_nt,NeuronTree& nt_out)
{
    NeuronTree nt;
    if(!max_arbor_single_tree(in_nt,nt)){cout<<"empty connected tree"<<endl;;return false;}
    V3DLONG siz=nt.listNeuron.size();
    if(!siz){return false;}
    QHash <V3DLONG, V3DLONG>  hashNeuron;
    V3DLONG somaid=get_soma(nt,false);
    for (V3DLONG i=0;i<siz;i++)
        hashNeuron.insert(nt.listNeuron[i].n,i);
    vector<int> ntype(siz,0);
    if(!getNodeType(nt,ntype))
        return false;

    //3. remove internode
    //add soma-node first
    if(somaid>=0&&somaid<siz){
        NeuronSWC soma_node = nt.listNeuron.at(somaid);
        nt_out.listNeuron.append(soma_node);
        nt_out.hashNeuron.insert(soma_node.n,nt_out.listNeuron.size()-1);
    }
    //add tip and branch nodes
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = nt.listNeuron.at(i);
        if(somaid==i)
            continue;
        long outpid=s.pn;
        int ptype=ntype.at(hashNeuron.value(outpid));
        if(ntype[i]==0||ntype[i]>=2)
        {
            //1. for tips to branch-node or soma-node
            //2. for branch-nodes to branch-node or soma-node
            while(ptype==1)
            {
                V3DLONG spn_id=hashNeuron.value(outpid);
                outpid=nt.listNeuron[spn_id].pn;
                ptype=ntype[hashNeuron.value(outpid)];
            }
        }
        else //internode
            continue;
        //update parent id
        s.pn=outpid;
        nt_out.listNeuron.append(s);
        nt_out.hashNeuron.insert(s.n,nt_out.listNeuron.size()-1);
    }
    return true;
}
bool get_arbor(NeuronTree nt, NeuronTree& nt_out,int arbor_type)
{
    V3DLONG siz=nt.listNeuron.size();
    if(siz==0) {return false;}
    V3DLONG soma_index=get_soma(nt,false);
    if(soma_index<0) {return false;}
    //get  the candicated nodes with the same arbor_type
    QList<V3DLONG> candicates,finals;
    finals.append(soma_index);
    if(arbor_type>=2&&arbor_type<=4)
    {
        for(V3DLONG i=0;i<siz;i++)
            if(nt.listNeuron.at(i).type==arbor_type)
                candicates.append(i);
    }
    else if(arbor_type==5)
    {
        for(V3DLONG i=0;i<siz;i++)
            if(nt.listNeuron.at(i).type==3||
                    nt.listNeuron.at(i).type==4)
                candicates.append(i);
    }
    if(candicates.size()==0)
        return false;
    nt_out.listNeuron.clear();nt.hashNeuron.clear();
    if(candicates.size()+1==siz){
        cout<<"all nodes are quested nodes"<<endl;
        nt_out.deepCopy(nt);
        return true;
    }
    else
        cout<<"Candicated nodes="<<candicates.size()+1<<endl;
    QHash <V3DLONG, V3DLONG>  hashNeuron;
    for (V3DLONG i=0;i<siz;i++)
        hashNeuron.insert(nt.listNeuron.at(i).n,i);
    //from candicates to soma
    for(V3DLONG i=0;i<candicates.size();i++)
    {
        V3DLONG cindex=candicates.at(i);
        if(finals.contains(cindex))
            continue;
        QList<V3DLONG> can_tmp;can_tmp.clear();
        can_tmp.append(cindex);

        NeuronSWC s=nt.listNeuron.at(cindex);
        if(hashNeuron.contains(s.pn)){
            V3DLONG pindex=hashNeuron.value(s.pn);
            NeuronSWC sp=nt.listNeuron.at(pindex);
            while(true){
                can_tmp.append(pindex);
                if(pindex==soma_index)
                    break;
                s=sp;
                if(s.pn<0||!hashNeuron.contains(s.pn))
                    break;
                pindex=hashNeuron.value(s.pn);
                sp=nt.listNeuron.at(pindex);
            }
        }
        //single tree check
        for(int c=0;c<can_tmp.size();c++)
            if(!finals.contains(can_tmp.at(c)))
                finals.append(can_tmp.at(c));
    }
    //out

    for(V3DLONG i=0;i<finals.size();i++)
    {
        NeuronSWC s = nt.listNeuron.at(finals.at(i));
        nt_out.listNeuron.append(s);
        nt_out.hashNeuron.insert(s.n,nt_out.listNeuron.size()-1);
    }
    return true;
}
NeuronTree pruning_subtree(NeuronTree nt, int pruning_thre)
{
    NeuronTree nt_out;nt_out.listNeuron.clear();nt_out.hashNeuron.clear();
    V3DLONG siz=nt.listNeuron.size();
    vector<double> n_subtree_len(siz,0.0);
    n_subtree_len=get_node_subtree_len_v1(nt,100,1);
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
bool pruning_subtree(NeuronTree nt, NeuronTree& nt_out,float tip_thre)
{
    nt_out.listNeuron.clear();nt_out.hashNeuron.clear();
    V3DLONG siz=nt.listNeuron.size();
    if(!siz){return false;}
    vector<double> nt_node_u(siz,0.0);
    nt_node_u=get_node_subtree_len_v1(nt);
    //get branch with maximum length
    QList<NeuronSWC>& listNeuron =  nt.listNeuron;
    QHash<int,int>& hashNeuron = nt.hashNeuron;

    SwcTree stree;    stree.initialize(nt);

    int max_branch_index=0;
    double mean_branch_len=0.0;
    double max_branch_len=0.0;
    for(int i=0; i<stree.branchs.size(); i++){
        Branch tb=stree.branchs.at(i);
        double tb_len=tb.get_branch_len(nt);
        mean_branch_len+=tb_len;
        if(max_branch_len<tb_len){
            max_branch_index=i;
            max_branch_len=tb_len;
        }
    }
    mean_branch_len/=stree.branchs.size();
    cout<<"Max br len= "<<max_branch_len<<", mean br len="<<mean_branch_len;
    int nt_tips=0;
    V3DLONG soma_index=get_soma(nt,false);
    if(!getNodeTips(nt,nt_tips,soma_index)){cout<<"Fail to get total tips"<<endl;return false;}

    Branch rm_br=stree.branchs.at(max_branch_index);
    int max_br_tips=0;
    getNodeTips(nt,max_br_tips,hashNeuron.value(rm_br.end_point.n));
    //if max_branch_len>2.68*mean_branch_len & branch_tips< tip_thre, remove this subtree
    cout<<", max_br_tips="<<max_br_tips<<endl;
    cout<<"pruning tip thre="<<tip_thre*nt_tips<<endl;
    if(max_br_tips<tip_thre*nt_tips&&max_branch_len>3*mean_branch_len){
        vector<int> rm_br_indexs = vector<int>();
        rm_br.get_points_of_branch(rm_br_indexs,nt);
        QList<V3DLONG> rm_indexs;rm_indexs.clear();
        for(int r=1;r<rm_br_indexs.size();r++)
            rm_indexs.append(rm_br_indexs.at(r));

        NeuronTree tmp_nt=getSubtree(nt,rm_br.end_point.n);
        //out
        QHash<int,int>& tmp_hN = tmp_nt.hashNeuron;
        cout<<"remove branch size="<<tmp_hN.size()+rm_indexs.size()<<endl;
        for(V3DLONG i=0;i<nt.listNeuron.size();i++)
        {
            NeuronSWC s = nt.listNeuron.at(i);
            if(tmp_hN.contains(s.n)||rm_indexs.contains(i))
                continue;
//
//            if(rm_indexs.contains(i))
//                s.type=4;
//                continue;
            nt_out.listNeuron.append(s);
            nt_out.hashNeuron.insert(s.n,nt_out.listNeuron.size()-1);
        }
        return true;
    }
    else
    {
        cout<<"no need to remove any branches"<<endl;
        return false;
    }
}
bool getNodeOrder(NeuronTree nt,vector<int> & norder)
{
    /*soma order=0
     * Workflow
     * 1. get node type;
     * 2. from one node to soma,count how many branch nodes will be scanned.
     * 3.out
     * PS: neuron tree must have only one soma node
    */
    V3DLONG siz=nt.listNeuron.size(); if(!siz) { return false;}

    QHash <V3DLONG, V3DLONG>  hashNeuron;
    V3DLONG somaid=get_soma(nt);
    if(somaid<0){cout<<"no soma"<<endl;return false;}
    cout<<"Soma index="<<somaid<<endl;
    vector<int> ntype(siz,0);
    if(!getNodeType(nt,ntype))
        return false;
    for (V3DLONG i=0;i<siz;i++)
        hashNeuron.insert(nt.listNeuron[i].n,i);
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
    return true;
}
bool axon_main_path(NeuronTree nt, NeuronTree& nt_out,double len_thre,int tip_thre){
    NeuronTree nt_axon;
    if(!get_arbor(nt,nt_axon,2)){cout<<"Fail to get axon arbor"<<endl;return false;}
    V3DLONG siz=nt_axon.listNeuron.size();
    if(!siz){return false;}

    nt_axon=reindexNT(nt_axon);
    int nor_len=100;
    int nt_axon_tips=0;
    V3DLONG soma_index=get_soma(nt_axon,false);

    if(!getNodeTips(nt_axon,nt_axon_tips,soma_index)){cout<<"Fail to get total tips"<<endl;return false;}
    cout<<"Total tips = "<<nt_axon_tips<<endl;
    vector<double> nt_axon_len=get_node_subtree_len_v1(nt_axon,nor_len,1,0);
    QList<V3DLONG> main_path_nodes;
    for (V3DLONG i=0;i<siz;i++)
    {
        if(nt_axon_len.at(i)>=nor_len*len_thre)
        {
            int stips=0;
            getNodeTips(nt_axon,stips,i);
            if(stips>=tip_thre*nt_axon_tips)
                main_path_nodes.append(i);
        }
    }
    if(!main_path_nodes.size())
        return false;
    for (V3DLONG i=0;i<main_path_nodes.size();i++){
        V3DLONG nindex=main_path_nodes.at(i);
        NeuronSWC s = nt_axon.listNeuron.at(nindex);
        nt_out.listNeuron.append(s);
        nt_out.hashNeuron.insert(s.n,nt_out.listNeuron.size()-1);
    }
    return true;
}
NeuronTree getSubtree(NeuronTree nt,V3DLONG nodeid)
{
    NeuronTree nt_out;
    V3DLONG siz=nt.listNeuron.size();
    if(!siz)
        return nt_out;

    QHash <V3DLONG, V3DLONG>  hashNeuron;
    for (V3DLONG i=0;i<siz;i++)
        hashNeuron.insert(nt.listNeuron[i].n,i);

    long nodeIndex=hashNeuron.value(nodeid);
    //soma node
    V3DLONG somaid=get_soma(nt,false);
    if(somaid<0||!hashNeuron.contains(nodeid)){return nt_out;}
    if(somaid==nodeIndex) {return nt;}
    vector<int> nkept(siz,0); nkept[nodeIndex]=1;
    vector<int> ntype(siz,0);
    getNodeType(nt,ntype);

    if(ntype.at(nodeIndex)!=0) //not tip node
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
NeuronTree reindexNT(NeuronTree nt)
{
    /*if parent node not exist, will set to -1*/
    NeuronTree nt_out_reindex;
    for(V3DLONG i=0;i<nt.listNeuron.size();i++)
    {
        NeuronSWC s = nt.listNeuron.at(i);
        s.pn=(s.pn<0||!nt.hashNeuron.contains(s.pn))?(-1):(nt.hashNeuron.value(s.pn)+1);
        s.n=i+1;
        nt_out_reindex.listNeuron.append(s);
        nt_out_reindex.hashNeuron.insert(s.n,nt_out_reindex.listNeuron.size()-1);
    }
   return nt_out_reindex;
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
    getNodeType(nt,ntype);
    QHash <V3DLONG, V3DLONG>  hashNeuron;
    V3DLONG somaid=get_soma(nt,false);
    for (V3DLONG i=0;i<siz;i++)
        hashNeuron.insert(nt.listNeuron[i].n,i);
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
vector<double> get_node_subtree_len_v1(NeuronTree nt,int normalized_size,double axonRatio, double otherR)
{
    cout<<"---------------in getNodeLength-----------------"<<endl;
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
                }else if(listNeuron.at(temp).type >= 3){
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
                }else if(listNeuron.at(prtIndex).type >= 3){
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
double Branch::get_branch_len(NeuronTree nt)
{
//    vector<int> r_points;
    vector<int> r_points = vector<int>();
    this->get_points_of_branch(r_points,nt);
    double br_len=0.0;
//    cout<<"branch node size="<<r_points.size()<<endl;
    for(int i=0;i<r_points.size()-1;i++)
        br_len+=dis(nt.listNeuron.at(r_points.at(i)),
                    nt.listNeuron.at(r_points.at(i+1)));
//    cout<<"br len="<<br_len<<endl;
    return br_len;
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
    cout<<"initial level"<<endl;
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
