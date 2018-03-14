#include "v3d_message.h"
#include <vector>
#include "TMD_plugin.h"
#include "preprocess.h"
#include "basic_surf_objs.h"
#include "createTMD.h"
//#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))



struct node_xy
{
    int id;
    double x;
    double y;
};
int id_n = -1;
bool createTMD(const V3DPluginArgList & input, V3DPluginArgList & output);
bool calculate_radial_distance_up(int bin,int count,int root_c,int index,QHash<V3DLONG,V3DLONG> &A_path,vector<V3DLONG> &branches,vector<vector<V3DLONG> > &childs,vector<V3DLONG> &A,NeuronSWC root,NeuronTree &nt,QHash<V3DLONG,double> &f,QHash<V3DLONG,QSet<V3DLONG> >&relation,V3DLONG remote_leaf,vector<node_xy> &node_coord);
bool calculate_radial_distance_down(int bin,int count,QHash<V3DLONG,V3DLONG> &A_path,vector<V3DLONG> &branches,vector<V3DLONG> &A,NeuronSWC root,NeuronTree &nt,QHash<V3DLONG,double> &f,QHash<V3DLONG,QSet<V3DLONG> >&relation,V3DLONG branch_node,vector<node_xy> &node_coord);
double rad_dist(NeuronSWC swc1,NeuronSWC swc2);
bool make_A_path(QHash<V3DLONG,V3DLONG> &A_path,NeuronTree &nt,vector<V3DLONG> &leaf);
V3DLONG down_child(NeuronTree nt,vector<vector<V3DLONG> > childs,V3DLONG node);
bool if_is_leaf(NeuronTree &nt,QHash<V3DLONG,double> &f,V3DLONG m_id,V3DLONG branches,QHash<V3DLONG,QSet<V3DLONG> > &relation);
V3DLONG find_line(NeuronTree nt,V3DLONG n);
V3DLONG find_next_node(int bin,int count,QHash<V3DLONG,QSet<V3DLONG> > &relation,QHash<V3DLONG,double> &f,vector<V3DLONG> &A,QHash<int,QHash<int,V3DLONG> > &next_node,QHash<int,V3DLONG> &next_node_t,QHash<int,QHash<int,double> > &m_l_r,QHash<int,double> &m_l_r_t);
bool get_branch_f_of_all(int bin,int count,QHash<V3DLONG,V3DLONG> &A_path,QHash<V3DLONG,QSet<V3DLONG> > &relation,vector<V3DLONG> &A,vector<V3DLONG> &branches,NeuronTree &nt,V3DLONG m_id,QHash<int,V3DLONG> &max_it_t,QHash<int,QHash<int,V3DLONG> > &max_it,V3DLONG branch_node,NeuronSWC root,QHash<V3DLONG,double> &f,vector<node_xy> &node_coord,QHash<int,QHash<int,V3DLONG> > &discard);
V3DLONG get__branch_f(int bin,int count,QHash<V3DLONG,V3DLONG> &A_path,vector<V3DLONG> &branches,NeuronTree &nt,V3DLONG it1,V3DLONG it2,V3DLONG up_b,NeuronSWC root,QHash<V3DLONG,double> &f,vector<node_xy> &node_coord,QHash<int,QHash<int,V3DLONG> > &discard);
bool make_A(vector<V3DLONG> &A,NeuronTree &nt,vector<V3DLONG> &leaf,QHash<V3DLONG,double> &f,NeuronSWC root);
V3DLONG up_par(NeuronTree &nt,V3DLONG node,vector<vector<V3DLONG> > &childs);
V3DLONG find_remote_leaf(NeuronTree nt,vector<V3DLONG>leaf,NeuronSWC root,V3DLONG rd_id,QHash<V3DLONG,double> rd_v);
void find_child(NeuronTree &nt,V3DLONG par,vector<vector<V3DLONG> > &childs,vector<V3DLONG> &save_par,QHash<V3DLONG,QSet<V3DLONG> > &temp,QHash<V3DLONG,QSet<V3DLONG> >&relation,int &count_b);
void find_par(NeuronTree &nt,V3DLONG first_leaf,vector<vector<V3DLONG> > &childs,QHash<V3DLONG,QSet<V3DLONG> > &temp,V3DLONG del_child,QHash<V3DLONG,QSet<V3DLONG> >&relation,vector<V3DLONG> &save_par,int &count_bb);
double make_r(vector<node_xy>node);
double make_r_2(vector<node_xy>node);
double distance_W(vector<node_xy> node_coord1,vector<node_xy> node_coord2);
V3DLONG find_root(NeuronTree nt);
V3DLONG find_root(NeuronTree nt,vector<NeuronSWC> root,vector<V3DLONG> leaf,vector<V3DLONG> branches);
V3DLONG find_root(QHash<V3DLONG,QSet<V3DLONG> > &relation,vector<V3DLONG> &branches,NeuronTree &nt,vector<V3DLONG> &leaf);
V3DLONG find_final_root(vector<V3DLONG> &leaf,NeuronTree &nt,vector<V3DLONG> &root_cad);
void use_relation(QHash<V3DLONG,QSet<V3DLONG> > &relation,V3DLONG branch,NeuronTree &nt,QHash<V3DLONG,V3DLONG> &maker);
//void use_relation(QHash<V3DLONG,QSet<V3DLONG> > &relation,V3DLONG branch,NeuronTree &nt,V3DLONG &mark_rela,vector<V3DLONG> &temp_rela);
V3DLONG compare_remote_rela(V3DLONG max_id,V3DLONG i,NeuronTree &nt,vector<V3DLONG> &mark_remote_rela1,vector<V3DLONG> &mark_remote_rela2,V3DLONG branches1,V3DLONG branches2);


