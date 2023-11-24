#include "neuro_morpho_lib.h"
void BranchUnit::get_radius(){
    std::vector<double> brrlist(this->listNode.size(),0.0);
    for(V3DLONG i=0;i<this->listNode.size()-1;i++)
            brrlist[i]=this->listNode.at(i).r;
//    this->radius/=(double)this->listNode.size();
    this->radius=seg_median(brrlist);
}
void BranchUnit::radius_smooth(int half_win){
    /*for smooth radius*/
    V3DLONG bsiz=this->listNode.size();
    if(bsiz>2*half_win+1){
        for(V3DLONG i=half_win;i<bsiz-half_win;i++){
            for(int hi=(-1)*half_win;hi<=half_win;hi++)
                this->listNode[i].r+=this->listNode.at(i+hi).r;
            this->listNode[i].r /= (2*half_win+1);
        }
    }
}
void BranchUnit::bouton_features(int btype,int interb_dist_index,int spatial_nb_index,int topo_nb_dist_index){
    V3DLONG bsiz=this->listNode.size();
    for(V3DLONG i=0;i<bsiz;i++)
    {
        if(this->listNode.at(i).fea_val.at(btype))
        {
            this->boutons+=1;
            this->mean_dist2parent_bouton+=this->listNode.at(i).fea_val.at(interb_dist_index);
            this->mean_spatial_neighbor_boutons+=this->listNode.at(i).fea_val.at(spatial_nb_index);
            this->mean_MINdist2topo_bouton+=this->listNode.at(i).fea_val.at(topo_nb_dist_index);
        }
    }
    if(this->boutons)
    {
        this->mean_dist2parent_bouton/=(double)this->boutons;
        this->mean_spatial_neighbor_boutons/=(double)this->boutons;
        this->mean_MINdist2topo_bouton/=(double)this->boutons;
        this->uniform_bouton_dist=this->pathLength/(double)this->boutons;
    }

}
void BranchUnit::get_features(bool bouton_fea){
    //length, path length,radius

    this->length=dis(this->listNode.at(0),this->listNode.at(this->listNode.size()-1),bouton_fea);
//    cout<<"len="<<this->length<<endl;
    if(this->listNode.size()==2)
        this->pathLength=this->length;
    else
    {
        for(V3DLONG i=0;i<this->listNode.size()-1;i++)
        {
            NeuronSWC snode=this->listNode.at(i);
            NeuronSWC enode=this->listNode.at(i+1);
            this->pathLength+=dis(snode,enode,bouton_fea);
        }
    }
        //radius
//    this->radius_smooth();
    this->get_radius();
    if(bouton_fea)
        this->bouton_features();

}
void BranchTree::get_globalFeatures(){
    /*get: length*/
    if(!this->initialized||this->listBranch.size()==0) {cout<<"Branchtree isn't initialized."<<endl;return;}
    V3DLONG siz=this->listBranch.size();
    vector<int> btype(siz,0); btype=this->getBranchType();

    this->total_length=this->total_path_length=0.0;
    this->tip_branches=this->soma_branches=this->max_branch_level=0;
    this->total_branches=siz;

    for (V3DLONG i=0;i<siz;i++)
    {
        BranchUnit bu = this->listBranch.at(i);
        this->total_length+=bu.length;
        this->total_path_length+=bu.pathLength;
        if(btype[i]==0)
            this->tip_branches+=1;
        if(bu.parent_id<0)
            this->soma_branches+=1;
        this->max_branch_level=(bu.level>this->max_branch_level)?bu.level:this->max_branch_level;
    }
    //get volume related
    double wl,wr,hl,hr,dl,dr;
    wl=hl=dl=MAXVALUE;
    wr=hr=dr=MINVALUE;
    for (V3DLONG i=0;i<this->nt.listNeuron.size();i++)
    {
        NeuronSWC node=this->nt.listNeuron.at(i);
        wl=(node.x<=wl) ? node.x:wl;
        wr=(node.x>wr)? node.x:wr;
        hl=(node.y<=hl) ? node.y:hl;
        hr=(node.y>hr)? node.y:hr;
        dl=(node.z<=dl) ? node.z:dl;
        dr=(node.z>dr)? node.z:dr;
    }
    this->width=wr-wl;
    this->height=hr-hl;
    this->depth=dr-dl;
    this->volume=this->width*this->height*this->depth;
    this->get_volsize();
}
vector< vector<V3DLONG> > BranchTree::get_branch_child_index(){
//    if(!this->initialized||this->listBranch.size()==0) {cout<<"Branchtree isn't initialized."<<endl;return false;}
    V3DLONG siz=this->listBranch.size();
    vector<int> btype(siz,0);
    btype=this->getBranchType();

    vector< vector<V3DLONG> > child_index_list(siz,vector<V3DLONG>());
    for (V3DLONG i=0;i<siz;i++)
    {
        BranchUnit bu = this->listBranch.at(i);
        if(bu.parent_id>0)
        {
            V3DLONG p_index=this->hashBranch.value(bu.parent_id);
            child_index_list[p_index].push_back(i);
        }
    }
    return child_index_list;
}
bool BranchTree::get_branch_angle_io()
{
    if(!this->initialized||this->listBranch.size()==0) {cout<<"Branchtree isn't initialized."<<endl;return false;}
    V3DLONG siz=this->listBranch.size();
    vector<int> btype=this->getBranchType();
    vector< vector<V3DLONG> > child_index_list=this->get_branch_child_index();

    for (V3DLONG i=0;i<siz;i++)
    {
        if(btype.at(i)!=2)
            continue;
        if(child_index_list.at(i).size()!=2) {
            cout<<this->listBranch.at(i).id<<" child branch size: "<<child_index_list.at(i).size()<<endl;
            return false;
        }
        BranchUnit bu = this->listBranch.at(i);
        NeuronSWC bu_snode=bu.listNode.at(bu.listNode.size()-1);
//        NeuronSWC bu_enode=bu.listNode.at(bu.listNode.size()-2);
        NeuronSWC bu_enode=getBranchNearNode(bu,false);
        NeuronSWC bu_renode=bu.listNode.at(0);
        //left branch
        V3DLONG c1=child_index_list.at(i).at(0);
        V3DLONG c2=child_index_list.at(i).at(1);
        V3DLONG lc=c1;
        if((this->listBranch.at(c1).lstips+this->listBranch.at(c1).rstips)<
                (this->listBranch.at(c2).lstips+this->listBranch.at(c2).rstips))
        {
            lc=c2;
            c2=c1;
        }
        BranchUnit lc_bu = this->listBranch.at(lc);
        V3DLONG lci=lc_bu.listNode.size()-1;
        NeuronSWC lc_bu_snode=lc_bu.listNode.at(0);
//        NeuronSWC lc_bu_enode=lc_bu.listNode.at(1);
        NeuronSWC lc_bu_enode=getBranchNearNode(lc_bu,true);
        NeuronSWC lc_bu_renode=lc_bu.listNode.at(lci);
        BranchUnit rc_bu = this->listBranch.at(c2);
        V3DLONG rci=rc_bu.listNode.size()-1;
        NeuronSWC rc_bu_snode=rc_bu.listNode.at(0);
//        NeuronSWC rc_bu_enode=rc_bu.listNode.at(1);
        NeuronSWC rc_bu_enode=getBranchNearNode(rc_bu,true);
        NeuronSWC rc_bu_renode=rc_bu.listNode.at(rci);

        this->listBranch[i].angle_io1=angle_3d(bu_snode,bu_enode,lc_bu_snode,lc_bu_enode);
        this->listBranch[i].angle_io1_remote=angle_3d(bu_snode,bu_renode,lc_bu_snode,lc_bu_renode);
        this->listBranch[i].angle_io2=angle_3d(bu_snode,bu_enode,rc_bu_snode,rc_bu_enode);
        this->listBranch[i].angle_io2_remote=angle_3d(bu_snode,bu_renode,rc_bu_snode,rc_bu_renode);
    }
    return true;
}
NeuronSWC getBranchNearNode(BranchUnit bu, bool near_head,double min_dist){

    /*near_head: chose the node from the direction of head node*/
    V3DLONG busiz=bu.listNode.size();
    int nindex=0;
    NeuronSWC outnode;

    double ndist=0.0;

    if(near_head){
        nindex=busiz-1;
        for(V3DLONG i=0;i<busiz-1;i++){
            ndist+=dis(bu.listNode.at(i),bu.listNode.at(i+1));
            if(ndist>=min_dist)
            {
                nindex=i+1;
                break;
            }
        }
    }
    else{
        for(V3DLONG i=busiz-1;i>0;i--){
            ndist+=dis(bu.listNode.at(i),bu.listNode.at(i-1));
            if(ndist>=min_dist)
            {
                nindex=i-1;
                break;
            }
        }
    }
    outnode=bu.listNode.at(nindex);
    return outnode;
}
bool BranchTree::get_volsize()
{
    if(!this->initialized||this->listBranch.size()==0) {cout<<"Branchtree isn't initialized."<<endl;return false;}
    V3DLONG siz=this->listBranch.size();
    vector<int> btype(siz,0); btype=this->getBranchType();

//    vector< vector<V3DLONG> > child_index_list=this->get_branch_child_index();
    for (V3DLONG i=0;i<siz;i++)
    {
         BranchUnit bu = this->listBranch.at(i);
         double wl,wr,hl,hr,dl,dr;
         wl=hl=dl=MAXVALUE;
         wr=hr=dr=MINVALUE;
        for(int j=0;j<bu.listNode.size();j++)
        {
            NeuronSWC node=bu.listNode.at(j);
            wl=(node.x<=wl) ? node.x:wl;
            wr=(node.x>wr)? node.x:wr;
            hl=(node.y<=hl) ? node.y:hl;
            hr=(node.y>hr)? node.y:hr;
            dl=(node.z<=dl) ? node.z:dl;
            dr=(node.z>dr)? node.z:dr;
        }
        this->listBranch[i].width=wr-wl;
        this->listBranch[i].height=hr-hl;
        this->listBranch[i].depth=dr-dl;
    }
    return true;
}

