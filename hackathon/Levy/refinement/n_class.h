#ifndef N_CLASS_H
#define N_CLASS_H


#include <v3d_interface.h>
#include <vector>
#include <math.h>
#include <iostream>
using namespace std;

#define IN 1000000000

#define zx_dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))

template<class T>
inline double norm_v(T &vector0)
{
    return sqrt(vector0.x*vector0.x+vector0.y*vector0.y+vector0.z*vector0.z);
}

template<class T>
inline double dot_vv(T &vector1,T &vector2)
{
    return (vector1.x*vector2.x+vector1.y*vector2.y+vector1.z*vector2.z);
}

template<class T1,class T2>
inline double p_to_line(T1 &point1,T1 &point2,T1 &point3)
{
    T2 a,c;
    a.x=point1.x-point2.x;
    a.y=point1.y-point2.y;
    a.z=point1.z-point2.z;

    c.x=point3.x-point2.x;
    c.y=point3.y-point2.y;
    c.z=point3.z-point2.z;

    double a_norm=norm_v(a);
    double c_norm=norm_v(c);

    double angle_ac=angle_three_point(point2,point1,point3);

    double sin_ac=sin(angle_ac);
    double cos_ac=cos(angle_ac);

    if(a_norm*cos_ac>c_norm||cos_ac<0)
    {
        double d1=distance_two_point(point1,point2);
        double d2=distance_two_point(point1,point3);
        return (d1>d2)?d1:d2;
    }

    return a_norm*sin_ac;
}

template<class T>
inline double distance_two_point(T &point1,T &point2)
{
    return sqrt(((double)point1.x-(double)point2.x)*((double)point1.x-(double)point2.x)+((double)point1.y-(double)point2.y)*((double)point1.y-(double)point2.y)+((double)point1.z-(double)point2.z)*((double)point1.z-point2.z));
}

template<class T>
inline double angle_three_point(T &point1,T &point2,T &point3)
{
    double x1=point2.x-point1.x;
    double y1=point2.y-point1.y;
    double z1=point2.z-point1.z;

    double x2=point3.x-point1.x;
    double y2=point3.y-point1.y;
    double z2=point3.z-point1.z;

    return acos((x1*x2+y1*y2+z1*z2)/(distance_two_point(point1,point2)*distance_two_point(point1,point3)));
}

template<class T>
inline double x_angle_two_point(T &point1,T &point2)
{
    double xy_d=sqrt((point1.x-point2.x)*(point1.x-point2.x)+(point1.y-point2.y)*(point1.y-point2.y));
    double x_d=point2.x-point1.x;
    return acos(x_d/xy_d);
}

template<class T>
inline double z_angle_two_point(T &point1,T &point2)
{
    return acos((point2.z-point1.z)/distance_two_point(point1,point2));
}

struct Angle{
    double x,y,z;
    Angle():x(0),y(0),z(0){}
    Angle(double x,double y,double z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    Angle norm_angle()
    {
        double s = norm_v(*this);
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }
    double operator *(const Angle &other) const
    {
        return (x*other.x+y*other.y+z*other.z);
    }
};

struct Trunk{
    int n;
    vector<int> branch_index;
    vector<double> length;
    int parent;
    Angle head_angle,end_angle;
    bool operator <(const Trunk &other) const
    {
        return (n>other.n);
    }
};

struct Branch{
    NeuronSWC head_point,end_point;
    vector<NeuronSWC> allpoints;
    int type;
    int flag_near_dendrite=0;
    Branch* parent;
    Branch* child_a;
//    Branch* child_b;
    int child_num=0;
    Angle head_angle,end_angle;
    int level;
    double distance,length,length_to_soma,sum_angle;
    Branch():head_angle(),end_angle(),level(0),distance(0),length(0),head_point(),end_point()
    {
        //head_point=0;
        //end_point=0;
        parent=0;
    }
    ~Branch()
    {
        //if(head_point) delete head_point;
        //if(end_point) delete end_point;
        //if(parent) delete parent;
    }
    inline double get_distance()
    {
        return distance_two_point(head_point,end_point);
    }

