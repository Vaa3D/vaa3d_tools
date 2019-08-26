//#include "QtGlobal"
//#include "math.h"
////#include "unistd.h" //remove the unnecessary include file. //by PHC 20131228
//#include "basic_surf_objs.h"
//#include "string.h"
//#include "vector"
//#include "iostream"
#include "sort_swc_redefined.h"
//#include "unordered_map"
using namespace std;


#ifndef VOID
#define VOID 1000000000
#endif

//#define PI 3.14159265359
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
#define NTDOT(a,b) ((a).x*(b).x+(a).y*(b).y+(a).z*(b).z)
#define angle(a,b,c) (acos((((b).x-(a).x)*((c).x-(a).x)+((b).y-(a).y)*((c).y-(a).y)+((b).z-(a).z)*((c).z-(a).z))/(NTDIS(a,b)*NTDIS(a,c)))*180.0/3.14159265359)

#ifndef MAX_DOUBLE
#define MAX_DOUBLE 1.79768e+308        //actual: 1.79769e+308
#endif

QVector< QVector<V3DLONG> > get_neighbors(QList<NeuronSWC> &neurons, const QHash<V3DLONG,V3DLONG> & LUT)
{
    // generate neighbor lists for each node, using new ids.
    // LUT (look-up table): old name -> new ids
    // ids are the line numbers
    // names are the node names (neurons.name)
    QList<V3DLONG> idlist = ((QSet<V3DLONG>)LUT.values().toSet()).toList();
    int siz = idlist.size();
    QList<int> nlist;
    for(V3DLONG i=0; i<neurons.size(); i++){nlist.append(neurons.at(i).n);}

//    qDebug()<<"Before defining qvector";
    QVector< QVector<V3DLONG> > neighbors = QVector< QVector<V3DLONG> >(siz, QVector<V3DLONG>() );
//    qDebug()<<"After defining qvector";
//    system("pause");
    for (V3DLONG i=0;i<neurons.size();i++)
    {
        // Find parent node
//        qDebug()<<i;
        int pid_old = nlist.lastIndexOf(neurons.at(i).pn);
        if(pid_old<0){
            continue;  // Skip root nodes
        }
        else{
            int pname_old = neurons.at(pid_old).n;
            int cname_old = neurons.at(i).n;
            int pid_new = LUT.value(pname_old);
            int cid_new = LUT.value(cname_old);
            if((pid_new>=siz) || (cid_new>=siz)){
                v3d_msg(QString("Out of range [0, %1]: pid:%2; cid:%3").arg(siz).arg(pid_new).arg(cid_new));
            }
            // add a new neighbor for the child node
            if(!neighbors.at(cid_new).contains(pid_new)){
//                qDebug()<<QString("Adding edge between %1 and %2").arg(cid_new).arg(pid_new);
                neighbors[cid_new].push_back(pid_new);
            }
            // add a new neighbor for the parent node
            if(!neighbors.at(pid_new).contains(cid_new)){
//                qDebug()<<QString("Adding edge between %1 and %2").arg(pid_new).arg(cid_new);
                neighbors[pid_new].push_back(cid_new);
            }
        }
    }
    return neighbors;
}

QHash<V3DLONG, V3DLONG> getUniqueLUT(QList<NeuronSWC> &neurons)
{
    // Range of LUT values: [0, # deduplicated neuron list)
    QHash<V3DLONG,V3DLONG> LUT;
    V3DLONG cur_id=0;
    for (V3DLONG i=0;i<neurons.size();i++)
    {
        V3DLONG j=0;
        for (j=0;j<i;j++) // Check whether this node is a duplicated with the previous ones
        {
            if (neurons.at(i).x==neurons.at(j).x && neurons.at(i).y==neurons.at(j).y && neurons.at(i).z==neurons.at(j).z)	break;
        }
        if(i==j){  // not a duplicate of the previous ones
            LUT.insertMulti(neurons.at(i).n, cur_id);
            cur_id++;
        }
        else{
            LUT.insertMulti(neurons.at(i).n, LUT.value(neurons.at(j).n));
        }

    }
    return (LUT);
};
QList<V3DLONG> DFS(QVector< QVector<V3DLONG> > neighbors, V3DLONG newrootid, V3DLONG siz)
{
    // siz: size of the whole neuronlist
    // The neuronlist may include multiple components
    // A component is a connected tree
    // Sorted components: other components that have already been sorted.
    // Current component: the component where newroot resides. We will sort it and append it to the sorted components

    // sorted_size: size of sorted components
    // neworder: new order of the sored components
    // *group: id of the current component

    QList<V3DLONG> neworder;

    // DFS to sort current component;

    // Initialization
    QStack<int> pstack;
    QList<int> visited;
    for(int i=0;i<siz; i++){visited.append(0);}
    visited[newrootid]=1;
    pstack.push(newrootid);
    neworder.append(newrootid);

    // Tree traverse
    bool is_push;
    int pid;
    while(!pstack.isEmpty()){
        is_push = false;
        pid = pstack.top();
        // whether exist unvisited neighbors of pid
        // if yes, push neighbor to stack;
        QVector<V3DLONG>::iterator it;
        QVector<V3DLONG> cur_neighbors = neighbors.at(pid);
        for(it=cur_neighbors.begin(); it!=cur_neighbors.end(); ++it)
        {
            if(visited.at(*it)==0)
            {
                pstack.push(*it);
                is_push=true;
                visited[*it]=1;
                neworder.append(*it);
                break;
            }
        }
        // else, pop pid
        if(!is_push){
            pstack.pop();
        }
    }
    return neworder;
};