bool BranchTree::get_branch_child_angle()
{
    /*angle of two child branches
     * 1. get branch type
      * 2. get child index of branch
    */
    if(!this->initialized||this->listBranch.size()==0) {cout<<"Branchtree isn't initialized."<<endl;return false;}
    V3DLONG siz=this->listBranch.size();
    vector<int> btype=this->getBranchType();
    vector< vector<V3DLONG> > child_index_list=this->get_branch_child_index();

    for (V3DLONG i=0;i<siz;i++)
    {
        if(btype.at(i)!=2)
            continue;
        if(child_index_list.at(i).size()!=2) {
            cout<<this->listBranch.at(i).id<<" child branch size: "<<child_index_list.at(i).size()<<endl;
            return false;
        }
        //left branch
        V3DLONG lc=child_index_list.at(i).at(0);
        BranchUnit lc_bu = this->listBranch.at(lc);
        NeuronSWC lc_bu_snode=lc_bu.listNode.at(0);
//        NeuronSWC lc_bu_enode=lc_bu.listNode.at(1);
        NeuronSWC lc_bu_enode=getBranchNearNode(lc_bu,true);
        NeuronSWC lc_bu_renode=lc_bu.listNode.at(lc_bu.listNode.size()-1);

        //right branch
        V3DLONG rc=child_index_list.at(i).at(1);
        BranchUnit rc_bu = this->listBranch.at(rc);
        NeuronSWC rc_bu_snode=rc_bu.listNode.at(0);
//        NeuronSWC rc_bu_enode=rc_bu.listNode.at(1);
        NeuronSWC rc_bu_enode=getBranchNearNode(rc_bu,true);
        NeuronSWC rc_bu_renode=rc_bu.listNode.at(rc_bu.listNode.size()-1);
//        if(int(lc_bu_snode.x)==5850&&int(lc_bu_snode.y)==4670&&int(lc_bu_snode.z)==5573){
//            cout<<"lcx="<<lc_bu_enode.x<<"lcy="<<lc_bu_enode.y<<"lcz="<<lc_bu_enode.z<<endl;
//            cout<<"rcx="<<rc_bu_snode.x<<"rcy="<<rc_bu_snode.y<<"rcz="<<rc_bu_snode.z<<endl;
//            cout<<"rcx="<<rc_bu_enode.x<<"rcy="<<rc_bu_enode.y<<"rcz="<<rc_bu_enode.z<<endl;
//            angle_3d(lc_bu_snode,lc_bu_enode,rc_bu_snode,rc_bu_enode,true);
//        }
        this->listBranch[i].angle=angle_3d(lc_bu_snode,lc_bu_enode,rc_bu_snode,rc_bu_enode);
        this->listBranch[i].angle_remote=angle_3d(lc_bu_snode,lc_bu_renode,
                                                  rc_bu_snode,rc_bu_renode);
    }
    return true;
}
bool BranchTree::get_enhacedFeatures(bool bouton_fea)
{
     /*1. get branch type
      * 2. get child index of branch
      * 3. get enhanced features
      *      lclength,lcpathLength,rclength,rcpathLength;
             lslength,lspathLength,rslength,rspathLength;
             lstips,rstips;
    */
    if(!this->initialized||this->listBranch.size()==0) {cout<<"Branchtree isn't initialized."<<endl;return false;}
    V3DLONG siz=this->listBranch.size();
    vector<int> btype=this->getBranchType();
    vector< vector<V3DLONG> > child_index_list=this->get_branch_child_index();
    NeuronSWC soma;
    for (V3DLONG i=0;i<siz;i++){
        BranchUnit bu = this->listBranch.at(i);
        if(bu.parent_id<0){
            soma=bu.listNode.at(0);
        }
    }
    for (V3DLONG i=0;i<siz;i++){
        BranchUnit bu = this->listBranch.at(i);
        BranchSequence brs;
        this->to_soma_br_seq(i,brs);
        this->listBranch[i].pdist2soma=brs.seqPathLength;
        this->listBranch[i].edist2soma=dis(bu.listNode.at(0),soma);
    }

    for (V3DLONG i=0;i<siz;i++)
    {
        QList<V3DLONG> subtreeBrlist;
        if(btype[i]>0){
            V3DLONG brpid=this->listBranch.at(i).parent_id;
            if(bouton_fea&&brpid>0)
                this->listBranch[i].pboutons=this->listBranch.at(brpid).boutons;
            if(child_index_list.at(i).size()!=2) { cout<<this->listBranch.at(i).id<<" child branch size: "<<child_index_list.at(i).size()<<endl; return false;}
            //left part
            V3DLONG lc_index=child_index_list.at(i).at(0);
            this->listBranch[i].lclength=this->listBranch[lc_index].length;
            if(bouton_fea)
                this->listBranch[i].lcboutons=this->listBranch[lc_index].boutons;
            this->listBranch[i].lcradius=this->listBranch[lc_index].radius;
            this->listBranch[i].lcpathLength=this->listBranch[lc_index].pathLength;
            subtreeBrlist.clear();
            subtreeBrlist=getSubtreeBranches(lc_index);
            for(int sb=0;sb<subtreeBrlist.size();sb++)
            {
                this->listBranch[i].lslength+=this->listBranch[subtreeBrlist.at(sb)].length;
                this->listBranch[i].lspathLength+=this->listBranch[subtreeBrlist.at(sb)].pathLength;
                if(btype[subtreeBrlist.at(sb)]==0)
                    this->listBranch[i].lstips+=1;
            }
            //right part
            V3DLONG rc_index=child_index_list.at(i).at(1);
            if(bouton_fea)
                this->listBranch[i].rcboutons=this->listBranch[rc_index].boutons;
            this->listBranch[i].rclength=this->listBranch[rc_index].length;
            this->listBranch[i].rcradius=this->listBranch[rc_index].radius;
            this->listBranch[i].rcpathLength=this->listBranch[rc_index].pathLength;
            subtreeBrlist.clear();
            subtreeBrlist=getSubtreeBranches(rc_index);
            for(int sb=0;sb<subtreeBrlist.size();sb++)
            {
                this->listBranch[i].rslength+=this->listBranch[subtreeBrlist.at(sb)].length;
                this->listBranch[i].rspathLength+=this->listBranch[subtreeBrlist.at(sb)].pathLength;
                if(btype[subtreeBrlist.at(sb)]==0)
                    this->listBranch[i].rstips+=1;
            }
            //swap left and right according to tip_num
            if(this->listBranch.at(i).lstips<this->listBranch.at(i).rstips)
            {
                std::swap(this->listBranch[i].lcradius,this->listBranch[i].rcradius);
                std::swap(this->listBranch[i].lclength,this->listBranch[i].rclength);
                std::swap(this->listBranch[i].lcpathLength,this->listBranch[i].rcpathLength);
                std::swap(this->listBranch[i].lslength,this->listBranch[i].rslength);
                std::swap(this->listBranch[i].lspathLength,this->listBranch[i].rspathLength);
                std::swap(this->listBranch[i].lstips,this->listBranch[i].rstips);
            }
        }
    }
    return true;
}
bool BranchTree::to_soma_br_seq(V3DLONG inbr_index,BranchSequence & brs)
{
//    BranchSequence brs;
    V3DLONG siz=this->listBranch.size();
    if(!siz||!this->initialized||inbr_index>=siz)
        return false;
    BranchUnit bu = this->listBranch.at(inbr_index);
    brs.seqLength+=bu.length;
    brs.seqPathLength+=bu.pathLength;
    brs.listbr.append(inbr_index);
    if(bu.parent_id>0)
    {
        V3DLONG bupid=this->hashBranch.value(bu.parent_id);
        BranchUnit bup=this->listBranch[bupid];
        while(true)
        {
            brs.seqLength+=bup.length;
            brs.seqPathLength+=bup.pathLength;
            brs.listbr.append(bupid);
            if(bup.parent_id<0)
                break;
            bupid=this->hashBranch.value(bup.parent_id);
            bup=this->listBranch[bupid];
        }
    }
    brs.seqType=this->listBranch[inbr_index].type;
    brs.seqSize=brs.listbr.size();
    return true;
}
bool BranchTree::init_branch_sequence()
{
    if(!this->listBranch.size()||!this->initialized)
        return false;
    //get tip-branch
    V3DLONG siz=this->listBranch.size();
    vector<int> btype(siz,0); btype=this->getBranchType();
    for(V3DLONG i=0;i<this->listBranch.size();i++)
    {
        BranchUnit bu = this->listBranch[i];
        if(btype[i]==0)
        {
            //start from tip-branch
            BranchSequence brs;
            brs.seqLength+=bu.length;
            brs.seqPathLength+=bu.pathLength;
            brs.listbr.append(i);
            if(bu.parent_id>0)
            {
                V3DLONG bupid=this->hashBranch.value(bu.parent_id);
                BranchUnit bup=this->listBranch[bupid];
                while(true)
                {
                    brs.seqLength+=bup.length;
                    brs.seqPathLength+=bup.pathLength;
                    brs.listbr.append(bupid);
                    if(bup.parent_id<0)
                        break;
                    bupid=this->hashBranch.value(bup.parent_id);
                    bup=this->listBranch[bupid];
                }
            }
            brs.seqType=this->listBranch[i].type;
            brs.seqSize=brs.listbr.size();
            this->branchseq.append(brs);
        }
    }
    cout<<"seq size: "<<this->branchseq.size()<<endl;
    return true;
}
bool BranchTree::init(NeuronTree in_nt,bool bouton_fea){
    this->nt.deepCopy(in_nt);
    //neuron tree to branches
    V3DLONG siz=in_nt.listNeuron.size();
    if(!siz) {return false;}
    V3DLONG somaid=get_soma(in_nt,false);
    if(somaid<0){return false;}
    cout<<"soma index="<<somaid<<endl;
    vector<int> ntype(siz,0);
    if(!getNodeType(in_nt,ntype,somaid)){return false;}
    vector<int> norder(siz,0);
    if(!getNodeOrder(in_nt,norder,somaid)){return false;}
    cout<<"start init branch tree"<<endl;
    QList<V3DLONG> br_parent_list;br_parent_list.clear();
    QList<V3DLONG> br_tail_list;br_tail_list.clear();
//    cout<<"stems size="<<ntype.at(soma_index)-2<<endl;
    for(V3DLONG i=0;i<siz;i++)
    {
        //from tip / branch node to branch / soma node.
        NeuronSWC s = in_nt.listNeuron[i];
        if(s.pn<0)
            continue;
        if(ntype[i]==0||ntype[i]==2)
        {
//            cout<<"debug index="<<s.n<<endl;
            QList<NeuronSWC> bu_nodes; bu_nodes.append(s);
            BranchUnit bru;
            bru.level=norder[i];
            bru.id=this->listBranch.size()+1;
            V3DLONG sp_id=in_nt.hashNeuron.value(s.pn);
            int ptype=ntype[sp_id];
            if(ptype==2)
            {
                //this branch doesn't have internode.
                NeuronSWC sp=in_nt.listNeuron[sp_id];
                bu_nodes.append(sp);
            }
            else
            {
                while(true)
                {
                    NeuronSWC sp=in_nt.listNeuron[sp_id];
                    bu_nodes.append(sp);
                    sp_id=in_nt.hashNeuron.value(sp.pn);
                    if(somaid==sp_id)
                    {
                        NeuronSWC sp=in_nt.listNeuron[sp_id];
                        bu_nodes.append(sp);
                        bru.parent_id=-1;
                        break;
                    }
                    if(ntype[sp_id]==2)
                    {
                        NeuronSWC sp=in_nt.listNeuron[sp_id];
                        bu_nodes.append(sp);
                        break;
                    }
                    ptype=ntype[sp_id];
                }
            }

            bru.type=bu_nodes.at(0).type;
            //sort and load into Branch struct
            for(V3DLONG b=bu_nodes.size()-1;b>=0;b--)
            {
                NeuronSWC bu_node=bu_nodes[b];
                bu_node.n=bu_nodes.size()-b;
                bu_node.pn=(b==bu_nodes.size()-1)?(-1):(bu_node.n-1);
                bru.listNode.append(bu_node);
                bru.hashNode.insert(bu_node.n,bru.listNode.size()-1);
            }
            //record the parent id of this branch
            br_tail_list.append(i);
            br_parent_list.append(sp_id);
            bru.get_features(bouton_fea);
            this->listBranch.append(bru);
            this->hashBranch.insert(bru.id,this->listBranch.size()-1);
        }
    }
    //get branch parent id
    for(V3DLONG i=0;i<this->listBranch.size();i++)
    {
        if(this->listBranch[i].parent_id<0)
            continue;
        //parent-node -> index ->
        for(V3DLONG t=0;t<br_tail_list.size();t++)
            if(br_tail_list.at(t)==br_parent_list.at(i))
                this->listBranch[i].parent_id=this->listBranch.at(t).id;
    }
    cout<<"branch size: "<<this->listBranch.size()<<endl;
    return true;
}
QList<V3DLONG> BranchTree::getSubtreeBranches(V3DLONG inbr_index){
    /*retrieval branch also contains in list*/
    V3DLONG siz=this->listBranch.size();
    QList<V3DLONG> subtreeBrlist; subtreeBrlist.clear();
    if(!siz||!this->initialized)
        return subtreeBrlist;
    vector<int> btype=this->getBranchType();
    for(V3DLONG i=0;i<siz;i++)
    {
        //start from tip-branch
        BranchUnit bu = this->listBranch[i];
        if(i==inbr_index) { subtreeBrlist.append(i); continue;}
        if(bu.parent_id<0)
            continue;
        if(btype[i]==0)
        {
            V3DLONG pbr_index=this->hashBranch.value(bu.parent_id);
            BranchUnit pbu=this->listBranch[pbr_index];
            while(true)
            {
                if(pbr_index!=inbr_index)
                {
                    if(pbu.parent_id>0){
                        pbr_index=this->hashBranch.value(pbu.parent_id);
                        pbu=this->listBranch[pbr_index];
                    }
                    else
                        break;
                }
                else {subtreeBrlist.append(i);break;}
            }
        }
    }
    return subtreeBrlist;
}
vector<int> BranchTree::getBranchType()
{
        /*soma-branch, interbranch: ntype=2; tip-branch: ntype=0*/
    V3DLONG siz=this->listBranch.size();
    vector<int> btype(siz,0);
    if(!siz||!this->initialized)
        return btype;

    for (V3DLONG i=0;i<siz;i++)
    {
        BranchUnit bu = this->listBranch[i];
        if(bu.parent_id&&this->hashBranch.contains(bu.parent_id))
        {
            int spn_id=this->hashBranch.value(bu.parent_id);
            btype[spn_id]+=1;
        }
    }
    return btype;
}
bool BranchTree::normalize_branchTree(){
    /*1. get neuron tree length / path_length
     * 2. branch_len /= neuron_len
    */
    V3DLONG siz=this->listBranch.size();
    if(!siz||!this->initialized)
        return false;
    if(this->total_length==0||this->total_path_length==0)
        return false;

    for (V3DLONG i=0;i<siz;i++)
    {
        this->listBranch[i].normalize_len(this->total_length);
        this->listBranch[i].normalize_pathlen(this->total_path_length);
//        if(this->total_branches)
//            this->listBranch[i].normalize_tip(this->total_branches);
    }
    return true;
}
BranchTree readBranchTree_file(const QString& filename)
{
    BranchTree bt;
    QFile brfile(filename);
    if (! brfile.open(QIODevice::ReadOnly | QIODevice::Text))
        return bt;
    else
    {
        int linestate=-1;BranchUnit bu;
        while (! brfile.atEnd())
        {
            char _buf[1000], *buf;
            brfile.readLine(_buf, sizeof(_buf));
            for (buf=_buf; (*buf && *buf==' '); buf++); //skip space
            if (buf[0]=='#')
            {
                 if (buf[1]=='B'&&buf[2]=='R'&&buf[3]=='S'&&buf[4]=='T'&&buf[5]=='A'
                         &&buf[6]=='R'&&buf[7]=='T')
                 {
                     //start a new branch ,state=0
                     linestate+=1;
                 }
                 else if(buf[1]=='#'&&buf[2]=='F'&&buf[3]=='e'&&buf[4]=='a'&&buf[5]=='t'
                         &&buf[6]=='u'&&buf[7]=='r'&&buf[8]=='e'&&buf[9]=='s')
                 {
                     //start to get branch features,state=1
                     linestate+=1;
                 }
                 else if(buf[1]=='#'&&buf[2]=='N'&&buf[3]=='o'&&buf[4]=='d'&&buf[5]=='e'&&buf[6]=='s')
                 {
                     //start to get branch nodes,state=2
                     linestate+=1;
                 }
                 else if(buf[1]=='B'&&buf[2]=='R'&&buf[3]=='E'&&buf[4]=='N'&&buf[5]=='D')
                 {
                     //end this branch
                     linestate=-1;
                     bt.listBranch.append(bu);
                     continue;
                 }
                 else
                     continue;
            }
            if(linestate==0)
            {
                bu.listNode.clear();bu.hashNode.clear();
            }
            else if(linestate==1)
            {
                QStringList qsl = QString(buf).trimmed().split(",");
                if (qsl.size()<6)   continue;
                bu.id=qsl[0].toLong();
                bu.parent_id=qsl[1].toLong();
                bu.type=qsl[2].toInt();
                bu.level=qsl[3].toInt();
                bu.length=qsl[4].toDouble();
                bu.pathLength=qsl[5].toDouble();
            }
            else if(linestate==2)
            {
                NeuronSWC S;
                QStringList qsl = QString(buf).trimmed().split(",");
                if (qsl.size()<7)   continue;
                S.n = qsl[0].toInt();
                S.type = qsl[1].toInt();
                S.x = qsl[2].toFloat();
                S.y = qsl[3].toFloat();
                S.z = qsl[4].toFloat();
                S.r = qsl[5].toFloat();
                S.pn = qsl[6].toInt();
                bu.listNode.append(S);
                bu.hashNode.insert(S.n,bu.listNode.size()-1);
            }
            else
                continue;
        }
    }
    bt.initialized=true;
    bt.init_branch_sequence();
    return bt;
}
bool writeBranchTree_file(const QString& filename, const BranchTree& bt,bool enhanced)
{
    /*File Format:
                 * #BRSTART
                 * ##Features or enhanced features
                 * ###id,parent_id,type,level,length,pathLength
                 * ##Nodes
                 * ###n,type,x,y,z,radius,parent
                 * #BREND
    */
    if (filename.isEmpty()||bt.listBranch.size()==0)
        return false;
    QFile tofile(filename);
    if(tofile.exists())
        cout<<"File overwrite to "<<filename.toStdString()<<endl;
    QString confTitle="#This file is used for recording all the branches in a neuron tree (by shengdian).\n";
    QString brstart="#BRSTART\n"; QString brend="#BREND\n";
    QString brfeatures="##Features\n";
    QString brfHead="#Fhead: id,parent_id,type,level,length,pathLength\n";
    if(enhanced)
        brfHead="#Fhead: id,parent_id,type,level,length,pathLength"
                ",lclength,lcpathLength,lslength,lspathLength,lstips"
                ",rclength,rcpathLength,rslength,rspathLength,rstips\n";
    QString brnodes="##Nodes\n";
    QString brnHead="#Nhead:n,type,x,y,z,radius,parent\n";
    if(tofile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        //title
        tofile.write(confTitle.toAscii());
        tofile.write(brfHead.toAscii());
        tofile.write(brnHead.toAscii());
        //inside for each branch
        for(V3DLONG i=0;i<bt.listBranch.size();i++)
        {
            BranchUnit bu = bt.listBranch[i];
            tofile.write(brstart.toAscii());
            tofile.write(brfeatures.toAscii());
            QString brf=QString::number(bu.id);
            brf+=(","+QString::number(bu.parent_id));
            brf+=(","+QString::number(bu.type));
            brf+=(","+QString::number(bu.level));
            brf+=(","+QString::number(bu.length));
            if(enhanced){
                brf+=(","+QString::number(bu.pathLength));
                brf+=(","+QString::number(bu.lclength));
                brf+=(","+QString::number(bu.lcpathLength));
                brf+=(","+QString::number(bu.lslength));
                brf+=(","+QString::number(bu.lspathLength));
                brf+=(","+QString::number(bu.lstips));
                brf+=(","+QString::number(bu.rclength));
                brf+=(","+QString::number(bu.rcpathLength));
                brf+=(","+QString::number(bu.rslength));
                brf+=(","+QString::number(bu.rspathLength));
                brf+=(","+QString::number(bu.rstips)+"\n");
            }
            else
                brf+=(","+QString::number(bu.pathLength)+"\n");
            tofile.write(brf.toAscii());

            tofile.write(brnodes.toAscii());
            for(int j=0;j<bu.listNode.size();j++)
            {
                NeuronSWC s=bu.listNode[j];
                QString brn=(QString::number(s.n));
                brn+=(","+QString::number(s.type));
                brn+=(","+QString::number(s.x));
                brn+=(","+QString::number(s.y));
                brn+=(","+QString::number(s.z));
                brn+=(","+QString::number(s.r));
                brn+=(","+QString::number(s.parent)+"\n");
                tofile.write(brn.toAscii());
            }
            tofile.write(brend.toAscii());
        }
        tofile.close();
        return true;
    }
    return false;
}
bool writeBranchSequence_file(const QString& filename, const BranchTree& bt,bool enhanced)
{
    /*File Format:
     *  (from soma to tip branch)
                 * #BRSSTART
                 * ##(soma-branch) id,parent_id,type,level,length,pathLength
                 * ...
                 * ##(tip-branch) id,parent_id,type,level,length,pathLength
                 * #BRSEND
    */
    if (filename.isEmpty()||bt.listBranch.size()==0)
        return false;
    QFile tofile(filename);
    if(tofile.exists())
        cout<<"File overwrite to "<<filename.toStdString()<<endl;
    QString confTitle="#This file is used for recording all the branch sequences in a neuron tree (by shengdian).\n";
    QString brsstart="#BRSSTART\n"; QString brsend="#BRSEND\n";
    QString brfHead="#Fhead: id,parent_id,type,level,length,pathLength\n";
    if(enhanced)
        brfHead="#Fhead: id,parent_id,type,level,length,pathLength"
                ",lclength,lcpathLength,lslength,lspathLength,lstips"
                ",rclength,rcpathLength,rslength,rspathLength,rstips\n";
    if(tofile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        //title
        tofile.write(confTitle.toAscii());
        tofile.write(brfHead.toAscii());
        //inside for each branch
        for(V3DLONG i=0;i<bt.branchseq.size();i++)
        {
            tofile.write(brsstart.toAscii());
            BranchSequence brs=bt.branchseq.at(i);
            for(int j=brs.listbr.size()-1;j>=0;j--)
            {
                V3DLONG bid=brs.listbr.at(j);
                BranchUnit bu = bt.listBranch[bid];
                QString brf=QString::number(bu.id);
                brf+=(","+QString::number(bu.parent_id));
                brf+=(","+QString::number(bu.type));
                brf+=(","+QString::number(bu.level));
                brf+=(","+QString::number(bu.length));
                if(enhanced){
                    brf+=(","+QString::number(bu.pathLength));
                    brf+=(","+QString::number(bu.lclength));
                    brf+=(","+QString::number(bu.lcpathLength));
                    brf+=(","+QString::number(bu.lslength));
                    brf+=(","+QString::number(bu.lspathLength));
                    brf+=(","+QString::number(bu.lstips));
                    brf+=(","+QString::number(bu.rclength));
                    brf+=(","+QString::number(bu.rcpathLength));
                    brf+=(","+QString::number(bu.rslength));
                    brf+=(","+QString::number(bu.rspathLength));
                    brf+=(","+QString::number(bu.rstips)+"\n");
                }
                else
                    brf+=(","+QString::number(bu.pathLength)+"\n");
                tofile.write(brf.toAscii());
            }
            tofile.write(brsend.toAscii());
        }
        tofile.close();
        return true;
    }
    return false;
}
bool branchTree2NeuronTree(BranchTree inbt, NeuronTree &outnt){
    /*1, assign NeuronTree node id to node in BranchTree;
     * 2, make connection
    */
    if (inbt.listBranch.size()==0)
        return false;
    V3DLONG nid=1;
    for(V3DLONG i=0;i<inbt.listBranch.size();i++)
    {
        BranchUnit bu = inbt.listBranch.at(i);
        for(V3DLONG n=0;n<bu.listNode.size();n++)
        {
            inbt.listBranch[i].listNode[n].n=nid;
            nid++;
        }
    }
    for(V3DLONG i=0;i<inbt.listBranch.size();i++)
    {
        BranchUnit bu = inbt.listBranch.at(i);
        V3DLONG bupid=bu.parent_id;
        if(bupid<0)
            continue;
        BranchUnit bup=inbt.listBranch.at(bupid);
        inbt.listBranch[i].listNode[1].pn=bup.listNode.at(bup.listNode.size()-1).n;
    }
    outnt.listNeuron.clear();
    outnt.hashNeuron.clear();
    for(V3DLONG i=0;i<inbt.listBranch.size();i++)
    {
        BranchUnit bu = inbt.listBranch.at(i);
        for(V3DLONG n=0;n<bu.listNode.size();n++)
        {
            if(bu.parent_id<0&&n==0)
                continue;
            NeuronSWC s=bu.listNode.at(n);
            outnt.listNeuron.append(s);
            outnt.hashNeuron.insert(s.n,outnt.listNeuron.size()-1);
        }
    }
    return true;
}
NeuronTree to_topology_tree(NeuronTree nt)
{
    /*1. get tip, branch and soma nodes;
     * 2. remove internodes
     * 3. reindex
     * 3. save
    */
    NeuronTree nt_out;
    if(!nt.listNeuron.size())
        return nt_out;
    V3DLONG siz=nt.listNeuron.size();

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
    std::vector<int> ntype(siz,0);
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
    //3. remove internode
    //add soma-node first
    NeuronSWC soma_node = nt.listNeuron[somaid];
    nt_out.listNeuron.append(soma_node);
    nt_out.hashNeuron.insert(soma_node.n,nt_out.listNeuron.size()-1);
    //add tip and branch nodes
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = nt.listNeuron[i];
        long outpid=s.pn;
        int ptype=ntype[hashNeuron.value(outpid)];
        if(ntype[i]==0||ntype[i]==2)
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
    //pruning
//    NeuronTree nt_out_pruning=tip_branch_pruning(nt_out,0);
    //reindex
//     NeuronTree nt_out_reindex=reindexNT(nt_out);
//    return nt_out_reindex;
    return nt_out;
}
void scale_nt_coor(NeuronTree& nt,float scale_xy,float scale_z){
    V3DLONG siz=nt.listNeuron.size();
    for(V3DLONG i=0;i<siz;i++){
        nt.listNeuron[i].x*=scale_xy;
        nt.listNeuron[i].y*=scale_xy;
        nt.listNeuron[i].z*=scale_z;
    }
}
void scale_nt_radius(NeuronTree& nt,float rs)
{
    V3DLONG siz=nt.listNeuron.size();
    for(V3DLONG i=0;i<siz;i++){
        nt.listNeuron[i].r*=rs;
    }
}
bool getNodeOrder(NeuronTree nt,vector<int> & norder,V3DLONG somaid)
{
    /*soma order=0
     * Workflow
     * 1. get node type;
     * 2. from one node to soma,count how many branch nodes will be scanned.
     * 3.out
     * PS: neuron tree must have only one soma node
    */
    V3DLONG siz=nt.listNeuron.size();
    if(!siz){return false;}
    if(somaid<0) {somaid=get_soma(nt);}
    if(somaid<0) {return false;}

    QHash <V3DLONG, V3DLONG>  hashNeuron; hashNeuron.clear();

    vector<int> ntype(siz,0);
    if(!getNodeType(nt,ntype,somaid)){return false;}
    for (V3DLONG i=0;i<siz;i++)
    {
        hashNeuron.insert(nt.listNeuron[i].n,i);
        if(ntype.at(i)>2&&somaid!=i)
            return false;
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
    return true;
}
bool getNodeType(NeuronTree nt,vector<int> & ntype,V3DLONG somaid)
{
    /*soma: ntype>=3, branch: ntype=2; tip: ntype=0; internodes: ntype=1
    PS: not have to be a single tree */
    V3DLONG siz=nt.listNeuron.size();
    if(!siz){return false;}
    if(somaid<0){somaid=get_soma(nt);}
    if(somaid<0){return false;}
    ntype[somaid]=2;
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
    return true;
}
NeuronTree reindexNT(NeuronTree nt, V3DLONG somaid)
{
    /*if parent node not exist, will set to -1*/
    NeuronTree nt_out_reindex;
//    if(somaid>=0){
//        cout<<"Set soma index =1"<<endl;
//        if(somaid>=nt.listNeuron.size()){
//            cout<<"Soma index Error"<<endl;
//            return nt_out_reindex;
//        }
//        NeuronSWC snode=nt.listNeuron.at(somaid);
//        snode.n=1;
//        snode.type=1;
//        snode.parent=-1;
//        nt_out_reindex.listNeuron.append(snode);
//        nt_out_reindex.hashNeuron.insert(snode.n,nt_out_reindex.listNeuron.size()-1);
//        for(V3DLONG i=0;i<nt.listNeuron.size();i++)
//        {
//            if(somaid==i)
//                continue;
//            NeuronSWC s = nt.listNeuron.at(i);
//            s.n=i+2;
////            pid=nt.hashNeuron.value(s.pn);
////            s.pn=
//            s.pn=nt.hashNeuron.value(s.pn)+2;
////            s.n=i+1;
//            nt_out_reindex.listNeuron.append(s);
//            nt_out_reindex.hashNeuron.insert(s.n,nt_out_reindex.listNeuron.size()-1);
//        }
//    }
    if(true){
        for(V3DLONG i=0;i<nt.listNeuron.size();i++)
        {
            NeuronSWC s = nt.listNeuron.at(i);
            s.pn=(s.pn<0||!nt.hashNeuron.contains(s.pn))?(-1):(nt.hashNeuron.value(s.pn)+1);
            s.n=i+1;
            nt_out_reindex.listNeuron.append(s);
            nt_out_reindex.hashNeuron.insert(s.n,nt_out_reindex.listNeuron.size()-1);
        }
    }
   return nt_out_reindex;
}
double getNT_len(NeuronTree nt,float *res)
{
    if(res[0]<=0||res[1]<=0||res[2]<=0){
        res[0]=res[1]=res[2]=1.0;
    }
    double out_len=0.0;
    V3DLONG siz=nt.listNeuron.size();
    if(!siz)
        return out_len;
     for (V3DLONG i=0;i<siz;i++)
     {
          NeuronSWC s = nt.listNeuron[i];
          if(s.pn>0&&nt.hashNeuron.contains(s.pn))
          {
              V3DLONG spid=nt.hashNeuron.value(s.pn);
              NeuronSWC sp=nt.listNeuron[spid];
              out_len+=sqrt
                      (res[0]*res[0]*(s.x-sp.x)*(s.x-sp.x)+
                      res[1]*res[1]*(s.y-sp.y)*(s.y-sp.y)+
                      res[2]*res[2]* (s.z-sp.z)*(s.z-sp.z));
          }
     }
    return out_len;
}
/*swc processing*/
V3DLONG get_soma(NeuronTree & nt,bool connect, bool soma_typed){
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
                        nt.listNeuron[i].pn=sj.n;
                }
            }
        }
    }
