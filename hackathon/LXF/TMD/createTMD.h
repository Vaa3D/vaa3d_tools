#ifndef CREATETMD_H
#define CREATETMD_H
using namespace std;
//bool createTMD(const V3DPluginArgList & input, V3DPluginArgList & output);
//bool calculate_radial_distance_up(int bin,int count,QHash<V3DLONG,V3DLONG> &A_path,vector<V3DLONG> &branches,vector<vector<V3DLONG> > &childs,vector<V3DLONG> &A,NeuronSWC root,NeuronTree &nt,QHash<V3DLONG,double> &f,QHash<V3DLONG,QSet<V3DLONG> >&relation,V3DLONG remote_leaf,QHash<V3DLONG,V3DLONG> &node_coord);
//bool calculate_radial_distance_down(int bin,int count,QHash<V3DLONG,V3DLONG> &A_path,vector<V3DLONG> &branches,vector<V3DLONG> &A,NeuronSWC root,NeuronTree &nt,QHash<V3DLONG,double> &f,QHash<V3DLONG,QSet<V3DLONG> >&relation,V3DLONG branch_node,QHash<V3DLONG,V3DLONG> &node_coord);
//double rad_dist(NeuronSWC swc1,NeuronSWC swc2);
//bool make_A_path(QHash<V3DLONG,V3DLONG> &A_path,NeuronTree &nt,vector<V3DLONG> &leaf);
//V3DLONG down_child(NeuronTree nt,vector<vector<V3DLONG> > childs,V3DLONG node);
//bool if_is_leaf(NeuronTree &nt,QHash<V3DLONG,double> &f,V3DLONG m_id,V3DLONG branches,QHash<V3DLONG,QSet<V3DLONG> > &relation);
//V3DLONG find_line(NeuronTree nt,V3DLONG n);
//V3DLONG find_next_node(int bin,int count,QHash<V3DLONG,QSet<V3DLONG> > &relation,QHash<V3DLONG,double> &f,vector<V3DLONG> &A,QHash<int,QHash<int,V3DLONG> > &next_node,QHash<int,V3DLONG> &next_node_t,QHash<int,QHash<int,double> > &m_l_r,QHash<int,double> &m_l_r_t);
//bool get_branch_f_of_all(int bin,int count,QHash<V3DLONG,V3DLONG> &A_path,QHash<V3DLONG,QSet<V3DLONG> > &relation,vector<V3DLONG> &A,vector<V3DLONG> &branches,NeuronTree &nt,V3DLONG m_id,QHash<int,V3DLONG> &max_it_t,QHash<int,QHash<int,V3DLONG> > &max_it,V3DLONG branch_node,NeuronSWC root,QHash<V3DLONG,double> &f,QHash<V3DLONG,V3DLONG> &node_coord,QHash<int,QHash<int,V3DLONG> > &discard);
//V3DLONG get__branch_f(int bin,int count,QHash<V3DLONG,V3DLONG> &A_path,vector<V3DLONG> &branches,NeuronTree &nt,V3DLONG it1,V3DLONG it2,V3DLONG up_b,NeuronSWC root,QHash<V3DLONG,double> &f,QHash<V3DLONG,V3DLONG> &node_coord,QHash<int,QHash<int,V3DLONG> > &discard);
//bool make_A(vector<V3DLONG> &A,NeuronTree &nt,vector<V3DLONG> &leaf,QHash<V3DLONG,double> &f,NeuronSWC root);
//V3DLONG up_par(NeuronTree &nt,V3DLONG node,vector<vector<V3DLONG> > &childs);
//V3DLONG find_remote_leaf(NeuronTree nt,vector<V3DLONG>leaf,NeuronSWC root,V3DLONG rd_id,QHash<V3DLONG,double> rd_v);
//void find_child(NeuronTree &nt,V3DLONG par,vector<vector<V3DLONG> > &childs,vector<V3DLONG> &save_par,QSet<V3DLONG> &temp,QHash<V3DLONG,QSet<V3DLONG> >&relation,int &count_b);
//void find_par(NeuronTree &nt,V3DLONG first_leaf,vector<vector<V3DLONG> > &childs,QSet<V3DLONG> &temp,V3DLONG del_child,QHash<V3DLONG,QSet<V3DLONG> >&relation,vector<V3DLONG> &save_par,int &count_bb);
void printHelp_createTMD();
#endif // CREATETMD_H