bool SortSWC(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result, V3DLONG newrootid, double thres)
{
    // node name list of
    QList<V3DLONG> nlist;
    for(int i=0; i<neurons.size(); i++){
        nlist.append(neurons.at(i).n);
    }

    //create a LUT, from the original id to the position in the listNeuron, different neurons with the same x,y,z & r are merged into one position
    QHash<V3DLONG, V3DLONG> LUT = getUniqueLUT(neurons);

    //create a new id list to give every different neuron a new id
    QList<V3DLONG> idlist = ((QSet<V3DLONG>)LUT.values().toSet()).toList();
    V3DLONG siz = idlist.size();

    // create a LUT from new id to neuron node
    QHash<V3DLONG, NeuronSWC> LUT_newid_to_node;
    for(int i=0; i<idlist.size(); i++){
        NeuronSWC node = neurons.at(nlist.indexOf(LUT.key(idlist.at(i))));
        LUT_newid_to_node.insertMulti(idlist.at(i), node);
    }

    // create a vector to keep neighbors of each node
    QVector< QVector<V3DLONG> > neighbors = get_neighbors(neurons, LUT);

    // Find the new id of the new root
    V3DLONG root = 0;
    if (newrootid==VOID)  // If unspecified, use the 1st root as new root.
    {
        for (V3DLONG i=0;i<neurons.size();i++)
            if (neurons.at(i).pn==-1){
                root = idlist.indexOf(LUT.value(neurons.at(i).n));
                break;
            }
    }
    else{
        root = idlist.indexOf(LUT.value(newrootid));
        if (LUT.keys().indexOf(newrootid)==-1)
        {
            v3d_msg("The new root id you have chosen does not exist in the SWC file.");
            return(false);
        }
    }

    //Major steps
    //do a DFS for the the matrix and re-allocate ids for all the nodes
    QList<V3DLONG> neworder;
    QList<V3DLONG> cur_neworder;
    QList<V3DLONG> component_id;
    for(int i=0; i<siz; i++){
        component_id.append(0);
    }
    V3DLONG sorted_size = 0;
    int cur_group = 1;

    // Begin with the new root node and
    // generate the 1st sorted tree.
    cur_neworder= DFS(neighbors, root, siz);
    qDebug()<<QString("cur_neworder=%1").arg(cur_neworder.size());
    sorted_size += cur_neworder.size();
    neworder.append(cur_neworder);
    for(int i=0; i<cur_neworder.size(); i++){
        component_id[cur_neworder.at(i)] = cur_group;
    }
    cout<<"Done 1st DFS"<<endl;

    // Continue to sort the rest of the tree
    while (sorted_size <siz)
    {
        V3DLONG new_root;
        cur_group++;
        for (V3DLONG iter=0;iter<siz;iter++)
        {
            if (!neworder.contains(iter))
            {
                new_root = iter;
                break;
            }
        }
        cur_neworder = DFS(neighbors, new_root, siz);
        qDebug()<<QString("cur_neworder=%1").arg(cur_neworder.size());
        sorted_size += cur_neworder.size();
        neworder.append(cur_neworder);
        for(int i=0; i<cur_neworder.size(); i++){
            component_id[cur_neworder.at(i)] = cur_group;
        }
    }
    qDebug()<<"Number of components before making connections"<<cur_group;

    QList<V3DLONG> output_newroot_list;
    if((thres != 1000000000) && (thres>0)){  // If distance threshold > 0: make new connections
        qDebug()<<"find the point in non-group 1 that is nearest to group 1";
        //find the point in non-group 1 that is nearest to group 1,
        //include the nearest point as well as its neighbors into group 1, until all the nodes are connected
        output_newroot_list.append(root);
        while(cur_group>1)
        {
            qDebug()<<"Remaining components: "<<cur_group;
            double min = VOID;
            double dist2 = 0;
            int mingroup = 1;

            // Find the closest pair of nodes between group 1 and the rest.
            V3DLONG m1,m2;
            for (V3DLONG ii=0;ii<siz;ii++)
            {
                if (component_id[ii]==1)
                {
//                    qDebug()<<QString("Distance check: %1").arg(ii);
                    for (V3DLONG jj=0;jj<siz;jj++)
                        if (component_id[jj]!=1)
                        {
//                            dist2 = computeDist2(neurons.at(nlist.indexOf(LUT.key(ii))),
//                                                 neurons.at(nlist.indexOf(LUT.key(jj))));
                            dist2 = computeDist2(LUT_newid_to_node.value(ii),
                                                 LUT_newid_to_node.value(jj));
                            if (dist2<min)
                            {
                                min = dist2;
                                mingroup = component_id[jj];
                                m1 = ii;
                                m2 = jj;
                            }
                        }
                }
            }
            for (V3DLONG i=0;i<siz;i++)
            {
                if (component_id[i]==mingroup)
                {
                    component_id[i] = 1;
                }
            }
            if (min<=thres)
            {
                qDebug()<<QString("New connection is made between %1 and %2").arg(m1).arg(m2);
                qDebug()<<QString("Original node name: %1 and %2")
                          .arg(LUT_newid_to_node.value(m1).n)
                          .arg(LUT_newid_to_node.value(m2).n);
                if(!neighbors.at(m1).contains(m2)){neighbors[m1].push_back(m2);}
                if(!neighbors.at(m2).contains(m1)){neighbors[m2].push_back(m1);}
            }
            else{  // set component the node closest to group 1 is root
                output_newroot_list.append(m2);
            }
            cur_group--;
        }
        qDebug()<<"Number of components after making connections"<<output_newroot_list.size();
    }
    else{
        int tp_group = 0;
        for(int i=0; i<siz; i++){
            if(component_id.at(i) != tp_group){
                output_newroot_list.append(neworder.at(i));
                tp_group = component_id.at(i);
            }
        }
    }

    // DFS sort of the neuronlist after new connections
    for (int i=0;i<siz;i++)
    {
        component_id[i] = 0;
        neworder[i]= VOID;
    }
    component_id.clear();
    neworder.clear();
    sorted_size = 0;
    cur_group = 1;

    V3DLONG offset=0;
    for(V3DLONG i=0; i<output_newroot_list.size(); i++)
    {
        V3DLONG new_root = output_newroot_list.at(i);
//        qDebug()<<QString("Output component %1, root id is %2").arg(i).arg(new_root);
        V3DLONG cnt = 0;
        // Sort current component;
        cur_neworder= DFS(neighbors, new_root, siz);
        qDebug()<<QString("cur_neworder=%1").arg(cur_neworder.size());
        sorted_size += cur_neworder.size();
        neworder.append(cur_neworder);
        for(int i=0; i<cur_neworder.size(); i++){
            component_id.append(cur_group);
        }
        NeuronSWC S;
        S = LUT_newid_to_node.value(new_root);
        S.n = offset+1;
        S.pn = -1;
        result.append(S);
        cnt++;
        qDebug()<<QString("New root %1:").arg(i)<<S.x<<S.y<<S.z;

        for (V3DLONG ii=offset+1;ii<(sorted_size);ii++)
        {
            for (V3DLONG jj=offset;jj<ii;jj++) //after DFS the id of parent must be less than child's
            {
                V3DLONG cid = neworder[ii];
                V3DLONG pid = neworder[jj];
                // If there's an edge between the two nodes, then pid is the parent of cid
                if (pid!=VOID && cid!=VOID && neighbors.at(pid).contains(cid))
                {
                        NeuronSWC S;
                        S = LUT_newid_to_node.value(cid);
                        S.n = ii+1;
                        S.pn = jj+1;

                        result.append(S);
                        cnt++;
                        break; //added by CHB to avoid problem caused by loops in swc, 20150313
                }
            }
        }
        offset += cnt;
    }

    if ((sorted_size)<siz) {
        v3d_msg(QString("Error!\nsorted_size:%1\nsize:%2").arg(sorted_size).arg(siz));
        return false;
    }

    // free space.
    neighbors.clear();
    return(true);
};