//   cout<<"checking start"<<endl;
    for(V3DLONG i=0;i<niz;i++){
        NeuronSWC s=nt.listNeuron.at(i);
        if(s.pn<0){
            if(soma_typed){
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
            else{
                // soma type is not defined
                if(somaid>0)
                {
                    cout<<"---------------Error: multiple soma nodes!!!-----------------------"<<endl;
                    return -1;
                }else
                    somaid=i;
            }
        }
    }
    return somaid;
}
QList<NeuronTree> nt_2_trees(NeuronTree nt){
    /*if parent=-1 or parent not exist, this is a root node;
    */
    QList<NeuronTree> nts;
    V3DLONG niz=nt.listNeuron.size();
    if(niz<=0) return nts;

    //1. get roots' id
    QList<V3DLONG> nt_roots;    QVector<V3DLONG> nodes_tree_index(niz,-1);
    for(V3DLONG i=0;i<niz;i++){
        NeuronSWC s=nt.listNeuron.at(i);
        if(s.pn<0&&!nt.hashNeuron.contains(s.pn)){
            nodes_tree_index[i]=nt_roots.size();
            nt_roots.append(i);
        }
    }
        //2, classification of nodes
    if(nt_roots.size()==1)
        nts.append(nt);
    else{
        for(V3DLONG i=0;i<niz;i++){
            NeuronSWC s=nt.listNeuron.at(i);
            if(nt_roots.contains(i)||nodes_tree_index.at(i)>=0)
                continue;
            V3DLONG pid=nt.hashNeuron.value(s.pn);
            NeuronSWC sp=nt.listNeuron.at(pid);
            QList<V3DLONG> scan_nodes; scan_nodes.clear();
            scan_nodes.append(i);
            while(!nt_roots.contains(pid)){
                scan_nodes.append(pid);
                pid=nt.hashNeuron.value(sp.pn);
                sp=nt.listNeuron.at(pid);
            }
            for(V3DLONG is=0;is<scan_nodes.size();is++)
                nodes_tree_index[scan_nodes.at(is)]=nt_roots.indexOf(pid);;
        }
        //3. get nt trees
        for(int t=0;t<nt_roots.size();t++){
            NeuronTree nt_tree;
            nt_tree.listNeuron.clear();
            nt_tree.hashNeuron.clear();
            for(V3DLONG i=0;i<niz;i++){
                if(nodes_tree_index.at(i)==t){
                    NeuronSWC s=nt.listNeuron.at(i);
                    nt_tree.listNeuron.append(s);
                    nt_tree.hashNeuron.insert(s.n,nt_tree.listNeuron.size()-1);
                }
            }
            cout<<"tree size="<<nt_tree.listNeuron.size()<<endl;
            nts.append(nt_tree);
        }
    }
    return nts;
}
bool loop_checking(NeuronTree nt){
    V3DLONG siz=nt.listNeuron.size();
    if(!siz) {return true;}
    QHash <V3DLONG, V3DLONG>  hashNeuron;
    for (V3DLONG i=0;i<siz;i++)
        hashNeuron.insert(nt.listNeuron.at(i).n,i);

    QVector<V3DLONG> scanned(siz,0);
    int loop_count=0;
    for (V3DLONG i=0;i<siz;i++){
        NeuronSWC s = nt.listNeuron.at(i);
        if(scanned.at(i)>0)
            continue;
        scanned[i]=1;
        if(s.pn>0&&hashNeuron.contains(s.pn)){
            QList<V3DLONG> snodes;
            snodes.clear();
            snodes.append(i);
            V3DLONG pid=hashNeuron.value(s.pn);
            NeuronSWC sp=nt.listNeuron.at(pid);
            while(true){
                scanned[pid]=1;
                snodes.append(pid);
                if(s.n==sp.n)
                {
                    loop_count++;
                    for(int s=0;s<snodes.size();s++)
                        scanned[snodes.at(s)]=loop_count+1;
                    cout<<"Loop at node "<<s.n<<endl;
                    break;
                }
                if(sp.pn<=0 || !hashNeuron.contains(sp.pn))
                    break;
                pid=hashNeuron.value(sp.pn);
                sp=nt.listNeuron.at(pid);
            }
        }
    }
    if(loop_count){
        cout<<"Total loop="<<loop_count<<endl;
        return true;
    }
    cout<<"no loop"<<endl;
    return false;
}
bool multi_bifurcations_checking(NeuronTree nt, QList<CellAPO> & out_3bifs,V3DLONG somaid){
    V3DLONG siz=nt.listNeuron.size(); if(!siz) {return true;}
    if(somaid<0)
        somaid=get_soma(nt,true);
    if(somaid<0){return true;}
    vector<int> ntype(siz,0);
    if(!getNodeType(nt,ntype,somaid)){return true;}
    //s1
    QList<V3DLONG> bifur_idlist; bifur_idlist.clear();
    for (V3DLONG i=0;i<siz;i++)
        if(somaid!=i&&ntype.at(i)>2)
            bifur_idlist.append(i);
    if(bifur_idlist.size()){
        cout<<"# of multiple bifurcations ="<<bifur_idlist.size()<<endl;
        out_3bifs.clear();
        for(V3DLONG i=0;i<bifur_idlist.size();i++){
            NeuronSWC bif_node=nt.listNeuron.at(bifur_idlist.at(i));
            CellAPO bif_marker;
            bif_marker.n=bif_node.n;
            bif_marker.x=bif_node.x;
            bif_marker.y=bif_node.y;
            bif_marker.z=bif_node.z;
            bif_marker.color.r=0;
            bif_marker.color.g=0;
            out_3bifs.append(bif_marker);
        }
        return true;
    }
    cout<<"No multiple bifurcations"<<endl;
    return false;
}
bool three_bif_decompose(NeuronTree& in_nt,V3DLONG bif_child_id,V3DLONG somaid)
{
    //bif_child_id: first child node of a 3 bifurcation
    // for the input multiple bifurcations, insert a new parent node and set as bif seg parent

    V3DLONG siz=in_nt.listNeuron.size();
    if(!siz || bif_child_id >=siz) {return false;}
    if(somaid<0){somaid=get_soma(in_nt,true);}
    if(somaid<0){return false;}
    vector<int> ntype(siz,0);
    if(!getNodeType(in_nt,ntype,somaid)){return false;}

    //s1: check is there a 3 bifurcation
    NeuronSWC bifseg_head=in_nt.listNeuron.at(bif_child_id);
//    cout<<"Node total size="<<siz<<endl;
    if(bifseg_head.pn<0||bifseg_head.pn==somaid){cout<<"branching at -1 node"<<endl;return false;}
    if(!in_nt.hashNeuron.contains(bifseg_head.pn)){cout<<"invalid 3 bifurcation"<<endl;return false;}
    V3DLONG pn_id=in_nt.hashNeuron.value(bifseg_head.pn);
    NeuronSWC bifnode=in_nt.listNeuron.at(pn_id);
    if(ntype.at(pn_id)<=2){cout<<"no 3 bifurcation at "<<bifnode.n<<endl;return false;}
    if(pn_id==somaid){
        cout<<"set to stem"<<endl;
        in_nt.listNeuron[bif_child_id].pn=in_nt.listNeuron.at(somaid).n;
        return true;
    }
    if(bifnode.pn<0){cout<<"multiple -1 nodes"<<endl;return false;}
//    cout<<"start insert"<<endl;
    NeuronSWC insertnode=bifnode;
    if(in_nt.hashNeuron.contains(bifnode.pn))
    {
        V3DLONG ppn_id=in_nt.hashNeuron.value(bifnode.pn);
        NeuronSWC bifnode_p=in_nt.listNeuron.at(ppn_id);
        insertnode.x=float((bifnode.x+bifnode_p.x)/float(2.0));
        insertnode.y=float((bifnode.y+bifnode_p.y)/float(2.0));
        insertnode.z=float((bifnode.z+bifnode_p.z)/float(2.0));
    }
    else{
        cout<<"No parent node ???"<<endl;
        return false;
    }
    insertnode.n=in_nt.listNeuron.size();
    while(true){
        if(!in_nt.hashNeuron.contains(insertnode.n))
            break;
        insertnode.n+=1;
    }
    cout<<"New bifurcation node id="<<insertnode.n<<endl;
    in_nt.listNeuron[pn_id].pn=insertnode.n;
    in_nt.listNeuron[bif_child_id].pn=insertnode.n;
    in_nt.listNeuron.append(insertnode);
    in_nt.hashNeuron.insert(insertnode.n,in_nt.listNeuron.size()-1);
//    cout<<"Node total size="<<in_nt.listNeuron.size()<<endl;
    return true;
}
vector<V3DLONG> child_node_indexes(NeuronTree in_nt, V3DLONG query_node_index)
{
    vector<V3DLONG> out_index;
    V3DLONG siz=in_nt.listNeuron.size();
    if(!siz || query_node_index >= siz) {return out_index;}
    NeuronSWC qnode=in_nt.listNeuron.at(query_node_index);
    for (V3DLONG i=0;i<siz;i++){
        NeuronSWC s=in_nt.listNeuron.at(i);
        if(qnode.n==s.pn && s.n!=qnode.n){
            out_index.push_back(i);
        }
    }
//    cout<<"child size="<<out_index.size()<<endl;
    return out_index;
}
bool multi_bifurcations_processing(NeuronTree& in_nt,V3DLONG somaid)
{
    //s1. detect three bifurcation points
    //s2. move one of the branch to the parent of bifurcation node
    //process bifurcation nodes one by one

    V3DLONG siz=in_nt.listNeuron.size();
    if(!siz) {return false;}
//    cout<<"Node raw size="<<siz<<endl;
    if(somaid<0){somaid=get_soma(in_nt);}
    if(somaid<0){return false;}
    vector<int> ntype(siz,0);
    if(!getNodeType(in_nt,ntype,somaid)){return false;}
//    cout<<"here"<<endl;
    //s1
    QList<V3DLONG> bifur_idlist; bifur_idlist.clear();
    for (V3DLONG i=0;i<siz;i++)
    {
        if((somaid!=i)&&ntype.at(i)>=3)
            bifur_idlist.append(i);
    }
    cout<<"#3-bifs="<<bifur_idlist.size()<<endl;
    //s2
    if(bifur_idlist.size()){
        for(int b=0;b<bifur_idlist.size();b++)
        {
            V3DLONG bifur_index=bifur_idlist.at(b);
//            cout<<"3bifs id="<<in_nt.listNeuron.at(bifur_index).n<<endl;
            vector<V3DLONG> bif_child_index=child_node_indexes(in_nt,bifur_index);
            if(bif_child_index.size()>=3){
                for(int c=2;c<bif_child_index.size();c++)
                {
                    V3DLONG decomposed_child_index=bif_child_index.at(c);
//                    cout<<"processed child id ="<<in_nt.listNeuron.at(decomposed_child_index).n<<endl;
                    if(!three_bif_decompose(in_nt,decomposed_child_index,somaid))
                        return false;
                }
            }
        }
//        cout<<"Node processed size="<<in_nt.listNeuron.size()<<endl;
        //check
        vector<int> new_ntype(in_nt.listNeuron.size(),0);
        if(!getNodeType(in_nt,new_ntype,somaid)){return false;}
        for (V3DLONG i=0;i<in_nt.listNeuron.size();i++)
        {
            if(somaid!=i&&new_ntype.at(i)>2){
                cout<<"3bifs id="<<in_nt.listNeuron.at(i).n<<endl;
//                cout<<new_ntype.at(i)<<" childs,";
                cout<<"bifurcation id="<<in_nt.listNeuron.at(i).n<<endl;
                return false;
            }
        }
    }
    return true;
}
NeuronTree tip_branch_pruning(NeuronTree nt, float in_thre)
{
    /*1. get tip, branch and soma nodes;
     * 2. from tip nodes, get tip-branch
     * 3. pruning
     * 4. save
    */
    NeuronTree nt_out;
    V3DLONG siz=nt.listNeuron.size(); if(!siz) {return nt_out;}
    QHash <V3DLONG, V3DLONG>  hashNeuron;hashNeuron.clear();
    for (V3DLONG i=0;i<siz;i++)
        hashNeuron.insert(nt.listNeuron[i].n,i);

    std::vector<int> ntype(siz,0);  getNodeType(nt,ntype);
    std::vector<int> nkept(siz,1);

    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = nt.listNeuron.at(i);
        std::vector<V3DLONG> nscanned;
        nscanned.clear();
        if(ntype.at(i)==0&&s.pn&&hashNeuron.contains(s.pn))
        {
            nscanned.push_back(i);
            //distance computing
            V3DLONG spn_id=hashNeuron.value(s.pn);
            NeuronSWC spn=nt.listNeuron.at(spn_id);
            double tipbranch_dist=0.0;
            while(true)
            {
                tipbranch_dist+=sqrt((spn.x-s.x)*(spn.x-s.x)+
                                                        (spn.y-s.y)*(spn.y-s.y)+
                                                        (spn.z-s.z)*(spn.z-s.z));
                if(ntype.at(spn_id)>1) {break;}
                nscanned.push_back(spn_id);
                s=spn;
                if(s.pn<0||!hashNeuron.contains(s.pn)){
                    nscanned.clear(); tipbranch_dist+=(in_thre+1);break;
                }
                spn_id=hashNeuron.value(s.pn);
                spn=nt.listNeuron.at(spn_id);
            }
            if(tipbranch_dist<=in_thre)
            {
                //remove this tip-branch
                cout<<"tip branch len="<<tipbranch_dist;
                cout<<",small tip-branch: "<<nt.listNeuron.at(i).n<<endl;

                for(V3DLONG n=0;n<nscanned.size();n++)
                    nkept[nscanned.at(n)]=0;
            }
        }
        else //internode
            continue;
    }
    //save
    for (V3DLONG i=0;i<siz;i++)
    {
        if(nkept[i]!=0)
        {
            NeuronSWC s = nt.listNeuron[i];
            nt_out.listNeuron.append(s);
            nt_out.hashNeuron.insert(s.n,nt_out.listNeuron.size()-1);
        }
    }
    cout<<"pruning nodes from tip-pruning="<<nt.listNeuron.size()-nt_out.listNeuron.size()<<endl;
    return nt_out;
}
NeuronTree node_interpolation(NeuronTree nt,int Min_Interpolation_Pixels,bool sort_index){
    cout<<"linear interpolation of neuron tree"<<endl;
    V3DLONG siz = nt.listNeuron.size(); if(!siz) return nt;

    V3DLONG max_index=siz;
    QList<NeuronSWC> listNeuron =  nt.listNeuron;
    QHash <V3DLONG, V3DLONG>  hashNeuron;
    for(V3DLONG i=0;i<siz;i++){
        hashNeuron.insert(listNeuron[i].n,i);
        max_index=MAX(max_index,listNeuron.at(i).n);
    }
    //step2
    QList <NeuronSWC> nt_out_listNeuron;
    V3DLONG new_node_count=0;
    for (V3DLONG i=0;i<listNeuron.size();i++)
    {
        NeuronSWC s = listNeuron.at(i);
        if(s.parent>0&&hashNeuron.contains(s.parent))
        {
            V3DLONG pid=hashNeuron.value(s.parent);
            NeuronSWC sp=listNeuron.at(pid);
            double cp_dist=dis(s,sp);
            double Min_Interpolation_Pixels_dist=sqrt(long(Min_Interpolation_Pixels*Min_Interpolation_Pixels));
            int interpolate_times=int(cp_dist/Min_Interpolation_Pixels_dist);

            if(interpolate_times==1)
            {
                NeuronSWC s_interpolated=s;
                s_interpolated.n=max_index+1;
                //one node at the center of p and sp
                s_interpolated.x=(sp.x+s.x)/float(interpolate_times+1);
                s_interpolated.y=(sp.y+s.y)/float(interpolate_times+1);
                s_interpolated.z=(sp.z+s.z)/float(interpolate_times+1);
                s.parent=s_interpolated.n;
                max_index++;
                nt_out_listNeuron.append(s_interpolated);
            }
            else if(interpolate_times>1)
            {
                //interpolate list of nodes
                float x_Interpolation_dis=(sp.x-s.x)/float(interpolate_times);
                float y_Interpolation_dis=(sp.y-s.y)/float(interpolate_times);
                float z_Interpolation_dis=(sp.z-s.z)/float(interpolate_times);

                NeuronSWC s_interpolated_start=s;
                long spid=s.pn;
                for(int ti=1;ti<=interpolate_times;ti++)
                {
                    NeuronSWC s_interpolated=s_interpolated_start;
                    s_interpolated.n=max_index+1;
                    s_interpolated.x=s_interpolated_start.x+x_Interpolation_dis;
                    s_interpolated.y=s_interpolated_start.y+y_Interpolation_dis;
                    s_interpolated.z=s_interpolated_start.z+z_Interpolation_dis;
                    s_interpolated.parent=max_index+2;
                    if(ti==interpolate_times)
                        s_interpolated.parent=spid;
                    else if(ti==1)
                        s.parent=s_interpolated.n;
                    nt_out_listNeuron.append(s_interpolated);
                    max_index++;
                    s_interpolated_start=s_interpolated;
                }
            }
        }
        nt_out_listNeuron.append(s);
    }
    cout<<"finished the interpolation"<<endl;
    cout<<"from size: "<<siz<<" to "<<nt_out_listNeuron.size()<<endl;
    //step3: re_sort index of nt_out

    NeuronTree nt_out;
    for(V3DLONG i=0;i<nt_out_listNeuron.size();i++)
    {
        NeuronSWC s = nt_out_listNeuron[i];
        nt_out.listNeuron.append(s);
        nt_out.hashNeuron.insert(s.n,nt_out.listNeuron.size()-1);
    }
    if(sort_index)
        return reindexNT(nt_out);
    else
        return nt_out;
}
NeuronTree internode_pruning_br(NeuronTree nt,float pruning_dist){
    /* pruning with a direction
     * This version will split neuron into branches, and then travesal all the branches.
     * pruning_dist<0, will prune all the internodes
    */
    V3DLONG siz=nt.listNeuron.size();
    NeuronTree out;    if(siz<=0){return out;}
    QHash <V3DLONG, V3DLONG>  hashNeuron;
    for(V3DLONG i=0;i<siz;i++){
        NeuronSWC s = nt.listNeuron.at(i);
        hashNeuron.insert(s.n,i);
    }
    vector<int> ntype(siz,0); getNodeType(nt,ntype);
    vector<uint> remove_nodes(siz,0);
//    cout<<"Inswc node size="<<siz<<endl;
    for (V3DLONG i=0;i<siz;i++)
    {
        V3DLONG sid=i;
        V3DLONG csid=i;
        NeuronSWC s = nt.listNeuron.at(sid);
        if((ntype.at(sid)==0||ntype.at(sid)==2)        // start from tip or bifurcation point
                &&(s.pn>0&&hashNeuron.contains(s.pn)))
        {
            V3DLONG sp_id=hashNeuron.value(s.pn);
            if(ntype.at(sp_id)>1)
                continue; // if parent node not an internode, skip this branch
            NeuronSWC sp;
            double ssp_dist=0;
            while(true)
            {
                if(ntype.at(sp_id)>1)
                    break; // if parent node not an internode, this branch is almost finished. only need to check the last internode
                sp=nt.listNeuron.at(sp_id);
                ssp_dist=dis(s,sp);
//                cout<<"sid"<<sid<<",spid"<<sp_id<<",n_d="<<ssp_dist<<endl;
                if(ssp_dist<=pruning_dist||pruning_dist<0)
                {
                    //sp is redundancy internode
                    remove_nodes[sp_id]=1;
                    nt.listNeuron[sid].parent=sp.pn;
                    //update sp
                    sp_id=hashNeuron.value(sp.pn);
                }
                else{
                    //sp is a useful node
                    //update s
                    csid=sid;
                    sid=sp_id;
                    s = nt.listNeuron.at(sid);
                    if(s.pn<0||!hashNeuron.contains(s.pn))
                        break;
                    //update sp
                    sp_id=hashNeuron.value(s.pn);
                }
            }
//            check last internode
            sp=nt.listNeuron.at(sp_id);
            ssp_dist=dis(s,sp);
            if(ntype.at(sp_id)>1&&
                    ntype.at(csid)==1&&
                    remove_nodes[csid]==0&&
                    ssp_dist<=pruning_dist)
            {
                remove_nodes[sid]=1; // remove child node
                nt.listNeuron[csid].parent=sp.n;
//                if(ntype.at(sid)>1)
//                    cout<<nt.listNeuron[csid].n<<","<<nt.listNeuron[sid].n<<","<<sp.n<<endl;
            }
        }
    }

    for(V3DLONG i=0;i<siz;i++){
        NeuronSWC s = nt.listNeuron.at(i);
        if(remove_nodes.at(i)&&ntype.at(i)==1){
            continue;
        }
        out.listNeuron.append(s);
        out.hashNeuron.insert(s.n,out.listNeuron.size()-1);
    }
    cout<<"Internodes pruning size="<<(nt.listNeuron.size()-out.listNeuron.size())<<endl;
    return reindexNT(out);
//    return out;
}
bool type_refine(NeuronTree &nt,bool &refined,bool &undefined_typed_seg,bool &one_axon,bool &one_apical,V3DLONG somaid){

    V3DLONG siz=nt.listNeuron.size(); if(!siz) {return true;}
    if(somaid<0)
        somaid=get_soma(nt,false);
    if(somaid<0){return false;}
    vector<int> ntype(siz,0);
    if(!getNodeType(nt,ntype,somaid)){return true;}
    NeuronTree nt_raw; nt_raw.deepCopy(nt);
    // make sure no other type
//    bool undefined_typed_seg=false;
    for (V3DLONG i=0;i<siz;i++){
        NeuronSWC s=nt.listNeuron.at(i);
        if(ntype.at(i)==0)
        {
            if(s.type<5&&s.type>1)
                continue;
            // follow path to soma, and set to near node type
            undefined_typed_seg=true;
            V3DLONG pid=i;
            while(true){
                if(!nt.hashNeuron.contains(s.pn)||s.pn<0)
                    break;
                pid=nt.hashNeuron.value(s.pn);
                if(pid==somaid||nt.listNeuron.at(pid).pn<0)
                    break;
                if(nt.listNeuron[pid].type>1&&nt.listNeuron.at(pid).type<=4){
                    undefined_typed_seg=false;
                    nt.listNeuron[i].type=nt.listNeuron[pid].type;
                    break;
                }
                //next node
                s=nt.listNeuron.at(pid);
            }
            if(undefined_typed_seg&&s.type==1){
                nt.listNeuron[i].type=3;
                undefined_typed_seg=false;
            }
            if(undefined_typed_seg){

                cout<<"can't assign tip type:"<<s.type<<endl;
                cout<<nt.listNeuron.at(pid).n<<endl;
                return false;
            }
        }
    }
    // axon retype
    cout<<"start to refine axonal arbor type"<<endl;
    for (V3DLONG i=0;i<siz;i++){
        NeuronSWC s=nt.listNeuron.at(i);
        if(ntype.at(i)==0&&s.type==2){
            //start from axonal tip nodes
            V3DLONG pid=i;
            while(true){
                if(!nt.hashNeuron.contains(s.pn)||s.pn<0)
                    break;
                pid=nt.hashNeuron.value(s.pn);
                if(pid==somaid||nt.listNeuron.at(pid).pn<0)
                    break;
                //retype to 2
                if(nt.listNeuron[pid].type!=2){
                    refined=true;
                    nt.listNeuron[pid].type=2;
                }
                //next node
                s=nt.listNeuron.at(pid);
            }
        }
    }
    //basal dendrite retype
    cout<<"start to refine basal dendritic arbor type"<<endl;
    for (V3DLONG i=0;i<siz;i++){
        NeuronSWC s=nt.listNeuron.at(i);
        if(ntype.at(i)==0&&s.type==3){
            //start from dendritic tip nodes
            V3DLONG pid=i;
            while(true){
                if(!nt.hashNeuron.contains(s.pn)||s.pn<0)
                    break;
                pid=nt.hashNeuron.value(s.pn);
                //retype to 3
                if(pid==somaid||nt.listNeuron.at(pid).pn<0)
                    break;
                if(nt.listNeuron[pid].type!=3){
                    refined=true;
                    nt.listNeuron[pid].type=3;
                }
                //next node
                s=nt.listNeuron.at(pid);
            }
        }
    }
    //apical dendrite retype
    cout<<"start to refine apical dendritic arbor type"<<endl;
    for (V3DLONG i=0;i<siz;i++){
        NeuronSWC s=nt.listNeuron.at(i);
        if(ntype.at(i)==0&&s.type==4){
            //start from dendritic tip nodes
            V3DLONG pid=i;
            while(true){
                if(!nt.hashNeuron.contains(s.pn)||s.pn<0)
                    break;
                pid=nt.hashNeuron.value(s.pn);
                //retype to 4
                if(pid==somaid||nt.listNeuron.at(pid).pn<0)
                    break;
                if(nt.listNeuron[pid].type!=4){
                    refined=true;
                    nt.listNeuron[pid].type=4;
                }
                //next node
                s=nt.listNeuron.at(pid);
            }
        }
    }
    // axon arbor start from one node
    QList<NeuronSWC> axon_start_points;
    one_axon=one_arbor_check(nt,axon_start_points,2);
    // apical arbor start from one node
    QList<NeuronSWC> apical_start_points;
    one_apical=one_arbor_check(nt,apical_start_points,4);
    if(!one_axon||!one_apical){
        nt.listNeuron.clear();
        nt.hashNeuron.clear();
        nt.deepCopy(nt_raw);
        refined=false;
        return false;
    }
    return true;
}
bool one_arbor_check(NeuronTree nt, QList<NeuronSWC> &start_points, int arbor_type){
    if(nt.listNeuron.size()==0)
        return true;
    NeuronTree check_arbor;
    for (V3DLONG i=0;i<nt.listNeuron.size();i++){
        NeuronSWC s=nt.listNeuron.at(i);
        if(s.type!=arbor_type)
            continue;
        check_arbor.listNeuron.append(s);
        check_arbor.hashNeuron.insert(s.n,check_arbor.listNeuron.size()-1);
    }
    start_points.clear();
    if(check_arbor.listNeuron.size()==0)
        return true;
    for (V3DLONG i=0;i<check_arbor.listNeuron.size();i++){
        NeuronSWC s=check_arbor.listNeuron.at(i);
        if(!check_arbor.hashNeuron.contains(s.pn))
            start_points.append(s);
    }
    if(start_points.size()==1)
        return true;
    else
        return false;
}
NeuronTree duplicated_tip_branch_pruning(NeuronTree nt,float dist_thre){
    NeuronTree nt_out;
    V3DLONG siz=nt.listNeuron.size(); if(!siz) {return nt_out;}
    QHash <V3DLONG, V3DLONG>  hashNeuron;hashNeuron.clear();
    for (V3DLONG i=0;i<siz;i++)
        hashNeuron.insert(nt.listNeuron[i].n,i);

    std::vector<int> ntype(siz,0); getNodeType(nt,ntype);
    std::vector<int> nkept(siz,1);

    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = nt.listNeuron.at(i);
        std::vector<V3DLONG> nscanned;
        nscanned.clear();
        if(ntype.at(i)==0&&s.pn&&hashNeuron.contains(s.pn))
        {
            nscanned.push_back(i);
            //distance computing
            V3DLONG spn_id=hashNeuron.value(s.pn);
            NeuronSWC spn=nt.listNeuron.at(spn_id);
            double tipbranch_dist=0.0;
            while(true)
            {
                tipbranch_dist+=sqrt((spn.x-s.x)*(spn.x-s.x)+
                                                        (spn.y-s.y)*(spn.y-s.y)+
                                                        (spn.z-s.z)*(spn.z-s.z));
                if(ntype.at(spn_id)>1) {break;}
                nscanned.push_back(spn_id);
                s=spn;
                if(s.pn<0||!hashNeuron.contains(s.pn)){
                    nscanned.clear(); tipbranch_dist+=(dist_thre+1);break;
                }
                spn_id=hashNeuron.value(s.pn);
                spn=nt.listNeuron.at(spn_id);
            }
            if(tipbranch_dist<=dist_thre)
            {
                //remove this tip-branch
                cout<<"tip branch len="<<tipbranch_dist;
                cout<<",small tip-branch: "<<nt.listNeuron.at(i).n<<endl;
                bool duplicated=true;
                for(V3DLONG n=0;n<nscanned.size();n++)
                {
                    NeuronSWC cs=nt.listNeuron.at(nscanned.at(n));
                    for(V3DLONG ii=0;ii<siz;ii++){
                        NeuronSWC si=nt.listNeuron.at(ii);
                        if(nscanned.at(n)!=ii&&
                                cs.x==si.x&&
                                cs.y==si.y&&
                                cs.z==si.z){
                            duplicated=true;
                            break;
                        }
                    }
                    if(!duplicated)
                        break;
                }
                if(!duplicated)
                    nscanned.clear();
                else
                    for(V3DLONG n=0;n<nscanned.size();n++)
                        nkept[nscanned.at(n)]=0;
            }
        }
        else //internode
            continue;
    }
    //save
    for (V3DLONG i=0;i<siz;i++)
    {
        if(nkept[i]!=0)
        {
            NeuronSWC s = nt.listNeuron[i];
            nt_out.listNeuron.append(s);
            nt_out.hashNeuron.insert(s.n,nt_out.listNeuron.size()-1);
        }
    }
    cout<<"#dup branch pruning nodes="<<nt.listNeuron.size()-nt_out.listNeuron.size()<<endl;
    return nt_out;
}
NeuronTree smooth_branch_movingAvearage(NeuronTree nt, int smooth_win_size)
{
    /*method: moving avearage
     * 1. get all the branches
     * 2. smooth every nodes in each branch
     * 3.
    */
    NeuronTree nt_smoothed,nt_out;
    V3DLONG siz=nt.listNeuron.size();
    if(!siz)
        return nt_out;
    QHash <V3DLONG, V3DLONG>  hashNeuron;hashNeuron.clear();
    for (V3DLONG i=0;i<siz;i++)
        hashNeuron.insert(nt.listNeuron[i].n,i);

    // 2. get node type: index -> node_type
    /*soma: ntype>=3, branch: ntype=2; tip: ntype=0; internodes: ntype=1*/
    std::vector<int> ntype(siz,0); getNodeType(nt,ntype);
    //get child id: n -> child_n; only for internodes
    QHash <V3DLONG, V3DLONG>  hashChild;hashChild.clear();
    for(V3DLONG i=0;i<siz;i++)
    {
        if(ntype[i]==1)
        {
            V3DLONG this_node_n=nt.listNeuron[i].n;
            for(V3DLONG j=0;j<siz;j++)
            {
                if(i==j)
                    continue;
                if(this_node_n==nt.listNeuron[j].pn)
                {hashChild.insert(i,j); break;}
            }
        }
    }
    //3. start from one key points and end at one key points
    int half_win_size=(smooth_win_size-1)/2;
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = nt.listNeuron[i];
        // make no changes to tip, branch and soma nodes
        if(ntype[i]==0||ntype[i]>=2)
        {
            nt_smoothed.listNeuron.append(s);
            nt_smoothed.hashNeuron.insert(s.n,nt_smoothed.listNeuron.size()-1);
            continue;
        }
        int this_smooth_win_size=1;
        //for children of this node
        V3DLONG child_index=hashChild.value(i);
        for(int c=0;c<half_win_size;c++)
        {
            NeuronSWC schild=nt.listNeuron[child_index];
            s.x+=schild.x;
            s.y+=schild.y;
            s.z+=schild.z;
            this_smooth_win_size++;
            if(ntype[child_index]==0||ntype[child_index]>=2)
                break;
            child_index=hashChild.value(child_index);
        }
        //for parents of this node
         V3DLONG parent_index=hashNeuron.value(s.pn);
        for(int p=0;p<half_win_size;p++)
        {
            NeuronSWC sparent=nt.listNeuron[parent_index];
            s.x+=sparent.x;
            s.y+=sparent.y;
            s.z+=sparent.z;
            this_smooth_win_size++;
            if(ntype[parent_index]==0||ntype[parent_index]>=2)
                break;
            parent_index=hashNeuron.value(sparent.pn);
        }
        // avearage
        s.x/=this_smooth_win_size;
        s.y/=this_smooth_win_size;
        s.z/=this_smooth_win_size;
        nt_smoothed.listNeuron.append(s);
        nt_smoothed.hashNeuron.insert(s.n,nt_smoothed.listNeuron.size()-1);
    }
    //remove very near nodes
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = nt_smoothed.listNeuron[i];
        if(ntype[i]==0||ntype[i]>=2)
            continue;

        NeuronSWC spn=nt_smoothed.listNeuron[hashNeuron.value(s.pn)];
        double to_parent_dist=0;
        to_parent_dist=sqrt((spn.x-s.x)*(spn.x-s.x)+
                                                (spn.y-s.y)*(spn.y-s.y)+
                                                (spn.z-s.z)*(spn.z-s.z));
        if(to_parent_dist<1)
        {
            //remove this node
            //get child id
            V3DLONG child_index=hashChild.value(i);
            nt_smoothed.listNeuron[child_index].pn=s.pn;
            nt_smoothed.listNeuron[i].pn=0;
            continue;
        }
    }
    for (V3DLONG i=0;i<siz;i++)
    {
        if(nt_smoothed.listNeuron[i].pn==0)
            continue;
        NeuronSWC s = nt_smoothed.listNeuron[i];
        nt_out.listNeuron.append(s);
        nt_out.hashNeuron.insert(s.n,nt_out.listNeuron.size()-1);
    }
    cout<<"Remove duplicated node size: "<<(siz-nt_out.listNeuron.size())<<endl;
    qDebug()<<"Finished smoothing process";
    return nt_out;
}
bool split_neuron_type(QString inswcpath,QString outpath,bool soma_typed,int saveESWC)
{
    /*make sure type consistence
     * dendrite,axon,apical_dendrite
     * each type should have soma point
    */
    NeuronTree nt = readSWC_file(inswcpath);
    V3DLONG siz=nt.listNeuron.size();
    if(!siz) {return false;}

    QHash <V3DLONG, V3DLONG>  hashNeuron;
    V3DLONG somaid=get_soma(nt,false,soma_typed); if(somaid<0){return false;}
    for (V3DLONG i=0;i<siz;i++)
        hashNeuron.insert(nt.listNeuron[i].n,i);

    //generation
    NeuronSWC soma_node = nt.listNeuron.at(somaid);

    NeuronTree nt_dendrite_whole;
    nt_dendrite_whole.listNeuron.append(soma_node);
    nt_dendrite_whole.hashNeuron.insert(soma_node.n,nt_dendrite_whole.listNeuron.size()-1);

    NeuronTree nt_dendrite;
    nt_dendrite.listNeuron.append(soma_node);
    nt_dendrite.hashNeuron.insert(soma_node.n,nt_dendrite.listNeuron.size()-1);

    NeuronTree nt_apicaldendrite;
    nt_apicaldendrite.listNeuron.append(soma_node);
    nt_apicaldendrite.hashNeuron.insert(soma_node.n,nt_apicaldendrite.listNeuron.size()-1);

    NeuronTree nt_axon;
    nt_axon.listNeuron.append(soma_node);
    nt_axon.hashNeuron.insert(soma_node.n,nt_axon.listNeuron.size()-1);

    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = nt.listNeuron[i];
        NeuronSWC sp;
        if(hashNeuron.contains(s.pn))
            sp=nt.listNeuron[hashNeuron.value(s.pn)];
        if(s.type==3||s.type==4)
        {
            if(sp.type!=3&&sp.type!=4&&sp.type!=1)
            {
                s.pn=soma_node.n;
            }
            nt_dendrite_whole.listNeuron.append(s);
            nt_dendrite_whole.hashNeuron.insert(s.n,nt_dendrite_whole.listNeuron.size()-1);
            if(s.type==3)
            {
                nt_dendrite.listNeuron.append(s);
                nt_dendrite.hashNeuron.insert(s.n,nt_dendrite.listNeuron.size()-1);
            }
            else if(s.type==4)
            {
                nt_apicaldendrite.listNeuron.append(s);
                nt_apicaldendrite.hashNeuron.insert(s.n,nt_apicaldendrite.listNeuron.size()-1);
            }
        }
        else if(s.type==2)
        {
            if(sp.type!=2&&sp.type!=1)
            {
                s.pn=soma_node.n;
            }
            nt_axon.listNeuron.append(s);
            nt_axon.hashNeuron.insert(s.n,nt_axon.listNeuron.size()-1);
        }
    }
    //reorder index
    nt_dendrite_whole=reindexNT(nt_dendrite_whole);
    nt_axon=reindexNT(nt_axon);
    nt_dendrite=reindexNT(nt_dendrite);
    nt_apicaldendrite=reindexNT(nt_apicaldendrite);
    //save
    QFileInfo infileinfo(inswcpath);
    QString outfile=(outpath.isEmpty())?(inswcpath):(outpath+"/"+infileinfo.completeBaseName());

    if(saveESWC)
    {
        if(nt_apicaldendrite.listNeuron.size()>1)
        {
            writeESWC_file(outfile+"_wholeDendrite.eswc",nt_dendrite_whole);
            writeESWC_file(outfile+"_apicalDendrite.eswc",nt_apicaldendrite);
        }
        if(nt_dendrite.listNeuron.size()>1)
            writeESWC_file(outfile+"_dendrite.eswc",nt_dendrite);
        if(nt_axon.listNeuron.size()>1)
            writeESWC_file(outfile+"_axon.eswc",nt_axon);
    }
    else
    {
        if(nt_apicaldendrite.listNeuron.size()>1)
        {
            writeSWC_file(outfile+"_wholeDendrite.swc",nt_dendrite_whole);
            writeSWC_file(outfile+"_apicalDendrite.swc",nt_apicaldendrite);
        }
        if(nt_dendrite.listNeuron.size()>1)
            writeSWC_file(outfile+"_dendrite.swc",nt_dendrite);
        if(nt_axon.listNeuron.size()>1)
            writeSWC_file(outfile+"_axon.swc",nt_axon);
    }
    return true;
}
double seg_median(std::vector<double> input)
{
    double mout=0;
    //冒泡排序将数组排序
    for(int i=0;i<input.size()-1;i++){
        for(int j=0;j<input.size()-1-i;j++){
            if(input[j]>input[j+1]){
                double t=input[j+1];
                input[j+1]=input[j];
                input[j]=t;
            }
        }
    }
    if(input.size()%2==0){
        int b=input.size()/2;
        mout=(input[b]+input[b-1])/2.0;
    }
    else
        mout=input[(input.size()-1)/2];
    return mout;
}
double vector_max(std::vector<double> input){
    double vmax=MINVALUE;
    for(V3DLONG i=0;i<input.size();i++)
        vmax=(vmax<input.at(i))?input.at(i):vmax;
    return vmax;
}
double vector_mean(std::vector<double> input){
    if(!input.size())
        return 0.0;
    double sum =0.0;
    for(V3DLONG i=0;i<input.size();i++)
        sum+=input.at(i);
    if(sum<0)
    {
        double absmean=double(-1.0)*sum/double(input.size());
        cout<<"abs="<<absmean<<endl;
        cout<<"return "<<double(-1.0)*(absmean)<<endl;
        return double(-1.0)*(absmean);
    }
    else
        return sum / double(input.size());
}
double vector_std(std::vector<double> input){
    if(input.size()<=1)
        return 0;
    double mean= (vector_mean(input));

    double accum  = 0.0;
    for(V3DLONG i=0;i<input.size();i++)
        accum  += (input.at(i)-mean)*(input.at(i)-mean);
    if(accum<0)
        accum*=double(-1);
    double stdev = sqrt(accum/double(input.size()-1));
    return stdev;
}
double vector_min(std::vector<double> input){
    double vmin=MAXVALUE;
    for(V3DLONG i=0;i<input.size();i++)
        vmin=(vmin>input.at(i))?input.at(i):vmin;
    return vmin;
}
bool teraImage_swc_crop(V3DPluginCallback2 &callback, string inimg, string inswc,QString save_path, int cropx, int cropy, int cropz,int crop_neighbor_voxels)
{
    cout<<"Base on swc boundingbox, crop image block and swc-in-block"<<endl;
    //read neuron tree
    NeuronTree nt=readSWC_file(QString::fromStdString(inswc));
    V3DLONG siz=nt.listNeuron.size();
    if(!siz){return false;}
    cout<<"finished reading swc: "<<siz<<endl;
    //read terafly  image
    V3DLONG *in_zz = 0;
    if(!callback.getDimTeraFly(inimg,in_zz)) {cout<<"can't load terafly img"<<endl;return false;}
    cout<<"load terafly:"<<inimg<<endl;
    //get crop center and crop size
   std::vector <double> x_coords(siz,0.0);
   std::vector <double> y_coords(siz,0.0);
   std::vector <double> z_coords(siz,0.0);
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = nt.listNeuron.at(i);
        x_coords[i]=s.x;
        y_coords[i]=s.y;
        z_coords[i]=s.z;
    }
    double crop_center_x_coord=vector_mean(x_coords);
    double crop_center_y_coord=vector_mean(y_coords);
    double crop_center_z_coord=vector_mean(z_coords);
    cout<<"x center: "<<crop_center_x_coord<<endl;
    cout<<"y center: "<<crop_center_y_coord<<endl;
    cout<<"z center: "<<crop_center_z_coord<<endl;
    V3DLONG x_min_coord=vector_min(x_coords)-crop_neighbor_voxels;
    V3DLONG x_max_coord=vector_max(x_coords)+crop_neighbor_voxels;
    if(cropx){
        x_min_coord=crop_center_x_coord-cropx-crop_neighbor_voxels;
        x_max_coord=crop_center_x_coord+cropx+crop_neighbor_voxels;
    }
    if(x_min_coord<0) x_min_coord = 0;
    if(x_max_coord >= in_zz[0]) x_max_coord = in_zz[0]-1;

    V3DLONG y_min_coord=vector_min(y_coords)-crop_neighbor_voxels;
    V3DLONG y_max_coord=vector_max(y_coords)+crop_neighbor_voxels;
    if(cropy){
        y_min_coord=crop_center_y_coord-cropy-crop_neighbor_voxels;
        y_max_coord=crop_center_y_coord+cropy+crop_neighbor_voxels;
    }
    if(y_min_coord<0) y_min_coord = 0;
    if(y_max_coord >= in_zz[1]) y_max_coord = in_zz[1]-1;

    V3DLONG z_min_coord=vector_min(z_coords)-crop_neighbor_voxels;
    V3DLONG z_max_coord=vector_max(z_coords)+crop_neighbor_voxels;
    if(cropz){
        z_min_coord=crop_center_z_coord-cropz-crop_neighbor_voxels;
        z_max_coord=crop_center_z_coord+cropz+crop_neighbor_voxels;
    }
    if(z_min_coord<0) z_min_coord = 0;
    if(z_max_coord >= in_zz[2]) z_max_coord = in_zz[2]-1;
    //crop image block
    V3DLONG *in_sz = new V3DLONG[4];
    in_sz[0] = x_max_coord-x_min_coord+1;
    in_sz[1] = y_max_coord-y_min_coord+1;
    in_sz[2] = z_max_coord-z_min_coord+1;
    in_sz[3]=in_zz[3];

    cout<<"x min: "<<x_min_coord<<", max: "<<x_max_coord<<endl;
    cout<<"y min: "<<y_min_coord<<", max: "<<y_max_coord<<endl;
    cout<<"z min: "<<z_min_coord<<", max: "<<z_max_coord<<endl;
    unsigned char * im_cropped = 0;
    V3DLONG pagesz;
    pagesz = in_sz[0]*in_sz[1]*in_sz[2]*in_sz[3];
    try {im_cropped = new unsigned char [pagesz];}
    catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return false;}
    im_cropped = callback.getSubVolumeTeraFly(inimg,x_min_coord,x_max_coord+1,y_min_coord,y_max_coord+1,z_min_coord,z_max_coord+1);
    if(im_cropped==NULL)
        return false;
    cout<<"page size="<<pagesz<<endl;
    //save cropped image
    QString fbname=QFileInfo(QString::fromStdString(inswc)).baseName();