bool createTMD(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    printf("welcome to createTMD\n");


    vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
    vector<char*>* outlist = NULL;
    vector<char*>* paralist = NULL;
    QString searchName = QString(inlist->at(0));
    QString patternName = QString(inlist->at(1));
    NeuronTree tree1;
    NeuronTree tree2;
    string name1 = searchName.toStdString();
    string name2 = patternName.toStdString();
    cout<<name1<<endl;
    cout<<name2<<endl;
    tree1 = readSWC_file(searchName);
    tree2 = readSWC_file(patternName);


    double prune_size = 0; //default case


    if(input.size() != 1)
    {
        cout<<"input size"<<input.size()<<endl;
        printf("Please specify parameter set.\n");
        cout<<"hahahahhahah"<<endl;
        printHelp_createTMD();
        return false;
    }

    paralist = (vector<char*>*)(input.at(1).p);

/*
    if (paralist->size()!=1)
    {
        cout<<"ipara size"<<paralist->size()<<endl;
        printf("Please specify all paramters in one text string.\n");
        cout<<"hahahahhahah"<<endl;
        printHelp_createTMD();
        return false;
    }*/

    int big_or_little;
    big_or_little = 1;
    NeuronTree prune1;
    NeuronTree prune2;
    if(big_or_little)
    {
        if (!prune_branch(tree1, prune1,prune_size))
        {
            fprintf(stderr,"Error in prune_short_branch.\n");
            return 1;
        }


        if (!prune_branch(tree2, prune2,prune_size))
        {
            fprintf(stderr,"Error in prune_short_branch.\n");
            return 1;
        }
        prune1.hashNeuron.clear();
        prune2.hashNeuron.clear();
        for(V3DLONG j=0; j<prune1.listNeuron.size();j++)
        {
           prune1.hashNeuron.insert(prune1.listNeuron[j].n, j);
        }
        for(V3DLONG j=0; j<prune2.listNeuron.size();j++)
        {
           prune2.hashNeuron.insert(prune2.listNeuron[j].n, j);
        }

    }



    NeuronTree pruned1;
    NeuronTree pruned2;



    V3DLONG area_size1 = prune1.listNeuron.size();
    V3DLONG area_size2 = prune2.listNeuron.size();
    vector<vector<V3DLONG> > childs1;
    vector<vector<V3DLONG> > childs2;
    childs1 = vector< vector<V3DLONG> >(area_size1, vector<V3DLONG>() );
    childs2 = vector< vector<V3DLONG> >(area_size2, vector<V3DLONG>() );
    bool mark1 = 0;
    V3DLONG temp_root1;
    V3DLONG temp_par1;
    int c1 = 0;
    bool mark2 = 0;
    V3DLONG temp_root2;
    V3DLONG temp_par2;
    vector<NeuronSWC> root1;
    vector<NeuronSWC> root2;
    int c2 = 0;
    for(V3DLONG i = 0;i<area_size1;i++)
    {
        for(V3DLONG j = 0;j<area_size1;j++)
        {

            if(prune1.listNeuron[i].pn != prune1.listNeuron[j].n)
            {
                c1 = c1 + 1;
            }
            if(prune1.listNeuron[i].pn == prune1.listNeuron[j].n)
            {
                c1=0;
                break;

            }
            if(c1 == area_size1)
            {
                mark1 = 1;
            }
            if(mark1)
            {
                temp_root1 = prune1.listNeuron[i].n;
                temp_par1 = i;
                root1.push_back(prune1.listNeuron[i]);
                prune1.listNeuron[i].pn = -2;
                break;
            }

        }
        if(mark1)
        {
            break;
        }
    }
    for(V3DLONG i = 0;i<area_size2;i++)
    {
        for(V3DLONG j = 0;j<area_size2;j++)
        {
            //cout<<"i = "<<i<<endl;
            //cout<<"j = "<<j<<endl;
            if(prune2.listNeuron[i].pn != prune2.listNeuron[j].n)
            {
                c2 = c2 + 1;
                //cout<<pruned2.listNeuron[i].pn<<"   "<<pruned2.listNeuron[j].n<<endl;
                //cout<<"c2 = "<<c2<<endl;

            }
            if(prune2.listNeuron[i].pn == prune2.listNeuron[j].n)
            {
                c2=0;
                break;

            }
            if(c2 == area_size2)
            {
                mark2 = 1;
            }
            if(mark2)
            {
                temp_root2 = prune2.listNeuron[i].n;
                temp_par2 = i;
                root2.push_back(prune2.listNeuron[i]);
                prune2.listNeuron[i].pn = -2;
                //pruned2.listNeuron[i].n = -1;
                //pruned2.listNeuron[i]
                break;
            }
            //cout<<"PPPPPPPPPPPPPp"<<endl;
            //cout<<"c2 = "<<c<<endl;
        }
        if(mark2)
        {
            break;
        }
    }
    cout<<"root = "<<temp_root1<<endl;
    cout<<"par = "<<temp_par1<<endl;
    getChildNum(prune1,childs1);
    getChildNum(prune2,childs2);
    cout<<"yyyyyyyyyyyyyyyyyyyyyyyyyyyyy"<<endl;
    for(V3DLONG i=0;i<childs1[temp_par1].size();i++)
    {
       // cout<<"9child = "<<pruned1.listNeuron[childs1[temp_par1][i]].pn<<endl;
        //pruned1.listNeuron[childs1[temp_par1][i]].pn = -1;
    }




    for(V3DLONG i=0;i<childs2[temp_par2].size();i++)
    {
        //pruned2.listNeuron[childs2[temp_par2][i]].pn = -1;
    }




    for(V3DLONG i = 0;i<area_size2;i++)
    {

         cout<<prune2.listNeuron[i].n<<"   "<<prune2.listNeuron[i].pn<<endl;

    }



    cout<<"**********************find branches nodes__pre**************************"<<endl;
    vector<V3DLONG> branches1;
    vector<V3DLONG> branches2;
    for(V3DLONG i = 0;i<area_size1;i++)
    {
        if(childs1[i].size()>1)
        {
            int w = prune1.listNeuron[i].n;
            branches1.push_back(w);
        }
    }
    cout<<branches1.size()<<endl;
    for(V3DLONG i = 0;i<area_size2;i++)
    {
        if(childs2[i].size()>1)
        {
            int l = prune2.listNeuron[i].n;
            branches2.push_back(l);
        }
    }
    cout<<branches2.size()<<endl;
    for(V3DLONG i=0;i<branches1.size();i++)
    {
        cout<<branches1[i]<<endl;
    }



    cout<<"**********************find leaf nodes__pre**************************"<<endl;
    vector<V3DLONG> leaf1;
    vector<V3DLONG> leaf2;
    for(V3DLONG i = 0;i<area_size1;i++)
    {
        if(childs1[i].size()==0)
        {
            //int j = sorted1[0].listNeuron[i].n;
            int j = prune1.listNeuron[i].n;
            leaf1.push_back(j);
            cout<<"j = "<<j<<endl;
        }
    }
    cout<<leaf1.size()<<endl;
    for(V3DLONG i = 0;i<area_size2;i++)
    {
        if(childs2[i].size()==0)
        {
            //int l = sorted2[0].listNeuron[i].n;
            int l = prune2.listNeuron[i].n;
            leaf2.push_back(l);
           //cout<<"l = "<<l<<endl;
        }
    }
    cout<<leaf2.size()<<endl;

    cout<<"*************************find relationship__pre*********************"<<endl;
    QHash<V3DLONG,QSet<V3DLONG> > relation1;
    QHash<V3DLONG,QSet<V3DLONG> > relation2;
    QHash<V3DLONG,QSet<V3DLONG> > temp1;
    QHash<V3DLONG,QSet<V3DLONG> > temp2;
    V3DLONG remote_leaf1;
    V3DLONG remote_leaf2;
    V3DLONG rd_id1;
    V3DLONG rd_id2;
    vector<V3DLONG> save_par1;
    vector<V3DLONG> save_par2;
    QHash<V3DLONG,double> rd_v1;
    QHash<V3DLONG,double> rd_v2;
    remote_leaf1 = find_remote_leaf(prune1,leaf1,root1[0],rd_id1,rd_v1);
    remote_leaf2 = find_remote_leaf(prune2,leaf2,root2[0],rd_id2,rd_v2);
    cout<<"remote_leaf1 = "<<remote_leaf1<<endl;
    cout<<"remote_leaf2 = "<<remote_leaf2<<endl;
    V3DLONG U1;
    V3DLONG U2;
    U1 = find_line(prune1,remote_leaf1);
    U2 = find_line(prune2,remote_leaf2);
    save_par1.push_back(U1);
    save_par2.push_back(U2);
    int count_1=0;
    int count_2=0;
    find_par(prune1,U1,childs1,temp1,U1,relation1,save_par1,count_1);
    find_par(prune2,U2,childs2,temp2,U2,relation2,save_par2,count_2);
    cout<<"relationship  size = "<<relation1.size()<<endl;


    int y=find_root(relation1,branches1,prune1,leaf1);
    cout<<"y = "<<y<<endl;
    int t=find_root(relation2,branches2,prune2,leaf2);
    cout<<"t = "<<t<<endl;


    cout<<"*************************Sort***********************"<<endl;
    vector<NeuronTree> sorted1;
    vector<NeuronTree> sorted2;

    sorted1.push_back(sort(prune1,y,0));
    sorted2.push_back(sort(prune2,t,0));
    QString name11="sort1.swc";
    QString name22="sort2.swc";
    writeSWC_file(name11,sorted1[0]);
    writeSWC_file(name22,sorted2[0]);


    childs1.clear();
    childs2.clear();
    childs1 = vector< vector<V3DLONG> >(area_size1, vector<V3DLONG>() );
    childs2 = vector< vector<V3DLONG> >(area_size2, vector<V3DLONG>() );
    cout<<"size ===="<<childs1.size()<<endl;
    getChildNum(sorted1[0],childs1);
    getChildNum(sorted2[0],childs2);
    area_size1 = sorted1[0].listNeuron.size();
    area_size2 = sorted2[0].listNeuron.size();





    cout<<"**********************find branches nodes**************************"<<endl;
    branches1.clear();
    branches2.clear();
    for(V3DLONG i = 0;i<area_size1;i++)
    {
        if(childs1[i].size()>1)
        {
            cout<<"i = "<<i<<endl;
            cout<<childs1[i].size()<<endl;
            int w = sorted1[0].listNeuron[i].n;
            branches1.push_back(w);
        }
    }
    cout<<"size bran1 = "<<branches1.size()<<endl;
    for(V3DLONG i = 0;i<area_size2;i++)
    {
        if(childs2[i].size()>1)
        {
            int l = sorted2[0].listNeuron[i].n;
            branches2.push_back(l);
        }
    }
    cout<<"size bran2 = "<<branches2.size()<<endl;
    for(V3DLONG i=0;i<branches1.size();i++)
    {
        cout<<branches1[i]<<endl;
    }



    cout<<"**********************find leaf nodes**************************"<<endl;
    leaf1.clear();
    leaf2.clear();
    for(V3DLONG i = 0;i<area_size1;i++)
    {
        if(childs1[i].size()==0)
        {
            int j = sorted1[0].listNeuron[i].n;
            //int j = prune1.listNeuron[i].n;
            leaf1.push_back(j);
            //cout<<"j = "<<j<<endl;
        }
    }
    cout<<"leaf1 size"<<leaf1.size()<<endl;
    for(V3DLONG i = 0;i<area_size2;i++)
    {
        if(childs2[i].size()==0)
        {
            int l = sorted2[0].listNeuron[i].n;
            //int l = prune2.listNeuron[i].n;
            leaf2.push_back(l);
           //cout<<"l = "<<l<<endl;
        }
    }
    cout<<"leaf2 size"<<leaf2.size()<<endl;

    cout<<"*************************find relationship*********************"<<endl;
    relation1.clear();
    relation2.clear();
    vector<NeuronSWC> true_root1;
    vector<NeuronSWC> true_root2;
    temp1.clear();
    temp2.clear();
    rd_id1=0;
    rd_id2=0;
    save_par1.clear();
    save_par2.clear();
    rd_v1.clear();
    rd_v2.clear();
    true_root1.push_back(sorted1[0].listNeuron[0]);
    true_root2.push_back(sorted2[0].listNeuron[0]);
    remote_leaf1 = find_remote_leaf(sorted1[0],leaf1,true_root1[0],rd_id1,rd_v1);
    remote_leaf2 = find_remote_leaf(sorted2[0],leaf2,true_root2[0],rd_id2,rd_v2);
    cout<<"remote_leaf1 = "<<remote_leaf1<<endl;
    cout<<"remote_leaf2 = "<<remote_leaf2<<endl;
    U1 = find_line(sorted1[0],remote_leaf1);
    U2 = find_line(sorted2[0],remote_leaf2);
    save_par1.push_back(U1);
    save_par2.push_back(U2);
    //find_par(pruned1,U1,childs1,temp1,U1,relation1,save_par1);
    count_1=0;
    count_2=0;
    find_par(sorted1[0],U1,childs1,temp1,U1,relation1,save_par1,count_1);
    find_par(sorted2[0],U2,childs2,temp2,U2,relation2,save_par2,count_2);
    cout<<"relationship  size = "<<relation1.size()<<endl;


    cout<<"*************************Do TMD*********************"<<endl;
    vector<V3DLONG> A1;
    QHash<V3DLONG,double> f1;
    vector<node_xy>   node_coord1;
    vector<V3DLONG> A2;
    QHash<V3DLONG,V3DLONG> A_path2;
    QHash<V3DLONG,V3DLONG> A_path1;
    QHash<V3DLONG,double> f2;
    vector<node_xy> node_coord2;
    make_A(A1,sorted1[0],leaf1,f1,true_root1[0]);
    make_A(A2,sorted2[0],leaf2,f2,true_root2[0]);
    make_A_path(A_path1,sorted1[0],leaf1);
    make_A_path(A_path2,sorted2[0],leaf2);
    int root1_c=childs1[find_line(sorted1[0],sorted1[0].listNeuron[0].n)].size();
    int root2_c=childs2[find_line(sorted2[0],sorted2[0].listNeuron[0].n)].size();
    cout<<"root1c = "<<root1_c<<endl;
    cout<<"root2c = "<<root2_c<<endl;
    calculate_radial_distance_up(0,0,root1_c,1,A_path1,branches1,childs1,A1,true_root1[0],sorted1[0],f1,relation1,remote_leaf1,node_coord1);

    id_n = -1;
    cout<<"++++++++++++++++++++++++++++++++++=first has done++++++++++++++++++++++++++++++++"<<endl;
    calculate_radial_distance_up(0,0,root2_c,1,A_path2,branches2,childs2,A2,true_root2[0],sorted2[0],f2,relation2,remote_leaf2,node_coord2);
    node_xy n;
    n.id = node_coord1.size();
    n.x = 0;
    n.y = f1[A_path1[1]];
    node_coord1.push_back(n);
    cout<<"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^6"<<endl;
    node_xy q;
    q.id = node_coord2.size();
    q.x = 0;
    q.y = f2[A_path2[1]];
    node_coord2.push_back(q);








    for(V3DLONG i=0;i<node_coord1.size();i++)
    {
        cout<<node_coord1[i].id<<"    "<<node_coord1[i].x<<"    "<<node_coord1[i].y<<endl;
    }

      cout<<"LLLLLLLLLLLLLLLLLLLLLL"<<endl;

    for(V3DLONG i=0;i<node_coord2.size();i++)
    {
        cout<<node_coord2[i].id<<"         "<<node_coord2[i].x<<"    "<<node_coord2[i].y<<endl;
    }

    for(V3DLONG i=0;i<relation1.size();i++)
    {
        for(QSet<V3DLONG>::iterator it =relation1[find_line(sorted1[0],branches1[i])].begin();it != relation1[find_line(sorted1[0],branches1[i])].end();it++)
        {
            cout<<"relation1["<<find_line(sorted1[0],branches1[i])<<"] = "<<*it<<endl;

        }
    }
    cout<<"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"<<endl;
    for(V3DLONG i=0;i<relation2.size();i++)
    {
        for(QSet<V3DLONG>::iterator it =relation2[find_line(sorted2[0],branches2[i])].begin();it != relation2[find_line(sorted2[0],branches2[i])].end();it++)
        {
            cout<<"relation2["<<find_line(sorted2[0],branches2[i])<<"] = "<<*it<<endl;

        }
    }

    cout<<"distance_w = "<<distance_W(node_coord1,node_coord2)<<endl;
















//    cout<<"temp1.size = "<<temp1.size()<<endl;
//    cout<<"temp2.size = "<<temp2.size()<<endl;
//    for(V3DLONG i=0;i<temp1.size();i++)
//    {
//        for(QSet<V3DLONG>::iterator it =temp1[find_line(sorted1[0],branches1[i])].begin();it != temp1[find_line(sorted1[0],branches1[i])].end();it++)
//        {
//            cout<<"temp1 = "<<*it<<endl;

//        }

//    }
//    cout<<"qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq"<<endl;
//    for(V3DLONG i=0;i<temp2.size();i++)
//    {
//        for(QSet<V3DLONG>::iterator it =temp2[find_line(sorted2[0],branches2[i])].begin();it != temp2[find_line(sorted2[0],branches2[i])].end();it++)
//        {
//            cout<<"temp2["<<find_line(sorted2[0],branches2[i])<<"] = "<<*it<<endl;

//        }
//    }
/*


    //cout<<"rad = "<<rad_dist(root1[0],pruned2.listNeuron[4]);
    cout<<"r1 = "<<fabs(make_r_2(node_coord1))<<endl;
    cout<<"r2 = "<<fabs(make_r_2(node_coord2))<<endl;
    cout<<"distance = "<<fabs(make_r_2(node_coord1))-fabs(make_r_2(node_coord2))<<endl;

    cout<<leaf1.size()<<endl;
    cout<<leaf2.size()<<endl;


*/












/*


    cout<<relation1.size()<<endl;
    cout<<temp1.size()<<endl;
    cout<<save_par1.size()<<endl;
    cout<<relation2.size()<<endl;
    cout<<temp2.size()<<endl;
    cout<<save_par2.size()<<endl;
    for(V3DLONG i=0;i<relation1.size();i++)
    {
        for(QSet<V3DLONG>::iterator it =relation1[find_line(pruned1,branches1[i])].begin();it != relation1[find_line(pruned1,branches1[i])].end();it++)
        {
            cout<<"relation1 = "<<*it<<endl;

        }
        cout<<"&&&&&&&&&&&&&&&&&&&&&&&&&"<<endl;
    }
    for(V3DLONG i=0;i<relation2.size();i++)
    {
        for(QSet<V3DLONG>::iterator it =relation2[find_line(pruned2,branches2[i])].begin();it != relation2[find_line(pruned2,branches2[i])].end();it++)
        {
            cout<<"relation2["<<find_line(pruned2,branches2[i])<<"] = "<<*it<<endl;

        }
    }
    for(QSet<V3DLONG>::iterator it =temp2.begin();it != temp2.end();it++)
    {
        //relation[g].insert(*it);
        //cout<<"it= "<<*it<<endl;
    }
    for(QSet<V3DLONG>::iterator it =temp1.begin();it != temp1.end();it++)
    {
        //relation[g].insert(*it);
        cout<<"it= "<<*it<<endl;
    }
*/

}