double computeDist2(const NeuronSWC & s1, const NeuronSWC & s2, double xscale, double yscale, double zscale)
{
    double xx = (s1.x-s2.x)*xscale;
    double yy = (s1.y-s2.y)*yscale;
    double zz = (s1.z-s2.z)*zscale;
    return sqrt(xx*xx+yy*yy+zz*zz);
};

bool combine_linker(vector<QList<NeuronSWC> > & linker, QList<NeuronSWC> & combined)
{
    V3DLONG neuronNum = linker.size();
    if (neuronNum<=0)
    {
        cout<<"the linker file is empty, please check your data."<<endl;
        return false;
    }
    V3DLONG offset = 0;
    combined = linker[0];
    for (V3DLONG i=1;i<neuronNum;i++)
    {
        V3DLONG maxid = -1;
        for (V3DLONG j=0;j<linker[i-1].size();j++)
            if (linker[i-1][j].n>maxid) maxid = linker[i-1][j].n;
        offset += maxid+1;
        for (V3DLONG j=0;j<linker[i].size();j++)
        {
            NeuronSWC S = linker[i][j];
            S.n = S.n+offset;
            if (S.pn>=0) S.pn = S.pn+offset;
            combined.append(S);
        }
    }
};

NeuronTree my_SortSWC(NeuronTree nt, V3DLONG newrootid, double thres){
    NeuronTree new_tree;
    QList<NeuronSWC> neuronlist;
//    v3d_msg(QString("id is %1, ths is %2").arg(newrootid).arg(thres));
//    writeSWC_file("C:/Users/pengx/Desktop/test/tmp.swc",nt);
    SortSWC(nt.listNeuron, neuronlist, newrootid, thres);
//    v3d_msg("done!");
    new_tree.deepCopy(neuronlist_2_neurontree(neuronlist));
    return new_tree;
}