//    QString tmpstr = "";
//    tmpstr.append("_x_").append(QString("%1").arg(crop_center_x_coord));
//    tmpstr.append("_y_").append(QString("%1").arg(crop_center_y_coord));
//    tmpstr.append("_z_").append(QString("%1").arg(crop_center_z_coord));
//    QString default_img_name = fbname+"_Img"+tmpstr+".v3draw";
    QString default_img_name = fbname+"_Img.v3dpbd";
    QDir path(save_path);
    if(!path.exists()) { path.mkpath(save_path);}
    QString save_path_img =save_path+"/"+default_img_name;
    cout<<"save cropped image path:"<<save_path_img.toStdString()<<endl;
    /*image enhancement*/
    unsigned char * inimg1d = 0;
    try {inimg1d = new unsigned char [pagesz];}
    catch(...)  {cout<<"cannot allocate memory for cropping."<<endl; return false;}
    if(!enhanceImage(im_cropped,inimg1d,in_sz))
        simple_saveimage_wrapper(callback, save_path_img.toStdString().c_str(),im_cropped,in_sz,1);
    else
        simple_saveimage_wrapper(callback, save_path_img.toStdString().c_str(),inimg1d,in_sz,1);
    if(im_cropped) {delete []im_cropped; im_cropped = 0;}
    if(inimg1d) {delete []inimg1d; inimg1d = 0;}
    if(in_zz) {delete []in_zz; in_zz = 0;}
     if(in_sz) {delete []in_sz; in_sz = 0;}
    //crop swc block
    NeuronTree out; out.listNeuron.clear();out.hashNeuron.clear();
    for(V3DLONG j=0;j<siz;j++)
    {
        NeuronSWC sn=nt.listNeuron.at(j);
        if(sn.x>=x_min_coord&&sn.x<=x_max_coord
                &&sn.y>=y_min_coord&&sn.y<=y_max_coord
                &&sn.z>=z_min_coord&&sn.z<=z_max_coord){
            //shift coordinates
            sn.x-=float(x_min_coord);
            sn.y-=float(y_min_coord);
            sn.z-=float(z_min_coord);
            out.listNeuron.append(sn);
            out.hashNeuron.insert(sn.n,out.listNeuron.size()-1);
        }
    }
    //save to file