double distance_W(vector<node_xy> node_coord1,vector<node_xy> node_coord2)
{
    double dis;
    double sum_w=0;
    if(node_coord1.size()==node_coord2.size())
    {
        cout<<"they are the same size"<<endl;
        for(V3DLONG i=0;i<node_coord1.size();i++)
        {
            dis = (node_coord1[i].x-node_coord2[i].x)*(node_coord1[i].x-node_coord2[i].x)+(node_coord1[i].y-node_coord2[i].y)*(node_coord1[i].y-node_coord2[i].y);
            dis = sqrt(dis);
            sum_w = sum_w+dis;

        }
        return sum_w;
    }
    else
    {
        cout<<"this size is not fit"<<endl;
        double min_w1=VOID;
        double min_w2=VOID;
        int i1;
        int i2;
        int num_w=abs(node_coord1.size()-node_coord2.size());
        cout<<node_coord1.size()-node_coord2.size()<<endl;
        cout<<"w="<<num_w<<endl;
        if(num_w==1)
        {
            cout<<"w="<<num_w<<endl;
            cout<<node_coord1.size()<<endl;
            cout<<node_coord2.size()<<endl;
            if(node_coord1.size()==node_coord2.size()+1)
            {
                for(V3DLONG i=0;i<node_coord1.size();i++)
                {
                    if(min_w1>fabs(node_coord1[i].x-node_coord1[i].y))
                    {
                        min_w1=fabs(node_coord1[i].x-node_coord1[i].y);
                        i1=i;
                    }
                }
                node_coord1.erase(node_coord1.begin()+i1);

            }

            else if(node_coord2.size()==node_coord1.size()+1)
            {
                for(V3DLONG i=0;i<node_coord2.size();i++)
                {
                    if(min_w1>fabs(node_coord2[i].x-node_coord2[i].y))
                    {
                        min_w1=fabs(node_coord2[i].x-node_coord2[i].y);
                        i1=i;
                    }
                }
                cout<<"i1 = "<<i1<<endl;
                node_coord2.erase(node_coord2.begin()+i1);
            }
            cout<<"before erase"<<endl;
            cout<<"i1 = "<<i1<<endl;

            cout<<"oooooooooooooooooooooooooooooooooooooooo"<<endl;
            for(V3DLONG i=0;i<node_coord1.size();i++)
            {
                cout<<node_coord1[i].id<<"    "<<node_coord1[i].x<<"    "<<node_coord1[i].y<<endl;
            }

            for(V3DLONG i=0;i<node_coord2.size();i++)
            {
                cout<<node_coord2[i].id<<"    "<<node_coord2[i].x<<"    "<<node_coord2[i].y<<endl;
            }

            cout<<"node1.size = "<<node_coord1.size()<<endl;
            cout<<"node2.size = "<<node_coord2.size()<<endl;
            for(V3DLONG i=0;i<node_coord2.size();i++)
            {
                dis = (node_coord1[i].x-node_coord2[i].x)*(node_coord1[i].x-node_coord2[i].x)+(node_coord1[i].y-node_coord2[i].y)*(node_coord1[i].y-node_coord2[i].y);
                dis = sqrt(dis);
                sum_w = sum_w+dis;

            }
            return sum_w;

        }
        else if(num_w==2)
        {
            cout<<"w=2"<<endl;
            if(node_coord1.size()==node_coord2.size()+2)
            {
                for(V3DLONG i=0;i<node_coord1.size();i++)
                {
                    if(min_w1>fabs(node_coord1[i].x-node_coord1[i].y))
                    {
                        min_w1=fabs(node_coord1[i].x-node_coord1[i].y);
                        i1=i;
                    }
                }
                node_coord1.erase(node_coord1.begin()+i1);
                for(V3DLONG i=0;i<node_coord1.size();i++)
                {
                    if(min_w2>fabs(node_coord1[i].x-node_coord1[i].y))
                    {
                        min_w2=fabs(node_coord1[i].x-node_coord1[i].y);
                        i2=i;
                    }
                }
                node_coord1.erase(node_coord1.begin()+i2);
            }
            else if(node_coord2.size()==node_coord1.size()+2)
            {
                for(V3DLONG i=0;i<node_coord2.size();i++)
                {
                    if(min_w1>fabs(node_coord2[i].x-node_coord2[i].y))
                    {
                        min_w1=fabs(node_coord2[i].x-node_coord2[i].y);
                        i1=i;
                    }
                }
                node_coord2.erase(node_coord2.begin()+i1);
                for(V3DLONG i=0;i<node_coord2.size();i++)
                {
                    if(min_w2>fabs(node_coord2[i].x-node_coord2[i].y))
                    {
                        min_w2=fabs(node_coord2[i].x-node_coord2[i].y);
                        i2=i;
                    }
                }
                node_coord2.erase(node_coord2.begin()+i2);
            }
            cout<<"i1 = "<<i1<<endl;
            cout<<"i2 = "<<i2<<endl;
            cout<<"node1.size = "<<node_coord1.size()<<endl;
            cout<<"node2.size = "<<node_coord2.size()<<endl;
            for(V3DLONG i=0;i<node_coord2.size();i++)
            {
                dis = (node_coord1[i].x-node_coord2[i].x)*(node_coord1[i].x-node_coord2[i].x)+(node_coord1[i].y-node_coord2[i].y)*(node_coord1[i].y-node_coord2[i].y);
                dis = sqrt(dis);
                sum_w = sum_w+dis;

            }
            return sum_w;

        }





        else
        {
            cout<<"iiiiiiiiiiiiiii"<<endl;
            return VOID;
        }


    }


}
V3DLONG find_root(QHash<V3DLONG,QSet<V3DLONG> > &relation,vector<V3DLONG> &branches,NeuronTree &nt,vector<V3DLONG> &leaf)
{
    QHash<V3DLONG,V3DLONG> maker;
    vector<V3DLONG> root_cad;
    for(V3DLONG i=0;i<branches.size();i++)
    {
        maker.insert(find_line(nt,branches[i]),0);
        cout<<"find_line(nt,branches[i]) = "<<find_line(nt,branches[i])<<endl;
    }
    int max_id;
    int max_rela=0;
    for(V3DLONG i=0;i<branches.size();i++)
    {
        use_relation(relation,branches[i],nt,maker);
    }
    for(V3DLONG i=0;i<branches.size();i++)
    {
        if(maker[find_line(nt,branches[i])]>max_rela)
        {
            max_rela=maker[find_line(nt,branches[i])];
            cout<<"max_rela = "<<max_rela<<endl;
            max_id=branches[i];
        }
    }

    cout<<"ttttttttttttt"<<endl;
    for(QHash<V3DLONG,V3DLONG>::iterator it = maker.begin();it != maker.end();it++)
    {
        cout<<it.key()<<"     "<<it.value()<<endl;
        if(*it==max_rela)
        {
            root_cad.push_back(it.key());
        }
    }
    cout<<"root_cad.size = "<<root_cad.size()<<endl;
    if(root_cad.size()>1)
    {
        max_id=find_final_root(leaf,nt,root_cad);
        max_id=nt.listNeuron[max_id].n;
    }


    //special
    if(branches.size()==1)
    {
        max_id = branches[0];
    }

    cout<<"branches_root = "<<max_id<<endl;
    return max_id;
}
V3DLONG find_final_root(vector<V3DLONG> &leaf,NeuronTree &nt,vector<V3DLONG> &root_cad)
{
    cout<<"enter into find final root"<<endl;
    vector<double> score;
    vector<double> D;
    double sc;
    double sum;
    double avr;
    double d;
    double min=VOID;
    int min_id;
    for(V3DLONG i=0;i<root_cad.size();i++)
    {
        cout<<"i = "<<i<<endl;
        avr=0;
        sum=0;
        d=0;
        score.clear();
        for(V3DLONG j=0;j<leaf.size();j++)
        {
            //cout<<"j = "<<j<<endl;
            sc = rad_dist(nt.listNeuron[root_cad[i]],nt.listNeuron[find_line(nt,leaf[i])]);
            score.push_back(sc);
        }
        for(V3DLONG k=0;k<score.size();k++)
        {
            sum=sum+score[i];
        }
        avr=sum/score.size();
        for(V3DLONG l=0;l<score.size();l++)
        {
            d=d+(avr-score[i])*(avr-score[i]);
        }
        D.push_back(d);
    }
    for(V3DLONG i=0;i<D.size();i++)
    {
        if(min>D[i])
        {
            min=D[i];
            min_id=i;
        }
    }
    return root_cad[min_id];
}
void use_relation(QHash<V3DLONG,QSet<V3DLONG> > &relation,V3DLONG branch,NeuronTree &nt,QHash<V3DLONG,V3DLONG> &maker)
{
    //vector<V3DLONG> temp_rela;
    //for(V3DLONG i=0;i<relation[find_line(nt,branch)].size();i++)//find_line
    {
        for(QSet<V3DLONG>::iterator it =relation[find_line(nt,branch)].begin();it != relation[find_line(nt,branch)].end();it++)
        {
            if(relation[*it].size()>0)
            {
                maker[find_line(nt,branch)]++;
                maker[*it]++;
                cout<<"*it = "<<*it<<endl;
                cout<<"maker[*it] = "<<maker[*it]<<endl;
            }
        }
    }
}
V3DLONG compare_remote_rela(V3DLONG max_id,V3DLONG i,NeuronTree &nt,vector<V3DLONG> &mark_remote_rela1,vector<V3DLONG> &mark_remote_rela2,V3DLONG branches1,V3DLONG branches2)
{
    double max_dis1=0;
    double max_dis2=0;
    double each_dis;
    //int id_max1;
    //int id_max2;
    for(V3DLONG i=0;i<mark_remote_rela1.size();i++)
    {
        each_dis=rad_dist(nt.listNeuron[find_line(nt,mark_remote_rela1[i])],nt.listNeuron[find_line(nt,branches1)]);
        if(each_dis>max_dis1)
        {
            //id_max1=i;
            max_dis1=each_dis;
        }
    }
    for(V3DLONG i=0;i<mark_remote_rela2.size();i++)
    {
        each_dis=rad_dist(nt.listNeuron[find_line(nt,mark_remote_rela2[i])],nt.listNeuron[find_line(nt,branches2)]);
        if(each_dis>max_dis2)
        {
            //id_max2=i;
            max_dis2=each_dis;
        }
    }
    if(max_dis1>max_dis2)
    {
        return max_id;
    }
    else if(max_dis1<max_dis2)
    {
        return i;
    }
    else
    {
        cout<<"they are equal"<<endl;
        return max_id;
    }
}
V3DLONG find_root(NeuronTree nt)
{

        NeuronSWC S;
        double min_d=VOID;
        vector<double> d_r;
        int id;
        S.x=0;
        S.y=0;
        S.z=0;
        for(V3DLONG i=0;i<nt.listNeuron.size();i++)
        {
            d_r.push_back(rad_dist(S,nt.listNeuron[i]));
        }
        for(V3DLONG i=0;i<nt.listNeuron.size();i++)
        {
            if(d_r[i]<min_d)
            {
                min_d=d_r[i];
                id=i;
            }
        }
        return nt.listNeuron[id].n;

}
V3DLONG find_root(NeuronTree nt,vector<NeuronSWC> root,vector<V3DLONG> leaf,vector<V3DLONG> branches)
{
    if(branches.size()==0)
    {
        return root[0].n;
    }
    else if(branches.size()==1)
    {
        return nt.listNeuron[find_line(nt,branches[0])].n;
    }
    else
    {

        vector<double> d_t;
        vector<vector<double> > d;
        double sum_dd=0;
        vector<double> sum_d;
        vector<double> D;
        int id;
        double min_d=VOID;
        for(V3DLONG i=0;i<branches.size();i++)
        {
            //cout<<"nnnnnn"<<endl;
            for(V3DLONG j=0;j<leaf.size();j++)
            {
                //cout<<"cccccccccccc"<<endl;
                //d_t.push_back(3.666);
                //d.push_back(d_t);
                d_t.push_back(rad_dist(nt.listNeuron[find_line(nt,branches[i])],nt.listNeuron[find_line(nt,leaf[j])]));


            }
            d.push_back(d_t);
            cout<<"d.size = "<<d.size()<<endl;
        }

        for(V3DLONG i=0;i<branches.size();i++)
        {
            for(V3DLONG k=0;k<d.size();k++)
            {
                sum_dd=sum_dd+d[i][k];

            }

            sum_d.push_back(sum_dd);
            sum_dd=0;
        }

        for(V3DLONG i=0;i<branches.size();i++)
        {
            sum_d[i]=sum_d[i]/d[i].size();
        }
        cout<<"cccccccccccc"<<endl;
        for(V3DLONG i=0;i<branches.size();i++)
        {
            for(V3DLONG k=0;k<d.size();k++)
            {
                sum_dd=sum_dd+(d[i][k]-sum_d[i])*(d[i][k]-sum_d[i]);
            }
            D.push_back(sum_dd);
            sum_dd=0;
        }
        for(V3DLONG i=0;i<D.size();i++)
        {
            if(D[i]<min_d)
            {
                min_d=D[i];
                id = i;
            }
        }
        return nt.listNeuron[find_line(nt,branches[id])].n;
    }
}
double make_r(vector<node_xy>node)
{
    double sum_x=0;
    double sum_y=0;
    double up_line=0;
    double down_line_x=0;
    double down_line_y=0;
    double down_line;
    double r;
    for(V3DLONG i=0;i<node.size();i++)
    {
        sum_x = sum_x + node[i].x;
        sum_y = sum_y + node[i].y;
    }
    sum_x = sum_x/node.size();
    sum_y = sum_y/node.size();
    for(V3DLONG i=0;i<node.size();i++)
    {
        up_line = up_line + (node[i].x-sum_x)*(node[i].y-sum_y);
        down_line_x = down_line_x + (node[i].x-sum_x)*(node[i].x-sum_x);
        down_line_y = down_line_y + (node[i].y-sum_y)*(node[i].y-sum_y);
    }
    down_line = sqrt(down_line_x*down_line_y);
    r = up_line/down_line;
    return r;
}
double make_r_2(vector<node_xy>node)
{
    double sum_x=0;
    double sum_y=0;
    double up_line=0;
    double down_line_x=0;
    double down_line_y=0;
    double down_line;
    double r;
    for(V3DLONG i=0;i<node.size();i++)
    {
        sum_x = sum_x + node[i].id;
        sum_y = sum_y + node[i].y-node[i].x;
    }
    sum_x = sum_x/node.size();
    sum_y = sum_y/node.size();
    for(V3DLONG i=0;i<node.size();i++)
    {
        up_line = up_line + (node[i].id-sum_x)*(node[i].y-node[i].x-sum_y);
        down_line_x = down_line_x + (node[i].id-sum_x)*(node[i].id-sum_x);
        down_line_y = down_line_y + (node[i].y-node[i].x-sum_y)*(node[i].y-node[i].x-sum_y);
    }
    down_line = sqrt(down_line_x*down_line_y);
    r = up_line/down_line;
    return r;
}
bool make_A_path(QHash<V3DLONG,V3DLONG> &A_path,NeuronTree &nt,vector<V3DLONG> &leaf)
{
    for(V3DLONG i=0;i<leaf.size();i++)
    {
        A_path.insert(leaf[i],leaf[i]);
    }
}
void find_par(NeuronTree &nt,V3DLONG first_leaf,vector<vector<V3DLONG> > &childs,QHash<V3DLONG,QSet<V3DLONG> > &temp,V3DLONG del_child,QHash<V3DLONG,QSet<V3DLONG> >&relation,vector<V3DLONG> &save_par,int &count_bb)
{
    //cout<<"************enter into find_par*************************"<<endl;
    V3DLONG r = getParent(del_child,nt);
    if(r!=VOID)
    {
        //V3DLONG r = getParent(del_child,nt);
        if(childs[r].size()==1)
        {
            find_par(nt,first_leaf,childs,temp,getParent(del_child,nt),relation,save_par,count_bb);
        }
        else
        {
            //int g = getParent(del_child,nt);
            cout<<"this is branches1 = "<<r<<endl;
            //save_par.clear();
            save_par.push_back(r);
             for(V3DLONG i=0;i< save_par.size();i++)
             {
                cout<<"save_par = "<<save_par[i]<<endl;
             }
                cout<<"r = "<<r<<endl;
                //cout<<"up_par(nt,save_par[count_bb],childs = "<<up_par(nt,save_par[count_bb],childs)<<endl;
                relation[r].insert(save_par[save_par.size()-2]);     //this is chage
                //temp[r].insert(save_par[save_par.size()-2]);

//                for(V3DLONG i=0;i<count_bb;i++)
//                {
//                    temp[save_par[count_bb]].insert(save_par[save_par.size()-2]);
//                    cout<<"                     save_par = "<<save_par[count_bb]<<endl;
//                    cout<<"                     save_par[save_par.size()-2] = "<<save_par[save_par.size()-2]<<endl;
//                }
                cout<<"count_bb = "<<count_bb<<endl;
                cout<<"save_par[count_bb] = "<<save_par[count_bb]<<endl;
                cout<<"childs[r].size() = "<<childs[r].size()<<endl;
            for(V3DLONG i=0;i<childs[r].size();i++)
            {
                cout<<"i = "<<i<<endl;
                if(childs[r][i]!=del_child)
                {
                    count_bb++;
                    find_child(nt,childs[r][i],childs,save_par,temp,relation,count_bb);
                    cout<<"count_bb="<<count_bb<<endl;
                    count_bb--;
                    //save_par.clear();


                }
            }
            find_par(nt,first_leaf,childs,temp,getParent(del_child,nt),relation,save_par,count_bb);
        }
        //relation[root]
    }
    else
    {
        cout<<"del_child = "<<del_child<<endl;
        cout<<"r = "<<r<<endl;
        //cout<<"down_child(nt,childs,r) = "<<down_child(nt,childs,del_child)<<endl;
        relation[r].insert(down_child(nt,childs,del_child));            //this is chage
//        for(V3DLONG i=0;i<count_bb;i++)
//        {
//            temp[save_par[count_bb]].insert(down_child(nt,childs,del_child));
//            cout<<"                  save_par[count_bb] = "<<save_par[count_bb]<<endl;
//            cout<<"                  down_child(nt,childs,del_child) = "<<down_child(nt,childs,del_child)<<endl;
//        }
        cout<<"zzzzzzzzzzzzzz"<<endl;
        //return;
    }
    //cout<<"*******************out find par*******************"<<endl;

}
V3DLONG down_child(NeuronTree nt,vector<vector<V3DLONG> > childs,V3DLONG node)
{
    int n;
    //cout<<"node = "<<node<<endl;
    if(childs[node].size() == 1)
    {
        n = down_child(nt,childs,childs[node][0]);
    }
    else if(childs[node].size() == 0)
    {
        n = -2;
    }
    else
    {
        //cout<<"node = "<<node<<endl;
        return node;
    }
    return n;
}
void find_child(NeuronTree &nt,V3DLONG par,vector<vector<V3DLONG> > &childs,vector<V3DLONG> &save_par,QHash<V3DLONG,QSet<V3DLONG> > &temp,QHash<V3DLONG,QSet<V3DLONG> >&relation,int &count_b)
{
    //cout<<"************enter into find_child*************************"<<endl;
    //int count_b=0;
    //cout<<"child = "<<par<<endl;
    int c_s = childs[par].size();
    //cout<<"cs = "<<c_s<<endl;
    if(childs[par].size()>1)
    {
        cout<<"this is branches2 = "<<par<<endl;
        //save_par.push_back(par);
        //temp.insert(par);
        relation[find_line(nt,up_par(nt,par,childs))].insert(par);    //this is chage
//        for(V3DLONG i=0;i<count_b+1;i++)
//        {
//            temp[save_par[count_b]].insert(par);
//            cout<<"                     save_par = "<<save_par[count_b]<<endl;
//            cout<<"                     par = "<<par<<endl;
//        }
        //count_b++;
    }
    if(childs[par].size()!=0)
    {
        for(V3DLONG i=0;i<c_s;i++)
        {
            if(childs[par].size()>1)
            {
                //cout<<"this is branches2 = "<<par<<endl;
//                save_par.push_back(par);
//                temp.insert(par);
                count_b++;
                    find_child(nt,childs[par][i],childs,save_par,temp,relation,count_b);
                count_b--;
//                cout<<"count_b = "<<count_b<<endl;
//                cout<<"save_par[count_b] = "<<save_par[count_b]<<endl;
//                cout<<"childs[par][i] = "<<childs[par][i]<<endl;
//                relation[save_par[count_b]].insert(childs[par][i]);
                }
             else
             {
                count_b++;
                find_child(nt,childs[par][i],childs,save_par,temp,relation,count_b);
                count_b--;
             }
        }
    //temp.insert(save_par[count_b]);
    }
    else
    {
        cout<<"this is leaf = "<<par<<endl;
        //temp.insert(par);
        cout<<"count_b = "<<count_b<<endl;
        cout<<"up_par(nt,par,childs) = "<<up_par(nt,par,childs)<<endl;
        relation[find_line(nt,up_par(nt,par,childs))].insert(par);
        //temp.insert(par);
//        for(V3DLONG i=0;i<count_b;i++)
//        {
//            temp[save_par[count_b]].insert(par);
//            cout<<"                     save_par = "<<save_par[count_b]<<endl;
//            cout<<"                     par = "<<par<<endl;
//        }

    }
    //cout<<"*******************out find child*******************"<<endl;
}

