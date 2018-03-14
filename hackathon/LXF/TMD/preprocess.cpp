#include "preprocess.h"
#include <math.h>
//#include <vector>
#define DIST(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))


bool prune_branch(NeuronTree nt, NeuronTree & result, double prune_size)
{

    V3DLONG siz = nt.listNeuron.size();
    vector<V3DLONG> branches(siz,0); //number of branches on the pnt: 0-tip, 1-internal, >=2-branch
    for (V3DLONG i=0;i<siz;i++)
    {
        if (nt.listNeuron[i].pn<0) continue;
        V3DLONG pid = nt.hashNeuron.value(nt.listNeuron[i].pn);
        branches[pid]++;
    }

    double diameter = calculate_diameter(nt, branches);
    printf("diameter=%.3f\n",diameter);

    if (prune_size  == -1 ){
        double thres = 0.05;
        prune_size = diameter * thres;
    }
    //calculate the shortest edge starting from each tip point
    vector<bool> to_prune(siz, false);
    for (V3DLONG i=0;i<siz;i++)
    {
        if (branches[i]!=0) continue;
        //only consider tip points
        vector<V3DLONG> segment;
        double edge_length = 0;
        V3DLONG cur = i;
        V3DLONG pid;
        do
        {
            NeuronSWC s = nt.listNeuron[cur];
            segment.push_back(cur);
            pid = nt.hashNeuron.value(s.pn);
            edge_length += DIST(s, nt.listNeuron[pid]);
            cur = pid;
        }
        while (branches[pid]==1 && pid>0);
        if (pid<0)
        {
            printf("The input tree has only 1 root point. Please check.\n");
            return false;
        }
        if (edge_length < prune_size)
        {
            for (int j=0;j<segment.size();j++)
                to_prune[segment[j]] = true;
        }
    }



    //prune branches
    result.listNeuron.clear();
    result.hashNeuron.clear();
    for (V3DLONG i=0;i<siz;i++)
    {
        if (!to_prune[i])
        {
            NeuronSWC s = nt.listNeuron[i];
            result.listNeuron.append(s);
            result.hashNeuron.insert(nt.listNeuron[i].n, result.listNeuron.size()-1);
        }
    }

    return true;
}

//the diameter of a tree is defined as the longest path between every pair of leafs in a rootless tree
double calculate_diameter(NeuronTree nt, vector<V3DLONG> branches)
{
    V3DLONG siz = nt.listNeuron.size();
    vector<vector<double> > longest_path(siz, vector<double>(2,0));//the 1st and 2nd longest path to each node in a rooted tree
    vector<V3DLONG> chd(siz, -1);//immediate child of the current longest path
    for (V3DLONG i=0;i<siz;i++)
    {
        if (branches[i]!=0) continue;
        V3DLONG cur = i;
        V3DLONG pid;
        do
        {
            NeuronSWC s = nt.listNeuron[cur];
            pid = nt.hashNeuron.value(s.pn);
            double dist = DIST(s, nt.listNeuron[pid]) + longest_path[cur][0];
            if (dist>longest_path[pid][0])
            {
                chd[pid] = cur;
                longest_path[pid][0] = dist;
            }
            else if (dist>longest_path[pid][1] && chd[pid]!=cur)
                longest_path[pid][1] = dist;
            cur = pid;
        }
        while (branches[cur]!=0 && pid>0);
    }

    double diam = -1;
    for (V3DLONG i=0;i<siz;i++)
    {
        if (longest_path[i][0] + longest_path[i][1]>diam)
            diam = longest_path[i][0]+longest_path[i][1];
    }
    return diam;
}

bool export_listNeuron_2swc(QList<NeuronSWC> & list, const char* filename)
{
    FILE * fp;
    fp = fopen(filename,"w");
    if (fp==NULL)
    {
        fprintf(stderr,"ERROR: %s: failed to open file to write!\n",filename);
        return false;
    }
    fprintf(fp,"##n,type,x,y,z,radius,parent\n");
    for (int i=0;i<list.size();i++)
    {
        NeuronSWC curr = list.at(i);
        fprintf(fp,"%d %d %.2f %.2f %.2f %.2f %d\n",curr.n,curr.type,curr.x,curr.y,curr.z,curr.r,curr.pn);
    }
    fclose(fp);
    return true;
}