//    QString default_swc_name=fbname+"_swc"+tmpstr+".eswc";
    QString infile_suffix=QFileInfo(QString::fromStdString(inswc)).suffix();
    QString default_swc_name=fbname+"_cropped."+infile_suffix;
    QString save_path_swc =save_path+"/"+default_swc_name;

    if(!QString::compare(QString("swc"),infile_suffix,Qt::CaseInsensitive))
        writeSWC_file(save_path_swc,out);
    else
        writeESWC_file(save_path_swc,out);
    QString default_ano_name=fbname+".ano";
    QString save_path_ano =save_path+"/"+default_ano_name;
    QFile anofile(save_path_ano);
    if(anofile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QString imgline="RAWIMG="; imgline+=(default_img_name+"\n");
        anofile.write(imgline.toAscii());
        QString swcline="SWCFILE="; swcline+=(default_swc_name+"\n");
        anofile.write(swcline.toAscii());
        anofile.close();
    }
    return true;
}
bool crop_local_swc_func(V3DPluginCallback2 &callback, string inswc,QString save_path,
                         double cropdist, double cropx, double cropy, double cropz,
                         bool topo_connected,
                         bool soma_typed)
{
    cout<<"center from soma, crop nodes-in-fixed-block"<<endl;
    //read neuron tree
    NeuronTree nt=readSWC_file(QString::fromStdString(inswc));
    if(cropdist>0){
        cropx=(cropx/2>cropdist)?cropdist*2:cropx;
        cropy=(cropy/2>cropdist)?cropdist*2:cropy;
        cropz=(cropz/2>cropdist)?cropdist*2:cropz;
    }
    else
        cropdist=sqrt(cropx*cropx+cropy*cropy+cropz*cropz);
    V3DLONG siz=nt.listNeuron.size();   if(!siz){return false;}

    //get crop center and crop size
    // get soma
    V3DLONG somaid=get_soma(nt,false,soma_typed); if(somaid<0){return false;}
    double xmin,ymin,zmin,xmax,ymax,zmax;
    xmin=nt.listNeuron.at(somaid).x-cropx/2;
    ymin=nt.listNeuron.at(somaid).y-cropy/2;
    zmin=nt.listNeuron.at(somaid).z-cropz/2;
    xmax=nt.listNeuron.at(somaid).x+cropx/2;
    ymax=nt.listNeuron.at(somaid).y+cropy/2;
    zmax=nt.listNeuron.at(somaid).z+cropz/2;

    NeuronSWC snode=nt.listNeuron.at(somaid);
    //get nodes-list in block
    QList<V3DLONG> nodes_list; nodes_list.clear();
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = nt.listNeuron.at(i);
        if(s.x>=xmin&&s.y>=ymin&&s.z>=zmin&&
                s.x<=xmax&&s.y<=ymax&&s.z<=zmax)
            if(dis(s,snode)<=cropdist)
                nodes_list.append(i);
    }
    QList<V3DLONG> out_nodes;
    if(topo_connected){
        // prune isolated node
        for (V3DLONG i=0;i<nodes_list.size();i++)
        {
            if(nodes_list.at(i)==somaid){
                out_nodes.append(somaid);
                continue;
            }
            NeuronSWC s = nt.listNeuron.at(nodes_list.at(i));
            bool out_flag=true;
            // travesal to soma
            V3DLONG pid=nt.hashNeuron.value(s.pn);
            NeuronSWC sp;
            while (true) {
                if(!nodes_list.contains(pid)){
                    out_flag=false;
                    break;
                }
                sp=nt.listNeuron.at(pid);
                if(sp.pn<0)
                    break;
                //next node
                s=nt.listNeuron.at(pid);
                pid=nt.hashNeuron.value(s.pn);
            }
            if(out_flag)
                out_nodes.append(nodes_list.at(i));
        }
    }
    else
        for(V3DLONG i=0;i<nodes_list.size();i++)
            out_nodes.append(nodes_list.at(i));

    //out
    NeuronTree nt_out;
    for(V3DLONG n=0;n<out_nodes.size();n++){
        NeuronSWC s = nt.listNeuron.at(out_nodes.at(n));
        nt_out.listNeuron.append(s);
        nt_out.hashNeuron.insert(s.n,nt_out.listNeuron.size()-1);
    }
    QString file_suffix=QFileInfo(save_path).suffix();
    if(!QString::compare(QString("swc"),file_suffix,Qt::CaseInsensitive))
        writeSWC_file(save_path,nt_out);
    else
        writeESWC_file(save_path,nt_out);

    return true;
}
bool get_files_in_dir(const QString& inpath,QStringList & outfiles,QStringList filefilters){
    QDir indir(inpath);
    if(!indir.exists()){cerr<<"Input error: not an existed input folder"<<endl; return false;}
    indir.setFilter(QDir::Dirs|QDir::NoSymLinks|QDir::Files);
    QFileInfoList rawdirs=indir.entryInfoList();
    for(int i=0;i<rawdirs.size();i++){
        QFileInfo thisdir=rawdirs.at(i);
        if(thisdir.fileName()=="."|thisdir.fileName()=="..")
            continue;
        if(thisdir.isDir())
            if(!get_files_in_dir(thisdir.filePath(),outfiles,filefilters))
                return false;
        if(thisdir.isFile()&&filefilters.contains(thisdir.suffix())){outfiles.append(thisdir.filePath());}
    }
    return true;
}