V3DLONG find_remote_leaf(NeuronTree nt,vector<V3DLONG>leaf,NeuronSWC root,V3DLONG rd_id,QHash<V3DLONG,double> rd_v)
{
    cout<<"************enter into find_remote_leaf*************************"<<endl;
    double rd,min_rd = VOID;
    NeuronSWC leaf_ln;
    for(V3DLONG i=0;i<leaf.size();i++)
    {

        //cout<<"leafsize = "<<leaf.size()<<endl;
        for(V3DLONG j=0;j<nt.listNeuron.size();j++)
        {
            //cout<<"leaf i = "<<leaf[i]<<endl;
            if(leaf[i] == nt.listNeuron[j].n)
            {
                leaf_ln = nt.listNeuron[j];
            }
        }
        //cout<<"************hahahahah"<<endl;
        rd = rad_dist(leaf_ln,root);
        rd_v.insert(leaf_ln.n,rd);
        cout<<"leaf_ln = "<<leaf_ln.n<<"      "<<"rd = "<<rd<<endl;
        if(min_rd > rd)
        {
            min_rd = rd;
            rd_id = i;
        }
    }
    return leaf[rd_id];
}
V3DLONG find_line(NeuronTree nt,V3DLONG n)
{
    //cout<<"***************enter into find_line**************"<<endl;
    V3DLONG id;
    //cout<<"nt.listNeuron.size() = "<<nt.listNeuron.size()<<endl;
    for(V3DLONG i=0;i<nt.listNeuron.size();i++)
    {
        //cout<<"i = "<<i<<endl;
         if(n == nt.listNeuron[i].n)
         {
             //cout<<nt.listNeuron[i].n<<endl;
             id = i;
             //cout<<"***************out find_line**************"<<endl;
             return id;
         }
    }

}
bool calculate_radial_distance_up(int bin,int count,int root_c,int index,QHash<V3DLONG,V3DLONG> &A_path,vector<V3DLONG> &branches,vector<vector<V3DLONG> > &childs,vector<V3DLONG> &A,NeuronSWC root,NeuronTree &nt,QHash<V3DLONG,double> &f,QHash<V3DLONG,QSet<V3DLONG> >&relation,V3DLONG remote_leaf,vector<node_xy> &node_coord)
{
    cout<<"**********************************enter into c_r_d_up******************"<<endl;
    QHash<int,QHash<int,V3DLONG> > max_it;
    QHash<int,V3DLONG> max_it_t;

    max_it_t.insert(count,remote_leaf);

    max_it.insert(bin,max_it_t);
    QHash<int,QHash<int,V3DLONG> > discard;
    QHash<int,V3DLONG> discard_t;
    cout<<"                remote_leaf = "<<remote_leaf<<endl;
    V3DLONG temp_leaf = remote_leaf;
    discard_t.insert(bin,find_line(nt,remote_leaf));
    discard.insert(bin,discard_t);
    vector <V3DLONG>::iterator Iter;
    for(Iter = A.begin(); Iter != A.end(); Iter++)
    {

        if(temp_leaf == *Iter)
        {
            //cout<<"this is remote leaf = "<<temp_leaf<<endl;
            Iter = A.erase(Iter);
            //cout<<"remote leaf is already removed"<<endl;
            break;
        }
    }
    QHash<int,QHash<int,V3DLONG> > up_b;
    QHash<int,V3DLONG> up_b_t;
    up_b_t.insert(count,find_line(nt,up_par(nt,find_line(nt,remote_leaf),childs)));
    up_b.insert(bin,up_b_t);

    cout<<"<<<<<<<<<<<<<<<<<<<<<<<<<"<<endl;
    QHash<int,QHash<int,V3DLONG> > next_node;
    QHash<int,V3DLONG> next_node_t;
    //next_node_t.insert(count,)
    QHash<int,QHash<int,double> >m_l_r;
    QHash<int,double> m_l_r_t;
    m_l_r_t.insert(count,VOID);
    m_l_r.insert(bin,m_l_r_t);


    //cout<<"up_b[bin][count] = "<<up_b[bin][count]<<endl;
    if(up_b[bin][count]!=find_line(nt,root.n))
    {
        if(if_is_leaf(nt,f,find_line(nt,remote_leaf),up_b[bin][count],relation))
        {
            get_branch_f_of_all(bin,count,A_path,relation,A,branches,nt,find_line(nt,remote_leaf),max_it_t,max_it,up_b[bin][count],root,f,node_coord,discard);
            calculate_radial_distance_up(bin,count+1,root_c,index,A_path,branches,childs,A,root,nt,f,relation,nt.listNeuron[up_b[bin][count]].n,node_coord);
            cout<<"finish_get_branch"<<endl;
            //cout<<"A.size = "<<A.size()<<endl;
        }
        else
        {
            find_next_node(bin,count,relation,f,A,next_node,next_node_t,m_l_r,m_l_r_t);
            cout<<"next_node = "<<next_node[bin][count]<<endl;
            calculate_radial_distance_up(bin,count+1,root_c,index,A_path,branches,childs,A,root,nt,f,relation,next_node[bin][count],node_coord);
        }
    }
    else
    {
        cout<<"remote       leaf = "<<remote_leaf<<endl;
        cout<<"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"<<endl;
        cout<<"up_b[bin][count] = "<<up_b[bin][count]<<endl;
        cout<<"root = "<<root.n<<endl;



        //cout<<"              count = "<<count<<endl;
       // cout<<"              bin = "<<bin<<endl;
       // cout<<"up_b = "<<up_b[bin][count]<<endl;


        cout<<"childs[find_line(nt,root.n)].size() = "<<childs[find_line(nt,root.n)].size()<<endl;
        if(childs[find_line(nt,root.n)].size()>1)
        {
            root_c--;
            cout<<"root_c = "<<root_c<<endl;
            if(root_c != 0)
            {
                if(if_is_leaf(nt,f,find_line(nt,remote_leaf),up_b[bin][count],relation))
                {
                                                                                                                            //problem here
                    get_branch_f_of_all(bin,count,A_path,relation,A,branches,nt,find_line(nt,remote_leaf),max_it_t,max_it,up_b[bin][count],root,f,node_coord,discard);

   //tonight                 //calculate_radial_distance_up(bin,count+1,root_c,index,A_path,branches,childs,A,root,nt,f,relation,nt.listNeuron[up_b[bin][count]].n,node_coord);
                    cout<<"finish_get_branch"<<endl;
                    //cout<<"A.size = "<<A.size()<<endl;

                }
                else
                {
                    cout<<"kkkkkkkkkkkkkkkkkkkkkkkkkk"<<endl;

                    find_next_node(bin,count,relation,f,A,next_node,next_node_t,m_l_r,m_l_r_t);
                    cout<<"next_node = "<<next_node[bin][count]<<endl;
                    calculate_radial_distance_up(bin,count+1,root_c,index,A_path,branches,childs,A,root,nt,f,relation,next_node[bin][count],node_coord);
     //chage here tonight               //get_branch_f_of_all(bin,count,A_path,relation,A,branches,nt,find_line(nt,remote_leaf),max_it_t,max_it,up_b[bin][count],root,f,node_coord,discard);

                }

            }
            else
            {
                cout<<"             @@@@@@@@@@@@@@@@@@@@@@@@@@@@@ index here   "<<endl;
                index=0;
            }


//            int t_id,root_next;
//            for(V3DLONG i=0;i<childs[find_line(nt,root.n)].size();i++)
//            {
//                //cout<<" i = "<<i<<endl;
//                t_id = down_child(nt,childs,childs[find_line(nt,root.n)][i]);
//                //cout<<"MMMMMMMMMMMMMMMMM"<<endl;
//                cout<<"t_id = "<<t_id<<endl;
//                if(t_id==-2)
//                {
//                    //cout<<"mmmmmmmmmmmmmmmmmm"<<endl;
//                    continue;
//                }
//                else
//                {
//                    root_next = t_id;
//                }
//            }
//            V3DLONG temin = A_path[nt.listNeuron[root_next].n];
//            A_path.insert(root.n,temin);
        }
        else
        {
//            cout<<"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"<<endl;
//            cout<<"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"<<endl;
//            cout<<"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"<<endl;

            cout<<"              count = "<<count<<endl;
            cout<<"              bin = "<<bin<<endl;
            cout<<"up_b = "<<up_b[bin][count]<<endl;
            V3DLONG temin = A_path[nt.listNeuron[down_child(nt,childs,up_b[bin][count])].n];
            cout<<"up_b = "<<up_b[bin][count]<<endl;
            cout<<"A_path[down_child(nt,childs,up_b[bin][count])]"<<A_path[nt.listNeuron[down_child(nt,childs,up_b[bin][count])].n]<<endl;
            A_path.clear();
            A_path.insert(root.n,temin);
        }
        cout<<"count = "<<count<<endl;
        if(index==0)
        {
            cout<<"###########################this is get all root###############"<<endl;
            index=1;
            get_branch_f_of_all(bin,count,A_path,relation,A,branches,nt,find_line(nt,remote_leaf),max_it_t,max_it,find_line(nt,root.n),root,f,node_coord,discard);
            cout<<"###########################out of get all root###############"<<endl;
        }
    }
    cout<<"***************c_r_d_up*************"<<endl;
}
bool calculate_radial_distance_down(int bin,int count,QHash<V3DLONG,V3DLONG> &A_path,vector<V3DLONG> &branches,vector<V3DLONG> &A,NeuronSWC root,NeuronTree &nt,QHash<V3DLONG,double> &f,QHash<V3DLONG,QSet<V3DLONG> >&relation,V3DLONG branch_node,vector<node_xy> &node_coord)
{
    cout<<"**********************************enter into c_r_d_down******************"<<endl;
    QHash<int,QHash<int,double> > m_rd;
    QHash<int,double> m_rd_t;
    m_rd_t.insert(count,VOID);
    m_rd.insert(bin,m_rd_t);
    QHash<int,QHash<int,V3DLONG> > m_id;
    QHash<int,QHash<int,V3DLONG> > max_it;
    QHash<int,QHash<int,V3DLONG> > discard;
    QHash<int,V3DLONG> m_id_t;
    QHash<int,V3DLONG> max_it_t;

    for(QSet<V3DLONG>::iterator it =relation[branch_node].begin();it != relation[branch_node].end();it++)
    {
        //m_rd = rad_dist(*it,root);
        if(f[*it] == 0)
        {
            calculate_radial_distance_down(bin+1,count+1,A_path,branches,A,root,nt,f,relation,*it,node_coord);
            cout<<"count = "<<count<<endl;
            count = count-1;

        }
        if(m_rd[bin][count]>f[*it])
        {
            m_rd_t.insert(count,f[*it]);
            m_rd.insert(bin,m_rd_t);
            m_id_t.insert(count,*it);
            m_id.insert(bin,m_id_t);

            //m_id[bin][count].insert(*it);
        }
        //max_it_t.insert(count,m_id[count]);
        max_it_t.insert(count,m_id[bin][count]);
        max_it.insert(bin,max_it_t);
        //get_branch_f_of_all(bin,count,relation,A,branches,nt,find_line(nt,discard[bin][count]),max_it_t,max_it,up_b[bin][count],root,f,node_coord,discard);
        get_branch_f_of_all(bin,count,A_path,relation,A,branches,nt,m_id[bin][count],max_it_t,max_it,branch_node,root,f,node_coord,discard);
    }
    cout<<"***************c_r_d_down*************"<<endl;
}
bool if_is_leaf(NeuronTree &nt,QHash<V3DLONG,double> &f,V3DLONG m_id,V3DLONG branches,QHash<V3DLONG,QSet<V3DLONG> > &relation)
{
    cout<<"******************enter into  if is leaf***************"<<endl;
    cout<<"m_id = "<<m_id<<endl;
    cout<<"branches = "<<branches<<endl;

    for(QSet<V3DLONG>::iterator it =relation[branches].begin();it != relation[branches].end();it++)
    {
        //cout<<"*it = "<<*it<<endl;

        if(m_id != *it)
        {
            cout<<"f[nt.listNeuron[*it].n]!=0"<<f[nt.listNeuron[*it].n]<<endl;
            if(f[nt.listNeuron[*it].n]==0)
            {
                cout<<"there is branches in them"<<endl;
                return false;/////youwenti
            //if_is_leaf[bin][count]=false;
            }
        }

    }
    cout<<"they are leaf"<<endl;
    return true;
}
V3DLONG find_next_node(int bin,int count,QHash<V3DLONG,QSet<V3DLONG> > &relation,QHash<V3DLONG,double> &f,vector<V3DLONG> &A,QHash<int,QHash<int,V3DLONG> > &next_node,QHash<int,V3DLONG> &next_node_t,QHash<int,QHash<int,double> > &m_l_r,QHash<int,double> &m_l_r_t)
{
        cout<<"this is find next node"<<endl;
        next_node.clear();
        next_node_t.clear();
        for(V3DLONG i=0;i<A.size();i++)
        {
            //cout<<"f[A[i]] = "<<f[A[i]]<<endl;
            //cout<<"m_l_r[bin][count] = "<<m_l_r[bin][count]<<endl;
            if( f[A[i]] < m_l_r[bin][count])
            {
                m_l_r_t.insert(count,f[A[i]]);
                m_l_r.insert(bin,m_l_r_t);
                next_node_t.insert(count,A[i]);
                next_node.insert(bin,next_node_t);

            }
            //cout<<"out find next node = "<<next_node[bin][count]<<endl;
        }


}
bool del_relation(NeuronTree &nt,vector<V3DLONG> &branches,QHash<V3DLONG,QSet<V3DLONG> > &relation,V3DLONG discard)
{
    //cout<<"discard = "<<discard<<endl;
    for(V3DLONG i=0;i<relation.size();i++)
    {
        for(QSet<V3DLONG>::iterator it =relation[find_line(nt,branches[i])].begin();it != relation[find_line(nt,branches[i])].end();it++)
        {
            if(discard == *it)
            {
              it = relation[find_line(nt,branches[i])].erase(it);
              //cout<<"del_relat_discard = "<<*it<<endl;
            }

        }
    }
    cout<<"********************************del_relation_done****************"<<endl;
}
bool get_branch_f_of_all(int bin,int count,QHash<V3DLONG,V3DLONG> &A_path,QHash<V3DLONG,QSet<V3DLONG> > &relation,vector<V3DLONG> &A,vector<V3DLONG> &branches,NeuronTree &nt,V3DLONG m_id,QHash<int,V3DLONG> &max_it_t,QHash<int,QHash<int,V3DLONG> > &max_it,V3DLONG branch_node,NeuronSWC root,QHash<V3DLONG,double> &f,vector<node_xy>   &node_coord,QHash<int,QHash<int,V3DLONG> > &discard)
{
    cout<<"********************************get into get_branch_f_of_all"<<endl;
    cout<<"m_id = "<<m_id<<endl;
    cout<<"branch_node = "<<branch_node<<endl;
    for(QSet<V3DLONG>::iterator it =relation[branch_node].begin();it != relation[branch_node].end();it++)
    {

        if(m_id!=*it)
        {
            cout<<"m_id = "<<m_id<<endl;
            cout<<"*it = "<<*it<<endl;
            //cout<<"it = "<<*it<<endl;
            //cout<<"fit = "<<f[nt.listNeuron[*it].n]<<endl;
            if(f[nt.listNeuron[*it].n] != 0)
            {
               // cout<<"max_it = "<<max_it[bin][count]<<endl;
                int get_b_f = get__branch_f(bin,count,A_path,branches,nt,max_it[bin][count],*it,branch_node,root,f,node_coord,discard);
                max_it_t.insert(count,get_b_f);
                max_it.insert(bin,max_it_t);
                //cout<<"max_it = "<<max_it[bin][count]<<endl;
                //max_it[bin][count] = get__branch_f(bin,count,f[*it],f[max_it],,branch_node,f,node_coord,discard);
                //cout<<"before1"<<endl;
                for(V3DLONG i=0;i<A.size();i++)
                {
                    //cout<<"A = "<<A[i]<<endl;
                }
                vector <V3DLONG>::iterator Iter;
                for(Iter = A.begin(); Iter != A.end(); Iter++)
                {
                    if(discard[bin][count] == *Iter)
                    {
                        //cout<<"discard[bin][count] = "<<find_line(nt,discard[bin][count])<<endl;
                        Iter = A.erase(Iter);
                        break;
                    }
                }
                 //cout<<"after1"<<endl;
//                for(V3DLONG i=0;i<A.size();i++)
//                {
//                    cout<<"A = "<<A[i]<<endl;
//                }
            }

            else
            {
                calculate_radial_distance_down(bin+1,count+1,A_path,branches,A,root,nt,f,relation,*it,node_coord);
                //bin--;
                //count--;
            }
        }
    }
    //del_relation(nt,branches,relation,max_it[bin][count]);
    //cout<<"before2"<<endl;
//    for(V3DLONG i=0;i<A.size();i++)
//    {
//        cout<<"A = "<<A[i]<<endl;
//    }
    vector <V3DLONG>::iterator Iter;
    for(Iter = A.begin(); Iter != A.end(); Iter++)
    {
        if(max_it[bin][count] == *Iter)
        {
            cout<<"max_it[bin][count] = "<<find_line(nt,max_it[bin][count])<<endl;
            Iter = A.erase(Iter);
            break;
        }
    }
     //cout<<"after2"<<endl;
//    for(V3DLONG i=0;i<A.size();i++)
//    {
//        cout<<"A = "<<A[i]<<endl;
//    }
cout<<"********************************out get_branch_f_of_all"<<endl;
}