bool export_list2file(QList<NeuronSWC> & lN, QString fileSaveName, QString fileOpenName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);
    myfile<<"# generated by Vaa3D Plugin sort_neuron_swc"<<endl;
    myfile<<"# source file(s): "<<fileOpenName<<endl;
    myfile<<"# id,type,x,y,z,r,pid"<<endl;
    for (V3DLONG i=0;i<lN.size();i++)
        myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";

    file.close();
    cout<<"swc file "<<fileSaveName.toStdString()<<" has been generated, size: "<<lN.size()<<endl;
    return true;
};

void connect_swc(NeuronTree nt,QList<NeuronSWC>& newNeuron, double disThr,double angThr)
{
    //rescale neurons
    QList<XYZ> scaledXYZ;
    for(V3DLONG i=0; i<nt.listNeuron.size(); i++){
        XYZ S;
        S.x = nt.listNeuron.at(i).x*1;
        S.y = nt.listNeuron.at(i).y*1;
        S.z = nt.listNeuron.at(i).z*4;
        scaledXYZ.append(S);
    }

    qDebug()<<"search for components and tips";
    //initialize tree components and get all tips
    QList<V3DLONG> cand;
    QList<XYZ> canddir;
    QVector<int> childNum(nt.listNeuron.size(), 0);
    QVector<int> connNum(nt.listNeuron.size(), 0);
    QList<V3DLONG> components;
    QList<V3DLONG> pList;
    V3DLONG curid=0;
    for(V3DLONG i=0; i<nt.listNeuron.size(); i++){
        if(nt.listNeuron.at(i).pn<0){
            connNum[i]--; //root that only have 1 child will also be a dead end
            components.append(curid); curid++;
            pList.append(-1);
        }else{
            V3DLONG pid = nt.hashNeuron.value(nt.listNeuron.at(i).pn);
            childNum[pid]++;
            connNum[pid]++;
            components.append(-1);
            pList.append(pid);
        }
    }

    qDebug()<<"components searching";
    //connected component
    for(V3DLONG cid=0; cid<curid; cid++){
        QStack<V3DLONG> pstack;
        V3DLONG chid;
        //recursively search for child and mark them as the same component
        pstack.push(components.indexOf(cid));
        while(!pstack.isEmpty()){
            V3DLONG pid=pstack.pop();
            chid = -1;
            chid = pList.indexOf(pid,chid+1);
            while(chid>=0){
                pstack.push(chid);
                components[chid]=cid;
                chid=pList.indexOf(pid,chid+1);
            }
        }
    }

    qDebug()<<"tips searching";
    vector< pair<int,int> > tip_pair;
    //get tips
    for(V3DLONG i=0; i<childNum.size(); i++){
        if(connNum.at(i)<1){
            cand.append(i);
            //get direction
            V3DLONG id=i;
            V3DLONG sid;
            if(childNum[id]==1){ //single child root
                sid = pList.indexOf(id);
            }else{ //tips
                sid = pList[id];
            }
            tip_pair.push_back(std::make_pair(id,sid));
        }
    }

    qDebug()<<connNum.size()<<":"<<childNum.size()<<":"<<cand.size();

    qDebug()<<"match tips";

    //match tips
    multimap<double, QVector<V3DLONG> > connMap;
    for(V3DLONG tid=0; tid<cand.size(); tid++){
        V3DLONG tidx=cand.at(tid);
        V3DLONG mvid=-1, mtid=-1;
        for(V3DLONG cid=0; cid<curid; cid++){
            if(cid==components.at(cand[tid])) continue;
            double mvdis=disThr, mtdis=disThr;
            V3DLONG id=components.indexOf(cid);
            while(id>=0){

                double dis=NTDIS(scaledXYZ.at(tidx),scaledXYZ.at(id));
                if(dis<mvdis){
                    mvdis=dis;
                    mvid=id;
                }
                if(dis<mtdis){
                    if(connNum.at(id)<1){//tips
                        V3DLONG tmpid=cand.indexOf(id);
                        double local_ang1 = angle(nt.listNeuron.at(tip_pair[tid].first),nt.listNeuron.at(tip_pair[tid].second),nt.listNeuron.at(tip_pair[tmpid].first));
                        double local_ang2 = angle(nt.listNeuron.at(tip_pair[tmpid].first),nt.listNeuron.at(tip_pair[tmpid].second),nt.listNeuron.at(tip_pair[tid].first));
                        if(local_ang1 >= angThr && local_ang2 >= angThr){
                            mtdis=dis;
                            mtid=id;
                        }
                    }
                }
                id=components.indexOf(cid, id+1);
            }

            if(mtid>=0){
                QVector<V3DLONG> tmp;
                tmp.append(tidx); tmp.append(mtid);
                connMap.insert(pair<double, QVector<V3DLONG> >(mtdis,tmp));
            }
        }
    }

    qDebug()<<"connecting tips";
    //find the best solution for connecting tips
    QMap<V3DLONG, QVector<V3DLONG> > connectPairs;
    for(multimap<double, QVector<V3DLONG> >::iterator iter=connMap.begin(); iter!=connMap.end(); iter++){
        if(components.at(iter->second.at(0))==components.at(iter->second.at(1))) //already connected
            continue;
        if(connectPairs.contains(iter->second.at(0))){
            connectPairs[iter->second.at(0)].append(iter->second.at(1));
        }else{
            QVector<V3DLONG> tmp; tmp.append(iter->second.at(1));
            connectPairs.insert(iter->second.at(0),tmp);
        }
        if(connectPairs.contains(iter->second.at(1))){
            connectPairs[iter->second.at(1)].append(iter->second.at(0));
        }else{
            QVector<V3DLONG> tmp; tmp.append(iter->second.at(0));
            connectPairs.insert(iter->second.at(1),tmp);
        }
        V3DLONG cid_0=components.at(iter->second.at(0));
        V3DLONG cid_1=components.at(iter->second.at(1));
        V3DLONG tmpid=components.indexOf(cid_1);
        while(tmpid>=0){
            components[tmpid]=cid_0;
            tmpid=components.indexOf(cid_1,tmpid+1);
        }
    }

    qDebug()<<"reconstruct neuron tree";
    //reconstruct tree
    QVector<V3DLONG> newid(nt.listNeuron.size(), -1);
    QVector<V3DLONG> newpn(nt.listNeuron.size(), -1); //id starts from 1, -1: not touched, 0: touched but overlap with parent
    curid=1;
    int rootID = -1;
    int rootidx=nt.hashNeuron.value(rootID);
    if(nt.listNeuron[rootidx].n != rootID)
        rootidx=-1;
    QVector<V3DLONG> prinode;
    if(rootidx!=-1){
        prinode.push_back(rootidx);
    }
    for(V3DLONG i=0; i<nt.listNeuron.size(); i++){
        if(nt.listNeuron[i].parent==-1){
            prinode.push_back(i);
        }
    }
    V3DLONG i=0;
    V3DLONG priIdx=0;
    while(1){
        if(priIdx<prinode.size()){
            i=prinode[priIdx];
            priIdx++;
        }else if(priIdx==prinode.size()){
            i=0;
            priIdx++;
        }else{
            i++;
            if(i>=nt.listNeuron.size())
                break;
        }
        if(newid[i]>0) continue;
        QQueue<V3DLONG> pqueue; pqueue.clear();
        pqueue.enqueue(i);
        newid[i]=curid++;
        while(!pqueue.isEmpty()){
            //add current node to the listNeuron
            V3DLONG oid=pqueue.dequeue();

            if(newid[oid]>0){
                NeuronSWC tmpNeuron;
                tmpNeuron.n = newid[oid];
                tmpNeuron.x = nt.listNeuron.at(oid).x;
                tmpNeuron.y = nt.listNeuron.at(oid).y;
                tmpNeuron.z = nt.listNeuron.at(oid).z;
                tmpNeuron.type = nt.listNeuron.at(oid).type;
                tmpNeuron.r = nt.listNeuron.at(oid).r;
                tmpNeuron.fea_val = nt.listNeuron.at(oid).fea_val;
                tmpNeuron.pn = newpn.at(oid);
                newNeuron.append(tmpNeuron);
            }

            //add current node's children/parent/new-neighbor to the stack
            //parent
            if(nt.listNeuron.at(oid).pn>=0){
                V3DLONG opid = nt.hashNeuron.value(nt.listNeuron.at(oid).pn);
                if(newid.at(opid)<0){
                    pqueue.enqueue(opid);
                    newpn[opid]=newid[oid];
                    newid[opid]=curid++;
                }
            }
            //child
            V3DLONG tmpid=pList.indexOf(oid);
            while(tmpid>=0){
                if(newid.at(tmpid)<0){
                    pqueue.enqueue(tmpid);
                    newpn[tmpid]=newid[oid];
                    newid[tmpid]=curid++;
                }
                tmpid=pList.indexOf(oid,tmpid+1);
            }
            //new-neighbor
            if(connectPairs.contains(oid)){
                for(V3DLONG j=0; j<connectPairs[oid].size(); j++){
                    V3DLONG onid=connectPairs[oid].at(j);
                    if(newid.at(onid)<0){
                        pqueue.enqueue(onid);
                        newpn[onid]=newid[oid];
                        newid[onid]=curid++;
                    }
                }
            }
        }
    }
}