    bool operator <(const Branch &other) const
    {
        if(level!=other.level)
            return (level>other.level);
        return (length<other.length);
    }

    bool get_points_of_branch(vector<NeuronSWC> &points,NeuronTree &nt);
    bool get_r_points_of_branch(vector<NeuronSWC> &r_points,NeuronTree &nt);

    bool refine_branch(vector<NeuronSWC> &points, QString braindir, V3DPluginCallback2 &callback, NeuronTree &nt);

    bool get_meanstd(QString braindir, V3DPluginCallback2 &callback, NeuronTree &nt, double &branchmean, double &branchstd, int mode = 0, int resolution = 3);
    bool get_meanstd_img(string inimg_file, V3DPluginCallback2 &callback, NeuronTree &nt, double &branchmean, double &branchstd, int mode = 0, int resolution = 3);
    bool splitbranch(NeuronTree& nt, vector<Branch> &segs, double thres);

    bool refine_by_gd(vector<LocationSimple> points, vector<LocationSimple> &outpoints, QString braindir, V3DPluginCallback2 &callback);
    bool refine_by_gd_img(vector<LocationSimple> points, vector<LocationSimple> &outpoints, string inimg_file, V3DPluginCallback2 &callback);
    bool refine_by_2gd(vector<LocationSimple> &outbranch, QString braindir, V3DPluginCallback2 &callback, NeuronTree &nt, double thres,vector<int> &neuron_type);
    bool refine_by_2gd_img(vector<LocationSimple> &outbranch, string inimg_file, V3DPluginCallback2 &callback, NeuronTree &nt, double thres,vector<int> &neuron_type);
    bool refine_branch_by_gd(LocationSimple points, LocationSimple p_childa, LocationSimple p_childb,NeuronTree &nt_gd, QString braindir, V3DPluginCallback2 &callback);
};

struct SwcTree{
    vector<Branch> branchs;
    vector<Trunk> trunks;
    NeuronTree nt;
    SwcTree():nt()
    {
        branchs.clear();
        trunks.clear();
    }
    bool initialize(NeuronTree t);
    void inline display()
    {
        cout<<"branch<<<<<<<<<<<<<<<<<<<<<<<<<<"<<endl;
        for(int i=0;i<branchs.size();++i)
        {
            cout<<"level: "<<branchs[i].level<<" "
               <<"distance: "<<branchs[i].distance<<" "<<"length: "
              <<branchs[i].length<<" "<<"head_angle: "<<branchs[i].head_angle.x
             <<" "<<branchs[i].head_angle.y<<" "<<branchs[i].head_angle.z<<" "<<"end_angle: "<<branchs[i].end_angle.x
            <<" "<<branchs[i].end_angle.y<<" "<<branchs[i].end_angle.z<<endl;
        }
        cout<<"trunk<<<<<<<<<<<<<<<<<<<<<<<<<<<"<<endl;
        for(int i=0;i<trunks.size();++i)
        {
            cout<<"branch count: "<<trunks[i].branch_index.size()<<" length: "<<trunks[i].length[trunks[i].length.size()-1]
               <<"head_angle: "<<trunks[i].head_angle.x
              <<" "<<trunks[i].head_angle.y<<" "<<trunks[i].head_angle.z<<" "<<"end_angle: "<<trunks[i].end_angle.x
             <<" "<<trunks[i].end_angle.y<<" "<<trunks[i].end_angle.z<<endl;
        }
    }
    bool branchs_to_nt(vector<int> more);
    bool cut_cross();
    bool find_big_turn();
    bool get_level_index(vector<int> &level_index,int level);
    bool get_points_of_branchs(vector<Branch> &b, vector<NeuronSWC> &points, NeuronTree &ntb);
    int get_max_level();
    // added by DZC 19 Jul 2019
//    NeuronTree refine_swc(QString braindir, double thresh, V3DPluginCallback2 &callback);
//    bool gd_on_nt(NeuronTree &branch_nt, NeuronTree & tree_out,QString braindir,V3DPluginCallback2 &callback);

    NeuronTree refine_swc_by_bdb(QString braindir, V3DPluginCallback2 &callback);

    bool test(QString braindir, V3DPluginCallback2 &callback);