void getChildNum(const NeuronTree &nt, vector<vector<V3DLONG> > &childs)
{
    V3DLONG nt_size=nt.listNeuron.size();
    //childs=vector<vector<V3DLONG> > (nt_size,vector<V3DLONG>() );
    for (V3DLONG i=0; i<nt_size;i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);

        //cout<<"i="<<i<<"   par="<<nt.hashNeuron.value(par)<<endl;
    }
}
vector<V3DLONG> get_parent_child(V3DLONG id,NeuronTree &nt, vector<vector<V3DLONG> > & childs_nt,int SEMI)
{
    vector<V3DLONG> points;
    V3DLONG id1=id;
    V3DLONG id2=id;
    cout<<"id1="<<id1<<endl;
    cout<<int(getParent(id1,nt))<<endl;
    for(int i=SEMI; i<0;i++)
    {
        if(int(getParent(id1,nt))>1000000)break;
        int cur_pid=getParent(id1,nt);
        points.push_back(cur_pid);
        id1=cur_pid;SEMI-=1;
    }
    for(int i=0;i<childs_nt[id2].size();i++)
    {
        V3DLONG cur_child = childs_nt[id2][i];
        for(int j=0;j<childs_nt[cur_child].size();j++)
        {
            V3DLONG cur_child2 = childs_nt[cur_child][j];
            points.push_back(cur_child2);
        }
        points.push_back(cur_child);
    }
    points.push_back(id);
    return points;
}