NeuronTree pruneswc(NeuronTree nt, double length)
{
    QVector<QVector<V3DLONG> > childs;

    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    V3DLONG *flag = new V3DLONG[neuronNum];

    for (V3DLONG i=0;i<neuronNum;i++)
    {
        flag[i] = 1;

        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    QList<NeuronSWC> list = nt.listNeuron;

    for (int i=0;i<list.size();i++)
    {
        if (childs[i].size()==0 && list.at(i).parent >=0)
        {
            int index_tip = 0;
            int parent_tip = getParent(i,nt);
            while(childs[parent_tip].size()<2)
            {
                parent_tip = getParent(parent_tip,nt);
                index_tip++;
                if(parent_tip == 1000000000)
                    break;
            }
            if(index_tip < length)
            {
                flag[i] = -1;

                int parent_tip = getParent(i,nt);
                while(childs[parent_tip].size()<2)
               {
                    flag[parent_tip] = -1;
                    parent_tip = getParent(parent_tip,nt);
                    if(parent_tip == 1000000000)
                        break;
               }
            }

        }else if (childs[i].size()==0 && list.at(i).parent < 0)
            flag[i] = -1;

    }

   //NeutronTree structure
   NeuronTree nt_prunned;
   QList <NeuronSWC> listNeuron;
   QHash <int, int>  hashNeuron;
   listNeuron.clear();
   hashNeuron.clear();

   //set node

   NeuronSWC S;
   for (int i=0;i<list.size();i++)
   {
       if(flag[i] == 1)
       {
            NeuronSWC curr = list.at(i);
            S.n 	= curr.n;
            S.type 	= curr.type;
            S.x 	= curr.x;
            S.y 	= curr.y;
            S.z 	= curr.z;
            S.r 	= curr.r;
            S.pn 	= curr.pn;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);
       }

  }
   nt_prunned.n = -1;
   nt_prunned.on = true;
   nt_prunned.listNeuron = listNeuron;
   nt_prunned.hashNeuron = hashNeuron;

   if(flag) {delete[] flag; flag = 0;}
   return nt_prunned;
}


