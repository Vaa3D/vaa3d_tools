#include"nsfunctions.h"
NeuronTree tip_branch_pruning(NeuronTree nt, int in_thre)
{
    /*1. get tip, branch and soma nodes;
     * 2. from tip nodes, get tip-branch
     * 3. pruning
     * 4. save
    */
    NeuronTree nt_out;nt_out.listNeuron.clear();nt_out.hashNeuron.clear();
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
    std::vector<int> nkept(siz,1);
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
    //3. from tip-nodes
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = nt.listNeuron[i];
        std::vector<V3DLONG> nscanned;nscanned.clear();
        if(ntype[i]==0)
        {
            nscanned.push_back(i);
            //distance computing
            double tipbranch_dist=0;
            int ptype=ntype[hashNeuron.value(s.pn)];
            while(ptype)
            {
                V3DLONG spn_id=hashNeuron.value(s.pn);
                NeuronSWC spn=nt.listNeuron[spn_id];
                tipbranch_dist+=sqrt((spn.x-s.x)*(spn.x-s.x)+
                                                        (spn.y-s.y)*(spn.y-s.y)+
                                                        (spn.z-s.z)*(spn.z-s.z));
                if(ptype>1)
                    break;
                nscanned.push_back(spn_id);
                s=spn;ptype=ntype[hashNeuron.value(s.pn)];
            }
            if(tipbranch_dist<=in_thre)
            {
                //remove this tip-branch
                for(V3DLONG n=0;n<nscanned.size();n++)
                    nkept[nscanned[n]]=0;
            }
        }
        else //internode
            continue;
    }
    //save
    for (V3DLONG i=0;i<siz;i++)
    {
        if(nkept[i])
        {
            NeuronSWC s = nt.listNeuron[i];
            nt_out.listNeuron.append(s);
            nt_out.hashNeuron.insert(s.n,nt_out.listNeuron.size()-1);
        }
        //debug
//        if(true)
//        {
//            NeuronSWC s = nt.listNeuron[i];
//            if(nkept[i]==0){s.type=1;s.r=10;}
//            nt_out.listNeuron.append(s);
//            nt_out.hashNeuron.insert(s.n,nt_out.listNeuron.size()-1);
//        }
    }
    //reindex
//     NeuronTree nt_out_reindex=reindexNT(nt_out);
//    return nt_out_reindex;
    return nt_out;
}
NeuronTree to_topology_tree(NeuronTree nt)
{
    /*1. get tip, branch and soma nodes;
     * 2. remove internodes
     * 3. reindex
     * 3. save
    */
    NeuronTree nt_out;nt_out.listNeuron.clear();nt_out.hashNeuron.clear();
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
NeuronTree reindexNT(NeuronTree nt)
{
    NeuronTree nt_out_reindex;nt_out_reindex.listNeuron.clear();nt_out_reindex.hashNeuron.clear();
    for(V3DLONG i=0;i<nt.listNeuron.size();i++)
    {
        NeuronSWC s = nt.listNeuron[i];
        s.pn=(s.pn<0)?s.pn:(nt.hashNeuron.value(s.pn)+1);
//        s.n=nt.hashNeuron.value(s.n)+1;
        s.n=i+1;
        nt_out_reindex.listNeuron.append(s);
        nt_out_reindex.hashNeuron.insert(s.n,nt_out_reindex.listNeuron.size()-1);
    }
   return nt_out_reindex;
}
NeuronTree three_bifurcation_processing(NeuronTree nt)
{
    //s1. detect three bifurcation points
    //s2. move one of the branch to the parent of bifurcation node
    NeuronTree nt_out;nt_out.listNeuron.clear();nt_out.hashNeuron.clear();
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
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = nt.listNeuron[i];
        if(ntype[i]<=2&&s.pn>0&&hashNeuron.contains(s.pn))
        {
            V3DLONG spn_id=hashNeuron.value(s.pn);
            if(ntype[spn_id]>2&&spn_id!=somaid)
            {
                cout<<"bifruction node id: "<<s.pn<<endl;
                V3DLONG spn_id_iter=spn_id;
                NeuronSWC sp = nt.listNeuron[spn_id_iter];
                while(ntype[spn_id_iter]!=1)
                {
                    spn_id_iter=hashNeuron.value(sp.pn);
                    s.pn=sp.pn;
                    if(spn_id_iter==somaid)
                        break;
                    sp = nt.listNeuron[spn_id_iter];
                }
                ntype[spn_id]-=1;
            }
        }
        nt_out.listNeuron.append(s);
        nt_out.hashNeuron.insert(s.n,nt_out.listNeuron.size()-1);
    }
    //check the performance
    return nt_out;
}
bool split_neuron_type(QString inswcpath,QString outpath,int saveESWC)
{
    /*dendrite,axon,apical_dendrite
     * each type should have soma point
    */
    NeuronTree nt = readSWC_file(inswcpath);
    if(!nt.listNeuron.size())
        return false;
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
    //generation
    NeuronSWC soma_node = nt.listNeuron[somaid];

    NeuronTree nt_dendrite_whole;nt_dendrite_whole.listNeuron.clear();nt_dendrite_whole.hashNeuron.clear();
    nt_dendrite_whole.listNeuron.append(soma_node);
    nt_dendrite_whole.hashNeuron.insert(soma_node.n,nt_dendrite_whole.listNeuron.size()-1);

    NeuronTree nt_dendrite;nt_dendrite.listNeuron.clear();nt_dendrite.hashNeuron.clear();
    nt_dendrite.listNeuron.append(soma_node);
    nt_dendrite.hashNeuron.insert(soma_node.n,nt_dendrite.listNeuron.size()-1);

    NeuronTree nt_apicaldendrite;nt_apicaldendrite.listNeuron.clear();nt_apicaldendrite.hashNeuron.clear();
    nt_apicaldendrite.listNeuron.append(soma_node);
    nt_apicaldendrite.hashNeuron.insert(soma_node.n,nt_apicaldendrite.listNeuron.size()-1);

    NeuronTree nt_axon;nt_axon.listNeuron.clear();nt_axon.hashNeuron.clear();
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
        else
            continue;
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
        writeESWC_file(outfile+"_dendrite.eswc",nt_dendrite);
        writeESWC_file(outfile+"_axon.eswc",nt_axon);
    }
    else
    {
        if(nt_apicaldendrite.listNeuron.size()>1)
        {
            writeSWC_file(outfile+"_wholeDendrite.swc",nt_dendrite_whole);
            writeSWC_file(outfile+"_apicalDendrite.swc",nt_apicaldendrite);
        }
        writeSWC_file(outfile+"_dendrite.swc",nt_dendrite);
        writeSWC_file(outfile+"_axon.swc",nt_axon);
    }
    return true;
}