vector<V3DLONG> getTargetNode(NeuronTree & nt, V3DLONG t_num)
{
    vector<V3DLONG> target_node_list;
    vector<vector<V3DLONG> > childs_nt;
    V3DLONG siz=nt.listNeuron.size();
    getChildNum(nt,childs_nt);
    for(V3DLONG i=0;i<siz;i++)
    {
        V3DLONG b_num=0;
        if(nt.listNeuron[i].pn<0) b_num=childs_nt[i].size();
        else b_num=childs_nt[i].size()+1;
        if(b_num == t_num) target_node_list.push_back(i);
    }
    return target_node_list;
}
NeuronTree  sort(NeuronTree input, V3DLONG newrootid, double thres)
{
    NeuronTree result;
    QList<NeuronSWC>  neurons = input.listNeuron;
    //create a LUT, from the original id to the position in the listNeuron, different neurons with the same x,y,z & r are merged into one position
    QHash<V3DLONG, V3DLONG> LUT = getUniqueLUT(neurons);

    //create a new id list to give every different neuron a new id
    QList<V3DLONG> idlist = ((QSet<V3DLONG>)LUT.values().toSet()).toList();

    //create a child-parent table, both child and parent id refers to the index of idlist
    QHash<V3DLONG, V3DLONG> cp = ChildParent(neurons,idlist,LUT);


    V3DLONG siz = idlist.size();

    bool** matrix = new bool*[siz];
    for (V3DLONG i = 0;i<siz;i++)
    {
        matrix[i] = new bool[siz];
        for (V3DLONG j = 0;j<siz;j++) matrix[i][j] = false;
    }


    //generate the adjacent matrix for undirected matrix
    for (V3DLONG i = 0;i<siz;i++)
    {
        QList<V3DLONG> parentSet = cp.values(i); //id of the ith node's parents
        for (V3DLONG j=0;j<parentSet.size();j++)
        {
            V3DLONG v2 = (V3DLONG) (parentSet.at(j));
            if (v2==-1) continue;
            matrix[i][v2] = true;
            matrix[v2][i] = true;
        }
    }


    //do a DFS for the the matrix and re-allocate ids for all the nodes
    V3DLONG root;
    if (newrootid==VOID)
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
            return(result);
        }
    }


    V3DLONG* neworder = new V3DLONG[siz];
    int* numbered = new int[siz];
    for (V3DLONG i=0;i<siz;i++) numbered[i] = 0;

    V3DLONG id[] = {0};

    int group[] = {1};
    DFS(matrix,neworder,root,id,siz,numbered,group);

    while (*id<siz)
    {
        V3DLONG iter;
        (*group)++;
        for (iter=0;iter<siz;iter++)
            if (numbered[iter]==0) break;
        DFS(matrix,neworder,iter,id,siz,numbered,group);
    }


    //find the point in non-group 1 that is nearest to group 1,
    //include the nearest point as well as its neighbors into group 1, until all the nodes are connected
    while((*group)>1)
    {
        double min = VOID;
        double dist2 = 0;
        int mingroup = 1;
        V3DLONG m1,m2;
        for (V3DLONG ii=0;ii<siz;ii++){
            if (numbered[ii]==1)
                for (V3DLONG jj=0;jj<siz;jj++)
                    if (numbered[jj]!=1)
                    {
                        dist2 = computeDist2(neurons.at(idlist.at(ii)),neurons.at(idlist.at(jj)));
                        if (dist2<min)
                        {
                            min = dist2;
                            mingroup = numbered[jj];
                            m1 = ii;
                            m2 = jj;
                        }
                    }
        }
        for (V3DLONG i=0;i<siz;i++)
            if (numbered[i]==mingroup)
                numbered[i] = 1;
        if (min<=thres*thres)
        {
            matrix[m1][m2] = true;
            matrix[m2][m1] = true;
        }
        (*group)--;
    }

    id[0] = 0;
    for (int i=0;i<siz;i++)
    {
        numbered[i] = 0;
        neworder[i]= VOID;
    }

    *group = 1;

    V3DLONG new_root=root;
    V3DLONG offset=0;
    while (*id<siz)
    {
        V3DLONG cnt = 0;
        DFS(matrix,neworder,new_root,id,siz,numbered,group);
        (*group)++;
        NeuronSWC S;
        S.n = offset+1;
        S.pn = -1;
        V3DLONG oripos = idlist.at(new_root);
        S.x = neurons.at(oripos).x;
        S.y = neurons.at(oripos).y;
        S.z = neurons.at(oripos).z;
        S.r = neurons.at(oripos).r;
        S.type = neurons.at(oripos).type;
        S.seg_id = neurons.at(oripos).seg_id;
        S.level = neurons.at(oripos).level;
        S.fea_val = neurons.at(oripos).fea_val;


        result.listNeuron.append(S);
        cnt++;

        for (V3DLONG ii=offset+1;ii<(*id);ii++)
        {
            for (V3DLONG jj=offset;jj<ii;jj++) //after DFS the id of parent must be less than child's
            {
                if (neworder[ii]!=VOID && neworder[jj]!=VOID && matrix[neworder[ii]][neworder[jj]] )
                {
                        NeuronSWC S;
                        S.n = ii+1;
                        S.pn = jj+1;
                        V3DLONG oripos = idlist.at(neworder[ii]);
                        S.x = neurons.at(oripos).x;
                        S.y = neurons.at(oripos).y;
                        S.z = neurons.at(oripos).z;
                        S.r = neurons.at(oripos).r;
                        S.type = neurons.at(oripos).type;
                        S.seg_id = neurons.at(oripos).seg_id;
                        S.level = neurons.at(oripos).level;
                        S.fea_val = neurons.at(oripos).fea_val;
                        result.listNeuron.append(S);
                        cnt++;

                        break; //added by CHB to avoid problem caused by loops in swc, 20150313
                }
            }
        }
        for (new_root=0;new_root<siz;new_root++)
            if (numbered[new_root]==0) break;
        offset += cnt;
    }

    if ((*id)<siz) {
        v3d_msg("Error!");
        return result ;
    }

    //free space by Yinan Wan 12-02-02
    if (neworder) {delete []neworder; neworder=NULL;}
    if (numbered) {delete []numbered; numbered=NULL;}
    if (matrix){
        for (V3DLONG i=0;i<siz;i++) {delete matrix[i]; matrix[i]=NULL;}
        if (matrix) {delete []matrix; matrix=NULL;}
    }

    for (V3DLONG i=0;i<result.listNeuron.size();i++)
        result.hashNeuron.insert(result.listNeuron[i].n, i);
    return(result);
}