bool make_A(vector<V3DLONG> &A,NeuronTree &nt,vector<V3DLONG> &leaf,QHash<V3DLONG,double> &f,NeuronSWC root)
{
    cout<<"root = "<<root.n<<endl;
    for(V3DLONG i=0;i<leaf.size();i++)
    {
        //cout<<"leaf = "<<leaf[i]<<endl;
        A.push_back(leaf[i]);
        f.insert(leaf[i],rad_dist(nt.listNeuron[find_line(nt,leaf[i])],root));
       // cout<<"rad_dist(bran,root) = "<<rad_dist(nt.listNeuron[find_line(nt,leaf[i])],root)<<endl;
    }
}
V3DLONG get__branch_f(int bin,int count,QHash<V3DLONG,V3DLONG> &A_path,vector<V3DLONG> &branches,NeuronTree &nt,V3DLONG it1,V3DLONG it2,V3DLONG up_b,NeuronSWC root,QHash<V3DLONG,double> &f,vector<node_xy>   &node_coord,QHash<int,QHash<int,V3DLONG> > &discard)
{
    NeuronSWC bran;
    node_xy node_x_y;
    QHash<int,V3DLONG> discard_t;
    for(V3DLONG i=0;i<nt.listNeuron.size();i++)
    {
        if(up_b == i)
        {
            bran = nt.listNeuron[i];
        }
    }
    cout<<"1="<<it1<<endl;
    cout<<"2="<<nt.listNeuron[it2].n<<endl;
 //   cout<<"up_b = "<<up_b<<endl;
    if(f[it1]>f[nt.listNeuron[it2].n])
    {
        cout<<"(1)"<<endl;
        discard_t.insert(count,nt.listNeuron[it2].n);
        discard.insert(bin,discard_t);
        A_path.insert(nt.listNeuron[up_b].n,A_path[it1]);

        //cout<<"nt.listNeuron[up_b].n"<<up_b<<" = "<<nt.listNeuron[up_b].n<<endl;
        //cout<<"A_path[nt.listNeuron[it2].n]"<<nt.listNeuron[it2].n<<" = "<<A_path[nt.listNeuron[it2].n]<<endl;

        f.insert(nt.listNeuron[up_b].n,f[it1]);
        cout<<"f"<<nt.listNeuron[up_b].n<<" = "<<f[it1]<<endl;

        id_n++;
        node_x_y.id = id_n;
        cout<<"id_n = "<<id_n<<endl;
        node_x_y.x = rad_dist(root,nt.listNeuron[up_b]);
        cout<<"root.n = "<<root.n<<endl;
        cout<<"nt.listNeuron[up_b].n = "<<nt.listNeuron[up_b].n<<endl;
        cout<<"A_path[it1] = "<<A_path[it1]<<endl;
        node_x_y.y = f[A_path[nt.listNeuron[it2].n]];
        cout<<"x = "<<node_x_y.x<<endl;
        cout<<"y = "<<node_x_y.y<<endl;

       // node_x_y.x = rad_dist(nt.listNeuron[up_b],root);
       // node_x_y.y = f[A_path[nt.listNeuron[it2].n]];
        node_coord.push_back(node_x_y);
        cout<<"node_coord_size = "<<node_coord.size()<<endl;
        return it1;
    }
    else if(f[it1]<f[nt.listNeuron[it2].n])
    {
        cout<<"(2)"<<endl;
        discard_t.insert(count,it1);
        discard.insert(bin,discard_t);
        A_path.insert(nt.listNeuron[up_b].n,A_path[nt.listNeuron[it2].n]);
        //node_coord.insert(nt.listNeuron[up_b].n,A_path[it1]);
        //cout<<"nt.listNeuron[up_b].n"<<up_b<<" = "<<nt.listNeuron[up_b].n<<endl;
        //cout<<"A_path[it1]"<<it1<<" = "<<A_path[it1]<<endl;
        f.insert(nt.listNeuron[up_b].n,f[nt.listNeuron[it2].n]);


        id_n++;
        node_x_y.id = id_n;
        cout<<"id_n = "<<id_n<<endl;
        node_x_y.x = rad_dist(root,nt.listNeuron[up_b]);
        node_x_y.y = f[A_path[it1]];

        //node_x_y.x = rad_dist(nt.listNeuron[up_b],root);
        //node_x_y.y = f[A_path[it1]];
        cout<<"x = "<<node_x_y.x<<endl;
        cout<<"y = "<<node_x_y.y<<endl;
        node_coord.push_back(node_x_y);
        cout<<"node_coord_size = "<<node_coord.size()<<endl;
        return nt.listNeuron[it2].n;
    }
    else
    {
        cout<<"(3)"<<endl;
        //cout<<"discard_t "<<nt.listNeuron[it2].n<<endl;
        discard_t.insert(count,nt.listNeuron[it2].n);
        discard.insert(bin,discard_t);
        A_path.insert(nt.listNeuron[up_b].n,A_path[it1]);
        f.insert(nt.listNeuron[up_b].n,f[it1]);


        id_n++;
        node_x_y.id = id_n;
        cout<<"id_n = "<<id_n<<endl;
        node_x_y.x = rad_dist(root,nt.listNeuron[up_b]);
        node_x_y.y = f[A_path[nt.listNeuron[it2].n]];
        //node_x_y.x = rad_dist(nt.listNeuron[up_b],root);
        //node_x_y.y = f[A_path[nt.listNeuron[it2].n]];
        cout<<"x = "<<node_x_y.x<<endl;
        cout<<"y = "<<node_x_y.y<<endl;
        node_coord.push_back(node_x_y);
        cout<<"node_coord_size = "<<node_coord.size()<<endl;
        return it1;
    }

}
V3DLONG up_par(NeuronTree &nt,V3DLONG node,vector<vector<V3DLONG> > &childs)
{
    //cout<<"***************enter into up_par**************"<<endl;
    int m;
    V3DLONG p = getParent(node,nt);
    if(p !=VOID)
    {
        if(childs[p].size() == 1)
        {
            m = up_par(nt,p,childs);
        }
        else
        {
        //cout<<"***************out up_par**************"<<endl;
            return nt.listNeuron[p].n;

        }
    }
    else
    {
        //cout<<"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUu"<<endl;
        cout<<nt.listNeuron[node].n<<endl;
        return nt.listNeuron[node].n;
    }
    return m;
}