    NeuronTree refine_swc_by_gd(QString braindir, V3DPluginCallback2 &callback);
    NeuronTree refine_bifurcation_by_gd(QString braindir, V3DPluginCallback2 &callback,QString eswcfile);
    NeuronTree refine_swc_by_gd_img(string inimg_file, V3DPluginCallback2 &callback);
    NeuronTree refine_swc_branch_by_gd_img(string inimg_file, V3DPluginCallback2 &callback);

};

class Swc_Compare{
public:
    int get_corresponding_branch(Branch &a, vector<int> &b_index, SwcTree &b,vector<bool> &flag_b);
    double get_distance_two_branch(Branch &a,Branch &b,NeuronTree &nta, NeuronTree &ntb);

    double get_distance_two_branchs(vector<Branch> &a,vector<Branch> &b,NeuronTree &nta, NeuronTree &ntb);

    double get_distance_branchs_to_branch(vector<Branch> &a, Branch &b, NeuronTree &nta, NeuronTree &ntb, map<Branch,int> map_b, map<int, int> &map_b_a);

    double get_distance_for_manual(Branch &auto_branch,NeuronTree &nta, NeuronTree &ntb);

    bool compare_two_swc(SwcTree &a, SwcTree &b, vector<int> &a_false, vector<int> &b_false, vector<int> &a_more, vector<int> &b_more, NeuronTree &nta, NeuronTree &ntb, QString dir_a , QString dir_b, QString braindir , V3DPluginCallback2 &callback);

    bool compare_two_swc(SwcTree &a, SwcTree &b, vector<int> &a_trunk, vector<int> &b_trunk, vector<int> &a_more, vector<int> &b_more, NeuronTree &nta, NeuronTree &ntb,int mode);

    bool get_sub_image(QString dir, vector<int> &false_index, SwcTree &t, SwcTree &t0, QString braindir, V3DPluginCallback2 &callback, int mode, map<int, int> &map_branch);

    bool get_sub_image(QString dir,vector<int> &a_false,vector<int> &b_false,SwcTree &a,SwcTree &b,QString braindir,V3DPluginCallback2 &callback);

    bool _get_sub_image(QString filename, vector<NeuronSWC> manual_points, vector<NeuronSWC> auto_points, vector<NeuronSWC> false_points, QString braindir, V3DPluginCallback2 &callback);

    bool get_sub_false_trunk_image(QString dir,vector<int> &manual_false,vector<int> auto_false,SwcTree &manual_t,SwcTree &auto_t,QString braindir,V3DPluginCallback2 &callback);

    bool get_false_point_image(QString dir, vector<int> & more, SwcTree & a_tree, SwcTree & b_tree, V3DPluginCallback2 &callback, QString braindir, bool manual);

    bool crop_swc(NeuronTree &nt_in, NeuronTree &nt_out, int type, size_t x0, size_t x1, size_t y0, size_t y1, size_t z0, size_t z1);

    double get_distance_branchs_to_point(vector<Branch> &a,NeuronSWC &b,NeuronTree &nta);

    bool global_compare(SwcTree & a_tree, SwcTree & b_tree, QString braindir, QString outdir, V3DPluginCallback2 &callback);

    bool get_accurate_false_point_image(QString dir, vector<int> & false_branches, SwcTree &a_tree, SwcTree &b_tree, V3DPluginCallback2 &callback, QString braindir, bool manual);

};

float get_feature_extraction_rate(string inimg_file,NeuronTree &nt,V3DPluginCallback2 &callback);
void  compute_image_snr(QString infolder_1,QString infolder_2,QString SNr_out, V3DPluginCallback2 &callback);
void  compare_2swc_change(NeuronTree &nt1,NeuronTree &nt2,V3DPluginCallback2 &callback,QString out_path);
NeuronSWC meanshift(V3DPluginCallback2 &callback, NeuronSWC p0, QString braindir);
NeuronSWC meanshift_img(V3DPluginCallback2 &callback, NeuronSWC p0,QString braindir);

bool sortSWC(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result, NeuronSWC s);

bool sortSWC(NeuronTree& nt, NeuronSWC s);
















#endif // N_CLASS_H