QHash<V3DLONG, V3DLONG> ChildParent(QList<NeuronSWC> &neurons, const QList<V3DLONG> & idlist, const QHash<V3DLONG,V3DLONG> & LUT)
{
    QHash<V3DLONG, V3DLONG> cp;
    for (V3DLONG i=0;i<neurons.size(); i++)
    {
        if (neurons.at(i).pn==-1)
            cp.insertMulti(idlist.indexOf(LUT.value(neurons.at(i).n)), -1);
        else if(idlist.indexOf(LUT.value(neurons.at(i).pn)) == 0 && neurons.at(i).pn != neurons.at(0).n)
            cp.insertMulti(idlist.indexOf(LUT.value(neurons.at(i).n)), -1);
        else
            cp.insertMulti(idlist.indexOf(LUT.value(neurons.at(i).n)), idlist.indexOf(LUT.value(neurons.at(i).pn)));
    }
        return cp;
}

QHash<V3DLONG, V3DLONG> getUniqueLUT(QList<NeuronSWC> &neurons)
{
    QHash<V3DLONG,V3DLONG> LUT;

    for (V3DLONG i=0;i<neurons.size();i++)
    {
        V3DLONG j;
        for (j=0;j<i;j++)
        {
            if (neurons.at(i).x==neurons.at(j).x && neurons.at(i).y==neurons.at(j).y && neurons.at(i).z==neurons.at(j).z)	break;
        }
        LUT.insertMulti(neurons.at(i).n,j);
    }
    return (LUT);
}