//bool SortSWC_old(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result, V3DLONG newrootid, double thres)
//{
//    qDebug()<<"Sort swc with new root name:"<<newrootid<<endl;
//    qDebug()<<"input size:"<<neurons.size();
//    //create a LUT, from the original id to the position in the listNeuron, different neurons with the same x,y,z & r are merged into one position
//    QHash<V3DLONG, V3DLONG> LUT = getUniqueLUT(neurons);

//    //create a new id list to give every different neuron a new id
//    QList<V3DLONG> idlist = ((QSet<V3DLONG>)LUT.values().toSet()).toList();

//    //create a child-parent table, both child and parent id refers to the index of idlist
//    QHash<V3DLONG, V3DLONG> cp = ChildParent(neurons,idlist,LUT);

////    V3DLONG siz = idlist.size();
//    V3DLONG siz = idlist.size();
//    qDebug()<<"size after dedup:"<<siz;


//    qDebug()<<"generate adjacent matrix";
//    unordered_map <int, int> matrix;


//    //generate the adjacent matrix for undirected matrix
//    for (V3DLONG i = 0;i<siz;i++)
//    {
//        QList<V3DLONG> parentSet = cp.values(i); //id of the ith node's parents
//        if(parentSet.size()>1){
//            qDebug()<<i<<parentSet.size();
//        }
//        for (V3DLONG j=0;j<parentSet.size();j++)
//        {
//            V3DLONG v2 = (V3DLONG) (parentSet.at(j));
//            if (v2==-1) continue;
//            matrix[i] = v2;
//        }
//    }