double rad_dist(NeuronSWC swc1,NeuronSWC swc2)
{
    //cout<<"2 = "<<(swc1.x - swc2.x)*(swc1.x - swc2.x)+(swc1.y - swc2.y)*(swc1.y - swc2.y)+(swc1.z - swc2.z)*(swc1.z - swc2.z)<<endl;
    return sqrt((swc1.x - swc2.x)*(swc1.x - swc2.x)+(swc1.y - swc2.y)*(swc1.y - swc2.y)+(swc1.z - swc2.z)*(swc1.z - swc2.z));
}
void printHelp_createTMD()
{
    printf("\nBlastNeuron Plugin - Neuron Comparison: given a query neuron, retrieve certain number of candidates from a database. by Yinan Wan.\n\n");
    printf("Usage:\n");
    printf("\t #d <featurebase_file>         input neuron featurebase file (.nfb) which contains neuron swc file names, morphological and invariant moment feature\n");
    printf("\t #q <query_file>               query neuron file (.swc)\n");
    printf("\t #n <candidate_number>         number of candidates you want to retrive.\n");
    printf("\t #o <output_file>              name of the output file, which will be a swc linker file.\n");
    printf("\t                               default result will be generated under the same directory of the query file and has a name of 'queryFileName_retrieved.ano'.\n");
    printf("\t #m <normalization_method>     feature computation and normalization  method\n");
    printf("\t                               m=1: global morphological feature; m=2: moment feature. m=3: whitening normalization; m=4: rankscore normalization\n");
    printf("\t                               use ',' to split if you want to combine: e.g. m=1,2,4 means you use morph & gmi features, together with rankScore norm\n");
    printf("\t                               [default]: m=1,2,3,4\n");
    printf("\t #h                            print this message.\n\n");
    printf("Example: vaa3d -x blastneuron -f global_retrieve -p \"#d myfeaturebase.nfb #q query.swc #n 10 #o result.ano #m 1,2,4\"\n\n");
}