void DFS(bool** matrix, V3DLONG* neworder, V3DLONG node, V3DLONG* id, V3DLONG siz, int* numbered, int *group)
{
    if (!numbered[node]){
        numbered[node] = *group;
        neworder[*id] = node;
        (*id)++;
        for (V3DLONG v=0;v<siz;v++)
            if (!numbered[v] && matrix[v][node])
            {
                DFS(matrix, neworder, v, id, siz,numbered,group);
            }
    }
};
double computeDist2(const NeuronSWC & s1, const NeuronSWC & s2)
{
    double xx = s1.x-s2.x;
    double yy = s1.y-s2.y;
    double zz = s1.z-s2.z;
    return (xx*xx+yy*yy+zz*zz);
}

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
}
bool SortSWC(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result, V3DLONG newrootid, double thres)
{

    //create a LUT, from the original id to the position in the listNeuron, different neurons with the same x,y,z & r are merged into one position
    QHash<V3DLONG, V3DLONG> LUT = getUniqueLUT(neurons);

    //create a new id list to give every different neuron a new id
    QList<V3DLONG> idlist = ((QSet<V3DLONG>)LUT.values().toSet()).toList();

    //create a child-parent table, both child and parent id refers to the index of idlist
    QHash<V3DLONG, V3DLONG> cp = ChildParent(neurons,idlist,LUT);


    V3DLONG siz = idlist.size();

    bool** matrix = new bool*[siz];
    for (V3DLONG i = 0;i<siz;i++)
    {
        matrix[i] = new bool[siz];
        for (V3DLONG j = 0;j<siz;j++) matrix[i][j] = false;
    }


    //generate the adjacent matrix for undirected matrix
    for (V3DLONG i = 0;i<siz;i++)
    {
        QList<V3DLONG> parentSet = cp.values(i); //id of the ith node's parents
        for (V3DLONG j=0;j<parentSet.size();j++)
        {
            V3DLONG v2 = (V3DLONG) (parentSet.at(j));
            if (v2==-1) continue;
            matrix[i][v2] = true;
            matrix[v2][i] = true;
        }
    }


    //do a DFS for the the matrix and re-allocate ids for all the nodes
    V3DLONG root = 0;
    if (newrootid==VOID)
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


    V3DLONG* neworder = new V3DLONG[siz];
    int* numbered = new int[siz];
    for (V3DLONG i=0;i<siz;i++) numbered[i] = 0;

    V3DLONG id[] = {0};

    int group[] = {1};
    DFS(matrix,neworder,root,id,siz,numbered,group);

    while (*id<siz)
    {
        V3DLONG iter;
        (*group)++;
        for (iter=0;iter<siz;iter++)
            if (numbered[iter]==0) break;
        DFS(matrix,neworder,iter,id,siz,numbered,group);
    }


    //find the point in non-group 1 that is nearest to group 1,
    //include the nearest point as well as its neighbors into group 1, until all the nodes are connected
    while((*group)>1)
    {
        double min = VOID;
        double dist2 = 0;
        int mingroup = 1;
        V3DLONG m1,m2;
        for (V3DLONG ii=0;ii<siz;ii++){
            if (numbered[ii]==1)
                for (V3DLONG jj=0;jj<siz;jj++)
                    if (numbered[jj]!=1)
                    {
                        dist2 = computeDist2(neurons.at(idlist.at(ii)),neurons.at(idlist.at(jj)));
                        if (dist2<min)
                        {
                            min = dist2;
                            mingroup = numbered[jj];
                            m1 = ii;
                            m2 = jj;
                        }
                    }
        }
        for (V3DLONG i=0;i<siz;i++)
            if (numbered[i]==mingroup)
                numbered[i] = 1;
        if (min<=thres*thres)
        {
            matrix[m1][m2] = true;
            matrix[m2][m1] = true;
        }
        (*group)--;
    }

    id[0] = 0;
    for (int i=0;i<siz;i++)
    {
        numbered[i] = 0;
        neworder[i]= VOID;
    }

    *group = 1;

    V3DLONG new_root=root;
    V3DLONG offset=0;
    while (*id<siz)
    {
        V3DLONG cnt = 0;
        DFS(matrix,neworder,new_root,id,siz,numbered,group);
        (*group)++;
        NeuronSWC S;
        S.n = offset+1;
        S.pn = -1;
        V3DLONG oripos = idlist.at(new_root);
        S.x = neurons.at(oripos).x;
        S.y = neurons.at(oripos).y;
        S.z = neurons.at(oripos).z;
        S.r = neurons.at(oripos).r;
        S.type = neurons.at(oripos).type;
        result.append(S);
        cnt++;

        for (V3DLONG ii=offset+1;ii<(*id);ii++)
        {
            for (V3DLONG jj=offset;jj<ii;jj++) //after DFS the id of parent must be less than child's
            {
                if (neworder[ii]!=VOID && neworder[jj]!=VOID && matrix[neworder[ii]][neworder[jj]] )
                {
                        NeuronSWC S;
                        S.n = ii+1;
                        S.pn = jj+1;
                        V3DLONG oripos = idlist.at(neworder[ii]);
                        S.x = neurons.at(oripos).x;
                        S.y = neurons.at(oripos).y;
                        S.z = neurons.at(oripos).z;
                        S.r = neurons.at(oripos).r;
                        S.type = neurons.at(oripos).type;
                        result.append(S);
                        cnt++;

                        break; //added by CHB to avoid problem caused by loops in swc, 20150313
                }
            }
        }
        for (new_root=0;new_root<siz;new_root++)
            if (numbered[new_root]==0) break;
        offset += cnt;
    }

    if ((*id)<siz) {
        v3d_msg("Error!");
        return false;
    }

    //free space by Yinan Wan 12-02-02
    if (neworder) {delete []neworder; neworder=NULL;}
    if (numbered) {delete []numbered; numbered=NULL;}
    if (matrix){
        for (V3DLONG i=0;i<siz;i++) {delete matrix[i]; matrix[i]=NULL;}
        if (matrix) {delete []matrix; matrix=NULL;}
    }


    return(true);
};