//    qDebug()<<"Reset root.";
//    //do a DFS for the the matrix and re-allocate ids for all the nodes
//    V3DLONG root = 0;
//    if (newrootid==VOID)
//    {
//        for (V3DLONG i=0;i<neurons.size();i++)
//            if (neurons.at(i).pn==-1){
//                root = idlist.indexOf(LUT.value(neurons.at(i).n));
//                break;
//            }
//    }
//    else{
//        root = idlist.indexOf(LUT.value(newrootid));
////        QList<V3DLONG> test = LUT.keys();
////        for(int i=0; i<test.size();i++){
////            qDebug()<<i<<test.at(i)<<test.indexOf(test.at(i));
////        }
////        qDebug()<<test.size()<<test.at(0)<<newrootid<<test.indexOf(newrootid);

//        if (LUT.keys().indexOf(newrootid)==-1)
//        {
//            v3d_msg("The new root id you have chosen does not exist in the SWC file.");
//            return(false);
//        }
//    }
//    qDebug()<<"Done reset root.";


//    V3DLONG* neworder = new V3DLONG[siz];
//    V3DLONG* numbered = new V3DLONG[siz];
//    for (V3DLONG i=0;i<siz;i++) numbered[i] = 0;

//    V3DLONG id[] = {0};
//    int group[] = {1};
//    cout<<"*id\t"<<*id<<endl;
//    cout<<"*group\t"<<*group<<endl;
//    try{
//        DFS(matrix,neworder,root,id,siz,numbered,group);
//        v3d_msg("check");
//    }
//    catch(exception& exc){
//        cout << "Standard exception: " << exc.what() << endl;
//    }
//    cout<<"Done 1st DFS"<<endl;
//    cout<<"*id\t"<<*id<<endl;

//    while (*id<siz)
//    {
//        qDebug()<<*id;
//        V3DLONG iter;
//        (*group)++;
//        for (iter=0;iter<siz;iter++)
//            if (numbered[iter]==0) break;
//        DFS(matrix,neworder,iter,id,siz,numbered,group);
//    }
//    qDebug()<<"Done DFS";

//    qDebug()<<"find the point in non-group 1 that is nearest to group 1";
//    //find the point in non-group 1 that is nearest to group 1,
//    //include the nearest point as well as its neighbors into group 1, until all the nodes are connected
//    while((*group)>1)
//    {
//        double min = VOID;
//        double dist2 = 0;
//        int mingroup = 1;
//        V3DLONG m1,m2;
//        for (V3DLONG ii=0;ii<siz;ii++){
//            if (numbered[ii]==1)
//                for (V3DLONG jj=0;jj<siz;jj++)
//                    if (numbered[jj]!=1)
//                    {
//                        dist2 = computeDist2(neurons.at(idlist.at(ii)),neurons.at(idlist.at(jj)));
//                        if (dist2<min)
//                        {
//                            min = dist2;
//                            mingroup = numbered[jj];
//                            m1 = ii;
//                            m2 = jj;
//                        }
//                    }
//        }
//        for (V3DLONG i=0;i<siz;i++)
//            if (numbered[i]==mingroup)
//                numbered[i] = 1;
//        if (min<=thres*thres)
//        {
//            matrix[m1]=m2;
//        }
//        (*group)--;
//    }

//    id[0] = 0;
//    for (int i=0;i<siz;i++)
//    {
//        numbered[i] = 0;
//        neworder[i]= VOID;
//    }

//    *group = 1;

//    V3DLONG new_root=root;
//    V3DLONG offset=0;
//    while (*id<siz)
//    {
//        V3DLONG cnt = 0;
//        DFS(matrix,neworder,new_root,id,siz,numbered,group);
//        (*group)++;
//        NeuronSWC S;
//        S.n = offset+1;
//        S.pn = -1;
//        V3DLONG oripos = idlist.at(new_root);
//        S.x = neurons.at(oripos).x;
//        S.y = neurons.at(oripos).y;
//        S.z = neurons.at(oripos).z;
//        S.r = neurons.at(oripos).r;
//        S.type = neurons.at(oripos).type;
//        result.append(S);
//        cnt++;

//        for (V3DLONG ii=offset+1;ii<(*id);ii++)
//        {
//            for (V3DLONG jj=offset;jj<ii;jj++) //after DFS the id of parent must be less than child's
//            {
//                if (neworder[ii]!=VOID && neworder[jj]!=VOID)// && matrix[neworder[ii]][neworder[jj]] )
//                {
//                        NeuronSWC S;
//                        S.n = ii+1;
//                        S.pn = jj+1;
//                        V3DLONG oripos = idlist.at(neworder[ii]);
//                        S.x = neurons.at(oripos).x;
//                        S.y = neurons.at(oripos).y;
//                        S.z = neurons.at(oripos).z;
//                        S.r = neurons.at(oripos).r;
//                        S.type = neurons.at(oripos).type;
//                        result.append(S);
//                        cnt++;

//                        break; //added by CHB to avoid problem caused by loops in swc, 20150313
//                }
//            }
//        }
//        for (new_root=0;new_root<siz;new_root++)
//            if (numbered[new_root]==0) break;
//        offset += cnt;
//    }

//    if ((*id)<siz) {
//        v3d_msg("Error!");
//        return false;
//    }

//    //free space by Yinan Wan 12-02-02
//    if (neworder) {delete []neworder; neworder=NULL;}
//    if (numbered) {delete []numbered; numbered=NULL;}
////    if (matrix){
////        for (V3DLONG i=0;i<siz;i++) {delete matrix[i]; matrix[i]=NULL;}
////        if (matrix) {delete []matrix; matrix=NULL;}
////    }


//    return(true);
//};

//void DFS_old(unordered_map <int, int>& matrix, V3DLONG* neworder, V3DLONG node, V3DLONG* id, V3DLONG siz, V3DLONG* numbered, int *group)
//{
//    try{
//        numbered[node];
//    }
//    catch(exception& exc){
//        cout<<"Standard exception of DFS:"<<exc.what()<<endl;
//    }
//    qDebug()<<"DFS: current node is"<<numbered[node];
//    if (!numbered[node]){
//        numbered[node] = *group;
//        neworder[*id] = node;
//        qDebug()<<"DFS: current node is"<<node<<*id;

//        (*id)++;
//        for (V3DLONG v=0;v<siz;v++)
//            if ((matrix[v] == node || matrix[node] == v))
//            {
//                try{
//                    DFS(matrix, neworder, v, id, siz,numbered,group);
//                }
//                catch(exception& exc){
//                    cout<<"Standard exception:"<<exc.what()<<endl;
//                }
//            }
//    }
//};
